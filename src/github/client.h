/*
 * Copyright (c) 2024 Emmett Zhang
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

#include "common.h"
#include "context.h"
#include "utils/util.h"

namespace linter::github {
  using namespace std::string_literals;

  class client {
  public:
    static void check_http_response(const httplib::Result &response) {
      auto code          = response->status / 100;
      const auto &reason = response->reason;
      throw_unless(code == 1 || code == 2,
                   std::format("http response error: http status code: {}, reason: {}",
                               response->status,
                               reason));
    }

    static void print_request(const httplib::Client &request) {
      spdlog::trace("request: ");
      spdlog::trace("host: {}", request.host());
      spdlog::trace("port: {}", request.port());
    }

    static auto is_our_comment(const nlohmann::json &comment) -> bool {
      if (!comment.contains("/user/login"_json_pointer)) {
        return true;
      }
      auto name = std::string{};
      comment["user"]["login"].get_to(name);
      return name == our_name;
    }

    void get_issue_comment_id(const runtime_context &ctx) {
      spdlog::info("Start to get issue comment id for pull request: {}.", ctx.pr_number);
      assert(std::ranges::contains(github_events_support_comments, ctx.event_name));

      auto path    = std::format("/repos/{}/issues/{}/comments", ctx.repo, ctx.pr_number);
      auto headers = httplib::Headers{
        {"Accept", "application/vnd.github+json"},
        {"Authorization", std::format("token {}", ctx.token)}
      };
      spdlog::info("Http request path: {}", path);

      auto response = client.Get(path, headers);

      check_http_response(response);
      spdlog::trace("Get github response body: {}", response->body);

      // data type check
      auto comments = nlohmann::json::parse(response->body);
      if (comments.is_null()) {
        spdlog::info("The pull request number {} doesn't have any comments yet", ctx.pr_number);
        return;
      }
      throw_unless(comments.is_array(), "issue comments are not an array");
      if (comments.empty()) {
        spdlog::info("The pull request number {} doesn't have any comments yet", ctx.pr_number);
        return;
      }

      auto comment = std::ranges::find_if(comments, is_our_comment);
      if (comment == comments.end()) {
        spdlog::info("The cpp-lint doesn't comments on pull request number {} yet", ctx.pr_number);
        return;
      }

      (*comment)["id"].get_to(comment_id_);
      spdlog::info("Successfully got comment id {} of pr {}", comment_id_, ctx.pr_number);
    }

    void add_issue_comment(const runtime_context &ctx, const std::string &body) {
      spdlog::info("Start to add issue comment for pr {}", ctx.pr_number);

      const auto path    = std::format("/repos/{}/issues/{}/comments", ctx.repo, ctx.pr_number);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx.token)}
      };
      spdlog::info("Http request path: {}", path);

      auto json_body    = nlohmann::json{};
      json_body["body"] = body;
      spdlog::trace("Http request body:\n{}", json_body.dump());

      auto response = client.Post(path, headers, json_body.dump(), "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);

      auto comment = nlohmann::json::parse(response->body);
      throw_unless(comment.is_object(), "comment isn't object");

      comment["id"].get_to(comment_id_);
      spdlog::info("Successfully add new comment for pull-request {}, the new "
                   "added comment id is {}",
                   ctx.pr_number,
                   comment_id_);
    }

    void update_issue_comment(const runtime_context &ctx, const std::string &body) {
      throw_if(comment_id_ == -1, "the client doesn't have comment_id yet");
      throw_if(ctx.pr_number == -1, "the context doesn't have pr-number yet");
      spdlog::info("Start to update issue comment");

      const auto path    = std::format("/repos/{}/issues/comments/{}", ctx.repo, comment_id_);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx.token)}
      };
      spdlog::info("Http request path: {}", path);

      auto json_body    = nlohmann::json{};
      json_body["body"] = body;
      spdlog::trace("Http request body:\n{}", json_body.dump());

      auto response = client.Post(path, headers, json_body.dump(), "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);
      spdlog::info("Successfully updated comment {} of pr {}", comment_id_, ctx.pr_number);
    }

    void add_or_update_issue_comment(const runtime_context &ctx, const std::string &body) {
      if (comment_id_ == -1) {
        add_issue_comment(ctx, body);
      } else {
        update_issue_comment(ctx, body);
      }
    }

    void post_pull_request_review(const runtime_context &ctx, const std::string &body) {
      spdlog::info("Start to post pull request review for pr number {}", ctx.pr_number);

      const auto path    = std::format("/repos/{}/pulls/{}/reviews", ctx.repo, ctx.pr_number);
      const auto headers = httplib::Headers{
        {"Accept", "application/vnd.github.use_diff"},
        {"Authorization", std::format("token {}", ctx.token)}
      };
      spdlog::info("Http request path: {}", path);
      spdlog::trace("Http request body:\n{}", body);

      auto response = client.Post(path, headers, body, "text/plain");
      spdlog::trace("Get github response body: {}", response->body);
      check_http_response(response);

      spdlog::info("Successfully post pull_request_review for pull-request {}", ctx.pr_number);
    }

    // TODO: support multiple pages.
    // void remove_old_pr_reviews() {
    //   spdlog::info("Start to remove old pr reviews");
    //   const auto path    = std::format("???", ctx_.repo, comment_id_);
    //   const auto headers = httplib::Headers{
    //     {"Accept", "application/vnd.github.use_diff"},
    //     {"Authorization", std::format("token {}", ctx_.token)}
    //   };
    //   spdlog::info("Path: {}", path);
    //
    //   auto response = client.Get(path, headers);
    //   spdlog::trace("Get github response body: {}", response->body);
    //   check_http_response(response);
    //
    //   auto res_json = nlohmann::json::parse(response->body);
    //   throw_if(!res_json.is_array(), "response body isn't array");
    //   for (auto& review: res_json) {
    //     if (review.contains("body") && review.contains("state")) {
    //       auto body =  review["body"].get_ref<std::string&>();
    //       if (!body.starts_with("Marker")) {
    //         continue;
    //       }
    //       auto state =  review["state"].get_ref<std::string&>();
    //       static const auto unsupported_states = {"PENDING", "DISMISSED"};
    //       if (std::ranges::contains(unsupported_states, state)) {
    //         continue;
    //       }
    //       throw_unless(review.contains("id"), "id not in review comment");
    //       auto id = review["id"].get_ref<std::string&>();
    //       auto dismiss_path = std::format("xxxx/{}/dismissals", id);
    //       auto data = nlohmann::json{
    //         {"message","outdated"},
    //         {"event","DISMISS"}
    //       };
    //       auto response = client.Put(path, headers, data.dump());
    //       check_http_response(response);
    //     }
    //   }
    // }

  private:
    std::uint32_t comment_id_ = -1;
    httplib::Client client{github_api};
  };
} // namespace linter::github
