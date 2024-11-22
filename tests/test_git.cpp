#include <cctype>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <print>

#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include "catch2/catch_test_macros.hpp"
#include "utils/git_utils.h"

using namespace linter;
using namespace std::string_literals;
using namespace std::string_view_literals;

const auto temp_dir = std::filesystem::temp_directory_path();
const auto temp_repo_dir = temp_dir / "test_git";
const auto default_branch = "master"sv;

namespace {
auto RefreshRepoDir() {
  if (std::filesystem::exists(temp_repo_dir)) {
    std::print("remove old repo directory");
    std::filesystem::remove_all(temp_repo_dir);
  }
  std::filesystem::create_directory(temp_repo_dir);
}

auto RemoveRepoDir() { std::filesystem::remove_all(temp_repo_dir); }

auto CreateTempFile(const std::string& file_name,
                    const std::string& content) {
  auto new_file_path = temp_repo_dir / file_name;
  if (std::filesystem::exists(new_file_path)) {
    std::filesystem::remove(new_file_path);
  }
  auto file = std::fstream(new_file_path, std::ios::out);
  REQUIRE(file.is_open());
  file << content;
  file.close();
}

} // namespace

TEST_CASE("basics", "[git2][repo]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));
  auto temp_repo_dir_with_git = temp_repo_dir / ".git/";
  REQUIRE(git::repo::path(repo.get()) == temp_repo_dir_with_git);
  RemoveRepoDir();
}

TEST_CASE("basics", "[git2][config]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
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


TEST_CASE("compare with head", "[git2][status]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));

  // default is HEAD
  auto options = git::status::default_options();
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

  auto options = git::status::default_options();
  auto status_list = git::status::gather(repo.get(), options);
  REQUIRE(git::status::entry_count(status_list.get()) == 2);

  const auto* entry0 = git::status::get_by_index(status_list.get(), 0);
  const auto* entry1 = git::status::get_by_index(status_list.get(), 1);
  REQUIRE(entry0->status == git::status_t::GIT_STATUS_INDEX_NEW);
  REQUIRE(entry1->status == git::status_t::GIT_STATUS_INDEX_NEW);

  // we did't have any branches yet.
  auto index_tree_obj = git::tree::lookup(repo.get(), &index_tree_oid);
  auto sig        = git::sig::create_default(repo.get());
  auto commit_oid = git::commit::create(
    repo.get(),
    "HEAD",
    sig.get(),
    sig.get(),
    "Initial commit",
    index_tree_obj.get(),
    0,
    {});
  REQUIRE(git::branch::current_name(repo.get()) == default_branch);
  RemoveRepoDir();
}

// TEST_CASE("basics", "[git2][index]") {
//   RefreshRepoDir();
//   auto repo = git::repo::init(temp_repo_dir, false);
//   REQUIRE(git::repo::is_empty(repo.get()));
//   auto config = git::repo::config(repo.get());
//   git::config::set_string(config.get(), "user.name", "cpp-linter");
//   git::config::set_string(config.get(), "user.email",
//   "cpp-linter@email.com"); auto index      = git::repo::index(repo.get());
//   auto tree_oid   = git::index::write_tree(index.get());
//   auto tree_obj   = git::tree::lookup(repo.get(), &tree_oid);
//   auto sig        = git::sig::create_default(repo.get());
//   auto commit_oid = git::commit::create(
//     repo.get(),
//     "HEAD",
//     sig.get(),
//     sig.get(),
//     "Initial commit",
//     tree_obj.get(),
//     0,
//     {});
//   auto commit_id = git::commit::lookup(repo.get(), &commit_oid);
//   auto ref_id    = git::branch::create(repo.get(), "test", commit_id.get(),
//   true); auto revsion = git::revparse::single(repo.get(), "test");
//   RemoveRepoDir();
// }

// TEST_CASE("basics", "[git2][revparse]") {
//   auto repo = git::repo::open(temp_repo_dir);
//   SECTION("single") {
//     auto *ret = git::revparse::single(repo.get(), "master");
//   }
//   // git::branch::create(repo.get(), "main", );
// }

// TEST_CASE("basics", "[git2][branch]") {
//   auto repo = git::repo::open(temp_repo_dir);
//   SECTION("lookup") {
//     // Empty repository still has a master branch.
//     auto *branch = git::branch::lookup(repo.get(), "master",
//     git::branch_t::local);
//   }
//   // git::branch::create(repo.get(), "main", );
// }

int main(int argc, char *argv[]) {
  git::setup();
  int result = Catch::Session().run(argc, argv);
  git::shutdown();
  RemoveRepoDir();
  return result;
}
