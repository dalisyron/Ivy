#!/usr/bin/env python3
import argparse
import pathlib
import statistics
import subprocess
import time


def list_imp_files(root: pathlib.Path) -> list[pathlib.Path]:
    return sorted(path for path in root.rglob("*.imp") if path.is_file())


def run_corpus(command_prefix: list[str], files: list[pathlib.Path], fail_on_error: bool) -> float:
    start = time.perf_counter()
    for imp_file in files:
        process = subprocess.run(
            command_prefix + [str(imp_file)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if process.returncode != 0 and fail_on_error:
            raise RuntimeError(f"Command failed for {imp_file}: {' '.join(command_prefix)}")
    return time.perf_counter() - start


def median_runtime(command_prefix: list[str], files: list[pathlib.Path], runs: int, fail_on_error: bool) -> float:
    samples = [run_corpus(command_prefix, files, fail_on_error) for _ in range(runs)]
    return statistics.median(samples)


def main() -> int:
    parser = argparse.ArgumentParser(description="Benchmark Java vs C++ full corpus runtime.")
    parser.add_argument("--corpus", default="TestData")
    parser.add_argument("--runs", type=int, default=5)
    parser.add_argument(
        "--java-cmd",
        default="java -cp lib/com.microsoft.z3.jar:lib/antlr4-runtime-4.13.2.jar:target/classes verification.ImpVerifier",
    )
    parser.add_argument("--cpp-cmd", default="build/ivy_verify")
    parser.add_argument("--fail-on-error", action="store_true")
    args = parser.parse_args()

    files = list_imp_files(pathlib.Path(args.corpus))
    if not files:
        raise RuntimeError("No .imp files found for benchmark")

    java_cmd = args.java_cmd.split()
    cpp_cmd = args.cpp_cmd.split()

    java_median = median_runtime(java_cmd, files, args.runs, args.fail_on_error)
    cpp_median = median_runtime(cpp_cmd, files, args.runs, args.fail_on_error)
    ratio = cpp_median / java_median if java_median > 0 else float("inf")

    print(f"Java median: {java_median:.6f}s")
    print(f"C++  median: {cpp_median:.6f}s")
    print(f"Ratio (C++/Java): {ratio:.3f}x")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
