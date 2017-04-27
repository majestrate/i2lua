#include "log.hpp"
#include "libi2pd/Log.h"
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
      s << luaL_checkstring(L, 1) << std::endl;
      generateStacktrace(L, s);
      lua_pushstring(L, s.str().c_str());
      return 1;
    }
  }
}
