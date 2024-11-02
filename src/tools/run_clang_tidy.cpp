#include <algorithm>
#include <cctype>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include "utils/shell.h"

namespace linter {

auto AssembleVersionExec(std::string_view tool_name,
                         std::string_view specified_version) -> std::string {
  auto exe_path = std::string{};
  auto semver = std::views::split(tool_name, ".");

  if (!semver.empty()) {
    auto is_digit = [](char c) { return std::isdigit(c); };
    if (std::ranges::all_of(semver.front(), is_digit)) {
      auto tool_name_with_version =
          std::format("{}-{}", tool_name, semver.front().data());
      if (auto [ec, std_out, _] = Which(tool_name_with_version); ec == 0) {
        return tool_name_with_version;
      }
    }
  }
}

struct ClangTidyAdivce {};

auto run_clang_tidy(std::string_view clang_tidy_cmd_path,
                    std::string_view file_path, std::string_view data_base_path,
                    bool only_check_changed_line) -> ClangTidyAdivce {
  auto cmd = std::string{clang_tidy_cmd_path};
  if (data_base_path != "") {
    cmd += std::format("-p {}", data_base_path);
  }
  cmd += file_path;

  //
}

void run_on_single_file(const std::string &file, int log_level,
                        std::string_view clang_tidy_cmd, int db_json,
                        std::optional<std::string> format_cmd) {

  //
  if (clang_tidy_cmd != "") {
  }

  //
}

} // namespace linter
