#include "i2lua.hpp"

#include <signal.h>
#include <string>

#include "i2pd/Tunnel.h"
#include "i2pd/Transports.h"
#include "i2pd/NetDb.h"
#include "signal.hpp"

//
// entry point for luajit i2p router
//
int main(int argc, char * argv[]) {

  if (argc == 2) {
    lua_State* l = luaL_newstate();
    luaL_openlibs(l);
    luaL_register(l, "i2lua", i2p::lua::funcs);
    int err = luaL_loadfile(l, argv[1]);
    if (err == LUA_ERRSYNTAX) {
      std::cout << "invalid syntax in " << argv[1];
    } else if ( err == LUA_ERRFILE) {
      std::cout << "failed to open " << argv[1];
    } else if ( err == LUA_ERRMEM) {
      std::cout << "out of memory when processing " << argv[1];
    } else if ( err ) {
      std::cout << "error " << err << " while processing " << argv[1];
    } else {
      signal(SIGINT, i2p::lua::handle_sigint);
      err = lua_pcall(l, 0, LUA_MULTRET, 0);
      if ( err ) {
        if ( err == LUA_ERRRUN ) {
          std::cout << "runtime error while executing " << argv[1] << std::endl;
          const char * msg  = lua_tostring(l, lua_gettop(l));
          std::cout << msg << std::endl;
        } else if ( err == LUA_ERRMEM ) {
          std::cout << "out of memory while executing " << argv[1];
        } else if ( err == LUA_ERRERR ) {
          std::cout << "error handler died while executing " << argv[1];
        } else {
          std::cout << "error " << err << " while executing " << argv[1];
        }
        
      }
    }
    std::cout << std::endl;
    try {
      i2p::tunnel::tunnels.Stop();
      i2p::transport::transports.Stop();
      i2p::data::netdb.Stop();
    } catch ( std::exception & ex ) {
      std::cout << "exception while ending router: " << ex.what() << std::endl;
    }
    lua_close(l);
    return err;
  } else {
    std::cout << "usage: " << argv[0] << " runtime.lua" << std::endl;
    return 1;
  }
}
