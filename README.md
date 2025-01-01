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
3. support macos
4. refine code:
  5.1 make some classes abstract [done]
  5.2 refine log
  5.3 copyright [done]
  5.4 print usage rather exception when meets invalid program options [discard]
  5.5 use reference rather than raw pointer in git2
5. add 100% code coverage
6. document
7. support cpplint
8. release 1.0.0

```
