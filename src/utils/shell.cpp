#include "shell.h"
#include "utils/util.h"

#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <iostream>

#define BOOST_PROCESS_V2_SEPARATE_COMPILATION
#include <boost/process/v2.hpp>
#include <boost/process/v2/src.hpp>

namespace linter {
  namespace bp = boost::process::v2;

  auto Execute(std::string_view command_path,
               const std::vector<std::string_view>& args,
               std::unordered_map<std::string, std::string> env) -> CommandResult {
    boost::asio::io_context ctx;
    boost::asio::readable_pipe rp_out{ctx};
    boost::asio::readable_pipe rp_err{ctx};

    std::vector<bp::string_view> temp{};
    temp.reserve(args.size());
    for (auto arg: args) {
      temp.push_back(arg.data());
    }

    bp::process proc(
      ctx,
      command_path,
      temp,
      bp::process_stdio{{}, rp_out, rp_err},
      bp::process_environment{env});

    boost::system::error_code ec;
    CommandResult res;
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    if (ec && ec != boost::asio::error::eof) {
      throw std::runtime_error(
        std::format("Read stdout faild: {} in executing {}", ec.message(), command_path));
    }
    ec.clear();
    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    if (ec && ec != boost::asio::error::eof) {
      throw std::runtime_error(
        std::format("Read stderr faild: {} in executing {}", ec.message(), command_path));
    }
    res.exit_code = proc.wait();
    return res;
  }

  auto Which(std::string_view command) -> CommandResult {
    CommandResult res = Execute("/usr/bin/which", {command.data()});
    std::cout << res.std_out << "\n";
    auto trimmed = Trim(res.std_out);
    res.std_out  = trimmed;
    return res;
  }

} // namespace linter
