#include "stdafx.h"
#include "mblua_util.h"


LUA_API const char* lua_pushstringW(lua_State* L, const WCHAR* w) {
	return lua_pushstring(L, W2MB(w).c_str());
}

void throw_L_error(lua_State* L, const char *str)
{
	lua_pushstring(L,str); 
	lua_error(L);
}

int wrap_cmsg(lua_State* L, CMessage* msg)
{
	MsgWrap *wrp = (MsgWrap*)lua_newuserdata(L,sizeof(MsgWrap));
	wrp->msg = msg;
	luaL_getmetatable(L,MESSAGEOBJECT);
	lua_setmetatable(L,-2);
	return 1;
}

CMessage* cmessage_arg(lua_State* L,LPCSTR module , int idx)
{
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L,idx, MESSAGEOBJECT);
	if (!wrp) 
	{	
		throw_L_error(L, string_format("%s:Argument %d isn't a message", module, idx).c_str());
	}
	if (!wrp->msg) {
		throw_L_error(L, string_format("%s:Argument %d deleted", module, idx).c_str());
	}
	return wrp->msg;
}
void cmesage_gc(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		throw_L_error(L, string_format("mesage_gc:Argument 1 isn't a message").c_str());
	}
	CMessage *msg = wrp->msg;
	if (msg && !msg->InternalRelease())
		wrp->msg = NULL;
}

void cmesage_Destroy(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		throw_L_error(L, string_format("mesage_Destroy:Argument 1 isn't a message").c_str());
	}
	CMessage *msg = wrp->msg;
	if (msg) 
		msg->InternalRelease();
	wrp->msg = NULL;
}