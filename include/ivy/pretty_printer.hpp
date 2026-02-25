#pragma once

#include <string>

#include "ivy/ast.hpp"

namespace ivy {

[[nodiscard]] std::string PrettyPrint(const Program &program);
[[nodiscard]] std::string PrettyPrint(const MethodDeclaration &method);
[[nodiscard]] std::string PrettyPrint(const Statement &statement);
[[nodiscard]] std::string PrettyPrint(const Expression &expression);

} // namespace ivy
