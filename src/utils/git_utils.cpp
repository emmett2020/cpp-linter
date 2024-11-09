#include "git_utils.h"
#include "utils/util.h"
#include <cassert>
#include <cstring>
#include <git2/diff.h>
#include <git2/errors.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <iostream>
#include <print>

namespace linter::git {
  auto setup() -> int {
    return git_libgit2_init();
  }

  auto shutdown() -> int {
    return git_libgit2_shutdown();
  }

  namespace repo {
    auto open(const std::string &repo_path) -> repo_ptr {
      auto *repo = repo_ptr{nullptr};
      auto ret   = git_repository_open(&repo, repo_path.c_str());
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return repo;
    }

    void free(repo_ptr repo) {
      git_repository_free(repo);
    }

    auto state(repo_ptr repo) -> int {
      return git_repository_state(repo);
    }

    auto path(repo_ptr repo) -> std::string {
      const auto *ret = git_repository_path(repo);
      ThrowIf(ret == nullptr, [] noexcept { return git_error_last()->message; });
      return ret;
    }

    bool is_empty(repo_ptr repo) {
      auto ret = git_repository_is_empty(repo);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return ret == 1;
    }

    auto init(const std::string &repo_path, bool is_bare) -> repo_ptr {
      auto *repo = repo_ptr{nullptr};
      auto ret = git_repository_init(&repo, repo_path.c_str(), static_cast<unsigned int>(is_bare));
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return repo;
    }

    auto config(repo_ptr repo) -> config_ptr {
      auto *config = config_ptr{nullptr};
      auto ret     = git_repository_config(&config, repo);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return config;
    }

    auto index(repo_ptr repo) -> index_ptr {
      auto *index = index_ptr{nullptr};
      auto ret    = git_repository_index(&index, repo);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return index;
    }

  } // namespace repo

  namespace config {
    void free(config_ptr config_ptr) {
      git_config_free(config_ptr);
    }
  } // namespace config

  namespace branch {
    auto create(repo_ptr repo, const std::string &branch_name, commit_cptr target, bool force)
      -> reference_ptr {
      auto *ptr = reference_ptr{nullptr};
      auto ret =
        git_branch_create(&ptr, repo, branch_name.c_str(), target, static_cast<int>(force));
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return ptr;
    }

    void del(reference_ptr branch) {
      auto ret = git_branch_delete(branch);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
    }

    auto name(reference_ptr ref) -> std::string_view {
      const char *name = nullptr;
      auto ret         = git_branch_name(&name, ref);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return name;
    }

    bool is_head(reference_cptr branch) {
      auto ret = git_branch_is_head(branch);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return ret == 1;
    }

  } // namespace branch

  namespace commit {
    auto tree(commit_cptr commit) -> tree_ptr {
      auto *ptr = tree_ptr{nullptr};
      auto ret  = git_commit_tree(&ptr, commit);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return ptr;
    }

  } // namespace commit

  namespace diff {
    void free(diff_ptr diff) {
      git_diff_free(diff);
    }

    auto index_to_workdir(repo_ptr repo, index_ptr index, diff_options_cptr opts) -> diff_ptr {
      auto *ptr = diff_ptr{nullptr};
      auto ret  = git_diff_index_to_workdir(&ptr, repo, index, opts);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
      return ptr;
    }

    void init_option(diff_options_ptr opts) {
      auto ret = git_diff_options_init(opts, GIT_DIFF_OPTIONS_VERSION);
      ThrowIf(ret < 0, [] noexcept { return git_error_last()->message; });
    }

    auto num_deltas(diff_ptr diff) -> std::size_t {
      return git_diff_num_deltas(diff);
    }

    auto get_delta(diff_cptr diff, size_t idx) -> diff_delta_cptr {
      return git_diff_get_delta(diff, idx);
    }

    auto for_each(
      diff_ptr diff,
      diff_file_cb file_cb,
      diff_binary_cb binary_cb,
      diff_hunk_cb hunk_cb,
      diff_line_cb line_cb,
      void *payload) -> int {
      return git_diff_foreach(diff, file_cb, binary_cb, hunk_cb, line_cb, payload);
    }

    auto details(diff_ptr diff) -> std::vector<diff_detail> {
      auto file_cb = [](diff_delta_cptr delta, float progress, void *payload) -> int {
        assert(delta && payload);
        auto *res = static_cast<std::vector<diff_detail> *>(payload);
        assert(res);

        auto detail = diff_detail{
          .status     = convert_to_delta_status(delta->status),
          .flag       = delta->flags,
          .similarity = delta->similarity,
          .num_files  = delta->nfiles,
          .old_file   = {.oid           = delta->old_file.id,
                         .relative_path = delta->old_file.path,
                         .size          = delta->old_file.size,
                         .flags         = delta->old_file.flags,
                         .mode          = delta->old_file.mode,
                         .id_abbrev     = delta->old_file.id_abbrev},
          .new_file   = {.oid           = delta->new_file.id,
                         .relative_path = delta->new_file.path,
                         .size          = delta->new_file.size,
                         .flags         = delta->new_file.flags,
                         .mode          = delta->new_file.mode,
                         .id_abbrev     = delta->new_file.id_abbrev},
        };

        std::println(
          "{}, {}, {}, {}, {}, {}",
          detail.old_file.relative_path,
          detail.new_file.relative_path,
          detail.similarity,
          detail.num_files,
          detail.flag,
          delta_status_str(detail.status));

        res->emplace_back(std::move(detail));
        return 0;
      };

      auto hunk_cb = [](diff_delta_cptr delta, diff_hunk_cptr hunk, void *payload) -> int {
        auto *res = static_cast<diff_detail *>(payload);
        assert(res);
        auto hd      = hunk_details{};
        hd.header    = {static_cast<const char *>(hunk->header), hunk->header_len};
        hd.old_lines = hunk->old_lines;
        hd.new_lines = hunk->new_lines;
        hd.old_start = hunk->old_start;
        hd.new_start = hunk->new_start;
        res->hunks.emplace_back(hd);
        return 0;
      };


      auto line_cb =
        [](diff_delta_cptr delta, diff_hunk_cptr hunk, diff_line_cptr line, void *payload) -> int {
        auto *res = static_cast<diff_detail *>(payload);
        assert(res);
        auto l = line_details{};
        // l.origin = line->origin;
        l.num_lines        = line->num_lines;
        l.old_lineno       = line->old_lineno;
        l.new_lineno       = line->new_lineno;
        l.offset_in_origin = line->content_offset;
        l.content          = line->content;
        return 0;
        // TODO: push to which hunk?
      };


      auto details = std::vector<diff_detail>{};
      for_each(diff, file_cb, nullptr, nullptr, nullptr, &details);
      return details;
    }

  } // namespace diff

  namespace oid { }

} // namespace linter::git
