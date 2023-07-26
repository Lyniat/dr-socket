# CMake file based on https://github.com/Lyniat/dr-cmake-template

cmake_minimum_required(VERSION 3.22)

# avoid DOWNLOAD_EXTRACT_TIMESTAMP warning in CMake 3.24
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

set(THIS_PROJECT_NAME "dr-socket")

project(${THIS_PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
    message(FATAL_ERROR "MSVC is not supported as compiler.")
endif()

if(EMSCRIPTEN)
    message(FATAL_ERROR "Emscripten is currently not supported and will probably never be. Sorry :(")
elseif(RASPBERRY_PI)
    message(FATAL_ERROR "Raspberry Pi is currently not supported.")
    #message("User set Raspberry Pi as target OS.")
    set(THIRD_PARTY_BIN_DIR linux-raspberrypi-${CMAKE_BUILD_TYPE})
    set(META_PLATFORM \"linux-raspberrypi\")
elseif(WIN32)
    message("Found Windows as OS.")
    set(THIRD_PARTY_BIN_DIR windows-amd64-${CMAKE_BUILD_TYPE})
    set(META_PLATFORM \"Windows_amd64\")
elseif(APPLE)
    if(APPLE_IOS)
        message("User set iOS as target OS.")
        set(THIRD_PARTY_BIN_DIR ios-device-${CMAKE_BUILD_TYPE})
        set(META_PLATFORM \"iOS_arm64\")
        #set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk")
        set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk")
    elseif (CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
        message("Found macOS (Intel) as target OS.")
        set(THIRD_PARTY_BIN_DIR macos-x86_64-${CMAKE_BUILD_TYPE})
        set(META_PLATFORM \"macOS_amd64\")
    else()
        message("Found macOS (Apple Silicon) as target OS.")
        set(THIRD_PARTY_BIN_DIR macos-arm64-${CMAKE_BUILD_TYPE})
        set(META_PLATFORM \"macOS_arm64\")
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("Found Linux as target OS.")
    set(THIRD_PARTY_BIN_DIR linux-amd64-${CMAKE_BUILD_TYPE})
    set(META_PLATFORM \"Linux_amd64\")
    set(LINUX TRUE)
elseif(ANDROID)
    set(THIRD_PARTY_BIN_DIR android-${ANDROID_ABI}-${CMAKE_BUILD_TYPE})
    if(${ANDROID_ABI} MATCHES "arm64-v8a")
        set(ANDROID_TYPE android-arm64)
    elseif(${ANDROID_ABI} MATCHES "armeabi-v7a")
        set(ANDROID_TYPE android-arm32)
    elseif(${ANDROID_ABI} MATCHES "x86_64")
        set(ANDROID_TYPE android-amd64)
    elseif(${ANDROID_ABI} MATCHES "x86")
        set(ANDROID_TYPE android-x86)
    else()
        message(FATAL_ERROR "Unsupported Android ABI ${ANDROID_ABI}.")
    endif()
    message("User set Android ${ANDROID_TYPE} as target OS.")
    set(META_PLATFORM \"Android-${ANDROID_TYPE}\")
    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)
else()
    message(FATAL_ERROR "Unsupported platform.")
endif()

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

# helping functions
if(${CMAKE_HOST_SYSTEM} MATCHES "Windows*") # "WIN32" not working here if cross compiling
    # find DR parent directory on Windows
    set(FIND_DR_COMMAND "cmd" "/C" "${CMAKE_CURRENT_SOURCE_DIR}/tools/find-dr.bat")
else()
    # find DR parent directory on Unix
    set(FIND_DR_COMMAND "sh" "${CMAKE_CURRENT_SOURCE_DIR}/tools/find-dr.sh")
endif()

message(${FIND_DR_COMMAND})

execute_process(COMMAND ${FIND_DR_COMMAND}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE FIND_DR_COMMAND_RESULT
        OUTPUT_VARIABLE FIND_DR_COMMAND_OUT)

# remove new line
string(REPLACE "\n" "" FIND_DR_COMMAND_OUT ${FIND_DR_COMMAND_OUT})
# only for windows
string(REPLACE "\\" "/" FIND_DR_COMMAND_OUT ${FIND_DR_COMMAND_OUT})

if(NOT ${FIND_DR_COMMAND_RESULT} STREQUAL "0")
    message(FATAL_ERROR ${FIND_DR_COMMAND_OUT})
else()
    message("Found the DragonRuby directory at ${FIND_DR_COMMAND_OUT}.")
endif ()

include_directories(
        ${FIND_DR_COMMAND_OUT}/include
        third-party/source/enet/include
)

file(GLOB_RECURSE LIB_SRC "native-app/*.c" "native-app/*.cpp" "native-app/*.h" "native-app/*.hpp")

add_library(${THIS_PROJECT_NAME} SHARED ${LIB_SRC})

target_link_directories(${THIS_PROJECT_NAME} PRIVATE third-party/bin/${THIRD_PARTY_BIN_DIR}/enet)

if (WIN32)
target_link_libraries(${THIS_PROJECT_NAME}
        PRIVATE
        enet
        ws2_32
        winmm
        )
else()
target_link_libraries(${THIS_PROJECT_NAME}
        PRIVATE
        enet
        )
endif()

set_target_properties(${THIS_PROJECT_NAME}
        PROPERTIES PREFIX ""
        OUTPUT_NAME "socket"
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IOS_EXTRA_FLAGS} -ObjC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${IOS_EXTRA_LINKER_FLAGS}")
elseif (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
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
if (ANDROID)
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
        -DCMAKE_C_FLAGS=${IOS_EXTRA_FLAGS}
            )
elseif(APPLE)
    set(EXTERNAL_EXTRA_FLAGS "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}")
endif()

include(ExternalProject)
ExternalProject_Add(
        enet
        URL https://github.com/lsalzman/enet/archive/refs/tags/v1.3.17.tar.gz
        URL_HASH MD5=1c3270f1e03ccb2a8544a88d91d2a368
        DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/download/enet
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/bin/${THIRD_PARTY_BIN_DIR}/enet
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/source/enet
        INSTALL_COMMAND ""
        CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        ${EXTERNAL_EXTRA_FLAGS}

)

# add some helpful information to library
# get the latest commit hash of the working branch
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE META_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# hash
add_definitions(-DMETA_GIT_HASH="git_hash:${META_GIT_HASH}")

# platform
add_definitions(-DMETA_PLATFORM=${META_PLATFORM})

# build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DMETA_TYPE="type:release")
else()
    add_definitions(-DMETA_TYPE="type:debug")
endif()

# time stamp
string(TIMESTAMP META_TIMESTAMP)
add_definitions(-DMETA_TIMESTAMP="time:${META_TIMESTAMP}")

# compiler
add_definitions(-DMETA_COMPILER="compiler:${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
