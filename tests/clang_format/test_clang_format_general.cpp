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
#include "tools/base_creator.h"
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
  auto creator_ptr = std::make_unique<clang_format::creator>();
  auto &creator = *creator_ptr;

  auto desc = program_options::create_desc();
  creator.register_option(desc);

  SECTION("Enable clang-format should work") {
    auto inputs =
        make_opt("--target-revision=main", "--enable-clang-format=true");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    creator.create_option(user_options);
    auto context = program_options::create_context(user_options);
    REQUIRE(creator.enabled(context));
  }

  SECTION("Disable clang-format should work") {
    auto inputs =
        make_opt("--target-revision=main", "--enable-clang-format=false");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    creator.create_option(user_options);
    auto context = program_options::create_context(user_options);
    REQUIRE(creator.enabled(context) == false);
  }

  SECTION("clang-format is defaultly enabled") {
    auto inputs = make_opt("--target-revision=main");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    creator.create_option(user_options);
    auto context = program_options::create_context(user_options);
    REQUIRE(creator.enabled(context) == true);
  }

  SECTION("test specify both version and binary") {
    auto inputs =
        make_opt("--target-revision=main", "--clang-format-version=18.0.1",
                 "--clang-format-binary=/usr/bin/clang-format-19");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    REQUIRE_THROWS(creator.create_option(user_options));
  }

  SECTION("test specify an invalid version") {
    auto inputs =
        make_opt("--target-revision=main", "--clang-format-version=18.x.1");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    REQUIRE_THROWS(creator.create_option(user_options));
  }

  SECTION("test specify an invalid binary") {
    auto inputs =
        make_opt("--target-revision=main",
                 "--clang-format-binary=/usr/bin/clang-format-invalid");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    REQUIRE_THROWS(creator.create_option(user_options));
  }

  SECTION("test other clang-format options") {
    auto inputs = make_opt("--target-revision=main",
                           "--enable-clang-format-fastly-exit=true",
                           "--clang-format-file-iregex=*.cpp");
    auto user_options =
        program_options::parse(inputs.size(), inputs.data(), desc);
    auto context = program_options::create_context(user_options);
    creator.create_option(user_options);
    REQUIRE(creator.enabled(context) == true);
    auto tool = creator.create_tool(context);
    auto option = creator_ptr->get_option();
    REQUIRE(option.enabled_fastly_exit == true);
    REQUIRE(option.file_filter_iregex == "*.cpp");
  }
}
