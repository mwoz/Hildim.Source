

#define TYPE_SPACE 0
#define TYPE_VBS 1
#define TYPE_XML 2
#define TYPE_SQL 3
#define TYPE_CUBRFORMULA 4
#define TYPE_WIREFORMAT 5

#define FM_NEXTFOLD 0x100
#define FM_ENDCOMMENT 0x200
#define FM_STARTCDATA 0x400
#define FM_ENDCDATA 0x800
#define FM_PREVCDATA 0x1000
#define FM_TYPEMASK 0x0F

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
#include <regex>
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
	bool frozen;  //флаг для временного отключения на время загрузки хелплистов
	std::string debugsuffix;
	OptionsFM() {
		fold = false;
		foldComment = false;
		foldCompact = false;
		foldAtElse = false;
		debugmode = true;
		foldcdata = false;
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
#define KW_VB_CONSTANTS 6
#define KW_VB_FUNCTIONSEX 7
#define KW_MSSQL_STATEMENTS 8
#define KW_MSSQL_DATA_TYPES 9
#define KW_MSSQL_SYSTEM_TABLES 10
#define KW_MSSQL_GLOBAL_VARIABLES 11
#define KW_MSSQL_FUNCTIONS 12
#define KW_MSSQL_STORED_PROCEDURES 13
#define KW_MSSQL_INDENT_CLASS 14
#define KW_MSSQL_RADIUS 16
#define KW_CF_FUNCTIONS 15

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
	"RadiusKeyWords",
	0,
};

struct OptionsSetFM : public OptionSet<OptionsFM> {
	OptionsSetFM() {

		DefineProperty("fold", &OptionsFM::fold);

		DefineProperty("fold.comment", &OptionsFM::foldComment,
			"This option enables folding multi-line comments and explicit fold points when using the C++ lexer. "
			"Explicit fold points allows adding extra folding by placing a //{ comment at the start and a //} "
			"at the end of a section that should fold.");

		DefineProperty("fold.compact", &OptionsFM::foldCompact);
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
	WordList keywords[17];	//переданные нам вордлисты
	WordList wRefold; //начала фолдинга
	WordList wFold;   //продолжения ифов и свитчей
	WordList wUnfold; //окончания конструкций
	WordList wEndWhat; //Слово после end
	WordList wEndWhat2; //Слово после end - c двойным уменьшением фолдинга
	WordList wDebug; //Используемые в данный момент дебаги с суффиксами
	WordList wTypes_wf; //типы полей в вайрформате
	WordList wKeydords_cf; //ключевые слов в формулах
	std::map<std::string, std::string> preprocessorDefinitionsStart;
	OptionsFM options;
	OptionsSetFM osFM;
	void SCI_METHOD ColoriseVBS(StyleContext &sc, int &visibleChars,int &fileNbDigits);
	void SCI_METHOD ColoriseXML(StyleContext &sc);
	void SCI_METHOD ColoriseSQL(StyleContext &sc);
	bool SCI_METHOD ColoriseWireFormat(StyleContext &sc);
	void SCI_METHOD ColoriseCubeFormula(StyleContext &sc);
	void SCI_METHOD LexerFormEngine::ResolveSqlID(StyleContext &sc);
	void SCI_METHOD LexerFormEngine::ResolveVBId(StyleContext &sc);
	bool PlainFold(unsigned int startPos, int length, int initStyle, IDocument *pAccess, LexAccessor &styler);
	const std::regex reSyntax;
public:
	LexerFormEngine(bool caseSensitive_) :
		setFoldingWordsBegin(CharacterSet::setLower, "idfecnwspl"),
		reSyntax(" syntax=\"(\\w+)\"[^>]*><!\\[cdat\0$", std::regex::ECMAScript),
		caseSensitive(caseSensitive_){
			wRefold.Set("else elseif");
			wFold.Set("do function sub for with private public property class while");
			wUnfold.Set("end next wend loop");
			wEndWhat.Set("with sub function property class");
			wEndWhat2.Set("if select");
			wTypes_wf.Set("string int float datetime bool null empty binary");
			wKeydords_cf.Set("and or function");
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
	unsigned int s = style & 0xFF;
	if (s < SCE_FM_VB_DEFAULT) return TYPE_SPACE;
	if (s < SCE_FM_X_DEFAULT) return TYPE_VBS;
	if (s < SCE_FM_SQL_DEFAULT) return TYPE_XML;
	if (s < SCE_FM_CF_DEFAULT) return TYPE_SQL;
	if (s < SCE_FM_WF_DEFAULT) return TYPE_CUBRFORMULA;
	return TYPE_WIREFORMAT;
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
			sc.GetCurrent(s, sizeof(s));
			if (s[0] == '_' && s[1] == '_') {
				if (keywords[KW_MSSQL_RADIUS].InList(s)) {
					sc.ChangeState(SCE_FM_SQL_RADIUSKEYWORDS);
				}
			} else {
				char *c = s;
				while (*c) {
					if (*c >= 'A' && *s <= 'Z') {
						*c += 'a' - 'A';
					}
					++c;
				}
				if (keywords[KW_MSSQL_STATEMENTS].InList(s)) {
					sc.ChangeState(SCE_FM_SQL_STATEMENT);
				} else if (keywords[KW_MSSQL_DATA_TYPES].InList(s)) {
					sc.ChangeState(SCE_FM_SQL_DATATYPE);
				} else if (keywords[KW_MSSQL_SYSTEM_TABLES].InList(s)) {
					sc.ChangeState(SCE_FM_SQL_SYSTABLE);
				} else if (keywords[KW_MSSQL_FUNCTIONS].InList(s)) {
					sc.ChangeState(SCE_FM_SQL_FUNCTION);
				} else if (keywords[KW_MSSQL_STORED_PROCEDURES].InList(s)) {
					sc.ChangeState(SCE_FM_SQL_STORED_PROCEDURE);
				}
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
		} else if (isoperator(static_cast<char>(sc.ch))) {
			sc.SetState(SCE_FM_VB_OPERATOR);
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

void SCI_METHOD LexerFormEngine::ColoriseCubeFormula(StyleContext &sc) {
	if (sc.state == SCE_FM_CF_IDENTIFIER && !iswordchar(sc.ch)) {
		char s[256];

		sc.GetCurrentLowered(s, sizeof(s));
		if (wKeydords_cf.InList(s)) {
			sc.ChangeState(SCE_FM_CF_KEYWORD);
			sc.SetState(SCE_FM_CF_DEFAULT);
		} else if (keywords[KW_CF_FUNCTIONS].InList(s)) {
			sc.ChangeState(SCE_FM_CF_FUNCTION);
			sc.SetState(SCE_FM_CF_DEFAULT);
		} else {
			sc.ChangeState(SCE_FM_CF_DEFAULT);
		}
	}
	switch (sc.state) {
	case SCE_FM_CF_STRING:
		if (sc.ch == '\"') {
			if (sc.chNext == '\"') {
				sc.Forward();
			} else {
				sc.ForwardSetState(SCE_FM_CF_DEFAULT);
			}
		}
		break;
	case SCE_FM_CF_COLUMN:
		if (sc.ch == ']')
			sc.ForwardSetState(SCE_FM_CF_DEFAULT);
		break;
	case SCE_FM_CF_NUMBER:
		if (!(isdigit(sc.ch) || sc.ch == '.')) {
			sc.SetState(SCE_FM_CF_DEFAULT);
		}
		break;
	case SCE_FM_CF_COMMENT:
		if (sc.ch == '\n' || sc.ch == '\r') {
			sc.ForwardSetState(SCE_FM_CF_DEFAULT);
		}
		break;
	case SCE_FM_CF_PARAMETR:
		if ((!iswordchar(sc.ch))) {
			sc.SetState(SCE_FM_CF_DEFAULT);
		}
		break;
	case SCE_FM_CF_OPERATOR:
		if (!isoperator(sc.ch)) {
			sc.SetState(SCE_FM_CF_DEFAULT);
		}

	}

	if (sc.state == SCE_FM_CF_DEFAULT) {
		switch (sc.ch) {
		case '"':
			sc.SetState(SCE_FM_CF_STRING);
			break;
		case '[':
			sc.SetState(SCE_FM_CF_COLUMN);
			break;
		case '@':
			sc.SetState(SCE_FM_CF_PARAMETR);
			break;
		default:
		{
			if (isdigit(sc.ch)) {
				sc.SetState(SCE_FM_CF_NUMBER);
			} else if (isoperator(sc.ch)) {
				if (sc.ch == '/' && sc.chNext == '/') {
					sc.SetState(SCE_FM_CF_COMMENT);
					sc.Forward();
					sc.Forward();
				} else {
					sc.SetState(SCE_FM_CF_OPERATOR);
				}
			} else if (iswordchar(sc.ch) && !iswordchar(sc.chPrev)) {
				sc.SetState(SCE_FM_CF_IDENTIFIER);
			}
		}
		}
	}
}

bool SCI_METHOD LexerFormEngine::ColoriseWireFormat(StyleContext &sc) {
	if (sc.state == SCE_FM_WF_OPERATOR) {
		bool isValue = (sc.chPrev == '=');
		for (bool doing2 = sc.More(); doing2 && isspacechar(sc.ch); doing2 = sc.More(), sc.Forward());
		if (!sc.More())
			return false;
		if (sc.Match("]]>"))
			return true;
		if (isValue && sc.ch == '"') {
			sc.SetState(SCE_FM_WF_STRING);
			sc.Forward();
		} else
			sc.SetState(SCE_FM_WF_DEFAULT);
	} else if (sc.state == SCE_FM_WF_STRING) {
		if (sc.ch == '\"') {
			if (sc.chNext == '\"') {
				sc.Forward();
			} else {
				sc.ForwardSetState(SCE_FM_WF_DEFAULT);
			}
		}
	}
	if (isspacechar(sc.ch))
		return false;
	switch (sc.state) {
	case SCE_FM_WF_FIELDNAME:
		switch (sc.ch) {
		case '=':
			sc.SetState(SCE_FM_WF_OPERATOR);
			break;
		}
		break;
	case SCE_FM_WF_MSGNAME:
		switch (sc.ch) {
		case '(':
			sc.SetState(SCE_FM_WF_OPERATOR);
			break;
		}
		break;
	case SCE_FM_WF_DEFAULT:
		switch (sc.ch) {
		case ':':
		{
			char s[16];

			sc.GetCurrentLowered(s, sizeof(s));
			if (sc.chNext == ':' && wTypes_wf.InList(s)) {
				sc.Forward(2);
				sc.ChangeState(SCE_FM_WF_FIELDTYPE);
				sc.SetState(SCE_FM_WF_DEFAULT);
			}
		}
		break;
		case '=':
			sc.ChangeState(SCE_FM_WF_FIELDNAME);
			sc.SetState(SCE_FM_WF_OPERATOR);
			break;
		case '(':
			sc.ChangeState(SCE_FM_WF_MSGNAME);
			sc.SetState(SCE_FM_WF_OPERATOR);
			break;
		case ')':
		case ';':
			sc.SetState(SCE_FM_WF_OPERATOR);
			break;
		case '"':
			sc.SetState(SCE_FM_WF_STRINGIDENTIFIER);
			break;
		}
		break;
	case SCE_FM_WF_STRINGIDENTIFIER:
		if (sc.ch == '\"') {
			if (sc.chNext == '\"') {
				sc.Forward();
			} else {
				sc.ChangeState(SCE_FM_WF_DEFAULT);
			}
		}
		break;

	}
	return false;
}

void SCI_METHOD LexerFormEngine::Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	if(options.frozen) return;
	LexAccessor styler(pAccess);

	int visibleChars = 0;
	int fileNbDigits = 0;

	StyleContext sc(startPos, length, initStyle, styler, 0xFF);
	int lineState = 0;
	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) { //!-change-[LexersLastWordFix]

		if (sc.atLineStart){
			sc.SetState(onStartNextLine(sc.state));
			lineState = 0;
		} else if (sc.atLineEnd) {
			styler.SetLineState(sc.currentLine, lineState);
			lineState = 0;
		}
		if(sc.state == SCE_FM_DEFAULT){
			if(sc.ch == '<') sc.ChangeState(SCE_FM_X_DEFAULT);
			else if(!IsASpace(sc.ch))sc.ChangeState(SCE_FM_VB_DEFAULT);
		}
		if(sc.Match("]]>")){
			lineState |= FM_ENDCDATA;
			sc.SetState(SCE_FM_CDATA);
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
				}else if(sc.chPrev == 'A' && sc.ch == '[') {
					std::string prevLine = styler.GetRangeLowered(styler.LineStart(sc.currentLine), sc.currentPos);
					std::smatch mtch;
					if (std::regex_search(prevLine, mtch, reSyntax)) 						{
						if (mtch[1] == "sql") {
							sc.ForwardSetState(SCE_FM_SQL_DEFAULT);
							ColoriseSQL(sc);
						} else if (mtch[1] == "vbs") {
							sc.ForwardSetState(SCE_FM_VB_DEFAULT);
							ColoriseVBS(sc, visibleChars, fileNbDigits);
						} else if (mtch[1] == "xml") {
							sc.ForwardSetState(SCE_FM_X_DEFAULT);
							ColoriseXML(sc);
						} else if (mtch[1] == "cubeformula") {
							sc.ForwardSetState(SCE_FM_CF_DEFAULT);
							ColoriseCubeFormula(sc);
						} else if (mtch[1] == "wireformat") {
							sc.ForwardSetState(SCE_FM_WF_DEFAULT);
							if (ColoriseWireFormat(sc)) {
								lineState |= FM_ENDCDATA;
								sc.SetState(SCE_FM_CDATA);
								continue;
							}
						} else
							sc.ForwardSetState(SCE_FM_PLAINCDATA);
					} else
						sc.ForwardSetState(SCE_FM_PLAINCDATA);
				}
			}
			break;
		case TYPE_WIREFORMAT:
			if (ColoriseWireFormat(sc)) {
				lineState |= FM_ENDCDATA;
				sc.SetState(SCE_FM_CDATA);
				continue;
			}
			break;
		case TYPE_CUBRFORMULA:
			ColoriseCubeFormula(sc);
			break;
		case TYPE_XML:
			ColoriseXML(sc);
			if (sc.state == SCE_FM_CDATA)
				lineState |= FM_STARTCDATA;
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
	int levelMinCurrent;
	int visibleChars;
	int style;
	bool foldCompact;
	bool foldAtElse;
	int lineFlag;
	bool startLineResolved; //Иногда в начале строки для корректной обработке придется подняться вверх; переменная выставляется в True, когда такая ситуация уже невозможна
	FoldContext(unsigned int startPos, unsigned int length,LexAccessor &styler_,  bool bFoldCompact, bool bFoldAtEsle):
	styler(styler_),
		endPos(startPos + length),
		currentPos(startPos),
		atLineStart(true),
		atLineEnd(false),
		ch(0),
		lineFlag(0),
		foldCompact(bFoldCompact),
		foldAtElse(bFoldAtEsle),
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
			style = styler.StyleAt(currentPos) & 0xFF;
			prevLevel = SC_FOLDLEVELBASE;
			if (currentLine > 0)
				prevLevel = (styler.LevelAt(currentLine - 1) >> 16) & SC_FOLDLEVELNUMBERMASK;
			currentLevel = prevLevel;
			levelMinCurrent = currentLevel;
			foldCompact = bFoldCompact;
			lineFlag = styler.GetLineState(currentLine);
			if (currentLine && styler.GetLineState(currentLine - 1) & FM_STARTCDATA)
				lineFlag |= FM_PREVCDATA;
	}
	bool More() const {
		return currentPos < endPos;
	}
	void DecreaseLevel(int flag = 0){
		currentLevel--;
		if (currentLine && flag) {
			styler.SetLineState(currentLine - 1, styler.GetLineState(currentLine - 1) | flag);
		}
		prevLevel = currentLevel;
		levelMinCurrent = currentLevel;
	}
	void AddFlag(int flag) {
		lineFlag |= flag;
	}
	void Forward(int n);
	void Forward();
	void GetRangeLowered(unsigned int start,unsigned int end,char *s,unsigned int len);
	bool GetNextLowered(char *s,unsigned int len);
	int MatchLowerStyledLine(unsigned int line, IDocument *pAccess);
	bool Skip();
	void WalkToEOL();
	bool FindThen();
	void FoldContext::UpAll() {
		currentLevel++;
		prevLevel++;
		levelMinCurrent++;

	}
	void FoldContext::Up() {
		if (levelMinCurrent > currentLevel)
			levelMinCurrent = currentLevel;
		currentLevel++;
	}
};

void FoldContext::Forward(int n){
	for(int i = 0; i < n; i++, Forward());
}
void FoldContext::Forward(){
	if (atLineEnd) {
		lineFlag |= GetSector(style);
		styler.SetLineState(currentLine, lineFlag);
		lineFlag = lineFlag & FM_STARTCDATA ? FM_PREVCDATA : 0;
	} 
	if (atLineEnd && currentPos < endPos){
		lineFlag |= styler.GetLineState(currentLine + 1);
		int lev = foldAtElse ? levelMinCurrent : prevLevel;
		if ((currentLevel > lev) && (visibleChars > 0)) {
			lev |= SC_FOLDLEVELHEADERFLAG;
		}
		if (visibleChars == 0 && foldCompact) {
			lev |= SC_FOLDLEVELWHITEFLAG;
		}
		lev |= (currentLevel & SC_FOLDLEVELNUMBERMASK) << 16;
		if (lev != styler.LevelAt(currentLine)) {
			styler.SetLevel(currentLine, lev);
		}
		
		visibleChars = 0;
		if (currentPos < endPos) {
			currentLine++;
			//startLineResolved = true;
			if ((currentLevel & SC_FOLDLEVELNUMBERMASK) < SC_FOLDLEVELBASE)
				currentLevel = SC_FOLDLEVELBASE;
			prevLevel = currentLevel;
			levelMinCurrent = currentLevel;
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

		//int flagsNext = styler.LevelAt(currentLine) & ~SC_FOLDLEVELNUMBERMASK;
		//styler.SetLevel(currentLine, (foldAtElse ? levelMinCurrent : prevLevel) | flagsNext | ((currentLevel & SC_FOLDLEVELNUMBERMASK) << 16));
	}
	/////////
	style = styler.StyleAt(currentPos) & 0xFF;
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
int FoldContext::MatchLowerStyledLine(unsigned int line, IDocument *pAccess) {
	int l = currentLine - line;
	if (l < 0)
		return 0;
	int ls = pAccess->LineStart(l) + pAccess->GetLineIndentation(l);
	return styler.StyleAt(ls);
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
	//while(!atLineEnd && currentPos < endPos && (ch != ':' || style != SCE_FM_VB_OPERATOR)){
	while(!atLineEnd && currentPos < endPos){
		bool found2dot = false;
		if (ch == ':' && style == SCE_FM_VB_OPERATOR)
			found2dot = true;
		
		atLineStart = atLineEnd;
		currentPos++;
		if (ch >= 0x100)
			currentPos++;
		ch = chNext;
		GetNextChar(currentPos + ((ch >= 0x100) ? 1 : 0));
		style = styler.StyleAt(currentPos) & 0xFF;
		if (found2dot) {
			visibleChars = 0;
			return;
		}
	}
}
bool FoldContext::FindThen() {
	char s[100];
	bool prevStrCont = false;
	do {
		while (Skip()) {
			if (style == SCE_FM_VB_KEYWORD) {//остановились перед каким-то ключевым словом = прчтем его
				GetNextLowered(s, 100);
			} else if (style != SCE_FM_VB_DEFAULT && style != SCE_FM_VB_COMMENT) {//остановились перед чем-то отличным от пустого пространства- значит прочитанное слово нам не нужно
				s[0] = 0;
			}
		}
		
		if (More()) {
			prevStrCont = (style == SCE_FM_VB_STRINGCONT);
			Forward();
			if (prevStrCont&& (style == SCE_FM_VB_KEYWORD))
				GetNextLowered(s, 100);
		} else {
			prevStrCont = false;
		}
	} while ((style == SCE_FM_VB_STRINGCONT || style == SCE_FM_VB_AFTERSTRINGCONT || prevStrCont) && More());
	return !strcmp(s, "then");
}

bool LexerFormEngine::PlainFold(unsigned int startPos, int length, int initStyle, IDocument *pAccess, LexAccessor &styler) {
	
	while ( initStyle == SCE_FM_VB_STRINGCONT) {
		//Возможно разрешить фолдинг начиная с данной строки не удасться, и тогда нам нужно перезапустить фолдинг со строчки выше
		int prevLine = pAccess->LineFromPosition(startPos) - 1;
		if (prevLine < 0) return true; //Хотя вообще-то процедура должна вернуть true для первой строки
		unsigned int startPosNew = pAccess->LineStart(prevLine);
		length += (startPos - startPosNew);
		startPos = startPosNew;
		initStyle = pAccess->StyleAt(startPos - 1);
	}

	FoldContext fc(startPos, length, styler, options.foldCompact, options.foldAtElse);

	int blockReFoldLine = -1;
	int processComment = 0;
	if (options.foldComment && fc.currentLine) {
		if (fc.MatchLowerStyledLine(1, pAccess) == SCE_FM_VB_COMMENT) {
			processComment++;
			if (fc.currentLine > 1 && fc.MatchLowerStyledLine(2, pAccess) == SCE_FM_VB_COMMENT)
				processComment++;
			int prevLev = styler.LevelAt(fc.currentLine - 1);
			int prevFold = prevLev & SC_FOLDLEVELNUMBERMASK;
			int prevFoldNext = (prevLev >> 16) & SC_FOLDLEVELNUMBERMASK;
			if (fc.MatchLowerStyledLine(0, pAccess) != SCE_FM_VB_COMMENT) 				{
			
				if ((prevFold < prevFoldNext) || (prevFold == prevFoldNext && processComment > 1)) {
					fc.currentLevel--;
					fc.prevLevel--;
					fc.levelMinCurrent--;
					if (processComment > 1) {
						styler.SetLevel(fc.currentLine - 1, prevFold | ((prevFold - 1) << 16));
					} else if (fc.currentLine > 1) {
						styler.SetLevel(fc.currentLine - 1, prevFold | prevFold << 16);
					}
				}
			} else {
				if (processComment == 1) {
					if (prevFoldNext == prevFold + 1) {
						fc.WalkToEOL();
						processComment++;
					}
				} else {
					if (prevFoldNext == prevFold - 1) {
						fc.UpAll();
						styler.SetLevel(fc.currentLine - 1, (prevLev & 0xFFFF) | fc.prevLevel << 16);
					}
				}
			}
		}
	}
	
	for (bool doing = fc.More(); doing; doing = fc.More(), fc.Forward()) {
		int sector = GetSector(fc.style);
		if (fc.atLineStart) 
			fc.AddFlag(sector<<4);

		switch (sector) {
		case TYPE_SPACE:
			break;
		case TYPE_XML:
			switch (fc.style) {
			case SCE_FM_X_DEFAULT:
			case SCE_FM_X_TAG:
				for (bool doing = fc.More(); doing; doing = fc.More() && !fc.atLineEnd) {
					if (fc.style == SCE_FM_X_TAG) {
						if ((fc.ch == '<') && (fc.chNext != '?') && (fc.chNext != '/')) {
							fc.Up();
						} else if (fc.ch == '/') {//Слеш в любом(валидном) месте уменьшает фолдинг
							fc.currentLevel--;
						}
					}
					fc.Forward();
					if (GetSector(fc.style) != TYPE_XML) break;
				}
			}
			break;
		case TYPE_SQL:
			for (bool doing = fc.More(); doing; doing = fc.More() && !fc.atLineEnd) {
				switch (fc.style) {
				case SCE_FM_SQL_OPERATOR:
					if (fc.ch == '(')
						fc.Up();
					else if (fc.ch == ')')
						fc.currentLevel--;
					break;
				case SCE_FM_SQL_STATEMENT:
					if (true) {//Нашли !fc.visibleCharsключквое слово в начале строки
						char s[100];
						if (!fc.visibleChars) {
							char lvl = 0;
							std::string s = styler.GetRange(fc.currentPos, styler.LineEnd(fc.currentLine) + 1);
							keywords[KW_MSSQL_INDENT_CLASS].InClassificator(s.c_str(), lvl);
							if (lvl)
								fc.AddFlag(lvl << 20);
						}
						fc.GetNextLowered(s, 100);
						if (!strcmp(s, "begin") || !strcmp(s, "case"))
							fc.Up();
						else if (!strcmp(s, "end"))
							fc.currentLevel--;
						if (fc.style == SCE_FM_SQL_STATEMENT)
							fc.Forward();
						continue;
					}
					break;
				}
				fc.Forward();
				if (GetSector(fc.style) != TYPE_SQL) break;
			}
			break;
		case TYPE_VBS:
			switch (fc.style) {
			case SCE_FM_VB_DEFAULT:
				if (!fc.visibleChars)continue;
				break;
			case SCE_FM_VB_KEYWORD:
				if (!fc.visibleChars) {//Нашли ключквое слово в начале строки
					char s[100];
					fc.GetNextLowered(s, 100);
					if (!strcmp(s, "if")) { //фолдим только если есть последнее Then
						if (fc.FindThen()) {//нашли then -действительно нужно фолдить
							fc.currentLevel += 2;
							blockReFoldLine = fc.currentLine + 1;//в следующей строке не фолдим IfElse
						}
					} else if (!strcmp(s, "select")) {
						fc.currentLevel += 2;
						blockReFoldLine = fc.currentLine + 1;//в следующей строке не фолдим IfElse
					} else if (wFold.InList(s)) {//начало фолдинга - do function sub for with property while
						fc.Up();
					} else if (!strcmp(s, "case")) {
						fc.DecreaseLevel(FM_NEXTFOLD); 
						fc.Up();
					} else if (wRefold.InList(s) && options.foldAtElse) {//промежуточный фолдинг для ифа и свитча - case else elseif
						//if (!strcmp(s, "elseif") && !fc.FindThen()) {
							//break;
						//} else {
							fc.DecreaseLevel(FM_NEXTFOLD);
							fc.Up();
						//}
					} else if (wUnfold.InList(s)) {//конец фолдинга - end next wend loop
						if (!strcmp(s, "end")){

							if (!fc.Skip())
								break;
							//точно нашли кусок в другом стиле на этой строке
							if (fc.style == SCE_FM_VB_KEYWORD) {
								fc.GetNextLowered(s, 100);
								if (wEndWhat2.InList(s)) {
									fc.currentLevel--;
								} else if (!wEndWhat.InList(s))
									break;//Енд непоняно чего не фолдим
							} else
								break;
							
						}
						fc.currentLevel--;
					}
				}
				break;

			case SCE_FM_VB_COMMENT:
				if (!fc.visibleChars && options.foldComment) {
					fc.visibleChars++;
					if (processComment == 1) {
						styler.SetLevel(fc.currentLine - 1, ((fc.currentLevel) | (((fc.currentLevel + 1) & SC_FOLDLEVELNUMBERMASK) << 16) | SC_FOLDLEVELHEADERFLAG));
						fc.UpAll();
					}

				//проверим, является ли следующая строка комментарием
					int lInd = pAccess->GetLineIndentation(fc.currentLine + 1);
					int ls = lInd + pAccess->LineStart(fc.currentLine + 1);
					bool bNoComment = '\'' != styler.SafeGetCharAt(ls, '\0');
					if (!bNoComment && (options.debugmode && ('#' == styler.SafeGetCharAt(ls + 1, '\0')))) {
						bNoComment = (pAccess->LineEnd(fc.currentLine + 1) == ls + 7 ? styler.Match(ls + 2, "DEBUG") : styler.Match(ls + 2, "DEBUG ") ||
							(!lInd && (styler.Match(ls + 2, "STARTDEBUG") || styler.Match(ls + 2, "ENDDEBUG ")) ));
						if (!bNoComment && styler.Match(ls + 2, "DEBUG-")) {
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
						if(processComment > 0){
							fc.currentLevel--;
						}
						processComment = 0;
					} else {
						processComment++;
					}	
					fc.WalkToEOL();	
					continue;	
				}
			default:
				break;
			}
			processComment = 0;
			fc.WalkToEOL(); //закончили обработку бэйсиковской строки
			break;
		case TYPE_WIREFORMAT:
		{
			if (fc.style == SCE_FM_WF_OPERATOR) {
				if (fc.ch == '(') {
					// Measure the minimum before a '{' to allow
					// folding on "} else {"
					fc.Up();
				} else if (fc.ch == ')') {
					fc.currentLevel--;
				}
			}
		}
			break;
		}
		fc.startLineResolved = true;
	}
	return true;
}

void SCI_METHOD LexerFormEngine::Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	if (!options.fold || options.frozen) return;

	LexAccessor styler(pAccess);
	PlainFold(startPos, length, initStyle, pAccess, styler);


}



LexerModule lmFormEngine(SCLEX_FORMENJINE, LexerFormEngine::LexerFactoryFM, "formenjine", fnWordLists);

