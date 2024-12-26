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
#include "tools/clang_format/creator.h"

#include "program_options.h"
#include "tools/base_tool.h"
#include "tools/clang_format/general/impl.h"
#include "tools/clang_format/version/v18.h"
#include "tools/util.h"

namespace linter::tool::clang_format {
using namespace std::string_view_literals;

namespace {
constexpr auto enable = "enable-clang-format";
constexpr auto enable_fastly_exit = "enable-clang-format-fastly-exit";
constexpr auto version = "clang-format-version";
constexpr auto binary = "clang-format-binary";
constexpr auto file_iregex = "clang-format-file-iregex";

// example:
// Ubuntu clang-format version 18.1.3 (1ubuntu1)
auto get_clang_format_version(std::string_view binary) -> std::string {
  auto [ec, std_out, std_err] = shell::execute(binary, {"--version"});
  if (ec != 0) {
    return "";
  }
  constexpr auto version_regex = R"(version\ (\d+\.\d+\.\d+))";
  auto regex = boost::regex{version_regex};
  auto match = boost::smatch{};
  auto matched = boost::regex_search(std_out, match, regex, boost::match_extra);
  if (matched) {
    return match[1].str();
  }
  return "";
}

} // namespace

void creator::register_option(
    program_options::options_description &desc) const {
  using program_options::value;
  using std::string;

  const auto *ver = value<string>()->value_name("version");
  const auto *bin = value<string>()->value_name("path");
  const auto *iregex = value<string>()->value_name("iregex")->default_value(
      option.file_filter_iregex);

  auto boolean = [](bool def) {
    return value<bool>()->value_name("boolean")->default_value(def);
  };

  // clang-format off
  desc.add_options()
    (enable,              boolean(true),   "Enabel clang-format check")
    (enable_fastly_exit,  boolean(false),  "Enabel clang-format fastly exit. This means "
                                           "CppLintAction will stop clang-format as soon "
                                           "as first file error occurs")
    (version,             ver,             "Set the version of clang-format. Don't specify "
                                           "both this option and the clang-format-binary option, "
                                           "to avoid ambigous")
    (binary,              bin,             "Set the full path of clang-format executable binary. "
                                           "Don't spefify both this option and the clang-format-version "
                                           "option, to avoid ambigous")
    (file_iregex,         iregex,          "Set the source file filter for clang-format.")
  ;
  // clang-format on
}

void creator::create_option(const program_options::variables_map &variables) {
  option.enabled = variables[enable].as<bool>();
  if (!option.enabled) {
    // Speed up option creation
    return;
  }

  if (variables.contains(enable_fastly_exit)) {
    option.enabled_fastly_exit = variables[enable_fastly_exit].as<bool>();
  }
  if (variables.contains(file_iregex)) {
    option.file_filter_iregex = variables[file_iregex].as<std::string>();
  }
  if (variables.contains(version)) {
    program_options::must_not_specify("specify clang-format-version", variables,
                                      {"clang-format-binary"});

    option.version = variables[version].as<std::string>();
    option.binary = find_clang_tool("clang-tidy", option.version);
  } else if (variables.contains(binary)) {
    program_options::must_not_specify("specify clang-format-binary", variables,
                                      {"clang-format-version"});

    option.binary = variables[binary].as<std::string>();
    auto [ec, std_out, std_err] = shell::which(option.binary);
    throw_unless(
        ec == 0,
        std::format("Can't find given clang-format binary: {}", option.binary));
    option.version = get_clang_format_version(option.binary);
    throw_if(option.version.empty(), "can't get clang-format version");
  } else {
    auto [ec, std_out, std_err] = shell::which("clang-format");
    throw_unless(ec == 0, "can't find clang-format");
    option.binary = std_out;
    option.version = get_clang_format_version(option.binary);
    throw_if(option.version.empty(), "can't get clang-format version");
  }
  spdlog::info("The clang-format executable path: {}", option.binary);
}

auto creator::create_tool(const runtime_context &context) -> tool_base_ptr {
  auto version = option.version;
  auto tool = tool_base_ptr{};
  if (version == version_18_1_3) {
    tool = std::make_unique<clang_format_v18_1_3>(option);
  } else if (version == version_18_1_0) {
    tool = std::make_unique<clang_format_v18_1_0>(option);
  } else {
    tool = std::make_unique<clang_format_general>(option);
  }
  throw_unless(tool->is_supported(context.os, context.arch),
               std::format("Create clang-format {} instance failed since not "
                           "supported on this platform",
                           version));

  return tool;
}

bool creator::enabled([[maybe_unused]] const runtime_context &context) {
  return option.enabled;
}

[[nodiscard]] auto creator::get_option() const -> const option_t & {
  return option;
}
} // namespace linter::tool::clang_format
