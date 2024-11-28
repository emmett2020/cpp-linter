#include <cctype>
#include <fstream>
#include <git2/oid.h>
#include <print>
#include <ranges>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/regex.hpp>
#include <unordered_map>

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

  auto print_changed_files(const std::vector< std::string> &files) {
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

  struct cpp_linter_result {
    // Added or modified or renamed, not included in deleted file.
    std::unordered_map<std::string, git::patch_ptr> patches;

    // Ignored by clang-tidy iregex.
    std::vector<std::string> clang_tidy_ignored_files;
    std::unordered_map<std::string, clang_tidy::result> clang_tidy_passed;
    std::unordered_map<std::string, clang_tidy::result> clang_tidy_failed;
  };

  void print_clang_tidy_total_result(const cpp_linter_result &result) {
    spdlog::info(
      "Total changed file number: {}. While {} files are igored by user, {} files check "
      "is passed, {} files check is failed",
      result.patches.size(),
      result.clang_tidy_ignored_files.size(),
      result.clang_tidy_passed.size(),
      result.clang_tidy_failed.size());
  }

  auto make_step_summary(const context &ctx, const cpp_linter_result &result) -> std::string {
    const auto title = "# The cpp-linter Result"s;
    auto prefix      = std::string{};
    auto comment     = std::string{};
    if (result.clang_tidy_failed.empty()) {
      prefix = ":rocket: All checks passed.";
    } else {
      prefix   = ":warning: Some files didn't pass the cpp-linter checks\n";
      comment += std::format("<details><summary>{} reports: <strong>",
                             ctx.clang_tidy_option.clang_tidy_binary);
      comment += std::format("{} fails</strong></summary>\n\n", result.clang_tidy_failed.size());
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
          comment += one;
        }
      }
      comment += "\n</details>";
    }
    return title + prefix + comment;
  }

  // Some changes in file may not in the same hunk.
  bool is_in_hunk(const git::diff_hunk &hunk, int row, [[maybe_unused]] int col) {
    return row >= hunk.new_start && row <= hunk.new_start + hunk.new_lines;
  }

  struct pr_review_comment {
    std::string path;
    std::size_t position;
    std::string body;
    std::size_t line;
    std::string side;
    std::size_t start_line;
    std::string start_side;
  };
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(pr_review_comment, path, position, body)

  auto make_pr_review_comment([[maybe_unused]] const context &ctx, const cpp_linter_result &results)
    -> std::vector<pr_review_comment> {
    auto comments = std::vector<pr_review_comment>{};

    for (const auto &[file, clang_tidy_result]: results.clang_tidy_failed) {
      // Get the same file's delta and clang-tidy result
      assert(clang_tidy_result.file == file);
      assert(results.patches.contains(file));
      const auto &patch = results.patches.at(file);

      for (const auto &diag: clang_tidy_result.diags) {
        const auto &header = diag.header;
        auto row           = std::stoi(header.row_idx);
        auto col           = std::stoi(header.col_idx);

        // For all clang-tidy result, check is this in hunk.
        auto pos      = std::size_t{0};
        auto num_hunk = git::patch::num_hunks(patch.get());
        for (int i = 0; i < num_hunk; ++i) {
          auto [hunk, num_lines] = git::patch::get_hunk(patch.get(), i);
          if (!is_in_hunk(hunk, row, col)) {
            pos += num_lines;
            continue;
          }
          auto comment     = pr_review_comment{};
          comment.path     = file;
          comment.position = pos + row - hunk.new_start + 1;
          comment.body =
            diag.details | std::views::join_with('\n') | std::ranges::to<std::string>();
          comments.emplace_back(std::move(comment));
        }
      }
    }
    return comments;
  }

  auto make_pr_review_comment_str(const std::vector<pr_review_comment> &comments) -> std::string {
    auto res        = nlohmann::json{};
    res["body"]     = "cpp-linter suggestion";
    res["event"]    = "COMMENT"; // TODO: DEBUG
    res["comments"] = comments;
    return res.dump();
  }

  void write_to_github_output([[maybe_unused]] const context &ctx,
                              const cpp_linter_result &result) {
    auto output = env::get(github_output);
    auto file   = std::fstream{output, std::ios::app};
    throw_unless(file.is_open(), "error to open output file to write");
    file << std::format("total_failed={}\n", result.clang_tidy_failed.size());
    file << std::format("clang_tidy_failed_number={}\n", result.clang_tidy_failed.size());
    file << std::format("clang_format_failed_number={}\n", 0);
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
    std::print("{}", get_current_version());
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

  auto linter_result    = cpp_linter_result{};
  linter_result.patches = git::patch::create_from_diff(diff.get());
  auto changed_files    = git::patch::changed_files(linter_result.patches);
  print_changed_files(changed_files);

  // Handle clang-tidy outputs.
  if (ctx.clang_tidy_option.enable_clang_tidy) {
    const auto &opt = ctx.clang_tidy_option;
    for (const auto &file: changed_files) {
      if (!file_needs_to_be_checked(opt.source_iregex, file)) {
        linter_result.clang_tidy_ignored_files.push_back(file);
        spdlog::trace("file is ignored {}", file);
        continue;
      }

      // Run clang-tidy then save result.
      auto result = clang_tidy::run(opt, ctx.repo_path, file);
      if (result.pass) {
        spdlog::info("file: {} passes {} check.", file, opt.clang_tidy_binary);
        linter_result.clang_tidy_passed[file] = std::move(result);
      } else {
        spdlog::error("file: {} doesn't passes {} check.", file, opt.clang_tidy_binary);
        linter_result.clang_tidy_failed[file] = std::move(result);

        if (ctx.clang_tidy_option.enable_clang_tidy_fastly_exit) {
          spdlog::info("clang-tidy fastly exit");
          return -1;
        }
      }
    }
    print_clang_tidy_total_result(linter_result);
  }

  if (ctx.enable_step_summary) {
    auto summary_file = env::get(github_step_summary);
    auto file         = std::fstream{summary_file, std::ios::app};
    throw_unless(file.is_open(), "error to open step summary file to write");
    auto step_summary = make_step_summary(ctx, linter_result);
    file << step_summary;
  }

  if (ctx.enable_update_issue_comment) {
    auto github_client = github_api_client{ctx};
    github_client.get_issue_comment_id();
    github_client.add_or_update_issue_comment(std::format(
      "{} passed, failed: {}",
      linter_result.clang_tidy_passed.size(),
      linter_result.clang_tidy_failed.size()));
  }

  if (ctx.enable_pull_request_review) {
    std::cout << "DEBUG-----\n";
    std::cout << git::diff::to_str(diff.get(), git::diff_format_t::GIT_DIFF_FORMAT_PATCH);
    std::cout << "DEBUG-----\n";
    auto comments      = make_pr_review_comment(ctx, linter_result);
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
