#pragma once

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <print>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "utils/env_manager.h"
#include "utils/util.h"
#include "utils/git_utils.h"

namespace linter {
  struct RateLimitHeaders {
    std::size_t reset     = 0;
    std::size_t remaining = 0;
    std::size_t retry     = 0;
  };

  constexpr auto kGithubAPI              = "https://api.github.com";
  constexpr auto kEnableDebug            = "ENABLE_DEBUG";
  constexpr auto kGithubEventPullRequest = "pull_request";
  constexpr auto kGithubEventPush        = "push";

  // Github Actions
  // https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables
  constexpr auto kGithubRepository = "GITHUB_REPOSITORY";
  constexpr auto kGithubToken      = "GITHUB_TOKEN";
  constexpr auto kGithubEventName  = "GITHUB_EVENT_NAME";
  constexpr auto kGithubEventPath  = "GITHUB_EVENT_PATH";
  constexpr auto kGithubSha        = "GITHUB_SHA";

  /// Reads from the actual Github runner.
  struct GithubEnv {
    std::string repository;
    std::string event_name;
    std::string event_path;
    std::string sha;
    std::string token;
  };

  struct Repo {
    explicit Repo(const std::string& repo_path) {
      git::setup();
      repo = git::repo::open(repo_path); // NOLINT
    }

    ~Repo() {
      git::repo::free(repo);
      git::shutdown();
    }

  private:
    git::repo_ptr repo = nullptr;
  };

  class GithubApiClient {
  public:
    GithubApiClient() {
      LoadEnvionmentVariables();
      InferConfigs();
    }

    // Return response
    void SendRequest(std::string_view method,
                     std::string_view payload,
                     const std::unordered_map<std::string, std::string>& headers) {
    }

    auto GetChangedFiles() -> std::vector<std::string> {
      auto path = std::format("/repos/{}", github_env_.repository);
      if (github_env_.event_name == kGithubEventPullRequest) {
        path += std::format("/pulls/{}", pull_request_number_);
      } else {
        ThrowUnless(github_env_.event_name == kGithubEventPush, "unsupported event");
        path += std::format("/commits/{}", github_env_.sha);
      }
      spdlog::info("Fetching changed files from: {}{}", kGithubAPI, path);
      auto client  = httplib::Client{kGithubAPI};
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", github_env_.token)}
      };

      auto response = client.Get(path, headers);
      ThrowIf(response->status != 200,
              std::format("Get changed files failed. Status code: {}", response->status));
      spdlog::debug(response->body);
      return {}; // parse
    }

    void ParseResponse() {
    }

  private:
    void LoadEnvionmentVariables() {
      github_env_.repository = env::get(kGithubRepository);
      github_env_.token      = env::get(kGithubToken);
      github_env_.event_name = env::get(kGithubEventName);
      github_env_.event_path = env::get(kGithubEventPath);
      github_env_.sha        = env::get(kGithubSha);
    }

    void InferConfigs() {
      if (!github_env_.event_path.empty()) {
        auto file = std::ifstream(github_env_.event_name);
        auto data = nlohmann::json::parse(file);
        data.at("number").get_to(pull_request_number_);
      }
    }

    GithubEnv github_env_;
    bool enable_debug_               = false;
    std::size_t pull_request_number_ = -1;
  };

  std::vector<std::string> ParseDiff();

} // namespace linter
