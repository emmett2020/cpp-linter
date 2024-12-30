/*
 * Copyright (c) 2024 Emmett Zhang
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cctype>
#include <filesystem>
#include <fstream>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <git2/diff.h>
#include <spdlog/spdlog.h>

#include "utils/git_utils.h"

using namespace linter; // NOLINT

// Get temporary repository directory.
auto get_temp_repo_dir() -> std::filesystem::path;

struct repo_t {
  explicit repo_t(const std::string &path = get_temp_repo_dir())
      : repo_path(path), repo(nullptr, git::repo::free) {
    remove();
    std::filesystem::create_directory(path);
    init();
  }

  ~repo_t() { remove(); }

  // This will always remove old file.
  auto add_file(const std::string &file_path, const std::string &content) {
    auto file_full_path = repo_path / file_path;
    if (std::filesystem::exists(file_full_path)) {
      std::filesystem::remove(file_full_path);
    }
    auto file = std::fstream(file_full_path, std::ios::out);
    REQUIRE(file.is_open());
    file << content;
    file.close();
    if (!ranges::contains(uncommitted_files, file_path)) {
      uncommitted_files.push_back(file_path);
    }
  }

  void append_content_to_exist_file(const std::string &file_path,
                                    const std::string &content) {
    auto file_full_path = repo_path / file_path;
    REQUIRE(std::filesystem::exists(file_full_path));

    auto file = std::fstream(file_full_path, std::ios::app);
    REQUIRE(file.is_open());
    file << content;
    file.close();
  }

  // Rewrite exist file's content. This could be safely called by user multiple
  // times. And the last version of it will be finally used.
  auto rewrite_file(const std::string &file_path,
                          const std::string &content) {
    auto file_full_path = repo_path / file_path;
    REQUIRE(std::filesystem::exists(file_full_path));

    auto file = std::fstream(file_full_path, std::ios::out);
    REQUIRE(file.is_open());
    file << content;
    file.close();
    if (!ranges::contains(uncommitted_files, file_path)) {
      uncommitted_files.push_back(file_path);
    }
  }

  auto commit_changes() -> std::tuple<std::string, linter::git::commit_ptr> {
    auto [index_oid, index] =
        git::index::add_files(repo.get(), uncommitted_files);
    auto message = fmt::format("Commit Index {}", commit_idx);
    auto [commit_oid, commit] =
        git::commit::create_head(repo.get(), message, index.get());
    return {git::oid::to_str(commit_oid), std::move(commit)};
  }

  void remove() {
    if (std::filesystem::exists(repo_path)) {
      std::filesystem::remove_all(repo_path);
    }
  }

  auto get_path() -> std::filesystem::path { return repo_path; }

private:
  void init() {
    using namespace linter;
    repo = git::repo::init(repo_path, false);
    REQUIRE(git::repo::is_empty(repo.get()));
    auto config = git::repo::config(repo.get());
    git::config::set_string(config.get(), "user.name", user_name);
    git::config::set_string(config.get(), "user.email", user_email);
  }

  static constexpr auto user_name = "test";
  static constexpr auto user_email = "test@email.com";

  std::filesystem::path repo_path;
  linter::git::repo_ptr repo;
  std::size_t commit_idx = 1;
  std::vector<std::string> uncommitted_files;
};

// Remove repository.
void remove_repo();

// Create temporary repository. If repository already exists, it'll be
// refreshed.
void create_temp_repo();

// Create file in temporary repository.
void create_temp_file(const std::string &file_name, const std::string &content);

// Create files in temporary repository. Each file contains the same content.
void create_temp_files(const std::vector<std::string> &file_names,
                       const std::string &content);

// Append the given content into the given file.
void append_content_to_file(const std::string &file,
                            const std::string &content);

// Initialize a basic repo for futhure test.
auto init_basic_repo() -> linter::git::repo_ptr;

// Initialize a basic repo for futhure test.
auto init_repo_with_commit(const std::vector<std::string> &files,
                           const std::string &commit_message = "")
    -> std::tuple<linter::git::repo_ptr, linter::git::commit_ptr>;
