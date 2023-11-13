#pragma once
#ifndef DR_SOCKET_PRINT_H
#define DR_SOCKET_PRINT_H

#include "api.h"
#include <mruby.h>

namespace lyniat::socket::print {

    typedef enum console_output_t {
        PRINT_LOG,
        PRINT_WARNING,
        PRINT_ERROR
    } console_output_t;

    template<typename... T>
    mrb_value print(mrb_state *state, console_output_t type, const char *text, T &&... args) {
        if(type == PRINT_ERROR){
            API->mrb_funcall(state, mrb_nil_value(), "raise", 1, API->mrb_str_new_cstr(state, text));
            return mrb_nil_value();
        }
        API->mrb_funcall(state, mrb_nil_value(), "puts", 1, API->mrb_str_new_cstr(state, text));
        return mrb_nil_value();
    }

    template<typename... T>
    mrb_value print(mrb_state *state, const char *text, T &&... args) {
        return print(state, PRINT_LOG, text, args...);
    }
}
#endif