#pragma once

#include <initializer_list>
#include <span>
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

  Result
  Execute(std::string_view command_path, const Options& options = {}, const Envionment& env = {});

  Result Which(std::string command);
} // namespace linter::shell


