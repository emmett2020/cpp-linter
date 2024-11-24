#include "program_options.h"

#include <algorithm>

#include <boost/algorithm/string/case_conv.hpp>
#include <initializer_list>
#include <stdexcept>

#include "utils/shell.h"
#include "utils/util.h"
#include "github/common.h"

namespace linter {
  namespace program_options = boost::program_options;

  namespace {
    constexpr auto help                            = "help";
    constexpr auto version                         = "version";
    constexpr auto log_level                       = "log-level";
    constexpr auto repo_path                       = "repo-path";
    constexpr auto repo                            = "repo";
    constexpr auto token                           = "token";
    constexpr auto target                          = "target";
    constexpr auto source                          = "source";
    constexpr auto event_name                      = "event-name";
    constexpr auto pr_number                       = "pr-number";
    constexpr auto enable_step_summary             = "enable-step_summary";
    constexpr auto enable_update_issue_comment     = "enable-update-issue-comment";
    constexpr auto enable_pull_request_review      = "enable-pull-request-review";
    constexpr auto enable_clang_tidy               = "enable-clang-tidy";
    constexpr auto enable_clang_tidy_fastly_exit   = "enable-clang-tidy-fastly-exit";
    constexpr auto clang_tidy_version              = "clang-tidy-version";
    constexpr auto clang_tidy_binary               = "clang-tidy-binary";
    constexpr auto clang_tidy_allow_no_checks      = "clang-tidy-allow-no-checks";
    constexpr auto clang_tidy_enable_check_profile = "clang-tidy-enable-check-profile";
    constexpr auto clang_tidy_checks               = "clang-tidy-checks";
    constexpr auto clang_tidy_config               = "clang-tidy-config";
    constexpr auto clang_tidy_config_file          = "clang-tidy-config-file";
    constexpr auto clang_tidy_database             = "clang-tidy-database";
    constexpr auto clang_tidy_header_filter        = "clang-tidy-header-filter";
    constexpr auto clang_tidy_line_filter          = "clang-tidy-line-filter";
    constexpr auto clang_tidy_iregex               = "clang-tidy-iregex";

    // Find the full executable path of clang tools with specific version.
    auto find_clang_tool(std::string_view tool, std::uint16_t version) -> std::string {
      auto command                = std::format("{}-{}", tool, version);
      auto [ec, std_out, std_err] = shell::which(command);
      throw_unless(ec == 0,
                   std::format("find {}-{} failed, error message: {}", tool, version, std_err));
      auto trimmed = trim(std_out);
      throw_if(trimmed.empty(), "got empty clang tool path");
      return {trimmed.data(), trimmed.size()};
    }

    // Some options must be specified on the given condition, check it.
    void must_specify(const std::string &condition,
                      const program_options::variables_map &variables,
                      const std::initializer_list<const char *> &options) {
      std::ranges::for_each(options, [&](const auto *option) {
        throw_unless(variables.contains(option), [&] noexcept {
          return std::format("must specify {} when {}", option, condition);
        });
      });
    }

    // Some options mustn't be specified on the given condition, check it.
    void must_not_specify(const std::string &condition,
                          const program_options::variables_map &variables,
                          const std::initializer_list<const char *> &options) {
      std::ranges::for_each(options, [&](const auto *option) {
        throw_if(variables.contains(option), [&] noexcept {
          return std::format("must not specify {} when {}", option, condition);
        });
      });
    }

    // Theses options work both on local and CI.
    void check_and_fill_context_common(const program_options::variables_map &variables,
                                       context &ctx) {
      spdlog::trace("check_and_fill_context_common");
      if (variables.contains(log_level)) {
        ctx.log_level = variables[log_level].as<std::string>();
        boost::algorithm::to_lower(ctx.log_level);
        throw_unless(std::ranges::contains(supported_log_level, ctx.log_level),
                     "unsupported log level");
      }

      auto must_specify_option = {target};
      must_specify("use cpp-linter on local and CI", variables, must_specify_option);
      ctx.target = variables[target].as<std::string>();

      // clang-tidy options
      {
        auto &tidy_opt = ctx.clang_tidy_option;
        if (variables.contains(enable_clang_tidy)) {
          tidy_opt.enable_clang_tidy = variables[enable_clang_tidy].as<bool>();
        }
        if (variables.contains(enable_clang_tidy_fastly_exit)) {
          tidy_opt.enable_clang_tidy_fastly_exit =
            variables[enable_clang_tidy_fastly_exit].as<bool>();
        }
        if (variables.contains(clang_tidy_version)) {
          tidy_opt.clang_tidy_version = variables[clang_tidy_version].as<std::uint16_t>();
          throw_if(variables.contains(clang_tidy_binary),
                   "specify both clang-tidy-binary and clang-tidy-version will be ambiguous");
          tidy_opt.clang_tidy_binary =
            find_clang_tool("clang-tidy", ctx.clang_tidy_option.clang_tidy_version);
        }
        if (variables.contains(clang_tidy_binary)) {
          throw_if(variables.contains(clang_tidy_version),
                   "specify both clang-tidy-binary and clang-tidy-version will be ambiguous");
          tidy_opt.clang_tidy_binary = variables[clang_tidy_binary].as<std::string>();
          if (tidy_opt.enable_clang_tidy) {
            auto [ec, stdout, stderr] = shell::which(tidy_opt.clang_tidy_binary);
            throw_unless(
              ec == 0,
              std::format("can't find given clang_tidy_binary: {}", tidy_opt.clang_tidy_binary));
          }
        }
        spdlog::info("The clang-tidy executable path: {}", ctx.clang_tidy_option.clang_tidy_binary);
        if (variables.contains(clang_tidy_allow_no_checks)) {
          tidy_opt.allow_no_checks = variables[clang_tidy_allow_no_checks].as<bool>();
        }
        if (variables.contains(clang_tidy_enable_check_profile)) {
          tidy_opt.enable_check_profile = variables[clang_tidy_enable_check_profile].as<bool>();
        }
        if (variables.contains(clang_tidy_checks)) {
          tidy_opt.checks = variables[clang_tidy_checks].as<std::string>();
        }
        if (variables.contains(clang_tidy_config)) {
          tidy_opt.config = variables[clang_tidy_config].as<std::string>();
        }
        if (variables.contains(clang_tidy_config_file)) {
          tidy_opt.config_file = variables[clang_tidy_config_file].as<std::string>();
        }
        if (variables.contains(clang_tidy_database)) {
          tidy_opt.database = variables[clang_tidy_database].as<std::string>();
        }
        if (variables.contains(clang_tidy_header_filter)) {
          tidy_opt.header_filter = variables[clang_tidy_header_filter].as<std::string>();
        }
        if (variables.contains(clang_tidy_line_filter)) {
          tidy_opt.line_filter = variables[clang_tidy_line_filter].as<std::string>();
        }
        if (variables.contains(clang_tidy_iregex)) {
          tidy_opt.source_iregex = variables[clang_tidy_iregex].as<std::string>();
        }
      }
    }

    void check_and_fill_context_on_ci(const program_options::variables_map &variables,
                                      [[maybe_unused]] context &ctx) {
      spdlog::trace("check_and_fill_context_on_ci");
      auto must_not_specify_option = {repo_path, repo, source, event_name, pr_number};
      must_not_specify("use cpp-linter on CI", variables, must_not_specify_option);

      // Automatically enable step summary when on CI environment.
      ctx.enable_step_summary = true;
      if (variables.contains(enable_step_summary)) {
        ctx.enable_step_summary = variables[enable_step_summary].as<bool>();
      }

      if (variables.contains(enable_update_issue_comment)) {
        ctx.enable_update_issue_comment = variables[enable_update_issue_comment].as<bool>();
      }

      if (variables.contains(enable_pull_request_review)) {
        ctx.enable_pull_request_review = variables[enable_pull_request_review].as<bool>();
      }
    }

    void check_and_fill_context_on_local(const program_options::variables_map &variables,
                                         context &ctx) {
      spdlog::trace("check_and_fill_context_on_local");
      auto must_specify_option = {repo_path, source, event_name};
      must_specify("use cpp-linter on local", variables, must_specify_option);

      ctx.repo_path  = variables[repo_path].as<std::string>();
      ctx.source     = variables[source].as<std::string>();
      ctx.event_name = variables[event_name].as<std::string>();
      throw_unless(std::ranges::contains(all_github_events, ctx.event_name),
                   std::format("unsupported event name: {}", ctx.event_name));

      if (variables.contains(enable_update_issue_comment)
          || variables.contains(enable_pull_request_review)) {
        must_specify("use cpp-linter on local and enable interactive with GITHUB",
                     variables,
                     {token, repo});
        ctx.token = variables[token].as<std::string>();
        ctx.repo  = variables[repo].as<std::string>();
      }

      if (variables.contains(enable_update_issue_comment)) {
        ctx.enable_update_issue_comment = variables[enable_update_issue_comment].as<bool>();
      }

      if (variables.contains(enable_pull_request_review)) {
        ctx.enable_pull_request_review = variables[enable_pull_request_review].as<bool>();
      }

      if (variables.contains(pr_number)) {
        throw_unless(
          std::ranges::contains(github_events_with_pr_number, ctx.event_name),
          std::format("event: {} doesn't support pull-request-number option", ctx.event_name));
        ctx.pr_number = variables[pr_number].as<std::int32_t>();
      }
    }

  } // namespace

  auto make_program_options_desc() -> program_options::options_description {
    using namespace program_options; // NOLINT
    using std::string;
    using std::uint16_t;
    auto desc = options_description{"cpp-linter options:"};

    // clang-format off
  desc.add_options()
      (help,    "produce help message")
      (version, "print current version")
      (log_level,                       value<string>(),   "Set the log level of cpp-linter")
      (repo_path,                       value<string>(),   "Set the full path of git repository")
      (repo,                            value<string>(),   "Set the owner/repo of git repository")
      (token,                           value<string>(),   "Set github token of git repository")
      (target,                          value<string>(),   "Set the target reference/commit of git repository")
      (source,                          value<string>(),   "Set the source reference/commit of git repository.")
      (event_name,                      value<string>(),   "Set the event name of git repository. Such as: push, pull_request")
      (pr_number,                       value<int32_t>(),  "Set the pull-request number of git repository.")
      (enable_update_issue_comment,     value<bool>(),     "Enable update issue comment. This will set http request to github")
      (enable_pull_request_review,      value<bool>(),     "Enable pull request reivew. This will set http request to github")
      (enable_step_summary,             value<bool>(),     "Enable step summary.")
      (enable_clang_tidy,               value<bool>(),     "Enabel clang-tidy check")
      (enable_clang_tidy_fastly_exit,   value<bool>(),     "Enabel clang-tidy fastly exit."
                                                           "This means cpp-linter will stop all clang-tidy"
                                                           "checks as soon as possible when an error occurs")
      (clang_tidy_version,              value<uint16_t>(), "The version of clang-tidy to be used")
      (clang_tidy_binary,               value<string>(),   "The binary of clang-tidy to be used. You are't allowed to specify"
                                                           "both this option and clang-tidy-version to reduce ambiguous.")
      (clang_tidy_allow_no_checks,      value<bool>(),     "Enabel clang-tidy allow_no_check option")
      (clang_tidy_enable_check_profile, value<bool>(),     "Enabel clang-tidy enable_check_profile option")
      (clang_tidy_checks,               value<string>(),   "Same as clang-tidy checks option")
      (clang_tidy_config,               value<string>(),   "Same as clang-tidy config option")
      (clang_tidy_config_file,          value<string>(),   "Same as clang-tidy config_file option")
      (clang_tidy_database,             value<string>(),   "Same as clang-tidy -p option")
      (clang_tidy_header_filter,        value<string>(),   "Same as clang-tidy header_filter option")
      (clang_tidy_line_filter,          value<string>(),   "Same as clang-tidy line_filter option")
    ;
    // clang-format on
    return desc;
  }

  auto parse_program_options(
    int argc,
    char **argv,
    const program_options::options_description &desc) -> program_options::variables_map {
    auto variables = program_options::variables_map{};
    program_options::store(program_options::parse_command_line(argc, argv, desc), variables);
    program_options::notify(variables);
    return variables;
  }

  // This function will be called after check context. So there's no need to do same check.
  void check_and_fill_context_by_program_options(const program_options::variables_map &variables,
                                                 context &ctx) {
    spdlog::debug("Start to check program_options and fill context by it");

    check_and_fill_context_common(variables, ctx);
    if (ctx.use_on_local) {
      check_and_fill_context_on_local(variables, ctx);
    } else {
      check_and_fill_context_on_ci(variables, ctx);
    }
  }


} // namespace linter
