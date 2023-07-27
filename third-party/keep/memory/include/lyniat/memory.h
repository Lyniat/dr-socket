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
#define MALLOC(size) lyniat::memory::debug_malloc(size, DEBUG_FILENAME)
#define MALLOC_CYCLE(size) lyniat::memory::debug_malloc_cycle(size, DEBUG_FILENAME)
#define FREE(ptr) lyniat::memory::debug_free(ptr, DEBUG_FILENAME)
#define FREE_CYCLE lyniat::memory::debug_free_cycle_memory(DEBUG_FILENAME);
#else
#define MALLOC(size) malloc(size)
#define MALLOC_CYCLE(size) lyniat::memory::malloc_cycle(size)
#define FREE(ptr) free(ptr)
#define FREE_CYCLE lyniat::memory::free_cycle_memory();
#endif

namespace lyniat {
    namespace memory {

        void *debug_malloc(size_t size, const char *info);

        void *malloc_cycle(size_t size, const char *info);

        void *debug_malloc_cycle(size_t size, const char *info);

        void debug_free(void *ptr, const char *info);

        void check_allocated_memory();

        void free_cycle_memory();

        void debug_free_cycle_memory(const char *info);

        void default_memory_exception(int, const char *);

        void set_memory_custom_exception(void (*exc)(int, const char *));

    }
}

#endif