#ifndef SRC_I2LUA_INTERPRETER_HPP
#define SRC_I2LUA_INTERPRETER_HPP
#include <lua.hpp>
#include <iostream>
#include <string>
#include "log.hpp"

namespace i2p
{
  namespace lua
  {
    // main i2lua interpreter
    class I2Lua
    {
    public:
      I2Lua();
      ~I2Lua();
      /** load file, prints error to stacktrace if syntax error */
      bool LoadFile(const std::string & filename, std::ostream & out);
      /** run lua mainloop, returns exit code */
      int Mainloop();
      /** stop i2p stuff */
      void Stop();
    private:
      lua_State* L;
      int errorfunc;
      LogPrinter log;
    };
  }
}

#endif
