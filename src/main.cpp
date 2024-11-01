
#include <boost/process/v2/bind_launcher.hpp>
#include <boost/process/v2/cstring_ref.hpp>
#include <boost/process/v2/default_launcher.hpp>
#include <boost/process/v2/environment.hpp>
#include <boost/process/v2/error.hpp>
#include <boost/process/v2/execute.hpp>
#include <boost/process/v2/exit_code.hpp>
#include <boost/process/v2/pid.hpp>
#include <boost/process/v2/popen.hpp>
#include <boost/process/v2/process.hpp>
#include <boost/process/v2/process_handle.hpp>
#include <boost/process/v2/shell.hpp>
#include <boost/process/v2/start_dir.hpp>
#include <boost/process/v2/stdio.hpp>

#include <boost/asio.hpp>

#include <print>
#include <string>

// using namespace boost;
using namespace boost::asio;
namespace asio = boost::asio;
using namespace boost::process::v2;

int main() {
  asio::io_context ctx;
  asio::readable_pipe rp{ctx};
  process proc(ctx, "/usr/bin/g++", {"--version"}, process_stdio{{}, rp, {}});
  std::string output;
  ctx.run();

  boost::system::error_code ec;
  std::string s;
  s.reserve(1024);
  rp.read_some(asio::buffer(s));
  proc.wait();
}
