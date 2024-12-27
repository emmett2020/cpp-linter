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
#include "program_options.h"

#include <algorithm>
#include <initializer_list>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/program_options/options_description.hpp>
#include <spdlog/spdlog.h>

#include "context.h"
#include "github/github.h"
#include "utils/util.h"

namespace linter::program_options {
namespace {
constexpr auto supported_log_level = {"trace", "debug", "info", "error"};

constexpr auto help = "help";
constexpr auto version = "version";
constexpr auto log_level = "log-level";
constexpr auto target = "target-revision";
constexpr auto enable_step_summary = "enable-step-summary";
constexpr auto enable_comment_on_issue = "enable-comment-on-issue";
constexpr auto enable_pull_request_review = "enable-pull-request-review";
constexpr auto enable_action_output = "enable-action-output";

// constexpr auto repo_path                  = "repo-path";
// constexpr auto repo                       = "repo";
// constexpr auto token                      = "token";
// constexpr auto source                     = "source";
// constexpr auto event_name                 = "event-name";
// constexpr auto pr_number                  = "pr-number";

} // namespace

using std::string;

auto create_desc() -> options_description {
  auto desc = options_description{"cpp-linter options"};

  const auto *level =
      value<string>()->value_name("level")->default_value("info");
  const auto *revision = value<string>()->value_name("revision");

  auto boolean = [](bool def) {
    return value<bool>()->value_name("boolean")->default_value(def);
  };

  // clang-format off
    desc.add_options()
      (help,                                         "Display help message")
      (version,                                      "Display current cpp-linter version")
      (log_level,                   level,           "Set the log verbose level of cpp-linter. "
                                                     "Supports: [trace, debug, info, error]")
      (target,                      revision,        "Set the target revision of git repository "
                                                     "which is usually the branch name you want to merged into")
      (enable_comment_on_issue,     boolean(true),   "Whether enable comment on Github issues")
      (enable_pull_request_review,  boolean(false),  "Whether enable Github pull-request reivew comment")
      (enable_step_summary,         boolean(true),   "Whether enable write step summary to Github action")
      (enable_action_output,        boolean(true),   "Whether enable write output to Github action")
    ;
  // clang-format on

  return desc;
}

auto parse(int argc, char **argv,
           const options_description &desc) -> variables_map {
  auto variables = variables_map{};
  store(parse_command_line(argc, argv, desc), variables);
  notify(variables);
  return variables;
}

void must_specify(const std::string &condition, const variables_map &variables,
                  const std::initializer_list<const char *> &options) {
  auto lacks = std::vector<std::string>();
  std::ranges::for_each(options, [&](const auto *option) {
    if (!variables.contains(option)) {
      lacks.push_back(option);
    }
  });
  throw_unless(lacks.empty(), [&]() noexcept {
    auto lacks_str = concat(lacks, ',');
    return fmt::format("must specify {} when {}", std::move(lacks_str),
                       condition);
  });
}

void must_not_specify(const std::string &condition,
                      const variables_map &variables,
                      const std::initializer_list<const char *> &options) {
  auto forbidden = std::vector<std::string>();
  std::ranges::for_each(options, [&](const auto *option) {
    if (variables.contains(option)) {
      forbidden.push_back(option);
    }
  });

  throw_unless(forbidden.empty(), [&]() noexcept {
    auto forbidden_str = concat(forbidden, ',');
    return fmt::format("must not specify {} when {}", forbidden_str, condition);
  });
}

// This function will be called after check context. So there's no need to do
// same check.
auto create_context(const variables_map &variables) -> runtime_context {
  spdlog::debug("Start to check program options and fill context by it");
  auto ctx = runtime_context{};

  auto must_specify_option = {target};
  must_specify("using CppLintAction", variables, must_specify_option);
  ctx.target = variables[target].as<std::string>();

  if (variables.contains(log_level)) {
    auto level = variables[log_level].as<std::string>();
    ctx.log_level = boost::algorithm::to_lower_copy(level);
    throw_unless(ranges::contains(supported_log_level, ctx.log_level),
                 fmt::format("unsupported log level: {}", level));
  }

  if (variables.contains(enable_step_summary)) {
    ctx.enable_step_summary = variables[enable_step_summary].as<bool>();
  }
  if (variables.contains(enable_comment_on_issue)) {
    ctx.enable_comment_on_issue = variables[enable_comment_on_issue].as<bool>();
  }
  if (variables.contains(enable_pull_request_review)) {
    ctx.enable_pull_request_review =
        variables[enable_pull_request_review].as<bool>();
  }
  if (variables.contains(enable_action_output)) {
    ctx.enable_action_output = variables[enable_action_output].as<bool>();
  }
  return ctx;
}

} // namespace linter::program_options
