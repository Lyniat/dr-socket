#include <cstdlib>
#include <map>

#include "../include/lyniat/memory.h"

namespace lyniat::memory {

    std::map<uintptr_t, const char *> memory_allocations;
    std::vector<uintptr_t> memory_allocations_cycle;

    typedef void (*custom_exception_t)(int, const char *);

    custom_exception_t custom_exception;

    void set_memory_custom_exception(void (*exc)(int, const char *)) {
        custom_exception = exc;
    }

    void default_memory_exception(int val, const char *info) {
        printf("MEMORY EXCEPTION: %s\n", info);
    }

    void *debug_malloc(size_t size, const char *info) {
        auto ptr = malloc(size);
        memory_allocations[(uintptr_t) ptr] = info;
        return ptr;
    }

    void debug_free(void *ptr, const char *info) {
        if (memory_allocations.count((uintptr_t) ptr) == 0) {
            if (custom_exception != nullptr) {
                custom_exception(1, info);
                return;
            }
            default_memory_exception(1, info);
            return;
        }
        memory_allocations.erase((uintptr_t) ptr);
        free(ptr);
    }

    void check_allocated_memory() {
        if (memory_allocations.size() == 0) {
            printf("No memory allocated.\n");
            return;
        }

        int i = 0;
        for (auto const &mem: memory_allocations) {
            ++i;
            printf("(%i) Memory allocated from %s.\n", i, mem.second);
        }
    }

    void *malloc_cycle(size_t size) {
        auto ptr = malloc(size);
        memory_allocations_cycle.push_back((uintptr_t) ptr);
        return ptr;
    }

    void *debug_malloc_cycle(size_t size, const char *info) {
        auto ptr = malloc(size);
        memory_allocations[(uintptr_t) ptr] = info;
        memory_allocations_cycle.push_back((uintptr_t) ptr);
        return ptr;
    }

    void free_cycle_memory() {
        for (auto m: memory_allocations_cycle) {
            free((void *) m);
        }
        memory_allocations_cycle.clear();
    }

    void debug_free_cycle_memory(const char *info) {
        for (auto m: memory_allocations_cycle) {
            if (memory_allocations.count(m) == 0) {
                if (custom_exception != nullptr) {
                    custom_exception(1, info);
                    return;
                }
                default_memory_exception(1, info);
                return;
            }
            memory_allocations.erase((uintptr_t) m);
            free((void *) m);
        }
        memory_allocations_cycle.clear();
    }
}