#ifndef SRC_I2LUA_LOG_HPP
#define SRC_I2LUA_LOG_HPP
#include <lua.hpp>
#include <iostream>
namespace i2p
{
  namespace lua
  {

    struct LogPrinter {
      std::ostream & out;
    };
    
    void generateStacktrace(lua_State* L, std::ostream & out);
    int writeStacktrace(lua_State* L);
  }
}
#endif
