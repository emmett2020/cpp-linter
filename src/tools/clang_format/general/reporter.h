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

#include <utility>

#include <spdlog/spdlog.h>

#include "context.h"
#include "github/common.h"
#include "github/utils.h"
#include "tools/base_reporter.h"
#include "tools/clang_format/general/option.h"
#include "tools/clang_format/general/result.h"
#include "utils/env_manager.h"
#include "utils/git_utils.h"
#include "utils/util.h"

namespace linter::tool::clang_format {
  using namespace std::string_view_literals;

  struct reporter_t : reporter_base {
    reporter_t(option_t opt, result_t res)
      : option(std::move(opt))
      , result(std::move(res)) {
    }

    auto make_brief_result() -> std::string {
      auto content = ""s;
      for (const auto &[name, failed]: result.fails) {
        auto one  = std::format("- {}\n", name);
        content  += one;
      }
      return content;
    }

    auto make_issue_comment([[maybe_unused]] const runtime_context &context)
      -> std::string override {
      return make_brief_result();
    }

    auto make_step_summary([[maybe_unused]] const runtime_context &context)
      -> std::string override {
      return make_brief_result();
    }

    static auto convert_row_number_into_patch_position(int row_number, git_patch &patch)
      -> std::optional<std::size_t> {
      const auto num_hunk = git::patch::num_hunks(patch);
      auto pos            = 0U;
      for (auto hunk_idx = 0; hunk_idx < num_hunk; ++hunk_idx) {
        auto [hunk, num_lines] = git::patch::get_hunk(patch, hunk_idx);
        if (!github::is_row_in_hunk(hunk, row_number)) {
          pos += num_lines;
        }
        return pos + row_number - hunk.new_start + 1;
      }
      return std::nullopt;
    }

    // The hunk splited rule must be same as github.
    static void make_per_hunk_review_comment(
      const runtime_context &context,
      const std::string &file,
      git::patch_raw_ptr patch,
      std::size_t patch_idx,
      github::review_comments &comments) {
      // The diff patch of source revision to target revision of checking file.
      const auto &patch_user = context.patches.at(file);

      const auto [hunk, hunk_line_num] = git::patch::get_hunk(patch, patch_idx);
      const auto row                   = hunk.old_start;
      auto pos                         = convert_row_number_into_patch_position(row, *patch_user);
      if (pos) {
        auto comment     = github::review_comment{};
        comment.path     = file;
        comment.position = *pos;

        auto temp    = git::patch::get_lines_in_hunk(patch, patch_idx);
        comment.body = git::patch::get_source_lines_in_hunk(*patch, patch_idx)
                     | std::views::join_with(' ')
                     | std::ranges::to<std::string>();
        comments.emplace_back(std::move(comment));
      }
    }

    static auto get_suggestion_patch(
      const runtime_context &context,
      const std::string &file,
      const per_file_result &format_result) {
      // Compare original content with formatted result of a file.
      const auto before_format =
        git::blob::get_raw_content(context.repo.get(), context.source_commit.get(), file);
      auto opts          = git::diff_options{};
      opts.context_lines = 0;
      git::diff::init_option(&opts);
      return git::patch::create_from_buffers(
        before_format,
        file,
        format_result.formatted_source_code,
        file,
        opts);
    }

    static void make_per_file_review_comment(
      const runtime_context &context,
      const std::string &file,
      const per_file_result &format_result,
      github::review_comments &comments) {
      auto patch_suggestion = get_suggestion_patch(context, file, format_result);
      auto num_hunks        = git::patch::num_hunks(patch_suggestion.get());
      for (int hunk_idx = 0; hunk_idx < num_hunks; ++hunk_idx) {
        make_per_hunk_review_comment(context, file, patch_suggestion.get(), hunk_idx, comments);
      }
    }

    auto make_review_comment(const runtime_context &context) -> github::review_comments override {
      auto comments = github::review_comments{};
      for (const auto &[file, format_result]: result.fails) {
        make_per_file_review_comment(context, file, format_result, comments);
      }
      return comments;
    }

    void write_to_action_output([[maybe_unused]] const runtime_context &ctx) override {
      auto output = env::get(github::github_output);
      auto file   = std::fstream{output, std::ios::app};
      throw_unless(file.is_open(), "error to open output file to write");
      file << std::format("clang_format_failed_number={}\n", result.fails.size());
    }

    auto get_brief_result() -> std::tuple<bool, std::size_t, std::size_t, std::size_t> override {
      return {result.final_passed,
              result.passes.size(),
              result.fails.size(),
              result.ignored.size()};
    }

    auto tool_name() -> std::string override {
      auto parts = std::views::split(option.binary, '/')
                 | std::ranges::to<std::vector<std::string>>();
      return parts.back();
    }

    option_t option;
    result_t result;
  };

} // namespace linter::tool::clang_format
