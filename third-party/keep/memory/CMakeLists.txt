cmake_minimum_required(VERSION 3.22)

set(THIS_PROJECT_NAME "memory")

project(${THIS_PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(SRC
        include/memory.h
        src/memory.cpp
)

add_library(${THIS_PROJECT_NAME} STATIC ${SRC})
