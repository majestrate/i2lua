#include "log.hpp"
#include "i2pd/Log.h"
#include <cassert>
#include <sstream>

namespace i2p
{
  namespace lua
  {
    // original from http://zeuxcg.org/2010/11/07/lua-callstack-with-c-debugger/
    void generateStacktrace(lua_State* L, std::ostream & out)
    {
      lua_Debug entry;
      int depth = 0; 
      while (lua_getstack(L, depth, &entry))  {
        int status = lua_getinfo(L, "Sln", &entry);
        assert(status);
        out << entry.short_src;
        out << "(" << entry.currentline;
        out << "): " << entry.name ? entry.name : "?";
        out << std::endl;
        depth++;
      }
    }

    int writeStacktrace(lua_State* L) {
      std::stringstream s;
      int top = lua_gettop(L);
      if (top == 2) {
        std::string msg = luaL_checkstring(L, 1);
        LogPrinter * log = (LogPrinter *) lua_touserdata(L, 1);
        log->out << "error: " << msg << std::endl;
        generateStacktrace(L, log->out);
        s << "stack dumped" << std::endl;
      } else if (top == 1) {
        s << luaL_checkstring(L, 1) << std::endl;
        generateStacktrace(L, s);
      } else {
        s << "unknown error wtflol" << std::endl;
      }
      lua_pushstring(L, s.str().c_str());
      return 1;
    }
  }
}
