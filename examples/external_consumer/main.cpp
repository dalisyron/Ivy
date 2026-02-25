#include <exception>
#include <iostream>

#include "ivy/parser.hpp"
#include "ivy/verifier.hpp"

int main() {
    constexpr const char *kProgram = R"(
method Double(int x) returns (int y)
    ensures y == x + x
{
    y = x + x;
}
)";

    try {
        const ivy::Program program = ivy::ParseStringCheckingTypes(kProgram);
        const std::vector<ivy::VerificationResult> results = ivy::VerifyProgram(program);

        for (const ivy::VerificationResult &result : results) {
            std::cout << result.method_name << ": " << (result.valid ? "valid" : "invalid") << '\n';
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
