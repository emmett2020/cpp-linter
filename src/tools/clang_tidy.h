#pragma once

#include <string>
#include <string_view>

namespace linter {
  struct ClangTidyAdivce { };

  auto GetClangToolFullPath(std::string_view tool_name,
                            std::string_view version) -> std::string;

  auto GetRepoFullPath() -> std::string;

  auto RunClangTidy(
    std::string_view clang_tidy_cmd,
    std::string_view file_path,
    std::string_view data_base_path,
    bool only_check_changed_line) -> ClangTidyAdivce;


} // namespace linter
