#include "git_utils.h"
#include "utils/util.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <git2/branch.h>
#include <git2/commit.h>
#include <git2/diff.h>
#include <git2/errors.h>
#include <git2/object.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/signature.h>
#include <git2/types.h>
#include <iostream>
#include <print>
#include <string>

namespace linter::git {
  namespace {
    inline auto make_str(const char *p, std::size_t len) -> std::string {
      return {p, len};
    }
  } // namespace

  auto setup() -> int {
    return ::git_libgit2_init();
  }

  auto shutdown() -> int {
    return ::git_libgit2_shutdown();
  }

  auto delta_status_t_str(delta_status_t status) -> std::string {
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

  auto file_mode_t_str(file_mode_t mode) -> std::string {
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

  auto file_flag_t_str(std::uint32_t flags) -> std::string {
    auto res    = std::string{};
    auto concat = [&](std::uint32_t exactor, std::string_view msg) {
      if (flags & exactor) {
        if (res.empty()) {
          res.append(msg);
        } else {
          res.append(", ").append(msg);
        }
      }
    };
    concat(0b0000'0000, "binary");
    concat(0b0000'0010, "not_binary");
    concat(0b0000'0100, "valid_id");
    concat(0b0000'1000, "exists");
    concat(0b0001'0000, "valid_size");
    return res;
  }

  auto diff_line_t_str(diff_line_t tp) -> std::string {
    switch (tp) {
    case diff_line_t::context      : return "context";
    case diff_line_t::addition     : return "addition";
    case diff_line_t::deletion     : return "deletion";
    case diff_line_t::context_eofnl: return "context_eofnl";
    case diff_line_t::add_eofnl    : return "add_eofnl";
    case diff_line_t::del_eofnl    : return "del_eofnl";
    case diff_line_t::file_hdr     : return "file_hdr";
    case diff_line_t::hunk_hdr     : return "hunk_hdr";
    case diff_line_t::binary       : return "binary";
    case diff_line_t::unknown      : return "unknown";
    }
    return "unknown";
  }

  auto ref_t_str(ref_t tp) -> std::string {
    switch (tp) {
    case ref_t::direct  : return "direct";
    case ref_t::symbolic: return "symbolic";
    case ref_t::all     : return "all";
    default             : return "invalid";
    }
    return "invalid";
  }

  auto branch_t_str(branch_t tp) -> std::string {
    switch (tp) {
    case branch_t::local : return "local";
    case branch_t::remote: return "remote";
    case branch_t::all   : return "all";
    }
    return "unknown";
  }

  auto object_t_str(object_t tp) -> std::string {
    switch (tp) {
    case object_t::any   : return "any";
    case object_t::commit: return "commit";
    case object_t::bad   : return "bad";
    case object_t::tag   : return "tag";
    case object_t::blob  : return "blob";
    case object_t::tree  : return "tree";
    }
    return "unknown";
  }

  auto convert_to_signature(signature_cptr sig_ptr) -> signature {
    auto sig  = signature{};
    sig.name  = sig_ptr->name;
    sig.email = sig_ptr->email;
    sig.when  = {.sec = sig_ptr->when.time, .offset = sig_ptr->when.offset};
    return sig;
  }

  auto is_same_file(const diff_file_detail &file1, const diff_file_detail &file2) -> bool {
    return file1.relative_path == file2.relative_path;
  }

  namespace repo {
    auto open(const std::string &repo_path) -> repo_ptr {
      auto *repo = repo_ptr{nullptr};
      auto ret   = ::git_repository_open(&repo, repo_path.c_str());
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return repo;
    }

    void free(repo_ptr repo) {
      ::git_repository_free(repo);
    }

    auto state(repo_ptr repo) -> int {
      return ::git_repository_state(repo);
    }

    auto path(repo_ptr repo) -> std::string {
      const auto *ret = ::git_repository_path(repo);
      throw_if(ret == nullptr, [] noexcept { return ::git_error_last()->message; });
      return ret;
    }

    bool is_empty(repo_ptr repo) {
      auto ret = ::git_repository_is_empty(repo);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ret == 1;
    }

    auto init(const std::string &repo_path, bool is_bare) -> repo_ptr {
      auto *repo = repo_ptr{nullptr};
      auto ret =
        ::git_repository_init(&repo, repo_path.c_str(), static_cast<unsigned int>(is_bare));
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return repo;
    }

    auto config(repo_ptr repo) -> config_ptr {
      auto *config = config_ptr{nullptr};
      auto ret     = ::git_repository_config(&config, repo);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return config;
    }

    auto index(repo_ptr repo) -> index_ptr {
      auto *index = index_ptr{nullptr};
      auto ret    = ::git_repository_index(&index, repo);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return index;
    }

  } // namespace repo

  namespace config {
    void free(config_ptr config_ptr) {
      ::git_config_free(config_ptr);
    }
  } // namespace config

  namespace branch {
    auto create(repo_ptr repo, const std::string &branch_name, commit_cptr target, bool force)
      -> reference_ptr {
      auto *ptr = reference_ptr{nullptr};
      auto ret =
        ::git_branch_create(&ptr, repo, branch_name.c_str(), target, static_cast<int>(force));
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ptr;
    }

    void del(reference_ptr branch) {
      auto ret = ::git_branch_delete(branch);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
    }

    auto name(reference_ptr ref) -> std::string_view {
      const char *name = nullptr;
      auto ret         = ::git_branch_name(&name, ref);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return name;
    }

    bool is_head(reference_cptr branch) {
      auto ret = ::git_branch_is_head(branch);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ret == 1;
    }

    auto lookup(repo_ptr repo, const std::string &name, branch_t branch_type) -> reference_ptr {
      auto convert_to = [&]() {
        switch (branch_type) {
        case branch_t::local : return git_branch_t::GIT_BRANCH_LOCAL;
        case branch_t::remote: return git_branch_t::GIT_BRANCH_REMOTE;
        case branch_t::all   : return git_branch_t::GIT_BRANCH_ALL;
        }
        return GIT_BRANCH_ALL;
      };

      auto *res = reference_ptr{nullptr};
      auto ret  = ::git_branch_lookup(&res, repo, name.c_str(), convert_to());
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return res;
    }

  } // namespace branch

  namespace commit {
    auto tree(commit_cptr commit) -> tree_ptr {
      auto *ptr = tree_ptr{nullptr};
      auto ret  = ::git_commit_tree(&ptr, commit);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ptr;
    }

    auto tree_id(commit_cptr commit) -> oid_cptr {
      const auto *ret = ::git_commit_tree_id(commit);
      return ret;
    }

    auto lookup(repo_ptr repo, oid_cptr id) -> commit_ptr {
      auto *commit = commit_ptr{nullptr};
      auto ret     = ::git_commit_lookup(&commit, repo, id);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return commit;
    }

    auto author(commit_cptr commit) -> signature {
      const auto *sig_ptr = ::git_commit_author(commit);
      throw_if(sig_ptr == nullptr, "The returned git_signature pointer is null");
      auto sig = convert_to_signature(sig_ptr);
      return sig;
    }

    auto committer(commit_cptr commit) -> signature {
      const auto *sig_ptr = ::git_commit_committer(commit);
      throw_if(sig_ptr == nullptr, "The returned git_signature pointer is null");
      auto sig = convert_to_signature(sig_ptr);
      return sig;
    }

    auto time(commit_cptr commit) -> int64_t {
      auto time = ::git_commit_time(commit);
      return time;
    }

    auto message(commit_cptr commit) -> std::string {
      const auto *ret = ::git_commit_message(commit);
      throw_if(ret == nullptr, [] noexcept { return ::git_error_last()->message; });
      return ret;
    }

    auto nth_gen_ancestor(commit_cptr commit, std::uint32_t n) -> commit_ptr {
      auto *out = commit_ptr{nullptr};
      auto ret  = ::git_commit_nth_gen_ancestor(&out, commit, n);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return out;
    }

    auto parent(commit_cptr commit, std::uint32_t n) -> commit_ptr {
      auto *out = commit_ptr{nullptr};
      auto ret  = ::git_commit_parent(&out, commit, n);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return out;
    }

    auto parent_id(commit_cptr commit, std::uint32_t n) -> oid_cptr {
      const auto *ret = ::git_commit_parent_id(commit, n);
      return ret;
    }

    auto parent_count(commit_cptr commit) -> std::uint32_t {
      return git_commit_parentcount(commit);
    }

    void free(commit_ptr commit) {
      git::object::free(reinterpret_cast<object_ptr>(commit));
    }

  } // namespace commit

  namespace diff {
    void free(diff_ptr diff) {
      ::git_diff_free(diff);
    }

    auto index_to_workdir(repo_ptr repo, index_ptr index, diff_options_cptr opts) -> diff_ptr {
      auto *ptr = diff_ptr{nullptr};
      auto ret  = ::git_diff_index_to_workdir(&ptr, repo, index, opts);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ptr;
    }

    auto tree_to_tree(repo_ptr repo, tree_ptr old_tree, tree_ptr new_tree, diff_options_cptr opts)
      -> diff_ptr {
      auto *ptr = diff_ptr{nullptr};
      auto ret  = ::git_diff_tree_to_tree(&ptr, repo, old_tree, new_tree, opts);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ptr;
    }

    void init_option(diff_options_ptr opts) {
      auto ret = ::git_diff_options_init(opts, GIT_DIFF_OPTIONS_VERSION);
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
    }

    auto num_deltas(diff_ptr diff) -> std::size_t {
      return ::git_diff_num_deltas(diff);
    }

    auto get_delta(diff_cptr diff, size_t idx) -> diff_delta_cptr {
      return ::git_diff_get_delta(diff, idx);
    }

    auto for_each(
      diff_ptr diff,
      diff_file_cb file_cb,
      diff_binary_cb binary_cb,
      diff_hunk_cb hunk_cb,
      diff_line_cb line_cb,
      void *payload) -> int {
      return ::git_diff_foreach(diff, file_cb, binary_cb, hunk_cb, line_cb, payload);
    }

    auto deltas(diff_ptr diff) -> std::vector<diff_delta_detail> {
      auto line_cb =
        [](diff_delta_cptr cur_delta,
           diff_hunk_cptr cur_hunk,
           diff_line_cptr cur_line,
           void *payload) -> int {
        assert(cur_delta && cur_hunk && cur_line && payload);
        auto &deltas = *static_cast<std::vector<diff_delta_detail> *>(payload);

        auto cur_old_id = oid::to_str(cur_delta->old_file.id);
        auto cur_new_id = oid::to_str(cur_delta->new_file.id);
        auto delta_iter = std::ranges::find_if(deltas, [&](const diff_delta_detail &delta) {
          return delta.old_file.oid == cur_old_id && delta.new_file.oid == cur_new_id;
        });

        if (delta_iter == deltas.end()) {
          auto delta = diff_delta_detail{
            .status     = convert_to_delta_status_t(cur_delta->status),
            .flags      = cur_delta->flags,
            .similarity = cur_delta->similarity,
            .file_num   = cur_delta->nfiles,
            .old_file   = {.oid           = cur_old_id,
                           .relative_path = cur_delta->old_file.path,
                           .size          = cur_delta->old_file.size,
                           .flags         = cur_delta->old_file.flags,
                           .mode          = convert_to_file_mode_t(cur_delta->old_file.mode)},
            .new_file   = {.oid           = cur_new_id,
                           .relative_path = cur_delta->new_file.path,
                           .size          = cur_delta->new_file.size,
                           .flags         = cur_delta->new_file.flags,
                           .mode          = convert_to_file_mode_t(cur_delta->new_file.mode)},
          };
          deltas.emplace_back(std::move(delta));
          delta_iter = --(deltas.end());
        }

        auto cur_hunk_header =
          std::string{static_cast<const char *>(cur_hunk->header), cur_hunk->header_len};
        auto hook_iter = std::ranges::find_if(delta_iter->hunks, [&](const diff_hunk_detail &hunk) {
          return hunk.header == cur_hunk_header;
        });
        if (hook_iter == delta_iter->hunks.end()) {
          auto hunk = diff_hunk_detail{
            .header    = cur_hunk_header,
            .old_start = cur_hunk->old_start,
            .old_lines = cur_hunk->old_lines,
            .new_start = cur_hunk->new_start,
            .new_lines = cur_hunk->new_lines};
          delta_iter->hunks.emplace_back(std::move(hunk));
          hook_iter = --(delta_iter->hunks.end());
        }

        auto line = diff_line_detail{
          .origin           = convert_to_diff_line_t(cur_line->origin),
          .old_lineno       = cur_line->old_lineno,
          .new_lineno       = cur_line->new_lineno,
          .num_lines        = cur_line->num_lines,
          .offset_in_origin = cur_line->content_offset,
          .content          = make_str(cur_line->content, cur_line->content_len),
        };
        hook_iter->lines.emplace_back(std::move(line));
        return 0;
      };


      auto deltas = std::vector<diff_delta_detail>{};
      for_each(diff, nullptr, nullptr, nullptr, line_cb, &deltas);
      return deltas;
    }

    auto deltas(repo_ptr repo, const std::string &ref1, const std::string &ref2)
      -> std::vector<git::diff_delta_detail> {
      auto oid1     = ref::name_to_oid(repo, ref1);
      auto oid2     = ref::name_to_oid(repo, ref2);
      auto *commit1 = commit::lookup(repo, &oid1);
      auto *commit2 = commit::lookup(repo, &oid2);
      auto *tree1   = commit::tree(commit1);
      auto *tree2   = commit::tree(commit2);

      auto *diff = diff::tree_to_tree(repo, tree1, tree2, nullptr);
      commit::free(commit1);
      commit::free(commit2);
      object::free(reinterpret_cast<git::object_ptr>(tree1));
      object::free(reinterpret_cast<git::object_ptr>(tree2));
      auto ret = deltas(diff);
      diff::free(diff);
      return ret;
    }

    auto changed_files(repo_ptr repo, const std::string &target_ref, const std::string &source_ref)
      -> std::vector<std::string> {
      auto details = deltas(repo, target_ref, source_ref);
      auto res     = std::vector<std::string>{};
      for (const auto &delta: details) {
        res.emplace_back(delta.new_file.relative_path);
      }
      return res;
    }

  } // namespace diff

  namespace oid {
    auto to_str(const git_oid &oid) -> std::string {
      auto buffer = std::string{};
      // +1 is for null terminated.
      buffer.resize(GIT_OID_MAX_HEXSIZE + 1);
      ::git_oid_tostr(buffer.data(), buffer.size(), &oid);
      return buffer;
    }

    auto to_str(oid_cptr oid_ptr) -> std::string {
      auto buffer = std::string{};
      // +1 is for null terminated.
      buffer.resize(GIT_OID_MAX_HEXSIZE + 1);
      ::git_oid_tostr(buffer.data(), buffer.size(), oid_ptr);
      return buffer;
    }

    auto equal(git_oid o1, git_oid o2) -> bool {
      return ::git_oid_equal(&o1, &o2) == 1;
    }

  } // namespace oid

  namespace ref {
    auto type(reference_cptr ref) -> ref_t {
      auto ret = ::git_reference_type(ref);
      switch (ret) {
      case git_ref_t::GIT_REFERENCE_INVALID : return ref_t::invalid;
      case git_ref_t::GIT_REFERENCE_SYMBOLIC: return ref_t::symbolic;
      case git_ref_t::GIT_REFERENCE_DIRECT  : return ref_t::direct;
      case git_ref_t::GIT_REFERENCE_ALL     : return ref_t::all;
      }
      return ref_t::invalid;
    }

    auto is_branch(reference_ptr ref) -> bool {
      return ::git_reference_is_branch(ref) == 1;
    }

    auto is_remote(reference_ptr ref) -> bool {
      return ::git_reference_is_remote(ref) == 1;
    }

    void free(reference_ptr ref) {
      ::git_reference_free(ref);
    }

    auto lookup(repo_ptr repo, const std::string &name) -> reference_ptr {
      auto *ref = reference_ptr{nullptr};
      auto ret  = ::git_reference_lookup(&ref, repo, name.c_str());
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return ref;
    }

    auto name_to_oid(repo_ptr repo, const std::string &name) -> git_oid {
      auto oid = ::git_oid{};
      auto ret = ::git_reference_name_to_id(&oid, repo, name.c_str());
      throw_if(ret < 0, [] noexcept { return git_error_last()->message; });
      return oid;
    }

  } // namespace ref

  namespace revparse {
    auto single(repo_ptr repo, const std::string &spec) -> object_ptr {
      auto *obj = object_ptr{nullptr};
      auto ret  = ::git_revparse_single(&obj, repo, spec.c_str());
      throw_if(ret < 0, [] noexcept { return git_error_last()->message; });
      return obj;
    }

  }; // namespace revparse

  namespace object {
    void free(object_ptr obj) {
      ::git_object_free(obj);
    }

    auto type(object_cptr obj) -> object_t {
      auto tp = ::git_object_type(obj);
      return convert_to_object_t(tp);
    }

    auto id(object_cptr obj) -> oid_cptr {
      const auto *ret = ::git_object_id(obj);
      throw_if(ret == nullptr, [] noexcept { return ::git_error_last()->message; });
      return ret;
    }

    auto lookup(repo_ptr repo, oid_cptr oid, object_t type) -> object_ptr {
      auto *obj = object_ptr{nullptr};
      auto ret  = git_object_lookup(&obj, repo, oid, convert_to_git_otype(type));
      throw_if(ret < 0, [] noexcept { return ::git_error_last()->message; });
      return obj;
    }


  } // namespace object

  namespace sig {
    void free(signature_ptr sig) {
      git_signature_free(sig);
    }

  } // namespace sig


} // namespace linter::git
