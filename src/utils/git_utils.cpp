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
    git_repo_ptr open(const std::string& repo_path) {
      auto* repo = git_repo_ptr{nullptr};
      auto ec    = git_repository_open(&repo, repo_path.c_str());
      ThrowIf(ec < 0, git_error_last()->message);
      return repo;
    }

    void free(git_repo_ptr repo) {
      git_repository_free(repo);
    }

    int state(git_repo_ptr repo) {
      return git_repository_state(repo);
    }

    std::string path(git_repo_ptr repo) {
      const auto* ret = git_repository_path(repo);
      ThrowIf(ret == nullptr, git_error_last()->message);
      return ret;
    }

    bool is_empty(git_repo_ptr repo) {
      auto ret = git_repository_is_empty(repo);
      ThrowIf(ret < 0, "git repository is corrupted");
      return ret == 1;
    }

    git_repo_ptr init(const std::string& repo_path, bool is_bare) {
      auto* repo = git_repo_ptr{nullptr};
      auto ec = git_repository_init(&repo, repo_path.c_str(), static_cast<unsigned int>(is_bare));
      ThrowIf(ec < 0, git_error_last()->message);
      return repo;
    }

    git_config_ptr config(git_repo_ptr repo) {
      auto* config = git_config_ptr{nullptr};
      auto ec      = git_repository_config(&config, repo);
      ThrowIf(ec < 0, git_error_last()->message);
      return config;
    }


  } // namespace repo

  namespace config {
    void free(git_config_ptr config_ptr) {
      git_config_free(config_ptr);
    }
  } // namespace config


} // namespace linter::git
