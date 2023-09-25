#include "serialize.h"
#include "help.h"
#include "api.h"
#include <lyniat/memory.h>

namespace lyniat::socket::serialize {

    serialized_type get_st(mrb_value data){
        if(mrb_nil_p(data)){
            return ST_NIL;
        }
        mrb_vtype type = mrb_type(data);
        switch (type) {
            case MRB_TT_FALSE:
                return ST_FALSE;
            case MRB_TT_TRUE:
                return ST_TRUE;
            case MRB_TT_STRING:
                return ST_STRING;
            case MRB_TT_INTEGER:
                return ST_INT;
            case MRB_TT_FLOAT:
                return ST_FLOAT;
            case MRB_TT_SYMBOL:
                return ST_SYMBOL;
            case MRB_TT_HASH:
                return ST_HASH;
            case MRB_TT_ARRAY:
                return ST_ARRAY;
            default:
                return ST_UNDEF;
        }
    }

    void append_var_size(buffer::BinaryBuffer *binary_buffer, var_size_t number){
        auto value = number.value;
        if(number.type == B1){
            binary_buffer->Append((unsigned char)(value >> 24));
        }
        else if(number.type == B2){
            binary_buffer->Append((unsigned char)(value >> 24));
            binary_buffer->Append((unsigned char)(value >> 16));
        }
        else if(number.type == B3){
            binary_buffer->Append((unsigned char)(value >> 24));
            binary_buffer->Append((unsigned char)(value >> 16));
            binary_buffer->Append((unsigned char)(value >> 8));
        }
        else if(number.type == B4){
            binary_buffer->Append((unsigned char)(value >> 24));
            binary_buffer->Append((unsigned char)(value >> 16));
            binary_buffer->Append((unsigned char)(value >> 8));
            binary_buffer->Append((unsigned char)(value >> 0));
        }
    }

    unsigned int read_var_size(buffer::BinaryBuffer *binary_buffer){
        unsigned char d0, d1, d2, d3;
        auto start_pos = binary_buffer->CurrentPos();
        binary_buffer->Read(&d0);
        binary_buffer->Read(&d1);
        binary_buffer->Read(&d2);
        binary_buffer->Read(&d3);
        unsigned int number = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;
        auto deserialized_number = deserialize_var_size(number);
        binary_buffer->SetReadPos(start_pos + deserialized_number.type);
        return deserialized_number.value;
        /*
        auto start_pos = binary_buffer->CurrentPos();
        unsigned int temp;
        binary_buffer->Read(&temp);
        auto deserialized_num = deserialize_var_size(temp);
        auto value = deserialized_num.value;
        auto size = deserialized_num.type;
        binary_buffer->SetReadPos(start_pos + size);
        return value;
         */
    }
    
    bool add_hash_key(buffer::BinaryBuffer *binary_buffer, mrb_state *state, mrb_value key){
        auto key_type = get_st(key);

        if(key_type == ST_STRING) {
            auto s_key = API->mrb_string_cstr(state, key);
            binary_buffer->Append(key_type);
            auto str_len = strlen(s_key) + 1;
            auto var_str_len = serialize_var_size(str_len);
            append_var_size(binary_buffer, var_str_len);
            binary_buffer->Append((void*)s_key, str_len);
        }
        else if(key_type == ST_SYMBOL) {
            auto s_key = API->mrb_sym_name(state, API->mrb_obj_to_sym(state, key));
            binary_buffer->Append(key_type);
            auto str_len = strlen(s_key) + 1;
            auto var_str_len = serialize_var_size(str_len);
            append_var_size(binary_buffer, var_str_len);
            binary_buffer->Append((void*)s_key, str_len);
        }
        else if(key_type == ST_INT) {
            auto num_key = cext_to_int(state, key);
            binary_buffer->Append(key_type);
            binary_buffer->Append(num_key);
        }
        else if(key_type == ST_FLOAT) {
            auto num_key = cext_to_float(state, key);
            binary_buffer->Append(key_type);
            binary_buffer->Append(num_key);
        }
        else {
            return false;
        }

        return true;
    }

    bool set_hash_key(buffer::BinaryBuffer *binary_buffer, mrb_state *state, mrb_value hash){
        serialized_type key_type;
        binary_buffer->Read(&key_type);
        mrb_value key;

        if(key_type == ST_STRING) {
            auto data_size = read_var_size(binary_buffer);
            auto str_ptr = MALLOC_CYCLE(data_size);
            binary_buffer->Read(str_ptr, data_size);
            key = API->mrb_str_new_cstr(state, (const char*)str_ptr);
        }
        else if(key_type == ST_SYMBOL) {
            auto data_size = read_var_size(binary_buffer);
            auto str_ptr = MALLOC_CYCLE(data_size);
            binary_buffer->Read(str_ptr, data_size);
            key = mrb_symbol_value(cext_sym(state, (const char*)str_ptr));
        }
        else if(key_type == ST_INT) {
            mrb_int num_key;
            binary_buffer->Read(&num_key);
            key = API->mrb_int_value(state, num_key);
        }
        else if(key_type == ST_FLOAT) {
            mrb_float num_key;
            binary_buffer->Read(&num_key);
            key = API->mrb_float_value(state, num_key);
        }
        else {
            return false;
        }

        mrb_value data = deserialize_data(binary_buffer, state);

        API->mrb_hash_set(state, hash, key, data);
        return true;
    }

    void serialize_data(buffer::BinaryBuffer *binary_buffer, mrb_state *state, mrb_value data) {
        auto stype = get_st(data);
        auto type = (unsigned char)stype;
        if(stype == ST_FALSE || stype == ST_TRUE || stype == ST_NIL) {
            binary_buffer->Append(type);
        }

        else if(stype == ST_INT){
            mrb_int number = cext_to_int(state, data);
            binary_buffer->Append(type);
            binary_buffer->Append(number);
        }

        else if(stype == ST_FLOAT){
            mrb_float number = cext_to_float(state, data);
            binary_buffer->Append(type);
            binary_buffer->Append(number);
        }

        else if(stype == ST_STRING){
            const char *string = cext_to_string(state, data);
            auto str_len = strlen(string) + 1;
            auto var_str_len = serialize_var_size(str_len);
            binary_buffer->Append(type);
            append_var_size(binary_buffer, var_str_len);
            binary_buffer->Append((void*)string, str_len);
        }

        else if(stype == ST_SYMBOL){
            const char *string = API->mrb_sym_name(state, API->mrb_obj_to_sym(state, data));
            auto str_len = strlen(string) + 1;
            auto var_str_len = serialize_var_size(str_len);
            binary_buffer->Append(type);
            append_var_size(binary_buffer, var_str_len);
            binary_buffer->Append((void*)string, str_len);
        }

        else if(stype == ST_ARRAY){
            binary_buffer->Append(type);
            auto current_pos = binary_buffer->CurrentPos();
            binary_buffer->Append((st_counter_t)0); // array_size
            st_counter_t array_size = 0;
            for (mrb_int i = 0; i < RARRAY_LEN(data); i++) {
                auto object = RARRAY_PTR(data)[i];
                serialize_data(binary_buffer, state, object);
                array_size++;
            }
            binary_buffer->SetAt(current_pos, array_size);
        }

        else if (stype == ST_HASH) {
            binary_buffer->Append(type);
            auto current_pos = binary_buffer->CurrentPos();
            binary_buffer->Append((st_counter_t)0); // hash_size
            st_counter_t hash_size = 0;
            auto hash = mrb_hash_ptr(data);

            typedef struct to_pass_t {buffer::BinaryBuffer *buffer; st_counter_t *counter;} to_pass_t;
            to_pass_t to_pass = {binary_buffer, &hash_size};

            API->mrb_hash_foreach(state, hash, {[](mrb_state *intern_state, mrb_value key, mrb_value val, void* passed) -> int {
                auto to_pass = (to_pass_t*)passed;
                auto binary_buffer = to_pass->buffer;
                st_counter_t *hash_size = to_pass->counter;

                if(add_hash_key(binary_buffer, intern_state, key)){
                    serialize_data(binary_buffer, intern_state, val);
                    *hash_size += 1;
                }
                return 0;
            }}, &to_pass);

            binary_buffer->SetAt(current_pos, hash_size);
        }
    }

    mrb_value deserialize_data(buffer::BinaryBuffer *binary_buffer, mrb_state *state) {
        unsigned char bin_type;
        binary_buffer->Read(&bin_type);
        auto type = (serialized_type)bin_type;
        if (type == ST_FALSE) {
            return mrb_false_value();
        }
        else if (type == ST_TRUE) {
            return mrb_true_value();
        }
        else if (type == ST_NIL) {
            return mrb_nil_value();
        }
        else if (type == ST_STRING) {
            unsigned int raw_data_size;
            auto start_pos = binary_buffer->CurrentPos();
            binary_buffer->Read(&raw_data_size);
            auto deserialized_number = deserialize_var_size(raw_data_size);
            auto data_size = deserialized_number.value;
            auto byte_size = deserialized_number.type;
            auto str_ptr = MALLOC_CYCLE(data_size);
            binary_buffer->SetReadPos(start_pos + byte_size);
            binary_buffer->Read(str_ptr, data_size);
            mrb_value data = API->mrb_str_new(state, (const char*)str_ptr, data_size - 1);
            return data;
        }
        else if (type == ST_SYMBOL) {
            unsigned int raw_data_size;
            auto start_pos = binary_buffer->CurrentPos();
            binary_buffer->Read(&raw_data_size);
            auto deserialized_number = deserialize_var_size(raw_data_size);
            auto data_size = deserialized_number.value;
            auto byte_size = deserialized_number.type;
            auto str_ptr = MALLOC_CYCLE(data_size);
            binary_buffer->SetReadPos(start_pos + byte_size);
            binary_buffer->Read(str_ptr, data_size);
            mrb_value data = API->mrb_symbol_value(API->mrb_intern_check(state, (const char*)str_ptr, data_size - 1));
            return data;
        }
        else if (type == ST_INT) {
            mrb_int num;
            binary_buffer->Read(&num);
            return API->mrb_int_value(state, num);
        }
        else if (type == ST_FLOAT) {
            mrb_float num;
            binary_buffer->Read(&num);
            return API->mrb_float_value(state, num);
        }
        else if (type == ST_HASH) {
            st_counter_t hash_size;
            binary_buffer->Read(&hash_size);

            mrb_value hash = API->mrb_hash_new(state);

            for (st_counter_t i = 0; i < hash_size; ++i) {
                set_hash_key(binary_buffer, state, hash);
            }
            return hash;
        }

        else if (type == ST_ARRAY) {
            st_counter_t array_size;
            binary_buffer->Read(&array_size);

            mrb_value array = API->mrb_ary_new_capa(state, array_size);

            for (st_counter_t i = 0; i < array_size; ++i) {
                mrb_value data = deserialize_data(binary_buffer, state);
                API->mrb_ary_set(state, array, i, data);
            }
            return array;
        }

        return mrb_nil_value();
    }

    void serialize_new_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb, mrb_value data){
        binary_buffer->Append(PROTOCOL_V);
        st_address_t flags_start = 0;
        binary_buffer->Append(flags_start);
        serialize_data(binary_buffer, mrb, data);
        binary_buffer->Append(ST_SKIP); // prevent error when reading var_size_t at last position
        binary_buffer->Append(ST_SKIP);
        binary_buffer->Append(ST_SKIP);
    }

    mrb_value deserialize_new_data(buffer::BinaryBuffer *binary_buffer, mrb_state *mrb){
        unsigned short int protocol_v;
        binary_buffer->Read(&protocol_v);
        st_address_t flags_start;
        binary_buffer->Read(&flags_start);
        return deserialize_data(binary_buffer, mrb);
    }

    var_size_t serialize_var_size(unsigned int size){
        if(size <= SIZE_1B){
            auto val = size << 24;
            return {B1, val};
        } else if (size <= SIZE_2B){
            auto val = ((((size & 0x00003F00) << 1) | ((size & 0x00000080) << 1) | (size & 0x0000007F)) << 16) | 0x80000000;
            return {B2, val};
        } else if (size <= SIZE_3B){
            auto val = ((((size & 0x001F0000) << 2) | ((size & 0x0000C000) << 2) |
                    ((size & 0x00003F00) << 1) | ((size & 0x00000080) << 1) | (size & 0x0000007F)) << 8) | 0x80800000;
            return {B3, val};
        } else if (size <= SIZE_4B){
            auto val= ((((size & 0x0F000000) << 3) | ((size & 0x00E00000) << 3) |
                    ((size & 0x001F0000) << 2) | ((size & 0x0000C000) << 2) |
                    ((size & 0x00003F00) << 1) | ((size & 0x00000080) << 1) | (size & 0x0000007F))) | 0x80808000;
            return {B4, val};
        } else {
            return {B_INAVLID, 0};
        }
    }

    var_size_t deserialize_var_size(unsigned int size){
        if((size & 0x80000000) != 0){
            if((size & 0x00800000) != 0){
                if((size & 0x00008000) != 0){
                    auto value = size;
                    auto val = ((value >> 3) & 0x0F000000) | ((value >> 3) & 0x00E00000) |
                            ((value >> 2) & 0x001F0000) | ((value >> 2) & 0x0000C000) |
                            ((value >> 1) & 0x00003F00) | ((value >> 1) & 0x00000080) | (value & 0x0000007F);
                    return {B4, val};
                }

                auto value = size >> 8;
                auto val = ((value >> 2) & 0x001F0000) | ((value >> 2) & 0x0000C000) |
                           ((value >> 1) & 0x00003F00) | ((value >> 1) & 0x00000080) | (value & 0x0000007F);
                return {B3, val};
            }

            auto value = size >> 16;
            auto val = ((value >> 1) & 0x00003F00) | ((value >> 1) & 0x00000080) | (value & 0x0000007F);
            return {B2, val};
        }

        auto val = size >> 24;
        return {B1, val};
    }
}