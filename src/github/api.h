#pragma once

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <string_view>
#include <unordered_map>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "utils/env_manager.h"
#include "utils/util.h"

namespace linter {
struct RateLimitHeaders {
  std::size_t reset = 0;
  std::size_t remaining = 0;
  std::size_t retry = 0;
};

constexpr auto kApiUrl = "https://api.github.com";
constexpr auto kEnableDebug = "ENABLE_DEBUG";
constexpr auto kEventPullRequest = "pull_request";
constexpr auto kEventPush = "pull_request";

// Github Actions
// https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables
constexpr auto kGithubRepository = "GITHUB_REPOSITORY";
constexpr auto kGithubToken = "GITHUB_TOKEN";
constexpr auto kGithubEventName = "GITHUB_EVENT_NAME";
constexpr auto kGithubEventPath = "GITHUB_EVENT_PATH";
constexpr auto kGithubSha = "GITHUB_SHA";

/// Reads from the actual Github runner.
struct GithubEnv {
  std::string repository;
  std::string event_name;
  std::string event_path;
  std::string sha;
  std::string token;
};

class GithubApiClient {
public:
  GithubApiClient() { LoadEnvionmentVariables(); }

  // Return response
  void
  SendRequest(std::string_view method, std::string_view payload,
              const std::unordered_map<std::string, std::string> &headers) {}

  auto GetChangedFiles() -> std::vector<std::string> {
    auto uri = std::format("{}/repos/{}", kApiUrl, github_env_.repository);
    if (github_env_.event_name == kEventPullRequest) {
      uri += std::format("/pulls/{}", pull_request_number_);
    } else {
      ThrowUnless(github_env_.event_name == kEventPush, "unsupported event");
      uri += std::format("/commits/{}", github_env_.sha);
    }
    spdlog::info("Fetching changed files from: {}", uri);
    auto client = httplib::Client{uri};
    auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", github_env_.token)}};
    auto response = client.Get("", headers);
    assert(response->status == 200);
    return {}; // parse
  }

  void ParseResponse() {}

private:
  void LoadEnvionmentVariables() {
    github_env_.repository = env::Load(kGithubRepository);
    github_env_.token = env::Load(kGithubToken);
    github_env_.event_name = env::Load(kGithubEventName);
    github_env_.event_path = env::Load(kGithubEventPath);
    github_env_.sha = env::Load(kGithubSha);
    if (!github_env_.event_path.empty()) {
      auto file = std::ifstream(github_env_.event_name);
      auto data = nlohmann::json::parse(file);
      data.at("number").get_to(pull_request_number_);
    }
  }

  GithubEnv github_env_;
  bool enable_debug_ = false;
  std::size_t pull_request_number_ = -1;
};

} // namespace linter
