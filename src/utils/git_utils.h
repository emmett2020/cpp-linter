#pragma once

#include <git2.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <memory>

namespace linter::git {

using GitRepositoryPtr = git_repository *;
// using GitRepositoryPtr = std::unique_ptr<git_repository>;

int setup();
int shutdown();
GitRepositoryPtr open(std::string_view repo_path);
void free(GitRepositoryPtr repo);

namespace repo {
/// Determines the status of a git repository
/// i.e. whether an operation (merge, cherry-pick, etc) is in progress.
/// https://libgit2.org/libgit2/#HEAD/group/repository/git_repository_state
int state(GitRepositoryPtr repo);

} // namespace repo

bool Init(GitRepositoryPtr repo);
bool Close(GitRepositoryPtr repo);
bool Clone(GitRepositoryPtr repo);
bool Add(GitRepositoryPtr repo);
bool Move(GitRepositoryPtr repo);
bool Restore(GitRepositoryPtr repo);
bool Remove(GitRepositoryPtr repo);
bool Diff(GitRepositoryPtr repo);
bool Log(GitRepositoryPtr repo);
bool Status(GitRepositoryPtr repo);
bool Show(GitRepositoryPtr repo);
bool Branch(GitRepositoryPtr repo);
bool Commit(GitRepositoryPtr repo);
bool Merge(GitRepositoryPtr repo);
bool Rebase(GitRepositoryPtr repo);
bool Reset(GitRepositoryPtr repo);
bool Switch(GitRepositoryPtr repo);
bool Tag(GitRepositoryPtr repo);
bool Fetch(GitRepositoryPtr repo);
bool Pull(GitRepositoryPtr repo);
bool Push(GitRepositoryPtr repo);

} // namespace linter::git
