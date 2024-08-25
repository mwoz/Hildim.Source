

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


#define KW_TYPES 0

static const char *const wfWordLists[] = {
	"Types",
	0,
};

class LexerWireFormat : public ILexer5 {
	CharacterSet setFoldingWordsBegin;
	WordList wTypes;	//переданные нам вордлисты

	const std::regex reCase, reIfElse, reComment;
	constexpr bool IsOperator(int ch) noexcept {
		if (IsAlphaNumeric(ch))
			return false;
		if (ch == '(' || ch == ')' || ch == '=' || ch == ';')
			return true;
		return false;
	}

public:
	LexerWireFormat(bool ) {
		wTypes.Set("string int float datetime bool null empty binary");
	}
	~LexerWireFormat() {
	}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvRelease4;
	}
	const char * SCI_METHOD PropertyNames() override {
		return NULL;
	}
	int SCI_METHOD PropertyType(const char*) override {
		return -1;
	}
	const char * SCI_METHOD DescribeProperty(const char*) override {
		return NULL;
	}
	Sci_Position SCI_METHOD PropertySet(const char*, const char*);
	const char * SCI_METHOD DescribeWordListSets() override {
		return NULL;
	}
	Sci_Position SCI_METHOD WordListSet(int n, const char *wl);
	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess);
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess);

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
	int SCI_METHOD AllocateSubStyles(int, int) noexcept override {
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

	const char * SCI_METHOD NameOfStyle(int) {
		return "";
	}

	const char * SCI_METHOD TagsOfStyle(int) {
		return "";
	}

	const char * SCI_METHOD DescriptionOfStyle(int) {
		return "";
	}
	// ILexer5 methods
	const char * SCI_METHOD GetName() override {
		return "wireformat";
	}
	int SCI_METHOD  GetIdentifier() override {
		return SCLEX_WIREFORMAT;
	}
	const char * SCI_METHOD PropertyGet(const char*) override {
		return NULL;
	}

	static ILexer5 *LexerFactoryWireFormat() {
		return new LexerWireFormat(true);
	}
	static ILexer5 *LexerFactoryCPPInsensitive() {
		return new LexerWireFormat(false);
	}

};

Sci_Position SCI_METHOD LexerWireFormat::PropertySet(const char*, const char*) {
	return -1;
}

Sci_Position SCI_METHOD LexerWireFormat::WordListSet(int, const char*) {
	return -1;
}
 //Functor used to truncate history


void SCI_METHOD LexerWireFormat::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) {
	LexAccessor styler(pAccess);
	//StyleContext sc(startPos, length, initStyle, styler, (char)(STYLE_MAX));
	StyleContext sc(startPos, length, initStyle, styler, static_cast<char>(STYLE_MAX));

	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) {

		if (sc.state == SCE_WF_OPERATOR) {
			bool isValue = (sc.chPrev == '=');
			for (bool doing2 = sc.More(); doing2 && isspacechar(sc.ch); doing2 = sc.More(), sc.Forward());
			if (!sc.More())
				break;
			if (isValue && sc.ch == '"') {
				sc.SetState(SCE_WF_STRING);
				sc.Forward();
			}
			else
				sc.SetState(SCE_WF_DEFAULT);
		} else if (sc.state == SCE_WF_STRING){
			if (sc.ch == '\"') {
				if (sc.chNext == '\"') {
					sc.Forward();
				} else {
					sc.ForwardSetState(SCE_WF_DEFAULT);
				}
			}
		}
		if (isspacechar(sc.ch))
			continue;
		switch (sc.state) {
		case SCE_WF_FIELDNAME:
			switch (sc.ch) {
			case '=':
				sc.SetState(SCE_WF_OPERATOR);
				break;
			}
			break;
		case SCE_WF_MSGNAME:
			switch (sc.ch) {
			case '(':
				sc.SetState(SCE_WF_OPERATOR);
			break;
			}
			break;
		case SCE_WF_DEFAULT:
			switch (sc.ch) {
			case ':':
			{
				char s[16];

				sc.GetCurrentLowered(s, sizeof(s));
				if (sc.chNext == ':' && wTypes.InList(s)) {
					sc.Forward(2);
					sc.ChangeState(SCE_WF_FIELDTYPE);
					sc.SetState(SCE_WF_DEFAULT);
				}
			}
			break;
			case '=':
				sc.ChangeState(SCE_WF_FIELDNAME);
				sc.SetState(SCE_WF_OPERATOR);
				break;
			case '(':
				sc.ChangeState(SCE_WF_MSGNAME);
				sc.SetState(SCE_WF_OPERATOR);
				break;
			case ')':
			case ';':
				sc.SetState(SCE_WF_OPERATOR);
				break;
			case '"':
				sc.SetState(SCE_WF_STRINGIDENTIFIER);
				break;
			}
			break;
		case SCE_WF_STRINGIDENTIFIER:
			if (sc.ch == '\"') {
				if (sc.chNext == '\"') {
					sc.Forward();
				} else {
					sc.ChangeState(SCE_WF_DEFAULT);
				}
			}
			break;
		//case SCE_WF_STRING:

		//	break;
		}

	}
	sc.Complete();
	styler.Flush();
}



void SCI_METHOD LexerWireFormat::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument *pAccess) {
	LexAccessor styler(pAccess);
	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	int visibleChars = 0;
	Sci_PositionU lineStartNext = styler.LineStart(lineCurrent + 1);
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent - 1) >> 16;
	int levelNext = levelCurrent;
	const Sci_PositionU endPos = startPos + length;
	for (Sci_PositionU i = startPos; i < endPos; i++) {
		const bool atEOL = i == (lineStartNext - 1);
		if (styler.StyleAt(i) == SCE_WF_OPERATOR) {
			if (styler[i] == '(') {
				// Measure the minimum before a '{' to allow
				// folding on "} else {"
				levelNext++;
			} else if (styler[i] == ')') {
				levelNext--;
			}
		}
		if (!IsASpace(styler[i]))
			visibleChars++;
		if (atEOL || (i == endPos - 1)) {
			int levelUse = levelCurrent;

			int lev = levelUse | levelNext << 16;
			if (visibleChars == 0 )
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelUse < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			lineStartNext = styler.LineStart(lineCurrent + 1);
			levelCurrent = levelNext;

			if (atEOL && (i == static_cast<Sci_PositionU>(styler.Length() - 1))) {
				// There is an empty line at end of file so give it same level and empty
				styler.SetLevel(lineCurrent, (levelCurrent | levelCurrent << 16) | SC_FOLDLEVELWHITEFLAG);
			}
			visibleChars = 0;
		}
	}

}

extern const LexerModule lmWireFormat(SCLEX_WIREFORMAT, LexerWireFormat::LexerFactoryWireFormat, "wireformat", wfWordLists);

