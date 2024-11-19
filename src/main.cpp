#include <algorithm>
#include <cctype>
#include <filesystem>
#include <print>
#include <ranges>
#include <string>

#include <spdlog/spdlog.h>
#include <git2/diff.h>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include "spdlog/common.h"
#include "tools/clang_tidy.h"
#include "github/api.h"
#include "utils/git_utils.h"
#include "utils/shell.h"
#include "utils/util.h"

using namespace linter; // NOLINT
using namespace std::string_literals;
namespace program_options = boost::program_options;

namespace {
  /// Find the full executable path of clang tools with specific version.
  auto find_clang_tool_exe_path(std::string_view tool, std::string_view version) -> std::string {
    auto command                = std::format("{}-{}", tool, version);
    auto [ec, std_out, std_err] = shell::which(command);
    throw_if(
      ec != 0,
      std::format("execute /usr/bin/which to find {}-{} failed, got: {}", tool, version, std_err));
    return std_out;
  }

  constexpr auto supported_log_level = {"TRACE", "DEBUG", "INFO", "ERROR"};

  /// This must be called before any spdlog use.
  void set_log_level(const std::string &log_level_str) {
    auto log_level = spdlog::level::info;
    if (log_level_str == "TRACE") {
      log_level = spdlog::level::trace;
    } else if (log_level_str == "DEBUG") {
      log_level = spdlog::level::debug;
    } else if (log_level_str == "ERROR") {
      log_level = spdlog::level::err;
    } else {
      log_level = spdlog::level::info;
    }
    spdlog::set_level(log_level);
  }

  /// https://docs.github.com/en/actions/writing-workflows/choosing-when-your-workflow-runs/events-that-trigger-workflows#pull_request
  struct user_options {
    bool use_on_local = false;
    std::string log_level;
    std::string repo_full_path;
    std::string owner_and_repo;
    std::string target_ref;
    std::string source_sha;
    std::string source_ref;
    std::string token;

    bool enable_clang_tidy           = true;
    bool enable_clang_tidy_fast_exit = false;
    std::string clang_tidy_version;
    clang_tidy::option clang_tidy_option;
  };

  void print_full_options(const user_options &options) {
    spdlog::debug("Log level: {}", options.log_level);
    spdlog::debug("Repository path: {}", options.repo_full_path);
    spdlog::debug("Repository: {}", options.owner_and_repo);
    spdlog::debug("Repository full path: {}/{}", options.repo_full_path, options.owner_and_repo);
    spdlog::debug("Repository target ref: {}", options.target_ref);
    spdlog::debug("Repository source ref: {}", options.source_ref);
    spdlog::debug("Repository source sha: {}", options.source_sha);

    const auto &tidy_option = options.clang_tidy_option;
    spdlog::debug("The options of clang-tidy:");
    spdlog::debug("enable clang tidy: {}", options.enable_clang_tidy);
    spdlog::debug("enable clang tidy fast exit: {}", options.enable_clang_tidy_fast_exit);
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

  auto print_changed_files(const std::vector<std::string> &files) {
    spdlog::info("Got {} changed files", files.size());
    for (const auto &file: files) {
      spdlog::debug(file);
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

  //
  auto parse_command_options(const program_options::variables_map &variables) -> user_options {
    auto options         = user_options();
    options.use_on_local = env::get(github_actions) == "true";

    if (variables.contains("log_level")) {
      options.log_level = variables["log_level"].as<std::string>();
      throw_unless(std::ranges::contains(supported_log_level, options.log_level),
                   "unsupported log level");
    }

    /// REFINE:
    if (variables.contains("repo_full_path")) {
      throw_unless(options.use_on_local, "repo_full_path option only supports on local use");
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
      options.enable_clang_tidy_fast_exit = variables["enable_clang_tidy_fast_exit"].as<bool>();
    }
    if (variables.contains("clang_tidy_version")) {
      options.clang_tidy_version = variables["clang_tidy_version"].as<std::string>();
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
      options.clang_tidy_option.checks = variables["clang_tidy_checks"].as<std::string>();
    }
    if (variables.contains("clang_tidy_config")) {
      options.clang_tidy_option.config = variables["config"].as<std::string>();
    }
    if (variables.contains("clang_tidy_config_file")) {
      options.clang_tidy_option.config_file = variables["config_file"].as<std::string>();
    }
    if (variables.contains("clang_tidy_databse")) {
      options.clang_tidy_option.database = variables["clang_tidy_databse"].as<std::string>();
    }
    if (variables.contains("clang_tidy_header_filter")) {
      options.clang_tidy_option.header_filter =
        variables["clang_tidy_header_filter"].as<std::string>();
    }
    if (variables.contains("clang_tidy_line_filter")) {
      options.clang_tidy_option.line_filter = variables["clang_tidy_line_filter"].as<std::string>();
    }

    return options;
  }

  void add_supported_option(program_options::options_description &desc) {
    desc.add_options()                                                                           //
      ("help", "produce help message")("version", "print current version")                       //
      ("log_level", program_options::value<std::string>(), "Set the log level of cpp-linter")    //
      ("repo_full_path",
       program_options::value<std::string>(),
       "Set the full path of to be checked git repository. This option shouldn't be used on CI") //
      ("target_ref",
       program_options::value<std::string>(),
       "Set the target reference of git repository")                                             //
      ("source_ref",
       program_options::value<std::string>(),
       "Set the source reference of git repository")                                             //
      ("source_sha",
       program_options::value<std::string>(),
       "Set the source sha1 of git repository")                                                  //
      ("token", program_options::value<std::string>(), "Set github token of git repository")     //
      ("enable_clang_tidy", program_options::value<bool>(), "Enabel clang-tidy check")           //
      ("enable_clang_tidy_fast_exit",
       "Enabel clang-tidy fast exit. This means cpp-linter will immediately stop all clang-tidy "
       "check when found first check error")                                                     //
      ("clang_tidy_version", "The version of clang-tidy")                                        //
      ("clang_tidy_allow_no_checks", "Enabel clang-tidy allow_no_check option");
  }


} // namespace

auto main(int argc, char **argv) -> int {
  auto desc      = program_options::options_description{"cpp-linter options"};
  auto variables = program_options::variables_map{};

  add_supported_option(desc);
  program_options::store(program_options::parse_command_line(argc, argv, desc), variables);
  program_options::notify(variables);

  // These options don't any real work.
  if (variables.contains("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (variables.contains("version")) {
    std::print("{}", get_current_version());
    return 0;
  }

  auto options = parse_command_options(variables);
  set_log_level(options.log_level);
  print_full_options(options);

  auto github_client = github_api_client{};
  github_client.read_envrionment_variables();
  auto repo_path = options.use_on_local ? options.repo_full_path : github_client.repo_full_path();

  git::setup();
  auto *repo = git::repo::open(repo_path);

  auto changed_files =
    git::diff::changed_files(repo, github_client.base_commit(), github_client.head_commit());
  print_changed_files(changed_files);

  if (options.enable_clang_tidy) {
    auto clang_tidy_exe = find_clang_tool_exe_path("clang-tidy", options.clang_tidy_version);
    spdlog::info("The clang-tidy executable path: {}", clang_tidy_exe);
    throw_if(clang_tidy_exe.empty(), "find clang tidy executable failed");

    for (const auto &file: changed_files) {
      auto result = clang_tidy::run(clang_tidy_exe, options.clang_tidy_option, repo_path, file);
      if (!result.pass && options.enable_clang_tidy_fast_exit) {
        spdlog::info("fast exit");
        return -1;
      }
    }
  }

  git::repo::free(repo);
  git::shutdown();
}
