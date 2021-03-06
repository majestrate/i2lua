REPO_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
LUA_SRC = $(REPO_DIR)/deps/lua-5.3.4/
# TODO: autodetect LUA_PLAT
LUA_PLAT = linux
LUA_INSTALL_PREFIX=$(REPO_DIR)/prefix
LIB_LUA=$(LUA_INSTALL_PREFIX)/lib/liblua.a

INC_FLAGS = -I$(LUA_INSTALL_PREFIX)/include

CXXFLAGS = $(INC_FLAGS) -std=c++14
AR = ar

I2LUA_SRC_DIR= $(REPO_DIR)/i2lua
SRC = $(wildcard $(I2LUA_SRC_DIR)/*.cc)
OBJ  = $(SRC:.cc=.o)
I2LUA_LIB = libi2lua.a
EXE = i2plua


I2PD_ROOT=$(REPO_DIR)/i2pd

INC_FLAGS += -I$(I2PD_ROOT) -I$(I2PD_ROOT)/libi2pd -I$(I2PD_ROOT)/libi2pd_client -I$(I2PD_ROOT)/daemon

LIBI2PD_CLIENT=$(I2PD_ROOT)/libi2pdclient.a
LIBI2PD = $(I2PD_ROOT)/libi2pd.a

LIBS=-lboost_system -lboost_date_time -lboost_filesystem -lboost_program_options -lssl -lcrypto -pthread -ldl -lz

all: $(EXE)

$(EXE): $(LIB_LUA) i2pd-build $(I2LUA_LIB)
	$(CXX) -o $(EXE) $(I2LUA_LIB) $(LIB_LUA) $(LIBI2PD) $(LIBI2PD_CLIENT) $(LIBS)

build-libs: $(LUA_LIB) $(I2LUA_LIB)

$(I2LUA_LIB): $(LUA_LIB) $(OBJ)
	$(AR) -r $(I2LUA_LIB) $(OBJ)

$(OBJ): $(SRC) $(LIB_LUA)

$(LIB_LUA): lua

lua:
	$(MAKE) -C $(LUA_SRC) INSTALL_TOP=$(LUA_INSTALL_PREFIX) $(LUA_PLAT)
	$(MAKE) -C $(LUA_SRC) INSTALL_TOP=$(LUA_INSTALL_PREFIX) install

lua-clean:
	$(MAKE) -C $(LUA_SRC) clean

$(I2PD_ROOT): i2pd-build

i2pd-build: 
	$(MAKE) -C $(I2PD_ROOT)

i2pd-clean:
	$(MAKE) -C $(I2PD_ROOT) clean

clean: lua-clean i2pd-clean
	rm -f $(OBJ)
	rm -rf $(LUA_INSTALL_PREFIX)
