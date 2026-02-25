#include <exception>
#include <iostream>

#include "ivy/z3_compat.hpp"

#include "ivy/parser.hpp"
#include "ivy/verifier.hpp"

int main() {
    constexpr const char *kProgram = R"(
method Increment(int x) returns (int y)
    ensures y == x + 1
{
    y = x + 1;
}
)";

    try {
        ivy::Program program = ivy::ParseStringCheckingTypes(kProgram);
        z3::context context;

        for (const ivy::MethodDeclaration &method : program.methods) {
            const bool valid = ivy::VerifyMethod(context, method);
            std::cout << method.name << ": " << (valid ? "valid" : "invalid") << '\n';
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
