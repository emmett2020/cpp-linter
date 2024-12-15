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

#include "base_option.h"
#include "base_result.h"

namespace linter {

  enum class system_t {
    none    = 0,
    windows = 1 << 0,
    macos   = 1 << 1,
    linux   = 1 << 2,
  };

  /// We don't consider 32-bit architectures.
  enum class arch_t {
    none,
    all,
    x64_64,
    arm64,
  };

  /// This is a base class represents linter tools. All specified tools should be
  /// derived from this.
  class base_tool {
  public:
    virtual ~base_tool()                                   = default;
    virtual auto supported_system() -> uint64_t            = 0;
    virtual auto supported_arch(system_t system) -> arch_t = 0;
    virtual constexpr auto name() -> std::string_view      = 0;
    virtual constexpr auto version() -> std::string_view   = 0;

    virtual auto apply_to_single_file(
      const base_user_option &option,
      const std::string &repo, //
      const std::string &file) -> per_file_base_result_ptr = 0;

    auto run(const base_user_option &option,
             const std::string &repo,
             const std::vector<std::string> &files) -> base_result;
  };

  using base_tool_ptr = std::unique_ptr<base_tool>;

  struct tool_creator {
    virtual ~tool_creator()                                                             = default;
    virtual auto create_instance(system_t cur_system, arch_t cur_arch) -> base_tool_ptr = 0;
  };

} // namespace linter
