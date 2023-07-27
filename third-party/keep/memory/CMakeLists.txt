cmake_minimum_required(VERSION 3.22)

set(THIS_PROJECT_NAME "memory")

project(${THIS_PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(SRC
        include/lyniat/memory.h
        src/memory.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-exceptions -fno-rtti")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -g -fno-eliminate-unused-debug-symbols")

add_library(${THIS_PROJECT_NAME} STATIC ${SRC})
