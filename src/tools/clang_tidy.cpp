#include "clang_tidy.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include "utils/shell.h"

namespace linter {
  constexpr auto kNoteHeader = std::string_view{
    "^(.+):(\\d+):(\\d+):\\s(\\w+):"
    "(.*)\\[()\\]$"};
  constexpr auto kFixedNote = std::string_view{"1"};

  auto ParseClangTidyOutput(std::string_view output) -> ClangTidyAdivce {
    for (auto line: ranges::views::split(output, "\n")) {
    }
    return {};
  }

  void RunOnSingleFile(
    const std::string& file,
    int log_level,
    std::string_view clang_tidy_cmd,
    int db_json,
    std::optional<std::string> format_cmd) {
    //
    if (clang_tidy_cmd != "") {
    }

    //
  }

  /// @detail Get the full path of clang tool
  /// @param tool_name Could be clang-tidy or clang-format
  /// @param version A number.
  auto GetClangToolFullPath(std::string_view tool_name,
                            std::string_view version) -> std::string {
    auto command                = std::format("{}-{}", tool_name, version);
    auto [ec, std_out, std_err] = Which(command);
    if (ec != 0) {
      throw std::runtime_error{std_err};
    }
    return std_out;
  }

  // If find specified version, use it.
  auto AssembleVersionExec(std::string_view tool_name,
                           std::string_view specified_version) -> std::string {
    auto exe_path = std::string{};
    auto semver   = std::views::split(tool_name, ".");

    if (!semver.empty()) {
      auto is_digit = [](char c) {
        return std::isdigit(c);
      };
      if (std::ranges::all_of(semver.front(), is_digit)) {
        auto tool_name_with_version =
          std::format("{}-{}", tool_name, semver.front().data());
        if (auto [ec, std_out, _] = Which(tool_name_with_version); ec == 0) {
          return tool_name_with_version;
        }
      }
    }
    return {};
  }

  auto RunClangTidy(
    std::string_view clang_tidy_cmd,
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
    spdlog::info("Running {} {}", clang_tidy_cmd, arg_str);

    auto [ec, std_out, std_err] = Execute(clang_tidy_cmd, args);
    spdlog::info("Output from clang-tidy:\n{}", std_out);
    if (ec != 0) {
      spdlog::info("clang-tidy made the following summary:\n{}", std_err);
    }

    return {};
  }

  auto GetRepoFullPath() -> std::string {
    return "";
  }

} // namespace linter
