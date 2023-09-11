#include "buffer.h"
#include "lyniat/memory.h"

namespace lyniat::socket::buffer {

    BinaryBuffer::BinaryBuffer() {
        ptr = nullptr;
        b_size = 0;
        b_length = 0;
        current_pos = 0;
        current_read_pos = 0;
        must_free = true;
        read_only = false;
    }

    BinaryBuffer::BinaryBuffer(void* new_ptr, unsigned int size, bool copy){
        ptr = nullptr;
        b_size = 0;
        b_length = 0;
        current_pos = 0;
        current_read_pos = 0;
        read_only = true;
        if(copy){
            ptr = MALLOC(size);
            memcpy(ptr, new_ptr, size);
            b_size = size;
            b_length = size;
            must_free = true;
        }else{
            ptr = new_ptr;
            b_size = size;
            b_length = size;
            must_free = false;
        }
    }

    BinaryBuffer::~BinaryBuffer() {
        if(must_free){
            FREE(ptr);
        }
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

    bool BinaryBuffer::ReadOnly(){
        return read_only;
    }

    unsigned int BinaryBuffer::CurrentPos(){
        return current_pos;
    }

    void BinaryBuffer::AppendData(void *data, unsigned int size) {
        if(read_only){
            return;
        }
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
        current_pos += size;
    }

    void BinaryBuffer::SetDataAt(unsigned int pos, void *data, unsigned int size){
        if(read_only){
            return;
        }
        if(pos + size < b_size){
            memcpy((char*)ptr + pos, data, size);
        }
    }

    void BinaryBuffer::ReadData(void *data, unsigned int size) {
        if(current_read_pos + size <= b_size){
            memcpy(data, (char*)ptr + current_read_pos, size);
            current_read_pos += size;
        }
    }
}