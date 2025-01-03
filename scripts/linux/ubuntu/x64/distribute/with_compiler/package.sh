#!/bin/bash

###
### Introduction of this script:
### This script supports compiling ${BINARY_NAME} and running ${BINARY_NAME}
### using different compiler versions. To achieve this goal, we need to use lddtree
### to collect compiled phase shared libraries and use patchelf to set runtime
### loader(interpreter) for distributed binary. So that the binary could use the
### loader in compiled phase.
### Dependencies: lddtree, patchelf
### The distribution will be placed into project-path/build directory.
### This script only supports one executable binary names ${BINARY_NAME}.
### The name of packaged product is ${DISTRIBUTION_NAME}.tar.gz
###

CUR_SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)

PROJECT_ROOT_PATH="${CUR_SCRIPT_DIR}/../../.."
echo "PROJECT_ROOT_PATH: ${PROJECT_ROOT_PATH}"

PROJECT_BUILD_PATH="${PROJECT_ROOT_PATH}"/build
pushd "${PROJECT_BUILD_PATH}" &> /dev/null

BINARY_NAME="cpp-lint-action"
DISTRIBUTION_NAME="cpp-lint-action-dist"
INTERPRETER_INSTALL_PATH="/usr/local/lib/${BINARY_NAME}"

echo "Start to package ${BINARY_NAME}"

echo "1. Start to collect dependent shared libraries for ${BINARY_NAME}"
lddtree "${BINARY_NAME}"  \
        --copy-to-tree "${DISTRIBUTION_NAME}" \
        --libdir "/lib/${BINARY_NAME}" \
        --bindir /bin

echo "2. Start to set new interpreter path: ${INTERPRETER_INSTALL_PATH}/${interpreter}"
interpreter=$(ls ${DISTRIBUTION_NAME}/lib/${BINARY_NAME} | grep "ld-linux")
patchelf --set-interpreter "${INTERPRETER_INSTALL_PATH}/${interpreter}" \
                           ${DISTRIBUTION_NAME}/bin/${BINARY_NAME}

echo "3. Start to set rpath for: ${BINARY_NAME}"
# ORIGIN shouldn't be translated while BINARY_NAME should be translated.
patchelf --force-rpath --set-rpath '$ORIGIN/../lib/'${BINARY_NAME} "${DISTRIBUTION_NAME}/bin/${BINARY_NAME}"


echo "4. Start to compress"
# -- ${DISTRIBUTION_NAME}
# ------- install.sh
# ------- uninstall.sh
# ------- bin/
# ------- lib/
cp ${CUR_SCRIPT_DIR}/install.sh ${DISTRIBUTION_NAME}
cp ${CUR_SCRIPT_DIR}/uninstall.sh ${DISTRIBUTION_NAME}
tar -cvf ${DISTRIBUTION_NAME}.tar.gz ${DISTRIBUTION_NAME}
popd &> /dev/null

echo "Successfully packaged ${BINARY_NAME} and it's dependencies into ${DISTRIBUTION_NAME}"
