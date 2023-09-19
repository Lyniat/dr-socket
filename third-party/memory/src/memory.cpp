/*
* MIT License
*
* Copyright (c) 2023 Laurin Muth
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
*         of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
*         to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*         copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
*         copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*         AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <map>
#include <vector>
#include <string>

#include "../include/lyniat/memory.h"

std::map<uintptr_t, const char *> memory_allocations;
std::vector<uintptr_t> memory_allocations_cycle;

typedef void (*custom_exception_t)(int, const char *);

custom_exception_t custom_exception;

void lyniat_memory_set_memory_custom_exception(void (*exc)(int, const char *)) {
    custom_exception = exc;
}

void lyniat_memory_default_memory_exception(int val, const char *info) {
    printf("MEMORY EXCEPTION: %s\n", info);
}

void *lyniat_memory_debug_malloc(size_t size, const char *info) {
    auto ptr = malloc(size);
    memory_allocations[(uintptr_t) ptr] = info;
    return ptr;
}

void *lyniat_memory_debug_calloc(size_t num, size_t size, const char *info) {
    auto ptr = malloc(size * num);
    memory_allocations[(uintptr_t) ptr] = info;
    memset(ptr, 0, size * num);
    return ptr;
}

void *lyniat_memory_debug_realloc(void* ptr, size_t new_size, const char *info){
    auto new_ptr = malloc(new_size);

    memory_allocations[(uintptr_t) new_ptr] = info;
    memcpy(new_ptr, ptr, new_size);

    memory_allocations.erase((uintptr_t) ptr);
    free(ptr);
    return new_ptr;
}

const char *lyniat_memory_debug_strdup(const char* ptr, const char *info){
    auto new_ptr = calloc(strlen(ptr) + 1, 1);

    memory_allocations[(uintptr_t) new_ptr] = info;
    memcpy(new_ptr, ptr, strlen(ptr));

    return (const char*)new_ptr;
}

void lyniat_memory_debug_free(void *ptr, const char *info) {
    if (memory_allocations.count((uintptr_t) ptr) == 0) {
        if (custom_exception != nullptr) {
            custom_exception(1, info);
            return;
        }
        lyniat_memory_default_memory_exception(1, info);
        return;
    }
    memory_allocations.erase((uintptr_t) ptr);
    free(ptr);
}

const char *lyniat_memory_check_allocated_memory() {
    if (memory_allocations.size() == 0) {
        return "No memory allocated.\n";
    }

    std::string output = "";
    int i = 0;
    for (auto const &mem: memory_allocations) {
        ++i;
        output.append("Memory allocated from ");
        output.append(mem.second);
        output.append(".\n");
    }
    std::string result = "";
    result.append(std::to_string(i));
    result.append(" memory leaks in total.\n");
    result.append(output);

    return STRDUP_CYCLE(output.c_str());
}

void *lyniat_memory_malloc_cycle(size_t size) {
    auto ptr = malloc(size);
    memory_allocations_cycle.push_back((uintptr_t) ptr);
    return ptr;
}

const char *lyniat_memory_strdup_cycle(const char* ptr) {
    auto new_ptr = calloc(strlen(ptr) + 1, 1);
    memcpy(new_ptr, ptr, strlen(ptr));
    memory_allocations_cycle.push_back((uintptr_t) new_ptr);
    return (const char*)new_ptr;
}

void *lyniat_memory_debug_malloc_cycle(size_t size, const char *info) {
    auto ptr = malloc(size);
    memory_allocations[(uintptr_t) ptr] = info;
    memory_allocations_cycle.push_back((uintptr_t) ptr);
    return ptr;
}

const char *lyniat_memory_debug_strdup_cycle(const char* ptr, const char *info) {
    auto new_ptr = calloc(strlen(ptr) + 1, 1);
    memory_allocations[(uintptr_t) new_ptr] = info;
    memcpy(new_ptr, ptr, strlen(ptr));
    memory_allocations_cycle.push_back((uintptr_t) new_ptr);
    return (const char*)new_ptr;
}

void lyniat_memory_free_cycle_memory() {
    for (auto m: memory_allocations_cycle) {
        free((void *) m);
    }
    memory_allocations_cycle.clear();
}

void lyniat_memory_debug_free_cycle_memory(const char *info) {
    for (auto m: memory_allocations_cycle) {
        if (memory_allocations.count(m) == 0) {
            if (custom_exception != nullptr) {
                custom_exception(1, info);
                return;
            }
            lyniat_memory_default_memory_exception(1, info);
            return;
        }
        memory_allocations.erase((uintptr_t) m);
        free((void *) m);
    }
    memory_allocations_cycle.clear();
}