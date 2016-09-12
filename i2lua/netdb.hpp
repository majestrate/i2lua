#ifndef SRC_CORE_LUA_NETDB_HPP_
#define SRC_CORE_LUA_NETDB_HPP_
#include <string>
#include <memory>
#include <vector>
#include <lua.hpp>
#include "i2pd/RouterInfo.h"

namespace i2p
{
namespace lua
{
  const std::shared_ptr<i2p::data::RouterInfo> FindRouterByHash(const std::string & hash);

  void push_RouterInfo(lua_State* L, std::shared_ptr<const i2p::data::RouterInfo> ri);
  
  /** interface defining a router info filter algorithm */
  struct IRouterInfoFilter
  {
    typedef std::shared_ptr<const i2p::data::RouterInfo> RI;
    /** return true if this filter matches this router info */
    virtual bool Filter(RI ri) = 0;
  };

  /** interface defining an entity that can visit all router infos in the netdb */
  struct IRouterInfoVisitor
  {
    typedef std::shared_ptr<const i2p::data::RouterInfo> RI;
    virtual void VisitRouterInfo(RI ri) = 0;
  };

  /** visit N random router infos that match a filter, return how many we actually visited */
  size_t VisitRandomRouterByFilter(IRouterInfoVisitor * visitor, IRouterInfoFilter * filter, size_t n);

  /** 
   *   f(ident, visitor)
   *   visitor(ri) 
   */
  int l_VisitRIByHash(lua_State *L);
  /**
   *   f(max, filter, visitor)
   *   visitor(ri)
   *   filter(ri) returns bool
   */
  int l_VisitRandomRIWithFilter(lua_State* L);
}
}

#endif
