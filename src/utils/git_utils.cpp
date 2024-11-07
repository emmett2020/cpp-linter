#include "git_utils.h"
#include "utils/util.h"
#include <git2/errors.h>
#include <git2/rebase.h>
#include <git2/repository.h>
#include <git2/types.h>

namespace linter::git {
//

int setup() { return git_libgit2_init(); }

int shutdown() { return git_libgit2_shutdown(); }

namespace repo {
int state(GitRepositoryPtr repo) { return git_repository_state(repo); }

} // namespace repo

GitRepositoryPtr open(std::string_view repo_path) {
  auto *repo = GitRepositoryPtr{nullptr};
  auto ec = git_repository_open(&repo, repo_path.data());
  ThrowIf(ec < 0, git_error_last()->message);
  return repo;
}

void free(GitRepositoryPtr repo) { git_repository_free(repo); }

} // namespace linter::git
