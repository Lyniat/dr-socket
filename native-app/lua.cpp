#include "lua.h"
#include "ruby.h"
#include "ext.h"
#include "api.h"
#include "help.h"

/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
mrb_value printf_error (mrb_state *state, const char *fmt, ...) {
    void* buffer = MALLOC(1024 * 1024); //TODO: do not hardcode this!
    auto alt_stdout = fmemopen(buffer, 1024 * 1024, "a"); //TODO: do not hardcode this!

    va_list arg;
    int done;
    va_start (arg, fmt);
    done = vfprintf (alt_stdout, fmt, arg);
    va_end (arg);

    fprintf(alt_stdout, "\n");

    fclose(alt_stdout);
    ruby_print_error(state, (char*)buffer);
    FREE(buffer);
    //return done;
    return mrb_nil_value();
}