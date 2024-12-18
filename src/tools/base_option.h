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

#include <memory>
#include <string>

namespace linter::tool {
  struct option_base {
    bool enabled             = false;
    bool enabled_fastly_exit = false;
    std::string version;
    std::string binary;
    std::string source_filter_iregex = R"(.*\.(cpp|cc|c\+\+|cxx|c|cl|h|hpp|m|mm|inc))";
  };

  using option_base_ptr = std::unique_ptr<option_base>;
} // namespace linter::tool
