#include "result.h"

namespace linter {

  auto make_clang_format_result_str(const context &ctx, const total_result &result) -> std::string {
    auto details = std::string{};
    details += std::format("<details>\n<summary>{} reports:<strong>{} fails</strong></summary>\n",
                           ctx.clang_format_option.clang_format_binary,
                           result.clang_format_failed.size());
    for (const auto &[name, failed]: result.clang_format_failed) {
      details += std::format("- {}\n", name);
    }
    details += "\n</details>";
    return details;
  }


} // namespace linter
