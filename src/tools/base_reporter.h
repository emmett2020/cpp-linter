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

#include <string>

#include "context.h"
#include "github/review_comment.h"

namespace lint::tool {
  using namespace std::string_literals;

  struct reporter_base {
    virtual ~reporter_base() = default;

    virtual auto make_issue_comment(const runtime_context &context) -> std::string = 0;

    virtual auto make_step_summary(const runtime_context &context) -> std::string = 0;

    virtual auto make_review_comment(const runtime_context &context) -> github::review_comments = 0;

    virtual void write_to_action_output(const runtime_context &context) = 0;

    // return sequence: is_pass, passed files number, failed files number, ignored files number.
    virtual auto get_brief_result() -> std::tuple<bool, std::size_t, std::size_t, std::size_t> = 0;

    // Used for show in result
    virtual auto tool_name() -> std::string = 0;
  };

  using reporter_base_ptr = std ::unique_ptr<reporter_base>;

  void write_to_github_action_output(const runtime_context &context,
                                     const std::vector<reporter_base_ptr> &reporters);

  void write_to_github_step_summary(const runtime_context &context,
                                    const std::vector<reporter_base_ptr> &reporters);

  void comment_on_github_issue(const runtime_context &context,
                               const std::vector<reporter_base_ptr> &reporters);

  void comment_on_github_pull_request_review(const runtime_context &context,
                                             const std::vector<reporter_base_ptr> &reporters);

  bool all_passed(const std::vector<reporter_base_ptr> &reporters);
} // namespace lint::tool
