
// #include <boost/asio.hpp>
// #include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/process/v2.hpp>

#include <iostream>
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
  std::cout << proc.running();

  boost::system::error_code ec;
  std::string s;
  s.resize(1024);
  asio::read(rp, asio::buffer(s), ec);
  std::cout << ec.message() << std::endl;
  std::cout << s << std::endl;
  assert(proc.wait() == 0);
}
