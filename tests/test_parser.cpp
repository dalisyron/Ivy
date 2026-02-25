#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "ivy/parser.hpp"
#include "ivy/pretty_printer.hpp"
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

void AssertRoundTrip(const std::string &program) {
    const Program parsed = ParseString(program);
    const std::string printed = PrettyPrint(parsed);
    const Program reparsed = ParseString(printed);
    EXPECT_EQ(parsed, reparsed);
}

} // namespace

TEST(ParserAstRoundTrip, ParserCorpus) {
    const std::filesystem::path parser_dir = SourceRoot() / "TestData" / "Parser";
    const std::vector<std::filesystem::path> files = ListImpFiles(parser_dir);
    ASSERT_FALSE(files.empty());

    for (const std::filesystem::path &file : files) {
        SCOPED_TRACE(file.string());
        AssertRoundTrip(ReadTextFile(file));
    }
}

TEST(ParserAstRoundTrip, SingleParserFile) {
    const std::filesystem::path file = SourceRoot() / "TestData" / "Parser" / "Test1.imp";
    AssertRoundTrip(ReadTextFile(file));
}

TEST(ParserAstRoundTrip, InlineProgram) {
    constexpr const char *kProgram = R"(
method Bar()
    requires y > 10 && true
{
    x = (x || y) && z;
    if (x < 10) {
        y = z ==> z;
    }
    int[] a;
    if (x == 10) {
        while (true)
            invariant y > 10
            invariant forall (bool x) :: x == y + 20
        {
            if (x == 10) {
            }
        }
    }
}
)";

    AssertRoundTrip(kProgram);
}

} // namespace ivy::test
