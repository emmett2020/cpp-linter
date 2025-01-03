#!/bin/bash
: << 'COMMENT'
|------------------------------|------------------------------|
|         🎃 item              |        👇 explanation        |
|------------------------------|------------------------------|
|    needs root permission?    |              No              |
|------------------------------|------------------------------|
|          dependencies        |           lddtree            |
|                              |           patchelf           |
|------------------------------|------------------------------|
|          fellows             |         install.sh           |
|                              |         uninstall.sh         |
|------------------------------|------------------------------|

Introduction of this script:
This script supports compiling binary and running binary using different compiler versions.
That's to say, the version of compiler during compiling could greater than the
version of compiler during running.
COMMENT

# Exit on error, treat unset variables as an error, and fail on pipeline errors
set -euo pipefail

CUR_SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
SCRIPTS_DIR="${CUR_SCRIPT_DIR}/../../../../.."
X64_DIR="${SCRIPTS_DIR}/linux/ubuntu/x64"

PROJECT_ROOT_PATH="${SCRIPTS_DIR}/.."
echo "PROJECT_ROOT_PATH: ${PROJECT_ROOT_PATH}"

PROJECT_BUILD_PATH="${PROJECT_ROOT_PATH}"/build
pushd "${PROJECT_BUILD_PATH}" &> /dev/null

BINARY_NAME="cpp-lint-action"
DISTRIBUTION_NAME="cpp-lint-action-dist"
INTERPRETER_INSTALL_PATH="/usr/local/lib/${BINARY_NAME}"

echo "Start to package ${BINARY_NAME}, distribution product name: ${DISTRIBUTION_NAME}"
[[ -d "${DISTRIBUTION_NAME}" ]] && rm -rf "${DISTRIBUTION_NAME}"

echo "1. Start to collect all dependent shared libraries for ${BINARY_NAME}"
lddtree "${BINARY_NAME}"                        \
        --copy-to-tree  "${DISTRIBUTION_NAME}"  \
        --libdir        "/lib/${BINARY_NAME}"   \
        --bindir        "/bin"

interpreter=$(ls ${DISTRIBUTION_NAME}/lib/${BINARY_NAME} | grep "ld-linux")
echo "2. Start to set new interpreter path: ${INTERPRETER_INSTALL_PATH}/${interpreter}"
patchelf --set-interpreter                            \
         "${INTERPRETER_INSTALL_PATH}/${interpreter}" \
         ${DISTRIBUTION_NAME}/bin/${BINARY_NAME}

echo "3. Start to set rpath for: ${BINARY_NAME}"
# ORIGIN shouldn't be translated while BINARY_NAME should be translated.
patchelf --force-rpath                     \
         --set-rpath                       \
         '$ORIGIN/../lib/'${BINARY_NAME}   \
         "${DISTRIBUTION_NAME}/bin/${BINARY_NAME}"


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
