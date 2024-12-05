#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace linter::clang_format {
  /// The option to be passed into clang-format executable.
  struct option {
    bool enable_clang_format             = true;
    bool enable_clang_format_fastly_exit = false;
    bool enable_warning_as_error         = false;
    bool enable_inplace_edit             = false;
    std::uint16_t clang_format_version   = -1;
    std::string clang_format_binary;
    std::string source_iregex = R"(.*\.(cpp|cc|c\+\+|cxx|c|cl|h|hpp|m|mm|inc))";
    std::vector<std::string> files;
  };

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
    std::string file;
    statistic stat;
    diagnostics diags;
    std::string origin_stderr;
  };

  /// Run clang tidy on one file.
  auto run(const option& option, const std::string& repo, const std::string& file) -> result;

} // namespace linter::clang_format

