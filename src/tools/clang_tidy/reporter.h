
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

#include "github/common.h"
#include "github/utils.h"
#include "tools/base_reporter.h"
#include "tools/clang_tidy/base_impl.h"
#include "utils/env_manager.h"

namespace linter::tool::clang_tidy {
//

struct reporter : tool_base<user_option, per_file_result> {
  auto make_issue_comment(const user_option &option,
                          const final_result_t &result) -> std::string {
    auto res = std::string{};
    res += std::format(
        "<details>\n<summary>{} reports:<strong>{} fails</strong></summary>\n",
        option.binary, result.fails.size());
    for (const auto &[name, failed] : result.fails) {
      for (const auto &diag : failed.diags) {
        auto one = std::format("- **{}:{}:{}:** {}: [{}]\n  > {}\n",
                               diag.header.file_name, diag.header.row_idx,
                               diag.header.col_idx, diag.header.serverity,
                               diag.header.diagnostic_type, diag.header.brief);
        res += one;
      }
    }
    return res;
  }

  auto make_step_summary(const user_option &option,
                         const final_result_t &result) -> std::string {
    return {};
  }

  auto make_pr_review_comment([[maybe_unused]] const user_option &option,
                              const final_result_t &result)
      -> github::pull_request::review_comments {
    auto comments = github::pull_request::review_comments{};

    for (const auto &[file, per_file_result] : result.fails) {
      // Get the same file's delta and clang-tidy result
      assert(per_file_result.file_path == file);
      assert(result.patches.contains(file));

      const auto &patch = result.patches.at(file);

      for (const auto &diag : per_file_result.diags) {
        const auto &header = diag.header;
        auto row = std::stoi(header.row_idx);
        auto col = std::stoi(header.col_idx);

        // For all clang-tidy result, check is this in hunk.
        auto pos = std::size_t{0};
        auto num_hunk = git::patch::num_hunks(patch.get());
        for (int i = 0; i < num_hunk; ++i) {
          auto [hunk, num_lines] = git::patch::get_hunk(patch.get(), i);
          if (!github::is_row_in_hunk(hunk, row)) {
            pos += num_lines;
            continue;
          }
          auto comment = github::pull_request::review_comment{};
          comment.path = file;
          comment.position = pos + row - hunk.new_start + 1;
          comment.body = diag.header.brief + diag.header.diagnostic_type;
          comments.emplace_back(std::move(comment));
        }
      }
    }
    return comments;
  }

  auto write_to_action_output() -> void {
    auto output = env::get(github_output);
    auto file = std::fstream{output, std::ios::app};
    throw_unless(file.is_open(), "error to open output file to write");

    const auto clang_tidy_failed = result.clang_tidy_failed.size();
    const auto clang_format_failed = result.clang_format_failed.size();
    const auto total_failed = clang_tidy_failed + clang_format_failed;

    file << std::format("total_failed={}\n", total_failed);
    file << std::format("clang_tidy_failed_number={}\n", clang_tidy_failed);
    file << std::format("clang_format_failed_number={}\n", clang_format_failed);
  }
};

} // namespace linter::tool::clang_tidy
