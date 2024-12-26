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
using namespace linter::program_options;

template <class... Opts>
auto make_opt(Opts &&...opts) -> std::array<char *, sizeof...(Opts) + 1> {
  return {const_cast<char *>("CppLintAction"), // NOLINT
          const_cast<char *>(opts)...};        // NOLINT
}

TEST_CASE("Test create program options descriptions",
          "[cpp-linter][program_options]") {
  auto desc = create_desc();

  SECTION("help") {
    auto opts = make_opt("--help");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE(user_options.contains("help"));
  }

  SECTION("version") {
    auto opts = make_opt("--version");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE(user_options.contains("version"));
  }
}

TEST_CASE("Test must_specify could throw", "[cpp-linter][program_options]") {
  auto desc = program_options::create_desc();
  auto opts = make_opt("--help");
  auto user_options = parse(opts.size(), opts.data(), desc);
  REQUIRE_NOTHROW(must_specify("test", user_options, {"help"}));
  REQUIRE_THROWS(must_specify("test", user_options, {"version"}));
}

TEST_CASE("Test must_not_specify could throw",
          "[cpp-linter][program_options]") {
  auto desc = create_desc();
  auto opts = make_opt("--help");
  auto user_options = parse(opts.size(), opts.data(), desc);
  REQUIRE_THROWS(must_not_specify("test", user_options, {"help"}));
  REQUIRE_NOTHROW(must_not_specify("test", user_options, {"version"}));
}

TEST_CASE("Test fill context by program options",
          "[cpp-linter][program_options]") {
  auto desc = create_desc();
  auto context = runtime_context{};

  SECTION("user not specifies target should throw exception") {
    auto opts = make_opt("--log-level=info");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_THROWS(context = create_context(user_options));
  }

  SECTION("user not specifies unsupported log level should throw exception") {
    auto opts = make_opt("--target-revision=main", "--log-level=WARN");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_THROWS(context = create_context(user_options));
  }

  SECTION("user specifies supported log level shouldn't cause exception") {
    auto opts = make_opt("--target-revision=main", "--log-level=iNfo");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_NOTHROW(context = create_context(user_options));
    REQUIRE(context.log_level == "info");
  }

  SECTION("enable_step_summary should be passed into context") {
    auto opts = make_opt("--target-revision=main", "--enable-step-summary=false");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_NOTHROW(context = create_context(user_options));
    REQUIRE(context.enable_step_summary == false);
  }

  SECTION("enable_action_output should be passed into context") {
    auto opts = make_opt("--target-revision=main", "--enable-action-output=false");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_NOTHROW(context = create_context(user_options));
    REQUIRE(context.enable_action_output == false);
  }

  SECTION("enable_comment_on_issue should be passed into context") {
    auto opts = make_opt("--target-revision=main", "--enable-step-summary=true");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_NOTHROW(context = create_context(user_options));
    REQUIRE(context.enable_step_summary == true);
  }

  SECTION("enable_pull_request_review should be passed into context") {
    auto opts =
        make_opt("--target-revision=main", "--enable-pull-request-review=true");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_NOTHROW(context = create_context(user_options));
    REQUIRE(context.enable_pull_request_review == true);
  }

  SECTION("default values should be passed into context") {
    auto opts = make_opt("--target-revision=main");
    auto user_options = parse(opts.size(), opts.data(), desc);
    REQUIRE_NOTHROW(context = create_context(user_options));
    REQUIRE(context.enable_step_summary == true);
    REQUIRE(context.enable_comment_on_issue == true);
    REQUIRE(context.enable_pull_request_review == false);
    REQUIRE(context.enable_action_output == true);
  }
}
