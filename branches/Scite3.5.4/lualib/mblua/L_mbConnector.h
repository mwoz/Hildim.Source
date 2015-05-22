#pragma once
#include "messagebus\mbtransport.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
class CL_mbConnector :
	public CmbConnector
{
protected:
	lua_State* L;
	int callback_idx;
	bool m_autodestroy;
public:
	CL_mbConnector(CmbTransport *pManager, lua_State *l,bool autodestroy = false)
		:CmbConnector(pManager),L(l),callback_idx(0),m_autodestroy(autodestroy){}
	~CL_mbConnector(void);
	void SetCallback(int idx=1);
	virtual HRESULT OnMbReply(mb_handle handle, void* pOpaque, int error, CMessage* pMsg);

};
