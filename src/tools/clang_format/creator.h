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

#include "tools/base_creator.h"
#include "tools/base_tool.h"
#include "tools/clang_format/general/option.h"

namespace linter::tool::clang_format {

  struct creator : creator_base {
    creator()                = default;
    ~creator() override      = default;
    creator(creator &&other) = default;

    void register_option(program_options::options_description &desc) const override;
    void create_option(const program_options::variables_map &variables) override;
    auto create_tool(const runtime_context &context) -> tool_base_ptr override;
    bool tool_is_enabled(const runtime_context &context) override;

  private:
    option_t option;
  };

} // namespace linter::tool::clang_format
