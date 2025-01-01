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

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

namespace linter {

  void fill_git_info(runtime_context &context) {
    spdlog::trace("Enter fill_git_info");
    assert(context.repo == nullptr && "given context already has a repository");
    assert(context.target_commit == nullptr && "given context already has a target commit");
    assert(context.source_commit == nullptr && "given context already has a source commit");
    assert(context.patches.empty() && "given context already has patches");
    assert(context.deltas.empty() && "given context already has deltas");
    assert(context.changed_files.empty() && "given context already has changed files");

    assert(!context.repo_path.empty() && "repo_path of context is empty()");
    assert(!context.target.empty() && "target of context is empty()");
    assert(!context.source.empty() && "source of context is empty()");

    context.repo          = git::repo::open(context.repo_path);
    context.target_commit = git::revparse::commit(*context.repo, context.target);
    context.source_commit = git::revparse::commit(*context.repo, context.source);
    auto diff       = git::diff::get(*context.repo, *context.target_commit, *context.source_commit);
    context.patches = git::patch::create_from_diff(*diff);
    context.deltas  = git::diff::deltas(diff.get());
    context.changed_files = git::patch::changed_files(context.patches);
  }

  void print_context(const runtime_context &ctx) {
    spdlog::info("Runtime Context:");
    spdlog::info("--------------------------------------------------");
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
    spdlog::info("\tchanged files:");
    for (const auto &file: ctx.changed_files) {
      spdlog::info("\t\t{}", file);
    }
    spdlog::info("");
  }
} // namespace linter
