#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace linter {

struct CommandResult {
  int exit_code;
  std::string std_out;
  std::string std_err;
};

CommandResult Exec(std::string_view command_path,
                   std::initializer_list<std::string_view> args,
                   std::unordered_map<std::string, std::string> env = {});

} // namespace linter
