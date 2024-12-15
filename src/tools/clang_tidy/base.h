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
#include <vector>

#include <spdlog/spdlog.h>

#include "tools/base_option.h"
#include "tools/base_result.h"
#include "tools/base_tool.h"

namespace linter::tool::clang_tidy {
  struct user_option : user_option_base {
    bool allow_no_checks      = false;
    bool enable_check_profile = false;
    std::string checks;
    std::string config;
    std::string config_file;
    std::string database;
    std::string header_filter;
    std::string line_filter;
  };

  /// Represents statistics outputed by clang-tidy. It's usually the stderr
  /// messages of clang-tidy.
  struct statistic {
    std::uint32_t warnings                   = 0;
    std::uint32_t errors                     = 0;
    std::uint32_t warnings_treated_as_errors = 0;
    std::uint32_t total_suppressed_warnings  = 0;
    std::uint32_t non_user_code_warnings     = 0;
    std::uint32_t no_lint_warnings           = 0;
  };

  /// Each diagnostic hase a header line.
  struct diagnostic_header {
    std::string file_name;
    std::string row_idx;
    std::string col_idx;
    std::string serverity;
    std::string brief;
    std::string diagnostic_type;
  };

  /// Represents one diagnostic which outputed by clang-tidy.
  /// Generally, each diagnostic has a header line and several details line
  /// which give a further detailed explanation.
  struct diagnostic {
    diagnostic_header header;
    std::string details;
  };

  /// Represents all diagnostics which outputed by clang-tidy.
  using diagnostics = std::vector<diagnostic>;

  struct per_file_result : per_file_result_base {
    statistic stat;
    diagnostics diags;
  };

  struct base_clang_tidy : tool_base<user_option, per_file_result> {
    bool is_supported(operating_system_t system, arch_t arch) override {
      return system == operating_system_t::linux && arch == arch_t::x86_64;
    }

    constexpr auto name() -> std::string_view override {
      return "clang_tidy";
    }

    auto apply_to_single_file(
      const user_option &user_opt,
      const std::string &repo,
      const std::string &file) -> per_file_result override;

    auto make_issue_comment(const user_option &option, const final_result_t &result)
      -> std::string override;

    auto make_step_summary(const user_option &option, const final_result_t &result)
      -> std::string override;

    auto make_pr_review_comment(const user_option &option, const final_result_t &result)
      -> github::pull_request::review_comments override;

    auto write_to_action_output() -> void override;
  };

  using clang_tidy_ptr = base_tool_ptr<user_option, per_file_result>;
} // namespace linter::tool::clang_tidy
