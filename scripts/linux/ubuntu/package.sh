#!/bin/bash

# Needs: lddtree, patchelf
# The distributions will be placed into project/build directory.

CUR_SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PROJECT_ROOT_PATH="${CUR_SCRIPT_DIR}/../../.."
PROJECT_BUILD_PATH="${PROJECT_ROOT_PATH}"/build

echo "PROJECT_ROOT_PATH: ${PROJECT_ROOT_PATH}"
pushd "${PROJECT_BUILD_PATH}" &> /dev/null

BINARY_NAME="cpp-linter"
DISTRIBUTION_NAME="cpp-linter-dist"
INTERPRETER_INSTALL_PATH="/usr/local/lib/${BINARY_NAME}"

echo "Start to package ${BINARY_NAME}"
echo "1. Start to collect dependent shared libraries ${BINARY_NAME}"
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
patchelf --set-rpath --force-rpath '$ORIGIN/../lib/'${BINARY_NAME} "${DISTRIBUTION_NAME}/bin/${BINARY_NAME}"


echo "4. Start to compress"
# -- cpp-linter-dist/
# ------- install.sh
# ------- uninstall.sh
# ------- bin/
# ------- lib/
cp ${CUR_SCRIPT_DIR}/install.sh ${DISTRIBUTION_NAME}
cp ${CUR_SCRIPT_DIR}/uninstall.sh ${DISTRIBUTION_NAME}
tar -cvf ${DISTRIBUTION_NAME}.tar.gz ${DISTRIBUTION_NAME}
popd &> /dev/null
echo "Successfully packaged ${BINARY_NAME} and it's dependencies into ${DISTRIBUTION_NAME}"
