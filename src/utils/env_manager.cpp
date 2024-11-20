#include "env_manager.h"

/// TODO: There is no standard std::setenv

namespace linter::env {

  auto thread_safe_env_manager::get_instance() noexcept -> thread_safe_env_manager & {
    static auto env_manager = thread_safe_env_manager{};
    return env_manager;
  }

  auto thread_safe_env_manager::get(const std::string &name) -> std::string {
    auto lg = std::lock_guard(mutex_);
    if (cache_.contains(name)) {
      return cache_[name];
    }
    auto ret = std::getenv(name.data()); // NOLINT
    if (ret == nullptr) {
      return "";
    }
    cache_[name] = ret;
    return cache_[name];
  }

  void thread_safe_env_manager::set_cache(const std::string &name, const std::string &value) {
    auto lg      = std::lock_guard(mutex_);
    cache_[name] = value;
  }

  void thread_safe_env_manager::set_cache(std::unordered_map<std::string, std::string> data) {
    auto lg = std::lock_guard(mutex_);
    cache_  = std::move(data);
  }

  [[nodiscard]] auto get(const std::string &name) -> std::string {
    auto &env_manager = thread_safe_env_manager::get_instance();
    return env_manager.get(name);
  }

  void set_cache(const std::string &name, const std::string &value) {
    auto &env_manager = thread_safe_env_manager::get_instance();
    env_manager.set_cache(name, value);
  }

  void set_cache(std::unordered_map<std::string, std::string> data) {
    auto &env_manager = thread_safe_env_manager::get_instance();
    env_manager.set_cache(std::move(data));
  }

} // namespace linter::env
