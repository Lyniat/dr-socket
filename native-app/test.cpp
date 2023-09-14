#include "test.h"
#include "serialize.h"
#include "help.h"
#include "api.h"

using namespace lyniat::socket;

#ifdef DEBUG

mrb_value test_serialize_data(mrb_state* mrb, mrb_value self){
    mrb_value data;
    API->mrb_get_args(mrb, "H", &data);

    auto buffer = new buffer::BinaryBuffer();
    serialize::serialize_data(buffer, mrb, data);

    auto ptr = (uintptr_t)buffer->Data();
    auto size = (mrb_int)buffer->Size();
    auto hash = API->mrb_hash_new(mrb);
    cext_hash_set_kstr(mrb, hash, "ptr", API->mrb_int_value(mrb, ptr));
    cext_hash_set_kstr(mrb, hash, "size", API->mrb_int_value(mrb, size));
    delete buffer;
    return hash;
}

mrb_value test_deserialize_data(mrb_state* mrb, mrb_value self){
    mrb_value data;
    API->mrb_get_args(mrb, "H", &data);
    auto hash_ptr = cext_hash_get(mrb, data, "ptr");
    auto hash_size = cext_hash_get(mrb, data, "size");
    auto ptr = (uintptr_t)cext_to_int(mrb, hash_ptr);
    mrb_int size = cext_to_int(mrb, hash_size);
    //auto result = serialize::deserialize_data(mrb, (const char*)ptr, size, &position);
    auto buffer = new buffer::BinaryBuffer((void*)ptr, size, false);
    auto result = serialize::deserialize_data(buffer, mrb);
    delete buffer;
    return result;
}

void register_test_functions(mrb_state* state, RClass* module){
    API->mrb_define_module_function(state, module, "__test_serialize", test_serialize_data, MRB_ARGS_REQ(1));
    API->mrb_define_module_function(state, module, "__test_deserialize", test_deserialize_data, MRB_ARGS_REQ(1));
}

#else
void register_test_functions(mrb_state* state, RClass* module){
}
#endif