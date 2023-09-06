#pragma once
#ifndef DR_SOCKET_TEST_H
#define DR_SOCKET_TEST_H

#include "ruby.h"

void register_test_functions(mrb_state* state, RClass* module);

#endif