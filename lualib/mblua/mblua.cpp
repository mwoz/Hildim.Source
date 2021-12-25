// mblua.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TLX_LIB/CrashStat.h"
#include "mblua.h"
#include "TLX_LIB/tlxMessage.h"
#include "MessageBus/mbTransport.h"
#include "L_mbConnector.h"
#include "mblua_util.h"
#include "tlx_lib/syslog.h"
#include "msxml2.h"
#include <string>
#include <io.h>  
#include <fcntl.h>  
#include <activscp.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "../../iup/include/iup.h"
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

class CSimpleScriptSite :
	public IActiveScriptSite,
	public IActiveScriptSiteWindow
{
public:
	CSimpleScriptSite(lua_State* pL) : m_cRefCount(1), m_hWnd(NULL), L(pL) {}

	// IUnknown

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

	// IActiveScriptSite

	STDMETHOD(GetLCID)(LCID *plcid) { *plcid = 0; return S_OK; }
	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti) { return TYPE_E_ELEMENTNOTFOUND; }
	STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion) { *pbstrVersion = SysAllocString(L"1.0"); return S_OK; }
	STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo) { return S_OK; }
	STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState) { return S_OK; }
	STDMETHOD(OnScriptError)(IActiveScriptError *pError) {
		DWORD dwCookie;
		long lChar;
		ULONG ulLineNo;
		BSTR bstrSource = NULL;
		EXCEPINFO ei;

		pError->GetSourcePosition(&dwCookie, &ulLineNo, &lChar);
		pError->GetSourceLineText(&bstrSource);
		pError->GetExceptionInfo(&ei);
		lua_pushstring(L, CString(ei.bstrSource));
		lua_pushstring(L, CString(ei.bstrDescription));
		lua_pushinteger(L, ulLineNo + 1);
		lua_pushinteger(L, lChar);
		isError = true;
		return S_OK; 
	
	}
	STDMETHOD(OnEnterScript)(void) { return S_OK; }
	STDMETHOD(OnLeaveScript)(void) { return S_OK; }

	// IActiveScriptSiteWindow

	STDMETHOD(GetWindow)(HWND *phWnd) { *phWnd = m_hWnd; return S_OK; }
	STDMETHOD(EnableModeless)(BOOL fEnable) { return S_OK; }

	// Miscellaneous

	HRESULT SetWindow(HWND hWnd) { m_hWnd = hWnd; return S_OK; }

public:
	LONG m_cRefCount;
	HWND m_hWnd;
	bool isError = false;
	lua_State* L;
};

STDMETHODIMP_(ULONG) CSimpleScriptSite::AddRef() {
	return InterlockedIncrement(&m_cRefCount);
}

STDMETHODIMP_(ULONG) CSimpleScriptSite::Release() {
	if (!InterlockedDecrement(&m_cRefCount)) {
		delete this;
		return 0;
	}
	return m_cRefCount;
}

STDMETHODIMP CSimpleScriptSite::QueryInterface(REFIID riid, void **ppvObject) {
	if (riid == IID_IUnknown || riid == IID_IActiveScriptSiteWindow) {
		*ppvObject = (IActiveScriptSiteWindow *)this;
		AddRef();
		return NOERROR;
	}
	if (riid == IID_IActiveScriptSite) {
		*ppvObject = (IActiveScriptSite *)this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}


static int do_CheckVbScript(lua_State* L) {
	HRESULT hr;
	CLSID clsid;
	char* strerror = NULL;
	// Active Scripting
	hr = CLSIDFromProgID(L"VBScript", &clsid);
	if (FAILED(hr)) {
		return 0;
	}

	CSimpleScriptSite* pScriptSite = new CSimpleScriptSite(L);
	CComPtr<IActiveScript> spVBScript;
	CComPtr<IActiveScriptParse> spVBScriptParse;
	hr = spVBScript.CoCreateInstance(OLESTR("VBScript"));
	if (FAILED(hr)) {
		strerror = "Internal Error 1";
		goto err;
	}
	hr = spVBScript->SetScriptSite(pScriptSite);
	if (FAILED(hr)) {
		strerror = "Internal Error 2";
		goto err;
	}

	hr = spVBScript->QueryInterface(&spVBScriptParse);
	if (FAILED(hr)) {
		return 0;
	}
	hr = spVBScriptParse->InitNew();
	if (FAILED(hr)) {
		strerror = "Internal Error 3";
		goto err;
	}

	EXCEPINFO ei = { };

	const char *code = lua_tostring(L, 1);

	int result_u;
	result_u = MultiByteToWideChar(CP_ACP, 0, code, -1, 0, 0);
	if (!result_u)
		return 0;

	wchar_t *wcode = new wchar_t[result_u];
	if (!MultiByteToWideChar(CP_ACP, 0, code, -1, wcode, result_u)) {
		delete[] wcode;
		strerror = "Internal Error 4";
		goto err;
	}
	//LPOLESTR wcode2 = OLESTR("MsgBox \"Hello World! The current time is: \"");
	hr = spVBScriptParse->ParseScriptText(wcode, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
	//hr = spVBScriptParse->ParseScriptText(OLESTR("MsgBox \"Hello World! The current time is: \" & Now"), NULL, NULL, NULL, 0, 0, 0, &result, &ei);
	bool isErr = pScriptSite->isError;
	spVBScriptParse = NULL;
	spVBScript = NULL;
	pScriptSite->Release();
	pScriptSite = NULL;

	delete[] wcode;

	if (isErr)
		return 4;

	if (FAILED(hr)) {
		strerror = "Internal Error 5";
		goto err;
	}
	return 0;
err: 
	lua_pushstring(L, strerror);
	return 1;
}

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
		mbTransport = NULL;
		//::ExitProcess(0);
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
int do_CreateMessageLite(lua_State* L)
{
	CMessage *msg = new CMessage();
	wrap_cmsg(L, msg);
	lua_pushlightuserdata(L, (void*)msg);
	return 1;
}

int do_GetGuid(lua_State* L) 	{
	CString s = mbTransport->mbCreateInbox(false);
	s.Replace("_INBOX.", "");
	lua_pushfstring(L, s);
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
		catch(CFileException& e)
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
		if( m_strLan != "xxx") ////xxx ����� ���� ������ ��� ������� ������� - �� ������ ����������
		{
			mbTransport->destroy();
		}
		m_strDaemon = strDaemon;
		m_strLan = strLan;
		m_strNetwork = strNetwork;
		m_strService = strService;
		mbTransport->setAppName(_T("HildiM"));
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
		lua_pushlightuserdata(L,(void*)h);
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
		CMessage *msg = NULL;
		MsgWrap *wrp = (MsgWrap*)luaL_testudata(L, 4, MESSAGEOBJECT);
		if (wrp) {
			msg = wrp->msg;
			msg->InternalAddRef();
		}

		mb_handle h = mbTransport->mbRequest(cn,cmessage_arg(L, "do_Request",2),luaL_checkinteger(L,3), (void*)msg);
		lua_pushlightuserdata(L,(void*)h);
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
	m_strLan = "xxx"; //������� ���������� ��������
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
int message_GetWireText(lua_State* L)
{
	lua_pushstring(L, (CString)cmessage_arg(L, "mesage_GetWireText")->GetWireText());
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
int mesage_Reset(lua_State* L) {
	CMessage* msg = cmessage_arg(L, "mesage_Counts");
	if (msg)
		msg->CleanUp();
	return 0;
}
int mesage_Counts(lua_State* L)
{
	CMessage* msg = cmessage_arg(L, "mesage_Counts");
	lua_pushinteger(L, msg->GetDataCount());
	lua_pushinteger(L, msg->GetMsgsCount());
	return 2;
}

int mesage_SetField(lua_State* L) {
	CString type = luaL_typename(L, 3);
	CMessage* msg = cmessage_arg(L, "mesage_SetField");
	CDatum* d;
	COleVariant v;

	if (type == "string") {
		v = luaL_checkstring(L, 3);

	} else if (type == "number") {
		v = luaL_checknumber(L, 3);
	} else if (type == "boolean") {
		v.boolVal = (lua_toboolean(L, 3) == 1);
	} else if (type == "nil") {
		v.vt = VT_NULL;
	} else if ( type == "no value") {
		v.vt = VT_EMPTY;
	} else {
		throw_L_error(L, "Invalid type of value");
	}


	type = luaL_typename(L, 2);
	if (type == "string") 
		d = msg->SetDatum(luaL_checkstring(L, 2), v);
	else if (type == "number" || (type == "integer")) {
		d = msg->GetDatum(lua_tointeger(L, 2));
		if (d)
			d->value(v);
		else
			throw_L_error(L, "Index not exist");
	}
	else {
		throw_L_error(L, "Invalid type of index");
	}
	return 0;
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
int mesage_FieldType(lua_State* L){
	CString type = luaL_typename(L, 2);
	CMessage* msg = cmessage_arg(L, "mesage_Field");
	CDatum* d;
	if (type == "string")
		d = msg->GetDatum(luaL_checkstring(L, 2));
	else
		d = msg->GetDatum(luaL_checkinteger(L, 2));

	if (!d) return 0;	 
	lua_pushstring(L, d->GetVarTypeText());
	return 1;

}
int mesage_FieldName(lua_State* L){
	CMessage* msg = cmessage_arg(L, "mesage_Field");
	int i = luaL_checkinteger(L, 2);
	CDatum* d = msg->GetDatum(i);
	if (!d) return 0;
	lua_pushstring(L, d->id());
	return 1;
}
int mesage_Field(lua_State* L){
	CString type = luaL_typename(L, 2);
	CMessage* msg = cmessage_arg(L, "mesage_Field");
	CDatum* d;
	if(type == "string")
		d = msg->GetDatum(luaL_checkstring(L, 2));
	else
		d = msg->GetDatum(luaL_checkinteger(L, 2));

	if (!d) return 0;
	switch (d->GetVarType())
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
	case VT_DATE:
		{
			ATL::COleDateTime dt;
			d->GetValueAsDate(dt);
			lua_pushstring(L, dt.Format("%Y-%m-%d %H:%M:%S").GetBuffer());
		}
		break;
	case VT_BSTR:
		lua_pushstring(L, d->GetValueText());
		break;
	default:
		lua_pushnil(L);
	}
	return 1;
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
	case VT_DATE:
		{
			ATL::COleDateTime dt;
			d->GetValueAsDate(dt);
			lua_pushstring(L, dt.Format("%Y-%m-%d %H:%M:%S").GetBuffer());
		}
		break;
	case VT_BSTR:
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
int mesage_gc(lua_State* L) {
	cmesage_gc(L);
	return 0;
}

int mesage_Destroy(lua_State* L)
{
	cmesage_Destroy(L);
	return 0;
}

 typedef void _PF(void* ,int, int, const char*);

 int mesage_RelecordsetCounts(lua_State* L)
{
	 CMessage* msg = cmessage_arg(L, "mesage_RelecordsetCounts");
	 CRecordset* rs = msg->GetRecordset();
	 if (!rs)
		 return 0;
	 lua_pushinteger(L, rs->GetColumnCount());
	 lua_pushinteger(L, rs->GetRecordCount());
	 return 2;
}
 int mesage_RelecordsetColumn(lua_State* L)
{
	 CMessage* msg = cmessage_arg(L, "mesage_RelecordsetColumn");
	 CRecordset* rs = msg->GetRecordset();
	 if (!rs)
		 return 0;
	 int col = luaL_checkinteger(L, 2);
	 lua_pushstring(L, rs->GetColumn(col)->GetName());
	 return 1;
}
 int mesage_RelecordsetGetRecord(lua_State* L)
{
	 CMessage* msg = cmessage_arg(L, "mesage_RelecordsetGetRecord");
	 CRecordset* rs = msg->GetRecordset();
	 if (!rs)
		 return 0;
	 int row = luaL_checkinteger(L, 2);

	 CString strId;
	 COleVariant value;
	 CRecordsetReader reader = rs->CreateReader();
	 for (int i = 0; i <= row; i++) {
		 if (!reader.MoveNext())
			 return 0;
	 }

	 lua_createtable(L, rs->GetColumnCount(), 0);

	 char* type;
	 for (int i = 0; i < rs->GetColumnCount(); i++) {
		 lua_pushinteger(L, i + 1);
		 lua_createtable(L, 0, 3);

		 reader.GetValue(i, value);
		 switch (value.vt) {
		 case VT_R8:
		 {
			 type = "R8";
			 lua_pushnumber(L, value.dblVal);
		 }
		 break;
		 case VT_I4:
		 {
			 type = "I4";
			 lua_pushinteger(L, value.lVal);
		 }
		 break;
		 case VT_BOOL:
		 {
			 type = "BOOL";
			 lua_pushboolean(L, (value.boolVal == VARIANT_TRUE));
		 }
		 break;
		 case VT_DATE:
		 {
			 type = "DATE";
			 ATL::COleDateTime dt = value.date;
			 lua_pushstring(L, dt.Format("%Y-%m-%d %H:%M:%S").GetBuffer());
		 }
		 break;
		 case VT_BSTR:
		 {
			 type = "BSTR";
			 CStringEx str = value.bstrVal;
			 lua_pushstring(L, str);
		 }
			 break;
		 default:
			 type = "NULL";
			 lua_pushnil(L);
		 }
		 lua_setfield(L, -2, "value");
		 
		 lua_pushstring(L, rs->GetColumn(i)->GetName());
		 lua_setfield(L, -2, "title");

		 lua_pushstring(L, type);
		 lua_setfield(L, -2, "type");

		 lua_settable(L, -3);

	 }
	 return 1;
}

 int mesage_Relecordset2Msg(lua_State* L) {
	 CMessage* msg = cmessage_arg(L, "mesage_Relecordset2Msg");
	 CRecordset* rs = msg->GetRecordset();
	 if (!rs)
		 return 0;

	 CMessage* pMsg = new CMessage();
	 CString strId;
	 COleVariant value; 

	 CRecordsetReader reader = rs->CreateReader();
	 while (reader.MoveNext()) {
		 CMessage* pRowMsg = new CMessage();
		 strId.Format(_T("%i"), pMsg->GetMsgsCount());
		 pRowMsg->id(strId);

		 for (int i = 0; i < rs->GetColumnCount(); i++) {
			 reader.GetValue(i, value);
			 pRowMsg->AddDatum(rs->GetColumn(i)->GetName(), value);
		 }
		 pMsg->AttachMsg(pRowMsg);
	 }
	 wrap_cmsg(L, pMsg);
	 return 1;
 }

 void SetBstrCell(_PF *pFunc, void* pList, int i, int j, bool bUnic, CStringEx& sVal) {
	 if (bUnic) {
		 //const char * str = d->GetValueText();
		 int result_u, result_c;
		 result_u = MultiByteToWideChar(CP_ACP, 0, sVal, -1, 0, 0);
		 if (!result_u) 
			 return ; 

		 wchar_t *ures = new wchar_t[result_u];
		 if (!MultiByteToWideChar(CP_ACP, 0, sVal, -1, ures, result_u)) {
			 delete[] ures;
			 return;
		 }
		 result_c = WideCharToMultiByte(65001, 0, ures, -1, 0, 0, 0, 0);
		 if (!result_c) {
			 delete[] ures;
			 return;
		 }
		 char *cres = new char[result_c];
		 if (!WideCharToMultiByte(65001, 0, ures, -1, cres, result_c, 0, 0)) {
			 delete[] cres;
			 return;
		 }
		 delete[] ures;
		 pFunc(pList, i + 1, j + 1, cres);
		 delete[] cres;
	 } else {
		 pFunc(pList, i + 1, j + 1, sVal);
	 }
 }

int mesage_FillList(lua_State* L) {
	CMessage* msg = cmessage_arg(L, "mesage_FillList");

    luaL_checkudata(L, 2, "iupHandle");

	int firstLine = luaL_checkinteger(L, 3);
	int firstCol = luaL_checkinteger(L, 4);
	bool setNum = false;
	if (firstCol < 0) {
		firstCol = firstCol * -1;
		setNum = true;
	}
	bool setNull = false;

	if(!lua_istable(L, 5))
		throw_L_error(L, "mesage_FillList:Argument 5 isn't a table");

	int nFields = luaL_len(L, 5);
	int *fMap = new int[nFields];
	bool *fUtf = new bool[nFields];

	for (int i = 0; i < nFields; i++) {
		lua_rawgeti(L, 5, i + 1);
		fMap[i] = luaL_checkinteger(L, -1);
	}

	if (lua_istable(L, 6)) {
		if(nFields != luaL_len(L, 6))
			throw_L_error(L, "mesage_FillList:Tables 5 and 6 have different lengths");
		
		for (int i = 0; i < nFields; i++) {
			lua_rawgeti(L, 6, i + 1);
			fUtf[i] = (luaL_checkinteger(L, -1) != 0);
		}
	} else {
		bool b = lua_toboolean(L, 6);
		for (int i = 0; i < nFields; i++) {
			fUtf[i] = b;
		}

	}
	if (lua_isboolean(L, 7))
		setNull = lua_toboolean(L, 7);

	lua_getglobal(L, "scite");
	lua_getfield(L, -1, "GetListHandlers");
	lua_pushvalue(L, 2);
	if(lua_pcall(L, 1, 2, 0) != LUA_OK)
		throw_L_error(L, "mesage_FillList:Internal error");

	_PF *pFunc = (_PF*)lua_touserdata(L, -2);
	void* pList = lua_touserdata(L, -1);

	int msgCnt;
	CRecordset* rs = msg->GetRecordset();
	if (rs) 
		msgCnt = rs->GetRecordCount();
	else
		msgCnt = msg->GetMsgsCount();

	lua_settop(L, 2);
	lua_getglobal(L, "iup");
	lua_getfield(L, -1, "GetAttribute");
	lua_pushvalue(L, 2);
	lua_pushstring(L, "NUMCOL");

	if (lua_pcall(L, 2, 1, 0) != LUA_OK)
		throw_L_error(L, "mesage_FillList:Internal error");


	const char* pCh;
	pCh = lua_tostring(L, -1);
	int curCol = atoi(pCh);
	if(curCol < nFields)
		throw_L_error(L, "mesage_FillList:not enough columns");


	lua_settop(L, 3);
	lua_getfield(L, -1, "GetAttribute");
	lua_pushvalue(L, 2);
	lua_pushstring(L, "NUMLIN");

	if (lua_pcall(L, 2, 1, 0) != LUA_OK)
		throw_L_error(L, "mesage_FillList:Internal error");
	pCh = lua_tostring(L, -1);
	
	
	char buf[256];

	lua_settop(L, 3);
	lua_getfield(L, -1, "SetAttribute");
	lua_pushvalue(L, 2);
	lua_pushstring(L, "DELLIN");

	strcpy(buf, "1-");
	strcat(buf, pCh);
	lua_pushstring(L, buf);
	if (lua_pcall(L, 3, 0, 0) != LUA_OK)
		throw_L_error(L, "mesage_FillList:Internal error");

	lua_settop(L, 3);
	lua_getfield(L, -1, "SetAttribute");
	lua_pushvalue(L, 2);
	lua_pushstring(L, "ADDLIN");
	strcpy(buf, "1-");
	itoa(msgCnt, buf + 2, 10); 
	lua_pushstring(L, buf);
	if (lua_pcall(L, 3, 0, 0) != LUA_OK)
		throw_L_error(L, "mesage_FillList:Internal error");

	if (rs) {
		CRecordsetReader reader = rs->CreateReader();
		COleVariant value;
		for (int i = 0; i < msgCnt; i++) {
			if (setNum) {
				_ltoa(i + firstLine, buf, 10);
				pFunc(pList, i + firstLine, 0, buf);
			}
			reader.MoveNext();
			for (int j = 0; j < nFields; j++) {
				reader.GetValue(fMap[j] - 1, value);

				const char *out = NULL;

				switch (value.vt) {
				case VT_R8:
				{
					sprintf(buf, "%.9f", value.dblVal);
					out = buf;
				}
				break;
				case VT_I4:
				{
					_ltoa(value.lVal, buf, 10);
					out = buf;
				}
				break;
				case VT_BOOL:
				{
					out = value.boolVal ? "<true>" : "<false>";
				}
				break;
				case VT_DATE:
				{
					ATL::COleDateTime dt = value.date;
					strcpy(buf, dt.Format("%Y-%m-%d %H:%M:%S").GetBuffer());
					out = buf;
				}
				break;
				case VT_BSTR:
				{
					SetBstrCell(pFunc, pList, i, j, fUtf[j], CStringEx(value.bstrVal));
				}
				break;
				case VT_NULL:
					if (setNull)
						out = "<null>";
					break;
				}
				if (out)
					pFunc(pList, i + firstLine, j + firstCol, out);
			}
		}
	} else {

		for (int i = 0; i < msgCnt; i++) {
			if (setNum) {
				_ltoa(i + firstLine, buf, 10);
				pFunc(pList, i + firstLine, 0, buf);
			}
			for (int j = 0; j < nFields; j++) {

				CDatum *d = msg->GetMsg(i)->GetDatum(fMap[j] - 1);

				const char *out = NULL;
				if (d) {
					switch (d->GetVarType()) {
					case VT_R8:
					{
						double v;
						d->GetValueAsDouble(v);
						sprintf(buf, "%.9f", v);
						out = buf;
					}
					break;
					case VT_I4:
					{
						long v;
						d->GetValueAsLong(v);
						_ltoa(v, buf, 10);
						out = buf;
					}
					break;
					case VT_BOOL:
					{
						bool v;
						d->GetValueAsBool(v);
						out = v ? "<true>" : "<false>";
					}
					break;
					case VT_DATE:
					{
						ATL::COleDateTime dt;
						d->GetValueAsDate(dt);
						strcpy(buf, dt.Format("%Y-%m-%d %H:%M:%S").GetBuffer());
						out = buf;
					}
					break;
					case VT_BSTR:
						SetBstrCell(pFunc, pList, i, j, fUtf[j], d->GetValueText());
						break;
					case VT_NULL:
						if (setNull)
							out = "<null>";
						break;
					}
				}
				if (out)
					pFunc(pList, i + firstLine, j + firstCol, out);
			}
		}
	}
	delete[] fMap;
	delete[] fUtf; 
	
	return 0;
}

int mesage_GetMessage(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_GetMessage");
	CString path = luaL_checkstring(L,2);
	CMessage* msgOut = msg->AddMsgByPath(path);
	if(msgOut)
	{
		msgOut->InternalAddRef();
		wrap_cmsg(L, msgOut);
		return 1;
	}
	return 0;
}
int mesage_Message(lua_State* L)
{
	CMessage* msg = cmessage_arg(L,"mesage_Message");
	int num = luaL_checkinteger(L,2);
	CMessage* msgOut = msg->GetMsg(num);
	if(msgOut)
	{
		msgOut->InternalAddRef();
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
		msgOut->InternalAddRef();
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

int mesage_CopyFrom(lua_State* L) {
	CMessage* msg = cmessage_arg(L, "mesage_CopyFrom");
	CMessage* subMsg = cmessage_arg(L, "mesage_CopyFrom", 2);

	msg->AddContentFrom(subMsg);
	return 0;
}

int mesage_SaveFieldBinary(lua_State* L) {
	int err = 0;
	char *b = NULL;
	CMessage* msg = cmessage_arg(L, "mesage_AddFieldBinary");
	CString fldName = luaL_checkstring(L, 2);
	CString path = luaL_checkstring(L, 3);
	char *description;
	DWORD l;

	CDatum *d = msg->GetDatum(fldName);
	if (d->GetVarType() != (VT_ARRAY | VT_UI1) ) {
		err = -1;
		description = "Not Binary Data";
		goto err;
	}
	{
		l = d->GetDataSize();
		b = new char[l];


		SAFEARRAY* pArray = d->value().parray;
		ASSERT(pArray->cDims == 1); // check we have 1 dimension array
		ASSERT(l == pArray->rgsabound[0].cElements * pArray->cbElements); // get size of array
		memcpy(b, (BYTE*)pArray->pvData, l);

		int charsLen = ::MultiByteToWideChar(CP_UTF8, 0, path, lstrlen(path), NULL, 0);
		std::wstring characters(charsLen, '\0');
		::MultiByteToWideChar(CP_UTF8, 0, path, lstrlen(path), &characters[0], charsLen);

		int pf;

		err = _wsopen_s(&pf, characters.c_str(), _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, _SH_DENYRW, _S_IWRITE);
		if (err) {
			description = "Open File Error";
			goto err;
		}
		if (l != _write(pf, (b), l)) {
			err = -2;
			description = "Write File Error";
			goto err;
		}

		err = _close(pf);
		if (err){
			description = "Close File Error";
			goto err;
		}
	}
err:
	if (b)
		delete[]b;
	lua_pushinteger(L, err);
	if (err)
		lua_pushstring(L, description);
	else
		lua_pushinteger(L, l);
	return 2;
}

int mesage_AddFieldBinary(lua_State* L) {
	CMessage* msg = cmessage_arg(L, "mesage_AddFieldBinary");
	CString fldName = luaL_checkstring(L, 2);
	CString path = luaL_checkstring(L, 3); 
	int err;
	char *description;

	int charsLen = ::MultiByteToWideChar(CP_UTF8, 0, path, lstrlen(path), NULL, 0);
	std::wstring characters(charsLen, '\0');
	::MultiByteToWideChar(CP_UTF8, 0, path, lstrlen(path), &characters[0], charsLen);

	int pf;
	char *b = NULL;
	err = _wsopen_s(&pf, characters.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD);
	if (err) {
		description = "Open File error";
		goto err;
	}

	DWORD l = _filelength(pf);

	b = new char[l];


	if (l != _read(pf, b, l)) {
		err = -1;
		description = "Read File error";
		goto err;
	}
	err = _close(pf);
	if (err) {
		description = "Close File error";
		goto err;
	}
	{
		COleSafeArray arr;
		arr.Create(VT_UI1, 1, &l);

		for (DWORD i = 0; i < l; i++) {

			arr.PutElement((long*)&i, &b[i]);
		}

		msg->AddDatum(fldName, arr);
	}

err:
	if (b)
		delete []b;
	lua_pushinteger(L, err);
	if (err)
		lua_pushstring(L, description);
	else
		lua_pushinteger(L, l);
	return 2;
}

int mesage_ExistsMessage(lua_State* L)
{	
	CMessage* msg = cmessage_arg(L,"mesage_ExistsMessage");
	CString subName = luaL_checkstring(L,2);
	lua_pushboolean(L, (msg->GetMsg(subName)!=NULL));
	return 1;
}
luaL_Reg mblua[] = {
	{"CreateMessage",do_CreateMessage},
	{"RestoreMessage",do_RestoreMessage},
	{"CreateMbTransport",do_CreateMbTransport},
	{"Publish",do_Publish},
	{"Subscribe", do_Subscribe},
	{"UnSubscribe", do_UnSubscribe},
	{"Request", do_Request},
	{"Destroy",do_Destroy},
	{ "CheckXML",do_CheckXML },
	{"GetGuid",do_GetGuid },
	{"CheckVbScript",do_CheckVbScript },
	{NULL, NULL},
};
luaL_Reg message_methods[] = {
	{"ToString",mesage_ToString},
	{"GetWireText",message_GetWireText},
	{"Subjects",mesage_Subjects},//Send, replay -  � � ���������� � � ����������
	{"Counts",mesage_Counts},//FieldCount, MessageCount
	{"SetPathValue",mesage_SetPathValue},
	{"GetPathValue", mesage_GetPathValue },
	{"Field", mesage_Field },
	{"SetField", mesage_SetField },
	{"FieldType", mesage_FieldType },
	{"FieldName", mesage_FieldName },
	{"Store",mesage_Store},
	{"Destroy", mesage_Destroy},
	{"GetMessage",mesage_GetMessage},
	{"Message",mesage_Message},
	{"Execute",mesage_Execute},
	{"Reset",mesage_Reset},
	{"RemoveMessage",mesage_RemoveMessage},
	{"AttachMessage",mesage_AttachMessage},
	{"ExistsMessage",mesage_ExistsMessage},
	{ "Name", mesage_GetName },
	{ "CopyFrom", mesage_CopyFrom },
	{ "AddFieldBinary", mesage_AddFieldBinary },
	{ "SaveFieldBinary", mesage_SaveFieldBinary },
	{ "FillList", mesage_FillList },
	{ "RS2Msg", mesage_Relecordset2Msg },
	{ "RSCounts", mesage_RelecordsetCounts },
	{ "RSColumn", mesage_RelecordsetColumn },
	{ "RSGetRecord", mesage_RelecordsetGetRecord },
	{"__gc", mesage_gc},
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
	luaL_newmetatable(L, MESSAGEOBJECT);  // create metatable for window objects
	lua_pushvalue(L, -1);  // push metatable
	lua_setfield(L, -2, "__index");  // metatable.__index = metatable
	luaL_setfuncs(L, message_methods, 0);

	lua_newtable(L);
	luaL_setfuncs(L, mblua, 0);
	return 1;
}
