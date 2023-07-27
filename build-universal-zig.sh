#!/bin/bash

BUILD_TYPE=Debug
ZIG_PATH=zig-macos-aarch64

PWD=$(pwd)

check_status()
{
    if ! [ "$?" = "0" ]; then
      echo "$1"
      exit 1
    fi
}

build_zig()
{
  cmake \
  -S. \
  -Bcmake-build-zig-$1-${BUILD_TYPE} \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_C_COMPILER="${PWD}/tools/zig-cc" \
  -DCMAKE_CXX_COMPILER="${PWD}/tools/zig-c++" \
  -DCMAKE_C_COMPILER_WORKS=1 \
  -DCMAKE_CXX_COMPILER_WORKS=1 \
  -DZIG_TARGET=$1 \
  -G Ninja

  check_status "Failed at configuring CMake for ${1}."

  cmake --build cmake-build-zig-$1-${BUILD_TYPE} -j 8
  check_status "Failed at building ${1}."
}

build_zig x86_64-macos
build_zig arm64-macos