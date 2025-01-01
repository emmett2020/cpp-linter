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
#include <memory>
#include <string>
#include <vector>

#include <git2/oid.h>
#include <boost/algorithm/string/case_conv.hpp>
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
#include "utils/git_utils.h"
#include "utils/util.h"

using namespace lint; // NOLINT
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {
  constexpr auto supported_log_level = {"trace", "debug", "info", "error"};

  // This function must be called before any spdlog operations.
  void set_log_level(const program_options::variables_map &vars) {
    // This name must be samed with the one we registered.
    constexpr auto log_level_opt = "log-level";
    assert(vars.contains(log_level_opt));

    auto level     = vars[log_level_opt].as<std::string>();
    auto log_level = boost::algorithm::to_lower_copy(level);
    throw_unless(ranges::contains(supported_log_level, log_level),
                 fmt::format("unsupported log level: {}", level));

    if (log_level == "trace") {
      spdlog::set_level(spdlog::level::trace);
    } else if (log_level == "debug") {
      spdlog::set_level(spdlog::level::debug);
    } else if (log_level == "info") {
      spdlog::set_level(spdlog::level::info);
    } else {
      spdlog::set_level(spdlog::level::err);
    }
  }

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Got {} changed files. File list:\n{}", files.size(), concat(files));
  }

  void print_version() {
    fmt::print("{}.{}.{}",
               CPP_LINT_ACTION_VERSION_MAJOR,
               CPP_LINT_ACTION_VERSION_MINOR,
               CPP_LINT_ACTION_VERSION_PATCH);
  }

  auto collect_tool_creators() -> std::vector<tool::creator_base_ptr> {
    auto ret = std::vector<tool::creator_base_ptr>{};
    ret.push_back(std::make_unique<tool::clang_format::creator>());
    ret.push_back(std::make_unique<tool::clang_tidy::creator>());
    return ret;
  }

  void check_repo_is_on_source(const runtime_context &ctx) {
    auto head = git::repo::head_commit(*ctx.repo);
    throw_unless(head == ctx.source_commit,
                 fmt::format("Head of repository isn't equal to source commit: {} != {}",
                             git::commit::id_str(*head),
                             git::commit::id_str(*ctx.source_commit)));
  }

} // namespace

auto main(int argc, char **argv) -> int {
  auto tool_creators = collect_tool_creators();

  // Handle program options.
  auto desc = program_options::create_desc();
  tool::register_tool_options(tool_creators, desc);
  auto user_options = program_options::parse(argc, argv, desc);
  if (user_options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (user_options.contains("version")) {
    print_version();
    return 0;
  }
  set_log_level(user_options);
  auto tools = tool::create_enabled_tools(tool_creators, user_options);

  // Create runtime context.
  auto context = runtime_context{};

  // Fill runtime context by program options.
  program_options::fill_context(user_options, context);

  // Fill runtime context by environment variables.
  auto env = github::read_env();
  github::fill_context(env, context);

  // Fill runtime context by git repositofy informations.
  git::setup();
  fill_git_info(context);

  print_context(context);
  check_repo_is_on_source(context);

  // Run tools within the given context and get reporters.
  auto reporters = tool::run_tools(tools, context);
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
