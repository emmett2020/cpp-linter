#pragma once

#include "tools/clang_tidy.h"

namespace linter {

  /// The base_ref is the default branch all most events
  /// execept for some events which is the real base reference.
  struct context {
    bool use_on_local = false;
    std::string log_level;
    bool enable_step_summary         = false;
    bool enable_update_issue_comment = false;
    bool enable_pull_request_review  = false;

    std::string repo_path;
    std::string repo;
    std::string token;
    std::string event_name;
    std::string target;
    std::string source;
    std::int32_t pr_number = -1;

    clang_tidy::option clang_tidy_option{};
  };

  void print_context(const context &ctx);
} // namespace linter
