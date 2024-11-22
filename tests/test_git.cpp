#include <cctype>
#include <filesystem>
#include <fstream>
#include <ios>
#include <print>

#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include "catch2/catch_test_macros.hpp"
#include "utils/git_utils.h"

using namespace linter;
using namespace std::string_literals;

const auto temp_dir      = std::filesystem::temp_directory_path();
const auto temp_repo_dir = temp_dir / "test_git";

namespace {
  auto RefreshRepoDir() {
    if (std::filesystem::exists(temp_repo_dir)) {
      std::print("remove old repo directory");
      std::filesystem::remove_all(temp_repo_dir);
    }
    std::filesystem::create_directory(temp_repo_dir);
  }

  auto RemoveRepoDir() {
    std::filesystem::remove_all(temp_repo_dir);
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

TEST_CASE("basics", "[git2][index]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));
  auto config = git::repo::config(repo.get());
  git::config::set_string(config.get(), "user.name", "cpp-linter");
  git::config::set_string(config.get(), "user.email", "cpp-linter@email.com");

  auto index      = git::repo::index(repo.get());
  SECTION("add a file to index") {
    auto file_path = temp_repo_dir / "temp_file.cpp";
    auto file = std::fstream(file_path, std::ios::out);
    REQUIRE(file.is_open());
    file << "hello world";
    file.close();
    git::index::add_by_path(index.get(), "temp_file.cpp");
    git::index::write(index.get());
  }
  // RemoveRepoDir();
}

// TEST_CASE("basics", "[git2][index]") {
//   RefreshRepoDir();
//   auto repo = git::repo::init(temp_repo_dir, false);
//   REQUIRE(git::repo::is_empty(repo.get()));
//   auto config = git::repo::config(repo.get());
//   git::config::set_string(config.get(), "user.name", "cpp-linter");
//   git::config::set_string(config.get(), "user.email", "cpp-linter@email.com");
//   auto index      = git::repo::index(repo.get());
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
//   auto ref_id    = git::branch::create(repo.get(), "test", commit_id.get(), true);
//   auto revsion = git::revparse::single(repo.get(), "test");
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
//     auto *branch = git::branch::lookup(repo.get(), "master", git::branch_t::local);
//   }
//   // git::branch::create(repo.get(), "main", );
// }

int main(int argc, char *argv[]) {
  git::setup();
  int result = Catch::Session().run(argc, argv);
  git::shutdown();
  return result;
}
