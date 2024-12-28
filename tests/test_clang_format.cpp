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
#include "github/common.h"
#include "program_options.h"
#include "tools/base_creator.h"
#include "tools/base_tool.h"
#include "tools/clang_format/clang_format.h"
#include "tools/clang_format/creator.h"
#include "tools/util.h"
#include "utils/env_manager.h"
#include "utils/shell.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <unordered_map>

using namespace linter;
using namespace linter::tool;

namespace {
// Pass in c_str
template <class... Opts>
auto make_opt(Opts &&...opts) -> std::array<char *, sizeof...(Opts) + 1> {
  return {const_cast<char *>("CppLintAction"), // NOLINT
          const_cast<char *>(opts)...};        // NOLINT
}

template <class... Opts>
auto parse_opt(const program_options::options_description &desc,
               Opts &&...opts) -> program_options::variables_map {
  auto inputs = make_opt(std::forward<Opts &&>(opts)...);
  return program_options::parse(inputs.size(), inputs.data(), desc);
}

// Check whether local environment contains clang-format otherwise some checks
// will be failed.
bool has_clang_format() {
  auto [ec, std_out, std_err] = shell::which("clang-format");
  return ec == 0;
}

// Check whether local environment contains specific clang-format version
// otherwise some checks will be failed.
bool has_clang_format(std::string_view version) {
  try {
    find_clang_tool("clang-format", version);
    return true;
  } catch (std::runtime_error &err) {
    return false;
  }
}

auto create_then_register_tool_desc(const clang_format::creator &creator)
    -> program_options::options_description {
  auto desc = program_options::create_desc();
  creator.register_option(desc);
  return desc;
}

// Create a git repository to test clang-format could work.
auto create_git_repo() {
  //
}

#define SKIP_IF_NO_CLANG_FORMAT                                                \
  if (!has_clang_format()) {                                                   \
    SKIP("Local environment doesn't have clang-format. So skip clang-format "  \
         "unit tests.");                                                       \
  }

// NOLINTNEXTLINE
#define SKIP_IF_NOT_HAS_CLANG_FORMAT_VERSION(version)                          \
  if (!has_clang_format(version)) {                                            \
    SKIP("Local environment doesn't have required clang-format version. So "   \
         "skip clang-format unit tests.");                                     \
  }

} // namespace

TEST_CASE("Test register and create clang-format option",
          "[CppLintAction][tool][clang_format][creator]") {
  SKIP_IF_NO_CLANG_FORMAT
  auto creator = std::make_unique<clang_format::creator>();
  auto desc = create_then_register_tool_desc(*creator);

  SECTION("Explicitly enables clang-format should work") {
    auto opts =
        parse_opt(desc, "--target-revision=main", "--enable-clang-format=true");
    creator->create_option(opts);
    REQUIRE(creator->enabled());
  }

  SECTION("Explicitly disable clang-format should work") {
    auto opts = parse_opt(desc, "--target-revision=main",
                          "--enable-clang-format=false");
    creator->create_option(opts);
    REQUIRE(creator->enabled() == false);
  }

  SECTION("clang-format is defaultly enabled") {
    auto opts = parse_opt(desc, "--target-revision=main");
    creator->create_option(opts);
    REQUIRE(creator->enabled() == true);
  }

  SECTION("Receive an invalid clang-format version should throw exception") {
    auto opts = parse_opt(desc, "--target-revision=main",
                          "--clang-format-version=18.x.1");
    REQUIRE_THROWS(creator->create_option(opts));
  }

  SECTION("Receive an invalid clang-format binary should throw exception") {
    auto opts =
        parse_opt(desc, "--target-revision=main",
                  "--clang-format-binary=/usr/bin/clang-format-invalid");
    REQUIRE_THROWS(creator->create_option(opts));
  }

  SECTION("Other options should be correctly created based on user input") {
    auto opts = parse_opt(desc, "--target-revision=main",
                          "--enable-clang-format-fastly-exit=true",
                          "--clang-format-file-iregex=*.cpp");
    creator->create_option(opts);
    auto option = creator->get_option();
    REQUIRE(option.enabled_fastly_exit == true);
    REQUIRE(option.file_filter_iregex == "*.cpp");
  }
}

TEST_CASE("Test clang-format should get full version even though user input a "
          "simplified version",
          "[CppLintAction][tool][clang_format][creator]") {
  SKIP_IF_NOT_HAS_CLANG_FORMAT_VERSION("18")
  auto creator = std::make_unique<clang_format::creator>();
  auto desc = create_then_register_tool_desc(*creator);

  auto vars =
      parse_opt(desc, "--target-revision=main", "--clang-format-version=18");
  creator->create_option(vars);
  auto context = program_options::create_context(vars);
  auto clang_format = creator->create_tool(context);
  auto version = clang_format->version();
  auto parts = ranges::views::split(version, '.') |
               ranges::to<std::vector<std::string>>();
  REQUIRE(parts.size() == 3);
  REQUIRE(parts[0] == "18");
  REQUIRE(ranges::all_of(parts[1], isdigit));
  REQUIRE(ranges::all_of(parts[2], isdigit));
}

TEST_CASE("Create tool of spefific version should work",
          "[CppLintAction][tool][clang_format][creator]") {
  SKIP_IF_NOT_HAS_CLANG_FORMAT_VERSION("18.1.3")
  auto creator = std::make_unique<clang_format::creator>();
  auto desc = create_then_register_tool_desc(*creator);

  SECTION("version 18.1.3") {
    auto vars = parse_opt(desc, "--target-revision=main",
                          "--clang-format-version=18.1.3");
    creator->create_option(vars);
    auto context = program_options::create_context(vars);
    auto clang_format = creator->create_tool(context);
    REQUIRE(clang_format->version() == "18.1.3");
    REQUIRE(clang_format->name() == "clang-format");
  }
}

// utilities:
// 3. set cpp file content
// 4. operator repo to
// 5. run clang-format

TEST_CASE("Test clang-format could check file error",
          "[cpp-linter][tool][clang_format][pull-request]") {
  SKIP_IF_NO_CLANG_FORMAT
  auto creator = std::make_unique<clang_format::creator>();
  auto desc = create_then_register_tool_desc(*creator);

  SECTION("general version") {
    auto vars = parse_opt(desc, "--target-revision=main");
    auto context = program_options::create_context(vars);
    creator->create_option(vars);
    auto clang_format = creator->create_tool(context);
  }
}
