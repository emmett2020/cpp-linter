#include "system_command.h"
#include <boost/process.hpp>
#include <boost/process/io.hpp>

namespace linter {
namespace bp = boost::process;

// TODO: environment?
// TODO: how to both collect stdout and stderr?
CommandResult Exec(std::string command, std::string args) {
  bp::ipstream std_out;
  bp::ipstream std_err;
  auto env = boost::this_process::environment();
  bp::child child(command, bp::std_out > std_out, bp::std_err > std_err);

  CommandResult result;
  std::string line;

  while (child.running() && std::getline(std_out, line) && !line.empty()) {
    result.std_out.push_back(line);
  }
  child.wait();
  result.exit_code = child.exit_code();

  return result;
}
} // namespace linter
