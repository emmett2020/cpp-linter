#include "api.h"

namespace linter {
  auto read_github_env() -> github_env {
    auto env               = github_env{};
    env.repository         = env::get(github_repository);
    env.token              = env::get(github_token);
    env.event_name         = env::get(github_event_name);
    env.event_path         = env::get(github_event_path);
    env.base_ref           = env::get(github_base_ref);
    env.head_ref           = env::get(github_head_ref);
    env.triggered_ref      = env::get(github_ref);
    env.triggered_sha      = env::get(github_sha);
    env.triggered_ref_type = env::get(github_ref_type);
    env.workspace          = env::get(github_workspace);
    return env;
  }

  void print_github_env(const github_env& env) {
    spdlog::debug("Git repository:{}", env.repository);
    spdlog::debug("Git token:{}", env.token);
    spdlog::debug("Git event name:{}", env.event_name);
    spdlog::debug("Git event path:{}", env.event_path);
    spdlog::debug("Git base ref:{}", env.base_ref);
    spdlog::debug("Git head ref:{}", env.head_ref);
    spdlog::debug("Git triggered ref:{}", env.triggered_ref);
    spdlog::debug("Git triggered sha:{}", env.triggered_sha);
    spdlog::debug("Git triggered ref_type:{}", env.triggered_ref_type);
    spdlog::debug("Git workspace:{}", env.workspace);
  }

  void merge_env_into_context(const github_env& env, context& ctx) {
    spdlog::debug("Merge github environment into context");
    ctx.repo_path  = env.workspace;
    ctx.repo       = env.repository;
    ctx.token      = env.token;
    ctx.event_name = env.event_name;

    /// TODO: check supported event
    if (env.event_name == github_event_push) {
      ctx.base_ref    = env.triggered_ref;
      ctx.head_ref    = env.triggered_ref;
      ctx.base_commit = "xxx";
      ctx.head_commit = env.triggered_sha;
    } else {
      // pull-request
      ctx.base_ref    = env.base_ref;
      ctx.head_ref    = env.head_ref;
      ctx.base_commit = "xxx";
      ctx.head_commit = env.triggered_sha;
    }
  }

} // namespace linter
