#include <memory.h>
#include <dragonruby.h>

#ifndef lua_State
#define lua_State mrb_state
#endif

// "\e]8;;file://" PATH "\e\\" FILE "\e]8;;\a"

#ifndef luaL_error
#ifdef DEBUG
#define luaL_error(state, fmt, ...) printf_error(state, "%s " fmt, DEBUG_FILENAME, ##__VA_ARGS__)
#else
#define luaL_error(state, fmt, ...) printf_error(state, fmt, ##__VA_ARGS__)
#endif
#endif

#ifndef DR_SOCKET_LUA_H
#define DR_SOCKET_LUA_H

// https://www.lua.org/source/5.4/lauxlib.c.html#luaL_where
void luaL_where (lua_State *L, int level);

/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
mrb_value printf_error (mrb_state *state, const char *fmt, ...);

#endif