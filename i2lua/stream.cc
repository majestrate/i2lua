#include "stream.hpp"
#include "destination.hpp"
#include "log.hpp"
#include "libi2pd_client/ClientContext.h"

namespace i2p
{
  namespace lua
  {

    /** stream write event */
    struct StreamWrite
    {
      std::shared_ptr<i2p::stream::Stream> stream;
      int callback;
      size_t n;
    };

    /** stream read event */
    struct StreamRead
    {
      std::shared_ptr<i2p::stream::Stream> stream;
      size_t read;
      uint8_t * buf;
      int callback;

      ~StreamRead() {
        stream = nullptr;
        delete [] buf;
      }

    };

    /**
       single tcp/i2p connection
     */
    struct StreamContext
    {
      std::shared_ptr<i2p::stream::Stream> impl;
      lua_State * thread ;
      int callback ;

      void Close()
      {
        impl->Close();
        impl = nullptr;
      }

      void OnConnectResult(std::shared_ptr<i2p::stream::Stream> stream)
      {
        lua_State * L = thread;
        lua_pushcfunction(L, writeStacktrace); // push error handler
        int err = lua_gettop(L);
        lua_pushvalue(L, callback); // push callback
        impl = stream;
        if(impl) {
          // connect success
          lua_pushlightuserdata(L, this);
          lua_pushnil(L);
        } else {
          // fail
          lua_pushnil(L);
          lua_pushliteral(L, "could not connect");
        }
        lua_pcall(L, 2, 0, err); // call
        if(!impl) {
          // this is a bad idea but whatever
          delete this;
        }
      }

      void Read(lua_State * other, int readCb, size_t sz)
      {
        StreamRead * r = new StreamRead;
        r->read = -1;
        r->stream = impl;
        r->buf = new uint8_t[sz];
        lua_State * L = thread;
        lua_pushvalue(other, readCb);
        lua_xmove(other, L, 1);
        r->callback = lua_gettop(L);
        r->stream->AsyncReceive(boost::asio::buffer(r->buf, sz), std::bind(&StreamContext::OnRead, this, std::placeholders::_1, std::placeholders::_2, r));
      }

      void OnRead(const boost::system::error_code & ec, size_t transferred, StreamRead * r)
      {
        lua_State * L = thread;
        lua_pushcfunction(L, writeStacktrace); // push error handler
        int err = lua_gettop(L);
        lua_pushvalue(L, r->callback);
        lua_pushlightuserdata(L, this);
        if(ec) {
          lua_pushnil(L);
          lua_pushstring(L, ec.message().c_str());
        } else {
          lua_pushlstring(L, (const char *) r->buf, transferred);
          lua_pushnil(L);
        }
        lua_pcall(L, 3, 0, err);
        lua_pop(L, 1); // pop error handler
        delete r;
      }

      void Write(lua_State * other, int writeCb, const char * data, size_t sz)
      {
        lua_State * L = thread;
        lua_pushvalue(other, writeCb);
        lua_xmove(other, L, 1);
        StreamWrite * w = new StreamWrite;
        w->callback = lua_gettop(L);
        w->n = sz;
        w->stream = impl;
        w->stream->AsyncSend((const uint8_t *)data, sz, std::bind(&StreamContext::OnWrote, this, std::placeholders::_1, w));
      }

      void OnWrote(const boost::system::error_code & ec, StreamWrite * wr)
      {
        lua_State * L = thread;
        lua_pushcfunction(L, writeStacktrace); // push error handler
        int err = lua_gettop(L);
        lua_pushvalue(L, wr->callback);
        lua_pushlightuserdata(L, this);
        if(ec) {
          lua_pushinteger(L, -1);
          lua_pushstring(L, ec.message().c_str());
        } else {
          lua_pushinteger(L, wr->n);
          lua_pushnil(L);
        }
        lua_pcall(L, 3, 0, err);
        lua_pop(L, 1); // pop error handler
        wr->stream = nullptr;
        delete wr;
      }

    };

    StreamContext * getStream(lua_State * L, int idx)
    {
      return (StreamContext *) lua_touserdata(L, idx);
    }

    int l_StreamOpen(lua_State * L)
    {
      int top = lua_gettop(L);
      if(top != 4) {
        return luaL_error(L, "wrong number of arguments: %d", top);
      }
      if(!lua_isfunction(L, 4)) {
        return luaL_error(L, "argument 3 must be a function");
      }
      const char * addr = luaL_checkstring(L, 2);
      if(!addr) {
        return luaL_error(L, "argument 2 must be a string");
      }

      lua_Integer port = lua_tointeger(L, 3);

      std::string name(addr);
      i2p::data::IdentHash ih;
      // resolve base32 address
      if(!i2p::client::context.GetAddressBook().GetIdentHash(name, ih)) {
        // failed to resolve
        lua_pushcfunction(L, writeStacktrace); // push error handler
        int err = lua_gettop(L);
        lua_pushvalue(L, 4); // push callback
        lua_pushnil(L); // push nil for stream
        lua_pushliteral(L, "failed to resolve name"); // push error message
        lua_pcall(L, 2, 0, err); // call
        lua_pop(L, 1); // pop error handler
        lua_pushnil(L); // push return value nil and return
        return 1;
      }
      Destination * dest = getDestination(L, 1);

      lua_pushvalue(L, 4);
      int callback = lua_gettop(L);
      StreamContext * stream = new StreamContext;
      // create new thread
      stream->thread = lua_newthread(L);
      // give callback to thread
      lua_pushvalue(L, callback);
      lua_xmove(L, stream->thread, 1);
      stream->callback = lua_gettop(stream->thread);

      // create stream
      dest->Dest->CreateStream(std::bind(&StreamContext::OnConnectResult, stream, std::placeholders::_1), ih, port);
      lua_pushnil(L);
      return 1;
    }

    int l_StreamRead(lua_State * L)
    {
      int top = lua_gettop(L);
      if (top != 3) {
        return luaL_error(L, "bad number of arguments: %d", top);
      }
      StreamContext * stream = getStream(L, 1);
      if(!stream) {
        return luaL_error(L, "first argument is not a stream");
      }
      lua_Integer n = lua_tointeger(L, 2);
      if(n <= 0) {
        return luaL_error(L, "second argument is <= 0 or not an integer");
      }
      if(!lua_isfunction(L, 3)) {
        return luaL_error(L, "third argument is not a function");
      }
      stream->Read(L, 3, n);
      lua_pushnil(L);
      return 1;
    }

    int l_StreamWrite(lua_State * L)
    {

      int top = lua_gettop(L);
      if (top != 3) {
        return luaL_error(L, "bad number of arguments: %d", top);
      }
      StreamContext * stream = getStream(L, 1);
      if(!stream) {
        return luaL_error(L, "first argument is not a stream");
      }
      size_t sz = 0;
      const char * data = lua_tolstring(L, 2, &sz);
      if(data == nullptr) {
        return luaL_error(L, "second argument is not a string");
      }
      if(sz <= 0) {
        return luaL_error(L, "second argument is <= 0 bytes large");
      }
      if(!lua_isfunction(L, 3)) {
        return luaL_error(L, "third argument is not a function");
      }
      stream->Write(L, 3, data, sz);
      lua_pushnil(L);
      return 1;
    }

    int l_StreamClose(lua_State * L)
    {
      int top = lua_gettop(L);
      if(top != 1) {
        return luaL_error(L, "wrong number of arguments: %d", top);
      }
      StreamContext * stream = getStream(L, 1);
      if(stream) {
        stream->Close();
        delete stream;
      }
      lua_pushnil(L);
      return 1;
    }
  }
}
