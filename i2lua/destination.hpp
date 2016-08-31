#ifndef SRC_CORE_LUA_DESTINATION_HPP_
#define SRC_CORE_LUA_DESTINATION_HPP_
#include <lua.hpp>
#include <memory>
#include "i2pd/Destination.h"
#include "i2pd/Identity.h"
#include "i2pd/TunnelPool.h"

namespace i2p
{
  namespace lua
  {    
    struct LuaTunnelPeerSelector : public i2p::tunnel::ITunnelPeerSelector
    {
      
      
      LuaTunnelPeerSelector(lua_State* L, int callback);
      ~LuaTunnelPeerSelector();
      
      virtual bool SelectPeers(TunnelPath & peer, int hops, bool isInbound);
       
      lua_State* L; // called in destination thread
      int callback;
      int thread;
      std::mutex LMutex;
    };

    /**
       
     */
    struct TunnelPathBuilder {
      typedef std::function<void(const i2p::data::IdentHash &)> HopVistitor;
      /* push a hop into hops */
      void PushHop(const std::string & ident);
      /** visit each hop sequentially */
      void Visit(HopVistitor v);
      /** return how many hops we have currently */
      size_t GetHops();
      // idents of hops to use in order of build
      std::vector<i2p::data::IdentHash> hops;
      std::mutex hopsMutex;
    };
      
    // wrapper for Client Destination
    struct Destination {
      typedef i2p::data::PrivateKeys Keys;
      Destination(const Keys & keys);
      std::shared_ptr<i2p::client::ClientDestination> Dest;
      std::promise<void> done;
      void Start();
      void Run();
      /** inform promise when destination is stopped, this function returns immediately */
      void Stop(std::promise<void> & p);
    };

    /** 
        create a new destination, does not start it
        f(keyfile, callback)
        callback(destination)
     */
    int l_CreateDestination(lua_State* L);

    /** 
        set tunnel build peer selector
        f(destination, callback)
        callback(pushhop, hops, isInbound) for every tunnel build
        callback must return true if selected enough peers otherwise must return false
     */
    int l_SetDestinationPeerSelector(lua_State* L);
    /**
        start destination and block until done 
        f(destination)
    */
    int l_RunDestination(lua_State* L);
    /**
       terminate destination
       desination is unusable after this call
       f(destination)
     */
    int l_DestroyDestination(lua_State* L);

    /**
       push the base64'd identhash for a hop into a tunnelPath
       f(tunnelPath, hop)
     */
    int l_TunnelPathPushHop(lua_State* L);
  }
}


#endif
