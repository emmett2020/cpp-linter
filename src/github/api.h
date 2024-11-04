#pragma once

#include <cstddef>
#include <cstdlib>
#include <string_view>
#include <unordered_map>

#include <httplib.h>

#include "utils/env_manager.h"

namespace linter {
struct RateLimitHeaders {
  std::size_t reset = 0;
  std::size_t remaining = 0;
  std::size_t retry = 0;
};

constexpr auto kApiUrl = "https://api.github.com";
constexpr auto kEnableDebug = "ENABLE_DEBUG";
constexpr auto kPullRequest = "pull_request";

// Github Actions
// https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables
constexpr auto kGithubRepository = "GITHUB_REPOSITORY";
constexpr auto kGithubEventName = "GITHUB_EVENT_NAME";
constexpr auto kGithubEventPath = "GITHUB_EVENT_PATH";
constexpr auto kGithubSha = "GITHUB_SHA";

/// Reads from the actual Github runner.
struct GithubEnv {
  std::string repository;
  std::string event_name;
  std::string event_path;
  std::string sha;
};

class GithubApiClient {
public:
  GithubApiClient() { LoadEnvionmentVariables(); }

  // Return response
  void
  SendRequest(std::string_view method, std::string_view payload,
              const std::unordered_map<std::string, std::string> &headers) {}

  auto GetChangedFiles() -> std::vector<std::string> {
    auto uri = std::format("{}/repos/{}", kApiUrl, github_env.repository);
    auto client = httplib::Client{uri};
    if (github_env.event_name == kPullRequest) {
      uri += std::format("/pulls/{}", pull_request_number_);
    }
  }

  void ParseResponse() {}

private:
  void LoadEnvionmentVariables() {
    github_env.repository = env::Load(kGithubRepository);
    github_env.event_name = env::Load(kGithubEventName);
    github_env.event_path = env::Load(kGithubEventPath);
    github_env.sha = env::Load(kGithubSha);
  }

  GithubEnv github_env;
  bool enable_debug_ = false;
  std::size_t pull_request_number_ = -1;
};

} // namespace linter
