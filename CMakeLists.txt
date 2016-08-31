cmake_minimum_required(VERSION 3.2)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
add_definitions("-std=c++14")
find_package("Lua")
set(I2LUA_NAME "i2lua")
set(I2LUA_SRC
  "i2lua/log.cc"
  "i2lua/main.cc"
  "i2lua/netdb.cc"
  "i2lua/router.cc"
  "i2lua/destination.cc"
)
include_directories("${LUA_INCLUDE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
add_executable(${I2LUA_NAME} ${I2LUA_SRC})
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/i2pd/build")
target_link_libraries(${I2LUA_NAME} ${LUA_LIBRARIES} pthread libi2pd i2pdclient boost_system boost_program_options ssl crypto boost_filesystem z boost_date_time)
