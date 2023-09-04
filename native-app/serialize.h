#pragma once
#ifndef DR_SOCKET_SERIALIZE_H
#define DR_SOCKET_SERIALIZE_H

#include "ruby.h"
#include "buffer.h"

namespace lyniat::socket::serialize {

    typedef struct serialized_data_t {
        mrb_vtype type;
        void *data;
        int size;
        int amount;
    } serialized_data_t;

    typedef struct serialized_hash_t {
        const char *key;
        serialized_data_t data;
    } serialized_hash_t;

    serialized_data_t serialize_data(mrb_state *mrb, mrb_value data);

    mrb_value deserialize_data(mrb_state *mrb, const char *buffer, int size, int *position);

    void serialize_data_to_buffer(buffer::BinaryBuffer *binary_buffer, serialized_data_t data);

}

#endif