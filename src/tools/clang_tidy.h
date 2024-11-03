#pragma once

#include "utils/shell.h"
#include <string>
#include <string_view>

namespace linter {
  struct ClangTidyAdivce { };

  /// @detail
  struct NotificationLine {
    std::string file_name;
    std::string row_idx;
    std::string col_idx;
    std::string serverity;
    std::string brief;
    std::string diagnostic;
  };

  auto GetClangToolFullPath(std::string_view tool_name, std::string_view version) -> std::string;

  auto GetRepoFullPath() -> std::string;

  auto ParseClangTidyOutput(std::string_view output)
    -> std::tuple<std::vector<NotificationLine>, std::vector<std::string>>;

  auto RunClangTidy(
    std::string_view clang_tidy_cmd,
    std::string_view file_path,
    std::string_view data_base_path,
    bool only_check_changed_line) -> CommandResult;


} // namespace linter
