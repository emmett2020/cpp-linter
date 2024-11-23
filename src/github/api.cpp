#include "api.h"

#include <algorithm>

#include "utils/git_utils.h"
#include "utils/util.h"
#include "utils/env_manager.h"

namespace linter {
  auto read_github_env() -> github_env {
    auto env            = github_env{};
    env.repository      = env::get(github_repository);
    env.token           = env::get(github_token);
    env.event_name      = env::get(github_event_name);
    env.base_ref        = env::get(github_base_ref);
    env.head_ref        = env::get(github_head_ref);
    env.github_ref      = env::get(github_ref);
    env.github_sha      = env::get(github_sha);
    env.github_ref_type = env::get(github_ref_type);
    env.workspace       = env::get(github_workspace);
    return env;
  }

  void check_github_env(const github_env &env) {
    throw_if(env.repository.empty(), "empty git repository, check env: GITHUB_REPOSITORY");
    throw_if(env.token.empty(), "empty token, check env: GITHUB_TOKEN");
    throw_if(env.event_name.empty(), "empty git event, check env: GITHUB_EVENT_NAME");
    if (std::ranges::contains(github_events_with_additional_ref, env.event_name)) {
      throw_if(env.base_ref.empty(), "empty base ref, check env: GITHUB_BASE_REF");
      throw_if(env.head_ref.empty(), "empty head ref, check env: GITHUB_HEAD_REF");
    }
    throw_if(env.github_ref.empty(), "empty github ref, check env: GITHUB_REF");
    throw_if(env.github_sha.empty(), "empty github sha, check env: GITHUB_SHA");
    throw_if(env.github_ref_type.empty(), "empty git ref type, check env: GITHUB_REF_TYPE");
    throw_if(env.workspace.empty(), "empty git repository workspace, check env: GITHUB_WORKSPACE");
  }

  void print_github_env(const github_env &env) {
    spdlog::debug("Github Env:");
    spdlog::debug("--------------------------------------------------");
    spdlog::debug("\tgit repository:{}", env.repository);
    spdlog::debug("\tgit token:{}", env.token);
    spdlog::debug("\tgit event name:{}", env.event_name);
    spdlog::debug("\tgit base ref:{}", env.base_ref);
    spdlog::debug("\tgit head ref:{}", env.head_ref);
    spdlog::debug("\tgit ref:{}", env.github_ref);
    spdlog::debug("\tgit sha:{}", env.github_sha);
    spdlog::debug("\tgit ref type:{}", env.github_ref_type);
    spdlog::debug("\tgit workspace:{}", env.workspace);
    spdlog::debug("");
  }

  void fill_context_by_env(const github_env &env, context &ctx) {
    spdlog::debug("Fill context by Github environment variables");
    throw_if(ctx.use_on_local,
             "The `fill_context_by_env` function must be called only on Github CI environment.");
    throw_unless(ctx.token.empty(),
                 "The `token` will be automatically acquired by `GITHUB_TOKEN`. Don't set it from "
                 "program option");
    throw_unless(ctx.repo.empty(),
                 "The `repo` will be automatically acquired by `GITHUB_REPOSITORY` env. Don't set "
                 "it from program option");
    throw_unless(ctx.repo_path.empty(),
                 "The `repo_path` will be automatically acquired by `GITHUB_WORKSPACE` env. Don't "
                 "set it from program option");
    throw_unless(ctx.event_name.empty(),
                 "The `event_name` will be automatically acquired by `GITHUB_EVENT_NAME` env. "
                 "Don't set it from program option");
    throw_unless(ctx.source.empty(),
                 "The `head_ref` will be automatically acquired by `GITHUB_REF` or `GITHUB_SHA` "
                 "env. Don't set it from program option");

    ctx.token      = env.token;
    ctx.repo       = env.repository;
    ctx.repo_path  = env.workspace;
    ctx.event_name = env.event_name;
    ctx.source     = env.github_sha;

    if (std::ranges::contains(github_events_with_additional_ref, ctx.event_name)) {
      spdlog::debug("Github event is {}, so automatically use {} instead of {} as base ref",
                    env.event_name,
                    ctx.target,
                    env.base_ref);
      ctx.target = env.base_ref;
    }
  }

} // namespace linter
