#include "stdafx.h"
#include "L_mbConnector.h"
#include "mblua_util.h"


CL_mbConnector::~CL_mbConnector(void)
{
}
void CL_mbConnector::SetCallback(int idx)
{
	if(callback_idx != 0)
	{
		luaL_unref(L,LUA_REGISTRYINDEX,callback_idx);
	}
	lua_pushvalue(L,idx);
	callback_idx = luaL_ref(L,LUA_REGISTRYINDEX);
}
HRESULT CL_mbConnector::OnMbReply(mb_handle handle, void* pOpaque, int error, CMessage* pMsg)
{
	if (callback_idx != 0) {
		lua_rawgeti(L,LUA_REGISTRYINDEX,callback_idx);
		lua_pushlightuserdata(L,handle);
		wrap_cmsg(L,(CMessage*)pOpaque);
		lua_pushinteger(L, error);
		wrap_cmsg(L,pMsg);
		
		if (lua_pcall(L,4,0,0)) { //обработка ошибки
			if (lua_isstring(L,-1)) {
				size_t len;
				const char *msg = lua_tolstring(L,-1,&len);
				char *buff = new char[len+2];
				strncpy_s(buff, len+2, msg, len);
				buff[len] = '\n';
				buff[len+1] = '\0';
				lua_pop(L,1);
				if (lua_checkstack(L,3)) {
					lua_getglobal(L,"output");
					lua_getfield(L,-1,"AddText");
					lua_insert(L,-2);
					lua_pushstring(L,buff);
					lua_pcall(L,2,0,0);
				}
				delete[] buff;
			}
		}
	}
	if(m_autodestroy)
	{
		m_pManager->mbUnsubscribeAll(this);
		delete this;
	}
	return 0;
}
