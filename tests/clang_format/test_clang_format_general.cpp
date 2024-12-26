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

#include "context.h"
#include "program_options.h"
#include "tools/clang_format/clang_format.h"
#include "tools/clang_format/creator.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {
template <class... Opts>
auto make_opt(Opts &&...opts) -> std::array<char *, sizeof...(Opts) + 1> {
  return {const_cast<char *>("CppLintAction"), // NOLINT
          const_cast<char *>(opts)...};        // NOLINT
}
} // namespace

using namespace linter;
using namespace linter::tool;

TEST_CASE("Test register option should work",
          "[cpp-linter][tool][clang_format][general]") {
  auto creator = clang_format::creator{};
  auto desc = program_options::create_desc();
  creator.register_option(desc);

  SECTION("Enable or disable clang-format should work") {
    auto inputs = make_opt("--target=main", "--enable-clang-format=true");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    auto context = program_options::create_context(user_options);
    creator.create_tool(context);
  }
}
