#include <cctype>
#include <filesystem>
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
  auto config = git::config::snapshot(origin.get());
  SECTION("set_string/get_string") {
    git::config::set_string(config.get(), "user.name", "test");
    REQUIRE(git::config::get_string(config.get(), "user.name") == "test");
  }
  SECTION("set_bool/get_bool") {
    git::config::set_bool(config.get(), "core.filemode", true);
    REQUIRE(git::config::get_bool(config.get(), "core.filemode") == true);
  }
  RemoveRepoDir();
}

TEST_CASE("basics", "[git2][index]") {
  RefreshRepoDir();
  auto repo = git::repo::init(temp_repo_dir, false);
  REQUIRE(git::repo::is_empty(repo.get()));
  auto config = git::repo::config(repo.get());
  git::config::set_string(config.get(), "user.name", "test");
  auto index = git::repo::index(repo.get());
  auto oid   = git::index::write_tree(index.get());
  auto sig   = git::sig::create_default(repo.get());
  RemoveRepoDir();
}

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
