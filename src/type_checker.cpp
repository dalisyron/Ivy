#include "ivy/type_checker.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ivy/errors.hpp"
#include "ivy/pretty_printer.hpp"

namespace ivy {
namespace {

class TypeSymbolTable final {
  public:
    TypeSymbolTable() {
        stack_.emplace_back();
    }

    void PushState() {
        stack_.push_back(stack_.back());
    }

    void PopState() {
        if (stack_.size() <= 1U) {
            throw std::logic_error("TypeSymTab: cannot pop last symbol table");
        }
        stack_.pop_back();
    }

    [[nodiscard]] bool Contains(const std::string &key) const {
        return stack_.back().find(key) != stack_.back().end();
    }

    void Put(const std::string &key, const DataType value) {
        stack_.back()[key] = value;
    }

    [[nodiscard]] std::optional<DataType> Get(const std::string &key) const {
        const auto it = stack_.back().find(key);
        if (it == stack_.back().end()) {
            return std::nullopt;
        }
        return it->second;
    }

  private:
    std::vector<std::unordered_map<std::string, DataType>> stack_;
};

class TypeCheckerImpl final {
  public:
    void Check(Program &program) {
        CollectMethodSignatures(program);
        for (MethodDeclaration &method : program.methods) {
            CheckMethod(method);
        }
    }

  private:
    void CollectMethodSignatures(const Program &program) {
        for (const MethodDeclaration &method : program.methods) {
            std::vector<DataType> parameter_types;
            parameter_types.reserve(method.parameters.size());
            for (const Parameter &parameter : method.parameters) {
                parameter_types.push_back(parameter.type);
            }
            const DataType return_type =
                method.return_value.has_value() ? method.return_value->type : DataType::kVoid;
            function_symbol_table_[method.name] = FunctionType{std::move(parameter_types), return_type};
        }
    }

    void CheckMethod(MethodDeclaration &method) {
        symbol_table_.PushState();

        for (const Parameter &parameter : method.parameters) {
            if (symbol_table_.Contains(parameter.name)) {
                throw TypeError("Parameter " + parameter.name + " already declared.");
            }
            symbol_table_.Put(parameter.name, parameter.type);
        }

        if (method.return_value.has_value()) {
            const Parameter &return_value = *method.return_value;
            if (symbol_table_.Contains(return_value.name)) {
                throw TypeError("Return variable " + return_value.name + " already declared as a parameter.");
            }
            symbol_table_.Put(return_value.name, return_value.type);
        }

        for (ConditionClause &clause : method.conditions) {
            const DataType condition_type = EvalExpression(*clause.expression);
            if (condition_type != DataType::kBool) {
                if (clause.kind == ConditionClause::Kind::kEnsures) {
                    throw TypeError("Ensures clause must be of type bool.");
                }
                throw TypeError("Requires clause must be of type bool.");
            }
        }

        for (StatementPtr &statement : method.body) {
            CheckStatement(*statement);
        }

        symbol_table_.PopState();
    }

    void CheckStatement(Statement &statement) {
        std::visit(
            [&](auto &node) {
                using T = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<T, Statement::Assign>) {
                    const DataType lhs_type = EvalExpression(*node.lhs);
                    const DataType rhs_type = EvalExpression(*node.rhs);
                    if (lhs_type != rhs_type) {
                        throw TypeError("Cannot assign " + ToString(rhs_type) + " to " +
                                        PrettyPrint(*node.lhs));
                    }
                } else if constexpr (std::is_same_v<T, Statement::If>) {
                    const DataType condition_type = EvalExpression(*node.condition);
                    if (condition_type != DataType::kBool) {
                        throw TypeError("If condition must be of type bool.");
                    }

                    symbol_table_.PushState();
                    for (StatementPtr &then_statement : node.then_block) {
                        CheckStatement(*then_statement);
                    }
                    symbol_table_.PopState();

                    if (node.else_block.has_value()) {
                        symbol_table_.PushState();
                        for (StatementPtr &else_statement : *node.else_block) {
                            CheckStatement(*else_statement);
                        }
                        symbol_table_.PopState();
                    }
                } else if constexpr (std::is_same_v<T, Statement::While>) {
                    const DataType condition_type = EvalExpression(*node.condition);
                    if (condition_type != DataType::kBool) {
                        throw TypeError("While condition must be of type bool.");
                    }

                    for (ExpressionPtr &invariant : node.invariants) {
                        const DataType invariant_type = EvalExpression(*invariant);
                        if (invariant_type != DataType::kBool) {
                            throw TypeError("Invariant expression must be of type bool.");
                        }
                    }

                    symbol_table_.PushState();
                    for (StatementPtr &body_statement : node.body) {
                        CheckStatement(*body_statement);
                    }
                    symbol_table_.PopState();
                } else if constexpr (std::is_same_v<T, Statement::Block>) {
                    symbol_table_.PushState();
                    for (StatementPtr &block_statement : node.statements) {
                        CheckStatement(*block_statement);
                    }
                    symbol_table_.PopState();
                } else if constexpr (std::is_same_v<T, Statement::VarDecl>) {
                    if (symbol_table_.Contains(node.variable)) {
                        throw TypeError("Variable " + node.variable + " already declared.");
                    }

                    symbol_table_.Put(node.variable, node.declared_type);

                    if (node.initializer.has_value()) {
                        const DataType initializer_type = EvalExpression(**node.initializer);
                        if (initializer_type != node.declared_type) {
                            throw TypeError("Cannot assign " + ToString(initializer_type) + " to variable " +
                                            node.variable + " of type " + ToString(node.declared_type));
                        }
                    }
                } else if constexpr (std::is_same_v<T, Statement::FuncCallStmt>) {
                    static_cast<void>(EvalExpression(*node.call));
                }
            },
            statement.node);
    }

    [[nodiscard]] DataType EvalExpression(Expression &expression) {
        return std::visit(
            [&](auto &node) -> DataType {
                using T = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<T, Expression::VarRef>) {
                    const std::optional<DataType> variable_type = symbol_table_.Get(node.name);
                    if (!variable_type.has_value()) {
                        throw TypeError("Variable " + node.name + " is not declared.");
                    }
                    node.resolved_type = variable_type;
                    return *variable_type;
                } else if constexpr (std::is_same_v<T, Expression::ArrayRef>) {
                    const std::optional<DataType> array_type = symbol_table_.Get(node.name);
                    if (!array_type.has_value()) {
                        throw TypeError("Array " + node.name + " is not declared.");
                    }
                    if (!IsArrayType(*array_type)) {
                        throw TypeError(node.name + " is not an array.");
                    }

                    const DataType index_type = EvalExpression(*node.index);
                    if (index_type != DataType::kInt) {
                        throw TypeError("Array index must be of type int.");
                    }

                    node.element_type =
                        *array_type == DataType::kBoolArray ? DataType::kBool : DataType::kInt;
                    return *node.element_type;
                } else if constexpr (std::is_same_v<T, Expression::FuncCall>) {
                    const auto function_it = function_symbol_table_.find(node.name);
                    if (function_it == function_symbol_table_.end()) {
                        throw TypeError("Unknown function: " + node.name);
                    }
                    const FunctionType &function_type = function_it->second;
                    if (node.arguments.size() != function_type.parameter_types.size()) {
                        throw TypeError("Function " + node.name + " expects " +
                                        std::to_string(function_type.parameter_types.size()) +
                                        " arguments, but got " + std::to_string(node.arguments.size()));
                    }

                    for (std::size_t i = 0; i < node.arguments.size(); ++i) {
                        const DataType argument_type = EvalExpression(*node.arguments[i]);
                        const DataType expected_type = function_type.parameter_types[i];
                        if (argument_type != expected_type) {
                            throw TypeError("Function " + node.name + " argument " + std::to_string(i + 1U) +
                                            " expects type " + ToString(expected_type) + ", but got " +
                                            ToString(argument_type));
                        }
                    }

                    node.resolved_type = function_type;
                    return function_type.return_type;
                } else if constexpr (std::is_same_v<T, Expression::Unary>) {
                    const DataType operand_type = EvalExpression(*node.operand);
                    const std::optional<DataType> result_type = UnaryResultType(node.op, operand_type);
                    if (!result_type.has_value()) {
                        throw TypeError("Invalid operand for operator '" + ToString(node.op) +
                                        "' in expression '" + PrettyPrint(expression) +
                                        "': " + ToString(operand_type));
                    }
                    return *result_type;
                } else if constexpr (std::is_same_v<T, Expression::Binary>) {
                    const DataType left_type = EvalExpression(*node.left);
                    const DataType right_type = EvalExpression(*node.right);
                    const std::optional<DataType> result_type =
                        BinaryResultType(node.op, left_type, right_type);
                    if (!result_type.has_value()) {
                        throw TypeError("Invalid operands for operator '" + ToString(node.op) +
                                        "' in expression '" + PrettyPrint(expression) +
                                        "': " + ToString(left_type) + ", " + ToString(right_type));
                    }
                    return *result_type;
                } else if constexpr (std::is_same_v<T, Expression::IntLiteral>) {
                    return DataType::kInt;
                } else if constexpr (std::is_same_v<T, Expression::BoolLiteral>) {
                    return DataType::kBool;
                } else if constexpr (std::is_same_v<T, Expression::Quantified>) {
                    symbol_table_.PushState();
                    symbol_table_.Put(node.variable, node.type);
                    const DataType body_type = EvalExpression(*node.body);
                    symbol_table_.PopState();

                    if (body_type != DataType::kBool) {
                        if (node.forall) {
                            throw TypeError("Body of forall expression must be of type bool.");
                        }
                        throw TypeError("Body of exists expression must be of type bool.");
                    }
                    return DataType::kBool;
                } else if constexpr (std::is_same_v<T, Expression::NewArray>) {
                    const DataType size_type = EvalExpression(*node.size);
                    if (size_type != DataType::kInt) {
                        throw TypeError("Array size must be of type int.");
                    }
                    return node.element_type == DataType::kBool ? DataType::kBoolArray : DataType::kIntArray;
                } else if constexpr (std::is_same_v<T, Expression::ArrayLength>) {
                    const DataType array_type = EvalExpression(*node.array);
                    if (!IsArrayType(array_type)) {
                        throw TypeError("Cannot get length of non-array type.");
                    }
                    return DataType::kInt;
                }
                throw TypeError("Unknown expression node");
            },
            expression.node);
    }

    [[nodiscard]] static std::optional<DataType> UnaryResultType(const UnaryOp op,
                                                                 const DataType operand_type) {
        if (op == UnaryOp::kNeg && operand_type == DataType::kInt) {
            return DataType::kInt;
        }
        if (op == UnaryOp::kNot && operand_type == DataType::kBool) {
            return DataType::kBool;
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<DataType> BinaryResultType(const BinaryOp op, const DataType left_type,
                                                                  const DataType right_type) {
        switch (op) {
        case BinaryOp::kAdd:
        case BinaryOp::kSub:
        case BinaryOp::kMul:
        case BinaryOp::kDiv:
        case BinaryOp::kMod:
            if (left_type == DataType::kInt && right_type == DataType::kInt) {
                return DataType::kInt;
            }
            return std::nullopt;
        case BinaryOp::kAnd:
        case BinaryOp::kOr:
        case BinaryOp::kImplies:
            if (left_type == DataType::kBool && right_type == DataType::kBool) {
                return DataType::kBool;
            }
            return std::nullopt;
        case BinaryOp::kLess:
        case BinaryOp::kLessOrEqual:
        case BinaryOp::kGreater:
        case BinaryOp::kGreaterOrEqual:
            if (left_type == DataType::kInt && right_type == DataType::kInt) {
                return DataType::kBool;
            }
            return std::nullopt;
        case BinaryOp::kEqual:
            if ((left_type == DataType::kInt && right_type == DataType::kInt) ||
                (left_type == DataType::kBool && right_type == DataType::kBool)) {
                return DataType::kBool;
            }
            return std::nullopt;
        }
        return std::nullopt;
    }

    TypeSymbolTable symbol_table_;
    std::unordered_map<std::string, FunctionType> function_symbol_table_;
};

} // namespace

void CheckTypes(Program &program) {
    TypeCheckerImpl checker;
    checker.Check(program);
}

} // namespace ivy
