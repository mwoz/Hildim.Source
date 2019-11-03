#include "stdafx.h"
#include "mblua_util.h"

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
		CString strError;
		strError.Format("%s:Argument %d isn't a message", module, idx);
		throw_L_error(L, strError);
	}
	if (!wrp->msg) {
		CString strError;
		strError.Format("%s:Argument %d deleted", module, idx);
		throw_L_error(L, strError);
	}
	return wrp->msg;
}
void cmesage_gc(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		CString strError;
		strError.Format("mesage_gc:Argument 1 isn't a message");
		throw_L_error(L, strError);
	}
	CMessage *msg = wrp->msg;
	if (msg && !msg->InternalRelease())
		wrp->msg = NULL;
}

void cmesage_Destroy(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		CString strError;
		strError.Format("mesage_Destroy:Argument 1 isn't a message");
		throw_L_error(L, strError);
	}
	CMessage *msg = wrp->msg;
	if (msg) 
		msg->InternalRelease();
	wrp->msg = NULL;
}