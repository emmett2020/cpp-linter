#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_map>

namespace linter::env {

  class ThreadSafeEnvManager {
  public:
    auto Get(const std::string &name) -> std::string;
    void SetCache(const std::string &name, const std::string &value);
    void SetCache(std::unordered_map<std::string, std::string> data);

  private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::string> cache_;
  };

  /// @brief: Wrapper of ThreadSafeEnvManager
  auto GetEnvManager() noexcept -> ThreadSafeEnvManager &;
  [[nodiscard]] auto Get(const std::string &name) -> std::string;
  void SetCache(const std::string &name, const std::string &value);
  void SetCache(std::unordered_map<std::string, std::string> data);


} // namespace linter::env
