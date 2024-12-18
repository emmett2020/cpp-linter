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

  constexpr auto enable_clang_format             = "enable-clang-format";
  constexpr auto enable_clang_format_fastly_exit = "enable-clang-format-fastly-exit";
  constexpr auto clang_format_version            = "clang-format-version";
  constexpr auto clang_format_binary             = "clang-format-binary";
  constexpr auto clang_format_iregex             = "clang-format-iregex";

  void creator::register_option(program_options::options_description &desc) const {
    using namespace program_options; // NOLINT
    using std::string;

    // clang-format off
  desc.add_options()
    (enable_clang_format,              value<bool>(),      "Enabel clang-format check")
    (enable_clang_format_fastly_exit,  value<bool>(),      "Enabel clang-format fastly exit. "
                                                           "This means cpp-linter will stop all clang-format "
                                                           "checks as soon as first file error occurs")
    (clang_format_version,             value<string>(),    "Set the version of clang-format")
    (clang_format_binary,              value<string>(),    "Set the full path of clang-format executable binary. "
                                                           "You are't allowed to specify both this option and "
                                                           "clang-format-version to avoid ambiguous")
  ;
    // clang-format on
  }

  void creator::create_option(const program_options::variables_map &variables) {
    // Speed up option creation
    if (!variables.contains(enable_clang_format)) {
      return;
    }
    option.enabled = variables[enable_clang_format].as<bool>();
    if (!option.enabled) {
      return;
    }

    if (variables.contains(enable_clang_format_fastly_exit)) {
      option.enabled_fastly_exit = variables[enable_clang_format_fastly_exit].as<bool>();
    }
    if (variables.contains(clang_format_version)) {
      option.version = variables[clang_format_version].as<std::string>();
      throw_if(variables.contains(clang_format_binary),
               "specify both clang-format-binary and clang-format-version is "
               "ambiguous");
      option.binary = find_clang_tool("clang-format", option.version);
    } else if (variables.contains(clang_format_binary)) {
      throw_if(variables.contains(clang_format_version),
               "specify both clang-format-binary and clang-format-version is "
               "ambiguous");
      option.binary               = variables[clang_format_binary].as<std::string>();
      auto [ec, std_out, std_err] = shell::which(option.binary);
      throw_unless(ec == 0, std::format("Can't find given clang_format binary: {}", option.binary));
      spdlog::info("The clang-format executable path: {}", option.binary);
    } else {
      auto [ec, std_out, std_err] = shell::which("clang-format");
      throw_unless(ec == 0, "Can't find clang-format");
      option.binary = std_out;
    }
  }

  auto creator::create_tool(const runtime_context &context) -> tool_base_ptr {
    auto version = option.version;
    auto tool    = tool_base_ptr{};
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

  bool creator::tool_is_enabled([[maybe_unused]] const runtime_context &context) {
    return option.enabled;
  }
} // namespace linter::tool::clang_format
