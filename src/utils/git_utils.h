#pragma once

#include <git2.h>
#include <git2/diff.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <memory>

namespace linter::git {
/// https://libgit2.org/libgit2/#HEAD/type/git_diff_file
using diff_file = git_diff_file;

/// https://libgit2.org/libgit2/#HEAD/type/git_diff_delta
using diff_delta = git_diff_delta;

/// https://libgit2.org/libgit2/#HEAD/type/git_diff_options
using diff_options = git_diff_options;

/// https://libgit2.org/libgit2/#HEAD/type/git_diff_line
using diff_line = git_diff_line;

/// https://libgit2.org/libgit2/#HEAD/type/git_diff_hunk
using diff_hunk = git_diff_hunk;

using repo_ptr = git_repository *;
using repo_cptr = const git_repository *;
using config_ptr = git_config *;
using config_cptr = const git_config *;
using reference_ptr = git_reference *;
using reference_cptr = const git_reference *;
using commit_ptr = git_commit *;
using commit_cptr = const git_commit *;
using diff_ptr = git_diff *;
using diff_cptr = const git_diff *;
using diff_options_ptr = git_diff_options *;
using diff_options_cptr = const git_diff_options *;
using tree_ptr = git_tree *;
using tree_cptr = const git_tree *;
using index_ptr = git_index *;
using index_cptr = const git_index *;
using blob_ptr = git_blob *;
using blob_cptr = const blob_ptr *;
using diff_delta_ptr = git_diff_delta *;
using diff_delta_cptr = const git_diff_delta *;

/// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_file_cb
using diff_file_cb = git_diff_file_cb;

/// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_hunk_cb
using diff_hunk_cb = git_diff_hunk_cb;

/// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_line_cb
using diff_line_cb = git_diff_line_cb;

/// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_binary_cb
using diff_binary_cb = git_diff_binary_cb;

/// TODO: maybe a standlone repository and add libgit2 as submodule

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
repo_ptr init(const std::string &repo_path, bool is_bare);

/// @brief Open a git repository.
/// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_open
repo_ptr open(const std::string &repo_path);

/// @brief Free a previously allocated repository
/// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_free
void free(repo_ptr repo);

/// @brief Determines the status of a git repository
/// @link
/// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_state
int state(repo_ptr repo);

/// @brief Get the path of this repository
/// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_path
std::string path(repo_ptr repo);

/// @brief Check if a repository is empty
/// @link
/// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_is_empty
bool is_empty(repo_ptr repo);

/// @brief Get the configuration file for this repository.
/// @link
/// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_config
config_ptr config(repo_ptr repo);

/// @brief Get the Index file for this repository.
/// @link
/// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_index
index_ptr index(repo_ptr repo);
} // namespace repo

namespace config {
/// @brief Free the configuration and its associated memory and files
/// @link https://libgit2.org/libgit2/#HEAD/group/config/git_config_free
void free(config_ptr config_ptr);

} // namespace config

namespace branch {
/// @brief Create a new branch pointing at a target commit
/// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_create
reference_ptr create(repo_ptr repo, const std::string &branch_name,
                     commit_cptr target, bool force);

/// @brief Delete an existing branch reference.
/// https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_delete
void del(reference_ptr branch);

/// @brief Get the branch name
/// @return Pointer to the abbreviated reference name. Owned by ref, do not
/// free. https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_name
std::string_view name(reference_ptr ref);

/// @brief Determine if HEAD points to the given branch
/// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_is_head
bool is_head(reference_cptr branch);

} // namespace branch

namespace commit {
/// @brief Get the tree pointed to by a commit.
/// https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
tree_ptr tree(commit_cptr commit);

} // namespace commit

/// @brief
/// @link

namespace diff {
/// @brief Deallocate a diff.
/// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_free
void free(diff_ptr diff);

/// @brief: Create a diff between the repository index and the workdir
/// directory.
/// @link:
/// https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_index_to_workdir
diff_ptr index_to_workdir(repo_ptr repo, index_ptr index,
                          diff_options_cptr opts);

/// @brief Initialize diff options structure
/// @link https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_options_init
void init_option(diff_options_ptr opts);

/// @brief Query how many diff records are there in a diff.
/// @link  https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_num_deltas
std::size_t num_deltas(diff_ptr diff);

/// @brief Return the diff delta for an entry in the diff list.
/// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_get_delta
diff_delta_cptr get_delta(diff_cptr diff, size_t idx);

/// @brief Loop over all deltas in a diff issuing callbacks.
/// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_foreach
int for_each(diff_ptr diff, diff_file_cb file_cb, diff_binary_cb binary_cb,
             diff_hunk_cb hunk_cb, diff_line_cb line_cb, void *payload);

} // namespace diff

} // namespace linter::git
