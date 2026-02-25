#include "ivy/parser.hpp"

#include <antlr4-runtime.h>

#include <any>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ImpBaseVisitor.h"
#include "ImpLexer.h"
#include "ImpParser.h"
#include "ivy/errors.hpp"
#include "ivy/type_checker.hpp"

namespace ivy {
namespace {

class ThrowingErrorListener final : public antlr4::BaseErrorListener {
  public:
    void syntaxError(antlr4::Recognizer * /*recognizer*/, antlr4::Token * /*offendingSymbol*/, size_t line,
                     size_t charPositionInLine, const std::string &message,
                     std::exception_ptr /*exception*/) override {
        const std::string formatted = "Syntax error at line " + std::to_string(line) + ":" +
                                      std::to_string(charPositionInLine) + " - " + message;
        throw ParseError(static_cast<int>(line), static_cast<int>(charPositionInLine), formatted);
    }
};

template <typename T> [[nodiscard]] T AnyCast(const std::any &value, const char *context) {
    try {
        return std::any_cast<T>(value);
    } catch (const std::bad_any_cast &) {
        throw std::runtime_error(std::string("Internal parser any_cast failure in ") + context);
    }
}

class AstBuilder final : public ImpBaseVisitor {
  public:
    std::any visitParse(ImpParser::ParseContext *context) override {
        std::vector<MethodDeclaration> methods;
        methods.reserve(context->methodDeclaration().size());
        for (ImpParser::MethodDeclarationContext *method_context : context->methodDeclaration()) {
            methods.push_back(
                AnyCast<MethodDeclaration>(visitMethodDeclaration(method_context), "visitParse"));
        }
        return Program{std::move(methods)};
    }

    std::any visitMethodDeclaration(ImpParser::MethodDeclarationContext *context) override {
        MethodDeclaration method;
        method.name = context->ID()->getText();

        if (context->formalParameters() != nullptr) {
            method.parameters =
                AnyCast<std::vector<Parameter>>(visitFormalParameters(context->formalParameters()),
                                                "visitMethodDeclaration.formalParameters");
        }

        if (context->returnsBlock() != nullptr) {
            method.return_value = AnyCast<Parameter>(visitReturnsBlock(context->returnsBlock()),
                                                     "visitMethodDeclaration.returnsBlock");
        }

        method.conditions = AnyCast<std::vector<ConditionClause>>(
            visitConditionBlock(context->conditionBlock()), "visitMethodDeclaration.conditionBlock");

        method.body =
            AnyCast<std::vector<StatementPtr>>(visitBlock(context->block()), "visitMethodDeclaration.body");

        return method;
    }

    std::any visitFormalParameters(ImpParser::FormalParametersContext *context) override {
        std::vector<Parameter> parameters;
        parameters.reserve(context->formalParameter().size());
        for (ImpParser::FormalParameterContext *parameter_context : context->formalParameter()) {
            parameters.push_back(AnyCast<Parameter>(visitFormalParameter(parameter_context),
                                                    "visitFormalParameters.parameter"));
        }
        return parameters;
    }

    std::any visitFormalParameter(ImpParser::FormalParameterContext *context) override {
        const DataType type = AnyCast<DataType>(visit(context->type()), "visitFormalParameter.type");
        return Parameter{type, context->ID()->getText()};
    }

    std::any visitReturnsBlock(ImpParser::ReturnsBlockContext *context) override {
        return visitFormalParameter(context->formalParameter());
    }

    std::any visitConditionBlock(ImpParser::ConditionBlockContext *context) override {
        std::vector<ConditionClause> clauses;
        if (context->children.empty()) {
            return clauses;
        }

        clauses.reserve(context->children.size());
        for (antlr4::tree::ParseTree *child : context->children) {
            if (auto *requires_clause = dynamic_cast<ImpParser::RequiresClauseContext *>(child)) {
                clauses.push_back(AnyCast<ConditionClause>(visitRequiresClause(requires_clause),
                                                           "visitConditionBlock.requires"));
                continue;
            }
            if (auto *ensures_clause = dynamic_cast<ImpParser::EnsuresClauseContext *>(child)) {
                clauses.push_back(AnyCast<ConditionClause>(visitEnsuresClause(ensures_clause),
                                                           "visitConditionBlock.ensures"));
                continue;
            }
        }
        return clauses;
    }

    std::any visitRequiresClause(ImpParser::RequiresClauseContext *context) override {
        ExpressionPtr expression =
            AnyCast<ExpressionPtr>(visit(context->expression()), "visitRequiresClause");
        return ConditionClause{ConditionClause::Kind::kRequires, std::move(expression)};
    }

    std::any visitEnsuresClause(ImpParser::EnsuresClauseContext *context) override {
        ExpressionPtr expression = AnyCast<ExpressionPtr>(visit(context->expression()), "visitEnsuresClause");
        return ConditionClause{ConditionClause::Kind::kEnsures, std::move(expression)};
    }

    std::any visitAssignStmt(ImpParser::AssignStmtContext *context) override {
        return visitAssignStatement(context->assignStatement());
    }

    std::any visitIfStmt(ImpParser::IfStmtContext *context) override {
        return visitIfStatement(context->ifStatement());
    }

    std::any visitWhileStmt(ImpParser::WhileStmtContext *context) override {
        return visitWhileStatement(context->whileStatement());
    }

    std::any visitBlockStmt(ImpParser::BlockStmtContext *context) override {
        std::vector<StatementPtr> statements =
            AnyCast<std::vector<StatementPtr>>(visitBlock(context->block()), "visitBlockStmt");
        return MakeStmt(Statement::Block{std::move(statements)});
    }

    std::any visitVarDeclStmt(ImpParser::VarDeclStmtContext *context) override {
        return visitVarDecl(context->varDecl());
    }

    std::any visitFuncCallStmt(ImpParser::FuncCallStmtContext *context) override {
        std::vector<ExpressionPtr> arguments;
        if (context->exprList() != nullptr) {
            arguments =
                AnyCast<std::vector<ExpressionPtr>>(visitExprList(context->exprList()), "visitFuncCallStmt");
        }

        ExpressionPtr call =
            MakeExpr(Expression::FuncCall{context->ID()->getText(), std::move(arguments), std::nullopt});
        return MakeStmt(Statement::FuncCallStmt{std::move(call)});
    }

    std::any visitBlock(ImpParser::BlockContext *context) override {
        std::vector<StatementPtr> statements;
        statements.reserve(context->statement().size());
        for (ImpParser::StatementContext *statement_context : context->statement()) {
            statements.push_back(AnyCast<StatementPtr>(visit(statement_context), "visitBlock.statement"));
        }
        return statements;
    }

    std::any visitIfStatement(ImpParser::IfStatementContext *context) override {
        ExpressionPtr condition = AnyCast<ExpressionPtr>(
            visitParenthesizedCondition(context->parenthesizedCondition()), "visitIfStatement.condition");
        std::vector<StatementPtr> then_block =
            AnyCast<std::vector<StatementPtr>>(visitBlock(context->block(0)), "visitIfStatement.thenBlock");

        std::optional<std::vector<StatementPtr>> else_block;
        if (context->ELSE() != nullptr) {
            else_block =
                AnyCast<std::vector<StatementPtr>>(visitBlock(context->block(1)), "visitIfStatement.else");
        }

        return MakeStmt(Statement::If{std::move(condition), std::move(then_block), std::move(else_block)});
    }

    std::any visitWhileStatement(ImpParser::WhileStatementContext *context) override {
        ExpressionPtr condition = AnyCast<ExpressionPtr>(
            visitParenthesizedCondition(context->parenthesizedCondition()), "visitWhileStatement.condition");
        std::vector<ExpressionPtr> invariants;
        if (context->invariantList() != nullptr) {
            invariants = AnyCast<std::vector<ExpressionPtr>>(visitInvariantList(context->invariantList()),
                                                             "visitWhileStatement.invariants");
        }
        std::vector<StatementPtr> body =
            AnyCast<std::vector<StatementPtr>>(visitBlock(context->block()), "visitWhileStatement.body");

        return MakeStmt(Statement::While{std::move(condition), std::move(invariants), std::move(body)});
    }

    std::any visitInvariantList(ImpParser::InvariantListContext *context) override {
        std::vector<ExpressionPtr> invariants;
        invariants.reserve(context->expression().size());
        for (ImpParser::ExpressionContext *expression_context : context->expression()) {
            invariants.push_back(
                AnyCast<ExpressionPtr>(visit(expression_context), "visitInvariantList.expression"));
        }
        return invariants;
    }

    std::any visitParenthesizedCondition(ImpParser::ParenthesizedConditionContext *context) override {
        return visit(context->expression());
    }

    std::any visitAssignStatement(ImpParser::AssignStatementContext *context) override {
        ExpressionPtr lhs = AnyCast<ExpressionPtr>(visit(context->reference()), "visitAssignStatement.lhs");
        ExpressionPtr rhs = AnyCast<ExpressionPtr>(visit(context->expression()), "visitAssignStatement.rhs");
        return MakeStmt(Statement::Assign{std::move(lhs), std::move(rhs)});
    }

    std::any visitVarDecl(ImpParser::VarDeclContext *context) override {
        const DataType type = AnyCast<DataType>(visit(context->type()), "visitVarDecl.type");
        std::optional<ExpressionPtr> initializer;
        if (context->expression() != nullptr) {
            initializer = AnyCast<ExpressionPtr>(visit(context->expression()), "visitVarDecl.initializer");
        }

        return MakeStmt(Statement::VarDecl{type, context->ID()->getText(), std::move(initializer)});
    }

    std::any visitBoolType(ImpParser::BoolTypeContext * /*context*/) override {
        return DataType::kBool;
    }

    std::any visitIntType(ImpParser::IntTypeContext * /*context*/) override {
        return DataType::kInt;
    }

    std::any visitIntArrayType(ImpParser::IntArrayTypeContext * /*context*/) override {
        return DataType::kIntArray;
    }

    std::any visitBoolArrayType(ImpParser::BoolArrayTypeContext * /*context*/) override {
        return DataType::kBoolArray;
    }

    std::any visitParenType(ImpParser::ParenTypeContext *context) override {
        return visit(context->type());
    }

    std::any visitAndExpr(ImpParser::AndExprContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitAndExpr.left");
        ExpressionPtr right = AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitAndExpr.right");
        return MakeExpr(Expression::Binary{BinaryOp::kAnd, std::move(left), std::move(right)});
    }

    std::any visitTrueExpr(ImpParser::TrueExprContext * /*context*/) override {
        return MakeExpr(Expression::BoolLiteral{true});
    }

    std::any visitQuantifiedExpr(ImpParser::QuantifiedExprContext *context) override {
        const Parameter parameter = AnyCast<Parameter>(visitFormalParameter(context->formalParameter()),
                                                       "visitQuantifiedExpr.parameter");
        ExpressionPtr body = AnyCast<ExpressionPtr>(visit(context->expression()), "visitQuantifiedExpr.body");

        return MakeExpr(Expression::Quantified{context->FORALL() != nullptr, parameter.name, parameter.type,
                                               std::move(body)});
    }

    std::any visitReferenceExpr(ImpParser::ReferenceExprContext *context) override {
        return visit(context->reference());
    }

    std::any visitArrayLength(ImpParser::ArrayLengthContext *context) override {
        ExpressionPtr array =
            AnyCast<ExpressionPtr>(visit(context->expression()), "visitArrayLength.expression");
        return MakeExpr(Expression::ArrayLength{std::move(array)});
    }

    std::any visitCompExpr(ImpParser::CompExprContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitCompExpr.left");
        ExpressionPtr right = AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitCompExpr.right");

        BinaryOp op = BinaryOp::kLess;
        if (context->LESS() != nullptr) {
            op = BinaryOp::kLess;
        } else if (context->LEQ() != nullptr) {
            op = BinaryOp::kLessOrEqual;
        } else if (context->GREATER() != nullptr) {
            op = BinaryOp::kGreater;
        } else if (context->GEQ() != nullptr) {
            op = BinaryOp::kGreaterOrEqual;
        }

        return MakeExpr(Expression::Binary{op, std::move(left), std::move(right)});
    }

    std::any visitUnaryExpr(ImpParser::UnaryExprContext *context) override {
        ExpressionPtr operand =
            AnyCast<ExpressionPtr>(visit(context->expression()), "visitUnaryExpr.operand");
        const UnaryOp op = context->MINUS() != nullptr ? UnaryOp::kNeg : UnaryOp::kNot;
        return MakeExpr(Expression::Unary{op, std::move(operand)});
    }

    std::any visitOrExpr(ImpParser::OrExprContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitOrExpr.left");
        ExpressionPtr right = AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitOrExpr.right");
        return MakeExpr(Expression::Binary{BinaryOp::kOr, std::move(left), std::move(right)});
    }

    std::any visitFalseExpr(ImpParser::FalseExprContext * /*context*/) override {
        return MakeExpr(Expression::BoolLiteral{false});
    }

    std::any visitNewArray(ImpParser::NewArrayContext *context) override {
        const DataType type = AnyCast<DataType>(visit(context->type()), "visitNewArray.type");
        ExpressionPtr size = AnyCast<ExpressionPtr>(visit(context->expression()), "visitNewArray.size");
        return MakeExpr(Expression::NewArray{type, std::move(size)});
    }

    std::any visitF_Implies(ImpParser::F_ImpliesContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitF_Implies.left");
        ExpressionPtr right = AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitF_Implies.right");
        return MakeExpr(Expression::Binary{BinaryOp::kImplies, std::move(left), std::move(right)});
    }

    std::any visitMulDivModExpr(ImpParser::MulDivModExprContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitMulDivModExpr.left");
        ExpressionPtr right =
            AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitMulDivModExpr.right");

        BinaryOp op = BinaryOp::kMul;
        if (context->TIMES() != nullptr) {
            op = BinaryOp::kMul;
        } else if (context->INTDIV() != nullptr) {
            op = BinaryOp::kDiv;
        } else if (context->INTMOD() != nullptr) {
            op = BinaryOp::kMod;
        }

        return MakeExpr(Expression::Binary{op, std::move(left), std::move(right)});
    }

    std::any visitEqExpr(ImpParser::EqExprContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitEqExpr.left");
        ExpressionPtr right = AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitEqExpr.right");
        return MakeExpr(Expression::Binary{BinaryOp::kEqual, std::move(left), std::move(right)});
    }

    std::any visitIntExpr(ImpParser::IntExprContext *context) override {
        const std::int64_t value = std::stoll(context->INT()->getText());
        return MakeExpr(Expression::IntLiteral{value});
    }

    std::any visitParenExpr(ImpParser::ParenExprContext *context) override {
        return visit(context->expression());
    }

    std::any visitAddSubExpr(ImpParser::AddSubExprContext *context) override {
        ExpressionPtr left = AnyCast<ExpressionPtr>(visit(context->expression(0)), "visitAddSubExpr.left");
        ExpressionPtr right = AnyCast<ExpressionPtr>(visit(context->expression(1)), "visitAddSubExpr.right");

        const BinaryOp op = context->PLUS() != nullptr ? BinaryOp::kAdd : BinaryOp::kSub;
        return MakeExpr(Expression::Binary{op, std::move(left), std::move(right)});
    }

    std::any visitFuncCallExpr(ImpParser::FuncCallExprContext *context) override {
        std::vector<ExpressionPtr> arguments;
        if (context->exprList() != nullptr) {
            arguments =
                AnyCast<std::vector<ExpressionPtr>>(visitExprList(context->exprList()), "visitFuncCallExpr");
        }
        return MakeExpr(Expression::FuncCall{context->ID()->getText(), std::move(arguments), std::nullopt});
    }

    std::any visitVarRef(ImpParser::VarRefContext *context) override {
        return MakeExpr(Expression::VarRef{context->ID()->getText(), std::nullopt});
    }

    std::any visitArrayRef(ImpParser::ArrayRefContext *context) override {
        ExpressionPtr index = AnyCast<ExpressionPtr>(visit(context->expression()), "visitArrayRef.index");
        return MakeExpr(Expression::ArrayRef{context->ID()->getText(), std::move(index), std::nullopt});
    }

    std::any visitExprList(ImpParser::ExprListContext *context) override {
        std::vector<ExpressionPtr> expressions;
        expressions.reserve(context->expression().size());
        for (ImpParser::ExpressionContext *expression_context : context->expression()) {
            expressions.push_back(
                AnyCast<ExpressionPtr>(visit(expression_context), "visitExprList.expression"));
        }
        return expressions;
    }
};

[[nodiscard]] Program ParseStringInternal(std::string_view program_text) {
    antlr4::ANTLRInputStream input{std::string(program_text)};
    ImpLexer lexer(&input);
    ThrowingErrorListener error_listener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&error_listener);

    antlr4::CommonTokenStream tokens(&lexer);
    ImpParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&error_listener);

    ImpParser::ParseContext *tree = parser.parse();
    AstBuilder builder;
    return AnyCast<Program>(builder.visitParse(tree), "ParseStringInternal.root");
}

[[nodiscard]] std::string ReadFile(const std::filesystem::path &file_path) {
    std::ifstream input(file_path);
    if (!input.good()) {
        throw InputError("Failed to read file " + file_path.string() + ".");
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

} // namespace

Program ParseString(std::string_view program_text) {
    return ParseStringInternal(program_text);
}

Program ParseStringCheckingTypes(std::string_view program_text) {
    Program program = ParseStringInternal(program_text);
    CheckTypes(program);
    return program;
}

Program ParseFile(const std::filesystem::path &file_path) {
    if (!std::filesystem::exists(file_path)) {
        throw InputError("File " + file_path.string() + " does not exist.");
    }
    if (file_path.extension() != ".imp") {
        throw InputError("File " + file_path.string() + " does not have .imp extension.");
    }
    return ParseStringInternal(ReadFile(file_path));
}

Program ParseFileCheckingTypes(const std::filesystem::path &file_path) {
    Program program = ParseFile(file_path);
    CheckTypes(program);
    return program;
}

} // namespace ivy
