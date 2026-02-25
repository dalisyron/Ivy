#include "ivy/ast.hpp"

#include <type_traits>

namespace ivy {
namespace {

template <typename... Ts> struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename T>
[[nodiscard]] bool VariantEquals(const std::variant<T> &lhs, const std::variant<T> &rhs) {
    return lhs == rhs;
}

[[nodiscard]] bool EqualsExpressionVector(const std::vector<ExpressionPtr> &lhs,
                                          const std::vector<ExpressionPtr> &rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (!DeepEquals(lhs[i], rhs[i])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool EqualsStatementVector(const std::vector<StatementPtr> &lhs,
                                         const std::vector<StatementPtr> &rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (!DeepEquals(lhs[i], rhs[i])) {
            return false;
        }
    }
    return true;
}

} // namespace

std::string ToString(const DataType type) {
    switch (type) {
    case DataType::kBool:
        return "bool";
    case DataType::kInt:
        return "int";
    case DataType::kIntArray:
        return "int[]";
    case DataType::kBoolArray:
        return "bool[]";
    case DataType::kVoid:
        return "void";
    }
    return "unknown";
}

bool IsArrayType(const DataType type) noexcept {
    return type == DataType::kIntArray || type == DataType::kBoolArray;
}

bool FunctionType::operator==(const FunctionType &other) const noexcept {
    return parameter_types == other.parameter_types && return_type == other.return_type;
}

std::string ToString(const UnaryOp op) {
    switch (op) {
    case UnaryOp::kNeg:
        return "-";
    case UnaryOp::kNot:
        return "!";
    }
    return "?";
}

std::string ToString(const BinaryOp op) {
    switch (op) {
    case BinaryOp::kMul:
        return "*";
    case BinaryOp::kDiv:
        return "/";
    case BinaryOp::kMod:
        return "%";
    case BinaryOp::kAdd:
        return "+";
    case BinaryOp::kSub:
        return "-";
    case BinaryOp::kLess:
        return "<";
    case BinaryOp::kLessOrEqual:
        return "<=";
    case BinaryOp::kGreater:
        return ">";
    case BinaryOp::kGreaterOrEqual:
        return ">=";
    case BinaryOp::kEqual:
        return "==";
    case BinaryOp::kAnd:
        return "&&";
    case BinaryOp::kOr:
        return "||";
    case BinaryOp::kImplies:
        return "==>";
    }
    return "?";
}

ExpressionPtr MakeExpr(Expression::Node value) {
    return std::make_shared<Expression>(std::move(value));
}

bool DeepEquals(const ExpressionPtr &lhs, const ExpressionPtr &rhs) {
    if (lhs == rhs) {
        return true;
    }
    if (!lhs || !rhs) {
        return false;
    }
    return *lhs == *rhs;
}

bool operator==(const Expression &lhs, const Expression &rhs) {
    if (lhs.node.index() != rhs.node.index()) {
        return false;
    }

    return std::visit(Overloaded{[&](const Expression::VarRef &left) {
                                     const auto *right = std::get_if<Expression::VarRef>(&rhs.node);
                                     return right != nullptr && left.name == right->name;
                                 },
                                 [&](const Expression::ArrayRef &left) {
                                     const auto *right = std::get_if<Expression::ArrayRef>(&rhs.node);
                                     return right != nullptr && left.name == right->name &&
                                            DeepEquals(left.index, right->index);
                                 },
                                 [&](const Expression::FuncCall &left) {
                                     const auto *right = std::get_if<Expression::FuncCall>(&rhs.node);
                                     return right != nullptr && left.name == right->name &&
                                            EqualsExpressionVector(left.arguments, right->arguments);
                                 },
                                 [&](const Expression::Unary &left) {
                                     const auto *right = std::get_if<Expression::Unary>(&rhs.node);
                                     return right != nullptr && left.op == right->op &&
                                            DeepEquals(left.operand, right->operand);
                                 },
                                 [&](const Expression::Binary &left) {
                                     const auto *right = std::get_if<Expression::Binary>(&rhs.node);
                                     return right != nullptr && left.op == right->op &&
                                            DeepEquals(left.left, right->left) &&
                                            DeepEquals(left.right, right->right);
                                 },
                                 [&](const Expression::IntLiteral &left) {
                                     const auto *right = std::get_if<Expression::IntLiteral>(&rhs.node);
                                     return right != nullptr && left.value == right->value;
                                 },
                                 [&](const Expression::BoolLiteral &left) {
                                     const auto *right = std::get_if<Expression::BoolLiteral>(&rhs.node);
                                     return right != nullptr && left.value == right->value;
                                 },
                                 [&](const Expression::Quantified &left) {
                                     const auto *right = std::get_if<Expression::Quantified>(&rhs.node);
                                     return right != nullptr && left.forall == right->forall &&
                                            left.variable == right->variable && left.type == right->type &&
                                            DeepEquals(left.body, right->body);
                                 },
                                 [&](const Expression::NewArray &left) {
                                     const auto *right = std::get_if<Expression::NewArray>(&rhs.node);
                                     return right != nullptr && left.element_type == right->element_type &&
                                            DeepEquals(left.size, right->size);
                                 },
                                 [&](const Expression::ArrayLength &left) {
                                     const auto *right = std::get_if<Expression::ArrayLength>(&rhs.node);
                                     return right != nullptr && DeepEquals(left.array, right->array);
                                 }},
                      lhs.node);
}

StatementPtr MakeStmt(Statement::Node value) {
    return std::make_shared<Statement>(std::move(value));
}

bool DeepEquals(const StatementPtr &lhs, const StatementPtr &rhs) {
    if (lhs == rhs) {
        return true;
    }
    if (!lhs || !rhs) {
        return false;
    }
    return *lhs == *rhs;
}

bool operator==(const Statement &lhs, const Statement &rhs) {
    if (lhs.node.index() != rhs.node.index()) {
        return false;
    }

    return std::visit(Overloaded{[&](const Statement::Assign &left) {
                                     const auto *right = std::get_if<Statement::Assign>(&rhs.node);
                                     return right != nullptr && DeepEquals(left.lhs, right->lhs) &&
                                            DeepEquals(left.rhs, right->rhs);
                                 },
                                 [&](const Statement::If &left) {
                                     const auto *right = std::get_if<Statement::If>(&rhs.node);
                                     if (right == nullptr || !DeepEquals(left.condition, right->condition) ||
                                         !EqualsStatementVector(left.then_block, right->then_block) ||
                                         left.else_block.has_value() != right->else_block.has_value()) {
                                         return false;
                                     }
                                     if (!left.else_block.has_value()) {
                                         return true;
                                     }
                                     return EqualsStatementVector(*left.else_block, *right->else_block);
                                 },
                                 [&](const Statement::While &left) {
                                     const auto *right = std::get_if<Statement::While>(&rhs.node);
                                     return right != nullptr &&
                                            DeepEquals(left.condition, right->condition) &&
                                            EqualsExpressionVector(left.invariants, right->invariants) &&
                                            EqualsStatementVector(left.body, right->body);
                                 },
                                 [&](const Statement::Block &left) {
                                     const auto *right = std::get_if<Statement::Block>(&rhs.node);
                                     return right != nullptr &&
                                            EqualsStatementVector(left.statements, right->statements);
                                 },
                                 [&](const Statement::VarDecl &left) {
                                     const auto *right = std::get_if<Statement::VarDecl>(&rhs.node);
                                     if (right == nullptr || left.declared_type != right->declared_type ||
                                         left.variable != right->variable ||
                                         left.initializer.has_value() != right->initializer.has_value()) {
                                         return false;
                                     }
                                     if (!left.initializer.has_value()) {
                                         return true;
                                     }
                                     return DeepEquals(*left.initializer, *right->initializer);
                                 },
                                 [&](const Statement::FuncCallStmt &left) {
                                     const auto *right = std::get_if<Statement::FuncCallStmt>(&rhs.node);
                                     return right != nullptr && DeepEquals(left.call, right->call);
                                 }},
                      lhs.node);
}

bool Parameter::operator==(const Parameter &other) const noexcept {
    return type == other.type && name == other.name;
}

bool ConditionClause::operator==(const ConditionClause &other) const {
    return kind == other.kind && DeepEquals(expression, other.expression);
}

bool MethodDeclaration::operator==(const MethodDeclaration &other) const {
    if (name != other.name || parameters != other.parameters || return_value != other.return_value ||
        conditions != other.conditions) {
        return false;
    }
    return EqualsStatementVector(body, other.body);
}

bool Program::operator==(const Program &other) const {
    return methods == other.methods;
}

} // namespace ivy
