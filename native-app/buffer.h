#pragma once
#ifndef DR_SOCKET_BUFFER_H
#define DR_SOCKET_BUFFER_H

#include <cstddef>

namespace lyniat::socket::buffer {

    class BinaryBuffer{
    public:
        BinaryBuffer();
        BinaryBuffer(void* ptr, unsigned int size, bool copy = true);
        ~BinaryBuffer();

        template<typename T>
        void Append(T data){
            AppendData(&data, sizeof(T));
        }

        template<typename T>
        void Append(T *data, unsigned int size){
            AppendData(data, size);
        }

        template<typename T>
        void SetAt(unsigned int pos, T data){
            SetDataAt(pos, &data, sizeof(T));
        }

        template<typename T>
        void SetAt(unsigned int pos, T *data, unsigned int size){
            SetDataAt(pos, data, size);
        }

        template<typename T>
        void Read(T *data){
            ReadData(data, sizeof(T));
        }

        template<typename T>
        void Read(T *data, unsigned int size){
            ReadData(data, size);
        }

        void *Data();

        unsigned int Size();

        bool ReadOnly();

        unsigned int CurrentPos();

        void SetReadPos(unsigned int pos);

        //void Append(void *data, int size);

    private:
        void* ptr;
        unsigned int b_size;
        unsigned int b_length;
        void AppendData(void *data, unsigned int size);
        void SetDataAt(unsigned int pos, void *data, unsigned int size);
        void ReadData(void *data, unsigned int size);
        unsigned int current_pos;
        bool must_free;
        bool read_only;

        // read feature
        unsigned int current_read_pos;
    };

}

#endif