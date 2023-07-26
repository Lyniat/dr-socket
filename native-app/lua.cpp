#include "lua.h"
#include <dragonruby.h>
#include "ext.h"
#include "api.h"
#include "help.h"

// https://www.lua.org/source/5.4/lauxlib.c.html#luaL_where
void luaL_where (lua_State *L, int level) {
    drb_api->mrb_print_backtrace(L);
}

/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
mrb_value printf_error (mrb_state *state, const char *fmt, ...) {
    void* buffer = malloc(1024 * 1024); //TODO: do not hardcode this!
    auto alt_stdout = fmemopen(buffer, 1024 * 1024, "a"); //TODO: do not hardcode this!

    va_list arg;
    int done;
    va_start (arg, fmt);
    done = vfprintf (alt_stdout, fmt, arg);
    va_end (arg);

    fprintf(alt_stdout, "\n");

    fclose(alt_stdout);
    ruby_print_error(state, (char*)buffer);
    free(buffer);
    //return done;
    return mrb_nil_value();
}