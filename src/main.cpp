#include <cctype>
#include <git2/oid.h>
#include <print>
#include <string>

#include <spdlog/spdlog.h>

#include "github/api.h"
#include "tools/clang_tidy.h"
#include "utils/context.h"
#include "utils/env_manager.h"
#include "utils/git_utils.h"
#include "utils/program_options.h"
#include "utils/shell.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

namespace {
  /// Find the full executable path of clang tools with specific version.
  auto find_clang_tool(std::string_view tool, std::uint16_t version) -> std::string {
    auto command                = std::format("{}-{}", tool, version);
    auto [ec, std_out, std_err] = shell::which(command);
    throw_unless(ec == 0,
                 std::format("find {}-{} failed, error message: {}", tool, version, std_err));
    auto trimmed = trim(std_out);
    throw_if(trimmed.empty(), "got empty clang tool path");
    return {trimmed.data(), trimmed.size()};
  }

  /// This must be called before any spdlog use.
  void set_log_level(const std::string &log_level_str) {
    auto log_level = spdlog::level::info;
    if (log_level_str == "trace") {
      log_level = spdlog::level::trace;
    } else if (log_level_str == "debuf") {
      log_level = spdlog::level::debug;
    } else if (log_level_str == "error") {
      log_level = spdlog::level::err;
    } else {
      log_level = spdlog::level::info;
    }
    spdlog::set_level(log_level);
  }

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Got {} changed files", files.size());
    for (const auto &[idx, file]: std::views::enumerate(files)) {
      spdlog::info("Index: {}, file: {}", idx, file);
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

  // For a CI environment, the target reference may not downloaded to git
  // repository. We download and set a reference to it.
  // The target reference uses remote type. It must be exists.
  auto create_target_ref() {
  }

} // namespace

auto main(int argc, char **argv) -> int {
  auto desc    = make_program_options_desc();
  auto options = parse_program_options(argc, argv, desc);
  if (options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (options.contains("version")) {
    std::print("{}", get_current_version());
    return 0;
  }

  auto ctx         = context{};
  ctx.use_on_local = env::get(github_actions) != "true";
  spdlog::info("Use cpp-linter on local: {} ", ctx.use_on_local);

  check_program_options(ctx.use_on_local, options);
  fill_context_by_program_options(options, ctx);
  set_log_level(ctx.log_level);

  if (!ctx.use_on_local) {
    auto env = read_github_env();
    print_github_env(env);
    check_github_env(env);
    fill_context_by_env(env, ctx);
  }

  print_context(ctx);
  check_context(ctx);

  git::setup();
  auto repo          = git::repo::open(ctx.repo_path);
  auto changed_files = git::diff::changed_files(repo.get(), ctx.target, ctx.source);
  print_changed_files(changed_files);

  auto github_client = github_api_client{ctx};
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    auto clang_tidy = find_clang_tool("clang-tidy", ctx.clang_tidy_option.clang_tidy_version);
    spdlog::info("The clang-tidy executable path: {}", clang_tidy);

    for (const auto &file: changed_files) {
      auto result = clang_tidy::run(clang_tidy, ctx.clang_tidy_option, ctx.repo_path, file);
      if (result.pass) {
        spdlog::info("file: {} passed {} check.", file, clang_tidy);
        continue;
      }
      spdlog::error("file: {} doesn't pass {} check.", file, clang_tidy);
      if (ctx.enable_update_issue_comment) {
        github_client.get_issue_comment_id();
        github_client.add_or_update_comment(result.origin_stderr);
      }
      if (ctx.clang_tidy_option.enable_clang_tidy_fastly_exit) {
        spdlog::info("clang-tidy fastly exit");
        return -1;
      }
    }
  }

  git::shutdown();
}
