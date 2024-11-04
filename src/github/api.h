#pragma once

#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <string_view>
#include <unordered_map>

#include <httplib.h>

namespace linter {
  struct RateLimitHeaders {
    std::size_t reset     = 0;
    std::size_t remaining = 0;
    std::size_t retry     = 0;
  };

  constexpr auto kApiUrl      = "https://api.github.com";
  constexpr auto kEnableDebug = "ENABLE_DEBUG";
  constexpr auto kPullRequest = "pull_request";

  // Github Actions
  // https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables
  constexpr auto kGithubRepository = "GITHUB_REPOSITORY";
  constexpr auto kGithubEventName  = "GITHUB_EVENT_NAME";
  constexpr auto kGithubEventPath  = "GITHUB_EVENT_PATH";
  constexpr auto kGithubSha        = "GITHUB_SHA";

  /// Reads from the actual Github runner.
  class GithubEnvironment {
   public:
    GithubEnvironment() {
      std::lock_guard<std::mutex> lg{mutex_};
      repository_ = std::getenv(kGithubRepository);
      event_name_ = std::getenv(kGithubEventName);
      event_path_ = std::getenv(kGithubEventPath);
    }


   private:
    std::mutex mutex_;
    std::string repository_;
    std::string event_name_;
    std::string event_path_;
    std::string sha_;
  };

  class GithubApiClient {
   public:
    GithubApiClient()
      : repository_(std::getenv(kGithubRepository))
      , event_name_(std::getenv(kGithubEventName))
      , event_path_(std::getenv(kGithubEventPath))
      , sha_(std::getenv(kGithubSha)) {
      if (!event_path_.empty()) {
      }
    }

    // Return response
    void SendRequest(std::string_view method,
                     std::string_view payload,
                     const std::unordered_map<std::string, std::string>& headers) {
      auto client = httplib::Client{kApiUrl};
      if (method == "GET") {
      }
    }

    auto GetChangedFiles() -> std::vector<std::string> {
      auto uri    = std::format("{}/repos/{}", kApiUrl, repository_);
      auto client = httplib::Client{uri};
      if (event_name_ == kPullRequest) {
        uri += "/pulls/" +
      }
    }

    void ParseResponse() {
    }

   private:
    std::string repository_;
    std::string event_name_;
    std::string event_path_;
    std::string sha_;
    bool enable_debug_           = false;
    std::size_t pull_request_no_ = -1;
  };

} // namespace linter
