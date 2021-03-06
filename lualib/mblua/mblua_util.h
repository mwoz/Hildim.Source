#pragma once
#include "messagebus\mbtransport.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define MESSAGEOBJECT "MESSAGEOBJECT*"

struct MsgWrap{
	CMessage *msg;
	void *data;
};
void throw_L_error(lua_State* L, const char *str);
int wrap_cmsg(lua_State* L, CMessage* msg);
CMessage* cmessage_arg(lua_State* L,LPCSTR module , int idx = 1);
