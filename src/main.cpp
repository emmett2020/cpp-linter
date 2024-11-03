#include <cctype>
#include <print>
#include <ranges>

#include <spdlog/spdlog.h>

#include "utils/shell.h"
#include "tools/clang_tidy.h"

using namespace linter; // NOLINT

void ForClangTidy() {
  int* hi = 0;
}

int main() {
  // spdlog::set_level(spdlog::level::trace); // FOR DEBUG
  auto cmd    = GetClangToolFullPath("clang-tidy", "20");
  auto option = TidyOption{
    .allow_no_checks      = true,
    .enable_check_profile = true,
    // .checks               = "-*",
    // .config      = "{Checks: '*', CheckOptions: {x: y}}",
    .config_file = ".clang-tidy",
    .database    = "build",
  };

  auto [ec, std_out, std_err] = RunClangTidy(cmd, option, "src/main.cpp");
  auto [noti_lines, codes]    = ParseClangTidyOutput(std_out);

  std::println("ec: {}", ec);
  std::println("{}", std_err);
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
