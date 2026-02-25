#include "ivy/pretty_printer.hpp"

#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace ivy {
namespace {

template <typename... Ts> struct Overloaded : Ts... {
    using Ts::operator()...;
};

[[nodiscard]] std::string PrintExpression(const Expression &expression);
[[nodiscard]] std::string PrintStatement(const Statement &statement);

[[nodiscard]] std::string JoinStatements(const std::vector<StatementPtr> &statements) {
    std::ostringstream out;
    for (const StatementPtr &statement : statements) {
        if (statement == nullptr) {
            throw std::runtime_error("Encountered null statement while pretty-printing");
        }
        out << PrintStatement(*statement) << '\n';
    }
    return out.str();
}

[[nodiscard]] std::string JoinExpressions(const std::vector<ExpressionPtr> &expressions,
                                          const std::string &separator) {
    std::ostringstream out;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (expressions[i] == nullptr) {
            throw std::runtime_error("Encountered null expression while pretty-printing");
        }
        out << PrintExpression(*expressions[i]);
        if (i + 1U < expressions.size()) {
            out << separator;
        }
    }
    return out.str();
}

[[nodiscard]] std::string PrintExpression(const Expression &expression) {
    return std::visit(
        Overloaded{[](const Expression::VarRef &var_ref) { return var_ref.name; },
                   [](const Expression::ArrayRef &array_ref) {
                       if (array_ref.index == nullptr) {
                           throw std::runtime_error("Array index expression is null");
                       }
                       return array_ref.name + "[" + PrintExpression(*array_ref.index) + "]";
                   },
                   [](const Expression::FuncCall &call) {
                       std::ostringstream out;
                       out << call.name << "(";
                       out << JoinExpressions(call.arguments, ", ");
                       out << ")";
                       return out.str();
                   },
                   [](const Expression::Unary &unary) {
                       if (unary.operand == nullptr) {
                           throw std::runtime_error("Unary operand is null");
                       }
                       return "(" + ToString(unary.op) + PrintExpression(*unary.operand) + ")";
                   },
                   [](const Expression::Binary &binary) {
                       if (binary.left == nullptr || binary.right == nullptr) {
                           throw std::runtime_error("Binary operand is null");
                       }
                       return "(" + PrintExpression(*binary.left) + " " + ToString(binary.op) + " " +
                              PrintExpression(*binary.right) + ")";
                   },
                   [](const Expression::IntLiteral &literal) { return std::to_string(literal.value); },
                   [](const Expression::BoolLiteral &literal) {
                       return literal.value ? std::string("true") : std::string("false");
                   },
                   [](const Expression::Quantified &quantified) {
                       if (quantified.body == nullptr) {
                           throw std::runtime_error("Quantified body is null");
                       }
                       const std::string quantifier = quantified.forall ? "forall" : "exists";
                       return quantifier + " (" + ToString(quantified.type) + " " + quantified.variable +
                              ") :: " + PrintExpression(*quantified.body);
                   },
                   [](const Expression::NewArray &new_array) {
                       if (new_array.size == nullptr) {
                           throw std::runtime_error("Array size expression is null");
                       }
                       return "new " + ToString(new_array.element_type) + "[" +
                              PrintExpression(*new_array.size) + "]";
                   },
                   [](const Expression::ArrayLength &length) {
                       if (length.array == nullptr) {
                           throw std::runtime_error("Array length expression is null");
                       }
                       return PrintExpression(*length.array) + ".length";
                   }},
        expression.node);
}

[[nodiscard]] std::string PrintBlock(const std::vector<StatementPtr> &statements) {
    std::ostringstream out;
    out << "{\n";
    out << JoinStatements(statements);
    out << "}";
    return out.str();
}

[[nodiscard]] std::string PrintStatement(const Statement &statement) {
    return std::visit(
        Overloaded{[](const Statement::Assign &assign) {
                       if (assign.lhs == nullptr || assign.rhs == nullptr) {
                           throw std::runtime_error("Assign statement contains null expressions");
                       }
                       return PrintExpression(*assign.lhs) + " = " + PrintExpression(*assign.rhs) + ";";
                   },
                   [](const Statement::If &if_statement) {
                       if (if_statement.condition == nullptr) {
                           throw std::runtime_error("If condition is null");
                       }
                       std::ostringstream out;
                       out << "if (" << PrintExpression(*if_statement.condition) << ") "
                           << PrintBlock(if_statement.then_block);
                       if (if_statement.else_block.has_value()) {
                           out << " else " << PrintBlock(*if_statement.else_block);
                       }
                       return out.str();
                   },
                   [](const Statement::While &while_statement) {
                       if (while_statement.condition == nullptr) {
                           throw std::runtime_error("While condition is null");
                       }
                       std::ostringstream invariants;
                       for (const ExpressionPtr &invariant : while_statement.invariants) {
                           if (invariant == nullptr) {
                               throw std::runtime_error("Invariant expression is null");
                           }
                           invariants << "invariant " << PrintExpression(*invariant) << '\n';
                       }
                       return "while (" + PrintExpression(*while_statement.condition) + ")" +
                              invariants.str() + " " + PrintBlock(while_statement.body);
                   },
                   [](const Statement::Block &block) { return PrintBlock(block.statements); },
                   [](const Statement::VarDecl &declaration) {
                       std::string output = ToString(declaration.declared_type) + " " + declaration.variable;
                       if (declaration.initializer.has_value()) {
                           if (*declaration.initializer == nullptr) {
                               throw std::runtime_error("Variable initializer is null");
                           }
                           output += " = " + PrintExpression(**declaration.initializer);
                       }
                       output += ";";
                       return output;
                   },
                   [](const Statement::FuncCallStmt &call_statement) {
                       if (call_statement.call == nullptr) {
                           throw std::runtime_error("Function call statement is null");
                       }
                       return PrintExpression(*call_statement.call) + ";";
                   }},
        statement.node);
}

} // namespace

std::string PrettyPrint(const Expression &expression) {
    return PrintExpression(expression);
}

std::string PrettyPrint(const Statement &statement) {
    return PrintStatement(statement);
}

std::string PrettyPrint(const MethodDeclaration &method) {
    std::ostringstream out;
    out << "method " << method.name;

    out << "(";
    for (std::size_t i = 0; i < method.parameters.size(); ++i) {
        const Parameter &parameter = method.parameters[i];
        out << ToString(parameter.type) << " " << parameter.name;
        if (i + 1U < method.parameters.size()) {
            out << ", ";
        }
    }
    out << ")";

    if (method.return_value.has_value()) {
        out << " returns (" << ToString(method.return_value->type) << " " << method.return_value->name << ")";
    }

    if (!method.conditions.empty()) {
        out << '\n';
        for (std::size_t i = 0; i < method.conditions.size(); ++i) {
            const ConditionClause &clause = method.conditions[i];
            const std::string keyword =
                clause.kind == ConditionClause::Kind::kRequires ? "requires" : "ensures";
            if (clause.expression == nullptr) {
                throw std::runtime_error("Condition expression is null");
            }
            out << keyword << " " << PrintExpression(*clause.expression);
            if (i + 1U < method.conditions.size()) {
                out << '\n';
            }
        }
    }

    out << " {\n";
    out << JoinStatements(method.body);
    out << "\n}";

    return out.str();
}

std::string PrettyPrint(const Program &program) {
    std::ostringstream out;
    for (std::size_t i = 0; i < program.methods.size(); ++i) {
        out << PrettyPrint(program.methods[i]);
        if (i + 1U < program.methods.size()) {
            out << "\n\n";
        }
    }
    return out.str();
}

} // namespace ivy
