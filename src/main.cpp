#include <algorithm>
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
  /// Find the full executable path of clang tools with specific version.
  auto find_clang_tool_exe_path(std::string_view tool, std::string_view version) -> std::string {
    auto command                = std::format("{}-{}", tool, version);
    auto [ec, std_out, std_err] = shell::which(command);
    throw_if(
      ec != 0,
      std::format("execute /usr/bin/which to find {}-{} failed, got: {}", tool, version, std_err));
    return std_out;
  }

  /// This must be called before any spdlog use.
  void set_log_level(const std::string &log_level_str) {
    auto log_level = spdlog::level::info;
    if (log_level_str == "TRACE") {
      log_level = spdlog::level::trace;
    } else if (log_level_str == "DEBUG") {
      log_level = spdlog::level::debug;
    } else {
      log_level = spdlog::level::info;
    }
    spdlog::set_level(log_level);
  }

  /// https://docs.github.com/en/actions/writing-workflows/choosing-when-your-workflow-runs/events-that-trigger-workflows#pull_request
  struct user_options {
    bool use_on_local = false;
    std::string log_level;
    std::string repo_path; // the path to repo, without owner
    std::string repo;      // owner/repo
    std::string target_ref;
    std::string source_sha;
    std::string source_ref;

    bool enable_clang_tidy;
    bool clang_tidy_fast_exit;
    std::string clang_tidy_version;
    clang_tidy::option clang_tidy_option;
  };

  void print_full_options(const user_options &options) {
    spdlog::debug("Log level: {}", options.log_level);
    spdlog::debug("Repository path: {}", options.repo_path);
    spdlog::debug("Repository: {}", options.repo);
    spdlog::debug("Repository full path: {}/{}", options.repo_path, options.repo);
    spdlog::debug("Repository target ref: {}", options.target_ref);
    spdlog::debug("Repository source ref: {}", options.source_ref);
    spdlog::debug("Repository source sha: {}", options.source_sha);

    const auto &tidy_option = options.clang_tidy_option;
    spdlog::debug("The options of clang-tidy:");
    spdlog::debug("clang tidy enabled: {}", options.enable_clang_tidy);
    spdlog::debug("clang tidy version: {}", options.clang_tidy_version);
    spdlog::debug("clang tidy fast exit: {}", options.clang_tidy_fast_exit);
    spdlog::debug("checks: {}", tidy_option.checks);
    spdlog::debug("config: {}", tidy_option.config);
    spdlog::debug("config file: {}", tidy_option.config_file);
    spdlog::debug("database: {}", tidy_option.database);
    spdlog::debug("header filter: {}", tidy_option.header_filter);
    spdlog::debug("line filter: {}", tidy_option.line_filter);
    spdlog::debug("allow no checks: {}", tidy_option.allow_no_checks);
    spdlog::debug("enable check profile: {}", tidy_option.enable_check_profile);
  }

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Got {} changed files", files.size());
    for (const auto &file: files) {
      spdlog::debug(file);
    }
  }

  /// WARN: Theses just for debug
  void set_pull_request_debug_env(user_options &options) {
    env::set_cache(github_repository, "emmett2020/temp");
    env::set_cache(github_event_name, github_event_pull_request);
    env::set_cache(github_sha, "");
    env::set_cache(github_ref, "refs/pull/7/merge");
    env::set_cache(github_token, "");

    options.log_level                     = "TRACE";
    options.enable_clang_tidy             = false;
    options.clang_tidy_fast_exit          = false;
    options.clang_tidy_version            = "20";
    options.clang_tidy_option.config_file = ".clang-tidy";
    options.clang_tidy_option.database    = "build";
    options.target_ref                    = "refs/heads/main";
  }

  /// Load user options from github.
  auto load_user_options() -> user_options {
    auto param = user_options{};
    set_pull_request_debug_env(param);
    param.repo       = env::get(github_repository);
    param.source_sha = env::get(github_sha);
    param.source_ref = env::get(github_ref);
    return param;
  }


} // namespace

int main() {
  auto options = load_user_options();
  set_log_level(options.log_level);
  print_full_options(options);
  const auto repo_full_path = options.repo_path + "/" + options.repo;

  git::setup();
  auto *repo = git::repo::open(repo_full_path);
  // auto changed_files = git::diff::changed_files(repo, options.target_ref, options.source_ref);
  // print_changed_files(changed_files);

  auto github_client = github_api_client{};
  github_client.get_issue_comment_id();

  // if (options.enable_clang_tidy) {
  //   auto clang_tidy_exe = find_clang_tool_exe_path("clang-tidy", options.clang_tidy_version);
  //   spdlog::info("The clang-tidy executable path: {}", clang_tidy_exe);
  //   throw_if(clang_tidy_exe.empty(), "find clang tidy executable failed");
  //
  //   for (const auto &file: changed_files) {
  //     auto result =
  //       clang_tidy::run(clang_tidy_exe, options.clang_tidy_option, repo_full_path, file);
  //     if (!result.pass && options.clang_tidy_fast_exit) {
  //       spdlog::info("fast exit");
  //       return -1;
  //     }
  //   }
  // }

  git::repo::free(repo);
  git::shutdown();
}
