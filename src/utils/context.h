#pragma once

#include "tools/clang_tidy.h"

namespace linter {

  /// The base_ref is the default branch all most events
  /// execept for some events which is the real base reference.
  struct context {
    bool use_on_local = false;
    std::string log_level;
    std::string repo_path;
    std::string repo;
    std::string token;
    std::string event_name;
    std::string target;
    std::string source;

    clang_tidy::option clang_tidy_option{};
  };

  void print_context(const context &ctx);

  void check_context(const context &ctx);

} // namespace linter
