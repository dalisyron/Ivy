# Ivy (C++ Rewrite)

Ivy is a verifier for the Imp language. This repository now contains:

- Legacy Java baseline (unchanged, used for parity reference)
- Modern C++20 implementation (`Ivy::ivy` library + `ivy_verify` CLI)

## Prerequisites

- CMake >= 3.24
- C++20 compiler (`g++`, `clang++`, or AppleClang)
- Java runtime (for ANTLR code generation)
- Z3 development package (`z3::libz3` CMake package)
- ANTLR jar at `/Users/nibom/v23/antlr-4.13.2-complete.jar`

## Build (C++)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run CLI

```bash
./build/ivy_verify TestData/Examples/Abs.imp
```

Expected output format:

```text
Method Abs is valid
```

## Run Tests (C++)

```bash
ctest --test-dir build --output-on-failure
```

## Quality gates (local)

Format:

```bash
FILES=$(git ls-files '*.hpp' '*.cpp')
clang-format --dry-run --Werror $FILES
```

Static analysis (`clang-tidy`):

```bash
cmake -S . -B build-tidy -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-tidy --target ivy_antlr_gen -j2
SDKROOT=$(xcrun --show-sdk-path)
/opt/homebrew/opt/llvm/bin/clang-tidy -p build-tidy --extra-arg=-isysroot --extra-arg=$SDKROOT src/*.cpp
```

Coverage gate (parser/typechecker/verifier):

```bash
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DIVY_ENABLE_COVERAGE=ON
cmake --build build-cov -j2
find build-cov -name '*.gcda' -delete
ctest --test-dir build-cov --output-on-failure
scripts/coverage_gate.py --build-dir build-cov --line-threshold 85 --branch-threshold 70
```

## Install and consume as package

```bash
cmake --install build --prefix /tmp/ivy-install
```

Then consume target `Ivy::ivy` from `/tmp/ivy-install/lib/cmake/Ivy`.

## External sample app (installed package)

A standalone sample consumer is included and links against the installed/public API target:

```bash
cmake -S examples/external_consumer -B /tmp/ivy-ext-build -DCMAKE_PREFIX_PATH=/tmp/ivy-install
cmake --build /tmp/ivy-ext-build -j
/tmp/ivy-ext-build/ivy_external_consumer
```

## Java baseline commands (for parity)

```bash
mvn test
java -cp lib/com.microsoft.z3.jar:lib/antlr4-runtime-4.13.2.jar:target/classes verification.ImpVerifier TestData/Examples/Abs.imp
```

## Parity / determinism / benchmark scripts

```bash
python3 scripts/parity_check.py --corpus TestData --cpp-cmd build/ivy_verify
python3 scripts/determinism_check.py --binary build/ivy_verify --corpus TestData
python3 scripts/benchmark_corpus.py --corpus TestData --cpp-cmd build/ivy_verify
```

## Fuzz target (60s)

```bash
cmake -S . -B build-fuzz -DIVY_ENABLE_FUZZ=ON
cmake --build build-fuzz -j
scripts/run_parser_fuzz.sh build-fuzz/ivy_parser_fuzz
```
