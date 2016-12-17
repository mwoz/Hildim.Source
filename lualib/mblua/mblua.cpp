// mblua.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "mblua.h"
#include "tlx_lib/tlxMessage.h"
#include "MessageBus/mbTransport.h"
#include "L_mbConnector.h"
#include "mblua_util.h"
#include "tlx_lib/syslog.h"
#include "msxml2.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
enum { SURROGATE_LEAD_FIRST = 0xD800 };
enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
enum { SURROGATE_TRAIL_LAST = 0xDFFF };

CmbTransport* mbTransport = NULL; 
CString m_strDaemon = "xxx";
CString m_strLan = "xxx";
CString m_strNetwork = "xxx";
CString m_strService = "xxx";

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CmbluaApp

BEGIN_MESSAGE_MAP(CmbluaApp, CWinApp)
END_MESSAGE_MAP()


// CmbluaApp construction

CmbluaApp::CmbluaApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CmbluaApp object

CmbluaApp theApp;


// CmbluaApp initialization

BOOL CmbluaApp::InitInstance()
{
	CWinApp::InitInstance();
	mbTransport = new CmbTransport();
	return TRUE;
}
int CmbluaApp::ExitInstance()
{
	CWinApp::ExitInstance();
	if(mbTransport != NULL)
	{
		delete mbTransport;
		::ExitProcess(0);
	}
	return 0;
}
/////////////////////////////////////////

int do_CreateMessage(lua_State* L)
{
	CMessage *msg = new CMessage();
	wrap_cmsg(L, msg);
	return 1;
}
int do_RestoreMessage(lua_State* L)
{
	CMessage *msg = new CMessage();
	CString path = luaL_checkstring(L,1);
	CFile f;
	CFileException e;
	if(!f.Open(path, CFile::modeRead | CFile::shareDenyWrite|CFile::typeBinary, &e))
	{	
		CString str;
		str.Format("mesage_Store:File could not be opened %d", e.m_cause);
		throw_L_error(L,str);
	}
	else
	{
		ULONGLONG s = f.GetLength();
		if(s > 4294967296)
		{
			CString str;
			str.Format("do_RestoreMessage:File %s too lage", path);
			throw_L_error(L,str);
		}
		DWORD dwSize = DWORD(s);
		BYTE* pByte = new BYTE[dwSize+20];
		try
		{
			dwSize = f.Read(pByte,dwSize);
		}
		catch(CFileException e)
		{
			CString str;
			str.Format("do_RestoreMessage:File could not be reader %d", e.m_cause);
			throw_L_error(L,str);
		}
		f.Close();
		msg->Restore(pByte,dwSize);
		delete[] pByte;
	}
	wrap_cmsg(L, msg);
	return 1;
}

int do_CreateMbTransport(lua_State* L)
{
	CString strDaemon = luaL_checkstring(L,1);
	CString strLan = luaL_checkstring(L,2);
	CString strNetwork = luaL_checkstring(L,3);
	CString strService = luaL_checkstring(L,4);
	if(m_strDaemon != strDaemon || m_strLan != strLan || m_strNetwork != strNetwork || m_strService != strService)
	{
		if( m_strLan != "xxx") ////xxx может быть только при перворм запуске - не делаем дисконнект
		{
			mbTransport->destroy();
		}
		m_strDaemon = strDaemon;
		m_strLan = strLan;
		m_strNetwork = strNetwork;
		m_strService = strService;
		mbTransport->create(strDaemon,strLan,strNetwork, strService);
	}
	return 0;
}
int do_Publish(lua_State* L)
{
	if( m_strLan != "xxx")
	{
		if(mbTransport->mbPublish(cmessage_arg(L, "do_Publish")) != MB_ERROR_OK)
		{
			//throw_L_error(L, "Error when publish message");
			lua_pushboolean(L, false);
		}
		else{
			lua_pushboolean(L, true);
		}
	}
	return 1;
}
int do_Subscribe(lua_State* L)
{
	if( m_strLan != "xxx")
	{
		CL_mbConnector *cn = new CL_mbConnector(mbTransport,L);
		cn->SetCallback();
		//mb_handle h = mbTransport->mbSubscribe(cn,luaL_checkstring(L,2),lua_touserdata(L,3));
		mb_handle h = mbTransport->mbSubscribe((CmbConnectorBase*)cn, luaL_checkstring(L, 2), lua_touserdata(L, 3));
		lua_pushlightuserdata(L,h);
		return 1;
	}
	return 0;
}
int do_Request(lua_State* L)
{
	if( m_strLan != "xxx")
	{
		CL_mbConnector *cn = new CL_mbConnector(mbTransport,L,true);
		cn->SetCallback();
		void* pOpaq = NULL;
		if(lua_type(L,4)!= LUA_TNIL)
			pOpaq=(void*)cmessage_arg(L, "do_Request",4);
		mb_handle h = mbTransport->mbRequest(cn,cmessage_arg(L, "do_Request",2),luaL_checkint(L,3),pOpaq);
		lua_pushlightuserdata(L,h);
		return 1;
	}
	return 0;
}
int do_UnSubscribe(lua_State* L)
{
    mbTransport->mbUnsubscribe((mb_handle)lua_touserdata(L,1));
	return 0;
}
int do_Destroy(lua_State* L)
{
	m_strLan = "xxx"; //ѕризнак отсутстви€ подписки
    mbTransport->destroy();
	return 0;
}
int do_CheckXML(lua_State* L)
{
	CString strXml = luaL_checkstring(L,1);

	HRESULT hr;
	IXMLDOMDocument * pXMLDoc;
	//...
	hr = CoInitialize(NULL); 
	// Check the return value, hr...
	hr = CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER, 
		   IID_IXMLDOMDocument, (void**)&pXMLDoc);
	// Check the return value, hr...
	VARIANT_BOOL bSuccessful;
	pXMLDoc->loadXML(strXml.AllocSysString(),&bSuccessful);
	int irez = 0;
	if(!bSuccessful)
	{
		IXMLDOMParseError *pIParseError = NULL;
		pXMLDoc->get_parseError(&pIParseError);

		long lineNum;
		pIParseError->get_line(&lineNum);
		
		long linePos;
		pIParseError->get_linepos(&linePos);

		BSTR reason;
		pIParseError->get_reason(&reason);

		lua_pushinteger(L, lineNum);
		lua_pushinteger(L, linePos);
		lua_pushstring(L, CString(reason));

		pIParseError->Release();
		pIParseError = NULL;
		irez = 3;

	}
	pXMLDoc->Release();

	return irez;
}
int mesage_GetName(lua_State* L)
{
	lua_pushstring(L, cmessage_arg(L, "Name")->id());
	return 1;
}
int mesage_ToString(lua_State* L)
{
	lua_pushstring(L, cmessage_arg(L, "mesage_ToString")->ToString());
	return 1;
}
int mesage_Subjects(lua_State* L)
{
	CString type2 = luaL_typename(L,2);
	CString type3 = luaL_typename(L,3);
	CMessage* msg = cmessage_arg(L,"mesage_Subjects");
	if(type2 == "string")
		msg->m_strSendSubject = luaL_checkstring(L,2);
	if(type3 == "string")
		msg->m_strReplySubject = luaL_checkstring(L,3);

	lua_pushstring(L, msg->m_strSendSubject);
	lua_pushstring(L, msg->m_strReplySubject);
	lua_pushstring(L, msg->id());
	return 3;
}
int mesage_Counts(lua_State* L)
{
	CMessage* msg = cmessage_arg(L, "mesage_Counts");
	lua_pushinteger(L, msg->GetDataCount());
	lua_pushinteger(L, msg->GetMsgsCount());
	return 2;
}

int mesage_SetPathValue(lua_State* L)
{
	CString type = luaL_typename(L,3);
	if(type == "string")
	{
		CString str = luaL_checkstring(L,3);
		COleDateTime dt;
		if(dt.ParseDateTime(str))
		{
			cmessage_arg(L,"mesage_SetPathValue")->SetDatumByPath(luaL_checkstring(L,2),COleVariant(dt));
		}else{
			cmessage_arg(L,"mesage_SetPathValue")->SetDatumByPath(luaL_checkstring(L,2),COleVariant(str));
		}
	}else if(type == "number")
	{
		cmessage_arg(L,"mesage_SetPathValue")->SetDatumByPath(luaL_checkstring(L,2),COleVariant(luaL_checknumber(L,3)));
	}else if(type == "boolean")
	{
		VARIANT var;
		var.vt = VT_BOOL;
		var.boolVal = (lua_toboolean(L,3)==1);
		cmessage_arg(L,"mesage_SetPathValue")->SetDatumByPath(luaL_checkstring(L,2),var);
	}else if(type == "nil")
	{
		VARIANT var;
		var.vt = VT_NULL;
		cmessage_arg(L,"mesage_SetPathValue")->SetDatumByPath(luaL_checkstring(L,2),var);
	}else
	{
		throw_L_error(L, "Invalid type of value");
	}
	return 0;
}
int mesage_GetPathValue(lua_State* L)
{
	CString path = luaL_checkstring(L,2);
	CMessage* msg = cmessage_arg(L,"mesage_GetPathValue");
	CDatum* d = msg->GetDatumByPath(path);
	if(!d) return 0;
	switch(d->GetVarType())
	{
	case VT_R8:
		{
			double i;
			d->GetValueAsDouble(i);
			lua_pushnumber(L, i);
		}
		break;
	case VT_I4:
		{
			long i;
			d->GetValueAsLong(i);
			lua_pushinteger(L, i);
		}
		break;
	case VT_BOOL:
		{
			bool i;
			d->GetValueAsBool(i);
			lua_pushboolean(L, i);
		}
		break;
	case VT_BSTR:
	case VT_DATE:
		lua_pushstring(L, d->GetValueText());
		break;
	default:
		lua_pushnil(L);
	}
	return 1;
}


int mesage_Store(lua_State* L)
{
	CString path = luaL_checkstring(L,2);
	CMessage* msg = cmessage_arg(L,"mesage_Store");
	DWORD dwSize;
	BYTE *pData = (BYTE*)msg->Store(dwSize);
	CFile f;
	CFileException e;
	if(!f.Open(path, CFile::modeCreate | CFile::modeWrite|CFile::typeBinary, &e))
	{	
		CString str;
		str.Format("mesage_Store:File could not be opened %d", e.m_cause);
		throw_L_error(L,str);
	}
	else
	{
		f.Write(pData,dwSize);
		f.Close();
	}
	return 0;
}
int mesage_Destroy(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_Destroy");
	delete msg;
	return 0;
}
int mesage_GetMessage(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_RemoveMessage");
	CString path = luaL_checkstring(L,2);
	CMessage* msgOut = msg->AddMsgByPath(path);
	if(msgOut)
	{
		wrap_cmsg(L, msgOut);
		return 1;
	}
	return 0;
}
int mesage_Message(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_Message");
	int num = luaL_checkint(L,2);
	CMessage* msgOut = msg->GetMsg(num);
	if(msgOut)
	{
		wrap_cmsg(L, msgOut);
		return 1;
	}
	return 0;
}
int mesage_Execute(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_Execute");
	CString cmd = luaL_checkstring(L,2);
	CMessage* msgOut = new CMessage();
	if(msg->ExecCommand(cmd,NULL,msgOut))
	{
		wrap_cmsg(L, msgOut);
		return 1;
	}
	delete msgOut;
	return 0;
}
int mesage_RemoveMessage(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_RemoveMessage");
	CString path = luaL_checkstring(L,2);
	CMessage* msgOut = msg->DetachMsgByPath(path);
	if(msgOut)
	{
		wrap_cmsg(L, msgOut);
		return 1;
	}
	return 0;
}
int mesage_AttachMessage(lua_State* L)
{	
	CMessage* msg = cmessage_arg(L,"mesage_AttachMessage");
	CString subName = luaL_checkstring(L,2);
	CMessage* subMsg = cmessage_arg(L,"mesage_AttachMessage",3);
	subMsg->id(subName);
	msg->AttachMsg(subMsg);
	return 0;
}
int mesage_CopyFrom(lua_State* L)
{	
	CMessage* msg = cmessage_arg(L,"mesage_CopyFrom");
	CMessage* subMsg = cmessage_arg(L,"mesage_CopyFrom",2);

	msg->AddContentFrom(subMsg);
	return 0;
}
int mesage_ExistsMessage(lua_State* L)
{	
	CMessage* msg = cmessage_arg(L,"mesage_ExistsMessage");
	CString subName = luaL_checkstring(L,2);
	lua_pushboolean(L, (msg->GetMsg(subName)!=NULL));
	return 1;
}
static const struct luaL_reg mblua[] = {
	{"CreateMessage",do_CreateMessage},
	{"RestoreMessage",do_RestoreMessage},
	{"CreateMbTransport",do_CreateMbTransport},
	{"Publish",do_Publish},
	{"Subscribe", do_Subscribe},
	{"UnSubscribe", do_UnSubscribe},
	{"Request", do_Request},
	{"Destroy",do_Destroy},
	{"CheckXML",do_CheckXML},
	{NULL, NULL},
};
static const struct luaL_reg message_methods[] = {
	{"ToString",mesage_ToString},
	{"Subjects",mesage_Subjects},//Send, replay -  и в аргументах и в результате
	{"Counts",mesage_Counts},//FieldCount, MessageCount
	{"SetPathValue",mesage_SetPathValue},
	{"GetPathValue",mesage_GetPathValue},
	{"Store",mesage_Store},
	{"Destroy", mesage_Destroy},
	{"GetMessage",mesage_GetMessage},
	{"Message",mesage_Message},
	{"Execute",mesage_Execute},
	{"RemoveMessage",mesage_RemoveMessage},
	{"AttachMessage",mesage_AttachMessage},
	{"ExistsMessage",mesage_ExistsMessage},
	{ "Name", mesage_GetName },
	//{ "CopyFrom", mesage_CopyFrom },
	//{ "CopyFrom", mesage_CopyFrom },
	{NULL, NULL},
};

extern "C" __declspec(dllexport)
int luaopen_mblua(lua_State *L)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	// at this point, the SciTE window is available. Can't always assume
	// that it is the foreground window, so we hunt through all windows
	// associated with this thread (the main GUI thread) to find a window
	// matching the appropriate class name
	sysLOGInit("", "", -1);
	luaL_openlib (L, "mblua", mblua, 0);
	luaL_newmetatable(L, MESSAGEOBJECT);  // create metatable for window objects
	lua_pushvalue(L, -1);  // push metatable
	lua_setfield(L, -2, "__index");  // metatable.__index = metatable
	luaL_register(L, NULL, message_methods);
	return 1;
}
