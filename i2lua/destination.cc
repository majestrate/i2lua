#include "destination.hpp"
#include "log.hpp"
#include "i2pd/Destination.h"
#include "i2pd/Log.h"
#include <fstream>

namespace i2p
{
  namespace lua
  {
    // does not pop stack
    static Destination * getDestination(lua_State* L, int idx) {
      return (Destination *) lua_touserdata(L, idx);
    }
    
    int l_CreateDestination(lua_State * L) {
      int n = lua_gettop(L);
      if (n != 2) {
        return luaL_error(L, "wrong number of arguments: %d", n);
      }
      // arg 1 is path to keyfile
      const char * keyfile = luaL_checkstring(L, 1);
      if(keyfile == nullptr) {
        return luaL_error(L, "argument 1 must be a string");
      }
      if ( ! lua_isfunction(L, 2) ) {
        return luaL_error(L, "argument 2 must be a function");
      }

      i2p::data::PrivateKeys keys;
      
      std::string filepath(keyfile);
      if (!i2p::fs::Exists(filepath)) {
        // no keys, generate new ones and save them
        auto k = i2p::data::PrivateKeys::CreateRandomKeys();
        std::ofstream s(filepath, std::ofstream::binary | std::ofstream::out);
        size_t sz = k.GetFullLen();
        uint8_t * buf = new uint8_t[sz];
        k.ToBuffer(buf, sz);
        s.write((char *)buf, sz);
        delete [] buf;
      }
      // read private keys
      std::ifstream f(filepath, std::ifstream::binary);
      
      if (!f.is_open())
        return luaL_error(L, "failed to open key file: %s", keyfile);
      f.seekg(0, std::ios::end);
      size_t s = f.tellg();
      f.seekg(0, std::ios::beg);
      uint8_t * buf = new uint8_t[s];
      if(!keys.FromBuffer(buf, s)) {
         delete [] buf;
         return luaL_error(L, "invalid private key file: %s", keyfile);
      }
      delete [] buf;
      // we haz our private keys now

      // create destination
      auto destination = new Destination(keys);
      // call callback with our destination
      lua_pushlightuserdata(L, destination);
      return lua_pcall(L, 1, 0, 0);
    }

    int l_DestroyDestination(lua_State* L) {
      int args = lua_gettop(L);
      if (args == 1 && lua_islightuserdata(L, 1)) {
        Destination * dest = getDestination(L, 1);
        std::promise<void> done;
        dest->Stop(done);
        done.get_future().wait();
        delete dest;
      }
      lua_pushnil(L);
      return 1;
    }

    int l_RunDestination(lua_State* L) {
      int args = lua_gettop(L);
      if (args == 1 && lua_islightuserdata(L, 1)) {
        auto dest = getDestination(L, 1);
        dest->Run();
      }
      lua_pushnil(L);
      return 1;
    }

    
    
    Destination::Destination(const Keys & k) {
      Dest = std::make_shared<i2p::client::ClientDestination>(k, true);
    }

    void Destination::Stop(std::promise<void> & p) {
      try { 
        done.set_value();
      } catch( std::future_error &) {}
      Dest->Stop();
      p.set_value();
    }

    void Destination::Run() {
      Dest->Start();
      done.get_future().wait();
    }
    
    LuaTunnelPeerSelector::LuaTunnelPeerSelector(lua_State *state, int cb) {
      L = state;
      lua_pushvalue(L, cb);
      callback = lua_gettop(L);
    }

    LuaTunnelPeerSelector::~LuaTunnelPeerSelector() {
    }

    static int l_PushPath(lua_State * L)
    {
      int args = lua_gettop(L);
      if (args == 2 && lua_islightuserdata(L, 1) && lua_isstring(L, 2)) {
        // correct number of arguments
        TunnelPathBuilder * builder = (TunnelPathBuilder *) lua_touserdata(L, 1);
        std::string ident(luaL_checkstring(L, 2));
        builder->PushHop(ident);
      }
      lua_pushnil(L);
      return 1;
    }
    
    bool LuaTunnelPeerSelector::SelectPeers(TunnelPath & peers, int hops, bool isInbound) {\
      std::lock_guard<std::mutex> lock(LMutex);
      TunnelPathBuilder builder;
      bool r = false;
      // call callback
      int top = lua_gettop(L);
      lua_pushvalue(L, thread);
      lua_pushvalue(L, callback);
      lua_pushlightuserdata(L, &builder);
      lua_pushcclosure(L, l_PushPath, 1);
      lua_pushinteger(L, hops);
      lua_pushboolean(L, isInbound);
      auto result = lua_pcall(L, 4, 1, 0);
      if(result == LUA_OK) {
        // success
        bool success = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if(success) {
          // successful select
          builder.Visit([&peers] (const i2p::data::IdentHash & ih) {
              auto ri = i2p::data::netdb.FindRouter(ih);
              if(!ri) return;
              auto ident = ri->GetRouterIdentity();
              if(ident) peers.push_back(ident);
          });
          r = peers.size() == builder.GetHops();
        }
      } else {
        // lua error
        printStacktrace(L);
      }
      return r;
    }

    int l_SetDestinationPeerSelector(lua_State* L) {
      int n = lua_gettop(L);
      if(n == 2 && lua_islightuserdata(L, 1) && lua_isfunction(L, 2)) {
        // valid args
        auto dest = getDestination(L, 1);
        // set builder
        auto builder = std::make_shared<LuaTunnelPeerSelector>(L, 2);
        dest->Dest->GetTunnelPool()->SetCustomPeerSelector(builder);
        // return nil
        lua_pushnil(L);
        return 1;
      } else {
        // invalid args
        return luaL_error(L, "bad arguments, reqires destination and function, got %d arguments", n);
      }
    }

    void TunnelPathBuilder::PushHop(const std::string & ident) {
      std::lock_guard<std::mutex> lock(hopsMutex);
      i2p::data::IdentHash ih;
      ih.FromBase64(ident);
      hops.push_back(ih);
    }

    void TunnelPathBuilder::Visit(HopVistitor v) {
      std::lock_guard<std::mutex> lock(hopsMutex);
      for (const auto & hop : hops)
        v(hop);
    }

    size_t TunnelPathBuilder::GetHops() {
      std::lock_guard<std::mutex> lock(hopsMutex);
      return hops.size();
    }
  }
}
