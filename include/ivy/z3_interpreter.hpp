#pragma once

#include <vector>

#include "ivy/ast.hpp"
#include "ivy/z3_compat.hpp"

namespace ivy {

class Z3Interpreter final {
  public:
    explicit Z3Interpreter(z3::context &context);

    [[nodiscard]] z3::expr Interpret(const Expression &expression) const;
    [[nodiscard]] z3::expr InterpretInvariants(const std::vector<ExpressionPtr> &invariants) const;
    [[nodiscard]] z3::expr InterpretConditionClause(const ConditionClause &clause) const;

  private:
    [[nodiscard]] z3::sort ToSort(DataType type) const;

    z3::context &context_;
};

} // namespace ivy
