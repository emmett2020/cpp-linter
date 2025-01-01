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

#include <concepts>
#include <stdexcept>
#include <string_view>

#include <boost/regex.hpp>
#include <range/v3/all.hpp>

namespace linter {
  constexpr auto trim_left(std::string_view str) -> std::string_view {
    if (auto idx = str.find_first_not_of(" \n"); idx < str.size()) {
      str.remove_prefix(idx);
    }
    return str;
  }

  constexpr auto trim_right(std::string_view str) -> std::string_view {
    auto idx = str.find_last_not_of(" \n");
    if (idx < str.size()) {
      str.remove_suffix(str.size() - idx - 1);
    }
    return str;
  }

  /// @brief: Trim both left and right.
  /// @param: str String to be trimmed.
  /// @return: The trimmed string.
  constexpr auto trim(std::string_view str) -> std::string_view {
    return trim_right(trim_left(str));
  }

  /// @brief: Throw a std::runtime_error with given msg if condition is true.
  /// @param: condition Condition to be checked.
  /// @param: msg Message used to be construct std::runtime_error.
  inline void throw_if(bool condition, const std::string &msg) {
    if (condition) {
      throw std::runtime_error{msg};
    }
  }

  template <typename Func>
  concept noexcept_str_func = requires(Func &&func) {
    requires noexcept(func());
    { func() } -> std::convertible_to<std::string>;
  };

  template <noexcept_str_func Func>
  void throw_if(bool condition, Func &&func) {
    if (condition) {
      auto msg = std::forward<Func>(func)();
      throw std::runtime_error{msg};
    }
  }

  /// @brief: Throw a std::runtime_error with given msg if condition isn't true.
  /// @param: condition Condition to be checked.
  /// @param: msg Message used to be construct std::runtime_error.
  inline void throw_unless(bool condition, const std::string &msg) {
    if (!condition) {
      throw std::runtime_error{msg};
    }
  }

  template <noexcept_str_func Func>
  void throw_unless(bool condition, Func &&func) {
    if (!condition) {
      auto msg = std::forward<Func>(func)();
      throw std::runtime_error{msg};
    }
  }

  inline bool filter_file(const std::string &iregex, const std::string &file) {
    auto regex = boost::regex{iregex, boost::regex::icase};
    return !boost::regex_match(file, regex);
  }

  inline auto concat(const std::vector<std::string> &strs, char delim = '\n') -> std::string {
    return ranges::views::join(strs, delim) | ranges::to<std::string>();

    // return strs | std::views::join_with(delim) |
    // std::ranges::to<std::string>();
  }

  /// A replacement of std::unreachable() (since C++23)
  [[noreturn]] inline void unreachable() {
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else                                        // GCC, Clang
    __builtin_unreachable();
#endif
  }

} // namespace linter
