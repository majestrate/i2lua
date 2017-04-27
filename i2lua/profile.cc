#include "profile.hpp"
#include "libi2pd/Profiling.h"
#include "libi2pd/Identity.h"
#include <memory>

namespace i2p
{
namespace lua
{
  // push a router profile to the top of the stack for L
  static void pushRouterProfile(lua_State* L, const i2p::data::IdentHash & ident)
  {
    auto profile = i2p::data::GetRouterProfile(ident);
    if(profile == nullptr) {
      lua_pushnil(L);
      return;
    }
    lua_newtable(L);
    int table = lua_gettop(L);
		/*
    lua_pushinteger(L, profile->GetTunnelsAgreed());
    lua_setfield(L, table, "agreed");
    lua_pushinteger(L, profile->GetTunnelsDeclined());
    lua_setfield(L, table, "declined");
    lua_pushinteger(L, profile->GetTunnelsTimeout());
    lua_setfield(L, table, "timedout");
    lua_pushinteger(L, profile->GetTaken());
    lua_setfield(L, table, "taken");
    lua_pushinteger(L, profile->GetRejected());
    lua_setfield(L, table, "rejected");
    lua_pushboolean(L, profile->IsBanned());
    lua_setfield(L, table, "banned");
		*/
    lua_pushboolean(L, profile->IsBad());
    lua_setfield(L, table, "bad");
    lua_pushstring(L, ident.ToBase64().c_str());
    lua_setfield(L, table, "ident");
  }

  int l_GetRouterProfile(lua_State* L)
  {
    int top = lua_gettop(L);
    if (top == 1) {
      auto str = luaL_checkstring(L, 1);
      if (str == nullptr) {
        lua_pushnil(L);
      } else {
        std::string s(str);
        i2p::data::IdentHash ident;
        ident.FromBase64(s);
        pushRouterProfile(L, ident);
      }
    } else {
      lua_pushnil(L);
    }
    return 1;
  }

  int l_BanRouterProfile(lua_State* L)
  {
    int top = lua_gettop(L);
    if (top >= 1) {
      auto str = luaL_checkstring(L, 1);
      if(str) {
        std::string s(str);
        i2p::data::IdentHash ident;
        ident.FromBase64(s);
        auto profile = i2p::data::GetRouterProfile(ident);
        if(profile) {
          if (top == 2) {
            str = luaL_checkstring(L, 2);
            if (str) {
              std::string s(str);
              // profile->BanWithReason(s);
            } else {
              // profile->Ban();
            }
          } else {
            // profile->Ban();
          }
          profile->Save(ident);
        }
      }
    }
  }

  int l_UnbanRouterProfile(lua_State* L)
  {
    bool saved = false;
		/*
    int top = lua_gettop(L);
    if (top == 1) {
      auto str = luaL_checkstring(L, 1);
      if (str) {
        std::string s(str);
        i2p::data::IdentHash ident;
        ident.FromBase64(s);
        auto profile = i2p::data::GetRouterProfile(ident);
        if(profile) {
          profile->Unban();
          saved = profile->Save();
        }
      }
    }
		*/
    lua_pushboolean(L, saved);
    return 1;
  }
}
}
