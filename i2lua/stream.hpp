#ifndef SRC_CORE_I2LUA_STREAM_HPP_
#define SRC_CORE_I2LUA_STREAM_HPP_
#include <lua.hpp>


namespace i2p
{
  namespace lua
  {

    /**
       f(dest, name, port, callback)

       callback(stream, err)

       stream is nil on failure otherwise a pointer to a stream
       err is nil on success or an error message if an error happened while connecting
     */
    int l_StreamOpen(lua_State * L);

    /**
       f(stream)
     */
    int l_StreamClose(lua_State * L);

    /**
       f(stream, buf, callback)

       callback(stream, bytesWritten, err)

       stream is the stream we used to write with
       bytesWritten is -1 on fail or the number of bytes transferred
       err is nil on success or an error message on fail

     */
    int l_StreamWrite(lua_State * L);

    /**
       f(stream, n, callback)

       callback(stream, buf, err)

       stream is the stream we read from
       buf contains at most n bytes or is nil on read fail
       err is nil on success or an error message on fail
     */
    int l_StreamRead(lua_State * L);


    struct StreamContext;

    StreamContext * getStream(lua_State * L, int idx);

  }
}

#endif
