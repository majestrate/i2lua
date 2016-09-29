cmake_minimum_required(VERSION 3.2.1)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
add_definitions("-std=c++14")

# external dependancies directory
set(DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/")
set(PROJECT_PREFIX "${DEPS_DIR}/prefix/")

# i2pd
set(I2P_PROJECT_NAME "i2pd")
set(I2P_REPO_URL "https://github.com/majestrate/i2pd")
set(I2P_DIR "${DEPS_DIR}/${I2P_PROJECT_NAME}")
set(I2P_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake-i2pd/")
set(I2P_ROOT_DOR "${I2P_DIR}/src/")
set(I2P_INCLUDE_DIR "${PROJECT_PREFIX}/include/i2pd/")
file(GLOB I2P_CMAKE_FILES "${I2P_CMAKE_DIR}*")
set(I2P_LIBS "${PROJECT_PREFIX}/lib/libi2pdclient.a" "${PROJECT_PREFIX}lib/libi2pd.a")


include(ExternalProject)

ExternalProject_Add(${I2P_PROJECT_NAME}
  GIT_REPOSITORY "${I2P_REPO_URL}"
  PATCH_COMMAND cp -av "${I2P_CMAKE_FILES}" "."
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DWITH_BINARY=OFF)

# lua 5.3
set(LUA_PROJECT_NAME "lua")
set(LUA_DOWNLOAD_URL "http://www.lua.org/ftp/lua-5.3.3.tar.gz")
set(LUA_DOWNLOAD_DIGEST "SHA1=A0341BC3D1415B814CC738B2EC01AE56045D64EF")
set(LUA_INCLUDE_DIR "${PROJECT_PREFIX}/include/") 
set(LUA_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake-lua/")
file(GLOB LUA_CMAKE_FILES "${LUA_CMAKE_DIR}*")
set(LUA_LIBS "${PROJECT_PREFIX}/lib/liblua_static.a")

ExternalProject_Add(${LUA_PROJECT_NAME}
  URL_HASH ${LUA_DOWNLOAD_DIGEST}
  URL "${LUA_DOWNLOAD_URL}"
  PATCH_COMMAND cp -av "${LUA_CMAKE_FILES}" "."
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_PREFIX})

set(I2LUA_NAME "i2lua")
set(I2LUA_SRC
  "i2lua/interpreter.cc"
  "i2lua/log.cc"
  "i2lua/main.cc"
  "i2lua/netdb.cc"
  "i2lua/router.cc"
  "i2lua/destination.cc"
  "i2lua/profile.cc"
)


# i2lua executable
add_executable(${I2LUA_NAME} ${I2LUA_SRC})
add_dependencies(${I2LUA_NAME} ${I2P_PROJECT_NAME} ${LUA_PROJECT_NAME})

# i2pd includes
include_directories(${I2LUA_NAME} ${I2P_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
# lua include
include_directories(${LUA_INCLUDE_DIR})

find_package(Boost COMPONENTS system filesystem program_options date_time REQUIRED)
find_package(OpenSSL)
find_package(ZLIB)

target_link_libraries(${I2LUA_NAME} ${I2P_LIBS} ${LUA_LIBS} -pthread ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARY})
