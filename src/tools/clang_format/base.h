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

namespace linter::tool::clang_format {
  struct user_option : user_option_base {
    bool enable_warning_as_error     = false;
    bool needs_formatted_source_code = false;
  };

  struct replacement_t {
    int offset;
    int length;
    std::string data;
  };

  using replacements_t = std::vector<replacement_t>;

  struct per_file_result : per_file_result_base {
    replacements_t replacements;
    std::string formatted_source_code;
  };

  struct base_clang_format : tool_base<user_option, per_file_result> {
    bool is_supported(operating_system_t system, arch_t arch) override {
      return system == operating_system_t::linux && arch == arch_t::x86_64;
    }

    constexpr auto name() -> std::string_view override {
      return "clang_format";
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
      -> std::string override;
  };

  using clang_format_ptr = base_tool_ptr<user_option, per_file_result>;
} // namespace linter::tool::clang_format
