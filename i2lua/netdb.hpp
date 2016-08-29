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

  /** interface defining a router info filter algorithm */
  struct IRouterInfoFilter
  {
    typedef i2p::data::RouterInfo RI;
    /** return true if this filter matches this router info */
    virtual bool Filter(const RI & ri) = 0;
  };

  /** interface defining an entity that can visit all router infos in the netdb */
  struct IRouterInfoVisitor
  {
    typedef i2p::data::RouterInfo RI;
    virtual void VisitRouterInfo(const RI & ri) = 0;
  };
  
  /** iterate over the netdb, filter matches, if exclude is true then the filter will be inverted */
  void VisitRoutersByFilter(IRouterInfoVisitor * visitor, IRouterInfoFilter * filter, bool exclude=false);

  int l_VisitRIByHash(lua_State *L);
  int l_VisitRIWithFilter(lua_State *L);
}
}

#endif
