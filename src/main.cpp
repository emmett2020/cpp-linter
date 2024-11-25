#include <cctype>
#include <fstream>
#include <git2/oid.h>
#include <print>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/regex.hpp>

#include "github/api.h"
#include "github/common.h"
#include "tools/clang_tidy.h"
#include "utils/context.h"
#include "utils/env_manager.h"
#include "utils/git_utils.h"
#include "utils/program_options.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

namespace {
  /// This must be called before any spdlog use.
  void set_log_level(const std::string &log_level_str) {
    auto log_level = spdlog::level::info;
    if (log_level_str == "trace") {
      log_level = spdlog::level::trace;
    } else if (log_level_str == "debug") {
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
      spdlog::info("File index: {}, file path: {}", idx, file);
    }
  }

  bool file_needs_to_be_checked(const std::string &iregex, const std::string &file) {
    auto regex = boost::regex{iregex, boost::regex::icase};
    return boost::regex_match(file, regex);
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

  struct clang_tidy_all_files_result {
    std::uint32_t total   = 0;
    std::uint32_t ignored = 0;
    std::vector<clang_tidy::result> passed;
    std::vector<clang_tidy::result> failed;
  };

  void print_clang_tidy_total_result(const clang_tidy_all_files_result &result) {
    spdlog::info(
      "Total changed file number: {}. While {} files are igored by user, {} files checked by "
      "clang-tidy, {} files check is passed, {} files check is failed",
      result.total,
      result.ignored,
      result.total - result.ignored,
      result.passed.size(),
      result.failed.size());
  }

  auto make_step_summary(const context &ctx, const clang_tidy_all_files_result &result)
    -> std::string {
    const auto title = "# The cpp-linter Result"s;
    auto prefix      = std::string{};
    auto comment     = std::string{};
    if (result.failed.empty()) {
      prefix = ":rocket: All checks passed.";
    } else {
      prefix   = ":warning: Some files didn't pass the cpp-linter checks\n";
      comment += std::format("<details><summary>{} reports: <strong>",
                             ctx.clang_tidy_option.clang_tidy_binary);
      comment += std::format("{} fails</strong></summary>\n\n", result.failed.size());
      for (const auto &failed: result.failed) {
        for (const auto &diag: failed.diags) {
          auto one = std::format(
            "- **{}:{}:{}:** {}: [{}]\n  > {}\n",
            diag.header.file_name,
            diag.header.row_idx,
            diag.header.col_idx,
            diag.header.serverity,
            diag.header.diagnostic_type,
            diag.header.brief);
          comment += one;
        }
      }
      comment += "\n</details>";
    }
    return title + prefix + comment;
  }

  auto make_pull_request_review(const context &ctx, const clang_tidy_all_files_result &result)
    -> std::string {
    //
    //
  }

  void write_to_github_output([[maybe_unused]] const context &ctx,
                              const clang_tidy_all_files_result &clang_tidy_result) {
    auto output = env::get(github_output);
    auto file   = std::fstream{output, std::ios::app};
    throw_unless(file.is_open(), "error to open output file to write");
    file << std::format("total_failed={}\n", clang_tidy_result.failed.size());
    file << std::format("clang_tidy_failed_number={}\n", clang_tidy_result.failed.size());
    file << std::format("clang_format_failed_number={}\n", 0);
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
  check_and_fill_context_by_program_options(options, ctx);
  set_log_level(ctx.log_level);

  if (!ctx.use_on_local) {
    auto env = read_github_env();
    print_github_env(env);
    check_github_env(env);
    fill_context_by_env(env, ctx);
  }
  print_context(ctx);

  git::setup();
  auto repo          = git::repo::open(ctx.repo_path);
  auto changed_files = git::diff::changed_files(repo.get(), ctx.target, ctx.source);
  print_changed_files(changed_files);

  auto clang_tidy_result  = clang_tidy_all_files_result{};
  clang_tidy_result.total = changed_files.size();

  if (ctx.clang_tidy_option.enable_clang_tidy) {
    const auto &opt = ctx.clang_tidy_option;
    for (const auto &file: changed_files) {
      if (!file_needs_to_be_checked(opt.source_iregex, file)) {
        ++clang_tidy_result.ignored;
        spdlog::trace("file is ignored {}", file);
        continue;
      }
      auto result_per_file = clang_tidy::run(opt, ctx.repo_path, file);
      if (result_per_file.pass) {
        spdlog::info("file: {} passes {} check.", file, opt.clang_tidy_binary);
        clang_tidy_result.passed.emplace_back(std::move(result_per_file));
        continue;
      }
      clang_tidy_result.failed.emplace_back(std::move(result_per_file));
      spdlog::error("file: {} doesn't passes {} check.", file, opt.clang_tidy_binary);

      if (ctx.clang_tidy_option.enable_clang_tidy_fastly_exit) {
        spdlog::info("clang-tidy fastly exit");
        return -1;
      }
    }
    print_clang_tidy_total_result(clang_tidy_result);
  }

  if (ctx.enable_step_summary) {
    auto summary_file = env::get(github_step_summary);
    auto file         = std::fstream{summary_file, std::ios::app};
    throw_unless(file.is_open(), "error to open step summary file to write");
    auto step_summary = make_step_summary(ctx, clang_tidy_result);
    file << step_summary;
  }

  if (ctx.enable_update_issue_comment) {
    auto github_client = github_api_client{ctx};
    github_client.get_issue_comment_id();
    github_client.add_or_update_issue_comment(std::format(
      "{} passed, failed: {}",
      clang_tidy_result.passed.size(),
      clang_tidy_result.failed.size()));
  }

  if (ctx.enable_pull_request_review) {
  }

  // teardown
  git::shutdown();

  if (!ctx.use_on_local) {
    write_to_github_output(ctx, clang_tidy_result);
  }

  auto all_passes = true;
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    all_passes &= (clang_tidy_result.failed.empty());
  }
  return all_passes ? 0 : 1;
}
