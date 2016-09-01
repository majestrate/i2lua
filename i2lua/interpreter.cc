#include "interpreter.hpp"
#include "i2lua.hpp"
#include "signal.hpp"
#include "log.hpp"
#include "i2pd/ClientContext.h"
#include "i2pd/Crypto.h"
#include "i2pd/Tunnel.h"
#include "i2pd/Transports.h"
#include "i2pd/NetDb.h"

#include <array>
#include <fstream>
#include <iostream>

namespace i2p {
  namespace lua {

    I2Lua::I2Lua() : log{std::cerr} {
      L = luaL_newstate();
      luaL_openlibs(L);
      luaL_newlib(L, i2p::lua::funcs);
      lua_setglobal(L, "i2p");
      signal(SIGINT, handle_sigint);
      lua_pushlightuserdata(L, &log);
      lua_pushcclosure(L, writeStacktrace, 1);
      errorfunc = lua_gettop(L);
    }
    
    void I2Lua::Stop() {
      i2p::client::context.Stop();
      i2p::tunnel::tunnels.Stop();
      i2p::transport::transports.Stop();
      i2p::data::netdb.Stop();
      i2p::crypto::TerminateCrypto();
    }

    bool I2Lua::LoadFile(const std::string & filename, std::ostream & out) {
      std::ifstream f(filename);
      if(!f.is_open()) {
        out << "Could not open " << filename << std::endl;
        return false;
      }
        // get filesize
      f.seekg(0, std::ios::end);
      const size_t filesize = f.tellg();
      f.seekg(0, std::ios::beg);
      char chunk[filesize];      
      // read file into memory
      f.read(chunk, filesize);
      
      auto result = luaL_loadbuffer(L, chunk, filesize, filename.c_str());
      // are we all good ?
      if(result == LUA_OK) return true; // yeh return true
      // we have an error
      out << "error in " << filename << std::endl;
      out << luaL_checkstring(L, -1) << std::endl;
      lua_pop(L, 1);
      return false;
    }

    int I2Lua::Mainloop() {
      auto result = lua_pcall(L, 0, LUA_MULTRET, errorfunc);
      if(result != LUA_OK) {
        // error happened
        log.out << luaL_checkstring(L, -1) << std::endl;
      }
    }
    
    I2Lua::~I2Lua() {
      Stop(); // this will throw if something bad happens
      lua_close(L);
    }
    
  }
}
