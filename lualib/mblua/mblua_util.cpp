#include "stdafx.h"
#include "mblua_util.h"

CMap<DWORD, DWORD, CMessage *, CMessage* >m_mapAllMessages;
ULONGLONG m_dmsgCounter = 0;

void throw_L_error(lua_State* L, const char *str)
{
	lua_pushstring(L,str); 
	lua_error(L);
}

int wrap_cmsg(lua_State* L, CMessage* msg)
{
	m_dmsgCounter++;
	m_mapAllMessages[m_dmsgCounter] = msg;
	MsgWrap *wrp = (MsgWrap*)lua_newuserdata(L,sizeof(MsgWrap));
	wrp->key = m_dmsgCounter;
	luaL_getmetatable(L,MESSAGEOBJECT);
	lua_setmetatable(L,-2);
	return 1;
}

int rewrap_cmsg(lua_State* L, MsgWrap* wrp){
	MsgWrap *wrp1 = (MsgWrap*)lua_newuserdata(L, sizeof(MsgWrap));
	wrp1->key = wrp->key;
	luaL_getmetatable(L, MESSAGEOBJECT);
	lua_setmetatable(L, -2);
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
	if (!m_mapAllMessages[wrp->key]) {
		CString strError;
		strError.Format("%s:Argument %d deleted", module, idx);
		throw_L_error(L, strError);
	}
	return m_mapAllMessages[wrp->key];
}
void cmesage_gc(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		CString strError;
		strError.Format("mesage_gc:Argument 1 isn't a message");
		throw_L_error(L, strError);
	}
	CMessage *msg = m_mapAllMessages[wrp->key];
	if (msg) {
		msg->InternalRelease();
		m_mapAllMessages.RemoveKey(wrp->key);
	}
}

void cmesage_Destroy(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		CString strError;
		strError.Format("mesage_Destroy:Argument 1 isn't a message");
		throw_L_error(L, strError);
	}
	CMessage *msg = m_mapAllMessages[wrp->key];
	if (msg) {
		delete msg;
		m_mapAllMessages.RemoveKey(wrp->key);
	}
}