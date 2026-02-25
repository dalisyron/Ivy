#include "test_utils.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include "ivy/errors.hpp"

namespace ivy::test {
namespace {

[[nodiscard]] std::filesystem::path SourceRoot() {
#ifdef IVY_SOURCE_DIR
    return std::filesystem::path(IVY_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

[[nodiscard]] std::vector<std::string> SplitTestSegments(const std::string &source) {
    std::vector<std::string> segments;
    std::size_t position = source.find("@Test");
    while (position != std::string::npos) {
        const std::size_t next = source.find("@Test", position + 5U);
        if (next == std::string::npos) {
            segments.push_back(source.substr(position));
            break;
        }
        segments.push_back(source.substr(position, next - position));
        position = next;
    }
    return segments;
}

[[nodiscard]] std::optional<std::string> ExtractMethodName(const std::string &segment) {
    static const std::regex kMethodRegex(R"(void\s+([A-Za-z0-9_]+)\s*\()", std::regex::ECMAScript);
    std::smatch match;
    if (std::regex_search(segment, match, kMethodRegex) && match.size() >= 2) {
        return match[1].str();
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<std::string> ExtractProgramTextBlock(const std::string &segment) {
    static constexpr std::string_view kMarker = "String program = \"\"\"";
    const std::size_t start = segment.find(kMarker.data());
    if (start == std::string::npos) {
        return std::nullopt;
    }
    const std::size_t program_start = start + kMarker.size();
    const std::size_t end = segment.find("\"\"\";", program_start);
    if (end == std::string::npos) {
        throw std::runtime_error("Malformed Java text block in test source");
    }
    return segment.substr(program_start, end - program_start);
}

[[nodiscard]] std::vector<ProgramCase> ExtractCases(const std::filesystem::path &file_path,
                                                    const std::string &success_marker,
                                                    const std::string &failure_marker) {
    const std::string source = ReadTextFile(file_path);
    const std::vector<std::string> segments = SplitTestSegments(source);

    std::vector<ProgramCase> cases;
    for (const std::string &segment : segments) {
        const std::optional<std::string> maybe_program = ExtractProgramTextBlock(segment);
        if (!maybe_program.has_value()) {
            continue;
        }

        const std::optional<std::string> maybe_name = ExtractMethodName(segment);
        if (!maybe_name.has_value()) {
            throw std::runtime_error("Could not parse Java test method name in " + file_path.string());
        }

        const bool has_success = segment.find(success_marker) != std::string::npos;
        const bool has_failure = segment.find(failure_marker) != std::string::npos;

        if (has_success == has_failure) {
            throw std::runtime_error("Could not infer expectation for Java test method " + *maybe_name);
        }

        cases.push_back(ProgramCase{*maybe_name, *maybe_program, has_success});
    }

    return cases;
}

} // namespace

std::string ReadTextFile(const std::filesystem::path &file_path) {
    std::ifstream input(file_path);
    if (!input.good()) {
        throw InputError("Failed to read file " + file_path.string() + ".");
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::vector<std::filesystem::path> ListImpFiles(const std::filesystem::path &directory) {
    std::vector<std::filesystem::path> files;
    for (const std::filesystem::directory_entry &entry :
         std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".imp") {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::vector<ProgramCase> ExtractInlineVerificationCases() {
    const std::filesystem::path path =
        SourceRoot() / "src" / "test" / "java" / "imp" / "SampleInlineProgramVerificationTests.java";
    return ExtractCases(path, "assertProgramVerifies(program)", "assertProgramNotVerifies(program)");
}

std::vector<ProgramCase> ExtractTypeCheckerCases() {
    const std::filesystem::path path =
        SourceRoot() / "src" / "test" / "java" / "imp" / "typechecker" / "TypeCheckerTest.java";
    return ExtractCases(path, "testTypeCheckerOnProgramString(program);",
                        "Assertions.assertThrows(TypeError.class");
}

std::string SanitizeTestName(const std::string &input) {
    std::string output;
    output.reserve(input.size());

    for (const char character : input) {
        const unsigned char value = static_cast<unsigned char>(character);
        if (std::isalnum(value) != 0) {
            output.push_back(character);
        } else {
            output.push_back('_');
        }
    }

    if (output.empty()) {
        output = "unnamed";
    }

    if (std::isdigit(static_cast<unsigned char>(output.front())) != 0) {
        output.insert(output.begin(), '_');
    }

    return output;
}

} // namespace ivy::test
