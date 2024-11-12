#include "clang_tidy.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <forward_list>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <spdlog/spdlog.h>
#include <boost/regex.hpp>

#include "utils/shell.h"
#include "utils/util.h"

namespace linter {
  using namespace std::string_view_literals;

  namespace {
    enum class LineType : std::uint8_t {
      NOTIFICATION,
      DETAIL_CODE,
      FIX_SUGGESTION
    };

    constexpr auto supported_serverity     = {"warning"sv, "info"sv, "error"sv};
    constexpr auto kTotalWraningsGenerated = "warnings generated."sv;

    /// @brief: Only do some check.
    auto ParseClangTidyDetailLine(std::string_view line) -> std::string_view {
      spdlog::debug(std::format("Parsing detaile code: {}", line));
      // auto parts = line | std::views::split('|');
      // ThrowIf(std::distance(parts.begin(), parts.end()) >= 2,
      //         "The size of detailed code must be greater than 2.");
      return line;
    }

    /// @brief: Parse the header line of clang-tidy.
    /// @return: If the given line meets header line rule, parse it. Otherwise return std::nullopt.
    auto ParseClangTidyHeaderLine(std::string_view line) -> std::optional<tidy_header_line> {
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

      auto header_line       = tidy_header_line{};
      header_line.file_name  = file_name;
      header_line.row_idx    = row_idx;
      header_line.col_idx    = col_idx;
      header_line.serverity  = serverity;
      header_line.brief      = brief;
      header_line.diagnostic = diagnostic;
      return header_line;
    }


  } // namespace

  auto parse_clang_tidy_stdout(std::string_view output)
    -> std::tuple<std::vector<tidy_header_line>, std::vector<std::string>> {
    auto tidy_header_lines = std::vector<tidy_header_line>{};
    auto details           = std::vector<std::string>{};
    auto needs_details     = false;

    for (auto part: std::views::split(output, '\n')) {
      auto line = std::string_view{part};
      spdlog::trace("Parsing clang-dity output line:\n{}", line);

      auto header_line = ParseClangTidyHeaderLine(line);
      if (header_line) {
        spdlog::trace("The parsing line is a header line.");

        tidy_header_lines.emplace_back(header_line.value());
        details.emplace_back();
        needs_details = true;
        continue;
      }

      auto detail = ParseClangTidyDetailLine(line);
      if (needs_details && !detail.empty()) {
        details.back() += line;
        details.back() += "\n";
      }
    }
    return std::make_tuple(tidy_header_lines, details);
  }

  constexpr auto warning_and_error  = "^(\\d+) warnings and (\\d+) error generated";
  constexpr auto warnings_generated = "^(\\d+) warnings generated.";
  constexpr auto errors_generated   = "^(\\d+) errors generated.";
  constexpr auto suppressed         = R"(Suppressed (\d+) warnings \((\d+) in non-user code\)\.)";
  constexpr auto warnings_as_errors = "^(\\d+) warnings treated as errors";

  auto parse_clang_tidy_stderr(std::string_view std_err) -> tidy_statistic {
    auto statistic = tidy_statistic{};
    auto warning_and_error_cb = [&](boost::smatch& match){
      spdlog::trace("Result: warning: {}, errors: {}", match[1].str(), match[2].str());
      statistic.total_warnings = stoi(match[1].str());
      statistic.total_errors = stoi(match[2].str());
    };

    auto warnings_generated_cb = [&](boost::smatch& match){
      spdlog::trace("Result: total warnings: {}", match[1].str());
      statistic.total_warnings = stoi(match[1].str());
    };

    auto errors_generated_cb = [&](boost::smatch& match){
      spdlog::trace("Result: total errors: {}", match[1].str());
      statistic.total_errors = stoi(match[1].str());
    };

    auto suppressed_cb = [&](boost::smatch& match){
      spdlog::trace("Result: suppressed: {}, non user code warnings: {}", match[1].str(), match[2].str());
      statistic.total_suppressed_warnings = stoi(match[1].str());
      statistic.non_user_code_warnings = stoi(match[2].str());
    };

    auto warnings_as_errors_cb = [&](boost::smatch& match){
      spdlog::trace("Result: warnings trated as errors: {}", match[1].str());
      statistic.warnings_trated_as_errors = stoi(match[1].str());
    };

    auto try_match = [&](const std::string& line, const char* regex_str, auto callback){
      auto regex = boost::regex{regex_str};
      auto match = boost::smatch{};
      auto matched = boost::regex_match(line, match, regex, boost::match_extra);
      if (matched) {
        callback(match);
      }
    };

    for (auto part: std::views::split(std_err, '\n')) {
      auto line  = std::string{part.data(), part.size()};
      spdlog::trace("Parsing: {}", line);
      try_match(line, warning_and_error, warning_and_error_cb);
      try_match(line, warnings_generated, warnings_generated_cb);
      try_match(line, errors_generated, errors_generated_cb);
      try_match(line, suppressed, suppressed_cb);
      try_match(line, warnings_as_errors, warnings_as_errors_cb);
    }
    return statistic;
  }

  /// @detail Get the full path of clang tools
  /// @param tool_name Could be "clang-tidy" or "clang-format"
  /// @param version A number.
  auto find_clang_tool_exe_path(std::string_view tool_name, std::string_view version)
    -> std::string {
    auto command                = std::format("{}-{}", tool_name, version);
    auto [ec, std_out, std_err] = shell::which(command);
    throw_if(ec != 0, std_err);
    return std_out;
  }

  /// @detail Run clang_tidy_cmd and return result
  /// @param clang_tidy_cmd The full path of clang-tidy-version
  /// @param file The full file path which is going to be checked
  auto run_clang_tidy(
    std::string_view clang_tidy_cmd,
    const clang_tidy_option& option,
    std::string_view repo_path,
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
    spdlog::info("Running {} {}", clang_tidy_cmd, arg_str);

    return shell::execute(clang_tidy_cmd, opts, repo_path);
  }

  auto get_repo_full_path() -> std::string {
    return "";
  }

} // namespace linter
