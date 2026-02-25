#include "ivy/z3_interpreter.hpp"

#include <stdexcept>
#include <string>
#include <type_traits>

#include "ivy/errors.hpp"

namespace ivy {
namespace {

template <typename... Ts> struct Overloaded : Ts... {
    using Ts::operator()...;
};

} // namespace

Z3Interpreter::Z3Interpreter(z3::context &context) : context_(context) {}

z3::sort Z3Interpreter::ToSort(const DataType type) const {
    switch (type) {
    case DataType::kInt:
        return context_.int_sort();
    case DataType::kBool:
        return context_.bool_sort();
    case DataType::kIntArray:
        return context_.array_sort(context_.int_sort(), context_.int_sort());
    case DataType::kBoolArray:
        return context_.array_sort(context_.int_sort(), context_.bool_sort());
    case DataType::kVoid:
        return context_.uninterpreted_sort("unit");
    }
    throw VerificationError("Unknown data type in Z3 sort conversion");
}

z3::expr Z3Interpreter::Interpret(const Expression &expression) const {
    return std::visit(
        Overloaded{
            [this](const Expression::VarRef &var_ref) {
                if (!var_ref.resolved_type.has_value()) {
                    throw VerificationError("VarRefExpression has no resolved type: " + var_ref.name);
                }
                return context_.constant(var_ref.name.c_str(), ToSort(*var_ref.resolved_type));
            },
            [this](const Expression::ArrayRef &array_ref) {
                if (!array_ref.element_type.has_value()) {
                    throw VerificationError("ArrayRefExpression has no resolved element type: " +
                                            array_ref.name);
                }
                if (array_ref.index == nullptr) {
                    throw VerificationError("ArrayRefExpression index is null");
                }
                const z3::sort element_sort = ToSort(*array_ref.element_type);
                z3::expr array_expr = context_.constant(
                    array_ref.name.c_str(), context_.array_sort(context_.int_sort(), element_sort));
                z3::expr index_expr = Interpret(*array_ref.index);
                return z3::select(array_expr, index_expr);
            },
            [this](const Expression::FuncCall &call) {
                if (!call.resolved_type.has_value()) {
                    throw VerificationError("FuncCallExpression has no resolved function type: " + call.name);
                }

                z3::sort_vector domain(context_);
                for (const DataType parameter_type : call.resolved_type->parameter_types) {
                    domain.push_back(ToSort(parameter_type));
                }
                const z3::sort return_sort = ToSort(call.resolved_type->return_type);
                z3::func_decl function = context_.function(call.name.c_str(), domain, return_sort);

                z3::expr_vector arguments(context_);
                for (const ExpressionPtr &argument : call.arguments) {
                    if (argument == nullptr) {
                        throw VerificationError("Function call argument is null");
                    }
                    arguments.push_back(Interpret(*argument));
                }
                return function(arguments);
            },
            [this](const Expression::Unary &unary) {
                if (unary.operand == nullptr) {
                    throw VerificationError("Unary expression operand is null");
                }
                z3::expr operand = Interpret(*unary.operand);
                if (unary.op == UnaryOp::kNeg) {
                    return -operand;
                }
                return !operand;
            },
            [this](const Expression::Binary &binary) {
                if (binary.left == nullptr || binary.right == nullptr) {
                    throw VerificationError("Binary expression operand is null");
                }
                z3::expr left = Interpret(*binary.left);
                z3::expr right = Interpret(*binary.right);

                switch (binary.op) {
                case BinaryOp::kMul:
                    return left * right;
                case BinaryOp::kDiv:
                    return left / right;
                case BinaryOp::kMod:
                    return left % right;
                case BinaryOp::kAdd:
                    return left + right;
                case BinaryOp::kSub:
                    return left - right;
                case BinaryOp::kLess:
                    return left < right;
                case BinaryOp::kLessOrEqual:
                    return left <= right;
                case BinaryOp::kGreater:
                    return left > right;
                case BinaryOp::kGreaterOrEqual:
                    return left >= right;
                case BinaryOp::kEqual:
                    return left == right;
                case BinaryOp::kAnd:
                    return left && right;
                case BinaryOp::kOr:
                    return left || right;
                case BinaryOp::kImplies:
                    return z3::implies(left, right);
                }
                throw VerificationError("Unknown binary operator in Z3 conversion");
            },
            [this](const Expression::IntLiteral &integer_literal) {
                return context_.int_val(integer_literal.value);
            },
            [this](const Expression::BoolLiteral &bool_literal) {
                return context_.bool_val(bool_literal.value);
            },
            [this](const Expression::Quantified &quantified) {
                if (quantified.body == nullptr) {
                    throw VerificationError("Quantified expression body is null");
                }
                z3::expr bound_var = context_.constant(quantified.variable.c_str(), ToSort(quantified.type));
                z3::expr body = Interpret(*quantified.body);

                if (quantified.forall) {
                    return z3::forall(bound_var, body);
                }
                return z3::exists(bound_var, body);
            },
            [](const Expression::NewArray & /*new_array*/) -> z3::expr {
                throw VerificationError("Not Supported");
            },
            [](const Expression::ArrayLength & /*array_length*/) -> z3::expr {
                throw VerificationError("Not Implemented");
            }},
        expression.node);
}

z3::expr Z3Interpreter::InterpretInvariants(const std::vector<ExpressionPtr> &invariants) const {
    z3::expr accumulated = context_.bool_val(true);
    for (const ExpressionPtr &invariant : invariants) {
        if (invariant == nullptr) {
            throw VerificationError("Invariant expression is null");
        }
        accumulated = accumulated && Interpret(*invariant);
    }
    return accumulated;
}

z3::expr Z3Interpreter::InterpretConditionClause(const ConditionClause &clause) const {
    if (clause.expression == nullptr) {
        throw VerificationError("Condition clause expression is null");
    }
    return Interpret(*clause.expression);
}

} // namespace ivy
