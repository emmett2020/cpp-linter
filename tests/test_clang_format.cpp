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
#include "test_common.h"
#include "tools/base_creator.h"
#include "tools/base_tool.h"
#include "tools/clang_format/clang_format.h"
#include "tools/clang_format/creator.h"
#include "tools/util.h"
#include "utils/shell.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

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

  auto context = runtime_context{};
  program_options::fill_context(vars, context);
  auto clang_format = creator->create_tool(vars);
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
    auto clang_format = creator->create_tool(vars);
    auto context = runtime_context{};
    program_options::fill_context(vars, context);
    REQUIRE(clang_format->version() == "18.1.3");
    REQUIRE(clang_format->name() == "clang-format");
  }
}

TEST_CASE("Test clang-format could correctly handle various file level cases",
          "[CppLintAction][tool][clang_format][general_version]") {
  SKIP_IF_NO_CLANG_FORMAT
  auto creator = std::make_unique<clang_format::creator>();
  auto desc = create_then_register_tool_desc(*creator);
  auto vars = parse_opt(
      desc, "--target-revision=master", "--enable-pull-request-review=false",
      "--enable-comment-on-issue=false", "--enable-action-output=false",
      "--enable-step-summary=false");
  auto clang_format = creator->create_tool(vars);

  auto context = runtime_context{};
  program_options::fill_context(vars, context);

  auto repo = repo_t{};

  auto debug_env = github::github_env{};
  debug_env.workspace = repo.get_path();
  debug_env.event_name = github::github_event_pull_request;

  constexpr auto *clang_format_content = R"(
    BasedOnStyle: Google
    AllowShortBlocksOnASingleLine: Never
  )";
  repo.add_file(".clang-format", clang_format_content);
  repo.commit_changes();

  SECTION("Test file filter should be work") {}

  SECTION("DELETED files shouldn't be checked") {}
  SECTION("NEW added files should be checked") {}
  SECTION("MODIFIED files should be checked") {}

  SECTION("The commits only delete file should check nothing") {}
  SECTION("The commits only add new cpp files should only check these files") {}
  SECTION("The commits modified one file and insert a new file should check "
          "these two files") {}
  SECTION("The commits modified one file and delete an old file should only "
          "check the modified file") {}
}

void check_result(tool_base &tool) {
  auto [pass, passed, failed, ignored] =
      tool.get_reporter()->get_brief_result();
  REQUIRE(pass);
  REQUIRE(passed == 1);
  REQUIRE(failed == 0);
  REQUIRE(ignored == 0);
}

TEST_CASE("Test clang-format could correctly check basic unformatted error",
          "[CppLintAction][tool][clang_format][general_version]") {
  SKIP_IF_NO_CLANG_FORMAT
  auto creator = std::make_unique<clang_format::creator>();
  auto desc = create_then_register_tool_desc(*creator);
  auto vars = parse_opt(
      desc, "--target-revision=master", "--enable-pull-request-review=false",
      "--enable-comment-on-issue=false", "--enable-action-output=false",
      "--enable-step-summary=false");
  auto clang_format = creator->create_tool(vars);

  auto context = runtime_context{};
  program_options::fill_context(vars, context);

  auto repo = repo_t{};

  auto debug_env = github::github_env{};
  debug_env.workspace = repo.get_path();
  debug_env.event_name = github::github_event_pull_request;

  constexpr auto *clang_format_content = R"(
    BasedOnStyle: Google
    AllowShortBlocksOnASingleLine: Never
  )";
  repo.add_file(".clang-format", clang_format_content);
  repo.commit_changes();

  SECTION("basic example") {
    repo.add_file("file.cpp", "int   n = 0;");
    auto [target_id, target] = repo.commit_changes();
    repo.rewrite_file("file.cpp", "int n = 0;");
    auto [source_id, source] = repo.commit_changes();
    spdlog::info("target_id: {}, source_id: {}", target_id, source_id);

    context.target = target_id;
    debug_env.github_sha = source_id;
    github::fill_context(debug_env, context);
    fill_git_info(context);

    clang_format->check(context);
    check_result(*clang_format);
  }

  SECTION("Insert unformatted lines shouldn't pass clang-format check") {}

  SECTION("Insert formatted lines should pass clang-format check") {}
  SECTION("Delete all unformatted lines will pass clang-format check") {}
  SECTION(
      "Delete only some unformatted lines shouldn't pass clang-format check") {}

  SECTION("Rewrite unformatted lines to unformatted lines shouldn't pass "
          "clang-format check") {}
  SECTION("Rewrite unformatted lines to formatted lines should pass "
          "clang-format check") {}
  SECTION("Rewrite formatted lines to unformatted lines shouldn't pass "
          "clang-format check") {}
  SECTION("Rewrite formatted lines to formatted lines should pass "
          "clang-format check") {}
}

TEST_CASE("Test parse replacements",
          "[CppLintAction][tool][clang_format][general_version]") {
  SKIP_IF_NO_CLANG_FORMAT

  SECTION("Empty replacements") {}
  SECTION("One replacement") {}
  SECTION("Two replacements") {}
}
