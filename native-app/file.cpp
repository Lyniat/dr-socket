#include "file.h"
#include "api.h"
#include "help.h"

namespace lyniat::socket::file {

    void save_binary(const char *name, void *ptr, unsigned int size) {
        auto file = API->PHYSFS_openWrite(name);
        auto length = API->PHYSFS_writeBytes(file, ptr, size);
        API->PHYSFS_close(file);
    }

    void save_buffer(const char *name, buffer::BinaryBuffer buffer) {
        save_binary(name, buffer.Data(), buffer.Size());
    }

    mrb_value debug_serialized_to_file(mrb_state* state, mrb_value self){
        mrb_value data;
        API->mrb_get_args(state, "H", &data);
        auto hash_ptr = cext_hash_get(state, data, "ptr");
        auto hash_size = cext_hash_get(state, data, "size");
        auto hash_name = cext_hash_get(state, data, "name");
        auto ptr = (uintptr_t)cext_to_int(state, hash_ptr);
        mrb_int size = cext_to_int(state, hash_size);
        auto c_name = cext_to_string(state, hash_name);
        save_binary(c_name, (void*)ptr, size);
        return mrb_nil_value();
    }
}