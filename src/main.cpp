#define BOOST_PROCESS_V2_SEPARATE_COMPILATION 1

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
#include <boost/process/v2/src.hpp>
#include <boost/process/v2/start_dir.hpp>
#include <boost/process/v2/stdio.hpp>
#include <iostream>

using namespace std;
using namespace boost::process::v2;
// using namespace boost;

int main() {
  boost::asio::io_context ctx;

  auto proc = process{ctx, "/bin/ls", {}};
  assert(proc.wait() == 0);
}
