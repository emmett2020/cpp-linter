#pragma once

#include <string>
#include <vector>

namespace linter {

struct CommandResult {
  int exit_code;
  std::vector<std::string> std_out;
  std::vector<std::string> std_err;
};

CommandResult Exec(std::string command, std::string args = "");

} // namespace linter
