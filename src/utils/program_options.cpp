#include "program_options.h"

#include "utils/env_manager.h"
#include "github/api.h"

namespace linter {
void print_full_options(const user_options &options) {
  spdlog::debug("Log level: {}", options.log_level);
  spdlog::debug("Repository path: {}", options.repo_full_path);
  spdlog::debug("Repository: {}", options.owner_and_repo);
  spdlog::debug("Repository full path: {}/{}", options.repo_full_path,
                options.owner_and_repo);
  spdlog::debug("Repository target ref: {}", options.target_ref);
  spdlog::debug("Repository source ref: {}", options.source_ref);
  spdlog::debug("Repository source sha: {}", options.source_sha);

  const auto &tidy_option = options.clang_tidy_option;
  spdlog::debug("The options of clang-tidy:");
  spdlog::debug("enable clang tidy: {}", options.enable_clang_tidy);
  spdlog::debug("enable clang tidy fast exit: {}",
                options.enable_clang_tidy_fast_exit);
  spdlog::debug("clang tidy version: {}", options.clang_tidy_version);
  spdlog::debug("checks: {}", tidy_option.checks);
  spdlog::debug("config: {}", tidy_option.config);
  spdlog::debug("config file: {}", tidy_option.config_file);
  spdlog::debug("database: {}", tidy_option.database);
  spdlog::debug("header filter: {}", tidy_option.header_filter);
  spdlog::debug("line filter: {}", tidy_option.line_filter);
  spdlog::debug("allow no checks: {}", tidy_option.allow_no_checks);
  spdlog::debug("enable check profile: {}", tidy_option.enable_check_profile);
}

auto parse_command_options(const program_options::variables_map &variables)
    -> user_options {
  auto options = user_options();
  options.use_on_local = env::get(github_actions) == "true";

  if (variables.contains("log_level")) {
    options.log_level = variables["log_level"].as<std::string>();
    throw_unless(std::ranges::contains(supported_log_level, options.log_level),
                 "unsupported log level");
  }

  /// REFINE:
  if (variables.contains("repo_full_path")) {
    throw_unless(options.use_on_local,
                 "repo_full_path option only supports on local use");
    options.repo_full_path = variables["repo_full_path"].as<std::string>();
  }

  if (variables.contains("target_ref")) {
    options.target_ref = variables["target_ref"].as<std::string>();
  }
  if (variables.contains("source_ref")) {
    options.source_ref = variables["source_ref"].as<std::string>();
  }
  if (variables.contains("source_sha")) {
    options.source_sha = variables["source_sha"].as<std::string>();
  }
  if (variables.contains("token")) {
    options.token = variables["toekn"].as<std::string>();
  }
  if (variables.contains("enable_clang_tidy")) {
    options.enable_clang_tidy = variables["enable_clang_tidy"].as<bool>();
  }
  if (variables.contains("enable_clang_tidy_fast_exit")) {
    options.enable_clang_tidy_fast_exit =
        variables["enable_clang_tidy_fast_exit"].as<bool>();
  }
  if (variables.contains("clang_tidy_version")) {
    options.clang_tidy_version =
        variables["clang_tidy_version"].as<std::uint16_t>();
  }
  if (variables.contains("clang_tidy_allow_no_checks")) {
    options.clang_tidy_option.allow_no_checks =
        variables["clang_tidy_allow_no_checks"].as<bool>();
  }
  if (variables.contains("clang_tidy_enable_check_profile")) {
    options.clang_tidy_option.enable_check_profile =
        variables["clang_tidy_enable_check_profile"].as<bool>();
  }
  if (variables.contains("clang_tidy_checks")) {
    options.clang_tidy_option.checks =
        variables["clang_tidy_checks"].as<std::string>();
  }
  if (variables.contains("clang_tidy_config")) {
    options.clang_tidy_option.config = variables["config"].as<std::string>();
  }
  if (variables.contains("clang_tidy_config_file")) {
    options.clang_tidy_option.config_file =
        variables["config_file"].as<std::string>();
  }
  if (variables.contains("clang_tidy_databse")) {
    options.clang_tidy_option.database =
        variables["clang_tidy_databse"].as<std::string>();
  }
  if (variables.contains("clang_tidy_header_filter")) {
    options.clang_tidy_option.header_filter =
        variables["clang_tidy_header_filter"].as<std::string>();
  }
  if (variables.contains("clang_tidy_line_filter")) {
    options.clang_tidy_option.line_filter =
        variables["clang_tidy_line_filter"].as<std::string>();
  }

  return options;
}

auto make_program_options_desc() -> program_options::options_description {
  auto desc = program_options::options_description{"cpp-linter options"};
  desc.add_options()                                                       //
      ("help", "produce help message")("version", "print current version") //
      ("log_level", program_options::value<std::string>(),
       "Set the log level of cpp-linter") //
      ("repo_full_path", program_options::value<std::string>(),
       "Set the full path of to be checked git repository. This option "
       "shouldn't be used on CI") //
      ("target_ref", program_options::value<std::string>(),
       "Set the target reference of git repository") //
      ("source_ref", program_options::value<std::string>(),
       "Set the source reference of git repository") //
      ("source_sha", program_options::value<std::string>(),
       "Set the source sha1 of git repository") //
      ("token", program_options::value<std::string>(),
       "Set github token of git repository") //
      ("enable_clang_tidy", program_options::value<bool>(),
       "Enabel clang-tidy check") //
      ("enable_clang_tidy_fast_exit",
       "Enabel clang-tidy fast exit. This means cpp-linter will immediately "
       "stop all clang-tidy "
       "check when found first check error") //
      ("clang_tidy_version", program_options::value<uint16_t>(),
       "The version of clang-tidy") //
      ("clang_tidy_allow_no_checks", "Enabel clang-tidy allow_no_check option");
  return desc;
}

auto parse_program_options(int argc, char **argv,
                           const program_options::options_description &desc)
    -> program_options::variables_map {
  auto variables = program_options::variables_map{};
  program_options::store(program_options::parse_command_line(argc, argv, desc),
                         variables);
  program_options::notify(variables);
  return variables;
}
} // namespace linter
