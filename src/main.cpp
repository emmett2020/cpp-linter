#include <cctype>
#include <print>
#include <ranges>

#include <spdlog/spdlog.h>

#include "github/api.h"
#include "tools/clang_tidy.h"
#include "utils/git_utils.h"
#include "utils/shell.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

void ForClangTidy() {
  int *hi = 0;
}

int main() {
  // spdlog::set_level(spdlog::level::trace); // FOR DEBUG
  // auto cmd    = GetClangToolFullPath("clang-tidy", "20");
  // auto option = TidyOption{
  //   .config_file = ".clang-tidy",
  //   .database    = "build",
  // };
  //
  // auto [ec, std_out, std_err] = RunClangTidy(cmd, option, "src/main.cpp");
  // auto [noti_lines, codes]    = ParseClangTidyStdout(std_out);
  //
  // std::println("ec: {}", ec);
  // // std::println("std_err\n------------\n{}------------\n", std_err);
  //
  // auto statistic = ParseClangTidyStderr(std_err);
  // std::println("{} warnings generated.", statistic.total_warnings);
  //
  // for (const auto& [line, code]: std::ranges::views::zip(noti_lines, codes))
  // {
  //   std::println(
  //     "{}:{}:{}: {}: {} {}",
  //     line.file_name,
  //     line.row_idx,
  //     line.col_idx,
  //     line.serverity,
  //     line.brief,
  //     line.diagnostic);
  //   std::print("{}", code);
  // }

  // ParseDiff();
  // env::SetCache(kGithubEventName, kGithubEventPush);
  // env::SetCache(kGithubToken, "");
  // env::SetCache(kGithubRepository, "emmett2020/temp");
  // // env::SetCache(kGithubEventPath, "----");
  // env::SetCache(kGithubSha, "b083715");
  // auto api_client = GithubApiClient{};
  // auto t          = api_client.GetChangedFiles();

  git::setup();
  auto *repo   = git::repo::open("/temp/temp");
  auto state   = git::repo::state(repo);
  auto path    = git::repo::path(repo);
  auto empty   = git::repo::is_empty(repo);
  auto *config = git::repo::config(repo);

  std::print("{}, {}, {}", state, path, empty);
  git::repo::free(repo);
  git::config::free(config);
  git::shutdown();
}
