#!/bin/bash

BUILD_TYPE="Debug"

if [ $(uname) = "Darwin" ]; then
  OS_TYPE="macos"
else
  OS_TYPE="linux"
fi

cmake -S. -Bcmake-build-${OS_TYPE}-${BUILD_TYPE} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
cmake --build cmake-build-${OS_TYPE}-${BUILD_TYPE} -j 8