#include "serialize.h"
#include "help.h"
#include "api.h"
#include <vector>
#include <lyniat/memory.h>

namespace lyniat::socket::serialize {

    serialized_data_t serialize_data(mrb_state *mrb, mrb_value data) {
        mrb_vtype type = mrb_type(data);
        if (type == MRB_TT_FALSE) {
            serialized_data_t serialized_bool = {MRB_TT_FALSE, nullptr, 0, 0};
            return serialized_bool;
        }

        if (type == MRB_TT_TRUE) {
            serialized_data_t serialized_bool = {MRB_TT_TRUE, nullptr, 0, 0};
            return serialized_bool;
        }

        if (type == MRB_TT_STRING) {
            const char *string = cext_to_string(mrb, data);
            size_t str_len = strlen(string);
            const char *string_dup = STRDUP_CYCLE(string);
            serialized_data_t serialized_string = {MRB_TT_STRING, (void *) string_dup, (int) (str_len + 1), 0};
            return serialized_string;
        }

        if (type == MRB_TT_INTEGER) {
            mrb_int number = cext_to_int(mrb, data);
            void *number_copy = MALLOC_CYCLE(sizeof(mrb_int));
            memcpy(number_copy, &number, sizeof(mrb_int));
            serialized_data_t serialized_int = {MRB_TT_INTEGER, number_copy, sizeof(mrb_int), 0};
            return serialized_int;
        }

        if (type == MRB_TT_FLOAT) {
            mrb_float number = cext_to_float(mrb, data);
            void *number_copy = MALLOC_CYCLE(sizeof(mrb_float));
            memcpy(number_copy, &number, sizeof(mrb_float));
            serialized_data_t serialized_float = {MRB_TT_FLOAT, number_copy, sizeof(mrb_float), 0};
            return serialized_float;
        }

        if (type == MRB_TT_SYMBOL) {
            const char *string = API->mrb_sym_name(mrb, API->mrb_obj_to_sym(mrb, data));
            size_t str_len = strlen(string);
            const char *string_dup = STRDUP_CYCLE(string);
            serialized_data_t serialized_string = {MRB_TT_SYMBOL, (void *) string_dup, (int) (str_len + 1), 0};
            return serialized_string;
        }

        if (type == MRB_TT_HASH) {
            mrb_value keys = API->mrb_hash_keys(mrb, data);
            mrb_value key = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, keys));
            std::vector<serialized_hash_t> data_vector;
            int data_size = 0;
            int hash_size = 0;
            while (cext_is_symbol(mrb, key) || cext_is_string(mrb, key)) {
                const char *s_key;
                if(cext_is_symbol(mrb, key)){
                    s_key = STRDUP_CYCLE(API->mrb_sym_name(mrb, API->mrb_obj_to_sym(mrb, key)));
                } else {
                    s_key = STRDUP_CYCLE(API->mrb_string_cstr(mrb, key));
                }
                mrb_value content = API->mrb_hash_get(mrb, data, key);
                serialized_data_t serialized_data = serialize_data(mrb, content);
                serialized_hash_t serialized_hash_entry = {s_key, serialized_data};
                hash_size++;
                data_size += serialized_data.size;
                data_vector.push_back(serialized_hash_entry);
                key = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, keys));
            }
            void *raw_data = data_vector.data();
            void *raw_data_copy = MALLOC_CYCLE(hash_size * sizeof(serialized_hash_t));
            memcpy(raw_data_copy, raw_data, hash_size * sizeof(serialized_hash_t));
            serialized_data_t serialized_hash = {MRB_TT_HASH, raw_data_copy, data_size, hash_size};
            return serialized_hash;
        }

        if (type == MRB_TT_ARRAY) {
            std::vector<serialized_data_t> data_vector;
            mrb_value object = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, data));
            int data_size = 0;
            int array_size = 0;
            while (object.w != 0) {
                serialized_data_t serialized_data = serialize_data(mrb, object);
                array_size++;
                data_size += serialized_data.size;
                data_vector.push_back(serialized_data);
                object = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, data));
            }
            void *raw_data = data_vector.data();
            void *raw_data_copy = MALLOC_CYCLE(array_size * sizeof(serialized_data_t));
            memcpy(raw_data_copy, raw_data, array_size * sizeof(serialized_data_t));
            serialized_data_t serialized_array = {MRB_TT_ARRAY, raw_data_copy, data_size, array_size};
            return serialized_array;
        }

        serialized_data_t serialized_undef = {MRB_TT_UNDEF, (void *) nullptr, 0};
        return serialized_undef;
    }

    mrb_value deserialize_data(mrb_state *mrb, const char *buffer, int size, int *position) {
        char c_type = buffer[*position];
        mrb_vtype type = (mrb_vtype) c_type;
        if (type == MRB_TT_FALSE) {
            ++*position;
            return mrb_false_value();
        }
        if (type == MRB_TT_TRUE) {
            ++*position;
            return mrb_true_value();
        }
        if (type == MRB_TT_STRING) {
            ++*position;
            int data_size = buffer[*position];
            *position += sizeof(int);
            mrb_value data = API->mrb_str_new_cstr(mrb, buffer + *position);
            *position += data_size;
            return data;
        }
        if (type == MRB_TT_SYMBOL) {
            ++*position;
            int data_size = buffer[*position];
            *position += sizeof(int);
            mrb_value data = API->mrb_symbol_value(cext_sym(mrb, buffer + *position));
            *position += data_size;
            return data;
        }
        if (type == MRB_TT_INTEGER) {
            ++*position;
            mrb_int *integer = (mrb_int *) (buffer + *position);
            *position += sizeof(mrb_int);
            return API->mrb_int_value(mrb, *integer);
        }
        if (type == MRB_TT_FLOAT) {
            ++*position;
            mrb_float *d = (mrb_float *) (buffer + *position);
            *position += sizeof(mrb_float);
            return API->mrb_float_value(mrb, *d);
        }
        if (type == MRB_TT_HASH) {
            ++*position;
            int data_size = buffer[*position];
            *position += sizeof(int);
            int data_amount = buffer[*position];
            *position += sizeof(int);

            mrb_value hash = API->mrb_hash_new(mrb);

            for (int i = 0; i < data_amount; ++i) {
                //key
                int key_size = buffer[*position];
                *position += sizeof(int);
                const char *key = buffer + *position;
                *position += key_size;

                mrb_value data = deserialize_data(mrb, buffer, size, position);

                cext_hash_set(mrb, hash, key, data);
            }
            return hash;
        }

        if (type == MRB_TT_ARRAY) {
            ++*position;
            int data_size = buffer[*position];
            *position += sizeof(int);
            int data_amount = buffer[*position];
            *position += sizeof(int);

            mrb_value array = API->mrb_ary_new(mrb);

            for (int i = 0; i < data_amount; ++i) {
                mrb_value data = deserialize_data(mrb, buffer, size, position);

                API->mrb_ary_push(mrb, array, data);
            }
            return array;
        }

        return mrb_nil_value();
    }

    int serialize_data_to_buffer(char *buffer, int size, int position, serialized_data_t data) {
        char c_type = data.type;
        if (data.type == MRB_TT_STRING || data.type == MRB_TT_SYMBOL) {
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, (void *) &data.size, sizeof(int));
            position += sizeof(int);
            memcpy(buffer + position, data.data, data.size);
            position += data.size;
        }
        if (data.type == MRB_TT_INTEGER) {
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, data.data, sizeof(mrb_int));
            position += sizeof(mrb_int);
        }
        if (data.type == MRB_TT_FALSE) {
            buffer[position] = c_type;
            ++position;
        }
        if (data.type == MRB_TT_TRUE) {
            buffer[position] = c_type;
            ++position;
        }
        if (data.type == MRB_TT_FLOAT) {
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, data.data, sizeof(mrb_float));
            position += sizeof(mrb_float);
        }
        if (data.type == MRB_TT_HASH) {
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, (void *) &data.size, sizeof(int));
            position += sizeof(int);
            memcpy(buffer + position, (void *) &data.amount, sizeof(int));
            position += sizeof(int);
            auto *hash_entries = (serialized_hash_t *) data.data;
            for (int i = 0; i < data.amount; ++i) {
                serialized_hash_t hash_entry = hash_entries[i];
                const char *key = hash_entry.key;

                //key
                int key_len = (int) strlen(key) + 1;
                memcpy(buffer + position, (void *) &key_len, sizeof(int));
                position += sizeof(int);
                memcpy(buffer + position, key, key_len);
                position += key_len;
                position = serialize_data_to_buffer(buffer, size, position, hash_entry.data);
            }
        }
        if (data.type == MRB_TT_ARRAY) {
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, (void *) &data.size, sizeof(int));
            position += sizeof(int);
            memcpy(buffer + position, (void *) &data.amount, sizeof(int));
            position += sizeof(int);
            auto *array_entries = (serialized_data_t *) data.data;
            for (int i = 0; i < data.amount; ++i) {
                serialized_data_t array_entry = array_entries[i];
                position = serialize_data_to_buffer(buffer, size, position, array_entry);
            }
        }
        return position;
    }
}