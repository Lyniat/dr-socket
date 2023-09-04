#include "buffer.h"
#include "lyniat/memory.h"

namespace lyniat::socket::buffer {

    BinaryBuffer::BinaryBuffer() {
        ptr = nullptr;
        b_size = 0;
        b_length = 0;
    }

    BinaryBuffer::~BinaryBuffer() {
        FREE(ptr);
    }

    /*
    void BinaryBuffer::Append(void *data, int size) {
        AppendData(data, size);
    }
     */

    void *BinaryBuffer::Data(){
        return ptr;
    }

    unsigned int BinaryBuffer::Size(){
        return b_size;
    }

    void BinaryBuffer::AppendData(void *data, int size) {
        if (b_size + size > b_length){
            if(b_length == 0){
                const unsigned int MiB = 1024 * 1024;
                b_length = MiB > size ? MiB : size;
                b_size = 0;
                ptr = MALLOC(b_length);
            }else{
                auto new_length = b_length * 2 + size;
                auto new_ptr = MALLOC(new_length);
                memcpy(new_ptr, ptr, b_size);
                b_length = new_length;
                FREE(ptr);
                ptr = new_ptr;
            }
        }
        memcpy((char*)ptr + b_size, data, size);
        b_size = b_size + size;
    }
}