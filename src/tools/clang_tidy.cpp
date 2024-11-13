#include "clang_tidy.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <spdlog/spdlog.h>
#include <boost/regex.hpp>

#include "utils/shell.h"
#include "utils/util.h"

namespace linter::clang_tidy {
  using namespace std::string_view_literals;

  namespace {
    constexpr auto supported_serverity = {"warning"sv, "info"sv, "error"sv};

    //Parse the header line of clang-tidy. If the given line meets header line
    //rule, parse it. Otherwise return std::nullopt.
    auto parse_clang_tidy_header_line(std::string_view line) -> std::optional<diagnostic_header> {
      auto parts = line | std::views::split(':');

      if (std::distance(parts.begin(), parts.end()) != 5) {
        return std::nullopt;
      }
      auto iter             = parts.begin();
      auto file_name        = std::string_view{*iter++};
      auto row_idx          = std::string_view{*iter++};
      auto col_idx          = std::string_view{*iter++};
      auto serverity        = trim_left(std::string_view{*iter++});
      auto brief_diagnostic = std::string_view{*iter++};

      if (!std::ranges::all_of(row_idx, ::isdigit)) {
        return std::nullopt;
      }
      if (!std::ranges::all_of(col_idx, ::isdigit)) {
        return std::nullopt;
      }
      if (!std::ranges::contains(supported_serverity, serverity)) {
        return std::nullopt;
      }

      const auto* square_brackets = std::ranges::find(brief_diagnostic, '[');
      if ((square_brackets == brief_diagnostic.end())
          || (brief_diagnostic.size() < 3)
          || (brief_diagnostic.back() != ']')) {
        return std::nullopt;
      }
      auto brief      = std::string_view{brief_diagnostic.begin(), square_brackets};
      auto diagnostic = std::string_view{square_brackets, brief_diagnostic.end()};

      auto header_line            = diagnostic_header{};
      header_line.file_name       = file_name;
      header_line.row_idx         = row_idx;
      header_line.col_idx         = col_idx;
      header_line.serverity       = serverity;
      header_line.brief           = brief;
      header_line.diagnostic_type = diagnostic;
      return header_line;
    }

    auto execute(std::string_view cmd,
                 const option& option,
                 std::string_view repo,
                 std::string_view file) -> shell::result {
      auto opts = std::vector<std::string>{};
      if (!option.database.empty()) {
        opts.emplace_back(std::format("-p={}", option.database));
      }
      if (!option.checks.empty()) {
        opts.emplace_back(std::format("-checks={}", option.checks));
      }
      if (option.allow_no_checks) {
        opts.emplace_back("--allow-no-checks");
      }
      if (!option.config.empty()) {
        opts.emplace_back(std::format("--config={}", option.config));
      }
      if (!option.config_file.empty()) {
        opts.emplace_back(std::format("--config-file={}", option.config_file));
      }
      if (option.enable_check_profile) {
        opts.emplace_back("--enable-check-profile");
      }
      if (!option.header_filter.empty()) {
        opts.emplace_back(std::format("--header-filter={}", option.header_filter));
      }
      if (!option.line_filter.empty()) {
        opts.emplace_back(std::format("--line-filter={}", option.line_filter));
      }

      opts.emplace_back(file);

      auto arg_str = opts | std::views::join_with(' ') | std::ranges::to<std::string>();
      spdlog::info("Running {} {}", cmd, arg_str);

      return shell::execute(cmd, opts, repo);
    }

    void try_match(const std::string& line, const char* regex_str, auto callback) {
      auto regex   = boost::regex{regex_str};
      auto match   = boost::smatch{};
      auto matched = boost::regex_match(line, match, regex, boost::match_extra);
      if (matched) {
        callback(match);
      }
    };

    auto parse_stdout(std::string_view std_out) -> diagnostics {
      auto diags         = diagnostics{};
      auto needs_details = false;

      for (auto part: std::views::split(std_out, '\n')) {
        auto line = std::string{part.data(), part.size()};
        spdlog::trace("Parsing: {}", line);

        auto header_line = parse_clang_tidy_header_line(line);
        if (header_line) {
          spdlog::trace(
            "Result: {}:{}:{}: {} {} {}",
            header_line->file_name,
            header_line->row_idx,
            header_line->col_idx,
            header_line->serverity,
            header_line->brief,
            header_line->diagnostic_type);

          diags.emplace_back(std::move(*header_line));
          needs_details = true;
          continue;
        }

        if (needs_details) {
          diags.back().details.emplace_back(std::move(line));
        }
      }

      spdlog::info("Parsed clang tidy stdout, got {} diagnostics.", diags.size());
      return diags;
    }

    constexpr auto warning_and_error  = "^(\\d+) warnings and (\\d+) error generated.";
    constexpr auto warnings_generated = "^(\\d+) warnings generated.";
    constexpr auto errors_generated   = "^(\\d+) errors generated.";
    constexpr auto suppressed         = R"(Suppressed (\d+) warnings \((\d+) in non-user code\)\.)";
    constexpr auto suppressed_lint =
      R"(Suppressed (\d+) warnings \((\d+) in non-user code, (\d+) NOLINT\)\.)";
    constexpr auto warnings_as_errors = "^(\\d+) warnings treated as errors";

    auto parse_stderr(std::string_view std_err) -> statistic {
      auto stat                 = statistic{};
      auto warning_and_error_cb = [&](boost::smatch& match) {
        spdlog::trace("Result: warning: {}, errors: {}", match[1].str(), match[2].str());
        stat.warnings = stoi(match[1].str());
        stat.errors   = stoi(match[2].str());
      };

      auto warnings_generated_cb = [&](boost::smatch& match) {
        spdlog::trace("Result: warnings: {}", match[1].str());
        stat.warnings = stoi(match[1].str());
      };

      auto errors_generated_cb = [&](boost::smatch& match) {
        spdlog::trace("Result: errors: {}", match[1].str());
        stat.errors = stoi(match[1].str());
      };

      auto suppressed_cb = [&](boost::smatch& match) {
        spdlog::trace("Result: suppressed: {}, non user code warnings: {}",
                      match[1].str(),
                      match[2].str());
        stat.total_suppressed_warnings = stoi(match[1].str());
        stat.non_user_code_warnings    = stoi(match[2].str());
      };

      auto suppressed_and_nolint_cb = [&](boost::smatch& match) {
        spdlog::trace("Result: suppressed: {}, non user code warnings: {}",
                      match[1].str(),
                      match[2].str(),
                      match[3].str());
        stat.total_suppressed_warnings = stoi(match[1].str());
        stat.non_user_code_warnings    = stoi(match[2].str());
        stat.no_lint_warnings          = stoi(match[3].str());
      };

      auto warnings_as_errors_cb = [&](boost::smatch& match) {
        spdlog::trace("Result: warnings trated as errors: {}", match[1].str());
        stat.warnings_trated_as_errors = stoi(match[1].str());
      };

      for (auto part: std::views::split(std_err, '\n')) {
        auto line = std::string{part.data(), part.size()};
        spdlog::trace("Parsing: {}", line);
        try_match(line, warning_and_error, warning_and_error_cb);
        try_match(line, warnings_generated, warnings_generated_cb);
        try_match(line, errors_generated, errors_generated_cb);
        try_match(line, suppressed, suppressed_cb);
        try_match(line, warnings_as_errors, warnings_as_errors_cb);
        try_match(line, suppressed_lint, suppressed_and_nolint_cb);
      }

      return stat;
    }

    void print_statistic(const statistic& stat) {
      spdlog::debug("Errors: {}", stat.errors);
      spdlog::debug("Warnings: {}", stat.warnings);
      spdlog::debug("Warnings trated as errors: {}", stat.warnings_trated_as_errors);
      spdlog::debug("Total suppressed warnings: {}", stat.total_suppressed_warnings);
      spdlog::debug("Non user code warnings: {}", stat.non_user_code_warnings);
      spdlog::debug("No lint warnings: {}", stat.no_lint_warnings);
    }


  } // namespace

  auto run(const std::string& cmd,
           const option& option,
           const std::string& repo,
           const std::string& file) -> result {
    auto [ec, std_out, std_err] = execute(cmd, option, repo, file);
    spdlog::trace("clang-tidy original output:\nreturn code: {}\nstdout:\n{}\nstderr:\n{}",
                  ec,
                  std_out,
                  std_err);
    spdlog::debug("Successfully ran clang-tidy, now start to parse the outputs of it.");
    auto res = result{};
    res.pass = ec == 0;
    res.diag = clang_tidy::parse_stdout(std_out);
    res.stat = clang_tidy::parse_stderr(std_err);
    print_statistic(res.stat);
    spdlog::info("Got total errors: {}", res.stat.total_errors());
    return res;
  }


} // namespace linter::clang_tidy
