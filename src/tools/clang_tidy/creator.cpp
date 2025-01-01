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

#include "creator.h"

#include <boost/program_options.hpp>

#include "tools/clang_tidy/general/option.h"
#include "tools/clang_tidy/version/v18.h"
#include "tools/util.h"

namespace lint::tool::clang_tidy {
  void creator::register_option(program_options::options_description &desc) const {
    using namespace program_options; // NOLINT
    using std::string;

    // clang-format off
  desc.add_options()
    (enable_clang_tidy,                value<bool>(),      "Enabel clang-tidy check")
    (enable_clang_tidy_fastly_exit,    value<bool>(),      "Enabel clang-tidy fastly exit. "
                                                           "This means CppLintAction will stop all clang-tidy "
                                                           "checks as soon as first file error occurs")
    (clang_tidy_version,               value<string>(),    "Set The version of clang-tidy")
    (clang_tidy_binary,                value<string>(),    "Set the full path of clang-tidy executable binary. "
                                                           "You are't allowed to specify both this option and "
                                                           "clang-format-version to avoid ambiguous")
    (clang_tidy_allow_no_checks,       value<bool>(),      "Enabel clang-tidy allow_no_check option")
    (clang_tidy_enable_check_profile,  value<bool>(),      "Enabel clang-tidy enable_check_profile option")
    (clang_tidy_checks,                value<string>(),    "Same as clang-tidy checks option")
    (clang_tidy_config,                value<string>(),    "Same as clang-tidy config option")
    (clang_tidy_config_file,           value<string>(),    "Same as clang-tidy config_file option")
    (clang_tidy_database,              value<string>(),    "Same as clang-tidy -p option")
    (clang_tidy_header_filter,         value<string>(),    "Same as clang-tidy header_filter option")
    (clang_tidy_line_filter,           value<string>(),    "Same as clang-tidy line_filter option")
  ;
    // clang-format on
  }

  void creator::create_option(const program_options::variables_map &variables) {
    // Speed up option creation
    if (!variables.contains(enable_clang_tidy)) {
      return;
    }
    option.enabled = variables[enable_clang_tidy].as<bool>();
    if (!option.enabled) {
      return;
    }

    if (variables.contains(enable_clang_tidy_fastly_exit)) {
      option.enabled_fastly_exit = variables[enable_clang_tidy_fastly_exit].as<bool>();
    }
    if (variables.contains(clang_tidy_version)) {
      option.version = variables[clang_tidy_version].as<std::string>();
      throw_if(variables.contains(clang_tidy_binary),
               "specify both clang-tidy-binary and clang-tidy-version is ambiguous");
      option.binary = find_clang_tool("clang-tidy", option.version);
    } else if (variables.contains(clang_tidy_binary)) {
      throw_if(variables.contains(clang_tidy_version),
               "specify both clang-tidy-binary and clang-tidy-version is ambiguous");
      option.binary               = variables[clang_tidy_binary].as<std::string>();
      auto [ec, std_out, std_err] = shell::which(option.binary);
      throw_unless(ec == 0, fmt::format("Can't find given clang_tidy_binary: {}", option.binary));
      spdlog::info("The clang-tidy executable path: {}", option.binary);
    } else {
      auto [ec, std_out, std_err] = shell::which("clang-tidy");
      throw_unless(ec == 0, "Can't find clang-tidy");
      option.binary = std_out;
    }

    if (variables.contains(clang_tidy_allow_no_checks)) {
      option.allow_no_checks = variables[clang_tidy_allow_no_checks].as<bool>();
    }
    if (variables.contains(clang_tidy_enable_check_profile)) {
      option.enable_check_profile = variables[clang_tidy_enable_check_profile].as<bool>();
    }
    if (variables.contains(clang_tidy_checks)) {
      option.checks = variables[clang_tidy_checks].as<std::string>();
    }
    if (variables.contains(clang_tidy_config)) {
      option.config = variables[clang_tidy_config].as<std::string>();
    }
    if (variables.contains(clang_tidy_config_file)) {
      option.config_file = variables[clang_tidy_config_file].as<std::string>();
    }
    if (variables.contains(clang_tidy_database)) {
      option.database = variables[clang_tidy_database].as<std::string>();
    }
    if (variables.contains(clang_tidy_header_filter)) {
      option.header_filter = variables[clang_tidy_header_filter].as<std::string>();
    }
    if (variables.contains(clang_tidy_line_filter)) {
      option.line_filter = variables[clang_tidy_line_filter].as<std::string>();
    }
    if (variables.contains(clang_tidy_iregex)) {
      option.file_filter_iregex = variables[clang_tidy_iregex].as<std::string>();
    }
  }

  auto creator::create_tool(const program_options::variables_map &variables) -> tool_base_ptr {
    create_option(variables);
    auto version = option.version;

    auto tool = tool_base_ptr{};
    if (version == version_18_1_3) {
      tool = std::make_unique<clang_tidy_v18_1_3>(option);
    } else if (version == version_18_1_0) {
      tool = std::make_unique<clang_tidy_v18_1_0>(option);
    } else {
      tool = std::make_unique<clang_tidy_general>(option);
    }

    const auto os   = operating_system_t::ubuntu;
    const auto arch = arch_t::x86_64;
    throw_unless(tool->is_supported(os, arch),
                 fmt::format("Create clang-tidy {} instance failed since not "
                             "supported on this platform",
                             version));
    return tool;
  }

  bool creator::enabled() {
    return option.enabled;
  }

} // namespace lint::tool::clang_tidy
