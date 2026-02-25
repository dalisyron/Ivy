#include "ivy/verifier.hpp"

#include <stdexcept>
#include <string>
#include <type_traits>

#include "ivy/errors.hpp"
#include "ivy/z3_interpreter.hpp"

namespace ivy {
namespace {

template <typename... Ts> struct Overloaded : Ts... {
    using Ts::operator()...;
};

class VerificationConditionGenerator final {
  public:
    explicit VerificationConditionGenerator(z3::context &context)
        : context_(context), interpreter_(context) {}

    [[nodiscard]] z3::expr Awp(const Statement &statement, const z3::expr &post_condition) {
        return std::visit(
            Overloaded{[this, &post_condition](const Statement::Assign &assign) {
                           if (assign.lhs == nullptr || assign.rhs == nullptr) {
                               throw VerificationError("Assign statement contains null expression");
                           }
                           const z3::expr lhs = interpreter_.Interpret(*assign.lhs);
                           const z3::expr rhs = interpreter_.Interpret(*assign.rhs);
                           return Substitute(post_condition, lhs, rhs);
                       },
                       [this, &post_condition](const Statement::If &if_statement) {
                           if (if_statement.condition == nullptr) {
                               throw VerificationError("If condition is null");
                           }
                           const z3::expr condition = interpreter_.Interpret(*if_statement.condition);
                           const z3::expr then_awp = AwpBlock(if_statement.then_block, post_condition);
                           const z3::expr else_awp = if_statement.else_block.has_value()
                                                         ? AwpBlock(*if_statement.else_block, post_condition)
                                                         : post_condition;
                           return z3::implies(condition, then_awp) && z3::implies(!condition, else_awp);
                       },
                       [this](const Statement::While &while_statement) {
                           return interpreter_.InterpretInvariants(while_statement.invariants);
                       },
                       [this, &post_condition](const Statement::Block &block) {
                           return AwpBlock(block.statements, post_condition);
                       },
                       [this, &post_condition](const Statement::VarDecl &declaration) {
                           if (!declaration.initializer.has_value()) {
                               return post_condition;
                           }
                           if (*declaration.initializer == nullptr) {
                               throw VerificationError("Variable declaration initializer is null");
                           }
                           const z3::expr rhs = interpreter_.Interpret(**declaration.initializer);
                           const z3::expr variable =
                               context_.constant(declaration.variable.c_str(), rhs.get_sort());
                           return Substitute(post_condition, variable, rhs);
                       },
                       [](const Statement::FuncCallStmt & /*call_statement*/) -> z3::expr {
                           throw VerificationError("Non-implemented AWP");
                       }},
            statement.node);
    }

    [[nodiscard]] z3::expr Avc(const Statement &statement, const z3::expr &post_condition) {
        return std::visit(
            Overloaded{[this](const Statement::Assign & /*assign*/) { return context_.bool_val(true); },
                       [this, &post_condition](const Statement::If &if_statement) {
                           const z3::expr then_avc = AvcBlock(if_statement.then_block, post_condition);
                           const z3::expr else_avc = if_statement.else_block.has_value()
                                                         ? AvcBlock(*if_statement.else_block, post_condition)
                                                         : context_.bool_val(true);
                           return then_avc && else_avc;
                       },
                       [this, &post_condition](const Statement::While &while_statement) {
                           if (while_statement.condition == nullptr) {
                               throw VerificationError("While condition is null");
                           }
                           const z3::expr invariants =
                               interpreter_.InterpretInvariants(while_statement.invariants);
                           const z3::expr condition = interpreter_.Interpret(*while_statement.condition);
                           const z3::expr part_a = AvcBlock(while_statement.body, invariants);
                           const z3::expr part_b = z3::implies(invariants && condition,
                                                               AwpBlock(while_statement.body, invariants));
                           const z3::expr part_c = z3::implies(invariants && !condition, post_condition);
                           return part_a && part_b && part_c;
                       },
                       [this, &post_condition](const Statement::Block &block) {
                           return AvcBlock(block.statements, post_condition);
                       },
                       [this](const Statement::VarDecl & /*declaration*/) { return context_.bool_val(true); },
                       [](const Statement::FuncCallStmt & /*call_statement*/) -> z3::expr {
                           throw VerificationError("Non-implemented AVC");
                       }},
            statement.node);
    }

    [[nodiscard]] z3::expr AwpBlock(const std::vector<StatementPtr> &statements,
                                    const z3::expr &post_condition) {
        if (statements.empty()) {
            return post_condition;
        }
        if (statements.size() == 1U) {
            if (statements.front() == nullptr) {
                throw VerificationError("Null statement in block");
            }
            return Awp(*statements.front(), post_condition);
        }

        std::vector<StatementPtr> tail(statements.begin() + 1, statements.end());
        const z3::expr rest_awp = AwpBlock(tail, post_condition);

        if (statements.front() == nullptr) {
            throw VerificationError("Null statement in block");
        }
        return Awp(*statements.front(), rest_awp);
    }

    [[nodiscard]] z3::expr AvcBlock(const std::vector<StatementPtr> &statements,
                                    const z3::expr &post_condition) {
        if (statements.empty()) {
            return context_.bool_val(true);
        }
        if (statements.size() == 1U) {
            if (statements.front() == nullptr) {
                throw VerificationError("Null statement in block");
            }
            return Avc(*statements.front(), post_condition);
        }

        std::vector<StatementPtr> tail(statements.begin() + 1, statements.end());
        const z3::expr part_a = AvcBlock(tail, post_condition);
        const z3::expr part_b = Avc(*statements.front(), AwpBlock(tail, post_condition));
        return part_a && part_b;
    }

  private:
    [[nodiscard]] z3::expr Substitute(const z3::expr &expression, const z3::expr &from,
                                      const z3::expr &to) const {
        z3::expr_vector sources(context_);
        z3::expr_vector targets(context_);
        sources.push_back(from);
        targets.push_back(to);
        z3::expr mutable_expression = expression;
        return mutable_expression.substitute(sources, targets);
    }

    z3::context &context_;
    Z3Interpreter interpreter_;
};

} // namespace

z3::expr GenerateAwp(z3::context &context, const Statement &statement, const z3::expr &post_condition) {
    VerificationConditionGenerator generator(context);
    return generator.Awp(statement, post_condition);
}

z3::expr GenerateAvc(z3::context &context, const Statement &statement, const z3::expr &post_condition) {
    VerificationConditionGenerator generator(context);
    return generator.Avc(statement, post_condition);
}

bool VerifyMethod(z3::context &context, const MethodDeclaration &method) {
    VerificationConditionGenerator generator(context);
    Z3Interpreter interpreter(context);

    z3::expr pre_conditions = context.bool_val(true);
    z3::expr post_conditions = context.bool_val(true);

    for (const ConditionClause &clause : method.conditions) {
        const z3::expr expression = interpreter.InterpretConditionClause(clause);
        if (clause.kind == ConditionClause::Kind::kRequires) {
            pre_conditions = pre_conditions && expression;
        } else {
            post_conditions = post_conditions && expression;
        }
    }

    const z3::expr avc = generator.AvcBlock(method.body, post_conditions);
    const z3::expr awp = generator.AwpBlock(method.body, post_conditions);
    const z3::expr verification_condition = !(z3::implies(pre_conditions, awp) && avc);
    z3::solver serialization_solver(context);
    serialization_solver.add(verification_condition);

    // Solve in a fresh context to avoid pathological performance on
    // in-memory quantified arithmetic terms in long-lived contexts.
    z3::context solving_context;
    z3::solver solving_solver(solving_context);
    solving_solver.from_string(serialization_solver.to_smt2().c_str());
    return solving_solver.check() == z3::unsat;
}

std::vector<VerificationResult> VerifyProgram(const Program &program) {
    std::vector<VerificationResult> results;
    results.reserve(program.methods.size());

    z3::context context;
    for (const MethodDeclaration &method : program.methods) {
        const bool valid = VerifyMethod(context, method);
        results.push_back(VerificationResult{method.name, valid});
    }

    return results;
}

} // namespace ivy
