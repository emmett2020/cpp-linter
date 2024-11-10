#include "shell.h"

#include <string>
#include <string_view>

#define BOOST_PROCESS_V2_SEPARATE_COMPILATION
#include <boost/process/v2.hpp>
#include <boost/process/v2/src.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>

#include "utils/util.h"

namespace linter::shell {
  namespace bp = boost::process::v2;

  auto Execute(std::string_view command_path, const Options& options, const Envionment& env)
    -> Result {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto proc = bp::process(
      context,
      command_path,
      options,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err},
      bp::process_environment{env});
    auto ec  = boost::system::error_code{};
    auto res = Result{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stdout message of {} faild since {}", command_path, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stderr message of {} faild since {}", command_path, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto Execute(std::string_view command_path, const Options& options) -> Result {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto proc = bp::process(
      context,
      command_path,
      options,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err});
    auto ec  = boost::system::error_code{};
    auto res = Result{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stdout message of {} faild since {}", command_path, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stderr message of {} faild since {}", command_path, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto Which(std::string command) -> Result {
    constexpr auto which = std::string_view{"/usr/bin/which"};
    auto res             = Execute(which, {std::move(command)});
    res.std_out          = Trim(res.std_out);
    return res;
  }

} // namespace linter::shell
