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

#include "creator.h"

#include <utility>

#include "tools/clang_format/base_impl.h"
#include "tools/clang_format/version_18.h"
#include "utils/util.h"

namespace linter::tool::clang_format {
auto create_instance(operating_system_t cur_system, arch_t cur_arch,
                     const std::string &version) -> clang_format_ptr {
  throw_if(std::ranges::contains(supported_version, version),
           "Create clang-format instance failed since unsupported version.");

  auto tool = clang_format_ptr{};
  if (version == version_18_1_3) {
    tool = std::make_unique<clang_format_v18_1_3>();
  } else if (version == version_18_1_0) {
    tool = std::make_unique<clang_format_v18_1_0>();
  } else {
    std::unreachable();
  }

  throw_unless(tool->is_supported(cur_system, cur_arch),
               std::format("Create clang-format {} instance failed since not "
                           "supported on this platform",
                           version));
  return tool;
}

} // namespace linter::tool::clang_format
