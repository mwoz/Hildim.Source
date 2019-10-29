#pragma once
#include "messagebus\mbtransport.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define MESSAGEOBJECT "MESSAGEOBJECT*"

struct MsgWrap{
	ULONGLONG key;
};
void throw_L_error(lua_State* L, const char *str);
int wrap_cmsg(lua_State* L, CMessage* msg);
int rewrap_cmsg(lua_State* L, MsgWrap* wrp);
CMessage* cmessage_arg(lua_State* L,LPCSTR module , int idx = 1);
void cmesage_Destroy(lua_State* L);
void cmesage_gc(lua_State* L);
