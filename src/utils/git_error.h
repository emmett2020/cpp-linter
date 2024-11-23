#pragma once
#include <stdexcept>

#include <git2/errors.h>

namespace linter::git {
  class git_exception : std::runtime_error {
  public:
    git_exception(int error, const std::string &msg)
      : error_(error)
      , std::runtime_error(msg) {
    }

    [[nodiscard]] auto error_code() const -> int {
      return error_;
    }

  private:
    int error_;
  };

  inline void throw_if(int error, const std::string &msg) {
    if (error < 0) {
      throw git_exception{error, msg};
    }
  }

  inline void throw_if(int error) {
    if (error < 0) {
      auto *msg = ::git_error_last()->message;
      throw git_exception{error, msg};
    }
  }

  /// Will throw a GIT_ERROR with given message if condition is true.
  inline void throw_if(bool condition, const std::string &msg) {
    if (condition) {
      throw_if(::git_error_code::GIT_ERROR, msg);
    }
  }

  /// Will throw a GIT_ERROR with given message if condition isn't true.
  inline void throw_unless(bool condition, const std::string &msg) {
    if (!condition) {
      throw_if(::git_error_code::GIT_ERROR, msg);
    }
  }

  inline void throw_unsupported() {
    throw git_exception{GIT_ERROR, "unsupported"};
  }
} // namespace linter::git


