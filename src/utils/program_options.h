#pragma once

#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>

#include "utils/context.h"

namespace linter {
  constexpr auto supported_log_level = {"trace", "debug", "info", "error"};

  auto make_program_options_desc() -> boost::program_options::options_description;

  auto parse_program_options(
    int argc,
    char** argv,
    const boost::program_options::options_description& desc)
    -> boost::program_options::variables_map;

  void fill_context_by_program_options(const boost::program_options::variables_map& variables,
                                       context& ctx);

  void check_program_options(bool use_on_local,
                             const boost::program_options::variables_map& variables);
} // namespace linter
