#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace linter::clang_format {
/// The option to be passed into clang-format executable.
struct option {
  bool enable_clang_format = true;
  bool enable_clang_format_fastly_exit = false;
  bool enable_warning_as_error = false;
  std::uint16_t clang_format_version = -1;
  std::string clang_format_binary;
  std::string source_iregex = R"(.*\.(cpp|cc|c\+\+|cxx|c|cl|h|hpp|m|mm|inc))";
};

struct replacement {
  std::size_t offset;
  std::size_t length;
  std::string data;
};

using replacements = std::vector<replacement>;

struct result {
  bool pass = false;
  std::string file;
  replacements replaces;
  std::string origin_stderr;
};

/// Run clang tidy on one file.
auto run(const option &option, const std::string &repo,
         const std::string &file) -> result;

} // namespace linter::clang_format
