#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace linter::clang_format {

/// The option to be passed into clang-format executable.
struct user_option {
  bool enable_clang_format = false;
  bool enable_clang_format_fastly_exit = false;
  bool enable_warning_as_error = false;
  std::uint16_t clang_format_version = -1;
  std::string clang_format_binary;
  std::string source_iregex = R"(.*\.(cpp|cc|c\+\+|cxx|c|cl|h|hpp|m|mm|inc))";
};

struct replacement_t {
  int offset;
  int length;
  std::string data;
};

void trace_replacement(const replacement_t &replacement);

using replacements_t = std::vector<replacement_t>;

struct result {
  bool pass = false;
  std::string file;
  replacements_t replacements;
  std::string origin_stderr;
  std::string formatted_source_code;
};

/// Apply clang format on one file.
auto apply_on_single_file(const user_option &user_opt,
                          bool needs_formatted_source_code,
                          const std::string &repo,
                          const std::string &file) -> result;

} // namespace linter::clang_format
