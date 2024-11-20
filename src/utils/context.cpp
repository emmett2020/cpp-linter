#include "context.h"

#include <spdlog/spdlog.h>

namespace linter {
  void print_context(const context &ctx) {
    spdlog::debug("Log level: {}", ctx.log_level);
    spdlog::debug("Repository path: {}", ctx.repo_path);
    spdlog::debug("Repository: {}", ctx.repo);
    spdlog::debug("Repository base ref: {}", ctx.base_ref);
    spdlog::debug("Repository head ref: {}", ctx.head_ref);
    spdlog::debug("Repository base commit: {}", ctx.base_commit);
    spdlog::debug("Repository head commit: {}", ctx.head_commit);

    const auto &tidy_opt = ctx.clang_tidy_option;
    spdlog::debug("The options of clang-tidy:");
    spdlog::debug("enable clang tidy: {}", tidy_opt.enable_clang_tidy);
    spdlog::debug("enable clang tidy fast exit: {}", tidy_opt.enable_clang_tidy_fastly_exit);
    spdlog::debug("allow no checks: {}", tidy_opt.allow_no_checks);
    spdlog::debug("enable check profile: {}", tidy_opt.enable_check_profile);
    spdlog::debug("clang tidy version: {}", tidy_opt.clang_tidy_version);
    spdlog::debug("checks: {}", tidy_opt.checks);
    spdlog::debug("config: {}", tidy_opt.config);
    spdlog::debug("config file: {}", tidy_opt.config_file);
    spdlog::debug("database: {}", tidy_opt.database);
    spdlog::debug("header filter: {}", tidy_opt.header_filter);
    spdlog::debug("line filter: {}", tidy_opt.line_filter);
  }


} // namespace linter
