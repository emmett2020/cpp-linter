#pragma once
#include <stdexcept>

#include <git2/errors.h>

namespace linter::git {
  inline void throw_if(int error, const std::string &msg) {
    if (error < 0) {
      throw std::runtime_error{msg};
    }
  }

  inline void throw_if(int error) {
    if (error < 0) {
      auto *msg = ::git_error_last()->message;
      throw std::runtime_error{msg};
    }
  }

  /// Will throw a GIT_ERROR with given message if condition is true.
  inline void throw_if(bool condition, const std::string &msg) {
    if (condition) {
      throw std::runtime_error{msg};
    }
  }

  /// Will throw a GIT_ERROR with given message if condition isn't true.
  inline void throw_unless(bool condition, const std::string &msg) {
    if (!condition) {
      throw std::runtime_error{msg};
    }
  }

  inline void throw_unsupported() {
    throw std::runtime_error{"unsupported"};
  }
} // namespace linter::git


