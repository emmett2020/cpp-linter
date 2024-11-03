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

  struct TidyArg {
    std::string database;
    std::string checks;
  };

  auto GetClangToolFullPath(std::string_view tool_name, std::string_view version) -> std::string;

  auto GetRepoFullPath() -> std::string;

  auto ParseClangTidyOutput(std::string_view output)
    -> std::tuple<std::vector<TidyHeaderLine>, std::vector<std::string>>;

  auto RunClangTidy(std::string_view clang_tidy_cmd, const TidyArg& arg, std::string_view file_path)
    -> shell::Result;


} // namespace linter
