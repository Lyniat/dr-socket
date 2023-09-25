#pragma once
#ifndef DR_SOCKET_SERIALIZE_H
#define DR_SOCKET_SERIALIZE_H

#include "ruby.h"
#include "buffer.h"

namespace lyniat::socket::serialize {

    static const unsigned char FLAG_SERVER =   0b00000001;
    static const unsigned char FLAG_CLIENTS =  0b00000010;
    static const unsigned char FLAG_SELF =     0b00000100;

    typedef unsigned short int st_counter_t;

    enum serialized_type : unsigned char {
        ST_FALSE = 0,
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

    void serialize_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb, mrb_value data);

    mrb_value deserialize_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb);

    serialized_type get_st(mrb_value data);

}

#endif