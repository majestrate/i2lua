#ifndef SRC_I2LUA_I2LUA_HPP
#define SRC_I2LUA_I2LUA_HPP
#include <lua.hpp>
#include "destination.hpp"
#include "netdb.hpp"
#include "router.hpp"

namespace i2p
{
  namespace lua
  {
    luaL_Reg funcs[] = {
      {"Init", l_InitRouter},
      {"Start", l_StartRouter},
      {"Stop", l_StopRouter},
      {"Wait", l_WaitUntilDone},
      {"Sleep", l_Sleep},
      {"VisitRIByHash", l_VisitRIByHash},
      {"VisitRandomRIWithFilter", l_VisitRandomRIWithFilter},
      {"NewDestination", l_CreateDestination},
      {"DelDestination", l_DestroyDestination},
      {"DestinationSetPeerSelector", l_SetDestinationPeerSelector},
      {"RunDestination", l_RunDestination},
      {"GetDestinationAddress", l_DestinationGetB32},
      {0, 0}
    };
  }
}

#endif
