# i2lua

simple lua interpreter wrapping i2pd

Right now I2Lua's API is subject to change at any time so be warned things will probably break

building:

    git clone --recursive https://github.com/majestrate/i2lua 
    cd i2lua
    mkdir build
    cmake ..
    make

running:

    ./build/i2lua ./contrib/examples/example.lua

see [contrib/examples/](here) for example scripts (work in progress)
