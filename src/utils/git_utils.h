#pragma once

#include <git2.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <memory>

namespace linter::git {

using GitRepositoryPtr = git_repository *;
// using GitRepositoryPtr = std::unique_ptr<git_repository>;

int Setup();
int Shutdown();
GitRepositoryPtr Open(std::string_view repo_path);
void Free(GitRepositoryPtr repo);

bool init(GitRepositoryPtr repo);
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
