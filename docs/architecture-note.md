# Ivy Java -> C++ Architecture Mapping

## Module map

- `imp.parser.Parser` + `imp.parser.ASTBuilder` -> `ivy::ParseString/ParseFile` in [src/parser.cpp](/Users/nibom/Ivy/src/parser.cpp)
- `imp.ast.*` -> unified value-oriented AST model in [include/ivy/ast.hpp](/Users/nibom/Ivy/include/ivy/ast.hpp)
- `imp.print.ImpPrettyPrinter` -> [src/pretty_printer.cpp](/Users/nibom/Ivy/src/pretty_printer.cpp)
- `imp.typechecker.TypeChecker` -> [src/type_checker.cpp](/Users/nibom/Ivy/src/type_checker.cpp)
- `interpreter.Z3Interpreter` -> [src/z3_interpreter.cpp](/Users/nibom/Ivy/src/z3_interpreter.cpp)
- `verification.generator.{AWP,AVC,Assignment,IfElse,While,Method}` -> [src/verifier.cpp](/Users/nibom/Ivy/src/verifier.cpp)
- `verification.ImpVerifier` CLI -> [src/main.cpp](/Users/nibom/Ivy/src/main.cpp)

## Key design choices

- C++20 API-first library target `Ivy::ivy` plus CLI target `ivy_verify`.
- No owning raw pointers: recursive AST nodes use `std::shared_ptr` and deterministic RAII throughout.
- Type checker mutates expression metadata (`resolved_type`, function signature) so verifier and Z3 translation remain behavior-compatible with Java.
- ANTLR C++ parser code is generated from the exact existing `Imp.g4` using `/Users/nibom/v23/antlr-4.13.2-complete.jar`.

## Public integration points

- Parse APIs: [include/ivy/parser.hpp](/Users/nibom/Ivy/include/ivy/parser.hpp)
- Type checking API: [include/ivy/type_checker.hpp](/Users/nibom/Ivy/include/ivy/type_checker.hpp)
- Verification API: [include/ivy/verifier.hpp](/Users/nibom/Ivy/include/ivy/verifier.hpp)
- Package export via CMake install config (`IvyConfig.cmake`, `IvyTargets.cmake`).
