#!/bin/bash

BUILD_TYPE="Debug"
FAT=false

if [ $1 = "--release" ]; then
  BUILD_TYPE="Release"
fi

build_macos()
{
  ARCH=$1
  cmake -S. -Bcmake-build-${OS_TYPE}-${ARCH}-${BUILD_TYPE} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_OSX_ARCHITECTURES=${ARCH}
  cmake --build cmake-build-${OS_TYPE}-${ARCH}-${BUILD_TYPE} -j 8
}

build_linux()
{
  cmake -S. -Bcmake-build-${OS_TYPE}-${BUILD_TYPE} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  cmake --build cmake-build-${OS_TYPE}-${BUILD_TYPE} -j 8
}

build_android arm64-v8a

if [ $(uname) = "Darwin" ]; then
  OS_TYPE="macos"

  build_macos arm64
  build_macos x86_64

  # create fat binary
  mkdir -p cmake-build-${OS_TYPE}-fat-${BUILD_TYPE}
  lipo -create -output cmake-build-${OS_TYPE}-fat-${BUILD_TYPE}/socket.dylib cmake-build-${OS_TYPE}-x86_64-${BUILD_TYPE}/socket.dylib cmake-build-${OS_TYPE}-arm64-${BUILD_TYPE}/socket.dylib
  cp cmake-build-${OS_TYPE}-fat-${BUILD_TYPE}/socket.dylib native/macos/socket.dylib
else
  OS_TYPE="linux"
  build_linux
fi
