#!/bin/bash

CUR_SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)

# This file needs sudo permission.
# Install interpreter into desired directory.
BINARY_NAME="cpp-linter"
LIB_INSTALL_PATH="/usr/local/lib/${BINARY_NAME}/"
BIN_INSTALL_PATH="/usr/local/bin/"

# Refresh libraries
if [[ -d "${LIB_INSTALL_PATH}" ]]; then
  echo "Remove old cpp-linter libraries"
  sudo rm -rf ${LIB_INSTALL_PATH}
fi
if [[ -d "${BIN_INSTALL_PATH}" ]]; then
  echo "Remove old cpp-linter binaries"
  sudo rm -rf ${BIN_INSTALL_PATH}
fi
sudo mkdir -p ${LIB_INSTALL_PATH}
sudo mkdir -p ${BIN_INSTALL_PATH}

sudo mv ${CUR_SCRIPT_DIR}/lib/* ${LIB_INSTALL_PATH}
sudo mv ${CUR_SCRIPT_DIR}/bin/* ${BIN_INSTALL_PATH}

echo "Successfully install ${BINARY_NAME}"
echo "Binaries install path: ${BIN_INSTALL_PATH}"
echo "Libraries install path: ${LIB_INSTALL_PATH}."
echo "You can safely remove this package now."



