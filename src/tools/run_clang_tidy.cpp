#include <algorithm>
#include <cctype>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include "range/v3/range_fwd.hpp"
#include "range/v3/view/join.hpp"
#include "utils/shell.h"

namespace linter {

  void run_on_single_file(
    const std::string &file,
    int log_level,
    std::string_view clang_tidy_cmd,
    int db_json,
    std::optional<std::string> format_cmd) {

    //
    if (clang_tidy_cmd != "") {
    }

    //
  }

  auto AssembleVersionExec(std::string_view tool_name, std::string_view specified_version)
    -> std::string {
    auto exe_path = std::string{};
    auto semver = std::views::split(tool_name, ".");

    if (!semver.empty()) {
      auto is_digit = [](char c) {
        return std::isdigit(c);
      };
      if (std::ranges::all_of(semver.front(), is_digit)) {
        auto tool_name_with_version = std::format("{}-{}", tool_name, semver.front().data());
        if (auto [ec, std_out, _] = Which(tool_name_with_version); ec == 0) {
          return tool_name_with_version;
        }
      }
    }
  }

  struct ClangTidyAdivce { };

  auto RunClangTidy(
    std::string_view clang_tidy_cmd_path,
    std::string_view file_path,
    std::string_view data_base_path,
    bool only_check_changed_line) -> ClangTidyAdivce {

    // Make additional args
    auto args = std::vector<std::string_view>{};
    if (data_base_path != "") {
      args.push_back(std::format("-p {}", data_base_path));
    }
    args.push_back(file_path);

    auto arg_str = args //
                 | ranges::views::join(' ')
                 | ranges::to<std::string>();
    spdlog::info("Running {} {}", clang_tidy_cmd_path, arg_str);

    auto [ec, std_out, std_err] = Execute(clang_tidy_cmd_path, args);

    //
  }

} // namespace linter
