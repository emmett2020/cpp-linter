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
#include <string>
#include <unordered_map>

#include "utils/git_utils.h"

namespace linter {

/// The runtime context for all tools.
struct context_t {
  bool use_on_local = false;
  std::string log_level;
  bool enable_step_summary = false;
  bool enable_comment_on_issue = false;
  bool enable_pull_request_review = false;
  bool enable_action_output = false;

  std::string repo_path;
  std::string repo;
  std::string token;
  std::string event_name;
  std::string target;
  std::string source;
  std::int32_t pr_number = -1;

  std::unordered_map<std::string, git::patch_ptr> patches;
};

void print_context(const context_t &ctx);
} // namespace linter
