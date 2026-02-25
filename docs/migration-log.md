# Migration Log

| Java module | C++ module | Commit |
|---|---|---|
| `imp.ast.*` | `include/ivy/ast.hpp`, `src/ast.cpp` | `1ec7c15` |
| `imp.print.ImpPrettyPrinter` | `src/pretty_printer.cpp` | `1ec7c15` |
| `imp.parser.Parser`, `imp.parser.ASTBuilder` | `src/parser.cpp` | `1ec7c15` |
| `imp.typechecker.TypeChecker` | `src/type_checker.cpp` | `1ec7c15` |
| `interpreter.Z3Interpreter` | `src/z3_interpreter.cpp` | `1ec7c15` |
| `verification.generator.*` | `src/verifier.cpp` | `1ec7c15` |
| `verification.ImpVerifier` | `src/main.cpp` | `1ec7c15` |
| Java tests (`src/test/java`) | C++ tests (`tests/*.cpp`) | `1ec7c15` |
| Java/C++ corpus comparison flow | `scripts/parity_check.py`, `scripts/determinism_check.py`, `scripts/benchmark_corpus.py` | `cef27fd` |
| CI and quality gates | `.github/workflows/ci.yml`, `.clang-format`, `.clang-tidy`, `scripts/coverage_gate.py`, `scripts/run_parser_fuzz.sh`, `CMakeLists.txt`, `tests/test_typechecker.cpp`, `tests/test_verifier.cpp` | `504d53b`, `0acbf56`, `541f0a9`, `947b26d`, `454295e`, `2d87a8d`, `5ab849e`, `9398dd1`, `4643f01`, `2a4dc0f`, `dbc0edf` |
