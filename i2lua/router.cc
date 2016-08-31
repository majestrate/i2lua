#include "router.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include "i2pd/Config.h"
#include "i2pd/Tunnel.h"
#include "i2pd/Transports.h"
#include "i2pd/NetDb.h"
#include "i2pd/Log.h"


namespace i2p
{
  namespace lua
  {
    std::promise<void> complete;

    int l_InitRouter(lua_State* L) {
      char * errmsg = nullptr;
      int n = lua_gettop(L);
      if (n && lua_isstring(L, 1)) {
        std::string conf = luaL_checkstring(L, 1);
        // try initializing config
        try {
          i2p::config::Init();
          i2p::config::ParseConfig(conf);
          std::string datadir; i2p::config::GetOption("datadir", datadir);

          i2p::fs::DetectDataDir(datadir);
          i2p::fs::Init();
          i2p::config::Finalize();
        } catch (std::exception & ex) {
          errmsg = (char*) ex.what();
          std::cerr << ex.what() << std::endl;
        }
        if(errmsg) {
          return luaL_error(L, "error initializing config: %s",  errmsg);
        }
        lua_pushnil(L);
        return 1;
      }
      return luaL_error(L, "bad arguments: %d", n);
    }
    
    int l_StartRouter(lua_State* L) {
      char * msg = nullptr;
      try {
        i2p::log::Logger().Ready();
        i2p::crypto::InitCrypto(false);
        i2p::context.Init();
        i2p::data::netdb.Start();
        i2p::transport::transports.Start();
        i2p::tunnel::tunnels.Start();
      } catch( std::runtime_error & ex ) {
        msg = (char*) ex.what();
      }
      if(msg)
        return luaL_error(L, "error while starting: %s", msg);

      // call callback
      if(lua_gettop(L) > 0 && lua_isfunction(L, 1)) {
        lua_pushvalue(L, 1);
        lua_call(L, 0, 0);
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
        return luaL_argerror(L, 1, "");
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

