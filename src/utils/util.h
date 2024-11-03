#pragma once

#include <stdexcept>
#include <string_view>

namespace linter {
  constexpr auto TrimLeft(std::string_view str) -> std::string_view {
    if (auto idx = str.find_first_not_of(" \n"); idx < str.size()) {
      str.remove_prefix(idx);
    }
    return str;
  }

  constexpr auto TrimRight(std::string_view str) -> std::string_view {
    auto idx = str.find_last_not_of(" \n");
    if (idx < str.size()) {
      str.remove_suffix(str.size() - idx - 1);
    }
    return str;
  }

  /// @brief: Trim both left and right.
  /// @param: str String to be trimmed.
  /// @return: The trimmed string.
  constexpr auto Trim(std::string_view str) -> std::string_view {
    return TrimRight(TrimLeft(str));
  }

  /// @brief: Throw a std::runtime_error with given msg if condition is true.
  /// @param: condition Condition to be checked.
  /// @param: msg Message used to be construct std::runtime_error.
  inline void ThrowIf(bool condition, const std::string& msg) {
    if (condition) {
      throw std::runtime_error{msg};
    }
  }

  /// @brief: Throw a std::runtime_error with given msg if condition isn't true.
  /// @param: condition Condition to be checked.
  /// @param: msg Message used to be construct std::runtime_error.
  inline void ThrowUnless(bool condition, const std::string& msg) {
    if (!condition) {
      throw std::runtime_error{msg};
    }
  }


} // namespace linter
