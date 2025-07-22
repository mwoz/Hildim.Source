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
CString ToCStr(const char* str) {
	// Determine the length of the wide string          CP_UTF8
	int len = MultiByteToWideChar(
		CP_ACP, 0, str, -1, nullptr, 0);
	if (len == 0) {
		return CString();
	}

	// Convert the string
	std::wstring wide_str(len, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1,
		&wide_str[0], len);

	CString s(wide_str.c_str());
	return s;
}
CString ToCStr(std::string& str) {
	return ToCStr(str.c_str());
	//// Determine the length of the wide string          CP_UTF8
	//int len = MultiByteToWideChar(
	//	CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	//if (len == 0) {
	//	return CString();
	//}
	//
	//// Convert the string
	//std::wstring wide_str(len, 0);
	//MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1,
	//	&wide_str[0], len);
	//
	//CString s(wide_str.c_str());
	//return s;
}
CString luaH_CheckCString(lua_State* L, int arg) {
	const char* str = luaL_checkstring(L, arg);

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
		CString s = dt.Format(L"%Y-%m-%d %H:%M:%S");
		s.Replace(L" 00:00:00", L"");
		lua_pushstring(L, ToStr( s.GetBuffer()).c_str() );
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
	typedef RefCountedObjectPtr<luaMessage> RCMessage;
	class luaDatum;
	typedef RefCountedObjectPtr<luaDatum> RCDatum;

	class luaDatum : public RefCountedObject
	{
	public:
		luaDatum(CDatum* pd, bool add = true) { d = pd ? pd : new CDatum(); if (add) d->InternalAddRef(); };
		~luaDatum() {
			d->InternalRelease();
		};
		int luaGetValue(lua_State* L) { luaH_PushVariant(L, d->value()); return 1; }
		void luaSetValue(lua_State* L) { d->value(luaH_CheckVariant(L, 2)); }
		std::string luaGetName() const { return ToStr(d->id()); };
		std::string luaValueText() const { return ToStr(d ->GetValueText(0)); };
	private:
		CDatum* d = nullptr;
	};

	class luaMessage : public RefCountedObject
	{
	public: 
		luaMessage() { m = new CMessage(); };
		luaMessage(CMessage* pm, bool add = true) { m = pm ? pm : new CMessage(); if(add) m->InternalAddRef();  };
		~luaMessage() { 
			m->InternalRelease(); 
		};
		//void luaAddField(lua_State* L);
		std::string luaToString(lua_State* L) const{ return ToStr(m->ToString()); }
	
		int xCounts(lua_State* L);
		int xSubjects(lua_State* L);
		int xFieldValue(lua_State* L){
			CDatum* d = getDatumByArg(L, 2);
			Variant& v = d ? d->value() : Variant();
			return luaH_PushVariant(L, v); 
		}
		int xRemoveField(lua_State* L) { getDatumByArg(L, 2)->~CDatum(); return 0; }
			
		int xSetField(lua_State* L);
		int xFieldName(lua_State* L) { lua_pushstring(L, ToStr(getDatumByArg(L, 2)->id()).c_str()); return 1; }
		int xDestroy(lua_State* L) {  return 0; }
		int xFillList(lua_State* L);
		int xRSCounts(lua_State* L);
		int xRSGetRecord(lua_State* L);
		int xRSColumn(lua_State* L);
		int xSaveFieldBinary(lua_State* L);
		int xAddFieldBinary(lua_State* L);
		
		std::string luaGetFieldype(lua_State* L);
		std::string luaGetPathType(std::string path);
	
		std::string luaGetName() { return ToStr(m->id());} 

		int luaGetFieldCount() { return m->GetDataCount(); }
	
		int luaGetMessageCount(){ return m->GetMsgsCount();}
	
		std::string luaGetSendSubject(lua_State* L) const { return ToStr(m->m_strSendSubject); }
		void luaSetSendSubject(const char* v, lua_State* L) { m->m_strSendSubject = ToCStr(v); }
		std::string luaGetReplySubject(lua_State* L) const { return ToStr(m->m_strReplySubject); }
		void luaSetReplySubject(const char* v, lua_State* L) { m->m_strReplySubject = ToCStr(v); }
	//	void vbsGetRecordset(vb::CallContext& ctx);
	//	void vbsSetRecordset(vb::CallContext& ctx);
		void luaReset() { m->CleanUp(); };
		void luaAddField(std::string id, lua_State* L);
		void luaInsertField(std::string id, lua_State* L);
		void luaUpdateField(std::string id, lua_State* L);
		RCDatum luaRemoveField(lua_State* L);
		bool luaAddMessage(std::string sub, luaMessage* added);
		bool luaInsertMessage(std::string id, luaMessage* inserted, LONG lIndex);
		bool luaUpdateMessage(std::string id, luaMessage* updFrom);
		RCMessage luaRemoveMessage(lua_State* L);
		RCDatum luaField(int i, lua_State* L) const;
		RCMessage lua_Message(int Index) { 
			return RCMessage(new luaMessage(m->GetMsg(Index)));
		}
		int luaGetMessageIndex(std::string id);
		bool luaExistsField(std::string id){ return (m->GetDatum(ToCStr(id)) != NULL); }
		bool luaExistsMessage(std::string path) { return (m->GetMsg(ToCStr(path)) != NULL); };
		void luaCopyFrom(luaMessage* src) { m->AddContentFrom(src->getCMsg()); };
	    std::string luaGetFieldText(lua_State* L);
	//	void vbsGetFieldNumber(vb::CallContext& ctx);
	//	void vbsGetFieldDate(vb::CallContext& ctx);
	//	void vbsToString(vb::CallContext& ctx);
		RCMessage luaGetMessage(std::string path) { 
			return RCMessage(new luaMessage(m->AddMsgByPath(ToCStr(path))) );
		}
		RCMessage luaExecute(std::string param);
		bool luaAddHeadMessage(std::string id, luaMessage* added);
		RCMessage luaRemoveHeadMessage() { return RCMessage(new luaMessage(m->DetachHeadMsg(), false)); }
		RCMessage luaRemoveTailMessage() { return RCMessage(new luaMessage(m->DetachTailMsg(), false)); }
		void luaUpdateFrom(luaMessage* from) { m->AddContentFrom(from->getCMsg()); }
		int luaGetFieldValue(lua_State* L) {
			CDatum* d;
			return (d = m->GetDatum(luaH_CheckCString(L, 2))) ? luaH_PushVariant(L, d->value()) : 0;
		}
		int luaGetPathValue(lua_State* L) {
			CDatum* d;
			return (d = m->GetDatumByPath(luaH_CheckCString(L, 2))) ? luaH_PushVariant(L, d->value()) : 0;
		}
		void luaFlatMessage() { m->vbsFlatMessage(vb::CallContext()); };
		void luaAttachContents(luaMessage* from);
		void luaAddPathField(std::string path, lua_State* L) { m->AddDatumByPath(ToCStr(path), luaH_CheckVariant(L, 3)); }
		void luaUpdatePathField(std::string path, lua_State* L) { m->SetDatumByPath(ToCStr(path), luaH_CheckVariant(L, 3)); }
		RCDatum luaRemovePathField(std::string path) { return RCDatum(new luaDatum(m->DetachDatumByPath(ToCStr(path)))); }
	//	void vbsStore(vb::CallContext& ctx);
	//	void vbsRestore(vb::CallContext& ctx);
		std::string luaGetWireText(lua_State* L) { return ToStr(m->GetWireText()); };
		int luaSetWireText(lua_State* L);
	
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
