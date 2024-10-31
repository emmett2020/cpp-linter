#include <algorithm>
#include <cctype>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

auto AssembleVersionExec(std::string_view tool_name,
                         std::string_view specified_version) -> std::string {
  auto exe_path = std::string{};
  auto semver = std::views::split(tool_name, ".");

  if (!semver.empty()) {
    auto is_digit = [](char c) { return std::isdigit(c); };
    if (std::ranges::all_of(semver.front(), is_digit)) {
      //
    }
  }
}

void run_on_single_file(const std::string &file, int log_level,
                        std::optional<std::string> tidy_cmd, int db_json,
                        std::optional<std::string> format_cmd) {

  //
}
