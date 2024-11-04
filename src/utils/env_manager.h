#include "utils/util.h"
#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_map>

namespace linter::env {

  /// TODO: Needs a more robust concurrency env manager components.
  class ThreadSafeEnvManager {
  public:
    auto GetEnv(const std::string &name) -> std::string {
      {
        auto lg = std::lock_guard(data_mutex_);
        if (data_.contains(name)) {
          return data_[name];
        }
      }
      LoadEnvValToMap(name);
      return data_[name];
    }


  private:
    auto LoadEnvValToMap(const std::string &name) -> bool {
      auto val = std::string{};
      {
        auto lg  = std::lock_guard(getenv_mutex_);
        auto ret = std::getenv(name.data()); // NOLINT
        if (ret == nullptr) {
          return false;
        }
        val = ret;
      }
      {
        auto lg     = std::lock_guard(data_mutex_);
        data_[name] = val;
      }
      return true;
    }

    std::mutex getenv_mutex_;
    std::mutex data_mutex_;
    std::unordered_map<std::string, std::string> data_;
  };

  inline auto GetEnvManager() noexcept -> ThreadSafeEnvManager & {
    static auto env_manager = ThreadSafeEnvManager{};
    return env_manager;
  }

  [[nodiscard]] inline auto Load(const std::string &name) -> std::string {
    auto &env_manager = GetEnvManager();
    return env_manager.GetEnv(name);
  }

} // namespace linter::env
