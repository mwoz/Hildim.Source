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
	wrp->data = NULL;
	luaL_getmetatable(L,MESSAGEOBJECT);
	lua_setmetatable(L,-2);
	return 1;
}

CMessage* cmessage_arg(lua_State* L,LPCSTR module , int idx)
{
	MsgWrap *wrp = (MsgWrap*)lua_touserdata(L,idx);
	if (!wrp) 
	{	
		CString strError;
		strError.Format("%s:Argument %d isn't a message", module, idx);
		throw_L_error(L, strError);
	}
	return (CMessage*)wrp->msg;
}
