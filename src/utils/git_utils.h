#pragma once

#include <git2.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <memory>

namespace linter::git {

  int setup();
  int shutdown();

  using git_repo_ptr   = git_repository*;
  using git_config_ptr = git_config*;

  namespace repo {

    /// @brief Creates a new Git repository in the given folder.
    /// @param repo_path The path to the repository
    /// @param is_bare If true, a Git repository without a working directory is
    ///                created at the pointed path. If false, provided path
    ///                will be considered as the working directory into which
    ///                the .git directory will be created.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_init
    git_repo_ptr init(const std::string& repo_path, bool is_bare);

    /// @brief Open a git repository.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_open
    git_repo_ptr open(const std::string& repo_path);

    /// @brief Free a previously allocated repository
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_free
    void free(git_repo_ptr repo);

    /// @brief Determines the status of a git repository
    /// @link  https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_state
    int state(git_repo_ptr repo);

    /// @brief Get the path of this repository
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_path
    std::string path(git_repo_ptr repo);

    /// @brief Check if a repository is empty
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_is_empty
    bool is_empty(git_repo_ptr repo);

    /// @brief Get the configuration file for this repository.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_config
    git_config_ptr config(git_repo_ptr repo);
  } // namespace repo

  namespace config {
    /// @brief Free the configuration and its associated memory and files
    /// @link https://libgit2.org/libgit2/#HEAD/group/config/git_config_free
    void free(git_config_ptr config_ptr);

  } // namespace config

} // namespace linter::git
