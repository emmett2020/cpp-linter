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

constexpr auto enable_clang_format = "enable-clang-format";
constexpr auto enable_clang_format_fastly_exit =
    "enable-clang-format-fastly-exit";
constexpr auto clang_format_version = "clang-format-version";
constexpr auto clang_format_binary = "clang-format-binary";
constexpr auto clang_format_iregex = "clang-format-iregex";

void creator::register_option(
    program_options::options_description &desc) const {
  using namespace program_options; // NOLINT
  using std::string;

  // clang-format off
    desc.add_options()
      (enable_clang_format,              value<bool>(),      "Enabel clang-format check")
      (enable_clang_format_fastly_exit,  value<bool>(),      "Enabel clang-format fastly exit."
                                                             "This means cpp-linter will stop all clang-format"
                                                             "checks as soon as first error occurs")
      (clang_format_version,             value<uint16_t>(),  "The version of clang-format to be used")
      (clang_format_binary,              value<string>(),    "The binary of clang-format to be used. You are't allowed to specify"
                                                             "both this option and clang-format-version to avoid ambiguous.")
       ;
  // clang-format on
}

void creator::create_option(const program_options::variables_map &variables) {
  if (variables.contains(enable_clang_format)) {
    option.enabled = variables[enable_clang_format].as<bool>();
  }
  if (variables.contains(enable_clang_format_fastly_exit)) {
    option.enabled_fastly_exit =
        variables[enable_clang_format_fastly_exit].as<bool>();
  }
  if (variables.contains(clang_format_version)) {
    option.version = variables[clang_format_version].as<std::string>();
    throw_if(variables.contains(clang_format_binary),
             "specify both clang-format-binary and clang-format-version is "
             "ambiguous");
    option.binary = find_clang_tool("clang-format", option.version);
  }
  if (variables.contains(clang_format_binary)) {
    throw_if(variables.contains(clang_format_version),
             "specify both clang-format-binary and clang-format-version is "
             "ambiguous");
    option.binary = variables[clang_format_binary].as<std::string>();
    if (option.enabled) {
      auto [ec, stdout, stderr] = shell::which(option.binary);
      throw_unless(ec == 0,
                   std::format("can't find given clang_format_binary: {}",
                               option.binary));
    }
  }
  spdlog::info("The clang-format executable path: {}", option.binary);
}

auto creator::create_tool(operating_system_t cur_system,
                          arch_t cur_arch) -> tool_base_ptr {
  auto version = option.version;
  auto tool = tool_base_ptr{};
  if (version == version_18_1_3) {
    tool = std::make_unique<clang_format_v18_1_3>();
  } else if (version == version_18_1_0) {
    tool = std::make_unique<clang_format_v18_1_0>();
  } else {
    tool = std::make_unique<clang_format_general>();
  }

  throw_unless(tool->is_supported(cur_system, cur_arch),
               std::format("Create clang-format {} instance failed since not "
                           "supported on this platform",
                           version));
  return tool;
}
} // namespace linter::tool::clang_format
