#include <cctype>
#include <git2/diff.h>
#include <print>
#include <ranges>

#include <spdlog/spdlog.h>
#include <string>

#include "github/api.h"
#include "tools/clang_tidy.h"
#include "utils/git_utils.h"
#include "utils/shell.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

void ForClangTidy() {
  int *hi = 0;
}

namespace {

  auto branch_to_branch(git::repo_ptr repo, const std::string &branch1, const std::string &branch2)
    -> std::vector<git::diff_delta_detail> {
    auto oid1     = git::ref::name_to_oid(repo, branch1);
    auto oid2     = git::ref::name_to_oid(repo, branch2);
    auto *commit1 = git::commit::lookup(repo, &oid1);
    auto *commit2 = git::commit::lookup(repo, &oid2);
    auto *tree1   = git::commit::tree(commit1);
    auto *tree2   = git::commit::tree(commit2);

    auto *diff = git::diff::tree_to_tree(repo, tree1, tree2, nullptr);
    git::commit::free(commit1);
    git::commit::free(commit2);
    git::object::free(reinterpret_cast<git::object_ptr>(tree1));
    git::object::free(reinterpret_cast<git::object_ptr>(tree2));
    auto ret = git::diff::details(diff);
    git::diff::free(diff);
    return ret;
  }

} // namespace

int main() {
  // spdlog::set_level(spdlog::level::trace); // FOR DEBUG
  // auto cmd    = GetClangToolFullPath("clang-tidy", "20");
  // auto option = TidyOption{
  //   .config_file = ".clang-tidy",
  //   .database    = "build",
  // };
  //
  // auto [ec, std_out, std_err] = RunClangTidy(cmd, option, "src/main.cpp");
  // auto [noti_lines, codes]    = ParseClangTidyStdout(std_out);
  //
  // std::println("ec: {}", ec);
  // // std::println("std_err\n------------\n{}------------\n", std_err);
  //
  // auto statistic = ParseClangTidyStderr(std_err);
  // std::println("{} warnings generated.", statistic.total_warnings);
  //
  // for (const auto& [line, code]: std::ranges::views::zip(noti_lines, codes))
  // {
  //   std::println(
  //     "{}:{}:{}: {}: {} {}",
  //     line.file_name,
  //     line.row_idx,
  //     line.col_idx,
  //     line.serverity,
  //     line.brief,
  //     line.diagnostic);
  //   std::print("{}", code);
  // }

  // ParseDiff();
  // env::SetCache(kGithubEventName, kGithubEventPush);
  // env::SetCache(kGithubToken, "");
  // env::SetCache(kGithubRepository, "emmett2020/temp");
  // // env::SetCache(kGithubEventPath, "----");
  // env::SetCache(kGithubSha, "b083715");
  // auto api_client = GithubApiClient{};
  // auto t          = api_client.GetChangedFiles();

  git::setup();
  auto *repo   = git::repo::open("/temp/temp");
  auto state   = git::repo::state(repo);
  auto path    = git::repo::path(repo);
  auto empty   = git::repo::is_empty(repo);
  auto *config = git::repo::config(repo);
  // auto *ref    = git::branch::create(repo, "bygit2", nullptr, false);

  // git_diff_options opts;
  // git::diff::init_option(&opts);
  auto *diff         = git::diff::index_to_workdir(repo, nullptr, nullptr);
  auto num_deltas    = git::diff::num_deltas(diff);
  const auto *deltas = git::diff::get_delta(diff, 0);

  auto diff_details = branch_to_branch(repo, "refs/heads/main", "refs/heads/test");
  // auto diff_details = git::diff::details(diff);
  // std::println("old_file_path: {}, new_file_path: {}, similarity: {}",
  //              diff_details.old_file_path,
  //              diff_details.new_file_path,
  //              diff_details.similarity);

  for (const auto &file: diff_details) {
    std::println(
      "rel_path:{}\nrel_path {}\nfile num: {}\nflag: {}\nstatus: {}\n",
      file.old_file.relative_path,
      file.new_file.relative_path,
      file.file_num,
      git::file_flag_t_str(file.flags),
      git::delta_status_t_str(file.status));

    std::println("\ndetails: ");
    std::println("oid: {}\nsize: {}\nflag: {}\nmode: {}\n",
                 file.old_file.oid,
                 file.old_file.size,
                 git::file_flag_t_str(file.old_file.flags),
                 git::file_mode_t_str(file.old_file.mode));
    std::println("oid: {}\nsize: {}\nflag: {}\nmode: {}\n",
                 file.new_file.oid,
                 file.new_file.size,
                 git::file_flag_t_str(file.new_file.flags),
                 git::file_mode_t_str(file.new_file.mode));

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

  // auto oid = git::ref::name_to_oid(repo, "refs/heads/main");
  // std::cout << git::oid::to_str(oid) << "\n";
  // auto *obj = git::object::lookup(repo, &oid, git::object_t::commit);
  // assert(git::oid::to_str(oid) == git::oid::to_str(git::object::id(obj)));
  // auto *commit = git::convert<git::commit_ptr>(obj);
  // auto cnt     = git::commit::parent_count(commit);
  // auto sig     = git::commit::author(commit);
  // auto msg     = git::commit::message(commit);
  //
  // std::cout << git::oid::to_str(git::commit::parent_id(commit, 2)) << "\n";
  //
  // std::print("{}, {}, {}, {}, {}\n", cnt, sig.name, sig.email, sig.when.sec, msg);

  // std::print("{}, {}, {}, {}", state, path, empty, num_deltas);
  git::repo::free(repo);
  git::config::free(config);
  // git::object::free(obj);
  git::shutdown();
}
