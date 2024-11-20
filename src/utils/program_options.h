#pragma once

#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include <spdlog/spdlog.h>

#include "tools/clang_tidy.h"

namespace linter {

  namespace program_options = boost::program_options;

  /// https://docs.github.com/en/actions/writing-workflows/choosing-when-your-workflow-runs/events-that-trigger-workflows#pull_request
  struct user_options {
    bool use_on_local = false;
    std::string log_level;
    std::string repo_full_path;
    std::string owner_and_repo;
    std::string target_ref;
    std::string source_sha;
    std::string source_ref;
    std::string token;

    // Move these to clang_tidy::option
    bool enable_clang_tidy           = true;
    bool enable_clang_tidy_fast_exit = false;
    std::uint16_t clang_tidy_version = 0;
    clang_tidy::option clang_tidy_option{};
  };

  void print_full_options(const user_options &options);

  constexpr auto supported_log_level = {"TRACE", "DEBUG", "INFO", "ERROR"};

  auto parse_command_options(const program_options::variables_map &variables) -> user_options;

  auto make_program_options_desc() -> program_options::options_description;

  auto parse_program_options(int argc, char** argv, const program_options::options_description& desc)
    -> program_options::variables_map;
}  // namespace linter
