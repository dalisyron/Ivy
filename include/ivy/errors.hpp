#pragma once

#include <stdexcept>
#include <string>

namespace ivy {

class IvyError : public std::runtime_error {
  public:
    explicit IvyError(const std::string &message) : std::runtime_error(message) {}
};

class InputError : public IvyError {
  public:
    explicit InputError(const std::string &message) : IvyError(message) {}
};

class ParseError : public IvyError {
  public:
    ParseError(int line, int column, const std::string &message)
        : IvyError(message), line_(line), column_(column) {}

    [[nodiscard]] int line() const noexcept {
        return line_;
    }
    [[nodiscard]] int column() const noexcept {
        return column_;
    }

  private:
    int line_;
    int column_;
};

class TypeError : public IvyError {
  public:
    explicit TypeError(const std::string &message) : IvyError("TypeError: " + message) {}
};

class VerificationError : public IvyError {
  public:
    explicit VerificationError(const std::string &message) : IvyError(message) {}
};

} // namespace ivy
