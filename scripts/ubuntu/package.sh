#!/bin/bash

# TODO: add this example to emmett
CUR_SCRIPT_PATH=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PROJECT_PATH="${CUR_SCRIPT_PATH}/../.."
BUILD_PATH="${PROJECT_PATH}"/build

lddtree "${BUILD_PATH}"/main --copy-to-tree "${BUILD_PATH}"/dist --libdir /lib --bindir /bin
cd "${BUILD_PATH}"
tar -cvf linter.tar.gz linter
