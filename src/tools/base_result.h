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

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/git_utils.h"

namespace linter {

  struct per_file_base_result {
    virtual ~per_file_base_result()         = default;
    virtual explicit operator std::string() = 0;

    bool passed = false;
    std::string file_path;
    std::string tool_stdout;
    std::string tool_stderr;
  };

  using per_file_base_result_ptr = std::unique_ptr<per_file_base_result>;

  struct base_result {
    virtual ~base_result() = default;

    virtual explicit operator std::string()                             = 0;
    virtual auto make_issue_comment() -> std::optional<std::string>     = 0;
    virtual auto make_step_summary() -> std::optional<std::string>      = 0;
    virtual auto make_pr_review_comment() -> std::optional<std::string> = 0;

    bool final_passed  = false;
    bool fastly_exited = false;
    std::unordered_map<std::string, git::patch_ptr> patches;
    std::vector<std::string> ignored_files;

    std::unordered_map<std::string, per_file_base_result_ptr> passes;
    std::unordered_map<std::string, per_file_base_result_ptr> fails;
  };

} // namespace linter
