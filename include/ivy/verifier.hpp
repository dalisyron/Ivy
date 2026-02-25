#pragma once

#include <vector>

#include "ivy/ast.hpp"
#include "ivy/z3_compat.hpp"

namespace ivy {

struct VerificationResult final {
    std::string method_name;
    bool valid;
};

[[nodiscard]] z3::expr GenerateAwp(z3::context &context, const Statement &statement,
                                   const z3::expr &post_condition);
[[nodiscard]] z3::expr GenerateAvc(z3::context &context, const Statement &statement,
                                   const z3::expr &post_condition);
[[nodiscard]] bool VerifyMethod(z3::context &context, const MethodDeclaration &method);
[[nodiscard]] std::vector<VerificationResult> VerifyProgram(const Program &program);

} // namespace ivy
