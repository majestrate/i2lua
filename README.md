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
    git clone --recursive https://github.com/majestrate/i2lua ~/git/i2lua
    cd ~/git/i2lua/
    make

running:

    i2plua ~/git/i2lua/contrib/examples/

see [here](contrib/examples/) for example scripts (work in progress)
