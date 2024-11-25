#pragma once

#include <algorithm>
#include <cstdlib>
#include <print>
#include <string>
#include <sys/types.h>
#include <utility>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "utils/util.h"
#include "utils/context.h"
#include "common.h"

namespace linter {
  using namespace std::string_literals;

  class github_api_client {
  public:
    explicit github_api_client(context ctx)
      : ctx_(std::move(ctx)) {
    }

    static void check_http_response(const httplib::Result& response) {
      auto code          = response->status / 100;
      const auto& reason = response->reason;
      throw_unless(code == 1 || code == 2,
                   std::format("http response error since http status code: {}, reason: {}",
                               response->status,
                               reason));
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

    void get_issue_comment_id() {
      spdlog::info("Start to get issue comment id for pull request: {}.", ctx_.pr_number);
      assert(std::ranges::contains(github_events_support_comments, ctx_.event_name));

      auto path    = std::format("/repos/{}/issues/{}/comments", ctx_.repo, ctx_.pr_number);
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github+json"},
        {"Authorization", std::format("token {}", ctx_.token)}
      };
      spdlog::info("Path: {}", path);

      auto response = client.Get(path, headers);

      check_http_response(response);
      spdlog::trace("Get github response body: {}", response->body);

      // data type check
      auto comments = nlohmann::json::parse(response->body);
      if (comments.is_null()) {
        spdlog::info("The pull request number {} doesn't have any comments yet", ctx_.pr_number);
        return;
      }
      throw_unless(comments.is_array(), "issue comments are not an array");
      if (comments.empty()) {
        spdlog::info("The pull request number {} doesn't have any comments yet", ctx_.pr_number);
        return;
      }

      auto comment = std::ranges::find_if(comments, is_our_comment);
      if (comment == comments.end()) {
        spdlog::info("The cpp-lint doesn't comments on pull request number {} yet", ctx_.pr_number);
        return;
      }

      (*comment)["id"].get_to(comment_id_);
      spdlog::info("Successfully got comment id {} of pr {}", comment_id_, ctx_.pr_number);
    }

    void add_issue_comment(const std::string& body) {
      spdlog::info("Start to add issue comment for pr {}", ctx_.pr_number);

      const auto path    = std::format("/repos/{}/issues/{}/comments", ctx_.repo, ctx_.pr_number);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx_.token)}
      };
      spdlog::info("Path: {}", path);

      auto json_body    = nlohmann::json{};
      json_body["body"] = body;
      spdlog::trace("Body:\n{}", json_body.dump());

      auto response = client.Post(path, headers, json_body.dump(), "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);

      auto comment = nlohmann::json::parse(response->body);
      throw_unless(comment.is_object(), "comment isn't object");

      comment["id"].get_to(comment_id_);
      spdlog::info(
        "Successfully add new comment for pull-request {}, the new added comment id is {}",
        ctx_.pr_number,
        comment_id_);
    }

    void update_issue_comment(const std::string& body) {
      throw_if(comment_id_ == -1, "doesn't have comment_id yet");
      throw_if(ctx_.pr_number == -1, "doesn't have comment_id yet");
      spdlog::info("Start to update issue comment");

      const auto path    = std::format("/repos/{}/issues/comments/{}", ctx_.repo, comment_id_);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx_.token)}
      };
      spdlog::info("Path: {}", path);

      auto json_body    = nlohmann::json{};
      json_body["body"] = body;
      spdlog::trace("Body:\n{}", body);

      auto response = client.Post(path, headers, json_body.dump(), "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);
      spdlog::info("Successfully updated comment {} of pr {}", comment_id_, ctx_.pr_number);
    }

    void add_or_update_issue_comment(const std::string& body) {
      if (comment_id_ == -1) {
        add_issue_comment(body);
      } else {
        update_issue_comment(body);
      }
    }

    void post_pull_request_review(const std::string& body) {
      spdlog::info("Start to post pull request review for pr number {}", ctx_.pr_number);

      const auto path    = std::format("/repos/{}/pulls/{}/reviews", ctx_.repo, ctx_.pr_number);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx_.token)}
      };
      spdlog::info("Path: {}", path);


      // debug
      auto json_body    = nlohmann::json{};
      json_body["body"] = body;
      spdlog::trace("Body:\n{}", json_body.dump());

      auto response = client.Post(path, headers, json_body.dump(), "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);

      auto comment = nlohmann::json::parse(response->body);
      throw_unless(comment.is_object(), "comment isn't object");

      comment["id"].get_to(comment_id_);
      spdlog::info(
        "Successfully add new comment for pull-request {}, the new added comment id is {}",
        ctx_.pr_number,
        comment_id_);
    }

    // TODO: support multiple pages.
    void remove_old_pr_reviews() {
      spdlog::info("Start to remove old pr reviews");
      const auto path    = std::format("???", ctx_.repo, comment_id_);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx_.token)}
      };
      spdlog::info("Path: {}", path);

      auto response = client.Get(path, headers);
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);

      auto res_json = nlohmann::json::parse(response->body);
      throw_if(!res_json.is_array(), "response body isn't array");
      for (auto& review: res_json) {
        if (review.contains("body") && review.contains("state")) {
          auto body =  review["body"].get_ref<std::string&>();
          if (!body.starts_with("Marker")) {
            continue;
          }
          auto state =  review["state"].get_ref<std::string&>();
          static const auto unsupported_states = {"PENDING", "DISMISSED"};
          if (std::ranges::contains(unsupported_states, state)) {
            continue;
          }
          throw_unless(review.contains("id"), "id not in review comment");
          auto id = review["id"].get_ref<std::string&>();
          auto dismiss_path = std::format("xxxx/{}/dismissals", id);
          auto data = nlohmann::json{
            {"message","outdated"},
            {"event","DISMISS"}
          };
          auto response = client.Put(path, headers, data.dump());
          check_http_response(response);
        }
      }
    }

    [[nodiscard]] auto ctx() const -> const context& {
      return ctx_;
    }

  private:
    context ctx_;
    std::uint32_t comment_id_ = -1;
    httplib::Client client{github_api};
  };
} // namespace linter
