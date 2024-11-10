#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace linter::shell {
  struct result {
    int exit_code;
    std::string std_out;
    std::string std_err;
  };

  using envionment = std::unordered_map<std::string, std::string>;
  using options    = std::vector<std::string>;

  auto execute(std::string_view command_path, const options& opts, const envionment& env) -> result;
  auto execute(std::string_view command_path, const options& opts) -> result;
  auto which(std::string command) -> result;
} // namespace linter::shell


