#include <vector>

#include <gtest/gtest.h>

#include "ivy/errors.hpp"
#include "ivy/parser.hpp"
#include "test_utils.hpp"

namespace ivy::test {
namespace {

[[nodiscard]] const std::vector<ProgramCase> &TypeCases() {
    static const std::vector<ProgramCase> cases = ExtractTypeCheckerCases();
    return cases;
}

class TypeCheckerParityTest : public ::testing::TestWithParam<ProgramCase> {};

} // namespace

TEST(TypeCheckerParityMeta, CaseCountMatchesJavaBaseline) {
    EXPECT_EQ(TypeCases().size(), 45U);
}

TEST_P(TypeCheckerParityTest, MatchesJavaExpectation) {
    const ProgramCase &test_case = GetParam();

    if (test_case.expected_success) {
        EXPECT_NO_THROW({
            Program program = ParseStringCheckingTypes(test_case.program);
            EXPECT_FALSE(program.methods.empty());
        });
    } else {
        EXPECT_THROW(
            {
                Program program = ParseStringCheckingTypes(test_case.program);
                static_cast<void>(program);
            },
            TypeError);
    }
}

INSTANTIATE_TEST_SUITE_P(JavaTypeChecker, TypeCheckerParityTest, ::testing::ValuesIn(TypeCases()),
                         [](const ::testing::TestParamInfo<ProgramCase> &info) {
                             return SanitizeTestName(info.param.name);
                         });

} // namespace ivy::test
