#pragma once

#include <git2.h>
#include <git2/diff.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <memory>

namespace linter::git {
  using repo_ptr            = git_repository*;
  using config_ptr          = git_config*;
  using reference_ptr       = git_reference*;
  using commit_ptr          = git_commit*;
  using diff_ptr            = git_diff*;
  using diff_options        = git_diff_options*;
  using tree_ptr            = git_tree*;
  using index_ptr           = git_index*;
  using const_repo_ptr      = const git_repository*;
  using const_config_ptr    = const git_config*;
  using const_reference_ptr = const git_reference*;
  using const_commit_ptr    = const git_commit*;
  using const_tree_ptr      = const git_tree*;
  using const_diff_ptr      = const git_diff*;
  using const_index_ptr     = const git_index*;
  using const_diff_options  = const git_diff_options*;


  /// @brief Shutdown the global state
  /// @link https://libgit2.org/libgit2/#HEAD/group/libgit2/git_libgit2_shutdown
  int shutdown();

  /// @brief Init the global state.
  /// @link https://libgit2.org/libgit2/#HEAD/group/libgit2/git_libgit2_init
  int setup();

  namespace repo {

    /// @brief Creates a new Git repository in the given folder.
    /// @param repo_path The path to the repository
    /// @param is_bare If true, a Git repository without a working directory is
    ///                created at the pointed path. If false, provided path
    ///                will be considered as the working directory into which
    ///                the .git directory will be created.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_init
    repo_ptr init(const std::string& repo_path, bool is_bare);

    /// @brief Open a git repository.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_open
    repo_ptr open(const std::string& repo_path);

    /// @brief Free a previously allocated repository
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_free
    void free(repo_ptr repo);

    /// @brief Determines the status of a git repository
    /// @link  https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_state
    int state(repo_ptr repo);

    /// @brief Get the path of this repository
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_path
    std::string path(repo_ptr repo);

    /// @brief Check if a repository is empty
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_is_empty
    bool is_empty(repo_ptr repo);

    /// @brief Get the configuration file for this repository.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_config
    config_ptr config(repo_ptr repo);
  } // namespace repo

  namespace config {
    /// @brief Free the configuration and its associated memory and files
    /// @link https://libgit2.org/libgit2/#HEAD/group/config/git_config_free
    void free(config_ptr config_ptr);

  } // namespace config

  namespace branch {
    /// @brief Create a new branch pointing at a target commit
    /// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_create
    reference_ptr
    create(repo_ptr repo, const std::string& branch_name, const_commit_ptr target, bool force);

    /// @brief Delete an existing branch reference.
    /// https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_delete
    void del(reference_ptr branch);

    /// @brief Get the branch name
    /// @return Pointer to the abbreviated reference name. Owned by ref, do not free.
    /// https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_name
    std::string_view name(reference_ptr ref);


    /// @brief Determine if HEAD points to the given branch
    /// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_is_head
    bool is_head(const_reference_ptr branch);

  } // namespace branch

  namespace commit {
    /// @brief Get the tree pointed to by a commit.
    /// https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
    tree_ptr tree(const_commit_ptr commit);


  } // namespace commit

  namespace diff {
    /// @brief Deallocate a diff.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_free
    void free(diff_ptr diff);

    ///@brief: Create a diff between the repository index and the workdir directory.
    /// @link: https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_index_to_workdir
    diff_ptr index_to_workdir(repo_ptr repo, index_ptr index, const_diff_options opts);

  } // namespace diff

} // namespace linter::git
