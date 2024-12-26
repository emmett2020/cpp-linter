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
#include "context.h"

#include <spdlog/spdlog.h>
#include <magic_enum/magic_enum.hpp>

namespace linter {
  void print_context(const runtime_context& ctx) {
    spdlog::info("Runtime Context:");
    spdlog::info("--------------------------------------------------");
    spdlog::info("\tlog level: {}", ctx.log_level);
    spdlog::info("\tenable step summary: {}", ctx.enable_step_summary);
    spdlog::info("\tenable update issue comment: {}", ctx.enable_comment_on_issue);
    spdlog::info("\tenable pull request review: {}", ctx.enable_pull_request_review);
    spdlog::info("\tenable action outptu: {}", ctx.enable_action_output);
    spdlog::info("\trepository path: {}", ctx.repo_path);
    spdlog::info("\trepository: {}", ctx.repo_pair);
    spdlog::info("\trepository token: {}", ctx.token.empty() ? "" : "***");
    spdlog::info("\trepository event_name: {}", ctx.event_name);
    spdlog::info("\trepository target: {}", ctx.target);
    spdlog::info("\trepository source: {}", ctx.source);
    spdlog::info("\trepository pull-request number: {}", ctx.pr_number);
    spdlog::info("\tcurrent operating system: {}", magic_enum::enum_name(ctx.os));
    spdlog::info("\tcurrent archecture: {}", magic_enum::enum_name(ctx.arch));
    spdlog::info("\tchanged files:");
    for (const auto& file: ctx.changed_files) {
      spdlog::info("\t\t{}", file);
    }
    spdlog::info("");
  }
} // namespace linter
