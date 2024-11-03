#include "clang_tidy.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

// #include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include "utils/shell.h"
#include "utils/util.h"

namespace linter {

  enum class LineType {
    NOTIFICATION,
    DETAIL_CODE,
    FIX_SUGGESTION
  };

  constexpr auto kSupportedServerity = std::array<std::string_view, 2>{"warning", "info"};

  /// @brief: Only do some check.
  auto ParseClangTidyOutputCode(std::string_view line) -> std::string_view {
    spdlog::debug(std::format("Parsing detaile code: {}", line));
    // auto parts = line | std::views::split('|');
    // ThrowIf(std::distance(parts.begin(), parts.end()) >= 2,
    //         "The size of detailed code must be greater than 2.");
    return line;
  }

  /// @brief: Parse the output notification of clang-tidy.
  /// @return: If the format of output line meets notification rules, return it. Otherwise return std::nullopt.
  auto ParseClangTidyOutputNotification(std::string_view line) -> std::optional<NotificationLine> {
    spdlog::debug(std::format("Parsing notification: {}", line));
    auto parts = line | std::views::split(':');

    if (std::distance(parts.begin(), parts.end()) != 5) {
      return std::nullopt;
    }
    auto iter             = parts.begin();
    auto file_name        = std::string_view{*iter++};
    auto row_idx          = std::string_view{*iter++};
    auto col_idx          = std::string_view{*iter++};
    auto serverity        = TrimLeft(std::string_view{*iter++});
    auto brief_diagnostic = std::string_view{*iter++};

    if (!std::ranges::all_of(row_idx, ::isdigit)) {
      return std::nullopt;
    }
    if (!std::ranges::all_of(col_idx, ::isdigit)) {
      return std::nullopt;
    }
    if (!std::ranges::contains(kSupportedServerity, serverity)) {
      return std::nullopt;
    }

    auto square_brackets = std::ranges::find(brief_diagnostic, '[');
    if ((square_brackets == brief_diagnostic.end())
        || (brief_diagnostic.size() < 3)
        || (brief_diagnostic.back() != ']')) {
      return std::nullopt;
    }
    auto brief      = std::string_view{brief_diagnostic.begin(), square_brackets};
    auto diagnostic = std::string_view{square_brackets, brief_diagnostic.end()};

    auto notification       = NotificationLine{};
    notification.file_name  = file_name;
    notification.row_idx    = row_idx;
    notification.col_idx    = col_idx;
    notification.serverity  = serverity;
    notification.brief      = brief;
    notification.diagnostic = diagnostic;
    return notification;
  }

  auto GetLineType(std::string_view line) -> LineType {
    if (true) {
      return LineType::NOTIFICATION;
    }
  }

  auto ParseClangTidyOutput(std::string_view output)
    -> std::tuple<std::vector<NotificationLine>, std::vector<std::string>> {
    auto notifications = std::vector<NotificationLine>{};
    auto detail_codes  = std::vector<std::string>{};

    for (auto part: std::views::split(output, '\n')) {
      auto line  = std::string_view{part};
      auto state = GetLineType({line});

      auto noti_result = ParseClangTidyOutputNotification(line);
      if (noti_result) {
        notifications.push_back(noti_result.value());
        detail_codes.emplace_back();
        continue;
      }

      auto detail_code_result = ParseClangTidyOutputCode(line);
      if (!detail_code_result.empty()) {
        detail_codes.back() += line;
        detail_codes.back() += "\n";
      }
    }
    return std::make_tuple(notifications, detail_codes);
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
  auto GetClangToolFullPath(std::string_view tool_name, std::string_view version) -> std::string {
    auto command                = std::format("{}-{}", tool_name, version);
    auto [ec, std_out, std_err] = Which(command);
    if (ec != 0) {
      throw std::runtime_error{std_err};
    }
    return std_out;
  }

  // If find specified version, use it.
  auto AssembleVersionExec(std::string_view tool_name, std::string_view specified_version)
    -> std::string {
    auto exe_path = std::string{};
    auto semver   = std::views::split(tool_name, ".");

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
    return {};
  }

  auto RunClangTidy(
    std::string_view clang_tidy_cmd,
    std::string_view file_path,
    std::string_view data_base_path,
    bool only_check_changed_line) -> CommandResult {
    // Make additional args
    auto args = std::vector<std::string_view>{};
    if (!data_base_path.empty()) {
      args.push_back(std::format("-p {}", data_base_path));
    }
    args.push_back(file_path);

    auto arg_str = args | std::views::join_with(' ') | std::ranges::to<std::string>();
    spdlog::info("Running {} {}", clang_tidy_cmd, arg_str);
    return Execute(clang_tidy_cmd, args);
  }

  auto GetRepoFullPath() -> std::string {
    return "";
  }

} // namespace linter
