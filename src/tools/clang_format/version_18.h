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

#include "tools/clang_format/base.h"

namespace linter::clang_format {
  using namespace std::string_view_literals;
  constexpr auto version_18_1_0 = "18.1.0"sv;
  constexpr auto version_18_1_3 = "18.1.3"sv;

  struct clang_format_v18_1_0 : base_clang_format {
    constexpr auto version() -> std::string_view override {
      return version_18_1_0;
    }
  };

  struct clang_format_v18_1_3 : base_clang_format {
    constexpr auto version() -> std::string_view override {
      return version_18_1_3;
    }
  };


} // namespace linter::clang_format
