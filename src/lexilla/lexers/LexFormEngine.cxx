

#define TYPE_SPACE 0
#define TYPE_VBS 1
#define TYPE_XML 2
#define TYPE_SQL 3


// Internal state, highlighted as number

#define SCE_FM_CONT_CONT 1
#define SCE_FM_CONT_NO 2

// Scintilla source code edit control
/** @file LexCPP.cxx
 ** Lexer for C++, C, Java, and JavaScript.
 **/
// Copyright 1998-2005 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif
#ifdef __BORLANDC__
// Borland C++ displays warnings in vector header without this
#pragma option -w-ccc -w-rch
#endif

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "PropSetSimple.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "DefaultLexer.h"

using namespace Scintilla;
using namespace Lexilla;


// Options used for LexerCPP
struct OptionsFM {
	bool fold;
	bool foldComment;
	bool foldCompact;
	bool foldAtElse;
	bool foldcdata;
	bool debugmode;
	bool foldonerror;
	bool frozen;  //флаг для временного отключения на время загрузки хелплистов
	std::string debugsuffix;
	OptionsFM() {
		fold = false;
		foldComment = false;
		foldCompact = false;
		foldAtElse = false;
		debugmode = false;
		foldcdata = false;
		foldonerror = false;
		frozen = false;
		debugsuffix = "";
	}
};

#define KW_VB_KEYWORDS 0
#define KW_VB_FUNCTIONS 1
#define KW_X_TAGNAMES 2
#define KW_X_TAGPROPERTIES 3
#define KW_VB_OBJECTS 4
#define KW_VB_PROPERTIES 5
#define KW_MSSQL_STATEMENTS 6
#define KW_MSSQL_DATA_TYPES 7
#define KW_MSSQL_SYSTEM_TABLES 8
#define KW_MSSQL_GLOBAL_VARIABLES 9
#define KW_MSSQL_FUNCTIONS 10
#define KW_MSSQL_STORED_PROCEDURES 11
#define KW_MSSQL_OPERATORS 12
#define KW_MSSQL_FUNCTIONSEX 13
#define KW_VB_CONSTANTS 14
#define KW_VB_FUNCTIONSEX 15

static const char *const fnWordLists[] = {
	"Statements",
	"Functions",
	"xmlTag",
	"xmlProperties",
	"Objects",
	"methods",
	"SQLStatements",
	"SQLDatatypes",
	"SQLSystemtables",
	"SQLGlobalvariables",
	"SQLFunctions",
	"SQLstoredprocedures",
	"SQLFunctionsEx",
	"SQLFunctions",
	"Constants",
	"functionsEx",
	0,
};
enum SelectState{
	Empty, Start, Next, End
};

struct OptionsSetFM : public OptionSet<OptionsFM> {
	OptionsSetFM() {

		DefineProperty("fold", &OptionsFM::fold);

		DefineProperty("fold.comment", &OptionsFM::foldComment,
			"This option enables folding multi-line comments and explicit fold points when using the C++ lexer. "
			"Explicit fold points allows adding extra folding by placing a //{ comment at the start and a //} "
			"at the end of a section that should fold.");

		DefineProperty("fold.compact", &OptionsFM::foldCompact);
		DefineProperty("fold.onerror", &OptionsFM::foldonerror);
		DefineProperty("fold.cdata", &OptionsFM::foldcdata);
		DefineProperty("precompiller.debugmode", &OptionsFM::debugmode);
		DefineProperty("precompiller.debugsuffix", &OptionsFM::debugsuffix);
		DefineProperty("lexer.formenjine.frozen", &OptionsFM::frozen);


		DefineProperty("fold.at.else", &OptionsFM::foldAtElse,
			"This option enables C++ folding on a \"} else {\" line of an if statement.");

		DefineWordListSets(fnWordLists);
	}
};

class LexerFormEngine : public ILexer5 {
	bool caseSensitive;
	CharacterSet setFoldingWordsBegin;
	WordList keywords[24];	//переданные нам вордлисты
	WordList wRefold; //начала фолдинга
	WordList wFold;   //продолжения ифов и свитчей
	WordList wUnfold; //окончания конструкций
	WordList wDebug; //Используемые в данный момент дебаги с суффиксами
	std::map<std::string, std::string> preprocessorDefinitionsStart;
	OptionsFM options;
	OptionsSetFM osFM;
	void SCI_METHOD ColoriseVBS(StyleContext &sc, int &visibleChars,int &fileNbDigits);
	void SCI_METHOD ColoriseXML(StyleContext &sc);
	void SCI_METHOD ColoriseSQL(StyleContext &sc);
	void SCI_METHOD LexerFormEngine::ResolveSqlID(StyleContext &sc);
	void SCI_METHOD LexerFormEngine::ResolveVBId(StyleContext &sc);
	bool PlainFold(unsigned int startPos, int length, int initStyle, IDocument *pAccess, LexAccessor &styler);
	SelectState selectionState = Empty;
public:
	LexerFormEngine(bool caseSensitive_) :
	    setFoldingWordsBegin(CharacterSet::setLower, "idfecnwspl"),
		caseSensitive(caseSensitive_){
			wRefold.Set("else elseif");
			wFold.Set("do function sub for with private public property class while");
			wUnfold.Set("end next wend loop");
			wDebug.Set("");
	}
	~LexerFormEngine() {
	}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvRelease4;
	}
	const char * SCI_METHOD PropertyNames() {
		return osFM.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) {
		return osFM.PropertyType(name);
	}
	const char * SCI_METHOD DescribeProperty(const char *name) {
		return osFM.DescribeProperty(name);
	}
	int SCI_METHOD PropertySet(const char *key, const char *val);
	const char * SCI_METHOD DescribeWordListSets() {
		return osFM.DescribeWordListSets();
	}
	int SCI_METHOD WordListSet(int n, const char *wl);
	void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess);
	void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess);

	void * SCI_METHOD PrivateCall(int cmd, void * pnt) {
		if(cmd < 32){
			const char *wl = reinterpret_cast<char *>(pnt);
			WordListSet(cmd,wl);
		}
		return 0;
	}
	int SCI_METHOD LineEndTypesSupported() noexcept override {
		return SC_LINE_END_TYPE_UNICODE;
	}
	int SCI_METHOD AllocateSubStyles(int styleBase, int numberStyles) noexcept override {
		return -1;
	}
	int SCI_METHOD SubStylesStart(int)  noexcept override {
		return -1;
	}

	int SCI_METHOD SubStylesLength(int)  noexcept override {
		return 0;
	}

	int SCI_METHOD StyleFromSubStyle(int subStyle)  noexcept override {
		return subStyle;
	}
	int SCI_METHOD PrimaryStyleFromStyle(int style)  noexcept override {
		return style;
	}

	void SCI_METHOD FreeSubStyles()  noexcept override {}

	void SCI_METHOD SetIdentifiers(int, const char *)  noexcept override {}

	int SCI_METHOD DistanceToSecondaryStyles()  noexcept override {
		return 0;
	}

	const char * SCI_METHOD GetSubStyleBases()  noexcept override {
		return { 0 };
	}
	int SCI_METHOD NamedStyles()  noexcept override {
		return 0;
	}

	const char * SCI_METHOD NameOfStyle(int style) {
		return "";
	}

	const char * SCI_METHOD TagsOfStyle(int style) {
		return "";
	}

	const char * SCI_METHOD DescriptionOfStyle(int style) {
		return "";
	}
	// ILexer5 methods
	const char * SCI_METHOD GetName() override {
		return "formenjine";
	}
	int SCI_METHOD  GetIdentifier() override {
		return SCLEX_FORMENJINE;
	}
	const char * SCI_METHOD PropertyGet(const char *key) override;

	static ILexer5 *LexerFactoryFM() {
		return new LexerFormEngine(true);
	}
	static ILexer5 *LexerFactoryCPPInsensitive() {
		return new LexerFormEngine(false);
	}

};

int SCI_METHOD LexerFormEngine::PropertySet(const char *key, const char *val) {
	if (osFM.PropertySet(&options, key, val)) {
		if (!strcmp(key, "precompiller.debugsuffix")) {
			wDebug.Set(val);
		}
		return 0;
	}
	return -1;
}

int SCI_METHOD LexerFormEngine::WordListSet(int n, const char *wl) {
	WordList *wordListN = 0;
	wordListN = &keywords[n];
	int firstModification = -1;
	if (wordListN) {
		WordList wlNew;
		wlNew.Set(wl);
		if (*wordListN != wlNew) {
			wordListN->Set(wl);
			firstModification = 0;
		}
	}
	return firstModification;
}

const char * SCI_METHOD LexerFormEngine::PropertyGet(const char *key) {
	return osFM.PropertyGet(key);
}

 //Functor used to truncate history

static bool IsVBComment(Accessor &styler, int pos, int len) {
	return len > 0 && styler[pos] == '\'';
}

static inline bool IsTypeCharacter(int ch) {
	return ch == '%' || ch == '&' || ch == '@' || ch == '!' || ch == '#' || ch == '$';
}

// Extended to accept accented characters
static inline bool IsAWordChar(int ch) {
	return ch >= 0x80 ||
		(isalnum(ch) || ch == '_');//|| ch == '.'
}

static inline bool IsAWordStart(int ch) {
	return ch >= 0x80 ||
		(isalpha(ch) || ch == '_');
}

static inline bool IsANumberChar(int ch) {
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases.
	return (ch < 0x80) &&
		(isdigit(ch) || toupper(ch) == 'E' ||
		ch == '.' || ch == '-' || ch == '+');
}
static inline int onStartNextLine(int initStyle)
{
	// Do not leak onto next line
	switch (initStyle)
	{
	case SCE_FM_VB_AFTERSTRINGCONT:
		initStyle = SCE_FM_VB_ERROR;
		break;
	case SCE_FM_VB_STRINGEOL:
	case SCE_FM_VB_COMMENT:
	case SCE_FM_VB_ERROR:
		initStyle = SCE_FM_VB_DEFAULT;
		break;
	case SCE_FM_VB_STRINGCONT:
		initStyle = SCE_FM_VB_AFTERSTRINGCONT;
		break;
	case SCE_FM_PREPROCESSOR:
		initStyle = SCE_FM_DEFAULT;
		break;
	case SCE_FM_X_ERROR:
		initStyle = SCE_FM_X_DEFAULT;
		break;
	case SCE_FM_X_IDENTIFIER:
		initStyle = SCE_FM_X_ERROR;
		break;
	case SCE_FM_SQL_LINE_COMMENT:
	case SCE_FM_SQL_FMPARAMETR:
		initStyle = SCE_FM_SQL_DEFAULT;
		break;
	}
	return initStyle;
}

static inline int GetSector(int style){
	if (style < SCE_FM_VB_DEFAULT) return TYPE_SPACE;
	if (style < SCE_FM_X_DEFAULT) return TYPE_VBS;
	if (style < SCE_FM_SQL_DEFAULT) return TYPE_XML;
	return TYPE_SQL;
}

static bool isMSSQLOperator(char ch) {
	if (isascii(ch) && isalnum(ch))
		return false;
	// '.' left out as it is used to make up numbers
	if (ch == '%' || ch == '^' || ch == '&' || ch == '*' ||
		ch == '-' || ch == '+' || ch == '=' || ch == '|' ||
		ch == '<' || ch == '>' || ch == '/' ||
		ch == '!' || ch == '~' || ch == '(' || ch == ')' ||
		ch == ',')
		return true;
	return false;
}

void SCI_METHOD LexerFormEngine::ResolveSqlID(StyleContext &sc)
{
	switch(sc.state){

	case SCE_FM_SQL_IDENTIFIER:
		{
			char s[256];
			sc.GetCurrentLowered(s, sizeof(s));
			if (keywords[KW_MSSQL_STATEMENTS].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_STATEMENT);
			} else if (keywords[KW_MSSQL_DATA_TYPES].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_DATATYPE);
			} else if (keywords[KW_MSSQL_SYSTEM_TABLES].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_SYSTABLE);
			} else if (keywords[KW_MSSQL_FUNCTIONS].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_FUNCTION);
			}else if (keywords[KW_MSSQL_STORED_PROCEDURES].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_STORED_PROCEDURE);
			}else if (keywords[KW_MSSQL_FUNCTIONSEX].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_FUNCTIONSEX);
			}
			sc.SetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	case SCE_FM_SQL_GLOBAL_VARIABLE_2:
		{
			char s[256];
			sc.GetCurrentLowered(s, sizeof(s));
			if (keywords[KW_MSSQL_GLOBAL_VARIABLES].InList(s)) {
				sc.ChangeState(SCE_FM_SQL_GLOBAL_VARIABLE);
			}
			sc.SetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	}

}
void SCI_METHOD LexerFormEngine::ColoriseSQL(StyleContext &sc) {

	switch(sc.state){
	case SCE_FM_SQL_OPERATOR:
		sc.SetState(SCE_FM_SQL_DEFAULT);
		break;
	case SCE_FM_SQL_IDENTIFIER:
		if (!iswordchar(sc.ch)) ResolveSqlID(sc);
		break;
	case SCE_FM_SQL_GLOBAL_VARIABLE_2:
		if (!iswordchar(sc.ch)) ResolveSqlID(sc);
		break;
	case SCE_FM_SQL_NUMBER:
		if(!(isdigit(sc.ch) || sc.ch == '.')){
			sc.SetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	case SCE_FM_SQL_VARIABLE:
		if(sc.ch == '@'){
			sc.Forward();
			sc.ChangeState(SCE_FM_SQL_GLOBAL_VARIABLE);
			sc.SetState(SCE_FM_SQL_GLOBAL_VARIABLE_2);
		}else if(!iswordchar(sc.ch)){
			sc.SetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	case SCE_FM_SQL_COMMENT:
		if (sc.ch == '/' && sc.chPrev == '*') {
			sc.ForwardSetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	case SCE_FM_SQL_STRING:
		if (sc.ch == '\'') {
			if ( sc.chNext == '\'' ) {
				sc.Forward();
			} else {
				sc.ForwardSetState(SCE_FM_SQL_DEFAULT);
			}
		}
		break;
	case SCE_FM_SQL_COLUMN_NAME:
		if (sc.ch == '"') {
			if (sc.chNext == '"') {
				sc.Forward();
			} else {
				sc.ForwardSetState(SCE_FM_SQL_DEFAULT);
			}
		}
		break;
	case SCE_FM_SQL_COLUMN_NAME_2:
		if (sc.ch == ']') {
			sc.ForwardSetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	case SCE_FM_SQL_FMPARAMETR:
		if (sc.ch == '}') {
			sc.ForwardSetState(SCE_FM_SQL_DEFAULT);
		}
		break;
	}

	if(sc.state == SCE_FM_SQL_DEFAULT){
		if (isdigit(sc.ch)){
			sc.SetState(SCE_FM_SQL_NUMBER);
		} else if (iswordstart(sc.ch)) {
			sc.SetState(SCE_FM_SQL_IDENTIFIER);
		} else if (sc.ch == '/' && sc.chNext == '*') {
			sc.SetState(SCE_FM_SQL_COMMENT);
			sc.Forward(2);
		} else if (sc.ch == '-' && sc.chNext == '-') {
			sc.SetState(SCE_FM_SQL_LINE_COMMENT);
		} else if (sc.ch == '\'') {
			sc.SetState(SCE_FM_SQL_STRING);
		} else if (sc.ch == '"') {
			sc.SetState(SCE_FM_SQL_COLUMN_NAME);
		} else if (sc.ch == '[') {
			sc.SetState(SCE_FM_SQL_COLUMN_NAME_2);
		} else if (isMSSQLOperator(sc.ch)) {
			sc.SetState(SCE_FM_SQL_OPERATOR);
		} else if (sc.ch == '@') {
			sc.SetState(SCE_FM_SQL_VARIABLE);
		}  else if (sc.ch == '{') {
			sc.SetState(SCE_FM_SQL_FMPARAMETR);
		}
	}

	//////////////////////styler.ColourTo(lengthDoc - 1, state);
}


void SCI_METHOD LexerFormEngine::ColoriseXML(StyleContext &sc){

	switch(sc.state){
	case SCE_FM_X_DEFAULT:
		if(sc.ch == '<'){
			sc.SetState(SCE_FM_X_TAG);
		}else if(sc.chPrev == '<'){
			sc.ChangeState(SCE_FM_X_TAG);
			ColoriseXML(sc);
		}else if(sc.ch == '#'){
			sc.SetState(SCE_FM_PREPROCESSOR);
			return;
		}//else if(!IsASpace(sc.ch)){
		//	sc.SetState(SCE_FM_X_ERROR);
		//}
	return;
	case SCE_FM_X_TAG:
		if(sc.ch == '"'){
			sc.SetState(SCE_FM_X_STRING);
		}else if(sc.ch == '>'){
			if(!sc.Match("><")){
				sc.ForwardSetState(SCE_FM_X_DEFAULT);
			}
		}else if(sc.ch == '<'){
			if(sc.Match("<![CDATA[")){
				sc.SetState(SCE_FM_CDATA);
			}else if(sc.chPrev != '>'){
				sc.SetState(SCE_FM_X_ERROR);
			}
		}else if(IsAWordChar(sc.ch) && isspacechar(sc.chPrev)){
			sc.SetState(SCE_FM_X_UNKNOWNPROP);
		}else if(IsAWordChar(sc.ch)){
			sc.SetState(SCE_FM_X_IDENTIFIER);
		}else if(sc.chPrev == '<'){
			if(sc.Match("![CDATA[")){
				sc.ChangeState(SCE_FM_CDATA);
			}else if(sc.Match("!--")){
				sc.ChangeState(SCE_FM_X_COMMENT);
			}else if((sc.ch != '/' && sc.ch != '?' && !IsAWordChar(sc.ch)) || (sc.ch == '/' && !IsAWordChar(sc.chNext))){
				sc.SetState(SCE_FM_X_ERROR);
			}
		}else if(sc.ch == '/' && ((sc.chPrev != '<' && sc.chNext != '>') || (sc.chPrev == '<' && !IsAWordChar(sc.chNext)))){
			sc.SetState(SCE_FM_X_ERROR);
		}else if(sc.chPrev == '>'){
			sc.SetState(SCE_FM_X_DEFAULT);
		}
	return;
	case SCE_FM_X_STRING:
		if(sc.ch == '"'){
			if((sc.chNext != '>' && sc.chNext != '/' && sc.chNext != '?' && !IsASpace(sc.chNext)) ||(sc.chNext == '/' && sc.GetRelative(2) != '>') ){
				sc.ForwardSetState(SCE_FM_X_ERROR);
			}else{
				sc.ForwardSetState(SCE_FM_X_TAG);
			}
		}else if(sc.ch == '<'){
			sc.SetState(SCE_FM_X_ERROR);
		}
	return;
	case SCE_FM_X_COMMENT:
		if(sc.Match("-->")){
			sc.Forward(3);
			sc.SetState(SCE_FM_X_DEFAULT);
		}
	return;
	case SCE_FM_X_UNKNOWNPROP:
		if (!IsAWordChar(sc.ch)) {
			char s[100];
			sc.GetCurrentLowered(s, sizeof(s));

			if (keywords[KW_X_TAGPROPERTIES].InList(s)) {
				sc.ChangeState(SCE_FM_X_TAGPROPERTIES);
			}else{
				sc.ChangeState(SCE_FM_X_TAG);
			}
			sc.SetState(SCE_FM_X_TAG);
		}
		return;
	case SCE_FM_X_IDENTIFIER:
		if (!IsAWordChar(sc.ch)) {
			char s[100];
			sc.GetCurrentLowered(s, sizeof(s));

			if (keywords[KW_X_TAGNAMES].InList(s)) {
				sc.ChangeState(SCE_FM_X_TAGNAMES);
			}else{
				sc.ChangeState(SCE_FM_X_TAG);
			}
			sc.SetState(SCE_FM_X_TAG);
		}
		if (sc.ch == '>'){
			if (!sc.Match("><")){
				sc.ForwardSetState(SCE_FM_X_DEFAULT);
			}
		}
	return;
	}
}


void SCI_METHOD LexerFormEngine::ResolveVBId(StyleContext &sc)
{
	bool skipType = false;
	switch(sc.state){
	case SCE_FM_VB_IDENTIFIER:
		if (IsTypeCharacter(sc.ch)) {
			sc.Forward();	// Skip it
			skipType = true;
		}
		if (sc.ch == ']') {
			sc.Forward();
		}
		char s[100];
		sc.GetCurrentLowered(s, sizeof(s));
		if (skipType) {
			s[strlen(s) - 1] = '\0';
		}
		if (strcmp(s, "_") == 0) {
			sc.ChangeState(SCE_FM_VB_STRINGCONT);
			return;
		}
		if (strcmp(s, "rem") == 0) {
			sc.ChangeState(SCE_FM_VB_COMMENT);
		} else {
			if (keywords[KW_VB_KEYWORDS].InList(s)) {
				sc.ChangeState(SCE_FM_VB_KEYWORD);
			} else if (keywords[KW_VB_FUNCTIONS].InList(s)) {
				sc.ChangeState(SCE_FM_VB_FUNCTIONS);
			} else if (keywords[KW_VB_OBJECTS].InList(s)) {
				sc.ChangeState(SCE_FM_VB_OBJECTS);
			} else if (keywords[KW_VB_CONSTANTS].InList(s)) {
				sc.ChangeState(SCE_FM_VB_CONSTANTS);
			} else if (keywords[KW_VB_FUNCTIONSEX].InList(s)) {
				sc.ChangeState(SCE_FM_VB_FUNCTIONSEX);
			}
			sc.SetState(SCE_FM_VB_DEFAULT);
		}
		break;
	case SCE_FM_VB_UNKNOWNPROP:
		{
			char s[100];
			sc.GetCurrentLowered(s, sizeof(s));
			if (keywords[KW_VB_PROPERTIES].InList(s)) {
					sc.ChangeState(SCE_FM_VB_PROPERTIES);
			}
		}
		sc.SetState(SCE_FM_VB_DEFAULT);
	}
}

void SCI_METHOD LexerFormEngine::ColoriseVBS(StyleContext &sc, int &visibleChars,int &fileNbDigits){
	switch(sc.state){
	case SCE_FM_VB_OPERATOR:
		sc.SetState(SCE_FM_VB_DEFAULT);
	break;
	case SCE_FM_VB_IDENTIFIER:
	case SCE_FM_VB_UNKNOWNPROP:
		if (!IsAWordChar(sc.ch)) ResolveVBId(sc);
	break;
	case SCE_FM_VB_NUMBER:
		// We stop the number definition on non-numerical non-dot non-eE non-sign char
		// Also accepts A-F for hex. numbers
		if (!IsANumberChar(sc.ch) && !(tolower(sc.ch) >= 'a' && tolower(sc.ch) <= 'f')) {
			sc.SetState(SCE_FM_VB_DEFAULT);
		}
	break;
	case SCE_FM_VB_STRING:
		// VB doubles quotes to preserve them, so just end this string
		// state now as a following quote will start again
		if (sc.ch == '\"') {
			if (sc.chNext == '\"') {
				sc.Forward();
			} else {
				if (tolower(sc.chNext) == 'c') {
					sc.Forward();
				}
				sc.ForwardSetState(SCE_FM_VB_DEFAULT);
			}
		} else if (sc.atLineEnd) {
			visibleChars = 0;
			sc.ChangeState(SCE_FM_VB_STRINGEOL);
			sc.ForwardSetState(SCE_FM_VB_DEFAULT);
		}
	break;
	case SCE_FM_VB_STRINGCONT:
		if(!IsASpace(sc.ch)){
			sc.SetState(SCE_FM_VB_ERROR);
		}
		break;
	case SCE_FM_VB_AFTERSTRINGCONT:
		if(sc.ch == '\''){
			sc.SetState(SCE_FM_VB_ERROR);
		}else if(!IsASpace(sc.ch)){
			sc.SetState(SCE_FM_VB_DEFAULT);
		}
		break;
	case SCE_FM_VB_COMMENT:
		if (sc.atLineEnd) {
			visibleChars = 0;
			sc.ForwardSetState(SCE_FM_VB_DEFAULT);
		}
	break;
	case SCE_FM_PREPROCESSOR:
		if (sc.atLineEnd) {
			visibleChars = 0;
			//sc.ForwardSetState(SCE_FM_VB_DEFAULT);
		}
	break;
	case SCE_FM_VB_DATEORNUMBER:
		if (IsADigit(sc.ch)) {
			fileNbDigits++;
			if (fileNbDigits > 3) {
				sc.ChangeState(SCE_FM_VB_DATE);
			}
		} else if (sc.ch == '\r' || sc.ch == '\n' || sc.ch == ',') {
			// Regular uses: Close #1; Put #1, ...; Get #1, ... etc.
			// Too bad if date is format #27, Oct, 2003# or something like that...
			// Use regular number state
			sc.ChangeState(SCE_FM_VB_NUMBER);
			sc.SetState(SCE_FM_VB_DEFAULT);
		} else if (sc.ch == '#') {
			sc.ChangeState(SCE_FM_VB_DATE);
			sc.ForwardSetState(SCE_FM_VB_DEFAULT);
		} else {
			sc.ChangeState(SCE_FM_VB_DATE);
		}
		if (sc.state != SCE_FM_VB_DATEORNUMBER) {
			fileNbDigits = 0;
		}
	break;
	case SCE_FM_VB_DATE:
		if (sc.atLineEnd) {
			visibleChars = 0;
			sc.ChangeState(SCE_FM_VB_STRINGEOL);
			sc.ForwardSetState(SCE_FM_VB_DEFAULT);
		} else if (sc.ch == '#') {
			sc.ForwardSetState(SCE_FM_VB_DEFAULT);
		}
	case SCE_FM_VB_UNACTIVE:
		if (sc.atLineStart && !options.debugmode && sc.Match("\'#ENDDEBUG")) {
			sc.Forward(10);
			if (IsASpace(sc.ch)) {
				sc.ForwardSetState(SCE_FM_VB_COMMENT);
			}
		}
	}

	if (sc.state == SCE_FM_VB_DEFAULT) {
		if (sc.ch == '\'') {
			sc.SetState(SCE_FM_VB_COMMENT);
			if(visibleChars == 0 && options.debugmode){
				bool atLineStart = sc.atLineStart;
				sc.Forward();
				if(sc.Match('#')){
					sc.Forward();
					if(sc.Match("DEBUG")){
						sc.Forward(5);
						if(IsASpace(sc.ch)){
							sc.ChangeState(SCE_FM_PREPROCESSOR);
							sc.SetState(SCE_FM_VB_DEFAULT);
						} else if (sc.ch == '-') {
							char s[100];
							sc.Forward();
							int i = 0;
							while (IsAlphaNumeric(sc.ch)) {
								s[i] = sc.ch;
								sc.Forward();
								i++;
							}
							s[i] = 0;
							if (wDebug.InList(s)) {
								sc.ChangeState(SCE_FM_PREPROCESSOR);
								sc.SetState(SCE_FM_VB_DEFAULT);
							}
						}
					} else if (atLineStart) {
						if (sc.Match("STARTDEBUG")) {
							sc.Forward(10);
							if (IsASpace(sc.ch)) {
								sc.ChangeState(SCE_FM_PREPROCESSOR);
								sc.SetState(SCE_FM_VB_DEFAULT);
							}
						} else if (sc.Match("ENDDEBUG")) {
							sc.Forward(8);
							if (IsASpace(sc.ch)) {
								sc.ChangeState(SCE_FM_PREPROCESSOR);
								sc.SetState(SCE_FM_VB_DEFAULT);
							}
						}
					}
				}
			} else if (sc.atLineStart) {
				if (sc.Match("\'#STARTDEBUG")) {
					sc.Forward(12);
					if (IsASpace(sc.ch)) {
						sc.ChangeState(SCE_FM_VB_UNACTIVE);
					}
				}
			}
		} else if (sc.ch == '\"') {
			sc.SetState(SCE_FM_VB_STRING);
		} else if (sc.ch == '#' && visibleChars == 0) {
			// Preprocessor commands are alone on their line
			sc.SetState(SCE_FM_PREPROCESSOR);
		} else if (sc.ch == '#') {
			// It can be a date literal, ending with #, or a file number, from 1 to 511
			// The date literal depends on the locale, so anything can go between #'s.
			// Can be #January 1, 1993# or #1 Jan 93# or #05/11/2003#, etc.
			// So we set the FILENUMBER state, and switch to DATE if it isn't a file number
			sc.SetState(SCE_FM_VB_DATEORNUMBER);
		} else if (sc.ch == '&' && tolower(sc.chNext) == 'h') {
			// Hexadecimal number
			sc.SetState(SCE_FM_VB_NUMBER);
			sc.Forward();
		} else if (sc.ch == '&' && tolower(sc.chNext) == 'o') {
			// Octal number
			sc.SetState(SCE_FM_VB_NUMBER);
			sc.Forward();
		} else if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
			sc.SetState(SCE_FM_VB_NUMBER);
		} else if (IsAWordStart(sc.ch) && (sc.chPrev == '.')) {
			sc.SetState(SCE_FM_VB_UNKNOWNPROP);
		} else if (IsAWordStart(sc.ch) || (sc.ch == '[')) {
			sc.SetState(SCE_FM_VB_IDENTIFIER);
		} else if (isoperator(static_cast<char>(sc.ch)) || (sc.ch == '\\')) {	// Integer division
			sc.SetState(SCE_FM_VB_OPERATOR);
		}
	}
}

void SCI_METHOD LexerFormEngine::Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	if(options.frozen) return;
	LexAccessor styler(pAccess);
	bool vbScriptSyntax = true;
	//styler.StartAt(startPos);

	int visibleChars = 0;
	int fileNbDigits = 0;

	// Do not leak onto next line
	//initStyle =onStartNextLine(initStyle);

	StyleContext sc(startPos, length, initStyle, styler, (char)(STYLE_MAX));

	//!	for (; sc.More(); sc.Forward()) {
	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) { //!-change-[LexersLastWordFix]

		if (sc.atLineStart){
			sc.SetState(onStartNextLine(sc.state));
		}
		if(sc.state == SCE_FM_DEFAULT){
			if(sc.ch == '<') sc.ChangeState(SCE_FM_X_DEFAULT);
			else if(!IsASpace(sc.ch))sc.ChangeState(SCE_FM_VB_DEFAULT);
		}
		if(sc.ch == ']' && sc.chNext == ']'){
			if(sc.GetRelative(2)=='>')sc.SetState(SCE_FM_CDATA);
			continue;
		}
		switch(GetSector(sc.state)){
		case TYPE_SPACE:
			if(sc.state == SCE_FM_CDATA){
				if(sc.Match("['']") || sc.Match("[']")){
					sc.ForwardSetState(SCE_FM_VB_COMMENT);
				}else if(sc.Match("[--]")){
					sc.ForwardSetState(SCE_FM_SQL_LINE_COMMENT);
					sc.Forward((3));
				}else if(sc.Match("[<!--]-->")){
					sc.ForwardSetState(SCE_FM_X_DEFAULT);
				}else if(sc.ch == '>'){
					sc.ForwardSetState(SCE_FM_X_DEFAULT);
				}else if(sc.chPrev == 'A' && sc.ch == '['){
					sc.ForwardSetState(SCE_FM_PLAINCDATA);
				}
			}
			break;
		case TYPE_XML:
			ColoriseXML(sc);
			break;
		case TYPE_SQL:
			ColoriseSQL(sc);
			break;
		case TYPE_VBS:
			ColoriseVBS(sc, visibleChars, fileNbDigits);
			break;
		}

		if (sc.atLineEnd) {
			visibleChars = 0;
		}
		if (!IsASpace(sc.ch)) {
			visibleChars++;
		}
	}
	if (!IsAWordChar(sc.ch)){
		switch(sc.state){
		case SCE_FM_VB_IDENTIFIER:
		case SCE_FM_X_UNKNOWNPROP:
			ResolveVBId(sc);
			break;
		case SCE_FM_SQL_GLOBAL_VARIABLE_2:
		case SCE_FM_SQL_IDENTIFIER:
			ResolveSqlID(sc);
			break;
		}
	}

	sc.Complete();
	styler.Flush();
}

// Store both the current line's fold level and the next lines in the
// level store to make it easy to pick up with each increment
// and to make it possible to fiddle the current level for "} else {".
class FoldContext {
	unsigned int endPos;
	LexAccessor &styler;
	//FoldContext &operator=(const FoldContext &);
	void GetNextChar(unsigned int pos) {
		chNext = static_cast<unsigned char>(styler.SafeGetCharAt(pos+1));
		if (styler.IsLeadByte(static_cast<char>(chNext))) {
			chNext = chNext << 8;
			chNext |= static_cast<unsigned char>(styler.SafeGetCharAt(pos+2));
		}
		// End of line?
		// Trigger on CR only (Mac style) or either on LF from CR+LF (Dos/Win)
		// or on LF alone (Unix). Avoid triggering two times on Dos/Win.
		atLineEnd = (ch == '\r' && chNext != '\n') ||
			(ch == '\n') ||
			(currentPos >= endPos);
	}
public:
	unsigned int currentPos;
	bool atLineStart;
	bool atLineEnd;
	int state;
	int ch;
	int chNext;
	int currentLine;
	int currentLevel;
	int prevLevel;
	int visibleChars;
	int style;
	bool foldCompact;
	bool startLineResolved; //Иногда в начале строки для корректной обработке придется подняться вверх; переменная выставляется в True, когда такая ситуация уже невозможна
	FoldContext(unsigned int startPos, unsigned int length,LexAccessor &styler_,  bool bFoldCompact):
	styler(styler_),
		endPos(startPos + length),
		currentPos(startPos),
		atLineStart(true),
		atLineEnd(false),
		ch(0),
		foldCompact(bFoldCompact),
		visibleChars(0){
			unsigned int pos = currentPos;
			ch = static_cast<unsigned char>(styler.SafeGetCharAt(pos));
			if (styler.IsLeadByte(static_cast<char>(ch))) {
				pos++;
				ch = ch << 8;
				ch |= static_cast<unsigned char>(styler.SafeGetCharAt(pos));
			}
			GetNextChar(pos);

			currentLine = styler.GetLine(currentPos);
			startLineResolved = !currentLine; //на нулевой строке мы не сможем подняться вверх за дополнительной информацией
			style = styler.StyleAt(currentPos);
			currentLevel = styler.LevelAt(currentLine) & SC_FOLDLEVELNUMBERMASK;
			prevLevel = currentLevel;
			foldCompact = bFoldCompact;
	}
	bool More() const {
		return currentPos < endPos;
	}
	void SetPrevLevel(int iLevel){
		int pprev = styler.LevelAt(currentLine-1)&~SC_FOLDLEVELHEADERFLAG;
		if(pprev > iLevel){
			iLevel |= SC_FOLDLEVELHEADERFLAG;
		}
		styler.SetLevel(currentLine-1, pprev);
		styler.SetLevel(currentLine, iLevel);
		prevLevel = iLevel;
	}
	void SetCurLevel(int iLevel){
		styler.SetLevel(currentLine, iLevel);
		prevLevel = iLevel;
	}
	void Forward(int n);
	void Forward();
	void GetRangeLowered(unsigned int start,unsigned int end,char *s,unsigned int len);
	bool GetNextLowered(char *s,unsigned int len);
	int MatchLowerStyledLine(unsigned int line, const char *mtch, const int style, IDocument *pAccess);
	bool Skip();
	void WalkToEOL();

};

void FoldContext::Forward(int n){
	for(int i = 0; i < n; i++, Forward());
}
void FoldContext::Forward(){
	if (atLineEnd && currentPos < endPos){
		int lev = prevLevel;
		if (visibleChars == 0 && foldCompact) {
			lev |= SC_FOLDLEVELWHITEFLAG;
		}
		if ((currentLevel > prevLevel) && (visibleChars > 0)) {
			lev |= SC_FOLDLEVELHEADERFLAG;
		}
		if (lev != styler.LevelAt(currentLine)) {
			styler.SetLevel(currentLine, lev);
		}
		visibleChars = 0;
		if (currentPos < endPos) {
			currentLine++;

		if ((currentLevel & SC_FOLDLEVELNUMBERMASK) < SC_FOLDLEVELBASE)
			currentLevel = SC_FOLDLEVELBASE;
		prevLevel = currentLevel;
			//currentLevel = styler.LevelAt(currentLine);
		}
	}else if(visibleChars || !isspacechar(ch)) visibleChars++;
	///////
	if (currentPos < endPos) {

		atLineStart = atLineEnd;
		currentPos++;
		if (ch >= 0x100)
			currentPos++;
		ch = chNext;
		GetNextChar(currentPos + ((ch >= 0x100) ? 1 : 0));
	} else {
		atLineStart = false;
		ch = ' ';
		chNext = ' ';
		atLineEnd = true;

		int flagsNext = styler.LevelAt(currentLine) & ~SC_FOLDLEVELNUMBERMASK;
		styler.SetLevel(currentLine, prevLevel | flagsNext);
	}
	/////////
	style = styler.StyleAt(currentPos);
}
void FoldContext::GetRangeLowered(unsigned int start,
	unsigned int end,
	char *s,
	unsigned int len) {
		unsigned int i = 0;
		while ((i < end - start + 1) && (i < len-1)) {
			s[i] = static_cast<char>(tolower(styler[start + i]));
			i++;
		}
		s[i] = '\0';
}
int FoldContext::MatchLowerStyledLine(unsigned int line, const char *mtch, const int style, IDocument *pAccess) {
	int l = currentLine - line;
	if (l < 0)
		return 0;
	int ls = pAccess->LineStart(l) + pAccess->GetLineIndentation(l);
	int lineStyle = styler.StyleAt(ls);
	if(lineStyle != style)
		return lineStyle;
	int len = strlen(mtch);
	for (int i = 0; i < len; i++) {
		if (mtch[i] != static_cast<char>(tolower(styler.SafeGetCharAt(ls + i))))
			return lineStyle;
	}
	return 0;
}
bool FoldContext::GetNextLowered(char *s,unsigned int len){
	int startstyle = style;
	unsigned int i = 0;
	s[0] = 0;
	while ((i < len-1) && (startstyle == style ) && More()) {
		s[i] = static_cast<char>(tolower(ch));
		i++;
		if((chNext == '\n') || (chNext == '\r')) break;
		Forward();
	}
	s[i] = '\0';
	return startstyle != style;
}
bool FoldContext::Skip(){
	int startstyle = style;
	while ((startstyle == style ) && More() && !atLineEnd && chNext != '\n' && chNext != '\r') {
		Forward();
	}
	return startstyle != style;
}
void FoldContext::WalkToEOL(){
	while(!atLineEnd && currentPos < endPos){
		atLineStart = atLineEnd;
		currentPos++;
		if (ch >= 0x100)
			currentPos++;
		ch = chNext;
		GetNextChar(currentPos + ((ch >= 0x100) ? 1 : 0));
	}
}
bool LexerFormEngine::PlainFold(unsigned int startPos, int length, int initStyle, IDocument *pAccess, LexAccessor &styler){

	FoldContext fc(startPos, length, styler, options.foldCompact);
	int blockReFoldLine = -1;
	int processComment = 0;
	for (bool doing = fc.More(); doing; doing = fc.More(), fc.Forward()){
		switch(GetSector(fc.style)){
		case TYPE_SPACE:
			if(fc.style == SCE_FM_CDATA && options.foldcdata){
				switch (fc.ch){
				case '>': //]]>
				case '!': //<|![CDATA[
					fc.SetPrevLevel(SC_FOLDLEVELBASE);
					fc.currentLevel = SC_FOLDLEVELBASE + 1;
					while(fc.More() && !fc.atLineStart) fc.Forward();
					break;
				}
			}
			break;
		case TYPE_XML:
			switch(fc.style){
			case SCE_FM_X_DEFAULT:
			case SCE_FM_X_TAG:
				for (bool doing = fc.More(); doing; doing = fc.More() && ! fc.atLineEnd){
					if(fc.style == SCE_FM_X_TAG){
						if((fc.ch == '<') && (fc.chNext != '?') && (fc.chNext != '/')){
							fc.currentLevel++;
						}else if(fc.ch == '/'){//Слеш в любом(валидном) месте уменьшает фолдинг
							fc.currentLevel--;
						}
					}
					fc.Forward();
					if(GetSector(fc.style) != TYPE_XML) break;
				}
			}
			break;
		case TYPE_SQL:
			for (bool doing = fc.More(); doing; doing = fc.More() && !fc.atLineEnd) {
				switch (fc.style) {
				case SCE_FM_SQL_OPERATOR:
					if (fc.ch == '(')
						fc.currentLevel++;
					else if (fc.ch == ')')
						fc.currentLevel--;
					break;
				case SCE_FM_SQL_STATEMENT:
					if (!fc.visibleChars) {//Нашли ключквое слово в начале строки
						char s[100];
						fc.GetNextLowered(s, 100);
						if (!strcmp(s, "begin"))
							fc.currentLevel++;
						else if (!strcmp(s, "end"))
							fc.currentLevel--;
						continue;
					}
					break;
				}
				fc.Forward();
				if (GetSector(fc.style) != TYPE_SQL) break;
			}
			break;
		case TYPE_VBS:
			switch(fc.style){
			case SCE_FM_VB_DEFAULT:
				if(!fc.visibleChars)continue;
				selectionState = Next;
				break;
			case SCE_FM_VB_KEYWORD:
				if(!fc.visibleChars){//Нашли ключквое слово в начале строки
					char s[100];
					fc.GetNextLowered(s,100);
					if(!strcmp(s, "if")){ //фолдим только если есть последнее Then
						do{
							while(fc.Skip()){
								if(fc.style == SCE_FM_VB_KEYWORD){//остановились перед каким-то ключевым словом = прчтем его
									fc.GetNextLowered(s,100);
								}else if(fc.style != SCE_FM_VB_DEFAULT && fc.style != SCE_FM_VB_COMMENT){//остановились перед чем-то отличным от пустого пространства- значит прочитанное слово нам не нужно
									s[0] = 0;
								}
							}
							if(fc.More()) fc.Forward();
						}
						while((fc.style == SCE_FM_VB_STRINGCONT || fc.style == SCE_FM_VB_AFTERSTRINGCONT)&& fc.More());
						if(!strcmp(s, "then")){//нашли then -действительно нужно фолдить
							fc.currentLevel += 2;
							blockReFoldLine = fc.currentLine + 1;//в следующей строке не фолдим IfElse
						}
						selectionState = Next;
					}else if(!strcmp(s, "on") && options.foldonerror){
						do{
							while(fc.Skip()){
								if(fc.style == SCE_FM_VB_KEYWORD){//остановились перед каким-то ключевым словом = прчтем его
									fc.GetNextLowered(s,100);
								}
							}
							if(fc.More()) fc.Forward();
						}
						while((fc.style == SCE_FM_VB_STRINGCONT || fc.style == SCE_FM_VB_AFTERSTRINGCONT)&& fc.More());
						if(!strcmp(s, "next")){//нашли then -действительно нужно фолдить
							fc.currentLevel ++;
						}else if(!strcmp(s,"goto")){
							fc.currentLevel--;
						}
					}else if(!strcmp(s, "select")){
						fc.currentLevel ++;
						selectionState = Start;
					}else if(wFold.InList(s)){//начало фолдинга - do function sub for with property while
						fc.currentLevel++;
						selectionState = Next;
					}
					else if (!strcmp(s, "case")){
						if (selectionState == Empty && fc.currentLine > 0 && !fc.startLineResolved) {
							int strOut = 1;
							selectionState = Start;
							for (int j = 1; strOut; j++) {
								strOut = fc.MatchLowerStyledLine(j, "select", SCE_FM_VB_KEYWORD, pAccess);
								if(strOut && (strOut != SCE_FM_VB_COMMENT && strOut != SCE_FM_VB_DEFAULT)) {
									selectionState = End;
									break;
								}
							}
						}
						if (selectionState == Start){
							fc.currentLevel++;
						}else{
							fc.SetCurLevel(fc.currentLevel - 1);
						}
						selectionState = Next;
					}
					else if (wRefold.InList(s) && options.foldAtElse ){//промежуточный фолдинг для ифа и свитча - case else elseif
						if (!fc.startLineResolved) {
							char *then = "then";
							int lEnd = pAccess->LineEnd(fc.currentLine - 1);
							bool bFind = false;
							for (int ls = pAccess->LineStart(fc.currentLine - 1); ls <= lEnd; ls++) {
								if (styler.StyleAt(ls) == SCE_FM_VB_KEYWORD && lEnd - ls >= 4) {
									bFind = true;
									for (int i = 0; i < 4; i++) {
										if (then[i] != static_cast<char>(tolower(styler.SafeGetCharAt(ls + i)))) {
											bFind = false;
											break;
										}
									}
									if (bFind) {
										blockReFoldLine = fc.currentLine;
										break;
									}
								}
							}
							fc.startLineResolved = true;
						}
						if(fc.currentLine != blockReFoldLine){
							fc.SetCurLevel(fc.currentLevel - 1); //Фолдим, если не фолдили в прошлой строке
						}
						blockReFoldLine= fc.currentLine + 1;//в следующей строке тоже не рефолдим
						selectionState = Next;
					}else if(wUnfold.InList(s)){//конец фолдинга - end next wend loop
						if (!strcmp(s, "end") && fc.ch != ' ')
							break;
						selectionState = Next;
						if(fc.Skip()){//точно нашли кусок в другом стиле на этой строке
							if(fc.style == SCE_FM_VB_KEYWORD){
								fc.GetNextLowered(s, 100);
								if (!strcmp(s, "select")){
									if(fc.MatchLowerStyledLine(1, "case", SCE_FM_VB_KEYWORD, pAccess))
										fc.SetCurLevel(fc.currentLevel - 1);
									selectionState = End;
								}
								if(!strcmp(s, "if") || !strcmp(s, "select")){
									fc.currentLevel--;
								}
							}
						}
						fc.currentLevel--;
					}
				}
				break;
			case SCE_FM_VB_AFTERSTRINGCONT:
				if(!fc.startLineResolved)return false;
				break;
			case SCE_FM_VB_COMMENT:
				if(!fc.visibleChars && options.foldComment  ){
					if (!fc.startLineResolved) {
						if (!fc.MatchLowerStyledLine(1, "'", SCE_FM_VB_COMMENT, pAccess)) {
							if (fc.MatchLowerStyledLine(2, "'", SCE_FM_VB_COMMENT, pAccess))
								return false;
							processComment = 1;
						}
						fc.startLineResolved = true;
					}
					if(processComment==0) fc.currentLevel++;
						//fc.SetPrevLevel(fc.currentLevel+1);

					//проверим, является ли следующая строка комментарием
				
					int ls = pAccess->GetLineIndentation(fc.currentLine + 1) + pAccess->LineStart(fc.currentLine + 1);
					bool bNoComment = '\'' != styler.SafeGetCharAt(ls, '\0');
					if (!bNoComment && (options.debugmode && ('#' == styler.SafeGetCharAt(ls + 1, '\0')))) {
						bNoComment = styler.Match(ls + 2, "DEBUG ");
						if (!bNoComment && styler.Match(ls + 2, "DEBUG-")){
							char s[100];
							char c;
							for (int j = 0; j < 100; j++) {
								c = styler.SafeGetCharAt(ls + 8 + j, '\0');
								if (!IsAlphaNumeric(c)) {
									s[j] = 0;
									break;
								}
								s[j] = c;
							}
							bNoComment = wDebug.InList(s);
						}
					}
					if (bNoComment) {
						fc.currentLevel--;
						processComment = 0;
					}else{
						processComment++;
					}
					//fc.WalkToEOL();
				}
				continue;
			default:
				selectionState = Next;
				break;
			}
			fc.WalkToEOL(); //закончили обработку бэйсиковской строки
			break;
		}
		fc.startLineResolved = true;
	}
	return true;
}

void SCI_METHOD LexerFormEngine::Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	if (!options.fold || options.frozen) return;

	int unShift = 0;
	LexAccessor styler(pAccess);
	selectionState = Empty;
	while(!PlainFold(startPos, length, initStyle, pAccess, styler)){
		//Возможно разрешить фолдинг начиная с данной строки не удасться, и тогда нам нужно перезапустить фолдинг со строчки выше
		int prevLine = pAccess ->LineFromPosition(startPos) - 1;
		if(prevLine < 0) return; //Хотя вообще-то процедура должна вернуть true для первой строки
		unsigned int startPosNew = pAccess->LineStart(prevLine);
		length += (startPos - startPosNew);
		startPos = startPosNew;
	}

}



LexerModule lmFormEngine(SCLEX_FORMENJINE, LexerFormEngine::LexerFactoryFM, "formenjine", fnWordLists);

