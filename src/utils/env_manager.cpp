#include "env_manager.h"
#include "utils/util.h"

/// TODO: There is no standard std::setenv

namespace linter::env {

  auto ThreadSafeEnvManager::Get(const std::string &name) -> std::string {
    auto lg = std::lock_guard(mutex_);
    if (cache_.contains(name)) {
      return cache_[name];
    }
    auto ret = std::getenv(name.data()); // NOLINT
    if (ret == nullptr) {
      return "";
    }
    return cache_[name];
  }

  void ThreadSafeEnvManager::SetCache(const std::string &name, const std::string &value) {
    auto lg      = std::lock_guard(mutex_);
    cache_[name] = value;
  }

  void ThreadSafeEnvManager::SetCache(std::unordered_map<std::string, std::string> data) {
    auto lg = std::lock_guard(mutex_);
    cache_  = std::move(data);
  }

  auto GetEnvManager() noexcept -> ThreadSafeEnvManager & {
    static auto env_manager = ThreadSafeEnvManager{};
    return env_manager;
  }

  [[nodiscard]] auto Get(const std::string &name) -> std::string {
    auto &env_manager = GetEnvManager();
    return env_manager.Get(name);
  }

  void SetCache(const std::string &name, const std::string &value) {
    auto &env_manager = GetEnvManager();
    env_manager.SetCache(name, value);
  }

  void SetCache(std::unordered_map<std::string, std::string> data) {
    auto &env_manager = GetEnvManager();
    env_manager.SetCache(std::move(data));
  }

} // namespace linter::env
