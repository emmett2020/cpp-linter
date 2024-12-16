
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

#include "tools/base_reporter.h"
#include "tools/clang_format/base_impl.h"

namespace linter::tool::clang_format {
//

struct reporter : tool_base<user_option, per_file_result> {
  auto make_issue_comment(const user_option &option,
                          const final_result_t &result) -> std::string {
    return "";
  }

  auto make_step_summary(const user_option &option,
                         const final_result_t &result) -> std::string {
    return {};
  }

  auto make_pr_review_comment([[maybe_unused]] const user_option &option,
                              const final_result_t &result)
      -> github::review_comments {
    auto comments = github::review_comments{};

    for (const auto &[file, per_file_result] : result.fails) {
      // auto old_buffer = git::blob::get_raw_content(ctx_.repo, commit, file);
      auto old_buffer = ""s;
      auto opts = git::diff_options{};
      git::diff::init_option(&opts);
      auto format_source_patch = git::patch::create_from_buffers(
          old_buffer, file, per_file_result.formatted_source_code, file, opts);
      spdlog::error(git::patch::to_str(format_source_patch.get()));

      const auto &patch = result.patches.at(file);

      auto format_num_hunk = git::patch::num_hunks(format_source_patch.get());
      for (int i = 0; i < format_num_hunk; ++i) {
        auto [format_hunk, format_num_lines] =
            git::patch::get_hunk(format_source_patch.get(), i);
        auto row = format_hunk.old_start;

        auto pos = std::size_t{0};
        auto num_hunk = git::patch::num_hunks(patch.get());
        for (int j = 0; j < num_hunk; ++j) {
          auto [hunk, num_lines] = git::patch::get_hunk(patch.get(), j);
          if (!github::is_row_in_hunk(hunk, row)) {
            pos += num_lines;
            continue;
          }
          auto comment = github::review_comment{};
          comment.path = file;
          comment.position = pos + row - hunk.new_start + 1;

          comment.body = git::patch::get_lines_in_hunk(patch.get(), i) |
                         std::views::join_with(' ') |
                         std::ranges::to<std::string>();
          comments.emplace_back(std::move(comment));
        }
      }
    }

    spdlog::error("Comments XXX:");
    for (const auto &comment : comments) {
    }

    return comments;
  }
};

} // namespace linter::tool::clang_format
