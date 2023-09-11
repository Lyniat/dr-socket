# CMake file based on https://github.com/Lyniat/dr-cmake-template

function(ensure_python_module module to_install)
    set(ENSURE_PY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/scripts/ensure.py")
    execute_process(COMMAND ${PYTHON_3} "${ENSURE_PY_PATH}" "${module}"
            RESULT_VARIABLE PYTHON_ENSURE_RESULT
            OUTPUT_VARIABLE PYTHON_ENSURE_OUT)

    if(NOT ${PYTHON_ENSURE_RESULT} STREQUAL "0")
        message("${PYTHON_ENSURE_OUT}")
        message("Trying to install '${module}'.")
        execute_process(COMMAND ${PYTHON_3} -m pip install "${to_install}"
                RESULT_VARIABLE PYTHON_INSTALL_RESULT)
        if(NOT ${PYTHON_INSTALL_RESULT} STREQUAL "0")
            message(FATAL_ERROR "Failed to install python module '${module}'! Please install it manually.")
        endif()
    else()
        message("Found '${module}'.")
    endif()
endfunction()

cmake_minimum_required(VERSION 3.22)

set(CMAKE_COMPILE_WARNING_AS_ERROR TRUE) # check for warnings
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wpedantic)

# avoid DOWNLOAD_EXTRACT_TIMESTAMP warning in CMake 3.24
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

set(THIS_PROJECT_NAME "dr-socket")

project(${THIS_PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    message(FATAL_ERROR "MSVC is not supported as compiler.")
endif()

find_program(PYTHON_3 python3 HINTS venv/bin)
if(NOT PYTHON_3)
    find_package (Python3 COMPONENTS Interpreter REQUIRED HINTS venv)
    set(PYTHON_3 Python3::Interpreter)
endif ()

ensure_python_module("yaml" "pyyaml")
ensure_python_module("shutil" "shutil")
ensure_python_module("pathlib" "pathlib")
ensure_python_module("psutil" "psutil")

include(tools/cmake/platform.cmake)

get_platform()
get_property(THIRD_PARTY_TYPE GLOBAL PROPERTY THIRD_PARTY_TYPE)
set(THIRD_PARTY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/third-party")
set(THIRD_PARTY_BIN_DIR "${THIRD_PARTY_BUILD_DIR}/bin")
set(THIRD_PARTY_SRC_DIR "${THIRD_PARTY_BUILD_DIR}/src")
set(THIRD_PARTY_DOWNLOAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/temp/download")
set(THIRD_PARTY_INSTALL_DIR "${THIRD_PARTY_BUILD_DIR}/install")
get_property(META_PLATFORM GLOBAL PROPERTY META_PLATFORM)

if(APPLE_IOS)
    execute_process(COMMAND "sh" "${CMAKE_CURRENT_SOURCE_DIR}/tools/get-ios-info.sh"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE IOS_INFO_RESULT
            OUTPUT_VARIABLE IOS_INFO_OUT)

    if(NOT ${IOS_INFO_RESULT} STREQUAL "0")
        message(FATAL_ERROR ${IOS_INFO_OUT})
    else()
        message("Found iOS app name: ${IOS_INFO_OUT}.")
    endif ()
endif ()

execute_process(COMMAND ${PYTHON_3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/scripts/find-dr.py"
        RESULT_VARIABLE FIND_DR_COMMAND_RESULT
        OUTPUT_VARIABLE FIND_DR_COMMAND_OUT)

if(NOT ${FIND_DR_COMMAND_RESULT} STREQUAL "0")
    message(FATAL_ERROR ${FIND_DR_COMMAND_OUT})
else()
    message("Found the DragonRuby directory at ${FIND_DR_COMMAND_OUT}.")
endif ()

include_directories(
        ${FIND_DR_COMMAND_OUT}
        ${THIRD_PARTY_INSTALL_DIR}/enet/include
        ${THIRD_PARTY_INSTALL_DIR}/memory/include
        ${THIRD_PARTY_INSTALL_DIR}/fmt/include
)

add_library(${THIS_PROJECT_NAME} SHARED
        native-app/api.h
        native-app/api.cpp
        native-app/file.h
        native-app/file.cpp
        native-app/buffer.h
        native-app/buffer.cpp
        native-app/enet.h
        native-app/enet.cpp
        native-app/ext.cpp
        native-app/help.h
        native-app/help.cpp
        native-app/ios.h
        native-app/serialize.h
        native-app/serialize.cpp
        native-app/print.h
        native-app/print.cpp
        native-app/test.h
        native-app/test.cpp
        native-app/socket.rb.h
        )

target_link_directories(${THIS_PROJECT_NAME} PRIVATE
        ${THIRD_PARTY_INSTALL_DIR}/enet/lib
        ${THIRD_PARTY_INSTALL_DIR}/memory/lib
        ${THIRD_PARTY_INSTALL_DIR}/fmt/lib
)

if (WIN32)
target_link_libraries(${THIS_PROJECT_NAME}
        PRIVATE
        fmt
        memory
        enet
        ws2_32
        winmm
        )
else()
target_link_libraries(${THIS_PROJECT_NAME}
        PRIVATE
        fmt
        memory
        enet
        )
endif()

set_target_properties(${THIS_PROJECT_NAME}
        PROPERTIES PREFIX ""
        OUTPUT_NAME "socket"
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-exceptions -fno-rtti")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -g -fno-eliminate-unused-debug-symbols")

if(APPLE_IOS)
    # use -arch arm64 for a real device
    set(IOS_EXTRA_FLAGS
            "-arch x86_64 \
            -install_name @rpath/socket.framework/socket \
            -fembed-bitcode -dead_strip -fobjc-arc -fobjc-link-runtime \
            -mios-simulator-version-min=10.3 -licucore -stdlib=libc++ \
            -framework CFNetwork -framework UIKit -framework Foundation"
            )
    set(IOS_EXTRA_LINKER_FLAGS
            "-rpath @loader_path/Frameworks -rpath -fobjc-arc -fobjc-link-runtime"
            )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IOS_EXTRA_FLAGS} -ObjC++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${IOS_EXTRA_LINKER_FLAGS}")
elseif (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# copy file to directory
if (APPLE_IOS)
    set(DR_TARGET_FILE ../native/ios-device/socket.framework/socket)
    add_custom_command(TARGET ${THIS_PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tools/Info.plist ${CMAKE_CURRENT_SOURCE_DIR}/native/ios-device/Info.plist
            )
    add_custom_command(TARGET ${THIS_PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tools/Info.plist ${CMAKE_CURRENT_SOURCE_DIR}/native/ios-device/socket.framework/Info.plist
            )
elseif (APPLE)
    set(DR_TARGET_FILE ../native/macos/socket.dylib)
elseif (LINUX)
    set(DR_TARGET_FILE ../native/linux-amd64/socket.so)
elseif (WIN32)
    set(DR_TARGET_FILE ../native/windows-amd64/socket.dll)
elseif(ANDROID)
    set(DR_TARGET_FILE ../native/${ANDROID_TYPE}/socket.so)
endif()

add_custom_command(TARGET ${THIS_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${THIS_PROJECT_NAME}> ${DR_TARGET_FILE}
        )

set (EXTERNAL_EXTRA_FLAGS "")
if(ZIG_TARGET)
    set(CMAKE_CXX_FLAGS "-target ${ZIG_TARGET} ${CMAKE_CXX_FLAGS}")

    set (EXTERNAL_EXTRA_FLAGS
            -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    )
elseif (ANDROID)
    set (EXTERNAL_EXTRA_FLAGS
            -DCMAKE_C_COMPILER_WORKS=${CMAKE_C_COMPILER_WORKS} -DCMAKE_CXX_COMPILER_WORKS=${CMAKE_CXX_COMPILER_WORKS}
            -DANDROID_ABI=${ANDROID_ABI} \\
            -DANDROID_PLATFORM=${ANDROID_PLATFORM} \\
            -DANDROID_NDK=${ANDROID_NDK} \\
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            )
elseif(APPLE_IOS)
    set (EXTERNAL_EXTRA_FLAGS
        -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
        -DCMAKE_CXX_FLAGS=${IOS_EXTRA_FLAGS}
            )
elseif(APPLE)
    set(EXTERNAL_EXTRA_FLAGS "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}")
endif()

#message(FATAL_ERROR ${CMAKE_C_COMPILER})

include(ExternalProject)
ExternalProject_Add(
        enet
        URL https://github.com/lsalzman/enet/archive/refs/tags/v1.3.17.tar.gz
        URL_HASH MD5=1c3270f1e03ccb2a8544a88d91d2a368
        DOWNLOAD_DIR ${THIRD_PARTY_DOWNLOAD_DIR}/enet
        BINARY_DIR ${THIRD_PARTY_BIN_DIR}/enet
        SOURCE_DIR ${THIRD_PARTY_SRC_DIR}/enet
        INSTALL_COMMAND ${PYTHON_3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/scripts/install.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/res/enet/"
        "${THIRD_PARTY_BUILD_DIR}"
        "${THIRD_PARTY_INSTALL_DIR}/enet"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        ${EXTERNAL_EXTRA_FLAGS}
)

ExternalProject_Add(
        memory
        BINARY_DIR ${THIRD_PARTY_BIN_DIR}/memory
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/memory
        BUILD_ALWAYS 1
        CMAKE_COMMAND ${THIRD_PARTY_CMAKE_COMMAND}
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}/memory
        ${EXTERNAL_EXTRA_FLAGS}
)

# https://github.com/fmtlib/fmt
ExternalProject_Add(
        fmt
        URL https://github.com/fmtlib/fmt/archive/refs/tags/10.0.0.tar.gz
        URL_HASH SHA256=ede1b6b42188163a3f2e0f25ad5c0637eca564bd8df74d02e31a311dd6b37ad8
        DOWNLOAD_DIR ${THIRD_PARTY_DOWNLOAD_DIR}/fmt
        BINARY_DIR ${THIRD_PARTY_BIN_DIR}/fmt
        SOURCE_DIR ${THIRD_PARTY_SRC_DIR}/fmt
        CMAKE_COMMAND ${THIRD_PARTY_CMAKE_COMMAND}
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}/fmt
        ${EXTERNAL_EXTRA_FLAGS}
)

# build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DMETA_TYPE="Release")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DMETA_TYPE="Debug")
    add_definitions(-DDEBUG)
else()
    add_definitions(-DMETA_TYPE="Other")
endif()

# add some helpful information to library
# get the latest commit hash of the working branch git branch
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE META_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get git branch name
execute_process(
        COMMAND git branch --show-current
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE META_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions(-DMETA_PLATFORM=${META_PLATFORM})
add_definitions(-DMETA_GIT_BRANCH="${META_GIT_BRANCH}")
add_definitions(-DMETA_GIT_HASH="${META_GIT_HASH}")
add_definitions(-DMETA_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
add_definitions(-DMETA_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
