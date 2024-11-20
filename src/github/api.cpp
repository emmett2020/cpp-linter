#include "api.h"

#include <git2.h>
#include <git2/repository.h>
#include <git2/types.h>

#include <httplib.h>
#include <vector>

namespace linter {
    auto read_github_env() -> github_env {
      auto env = github_env{};
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
      spdlog::debug("git repository:{}",env.repository);
      spdlog::debug("git token:{}",env.token);
      spdlog::debug("git event name:{}",env.event_name);
      spdlog::debug("git event path:{}",env.event_path);
      spdlog::debug("git base ref:{}",env.base_ref);
      spdlog::debug("git head ref:{}",env.head_ref);
      spdlog::debug("git triggered ref:{}",env.triggered_ref);
      spdlog::debug("git triggered sha:{}",env.triggered_sha);
      spdlog::debug("git triggered ref_type:{}",env.triggered_ref_type);
      spdlog::debug("git workspace:{}",env.workspace);
    }


    void merge_env_into_context(const github_env& env, context& ctx) {}

} // namespace linter
