#pragma once

#include <string>
#include <unordered_map>

#include "tools/clang_format.h"
#include "tools/clang_tidy.h"
#include "utils/git_utils.h"
// TODO: some files should move out from utils/
#include "utils/context.h"

namespace linter {
/// The result of apply all tools on all changed files.
struct total_result {
  // key: changed file, value: the patch of changed file
  std::unordered_map<std::string, git::patch_ptr> patches;

  std::vector<std::string> clang_tidy_ignored_files;
  std::unordered_map<std::string, clang_tidy::result> clang_tidy_passed;
  std::unordered_map<std::string, clang_tidy::result> clang_tidy_failed;
  bool clang_tidy_fastly_exited = false;

  std::vector<std::string> clang_format_ignored_files;
  std::unordered_map<std::string, clang_format::result> clang_format_passed;
  std::unordered_map<std::string, clang_format::result> clang_format_failed;
  bool clang_format_fastly_exited = false;
};

/// This is used for both step_threshold and issue_comment.
auto make_clang_format_result_str(const context &ctx,
                                  const total_result &result) -> std::string;

} // namespace linter
