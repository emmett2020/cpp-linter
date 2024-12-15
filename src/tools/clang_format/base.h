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
#include <vector>

#include "tools/base_option.h"
#include "tools/base_result.h"
#include "tools/base_tool.h"

namespace linter::clang_format {
  struct user_option : base_user_option {
    bool enable_warning_as_error = false;
  };

  struct replacement_t {
    int offset;
    int length;
    std::string data;
  };

  using replacements_t = std::vector<replacement_t>;
  void trace_replacement(const replacement_t &replacement);

  struct per_file_result : per_file_base_result {
    replacements_t replacements;
    std::string formatted_source_code;
  };

  struct base_clang_format : base_tool {
    auto supported_system() -> uint64_t override {
      return static_cast<uint64_t>(system_t::linux);
    }

    auto supported_arch(system_t system) -> arch_t override {
      if (system == system_t::linux) {
        return arch_t::all;
      }
      return arch_t::none;
    }

    constexpr auto name() -> std::string_view override {
      return "clang_format";
    }
  };
} // namespace linter::clang_format
