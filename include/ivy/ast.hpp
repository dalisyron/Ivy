#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ivy {

enum class DataType {
    kBool,
    kInt,
    kIntArray,
    kBoolArray,
    kVoid,
};

[[nodiscard]] std::string ToString(DataType type);
[[nodiscard]] bool IsArrayType(DataType type) noexcept;

struct FunctionType {
    std::vector<DataType> parameter_types;
    DataType return_type{DataType::kVoid};

    [[nodiscard]] bool operator==(const FunctionType &other) const noexcept;
};

enum class UnaryOp {
    kNeg,
    kNot,
};

enum class BinaryOp {
    kMul,
    kDiv,
    kMod,
    kAdd,
    kSub,
    kLess,
    kLessOrEqual,
    kGreater,
    kGreaterOrEqual,
    kEqual,
    kAnd,
    kOr,
    kImplies,
};

[[nodiscard]] std::string ToString(UnaryOp op);
[[nodiscard]] std::string ToString(BinaryOp op);

struct Expression;
struct Statement;

using ExpressionPtr = std::shared_ptr<Expression>;
using StatementPtr = std::shared_ptr<Statement>;

struct Expression final {
    struct VarRef final {
        std::string name;
        std::optional<DataType> resolved_type;
    };

    struct ArrayRef final {
        std::string name;
        ExpressionPtr index;
        std::optional<DataType> element_type;
    };

    struct FuncCall final {
        std::string name;
        std::vector<ExpressionPtr> arguments;
        std::optional<FunctionType> resolved_type;
    };

    struct Unary final {
        UnaryOp op;
        ExpressionPtr operand;
    };

    struct Binary final {
        BinaryOp op;
        ExpressionPtr left;
        ExpressionPtr right;
    };

    struct IntLiteral final {
        std::int64_t value;
    };

    struct BoolLiteral final {
        bool value;
    };

    struct Quantified final {
        bool forall;
        std::string variable;
        DataType type;
        ExpressionPtr body;
    };

    struct NewArray final {
        DataType element_type;
        ExpressionPtr size;
    };

    struct ArrayLength final {
        ExpressionPtr array;
    };

    using Node = std::variant<VarRef, ArrayRef, FuncCall, Unary, Binary, IntLiteral, BoolLiteral, Quantified,
                              NewArray, ArrayLength>;

    explicit Expression(Node value) : node(std::move(value)) {}

    Node node;
};

[[nodiscard]] ExpressionPtr MakeExpr(Expression::Node value);
[[nodiscard]] bool operator==(const Expression &lhs, const Expression &rhs);
[[nodiscard]] bool DeepEquals(const ExpressionPtr &lhs, const ExpressionPtr &rhs);

struct Statement final {
    struct Assign final {
        ExpressionPtr lhs;
        ExpressionPtr rhs;
    };

    struct If final {
        ExpressionPtr condition;
        std::vector<StatementPtr> then_block;
        std::optional<std::vector<StatementPtr>> else_block;
    };

    struct While final {
        ExpressionPtr condition;
        std::vector<ExpressionPtr> invariants;
        std::vector<StatementPtr> body;
    };

    struct Block final {
        std::vector<StatementPtr> statements;
    };

    struct VarDecl final {
        DataType declared_type;
        std::string variable;
        std::optional<ExpressionPtr> initializer;
    };

    struct FuncCallStmt final {
        ExpressionPtr call;
    };

    using Node = std::variant<Assign, If, While, Block, VarDecl, FuncCallStmt>;

    explicit Statement(Node value) : node(std::move(value)) {}

    Node node;
};

[[nodiscard]] StatementPtr MakeStmt(Statement::Node value);
[[nodiscard]] bool operator==(const Statement &lhs, const Statement &rhs);
[[nodiscard]] bool DeepEquals(const StatementPtr &lhs, const StatementPtr &rhs);

struct Parameter final {
    DataType type;
    std::string name;

    [[nodiscard]] bool operator==(const Parameter &other) const noexcept;
};

struct ConditionClause final {
    enum class Kind {
        kRequires,
        kEnsures,
    };

    Kind kind;
    ExpressionPtr expression;

    [[nodiscard]] bool operator==(const ConditionClause &other) const;
};

struct MethodDeclaration final {
    std::string name;
    std::vector<Parameter> parameters;
    std::optional<Parameter> return_value;
    std::vector<ConditionClause> conditions;
    std::vector<StatementPtr> body;

    [[nodiscard]] bool operator==(const MethodDeclaration &other) const;
};

struct Program final {
    std::vector<MethodDeclaration> methods;

    [[nodiscard]] bool operator==(const Program &other) const;
};

} // namespace ivy
