#!/bin/bash

# This file needs sudo permission.
# Uninstall interpreter.
BINARY_NAME="cpp-lint-action"
LIB_INSTALL_PATH="/usr/local/lib/${BINARY_NAME}/"
BIN_INSTALL_PATH="/usr/local/bin/"

# Refresh libraries
if [[ -d "${LIB_INSTALL_PATH}" ]]; then
  echo "Remove old cpp-lint-action libraries"
  sudo rm -rf ${LIB_INSTALL_PATH}
fi
if [[ -d "${BIN_INSTALL_PATH}" ]]; then
  echo "Remove old cpp-lint-action binaries"
  sudo rm -rf ${BIN_INSTALL_PATH}
fi

echo "Successfully uninstall ${BINARY_NAME}"



