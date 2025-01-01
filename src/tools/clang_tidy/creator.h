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

#include <boost/program_options.hpp>

#include "tools/base_creator.h"
#include "tools/base_tool.h"
#include "tools/clang_tidy/general/option.h"

namespace lint::tool::clang_tidy {

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
  constexpr auto clang_tidy_iregex               = "clang-tidy-iregex";

  struct creator : public creator_base {
    void register_option(program_options::options_description &desc) const override;

    void create_option(const program_options::variables_map &variables);

    auto create_tool(const program_options::variables_map &variables) -> tool_base_ptr override;

    bool enabled() override;

    option_t option;
  };

} // namespace lint::tool::clang_tidy
