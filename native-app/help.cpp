#include "api.h"
#include "help.h"

mrb_int cext_to_int(mrb_state *mrb, mrb_value value){
    return mrb_integer_func(API->mrb_to_int(mrb, value));
}

mrb_float cext_to_float(mrb_state *mrb, mrb_value value){
    return API->mrb_to_flo(mrb, value);
}

const char* cext_to_string(mrb_state *mrb, mrb_value value){
    return API->mrb_string_cstr(mrb, value);
}

mrb_sym cext_sym(mrb_state *mrb, const char* str){
    return API->mrb_intern_check_cstr(mrb, str);
}

mrb_value cext_key(mrb_state *mrb, const char* str){
    //return mrb_symbol_value(cext_sym(mrb, str));
    return API->mrb_str_new_cstr(mrb, str);
}

mrb_value cext_key_sym(mrb_state *mrb, const char* str){
    return mrb_symbol_value(cext_sym(mrb, str));
}

mrb_value cext_hash_get(mrb_state *mrb, mrb_value hash, const char* key){
    /*
    auto result_with_str = API->mrb_hash_get(mrb, hash, cext_key(mrb, key));
    if(result_with_str.w == 0){
        return API->mrb_hash_get(mrb, hash, cext_key_sym(mrb, key));
    }
    return result_with_str;
     */
    return API->mrb_hash_get(mrb, hash, cext_key_sym(mrb, key));
}

mrb_int cext_hash_get_int(mrb_state *mrb, mrb_value hash, const char* key){
    return cext_to_int(mrb, API->mrb_hash_get(mrb, hash, cext_key(mrb, key)));
}

const char* cext_hash_get_string(mrb_state *mrb, mrb_value hash, const char* key){
    return cext_to_string(mrb, API->mrb_hash_get(mrb, hash, cext_key(mrb, key)));
}

mrb_sym cext_hash_get_sym(mrb_state *mrb, mrb_value hash, const char* key){
    return API->mrb_obj_to_sym(mrb, cext_hash_get(mrb, hash, key));
}

mrb_value cext_hash_get_save_hash(mrb_state *mrb, mrb_value hash, const char* key){
    auto h = cext_hash_get(mrb, hash, key);
    if((cext_is_hash(mrb, h))){
        return h;
    }
    return API->mrb_hash_new(mrb);
}

void cext_hash_set(mrb_state *mrb, mrb_value hash, const char* key, mrb_value val){
    //API->mrb_hash_set(mrb, hash, API->mrb_str_new_cstr(mrb, key), val);
    API->mrb_hash_set(mrb, hash, cext_key_sym(mrb, key), val);
    //API->mrb_hash_set(mrb, hash, API->mrb_(mrb, API->mrb_str_new_cstr(mrb, key)), val);
}

bool cext_is_string(mrb_state *mrb, mrb_value value){
    return mrb_type(value) == MRB_TT_STRING;
}

bool cext_is_symbol(mrb_state *mrb, mrb_value value){
    return mrb_type(value) == MRB_TT_SYMBOL;
}

bool cext_is_int(mrb_state *mrb, mrb_value value){
    return mrb_type(value) == MRB_TT_INTEGER;
}

bool cext_is_hash(mrb_state *mrb, mrb_value value){
    return mrb_type(value) == MRB_TT_HASH;
}

bool cext_is_array(mrb_state *mrb, mrb_value value){
    return mrb_type(value) == MRB_TT_ARRAY;
}

bool cext_is_undef(mrb_state *mrb, mrb_value value){
    return mrb_type(value) == MRB_TT_UNDEF;
}

mrb_int cext_hash_get_int_default(mrb_state *mrb, mrb_value hash, const char* key, mrb_int def){
    auto value = cext_hash_get(mrb, hash, key);
    if(cext_is_int(mrb, value)){
        return cext_to_int(mrb, value);
    }
    return def;
}

const char* cext_hash_get_string_default(mrb_state *mrb, mrb_value hash, const char* key, const char* def){
    auto value = cext_hash_get(mrb, hash, key);
    if(cext_is_string(mrb, value)){
        return cext_to_string(mrb, value);
    }
    return def;
}

mrb_sym cext_hash_get_sym_default(mrb_state *mrb, mrb_value hash, const char* key, mrb_sym def){
    auto value = cext_hash_get(mrb, hash, key);
    if(cext_is_symbol(mrb, value)){
        return API->mrb_obj_to_sym(mrb, value);
    }
    return def;
}