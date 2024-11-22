#pragma once

#include <cstdint>
#include <cstring>
#include <git2/repository.h>
#include <git2/status.h>
#include <string>
#include <utility>
#include <vector>

#include <git2.h>
#include <git2/deprecated.h>
#include <git2/types.h>

#include "utils/util.h"

/// TODO: Maybe a standlone repository with libgit2 as submodule
/// TODO: Could we use unique_ptr and automaticly free the allocated pointer?

/// This is based on v1.8.4

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

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_file_cb
  using diff_file_cb = git_diff_file_cb;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_hunk_cb
  using diff_hunk_cb = git_diff_hunk_cb;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_line_cb
  using diff_line_cb = git_diff_line_cb;

  /// https://libgit2.org/libgit2/#HEAD/group/callback/git_diff_binary_cb
  using diff_binary_cb = git_diff_binary_cb;

  /// In-memory representation of a file entry in the index.
  using index_entry    = git_index_entry;
  using status_options = git_status_options;
  using status_entry   = git_status_entry;

  using repo_ptr        = std::unique_ptr<git_repository, decltype(::git_repository_free) *>;
  using config_ptr      = std::unique_ptr<git_config, decltype(::git_config_free) *>;
  using ref_ptr         = std::unique_ptr<git_reference, decltype(::git_reference_free) *>;
  using commit_ptr      = std::unique_ptr<git_commit, decltype(::git_commit_free) *>;
  using diff_ptr        = std::unique_ptr<git_diff, decltype(::git_diff_free) *>;
  using tree_ptr        = std::unique_ptr<git_tree, decltype(::git_tree_free) *>;
  using index_ptr       = std::unique_ptr<git_index, decltype(::git_index_free) *>;
  using blob_ptr        = std::unique_ptr<git_blob, decltype(::git_blob_free) *>;
  using tag_ptr         = std::unique_ptr<git_tag, decltype(::git_tag_free) *>;
  using object_ptr      = std::unique_ptr<git_object, decltype(::git_object_free) *>;
  using signature_ptr   = std::unique_ptr<git_signature, decltype(::git_signature_free) *>;
  using status_list_ptr = std::unique_ptr<git_status_list, decltype(::git_status_list_free) *>;

  using repo_raw_ptr         = git_repository *;
  using config_raw_ptr       = git_config *;
  using ref_raw_ptr          = git_reference *;
  using commit_raw_ptr       = git_commit *;
  using diff_raw_ptr         = git_diff *;
  using diff_options_raw_ptr = git_diff_options *;
  using tree_raw_ptr         = git_tree *;
  using index_raw_ptr        = git_index *;
  using blob_raw_ptr         = git_blob *;
  using tag_raw_ptr          = git_tag *;
  using diff_delta_raw_ptr   = git_diff_delta *;
  using diff_hunk_raw_ptr    = git_diff_hunk *;
  using diff_line_raw_ptr    = git_diff_line *;
  using object_raw_ptr       = git_object *;
  using oid_raw_ptr          = git_oid *;
  using signature_raw_ptr    = git_signature *;
  using status_list_raw_ptr  = git_status_list *;
  using status_entry_raw_ptr = git_status_entry *;

  using repo_raw_cptr         = const git_repository *;
  using config_raw_cptr       = const git_config *;
  using ref_raw_cptr          = const git_reference *;
  using commit_raw_cptr       = const git_commit *;
  using diff_raw_cptr         = const git_diff *;
  using diff_options_raw_cptr = const git_diff_options *;
  using tree_raw_cptr         = const git_tree *;
  using index_raw_cptr        = const git_index *;
  using blob_raw_cptr         = const blob_raw_ptr *;
  using tag_raw_cptr          = const git_tag *;
  using diff_delta_raw_cptr   = const git_diff_delta *;
  using diff_hunk_raw_cptr    = const git_diff_hunk *;
  using diff_line_raw_cptr    = const git_diff_line *;
  using object_raw_cptr       = const git_object *;
  using oid_raw_cptr          = const git_oid *;
  using signature_raw_cptr    = const git_signature *;
  using status_list_raw_cptr  = const git_status_list *;
  using status_entry_raw_cptr = const git_status_entry *;

  using status_t = ::git_status_t;

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
    unknown       = '?',
  };

  enum class ref_t : uint8_t {
    invalid,
    direct, // OID
    symbolic,
    all,
  };

  enum class branch_t : uint8_t {
    local,
    remote,
    all
  };

  enum class object_t : uint8_t {
    any,
    bad,
    commit,
    tree,
    blob,
    tag,
  };

  enum class repo_state_t : uint8_t {
    none,
    merge,
    revert,
    revert_sequence,
    cherrypick,
    cherrypick_sequence,
    bisect,
    rebase,
    rebase_interactive,
    rebase_merge,
    apply_mailbox,
    apply_mailbox_or_rebase
  };

  struct diff_line_detail {
    diff_line_t origin;
    std::int64_t old_lineno;
    std::int64_t new_lineno;
    std::int64_t num_lines;
    std::int64_t offset_in_origin;
    std::string content;
  };

  struct diff_hunk_detail {
    std::string header;
    std::int32_t old_start;
    std::int32_t old_lines;
    std::int32_t new_start;
    std::int32_t new_lines;
    std::vector<diff_line_detail> lines;
  };

  struct diff_file_detail {
    std::string oid;
    std::string relative_path;
    std::uint64_t size;
    std::uint32_t flags;
    file_mode_t mode;
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

  struct time {
    std::int64_t sec;
    int offset;
  };

  struct signature {
    std::string name;
    std::string email;
    time when;
  };

  constexpr auto convert_to_delta_status_t(git_delta_t t) -> delta_status_t {
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

  constexpr auto convert_to_file_mode_t(uint16_t m) -> file_mode_t {
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

  constexpr auto convert_to_diff_line_t(char m) -> diff_line_t {
    switch (m) {
    case ' ': return diff_line_t::context;
    case '+': return diff_line_t::addition;
    case '-': return diff_line_t::deletion;
    case '=': return diff_line_t::context_eofnl;
    case '>': return diff_line_t::add_eofnl;
    case '<': return diff_line_t::del_eofnl;
    case 'F': return diff_line_t::file_hdr;
    case 'H': return diff_line_t::hunk_hdr;
    case 'B': return diff_line_t::binary;
    default : return diff_line_t::unknown;
    }
    return diff_line_t::unknown;
  }

  constexpr auto convert_to_object_t(git_otype tp) -> object_t {
    switch (tp) {
    case GIT_OBJ_ANY   : return object_t::any;
    case GIT_OBJ_BAD   : return object_t::bad;
    case GIT_OBJ_TAG   : return object_t::tag;
    case GIT_OBJ_BLOB  : return object_t::blob;
    case GIT_OBJ_TREE  : return object_t::tree;
    case GIT_OBJ_COMMIT: return object_t::commit;
    default            : return object_t::any;
    }
    return object_t::any;
  }

  constexpr auto convert_to_git_otype(object_t tp) -> git_otype {
    switch (tp) {
    case object_t::any   : return GIT_OBJ_ANY;
    case object_t::bad   : return GIT_OBJ_BAD;
    case object_t::tag   : return GIT_OBJ_TAG;
    case object_t::blob  : return GIT_OBJ_BLOB;
    case object_t::tree  : return GIT_OBJ_TREE;
    case object_t::commit: return GIT_OBJ_COMMIT;
    default              : return GIT_OBJ_ANY;
    }
    return GIT_OBJ_ANY;
  }

  constexpr auto convert_to_repo_state(int state) -> repo_state_t {
    switch (state) {
    case GIT_REPOSITORY_STATE_NONE                   : return repo_state_t::none;
    case GIT_REPOSITORY_STATE_MERGE                  : return repo_state_t::merge;
    case GIT_REPOSITORY_STATE_REVERT                 : return repo_state_t::revert;
    case GIT_REPOSITORY_STATE_REVERT_SEQUENCE        : return repo_state_t::revert_sequence;
    case GIT_REPOSITORY_STATE_CHERRYPICK             : return repo_state_t::cherrypick;
    case GIT_REPOSITORY_STATE_CHERRYPICK_SEQUENCE    : return repo_state_t::cherrypick_sequence;
    case GIT_REPOSITORY_STATE_BISECT                 : return repo_state_t::bisect;
    case GIT_REPOSITORY_STATE_REBASE                 : return repo_state_t::rebase;
    case GIT_REPOSITORY_STATE_REBASE_INTERACTIVE     : return repo_state_t::rebase_interactive;
    case GIT_REPOSITORY_STATE_REBASE_MERGE           : return repo_state_t::rebase_merge;
    case GIT_REPOSITORY_STATE_APPLY_MAILBOX          : return repo_state_t::apply_mailbox;
    case GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE: return repo_state_t::apply_mailbox_or_rebase;
    default                                          : std::unreachable();
    }
    std::unreachable();
  }

  auto convert_to_signature(signature_raw_cptr sig) -> signature;

  auto is_same_file(const diff_file_detail &file1, const diff_file_detail &file2) -> bool;
  auto delta_status_t_str(delta_status_t status) -> std::string;
  auto file_mode_t_str(file_mode_t mode) -> std::string;
  auto file_flag_t_str(std::uint32_t flag) -> std::string;
  auto diff_line_t_str(diff_line_t tp) -> std::string;
  auto ref_t_str(ref_t tp) -> std::string;
  auto branch_t_str(branch_t tp) -> std::string;
  auto object_t_str(object_t tp) -> std::string;
  auto repo_state_str(repo_state_t state) -> std::string;


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

    /// @brief Free a previously allocated repository. If you use repo_ptr instead
    /// of repo_raw_ptr, you didn't need to explicitly call this function.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_free
    void free(repo_raw_ptr repo);

    /// @brief Determines the status of a git repository - ie, whether an operation
    /// (merge, cherry-pick, etc) is in progress.
    auto state(repo_raw_ptr repo) -> repo_state_t;

    /// @brief Get the path of this repository. This is the path of the .git folder
    /// for normal repositories, or of the repository itself for bare repositories.
    /// @link https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_path
    auto path(repo_raw_ptr repo) -> std::string;

    /// @brief Check if a repository is empty
    /// @link
    /// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_is_empty
    auto is_empty(repo_raw_ptr repo) -> bool;

    /// Get the configuration file for this repository.
    /// If a configuration file has not been set, the default config set for
    /// the repository will be returned, including global and system configurations
    /// (if they are available).
    auto config(repo_raw_ptr repo) -> config_ptr;

    /// Get a snapshot of the repository's configuration.
    /// Convenience function to take a snapshot from the repository's
    /// configuration. The contents of this snapshot will not change, even if the
    /// underlying config files are modified.
    auto config_snapshot(repo_raw_ptr repo) -> config_ptr;

    /// Get the Index file for this repository. If a custom index has not been
    /// set, the default index for the repository will be returned (the one located
    /// in .git/index).
    auto index(repo_raw_ptr repo) -> index_ptr;

    /// Retrieve and resolve the reference pointed at by HEAD.
    auto head(repo_raw_ptr repo) -> ref_ptr;
  } // namespace repo

  namespace config {
    /// Free the configuration and its associated memory and files
    void free(config_raw_ptr config_ptr);

    /// Get the value of a string config variable.
    /// This function can only be used on snapshot config objects. The string
    /// is owned by the config and should not be freed by the user. The pointer
    /// will be valid until the config is freed. All config files will be looked
    /// into, in the order of their defined level. A higher level means a higher
    /// priority. The first occurrence of the variable will be returned here.
    auto get_string(config_raw_cptr config_ptr, const std::string &key) -> std::string;

    /// Get the value of a boolean config variable.
    /// This function uses the usual C convention of 0 being false and anything
    /// else true. All config files will be looked into, in the order of their
    /// defined level. A higher level means a higher priority. The first occurrence
    /// of the variable will be returned here.
    auto get_bool(config_raw_cptr config_ptr, const std::string &key) -> bool;

    /// Get the value of an integer config variable.
    /// All config files will be looked into, in the order of their defined
    /// level. A higher level means a higher priority. The first occurrence of the
    /// variable will be returned here.
    auto get_int32(config_raw_cptr config_ptr, const std::string &key) -> int32_t;

    /// Get the value of a long integer config variable.
    /// All config files will be looked into, in the order of their defined
    /// level. A higher level means a higher priority. The first occurrence of the
    /// variable will be returned here.
    auto get_int64(config_raw_cptr config_ptr, const std::string &key) -> int64_t;

    /// Set the value of a string config variable in the config file with the
    /// highest level (usually the local one).
    void set_string(config_raw_ptr config_ptr, const std::string &key, const std::string &value);

    /// Set the value of a boolean config variable in the config file with the
    /// highest level (usually the local one).
    void set_bool(config_raw_ptr config_ptr, const std::string &key, bool value);

    /// Set the value of an integer config variable in the config file with the
    /// highest level (usually the local one).
    void set_int32(config_raw_ptr config_ptr, const std::string &key, int32_t value);

    /// Set the value of a long integer config variable in the config file with
    /// the highest level (usually the local one).
    void set_int64(config_raw_ptr config_ptr, const std::string &key, int64_t value);

    /// Create a snapshot of the configuration
    /// Create a snapshot of the current state of a configuration, which allows
    /// you to look into a consistent view of the configuration for looking up
    /// complex values (e.g. a remote, submodule). The string returned when
    /// querying such a config object is valid until it is freed.
    /// All the modification will be wrote back into configuration file when
    /// you use git_config while snapshot won't.
    auto snapshot(config_raw_ptr config) -> config_ptr;

  } // namespace config

  namespace branch {
    /// Create a new branch pointing at a target commit.
    /// A new direct reference will be created pointing to this target commit.
    /// If force is true and a reference already exists with the given name, it'll
    /// be replaced.
    /// @param repo the repository to create the branch in.
    /// @param 	branch_name Name for the branch; this name is validated for
    /// consistency. It should also not conflict with an already existing branch
    /// name.
    /// @param target Commit to which this branch should point. This object
    /// must belong to the given `repo`.
    /// @param force Overwrite existing branch.
    auto create(repo_raw_ptr repo,
                const std::string &branch_name,
                commit_raw_cptr target,
                bool force) -> ref_ptr;

    /// @brief Delete an existing branch reference.
    /// https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_delete
    void del(ref_raw_ptr branch);

    /// @brief Get the branch name
    /// @return Pointer to the abbreviated reference name. Owned by ref, do not
    /// free. https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_name
    auto name(ref_raw_ptr ref) -> std::string_view;

    /// @brief Determine if HEAD points to the given branch
    /// @link https://libgit2.org/libgit2/#HEAD/group/branch/git_branch_is_head
    bool is_head(ref_raw_cptr branch);

    /// Lookup a branch by its name in a repository.
    auto lookup(repo_raw_ptr repo, const std::string &name, branch_t branch_type) -> ref_ptr;

    /// Get the current branch name indicated by HEAD reference.
    auto current_name(repo_raw_ptr repo) -> std::string;
  } // namespace branch

  namespace commit {
    /// Create a commit object from the given buffer and signature
    /// Given the unsigned commit object's contents, its signature and the
    /// header field in which to store the signature, attach the signature to the
    /// commit and write it into the given repository.
    /// TODO:
    auto create_with_signature(repo_raw_ptr repo, const std::string &commit_content) -> git_oid;

    /// Create new commit in the repository from a list of git_object pointers.
    /// @param repo Repository where to store the commit
    /// @param update_ref If not NULL, name of the reference that will be
    /// updated to point to this commit. If the reference is not direct, it will be
    /// resolved to a direct reference. Use "HEAD" to update the HEAD of the
    /// current branch and make it point to this commit. If the reference doesn't
    /// exist yet, it will be created. If it does exist, the first parent must be
    /// the tip of this branch.
    /// @param author Signature with author and author time of commit
    /// @param committer Signature with committer and * commit time of commit
    /// @param message Full message for this commit
    /// @param tree An instance of a `git_tree` object that will be used as the
    /// tree for the commit. This tree object must also be owned by the given
    /// `repo`.
    /// @param parent_count Number of parents for this commit
    /// @param parents Array of `parent_count` pointers to `git_commit` objects
    /// that will be used as the parents for this commit. This array may be NULL if
    /// `parent_count` is 0 (root commit). All the given commits must be owned by
    /// the `repo`.
    auto create(
      repo_raw_ptr repo,
      const std::string &updated_ref,
      signature_raw_cptr author,
      signature_raw_cptr committer,
      const std::string &message,
      tree_raw_cptr tree,
      std::size_t parent_count,
      std::span<commit_raw_cptr> parents) -> git_oid;

    /// Create a new commit and set it to HEAD with default signature.
    auto create_head(repo_raw_ptr repo, const std::string &message, tree_raw_cptr index_tree)
      -> std::tuple<git_oid, commit_ptr>;

    /// @brief Get the tree pointed to by a commit.
    /// https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
    auto tree(commit_raw_cptr commit) -> tree_raw_ptr;

    /// @brief Get the id of the tree pointed to by a commit. This differs from
    /// git_commit_tree in that no attempts are made to fetch an object from the
    /// ODB.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_tree_id
    auto tree_id(commit_raw_cptr commit) -> oid_raw_cptr;

    /// @brief Lookup a commit object from a repository.
    auto lookup(repo_raw_ptr repo, oid_raw_cptr id) -> commit_ptr;

    /// @brief Get the author of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_author
    auto author(commit_raw_cptr commit) -> signature;

    /// @brief Get the committer of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_committer
    auto committer(commit_raw_cptr commit) -> signature;

    /// @brief Get the commit time (i.e. committer time) of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_time
    auto time(commit_raw_cptr commit) -> int64_t;

    /// @brief Get the full message of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_message
    auto message(commit_raw_cptr commit) -> std::string;

    /// @brief Get the commit object that is the <n
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_nth_gen_ancestor
    auto nth_gen_ancestor(commit_raw_cptr commit, std::uint32_t n) -> commit_raw_ptr;

    /// @brief Get the specified parent of the commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_parent
    auto parent(commit_raw_cptr commit, std::uint32_t n) -> commit_raw_ptr;

    /// @brief Get the oid of a specified parent for a commit. This is
    /// different from git_commit_parent, which will attempt to load the parent
    /// commit from the ODB.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_parent_id
    auto parent_id(commit_raw_cptr commit, std::uint32_t n) -> oid_raw_cptr;

    /// @brief Get the number of parents of this commit
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_parentcount
    auto parent_count(commit_raw_cptr commit) -> std::uint32_t;

    void free(commit_raw_ptr commit);

  } // namespace commit

  namespace diff {
    /// @brief Deallocate a diff.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_free
    void free(diff_raw_ptr diff);

    /// @brief: Create a diff between the repository index and the workdir
    /// directory.
    /// @link:
    /// https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_index_to_workdir
    auto index_to_workdir(repo_raw_ptr repo, index_raw_ptr index, diff_options_raw_cptr opts)
      -> diff_raw_ptr;

    /// @brief: Create a diff with the difference between two tree objects.
    /// @link: https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_tree_to_tree
    auto tree_to_tree(
      repo_raw_ptr repo,
      tree_raw_ptr old_tree,
      tree_raw_ptr new_tree,
      diff_options_raw_cptr opts) -> diff_raw_ptr;

    /// @brief Initialize diff options structure
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_options_init
    void init_option(diff_options_raw_ptr opts);

    /// @brief Query how many diff records are there in a diff.
    /// @link  https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_num_deltas
    auto num_deltas(diff_raw_ptr diff) -> std::size_t;

    /// @brief Return the diff delta for an entry in the diff list.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_get_delta
    auto get_delta(diff_raw_cptr diff, size_t idx) -> diff_delta_raw_cptr;

    /// @brief Loop over all deltas in a diff issuing callbacks.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_foreach
    auto for_each(
      diff_raw_ptr diff,
      diff_file_cb file_cb,
      diff_binary_cb binary_cb,
      diff_hunk_cb hunk_cb,
      diff_line_cb line_cb,
      void *payload) -> int;

    /// @brief A simple implmentation which uses for_each to get diff delta details.
    auto deltas(diff_raw_ptr diff) -> std::vector<diff_delta_detail>;

    /// @brief A simple implmentation which compares ref1 with ref2's differences.
    auto deltas(repo_raw_ptr repo, const std::string &ref1, const std::string &ref2)
      -> std::vector<git::diff_delta_detail>;

    /// @brief Get changed files between two refs.
    /// @return The modified and new added file names in source reference.
    auto changed_files(repo_raw_ptr repo,
                       const std::string &target_ref,
                       const std::string &source_ref) -> std::vector<std::string>;

  } // namespace diff

  namespace oid {
    /// @brief Format a git_oid into a buffer as a hex format c-string.
    /// @link https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_tostr
    auto to_str(const git_oid &oid) -> std::string;
    auto to_str(oid_raw_cptr oid_ptr) -> std::string;

    /// @brief Compare two oid structures for equality
    /// @link https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_equal
    auto equal(git_oid o1, git_oid o2) -> bool;

    /// @brief Parse a hex formatted object id into a git_oid.
    /// https://libgit2.org/libgit2/#v0.20.0/group/oid/git_oid_fromstr
    auto from_str(const std::string &str) -> git_oid;
  } // namespace oid

  namespace ref {
    /// @brief Get the type of a reference.
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_type
    auto type(ref_raw_cptr ref) -> ref_t;

    /// @brief Check if a reference is a local branch. That's to say, the
    /// reference lives in the refs/heads namespace.
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_is_branch
    auto is_branch(ref_raw_ptr ref) -> bool;

    /// @brief Check if a reference is a remote tracking branch
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_is_remote
    auto is_remote(ref_raw_ptr ref) -> bool;

    /// @brief Check if a reference is a tag
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_is_tag
    auto is_tag(ref_raw_ptr ref) -> bool;

    /// @brief Free the given reference.
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_free
    void free(ref_raw_ptr ref);

    /// @brief Get the full name of a reference. E.g. refs/heads/main
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_name
    auto name(ref_raw_cptr ref) -> std::string;

    /// @brief: Lookup a reference by name in a repository.
    /// @param name: the long name for the reference (e.g. HEAD, refs/heads/master,
    /// refs/tags/v0.1.0, ...)
    /// @link:
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_lookup
    auto lookup(repo_raw_ptr repo, const std::string &name) -> ref_raw_ptr;

    /// @brief: Lookup a reference by name and resolve immediately to OID.
    /// @param name: the long name for the reference (e.g. HEAD, refs/heads/master,
    /// refs/tags/v0.1.0, ...)
    /// @link:
    /// https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_name_to_id
    auto name_to_oid(repo_raw_ptr repo, const std::string &name) -> git_oid;

    /// Get the reference's short name. This will transform the reference name
    /// into a name "human-readable" version. If no shortname is appropriate, it
    /// will return the full name.
    auto shorthand(ref_raw_cptr ref) -> std::string;

    /// Resolve a symbolic reference to a direct reference.
    /// If a direct reference is passed as an argument, a copy of that
    /// reference is returned.
    auto resolve(ref_raw_cptr symbolic_ref) -> ref_ptr;

  } // namespace ref

  namespace revparse {
    /// @brief Find a single object, as specified by a revision string.
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/revparse/git_revparse_single
    /// https://git-scm.com/docs/git-rev-parse.html#_specifying_revisions
    auto single(repo_raw_ptr repo, const std::string &spec) -> object_ptr;

    /// @brief Find a complete sha based on given short sha
    auto complete_sha(repo_raw_ptr repo, const std::string &short_sha) -> std::string;

  }; // namespace revparse

  namespace object {
    /// @brief Close an open object
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_free
    void free(object_raw_ptr obj);

    /// @brief Get the object type of an object
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_type
    auto type(object_raw_cptr obj) -> object_t;

    /// @brief Get the id (SHA1) of a repository object
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_id
    auto id(object_raw_cptr obj) -> oid_raw_cptr;

    /// Get the id (SHA1) string of given repository object.
    auto id_str(object_raw_cptr obj) -> std::string;

    /// @brief Lookup a reference to one of the objects in a repository.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_lookup
    auto lookup(repo_raw_ptr repo, oid_raw_cptr oid, object_t type) -> object_raw_ptr;

  } // namespace object

  template <typename T>
  auto convert(object_raw_ptr obj) -> T {
    auto type = object::type(obj);
    if constexpr (std::same_as<std::decay_t<T>, commit_raw_ptr>) {
      throw_if(type != object_t::commit, "The given object isn't git_commit*");
      return reinterpret_cast<commit_raw_ptr>(obj);
    }
    if constexpr (std::same_as<std::decay_t<T>, tree_raw_ptr>) {
      throw_if(type != object_t::tree, "The given object isn't git_tree*");
      return reinterpret_cast<tree_raw_ptr>(obj);
    }
    if constexpr (std::same_as<std::decay_t<T>, blob_raw_ptr>) {
      throw_if(type != object_t::blob, "The given object isn't git_blob*");
      return reinterpret_cast<blob_raw_ptr>(obj);
    }
    if constexpr (std::same_as<std::decay_t<T>, tag_raw_ptr>) {
      throw_if(type != object_t::tag, "The given object isn't git_blob*");
      return reinterpret_cast<tag_raw_ptr>(obj);
    }
  }

  namespace sig {
    /// @brief Free an existing signature.
    /// @link
    /// https://libgit2.org/libgit2/#v0.20.0/group/signature/git_signature_free
    void free(signature_raw_ptr sig);

    /// This looks up the user.name and user.email from the configuration and
    /// uses the current time as the timestamp, and creates a new signature based
    /// on that information. Either the user.name or user.email must be set.
    auto create_default(repo_raw_ptr repo) -> signature_ptr;

  } // namespace sig

  namespace index {
    ///  Write an existing index object from memory back to disk using an atomic file lock.
    void write(index_raw_ptr index);

    /// Write the index as a tree.
    /// This method will scan the index and write a representation of its
    /// current state back to disk; it recursively creates tree objects for each of
    /// the subtrees stored in the index, but only returns the OID of the root
    /// tree. This is the OID that can be used e.g. to create a commit. The index
    /// instance cannot be bare, and needs to be associated to an existing
    /// repository. The index must not contain any file in conflict.
    [[nodiscard]] auto write_tree(index_raw_ptr index) -> git_oid;

    /// Add or update an index entry from a file on disk
    /// The file path must be relative to the repository's working folder and
    /// must be readable.
    /// This method will fail in bare index instances.
    /// This forces the file to be added to the index, not looking at gitignore rules. Those rules can be evaluated through the git_status APIs (in status.h) before calling this.
    /// If this file currently is the result of a merge conflict, this file will no longer be marked as conflicting. The data about the conflict will be moved to the "resolve undo" (REUC) section
    void add_by_path(index_raw_ptr index, const std::string &path);

    /// A utility to forcely and fastly add all files to staging area.
    /// This function will automatically call write tree to enable this changes.
    auto add_files(repo_raw_ptr repo, const std::vector<std::string> &files)
      -> std::tuple<git_oid, tree_ptr>;

  } // namespace index

  namespace tree {
    /// Lookup a tree object from the repository.
    auto lookup(repo_raw_ptr repo, oid_raw_cptr oid) -> tree_ptr;

  } // namespace tree

  namespace status {
    /// Gather file status information and populate the git_status_list.
    /// Note that if a pathspec is given in the git_status_options to filter
    /// the status, then the results from rename detection (if you enable it) may
    /// not be accurate. To do rename detection properly, this must be called with
    /// no pathspec so that all files can be considered.
    auto gather(repo_raw_ptr repo, const status_options &options) -> status_list_ptr;

    /// Gets the count of status entries in this list. If there are no changes
    /// in status (at least according the options given when the status list was
    /// created), this can return 0.
    auto entry_count(status_list_raw_ptr status_list) -> std::size_t;

    /// Get the default options.
    auto default_options() -> status_options;

    /// Get a pointer to one of the entries in the status list.
    /// No need to free this pointer
    auto get_by_index(status_list_raw_ptr status_list, std::size_t idx) -> status_entry_raw_cptr;
  } // namespace status
} // namespace linter::git

