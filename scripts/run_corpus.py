#!/usr/bin/env python3
import argparse
import pathlib
import subprocess
import sys


def list_imp_files(root: pathlib.Path) -> list[pathlib.Path]:
    return sorted(path for path in root.rglob("*.imp") if path.is_file())


def main() -> int:
    parser = argparse.ArgumentParser(description="Run verifier over all .imp files and stream deterministic output")
    parser.add_argument("corpus", nargs="?", default="TestData")
    parser.add_argument("--binary", default="build/ivy_verify")
    parser.add_argument("--fail-on-error", action="store_true", help="Return non-zero if any file fails")
    args = parser.parse_args()

    corpus = pathlib.Path(args.corpus)
    binary = pathlib.Path(args.binary)

    files = list_imp_files(corpus)
    if not files:
        print(f"No .imp files found under {corpus}", file=sys.stderr)
        return 1

    had_failure = False
    for imp_file in files:
        process = subprocess.run(
            [str(binary), str(imp_file)],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
        print(f"== {imp_file}")
        print(process.stdout.rstrip())
        if process.returncode != 0:
            had_failure = True
            print(f"[exit-code] {process.returncode}")
    if had_failure and args.fail_on_error:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
