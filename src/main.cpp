#include <cctype>
#include <print>
#include <ranges>

#include <spdlog/spdlog.h>

#include "utils/shell.h"
#include "tools/clang_tidy.h"

using namespace linter; // NOLINT

int main() {
  // spdlog::set_level(spdlog::level::trace); // FOR DEBUG
  auto cmd                    = GetClangToolFullPath("clang-tidy", "20");
  auto [ec, std_out, std_err] = RunClangTidy(cmd, "/linter/build/t.cpp", "");
  auto [noti_lines, codes]    = ParseClangTidyOutput(std_out);
  for (const auto& [line, code]: std::ranges::views::zip(noti_lines, codes)) {
    std::println(
      "{}:{}:{}: {}: {} {}",
      line.file_name,
      line.row_idx,
      line.col_idx,
      line.serverity,
      line.brief,
      line.diagnostic);
    std::print("{}", code);
  }
}
