#pragma once
#ifndef DR_SOCKET_BUFFER_H
#define DR_SOCKET_BUFFER_H

#include <cstddef>

namespace lyniat::socket::buffer {

    class BinaryBuffer{
    public:
        BinaryBuffer();
        ~BinaryBuffer();

        template<typename T>
        void Append(T data){
            AppendData(&data, sizeof(data));
        }

        template<typename T>
        void Append(T *data, int size){
            AppendData(data, size);
        }

        void *Data();

        unsigned int Size();

        //void Append(void *data, int size);

    private:
        void* ptr;
        unsigned int b_size;
        unsigned int b_length;
        void AppendData(void *data, int size);
    };

}

#endif