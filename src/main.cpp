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

auto collect_tool_creators() -> std::vector<tool::creator_base_ptr> {
  auto ret = std::vector<tool::creator_base_ptr>{};
  ret.push_back(std::make_unique<tool::clang_format::creator>());
  ret.push_back(std::make_unique<tool::clang_tidy::creator>());
  return ret;
}

} // namespace

auto main(int argc, char **argv) -> int {
  auto tool_creators = collect_tool_creators();

  // Handle user options.
  auto desc = create_program_options_desc();
  tool::register_options(tool_creators, desc);
  auto options = parse_program_options(argc, argv, desc);
  if (options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (options.contains("version")) {
    print_version();
    return 0;
  }
  tool::create_options(tool_creators, options);

  // Fill runtime context by user options and environment variables.
  auto context = runtime_context{};
  context.use_on_local = env::get(github_actions) != "true";
  check_and_fill_context_by_program_options(options, context);
  set_log_level(context.log_level);

  if (!context.use_on_local) {
    auto env = read_github_env();
    print_github_env(env);
    check_github_env(env);
    fill_context_by_env(env, context);
  }
  print_context(context);

  // Fill runtime context by git repositofy informations.
  git::setup();
  auto repo = git::repo::open(context.repo_path);
  auto target_commit = git::revparse::commit(*repo, context.target);
  auto source_commit = git::revparse::commit(*repo, context.source);
  auto diff = git::diff::get(*repo, *target_commit, *source_commit);
  context.patches = git::patch::create_from_diff(*diff);
  context.changed_files = git::patch::changed_files(context.patches);

  auto tools = tool::create_tools(tool_creators, context);
  tool::do_check(tools, context);
  auto reporters = tool::get_reporters(tools);

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
