#pragma once

#include <cstdint>
#include <cstring>
#include <git2.h>
#include <git2/diff.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <memory>
#include <vector>

/// TODO: maybe a standlone repository and add libgit2 as submodule

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

  using repo_ptr         = git_repository *;
  using config_ptr       = git_config *;
  using reference_ptr    = git_reference *;
  using commit_ptr       = git_commit *;
  using diff_ptr         = git_diff *;
  using diff_options_ptr = git_diff_options *;
  using tree_ptr         = git_tree *;
  using index_ptr        = git_index *;
  using blob_ptr         = git_blob *;
  using diff_delta_ptr   = git_diff_delta *;
  using diff_hunk_ptr    = git_diff_hunk *;
  using diff_line_ptr    = git_diff_line *;

  using repo_cptr         = const git_repository *;
  using config_cptr       = const git_config *;
  using reference_cptr    = const git_reference *;
  using commit_cptr       = const git_commit *;
  using diff_cptr         = const git_diff *;
  using diff_options_cptr = const git_diff_options *;
  using tree_cptr         = const git_tree *;
  using index_cptr        = const git_index *;
  using blob_cptr         = const blob_ptr *;
  using diff_delta_cptr   = const git_diff_delta *;
  using diff_hunk_cptr    = const git_diff_hunk *;
  using diff_line_cptr    = const git_diff_line *;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_file_cb
  using diff_file_cb = git_diff_file_cb;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_hunk_cb
  using diff_hunk_cb = git_diff_hunk_cb;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_line_cb
  using diff_line_cb = git_diff_line_cb;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_binary_cb
  using diff_binary_cb = git_diff_binary_cb;

  /// https://libgit2.org/libgit2/#HEAD/type/git_delta_t
  enum class delta_status_t : uint8_t {
    unmodified,
    added,
    deleted,
    modified,
    renamed,
    copied,
    ignored,
    untracked,
    type_change,
    unreadable,
    conflicted,
    unknown,
  };

  constexpr auto convert_to_delta_status(git_delta_t t) -> delta_status_t {
    switch (t) {
    case GIT_DELTA_UNMODIFIED: return delta_status_t::unmodified;
    case GIT_DELTA_ADDED     : return delta_status_t::added;
    case GIT_DELTA_DELETED   : return delta_status_t::deleted;
    case GIT_DELTA_MODIFIED  : return delta_status_t::modified;
    case GIT_DELTA_RENAMED   : return delta_status_t::renamed;
    case GIT_DELTA_COPIED    : return delta_status_t::copied;
    case GIT_DELTA_IGNORED   : return delta_status_t::ignored;
    case GIT_DELTA_UNTRACKED : return delta_status_t::untracked;
    case GIT_DELTA_TYPECHANGE: return delta_status_t::type_change;
    case GIT_DELTA_UNREADABLE: return delta_status_t::unreadable;
    case GIT_DELTA_CONFLICTED: return delta_status_t::conflicted;
    }
    return delta_status_t::unknown;
  }

  inline auto delta_status_str(delta_status_t status) -> std::string {
    switch (status) {
    case delta_status_t::unmodified : return "unmodified";
    case delta_status_t::added      : return "added";
    case delta_status_t::deleted    : return "deleted";
    case delta_status_t::modified   : return "modified";
    case delta_status_t::renamed    : return "renamed";
    case delta_status_t::copied     : return "copied";
    case delta_status_t::ignored    : return "ignored";
    case delta_status_t::untracked  : return "untracked";
    case delta_status_t::type_change: return "type_change";
    case delta_status_t::unreadable : return "unreadable";
    case delta_status_t::conflicted : return "conflicted";
    case delta_status_t::unknown    : return "unknown";
    }
    return "unknown";
  }


  /// https://libgit2.org/libgit2/#HEAD/type/git_diff_flag_t
  enum class diff_flag_t : uint8_t {
    binary     = 1U << 0,
    not_binary = 1U << 1,
    valid_id   = 1U << 2,
    exists     = 1U << 3,
    valid_size = 1U << 4
  };


  enum class file_mode_t : uint16_t {
    unreadable      = 0000000,
    tree            = 0040000,
    blob            = 0100644,
    blob_executable = 0100755,
    link            = 0120000,
    commit          = 0160000,
    unknown         = 0177777, // -1
  };

  constexpr auto convert_to_file_mode(uint16_t m) -> file_mode_t {
    switch (m) {
    case 0000000: return file_mode_t::unreadable;
    case 0040000: return file_mode_t::tree;
    case 0100644: return file_mode_t::blob;
    case 0100755: return file_mode_t::blob_executable;
    case 0120000: return file_mode_t::link;
    case 0160000: return file_mode_t::commit;
    default     : return file_mode_t::unknown;
    }
    return file_mode_t::unknown;
  }

  inline auto file_mode_str(file_mode_t mode) -> std::string {
    switch (mode) {
    case file_mode_t::unreadable     : return "unreadable";
    case file_mode_t::tree           : return "tree";
    case file_mode_t::blob           : return "blob";
    case file_mode_t::blob_executable: return "blob_executable";
    case file_mode_t::link           : return "link";
    case file_mode_t::commit         : return "commit";
    case file_mode_t::unknown        : return "unknown";
    }
    return "unknown";
  }

  auto file_flag_str(std::uint32_t flag) -> std::string;

  struct diff_file_detail {
    std::string oid;
    std::string relative_path;
    std::uint64_t size;
    std::uint32_t flags;
    file_mode_t mode;
  };

  auto is_same_file(const diff_file_detail &file1, const diff_file_detail &file2) -> bool;


  enum class diff_line_t : uint8_t {
    context       = ' ',
    addition      = '+',
    deletion      = '-',
    context_eofnl = '=',
    add_eofnl     = '>',
    del_eofnl     = '<',
    file_hdr      = 'F',
    hunk_hdr      = 'H',
    binary        = 'B',
  };

  struct line_details {
    diff_line_t origin;
    std::uint32_t old_lineno;
    std::uint32_t new_lineno;
    std::uint32_t num_lines;
    std::uint32_t offset_in_origin;
    std::string content;
  };

  struct diff_hunk_detail {
    std::string header;
    std::int32_t old_start;
    std::int32_t old_lines;
    std::int32_t new_start;
    std::int32_t new_lines;
    std::vector<line_details> lines;
  };

  struct diff_delta_detail {
    delta_status_t status;
    std::uint32_t flags;
    std::uint16_t similarity;
    std::uint16_t file_num;
    diff_file_detail old_file;
    diff_file_detail new_file;
    std::vector<diff_hunk_detail> hunks;
  };

  /// @brief Init the global state.
  /// @link https://libgit2.org/libgit2/#HEAD/group/libgit2/git_libgit2_init
  auto setup() -> int;

  /// @brief Shutdown the global state
  /// @link https://libgit2.org/libgit2/#HEAD/group/libgit2/git_libgit2_shutdown
  auto shutdown() -> int;

  namespace repo {
    /// @brief Creates a new Git repository in the given folder.
    /// @param repo_path The path to the repository
    /// @param is_bare If true, a Git repository without a working directory is
    ///                created at the pointed path. If false, provided path
    ///                will be considered as the working directory into which
    ///                the .git directory will be created.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_init
    auto init(const std::string &repo_path, bool is_bare) -> repo_ptr;

    /// @brief Open a git repository.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_open
    auto open(const std::string &repo_path) -> repo_ptr;

    /// @brief Free a previously allocated repository
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_free
    void free(repo_ptr repo);

    /// @brief Determines the status of a git repository
    /// @link
    /// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_state
    auto state(repo_ptr repo) -> int;

    /// @brief Get the path of this repository
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_path
    auto path(repo_ptr repo) -> std::string;

    /// @brief Check if a repository is empty
    /// @link
    /// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_is_empty
    auto is_empty(repo_ptr repo) -> bool;

    /// @brief Get the configuration file for this repository.
    /// @link
    /// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_config
    auto config(repo_ptr repo) -> config_ptr;

    /// @brief Get the Index file for this repository.
    /// @link
    /// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_index
    auto index(repo_ptr repo) -> index_ptr;
  } // namespace repo

  namespace config {
    /// @brief Free the configuration and its associated memory and files
    /// @link https://libgit2.org/libgit2/#HEAD/group/config/git_config_free
    void free(config_ptr config_ptr);

  } // namespace config

  namespace branch {
    /// @brief Create a new branch pointing at a target commit
    /// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_create
    auto create(repo_ptr repo, const std::string &branch_name, commit_cptr target, bool force)
      -> reference_ptr;

    /// @brief Delete an existing branch reference.
    /// https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_delete
    void del(reference_ptr branch);

    /// @brief Get the branch name
    /// @return Pointer to the abbreviated reference name. Owned by ref, do not
    /// free. https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_name
    auto name(reference_ptr ref) -> std::string_view;

    /// @brief Determine if HEAD points to the given branch
    /// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_is_head
    bool is_head(reference_cptr branch);

  } // namespace branch

  namespace commit {
    /// @brief Get the tree pointed to by a commit.
    /// https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
    auto tree(commit_cptr commit) -> tree_ptr;

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
    auto index_to_workdir(repo_ptr repo, index_ptr index, diff_options_cptr opts) -> diff_ptr;

    /// @brief Initialize diff options structure
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_options_init
    void init_option(diff_options_ptr opts);

    /// @brief Query how many diff records are there in a diff.
    /// @link  https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_num_deltas
    auto num_deltas(diff_ptr diff) -> std::size_t;

    /// @brief Return the diff delta for an entry in the diff list.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_get_delta
    auto get_delta(diff_cptr diff, size_t idx) -> diff_delta_cptr;

    /// @brief Loop over all deltas in a diff issuing callbacks.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_foreach
    auto for_each(
      diff_ptr diff,
      diff_file_cb file_cb,
      diff_binary_cb binary_cb,
      diff_hunk_cb hunk_cb,
      diff_line_cb line_cb,
      void *payload) -> int;

    /// @brief A simple implmentation which uses for_each to get diff details.
    auto details(diff_ptr diff) -> std::vector<diff_delta_detail>;

  } // namespace diff

  namespace oid {
    /// @brief Format a git_oid into a buffer as a hex format c-string.
    /// @link https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_tostr
    auto to_str(git_oid oid) -> std::string;

    /// @brief Compare two oid structures for equality
    /// @link https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_equal
    auto equal(git_oid o1, git_oid o2) -> bool;

  } // namespace oid

} // namespace linter::git
