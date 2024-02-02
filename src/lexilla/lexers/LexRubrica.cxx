

// Scintilla source code edit control
/** @file LexCPP.cxx
 ** Lexer for C++, C, Java, && JavaScript.
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

static const char *const rbrWordLists[] = {
	"Types",
	0,
};

class LexerRubrica : public ILexer5 {
	CharacterSet setFoldingWordsBegin;
	WordList wTypes;	//переданные нам вордлисты

	const std::regex reLnkBody, rePicBody, reCellEx, reCellEx2, reCellLnk;
	constexpr bool IsOperator(int ch) noexcept {
		if (IsAlphaNumeric(ch))
			return false;
		if (ch == '(' || ch == ')' || ch == '=' || ch == ';')
			return true;
		return false;
	}

private:
	std::string LineEnd(LexAccessor &styler, StyleContext &sc);
	void Anchor(LexAccessor &styler, StyleContext &sc);
public:
	LexerRubrica(bool caseSensitive_) :
		rePicBody("^((?:(?!\\}\\}).)+)\\}\\}", std::regex::ECMAScript),
		reLnkBody("^((?:(?!\\]\\]).)*)\\]\\]", std::regex::ECMAScript),
		reCellEx2("^((?:(?!\\}\\})[^|{])*)\\}\\}", std::regex::ECMAScript),
		reCellLnk("^((?:(?!\\]\\])[^|{])*)\\|", std::regex::ECMAScript),
		reCellEx("^((?:(?!\\}\\})[^|{])*)\\|", std::regex::ECMAScript)
	{
		wTypes.Set("string int float datetime bool null empty binary");
	}
	~LexerRubrica() {
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
	int SCI_METHOD PropertyType(const char *name) override {
		return -1;
	}
	const char * SCI_METHOD DescribeProperty(const char *name) override {
		return NULL;
	}
	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val);
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
		return "rubrica";
	}
	int SCI_METHOD  GetIdentifier() override {
		return SCLEX_RUBRICA;
	}
	const char * SCI_METHOD PropertyGet(const char *key) override {
		return NULL;
	}

	static ILexer5 *LexerFactoryRubrica() {
		return new LexerRubrica(true);
	}
	static ILexer5 *LexerFactoryCPPInsensitive() {
		return new LexerRubrica(false);
	}

};

Sci_Position SCI_METHOD LexerRubrica::PropertySet(const char *key, const char *val) {
	return -1;
}

Sci_Position SCI_METHOD LexerRubrica::WordListSet(int n, const char *wl) {
	return -1;
}
 //Functor used to truncate history

std::string LexerRubrica::LineEnd(LexAccessor &styler, StyleContext &sc) {
	Sci_Position s = sc.currentPos;
	Sci_Position e = styler.LineEnd(sc.currentLine) + 1;
	if (s >= e)
		return "";
	return styler.GetRange(s , e);
}

void LexerRubrica::Anchor(LexAccessor &styler, StyleContext &sc) {
	sc.SetState(SCE_RBR_OPERATOR);
	sc.Forward(2);
	sc.SetState(SCE_RBR_CELL);
	sc.Forward(6);
	sc.SetState(SCE_RBR_OPERATOR);
	sc.Forward();

	std::string l = LineEnd(styler, sc);
	std::smatch mtch;  
	if (l.length() && std::regex_search(l, mtch, reCellEx2) && (mtch.length() > 0) ) {

		sc.SetState(SCE_RBR_CELL);
		sc.Forward(mtch[1].length());
		sc.SetState(SCE_RBR_OPERATOR);
		sc.Forward(2);
	}
}

void SCI_METHOD LexerRubrica::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) {

	LexAccessor styler(pAccess);
	StyleContext sc(startPos, length, initStyle, styler, (char)(STYLE_MAX));

	//for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) {
	while (sc.More()) {
		int curState = sc.state;

		if (curState == SCE_RBR_DEFAULT || curState == SCE_RBR_BOLD || curState == SCE_RBR_ITALICS || curState == SCE_RBR_UNDERLINED || curState == SCE_RBR_OPERATOR || curState == SCE_RBR_KEYWORD) {
			if (sc.Match("{{color|")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				sc.SetState(SCE_RBR_CELL);
				sc.Forward(5);
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward();

				std::string l = LineEnd(styler, sc);
				//local l = editor : textrange(sc.Position(), editor : PositionFromLine(sc.Line(sc.Position()) + 1) - 2)

				std::smatch mtch;

				if (l.length() && std::regex_search(l, mtch, reCellEx) && mtch.length() > 0) {

					sc.SetState(SCE_RBR_CELL);
					sc.Forward(mtch[1].length());
					sc.SetState(SCE_RBR_OPERATOR);
					sc.Forward();
					sc.SetState(SCE_RBR_DEFAULT);
				}

				l = LineEnd(styler, sc); 
				if (l.length() && std::regex_search(l, mtch, reCellEx) && mtch.length() > 0) {

					sc.SetState(SCE_RBR_CELL);
					sc.Forward(mtch[1].length());
					sc.SetState(SCE_RBR_OPERATOR);
					sc.Forward();
					sc.SetState(SCE_RBR_DEFAULT);
				}
				sc.SetState(SCE_RBR_DEFAULT);
				continue;
			} else if (sc.Match("{{anchor|")) {
				Anchor(styler, sc);
			} else if (sc.Match("{{break}}")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(9);
				continue;
			} else if (sc.Match("{{")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				std::string l = LineEnd(styler, sc);

				std::smatch mtch;
				if (l.length() && std::regex_search(l, mtch, rePicBody) && mtch.length() > 0) {

					sc.SetState(SCE_RBR_PICTIRE);
					sc.Forward(mtch[1].length());
					sc.SetState(SCE_RBR_OPERATOR);
					sc.Forward(2);
				}
				sc.SetState(SCE_RBR_DEFAULT);
			} else if (sc.Match("[[")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				int sLast = SCE_RBR_LNKCODE;

				std::string l = LineEnd(styler, sc);
				std::smatch mtch;
				if (l.length() && std::regex_search(l, mtch, reCellLnk) && mtch.length() > 0) {

					sc.SetState(SCE_RBR_LNKCODE);
					sc.Forward(mtch[1].length());
					sc.SetState(SCE_RBR_OPERATOR);
					sc.Forward();
					sLast = SCE_RBR_LNKBODY;
					l = mtch.suffix();
				}
				if (l.length() && std::regex_search(l, mtch, reLnkBody) && mtch.length() > 0) {
					sc.SetState(sLast);
					sc.Forward(mtch[1].length());
					sc.SetState(SCE_RBR_OPERATOR);
					sc.Forward(2);
				}
			} else if (sc.Match("<math>")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(6);
				sc.SetState(SCE_RBR_MATH);
			} else if (sc.Match("}}")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				sc.SetState(SCE_RBR_DEFAULT);
				continue;
			} else if (sc.Match("//")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				if (curState == SCE_RBR_ITALICS) {
					//sc.SetState(SCE_RBR_DEFAULT);
				} else {
					sc.SetState(SCE_RBR_ITALICS);
				}
				continue;
			} else if (sc.Match("**")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				if (curState == SCE_RBR_BOLD) {
					//sc.SetState(SCE_RBR_DEFAULT);
				} else {
					sc.SetState(SCE_RBR_BOLD);
				}
				continue;
			} else if (sc.Match("__")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				if (curState == SCE_RBR_UNDERLINED) {
					//sc.SetState(SCE_RBR_DEFAULT);
				} else {
					sc.SetState(SCE_RBR_UNDERLINED);
				}
				continue;
			} else if (sc.Match("<sub>") || sc.Match("<sup>")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(4);
				//sc.SetState(SCE_RBR_DEFAULT);
				continue;
			} else if (sc.Match("</sub>") || sc.Match("</sup>")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(5);
				//sc.SetState(SCE_RBR_DEFAULT);
				continue;
			} else if (sc.Match("<nowiki>")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(8);
				if (curState == SCE_RBR_BOLD) {
					sc.SetState(SCE_RBR_BOLD_NOWIKI);
				} else if (curState == SCE_RBR_ITALICS) {
					sc.SetState(SCE_RBR_ITALICS_NOWIKI);
				} else if (curState == SCE_RBR_UNDERLINED) {
					sc.SetState(SCE_RBR_UNDERLINED_NOWIKI);
				} else {
					sc.SetState(SCE_RBR_NOWIKI);
				}
				continue;
			} else if (curState == SCE_RBR_OPERATOR) {
				sc.SetState(SCE_RBR_DEFAULT);
			}
		} else if (curState == SCE_RBR_PRE && sc.Match("</pre>")) {
			sc.SetState(SCE_RBR_KEYWORD);
			sc.Forward(6);
			//sc.SetState(SCE_RBR_DEFAULT);
			continue;
		} else if (curState == SCE_RBR_PRE_TAG && sc.Match(">")) {
			sc.Forward();
			sc.SetState(SCE_RBR_PRE);
			continue;
		} else if (curState == SCE_RBR_NOWIKI && sc.Match("</nowiki>")) {
			sc.SetState(SCE_RBR_KEYWORD);
			sc.Forward(9);
			//sc.SetState(SCE_RBR_DEFAULT);
			continue;
		} else if (curState == SCE_RBR_BOLD_NOWIKI && sc.Match("</nowiki>")) {
			sc.SetState(SCE_RBR_KEYWORD);
			sc.Forward(9);
			sc.SetState(SCE_RBR_BOLD);
			continue;
		} else if (curState == SCE_RBR_ITALICS_NOWIKI && sc.Match("</nowiki>")) {
			sc.SetState(SCE_RBR_KEYWORD);
			sc.Forward(9);
			sc.SetState(SCE_RBR_ITALICS);
			continue;
		} else if (curState == SCE_RBR_UNDERLINED_NOWIKI && sc.Match("</nowiki>")) {
			sc.SetState(SCE_RBR_KEYWORD);
			sc.Forward(9);
			sc.SetState(SCE_RBR_UNDERLINED);
			continue;
		} else if (curState == SCE_RBR_MATH) {
			if (sc.Match("</math>")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(7);
				continue;
			}
			goto forward;
		}
		
		if (sc.atLineStart) {
			if (curState == SCE_RBR_OPERATOR_END || curState == SCE_RBR_KEYWORD) {
				sc.SetState(SCE_RBR_DEFAULT);
			}
			if (sc.Match("{|") || sc.Match("|-")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				sc.SetState(SCE_RBR_OPERATOR_END);
			} else if (sc.Match("|}") || sc.Match("||") || sc.Match("!|") || sc.Match("#*") || sc.Match("*#") || sc.Match("##")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(1);
			} else if (sc.Match("!") || sc.Match("|")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward();
				sc.SetState(SCE_RBR_DEFAULT);

				std::string l = LineEnd(styler, sc);
				std::smatch mtch;

				if (l.length() && std::regex_search(l, mtch, reCellEx) && mtch.length() > 0) {

					sc.SetState(SCE_RBR_CELL);
					sc.Forward(mtch[1].length());
					sc.SetState(SCE_RBR_OPERATOR);
					sc.Forward();
					sc.SetState(SCE_RBR_DEFAULT);
				}
			} else if (sc.Match("*") || sc.Match("#")) {
				sc.SetState(SCE_RBR_OPERATOR);

			} else if (sc.Match("====")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(4);
				if (sc.Match("{{anchor|"))
					Anchor(styler, sc);

				sc.SetState(SCE_RBR_HEADER4);
			} else if (sc.Match("===")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(3);
				if (sc.Match("{{anchor|"))
					Anchor(styler, sc);
				sc.SetState(SCE_RBR_HEADER3);
			} else if (sc.Match("==")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				if (sc.Match("{{anchor|"))
					Anchor(styler, sc);
				sc.SetState(SCE_RBR_HEADER2);
			} else if (sc.Match("=")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward();
				if (sc.Match("{{anchor|"))
					Anchor(styler, sc);
				sc.SetState(SCE_RBR_HEADER1);
			} else if (sc.Match("<pre>")) {
				sc.SetState(SCE_RBR_KEYWORD);
				sc.Forward(5);
				sc.SetState(SCE_RBR_PRE);
			} else if (sc.Match("<pre ")) {
				sc.SetState(SCE_RBR_PRE_TAG);
				sc.Forward(4);
			}
		} else if (sc.atLineEnd) {
			//sc.SetState(SCE_RBR_DEFAULT);
		} else if (curState >= SCE_RBR_HEADER1 && curState <= SCE_RBR_HEADER4) {

			if (curState == SCE_RBR_HEADER4 && sc.Match("====")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(4);
				sc.SetState(SCE_RBR_DEFAULT);
			} else if (curState == SCE_RBR_HEADER3 && sc.Match("===")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(3);
				sc.SetState(SCE_RBR_DEFAULT);
			} else if (curState == SCE_RBR_HEADER2 && sc.Match("==")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward(2);
				sc.SetState(SCE_RBR_DEFAULT);
			} else if (curState == SCE_RBR_HEADER1 && sc.Match("=")) {
				sc.SetState(SCE_RBR_OPERATOR);
				sc.Forward();
				sc.SetState(SCE_RBR_DEFAULT);
			} else if (sc.Match("{{anchor|")) {
				int s = curState;
				Anchor(styler, sc);
				sc.SetState(s);
			}
		} else if(sc.state == SCE_RBR_KEYWORD) {
			sc.SetState(SCE_RBR_DEFAULT);
		}
forward:		
		sc.Forward();
	}

	sc.Complete();
	styler.Flush();
}



void SCI_METHOD LexerRubrica::Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) {

}

LexerModule lmRubrica(SCLEX_RUBRICA, LexerRubrica::LexerFactoryRubrica, "rubrica", rbrWordLists);

