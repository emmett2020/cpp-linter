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

#include <cstdint>
#include <git2/repository.h>
#include <string>
#include <unordered_map>

#include "utils/git_utils.h"
#include "utils/platform.h"

namespace linter {
  /// The runtime context for all tools.
  struct runtime_context {
    bool use_on_local               = false;
    std::string log_level           = "debug";
    bool enable_step_summary        = false;
    bool enable_comment_on_issue    = false;
    bool enable_pull_request_review = false;
    bool enable_action_output       = false;

    std::string repo_path;
    std::string repo_pair;
    std::string token;
    std::string event_name;
    std::string target;
    std::string source;
    std::int32_t pr_number = -1;

    operating_system_t os = operating_system_t::ubuntu;
    arch_t arch           = arch_t::x86_64;

    std::vector<std::string> changed_files;
    git::repo_ptr repo{nullptr, ::git_repository_free};
    git::commit_ptr target_commit{nullptr, ::git_commit_free};
    git::commit_ptr source_commit{nullptr, ::git_commit_free};
    std::unordered_map<std::string, git::patch_ptr> patches;
  };

  void print_context(const runtime_context &ctx);
} // namespace linter
