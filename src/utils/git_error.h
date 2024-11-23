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
    auto *msg = ::git_error_last()->message;
    throw_if(error, msg);
  }

  // inline void throw_if(bool condition) {
  //   if (condition) {
  //     auto *msg = ::git_error_last()->message;
  //     throw_if(::git_error_code::GIT_ERROR, msg);
  //   }
  // }

  inline void throw_if(bool condition, const std::string &msg) {
    if (condition) {
      throw_if(::git_error_code::GIT_ERROR, msg);
    }
  }

  inline void throw_unless(bool condition, const std::string &msg) {
    if (!condition) {
      throw_if(::git_error_code::GIT_ERROR, msg);
    }
  }
} // namespace linter::git


