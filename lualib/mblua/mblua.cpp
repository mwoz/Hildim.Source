// mblua.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "Utils/CrashStat.h"
#include "mblua.h"
//#include "Containers/tlxMessage.h"
//#include "MessageBus/mbTransport.h"
//#include "L_mbConnector.h"
#include "mblua_util.h"
#include "Utils/syslog.h"
#include <string>
#include <io.h>  
#include <fcntl.h>  
#include "VbEngine\VbLexer.h"
#include "VbEngine\VbRuntime.h"
#include <functional>
#include <sstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif                                               

CmbTransport* mbTransport = NULL; 
CString m_strDaemon = "xxx";
CString m_strLan = "xxx";
CString m_strNetwork = "xxx";
CString m_strService = "xxx";


class SysLogCalback : public IsysLogCallback
{
public:
	SysLogCalback() {
	}
	void StopLogging(lua_State* L, bool bPrint) {
		sysLOGProfiler()->SetLogLevel(-1);
		if (bPrint) {
			lua_getglobal(L, "print");
			lua_pushstringW(L, ssOut.str().c_str());
			lua_pcall(L, 1, 0, 0);
		}
		curLen = 0; 
		ssOut.str(_T(""));

	}
	virtual void OnLog(const CString& strLogFile, SYSTEMTIME st, TCHAR chSeverity, const CString& sOut) {

		if (curLen > 50000) {
			curLen = 0;
			ssOut.str(_T(""));
			sysLOGProfiler()->SetLogLevel(-1);
			return;
		}
		curLen += sOut.GetLength();
		ssOut << st.wHour << ':' << st.wMinute << ':' << st.wSecond << '.' << st.wMilliseconds << " \t" << sOut.GetString() << '\n';
	}
protected:
	CString out = _T("");
	std::wstringstream ssOut;
	size_t curLen = 0;
};

static SysLogCalback* m_pSysLogCalback = nullptr;

namespace vb {

	class VbParser;

	class VbParserCallback
	{
	public:
		virtual void VbParser_Error(VbParser* pParser, int number, const CString& message, int lineIndex, int charIndex, intptr_t opaque) = 0;
		virtual void VbParser_Trace(VbParser* pParser, const CString& message, int lineIndex, intptr_t opaque) {}
	};

	class VbParser
	{
	public:
		VbParser(const CString& debugInfo = CString(), VbParserCallback* pCallback = nullptr);
		~VbParser();

		void InitializeRuntime(short options = 0);
		void ShutdownRuntime();

		CString GetDebugInfo() { return m_debugInfo; }
		CString GetScriptText(int index);
		int GetScriptCount();

		bool Parse(const CString& text, intptr_t opaque = 0);
		runtime::VbFunctionDescriptor* ParseAsFunction(const CString& text, const CString& name, intptr_t opaque = 0);
		runtime::VbRuntime* GetRuntime() { return m_pRuntime; }

		Variant Eval(const CString& text);  // throws VbRuntimeException
		void Execute(const CString& text, bool globalScope);  // throws VbRuntimeException

		void OnTrace(const CString& message, int lineIndex, int scriptIndex)
		{
			if (m_pCallback)
			{
				intptr_t opaque = 0;
				if (scriptIndex >= 0 && scriptIndex < m_scripts.GetSize())
					opaque = m_scripts[scriptIndex].opaque;
				m_pCallback->VbParser_Trace(this, message, lineIndex, opaque);
			}
		}

		void OnError(int number, const CString& message, int lineIndex, int charIndex, int scriptIndex)
		{
			if (m_pCallback)
			{
				intptr_t opaque = 0;
				if (scriptIndex >= 0 && scriptIndex < m_scripts.GetSize())
					opaque = m_scripts[scriptIndex].opaque;
				m_pCallback->VbParser_Error(this, number, message, lineIndex, charIndex, opaque);
			}
		}

	private:
		void Init();
		void Parse_Program();
		void Parse_Script();
		void Parse_ProgramOption();
		void Parse_ProgramElement();
		void Parse_Statement();
		void Parse_StatementIf();
		void Parse_StatementSelect();
		void Parse_StatementForEach();
		void Parse_StatementFor();
		void Parse_StatementDoLoop();
		void Parse_StatementWhile();
		void Parse_StatementExit();
		void Parse_StatementOnError();
		void Parse_StatementWith();
		void Parse_StatementRedim();
		void Parse_StatementVarDeclaration();
		void Parse_StatementConstDeclaration();
		void Parse_StatementCallOrAssignment();
		void Parse_Function();
		void Parse_Class();
		void Parse_ClassMember();
		void Parse_ClassProperty();

		void Parse_Statements(std::function<bool()> endChecker, bool endingWithNewLine = false, bool endingWithTerminator = false);
		runtime::CallChain* Parse_ObjectRef(bool assumeAssignOrCall = false);
		runtime::ArgInfo* Parse_ArgumentList();
		void Parse_ArgumentDeclaration();
		runtime::VbExpr* Parse_Expression(bool copyResultIfInParents = false);
		const Variant* ParseLiteral(bool expressionMode);
		void EatLexema();
		void EatLexema(VbLexemaType type);
		void EatTerminators(bool mustHave = false);
		bool IsStatementTerminator(const VbLexema& lexema);
		const VbLexema& LookAfterParentheses();
		void ReduceExprStacks(CList<runtime::VbExpr*>& argStack, CList<unsigned int>& operationStack, short precedence);
		void ReplaceFunctionName(vb::runtime::CallChain* pCallChain);

		void ThrowException_UnexpectedLexema(const VbLexema& lexema);

		void Emit(runtime::Command* pCommand);
		runtime::VbRuntimeString AllocName(const CString& name);
		int GetNextCommandIndex();
		runtime::VbVariableDescriptor* AllocVariable(runtime::VbRuntimeString name, short flags, const Variant* pValue = nullptr);
		runtime::VbVariableDescriptor* AllocVariable(const VbLexema& lexema, short flags, const Variant* pValue = nullptr);
		runtime::VbRuntimeString GetTempVarName(int absVarIndex = 0);

		// working with nested statement stack
		struct Statement;
		struct FunctionContext;
		void PushNestedStmt(VbLexemaType type, int tempVarCount = 0, runtime::VbRuntimeString varName = nullptr);
		void PopNestedStmt();  // fixes all linked jumps
		runtime::VbRuntimeString GetNestedStmtTempVarName(VbLexemaType type, int varIndex = 0);
		void LinkNestedStmtExit(VbLexemaType type, runtime::JumpCommand* pExitJump);
		bool HasNestedStmt(VbLexemaType type);
		void CheckForVarName(const VbLexema& name);
		FunctionContext& GetFunctionContext();

	private:
		CString m_debugInfo;
		VbLexer m_lexer;
		runtime::VbRuntime* m_pRuntime;
		VbParserCallback* m_pCallback;
		Heap* m_pHeap;
		Heap* m_pDefaultHeap;
		runtime::VbFunction* m_pDefaultFunction;
		runtime::VbRuntimeString m_nameForReturnValue;
		int m_newLineDisableCounter;
		int m_functionCounter;
		CMap<unsigned short, unsigned short, short, short> m_precedence;
		const unsigned short UnaryFlag = (unsigned short)0x8000;

		struct SCRIPT
		{
			CString text;
			intptr_t opaque;
			SCRIPT() {}
			SCRIPT(const CString& text, intptr_t opaque)
			{
				this->text = text;
				this->opaque = opaque;
			}
		};
		CArray<SCRIPT> m_scripts;

		////////////////////////////////////////////
		// parse context
		CArray<runtime::VbRuntimeString> m_tempVarNames;
		runtime::VbFunction* m_pFunction;
		runtime::VbClassDescriptor* m_pClass;
		int m_scriptIndex;

		struct LinkInfo
		{
			LinkInfo* pNext{ nullptr };
			runtime::JumpCommand* pExitJump{ nullptr };
		};
		struct Statement
		{
			LinkInfo* pLinkInfo;
			int TempVarStartIndex;
			int TempVarCount;
			runtime::VbRuntimeString VarName;  // used for 'for' to ensure unique variables in nested loops
			VbLexemaType Type;  // if, for, do, while, select and others
		};

		struct FunctionContext
		{
			StackContainer<Statement> StmtStack;
			StackContainer<LinkInfo> Heap;
			int TempVarCount{ 0 };
		} m_funcContext, m_funcContextDefault;

	};

}


class VbSyntaxChecker : public vb::VbParser, public vb::VbParserCallback
{
public:
	CString m_strError;
	int m_LineIndex = 0, m_charIndex = 0;
	VbSyntaxChecker() : vb::VbParser(CString(), this)
	{
	
	}

protected:
	virtual void VbParser_Error(VbParser* pParser, int number, const CString& message, int lineIndex, int charIndex, intptr_t opaque) override
	{
		if (!m_strError.IsEmpty())
			m_strError += _T("\r\n");
		m_strError += message;

		m_LineIndex = lineIndex + 1;
		m_charIndex = charIndex + 1;

	}

};


static int do_CheckVbScriptPlus(lua_State* L) {

	CString strText = luaL_checkstring(L, 1);

	VbSyntaxChecker checker;
	bool success = checker.Parse(strText);
	if (success)
		return 0;

	lua_pushstring(L, "VB+ Hildim Checker");
	lua_pushstringW(L, checker.m_strError);
	lua_pushinteger(L, checker.m_LineIndex);
	lua_pushinteger(L, checker.m_charIndex);
	return 4;
}

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
	sysLOGInit(L"XX:", L"", -1);
	m_pSysLogCalback = new SysLogCalback();
	sysLOGProfiler()->SetLogCallback(m_pSysLogCalback);
	return TRUE; 
}
int CmbluaApp::ExitInstance()
{
	CWinApp::ExitInstance();
	if(mbTransport != NULL)
	{
		delete mbTransport;
		mbTransport = NULL;
	}
	return 0;
}
/////////////////////////////////////////

CL_mbConnector::~CL_mbConnector(void)
{
}
void CL_mbConnector::SetCallback(int idx)
{
	if (callback_idx != 0)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, callback_idx);
	}
	lua_pushvalue(L, idx);
	callback_idx = luaL_ref(L, LUA_REGISTRYINDEX);
}


HRESULT CL_mbConnector::OnMbReply(mb_handle handle, void* pOpaque, int error, CMessage* pMsg)
{
	HRESULT r = 0;
	if (callback_idx != 0) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, callback_idx);
		lua_pushlightuserdata(L, (void*)handle);
		if (pOpaque) {
			luabridge::luaMessage* opaq = new luabridge::luaMessage((CMessage*)pOpaque, false);
			luabridge::detail::StackHelper<luabridge::RCMessage, true>::push(L, luabridge::RCMessage(opaq));
		}
		else
			lua_pushnil(L);
		lua_pushinteger(L, error);

		luabridge::luaMessage* pm = new luabridge::luaMessage(pMsg, false);

		luabridge::detail::StackHelper<luabridge::RCMessage, true>::push(L, luabridge::RCMessage(pm));

		r = 1;
		if (lua_pcall(L, 4, 0, 0)) { //обработка ошибки
			if (lua_isstring(L, -1)) {
				size_t len;
				const char* msg = lua_tolstring(L, -1, &len);
				char* buff = new char[len + 2];
				strncpy_s(buff, len + 2, msg, len);
				buff[len] = '\n';
				buff[len + 1] = '\0';
				lua_pop(L, 1);
				if (lua_checkstack(L, 3)) {
					lua_getglobal(L, "output");
					lua_getfield(L, -1, "AddText");
					lua_insert(L, -2);
					lua_pushstring(L, buff);
					lua_pcall(L, 2, 0, 0);
				}
				delete[] buff;
			}
		}
	}
	if (m_autodestroy)
	{
		m_pManager->mbUnsubscribeAll(this);
		delete this;
	}
	return r;
}


luabridge::RCMessage do_CreateMessage2(lua_State* L)
{
	luabridge::luaMessage*msg = new luabridge::luaMessage();
	return luabridge::RCMessage(msg);
}


int do_GetGuid(lua_State* L) 	{
	CString s = mbTransport->mbCreateInbox(false);
	s.Replace(L"_INBOX.", L"");
	lua_pushstringW(L, s);
	return 1;
}

int do_StartMbDebug(lua_State* L) {
	CString path = luaL_checkstring(L, 1);
	int lvl = static_cast<int>(luaL_checkinteger(L, 2));
	mbTransport->StartDebugLog(path, _T("HildiMb"), lvl);
	return 0;
}
int do_StopMbDebug(lua_State* L) {
	mbTransport->StopDebugLog();
	return 0;
}
int do_StopSyslog(lua_State* L) {
	bool bPrint = lua_toboolean(L, 1);
	if(m_pSysLogCalback)
		m_pSysLogCalback->StopLogging(L, bPrint);
	return 0;
}
int do_CreateMbTransport(lua_State* L)
{
	CString strDaemon = luaL_checkstring(L,1);
	CString strLan = luaL_checkstring(L,2);
	CString strNetwork = luaL_checkstring(L,3);
	CString strService = luaL_checkstring(L,4);

	CString DaemonCertificatePath = lua_tostring(L,5);
	CString CertificatePath = lua_tostring(L,6);
	CString PrivateKeyPath = lua_tostring(L,7);

	CString strUser = _T("<NotSet>");
	CString strApp = _T("HildiM_");
	CString strVer = _T("");
	CString strPath = _T("");

	lua_getglobal(L, "_ONMBCONNECT");
	if (lua_isfunction(L, -1)) {
		if (!lua_pcall(L, 0, 3, 0)) {
			strVer = luaL_checkstring(L, -1);
			strPath = luaL_checkstring(L, -2);
			strUser = luaL_checkstring(L, -3);
			lua_pop(L, 3);
		}
		else {
			lua_pop(L, 3);
			throw_L_error(L, "OnMbConnet - internal error");
		}
	}
	else {
		lua_pop(L, 1);
		throw_L_error(L, "_ONMBCONNECT function not found. Outdated scripts version. Disconnected.");
	}
	strApp += strVer;
	strApp += _T(" (");
	strApp += strUser;
	strApp += _T(")");
	
	const WCHAR* wDaemonCertificatePath = nullptr;
	const WCHAR* wCertificatePath = nullptr;
	const WCHAR* wPrivateKeyPath = nullptr;

	if (DaemonCertificatePath != _T("")) {
		DaemonCertificatePath = strPath + DaemonCertificatePath;
		wDaemonCertificatePath = DaemonCertificatePath;
	}
	if (CertificatePath != _T("")) {
		CertificatePath = strPath + CertificatePath;
		wCertificatePath = CertificatePath;
	}
	if (PrivateKeyPath != _T("")) {
		PrivateKeyPath = strPath + PrivateKeyPath;
		wPrivateKeyPath = PrivateKeyPath;
	}

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
		mbTransport->setAppName(strApp);
		sysLOGProfiler()->SetLogLevel(1);
		if (!mbTransport->create(strDaemon, strLan, strNetwork, strService,
			wDaemonCertificatePath, wCertificatePath, wPrivateKeyPath)) {
			delete mbTransport;
			mbTransport = new CmbTransport();
			m_pSysLogCalback = new SysLogCalback();
			sysLOGProfiler()->SetLogCallback(m_pSysLogCalback);
			sysLOGProfiler()->SetLogLevel(-1);
			sysLOGProfiler()->SetPath("XX:");

			throw_L_error(L, "MbConnet - critical Error. The certificate file path may be incorrect");
		}
	}
	return 0;
}
bool do_Publish2(luabridge::luaMessage* src, lua_State* L) {
	if (m_strLan != "xxx")
	{
		if (!src)
			throw_L_error(L, "Bad argument #2 to 'Publish' (Message expected, got no value)");
		return (mbTransport->mbPublish(src->getCMsg()) == MB_ERROR_OK);
	}else
		throw_L_error(L, "Disconnected! Publish Impossible");
	return false;
}
bool do_getConnected() {
	return m_strLan != "xxx";
}

int do_Subscribe2(lua_State* L) {
	if (m_strLan != "xxx")
	{
		const char* cc = luaL_typename(L, 1);
		CL_mbConnector* cn = new CL_mbConnector(mbTransport, L);
		cn->SetCallback();
		//mb_handle h = mbTransport->mbSubscribe(cn,luaL_checkstring(L,2),lua_touserdata(L,3));
		

		mb_handle h = mbTransport->mbSubscribe((CmbConnectorBase*)cn, luaL_checkstring(L, 2), lua_touserdata(L, 3));
		lua_pushlightuserdata(L, (void*)h);
		return 1;
	}
	return 0;
}
int do_Subscribe(lua_State* L)
{
	if( m_strLan != "xxx")
	{
		const char* cc = luaL_typename(L, 1);
		CL_mbConnector *cn = new CL_mbConnector(mbTransport,L);
		cn->SetCallback(); 
		//mb_handle h = mbTransport->mbSubscribe(cn,luaL_checkstring(L,2),lua_touserdata(L,3));
		mb_handle h = mbTransport->mbSubscribe((CmbConnectorBase*)cn, luaL_checkstring(L, 2), lua_touserdata(L, 3));
		lua_pushlightuserdata(L,(void*)h);
		return 1;
	}
	return 0;
}
int do_Request2(lua_State* L)
{
	if( m_strLan != "xxx")
	{
		CL_mbConnector *cn = new CL_mbConnector(mbTransport,L,true);
		cn->SetCallback();
		CMessage* opaque = NULL;
		luabridge::luaMessage* lOpaque = luabridge::detail::StackHelper<luabridge::RCMessage, true>::get(L, 4);
		if (lOpaque) {
			opaque = lOpaque->getCMsg();
			opaque->InternalAddRef();	
		}
		else if (lua_type(L, 4) == LUA_TNONE)
			throw_L_error(L, "Opaque is not a message");

		luabridge::luaMessage* lMsg = luabridge::detail::StackHelper<luabridge::RCMessage, true>::get(L, 2);

		if(!lMsg)
			throw_L_error(L, "Bad argument #2 to 'Request' (Message expected, got no value)");

		mb_handle h = mbTransport->mbRequest(cn,lMsg->getCMsg(), static_cast<int>(luaL_checkinteger(L, 3)), (void*)opaque);
		lua_pushlightuserdata(L,(void*)h);
		return 1;
	}
	else
		throw_L_error(L, "Disconnected! Request Impossible");
	return 0;
}

int do_UnSubscribe(lua_State* L)
{
	mbTransport->mbUnsubscribe((mb_handle)lua_touserdata(L, 1));
	return 0;
}
int do_Destroy(lua_State* L)
{
	m_strLan = "xxx"; //ѕризнак отсутстви€ подписки
	mbTransport->destroy();
	return 0;
}


namespace luabridge {
	/*void luaMessage::luaAddField(lua_State* L) {
		CString Id = luaH_CheckCString(L, 2);
		const Variant& Value = luaH_CheckVariant(L, 3);
		CDatum* pDatum = CDatum::FromVariant(Value);
		if (pDatum)
			m->AddDatum(Id, pDatum->value());
		else
			m->AddDatum(Id, ::VariantFromVariant(Value));
	
		//const Variant& Value = *ctx.pParams[1];
	}*/

	CDatum* luaMessage::getDatumByArg(lua_State* L, int arg) {
		switch (lua_type(L, 2)) {
		case LUA_TSTRING:
			return m->GetDatum(luaH_CheckCString(L, 2));
			break;
		case LUA_TNUMBER:
			return m->GetDatum(static_cast<int>(luaL_checkinteger(L, 2)));
			break;
		default:
		{
			std::string er("Invalid type for argumrnent: \"");
			er += luaL_typename(L, 2);
			er += "\"";
			throw_L_error(L, er.c_str());
		}
		}
		return nullptr;
	}

	int  luaMessage::xSetField(lua_State* L) { 

		CDatum* d;
		Variant v = luaH_CheckVariant(L, 3);

		switch (lua_type(L, 2)) {
		case LUA_TSTRING:
			m->SetDatum(CString(luaL_checkstring(L, 2)), v);
			break;
		case LUA_TNUMBER:
			d = m->GetDatum(static_cast<int>(lua_tointeger(L, 2)));
			if (d)
				d->value(v);
			else
			{
				std::string er("Index not exist: ");
				er += std::to_string(v.intValue);
				throw_L_error(L, er.c_str());
			}
			break;
		default:
		{
			std::string er("Invalid type for argumrnent: \"");
			er += luaL_typename(L, 2);
			er += "\"";
			throw_L_error(L, er.c_str());
		}
		}
		return 0;
	}

	int luaMessage::xCounts(lua_State* L)
	{
		lua_pushinteger(L, m->GetDataCount());
		lua_pushinteger(L, m->GetMsgsCount());
		return 2;
	}
	int luaMessage::xSubjects(lua_State* L)
	{
		if (lua_type(L, 2) == LUA_TSTRING)
			m->m_strSendSubject = luaL_checkstring(L, 2);
		if (lua_type(L, 3) == LUA_TSTRING)
			m->m_strReplySubject = luaL_checkstring(L, 3);

		lua_pushstringW(L, m->m_strSendSubject);
		lua_pushstringW(L, m->m_strReplySubject);
		lua_pushstringW(L, m->id());
		return 3;
	}

	int luaMessage::xRSCounts(lua_State* L) {
		CRecordset* rs = m->GetRecordset();
		if (!rs)
			return 0;
		lua_pushinteger(L, rs->GetColumnCount());
		lua_pushinteger(L, rs->GetRecordCount());
		return 2;
	}

	typedef void _PF(void*, int, int, const char*);

	void SetBstrCell(_PF* pFunc, void* pList, int i, int j, bool bUnic, CStringEx& sVal) {
		if (bUnic) {
			//const char * str = d->GetValueText();
			int result_c;

			result_c = WideCharToMultiByte(CP_UTF8, 0, sVal, -1, 0, 0, 0, 0);
			if (!result_c) {
				return;
			}
			char* cres = new char[result_c];
			if (!WideCharToMultiByte(CP_UTF8, 0, sVal, -1, cres, result_c, 0, 0)) {
				delete[] cres;
				return;
			}
			pFunc(pList, i + 1, j + 1, cres);
			delete[] cres;
		}
		else {
			pFunc(pList, i + 1, j + 1, W2MB(sVal).c_str());
		}
	}

	int luaMessage::xFillList(lua_State* L) {

		luaL_checkudata(L, 2, "iupHandle");

		int firstLine = static_cast<int>(luaL_checkinteger(L, 3));
		int firstCol = static_cast<int>(luaL_checkinteger(L, 4));
		bool setNum = false;
		bool setId = false;


		if (firstCol == 0) {
			firstCol = 1;
			setId = true;
		}
		else if (firstCol < 0) {
			firstCol = firstCol * -1;
			setNum = true;
		}
		bool setNull = false;

		if (!lua_istable(L, 5))
			throw_L_error(L, "mesage_FillList:Argument 5 isn't a table");

		int nFields = static_cast<int>(luaL_len(L, 5));
		int* fMap = new int[nFields];
		bool* fUtf = new bool[nFields];

		for (int i = 0; i < nFields; i++) {
			lua_rawgeti(L, 5, i + 1);
			fMap[i] = static_cast<int>(luaL_checkinteger(L, -1));
		}

		if (lua_istable(L, 6)) {
			if (nFields != luaL_len(L, 6))
				throw_L_error(L, "mesage_FillList:Tables 5 and 6 have different lengths");

			for (int i = 0; i < nFields; i++) {
				lua_rawgeti(L, 6, i + 1);
				fUtf[i] = (luaL_checkinteger(L, -1) != 0);
			}
		}
		else {
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
		if (lua_pcall(L, 1, 2, 0) != LUA_OK)
			throw_L_error(L, "mesage_FillList:Internal error");

		_PF* pFunc = (_PF*)lua_touserdata(L, -2);
		void* pList = lua_touserdata(L, -1);

		int msgCnt;
		CRecordset* rs = m->GetRecordset();
		if (rs)
			msgCnt = rs->GetRecordCount();
		else
			msgCnt = m->GetMsgsCount();

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
		if (curCol < nFields)
			throw_L_error(L, "mesage_FillList:not enough columns");


		lua_settop(L, 3);
		lua_getfield(L, -1, "GetAttribute");
		lua_pushvalue(L, 2);
		lua_pushstring(L, "NUMLIN");

		if (lua_pcall(L, 2, 1, 0) != LUA_OK)
			throw_L_error(L, "mesage_FillList:Internal error");
		pCh = lua_tostring(L, -1);


		std::string buf(256, 0);

		lua_settop(L, 3);
		lua_getfield(L, -1, "SetAttribute");
		lua_pushvalue(L, 2);
		lua_pushstring(L, "DELLIN");

		buf = "1-";
		buf += pCh;
		lua_pushstring(L, buf.c_str());
		if (lua_pcall(L, 3, 0, 0) != LUA_OK)
			throw_L_error(L, "mesage_FillList:Internal error");

		lua_settop(L, 3);
		lua_getfield(L, -1, "SetAttribute");
		lua_pushvalue(L, 2);
		lua_pushstring(L, "ADDLIN");
		buf = "1-";
		buf += std::to_string(msgCnt);
		lua_pushstring(L, buf.c_str());
		if (lua_pcall(L, 3, 0, 0) != LUA_OK)
			throw_L_error(L, "mesage_FillList:Internal error");

		if (rs) {
			CRecordsetReader reader = rs->CreateReader();
			Variant value;
			for (int i = 0; i < msgCnt; i++) {
				if (setNum || setId) {
					buf = std::to_string(i + firstLine);
					pFunc(pList, i + firstLine, 0, buf.c_str());
				}
				reader.MoveNext();
				for (int j = 0; j < nFields; j++) {
					reader.GetValue(fMap[j] - 1, value);

					const char* out = NULL;

					switch (value.type) {
					case Variant::Type::Double:
					{
						std::snprintf(&buf[0], 250, "%.9f", std::forward<double>(value.dblValue));
						out = buf.c_str();
					}
					break;
					case Variant::Type::Integer:
					{
						buf = std::to_string(static_cast<long>(value.longValue));
						out = buf.c_str();
					}
					break;
					case Variant::Type::Boolean:
					{
						out = value.boolValue ? "<true>" : "<false>";
					}
					break;
					case Variant::Type::Date:
					{
						ATL::COleDateTime dt = value.dateValue;
						buf = ToStr(dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer());
						out = buf.c_str();
					}
					break;
					case Variant::Type::String:
					{
						SetBstrCell(pFunc, pList, i, j, fUtf[j], CStringEx(value.strValue));
					}
					break;
					case Variant::Type::Null:
					{
						if (setNull)
							out = "<null>";
					}
					break;
					}
					if (out)
						pFunc(pList, i + firstLine, j + firstCol, out);
				}
			}
		}
		else {

			for (int i = 0; i < msgCnt; i++) {
				if (setNum) {
					buf = std::to_string(i + firstLine);
					pFunc(pList, i + firstLine, 0, buf.c_str());
				}
				else if (setId) {
					pFunc(pList, i + firstLine, 0, W2MB(m->GetMsg(i)->id()).c_str());
				}
				for (int j = 0; j < nFields; j++) {

					CDatum* d = m->GetMsg(i)->GetDatum(fMap[j] - 1);

					const char* out = NULL;
					if (d) {
						if (!d) return 0;
						switch (d->value().type) {
						case Variant::Type::Double:
						{
							double v;
							d->GetValueAsDouble(v);

							std::snprintf(&buf[0], 250, "%.9f", std::forward<double>(v));
							out = buf.c_str();

						}
						break;
						case Variant::Type::Integer:
						{
							long v;
							d->GetValueAsLong(v);
							buf = std::to_string(v);
							out = buf.c_str();
						}
						break;
						case Variant::Type::Boolean:
						{
							bool v;
							d->GetValueAsBool(v);
							out = v ? "<true>" : "<false>";
						}
						break;
						case Variant::Type::Date:
						{
							ATL::COleDateTime dt;
							d->GetValueAsDate(dt);
							buf = ToStr(dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer());
							out = buf.c_str();
						}
						break;
						case Variant::Type::String:
						{
							SetBstrCell(pFunc, pList, i, j, fUtf[j], d->GetValueText());
						}
						break;
						case Variant::Type::Null:
						{
							if (setNull)
								out = "<null>";
						}
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
	
	int luaMessage::xRSGetRecord(lua_State* L)
	{
		CRecordset* rs = m->GetRecordset();
		if (!rs)
			return 0;
		int row = static_cast<int>(luaL_checkinteger(L, 2));

		CString strId;
		Variant value;
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
			switch (value.type) {
			case Variant::Type::Double:
			{
				type = "R8";
				lua_pushnumber(L, value.dblValue);
			}
			break;
			case Variant::Type::Integer:
			{
				type = "I4";
				lua_pushinteger(L, value.longValue);
			}
			break;
			case Variant::Type::Boolean:
			{
				type = "BOOL";
				lua_pushboolean(L, (value.boolValue));
			}
			break;
			case Variant::Type::Date:
			{
				type = "DATE";
				ATL::COleDateTime dt = value.dateValue;
				lua_pushstringW(L, dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer());
			}
			break;
			case Variant::Type::String:
			{
				type = "BSTR";
				CStringEx str = value.strValue;
				lua_pushstringW(L, str);
			}
			break;
			default:
				type = "NULL";
				lua_pushnil(L);
			}
			lua_setfield(L, -2, "value");

			lua_pushstringW(L, rs->GetColumn(i)->GetName());
			lua_setfield(L, -2, "title");

			lua_pushstring(L, type);
			lua_setfield(L, -2, "type");

			lua_settable(L, -3);

		}
		return 1;
	}
	
	int luaMessage::xRSColumn(lua_State* L)
	{
		CRecordset* rs = m->GetRecordset();
		if (!rs)
			return 0;
		int col = static_cast<int>(luaL_checkinteger(L, 2));
		lua_pushstringW(L, rs->GetColumn(col)->GetName());
		return 1;
	}
	
	int luaMessage::xSaveFieldBinary(lua_State* L) {
		int err = 0;
		char* b = NULL;

		CString fldName = luaL_checkstring(L, 2);
		CString path = luaL_checkstring(L, 3);
		char* description;
		DWORD l;

		CDatum* d = m->GetDatum(fldName);
		if (!d->value().IsBuffer()) {
			err = -1;
			description = "Not Binary Data";
			goto err;
		}
		{
			l = d->GetDataSize();
			b = new char[l];

			Variant::Buffer* pBuffer = (Variant::Buffer*)d->value().customValue;
			memcpy(b, pBuffer->ptr, pBuffer->size);

			int pf;

			err = _wsopen_s(&pf, path, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, _SH_DENYRW, _S_IWRITE);
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
			if (err) {
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

	int luaMessage::xAddFieldBinary(lua_State* L) {
		CString fldName = luaL_checkstring(L, 2);
		CString path = luaL_checkstring(L, 3);
		int err;
		char* description;

		int pf;
		char* b = NULL;
		err = _wsopen_s(&pf, path, _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD);
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
			Variant::Buffer* pBuffer = new Variant::Buffer();
			pBuffer->ptr = new BYTE[l];
			pBuffer->size = l;
			//v.Attach(pBuffer);


			for (DWORD i = 0; i < l; i++) {
				((BYTE*)pBuffer->ptr)[i] = (BYTE)b[i];
			}

			m->AddDatum(fldName, pBuffer);
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

	
	RCMessage luaMessage::luaExecute(std::string param) {
		CMessage* msgOut = new CMessage();
		if (m->ExecCommand(param.c_str(), NULL, msgOut))
			return new luaMessage(msgOut, false);
		
		delete msgOut;
		return nullptr;
	}

	RCMessage luaMessage::luaRemoveMessage(lua_State* L) {
		const Variant& Index = luaH_CheckVariant(L, 2);

		LONG lIndex;
		CMessage* pMsg = NULL;
		if (Index.type == Variant::Type::String)
			pMsg = m->DetachMsg(Index.strValue);
		else if (::LongFromVariant(Index, &lIndex))
			pMsg = m->DetachMsg(lIndex);
		if(!pMsg)
			return nullptr;

		return RCMessage(new luaMessage(pMsg));
	}
	
	bool luaMessage::luaAddMessage(std::string sub, luaMessage* added) {

		CString Id = ToCStr(sub);

		if (!added || !added->getCMsg())
			return false;
		CMessage* pMsg = added->getCMsg();


		added->getCMsg()->InternalAddRef();
		added->getCMsg()->id(Id);
		m->AttachMsg(pMsg);

		return true;

	}
	

	void luaMessage::luaAddField(std::string id, lua_State* L)
	{
		CString Id = ToCStr(id);
		const Variant& Value = luaH_CheckVariant(L, 3);

		m->AddDatum(Id, ::VariantFromVariant(Value));
	}
      
	void luaMessage::luaInsertField(std::string id, lua_State* L)
	{
		CString Id = ToCStr(id);
		const Variant& Value = luaH_CheckVariant(L, 3);

		m->AddDatum(Id, ::VariantFromVariant(Value), luaL_checkinteger(L, 4));
	}

	void luaMessage::luaUpdateField(std::string id, lua_State* L)
	{
		CString Id = ToCStr(id);
		const Variant& Value = luaH_CheckVariant(L, 3);

		CDatum* pOldDatum = m->GetDatum(Id);
		if (pOldDatum == NULL)
			m->AddDatum(Id, ::VariantFromVariant(Value));
		else
			pOldDatum->value(::VariantFromVariant(Value));
	}

	RCDatum luaMessage::luaRemoveField( lua_State* L)
	{
		CDatum* d;

		switch (lua_type(L, 2)) {
		case LUA_TSTRING:
			d = m->DetachDatum(CString(luaL_checkstring(L, 2)));
			break;
		case LUA_TNUMBER:
			d = m->DetachDatum(static_cast<int>(lua_tointeger(L, 2)));
			break;
		default:
		{
			std::string er("Invalid type for argumrnent: \"");
			er += luaL_typename(L, 2);
			er += "\"";
			throw_L_error(L, er.c_str());
			return nullptr;
		}
		}
		return RCDatum(new luaDatum(d));
	}
	
	RCDatum luaMessage::luaField(int i, lua_State* L) const{
		CDatum* d = m->GetDatum(i);
		if (!d)
			return nullptr;
			//throw_L_error(L, "Invalid index");
		return RCDatum(new luaDatum(m->GetDatum(i)));
	}

	bool luaMessage::luaInsertMessage(std::string id, luaMessage* inserted, LONG lIndex)
	{
		CString Id = ToCStr(id);
	
		CMessage* pMsg = inserted->getCMsg();
		if (pMsg == NULL)
			return false;

		pMsg->InternalAddRef();
		pMsg->id(Id);

		m->AttachMsg(pMsg, lIndex);

		return true;

	}

	bool luaMessage::luaUpdateMessage(std::string id, luaMessage* updFrom)
	{
		CString Id = ToCStr(id);

		CMessage* pMsg = updFrom->getCMsg();
		if (pMsg == NULL)
			return false;

		pMsg->InternalAddRef();
		pMsg->id(Id);
		CMessage* pOldMsg = m->ReplaceMsg(pMsg);
		if (pOldMsg) pOldMsg->InternalRelease();

		return true;

	}

	int luaMessage::luaGetMessageIndex(std::string id)
	{
		CString Id = ToCStr(id);
		int cnt = m->GetMsgsCount();
	
		for (int i = 0; i < cnt; i++)
		{
			if (m->GetMsg(i)->id() == Id)
			    return i;
		}
		return -1;
	}

	bool luaMessage::luaAddHeadMessage(std::string id, luaMessage* added)
	{
		CString Id = ToCStr(id);

		CMessage* pMsg = added->getCMsg();
		if (pMsg == NULL)
			return false;

		pMsg->InternalAddRef();

		pMsg->id(Id);
		return (m->AttachHeadMsg(pMsg) != 0);
	}

	void luaMessage::luaAttachContents(luaMessage* from)
	{
		CMessage* pMsg = from->getCMsg();
		if (pMsg == NULL) return;

		for (int i = 0; i < pMsg->GetMsgsCount(); i++)
		{
			CMessage* pSubMsg = pMsg->GetMsg(i);
			pSubMsg->InternalAddRef();
			m->AttachMsg(pSubMsg);
		}
		for (int i = 0; i < pMsg->GetDataCount(); i++)
		{
			CDatum* pDatum = pMsg->GetDatum(i);
			pDatum->InternalAddRef();
			m->AttachDatum(pDatum);
		}

		// recordset
		CRecordset* pRecordset = pMsg->GetRecordset();
		if (pRecordset != NULL)
		{
			pRecordset->InternalAddRef();
			m->AttachRecordset(pRecordset);
		}
	}
	int luaMessage::luaSetWireText(lua_State* L)
	{
		CString strData = luaH_CheckCString(L, 2);

		CString strError;
		bool bResult = m->SetWireText(strData, strError);
		if (!bResult) {
			lua_pushnil(L);
			lua_pushstring(L, ToStr(strError).c_str());
			return 2;
		}
		lua_pushboolean(L, true);
		return 1;
	}

	void bindToLUA(lua_State* L)
	{

		auto nspace =
			getGlobalNamespace(L)
			 .beginNamespace("mblua")
			 //.beginNamespace("MBLUA2")
			 .addConstant("VERSION", 321)
			 .addFunction("GetGuid", do_GetGuid)
			 .addFunction("CreateMbTransport", do_CreateMbTransport)
			 .addFunction("StopSyslog", do_StopSyslog)
			 .addFunction("StartMbDebug", do_StartMbDebug)
			 .addFunction("StopMbDebug", do_StopMbDebug)
			 .addFunction("Publish", do_Publish2)
			 .addFunction("Subscribe", do_Subscribe2)
			 .addFunction("UnSubscribe", do_UnSubscribe)
			 .addFunction("Destroy", do_Destroy)
			 .addFunction("Request", do_Request2)
			 .addFunction("CheckVbScript", do_CheckVbScriptPlus)
			 .addFunction("CreateMessage", do_CreateMessage2)
			 .addProperty("connected", do_getConnected)
			 .beginClass<luaMessage>("Message")
			  .addConstructor <void (*) (void), RCMessage >()
			  .addFunction("Subjects", &luaMessage::xSubjects)
			  .addFunction("Counts", &luaMessage::xCounts)
			  .addFunction("Field", &luaMessage::xFieldValue)
			  .addFunction("RemoveField", &luaMessage::xRemoveField)
			  .addFunction("SetField", &luaMessage::xSetField)
			  .addFunction("FieldType", &luaMessage::xFieldType)
			  .addFunction("FieldName", &luaMessage::xFieldName)
			  .addFunction("FillList", &luaMessage::xFillList)
			  .addFunction("RSCounts", &luaMessage::xRSCounts)
			  .addFunction("RSGetRecord", &luaMessage::xRSGetRecord)
			  .addFunction("RSColumn", &luaMessage::xRSColumn)
			  .addFunction("SaveFieldBinary", &luaMessage::xSaveFieldBinary)
			  .addFunction("AddFieldBinary", &luaMessage::xAddFieldBinary)

			  .addFunction("ToString", &luaMessage::luaToString)
			  .addFunction("SetPathValue", &luaMessage::luaUpdatePathField)
			  .addFunction("GetWireText", &luaMessage::luaGetWireText)
			  .addFunction("GetPathValue", &luaMessage::luaGetPathValue)
		      .addFunction("Destroy", &luaMessage::xDestroy)
			  .addFunction("GetMessage", &luaMessage::luaGetMessage)
			  .addFunction("Message", &luaMessage::lua_Message)
			  .addFunction("Execute", &luaMessage::luaExecute)
			  .addFunction("Reset", &luaMessage::luaReset)
			  .addFunction("RemoveMessage", &luaMessage::luaRemoveMessage)
			  .addFunction("AttachMessage", &luaMessage::luaAddMessage)
			  .addFunction("ExistsMessage", &luaMessage::luaExistsMessage)
			  .addFunction("Name", &luaMessage::luaGetName)
			  .addFunction("CopyFrom", &luaMessage::luaCopyFrom)

			  .addFunction("__tostring", &luaMessage::luaToString)
			  .addFunction("addField", &luaMessage::luaAddField)
			  .addFunction("addMessage", &luaMessage::luaAddMessage)
			  .addFunction("addHeadMessage", &luaMessage::luaAddHeadMessage)
			  .addFunction("addPathField", &luaMessage::luaAddPathField)
			  .addFunction("addTailMessage", &luaMessage::luaAddMessage) //правильно!
			  .addFunction("attachContents", &luaMessage::luaAttachContents) 
			  .addFunction("copyFrom", &luaMessage::luaCopyFrom)
			  .addFunction("execute", &luaMessage::luaExecute)
			  .addFunction("existsField", &luaMessage::luaExistsField)
			  .addFunction("existsMessage", &luaMessage::luaExistsMessage)
			  .addFunction("field", &luaMessage::luaField)
			  .addFunction("flatMessage", &luaMessage::luaFlatMessage)
			  .addFunction("getMessage", &luaMessage::luaGetMessage)
			  .addFunction("getMessageIndex", &luaMessage::luaGetMessageIndex)
			  .addFunction("getFieldValue", &luaMessage::luaGetFieldValue)
			  .addFunction("getPathValue", &luaMessage::luaGetPathValue)
			  .addFunction("getWireText", &luaMessage::luaGetWireText)
			  .addFunction("insertField", &luaMessage::luaInsertField)
			  .addFunction("insertMessage", &luaMessage::luaInsertMessage)
			  .addFunction("message", &luaMessage::lua_Message)
			  .addFunction("removeField", &luaMessage::luaRemoveField)
			  .addFunction("removeHeadMessage", &luaMessage::luaRemoveHeadMessage)
			  .addFunction("removeMessage", &luaMessage::luaRemoveMessage)
			  .addFunction("removePathField", &luaMessage::luaRemovePathField)
			  .addFunction("removeTailMessage", &luaMessage::luaRemoveTailMessage)
			  .addFunction("reset", &luaMessage::luaReset)
			  .addFunction("setWireText", &luaMessage::luaSetWireText)
			  .addFunction("updateField", &luaMessage::luaUpdateField)
			  .addFunction("updateFrom", &luaMessage::luaUpdateFrom)
			  .addFunction("updateMessage", &luaMessage::luaUpdateMessage)
			  .addFunction("updatePathField", &luaMessage::luaUpdatePathField)
			  .addProperty("fieldCount", &luaMessage::luaGetFieldCount)
			  .addProperty("messageCount", &luaMessage::luaGetMessageCount)
			  .addProperty("name", &luaMessage::luaGetName)
			  .addProperty("replySubject", &luaMessage::luaGetReplySubject, &luaMessage::luaSetReplySubject)
			  .addProperty("sendSubject", &luaMessage::luaGetSendSubject, &luaMessage::luaSetSendSubject)
			  .addProperty("toString", &luaMessage::luaToString)
			 .endClass()
			 .beginClass<luaDatum>("LUADatum")
			  .addFunction("__tostring", &luaDatum::luaValueText)
			  .addFunction("getValue", &luaDatum::luaGetValue)
			  .addFunction("setValue", &luaDatum::luaSetValue)
			  .addProperty("name", &luaDatum::luaGetName)
			  .addProperty("text", &luaDatum::luaValueText)
			 .endClass()
		   .endNamespace();
	}
}

extern "C" __declspec(dllexport)
int luaopen_mblua(lua_State *L)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	// at this point, the SciTE window is available. Can't always assume
	// that it is the foreground window, so we hunt through all windows
	// associated with this thread (the main GUI thread) to find a window
	// matching the appropriate class name
//	luaL_newmetatable(L, MESSAGEOBJECT);  // create metatable for window objects
//	lua_pushvalue(L, -1);  // push metatable
//	lua_setfield(L, -2, "__index");  // metatable.__index = metatable
//	luaL_setfuncs(L, message_methods, 0);
//
//	lua_newtable(L);
//	luaL_setfuncs(L, mblua, 0);

	luabridge::bindToLUA(L);

	return 1;
}
