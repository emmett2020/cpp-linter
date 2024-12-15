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

#include "tools/base_tool.h"
#include "tools/clang_tidy/base.h"
#include "tools/clang_tidy/version_18.h"

namespace linter::clang_tidy {
  constexpr auto supported_version = {version_18_1_0, version_18_1_3};

  auto create_option() -> user_option;

  auto create_instance(operating_system_t cur_system, arch_t cur_arch, const std::string& version)
    -> clang_tidy_ptr;

} // namespace linter::clang_tidy
