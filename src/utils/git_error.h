/*
 * Copyright (c) 2024 Emmett Zhang
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
