# cpp-linter
Platform: Linux

# build
```
apt install libgit2-dev


# How to package
./package.sh

# How to use package
```
tar -xvf linter-x.x.x.tar.gz
cd linter-xxx
export LD_LIBRARY_PATH=/path/to/linter-xxx/lib:$LD_LIBRARY_PATH
```shell

# TODO:
1. support clang-format
2. support windows
3. support macos
4. support cpplint
5. refine code:
  5.1 make some classes abstract
  5.2 refine log
6. add 100% code coverage
7. document
8. release 1.0.0

```
