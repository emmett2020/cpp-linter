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

#include <spdlog/spdlog.h>
#include <string>

#include "base_option.h"
#include "base_result.h"
#include "github/review_comment.h"

namespace linter::tool {
using namespace std::string_literals;

template <class UserOption, class PerFileResult> struct reporter_interface {
  using user_option_t = UserOption;
  using per_file_result_t = PerFileResult;
  using final_result_t = final_result<PerFileResult>;

  virtual auto make_issue_comment(const user_option_t &user_opt,
                                  const final_result_t &result) -> std::string {
  }

  virtual auto make_step_summary(const user_option_t &option,
                                 const final_result_t &result) -> std::string {}

  virtual auto
  make_review_comment(const user_option_t &option,
                      const final_result_t &result) -> github::review_comments {
  }

  virtual auto write_to_action_output(const user_option_t &option,
                                      const final_result_t &result) -> void {}
};

template <class UserOption, class Result>
struct reporter_base : reporter_interface<UserOption, Result> {
  UserOption opt;
  Result result;
};

template <class... Reporter>
auto make_step_summary(Reporter &&...reporters) -> std::string {
  static const auto title = "# The cpp-linter Result"s;
  static const auto hint_pass = ":rocket: All checks on all file passed."s;
  static const auto hint_fail =
      ":warning: Some files didn't pass the cpp-linter checks\n"s;

  constexpr auto reporter_total_size = sizeof...(reporters);
  auto all_passed = ((reporters.final_passed & ...));
  if (all_passed) {
    return title + hint_pass;
  }

  auto summary = std::string{};
  summary += ((reporters.make_step_summary() + "\n"), ...);
  return title + hint_fail + summary;
}

} // namespace linter::tool
