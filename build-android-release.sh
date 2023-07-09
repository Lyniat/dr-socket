#!/bin/bash

NDK_PATH=~/Library/Android/sdk/ndk/25.2.9519653
ANDROID_PLATFORM=android-19

BUILD_TYPE=Release

if [! -d "$NDK_PATH" ]; then
  echo "NDK_PATH ${NDK_PATH} does not exist. Please change your shell build script."
  exit 1
fi

check_status()
{
    if ! [ "$?" = "0" ]; then
      echo "$1"
      exit 1
    fi
}

build_android()
{
  cmake \
  -S. \
  -Bcmake-build-android-$1-${BUILD_TYPE} \
  -DANDROID_ABI=$1 \
  -DANDROID_PLATFORM=$ANDROID_PLATFORM \
  -DANDROID_NDK=$NDK_PATH/ \
  -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -G Ninja

  check_status "Failed at configuring CMake for ${1}."

  cmake --build cmake-build-android-$1-${BUILD_TYPE} -j 8
  check_status "Failed at building ${1}."
}

build_android arm64-v8a
build_android armeabi-v7a
build_android x86_64
build_android x86