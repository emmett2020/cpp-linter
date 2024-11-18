#!/bin/bash

CUR_SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PROJECT_ROOT_PATH="${CUR_SCRIPT_DIR}/../../.."
PROJECT_BUILD_PATH="${PROJECT_PATH}"/build

lddtree "${PROJECT_BUILD_PATH}"/cpp-linter --copy-to-tree "${PROJECT_BUILD_PATH}"/cpp-linter-dist --libdir /lib/cpp-linter --bindir /bin
# patchelf --set-interpreter dist/lib/ld-linux-x86-64.so.2 ${PROJECT_BUILD_PATH}/dist/bin/linter

#
# pushd "${BUILD_PATH}"
# tar -cvf dist.tar.gz dist
# popd
