#!/bin/bash

CUR_SCRIPT_PATH=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PROJECT_PATH="${CUR_SCRIPT_PATH}"
BUILD_PATH="${PROJECT_PATH}"/build

echo "Start to package this project"
echo "${PROJECT_PATH}"


# Ubuntu
bash "${PROJECT_PATH}/scripts/ubuntu/package.sh"


