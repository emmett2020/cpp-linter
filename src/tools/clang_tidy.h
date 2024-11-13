#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "utils/shell.h"

namespace linter::clang_tidy {
  /// Each diagnostic hase a header line.
  struct diagnostic_header {
    std::string file_name;
    std::string row_idx;
    std::string col_idx;
    std::string serverity;
    std::string brief;
    std::string diagnostic_type;
  };

  /// Represents one diagnostic which outputed by clang-tidy.
  /// Generally, each diagnostic has a header line and several details line
  /// which give a further detailed explanation.
  struct diagnostic {
    diagnostic_header header;
    std::vector<std::string> details; // Each line withouts the trailing "\n".
  };

  /// Represents all diagnostics which outputed by clang-tidy.
  using diagnostics = std::vector<diagnostic>;

  /// Represents statistics outputed by clang-tidy. It's usually the stderr
  /// messages of clang-tidy.
  struct statistic {
    std::uint32_t warnings                   = 0;
    std::uint32_t errors                     = 0;
    std::uint32_t warnings_treated_as_errors = 0;
    std::uint32_t total_suppressed_warnings  = 0;
    std::uint32_t non_user_code_warnings     = 0;
    std::uint32_t no_lint_warnings           = 0;

    [[nodiscard]] auto total_errors() const noexcept -> std::uint32_t {
      return errors + warnings_treated_as_errors;
    }
  };

  struct result {
    bool pass = false;
    statistic stat;
    diagnostics diag;
  };

  /// The option to be passed into clang-tidy executable.
  struct option {
    bool allow_no_checks      = false;
    bool enable_check_profile = false;
    std::string checks;
    std::string config;
    std::string config_file;
    std::string database;
    std::string header_filter;
    std::string line_filter;
  };

  /// Run clang tidy on one file.
  auto run(const std::string& cmd,
           const option& option,
           const std::string& repo,
           const std::string& file) -> result;

} // namespace linter::clang_tidy

