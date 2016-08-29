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

  // push router info as meta table onto stack
  static void pushRouterInfo(lua_State *L, const i2p::data::RouterInfo & ri) {
    lua_newtable(L);
    // floodfill
    lua_pushboolean(L, ri.IsFloodfill());
    lua_setfield(L, lua_gettop(L), "floodfill");
    // base64 ident
    auto ident = ri.GetIdentHashBase64();
    lua_pushstring(L, ident.c_str());
    lua_setfield(L, lua_gettop(L), "ident");
  }

  // call a lua function as router info filter
  class LuaRouterInfoFilter : public IRouterInfoFilter {

  public:
    LuaRouterInfoFilter(lua_State *state, int filter, int error=0) :
      L(state),
      filterfunc(filter),
      errorfunc(error) {}
    
    virtual bool Filter(const RI & ri) {
      lua_pushvalue(L, filterfunc);
      pushRouterInfo(L, ri);
      int err = lua_pcall(L, 1, 1, errorfunc);
      bool result = err == 0 && lua_toboolean(L, -1);
      lua_pop(L, 1);
      return result;
    }
  private:
    lua_State * L;
    int filterfunc;
    int errorfunc;
  };

  // visit a router info with a lua function
  class LuaRouterInfoVisitor : public IRouterInfoVisitor {

  public:
    LuaRouterInfoVisitor(lua_State *state, int visit, int error=0) :
      L(state),
      visitfunc(visit),
      errorfunc(error) {}

    virtual void VisitRouterInfo(const RI & ri) {
      lua_pushvalue(L, visitfunc);
      pushRouterInfo(L, ri);
      int result = lua_pcall(L, 1, 0, errorfunc);
      if (result == LUA_ERRRUN) {
        // runtime error
        return;
      } else if(result) {
        // some other error
        throw std::runtime_error("fatal lua error while visiting router info");
      }
    }
    
  private:
    lua_State * L;
    int visitfunc;
    int errorfunc;
  };
  
  int l_VisitRIByHash(lua_State *L)
  {
    // get number of args
    int n = lua_gettop(L);
    if ( n != 3) {
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
    int errorfunc = 0;
    if (lua_isfunction(L, 3)) {
      // we have an error function
      errorfunc = 3;
    }
    // get RI
    auto ri = FindRouterByHash(str);
    // push callback
    lua_pushvalue(L, 2);
    if(ri)
      lua_pushlightuserdata(L, ri.get());
    else
      lua_pushnil(L);
    auto result = lua_pcall(L, 1, 0, errorfunc);
    if(result) {
      // error handling callback
      if(result == LUA_ERRRUN) {
        // runtime error
        return luaL_error(L, "runtime error visiting RI %s", str);
      } else {
        // another error, really bad
        return luaL_error(L, "really bad error happened: %d", result);
      }
    }
    return 0;
  }

  int l_VisitRIWithFilter(lua_State *L)
  {
    int n = lua_gettop(L);
    if ( n != 3) {
      // bad number of args
      return luaL_error(L, "incorrect number of arguments: %i", n);
    }
    if (lua_isfunction(L, 1) && lua_isfunction(L, 2)) {
      int filterFunc = 1;
      int visitFunc = 2;
      int errorFunc = 0;
      if (lua_isfunction(L, 3)) {
        errorFunc = 3;
      }
      LuaRouterInfoFilter f(L, filterFunc, errorFunc);
      LuaRouterInfoVisitor v(L, visitFunc, errorFunc);
      VisitRoutersByFilter(&v, &f);
      return 0;
    }
    return luaL_error(L, "bad arguments, must pass 2 functions");
  }
                       
  
}
}
