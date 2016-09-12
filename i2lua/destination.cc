#include "destination.hpp"
#include "netdb.hpp"
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

      std::string filepath(keyfile);
      if (!i2p::fs::Exists(filepath)) {
        // no keys, generate new ones and save them
        auto k = i2p::data::PrivateKeys::CreateRandomKeys();
        std::ofstream s(filepath, std::ofstream::binary | std::ofstream::out);
        size_t sz = k.GetFullLen();
        uint8_t * buf = new uint8_t[sz];
        k.ToBuffer(buf, sz);
        s.write((char*)buf, sz);
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
      f.read((char*) buf, s);
      i2p::data::PrivateKeys keys;
      if(!keys.FromBuffer(buf, s)) {
         delete [] buf;
         return luaL_error(L, "invalid private key file: %s", keyfile);
      }
      delete [] buf;
      // we haz our private keys now

      // create destination
      auto destination = new Destination(keys);
      // start destination in another thread
      std::thread t(std::bind(&Destination::Run, destination));
      t.detach();
      // get error function
      lua_pushcfunction(L, i2p::lua::writeStacktrace);
      int err = lua_gettop(L);
      // call callback with our destination
      lua_pushvalue(L, 2);
      lua_pushlightuserdata(L, destination);
      int result = lua_pcall(L, 1, 1, err);
      // remove our error function
      lua_remove(L, err);
      if(result != LUA_OK) {
        // error
        std::string errmsg = luaL_checkstring(L, -1);
        lua_pop(L, 2); // i2p::lua::writeStacktrace and error message
        return luaL_error(L, "error calling destination callback:\n%s", errmsg.c_str());
      }
      // return result of pcall
      return 1;
    }

    int l_StopDestination(lua_State* L) {
      int args = lua_gettop(L);
      if (args == 1 && lua_islightuserdata(L, 1)) {
        Destination * dest = getDestination(L, 1);
        std::promise<void> done;
        dest->Stop(done);
        done.get_future().wait();
      }
      lua_pushnil(L);
      return 1;
    }

    int l_WaitDestination(lua_State* L) {
      int args = lua_gettop(L);
      if (args == 1 && lua_islightuserdata(L, 1)) {
        auto dest = getDestination(L, 1);
        std::mutex m;
        std::unique_lock<std::mutex> l(m);
        dest->Wait(l);
      }
      lua_pushnil(L);
      return 1;
    }

    int l_DestroyDestination(lua_State* L) {
      int top = lua_gettop(L);
      if (top == 1 && lua_islightuserdata(L, 1)) {
        Destination * dest = getDestination(L, 1);
        delete dest;
      }
      lua_pushnil(L);
      return 1;
    }
    
    
    Destination::Destination(const Keys & k) : thread(nullptr) {
      Dest = std::make_shared<i2p::client::ClientDestination>(k, true);
    }

    void Destination::Stop(std::promise<void> & p) {
      Dest->Stop();
      running = false;
      try { 
        done.set_value();
      } catch( std::future_error &) {}
      p.set_value();
    }

    void Destination::Wait(std::unique_lock<std::mutex> & l) {
      waiter.wait(l);
    }
    
    void Destination::Run() {
      Dest->Start();
      done.get_future().wait();
      waiter.notify_all();
    }

    LuaTunnelPeerSelector::LuaTunnelPeerSelector(Destination * d, int selectpeers, int build_success, int build_fail) :
      dest(d), select_peers_callback(selectpeers), build_success_callback(build_success), build_fail_callback(build_fail) {}

    LuaTunnelPeerSelector::~LuaTunnelPeerSelector() {}

    static int l_PushPath(lua_State * L)
    {
      int idx = lua_upvalueindex(1);
      int args = lua_gettop(L);
      if (args == 1 && lua_isstring(L, 1) && lua_islightuserdata(L, idx)) {
        // correct number of arguments
        TunnelPathBuilder * builder = (TunnelPathBuilder *) lua_touserdata(L, idx);
        std::string ident(luaL_checkstring(L, 1));
        builder->PushHop(ident);
        LogPrint(eLogDebug, "Lua: pushed hop ", ident);
      } else {
        LogPrint(eLogError, "Lua: bad arguments to l_PushPath top=", args, " idx=", idx);
      }
      lua_pushnil(L);
      return 1;
    }
    
    bool LuaTunnelPeerSelector::SelectPeers(TunnelPath & peers, int numhops, bool isInbound) {
      lua_State * L = dest->thread;
      bool r = false;
      TunnelPathBuilder builder;
      LogPrinter log{std::cout};
      LogPrint(eLogDebug, "Lua: select peers");
      // add log handler (1)
      lua_pushlightuserdata(L, &log);
      lua_pushcclosure(L, writeStacktrace, 1);
      int err = lua_gettop(L);
      // call callback (2)
      lua_pushvalue(L, select_peers_callback);
      // arg 1 (3)
      lua_pushlightuserdata(L, &builder);
      lua_pushcclosure(L, l_PushPath, 1);
      // arg 2 (4)
      lua_pushinteger(L, numhops);
      // arg 3 (5)
      lua_pushboolean(L, isInbound);
      // call
      auto res = lua_pcall(L, 3, 1, err);
      if(res == LUA_OK) {
        // success (2)
        LogPrint(eLogDebug, "Lua: select peers called okay");
        bool success = lua_toboolean(L, -1);
        lua_pop(L, 1); // pop return value
        if(success) {
          // successful select
          LogPrint(eLogDebug, "Lua: select peers success");
          for (const auto & hop : builder.hops) {
            auto ri = i2p::data::netdb.FindRouter(hop);
            if(!ri) continue;
            auto ident = ri->GetRouterIdentity();
            if(ident) peers.push_back(ident);
          }
          r = peers.size() == builder.GetHops();
        } else {
          LogPrint(eLogError, "Lua: failed to select peers");
          r = false;
        }
      } else {
        LogPrint(eLogError, "Lua: error in peer selection ", luaL_checkstring(L, -1));
        r = false;
      }
      lua_pop(L, 1);
      return r;
    }

    static void push_TunnelPath(lua_State* L, const std::vector<std::shared_ptr<const i2p::data::IdentityEx> > & peers)
    {
      auto numpeers = peers.size();
      lua_createtable(L, numpeers, 0);
      int table = lua_gettop(L);
      lua_Integer i = 1;
      for ( const auto & peer : peers)
      {
        if (peer)
        {
          auto ri = i2p::data::netdb.FindRouter(peer->GetIdentHash());
          if (ri == nullptr)
            lua_pushnil(L);
          else
            push_RouterInfo(L, ri);
        }
        else
          lua_pushnil(L);
        lua_rawseti(L, table, i);
        ++i;
      }
    }

    bool LuaTunnelPeerSelector::OnBuildResult(TunnelPath & peer, bool isInbound, i2p::tunnel::TunnelBuildResult result)
    {
      lua_State * L = dest->thread;
      LogPrint(eLogDebug, "Lua: OnBuildResult");
      // add log handler (1)
      LogPrinter log{std::cout};
      lua_pushlightuserdata(L, &log);
      lua_pushcclosure(L, writeStacktrace, 1);
      int err = lua_gettop(L);
      // call callback (2)
      if(result == i2p::tunnel::eBuildResultOkay)
        lua_pushvalue(L, build_success_callback);
      else
        lua_pushvalue(L, build_fail_callback);
      // arg 1 (3)
      push_TunnelPath(L, peer);
      // arg 2 (4)
      lua_pushboolean(L, isInbound);
      // call
      auto res = lua_pcall(L, 2, 1, err);
      if(res == LUA_OK) {
        // success (2)
        LogPrint(eLogDebug, "Lua: OnBuildResult called okay");
      } else {
        LogPrint(eLogError, "Lua: error in OnBuildResult ", luaL_checkstring(L, -1));
      }
      lua_pop(L, 1);
      return true;
    }
    
    int l_SetDestinationPeerSelector(lua_State* L) {
      int n = lua_gettop(L);
      if(n == 4 && lua_islightuserdata(L, 1) && lua_isfunction(L, 2) && lua_isfunction(L, 3) && lua_isfunction(L, 4)) {
        // valid args
        auto dest = getDestination(L, 1);
        // create new thread (3)
        dest->thread = lua_newthread(L);
        int thread = lua_gettop(L);
        // give select peers callback to new thread
        lua_pushvalue(L, 2); // (4)
        lua_xmove(L, dest->thread, 1); // (3)
        int selectPeers = lua_gettop(dest->thread);
        // give success callback to new thread
        lua_pushvalue(L, 3);
        lua_xmove(L, dest->thread, 1);
        int buildSuccess = lua_gettop(dest->thread);
        // give fail callback to new thread
        lua_pushvalue(L, 4);
        lua_xmove(L, dest->thread, 1);
        int buildFail = lua_gettop(dest->thread);
        
        // create builder
        auto builder = std::make_shared<LuaTunnelPeerSelector>(dest, selectPeers, buildSuccess, buildFail);
        // set it
        dest->Dest->GetTunnelPool()->SetCustomPeerSelector(builder);
        // return thread (should be on top of stack)
        return 1;
      } else {
        // invalid arguments
        lua_pushnil(L);
        return 1;
      }
    }

    void TunnelPathBuilder::PushHop(const std::string & ident) {
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

    int l_DestinationGetB32(lua_State* L) {
      int args = lua_gettop(L);
      if (args == 1 && lua_islightuserdata(L, 1)) {
        auto dest = getDestination(L, 1);
        auto str = dest->Dest->GetIdentHash().ToBase32();
        lua_pushstring(L, str.c_str());
      } else {
        lua_pushnil(L);
      }
      return 1;
    }
  }
}
