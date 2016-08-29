#include "netdb.hpp"
#include "i2pd/NetDb.h"

namespace i2p
{
namespace lua
{
  const std::shared_ptr<i2p::data::RouterInfo> FindRouterByHash(const std::string & hash) {
    i2p::data::IdentHash rh;
    rh.FromBase64(hash);
    return i2p::data::netdb.FindRouter(rh);
  }

  void VisitRoutersByFilter(IRouterInfoVisitor * v, IRouterInfoFilter * f, bool exclude)
  {
    i2p::data::netdb.VisitRouterInfos([v, f, exclude] (const i2p::data::RouterInfo & ri) {
        if(f->Filter(ri) ^ exclude)
          v->VisitRouterInfo(ri);
    });
  }

  // push router info as meta table onto stack, return stack index of meta table
  void pushRouterInfo(lua_State *L, const i2p::data::RouterInfo & ri) {
    lua_newtable(L);
    int table = lua_gettop(L);
    // floodfill
    lua_pushboolean(L, ri.IsFloodfill());
    lua_setfield(L, table, "floodfill");
    // reachable
    lua_pushboolean(L, ri.IsReachable());
    lua_setfield(L, table, "reachable");
    // base64 ident
    auto ident = ri.GetIdentHashBase64();
    lua_pushstring(L, ident.c_str());
    lua_setfield(L, table, "ident");
    // ssu addresses
    auto ssu4 = ri.GetSSUAddress();
    if(ssu4) {
      lua_pushstring(L, ssu4->host.to_string().c_str());
      lua_setfield(L, table, "ssu");
    }
    // ssu6 address
    auto ssu6 = ri.GetSSUV6Address();
    if(ssu6) {
      lua_pushstring(L, ssu6->host.to_string().c_str());
      lua_setfield(L, table, "ssu6");
    }
    // ntcp address
    auto ntcp = ri.GetNTCPAddress();
    if(ntcp) {
      lua_pushstring(L, ntcp->host.to_string().c_str());
      lua_setfield(L, table, "ntcp");
    }
  }

  // call a lua function as router info filter
  class LuaRouterInfoFilter : public IRouterInfoFilter {

  public:
    LuaRouterInfoFilter(lua_State *state, int filter) :
      L(state) {
      lua_pushvalue(state, filter);
      filterfunc = lua_gettop(state);
    }

    ~LuaRouterInfoFilter() {
      lua_remove(L, filterfunc);
    }
    
    virtual bool Filter(const RI & ri) {
      lua_pushvalue(L, filterfunc);
      pushRouterInfo(L, ri);
      lua_call(L, 1, 1);
      return lua_toboolean(L, -1);
    }
  private:
    lua_State * L;
    int filterfunc;
  };

  // visit a router info with a lua function
  class LuaRouterInfoVisitor : public IRouterInfoVisitor {

  public:
    LuaRouterInfoVisitor(lua_State *state, int visit) :
      L(state) {
      lua_pushvalue(state, visit);
      visitfunc = lua_gettop(state);
    }

    ~LuaRouterInfoVisitor() {
      lua_remove(L, visitfunc);
    }
    
    virtual void VisitRouterInfo(const RI & ri) {
      lua_pushvalue(L, visitfunc);
      pushRouterInfo(L, ri);
      lua_call(L, 1, 0);
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
      pushRouterInfo(L, *ri);
    else
      lua_pushnil(L);
    // call callback
    lua_call(L, 1, 0);
    lua_pushnil(L);
    return 1;
  }

  int l_VisitRIWithFilter(lua_State *L)
  {
    int n = lua_gettop(L);
    if ( n != 2) {
      // bad number of args
      return luaL_error(L, "incorrect number of arguments: %d", n);
    }
    if (lua_isfunction(L, 1) && lua_isfunction(L, 2)) {
      lua_pushvalue(L, 1);
      int filterFunc = lua_gettop(L);
      lua_pushvalue(L, 2);
      int visitFunc = lua_gettop(L);
      LuaRouterInfoFilter f(L, filterFunc);
      LuaRouterInfoVisitor v(L, visitFunc);
      VisitRoutersByFilter(&v, &f);
      lua_pop(L, 2);
      lua_pushnil(L);
      return 1;
    }
    return luaL_error(L, "bad arguments, must pass 2 functions");
  }
                       
  
}
}
