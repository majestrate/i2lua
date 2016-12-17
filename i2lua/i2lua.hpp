#ifndef SRC_I2LUA_I2LUA_HPP
#define SRC_I2LUA_I2LUA_HPP
#include <lua.hpp>
#include "destination.hpp"
#include "netdb.hpp"
#include "router.hpp"
#include "profile.hpp"
#include "stream.hpp"

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
      {"WaitDestination", l_WaitDestination},
      {"StopDestination", l_StopDestination},
      {"GetDestinationAddress", l_DestinationGetB32},

      {"OpenStream", l_StreamOpen},
      {"CloseStream", l_StreamClose},
      {"ReadStream", l_StreamRead},
      {"WriteStream", l_StreamWrite},

      {"GetRouterProfile", l_GetRouterProfile},
      {"BanRouterByHash", l_BanRouterProfile},
      {"UnbanRouterByHash", l_UnbanRouterProfile},
      {0, 0}
    };
  }
}

#endif
