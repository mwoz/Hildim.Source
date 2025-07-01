// mblua.h : main header file for the mblua DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif
#include "resource.h"		// main symbols

#include <locale>
#include <codecvt>
#include <variant>
#include "Containers/tlxMessage.h"
#include "MessageBus/mbTransport.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../../iup/include/iup.h"
}
#include "LuaBridge.h"
#include "RefCountedObject.h"
#include "RefCountedPtr.h"

class CL_mbConnector :
	public CmbConnector
{
protected:
	lua_State* L;
	int callback_idx;
	bool m_autodestroy;
public:
	CL_mbConnector(CmbTransport* pManager, lua_State* l, bool autodestroy = false)
		:CmbConnector(pManager), L(l), callback_idx(0), m_autodestroy(autodestroy) {
	}
	~CL_mbConnector(void);
	void SetCallback(int idx = 1);
	virtual HRESULT OnMbReply(mb_handle handle, void* pOpaque, int error, CMessage* pMsg);

};

std::string ToStr(LPCWSTR lpcwszStr, int CP = CP_ACP)
{
	// Determine the length of the converted string
	int strLength
		= WideCharToMultiByte(CP, 0, lpcwszStr, -1,
			nullptr, 0, nullptr, nullptr);

	if (strLength > 0) {
		std::string str(strLength - 1, 0);
		str[0] = 0;
		WideCharToMultiByte(CP, 0, lpcwszStr, -1, &str[0],
			strLength, nullptr, nullptr);
		return str;
	}
	return "";
}
CString ToCStr(std::string& str) {
	// Determine the length of the wide string          CP_UTF8
	int len = MultiByteToWideChar(
		CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (len == 0) {
		return CString();
	}

	// Convert the string
	std::wstring wide_str(len, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1,
		&wide_str[0], len);

	CString s(wide_str.c_str());
	return s;
}
CString luaH_CheckCString(lua_State* L, int arg) {
	std::string str = luaL_checkstring(L, arg);

	return ToCStr(str);
}

Variant& luaH_CheckVariant(lua_State* L, int arg) {
	//CString type = luaL_typename(L, arg); 
	int type = lua_type(L, arg);
	static Variant var;
	if (type == LUA_TSTRING)
	{
		CString str = luaH_CheckCString(L, arg);
		COleDateTime dt;
		if (dt.ParseDateTime(str))
			var = dt;
		else
			var = str;

	}
	else if (type == LUA_TNUMBER)
	{
		int isnum = 0;
		lua_Integer res = lua_tointegerx(L, arg, &isnum);
		if (isnum)
			var = static_cast<int>(res);
		else
		    var = luaL_checknumber(L, arg);
	}
	else if (type == LUA_TBOOLEAN)
	{
		
		var = (lua_toboolean(L, arg) == 1);
	}
	else if (type == LUA_TNIL)
	{
		var.SetNull();
		
	}
	else
	{	
		std::string er("Invalid type of value: """);
		er += luaL_typename(L, arg);
		er += """";
		lua_pushstring(L, er.c_str());
		lua_error(L);
	}
	return var;
}



int luaH_PushVariant(lua_State* L, Variant v) {
	switch(v.type) {
	case Variant::Type::Double:
		lua_pushnumber(L, v.dblValue);
		break;
	case Variant::Type::Integer:
	case Variant::Type::Long:
		lua_pushinteger(L, v.intValue);
		break;
	case Variant::Type::Boolean:
		lua_pushboolean(L, v.boolValue);
		break;
	case Variant::Type::Date:
	{
		ATL::COleDateTime dt = v.dateValue;
		dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer();
		lua_pushstring(L, ToStr( dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer()).c_str() );
	}
	break;
	case Variant::Type::String:
		lua_pushstring(L, ToStr(v.strValue).c_str());
		break;
	default:
		lua_pushnil(L);
	}
	return 1;
}



namespace luabridge {



	class luaMessage;
	//typedef RefCountedScriptPtr<luaMessage> RCMessage;
	typedef RefCountedObjectPtr<luaMessage> RCMessage;

	int pushLuaMessage(lua_State* L, luaMessage* msg)
	{ 
		//detail::StackHelper<RCMessage, false>::push(L, RCMessage(msg))

		new (lua_newuserdata(L, sizeof(detail::UserdataShared<RCMessage>))) detail::UserdataShared<RCMessage>(RCMessage(msg));
		lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<luaMessage>());
		// If this goes off it means the class T is unregistered!
		assert(lua_istable(L, -1));
		lua_setmetatable(L, -2);
		return 1;
	}


	class luaMessage : public RefCountedObject//
	{
	public: 
		luaMessage() { m = new CMessage(); };
		luaMessage(CMessage* pm, bool add = true) { m = pm ? pm : new CMessage(); if(add) m->InternalAddRef();  };
		~luaMessage() { 
			m->InternalRelease(); 
		};
		void luaAddField(lua_State* L);
		std::string luaToString(lua_State* L) const{ return ToStr(m->ToString()); }
		void xSetPathValue(lua_State* L);
		int xCounts(lua_State* L);
		int xSubjects(lua_State* L);
		int xFieldValue(lua_State* L){
			CDatum* d = getDatumByArg(L, 2);
			Variant& v = d ? d->value() : Variant();
			return luaH_PushVariant(L, v); 
		}
		int xRemoveField(lua_State* L) { getDatumByArg(L, 2)->~CDatum(); return 0; }
			
		int xSetField(lua_State* L);
		int xFieldType(lua_State* L) { lua_pushstring(L, ToStr(getDatumByArg(L, 2)->GetVarTypeText()).c_str()); return 1; }
		int xFieldName(lua_State* L) { lua_pushstring(L, ToStr(getDatumByArg(L, 2)->id()).c_str()); return 1; }
		int xDestroy(lua_State* L) {  return 0; }
		int xFillList(lua_State* L);
		int xRSCounts(lua_State* L);
		int xRSGetRecord(lua_State* L);
		int xRSColumn(lua_State* L);
		int xSaveFieldBinary(lua_State* L);
		int xAddFieldBinary(lua_State* L);
	
		std::string luaGetName() { 
			return ToStr(m->id()); 
		
		}
	//	void vbsGetFieldCount(vb::CallContext& ctx);
	//	void vbsGetMessageCount(vb::CallContext& ctx);
	//	void vbsGetSendSubject(vb::CallContext& ctx);
	//	void vbsSetSendSubject(vb::CallContext& ctx);
	//	void vbsGetReplySubject(vb::CallContext& ctx);
	//	void vbsSetReplySubject(vb::CallContext& ctx);
	//	void vbsGetRecordset(vb::CallContext& ctx);
	//	void vbsSetRecordset(vb::CallContext& ctx);
		void luaReset() { m->CleanUp(); };
	//	void vbsAddField(vb::CallContext& ctx);
	//	void vbsInsertField(vb::CallContext& ctx);
	//	void vbsUpdateField(vb::CallContext& ctx);
	//	void vbsRemoveField(vb::CallContext& ctx);
		bool luaAddMessage(std::string sub, luaMessage* added);
	//	void vbsInsertMessage(vb::CallContext& ctx);
	//	void vbsUpdateMessage(vb::CallContext& ctx);
		RCMessage luaRemoveMessage(lua_State* L);
	//	void vbsField(vb::CallContext& ctx);
		RCMessage lua_Message(int Index) { 
			return RCMessage(new luaMessage(m->GetMsg(Index)));
		}
	//	void vbsGetMessageIndex(vb::CallContext& ctx);
	//	void vbsExistsField(vb::CallContext& ctx);
		bool luaExistsMessage(std::string path) { return (m->GetMsg(ToCStr(path)) != NULL); };
		void luaCopyFrom(luaMessage* src) { m->AddContentFrom(src->getCMsg()); };
	//	void vbsGetFieldText(vb::CallContext& ctx);
	//	void vbsGetFieldNumber(vb::CallContext& ctx);
	//	void vbsGetFieldDate(vb::CallContext& ctx);
	//	void vbsToString(vb::CallContext& ctx);
		RCMessage luaGetMessage(std::string path) { 
			return RCMessage(new luaMessage(m->AddMsgByPath(ToCStr(path))) );
		}
		RCMessage luaExecute(std::string param);
	//	void vbsAddHeadMessage(vb::CallContext& ctx);
	//	void vbsAddTailMessage(vb::CallContext& ctx);
	//	void vbsRemoveHeadMessage(vb::CallContext& ctx);
	//	void vbsRemoveTailMessage(vb::CallContext& ctx);
	//	void vbsUpdateFrom(vb::CallContext& ctx);
	//	void vbsGetFieldValue(vb::CallContext& ctx);
		int luaGetPathValue(lua_State* L) {
			CDatum* d = m->GetDatumByPath(luaH_CheckCString(L, 2));
			Variant& v = d ? d->value() : Variant();
			return luaH_PushVariant(L, v);
			//return luaH_PushVariant(L, m->GetDatumByPath(luaH_CheckCString(L, 2))->value() ); 
		}
	//	void vbsFlatMessage(vb::CallContext& ctx);
	//	void vbsAttachContents(vb::CallContext& ctx);
	//	void vbsAddPathField(vb::CallContext& ctx);
	//	void vbsUpdatePathField(vb::CallContext& ctx);
	//	void vbsRemovePathField(vb::CallContext& ctx);
	//	void vbsStore(vb::CallContext& ctx);
	//	void vbsRestore(vb::CallContext& ctx);
		std::string luaGetWireText(lua_State* L) { return ToStr(m->GetWireText()); };
	//	void vbsSetWireText(vb::CallContext& ctx);
	
		CMessage* getCMsg() { return m; } 
	private:
		CDatum* getDatumByArg(lua_State* L, int arg);
		CMessage* m = nullptr;
	
	
	
	};

}

// CmbluaApp
// See mblua.cpp for the implementation of this class
//

class CmbluaApp : public CWinApp
{
public:
	CmbluaApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};
