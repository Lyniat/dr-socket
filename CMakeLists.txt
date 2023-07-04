cmake_minimum_required(VERSION 3.24)

project(dr-socket LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

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
endif()

if (WIN32)
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