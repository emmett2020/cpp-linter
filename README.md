# CppLintAction
Platform: Linux

# build
```
apt install libgit2-dev


# How to package
./package.sh

# How to use package
```
tar -xvf cpp-lint-action-x.x.x.tar.gz
cd cpp-lint-action
export LD_LIBRARY_PATH=/path/to/cpp-lint-action-xxx/lib:$LD_LIBRARY_PATH
```shell

# TODO:
1. support clang-format [done]
2. support windows [hold]
3. support macos [hold]
4. refine code:
  4.1 make some classes abstract [done]
  4.2 refine log
  4.3 copyright [done]
  4.4 print usage rather exception when meets invalid program options [discard]
  4.5 use reference rather than raw pointer in git2 [done]
  4.6 finish clang-format tests
  4.7 finish clang-tidy tests
  4.8 finish all tests
  4.9 finish all TODOs
  4.10 Github action
  4.11 Real experience
5. add 100% code coverage
6. document
7. support cpplint
8. release 1.0.0

```
