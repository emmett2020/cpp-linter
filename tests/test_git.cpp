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
#include <git2/diff.h>
#include <iostream>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

#include "test_common.h"
#include "utils/git_utils.h"

using namespace linter;
using namespace std::string_literals;
using namespace std::string_view_literals;

const auto default_branch = "master"s;

TEST_CASE("Create repo should work", "[CppLintAction][git2][repo]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  auto repo = git::repo::init(get_temp_repo_dir(), false);
  REQUIRE(git::repo::is_empty(repo.get()));
  auto temp_repo_dir_with_git = get_temp_repo_dir() / ".git/";
  REQUIRE(git::repo::path(repo.get()) == temp_repo_dir_with_git);
}

TEST_CASE("Set config should work", "[CppLintAction][git2][config]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  auto repo   = git::repo::init(get_temp_repo_dir(), false);
  auto origin = git::repo::config(repo.get());
  SECTION("set_string") {
    git::config::set_string(origin.get(), "user.name", "test");
  }
  SECTION("set_bool") {
    git::config::set_bool(origin.get(), "core.filemode", true);
  }

  auto config = git::repo::config_snapshot(repo.get());
  SECTION("get_bool") {
    REQUIRE(git::config::get_bool(config.get(), "core.filemode") == true);
  }
}

TEST_CASE("Compare with head", "[CppLintAction][git2][status]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  auto repo = git::repo::init(get_temp_repo_dir(), false);
  REQUIRE(git::repo::is_empty(repo.get()));

  // Default is HEAD
  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 0);
}

TEST_CASE("Commit two new files step by step", "[CppLintAction][git2][commit]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  // Create repository instance
  auto repo   = git::repo::init(get_temp_repo_dir(), false);
  auto config = git::repo::config(repo.get());
  git::config::set_string(config.get(), "user.name", "test");
  git::config::set_string(config.get(), "user.email", "test@email.com");

  // Add files to repository
  create_temp_file("file1.cpp", "hello world");
  create_temp_file("file2.cpp", "hello world");
  auto index = git::repo::index(repo.get());
  git::index::add_by_path(index.get(), "file1.cpp");
  git::index::add_by_path(index.get(), "file2.cpp");
  auto index_tree_oid = git::index::write_tree(index.get());

  // Get repository status
  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 2);

  const auto *entry0 = git::status::get_by_index(status_list.get(), 0);
  const auto *entry1 = git::status::get_by_index(status_list.get(), 1);
  REQUIRE(entry0->status == git::status_t::GIT_STATUS_INDEX_NEW);
  REQUIRE(entry1->status == git::status_t::GIT_STATUS_INDEX_NEW);

  // We did't have any branches yet.
  auto index_tree_obj = git::tree::lookup(repo.get(), &index_tree_oid);
  auto sig            = git::sig::create_default(repo.get());
  auto commit_oid     = git::commit::create(
    repo.get(),
    "HEAD",
    sig.get(),
    sig.get(),
    "Initial commit",
    index_tree_obj.get(),
    {});
  REQUIRE(git::branch::current_name(repo.get()) == default_branch);
}

TEST_CASE("Add three files to index by our utility", "[CppLintAction][git2][index]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  auto repo = init_basic_repo();

  // Stage two files.
  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  create_temp_files(files, "hello world");
  git::index::add_files(repo.get(), files);
  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 2);

  // Stage a new files.
  create_temp_file("file3.cpp", "hello world");
  git::index::add_files(repo.get(), {"file3.cpp"});
  status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 3);
}

TEST_CASE("Delete two files to index by our utility", "[CppLintAction][git2][index]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};
  auto repo  = init_basic_repo();

  // Firstly, we create two files and commit it.
  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  create_temp_files(files, "hello world");
  auto ret = git::index::add_files(repo.get(), files);
  git::commit::create_head(repo.get(), "Add two files", std::get<1>(ret).get());

  // Then we remove the files just created.
  git::index::remove_files(repo.get(), get_temp_repo_dir(), files);
  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 2);

  const auto file1_path = get_temp_repo_dir() / "file1.cpp";
  const auto file2_path = get_temp_repo_dir() / "file2.cpp";
  REQUIRE(!std::filesystem::exists(file1_path));
  REQUIRE(!std::filesystem::exists(file2_path));
}

TEST_CASE("Parse single uses revparse", "[CppLintAction][git2][revparse]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  create_temp_files(files, "hello world");

  auto repo                    = init_basic_repo();
  auto [index_oid, index_tree] = git::index::add_files(repo.get(), files);
  auto [commit_oid, _]         = git::commit::create_head(repo.get(), "Init", index_tree.get());

  SECTION("get commit id by head") {
    auto ret = git::revparse::single(repo.get(), default_branch);
    REQUIRE_FALSE(git::object::id_str(ret.get()).empty());
  }
}

TEST_CASE("Get HEAD", "[CppLintAction][git2][commit]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  create_temp_files({"file0.cpp", "file1.cpp"}, "hello world");
  auto [repo, commit] = init_repo_with_commit({"file0.cpp", "file1.cpp"});
  auto ref            = git::repo::head(repo.get());
  auto head_commit    = git::ref::peel<git::commit_ptr>(ref.get());
  REQUIRE(git::commit::id_str(head_commit.get()) == git::commit::id_str(commit.get()));
  remove_temp_repo_dir();
}

TEST_CASE("Push two commits and get diff files", "[CppLintAction][git2][diff]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  create_temp_files(files, "hello world");
  auto repo                   = init_basic_repo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());

  auto head_commit = git::repo::head_commit(repo.get());
  std::cout << git::commit::id_str(head_commit.get()) << "\n";
  REQUIRE(git::commit::id_str(head_commit.get()) == git::commit::id_str(commit1.get()));

  append_content_to_file("file1.cpp", "hello world2");
  auto [index_oid2, index2]   = git::index::add_files(repo.get(), {"file1.cpp"});
  auto [commit_oid2, commit2] = git::commit::create_head(repo.get(), "Two", index2.get());
  auto head_commit2           = git::repo::head_commit(repo.get());
  REQUIRE(git::commit::id_str(head_commit2.get()) == git::commit::id_str(commit2.get()));

  auto changed_files = git::diff::changed_files(repo.get(), "HEAD~1", "HEAD");
  REQUIRE(changed_files.size() == 1);
}

TEST_CASE("Simple use of patch ", "[CppLintAction][git2][patch]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  create_temp_files(files, "hello world");
  auto repo                   = init_basic_repo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());

  auto head_commit = git::repo::head_commit(repo.get());
  std::cout << git::commit::id_str(head_commit.get()) << "\n";
  REQUIRE(git::commit::id_str(head_commit.get()) == git::commit::id_str(commit1.get()));

  append_content_to_file("file1.cpp", "hello world2");
  auto [index_oid2, index2]   = git::index::add_files(repo.get(), {"file1.cpp"});
  auto [commit_oid2, commit2] = git::commit::create_head(repo.get(), "Two", index2.get());
  auto head_commit2           = git::repo::head_commit(repo.get());
  REQUIRE(git::commit::id_str(head_commit2.get()) == git::commit::id_str(commit2.get()));

  auto diff  = git::diff::commit_to_commit(repo.get(), commit1.get(), commit2.get());
  auto patch = git::patch::create_from_diff(diff.get(), 0);
}

TEST_CASE("Create patch from buffers", "[CppLintAction][git2][patch]") {
  auto old_content = "int n = 2;"s;
  auto new_content = "double n = 2;"s;
  auto opt         = git_diff_options{};
  git::diff::init_option(&opt);
  auto patch =
    git::patch::create_from_buffers(old_content, "temp.cpp", new_content, "temp.cpp", opt);
  std::cout << git::patch::to_str(patch.get());
}

TEST_CASE("Get file content from a specific commit", "[CppLintAction][git2][blob]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  const auto files = std::vector<std::string>{"file1.cpp"};
  create_temp_files(files, "hello world");
  auto repo                   = init_basic_repo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());
  auto content                = git::blob::get_raw_content(repo.get(), commit1.get(), "file1.cpp");
  REQUIRE(content == "hello world");
}

TEST_CASE("Get lines in a hunk", "[CppLintAction][git2][patch]") {
  create_temp_repo_dir();
  auto guard = scope_guard{remove_temp_repo_dir};

  const auto files = std::vector<std::string>{"file1.cpp"};
  create_temp_files(files, "hello world\nhello world2\n");
  auto repo                   = init_basic_repo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());

  append_content_to_file("file1.cpp", "hello world3");
  auto [index_oid2, index2]   = git::index::add_files(repo.get(), {"file1.cpp"});
  auto [commit_oid2, commit2] = git::commit::create_head(repo.get(), "Two", index2.get());
  auto head_commit2           = git::repo::head_commit(repo.get());

  auto diff     = git::diff::commit_to_commit(repo.get(), commit1.get(), commit2.get());
  auto patch    = git::patch::create_from_diff(diff.get(), 0);
  auto contents = git::patch::get_lines_in_hunk(patch.get(), 0);
  REQUIRE(contents[0] == "hello world\n");
  REQUIRE(contents[1] == "hello world2\n");
  REQUIRE(contents[2] == "hello world3");
}

TEST_CASE("Compare from buffer", "[CppLintAction][git2][patch]") {
  // Compare original content with formatted result of a file.

  std::string before = R"""(
namespace {
intt;
intt1;
intt2;
intt3;
int x = 1.1;
  int y = 1.1;
    int z = 1.1;
}
intu1;
intu2;
intu3;
)""";

  std::string after = R"""(
namespace {
intt;
intt1;
intt2;
intt3;
int x = 1.1;
int y = 1.1;
int z = 1.1;
}
intu1;
intu2;
intu3;
)""";

  auto opts          = git::diff::init_option();
  opts.context_lines = 0;
  auto patch         = git::patch::create_from_buffers(before, "name", after, "name", opts);

  auto num_hunks = git::patch::num_hunks(*patch);
  REQUIRE(num_hunks == 1);

  auto lines = git::patch::get_target_lines_in_hunk(*patch, 0);
  REQUIRE(lines.size() == 2);
}
