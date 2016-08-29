#include "router.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include "i2pd/Config.h"
#include "i2pd/Tunnel.h"
#include "i2pd/Transports.h"
#include "i2pd/NetDb.h"



namespace i2p
{
  namespace lua
  {
    std::promise<void> complete;

    int l_InitRouter(lua_State* L) {
  
      char * err_msg = nullptr;
      // try initializing config
      try {
        i2p::config::Init();
      } catch (std::exception & ex) {
        err_msg = (char*) ex.what();
      }
      if (err_msg) {
        return luaL_error(L, "error initializing config: %s", err_msg);
      }
      // try setting config options
      try {
        
      } catch ( std::exception & ex) {
        err_msg = (char*) ex.what();
      }
      if (err_msg) {
        return luaL_error(L, "error setting config options: %s", err_msg);
      }
      lua_pushnil(L);
      return 1;
    }
    
    int l_StartRouter(lua_State* L) {
      char * err = nullptr;
      try {
        i2p::data::netdb.Start();
        i2p::transport::transports.Start();
        i2p::tunnel::tunnels.Start();
      } catch( std::runtime_error & ex ) {
        return luaL_error(L, "error while initializing: %s", ex.what());
      }
      lua_pushnil(L);
      return 1;
    }

    int l_StopRouter(lua_State* L) {
      char * msg = nullptr;
      try {
        complete.set_value();
      } catch ( std::future_error & err) {
        return luaL_error(L, "error stopping: %s", err.what());
      }
      lua_pushnil(L);
      return 1;
    }

    int l_WaitUntilDone(lua_State* L) {
      complete.get_future().wait();
      lua_pushnil(L);
      return 1;
    }
    
    int l_Sleep(lua_State* L) {
      int n = lua_gettop(L);
      if ( n != 1 ) {
        return luaL_error(L, "invalid number of arguments: %d", n);
      }
      if (!lua_isnumber(L, 1)) {
        return luaL_argerror(L, 1, "not an integer");
      }
      n = lua_tointeger(L, 1);
      std::this_thread::sleep_for(std::chrono::milliseconds(n));
      lua_pushnil(L);
      return 1;
    }

    void handle_sigint(int sig) {
      (void)sig;
      try {
        complete.set_value();
      } catch( std::future_error & ex) {
        (void) ex;
      }
    }
  }
}

