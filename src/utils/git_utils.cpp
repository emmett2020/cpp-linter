#include "git_utils.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <stdexcept>

#include <git2/config.h>
#include <git2/oid.h>
#include <git2/refs.h>
#include <git2/branch.h>
#include <git2/commit.h>
#include <git2/diff.h>
#include <git2/errors.h>
#include <git2/object.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/signature.h>
#include <git2/types.h>

#include "utils/git_error.h"

namespace linter::git {
  namespace {
    inline auto make_str(const char *p, std::size_t len) -> std::string {
      return {p, len};
    }
  } // namespace

  int setup() {
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
      if ((flags & exactor) != 0U) {
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

  auto repo_state_str(repo_state_t state) -> std::string {
    switch (state) {
    case repo_state_t::none                   : return "none";
    case repo_state_t::merge                  : return "merge";
    case repo_state_t::revert                 : return "revert";
    case repo_state_t::revert_sequence        : return "revert_sequence";
    case repo_state_t::cherrypick             : return "cherrypick";
    case repo_state_t::cherrypick_sequence    : return "cherrypick_sequence";
    case repo_state_t::bisect                 : return "bisect";
    case repo_state_t::rebase                 : return "rebase";
    case repo_state_t::rebase_interactive     : return "rebase_interactive";
    case repo_state_t::rebase_merge           : return "rebase_merge";
    case repo_state_t::apply_mailbox          : return "apply_mailbox";
    case repo_state_t::apply_mailbox_or_rebase: return "apply_mailbox_or_rebase";
    }
    return "unknown";
  }

  auto convert_to_signature(signature_raw_cptr sig_ptr) -> signature {
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
      auto *repo = repo_raw_ptr{nullptr};
      auto ret   = ::git_repository_open(&repo, repo_path.c_str());
      throw_if(ret);
      return {repo, ::git_repository_free};
    }

    void free(repo_raw_ptr repo) {
      ::git_repository_free(repo);
    }

    auto state(repo_raw_ptr repo) -> repo_state_t {
      return convert_to_repo_state(::git_repository_state(repo));
    }

    auto path(repo_raw_ptr repo) -> std::string {
      const auto *ret = ::git_repository_path(repo);
      throw_if(ret == nullptr, "can't get repository path");
      return ret;
    }

    bool is_empty(repo_raw_ptr repo) {
      auto ret = ::git_repository_is_empty(repo);
      throw_if(ret);
      return ret == 1;
    }

    auto init(const std::string &repo_path, bool is_bare) -> repo_ptr {
      auto *repo = repo_raw_ptr{nullptr};
      auto ret =
        ::git_repository_init(&repo, repo_path.c_str(), static_cast<unsigned int>(is_bare));
      throw_if(ret);
      return {repo, ::git_repository_free};
    }

    auto config(repo_raw_ptr repo) -> config_ptr {
      auto *config = config_raw_ptr{nullptr};
      auto ret     = ::git_repository_config(&config, repo);
      throw_if(ret);
      return {config, ::git_config_free};
    }

    auto config_snapshot(repo_raw_ptr repo) -> config_ptr {
      auto *config = config_raw_ptr{nullptr};
      auto ret     = ::git_repository_config_snapshot(&config, repo);
      throw_if(ret);
      return {config, ::git_config_free};
    }

    auto index(repo_raw_ptr repo) -> index_ptr {
      auto *index = index_raw_ptr{nullptr};
      auto ret    = ::git_repository_index(&index, repo);
      throw_if(ret);
      return {index, ::git_index_free};
    }

    auto head(repo_raw_ptr repo) -> ref_ptr {
      auto *ref = ref_raw_ptr{nullptr};
      auto ret  = ::git_repository_head(&ref, repo);
      if (ret == GIT_EUNBORNBRANCH || ret == GIT_ENOTFOUND) {
        return {nullptr, ::git_reference_free};
      }

      throw_if(ret);
      return {ref, ::git_reference_free};
    }

    auto head_commit(repo_raw_ptr repo) -> commit_ptr {
      auto head_ref = head(repo);
      if (head_ref == nullptr) {
        return {nullptr, ::git_commit_free};
      }
      auto obj = git::ref::peel<commit_ptr>(head_ref.get());
      return obj;
    }

  } // namespace repo

  namespace config {
    void free(config_raw_ptr config_ptr) {
      ::git_config_free(config_ptr);
    }

    auto get_string(config_raw_cptr config_ptr, const std::string &key) -> std::string {
      const char *value = nullptr;
      auto ret          = ::git_config_get_string(&value, config_ptr, key.c_str());
      throw_if(ret);
      return value;
    }

    auto get_bool(config_raw_cptr config_ptr, const std::string &key) -> bool {
      auto value = 0;
      auto ret   = ::git_config_get_bool(&value, config_ptr, key.c_str());
      throw_if(ret);
      return value == 1;
    }

    auto get_int32(config_raw_cptr config_ptr, const std::string &key) -> int32_t {
      auto value = 0;
      auto ret   = ::git_config_get_int32(&value, config_ptr, key.c_str());
      throw_if(ret);
      return value;
    }

    auto get_int64(config_raw_cptr config_ptr, const std::string &key) -> int64_t {
      auto value = int64_t{0};
      auto ret   = ::git_config_get_int64(&value, config_ptr, key.c_str());
      throw_if(ret);
      return value;
    }

    void set_string(config_raw_ptr config_ptr, const std::string &key, const std::string &value) {
      auto ret = ::git_config_set_string(config_ptr, key.c_str(), value.c_str());
      throw_if(ret);
    }

    void set_bool(config_raw_ptr config_ptr, const std::string &key, bool value) {
      auto ret = ::git_config_set_bool(config_ptr, key.c_str(), static_cast<int>(value));
      throw_if(ret);
    }

    void set_int32(config_raw_ptr config_ptr, const std::string &key, int32_t value) {
      auto ret = ::git_config_set_int32(config_ptr, key.c_str(), value);
      throw_if(ret);
    }

    void set_int64(config_raw_ptr config_ptr, const std::string &key, int64_t value) {
      auto ret = ::git_config_set_int64(config_ptr, key.c_str(), value);
      throw_if(ret);
    }

    auto snapshot(config_raw_ptr config) -> config_ptr {
      auto *out = config_raw_ptr{nullptr};
      auto ret  = ::git_config_snapshot(&out, config);
      throw_if(ret);
      return {out, ::git_config_free};
    }
  } // namespace config

  namespace branch {
    auto create(repo_raw_ptr repo,
                const std::string &branch_name,
                commit_raw_cptr target,
                bool force) -> ref_ptr {
      auto *ptr = ref_raw_ptr{nullptr};
      auto ret =
        ::git_branch_create(&ptr, repo, branch_name.c_str(), target, static_cast<int>(force));
      throw_if(ret);
      return {ptr, ::git_reference_free};
    }

    void del(ref_raw_ptr branch) {
      auto ret = ::git_branch_delete(branch);
      throw_if(ret);
    }

    auto name(ref_raw_ptr ref) -> std::string_view {
      const char *name = nullptr;
      auto ret         = ::git_branch_name(&name, ref);
      throw_if(ret);
      return name;
    }

    bool is_head(ref_raw_cptr branch) {
      auto ret = ::git_branch_is_head(branch);
      throw_if(ret);
      return ret == 1;
    }

    auto lookup(repo_raw_ptr repo, const std::string &name, branch_t branch_type) -> ref_ptr {
      auto convert_to = [&]() {
        switch (branch_type) {
        case branch_t::local : return git_branch_t::GIT_BRANCH_LOCAL;
        case branch_t::remote: return git_branch_t::GIT_BRANCH_REMOTE;
        case branch_t::all   : return git_branch_t::GIT_BRANCH_ALL;
        }
        return GIT_BRANCH_ALL;
      };

      auto *res = ref_raw_ptr{nullptr};
      auto ret  = ::git_branch_lookup(&res, repo, name.c_str(), convert_to());
      throw_if(ret);
      return {res, ::git_reference_free};
    }

    auto current_name(repo_raw_ptr repo) -> std::string {
      auto head_ref = repo::head(repo);
      if (head_ref == nullptr) {
        return "";
      }
      auto ref_type = ref::type(head_ref.get());
      throw_if(ref_type == ref_t::invalid, "get current branch failed since invalid ref type");

      if (ref_type == ref_t::symbolic) {
        auto direct_ref = ref::resolve(head_ref.get());
        return ref::shorthand(direct_ref.get());
      }
      return ref::shorthand(head_ref.get());
    }

  } // namespace branch

  namespace commit {
    auto create(
      repo_raw_ptr repo,
      const std::string &updated_ref,
      signature_raw_cptr author,
      signature_raw_cptr committer,
      const std::string &message,
      tree_raw_cptr tree,
      std::vector<commit_raw_cptr> parents) -> git_oid {
      auto id  = git_oid{};
      auto ret = ::git_commit_create(
        &id,
        repo,
        updated_ref.c_str(),
        author,
        committer,
        "UTF-8",
        message.c_str(),
        tree,
        parents.size(),
        parents.data());
      throw_if(ret);
      return id;
    }

    auto create_head(repo_raw_ptr repo, const std::string &message, tree_raw_cptr index_tree)
      -> std::tuple<git_oid, commit_ptr> {
      auto sig         = git::sig::create_default(repo);
      auto head_commit = repo::head_commit(repo);
      if (head_commit == nullptr) {
        auto oid    = create(repo, "HEAD", sig.get(), sig.get(), message, index_tree, {});
        auto commit = commit::lookup(repo, &oid);
        return {oid, std::move(commit)};
      }
      auto oid =
        create(repo, "HEAD", sig.get(), sig.get(), message, index_tree, {head_commit.get()});
      auto commit = commit::lookup(repo, &oid);
      return {oid, std::move(commit)};
    }

    auto tree(commit_raw_cptr commit) -> tree_ptr {
      auto *ptr = tree_raw_ptr{nullptr};
      auto ret  = ::git_commit_tree(&ptr, commit);
      throw_if(ret);
      return {ptr, ::git_tree_free};
    }

    auto tree_id(commit_raw_cptr commit) -> oid_raw_cptr {
      const auto *ret = ::git_commit_tree_id(commit);
      return ret;
    }

    auto lookup(repo_raw_ptr repo, oid_raw_cptr id) -> commit_ptr {
      auto *commit = commit_raw_ptr{nullptr};
      auto ret     = ::git_commit_lookup(&commit, repo, id);
      throw_if(ret);
      return {commit, ::git_commit_free};
    }

    auto author(commit_raw_cptr commit) -> signature {
      const auto *sig_ptr = ::git_commit_author(commit);
      throw_if(sig_ptr == nullptr, "The returned git_signature pointer is null");
      auto sig = convert_to_signature(sig_ptr);
      return sig;
    }

    auto committer(commit_raw_cptr commit) -> signature {
      const auto *sig_ptr = ::git_commit_committer(commit);
      throw_if(sig_ptr == nullptr, "The returned git_signature pointer is null");
      auto sig = convert_to_signature(sig_ptr);
      return sig;
    }

    auto time(commit_raw_cptr commit) -> int64_t {
      auto time = ::git_commit_time(commit);
      return time;
    }

    auto message(commit_raw_cptr commit) -> std::string {
      const auto *ret = ::git_commit_message(commit);
      throw_if(ret == nullptr, "get commit message error");
      return ret;
    }

    auto nth_gen_ancestor(commit_raw_cptr commit, std::uint32_t n) -> commit_ptr {
      auto *out = commit_raw_ptr{nullptr};
      auto ret  = ::git_commit_nth_gen_ancestor(&out, commit, n);
      throw_if(ret);
      return {out, ::git_commit_free};
    }

    auto parent(commit_raw_cptr commit, std::uint32_t n) -> commit_ptr {
      auto *out = commit_raw_ptr{nullptr};
      auto ret  = ::git_commit_parent(&out, commit, n);
      throw_if(ret);
      return {out, ::git_commit_free};
    }

    auto parent_id(commit_raw_cptr commit, std::uint32_t n) -> oid_raw_cptr {
      const auto *ret = ::git_commit_parent_id(commit, n);
      return ret;
    }

    auto parent_count(commit_raw_cptr commit) -> std::uint32_t {
      return git_commit_parentcount(commit);
    }

    void free(commit_raw_ptr commit) {
      git::object::free(reinterpret_cast<object_raw_ptr>(commit));
    }

    auto id_str(commit_raw_cptr commit) -> std::string {
      const auto *obj = reinterpret_cast<object_raw_cptr>(commit);
      return object::id_str(obj);
    }

  } // namespace commit

  namespace diff {
    void free(diff_raw_ptr diff) {
      ::git_diff_free(diff);
    }

    auto index_to_workdir(repo_raw_ptr repo, index_raw_ptr index, diff_options_raw_cptr opts)
      -> diff_ptr {
      auto *ptr = diff_raw_ptr{nullptr};
      auto ret  = ::git_diff_index_to_workdir(&ptr, repo, index, opts);
      throw_if(ret);
      return {ptr, ::git_diff_free};
    }

    auto tree_to_tree(
      repo_raw_ptr repo,
      tree_raw_ptr old_tree,
      tree_raw_ptr new_tree,
      diff_options_raw_cptr opts) -> diff_ptr {
      auto *ptr = diff_raw_ptr{nullptr};
      auto ret  = ::git_diff_tree_to_tree(&ptr, repo, old_tree, new_tree, opts);
      throw_if(ret);
      return {ptr, ::git_diff_free};
    }

    void init_option(diff_options_raw_ptr opts) {
      auto ret = ::git_diff_options_init(opts, GIT_DIFF_OPTIONS_VERSION);
      throw_if(ret);
    }

    auto num_deltas(diff_raw_ptr diff) -> std::size_t {
      return ::git_diff_num_deltas(diff);
    }

    auto get_delta(diff_raw_cptr diff, size_t idx) -> diff_delta_raw_cptr {
      return ::git_diff_get_delta(diff, idx);
    }

    auto for_each(
      diff_raw_ptr diff,
      diff_file_cb file_cb,
      diff_binary_cb binary_cb,
      diff_hunk_cb hunk_cb,
      diff_line_cb line_cb,
      void *payload) -> int {
      return ::git_diff_foreach(diff, file_cb, binary_cb, hunk_cb, line_cb, payload);
    }

    auto deltas(diff_raw_ptr diff) -> std::vector<diff_delta_detail> {
      auto line_cb =
        [](diff_delta_raw_cptr cur_delta,
           diff_hunk_raw_cptr cur_hunk,
           diff_line_raw_cptr cur_line,
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

    auto deltas(repo_raw_ptr repo, const std::string &spec1, const std::string &spec2)
      -> std::vector<git::diff_delta_detail> {
      auto obj1  = revparse::single(repo, spec1);
      auto obj2  = revparse::single(repo, spec2);
      auto type1 = object::type(obj1.get());
      auto type2 = object::type(obj2.get());
      throw_if(type1 == object_t::bad || type1 == object_t::blob,
               std::format("get deltas error since unknwon spec1: {}", spec1));
      throw_if(type2 == object_t::bad || type2 == object_t::blob,
               std::format("get deltas error since unknwon spec2: {}", spec2));

      auto tree1 = tree_ptr{nullptr, ::git_tree_free};
      auto tree2 = tree_ptr{nullptr, ::git_tree_free};

      if (type1 == object_t::commit) {
        tree1 = commit::tree(convert<commit_raw_cptr>(obj1.get()));
      } else {
        throw std::runtime_error{"unsupported"};
      }

      if (type2 == object_t::commit) {
        tree2 = commit::tree(convert<commit_raw_ptr>(obj2.get()));
      } else {
        throw std::runtime_error{"unsupported"};
      }

      auto diff = diff::tree_to_tree(repo, tree1.get(), tree2.get(), nullptr);
      auto ret  = deltas(diff.get());
      return ret;
    }

    auto changed_files(repo_raw_ptr repo, const std::string &spec1, const std::string &spec2)
      -> std::vector<std::string> {
      auto details = deltas(repo, spec1, spec2);
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

    auto to_str(oid_raw_cptr oid_ptr) -> std::string {
      auto buffer = std::string{};
      // +1 is for null terminated.
      buffer.resize(GIT_OID_MAX_HEXSIZE + 1);
      ::git_oid_tostr(buffer.data(), buffer.size(), oid_ptr);
      return buffer;
    }

    auto equal(git_oid o1, git_oid o2) -> bool {
      return ::git_oid_equal(&o1, &o2) == 1;
    }

    auto from_str(const std::string &str) -> git_oid {
      auto oid = git_oid{};
      auto ret = ::git_oid_fromstr(&oid, str.c_str());
      throw_if(ret);
      return oid;
    }

  } // namespace oid

  namespace ref {
    auto type(ref_raw_cptr ref) -> ref_t {
      auto ret = ::git_reference_type(ref);
      switch (ret) {
      case git_ref_t::GIT_REFERENCE_INVALID : return ref_t::invalid;
      case git_ref_t::GIT_REFERENCE_SYMBOLIC: return ref_t::symbolic;
      case git_ref_t::GIT_REFERENCE_DIRECT  : return ref_t::direct;
      case git_ref_t::GIT_REFERENCE_ALL     : return ref_t::all;
      }
      return ref_t::invalid;
    }

    auto is_branch(ref_raw_ptr ref) -> bool {
      return ::git_reference_is_branch(ref) == 1;
    }

    auto is_remote(ref_raw_ptr ref) -> bool {
      return ::git_reference_is_remote(ref) == 1;
    }

    auto is_tag(ref_raw_ptr ref) -> bool {
      return ::git_reference_is_tag(ref) == 1;
    }

    void free(ref_raw_ptr ref) {
      ::git_reference_free(ref);
    }

    auto name(ref_raw_cptr ref) -> std::string {
      const auto *ret = ::git_reference_name(ref);
      throw_if(ret == nullptr, "get reference name error");
      return ret;
    }

    auto lookup(repo_raw_ptr repo, const std::string &name) -> ref_ptr {
      auto *ref = ref_raw_ptr{nullptr};
      auto ret  = ::git_reference_lookup(&ref, repo, name.c_str());
      throw_if(ret);
      return {ref, ::git_reference_free};
    }

    auto name_to_oid(repo_raw_ptr repo, const std::string &name) -> git_oid {
      auto oid = ::git_oid{};
      auto ret = ::git_reference_name_to_id(&oid, repo, name.c_str());
      throw_if(ret);
      return oid;
    }

    auto shorthand(ref_raw_cptr ref) -> std::string {
      const auto *ret = ::git_reference_shorthand(ref);
      throw_if(ret == nullptr, "get shorthand error since unexpcetd null pointer");
      return ret;
    }

    auto resolve(ref_raw_cptr symbolic_ref) -> ref_ptr {
      auto *ref = ref_raw_ptr{nullptr};
      auto ret  = ::git_reference_resolve(&ref, symbolic_ref);
      throw_if(ret);
      return {ref, ::git_reference_free};
    }

    auto peel(ref_raw_cptr ref, object_t obj_type) -> object_ptr {
      auto *obj = object_raw_ptr{nullptr};
      auto type = convert_to_git_otype(obj_type);
      auto ret  = ::git_reference_peel(&obj, ref, type);
      if (ret == GIT_ENOTFOUND) {
        return {nullptr, ::git_object_free};
      }
      throw_if(ret);
      return {obj, ::git_object_free};
    }

  } // namespace ref

  namespace revparse {
    auto single(repo_raw_ptr repo, const std::string &spec) -> object_ptr {
      auto *obj = object_raw_ptr{nullptr};
      auto ret  = ::git_revparse_single(&obj, repo, spec.c_str());
      throw_if(ret);
      return {obj, ::git_object_free};
    }

    auto complete_sha(repo_raw_ptr repo, const std::string &short_sha) -> std::string {
      auto obj  = single(repo, short_sha);
      auto type = object::type(obj.get());
      throw_unless(type == object_t::commit, "the given sha is not commit");
      return oid::to_str(object::id(obj.get()));
    }

  }; // namespace revparse

  namespace object {
    void free(object_raw_ptr obj) {
      ::git_object_free(obj);
    }

    auto type(object_raw_cptr obj) -> object_t {
      auto tp = ::git_object_type(obj);
      return convert_to_object_t(tp);
    }

    auto id(object_raw_cptr obj) -> oid_raw_cptr {
      const auto *ret = ::git_object_id(obj);
      throw_if(ret == nullptr, "get object id failed");
      return ret;
    }

    auto id_str(object_raw_cptr obj) -> std::string {
      return oid::to_str(id(obj));
    }

    auto lookup(repo_raw_ptr repo, oid_raw_cptr oid, object_t type) -> object_ptr {
      auto *obj = object_raw_ptr{nullptr};
      auto ret  = ::git_object_lookup(&obj, repo, oid, convert_to_git_otype(type));
      throw_if(ret);
      return {obj, ::git_object_free};
    }

  } // namespace object

  namespace sig {
    void free(signature_raw_ptr sig) {
      git_signature_free(sig);
    }

    auto create_default(repo_raw_ptr repo) -> signature_ptr {
      auto *sig = signature_raw_ptr{nullptr};
      auto ret  = ::git_signature_default(&sig, repo);
      throw_if(ret);
      return {sig, ::git_signature_free};
    }

  } // namespace sig

  namespace index {
    void write(index_raw_ptr index) {
      auto ret = ::git_index_write(index);
      throw_if(ret);
    }

    auto write_tree(index_raw_ptr index) -> git_oid {
      auto oid = git_oid{};
      auto ret = ::git_index_write_tree(&oid, index);
      throw_if(ret);
      return oid;
    }

    void add_by_path(index_raw_ptr index, const std::string &path) {
      auto ret = ::git_index_add_bypath(index, path.c_str());
      throw_if(ret);
    }

    auto add_files(repo_raw_ptr repo, const std::vector<std::string> &files)
      -> std::tuple<git_oid, tree_ptr> {
      auto index = repo::index(repo);
      for (const auto &file: files) {
        git::index::add_by_path(index.get(), file);
      }
      auto oid = index::write_tree(index.get());
      auto obj = tree::lookup(repo, &oid);
      return {oid, std::move(obj)};
    }
  } // namespace index

  namespace tree {
    /// Lookup a tree object from the repository.
    auto lookup(repo_raw_ptr repo, oid_raw_cptr oid) -> tree_ptr {
      auto *tree = tree_raw_ptr{nullptr};
      auto ret   = ::git_tree_lookup(&tree, repo, oid);
      throw_if(ret);
      return {tree, git_tree_free};
    }

  } // namespace tree

  namespace status {
    auto gather(repo_raw_ptr repo, const status_options &options) -> status_list_ptr {
      auto *list = status_list_raw_ptr{nullptr};
      auto ret   = ::git_status_list_new(&list, repo, &options);
      throw_if(ret);
      return {list, ::git_status_list_free};
    }

    auto entry_count(status_list_raw_ptr status_list) -> std::size_t {
      auto ret = ::git_status_list_entrycount(status_list);
      return ret;
    }

    auto default_options() -> status_options {
      auto options = status_options{};
      throw_if(::git_status_options_init(&options, GIT_STATUS_OPTIONS_VERSION));
      return options;
    }

    auto get_by_index(status_list_raw_ptr status_list, std::size_t idx) -> status_entry_raw_cptr {
      const auto *ret = ::git_status_byindex(status_list, idx);
      throw_if(ret == nullptr, "get status list error since the given idx is out of range");
      return ret;
    }

  } // namespace status

} // namespace linter::git
