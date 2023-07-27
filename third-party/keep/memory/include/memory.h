#ifndef DR_SOCKET_MEMORY_H
#define DR_SOCKET_MEMORY_H

#include <cstdlib>
#include <map>
#include <cstdio>
#include <string>

#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
#define DEBUG_FILENAME (char*)__FILE_NAME__ ":" STRINGIFY(__LINE__)

#ifdef DEBUG
#define MALLOC(size) debug_malloc(size, DEBUG_FILENAME)
#define FREE(ptr) debug_free(ptr)
#else
#define MALLOC(size) malloc(size)
#define FREE(ptr) free(ptr)
#endif

void *debug_malloc(size_t size, const char* info);

void debug_free(void *ptr);

void check_allocated_memory();

#endif