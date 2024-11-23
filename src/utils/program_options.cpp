#include "program_options.h"

#include <algorithm>

#include <boost/algorithm/string/case_conv.hpp>

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
    constexpr auto enable_clang_tidy               = "enable-clang-tidy";
    constexpr auto enable_clang_tidy_fastly_exit   = "enable-clang-tidy-fastly-exit";
    constexpr auto clang_tidy_version              = "clang-tidy-version";
    constexpr auto clang_tidy_allow_no_checks      = "clang-tidy-allow-no-checks";
    constexpr auto clang_tidy_enable_check_profile = "clang-tidy-enable-check-profile";
    constexpr auto clang_tidy_checks               = "clang-tidy-checks";
    constexpr auto clang_tidy_config               = "clang-tidy-config";
    constexpr auto clang_tidy_config_file          = "clang-tidy-config-file";
    constexpr auto clang_tidy_database             = "clang-tidy-database";
    constexpr auto clang_tidy_header_filter        = "clang-tidy-header-filter";
    constexpr auto clang_tidy_line_filter          = "clang-tidy-line-filter";
  } // namespace

  void fill_context_by_program_options(const program_options::variables_map &variables,
                                       context &ctx) {
    if (variables.contains(log_level)) {
      ctx.log_level = variables[log_level].as<std::string>();
      boost::algorithm::to_lower(ctx.log_level);
      throw_unless(std::ranges::contains(supported_log_level, ctx.log_level),
                   "unsupported log level");
    }

    if (variables.contains(repo_path)) {
      ctx.repo_path = variables[repo_path].as<std::string>();
    }
    if (variables.contains(repo)) {
      ctx.repo = variables[repo].as<std::string>();
    }
    if (variables.contains(token)) {
      ctx.token = variables[token].as<std::string>();
    }
    if (variables.contains(target)) {
      ctx.target = variables[target].as<std::string>();
    }
    if (variables.contains(source)) {
      ctx.source = variables[source].as<std::string>();
    }
    if (variables.contains(event_name)) {
      ctx.event_name = variables[event_name].as<std::string>();
    }
    if (variables.contains(pr_number)) {
      ctx.pr_number = variables[pr_number].as<int32_t>();
    }
    if (variables.contains(enable_clang_tidy)) {
      ctx.clang_tidy_option.enable_clang_tidy = variables[enable_clang_tidy].as<bool>();
    }
    if (variables.contains(enable_clang_tidy_fastly_exit)) {
      ctx.clang_tidy_option.enable_clang_tidy_fastly_exit =
        variables[enable_clang_tidy_fastly_exit].as<bool>();
    }
    if (variables.contains(clang_tidy_version)) {
      ctx.clang_tidy_option.clang_tidy_version = variables[clang_tidy_version].as<std::uint16_t>();
    }
    if (variables.contains(clang_tidy_allow_no_checks)) {
      ctx.clang_tidy_option.allow_no_checks = variables[clang_tidy_allow_no_checks].as<bool>();
    }
    if (variables.contains(clang_tidy_enable_check_profile)) {
      ctx.clang_tidy_option.enable_check_profile =
        variables[clang_tidy_enable_check_profile].as<bool>();
    }
    if (variables.contains(clang_tidy_checks)) {
      ctx.clang_tidy_option.checks = variables[clang_tidy_checks].as<std::string>();
    }
    if (variables.contains(clang_tidy_config)) {
      ctx.clang_tidy_option.config = variables[clang_tidy_config].as<std::string>();
    }
    if (variables.contains(clang_tidy_config_file)) {
      ctx.clang_tidy_option.config_file = variables[clang_tidy_config_file].as<std::string>();
    }
    if (variables.contains(clang_tidy_database)) {
      ctx.clang_tidy_option.database = variables[clang_tidy_database].as<std::string>();
    }
    if (variables.contains(clang_tidy_header_filter)) {
      ctx.clang_tidy_option.header_filter = variables[clang_tidy_header_filter].as<std::string>();
    }
    if (variables.contains(clang_tidy_line_filter)) {
      ctx.clang_tidy_option.line_filter = variables[clang_tidy_line_filter].as<std::string>();
    }
  }

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
      (enable_clang_tidy,               value<bool>(),     "Enabel clang-tidy check")
      (enable_clang_tidy_fastly_exit,   value<bool>(),     "Enabel clang-tidy fastly exit."
                                                           "This means cpp-linter will stop all clang-tidy"
                                                           "checks as soon as possible when an error occurs")
      (clang_tidy_version,              value<uint16_t>(), "The version of clang-tidy to be used")
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

  void check_program_options(bool use_on_local,
                             const boost::program_options::variables_map &variables) {
    if (use_on_local) {
      throw_unless(variables.contains(repo_path),
                   "must specify repo-path when use cpp-linter on local");
      throw_unless(variables.contains(event_name),
                   "must specify repo when use cpp-linter on local");
      auto event = variables[event_name].as<std::string>();
      throw_unless(std::ranges::contains(all_github_events, event),
                   std::format("unsupported event name: {}", event));
      throw_unless(variables.contains(target), "must specify target when use cpp-linter on local");
      throw_unless(variables.contains(source), "must specify source when use cpp-linter on local");
    }

    if (variables.contains(pr_number)) {
      throw_unless(use_on_local, "pr-number option only supports use on local");
      auto event = variables[event_name].as<std::string>();
      throw_unless(std::ranges::contains(github_events_support_pr_number, event),
                   std::format("event: {} doesn't support pr-number option", event));
    }
  }
} // namespace linter
