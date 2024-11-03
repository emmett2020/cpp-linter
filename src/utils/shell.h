#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace linter::shell {
  struct Result {
    int exit_code;
    std::string std_out;
    std::string std_err;
  };

  using Envionment = std::unordered_map<std::string, std::string>;
  using Options    = std::vector<std::string>;

  auto Execute(std::string_view command_path, const Options& options, const Envionment& env)
    -> Result;

  auto Execute(std::string_view command_path, const Options& options) -> Result;

  auto Which(std::string command) -> Result;
} // namespace linter::shell


