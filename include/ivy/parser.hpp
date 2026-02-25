#pragma once

#include <filesystem>
#include <string_view>

#include "ivy/ast.hpp"

namespace ivy {

[[nodiscard]] Program ParseString(std::string_view program_text);
[[nodiscard]] Program ParseStringCheckingTypes(std::string_view program_text);
[[nodiscard]] Program ParseFile(const std::filesystem::path &file_path);
[[nodiscard]] Program ParseFileCheckingTypes(const std::filesystem::path &file_path);

} // namespace ivy
