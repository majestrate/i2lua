#ifndef SRC_I2LUA_ROUTER_HPP
#define SRC_I2LUA_ROUTER_HPP
#include <lua.hpp>

namespace i2p
{
  namespace lua
  {
    // initialize the i2p router parameters
    int l_InitRouter(lua_State* L);
    // run the i2p router
    int l_StartRouter(lua_State* L);
    // sleep until router is done
    int l_WaitUntilDone(lua_State* L);
    // stop execution of router immediately
    int l_StopRouter(lua_State* L);
    // sleep for n milliseconds
    int l_Sleep(lua_State* L);    
  }
}


#endif
