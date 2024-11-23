#pragma once

#include <cstddef>
#include <cstdlib>
#include <print>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "utils/context.h"

namespace linter {
  struct rate_limit_headers {
    std::size_t reset     = 0;
    std::size_t remaining = 0;
    std::size_t retry     = 0;
  };

  constexpr auto our_name                         = "emmett2020"; // For test
  constexpr auto github_api                       = "https://api.github.com";
  constexpr auto github_event_push                = "push";
  constexpr auto github_event_pull_request        = "pull_request";
  constexpr auto github_event_pull_request_target = "pull_request_target";
  constexpr auto github_event_workflow_dispatch   = "workflow_dispatch";

  constexpr auto all_github_events =
    {github_event_push, github_event_pull_request, github_event_pull_request_target};
  constexpr auto github_events_with_additional_ref = {github_event_pull_request,
                                                      github_event_pull_request_target};
  constexpr auto github_events_support_comments    = {github_event_pull_request,
                                                      github_event_pull_request_target};
  constexpr auto github_events_support_pr_number   = {github_event_pull_request,
                                                      github_event_pull_request_target};

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
    std::string token;
    std::string event_name;
    std::string base_ref;
    std::string head_ref;
    std::string github_ref;
    std::string github_sha;
    std::string github_ref_type;
    std::string workspace;
  };

  auto read_github_env() -> github_env;
  void check_github_env(const github_env& env);
  void print_github_env(const github_env& env);
  void fill_context_by_env(const github_env& env, context& ctx);
} // namespace linter
