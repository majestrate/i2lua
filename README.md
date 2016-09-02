# i2lua

simple lua interpreter wrapping i2pd

Right now I2Lua's API is subject to change at any time so be warned things will probably break

dependancies:

* a modern c++ compiler
* git
* cmake
* boost
* libssl
* zlib

building:

    mkdir ~/git/
    git clone https://github.com/majestrate/i2lua ~/git/i2lua
    mkdir /tmp/i2lua
    cd /tmp/i2lua
    cmake ~/git/i2lua
    make -j4

running:

    cd ~/git/i2lua/contrib/examples/
    /tmp/i2lua/i2lua example.lua

see [here](contrib/examples/) for example scripts (work in progress)
