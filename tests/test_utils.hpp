#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "ivy/ast.hpp"

namespace ivy::test {

struct ProgramCase final {
    std::string name;
    std::string program;
    bool expected_success;
};

[[nodiscard]] std::string ReadTextFile(const std::filesystem::path &file_path);
[[nodiscard]] std::vector<std::filesystem::path> ListImpFiles(const std::filesystem::path &directory);

[[nodiscard]] std::vector<ProgramCase> ExtractInlineVerificationCases();
[[nodiscard]] std::vector<ProgramCase> ExtractTypeCheckerCases();

[[nodiscard]] std::string SanitizeTestName(const std::string &input);

} // namespace ivy::test
