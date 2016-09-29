#ifndef SRC_CORE_LUA_PROFILE_HPP
#define SRC_CORE_LUA_PROFILE_HPP
#include <lua.hpp>

namespace i2p
{
namespace lua
{

  int l_GetRouterProfile(lua_State* L);
  int l_BanRouterProfile(lua_State* L);
  int l_UnbanRouterProfile(lua_State* L);
}
}

#endif
