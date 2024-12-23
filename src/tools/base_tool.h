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
#include <spdlog/spdlog.h>
#include <string>

#include "context.h"
#include "tools/base_reporter.h"

namespace linter::tool {
  /// This is a base class represents linter tools. All specified tools should be
  /// derived from this.
  struct tool_base {
    virtual ~tool_base() = default;

    /// Check whether this tool is supported on the given platform.
    virtual bool is_supported(operating_system_t system, arch_t arch) = 0;

    /// Return unique name of this tool.
    virtual auto name() -> std::string_view = 0;

    /// Return version of this tool.
    virtual auto version() -> std::string_view = 0;

    /// Apply this tool to a single file.
    virtual void check(const runtime_context &context) = 0;

    virtual auto get_reporter() -> reporter_base_ptr = 0;
  };

  /// An unique pointer for base tool.
  using tool_base_ptr = std::unique_ptr<tool_base>;

  inline void do_check(const std::vector<tool_base_ptr> &tools, const runtime_context &context) {
    for (const auto &tool: tools) {
      tool->check(context);
    }
  }

  inline auto
  get_reporters(const std::vector<tool_base_ptr> &tools) -> std::vector<reporter_base_ptr> {
    auto ret = std::vector<reporter_base_ptr>{};
    for (const auto &tool: tools) {
      ret.emplace_back(tool->get_reporter());
    }
    return ret;
  }

  inline auto
  check_then_get_reporters(const std::vector<tool_base_ptr> &tools, const runtime_context &context)
    -> std::vector<reporter_base_ptr> {
    auto ret = std::vector<reporter_base_ptr>{};
    for (const auto &tool: tools) {
      tool->check(context);
      ret.emplace_back(tool->get_reporter());
    }
    return ret;
  }

} // namespace linter::tool
