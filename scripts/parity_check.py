#!/usr/bin/env python3
import argparse
import pathlib
import re
import subprocess
import sys
from typing import Dict, Tuple

RESULT_RE = re.compile(r"Method\s+(?P<name>\S+)\s+is\s+(?P<status>valid|invalid)")


def run_verifier(command: list[str], imp_file: pathlib.Path) -> Tuple[Dict[str, str], str, int]:
    process = subprocess.run(
        command + [str(imp_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    output = process.stdout
    results: Dict[str, str] = {}
    for line in output.splitlines():
        match = RESULT_RE.search(line)
        if match:
            results[match.group("name")] = match.group("status")

    return results, output, process.returncode


def find_imp_files(root: pathlib.Path) -> list[pathlib.Path]:
    return sorted(path for path in root.rglob("*.imp") if path.is_file())


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare Java and C++ verifier outcomes.")
    parser.add_argument("--corpus", default="TestData", help="Corpus root directory")
    parser.add_argument(
        "--java-cmd",
        default="java -cp lib/com.microsoft.z3.jar:lib/antlr4-runtime-4.13.2.jar:target/classes verification.ImpVerifier",
        help="Java verifier command prefix",
    )
    parser.add_argument("--cpp-cmd", default="build/ivy_verify", help="C++ verifier command prefix")
    args = parser.parse_args()

    corpus = pathlib.Path(args.corpus)
    if not corpus.exists():
        print(f"Corpus path does not exist: {corpus}", file=sys.stderr)
        return 1

    imp_files = find_imp_files(corpus)
    if not imp_files:
        print(f"No .imp files found under {corpus}", file=sys.stderr)
        return 1

    java_command = args.java_cmd.split()
    cpp_command = args.cpp_cmd.split()

    mismatches = []
    for imp_file in imp_files:
        java_results, java_output, java_rc = run_verifier(java_command, imp_file)
        cpp_results, cpp_output, cpp_rc = run_verifier(cpp_command, imp_file)

        java_success = java_rc == 0
        cpp_success = cpp_rc == 0
        if java_success != cpp_success or java_results != cpp_results:
            mismatches.append(
                (imp_file, java_success, cpp_success, java_results, cpp_results, java_output, cpp_output))

    if mismatches:
        print(f"Found {len(mismatches)} mismatch(es).")
        for imp_file, java_success, cpp_success, java_results, cpp_results, java_output, cpp_output in mismatches:
            print(f"\n== {imp_file}")
            print(f"Java success: {java_success}")
            print(f"C++ success : {cpp_success}")
            print(f"Java: {java_results}")
            print(f"C++ : {cpp_results}")
            print("-- Java output --")
            print(java_output.strip())
            print("-- C++ output --")
            print(cpp_output.strip())
        return 1

    print(f"Parity check passed on {len(imp_files)} files. 0 mismatches.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
