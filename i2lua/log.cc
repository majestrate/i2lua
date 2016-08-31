#include "log.hpp"
#include "i2pd/Log.h"

namespace i2p
{
  namespace lua
  {
    void printStacktrace(lua_State* L) {
      lua_Debug debug;
      int level = 0;
      LogPrint(eLogError, "---- begin i2lua stacktrace ----");
      int last;
      do {
        last = lua_getstack(L, level++, &debug);
        lua_getinfo(L, ">n", &debug);
        lua_getinfo(L, ">t", &debug);
        LogPrint(eLogError, "-- line ", debug.currentline);
      } while(last != 0);
      LogPrint(eLogError, "----- end i2lua stacktrace -----");
    }
  }
}
