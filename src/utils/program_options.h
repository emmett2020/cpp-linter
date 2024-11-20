#pragma once

#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <spdlog/spdlog.h>

#include "utils/context.h"

namespace linter {
  constexpr auto supported_log_level = {"TRACE", "DEBUG", "INFO", "ERROR"};

  auto make_program_options_desc() -> boost::program_options::options_description;

  auto parse_program_options(int argc,
                             char** argv,
                             const boost::program_options::options_description& desc)
    -> boost::program_options::variables_map;

  auto create_context_by_program_options(const boost::program_options::variables_map &variables) -> context;
}  // namespace linter
