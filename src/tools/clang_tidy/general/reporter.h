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

#include "context.h"

#include <spdlog/spdlog.h>

#include "github/common.h"
#include "tools/base_reporter.h"
#include "tools/clang_tidy/general/option.h"
#include "tools/clang_tidy/general/result.h"
#include "utils/env_manager.h"
#include "utils/error.h"

namespace lint::tool::clang_tidy {

  struct reporter_t : reporter_base {
    ~reporter_t() override = default;

    reporter_t(option_t opt, result_t res)
      : option(std::move(opt))
      , result(std::move(res)) {
    }

    auto make_brief() -> std::string {
      // TODO: replace diagnostic_type with linkable name
      auto ret = ""s;
      for (const auto &[name, failed]: result.fails) {
        for (const auto &diag: failed.diags) {
          // use relative file name rather than diag.header.file_name which is
          // absolute name
          auto one = fmt::format(
            "- **{}:{}:{}:** {}: [{}]\n  > {}\n",
            name,
            diag.header.row_idx,
            diag.header.col_idx,
            diag.header.serverity,
            diag.header.diagnostic_type,
            diag.header.brief);
          ret += one;
        }
      }
      return ret;
    }

    auto make_issue_comment([[maybe_unused]] const runtime_context &context)
      -> std::string override {
      return make_brief();
    }

    auto make_step_summary([[maybe_unused]] const runtime_context &context)
      -> std::string override {
      return make_brief();
    }

    auto make_review_comment(const runtime_context &context) -> github::review_comments override {
      auto comments = github::review_comments{};

      // For each failed file:
      for (const auto &[file, per_file_result]: result.fails) {
        assert(per_file_result.file_path == file);
        assert(context.patches.contains(file));

        const auto &patch   = context.patches.at(file);
        const auto num_hunk = git::patch::num_hunks(*patch);

        // For each clang-tidy diagnostic result in current file:
        for (const auto &diag: per_file_result.diags) {
          auto row = std::stoi(diag.header.row_idx);
          auto col = std::stoi(diag.header.col_idx);

          // Check current diagnostic is in diff hunk.
          auto pos = std::size_t{0};
          for (int hunk_idx = 0; hunk_idx < num_hunk; ++hunk_idx) {
            auto [hunk, num_lines] = git::patch::get_hunk(*patch, hunk_idx);
            if (!git::hunk::is_row_in_hunk(hunk, row)) {
              pos += num_lines;
            } else {
              auto comment     = github::review_comment{};
              comment.path     = file;
              comment.position = pos + row - hunk.new_start + 1;
              comment.body     = diag.header.brief + diag.header.diagnostic_type;
              comments.emplace_back(std::move(comment));
            }
          }
        }
      }
      return comments;
    }

    auto write_to_action_output([[maybe_unused]] const runtime_context &context) -> void override {
      auto output = env::get(github::github_output);
      auto file   = std::fstream{output, std::ios::app};
      throw_unless(file.is_open(), "error to open output file to write");
      file << fmt::format("clang_tidy_failed_number={}\n", result.fails.size());
    }

    auto get_brief_result() -> std::tuple<bool, std::size_t, std::size_t, std::size_t> override {
      return {result.final_passed,
              result.passes.size(),
              result.fails.size(),
              result.ignored.size()};
    }

    auto tool_name() -> std::string override {
      auto parts = ranges::views::split(option.binary, '/')
                 | ranges::to<std::vector<std::string>>();
      return parts.back();
    }

    option_t option;
    result_t result;
  };

} // namespace lint::tool::clang_tidy
