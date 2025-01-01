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

#include <algorithm>
#include <git2/diff.h>
#include <ranges>
#include <utility>

#include <spdlog/spdlog.h>

#include "context.h"
#include "github/common.h"
#include "github/review_comment.h"
#include "tools/base_reporter.h"
#include "tools/clang_format/general/option.h"
#include "tools/clang_format/general/result.h"
#include "utils/env_manager.h"
#include "utils/util.h"

namespace lint::tool::clang_format {
  using namespace std::string_view_literals;

  struct reporter_t : reporter_base {
    reporter_t(option_t opt, result_t res)
      : option(std::move(opt))
      , result(std::move(res)) {
    }

    auto make_brief_result() -> std::string {
      auto content = ""s;
      for (const auto &[name, failed]: result.fails) {
        auto one  = fmt::format("- {}\n", name);
        content  += one;
      }
      return content;
    }

    auto make_issue_comment([[maybe_unused]] const runtime_context &context)
      -> std::string override {
      return make_brief_result();
    }

    auto make_step_summary([[maybe_unused]] const runtime_context &context)
      -> std::string override {
      return make_brief_result();
    }

    auto make_review_comment([[maybe_unused]] const runtime_context &context)
      -> github::review_comments override {
      return {};
    }

    void write_to_action_output([[maybe_unused]] const runtime_context &ctx) override {
      auto output = env::get(github::github_output);
      auto file   = std::fstream{output, std::ios::app};
      throw_unless(file.is_open(), "error to open output file to write");
      file << fmt::format("clang_format_failed_number={}\n", result.fails.size());
    }

    auto get_brief_result() -> std::tuple<bool, std::size_t, std::size_t, std::size_t> override {
      return {result.final_passed,
              result.passes.size(),
              result.fails.size(),
              result.ignored.size()};
    }

    auto tool_name() -> std::string override {
      auto parts = ranges::views::split(option.binary, '/')
                 | ranges::to<std::vector<std::string>>();
      return parts.back();
    }

    option_t option;
    result_t result;
  };

} // namespace lint::tool::clang_format
