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

#pragma once
#ifndef LYNIAT_MEMORY_MEMORY_H
#define LYNIAT_MEMORY_MEMORY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
#define DEBUG_FILENAME (char*)__FILE_NAME__ ":" STRINGIFY(__LINE__)

#ifdef DEBUG
// custom defines
#define MALLOC(size) lyniat_memory_debug_malloc(size, DEBUG_FILENAME)
#define CALLOC(num, size) lyniat_memory_debug_calloc(num, size, DEBUG_FILENAME)
#define REALLOC(ptr, new_size) lyniat_memory_debug_realloc(ptr, new_size, DEBUG_FILENAME)
#define STRDUP(ptr) lyniat_memory_debug_strdup(ptr, DEBUG_FILENAME)
#define MALLOC_CYCLE(size) lyniat_memory_debug_malloc_cycle(size, DEBUG_FILENAME)
#define STRDUP_CYCLE(ptr) lyniat_memory_debug_strdup_cycle(ptr, DEBUG_FILENAME)
#define FREE(ptr) lyniat_memory_debug_free(ptr, DEBUG_FILENAME)
#define FREE_CYCLE lyniat_memory_debug_free_cycle_memory(DEBUG_FILENAME);

#else
#define MALLOC(size) malloc(size)
#define CALLOC(num, size) calloc(num, size)
#define REALLOC(ptr, new_size) realloc(ptr, new_size)
#define STRDUP(ptr) strdup(ptr)
#define MALLOC_CYCLE(size) lyniat_memory_malloc_cycle(size)
#define STRDUP_CYCLE(ptr) lyniat_memory_strdup_cycle(ptr)
#define FREE(ptr) free(ptr)
#define FREE_CYCLE lyniat_memory_free_cycle_memory();
#endif

void *lyniat_memory_debug_malloc(size_t size, const char *info);

void *lyniat_memory_debug_calloc(size_t num, size_t size, const char *info);

void *lyniat_memory_debug_realloc(void *ptr, size_t new_size, const char *info);

const char *lyniat_memory_debug_strdup(const char* ptr, const char *info);

void *lyniat_memory_malloc_cycle(size_t size);

const char *lyniat_memory_strdup_cycle(const char* ptr);

void *lyniat_memory_debug_malloc_cycle(size_t size, const char *info);

const char *lyniat_memory_debug_strdup_cycle(const char* ptr, const char *info);

void lyniat_memory_debug_free(void *ptr, const char *info);

void lyniat_memory_check_allocated_memory();

void lyniat_memory_free_cycle_memory();

void lyniat_memory_debug_free_cycle_memory(const char *info);

void lyniat_memory_default_memory_exception(int, const char *);

void lyniat_memory_set_memory_custom_exception(void (*exc)(int, const char *));

#ifdef __cplusplus
}
#endif

#endif // LYNIAT_MEMORY_MEMORY_H