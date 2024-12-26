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
#include <git2/oid.h>
#include <memory>
#include <print>
#include <string>
#include <vector>

#include <boost/program_options/variables_map.hpp>
#include <spdlog/spdlog.h>

#include "configs/version.h"
#include "context.h"
#include "github/common.h"
#include "program_options.h"
#include "tools/base_creator.h"
#include "tools/base_reporter.h"
#include "tools/base_tool.h"
#include "tools/clang_format/clang_format.h"
#include "tools/clang_tidy/clang_tidy.h"
#include "utils/env_manager.h"
#include "utils/git_utils.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {

  // This function must be called before any spdlog operations.
  void set_log_level(std::string_view log_level_str) {
    static constexpr auto valid_log_levels = {"trace", "debug", "error", "info"};
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
    spdlog::info("Got {} changed files. File list:\n{}", files.size(), concat(files));
  }

  void print_version() {
    std::print("{}.{}.{}",
               cpp_linter_VERSION_MAJOR,
               cpp_linter_VERSION_MINOR,
               cpp_linter_VERSION_PATCH);
  }

  auto collect_tool_creators() -> std::vector<tool::creator_base_ptr> {
    auto ret = std::vector<tool::creator_base_ptr>{};
    ret.push_back(std::make_unique<tool::clang_format::creator>());
    ret.push_back(std::make_unique<tool::clang_tidy::creator>());
    return ret;
  }

  void check_repo_is_on_source(const runtime_context& ctx) {
    auto head = git::repo::head_commit(ctx.repo.get());
    throw_unless(head == ctx.source_commit,
                 std::format("Head of repository isn't equal to source commit: {} != {}",
                              git::commit::id_str(head.get()),
                              git::commit::id_str(ctx.source_commit.get())));
  }

} // namespace

auto main(int argc, char **argv) -> int {
  auto tool_creators = collect_tool_creators();

  // Handle user options.
  auto desc = create_program_options_desc();
  tool::register_tool_options(tool_creators, desc);
  auto user_options = parse_program_options(argc, argv, desc);
  if (user_options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (user_options.contains("version")) {
    print_version();
    return 0;
  }

  // Fill runtime context by user options and environment variables.
  auto context = runtime_context{};
  fill_context_by_program_options(user_options, context);
  set_log_level(context.log_level);

  // Fill runtime context by environment variables.
  auto env = github::read_env();
  github::check_env(env);
  github::fill_context_by_env(env, context);

  // Fill runtime context by git repositofy informations.
  git::setup();
  context.repo          = git::repo::open(context.repo_path);
  context.target_commit = git::revparse::commit(*context.repo, context.target);
  context.source_commit = git::revparse::commit(*context.repo, context.source);
  auto diff       = git::diff::get(*context.repo, *context.target_commit, *context.source_commit);
  context.patches = git::patch::create_from_diff(*diff);
  context.deltas = git::diff::deltas(diff.get());
  context.changed_files = git::patch::changed_files(context.patches);
  print_context(context);
  check_repo_is_on_source(context);

  tool::create_tool_options(tool_creators, user_options);
  auto tools     = tool::create_enabled_tools(tool_creators, context);
  auto reporters = tool::check_then_get_reporters(tools, context);

  if (context.enable_action_output) {
    write_to_github_action_output(context, reporters);
  }
  if (context.enable_step_summary) {
    write_to_github_step_summary(context, reporters);
  }
  if (context.enable_comment_on_issue) {
    comment_on_github_issue(context, reporters);
  }
  if (context.enable_pull_request_review) {
    comment_on_github_pull_request_review(context, reporters);
  }

  git::shutdown();
  return all_passed(reporters) ? 0 : 1;
}
