#pragma once

#include <cassert>
#include <format>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


using CompileCommands = std::vector<std::unordered_map<std::string, std::string>>;

constexpr std::string_view clang_notes = "clang-diagnostic-";

// Creates a markdown link to the diagnostic documentation.
constexpr std::string_view diagnostic_link(std::string_view diagnostic) {
  //
  if (diagnostic.starts_with(clang_notes)) {
    return diagnostic;
  }

  auto link = std::format("[{}](https://clang.llvm.org/extra/clang-tidy/checks)", diagnostic);
  if (diagnostic.starts_with("clang-analyzer-")) {
    // auto check_name_parts = diagnostic.split("-");
    // return link + std::format("clang-analyzer/{}.html",
    // check_name_parts[2]);
    assert(false);
  }
}

struct ClangTidyNotification {
  ClangTidyNotification(const NotificationLine &notification_line,
                        std::optional<CompileCommands> compile_commands) {
    // TODO: strip rationale
    // TODO: strip serverity

    int line_number = std::stoi(notification_line.line);
    int col_number  = std::stoi(notification_line.col);
  }

  std::string filename;
  int row;
  int col;
  std::string serverity;
  std::string diagnostic;

  std::vector<std::string> fixit_lines;
  std::vector<std::string> applied_lines;

  std::string to_string() {
    return std::format("<ClangTidyNotification> {0}:{1}:{2} {3}", filename, row, col, diagnostic);
  }
};

struct ClangTidyAdvice {
  std::vector<ClangTidyNotification> notifications;

  explicit ClangTidyAdvice(const std::vector<ClangTidyNotification> &notifications) {
    //
  }

  // Get a markdown formatted list of fixed diagnostics found between a 'start'
  // and 'end' range of lines.
  std::string diagnostics_in_range(int start, int end) {
    //
    std::string diagnostics;
    for (const auto &notification: notifications) {
      for (const auto &fix_line: notification.applied_lines) {
        // if (fix_line > start && fix_line < end)  {
        //
        // }
      }
    }
    return diagnostics;
  }

  std::string get_suggestion_help(int start, int end) {
    auto diagnostics = diagnostics_in_range(start, end);
    auto prefix      = super().get_suggestion_help(start, end);
  }
};
