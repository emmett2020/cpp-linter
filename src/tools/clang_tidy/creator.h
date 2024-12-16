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
#pragma once

#include <memory>
#include <string>

#include <boost/program_options.hpp>

#include "tools/base_creator.h"
#include "tools/base_tool.h"
#include "tools/clang_tidy/base_impl.h"
#include "tools/clang_tidy/version_18.h"
#include "tools/util.h"
#include "utils/util.h"

namespace linter::tool::clang_tidy {
constexpr auto supported_version = {version_18_1_0, version_18_1_3};

constexpr auto enable_clang_tidy = "enable-clang-tidy";
constexpr auto enable_clang_tidy_fastly_exit = "enable-clang-tidy-fastly-exit";
constexpr auto clang_tidy_version = "clang-tidy-version";
constexpr auto clang_tidy_binary = "clang-tidy-binary";
constexpr auto clang_tidy_allow_no_checks = "clang-tidy-allow-no-checks";
constexpr auto clang_tidy_enable_check_profile =
    "clang-tidy-enable-check-profile";
constexpr auto clang_tidy_checks = "clang-tidy-checks";
constexpr auto clang_tidy_config = "clang-tidy-config";
constexpr auto clang_tidy_config_file = "clang-tidy-config-file";
constexpr auto clang_tidy_database = "clang-tidy-database";
constexpr auto clang_tidy_header_filter = "clang-tidy-header-filter";
constexpr auto clang_tidy_line_filter = "clang-tidy-line-filter";
constexpr auto clang_tidy_iregex = "clang-tidy-iregex";

struct creator : public creator_base<user_option, per_file_result> {
  void
  register_option_desc(program_options::options_description &desc) override {
    using namespace program_options; // NOLINT
    using std::string;

    // clang-format off
    desc.add_options()
          (enable_clang_tidy,                value<bool>(),      "Enabel clang-tidy check")
          (enable_clang_tidy_fastly_exit,    value<bool>(),      "Enabel clang-tidy fastly exit."
                                                                 "This means cpp-linter will stop all clang-tidy"
                                                                 "checks as soon as first error occurs")
          (clang_tidy_version,               value<uint16_t>(),  "The version of clang-tidy to be used")
          (clang_tidy_binary,                value<string>(),    "The binary of clang-tidy to be used. You are't allowed to specify"
                                                                 "both this option and clang-tidy-version to avoid ambiguous.")
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

  auto create_option(const program_options::variables_map &variables)
      -> user_option override {
    auto opt = user_option{};
    if (variables.contains(enable_clang_tidy)) {
      opt.enabled = variables[enable_clang_tidy].as<bool>();
    }
    if (variables.contains(enable_clang_tidy_fastly_exit)) {
      opt.enabled_fastly_exit =
          variables[enable_clang_tidy_fastly_exit].as<bool>();
    }
    if (variables.contains(clang_tidy_version)) {
      opt.version = variables[clang_tidy_version].as<std::string>();
      throw_if(
          variables.contains(clang_tidy_binary),
          "specify both clang-tidy-binary and clang-tidy-version is ambiguous");
      opt.binary = find_clang_tool("clang-tidy", opt.version);
    }
    if (variables.contains(clang_tidy_binary)) {
      throw_if(
          variables.contains(clang_tidy_version),
          "specify both clang-tidy-binary and clang-tidy-version is ambiguous");
      opt.binary = variables[clang_tidy_binary].as<std::string>();
      if (opt.enabled) {
        auto [ec, stdout, stderr] = shell::which(opt.binary);
        throw_unless(
            ec == 0,
            std::format("can't find given clang_tidy_binary: {}", opt.binary));
      }
    }
    spdlog::info("The clang-tidy executable path: {}", opt.binary);
    if (variables.contains(clang_tidy_allow_no_checks)) {
      opt.allow_no_checks = variables[clang_tidy_allow_no_checks].as<bool>();
    }
    if (variables.contains(clang_tidy_enable_check_profile)) {
      opt.enable_check_profile =
          variables[clang_tidy_enable_check_profile].as<bool>();
    }
    if (variables.contains(clang_tidy_checks)) {
      opt.checks = variables[clang_tidy_checks].as<std::string>();
    }
    if (variables.contains(clang_tidy_config)) {
      opt.config = variables[clang_tidy_config].as<std::string>();
    }
    if (variables.contains(clang_tidy_config_file)) {
      opt.config_file = variables[clang_tidy_config_file].as<std::string>();
    }
    if (variables.contains(clang_tidy_database)) {
      opt.database = variables[clang_tidy_database].as<std::string>();
    }
    if (variables.contains(clang_tidy_header_filter)) {
      opt.header_filter = variables[clang_tidy_header_filter].as<std::string>();
    }
    if (variables.contains(clang_tidy_line_filter)) {
      opt.line_filter = variables[clang_tidy_line_filter].as<std::string>();
    }
    if (variables.contains(clang_tidy_iregex)) {
      opt.source_filter_iregex = variables[clang_tidy_iregex].as<std::string>();
    }
    return opt;
  }

  auto create_instance(operating_system_t cur_system, arch_t cur_arch,
                       const std::string &version) -> clang_tidy_ptr override {
    throw_if(std::ranges::contains(supported_version, version),
             "Create clang-tidy instance failed since unsupported version.");

    auto tool = clang_tidy_ptr{};
    if (version == version_18_1_3) {
      tool = std::make_unique<clang_tidy_v18_1_3>();
    } else if (version == version_18_1_0) {
      tool = std::make_unique<clang_tidy_v18_1_0>();
    } else {
      std::unreachable();
    }

    throw_unless(tool->is_supported(cur_system, cur_arch),
                 std::format("Create clang-tidy {} instance failed since not "
                             "supported on this platform",
                             version));
    return tool;
  }

  auto create_reporter()
      -> reporter_base<user_option, final_result<per_file_result>> override {
    return {};
  }
};

} // namespace linter::tool::clang_tidy
