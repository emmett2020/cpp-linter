#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "utils/shell.h"

namespace linter {
  struct ClangTidyAdivce { };

  struct TidyHeaderLine {
    std::string file_name;
    std::string row_idx;
    std::string col_idx;
    std::string serverity;
    std::string brief;
    std::string diagnostic;
  };

  struct TidyOption {
    bool allow_no_checks      = false;
    bool enable_check_profile = false;
    std::string checks;
    std::string config;
    std::string config_file;
    std::string database;
    std::string header_filter;
    std::string line_filter;
  };

  struct TidyStatistic {
    std::size_t total_warnings            = 0;
    std::size_t non_user_code_warnings    = 0;
    std::size_t no_lint_warnings          = 0;
    std::size_t warnings_trated_as_errors = 0;
  };

  auto GetClangToolFullPath(std::string_view tool_name, std::string_view version) -> std::string;

  auto GetRepoFullPath() -> std::string;

  auto ParseClangTidyStdout(std::string_view output)
    -> std::tuple<std::vector<TidyHeaderLine>, std::vector<std::string>>;


  auto ParseClangTidyStderr(std::string_view std_err) -> TidyStatistic;

  auto RunClangTidy(std::string_view clang_tidy_cmd,
                    const TidyOption& option,
                    std::string_view file_path) -> shell::Result;


} // namespace linter
