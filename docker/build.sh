#!/bin/bash

source ~/.bashrc

usage() {
  echo "Usage: ${0} [-a|--asan]" 1>&2
  exit 1
}

ASAN_OPTION=OFF
while [[ $# -gt 0 ]];do
  key=${1}
  case ${key} in
    -d|--asan)
      ASAN_OPTION=ON
      shift 1
      ;;
    *)
      usage
      shift
      ;;
  esac
done

CUR_DIR=${PWD}

set -exuf -o pipefail

BUILD_DIR=${CUR_DIR}/build

if [ -d ${BUILD_DIR} ];
then
    rm -rf ${BUILD_DIR}
fi

service docker start || true

mkdir ${BUILD_DIR}
BUILD_TYPE=Release
pushd ${BUILD_DIR}
# Configure
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_BUILD_ASAN=${ASAN_OPTION}

# Build
cmake --build .

# Test
GTEST_COLOR=TRUE ctest -V --output-on-failure

# Install
cmake --build . --target install

# Package
cmake --build . --target package

# Docker
cmake --build . --target docker

popd
