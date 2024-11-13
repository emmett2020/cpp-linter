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
  struct rate_limit_headers {
    std::size_t reset     = 0;
    std::size_t remaining = 0;
    std::size_t retry     = 0;
  };

  constexpr auto github_api                = "https://api.github.com";
  constexpr auto github_event_pull_request = "pull_request";
  constexpr auto github_event_push         = "push";

  // Github Actions
  // https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables
  constexpr auto github_repository = "GITHUB_REPOSITORY";
  constexpr auto github_token      = "GITHUB_TOKEN";
  constexpr auto github_event_name = "GITHUB_EVENT_NAME";
  constexpr auto github_event_path = "GITHUB_EVENT_PATH";
  constexpr auto github_sha        = "GITHUB_SHA";
  constexpr auto github_ref        = "GITHUB_REF";

  /// Reads from the actual Github runner.
  struct github_env {
    std::string repository;
    std::string event_name;
    std::string event_path;
    std::string sha;
    std::string token;
  };

  class github_api_client {
  public:
    github_api_client() {
      load_envionment_variables();
      infer_configs();
    }

    static void check_http_status_code(int code) {
      throw_if(code == 404, "Resource not found");
      throw_if(code == 422, "Validation failed");
      throw_if(code != 200, std::format("http status code error: {}", code));
    }

    bool update_issue_comment() {
      spdlog::info("Updating issue comment");
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github+json"},
        {"Authorization", std::format("token {}", github_env_.token)}
      };
      auto path     = std::format("/repos/{}/issues/comments", github_env_.repository);
      auto response = client.Get(path, headers);
      check_http_status_code(response->status);
      spdlog::trace(response->body);
      return true;
    }

    auto get_changed_files() -> std::vector<std::string> {
      auto path = std::format("/repos/{}", github_env_.repository);
      if (github_env_.event_name == github_event_pull_request) {
        path += std::format("/pulls/{}", pull_request_number_);
      } else {
        throw_unless(github_env_.event_name == github_event_push, "unsupported event");
        path += std::format("/commits/{}", github_env_.sha);
      }
      spdlog::info("Fetching changed files from: {}{}", github_api, path);
      auto client  = httplib::Client{github_api};
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", github_env_.token)}
      };

      auto response = client.Get(path, headers);
      throw_if(response->status != 200,
               std::format("Get changed files failed. Status code: {}", response->status));
      spdlog::debug(response->body);
      return {}; // parse
    }

  private:
    void load_envionment_variables() {
      github_env_.repository = env::get(github_repository);
      github_env_.token      = env::get(github_token);
      github_env_.event_name = env::get(github_event_name);
      github_env_.event_path = env::get(github_event_path);
      github_env_.sha        = env::get(github_sha);
    }

    void infer_configs() {
      if (!github_env_.event_path.empty()) {
        auto file = std::ifstream(github_env_.event_name);
        auto data = nlohmann::json::parse(file);
        data.at("number").get_to(pull_request_number_);
      }
    }

    github_env github_env_;
    bool enable_debug_               = false;
    std::size_t pull_request_number_ = -1;
    httplib::Client client{github_api};
  };
} // namespace linter
