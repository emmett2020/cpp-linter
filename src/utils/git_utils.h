#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

#include <git2.h>
#include <git2/deprecated.h>
#include <git2/types.h>

#include "utils/util.h"

/// TODO: Maybe a standlone repository with libgit2 as submodule
/// TODO: Could we use unique_ptr and automaticly free the allocated pointer?

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

  using repo_ptr         = git_repository *;
  using config_ptr       = git_config *;
  using reference_ptr    = git_reference *;
  using commit_ptr       = git_commit *;
  using diff_ptr         = git_diff *;
  using diff_options_ptr = git_diff_options *;
  using tree_ptr         = git_tree *;
  using index_ptr        = git_index *;
  using blob_ptr         = git_blob *;
  using tag_ptr          = git_tag *;
  using diff_delta_ptr   = git_diff_delta *;
  using diff_hunk_ptr    = git_diff_hunk *;
  using diff_line_ptr    = git_diff_line *;
  using object_ptr       = git_object *;
  using oid_ptr          = git_oid *;
  using signature_ptr    = git_signature *;

  using repo_cptr         = const git_repository *;
  using config_cptr       = const git_config *;
  using reference_cptr    = const git_reference *;
  using commit_cptr       = const git_commit *;
  using diff_cptr         = const git_diff *;
  using diff_options_cptr = const git_diff_options *;
  using tree_cptr         = const git_tree *;
  using index_cptr        = const git_index *;
  using blob_cptr         = const blob_ptr *;
  using tag_cptr          = const git_tag *;
  using diff_delta_cptr   = const git_diff_delta *;
  using diff_hunk_cptr    = const git_diff_hunk *;
  using diff_line_cptr    = const git_diff_line *;
  using object_cptr       = const git_object *;
  using oid_cptr          = const git_oid *;
  using signature_cptr    = const git_signature *;

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

  auto convert_to_signature(signature_cptr sig) -> signature;

  auto is_same_file(const diff_file_detail &file1, const diff_file_detail &file2) -> bool;
  auto delta_status_t_str(delta_status_t status) -> std::string;
  auto file_mode_t_str(file_mode_t mode) -> std::string;
  auto file_flag_t_str(std::uint32_t flag) -> std::string;
  auto diff_line_t_str(diff_line_t tp) -> std::string;
  auto ref_t_str(ref_t tp) -> std::string;
  auto branch_t_str(branch_t tp) -> std::string;
  auto object_t_str(object_t tp) -> std::string;

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

    /// @brief Lookup a branch by its name in a repository.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/branch/git_branch_lookup
    auto lookup(repo_ptr repo, const std::string &name, branch_t branch_type) -> reference_ptr;

  } // namespace branch

  namespace commit {
    /// @brief Get the tree pointed to by a commit.
    /// https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
    auto tree(commit_cptr commit) -> tree_ptr;

    /// @brief Get the id of the tree pointed to by a commit. This differs from
    /// git_commit_tree in that no attempts are made to fetch an object from the
    /// ODB.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_tree_id
    auto tree_id(commit_cptr commit) -> oid_cptr;

    /// @brief Lookup a commit object from a repository.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_lookup
    auto lookup(repo_ptr repo, oid_cptr id) -> commit_ptr;

    /// @brief Get the author of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_author
    auto author(commit_cptr commit) -> signature;

    /// @brief Get the committer of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_committer
    auto committer(commit_cptr commit) -> signature;

    /// @brief Get the commit time (i.e. committer time) of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_time
    auto time(commit_cptr commit) -> int64_t;

    /// @brief Get the full message of a commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_message
    auto message(commit_cptr commit) -> std::string;

    /// @brief Get the commit object that is the <n
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_nth_gen_ancestor
    auto nth_gen_ancestor(commit_cptr commit, std::uint32_t n) -> commit_ptr;

    /// @brief Get the specified parent of the commit.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_parent
    auto parent(commit_cptr commit, std::uint32_t n) -> commit_ptr;

    /// @brief Get the oid of a specified parent for a commit. This is
    /// different from git_commit_parent, which will attempt to load the parent
    /// commit from the ODB.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_parent_id
    auto parent_id(commit_cptr commit, std::uint32_t n) -> oid_cptr;

    /// @brief Get the number of parents of this commit
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/commit/git_commit_parentcount
    auto parent_count(commit_cptr commit) -> std::uint32_t;

    void free(commit_ptr commit);

  } // namespace commit

  namespace diff {
    /// @brief Deallocate a diff.
    /// @link https://libgit2.org/libgit2/#HEAD/group/diff/git_diff_free
    void free(diff_ptr diff);

    /// @brief: Create a diff between the repository index and the workdir directory.
    /// @link: https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_index_to_workdir
    auto index_to_workdir(repo_ptr repo, index_ptr index, diff_options_cptr opts) -> diff_ptr;

    /// @brief: Create a diff with the difference between two tree objects.
    /// @link: https://libgit2.org/libgit2/#v0.20.0/group/diff/git_diff_tree_to_tree
    auto tree_to_tree(repo_ptr repo, tree_ptr old_tree, tree_ptr new_tree, diff_options_cptr opts)
      -> diff_ptr;

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

    /// @brief A simple implmentation which uses for_each to get diff delta details.
    auto deltas(diff_ptr diff) -> std::vector<diff_delta_detail>;

    /// @brief A simple implmentation which compares ref1 with ref2's differences.
    auto deltas(repo_ptr repo, const std::string &ref1, const std::string &ref2)
      -> std::vector<git::diff_delta_detail>;

    /// @brief Get changed files between two refs.
    /// @return The modified and new added file names in source reference.
    auto changed_files(repo_ptr repo, const std::string &target_ref, const std::string &source_ref)
      -> std::vector<std::string>;


  } // namespace diff

  namespace oid {
    /// @brief Format a git_oid into a buffer as a hex format c-string.
    /// @link https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_tostr
    auto to_str(const git_oid &oid) -> std::string;
    auto to_str(oid_cptr oid_ptr) -> std::string;

    /// @brief Compare two oid structures for equality
    /// @link https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_equal
    auto equal(git_oid o1, git_oid o2) -> bool;

  } // namespace oid

  namespace ref {
    /// @brief Get the type of a reference.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_type
    auto type(reference_cptr ref) -> ref_t;

    /// @brief Check if a reference is a local branch.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_is_branch
    auto is_branch(reference_ptr ref) -> bool;


    /// @brief Check if a reference is a remote tracking branch
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_is_remote
    auto is_remote(reference_ptr ref) -> bool;

    /// @brief Free the given reference.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_free
    void free(reference_ptr ref);

    /// @brief: Lookup a reference by name in a repository.
    /// @param name: the long name for the reference (e.g. HEAD, refs/heads/master, refs/tags/v0.1.0, ...)
    /// @link: https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_lookup
    auto lookup(repo_ptr repo, const std::string &name) -> reference_ptr;

    /// @brief: Lookup a reference by name and resolve immediately to OID.
    /// @link: https://libgit2.org/libgit2/#v0.20.0/group/reference/git_reference_name_to_id
    auto name_to_oid(repo_ptr repo, const std::string &name) -> git_oid;

  } // namespace ref

  namespace revparse {
    /// @brief Find a single object, as specified by a revision string.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/revparse/git_revparse_single
    auto single(repo_ptr repo, const std::string &spec) -> object_ptr;

  }; // namespace revparse

  namespace object {
    /// @brief Close an open object
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_free
    void free(object_ptr obj);

    /// @brief Get the object type of an object
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_type
    auto type(object_cptr obj) -> object_t;

    /// @brief Get the id (SHA1) of a repository object
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_id
    auto id(object_cptr obj) -> oid_cptr;

    /// @brief Lookup a reference to one of the objects in a repository.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/object/git_object_lookup
    auto lookup(repo_ptr repo, oid_cptr oid, object_t type) -> object_ptr;

  } // namespace object

  template <typename T>
  auto convert(object_ptr obj) -> T {
    auto type = object::type(obj);
    if constexpr (std::same_as<std::decay_t<T>, commit_ptr>) {
      throw_if(type != object_t::commit, "The given object isn't git_commit*");
      return reinterpret_cast<commit_ptr>(obj);
    }
    if constexpr (std::same_as<std::decay_t<T>, tree_ptr>) {
      throw_if(type != object_t::tree, "The given object isn't git_tree*");
      return reinterpret_cast<tree_ptr>(obj);
    }
    if constexpr (std::same_as<std::decay_t<T>, blob_ptr>) {
      throw_if(type != object_t::blob, "The given object isn't git_blob*");
      return reinterpret_cast<blob_ptr>(obj);
    }
    if constexpr (std::same_as<std::decay_t<T>, tag_ptr>) {
      throw_if(type != object_t::tag, "The given object isn't git_blob*");
      return reinterpret_cast<tag_ptr>(obj);
    }
  }

  namespace sig {
    /// @brief Free an existing signature.
    /// @link https://libgit2.org/libgit2/#v0.20.0/group/signature/git_signature_free
    void free(signature_ptr sig);

  } // namespace sig


} // namespace linter::git
