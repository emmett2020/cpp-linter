#include <cctype>
#include <git2/diff.h>
#include <print>

#include <spdlog/spdlog.h>

#include "utils/git_utils.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

int main() {
  git::setup();
  auto *repo        = git::repo::open("/temp/temp");
  auto state        = git::repo::state(repo);
  auto path         = git::repo::path(repo);
  auto empty        = git::repo::is_empty(repo);
  auto *config      = git::repo::config(repo);
  auto *diff        = git::diff::index_to_workdir(repo, nullptr, nullptr);
  auto diff_details = git::diff::details(diff);

  for (const auto &file: diff_details) {
    std::println(
      "rel_path:{}\nrel_path {}\nfile num: {}\nflag: {}\nstatus: {}\n",
      file.old_file.relative_path,
      file.new_file.relative_path,
      file.file_num,
      git::file_flag_str(file.flags),
      git::delta_status_str(file.status));

    std::println("\ndetails: ");
    std::println("oid: {}\nsize: {}\nflag: {}\nmode: {}\n",
                 file.old_file.oid,
                 file.old_file.size,
                 git::file_flag_str(file.old_file.flags),
                 git::file_mode_str(file.old_file.mode));
    std::println("oid: {}\nsize: {}\nflag: {}\nmode: {}\n",
                 file.new_file.oid,
                 file.new_file.size,
                 git::file_flag_str(file.new_file.flags),
                 git::file_mode_str(file.new_file.mode));

    for (const auto &hunk: file.hunks) {
      std::print("hunk header: {}\nold_start: {}\nold_lines: {}\nnew_start: {}\nnew_lines: {}\n",
                 hunk.header,
                 hunk.old_start,
                 hunk.old_lines,
                 hunk.new_start,
                 hunk.new_lines);

      for (const auto &line: hunk.lines) {
        std::print("line content: {}", line.content);
      }
    }
  }


  git::repo::free(repo);
  git::config::free(config);
  git::shutdown();
}
