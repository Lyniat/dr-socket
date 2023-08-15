cmake_minimum_required(VERSION 3.22)

project(memory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(SRC
        include/lyniat/memory.h
        src/memory.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-exceptions -fno-rtti")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -g -fno-eliminate-unused-debug-symbols")

add_library(memory STATIC ${SRC})

install(FILES $<TARGET_FILE:memory> DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES include/lyniat/memory.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lyniat)