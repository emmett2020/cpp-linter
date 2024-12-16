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
#include "tools/base_option.h"
#include "tools/base_reporter.h"
#include "tools/base_result.h"
#include "tools/base_tool.h"

namespace linter::tool {

template <class UserOption, class PerFileResult> struct creator_base {
  virtual ~creator_base() = default;

  virtual void
  register_option_desc(program_options::options_description &desc) = 0;

  virtual auto create_option(const program_options::variables_map &variables)
      -> UserOption = 0;
  virtual auto create_instance(operating_system_t cur_system, arch_t cur_arch,
                               const std::string &version)
      -> tool_base_ptr<UserOption, PerFileResult> = 0;
  virtual auto create_reporter()
      -> reporter_base<UserOption, final_result<PerFileResult>> = 0;
};

} // namespace linter::tool
