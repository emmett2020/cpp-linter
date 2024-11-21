#include <cctype>
#include <filesystem>
#include <git2/diff.h>
#include <print>

#include <spdlog/spdlog.h>
#include <catch2/catch_all.hpp>

#include "catch2/catch_test_macros.hpp"
#include "utils/git_utils.h"

using namespace linter; // NOLINT
using namespace std::string_literals;

TEST_CASE("repo", "[git2][basic]") {
  //
  auto temp_dir = std::filesystem::temp_directory_path();
  auto temp_repo_dir = temp_dir / "test_git";
  if (std::filesystem::exists(temp_repo_dir)) {
    std::filesystem::remove(temp_repo_dir);
  }
  std::filesystem::create_directory(temp_repo_dir);
  std::print("{}", temp_dir.string());

  // SECTION("init a new repo") {
  //   git::repo::init();
  //
  // }
  //
  // SECTION("open") {
  //   git::repo::open(const std::string &repo_path);
  //
  // }

  std::filesystem::remove(temp_repo_dir);
}
