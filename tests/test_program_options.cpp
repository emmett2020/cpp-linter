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

#include "program_options.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace linter;

TEST_CASE("Test create program options descriptions",
          "[cpp-linter][program_options][create]") {
  int argc = 2;

  char cpp_name[] = "./cpp-linter";
  char help_opt[] = "--help";
  char *argv[] = {cpp_name, help_opt};

  auto desc = create_program_options_desc();
  auto user_options = parse_program_options(argc, argv, desc);
  REQUIRE(user_options.contains("help"));
}

TEST_CASE("Test help meesage",
          "[cpp-linter][program_options][basic_options][help]") {
  auto desc = create_program_options_desc();
}
