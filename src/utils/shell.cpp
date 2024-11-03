#include "shell.h"

#include <string>

#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>

#define BOOST_PROCESS_V2_SEPARATE_COMPILATION
#include <boost/process/v2.hpp>
#include <boost/process/v2/src.hpp>

#include "utils/util.h"

namespace linter {
  namespace bp = boost::process::v2;

  auto Execute(std::string_view command_path,
               const std::vector<std::string_view>& args,
               std::unordered_map<std::string, std::string> env) -> CommandResult {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto temp_args = std::vector<bp::string_view>{};
    temp_args.reserve(args.size());
    for (auto arg: args) {
      temp_args.emplace_back(arg.data(), arg.size());
    }

    auto proc = bp::process(
      context,
      command_path,
      temp_args,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err},
      bp::process_environment{env});

    auto ec  = boost::system::error_code{};
    auto res = CommandResult{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    ThrowIf(ec && ec != boost::asio::error::eof,
            std::format("Read stdout message of {} faild since {}", command_path, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    ThrowIf(ec && ec != boost::asio::error::eof,
            std::format("Read stderr message of {} faild since {}", command_path, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto Which(std::string_view command) -> CommandResult {
    CommandResult res = Execute("/usr/bin/which", {command});
    res.std_out       = Trim(res.std_out);
    return res;
  }

} // namespace linter
