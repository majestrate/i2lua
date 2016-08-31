#ifndef SRC_I2LUA_LOG_HPP
#define SRC_I2LUA_LOG_HPP
#include <lua.hpp>
namespace i2p
{
  namespace lua
  {
    void printStacktrace(lua_State* L);
  }
}
#endif
