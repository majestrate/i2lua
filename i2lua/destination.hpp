#ifndef SRC_CORE_LUA_DESTINATION_HPP_
#define SRC_CORE_LUA_DESTINATION_HPP_
#include <lua.hpp>
#include <future>
#include <memory>
#include "i2pd/Destination.h"
#include "i2pd/Identity.h"
#include "i2pd/TunnelPool.h"

namespace i2p
{
  namespace lua
  {    

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
      typedef std::function<void(lua_State*)> ThreadAccessor;
      Destination(const Keys & keys);
      std::shared_ptr<i2p::client::ClientDestination> Dest;
      std::promise<void> done;
      std::condition_variable waiter;
      void Start();
      void Run();
      /** inform promise when destination is stopped, this function returns immediately */
      void Stop(std::promise<void>& p);
      /** wait until done using mutex*/
      void Wait(std::unique_lock<std::mutex> & l);
      
      lua_State* thread; // destination thread
      bool running;
    };

    struct LuaTunnelPeerSelector : public i2p::tunnel::ITunnelPeerSelector
    {      
      typedef std::tuple<TunnelPath, bool> SelectResult;
        
      
      LuaTunnelPeerSelector(Destination * d, int callback);
      ~LuaTunnelPeerSelector();
      
      virtual bool SelectPeers(TunnelPath & peer, int hops, bool isInbound);
       
      Destination * dest;
      int callback;
      std::mutex callmtx;
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
        wait for the destination to complete execution
        f(destination)
    */
    int l_WaitDestination(lua_State* L);
    /**
       stop the destination run
       desination is unusable after this call
       f(destination)
     */
    int l_StopDestination(lua_State* L);

    /** 
        explicit free of destination light user data
        only call after destination is stoped and completed execution
     */
    int l_DestroyDestination(lua_State* L);
    
    /**
       push the base64'd identhash for a hop into a tunnelPath
       f(tunnelPath, hop)
     */
    int l_TunnelPathPushHop(lua_State* L);

    /** 
        get destination b32 address
        f(destination) returns string
     */
    int l_DestinationGetB32(lua_State* L);
  }
}


#endif
