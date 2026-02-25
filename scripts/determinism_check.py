#!/usr/bin/env python3
import argparse
import pathlib
import subprocess
import sys


def run_once(command: list[str], corpus: pathlib.Path) -> bytes:
    process = subprocess.run(
        command + [str(corpus)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if process.returncode != 0:
        raise RuntimeError(process.stdout.decode("utf-8", errors="replace"))
    return process.stdout


def main() -> int:
    parser = argparse.ArgumentParser(description="Run corpus twice and require byte-identical output.")
    parser.add_argument("--runner", default="scripts/run_corpus.py", help="Runner script")
    parser.add_argument("--binary", default="build/ivy_verify", help="Verifier binary path")
    parser.add_argument("--corpus", default="TestData", help="Corpus root")
    args = parser.parse_args()

    corpus = pathlib.Path(args.corpus)
    runner = pathlib.Path(args.runner)
    binary = pathlib.Path(args.binary)

    command = [sys.executable, str(runner), "--binary", str(binary)]

    first = run_once(command, corpus)
    second = run_once(command, corpus)

    if first != second:
        print("Determinism check failed: outputs differ.", file=sys.stderr)
        return 1

    print("Determinism check passed: byte-identical outputs.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
