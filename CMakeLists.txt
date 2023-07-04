cmake_minimum_required(VERSION 3.22)

project(dr-socket LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

if(WIN32)
    message("Found Windows as OS.")
    set(META_PLATFORM \"Windows_amd64\")
elseif(APPLE)
    if (CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
        message("Found macOS (Intel) as target OS.")
        set(META_PLATFORM \"macOS_amd64\")
    else()
        message("Found macOS (Apple Silicon) as target OS.")
        set(META_PLATFORM \"macOS_arm64\")
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("Found Linux as target OS.")
    set(META_PLATFORM \"Linux_amd64\")
    set(LINUX TRUE)
else()
    message(FATAL_ERROR "Unsupported platform.")
endif()

include_directories(
        ../include
        third-party/enet/include
)

set(ENGINE_SRC
        native-app/ext.cpp
        native-app/api.h
        native-app/api.cpp
        native-app/socket.cpp
        native-app/socket.h
        native-app/help.cpp
        native-app/help.h
        native-app/serialize.cpp
        native-app/serialize.h
)

add_library(dr-socket SHARED ${ENGINE_SRC})

target_link_directories(dr-socket PRIVATE third-party/bin)

if (WIN32)
target_link_libraries(dr-socket
        PRIVATE
        enet
        ws2_32
        winmm
        )
else()
target_link_libraries(dr-socket
        PRIVATE
        enet
        )
endif()

set_target_properties(dr-socket
        PROPERTIES PREFIX ""
        OUTPUT_NAME "socket"
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# copy file to directory
if (APPLE)
    add_custom_command(TARGET dr-socket
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dr-socket> ../native/macos/socket.dylib)
elseif (LINUX)
    add_custom_command(TARGET dr-socket
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dr-socket> ../native/linux-amd64/socket.so)
elseif (WIN32)
    add_custom_command(TARGET dr-socket
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dr-socket> ../native/windows-amd64/socket.dll)
endif()

include(ExternalProject)
ExternalProject_Add(
        enet
        URL http://enet.bespin.org/download/enet-1.3.17.tar.gz
        URL_HASH MD5=c8e96051c5a60afa9f3052d02b2cbe19
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/bin
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/enet
        INSTALL_COMMAND ""
        CMAKE_ARGS DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
)

# add some helpful information to library
# get the latest commit hash of the working branch
execute_process(
        COMMAND git log -1 --format=%h
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