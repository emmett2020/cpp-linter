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

#include <spdlog/spdlog.h>
#include <string>

#include "base_result.h"
#include "context.h"
#include "github/review_comment.h"

namespace linter::tool {
/// The operating system type.
enum class operating_system_t : std::uint8_t {
  windows,
  macos,
  linux,
};

/// The archecture type.
enum class arch_t : std::uint8_t {
  x86_64,
  arm64,
};

template <class UserOption, class PerFileResult> struct interface {
  using user_option_t = UserOption;
  using per_file_result_t = PerFileResult;
  using final_result_t = final_result<PerFileResult>;

  virtual ~interface() = default;

  /// Check whether this tool is supported on the given platform.
  virtual bool is_supported(operating_system_t system, arch_t arch) = 0;

  /// Return unique name of this tool.
  virtual constexpr auto name() -> std::string_view = 0;

  /// Return version of this tool.
  virtual constexpr auto version() -> std::string_view = 0;

  /// Apply this tool to a single file and return the execution result.
  virtual auto
  apply_to_single_file(const user_option_t &option, const std::string &repo,
                       const std::string &file) -> per_file_result_t = 0;
};

/// This is a base class represents linter tools. All specified tools should be
/// derived from this.
template <class UserOption, class PerFileResult>
struct tool_base : public interface<UserOption, PerFileResult> {
  using user_option_t = UserOption;
  using per_file_result_t = PerFileResult;
  using final_result_t = final_result<PerFileResult>;

  ~tool_base() override = default;

  auto run([[maybe_unused]] const context_t &ctx, const user_option_t &opt,
           const std::string &repo,
           const std::vector<std::string> &files) -> final_result_t {
    auto result = final_result_t{};

    for (const auto &file : files) {
      if (filter_file(opt.source_iregex, file)) {
        result.ignored_files.push_back(file);
        spdlog::trace("file is ignored {} by {}", file, opt.binary);
        continue;
      }

      auto per_file_result = apply_to_single_file(opt, repo, file);
      if (per_file_result.passed) {
        spdlog::info("file: {} passes {} check.", file, opt.binary);
        result.passes[file] = std::move(per_file_result);
        continue;
      }

      spdlog::error("file: {} doesn't pass {} check.", file, opt.binary);
      result.fails[file] = std::move(per_file_result);

      if (opt.enabled_fastly_exit) {
        spdlog::info("{} fastly exit since check failed", opt.binary);
        result.final_passed = false;
        result.fastly_exited = true;
        return per_file_result;
      }
    }

    result.final_passed = true;
    return result;
  }
};

/// An unique pointer for base tool.
template <class Option, class PerFileResult>
using tool_base_ptr = std::unique_ptr<tool_base<Option, PerFileResult>>;

// Return a string contains all tools brief result.
template <class UserOption, class PerFileResult>
auto make_step_summary(const std::vector<tool_base<UserOption, PerFileResult>>
                           &tools) -> std::string {
  //
}

} // namespace linter::tool
