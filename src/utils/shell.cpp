#include "shell.h"

#include <string>
#include <string_view>

#define BOOST_PROCESS_V2_SEPARATE_COMPILATION
#include <boost/process/v2.hpp>
#include <boost/process/v2/src.hpp>
#include <boost/process/v2/start_dir.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>

#include "utils/util.h"

namespace linter::shell {
  namespace bp = boost::process::v2;

  auto execute(std::string_view command, const options& opts) -> result {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto proc = bp::process(
      context,
      command,
      opts,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err});
    auto ec  = boost::system::error_code{};
    auto res = result{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stdout message of {} faild since {}", command, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stderr message of {} faild since {}", command, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto execute(std::string_view command, const options& opts, std::string_view start_dir)
    -> result {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto proc = bp::process(
      context,
      command,
      opts,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err},
      bp::process_start_dir{start_dir});
    auto ec  = boost::system::error_code{};
    auto res = result{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stdout message of {} faild since {}", command, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stderr message of {} faild since {}", command, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto execute(std::string_view command, const options& opts, const envrionment& env) -> result {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto proc = bp::process(
      context,
      command,
      opts,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err},
      bp::process_environment{env});
    auto ec  = boost::system::error_code{};
    auto res = result{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stdout message of {} faild since {}", command, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stderr message of {} faild since {}", command, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto execute(std::string_view command,
               const options& opts,
               const envrionment& env,
               std::string_view start_dir) -> result {
    auto context = boost::asio::io_context{};
    auto rp_out  = boost::asio::readable_pipe{context};
    auto rp_err  = boost::asio::readable_pipe{context};

    auto proc = bp::process(
      context,
      command,
      opts,
      bp::process_stdio{.in = {}, .out = rp_out, .err = rp_err},
      bp::process_environment{env},
      bp::process_start_dir(start_dir));
    auto ec  = boost::system::error_code{};
    auto res = result{};
    boost::asio::read(rp_out, boost::asio::dynamic_buffer(res.std_out), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stdout message of {} faild since {}", command, ec.message()));
    ec.clear();

    boost::asio::read(rp_err, boost::asio::dynamic_buffer(res.std_err), ec);
    throw_if(ec && ec != boost::asio::error::eof,
             std::format("Read stderr message of {} faild since {}", command, ec.message()));

    res.exit_code = proc.wait();
    return res;
  }

  auto which(std::string command) -> result {
    constexpr auto which = std::string_view{"/usr/bin/which"};
    auto res             = execute(which, {std::move(command)});
    res.std_out          = trim(res.std_out);
    return res;
  }

} // namespace linter::shell
