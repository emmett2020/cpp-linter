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
#include <spdlog/spdlog.h>

#include "context.h"

namespace linter {
  namespace program_options = boost::program_options;

  /// Create the options description of cpp-linter command line.
  auto create_program_options_desc() -> program_options::options_description;

  /// Parse the given options_description with user inputs.
  auto parse_program_options(
    int argc,
    char **argv,
    const program_options::options_description &desc) -> program_options::variables_map;

  /// Fill runtime context by program options.
  void fill_context_by_program_options(const program_options::variables_map &variables,
                                       runtime_context &ctx);


  /// Some options must be specified on the given condition, check it.
  void must_specify(const std::string &condition,
                    const program_options::variables_map &variables,
                    const std::initializer_list<const char *> &options);

  /// Some options mustn't be specified on the given condition, check it.
  void must_not_specify(const std::string &condition,
                        const program_options::variables_map &variables,
                        const std::initializer_list<const char *> &options);
} // namespace linter
