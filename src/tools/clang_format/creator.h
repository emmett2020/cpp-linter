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
#include "tools/clang_format/base_impl.h"
#include "tools/clang_format/version_18.h"
#include "tools/util.h"
#include "utils/util.h"

namespace linter::tool::clang_format {
constexpr auto supported_version = {version_18_1_0, version_18_1_3};

constexpr auto enable_clang_format = "enable-clang-format";
constexpr auto enable_clang_format_fastly_exit =
    "enable-clang-format-fastly-exit";
constexpr auto clang_format_version = "clang-format-version";
constexpr auto clang_format_binary = "clang-format-binary";
constexpr auto clang_format_iregex = "clang-format-iregex";

struct creator : public creator_base<user_option, per_file_result> {
  void
  register_option_desc(program_options::options_description &desc) override {
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

  auto create_option(const program_options::variables_map &variables)
      -> user_option override {
    auto opt = user_option{};
    if (variables.contains(enable_clang_format)) {
      opt.enabled = variables[enable_clang_format].as<bool>();
    }
    if (variables.contains(enable_clang_format_fastly_exit)) {
      opt.enabled_fastly_exit =
          variables[enable_clang_format_fastly_exit].as<bool>();
    }
    if (variables.contains(clang_format_version)) {
      opt.version = variables[clang_format_version].as<std::string>();
      throw_if(variables.contains(clang_format_binary),
               "specify both clang-format-binary and clang-format-version is "
               "ambiguous");
      opt.binary = find_clang_tool("clang-format", opt.version);
    }
    if (variables.contains(clang_format_binary)) {
      throw_if(variables.contains(clang_format_version),
               "specify both clang-format-binary and clang-format-version is "
               "ambiguous");
      opt.binary = variables[clang_format_binary].as<std::string>();
      if (opt.enabled) {
        auto [ec, stdout, stderr] = shell::which(opt.binary);
        throw_unless(ec == 0,
                     std::format("can't find given clang_format_binary: {}",
                                 opt.binary));
      }
    }
    spdlog::info("The clang-format executable path: {}", opt.binary);
    return opt;
  }

  auto
  create_instance(operating_system_t cur_system, arch_t cur_arch,
                  const std::string &version) -> clang_format_ptr override {
    throw_if(std::ranges::contains(supported_version, version),
             "Create clang-format instance failed since unsupported version.");

    auto tool = clang_format_ptr{};
    if (version == version_18_1_3) {
      tool = std::make_unique<clang_format_v18_1_3>();
    } else if (version == version_18_1_0) {
      tool = std::make_unique<clang_format_v18_1_0>();
    } else {
      std::unreachable();
    }

    throw_unless(tool->is_supported(cur_system, cur_arch),
                 std::format("Create clang-format {} instance failed since not "
                             "supported on this platform",
                             version));
    return tool;
  }

  auto create_reporter()
      -> reporter_base<user_option, final_result<per_file_result>> override {
    return {};
  }
};

} // namespace linter::tool::clang_format
