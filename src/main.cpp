#include <cctype>
#include <print>
#include <ranges>

#include <spdlog/spdlog.h>
#include <git2/diff.h>
#include <string>

#include "tools/clang_tidy.h"
#include "github/api.h"
#include "utils/git_utils.h"
#include "utils/shell.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

namespace {
  void set_log_level(const std::string &log_level_env) {
    auto log_level = spdlog::level::info;
    if (log_level_env == "TRACE") {
      log_level = spdlog::level::trace;
    } else if (log_level_env == "DEBUG") {
      log_level = spdlog::level::debug;
    } else {
      log_level = spdlog::level::info;
    }
    spdlog::set_level(log_level);
  }

  struct user_options {
    // Common option.
    std::string log_level;

    // Git repository option.
    std::string repo_path;
    std::string target_ref;
    // https://docs.github.com/en/actions/writing-workflows/choosing-when-your-workflow-runs/events-that-trigger-workflows#pull_request
    std::string source_sha;
    std::string source_ref;

    bool enable_clang_tidy;
    std::string clang_tidy_version;
    bool clang_tidy_fast_exit;
    clang_tidy_option tidy_option;

    [[nodiscard]] auto to_str() const noexcept -> std::string {
      auto option  = std::string{};
      option      += std::format("Log level: {}\n", log_level);
      option      += std::format("Repository path: {}\n", repo_path);
      option      += std::format("Repository target ref: {}\n", target_ref);
      option      += std::format("Repository source ref: {}\n", source_ref);
      option      += std::format("Repository source sha: {}\n", source_sha);
      option      += std::format("clang tidy enabled: {}\n", enable_clang_tidy);
      option      += std::format("clang tidy version: {}\n", clang_tidy_version);
      option      += std::format("clang tidy fast exit: {}\n", clang_tidy_fast_exit);
      option      += tidy_option.to_str();
      return option;
    }
  };

  const auto linter_log_level = std::string{"LINTER_LOG_LEVEL"};

  /// WARN: Theses just for debug
  void set_pull_request_debug_env(user_options &param) {
    env::set_cache(kGithubRepository, "/temp/temp");
    env::set_cache(kGithubEventName, kGithubEventPullRequest);
    env::set_cache(kGithubSha, "");
    env::set_cache(kGithubRef, "refs/heads/test");

    param.log_level               = "DEBUG";
    param.clang_tidy_fast_exit    = false;
    param.clang_tidy_version      = "20";
    param.tidy_option.config_file = ".clang-tidy";
    param.tidy_option.database    = "build";
    param.target_ref              = "refs/heads/main";
  }

  auto load_user_options() -> user_options {
    auto param = user_options{};
    set_pull_request_debug_env(param);
    param.repo_path  = env::get(kGithubRepository);
    param.source_sha = env::get(kGithubSha);
    param.source_ref = env::get(kGithubRef);
    return param;
  }

  bool run_clang_tidy_once(const std::string &file, const user_options &options) {
    return true;
  }

  void setup() {
    git::setup();
  }

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Have {} changed files", files.size());
    for (const auto &file: files) {
      spdlog::debug(file);
    }
  }

} // namespace

int main() {
  setup();
  auto options = load_user_options();
  set_log_level(options.log_level);
  spdlog::debug("Linter Options:\n" + options.to_str());

  auto *repo         = git::repo::open(options.repo_path);
  auto changed_files = git::diff::changed_files(repo, options.target_ref, options.source_ref);
  print_changed_files(changed_files);

  // if (param.enable_clang_tidy) {
  //   auto clang_tool_exe = find_clang_tool_exe_path("clang-tidy", param.clang_tidy_version);
  //   throw_if(clang_tool_exe.empty(), "find clang tidy executable failed");
  // }

  // for (const auto &file: changed_files) {
  //   auto [ec, std_out, std_err] = run_clang_tidy(clang_tool_exe, param.tidy_option, file);
  //   auto [noti_lines, codes]    = parse_clang_tidy_stdout(std_out);
  //
  //   auto statistic = parse_clang_tidy_stderr(std_err);
  //   std::println("{} warnings generated.", statistic.total_warnings);
  //   std::println("{} errors generated.", statistic.total_errors);
  //   if (param.clang_tidy_fast_exit && statistic.total_errors > 0) {
  //     spdlog::info("fast exit");
  //     return -1;
  //   }
  //
  //   for (const auto &[line, code]: std::ranges::views::zip(noti_lines, codes)) {
  //     std::println(
  //       "{}:{}:{}: {}: {} {}",
  //       line.file_name,
  //       line.row_idx,
  //       line.col_idx,
  //       line.serverity,
  //       line.brief,
  //       line.diagnostic);
  //     std::print("{}", code);
  //   }
  // }

  auto teardown = [&]() {
    git::repo::free(repo);
    git::shutdown();
  };
  teardown();
}
