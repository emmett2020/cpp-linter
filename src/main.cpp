#include <cctype>
#include <fstream>
#include <git2/oid.h>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>
#include <unordered_map>

#include "configs/version.h"
#include "github/api.h"
#include "github/common.h"
#include "utils/context.h"
#include "utils/env_manager.h"
#include "utils/git_utils.h"
#include "utils/program_options.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {

  // This function must be called before any spdlog operations.
  void set_log_level(std::string_view log_level_str) {
    static constexpr auto valid_log_levels = {"trace", "debug", "error"};
    assert(std::ranges::contains(valid_log_levels, log_level_str));

    auto log_level = spdlog::level::info;
    if (log_level_str == "trace") {
      log_level = spdlog::level::trace;
    } else if (log_level_str == "debug") {
      log_level = spdlog::level::debug;
    } else {
      log_level = spdlog::level::err;
    }
    spdlog::set_level(log_level);
  }

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Got {} changed files. File list:\n{}", files.size(), concat(files));
  }

  auto make_clang_tidy_result_str(const context &ctx, const total_result &result) -> std::string {
    auto details = std::string{};
    details += std::format("<details>\n<summary>{} reports:<strong>{} fails</strong></summary>\n",
                           ctx.clang_tidy_option.clang_tidy_binary,
                           result.clang_tidy_failed.size());
    for (const auto &[name, failed]: result.clang_tidy_failed) {
      for (const auto &diag: failed.diags) {
        auto one = std::format(
          "- **{}:{}:{}:** {}: [{}]\n  > {}\n",
          diag.header.file_name,
          diag.header.row_idx,
          diag.header.col_idx,
          diag.header.serverity,
          diag.header.diagnostic_type,
          diag.header.brief);
        details += one;
      }
    }
    return details;
  }

  auto make_brief_result(const context &ctx, const total_result &result) -> std::string {
    static const auto title     = "# The cpp-linter Result"s;
    static const auto hint_pass = ":rocket: All checks on all file passed."s;
    static const auto hint_fail = ":warning: Some files didn't pass the cpp-linter checks\n"s;

    auto clang_tidy_passed   = result.clang_tidy_failed.empty();
    auto clang_format_passed = result.clang_format_failed.empty();
    auto all_check_passes    = clang_tidy_passed && clang_format_passed;
    if (all_check_passes) {
      return title + hint_pass;
    }

    auto details = std::string{};
    if (!clang_format_passed) {
      details += make_clang_format_result_str(ctx, result);
    }
    if (!clang_tidy_passed) {
      details += make_clang_tidy_result_str(ctx, result);
    }

    return title + hint_fail + details;
  }

  auto make_clang_format_pr_review_comment(
    const context &ctx,
    const total_result &results,
    git::repo_raw_ptr repo,
    git::commit_raw_cptr commit) -> std::vector<pr_review_comment> {
    auto comments = std::vector<pr_review_comment>{};

    for (const auto &[file, result]: results.clang_format_failed) {
      auto old_buffer = git::blob::get_raw_content(repo, commit, file);
      auto opts       = git::diff_options{};
      git::diff::init_option(&opts);
      auto format_source_patch =
        git::patch::create_from_buffers(old_buffer, file, result.formatted_source_code, file, opts);
      spdlog::error(git::patch::to_str(format_source_patch.get()));

      const auto &patch = results.patches.at(file);

      auto format_num_hunk = git::patch::num_hunks(format_source_patch.get());
      for (int i = 0; i < format_num_hunk; ++i) {
        auto [format_hunk, format_num_lines] = git::patch::get_hunk(format_source_patch.get(), i);
        auto row                             = format_hunk.old_start;

        auto pos      = std::size_t{0};
        auto num_hunk = git::patch::num_hunks(patch.get());
        for (int j = 0; j < num_hunk; ++j) {
          auto [hunk, num_lines] = git::patch::get_hunk(patch.get(), j);
          if (!is_in_hunk(hunk, row)) {
            pos += num_lines;
            continue;
          }
          auto comment     = pr_review_comment{};
          comment.path     = file;
          comment.position = pos + row - hunk.new_start + 1;

          comment.body = git::patch::get_lines_in_hunk(patch.get(), i)
                       | std::views::join_with(' ')
                       | std::ranges::to<std::string>();
          comments.emplace_back(std::move(comment));
        }
      }
    }

    spdlog::error("Comments XXX:");
    for (const auto &comment: comments) {
      print_pr_review_comment(comment);
    }

    return comments;
  }

  void write_to_github_output([[maybe_unused]] const context &ctx, const total_result &result) {
    auto output = env::get(github_output);
    auto file   = std::fstream{output, std::ios::app};
    throw_unless(file.is_open(), "error to open output file to write");

    const auto clang_tidy_failed   = result.clang_tidy_failed.size();
    const auto clang_format_failed = result.clang_format_failed.size();
    const auto total_failed        = clang_tidy_failed + clang_format_failed;

    file << std::format("total_failed={}\n", total_failed);
    file << std::format("clang_tidy_failed_number={}\n", clang_tidy_failed);
    file << std::format("clang_format_failed_number={}\n", clang_format_failed);
  }

  auto convert_deltas_to_map(const std::vector<git::diff_delta_detail> &deltas)
    -> std::unordered_map<std::string, git::diff_delta_detail> {
    auto res = std::unordered_map<std::string, git::diff_delta_detail>{};
    for (const auto &delta: deltas) {
      res[delta.new_file.relative_path] = delta;
    }
    return res;
  }

} // namespace

auto main(int argc, char **argv) -> int {
  // Handle user inputs.
  auto desc    = make_program_options_desc();
  auto options = parse_program_options(argc, argv, desc);
  if (options.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (options.contains("version")) {
    std::print("{}.{}.{}",
               cpp_linter_VERSION_MAJOR,
               cpp_linter_VERSION_MINOR,
               cpp_linter_VERSION_PATCH);
    return 0;
  }

  auto ctx         = context{};
  ctx.use_on_local = env::get(github_actions) != "true";
  check_and_fill_context_by_program_options(options, ctx);
  set_log_level(ctx.log_level);

  // Get some additional informations when on Github environment.
  if (!ctx.use_on_local) {
    auto env = read_github_env();
    print_github_env(env);
    check_github_env(env);
    fill_context_by_env(env, ctx);
  }
  print_context(ctx);

  // Open user's git repository.
  git::setup();
  auto repo          = git::repo::open(ctx.repo_path);
  auto target_commit = git::convert<git::commit_ptr>(git::revparse::single(repo.get(), ctx.target));
  auto source_commit = git::convert<git::commit_ptr>(git::revparse::single(repo.get(), ctx.source));
  auto diff = git::diff::commit_to_commit(repo.get(), target_commit.get(), source_commit.get());

  auto linter_result    = total_result{};
  linter_result.patches = git::patch::create_from_diff(diff.get());
  auto changed_files    = git::patch::changed_files(linter_result.patches);
  print_changed_files(changed_files);

  if (ctx.clang_format_option.enable_clang_format) {
    apply_clang_format_on_files(ctx, changed_files, linter_result);
  }
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    apply_clang_tidy_on_files(ctx, changed_files, linter_result);
  }

  if (ctx.enable_step_summary) {
    auto summary_file = env::get(github_step_summary);
    auto file         = std::fstream{summary_file, std::ios::app};
    throw_unless(file.is_open(), "failed to open step summary file to write");
    file << make_brief_result(ctx, linter_result);
  }

  if (ctx.enable_comment_on_issue) {
    auto github_client = github_api_client{ctx};
    github_client.get_issue_comment_id();
    github_client.add_or_update_issue_comment(make_brief_result(ctx, linter_result));
  }

  if (ctx.enable_pull_request_review) {
    // TODO: merge
    auto comments = make_clang_tidy_pr_review_comment(ctx, linter_result);
    auto clang_format_comments =
      make_clang_format_pr_review_comment(ctx, linter_result, repo.get(), source_commit.get());
    comments.insert(comments.end(), clang_format_comments.begin(), clang_format_comments.end());
    auto body          = make_pr_review_comment_str(comments);
    auto github_client = github_api_client{ctx};
    github_client.post_pull_request_review(body);
  }

  if (!ctx.use_on_local) {
    write_to_github_output(ctx, linter_result);
  }

  git::shutdown();
  auto all_passes = true;
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    all_passes &= (linter_result.clang_tidy_failed.empty());
  }
  return all_passes ? 0 : 1;
}
