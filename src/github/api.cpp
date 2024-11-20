#include "api.h"
#include "utils/git_utils.h"
#include "utils/util.h"
#include <utility>

namespace linter {
  auto read_github_env() -> github_env {
    auto env            = github_env{};
    env.repository      = env::get(github_repository);
    env.token           = env::get(github_token);
    env.event_name      = env::get(github_event_name);
    env.event_path      = env::get(github_event_path);
    env.base_ref        = env::get(github_base_ref);
    env.head_ref        = env::get(github_head_ref);
    env.github_ref      = env::get(github_ref);
    env.github_sha      = env::get(github_sha);
    env.github_ref_type = env::get(github_ref_type);
    env.workspace       = env::get(github_workspace);
    return env;
  }

  void print_github_env(const github_env& env) {
    spdlog::debug("Github Env:");
    spdlog::debug("--------------------------------------------------");
    spdlog::debug("\tgit repository:{}", env.repository);
    spdlog::debug("\tgit token:{}", env.token);
    spdlog::debug("\tgit event name:{}", env.event_name);
    spdlog::debug("\tgit event path:{}", env.event_path);
    spdlog::debug("\tgit base ref:{}", env.base_ref);
    spdlog::debug("\tgit head ref:{}", env.head_ref);
    spdlog::debug("\tgit ref:{}", env.github_ref);
    spdlog::debug("\tgit sha:{}", env.github_sha);
    spdlog::debug("\tgit ref type:{}", env.github_ref_type);
    spdlog::debug("\tgit workspace:{}", env.workspace);
    spdlog::debug("");
  }

  void merge_env_into_context(const github_env& env, context& ctx) {
    spdlog::debug("Merge github environment into context");
    ctx.repo_path  = env.workspace;
    ctx.repo       = env.repository;
    ctx.token      = env.token;
    ctx.event_name = env.event_name;

    throw_if(std::ranges::count(supported_github_event, ctx.event_name) == 0,
             std::format("unsupported event name: {}", ctx.event_name));

    git::branch::lookup(repo, ctx.default_branch, git::branch_t::remote);

    if (env.event_name == github_event_push) {
      ctx.base_ref    = ctx.default_branch;
      ctx.head_ref    = env.github_ref;
      ctx.base_commit = "";
      ctx.head_commit = env.github_sha;
    } else if (env.event_name == github_event_pull_request) {
      ctx.base_ref    = env.base_ref;
      ctx.head_ref    = env.head_ref;
      ctx.base_commit = "xxx";
      ctx.head_commit = env.github_sha;
    } else if (env.event_name == github_event_pull_request_target) {
    } else if (env.event_name == github_event_workflow_dispatch) {
    } else {
      std::unreachable();
    }
  }

} // namespace linter
