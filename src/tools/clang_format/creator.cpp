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
} // namespace

void creator::register_option(
    program_options::options_description &desc) const {
  using program_options::value;
  using std::string;

  const auto *ver = value<string>()->value_name("version");
  const auto *bin = value<string>()->value_name("path")->default_value(
      "/usr/bin/clang-format");
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
    (version,             ver,             "Set the version of clang-format You aren't "
                                           "allowed to specify both this option and "
                                           "clang-format-binary to avoid ambiguous")
    (binary,              bin,             "Set the full path of clang-format executable binary. "
                                           "You are't allowed to specify both this option and "
                                           "clang-format-version to avoid ambiguous")
    (file_iregex,         iregex,          "Set the source file filter for clang-format.")
  ;
  // clang-format on
}

void creator::create_option(const program_options::variables_map &variables) {
  // Speed up option creation
  if (!variables.contains(enable)) {
    return;
  }
  option.enabled = variables[enable].as<bool>();
  if (!option.enabled) {
    return;
  }

  if (variables.contains(enable_fastly_exit)) {
    option.enabled_fastly_exit = variables[enable_fastly_exit].as<bool>();
  }
  if (variables.contains(version)) {
    option.version = variables[version].as<std::string>();
    throw_if(variables.contains(binary),
             "specify both clang-format-binary and clang-format-version is "
             "ambiguous");
    option.binary = find_clang_tool("clang-format", option.version);
  } else if (variables.contains(binary)) {
    throw_if(variables.contains(version),
             "specify both clang-format-binary and clang-format-version is "
             "ambiguous");
    option.binary = variables[binary].as<std::string>();
    auto [ec, std_out, std_err] = shell::which(option.binary);
    throw_unless(
        ec == 0,
        std::format("Can't find given clang_format binary: {}", option.binary));
    spdlog::info("The clang-format executable path: {}", option.binary);
  } else {
    auto [ec, std_out, std_err] = shell::which("clang-format");
    throw_unless(ec == 0, "Can't find clang-format");
    option.binary = std_out;
  }
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
} // namespace linter::tool::clang_format
