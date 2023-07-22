//-*- coding: utf-8 -*-
// Scintilla source code edit control
/** @file LexSQL.cxx
 ** Lexer for SQL, including PL/SQL and SQL*Plus.
 ** Improved by Jérôme LAFORGE <jerome.laforge_AT_gmail_DOT_com> from 2010 to 2012.
 **/
// Copyright 1998-2012 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <regex>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "SparseState.h"
#include "DefaultLexer.h"

using namespace Scintilla;
using namespace Lexilla;

static inline bool IsAWordChar(int ch) {
	return (ch < 0x80) && (isalnum(ch) || ch == '_' || ch == '$');

}

static inline bool IsAWordStart(int ch) {
	return (ch < 0x80) && (isalpha(ch) || ch == '_');
}


static inline bool IsANumberChar(int ch, int chPrev) {
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases.
	return (ch < 0x80) &&
	       (isdigit(ch) || toupper(ch) == 'E' ||
	        ch == '.' || ((ch == '-' || ch == '+') && chPrev < 0x80 && toupper(chPrev) == 'E'));
}

// Options used for LexerPGSQL
struct OptionsPGSQL {
	bool fold;
	bool foldAtElse;
	bool foldComment;
	bool foldCompact;
	bool foldOnlyBegin;
	std::string tag$ignore;
	OptionsPGSQL() {
		fold = false;
		foldAtElse = true;
		foldComment = false;
		foldCompact = false;
		foldOnlyBegin = false;
		tag$ignore = "$function$ $procedure$ $sql$";
	}
};

#define KW_PGSQL_STATEMENTS         0
#define KW_PGSQL_FUNCTIONS          1
#define KW_PGSQL_DATA_TYPES         2
#define KW_PGSQL_USER1              3
#define KW_PGSQL_USER2              4
#define KW_PGSQL_USER3              5
#define KW_PGSQL_RADIUS             6
#define KW_PGSQL_M4KEYS             7
#define KW_PGSQL_INDENT_CLASS       8
#define KW_PGSQL_NKEYWORDS          9

static const char * const pgsqlWordListDesc[] = {
	"Statements",
	"Functions",
	"Data Types",
	"User 1",
	"User 2",
	"User 3",
	"Radius constants",
	"M4 preprocessor",
	"Indent classes",
	0
};

struct OptionSetPGSQL : public OptionSet<OptionsPGSQL> {
	OptionSetPGSQL() {
		DefineProperty("fold", &OptionsPGSQL::fold);

		DefineProperty("fold.sql.at.else", &OptionsPGSQL::foldAtElse,
		               "This option enables SQL folding on a \"ELSE\" and \"ELSIF\" line of an IF statement.");

		DefineProperty("fold.comment", &OptionsPGSQL::foldComment);

		DefineProperty("fold.compact", &OptionsPGSQL::foldCompact);

		DefineProperty("fold.sql.only.begin", &OptionsPGSQL::foldOnlyBegin);

		DefineProperty("lexer.pgsql.transparent.tags", &OptionsPGSQL::tag$ignore);


		DefineWordListSets(pgsqlWordListDesc);
	}
};

class LexerPGSQL : public DefaultLexer {
public :
	LexerPGSQL() : DefaultLexer("pgsql", SCLEX_PGSQL) ,
		reRegisterMacto("^\\s*__REGISTER_(REPORT)|(PLUGIN)|(WIZARD)|(FORM)_", std::regex::ECMAScript)
	{
		transparentTags.Set(options.tag$ignore.c_str());
	}

	virtual ~LexerPGSQL() {}

	int SCI_METHOD Version () const override {
		return lvRelease5;
	}

	void SCI_METHOD Release() override {
		delete this;
	}

	const char * SCI_METHOD PropertyNames() override {
		return osSQL.PropertyNames();
	}

	int SCI_METHOD PropertyType(const char *name) override {
		return osSQL.PropertyType(name);
	}

	const char * SCI_METHOD DescribeProperty(const char *name) override {
		return osSQL.DescribeProperty(name);
	}

	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override {
		if (osSQL.PropertySet(&options, key, val)) {
			if (!strcmp(key, "lexer.pgsql.transparent.tags") && options.tag$ignore.length() > 0) {
				transparentTags.Set(options.tag$ignore.c_str());
			}
			return 0;
		}
		return -1;
	}

	const char * SCI_METHOD PropertyGet(const char *key) override {
		return osSQL.PropertyGet(key);
	}

	const char * SCI_METHOD DescribeWordListSets() override {
		return osSQL.DescribeWordListSets();
	}

	Sci_Position SCI_METHOD WordListSet(int n, const char *wl) override;
	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;

	void * SCI_METHOD PrivateCall(int, void *) override {
		return 0;
	}

	static ILexer5 *LexerFactoryPGSQL() {
		return new LexerPGSQL();
	}
private:
	bool IsStreamCommentStyle(int style) {
		return style == SCE_PGSQL_COMMENT;
	}
	WordList transparentTags;

	bool IsCommentStyle (int style) {
		switch (style) {
		case SCE_PGSQL_COMMENT :
		case SCE_PGSQL_LINE_COMMENT :
			return true;
		default :
			return false;
		}
	}

	bool IsCommentLine (Sci_Position line, LexAccessor &styler) {
		Sci_Position pos = styler.LineStart(line);
		Sci_Position eol_pos = styler.LineStart(line + 1) - 1;
		for (Sci_Position i = pos; i + 1 < eol_pos; i++) {
			int style = styler.StyleAt(i);
			// MySQL needs -- comments to be followed by space or control char
			if (style == SCE_PGSQL_LINE_COMMENT && styler.Match(i, "--"))
				return true;
			else if (!IsASpaceOrTab(styler[i]))
				return false;
		}
		return false;
	}

	OptionsPGSQL options;
	OptionSetPGSQL osSQL;

	WordList keywords[KW_PGSQL_NKEYWORDS];
	const std::regex reRegisterMacto;

	std::string $tag = "";
	Sci_PositionU posTagStart = SIZE_MAX;
	int commentLevel = 0;
	Sci_PositionU posCommentStart = SIZE_MAX;

};

Sci_Position SCI_METHOD LexerPGSQL::WordListSet(int n, const char* wl) {
	int firstModification = -1;
	if (n < KW_PGSQL_NKEYWORDS) {
		WordList* wordListN = 0;
		wordListN = &keywords[n];
		if (wordListN) {
			WordList wlNew;
			wlNew.Set(wl);
			if (*wordListN != wlNew) {
				wordListN->Set(wl);
				firstModification = 0;
			}
		}
	}
	return firstModification;
}

inline bool HasNotLwr(const char* s) {
	for (int i = 0; s[i]; i++) {
		if (s[i] >= 'a' && s[i] <= 'z')
			return false;
	}
	return true;
}
inline bool IsStringTag(const char* s) {
	int l = strlen(s);
	if (s[l - 1] != '$')
		return false;

	for (int i = 0; i < l - 1; i++) {
		if (s[i] == '$')
			return false;
	}
	return true;
}

void SCI_METHOD LexerPGSQL::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) {
	LexAccessor styler(pAccess);
	StyleContext sc(startPos, length, initStyle, styler);
	int styleBeforeDCKeyword = SCE_PGSQL_DEFAULT;

	if (startPos) {
		switch (initStyle) {
		case SCE_PGSQL_$STRING:
			if (posTagStart > startPos) {
		        //Найдем тег, с которого начиналась текущая строка
				for (Sci_PositionU i = startPos - 1; i; i--) {
					if (styler.StyleAt(i) == SCE_PGSQL_$TAG) {
						Sci_PositionU j;
						for (j = i; j && styler.StyleAt(j) == SCE_PGSQL_$TAG; j--);
						if (styler.StyleAt(j) != SCE_PGSQL_$TAG)
							j++;
						$tag = styler.GetRange(j, i + 1);
						break;
					}
				}
			}
			break;
		case SCE_PGSQL_COMMENT:
			if (posCommentStart > startPos) {
				commentLevel = 0;
				Sci_PositionU i;
				for (i = startPos - 1; i && styler.StyleAt(i) == SCE_PGSQL_COMMENT; i--);
				for(Sci_PositionU j = i + 1; j <= startPos; j++)				
				{
					if (styler.Match(j, "/*")) {
						commentLevel++;
						j++;
					}
					else if (styler.Match(j, "*/")) {
						commentLevel--;
						j++;
					}
				}
			}
		}
	}

	for (; sc.More(); sc.Forward()) {
		// Determine if the current state should terminate.
		switch (sc.state) {
		case SCE_PGSQL_$TAG:
			if (sc.ch == '$') {
				int nextState = SCE_PGSQL_$STRING;
				char s[1000];
				sc.GetCurrent(s, sizeof(s));
				$tag = s;
				$tag += "$";
				posTagStart = sc.currentPos;

				if (transparentTags.InList($tag)) {
					$tag = "";
					nextState = SCE_PGSQL_DEFAULT;
				}

				sc.ForwardSetState(nextState);
			} else if (!IsAWordChar(sc.ch)) {
				sc.SetState(SCE_PGSQL_DEFAULT);
			}
			break;
		case SCE_PGSQL_$STRING:
			if (sc.ch == '$' ) {
				if (sc.Match($tag.c_str())) {
					sc.SetState(SCE_PGSQL_$TAG);
					sc.Forward($tag.length() );
					sc.SetState(SCE_PGSQL_DEFAULT);
					$tag = "";
					posTagStart = SIZE_MAX;
				}
			}
			break;
		}
		
		switch (sc.state) {
		case SCE_PGSQL_OPERATOR:
			sc.SetState(SCE_PGSQL_DEFAULT);
			break;
		case SCE_PGSQL_NUMBER:
			// We stop the number definition on non-numerical non-dot non-eE non-sign char
			if (!IsANumberChar(sc.ch, sc.chPrev)) {
				sc.SetState(SCE_PGSQL_DEFAULT);
			}
			break;
		case SCE_PGSQL_IDENTIFIER:
			if (!IsAWordChar(sc.ch)) {
				int nextState = SCE_PGSQL_DEFAULT;
				char s[1000];
				sc.GetCurrent(s, sizeof(s));
				int lenW = strlen(s);

				if (s[0] == '_') {
					if (keywords[KW_PGSQL_RADIUS].InList(s)) {
						sc.ChangeState(SCE_PGSQL_RADIUSKEYWORDS);
					}
					else if (s[1] == '_' && HasNotLwr(s)) {
						sc.ChangeState(SCE_PGSQL_SYSMCONSTANTS);
					}
					else {
						sc.ChangeState(SCE_PGSQL_VARIABLE);
					}
				}
				else {
					_strlwr(s);
					//sc.GetCurrentLowered(s, sizeof(s));

					if (keywords[KW_PGSQL_FUNCTIONS].InList(s)) {
						sc.ChangeState(SCE_PGSQL_FUNCTION);
					}
					else if (keywords[KW_PGSQL_DATA_TYPES].InList(s)) {
						sc.ChangeState(SCE_PGSQL_DATATYPE);
					}
					else if (keywords[KW_PGSQL_USER1].InList(s)) {
						sc.ChangeState(SCE_PGSQL_USER1);
					}
					else if (keywords[KW_PGSQL_USER2].InList(s)) {
						sc.ChangeState(SCE_PGSQL_USER2);
					}
					else if (keywords[KW_PGSQL_USER3].InList(s)) {
						sc.ChangeState(SCE_PGSQL_USER3);
					}
					else if (keywords[KW_PGSQL_STATEMENTS].InList(s)) {
						sc.ChangeState(SCE_PGSQL_STATEMENT);
					}
					else if (keywords[KW_PGSQL_M4KEYS].InList(s)) {
						sc.ChangeState(SCE_PGSQL_M4KEYS);
					}
				}
				sc.SetState(nextState);
			}
			break;
		case SCE_PGSQL_COMMENT:
			if (sc.Match('*', '/')) {
				sc.Forward();
				commentLevel--;
				if (commentLevel <= 0) {
					sc.ForwardSetState(SCE_PGSQL_DEFAULT);
					posCommentStart = SIZE_MAX;
				}
			}
			else if (sc.Match('/', '*')) {
				commentLevel++;
				sc.Forward();
			}
			break;
		case SCE_PGSQL_LINE_COMMENT:
			if (sc.atLineStart) {
				sc.SetState(SCE_PGSQL_DEFAULT);
			}
			break;
		case SCE_PGSQL_1QSTRING:
			if (sc.ch == '\\') {
				sc.Forward();
			} else if (sc.ch == '\'') {
				if (sc.chNext == '\'') {
					sc.Forward();
				} else {
					sc.ForwardSetState(SCE_PGSQL_DEFAULT);
				}
			}
			break;
		case SCE_PGSQL_2QSTRING:
			if (sc.ch == '\\') {
				// Escape sequence
				sc.Forward();
			} else if (sc.ch == '\"') {
				if (sc.chNext == '\"') {
					sc.Forward();
				} else {
					sc.ForwardSetState(SCE_PGSQL_DEFAULT);
				}
			}
			break;
		case SCE_PGSQL_BYTESTRING:
			if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_PGSQL_DEFAULT);
			}
			else if (sc.ch != '0' && sc.ch != '1') {
				sc.ChangeState(SCE_PGSQL_DEFAULT);
			}
			break;		
		case SCE_PGSQL_HEXSTRING:
			if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_PGSQL_DEFAULT);
			}
			else if (!IsAHeXDigit(sc.ch)) {
				sc.ChangeState(SCE_PGSQL_DEFAULT);
			}
			break;
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_PGSQL_DEFAULT) {
			if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext)) ||
			          ((sc.ch == '-' || sc.ch == '+') && IsADigit(sc.chNext) && !IsADigit(sc.chPrev))) {
				sc.SetState(SCE_PGSQL_NUMBER);
			} 
			else if (sc.MatchIgnoreCase("b'")) {
				sc.SetState(SCE_PGSQL_BYTESTRING);
				sc.Forward();
			}
			else if (sc.MatchIgnoreCase("x'")) {
				sc.SetState(SCE_PGSQL_HEXSTRING);
				sc.Forward();
			}
			else if (IsAWordStart(sc.ch)) {
				sc.SetState(SCE_PGSQL_IDENTIFIER);
			} else if (sc.Match('/', '*')) {
				sc.SetState(SCE_PGSQL_COMMENT);
				posCommentStart = sc.currentPos;
				commentLevel = 1;
				sc.Forward();	// Eat the * so it isn't used for the end of the comment
			} else if (sc.Match('-', '-')) {
				// MySQL requires a space or control char after --
				// http://dev.mysql.com/doc/mysql/en/ansi-diff-comments.html
				// Perhaps we should enforce that with proper property:
				//~ 			} else if (sc.Match("-- ")) {
				sc.SetState(SCE_PGSQL_LINE_COMMENT);
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_PGSQL_1QSTRING);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_PGSQL_2QSTRING);
			} else if (sc.ch == '$' && IsAWordStart(sc.chNext)) {
				sc.SetState(SCE_PGSQL_$TAG);
			} else if (isoperator(static_cast<char>(sc.ch))) {
				sc.SetState(SCE_PGSQL_OPERATOR);
			}
			else if ((sc.ch == '{' || sc.ch == '}') && keywords[KW_PGSQL_M4KEYS].InList("}")) {
				sc.SetState(SCE_PGSQL_M4KBRASHES);
			}
		}
	}
	sc.Complete();
}


class FoldContextPGSQL {
	Sci_PositionU endPos;
	LexAccessor& styler;
	void Init() {
		chPrev = currentPos ? styler.SafeGetCharAt(currentPos - 1) : '\n';
		ch = styler.SafeGetCharAt(currentPos);
		chNext = styler.SafeGetCharAt(currentPos + 1);
		style = styler.StyleAt(currentPos);
		atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		lineCurrent = styler.GetLine(currentPos);
	}
public:
	Sci_PositionU currentPos;
	Sci_Position lineCurrent;
	char chPrev;
	char ch;
	char chNext;
	int style;
	bool atEOL;
	FoldContextPGSQL(Sci_PositionU startPos, Sci_Position length, LexAccessor& styler_) :
		styler(styler_),
		endPos(startPos + length),
		currentPos(startPos),
		atEOL(false),
		ch(0) {
		Init();
	}
	bool More() const {
		return currentPos < endPos;
	}
	void Forward() {
		if (atEOL)
			lineCurrent++;
		currentPos++;
		chPrev = ch;
		ch = chNext;
		chNext = styler.SafeGetCharAt(currentPos + 1);
		style = styler.StyleAt(currentPos);
		atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
	}
	void SkipSameStyle() {
		Sci_PositionU j = currentPos;
		for (; (style == styler.StyleAt(currentPos)) && (currentPos < endPos) && (styler[currentPos] != '\r)' && (styler[currentPos] != '\n')); currentPos++);
		currentPos--;

		Init();
	}	
	std::string GetCurLowered() {
		Sci_PositionU prevPos = currentPos;
		SkipSameStyle();
		if (prevPos <= currentPos + 1)
			return "";
		return styler.GetRangeLowered(prevPos, currentPos + 1);
	}

	std::string GetNextLowered(int scyppedStyle) {
		if (atEOL || (styler.StyleAt(currentPos + 1) != scyppedStyle))
			return "";
		int prevStyle = style;
		Forward();
		if(atEOL)
			return "";
		SkipSameStyle();
		if (atEOL || (styler.StyleAt(currentPos + 1) != prevStyle))
			return "";
		Forward();
		return GetCurLowered();
	}

};

void SCI_METHOD LexerPGSQL::Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) {
	if (!options.fold)
		return;
	LexAccessor styler(pAccess);
	FoldContextPGSQL fc(startPos, length, styler);

	bool foldComment = options.foldComment;
	bool foldCompact = options.foldCompact;
	Sci_PositionU endPos = startPos + length;
	int visibleChars = 0;

	bool caseSwitcher = false;

	int levelPrev = SC_FOLDLEVELBASE;
	if (fc.lineCurrent > 0)
		levelPrev = (styler.LevelAt(fc.lineCurrent - 1) >> 16) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	int levelMinCurrent = levelPrev;
	//char chNext = styler[startPos];
	///char ch, chPrev;
	//ch = '\n';
	bool inComment = (styler.StyleAt(startPos - 1) == SCE_PGSQL_COMMENT);
	char s[15];
	int lineState = 0;
	if (fc.lineCurrent > 0)
		lineState = styler.GetLineState(fc.lineCurrent - 1) & 0x1000000;
	for (; fc.More(); fc.Forward()) {

		// Comment folding
		if (foldComment) {
			if (!inComment && (fc.style == SCE_PGSQL_COMMENT)) {
				if (levelMinCurrent > levelCurrent)
					levelMinCurrent = levelCurrent;
				levelCurrent++;
			}
			else if (inComment && (fc.style != SCE_PGSQL_COMMENT))
				levelCurrent--;
			inComment = (fc.style == SCE_PGSQL_COMMENT);
		}
		if (fc.style == SCE_PGSQL_M4KBRASHES) {
			if (fc.ch == '{') {
				if (visibleChars) {
					std::string s = styler.GetRange(fc.currentPos - visibleChars, fc.currentPos);
					if (std::regex_search(s, reRegisterMacto)) {
						lineState |= 0x1000000;
					}
				}
				if (levelMinCurrent > levelCurrent)
					levelMinCurrent = levelCurrent;
				levelCurrent++;
			}
			else {
				if ((styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK) < levelCurrent)
					levelCurrent--;
				lineState &= ~0x1000000;
			}
		}
		if (!(lineState & 0x1000000)) {
			if (fc.style == SCE_PGSQL_STATEMENT) {
				if (!visibleChars) {
					char lvl = 0;
					std::string s = styler.GetRangeLowered(fc.currentPos, styler.LineEnd(fc.lineCurrent) + 1);
					keywords[KW_PGSQL_INDENT_CLASS].InClassificator(s.c_str(), lvl);
					if (lvl)
						lineState |= lvl << 20;

				}
				// Folding between begin or case and end
				char c = static_cast<char>(tolower(fc.ch));
				if ((c == 'b' || c == 'c' || c == 'e' || c == 'g' || c == 'i' || c == 'l' || c == 'f' || c == 'w') && isspacechar(fc.chPrev)) {
					std::string strSt = fc.GetCurLowered();
					
					Sci_PositionU j;

					if (strSt == "begin") {
						if (fc.GetNextLowered(SCE_PGSQL_DEFAULT) != "transaction") { //не фолдим начало транзакций
							if (levelMinCurrent > levelCurrent)
								levelMinCurrent = levelCurrent;
							levelCurrent++;
						}
					}
					else if (strSt == "loop" || strSt == "case") {
						if (levelMinCurrent > levelCurrent)
							levelMinCurrent = levelCurrent;
						levelCurrent++;
					}
					else if (strSt ==  "if") {
						if (levelMinCurrent > levelCurrent)
							levelMinCurrent = levelCurrent;
						levelCurrent++;
						levelCurrent++;
					}
					else if (strSt ==  "else" || strSt == "elseif" || strSt == "when") {
						int prevLevel = styler.LevelAt(fc.lineCurrent - 1);
						if (!(prevLevel & SC_FOLDLEVELHEADERFLAG) && !visibleChars) {
							levelMinCurrent--;
						}

					}
					else if (strSt == "create") {
						strSt = fc.GetNextLowered(SCE_PGSQL_DEFAULT);
						if (strSt == "proc" || strSt == "procedure" || strSt == "function" || strSt == "trigget" || strSt == "view" || strSt == "table") {//не фолдим создание транзакций и временных таблиц						
							if (levelMinCurrent > levelCurrent)
								levelMinCurrent = levelCurrent;
							levelCurrent++;
						}
					}
					else if (strSt == "end" && ((styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK) < levelCurrent)) {
						levelCurrent--;
						strSt = fc.GetNextLowered(SCE_PGSQL_DEFAULT);
						if (strSt == "if") {
							levelCurrent--;
						}
					}
					else if (strcmp(s, "go") == 0) {
						levelCurrent = styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK;
					}
				}
			}
			else if (fc.style == SCE_PGSQL_OPERATOR) {
				if (fc.ch == ')')
					levelCurrent--;
				else if (fc.ch == '(') {
					if (levelMinCurrent > levelCurrent)
						levelMinCurrent = levelCurrent;
					levelCurrent++;
				}
			}
			else if (fc.style == SCE_PGSQL_SYSMCONSTANTS) {
				if (fc.ch == '_') {
					Sci_PositionU j;
					for (j = 0; j < 13; j++) {
						if (!iswordchar(styler[fc.currentPos + j])) {
							break;
						}
						s[j] = static_cast<char>(tolower(styler[fc.currentPos + j]));
						s[j + 1] = '\0';
					}
					if (strcmp(s, "__cmd_check_p") == 0 || strcmp(s, "__cmd_check_f") == 0 || strcmp(s, "__cmd_check_t") == 0 || strcmp(s, "__cmd_check_v") == 0) {
						levelCurrent = styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK;
					}
				}
			}
		}
		if (fc.atEOL) {
			styler.SetLineState(fc.lineCurrent, lineState);
			lineState &= 0x1000000;
			int lev = levelMinCurrent;
			if (((levelCurrent > lev) && (visibleChars > 0)) || caseSwitcher)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			lev |= levelCurrent << 16;
			if (lev != styler.LevelAt(fc.lineCurrent)) {
				styler.SetLevel(fc.lineCurrent, lev);
			}
			caseSwitcher = false;
			levelPrev = levelCurrent;
			levelMinCurrent = levelCurrent;
			visibleChars = 0;
		}
		else if (fc.currentPos == endPos - 1)
			styler.SetLineState(fc.lineCurrent, lineState);
		if (!isspacechar(fc.ch))
			visibleChars++;
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	// int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	// styler.SetLevel(lineCurrent, levelPrev | flagsNext);	
}

LexerModule lmPGSQL(SCLEX_PGSQL, LexerPGSQL::LexerFactoryPGSQL, "pgsql", pgsqlWordListDesc);
