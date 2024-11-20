#include "program_options.h"

#include "utils/env_manager.h"
#include "github/api.h"
#include "utils/util.h"
#include <ranges>

namespace linter {
  namespace program_options = boost::program_options;

  namespace {
    auto get_repo(const std::string &repo_path) -> std::string {
      auto parts = repo_path | std::views::split('/') | std::ranges::to<std::vector<std::string>>();
      throw_if(parts.empty(), "repo path is empty");
      throw_if(parts.size() < 2, std::format("repo path:{} format error", repo_path));
      return parts[parts.size() - 2] + "/" + parts.back();
    }

    constexpr auto help                            = "help";
    constexpr auto version                         = "version";
    constexpr auto log_level                       = "log-level";
    constexpr auto repo_path                       = "repo-path";
    constexpr auto token                           = "token";
    constexpr auto base_ref                        = "base-ref";       // only used in pr
    constexpr auto head_ref                        = "head-ref";
    constexpr auto base_commit                     = "base-commit";    // pr
    constexpr auto head_commit                     = "head-commit";
    constexpr auto default_branch                  = "default-branch"; // pull
    constexpr auto event_name                      = "event-name";
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

  auto create_context_by_program_options(const program_options::variables_map &variables)
    -> context {
    auto ctx = context{};
    spdlog::info("Github actions environment: {}", env::get(github_actions));
    ctx.use_on_local = env::get(github_actions) != "true";

    if (variables.contains(log_level)) {
      ctx.log_level = variables[log_level].as<std::string>();
      throw_unless(std::ranges::contains(supported_log_level, ctx.log_level),
                   "unsupported log level");
    }

    if (variables.contains(repo_path)) {
      ctx.repo_path = variables[repo_path].as<std::string>();
      ctx.repo      = get_repo(ctx.repo_path);
    }
    if (variables.contains(token)) {
      ctx.token = variables[token].as<std::string>();
    }
    if (variables.contains(base_ref)) {
      ctx.base_ref = variables[base_ref].as<std::string>();
    }
    if (variables.contains(head_ref)) {
      ctx.head_ref = variables[head_ref].as<std::string>();
    }
    if (variables.contains(base_commit)) {
      ctx.base_commit = variables[base_commit].as<std::string>();
    }
    if (variables.contains(head_commit)) {
      ctx.head_commit = variables[head_commit].as<std::string>();
    }
    if (variables.contains(default_branch)) {
      ctx.default_branch = variables[default_branch].as<std::string>();
    }
    if (variables.contains(event_name)) {
      ctx.event_name = variables[event_name].as<std::string>();
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

    return ctx;
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
      (token,                           value<string>(),   "Set github token of git repository")
      (base_ref,                        value<string>(),   "Set the base/target reference of git repository")
      (head_ref,                        value<string>(),   "Set the head/cur reference of git repository."
                                                           "This could be same as base_ref.")
      (base_commit,                     value<string>(),   "Set the base commit of git repository")
      (head_commit,                     value<string>(),   "Set the head commit of git repository")
      (default_branch,                  value<string>(),   "Set the default branch of git repository")
      (event_name,                      value<string>(),   "Set the event name of git repository. Such as: push, pull_request")
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
} // namespace linter
