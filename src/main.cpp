#include <algorithm>
#include <cctype>
#include <filesystem>
#include <print>
#include <ranges>
#include <string>

#include <spdlog/spdlog.h>
#include <git2/diff.h>

#include "spdlog/common.h"
#include "tools/clang_tidy.h"
#include "github/api.h"
#include "utils/git_utils.h"
#include "utils/shell.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

namespace {
  /// Find the full executable path of clang tools with specific version.
  auto find_clang_tool_exe_path(std::string_view tool, std::uint16_t version) -> std::string {
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
    } else if (log_level_str == "ERROR") {
      log_level = spdlog::level::err;
    } else {
      log_level = spdlog::level::info;
    }
    spdlog::set_level(log_level);
  }

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Got {} changed files", files.size());
    for (const auto &file: files) {
      spdlog::debug(file);
    }
  }

  auto get_current_version() -> std::string {
    auto file = std::ifstream{"VERSION"};
    throw_if(file.bad(), "Open VERSION file to read failed");
    auto buffer = std::string{};
    std::getline(file, buffer);
    auto trimmed_version = linter::trim(buffer);
    throw_if(buffer.empty(), "VERSION file is empty");
    return {trimmed_version.data(), trimmed_version.size()};
  }

} // namespace

auto main(int argc, char **argv) -> int {
  auto desc = make_program_options_desc();
  auto options = parse_program_options(argc,  argv, desc);
  if (options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (options.contains("version")) {
    std::print("{}", get_current_version());
    return 0;
  }

  auto ctx = create_context_by_program_options(options);
  set_log_level(ctx.log_level);
  print_context(ctx);

  if (! ctx.use_on_local) {
    auto env = read_github_env();
    print_github_env(env);
    merge_env_into_context(env, ctx);
  }

  git::setup();
  auto *repo = git::repo::open(ctx.repo_path);
  auto changed_files = git::diff::changed_files(repo, ctx.base_commit, ctx.head_commit);
  print_changed_files(changed_files);

  auto github_client = github_api_client{};

  if (ctx.clang_tidy_option.enable_clang_tidy) {
    auto clang_tidy_exe = find_clang_tool_exe_path("clang-tidy", ctx.clang_tidy_option.clang_tidy_version);
    spdlog::info("The clang-tidy executable path: {}", clang_tidy_exe);
    throw_if(clang_tidy_exe.empty(), "find clang tidy executable failed");

    for (const auto &file: changed_files) {
      auto result = clang_tidy::run(clang_tidy_exe, ctx.clang_tidy_option, ctx.repo_path, file);
      if (!result.pass && ctx.clang_tidy_option.enable_clang_tidy_fastly_exit) {
        spdlog::info("fast exit");
        return -1;
      }
    }
  }

  git::repo::free(repo);
  git::shutdown();
}
