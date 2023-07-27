#include <cstdlib>
#include <map>

std::map<uintptr_t, const char*> memory_allocations;

void *debug_malloc(size_t size, const char* info){
    auto ptr = malloc(size);
    memory_allocations[(uintptr_t)ptr] = info;
    return ptr;
}

void debug_free(void *ptr){
    memory_allocations.erase((uintptr_t)ptr);
    free(ptr);
}

void check_allocated_memory(){
    if(memory_allocations.size() == 0){
        printf("No memory allocated.\n");
        return;
    }

    int i = 0;
    for (auto const& mem : memory_allocations)
    {
        ++i;
        printf("(%i) Memory allocated from %s.\n",i , mem.second);
    }
}