#pragma once

// #include "tools/clang_format.h"
// #include "tools/clang_tidy.h"
#include <string>
#include <cstdint>

namespace linter {

  /// The base_ref is the default branch all most events
  /// execept for some events which is the real base reference.
  struct context_t {
    bool use_on_local = false;
    std::string log_level;
    bool enable_step_summary        = false;
    bool enable_comment_on_issue    = false;
    bool enable_pull_request_review = false;

    std::string repo_path;
    std::string repo;
    std::string token;
    std::string event_name;
    std::string target;
    std::string source;
    std::int32_t pr_number = -1;

    // clang_tidy::option clang_tidy_option{};
    // clang_format::user_option clang_format_option{};
  };

  void print_context(const context_t &ctx);
} // namespace linter
