#include "api.h"

#include <git2.h>
#include <git2/repository.h>
#include <git2/types.h>

#include <httplib.h>
#include <vector>

namespace linter {

// Local detect

std::vector<std::string> ParseDiff() {
  git_repository *g = nullptr;
  std::cout << git_repository_open(&g, "/tmp");
  return {};
}

} // namespace linter
