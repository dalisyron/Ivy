# Changelog

## Unreleased

### Added
- Portfolio documentation and migration traceability updates.

### Fixed
- CI GCC warning gate failures from `-Wshadow` in parameterized test-name generators.
- Java parity/benchmark runner configuration to include the native Z3 Java library path in CI.

### Changed
- CI build parallelism defaults to `-j1` on heavy jobs to reduce memory pressure and runner instability.
- Benchmark gate behavior to keep Java sanity validation while tolerating expected invalid corpus inputs.

## v1-portfolio-quality

### Added
- Architecture note and migration log mapping Java modules to C++ modules and commits.
- README quality-gate commands covering build, tests, parity, determinism, benchmark, fuzz, format, tidy, and coverage.
- External consumer sample using installed `Ivy::ivy` package target.

## v0-parity

### Added
- Corpus parity, determinism, and benchmark scripts:
  - `scripts/parity_check.py`
  - `scripts/determinism_check.py`
  - `scripts/benchmark_corpus.py`
  - `scripts/run_corpus.py`

## v0-verifier

### Added
- C++ verifier pipeline with AWP/AVC generation and Z3 translation in `src/verifier.cpp` and `src/z3_interpreter.cpp`.
- CLI executable `ivy_verify` with baseline error behavior parity.

## v0-typechecker

### Added
- C++ type checker implementation in `src/type_checker.cpp` preserving Java type-check rules.
- Parity-oriented C++ tests for Java type-check baseline cases.

## v0-parser

### Added
- C++ parser implementation generated from the existing `Imp.g4` grammar.
- C++ AST model and pretty-printer round-trip support.
- CMake build and install/export package support (`Ivy::ivy`).
