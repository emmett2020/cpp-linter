#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "utils/shell.h"

namespace linter {
  struct ClangTidyAdivce { };

  struct tidy_header_line {
    std::string file_name;
    std::string row_idx;
    std::string col_idx;
    std::string serverity;
    std::string brief;
    std::string diagnostic;
  };

  struct clang_tidy_option {
    bool allow_no_checks      = false;
    bool enable_check_profile = false;
    std::string checks;
    std::string config;
    std::string config_file;
    std::string database;
    std::string header_filter;
    std::string line_filter;

    [[nodiscard]] auto to_str() const noexcept -> std::string {
      auto option = std::string{};
      option.reserve(512);
      option += std::format("Checks: {}\n", checks);
      option += std::format("Config: {}\n", config);
      option += std::format("Config file: {}\n", config_file);
      option += std::format("Database: {}\n", database);
      option += std::format("Header filter: {}\n", header_filter);
      option += std::format("Line filter: {}\n", line_filter);
      option += std::format("Allow no checks: {}\n", allow_no_checks);
      option += std::format("Enable check profile: {}\n", enable_check_profile);
      return option;
    }
  };

  struct tidy_statistic {
    std::uint32_t total_warnings            = 0;
    std::uint32_t total_errors              = 0;
    std::uint32_t non_user_code_warnings    = 0;
    std::uint32_t no_lint_warnings          = 0;
    std::uint32_t warnings_trated_as_errors = 0;

    [[nodiscard]] auto to_str() const noexcept -> std::string {
      auto statistics  = std::string{};
      statistics      += std::format("Total warnings: {}\n", total_warnings);
      statistics      += std::format("Total errors: {}\n", total_warnings);
      statistics      += std::format("Non user code warnings: {}\n", non_user_code_warnings);
      statistics      += std::format("No lint warnings: {}\n", no_lint_warnings);
      statistics      += std::format("Warnings trated as errors: {}\n", warnings_trated_as_errors);
      return statistics;
    }
  };

  auto find_clang_tool_exe_path(std::string_view tool_name, std::string_view version)
    -> std::string;

  auto get_repo_full_path() -> std::string;

  auto parse_clang_tidy_stdout(std::string_view output)
    -> std::tuple<std::vector<tidy_header_line>, std::vector<std::string>>;


  auto parse_clang_tidy_stderr(std::string_view std_err) -> tidy_statistic;

  auto run_clang_tidy(
    std::string_view clang_tidy_cmd,
    const clang_tidy_option& option,
    std::string_view repo_path,
    std::string_view file_path) -> shell::result;


} // namespace linter
