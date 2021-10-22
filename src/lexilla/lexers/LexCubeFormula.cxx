

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
struct OptionsCF {
	bool fold;
	bool foldComment;
	bool foldCompact;
	bool foldAtElse;
	bool foldcdata;
	bool debugmode;
	bool frozen;  //флаг для временного отключения на время загрузки хелплистов
	std::string debugsuffix;
	OptionsCF() {
		fold = false;
		foldComment = false;
		foldCompact = false;
		foldAtElse = false;
		debugmode = false;
		foldcdata = false;
		frozen = false;
		debugsuffix = "";
	}
};

#define KW_KEYWORDS 0
#define KW_FUNCTIONS 1

static const char* const cfWordLists[] = {
	"Functions",
	"Keywords",
	0,
};

struct OptionsSetCF : public OptionSet<OptionsCF> {
	OptionsSetCF() {

		DefineWordListSets(cfWordLists);
	}
};

class LexerCubeFormula : public ILexer5 {
	bool caseSensitive;
	CharacterSet setFoldingWordsBegin;
	WordList keywords[24];	//переданные нам вордлисты
	WordList wRefold; //начала фолдинга
	WordList wFold;   //продолжения ифов и свитчей
	WordList wUnfold; //окончания конструкций
	WordList wDebug; //Используемые в данный момент дебаги с суффиксами
	std::map<std::string, std::string> preprocessorDefinitionsStart;
	OptionsCF options;
	OptionsSetCF osFM;

	const std::regex reCase, reIfElse, reComment;
	constexpr bool IsOperator(int ch) noexcept {
		if (IsAlphaNumeric(ch))
			return false;
		if (ch == '%' || ch == '^' || ch == '&' || ch == '*' ||
			ch == '(' || ch == ')' || ch == '-' || ch == '+' ||
			ch == '=' || ch == '|' || ch == ':' || ch == ';' ||
			ch == '<' || ch == '>' || ch == ',' || ch == '/' ||
			ch == '?' || ch == '!' || ch == '.' || ch == '~')
			return true;
		return false;
	}

public:
	LexerCubeFormula(bool caseSensitive_) :
		setFoldingWordsBegin(CharacterSet::setLower, "idfecnwspl"),
		reCase("^[ \t]*case\\W", std::regex::ECMAScript),
		reIfElse("^[ \t]*else(if)?\\W", std::regex::ECMAScript),
		reComment("^[ \t]*'", std::regex::ECMAScript),
		caseSensitive(caseSensitive_) {
			wRefold.Set("else elseif");
			wFold.Set("do function sub for with private public property class while");
			wUnfold.Set("end next wend loop");
			wDebug.Set("");
	}
	~LexerCubeFormula() {
	}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvRelease4;
	}
	const char* SCI_METHOD PropertyNames() {
		return osFM.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char* name) {
		return osFM.PropertyType(name);
	}
	const char* SCI_METHOD DescribeProperty(const char* name) {
		return osFM.DescribeProperty(name);
	}
	int SCI_METHOD PropertySet(const char* key, const char* val);
	const char* SCI_METHOD DescribeWordListSets() {
		return osFM.DescribeWordListSets();
	}
	int SCI_METHOD WordListSet(int n, const char* wl);
	void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument* pAccess);
	void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess);

	void* SCI_METHOD PrivateCall(int cmd, void* pnt) {
		if (cmd < 32) {
			const char* wl = reinterpret_cast<char*>(pnt);
			WordListSet(cmd, wl);
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

	void SCI_METHOD SetIdentifiers(int, const char*)  noexcept override {}

	int SCI_METHOD DistanceToSecondaryStyles()  noexcept override {
		return 0;
	}

	const char* SCI_METHOD GetSubStyleBases()  noexcept override {
		return { 0 };
	}
	int SCI_METHOD NamedStyles()  noexcept override {
		return 0;
	}

	const char* SCI_METHOD NameOfStyle(int style) {
		return "";
	}

	const char* SCI_METHOD TagsOfStyle(int style) {
		return "";
	}

	const char* SCI_METHOD DescriptionOfStyle(int style) {
		return "";
	}
	// ILexer5 methods
	const char* SCI_METHOD GetName() override {
		return "cubeformula";
	}
	int SCI_METHOD  GetIdentifier() override {
		return SCLEX_CUBEFORMULA;
	}
	const char* SCI_METHOD PropertyGet(const char* key) override;

	static ILexer5* LexerFactoryCubeFormula() {
		return new LexerCubeFormula(true);
	}
	static ILexer5* LexerFactoryCPPInsensitive() {
		return new LexerCubeFormula(false);
	}

};

int SCI_METHOD LexerCubeFormula::PropertySet(const char* key, const char* val) {
	if (osFM.PropertySet(&options, key, val)) {
		if (!strcmp(key, "precompiller.debugsuffix")) {
			wDebug.Set(val);
		}
		return 0;
	}
	return -1;
}

int SCI_METHOD LexerCubeFormula::WordListSet(int n, const char* wl) {
	WordList* wordListN = 0;
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

const char* SCI_METHOD LexerCubeFormula::PropertyGet(const char* key) {
	return osFM.PropertyGet(key);
}

//Functor used to truncate history


void SCI_METHOD LexerCubeFormula::Lex(unsigned int startPos, int length, int initStyle, IDocument* pAccess) {

	LexAccessor styler(pAccess);
	StyleContext sc(startPos, length, initStyle, styler, (char)(STYLE_MAX));
	
	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) {
		if (sc.state == SCE_CF_IDENTIFIER && !iswordchar(sc.ch)) {
			char s[256];

			sc.GetCurrentLowered(s, sizeof(s));
			if (keywords[KW_KEYWORDS].InList(s)) {
				sc.ChangeState(SCE_CF_KEYWORD);
				sc.SetState(SCE_CF_DEFAULT);
			}
			else if (keywords[KW_FUNCTIONS].InList(s)) {
				sc.ChangeState(SCE_CF_FUNCTION);
				sc.SetState(SCE_CF_DEFAULT);
			}
			else {
				sc.ChangeState(SCE_CF_DEFAULT);
			}
		}
		switch (sc.state) {
		case SCE_CF_STRING:
			if (sc.ch == '\"') {
				if (sc.chNext == '\"') {
					sc.Forward();
				}
				else {
					sc.ForwardSetState(SCE_CF_DEFAULT);
				}
			}
			break;
		case SCE_CF_COLUMN:
			if (sc.ch == ']')
				sc.ForwardSetState(SCE_CF_DEFAULT);
			break;
		case SCE_CF_NUMBER:
			if (!(isdigit(sc.ch) || sc.ch == '.')) {
				sc.SetState(SCE_CF_DEFAULT);
			}
			break;
		case SCE_CF_COMMENT:
			if (sc.ch == '\n' || sc.ch == '\r') {
				sc.ForwardSetState(SCE_CF_DEFAULT);
			}
			break;
		case SCE_CF_PARAMETR:
			if ((!iswordchar(sc.ch))) {
				sc.SetState(SCE_CF_DEFAULT);
			}
			break;
		case SCE_CF_OPERATOR:
			if (!IsOperator(sc.ch)) {
				sc.SetState(SCE_CF_DEFAULT);
			}
			
		}

		if (sc.state == SCE_CF_DEFAULT) {
			switch (sc.ch) {
			case '"':
				sc.SetState(SCE_CF_STRING);
				break;
			case '[':
				sc.SetState(SCE_CF_COLUMN);
				break;
			case '@':
				sc.SetState(SCE_CF_PARAMETR);
				break;
			default:
			{
				if (isdigit(sc.ch)) {
					sc.SetState(SCE_CF_NUMBER);
				}
				else if (IsOperator(sc.ch)) {
					if (sc.ch == '/' && sc.chNext == '/') {
						sc.SetState(SCE_CF_COMMENT);
						sc.Forward();
						sc.Forward();
					}
					else {
						sc.SetState(SCE_CF_OPERATOR);
					}
				}
				else if (iswordchar(sc.ch) && !iswordchar(sc.chPrev)) {
					sc.SetState(SCE_CF_IDENTIFIER);
				}
			}
			}
		}
	}
	sc.Complete();
	styler.Flush();
}



void SCI_METHOD LexerCubeFormula::Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess) {
	



}



LexerModule lmCubeFormula(SCLEX_CUBEFORMULA, LexerCubeFormula::LexerFactoryCubeFormula, "cubeformula", cfWordLists);

