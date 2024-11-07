#include "git_utils.h"
#include "utils/util.h"
#include <git2/errors.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/types.h>

namespace linter::git {
//

int Setup() { return git_libgit2_init(); }

int Shutdown() { return git_libgit2_shutdown(); }

GitRepositoryPtr Open(std::string_view repo_path) {
  auto *repo = GitRepositoryPtr{nullptr};
  auto ec = git_repository_open(&repo, repo_path.data());
  ThrowIf(ec < 0, git_error_last()->message);
  return repo;
}

void Free(GitRepositoryPtr repo) { git_repository_free(repo); }

} // namespace linter::git
