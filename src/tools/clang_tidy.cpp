#include "clang_tidy.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <spdlog/spdlog.h>

#include "utils/shell.h"
#include "utils/util.h"

namespace linter {
  namespace {
    enum class LineType : std::uint8_t {
      NOTIFICATION,
      DETAIL_CODE,
      FIX_SUGGESTION
    };

    using namespace std::string_view_literals;
    constexpr auto kSupportedServerity = {"warning"sv, "info"sv, "error"sv};

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
    auto ParseClangTidyOutputNotification(std::string_view line)
      -> std::optional<NotificationLine> {
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

      const auto *square_brackets = std::ranges::find(brief_diagnostic, '[');
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


  } // namespace

  auto ParseClangTidyOutput(std::string_view output)
    -> std::tuple<std::vector<NotificationLine>, std::vector<std::string>> {
    auto notifications = std::vector<NotificationLine>{};
    auto detail_codes  = std::vector<std::string>{};
    auto has_noti_line = false;

    for (auto part: std::views::split(output, '\n')) {
      auto line = std::string_view{part};

      auto noti_result = ParseClangTidyOutputNotification(line);
      if (noti_result) {
        notifications.push_back(noti_result.value());
        detail_codes.emplace_back("");
        has_noti_line = true;
        continue;
      }

      auto detail_code_result = ParseClangTidyOutputCode(line);
      if (has_noti_line && !detail_code_result.empty()) {
        detail_codes.back() += line;
        detail_codes.back() += "\n";
      }
    }
    return std::make_tuple(notifications, detail_codes);
  }

  /// @detail Get the full path of clang tools
  /// @param tool_name Could be "clang-tidy" or "clang-format"
  /// @param version A number.
  auto GetClangToolFullPath(std::string_view tool_name, std::string_view version) -> std::string {
    auto command                = std::format("{}-{}", tool_name, version);
    auto [ec, std_out, std_err] = Which(command);
    ThrowIf(ec != 0, std_err);
    return std_out;
  }

  auto RunClangTidy(std::string_view clang_tidy_cmd,
                    std::string_view file_path,
                    std::string_view data_base_path) -> CommandResult {
    auto args = std::vector<std::string_view>{};
    auto db   = data_base_path.empty() ? "" : std::format("-p {}", data_base_path);
    args.emplace_back(db);
    args.emplace_back(file_path);

    auto arg_str = args | std::views::join_with(' ') | std::ranges::to<std::string>();
    spdlog::info("Running {} {}", clang_tidy_cmd, arg_str);
    return Execute(clang_tidy_cmd, args);
  }

  auto GetRepoFullPath() -> std::string {
    return "";
  }

} // namespace linter
