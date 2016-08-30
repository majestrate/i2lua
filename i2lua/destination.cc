#include "destination.hpp"
#include "i2pd/Destination.h"
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
        auto k = i2p::data::PrivateKeys::CreateRandomKeys(i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519);
        std::ofstream s(filepath, std::ofstream::binary | std::ofstream::out);
        size_t sz = k.GetFullLen();
        uint8_t * buf = new uint8_t[sz];
        keys.ToBuffer(buf, sz);
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
      auto & service = Dest->GetService();
      auto pr = &p;
      service.post([&, pr] () {
          Dest->Stop();
          pr->set_value();
      });
    }

    void Destination::Run() {
      Dest->Start();
      done.get_future().wait();
    }
    
    LuaTunnelPeerSelector::LuaTunnelPeerSelector(lua_State *state, int cb) {
      L = lua_newthread(state);
      lua_xmove(state, L, cb);
      callback = lua_gettop(L);
    }

    LuaTunnelPeerSelector::~LuaTunnelPeerSelector() {}

    bool LuaTunnelPeerSelector::SelectPeers(TunnelPath & peer, int hops, bool isInbound) {
      auto builder = std::make_shared<TunnelPathBuilder>();
      // call callback
      lua_pushvalue(L, callback);
      lua_pushlightuserdata(L, builder.get());
      lua_pushinteger(L, hops);
      lua_pushboolean(L, isInbound);
      auto result = lua_pcall(L, 3, 1, 0);
      if(result == LUA_OK) {
        // success
        if(lua_isboolean(L, lua_gettop(L))) {
          // for each hop ...
          for (const auto & ident : builder->hops) {
            // lookup RouterInfo
            auto ri = i2p::data::netdb.FindRouter(ident);
            if (ri == nullptr) return false;
            // push back the router identity
            auto i = ri->GetRouterIdentity();
            if(i) peer.push_back(i);
          }
          return true;
        } else {
          // failure to select
          return false;
        }
      } else {
        return false;
      }
    }

    int l_SetDestinationPeerSelector(lua_State* L) {
      int n = lua_gettop(L);
      if(n == 2 && lua_islightuserdata(L, 1) && lua_isfunction(L, 2)) {
        // valid args
        auto dest = getDestination(L, 1);
        // reset previous tunnel builder
        if(dest->TunnelBuilder)
          dest->TunnelBuilder = nullptr;
        // assign new tunnel builder
        dest->TunnelBuilder = std::make_shared<LuaTunnelPeerSelector>(L, 2);
        // return nil
        lua_pushnil(L);
        return 1;
      } else {
        // invalid args
        return luaL_error(L, "bad arguments, reqires destination and function, got %d arguments", n);
      }
    }
  }
}
