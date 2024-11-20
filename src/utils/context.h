#pragma once

#include "tools/clang_tidy.h"

namespace linter {
  struct context {
    bool use_on_local = false;
    std::string log_level;
    std::string repo_path;
    std::string repo;
    std::string token;
    std::string event_name;
    std::string base_commit;
    std::string head_commit;
    std::string base_ref;
    std::string head_ref;
    std::string default_branch;

    clang_tidy::option clang_tidy_option{};
  };

  void print_context(const context &ctx);

  void check_context(const context &ctx);

} // namespace linter
