
#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


void luaL_traceback(lua_State* L, lua_State* L1, const char* msg, int level);

int lua_getfield(lua_State* L, int idx, const char* k);
void lua_setfield(lua_State* L, int idx, const char* k);
void lua_copy(lua_State* L, int fromidx, int toidx);



#ifdef __cplusplus
}
#endif