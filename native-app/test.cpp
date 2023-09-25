#include "test.h"
#include "serialize.h"
#include "help.h"
#include "api.h"
#include "lyniat/memory.h"
#include "fmt/format.h"
#include <bitset>

using namespace lyniat::socket;

#ifdef DEBUG

mrb_value test_serialize_data(mrb_state* state, mrb_value self){
    mrb_value data;
    API->mrb_get_args(state, "H", &data);

    auto buffer = new buffer::BinaryBuffer();
    serialize::serialize_new_data(buffer, state, data);

    auto ptr = (uintptr_t)buffer->Data();
    auto size = (mrb_int)buffer->Size();
    auto hash = API->mrb_hash_new(state);
    cext_hash_set_kstr(state, hash, "ptr", API->mrb_int_value(state, ptr));
    cext_hash_set_kstr(state, hash, "size", API->mrb_int_value(state, size));
    delete buffer;
    return hash;
}

mrb_value test_deserialize_data(mrb_state* state, mrb_value self){
    mrb_value data;
    API->mrb_get_args(state, "H", &data);
    auto hash_ptr = cext_hash_get(state, data, "ptr");
    auto hash_size = cext_hash_get(state, data, "size");
    auto ptr = (uintptr_t)cext_to_int(state, hash_ptr);
    mrb_int size = cext_to_int(state, hash_size);
    auto buffer = new buffer::BinaryBuffer((void*)ptr, size, false);
    auto result = serialize::deserialize_new_data(buffer, state);
    delete buffer;
    return result;
}

std::string to_bin_string(serialize::var_size_t var){
    std::string bin_string;

    if(var.type == serialize::B1){
        auto d0 = std::bitset<8>(var.value >> 24).to_string();
        bin_string = fmt::format("0b{}", d0);
    }else if(var.type == serialize::B2){
        auto d0 = std::bitset<8>(var.value >> 24).to_string();
        auto d1 = std::bitset<8>(var.value >> 16).to_string();
        bin_string = fmt::format("0b{}.{}", d0, d1);
    }else if(var.type == serialize::B3){
        auto d0 = std::bitset<8>(var.value >> 24).to_string();
        auto d1 = std::bitset<8>(var.value >> 16).to_string();
        auto d2 = std::bitset<8>(var.value >> 8).to_string();
        bin_string = fmt::format("0b{}.{}.{}", d0, d1, d2);
    }else if(var.type == serialize::B4){
        auto d0 = std::bitset<8>(var.value >> 24).to_string();
        auto d1 = std::bitset<8>(var.value >> 16).to_string();
        auto d2 = std::bitset<8>(var.value >> 8).to_string();
        auto d3 = std::bitset<8>(var.value).to_string();
        bin_string = fmt::format("0b{}.{}.{}.{}", d0, d1, d2, d3);
    }

    return bin_string;
}

mrb_value test_serialize_number(mrb_state* state, mrb_value self){
    mrb_int number;
    API->mrb_get_args(state, "i", &number);
    signed int s_number = number;
    unsigned int u_number = s_number;
    auto data = serialize::serialize_var_size(u_number);

    auto hash = API->mrb_hash_new(state);
    cext_hash_set_kstr(state, hash, "number", API->mrb_int_value(state, data.value));
    cext_hash_set_kstr(state, hash, "size", API->mrb_int_value(state, data.type));

    auto bin_string = to_bin_string(data);
    cext_hash_set_kstr(state, hash, "bin_str", API->mrb_str_new_cstr(state, bin_string.c_str()));
    return hash;
}

mrb_value test_deserialize_number(mrb_state* state, mrb_value self){
    mrb_value data;
    API->mrb_get_args(state, "H", &data);
    auto number_data = cext_hash_get(state, data, "number");
    auto u_number = serialize::deserialize_var_size(cext_to_int(state, number_data));
    signed int s_number = u_number.value;
    return API->mrb_int_value(state, s_number);
}

void register_test_functions(mrb_state* state, RClass* module){
    API->mrb_define_module_function(state, module, "__test_serialize", test_serialize_data, MRB_ARGS_REQ(1));
    API->mrb_define_module_function(state, module, "__test_deserialize", test_deserialize_data, MRB_ARGS_REQ(1));
    API->mrb_define_module_function(state, module, "__test_serialize_number", test_serialize_number, MRB_ARGS_REQ(1));
    API->mrb_define_module_function(state, module, "__test_deserialize_number", test_deserialize_number, MRB_ARGS_REQ(1));
}

#else
void register_test_functions(mrb_state* state, RClass* module){
}
#endif