#include "utils/shell.h"
#include "tools/clang_tidy.h"

#include <cctype>
#include <iostream>
#include <print>
#include <ranges>

#include <spdlog/spdlog.h>

using namespace linter;

int main() {
  // spdlog::set_level(spdlog::level::trace); // FOR DEBUG
  auto cmd                    = GetClangToolFullPath("clang-tidy", "20");
  auto [ec, std_out, std_err] = RunClangTidy(cmd, "/linter/build/t.cpp", "", false);
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
    std::println("{}", code);
  }
}
