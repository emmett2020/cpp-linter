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
#include <cctype>
#include <fstream>
#include <git2/oid.h>
#include <print>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "configs/version.h"
#include "context.h"
#include "github/api.h"
#include "github/common.h"
#include "program_options.h"
#include "tools/clang_format/clang_format.h"
#include "tools/clang_tidy/base_impl.h"
#include "tools/clang_tidy/clang_tidy.h"
#include "tools/clang_tidy/creator.h"
#include "utils/env_manager.h"
#include "utils/git_utils.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {

// This function must be called before any spdlog operations.
void set_log_level(std::string_view log_level_str) {
  static constexpr auto valid_log_levels = {"trace", "debug", "error"};
  assert(std::ranges::contains(valid_log_levels, log_level_str));

  auto log_level = spdlog::level::info;
  if (log_level_str == "trace") {
    log_level = spdlog::level::trace;
  } else if (log_level_str == "debug") {
    log_level = spdlog::level::debug;
  } else {
    log_level = spdlog::level::err;
  }
  spdlog::set_level(log_level);
}

auto print_changed_files(const std::vector<std::string> &files) {
  spdlog::info("Got {} changed files. File list:\n{}", files.size(),
               concat(files));
}

void print_version() {
  std::print("{}.{}.{}", cpp_linter_VERSION_MAJOR, cpp_linter_VERSION_MINOR,
             cpp_linter_VERSION_PATCH);
}

} // namespace

auto main(int argc, char **argv) -> int {
  auto clang_tidy_creator = tool::clang_tidy::creator{};
  auto clang_format_creator = tool::clang_format::creator{};

  auto desc = create_program_options_desc();
  clang_tidy_creator.register_option_desc(desc);
  clang_format_creator.register_option_desc(desc);
  auto options = parse_program_options(argc, argv, desc);
  if (options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (options.contains("version")) {
    print_version();
    return 0;
  }

  auto ctx = context_t{};
  ctx.use_on_local = env::get(github_actions) != "true";
  check_and_fill_context_by_program_options(options, ctx);
  set_log_level(ctx.log_level);

  // Get some additional informations when on Github environment.
  if (!ctx.use_on_local) {
    auto env = read_github_env();
    print_github_env(env);
    check_github_env(env);
    fill_context_by_env(env, ctx);
  }
  print_context(ctx);

  // Open user's git repository.
  git::setup();
  auto repo = git::repo::open(ctx.repo_path);
  auto target_commit = git::convert<git::commit_ptr>(
      git::revparse::single(repo.get(), ctx.target));
  auto source_commit = git::convert<git::commit_ptr>(
      git::revparse::single(repo.get(), ctx.source));
  auto diff = git::diff::commit_to_commit(repo.get(), target_commit.get(),
                                          source_commit.get());

  auto linter_result = total_result{};
  linter_result.patches = git::patch::create_from_diff(diff.get());
  auto changed_files = git::patch::changed_files(linter_result.patches);
  print_changed_files(changed_files);

  if (ctx.clang_format_option.enable_clang_format) {
    apply_clang_format_on_files(ctx, changed_files, linter_result);
  }
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    apply_clang_tidy_on_files(ctx, changed_files, linter_result);
  }

  if (ctx.enable_step_summary) {
    auto summary_file = env::get(github_step_summary);
    auto file = std::fstream{summary_file, std::ios::app};
    throw_unless(file.is_open(), "failed to open step summary file to write");
    file << make_brief_result(ctx, linter_result);
  }

  if (ctx.enable_comment_on_issue) {
    auto github_client = github_api_client{ctx};
    github_client.get_issue_comment_id();
    github_client.add_or_update_issue_comment(
        make_brief_result(ctx, linter_result));
  }

  if (ctx.enable_pull_request_review) {
    // TODO: merge
    auto comments = make_clang_tidy_pr_review_comment(ctx, linter_result);
    auto clang_format_comments = make_clang_format_pr_review_comment(
        ctx, linter_result, repo.get(), source_commit.get());
    comments.insert(comments.end(), clang_format_comments.begin(),
                    clang_format_comments.end());
    auto body = make_pr_review_comment_str(comments);
    auto github_client = github_api_client{ctx};
    github_client.post_pull_request_review(body);
  }

  if (!ctx.use_on_local) {
    write_to_github_output(ctx, linter_result);
  }

  git::shutdown();
  auto all_passes = true;
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    all_passes &= (linter_result.clang_tidy_failed.empty());
  }
  return all_passes ? 0 : 1;
}
