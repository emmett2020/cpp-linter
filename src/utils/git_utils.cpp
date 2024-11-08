#include "git_utils.h"
#include "utils/util.h"
#include <git2/diff.h>
#include <git2/errors.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <iostream>
#include <print>

namespace linter::git {
int setup() { return git_libgit2_init(); }

int shutdown() { return git_libgit2_shutdown(); }

namespace repo {
repo_ptr open(const std::string &repo_path) {
  auto *repo = repo_ptr{nullptr};
  auto ret = git_repository_open(&repo, repo_path.c_str());
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return repo;
}

void free(repo_ptr repo) { git_repository_free(repo); }

int state(repo_ptr repo) { return git_repository_state(repo); }

std::string path(repo_ptr repo) {
  const auto *ret = git_repository_path(repo);
  ThrowIf(ret == nullptr, [] noexcept { return git_error_last()->message; });
  return ret;
}

bool is_empty(repo_ptr repo) {
  auto ret = git_repository_is_empty(repo);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return ret == 1;
}

repo_ptr init(const std::string &repo_path, bool is_bare) {
  auto *repo = repo_ptr{nullptr};
  auto ret = git_repository_init(&repo, repo_path.c_str(),
                                 static_cast<unsigned int>(is_bare));
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return repo;
}

config_ptr config(repo_ptr repo) {
  auto *config = config_ptr{nullptr};
  auto ret = git_repository_config(&config, repo);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return config;
}

index_ptr index(repo_ptr repo) {
  auto *index = index_ptr{nullptr};
  auto ret = git_repository_index(&index, repo);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return index;
}

} // namespace repo

namespace config {
void free(config_ptr config_ptr) { git_config_free(config_ptr); }
} // namespace config

namespace branch {
reference_ptr create(repo_ptr repo, const std::string &branch_name,
                     commit_cptr target, bool force) {
  auto *ptr = reference_ptr{nullptr};
  auto ret = git_branch_create(&ptr, repo, branch_name.c_str(), target,
                               static_cast<int>(force));
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return ptr;
}

void del(reference_ptr branch) {
  auto ret = git_branch_delete(branch);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
}

std::string_view name(reference_ptr ref) {
  const char *name = nullptr;
  auto ret = git_branch_name(&name, ref);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return name;
}

bool is_head(reference_cptr branch) {
  auto ret = git_branch_is_head(branch);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return ret == 1;
}

} // namespace branch

namespace commit {
tree_ptr tree(commit_cptr commit) {
  auto *ptr = tree_ptr{nullptr};
  auto ret = git_commit_tree(&ptr, commit);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return ptr;
}

} // namespace commit

namespace diff {
void free(diff_ptr diff) { git_diff_free(diff); }

diff_ptr index_to_workdir(repo_ptr repo, index_ptr index,
                          diff_options_cptr opts) {
  auto *ptr = diff_ptr{nullptr};
  auto ret = git_diff_index_to_workdir(&ptr, repo, index, opts);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
  return ptr;
}

void init_option(diff_options_ptr opts) {
  auto ret = git_diff_options_init(opts, GIT_DIFF_OPTIONS_VERSION);
  ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
}

std::size_t num_deltas(diff_ptr diff) { return git_diff_num_deltas(diff); }

diff_delta_cptr get_delta(diff_cptr diff, size_t idx) {
  return git_diff_get_delta(diff, idx);
}

int for_each(diff_ptr diff, diff_file_cb file_cb, diff_binary_cb binary_cb,
             diff_hunk_cb hunk_cb, diff_line_cb line_cb, void *payload) {
  return git_diff_foreach(diff, file_cb, binary_cb, hunk_cb, line_cb, payload);
}

} // namespace diff

} // namespace linter::git
