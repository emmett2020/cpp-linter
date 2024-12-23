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

#include "program_options.h"
#include "tools/base_tool.h"
#include <boost/program_options/variables_map.hpp>

namespace linter::tool {

  struct creator_base {
    virtual ~creator_base() = default;

    virtual void register_option(program_options::options_description &desc) const = 0;

    virtual void create_option(const program_options::variables_map &variables) = 0;

    virtual bool tool_is_enabled(const runtime_context &context) = 0;

    virtual auto create_tool(const runtime_context &context) -> tool_base_ptr = 0;
  };

  using creator_base_ptr = std::unique_ptr<creator_base>;

  inline auto register_tool_options(const std::vector<creator_base_ptr> &creators,
                                    program_options::options_description &desc) {
    for (const auto &creator: creators) {
      creator->register_option(desc);
    }
  }

  inline auto create_tool_options(const std::vector<creator_base_ptr> &creators,
                                  program_options::variables_map &variables) {
    for (const auto &creator: creators) {
      creator->create_option(variables);
    }
  }

  inline auto create_enabled_tools(const std::vector<creator_base_ptr> &creators,
                                   const runtime_context &context) -> std::vector<tool_base_ptr> {
    auto ret = std::vector<tool_base_ptr>{};
    for (const auto &creator: creators) {
      if (creator->tool_is_enabled(context)) {
        ret.emplace_back(creator->create_tool(context));
      }
    }
    return ret;
  }

} // namespace linter::tool
