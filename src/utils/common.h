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

#include <string_view>
#include <boost/regex.hpp>

namespace lint {
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

  /// Trim both left and right.
  constexpr auto trim(std::string_view str) -> std::string_view {
    return trim_right(trim_left(str));
  }

  inline bool filter_file(const std::string &iregex, const std::string &file) {
    auto regex = boost::regex{iregex, boost::regex::icase};
    return !boost::regex_match(file, regex);
  }

} // namespace lint
