#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_map>

namespace linter::env {

/// TODO: Needs a more robust concurrency env manager components.
class ThreadSafeEnvManager {
public:
  auto GetEnv(const std::string &name, bool force_load = false) -> std::string {
    if (force_load) {
      LoadEnvToMap(name);
    } else {
      LoadEnvToMapIfNotExists(name);
    }
    return env_vars_[name];
  }

private:
  void LoadEnvToMapIfNotExists(const std::string &name) {
    {
      auto lg = std::lock_guard(map_mutex_);
      if (env_vars_.contains(name)) {
        return;
      }
    }
    LoadEnvToMap(name);
  }

  void LoadEnvToMap(const std::string &name) {
    auto val = std::string{};
    {
      auto lg = std::lock_guard(mutex_);
      val = std::getenv(name.data()); // NOLINT
    }
    {
      auto lg = std::lock_guard(map_mutex_);
      env_vars_[name] = val;
    }
  }

  std::mutex mutex_;
  std::mutex map_mutex_;
  std::unordered_map<std::string, std::string> env_vars_;
};

inline auto GetEnvManager() noexcept -> ThreadSafeEnvManager & {
  static ThreadSafeEnvManager env_manager;
  return env_manager;
}

[[nodiscard]] inline auto Load(const std::string &name) -> std::string {
  auto &env_manager = GetEnvManager();
  return env_manager.GetEnv(name);
}

} // namespace linter::env
