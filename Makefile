REPO_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
LUA_SRC = $(REPO_DIR)/deps/lua-5.3.4/
# TODO: autodetect LUA_PLAT
LUA_PLAT = linux
LUA_INSTALL_PREFIX=$(REPO_DIR)/prefix/

lua:
	$(MAKE) -C $(LUA_SRC) INSTALL_TOP=$(LUA_INSTALL_PREFIX) $(LUA_PLAT) install
