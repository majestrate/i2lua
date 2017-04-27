#include "log.hpp"
#include "netdb.hpp"

#include "libi2pd/NetDb.hpp"
#include "libi2pd/Log.h"

namespace i2p
{
namespace lua
{
  const std::shared_ptr<i2p::data::RouterInfo> FindRouterByHash(const std::string & hash) {
    i2p::data::IdentHash rh;
    rh.FromBase64(hash);
    return i2p::data::netdb.FindRouter(rh);
  }

  size_t VisitRandomRoutersByFilter(IRouterInfoVisitor * v, IRouterInfoFilter * f, size_t n)
  {
    return i2p::data::netdb.VisitRandomRouterInfos([f] (std::shared_ptr<const i2p::data::RouterInfo> ri) -> bool {
        return f->Filter(ri);
      }, [v] (std::shared_ptr<const i2p::data::RouterInfo> ri) {
        v->VisitRouterInfo(ri);
      }, n);
  }


  // push router info as meta table onto stack, return stack index of meta table
  void push_RouterInfo(lua_State *L, std::shared_ptr<const i2p::data::RouterInfo> ri) {
    lua_newtable(L);
    int table = lua_gettop(L);
    // floodfill
    lua_pushboolean(L, ri->IsFloodfill());
    lua_setfield(L, table, "floodfill");
    // reachable
    lua_pushboolean(L, ri->IsReachable());
    lua_setfield(L, table, "reachable");
    // base64 ident
    auto ident = ri->GetIdentHashBase64();
    lua_pushstring(L, ident.c_str());
    lua_setfield(L, table, "ident");
    // ssu addresses
    auto ssu4 = ri->GetSSUAddress();
    if(ssu4) {
      lua_pushstring(L, ssu4->host.to_string().c_str());
      lua_setfield(L, table, "ssu");
    }
    // ssu6 address
    auto ssu6 = ri->GetSSUV6Address();
    if(ssu6) {
      lua_pushstring(L, ssu6->host.to_string().c_str());
      lua_setfield(L, table, "ssu6");
    }
    // ntcp address
    auto ntcp = ri->GetNTCPAddress();
    if(ntcp) {
      lua_pushstring(L, ntcp->host.to_string().c_str());
      lua_setfield(L, table, "ntcp");
    }
  }

  // call a lua function as router info filter
  class LuaRouterInfoFilter : public IRouterInfoFilter {

  public:
    LuaRouterInfoFilter(lua_State *state, int filter) :
      L(state),
      filterfunc(filter) {}

    virtual bool Filter(RI ri) {
      lua_pushcfunction(L, writeStacktrace);
      int err = lua_gettop(L);
      lua_pushvalue(L, filterfunc);
      push_RouterInfo(L, ri);
      int result = lua_pcall(L, 1, 1, err);
      bool ret = false;
      if(result == LUA_OK) {
        ret = lua_toboolean(L, -1) == 1;
      } else {
        // call errored
        LogPrint(eLogError, "Lua: error in netdb filter ", luaL_checkstring(L, -1));
      }
      lua_pop(L, 2); // result of call and error handler
      return ret;
    }
  private:
    lua_State * L;
    int filterfunc;
  };

  // visit a router info with a lua function
  class LuaRouterInfoVisitor : public IRouterInfoVisitor {

  public:
    LuaRouterInfoVisitor(lua_State *state, int visit) :
      L(state),
      visitfunc(visit) {}

    virtual void VisitRouterInfo(RI ri) {
      lua_pushcfunction(L, writeStacktrace);
      int err = lua_gettop(L);
      lua_pushvalue(L, visitfunc);
      push_RouterInfo(L, ri);
      int result = lua_pcall(L, 1, 1, err);
      if(result == LUA_OK) {
        // we good
      } else {
        // error
        LogPrint(eLogError, "Lua: failed to visit RI ident=", ri->GetIdentHashBase64(), " ", luaL_checkstring(L, -1));
      }
      lua_pop(L, 2);
    }

  private:
    lua_State * L;
    int visitfunc;
  };

  int l_VisitRIByHash(lua_State *L)
  {
    // get number of args
    int n = lua_gettop(L);
    if ( n != 2) {
      // bad number of args
      return luaL_error(L, "incorrect number of arguments: %i", n);
    }
    const char * str = luaL_checkstring(L, 1);
    if(str == nullptr) {
      return luaL_error(L, "first argument is not a string");
    }
    if(!lua_isfunction(L, 2)) {
      return luaL_error(L, "second argument is not a function");
    }
    // get RI
    auto ri = FindRouterByHash(str);
    // push callback
    lua_pushvalue(L, 2);
    if(ri)
      push_RouterInfo(L, ri);
    else
      lua_pushnil(L);
    // call callback
    lua_call(L, 1, 0);
    lua_pushnil(L);
    return 1;
  }

  int l_VisitRandomRIWithFilter(lua_State* L)
  {
    int n = lua_gettop(L);
    lua_Integer i = 0;
    size_t visited = 0;
    if ( n == 3) {
      int isint = 0;
      i = lua_tointegerx(L, 1, &isint);
      if (isint) {
        if( lua_isfunction(L, 2) && lua_isfunction(L, 3)) {
          if(i <= 0) {
            LogPrint(eLogError, "Lua: cannot iterate over ", i, " routers");
          } else {
            LuaRouterInfoFilter f(L, 2);
            LuaRouterInfoVisitor v(L, 3);
            visited = VisitRandomRoutersByFilter(&v, &f, i);
          }
        } else {
          LogPrint(eLogError, "Lua: bad arguments to VisitRandomRIWithFilter, arguments 2 or 3 are not functions");
        }
      } else {
        LogPrint(eLogError, "Lua: bad arguments to VisitRandomRIWithFilter, argument 1 is not integer");
      }
    } else {
      LogPrint(eLogError, "Lua: bag arguments to VisitRandomRIWithFilter");
    }
    lua_pushinteger(L, visited);
    return 1;
  }

}
}
