#include <exception>
#include <iostream>

#include "ivy/z3_compat.hpp"

#include "ivy/parser.hpp"
#include "ivy/verifier.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ivy_verify <input-file.imp>\n";
        return 1;
    }

    try {
        ivy::Program program = ivy::ParseFileCheckingTypes(argv[1]);
        z3::context context;

        for (const ivy::MethodDeclaration &method : program.methods) {
            const bool valid = ivy::VerifyMethod(context, method);
            std::cout << "Method " << method.name << " is " << (valid ? "valid" : "invalid") << '\n';
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
