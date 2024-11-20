#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <ranges>
#include <print>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

#include "nlohmann/json_fwd.hpp"
#include "utils/env_manager.h"
#include "utils/util.h"

namespace linter {
  struct rate_limit_headers {
    std::size_t reset     = 0;
    std::size_t remaining = 0;
    std::size_t retry     = 0;
  };

  constexpr auto our_name                         = "emmett2020"; // For test
  constexpr auto github_api                       = "https://api.github.com";
  constexpr auto github_event_pull_request        = "pull_request";
  constexpr auto github_event_pull_request_target = "pull_request_target";
  constexpr auto github_event_push                = "push";

  // Github Actions
  // https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables
  constexpr auto github_actions    = "GITHUB_ACTIONS";
  constexpr auto github_repository = "GITHUB_REPOSITORY"; // The owner and repository name
  constexpr auto github_token      = "GITHUB_TOKEN";
  constexpr auto github_event_name = "GITHUB_EVENT_NAME";
  constexpr auto github_event_path = "GITHUB_EVENT_PATH";


  /// The default working directory on the runner for steps, and the default
  /// location of your repository when using the checkout action. For example,
  /// /home/runner/work/my-repo-name/my-repo-name.
  constexpr auto github_workspace = "GITHUB_WORKSPACE";

  /// The commit SHA that triggered the workflow. The value of this commit SHA
  /// depends on the event that triggered the workflow. For more information, see
  /// "Events that trigger workflows." For example,
  /// ffac537e6cbbf934b08745a378932722df287a53.
  constexpr auto github_sha = "GITHUB_SHA";

  /// The name of the base ref or target branch of the pull request in a
  /// workflow run. This is only set when the event that triggers a workflow
  /// run is either pull_request or pull_request_target. For example, main.
  constexpr auto github_base_ref = "GITHUB_BASE_REF";

  /// The head ref or source branch of the pull request in a workflow run. This
  /// property is only set when the event that triggers a workflow run is either
  /// pull_request or pull_request_target. For example, feature-branch-1.
  constexpr auto github_head_ref = "GITHUB_HEAD_REF";

  /// The fully-formed ref of the branch or tag that triggered the workflow
  /// run. For workflows triggered by push, this is the branch or tag ref that
  /// was pushed. For workflows triggered by pull_request, this is the pull
  /// request merge branch. For workflows triggered by release, this is the
  /// release tag created. For other triggers, this is the branch or tag ref that
  /// triggered the workflow run. This is only set if a branch or tag is
  /// available for the event type. The ref given is fully-formed, meaning that
  /// for branches the format is refs/heads/<branch_name>, for pull requests it
  /// is refs/pull/<pr_number>/merge, and for tags it is refs/tags/<tag_name>.
  /// For example, refs/heads/feature-branch-1
  constexpr auto github_ref = "GITHUB_REF";

  /// The type of ref that triggered the workflow run. Valid values are branch or tag
  constexpr auto github_ref_type = "GITHUB_REF";

  /// Reads from the actual Github runner.
  struct github_env {
    std::string repository;
    std::string event_name;
    std::string event_path;
    std::string base_ref;      // Only used in pr event
    std::string head_ref;      // Only used in pr event
    std::string triggered_ref; // The ref triggered the workflow. Used in all events.
    std::string triggered_sha;
    std::string triggered_ref_type;
    std::string workspace;
    std::string token;
  };

  class github_api_client {
  public:
    github_api_client() {
      infer_configs();
      if (env_.event_name == github_event_pull_request) {
        parse_pr_number();
      }
    }

    static void check_http_response(const httplib::Result& response) {
      auto code          = response->status / 100;
      const auto& reason = response->reason;
      throw_unless(code == 1 || code == 2,
                   std::format("Got http status code: {}, reason: {}", response->status, reason));
    }

    static void print_request(const httplib::Client& request) {
      spdlog::trace("request: ");
      spdlog::trace("host: {}", request.host());
      spdlog::trace("port: {}", request.port());
    }

    static auto is_our_comment(const nlohmann::json& comment) -> bool {
      if (!comment.contains("/user/login"_json_pointer)) {
        return true;
      }
      auto name = std::string{};
      comment["user"]["login"].get_to(name);
      return name == our_name;
    }

    // TODO: Maybe do it in another thread to speed up or do it in same thread to eliminate call times?
    void get_issue_comment_id() {
      spdlog::info("Start to get issue comment id for pull request: {}.", pr_number_);
      assert(env_.event_name == github_event_pull_request);

      auto path    = std::format("/repos/{}/issues/{}/comments", env_.repository, pr_number_);
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github+json"},
        {"Authorization", std::format("token {}", env_.token)}
      };
      spdlog::trace("path: {}", path);

      auto response = client.Get(path, headers);

      check_http_response(response);
      spdlog::trace("Get github response body: {}", response->body);

      // data type check
      auto comments = nlohmann::json::parse(response->body);
      if (comments.is_null()) {
        spdlog::info("The pull request number {} doesn't have any comments yet", pr_number_);
        return;
      }
      throw_unless(comments.is_array(), "issue comments are not an array");
      if (comments.empty()) {
        spdlog::info("The pull request number {} doesn't have any comments yet", pr_number_);
        return;
      }

      auto comment = std::ranges::find_if(comments, is_our_comment);
      if (comment == comments.end()) {
        spdlog::info("The lint doesn't comments on pull request number {} yet", pr_number_);
        return;
      }

      (*comment)["id"].get_to(comment_id_);
      spdlog::info("Got comment id {} in  pr {}", comment_id_, pr_number_);
    }

    void add_comment(const std::string& body) {
      spdlog::info("Start to add issue comment for pr {}", pr_number_);

      const auto path    = std::format("/repos/{}/issues/{}/comments", env_.repository, pr_number_);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", env_.token)}
      };
      spdlog::trace("Path: {}, Body: {}", path, body);

      auto response = client.Post(path, headers, body, "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);

      auto comment = nlohmann::json::parse(response->body);
      throw_unless(comment.is_object(), "comment isn't object");

      comment["id"].get_to(comment_id_);
      spdlog::info("The new added comment id is {}", comment_id_);
    }

    void update_comment(const std::string& body) {
      throw_if(comment_id_ == -1, "doesn't have comment_id yet");
      throw_if(pr_number_ == -1, "doesn't have comment_id yet");
      spdlog::info("Start to update issue comment");

      const auto path = std::format("/repos/{}/issues/comments/{}", env_.repository, comment_id_);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", env_.token)}
      };
      spdlog::trace("Path: {}, Body: {}", path, body);

      auto response = client.Post(path, headers, body, "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);
      spdlog::info("Successfully update comment {} of pr {}", comment_id_, pr_number_);
    }

    void add_or_update_comment(const std::string& body) {
      if (comment_id_ == -1) {
        add_comment(body);
      } else {
        update_comment(body);
      }
    }

    auto get_changed_files() -> std::vector<std::string> {
      auto path = std::format("/repos/{}", env_.repository);
      if (env_.event_name == github_event_pull_request) {
        path += std::format("/pulls/{}", pr_number_);
      } else {
        throw_unless(env_.event_name == github_event_push, "unsupported event");
        path += std::format("/commits/{}", env_.triggered_sha);
      }
      spdlog::info("Fetching changed files from: {}{}", github_api, path);
      auto client  = httplib::Client{github_api};
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", env_.token)}
      };

      auto response = client.Get(path, headers);
      throw_if(response->status != 200,
               std::format("Get changed files failed. Status code: {}", response->status));
      spdlog::debug(response->body);
      return {}; // parse
    }

    [[nodiscard]] auto env() const -> const github_env& {
      return env_;
    }

    void read_environment_variables() {
      env_.repository         = env::get(github_repository);
      env_.token              = env::get(github_token);
      env_.event_name         = env::get(github_event_name);
      env_.event_path         = env::get(github_event_path);
      env_.base_ref           = env::get(github_base_ref);
      env_.head_ref           = env::get(github_head_ref);
      env_.triggered_ref      = env::get(github_ref);
      env_.triggered_sha      = env::get(github_sha);
      env_.triggered_ref_type = env::get(github_ref_type);
      env_.workspace          = env::get(github_workspace);
    }

    void print_environment_variables() {
      spdlog::debug("git repository:{}",env_.repository);
      spdlog::debug("git token:{}",env_.token);
      spdlog::debug("git event name:{}",env_.event_name);
      spdlog::debug("git event path:{}",env_.event_path);
      spdlog::debug("git base ref:{}",env_.base_ref);
      spdlog::debug("git head ref:{}",env_.head_ref);
      spdlog::debug("git triggered ref:{}",env_.triggered_ref);
      spdlog::debug("git triggered sha:{}",env_.triggered_sha);
      spdlog::debug("git triggered ref_type:{}",env_.triggered_ref_type);
      spdlog::debug("git workspace:{}",env_.workspace);
    }

    [[nodiscard]] auto base_commit() const -> const std::string& {
      if (env_.event_name == github_event_push) {
        throw_if(true, "unsupported");
      }
      if (this->env_.event_name == github_event_pull_request) {
        return this->env_.triggered_sha;
      }

      throw_if(true, "unsupported");
      std::unreachable();
    }

    [[nodiscard]] auto head_commit() const -> const std::string& {
      if (env_.event_name == github_event_push) {
        throw_if(true, "unsupported");
      }
      if (this->env_.event_name == github_event_pull_request) {
        return this->env_.triggered_sha;
      }

      throw_if(true, "unsupported");
      std::unreachable();
    }

    [[nodiscard]] auto repo_full_path() const -> const std::string& {
      return env_.workspace;
    }

  private:
    /// PR merge branch refs/pull/PULL_REQUEST_NUMBER/merge
    void parse_pr_number() {
      assert(!env_.head_ref.empty());
      auto parts = std::views::split(env_.head_ref, '/')
                 | std::ranges::to<std::vector<std::string>>();
      throw_if(parts.size() != 4, std::format("source ref format error: {}", env_.head_ref));
      pr_number_ = std::stoi(parts[2]);
    }

    void infer_configs() {
      if (!env_.event_path.empty()) {
        auto file = std::ifstream(env_.event_name);
        auto data = nlohmann::json::parse(file);
        data.at("number").get_to(pr_number_);
      }
    }

    github_env env_;
    bool enable_debug_        = false;
    std::size_t pr_number_    = -1;
    std::uint32_t comment_id_ = -1;
    httplib::Client client{github_api};
  };
} // namespace linter
