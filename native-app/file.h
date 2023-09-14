#pragma once
#ifndef DR_SOCKET_FILE_H
#define DR_SOCKET_FILE_H

#include "buffer.h"
#include "api.h"

using namespace lyniat::socket;

namespace lyniat::socket::file {

    void save_binary(const char *name, void *ptr, unsigned int size);

    void save_buffer(const char *name, buffer::BinaryBuffer buffer);

    mrb_value debug_serialized_to_file(mrb_state* state, mrb_value self);

}

#endif