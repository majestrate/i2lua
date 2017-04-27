#include "interpreter.hpp"
#include <signal.h>
#include <string>

#include "libi2pd/Tunnel.h"
#include "libi2pd/Transports.h"
#include "libi2pd/NetDb.hpp"
#include "signal.hpp"

//
// entry point for luajit i2p router
//
int main(int argc, char * argv[]) {
  int exitcode = 0;
  if (argc == 2) {
    i2p::lua::I2Lua i2lua;
    if(i2lua.LoadFile(argv[1], std::cerr)) {
      // we loaded the file
      exitcode = i2lua.Mainloop();
    } else {
      // failed to load file
      exitcode = 1;
    }
  } else {
    // print usage
    std::cout << "usage: " << argv[0] << " file.lua" << std::endl;
  }
  return exitcode;
}
