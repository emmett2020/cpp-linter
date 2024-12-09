#include <cctype>
#include <filesystem>
#include <fstream>
#include <git2/diff.h>
#include <ios>
#include <iostream>
#include <print>

#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include "utils/git_utils.h"

using namespace linter;
using namespace std::string_literals;
using namespace std::string_view_literals;

const auto temp_dir       = std::filesystem::temp_directory_path();
const auto temp_repo_dir  = temp_dir / "test_git";
const auto default_branch = "master"s;

namespace {
  auto RefreshRepoDir() {
    if (std::filesystem::exists(temp_repo_dir)) {
      // std::println("remove old repo directory");
      std::filesystem::remove_all(temp_repo_dir);
    }
    std::filesystem::create_directory(temp_repo_dir);
  }

  auto RemoveRepoDir() {
    if (std::filesystem::exists(temp_repo_dir)) {
      std::filesystem::remove_all(temp_repo_dir);
    }
  }

  auto CreateTempFile(const std::string& file_name, const std::string& content) {
    auto new_file_path = temp_repo_dir / file_name;
    if (std::filesystem::exists(new_file_path)) {
      std::filesystem::remove(new_file_path);
    }
    auto file = std::fstream(new_file_path, std::ios::out);
    REQUIRE(file.is_open());
    file << content;
    file.close();
  }

  auto AppendToFile(const std::string& file_name, const std::string& content) {
    auto new_file_path = temp_repo_dir / file_name;
    auto file          = std::fstream(new_file_path, std::ios::app);
    REQUIRE(file.is_open());
    file << content;
    file.close();
  }

  auto CreateTempFilesWithSameContent(const std::vector<std::string>& file_names,
                                      const std::string& content) {
    for (const auto& file: file_names) {
      CreateTempFile(file, content);
    }
  }

  // Initialize a basic repo for futhure test.
  auto InitBasicRepo() -> git::repo_ptr {
    auto repo = git::repo::init(temp_repo_dir, false);
    REQUIRE(git::repo::is_empty(repo.get()));
    auto config = git::repo::config(repo.get());
    git::config::set_string(config.get(), "user.name", "cpp-linter");
    git::config::set_string(config.get(), "user.email", "cpp-linter@email.com");
    return repo;
  }

  auto InitRepoWithACommit(const std::vector<std::string>& files)
    -> std::tuple<git::repo_ptr, git::commit_ptr> {
    auto repo                 = InitBasicRepo();
    auto [index_oid, index]   = git::index::add_files(repo.get(), files);
    auto [commit_oid, commit] = git::commit::create_head(repo.get(), "Init", index.get());
    return {std::move(repo), std::move(commit)};
  }

} // namespace

TEST_CASE("Create repo", "[git2][repo]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));
  auto temp_repo_dir_with_git = temp_repo_dir / ".git/";
  REQUIRE(git::repo::path(repo.get()) == temp_repo_dir_with_git);
  RemoveRepoDir();
}

TEST_CASE("Set config", "[git2][config]") {
  RefreshRepoDir();
  auto repo   = git::repo::init(temp_repo_dir, false);
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
  RemoveRepoDir();
}

TEST_CASE("Compare with head", "[git2][status]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));

  // default is HEAD
  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 0);
  RemoveRepoDir();
}

TEST_CASE("Commit two new added files", "[git2][index][status][commit][branch][sig]") {
  RefreshRepoDir();
  CreateTempFile("file1.cpp", "hello world");
  CreateTempFile("file2.cpp", "hello world");

  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));
  auto config = git::repo::config(repo.get());
  git::config::set_string(config.get(), "user.name", "cpp-linter");
  git::config::set_string(config.get(), "user.email", "cpp-linter@email.com");

  auto index = git::repo::index(repo.get());
  git::index::add_by_path(index.get(), "file1.cpp");
  git::index::add_by_path(index.get(), "file2.cpp");
  auto index_tree_oid = git::index::write_tree(index.get());

  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 2);

  const auto* entry0 = git::status::get_by_index(status_list.get(), 0);
  const auto* entry1 = git::status::get_by_index(status_list.get(), 1);
  REQUIRE(entry0->status == git::status_t::GIT_STATUS_INDEX_NEW);
  REQUIRE(entry1->status == git::status_t::GIT_STATUS_INDEX_NEW);

  // we did't have any branches yet.
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
  RemoveRepoDir();
}

TEST_CASE("Add three files to index by utility", "[git2][index][utility]") {
  RefreshRepoDir();
  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  CreateTempFilesWithSameContent(files, "hello world");
  auto repo = InitBasicRepo();

  git::index::add_files(repo.get(), files);
  auto options     = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 2);
  CreateTempFile("file3.cpp", "hello world");
  git::index::add_files(repo.get(), {"file3.cpp"});

  status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 3);
  RemoveRepoDir();
}

TEST_CASE("Parse single uses revparse", "[git2][revparse]") {
  RefreshRepoDir();
  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  CreateTempFilesWithSameContent(files, "hello world");
  auto repo                    = InitBasicRepo();
  auto [index_oid, index_tree] = git::index::add_files(repo.get(), files);
  auto [commit_oid, _]         = git::commit::create_head(repo.get(), "Init", index_tree.get());

  SECTION("parse head to get commit") {
    auto ret = git::revparse::single(repo.get(), default_branch);
    REQUIRE_FALSE(git::object::id_str(ret.get()).empty());
  }
  RemoveRepoDir();
}

TEST_CASE("Get HEAD", "[git2][repo][commit]") {
  RefreshRepoDir();
  CreateTempFilesWithSameContent({"file0.cpp", "file1.cpp"}, "hello world");
  auto [repo, commit] = InitRepoWithACommit({"file0.cpp", "file1.cpp"});
  auto ref            = git::repo::head(repo.get());
  auto head_commit    = git::ref::peel<git::commit_ptr>(ref.get());
  REQUIRE(git::commit::id_str(head_commit.get()) == git::commit::id_str(commit.get()));
  RemoveRepoDir();
}

TEST_CASE("Push two commits and get diff files", "[git2][diff]") {
  RefreshRepoDir();
  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  CreateTempFilesWithSameContent(files, "hello world");
  auto repo                   = InitBasicRepo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());

  auto head_commit = git::repo::head_commit(repo.get());
  std::cout << git::commit::id_str(head_commit.get()) << "\n";
  REQUIRE(git::commit::id_str(head_commit.get()) == git::commit::id_str(commit1.get()));

  AppendToFile("file1.cpp", "hello world2");
  auto [index_oid2, index2]   = git::index::add_files(repo.get(), {"file1.cpp"});
  auto [commit_oid2, commit2] = git::commit::create_head(repo.get(), "Two", index2.get());
  auto head_commit2           = git::repo::head_commit(repo.get());
  REQUIRE(git::commit::id_str(head_commit2.get()) == git::commit::id_str(commit2.get()));

  auto changed_files = git::diff::changed_files(repo.get(), "HEAD~1", "HEAD");
  REQUIRE(changed_files.size() == 1);
  RemoveRepoDir();
}

TEST_CASE("Simple use of patch ", "[git2][patch]") {
  RefreshRepoDir();
  const auto files = std::vector<std::string>{"file1.cpp", "file2.cpp"};
  CreateTempFilesWithSameContent(files, "hello world");
  auto repo                   = InitBasicRepo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());

  auto head_commit = git::repo::head_commit(repo.get());
  std::cout << git::commit::id_str(head_commit.get()) << "\n";
  REQUIRE(git::commit::id_str(head_commit.get()) == git::commit::id_str(commit1.get()));

  AppendToFile("file1.cpp", "hello world2");
  auto [index_oid2, index2]   = git::index::add_files(repo.get(), {"file1.cpp"});
  auto [commit_oid2, commit2] = git::commit::create_head(repo.get(), "Two", index2.get());
  auto head_commit2           = git::repo::head_commit(repo.get());
  REQUIRE(git::commit::id_str(head_commit2.get()) == git::commit::id_str(commit2.get()));

  auto diff  = git::diff::commit_to_commit(repo.get(), commit1.get(), commit2.get());
  auto patch = git::patch::create_from_diff(diff.get(), 0);
  std::cout << git::patch::to_str(patch.get());

  RemoveRepoDir();
}

TEST_CASE("Create patch from buffers", "[git2][patch]") {
  auto old_content = "int n = 2;"s;
  auto new_content = "double n = 2;"s;
  auto opt         = git_diff_options{};
  git::diff::init_option(&opt);
  auto patch =
    git::patch::create_from_buffers(old_content, "temp.cpp", new_content, "temp.cpp", opt);
  std::cout << git::patch::to_str(patch.get());
}

TEST_CASE("Get file content from a specific commit", "[git2][blob]") {
  RefreshRepoDir();
  const auto files = std::vector<std::string>{"file1.cpp"};
  CreateTempFilesWithSameContent(files, "hello world");
  auto repo                   = InitBasicRepo();
  auto [index_oid1, index1]   = git::index::add_files(repo.get(), files);
  auto [commit_oid1, commit1] = git::commit::create_head(repo.get(), "Init", index1.get());
  auto content                = git::blob::get_raw_content(repo.get(), commit1.get(), "file1.cpp");
  REQUIRE(content == "hello world");

  RemoveRepoDir();
}

int main(int argc, char* argv[]) {
  git::setup();
  int result = Catch::Session().run(argc, argv);
  git::shutdown();
  RemoveRepoDir();
  return result;
}
