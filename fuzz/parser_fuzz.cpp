#include <cstddef>
#include <cstdint>
#include <string>

#include "ivy/parser.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
    try {
        const std::string input(reinterpret_cast<const char *>(data), size);
        static_cast<void>(ivy::ParseString(input));
    } catch (...) {
    }
    return 0;
}
