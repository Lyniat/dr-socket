#pragma once
#ifndef DR_SOCKET_SERIALIZE_H
#define DR_SOCKET_SERIALIZE_H

#include "ruby.h"
#include "buffer.h"

namespace lyniat::socket::serialize {

    static const unsigned char FLAG_SERVER =   0b00000001;
    static const unsigned char FLAG_CLIENTS =  0b00000010;
    static const unsigned char FLAG_SELF =     0b00000100;

    static const unsigned int SIZE_1B =     0x7F; // 127 (128 - 1)
    static const unsigned int SIZE_2B =     0x3FFF; // 16383 (128^2 - 1)
    static const unsigned int SIZE_3B =     0x1FFFFF; // 2097151 (128^3 - 1)
    static const unsigned int SIZE_4B =     0x0FFFFFFF; // 268435455 (128^4 - 1)

    static const unsigned short int PROTOCOL_V = 1;

    typedef unsigned short int st_counter_t;

    typedef unsigned int st_address_t;
    typedef unsigned int st_flags_t;

    enum var_size_type : unsigned char {
        B_INAVLID = 0,
        B1 = 1,
        B2 = 2,
        B3 = 3,
        B4 = 4
    };

    typedef struct var_size_t {
        var_size_type type;
        unsigned int value;
    } var_size_t;

    enum serialized_type : unsigned char {
        ST_SKIP = 0,
        ST_FALSE = 1,
        ST_TRUE,
        ST_INT,
        ST_FLOAT,
        ST_SYMBOL,
        ST_HASH,
        ST_ARRAY,
        ST_STRING,
        ST_UNDEF,
        ST_NIL
    };

    var_size_t serialize_var_size(unsigned int size);

    var_size_t deserialize_var_size(unsigned int);

    void serialize_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb, mrb_value data);

    mrb_value deserialize_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb);

    void serialize_new_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb, mrb_value data);

    mrb_value deserialize_new_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb);

}

#endif