#include "serialize.h"
#include "help.h"
#include "api.h"
#include <vector>
#include <lyniat/memory.h>

namespace lyniat::socket::serialize {

    serialized_data_t serialize_data(mrb_state *mrb, mrb_value data) {
        mrb_vtype type = mrb_type(data);
        if (type == MRB_TT_FALSE) {
            serialized_data_t serialized_bool = {.type = MRB_TT_FALSE, .data = nullptr, .size = 0, .amount = 0};
            return serialized_bool;
        }

        if (type == MRB_TT_TRUE) {
            serialized_data_t serialized_bool = {.type = MRB_TT_TRUE, .data = nullptr, .size = 0, .amount = 0};
            return serialized_bool;
        }

        if (type == MRB_TT_STRING) {
            const char *string = cext_to_string(mrb, data);
            //printf("%s\n", string);
            size_t str_len = strlen(string);
            const char *string_dup = strdup(string);
            serialized_data_t serialized_string = {.type = MRB_TT_STRING, .data = (void *) string_dup, .size = (int) (
                    str_len + 1), .amount = 0};
            return serialized_string;
        }

        if (type == MRB_TT_INTEGER) {
            mrb_int number = cext_to_int(mrb, data);
            void *number_copy = MALLOC_CYCLE(sizeof(mrb_int));
            memcpy(number_copy, &number, sizeof(mrb_int));
            serialized_data_t serialized_int = {.type = MRB_TT_INTEGER, .data = number_copy, .size = sizeof(mrb_int), .amount = 0};
            return serialized_int;
        }

        if (type == MRB_TT_FLOAT) {
            mrb_float number = cext_to_float(mrb, data);
            void *number_copy = MALLOC_CYCLE(sizeof(mrb_float));
            memcpy(number_copy, &number, sizeof(mrb_float));
            serialized_data_t serialized_float = {.type = MRB_TT_FLOAT, .data = number_copy, .size = sizeof(mrb_float), .amount = 0};
            return serialized_float;
        }

        if (type == MRB_TT_SYMBOL) {
            const char *string = API->mrb_sym_name(mrb, API->mrb_obj_to_sym(mrb, data));
            //printf("%s\n", string);
            size_t str_len = strlen(string);
            const char *string_dup = strdup(string);
            serialized_data_t serialized_string = {.type = MRB_TT_SYMBOL, .data = (void *) string_dup, .size = (int) (
                    str_len + 1), .amount = 0};
            return serialized_string;
        }

        if (type == MRB_TT_HASH) {
            mrb_value keys = API->mrb_hash_keys(mrb, data);
            mrb_value key = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, keys));
            std::vector<serialized_hash_t> data_vector;
            int data_size = 0;
            int hash_size = 0;
            while (cext_is_symbol(mrb, key)) {
                const char *s_key = strdup(API->mrb_sym_name(mrb, API->mrb_obj_to_sym(mrb, key)));
                mrb_value content = API->mrb_hash_get(mrb, data, key);
                serialized_data_t serialized_data = serialize_data(mrb, content);
                serialized_hash_t serialized_hash_entry = {.key = s_key, .data = serialized_data};
                printf("key: %s\n", s_key);
                //data_size += serialized_hash.size;
                hash_size++;
                data_size += serialized_data.size;
                data_vector.push_back(serialized_hash_entry);
                key = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, keys));
            }
            void *raw_data = data_vector.data();
            void *raw_data_copy = MALLOC_CYCLE(hash_size * sizeof(serialized_hash_t));
            memcpy(raw_data_copy, raw_data, hash_size * sizeof(serialized_hash_t));
            serialized_data_t serialized_hash = {.type = MRB_TT_HASH, .data = raw_data_copy, .size = data_size, .amount = hash_size};
            return serialized_hash;
        }

        if (type == MRB_TT_ARRAY) {
            std::vector<serialized_data_t> data_vector;
            mrb_value object = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, data));
            int data_size = 0;
            int array_size = 0;
            while (cext_is_valid_type(mrb, object)) { // true and false currently not supported
                serialized_data_t serialized_data = serialize_data(mrb, object);
                array_size++;
                data_size += serialized_data.size;
                data_vector.push_back(serialized_data);
                object = API->mrb_ary_shift(mrb, API->mrb_ensure_array_type(mrb, data));
            }
            void *raw_data = data_vector.data();
            void *raw_data_copy = MALLOC_CYCLE(array_size * sizeof(serialized_data_t));
            memcpy(raw_data_copy, raw_data, array_size * sizeof(serialized_data_t));
            serialized_data_t serialized_array = {.type = MRB_TT_ARRAY, .data = raw_data_copy, .size = data_size, .amount = array_size};
            return serialized_array;
        }

        serialized_data_t serialized_undef = {.type = MRB_TT_UNDEF, .data = (void *) nullptr, .size = 0};
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
            printf("%s\n", (char *) data.data);
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, (void *) &data.size, sizeof(int));
            position += sizeof(int);
            memcpy(buffer + position, data.data, data.size);
            position += data.size;
        }
        if (data.type == MRB_TT_INTEGER) {
            printf("%lld\n", *(mrb_int *) data.data);
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, data.data, sizeof(mrb_int));
            position += sizeof(mrb_int);
        }
        if (data.type == MRB_TT_FALSE || data.type == MRB_TT_TRUE) {
            if (data.type == MRB_TT_TRUE) {
                printf("True\n");
            } else {
                printf("False\n");
            }
            buffer[position] = c_type;
            ++position;
        }
        if (data.type == MRB_TT_FLOAT) {
            printf("%f\n", *((mrb_float *) data.data));
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, data.data, sizeof(mrb_float));
            position += sizeof(mrb_float);
        }
        if (data.type == MRB_TT_HASH) {
            printf("hash: ");
            serialized_data_t serialized_data = *((serialized_data_t *) data.data);
            //int type = hash_data.type;
            //printf("type: %i\n", type);
            //deserialize_data(hash_data);
            int data_size = data.size;
            printf("amount: %i\n", data.amount);
            printf("size: %i\n", data.size);
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, (void *) &data.size, sizeof(int));
            position += sizeof(int);
            memcpy(buffer + position, (void *) &data.amount, sizeof(int));
            position += sizeof(int);
            serialized_hash_t *hash_entries = (serialized_hash_t *) data.data;
            for (int i = 0; i < data.amount; ++i) {
                serialized_hash_t hash_entry = hash_entries[i];
                const char *key = hash_entry.key;
                printf("key: %s\n", key);

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
            printf("array: ");
            serialized_data_t serialized_data = *((serialized_data_t *) data.data);
            //int type = hash_data.type;
            //printf("type: %i\n", type);
            //deserialize_data(hash_data);
            int data_size = data.size;
            printf("amount: %i\n", data.amount);
            printf("size: %i\n", data.size);
            buffer[position] = c_type;
            ++position;
            memcpy(buffer + position, (void *) &data.size, sizeof(int));
            position += sizeof(int);
            memcpy(buffer + position, (void *) &data.amount, sizeof(int));
            position += sizeof(int);
            serialized_data_t *array_entries = (serialized_data_t *) data.data;
            for (int i = 0; i < data.amount; ++i) {
                serialized_data_t array_entry = array_entries[i];
                position = serialize_data_to_buffer(buffer, size, position, array_entry);
            }
        }
        return position;
    }
}