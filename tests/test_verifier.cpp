#include <filesystem>
#include <string>
#include <vector>

#include "ivy/z3_compat.hpp"
#include <gtest/gtest.h>

#include "ivy/ast.hpp"
#include "ivy/parser.hpp"
#include "ivy/verifier.hpp"
#include "test_utils.hpp"

namespace ivy::test {
namespace {

[[nodiscard]] std::filesystem::path SourceRoot() {
#ifdef IVY_SOURCE_DIR
    return std::filesystem::path(IVY_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

[[nodiscard]] const std::vector<ProgramCase> &InlineCases() {
    static const std::vector<ProgramCase> cases = ExtractInlineVerificationCases();
    return cases;
}

class InlineVerificationParityTest : public ::testing::TestWithParam<ProgramCase> {};

} // namespace

TEST(VerifierParityMeta, InlineCaseCountMatchesJavaBaseline) {
    EXPECT_EQ(InlineCases().size(), 37U);
}

TEST_P(InlineVerificationParityTest, MatchesJavaExpectation) {
    const ProgramCase &test_case = GetParam();

    Program program = ParseStringCheckingTypes(test_case.program);
    ASSERT_FALSE(program.methods.empty());

    z3::context context;
    for (const MethodDeclaration &method : program.methods) {
        const bool valid = VerifyMethod(context, method);
        EXPECT_EQ(valid, test_case.expected_success) << "Method: " << method.name;
    }
}

INSTANTIATE_TEST_SUITE_P(JavaInlineVerification, InlineVerificationParityTest,
                         ::testing::ValuesIn(InlineCases()),
                         [](const ::testing::TestParamInfo<ProgramCase> &param_info) {
                             return SanitizeTestName(param_info.param.name);
                         });

TEST(VerifierSamplePrograms, ExamplesCorpusAllValid) {
    const std::filesystem::path examples_dir = SourceRoot() / "TestData" / "Examples";
    const std::vector<std::filesystem::path> files = ListImpFiles(examples_dir);
    ASSERT_FALSE(files.empty());

    for (const std::filesystem::path &file : files) {
        SCOPED_TRACE(file.string());
        Program program = ParseStringCheckingTypes(ReadTextFile(file));
        z3::context context;
        for (const MethodDeclaration &method : program.methods) {
            EXPECT_TRUE(VerifyMethod(context, method)) << "Expected valid method: " << method.name;
        }
    }
}

TEST(VerifierSamplePrograms, SolverSanityUnsat) {
    z3::context context;
    z3::expr a = context.int_const("a");

    z3::sort_vector domain(context);
    domain.push_back(context.int_sort());
    z3::func_decl f = context.function("f", domain, context.int_sort());

    z3::expr p1 = f(f(f(a))) == a;
    z3::expr p2 = f(f(f(f(f(a))))) == a;
    z3::expr p3 = !(f(a) == a);

    z3::solver solver(context);
    solver.add(p1);
    solver.add(p2);
    solver.add(p3);

    EXPECT_EQ(solver.check(), z3::unsat);
}

TEST(VerifierUnits, AssignmentAwpAndAvc) {
    z3::context context;

    ExpressionPtr lhs = MakeExpr(Expression::VarRef{"x", DataType::kInt});
    ExpressionPtr rhs = MakeExpr(Expression::IntLiteral{1});
    Statement assignment(Statement::Assign{lhs, rhs});

    z3::expr x = context.int_const("x");
    z3::expr q = (x == context.int_val(1));

    const z3::expr awp = GenerateAwp(context, assignment, q);
    const z3::expr avc = GenerateAvc(context, assignment, q);

    z3::solver solver(context);
    solver.add(!(awp && avc));

    EXPECT_EQ(solver.check(), z3::unsat);
}

TEST(VerifierUnits, IfElseAwpAndAvc) {
    ExpressionPtr var_x = MakeExpr(Expression::VarRef{"x", DataType::kInt});
    ExpressionPtr var_y_then = MakeExpr(Expression::VarRef{"y", DataType::kInt});
    ExpressionPtr var_y_else = MakeExpr(Expression::VarRef{"y", DataType::kInt});

    ExpressionPtr condition =
        MakeExpr(Expression::Binary{BinaryOp::kLess, var_x, MakeExpr(Expression::IntLiteral{1})});

    StatementPtr then_assign = MakeStmt(Statement::Assign{var_y_then, MakeExpr(Expression::IntLiteral{1})});
    StatementPtr else_assign = MakeStmt(Statement::Assign{var_y_else, MakeExpr(Expression::IntLiteral{2})});

    Statement if_statement(Statement::If{condition, std::vector<StatementPtr>{then_assign},
                                         std::vector<StatementPtr>{else_assign}});

    z3::context context;
    z3::expr x = context.int_const("x");
    z3::expr y = context.int_const("y");

    z3::expr p1 = (x == context.int_val(0));
    z3::expr p2 = (x == context.int_val(2));
    z3::expr q1 = (y == context.int_val(1));
    z3::expr q2 = (y == context.int_val(2));

    const z3::expr awp1 = GenerateAwp(context, if_statement, q1);
    const z3::expr avc1 = GenerateAvc(context, if_statement, q1);
    const z3::expr awp2 = GenerateAwp(context, if_statement, q2);
    const z3::expr avc2 = GenerateAvc(context, if_statement, q2);

    const z3::expr vc1 = avc1 && z3::implies(p1, awp1);
    const z3::expr vc2 = avc2 && z3::implies(p2, awp2);

    z3::solver solver(context);
    solver.add(!(vc1 && vc2));

    EXPECT_EQ(solver.check(), z3::unsat);
}

} // namespace ivy::test
