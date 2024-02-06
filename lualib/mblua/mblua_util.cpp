#include "stdafx.h"
#include "mblua_util.h"

#include "VbEngine\VbLexer.h"
#include "VbEngine\VbRuntime.h"
#include <functional>

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
	VbSyntaxChecker() : vb::VbParser(CString(), this)
	{

	}

protected:
	virtual void VbParser_Error(VbParser* pParser, int number, const CString& message, int lineIndex, int charIndex, intptr_t opaque) override
	{
		if (!m_strError.IsEmpty())
			m_strError += _T("\r\n");

		if (lineIndex >= 0)
			m_strError += ::FormatString(_T("Line %i, Position %i: %s"), lineIndex + 1, charIndex + 1, (LPCTSTR)message);
		else
			m_strError += message;
	}

	virtual void VbParser_Trace(VbParser* pParser, const CString& message, int lineIndex, intptr_t opaque)
	{
		if (!m_strTrace.IsEmpty())
			m_strTrace += _T("\r\n");

		if (lineIndex >= 0)
			m_strTrace += ::FormatString(_T("Line %i: %s"), lineIndex + 1, (LPCTSTR)message);
		else
			m_strTrace += message;
	}

public:
	CString m_strError;
	CString m_strTrace;
};



LUA_API const char* lua_pushstringW(lua_State* L, const WCHAR* w) {
	return lua_pushstring(L, W2MB(w).c_str());
}

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
		throw_L_error(L, string_format("%s:Argument %d isn't a message", module, idx).c_str());
	}
	if (!wrp->msg) {
		throw_L_error(L, string_format("%s:Argument %d deleted", module, idx).c_str());
	}
	return wrp->msg;
}
void cmesage_gc(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		throw_L_error(L, string_format("mesage_gc:Argument 1 isn't a message").c_str());
	}
	CMessage *msg = wrp->msg;
	if (msg && !msg->InternalRelease())
		wrp->msg = NULL;
}

void cmesage_Destroy(lua_State* L) {
	MsgWrap *wrp = (MsgWrap*)luaL_checkudata(L, 1, MESSAGEOBJECT);
	if (!wrp) {
		throw_L_error(L, string_format("mesage_Destroy:Argument 1 isn't a message").c_str());
	}
	CMessage *msg = wrp->msg;
	if (msg) 
		msg->InternalRelease();
	wrp->msg = NULL;
}