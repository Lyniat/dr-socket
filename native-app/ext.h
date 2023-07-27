#ifndef DR_SOCKET_EXT_H
#define DR_SOCKET_EXT_H

#include <dragonruby.h>

void ruby_print(mrb_state *state, const char *text);
void ruby_print_error(mrb_state *state, const char *text);

#endif