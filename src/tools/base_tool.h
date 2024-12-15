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

#include <string>

#include "base_option.h"
#include "base_result.h"
#include "github/review_comment.h"

namespace linter {
  /// The operating system type.
  enum class operating_system_t : std::uint8_t {
    windows,
    macos,
    linux,
  };

  /// The archecture type.
  enum class arch_t : std::uint8_t {
    x86_64,
    arm64,
  };

  /// This is a base class represents linter tools. All specified tools should be
  /// derived from this.
  template <class UserOption, class PerFileResult>
  class tool_base {
  public:
    using per_file_result_t = PerFileResult;
    using user_option_t     = UserOption;
    using final_result_t    = final_result<PerFileResult>;

    virtual ~tool_base()                                              = default;
    virtual bool is_supported(operating_system_t system, arch_t arch) = 0;
    virtual constexpr auto name() -> std::string_view                 = 0;
    virtual constexpr auto version() -> std::string_view              = 0;

    virtual auto apply_to_single_file(
      const user_option_t &option,
      const std::string &repo,
      const std::string &file) -> per_file_result_t = 0;

    virtual auto make_issue_comment(const user_option_t &user_opt, const final_result_t &result)
      -> std::string = 0;

    virtual auto
    make_step_summary(const user_option_t &option, const final_result_t &result) -> std::string = 0;

    virtual auto make_pr_review_comment(const user_option_t &option, const final_result_t &result)
      -> github::pull_request::review_comments = 0;

    auto run(const user_option_base &option,
             const std::string &repo,
             const std::vector<std::string> &files) -> final_result_t {
    }
  };

  /// An unique pointer for base tool.
  template <class Option, class PerFileResult>
  using base_tool_ptr = std::unique_ptr<tool_base<Option, PerFileResult>>;

} // namespace linter
