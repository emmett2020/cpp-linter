#!/bin/bash

# TODO: add this example to emmett
CUR_SCRIPT_PATH=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PROJECT_PATH="${CUR_SCRIPT_PATH}/../.."
BUILD_PATH="${PROJECT_PATH}"/build

lddtree "${BUILD_PATH}"/linter --copy-to-tree "${BUILD_PATH}"/dist --libdir /lib --bindir /bin
# patchelf --set-interpreter ../lib/ld-linux-aarch64.so.1 ${BUILD_PATH}/dist/bin/linter
patchelf --set-interpreter ../lib/ld-linux-x86-64.so.2 ${BUILD_PATH}/dist/bin/linter

cd "${BUILD_PATH}"
tar -cvf dist.tar.gz dist
