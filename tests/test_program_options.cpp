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

#include "program_options.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace linter;

template <class... Opts>
auto make_opt(Opts &&...opts) -> std::array<char *, sizeof...(Opts) + 1> {
  return {const_cast<char *>("CppLintAction"), // NOLINT
          const_cast<char *>(opts)...};        // NOLINT
}

TEST_CASE("Test create program options descriptions",
          "[cpp-linter][program_options]") {
  auto desc = create_program_options_desc();

  SECTION("help") {
    auto opts = make_opt("--help");
    auto user_options = parse_program_options(opts.size(), opts.data(), desc);
    REQUIRE(user_options.contains("help"));
  }

  SECTION("version") {
    auto opts = make_opt("--version");
    auto user_options = parse_program_options(opts.size(), opts.data(), desc);
    REQUIRE(user_options.contains("version"));
  }
}

TEST_CASE("Test must_specify could throw", "[cpp-linter][program_options]") {
  auto desc = create_program_options_desc();
  auto opts = make_opt("--help");
  auto user_options = parse_program_options(opts.size(), opts.data(), desc);
  REQUIRE_NOTHROW(must_specify("test", user_options, {"help"}));
  REQUIRE_THROWS(must_specify("test", user_options, {"version"}));
}

TEST_CASE("Test must_not_specify could throw",
          "[cpp-linter][program_options]") {
  auto desc = create_program_options_desc();
  auto opts = make_opt("--help");
  auto user_options = parse_program_options(opts.size(), opts.data(), desc);
  REQUIRE_THROWS(must_not_specify("test", user_options, {"help"}));
  REQUIRE_NOTHROW(must_not_specify("test", user_options, {"version"}));
}

TEST_CASE("Test fill context by program options",
          "[cpp-linter][program_options]") {
  auto desc = create_program_options_desc();
  auto context = runtime_context{};

  SECTION("user not specify target will cause exception thrown") {
    auto desc = create_program_options_desc();
    auto opts = make_opt("--log-level=info");
    auto user_options = parse_program_options(opts.size(), opts.data(), desc);
    REQUIRE_THROWS(fill_context_by_program_options(user_options, context));
  }

  SECTION("user not specify target will cause exception thrown") {
    auto opts = make_opt("--target=main");
    auto user_options = parse_program_options(opts.size(), opts.data(), desc);
  }
}
