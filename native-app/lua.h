#ifndef lua_State
#define lua_State mrb_state
#endif

#ifndef LUA_API
#define LUA_API static
#endif

#ifndef LUALIB_API
#define LUALIB_API LUA_API
#endif

// https://www.lua.org/source/5.3/luaconf.h.html
#ifndef LUA_IDSIZE
#define LUA_IDSIZE 60
#endif

#ifndef DR_SOCKET_LUA_H
#define DR_SOCKET_LUA_H

#include <dragonruby.h>

// https://www.lua.org/source/5.4/lauxlib.c.html#luaL_where
LUALIB_API void luaL_where (lua_State *L, int level);

/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
LUALIB_API mrb_value luaL_error (lua_State *L, const char *fmt, ...);

#endif