#include "git_utils.h"
#include "utils/util.h"
#include <git2/errors.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <iostream>

namespace linter::git {
  int setup() {
    return git_libgit2_init();
  }

  int shutdown() {
    return git_libgit2_shutdown();
  }

  namespace repo {
    repo_ptr open(const std::string& repo_path) {
      auto* repo = repo_ptr{nullptr};
      auto ec    = git_repository_open(&repo, repo_path.c_str());
      ThrowIf(ec < 0, git_error_last()->message);
      return repo;
    }

    void free(repo_ptr repo) {
      git_repository_free(repo);
    }

    int state(repo_ptr repo) {
      return git_repository_state(repo);
    }

    std::string path(repo_ptr repo) {
      const auto* ret = git_repository_path(repo);
      ThrowIf(ret == nullptr, git_error_last()->message);
      return ret;
    }

    bool is_empty(repo_ptr repo) {
      auto ret = git_repository_is_empty(repo);
      ThrowIf(ret < 0, "git repository is corrupted");
      return ret == 1;
    }

    repo_ptr init(const std::string& repo_path, bool is_bare) {
      auto* repo = repo_ptr{nullptr};
      auto ec = git_repository_init(&repo, repo_path.c_str(), static_cast<unsigned int>(is_bare));
      ThrowIf(ec < 0, git_error_last()->message);
      return repo;
    }

    config_ptr config(repo_ptr repo) {
      auto* config = config_ptr{nullptr};
      auto ec      = git_repository_config(&config, repo);
      ThrowIf(ec < 0, git_error_last()->message);
      return config;
    }


  } // namespace repo

  namespace config {
    void free(config_ptr config_ptr) {
      git_config_free(config_ptr);
    }
  } // namespace config

  namespace branch {
    reference_ptr
    create(repo_ptr repo, const std::string& branch_name, const_commit_ptr target, bool force) {
      auto* ptr = reference_ptr{nullptr};
      auto ec = git_branch_create(&ptr, repo, branch_name.c_str(), target, static_cast<int>(force));
      ThrowIf(ec < 0, git_error_last()->message);
      return ptr;
    }

    void del(reference_ptr branch) {
      auto ec = git_branch_delete(branch);
      ThrowIf(ec < 0, git_error_last()->message);
    }

    std::string_view name(reference_ptr ref) {
      const char* name = nullptr;
      auto ec          = git_branch_name(&name, ref);
      ThrowIf(ec < 0, git_error_last()->message);
      return name;
    }

    bool is_head(const_reference_ptr branch) {
      auto ret = git_branch_is_head(branch);
      ThrowIf(ret < 0, git_error_last()->message);
      return ret == 1;
    }

  } // namespace branch

  namespace commit {
    tree_ptr tree(const_commit_ptr commit) {
      auto* ptr = tree_ptr{nullptr};
      auto ec   = git_commit_tree(&ptr, commit);
      ThrowIf(ec < 0, git_error_last()->message);
      return ptr;
    }

  } // namespace commit

  namespace diff {
    void free(diff_ptr diff) {
      git_diff_free(diff);
    }

    diff_ptr index_to_workdir(repo_ptr repo, index_ptr index, const_diff_options opts) {
      auto* ptr = diff_ptr{nullptr};
      return ptr;
    }
  } // namespace diff


} // namespace linter::git
