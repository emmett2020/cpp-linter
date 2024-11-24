#include "context.h"

#include <spdlog/spdlog.h>

namespace linter {
  void print_context(const context &ctx) {
    spdlog::info("Context: ");
    spdlog::info("--------------------------------------------------");
    spdlog::info("Common Options:");
    spdlog::info("\tuse on local: {}", ctx.use_on_local);
    spdlog::info("\tlog level: {}", ctx.log_level);
    spdlog::info("\tenable step summary: {}", ctx.enable_step_summary);
    spdlog::info("\tenable update issue comment: {}", ctx.enable_update_issue_comment);
    spdlog::info("\tenable pull request review: {}", ctx.enable_pull_request_review);
    spdlog::info("Repository Options:");
    spdlog::info("\trepository path: {}", ctx.repo_path);
    spdlog::info("\trepository: {}", ctx.repo);
    spdlog::info("\trepository token: {}", "***");
    spdlog::info("\trepository event_name: {}", ctx.event_name);
    spdlog::info("\trepository target: {}", ctx.target);
    spdlog::info("\trepository source: {}", ctx.source);
    spdlog::info("\trepository pull-request number: {}", ctx.pr_number);

    const auto &tidy_opt = ctx.clang_tidy_option;
    spdlog::info("Options of clang-tidy:");
    spdlog::info("\tenable clang tidy: {}", tidy_opt.enable_clang_tidy);
    spdlog::info("\tenable clang tidy fastly exit: {}", tidy_opt.enable_clang_tidy_fastly_exit);
    spdlog::info("\tallow no checks: {}", tidy_opt.allow_no_checks);
    spdlog::info("\tenable check profile: {}", tidy_opt.enable_check_profile);
    spdlog::info("\tclang tidy version: {}", tidy_opt.clang_tidy_version);
    spdlog::info("\tclang tidy binary: {}", tidy_opt.clang_tidy_binary);
    spdlog::info("\tchecks: {}", tidy_opt.checks);
    spdlog::info("\tconfig: {}", tidy_opt.config);
    spdlog::info("\tconfig file: {}", tidy_opt.config_file);
    spdlog::info("\tdatabase: {}", tidy_opt.database);
    spdlog::info("\theader filter: {}", tidy_opt.header_filter);
    spdlog::info("\tline filter: {}", tidy_opt.line_filter);
    spdlog::info("\tsource file iregex: {}", tidy_opt.source_iregex);
    spdlog::info("");
  }

} // namespace linter
