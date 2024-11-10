#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_map>

namespace linter::env {

  class thread_safe_env_manager {
  public:
    static thread_safe_env_manager &get_instance() noexcept;

    auto get(const std::string &name) -> std::string;
    void set_cache(const std::string &name, const std::string &value);
    void set_cache(std::unordered_map<std::string, std::string> data);

    thread_safe_env_manager(const thread_safe_env_manager &)            = delete;
    thread_safe_env_manager &operator=(const thread_safe_env_manager &) = delete;
  private:
    thread_safe_env_manager()  = default;
    ~thread_safe_env_manager() = default;

    std::mutex mutex_;
    std::unordered_map<std::string, std::string> cache_;
  };

  /// @brief: Wrapper of ThreadSafeEnvManager
  [[nodiscard]] auto get(const std::string &name) -> std::string;
  void set_cache(const std::string &name, const std::string &value);
  void set_cache(std::unordered_map<std::string, std::string> data);


} // namespace linter::env
