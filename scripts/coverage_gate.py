#!/usr/bin/env python3
"""Enforce Ivy module coverage thresholds using gcov output.

This gate targets core production modules:
  - src/parser.cpp
  - src/type_checker.cpp
  - src/verifier.cpp

For each file we extract:
  - line coverage       (gcov: "Lines executed")
  - branch coverage     (gcov: "Branches executed")
  - branch taken metric (gcov: "Taken at least once", informational)
"""

from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass


@dataclass(frozen=True)
class FileCoverage:
    path: str
    lines_executed: float
    branches_executed: float
    branches_taken_once: float


_FILE_RE = re.compile(r"^File '(.+)'$")
_LINES_RE = re.compile(r"^Lines executed:([0-9.]+)% of \d+$")
_BRANCHES_EXEC_RE = re.compile(r"^Branches executed:([0-9.]+)% of \d+$")
_BRANCHES_TAKEN_RE = re.compile(r"^Taken at least once:([0-9.]+)% of \d+$")


def parse_gcov_metrics(gcov_output: str, source_path: pathlib.Path) -> FileCoverage:
    target = str(source_path.resolve())
    current_file: str | None = None
    lines_executed: float | None = None
    branches_executed: float | None = None
    branches_taken_once: float | None = None

    for raw_line in gcov_output.splitlines():
        line = raw_line.strip()

        file_match = _FILE_RE.match(line)
        if file_match:
            current_file = file_match.group(1)
            continue

        if current_file != target:
            continue

        if lines_executed is None:
            lines_match = _LINES_RE.match(line)
            if lines_match:
                lines_executed = float(lines_match.group(1))
                continue

        if branches_executed is None:
            branches_match = _BRANCHES_EXEC_RE.match(line)
            if branches_match:
                branches_executed = float(branches_match.group(1))
                continue

        if branches_taken_once is None:
            taken_match = _BRANCHES_TAKEN_RE.match(line)
            if taken_match:
                branches_taken_once = float(taken_match.group(1))
                continue

        if (
            lines_executed is not None
            and branches_executed is not None
            and branches_taken_once is not None
        ):
            break

    if lines_executed is None or branches_executed is None or branches_taken_once is None:
        raise RuntimeError(f"Could not parse gcov metrics for {source_path}")

    return FileCoverage(
        path=str(source_path),
        lines_executed=lines_executed,
        branches_executed=branches_executed,
        branches_taken_once=branches_taken_once,
    )


def run_gcov_for_file(repo_root: pathlib.Path, build_dir: pathlib.Path, source_path: pathlib.Path) -> FileCoverage:
    gcda_path = build_dir / "CMakeFiles" / "ivy.dir" / source_path.with_suffix(".cpp.gcda")
    if not gcda_path.exists():
        raise RuntimeError(f"Coverage data missing: {gcda_path}")

    # Run gcov in a temp directory so generated *.gcov files do not pollute the repo.
    with tempfile.TemporaryDirectory(prefix="ivy-gcov-") as temp_dir:
        process = subprocess.run(
            ["gcov", "-b", "-c", str(gcda_path)],
            cwd=temp_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )

    if process.returncode != 0:
        raise RuntimeError(f"gcov failed for {source_path}:\n{process.stdout}")

    return parse_gcov_metrics(process.stdout, repo_root / source_path)


def main() -> int:
    parser = argparse.ArgumentParser(description="Enforce Ivy module coverage thresholds.")
    parser.add_argument("--build-dir", default="build-cov", help="Coverage build directory")
    parser.add_argument("--line-threshold", type=float, default=85.0, help="Minimum line coverage (%)")
    parser.add_argument(
        "--branch-threshold",
        type=float,
        default=70.0,
        help="Minimum branch-executed coverage (%)",
    )
    args = parser.parse_args()

    repo_root = pathlib.Path.cwd()
    build_dir = (repo_root / args.build_dir).resolve()
    targets = [
        pathlib.Path("src/parser.cpp"),
        pathlib.Path("src/type_checker.cpp"),
        pathlib.Path("src/verifier.cpp"),
    ]

    try:
        results = [run_gcov_for_file(repo_root, build_dir, target) for target in targets]
    except RuntimeError as error:
        print(str(error), file=sys.stderr)
        return 1

    print("Coverage gate (core modules):")
    min_lines = 100.0
    min_branches = 100.0
    for result in results:
        print(
            f"  {result.path}: "
            f"line={result.lines_executed:.2f}% "
            f"branch_executed={result.branches_executed:.2f}% "
            f"(taken_once={result.branches_taken_once:.2f}%)"
        )
        min_lines = min(min_lines, result.lines_executed)
        min_branches = min(min_branches, result.branches_executed)

    print(
        f"Thresholds: line>={args.line_threshold:.2f}% "
        f"branch_executed>={args.branch_threshold:.2f}%"
    )
    print(f"Observed minima: line={min_lines:.2f}% branch_executed={min_branches:.2f}%")

    if min_lines < args.line_threshold or min_branches < args.branch_threshold:
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
