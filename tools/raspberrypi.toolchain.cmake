# this file was taken from https://web.archive.org/web/20210625121455/https://dyatkovskiy.com/2018/11/04/173/
SET(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Custom toolchain-specific definitions for your project
set(PLATFORM_ARM "1")
set(PLATFORM_COMPILE_DEFS "COMPILE_GLES")

# There we go!
# Below, we specify toolchain itself!

SET(TARGET_TRIPLE arm-linux-gnueabihf)

# Specify your target rootfs mount point on your compiler host machine
SET(TARGET_ROOTFS /Volumes/rootfs-${TARGET_TRIPLE})

# Specify clang paths
SET(LLVM_DIR /Users/stepan/projects/shared/toolchains/llvm-7.0.darwin-release-x86_64/install)
SET(CLANG ${LLVM_DIR}/bin/clang)
SET(CLANGXX ${LLVM_DIR}/bin/clang++)

# Specify compiler (which is clang)
SET(CMAKE_C_COMPILER   ${CLANG})
SET(CMAKE_CXX_COMPILER ${CLANGXX})

# Specify binutils

SET (CMAKE_AR      "${LLVM_DIR}/bin/llvm-ar" CACHE FILEPATH "Archiver")
SET (CMAKE_LINKER  "${LLVM_DIR}/bin/llvm-ld" CACHE FILEPATH "Linker")
SET (CMAKE_NM      "${LLVM_DIR}/bin/llvm-nm" CACHE FILEPATH "NM")
SET (CMAKE_OBJDUMP "${LLVM_DIR}/bin/llvm-objdump" CACHE FILEPATH "Objdump")
SET (CMAKE_RANLIB  "${LLVM_DIR}/bin/llvm-ranlib" CACHE FILEPATH "ranlib")

# You may use legacy binutils though.
#SET(BINUTILS /usr/local/Cellar/arm-linux-gnueabihf-binutils/2.31.1)
#SET (CMAKE_AR      "${BINUTILS}/bin/${TARGET_TRIPLE}-ar" CACHE FILEPATH "Archiver")
#SET (CMAKE_LINKER  "${BINUTILS}/bin/${TARGET_TRIPLE}-ld" CACHE FILEPATH "Linker")
#SET (CMAKE_NM      "${BINUTILS}/bin/${TARGET_TRIPLE}-nm" CACHE FILEPATH "NM")
#SET (CMAKE_OBJDUMP "${BINUTILS}/bin/${TARGET_TRIPLE}-objdump" CACHE FILEPATH "Objdump")
#SET (CMAKE_RANLIB  "${BINUTILS}/bin/${TARGET_TRIPLE}-ranlib" CACHE FILEPATH "ranlib")

# Specify sysroot (almost same as rootfs)
SET(CMAKE_SYSROOT ${TARGET_ROOTFS})
SET(CMAKE_FIND_ROOT_PATH ${TARGET_ROOTFS})

# Specify lookup methods for cmake
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Sometimes you also need this:
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Specify raspberry triple
set(CROSS_FLAGS "--target=${TARGET_TRIPLE}")

# Specify other raspberry related flags
set(RASP_FLAGS "-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")

# Gather and distribute flags specified at prev steps.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CROSS_FLAGS} ${RASP_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CROSS_FLAGS} ${RASP_FLAGS}")

# Use clang linker. Why?
# Well, you may install custom arm-linux-gnueabihf binutils,
# but then, you also need to recompile clang, with customized triple;
# otherwise clang will try to use host 'ld' for linking,
# so... use clang linker.
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld)