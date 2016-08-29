#include "i2lua.hpp"
#include <signal.h>
#include <string>

#include "i2pd/Tunnel.h"
#include "i2pd/Transports.h"
#include "i2pd/NetDb.h"
#include "signal.hpp"

// from https://stackoverflow.com/questions/12256455/print-stacktrace-from-c-code-with-embedded-lua

//
// entry point for luajit i2p router
//
int main(int argc, char * argv[]) {
  int exitcode = 0;
  if (argc == 2) {
    lua_State* l = luaL_newstate();
    luaL_openlibs(l);
    luaL_newlib(l, i2p::lua::funcs);
    lua_setglobal(l, "i2p");
    signal(SIGINT, i2p::lua::handle_sigint);
    if(luaL_dofile(l, argv[1])) {
      lua_State* l2 = luaL_newstate();
      luaL_traceback(l2, l, "i2lua runtime error", 0);
      std::cout << luaL_checkstring(l2, -1) << std::endl;
      lua_close(l2);
      exitcode = 2;
    }
    try {
      i2p::tunnel::tunnels.Stop();
      i2p::transport::transports.Stop();
      i2p::data::netdb.Stop();
    } catch ( std::exception & ex ) {
      std::cout << "exception while ending router: " << ex.what() << std::endl;
    }
    lua_close(l);
  } else {
    std::cout << "usage: " << argv[0] << " runtime.lua" << std::endl;
    exitcode = 1;
  }
  return exitcode;
}
