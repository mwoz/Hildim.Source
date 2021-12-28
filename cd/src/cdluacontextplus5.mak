PROJNAME = cd
LIBNAME = cdluacontextplus

OPT = YES

SRCDIR = lua5
SRC = cdluacontextplus5.c
DEF_FILE = cdluacontextplus5.def
DEPENDDIR = dep

LIBS = cdcontextplus

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  CHECK_GDIPLUS = Yes
else
  ifdef GTK_DEFAULT
    CHECK_GTK = Yes
  else
    CHECK_XRENDER = Yes
  endif
endif

ifdef USE_LUA_VERSION
  USE_LUA51:=
  USE_LUA52:=
  USE_LUA53:=
  USE_LUA54:=
  ifeq ($(USE_LUA_VERSION), 54)
    USE_LUA54:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 53)
    USE_LUA53:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 52)
    USE_LUA52:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 51)
    USE_LUA51:=Yes
  endif
endif

ifdef USE_LUA54
  LIBNAME := $(LIBNAME)54
else
ifdef USE_LUA53
  LIBNAME := $(LIBNAME)53
else
ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif
endif
endif

# To not link with the Lua dynamic library in UNIX
NO_LUALINK = Yes
# To use a subfolder with the Lua version for binaries
LUAMOD_DIR = Yes
USE_CDLUA = YES
CD = ..

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  USE_CD = YES
  USE_CDLUA:=
  INCLUDES += ../include
  LDIR = ../lib/$(TEC_UNAME)
endif
