#pragma once
#ifndef DR_SOCKET_API_H
#define DR_SOCKET_API_H

#include "ruby.h"

#define API lyniat::socket::api::drb_api

namespace lyniat::socket::api {

    extern drb_api_t *drb_api;

    extern struct RClass *drb_gtk;
    extern struct RClass *drb_runtime;
    extern struct RClass *drb_console;

}

#endif