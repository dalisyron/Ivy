#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "ivy/errors.hpp"
#include "ivy/parser.hpp"
#include "ivy/verifier.hpp"
#include "ivy/z3_compat.hpp"
#include <gtest/gtest.h>

namespace ivy::test {
namespace {

void ExpectTypeErrorContains(const std::string &program, const std::string &needle) {
    try {
        Program parsed = ParseStringCheckingTypes(program);
        static_cast<void>(parsed);
        FAIL() << "Expected TypeError containing: " << needle;
    } catch (const TypeError &error) {
        const std::string message = error.what();
        EXPECT_NE(message.find(needle), std::string::npos) << message;
    }
}

TEST(ParserCoverage, CoversOptionalGrammarPathsAndOperators) {
    constexpr const char *kProgram = R"(
method Callee(int p, bool q) returns (int out)
    requires q || !q
    ensures out >= p
{
    int[] arr = new int[3];
    bool[] flags;
    int i = 0;
    bool b = true && false || !false ==> true;
    if (q) {
        out = p + 1 - 2 * 3 / 1 % 2;
    } else {
        out = p;
    }
    while (i <= arr.length)
        invariant i >= 0
        invariant exists (int x) :: x >= 0
    {
        i = i + 1;
    }
}

method Caller() returns (int x)
{
    bool ok = false;
    x = Callee(1, ok);
    Callee(x, ok);
}

method NoCondsNoReturns()
{
    int a;
    a = 1;
}
)";

    Program parsed = ParseString(kProgram);
    EXPECT_EQ(parsed.methods.size(), 3U);

    Program typed = ParseStringCheckingTypes(kProgram);
    EXPECT_EQ(typed.methods.size(), 3U);
}

TEST(ParserCoverage, ParseStringReportsSyntaxLocation) {
    try {
        static_cast<void>(ParseString("method M(){ int x = ; }"));
        FAIL() << "Expected ParseError";
    } catch (const ParseError &error) {
        EXPECT_GE(error.line(), 1);
        EXPECT_GE(error.column(), 0);
        EXPECT_NE(std::string(error.what()).find("Syntax error at line"), std::string::npos);
    }
}

TEST(ParserCoverage, ParseFileErrors) {
    const std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    const std::filesystem::path wrong_ext = temp_dir / "ivy_branch_coverage.txt";
    {
        std::ofstream out(wrong_ext);
        out << "method M() {}\n";
    }

    EXPECT_THROW(static_cast<void>(ParseFile(temp_dir / "definitely_missing_ivy_file.imp")), InputError);
    EXPECT_THROW(static_cast<void>(ParseFile(wrong_ext)), InputError);

    std::filesystem::remove(wrong_ext);
}

TEST(TypeCheckerCoverage, ParameterAndReturnDeclarationErrors) {
    ExpectTypeErrorContains("method M(int x, int x) { }", "Parameter x already declared.");
    ExpectTypeErrorContains("method M(int x) returns (int x) { }",
                            "Return variable x already declared as a parameter.");
}

TEST(TypeCheckerCoverage, ConditionAndControlFlowErrors) {
    ExpectTypeErrorContains("method M() requires 1 { }", "Requires clause must be of type bool.");
    ExpectTypeErrorContains("method M() ensures 1 { }", "Ensures clause must be of type bool.");
    ExpectTypeErrorContains("method M(){ if (1) { } }", "If condition must be of type bool.");
    ExpectTypeErrorContains("method M(){ while (1) { } }", "While condition must be of type bool.");
    ExpectTypeErrorContains("method M(){ int x = 0; while (x < 1) invariant 1 { x = x + 1; } }",
                            "Invariant expression must be of type bool.");
}

TEST(TypeCheckerCoverage, AssignmentAndDeclarationErrors) {
    ExpectTypeErrorContains("method M(){ int x; x = true; }", "Cannot assign bool");
    ExpectTypeErrorContains("method M(){ int x = true; }", "Cannot assign bool to variable x");
    ExpectTypeErrorContains("method M(){ int x; int x; }", "Variable x already declared.");
}

TEST(TypeCheckerCoverage, FunctionCallErrors) {
    ExpectTypeErrorContains("method M(){ int x = Unknown(); }", "Unknown function: Unknown");
    ExpectTypeErrorContains(
        "method Add(int a, int b) returns (int r){ r = a + b; } method M(){ int x = Add(1); }",
        "expects 2 arguments");
    ExpectTypeErrorContains("method Add(int a) returns (int r){ r = a; } method M(){ int x = Add(true); }",
                            "expects type int");
}

TEST(TypeCheckerCoverage, ExpressionErrors) {
    ExpectTypeErrorContains("method M(){ int x = !1; }", "Invalid operand for operator '!'");
    ExpectTypeErrorContains("method M(){ int x = 1 + true; }", "Invalid operands for operator '+'");
    ExpectTypeErrorContains("method M(){ bool b = forall (int i) :: i; }",
                            "Body of forall expression must be of type bool.");
    ExpectTypeErrorContains("method M(){ bool b = exists (int i) :: i; }",
                            "Body of exists expression must be of type bool.");
    ExpectTypeErrorContains("method M(){ int[] a = new int[true]; }", "Array size must be of type int.");
    ExpectTypeErrorContains("method M(){ int x = 1.length; }", "Cannot get length of non-array type.");
    ExpectTypeErrorContains("method M(){ int x = arr[0]; }", "Array arr is not declared.");
    ExpectTypeErrorContains("method M(){ int[] arr = new int[1]; int x = arr[true]; }",
                            "Array index must be of type int.");
    ExpectTypeErrorContains("method M(){ int x = 0; int y = x[0]; }", "x is not an array.");
}

TEST(VerifierCoverage, AwpAndAvcRejectNullAndUnsupportedNodes) {
    z3::context context;
    z3::expr q = context.bool_val(true);

    Statement null_assign(Statement::Assign{nullptr, MakeExpr(Expression::IntLiteral{1})});
    EXPECT_THROW(static_cast<void>(GenerateAwp(context, null_assign, q)), VerificationError);

    Statement null_if(Statement::If{nullptr, std::vector<StatementPtr>{}, std::nullopt});
    EXPECT_THROW(static_cast<void>(GenerateAwp(context, null_if, q)), VerificationError);

    Statement null_while(
        Statement::While{nullptr, std::vector<ExpressionPtr>{}, std::vector<StatementPtr>{}});
    EXPECT_THROW(static_cast<void>(GenerateAvc(context, null_while, q)), VerificationError);

    Statement null_init(
        Statement::VarDecl{DataType::kInt, "x", std::optional<ExpressionPtr>{ExpressionPtr{}}});
    EXPECT_THROW(static_cast<void>(GenerateAwp(context, null_init, q)), VerificationError);

    Statement call_stmt(Statement::FuncCallStmt{MakeExpr(Expression::FuncCall{"f", {}, std::nullopt})});
    EXPECT_THROW(static_cast<void>(GenerateAwp(context, call_stmt, q)), VerificationError);
    EXPECT_THROW(static_cast<void>(GenerateAvc(context, call_stmt, q)), VerificationError);
}

TEST(VerifierCoverage, BlockEdgeCasesAndProgramEntryPoint) {
    z3::context context;
    z3::expr q = context.bool_val(true);

    Statement empty_block(Statement::Block{std::vector<StatementPtr>{}});
    EXPECT_NO_THROW(static_cast<void>(GenerateAwp(context, empty_block, q)));
    EXPECT_NO_THROW(static_cast<void>(GenerateAvc(context, empty_block, q)));

    Statement bad_block(Statement::Block{std::vector<StatementPtr>{nullptr}});
    EXPECT_THROW(static_cast<void>(GenerateAwp(context, bad_block, q)), VerificationError);
    EXPECT_THROW(static_cast<void>(GenerateAvc(context, bad_block, q)), VerificationError);

    constexpr const char *kProgram = R"(
method Trivial(int x) returns (int y)
    requires x >= 0
    ensures y >= 0
{
    y = x;
}
)";

    Program parsed = ParseStringCheckingTypes(kProgram);
    const std::vector<VerificationResult> results = VerifyProgram(parsed);
    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results.front().method_name, "Trivial");
    EXPECT_TRUE(results.front().valid);
}

} // namespace
} // namespace ivy::test
