#define BOOST_PROCESS_V2_SEPARATE_COMPILATION

#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/process/v2.hpp>
#include <boost/process/v2/cstring_ref.hpp>
#include <boost/process/v2/src.hpp>

#include <boost/throw_exception.hpp>
#include <iostream>
#include <print>
#include <stdexcept>
#include <string>
namespace bp = boost::process::v2;
using namespace std::string_literals;

struct CommandResult {
  int exit_code;
  std::string std_out;
  std::string std_err;
};

CommandResult Exec(std::string_view command_path,
                   std::initializer_list<bp::string_view> args,
                   std::unordered_map<std::string, std::string> env = {}) {
  boost::asio::io_context ctx;
  boost::asio::readable_pipe rp_out{ctx};
  boost::asio::readable_pipe rp_err{ctx};

  bp::process proc(ctx, command_path, args,
                   bp::process_stdio{{}, rp_out, rp_err},
                   bp::process_environment{env});

  boost::system::error_code ec;
  CommandResult res;
  boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
  if (ec && ec != boost::asio::error::eof) {
    throw std::runtime_error(std::format(
        "Read stdout faild: {} in executing {}", ec.message(), command_path));
  }
  ec.clear();
  boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
  if (ec && ec != boost::asio::error::eof) {
    throw std::runtime_error(std::format(
        "Read stderr faild: {} in executing {}", ec.message(), command_path));
  }
  res.exit_code = proc.wait();
  return res;
}

int main() {
  auto [exit_code, std_out, std_err] =
      Exec("/linter/exe", {""}, {{"ADD", "HI"}});
  std::print("{}\n", exit_code);
  std::print("{}\n", std_out);
  std::print("{}\n", std_err);
}
