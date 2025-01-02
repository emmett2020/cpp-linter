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
  namespace {
    constexpr auto enable_clang_tidy               = "enable-clang-tidy";
    constexpr auto enable_clang_tidy_fastly_exit   = "enable-clang-tidy-fastly-exit";
    constexpr auto clang_tidy_version              = "clang-tidy-version";
    constexpr auto clang_tidy_binary               = "clang-tidy-binary";
    constexpr auto clang_tidy_allow_no_checks      = "clang-tidy-allow-no-checks";
    constexpr auto clang_tidy_enable_check_profile = "clang-tidy-enable-check-profile";
    constexpr auto clang_tidy_checks               = "clang-tidy-checks";
    constexpr auto clang_tidy_config               = "clang-tidy-config";
    constexpr auto clang_tidy_config_file          = "clang-tidy-config-file";
    constexpr auto clang_tidy_database             = "clang-tidy-database";
    constexpr auto clang_tidy_header_filter        = "clang-tidy-header-filter";
    constexpr auto clang_tidy_line_filter          = "clang-tidy-line-filter";
    constexpr auto clang_tidy_file_iregex          = "clang-tidy-file-iregex";
  } // namespace

  // Get version from clang-tidy output.
  // Example: Ubuntu LLVM version 18.1.3
  auto get_version(const std::string &binary) -> std::string {
    auto [ec, std_out, std_err] = shell::execute(binary, {"--version"});
    if (ec != 0) {
      return "";
    }
    constexpr auto version_regex = R"(version\ (\d+\.\d+\.\d+))";
    auto regex                   = boost::regex{version_regex};
    auto match                   = boost::smatch{};
    auto matched                 = boost::regex_search(std_out, match, regex, boost::match_extra);
    throw_unless(matched, "Can't get clang-tidy version");
    return match[1].str();
  }

  void creator::register_option(program_options::options_description &desc) const {
    using program_options::value;

    const auto *ver    = value<std::string>()->value_name("version");
    const auto *bin    = value<std::string>()->value_name("path");
    const auto *iregex = value<std::string>()->value_name("iregex")->default_value(
      option.file_filter_iregex);
    const auto *database = value<std::string>()->value_name("path")->default_value("build");

    auto boolean = [](bool def) {
      return value<bool>()->value_name("bool")->default_value(def);
    };

    auto str = []() {
      return value<std::string>()->value_name("string")->default_value("");
    };


    // clang-format off
    desc.add_options()
      (enable_clang_tidy,                boolean(true),   "Enabel clang-tidy check")
      (enable_clang_tidy_fastly_exit,    boolean(false),  "Enabel clang-tidy fastly exit. This means "
                                                          "CppLintAction will stop all clang-tidy as soon "
                                                          "as first file error occurs")
      (clang_tidy_version,               ver,             "Set the version of clang-tidy. Don't specify "
                                                          "both this option and the clang-tidy-binary option, "
                                                          "to avoid ambigous. And the clang-tidy-${version} must "
                                                          "exist in your $PATH")
      (clang_tidy_binary,                bin,             "Set the full path of clang-format executable binary. "
                                                          "Don't spefify both this option and the clang-format-version "
                                                          "option to avoid ambigous")
      (clang_tidy_file_iregex,           iregex,          "Set the source file filter for clang-format.")
      (clang_tidy_allow_no_checks,       boolean(false),  "Enabel clang-tidy allow_no_check option")
      (clang_tidy_enable_check_profile,  boolean(false),  "Enabel clang-tidy enable_check_profile option")
      (clang_tidy_database,              database,        "Same as clang-tidy -p option")
      (clang_tidy_checks,                str(),           "Same as clang-tidy checks option")
      (clang_tidy_config,                str(),           "Same as clang-tidy config option")
      (clang_tidy_config_file,           str(),           "Same as clang-tidy config-file option")
      (clang_tidy_header_filter,         str(),           "Same as clang-tidy header-filter option")
      (clang_tidy_line_filter,           str(),           "Same as clang-tidy line-filter option")
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
    if (variables.contains(clang_tidy_file_iregex)) {
      option.file_filter_iregex = variables[clang_tidy_file_iregex].as<std::string>();
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

  [[nodiscard]] auto creator::get_option() const -> const option_t & {
    return option;
  }

} // namespace lint::tool::clang_tidy
