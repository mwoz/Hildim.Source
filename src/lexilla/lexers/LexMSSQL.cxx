// Scintilla source code edit control
/** @file LexMSSQL.cxx
 ** Lexer for MSSQL.
 **/
// By Filip Yaghob <fyaghob@gmail.com>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <string>
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
#include "DefaultLexer.h"

using namespace Scintilla;
using namespace Lexilla;

#define KW_MSSQL_STATEMENTS         0
#define KW_MSSQL_DATA_TYPES         1
#define KW_MSSQL_SYSTEM_TABLES      2
#define KW_MSSQL_GLOBAL_VARIABLES   3
#define KW_MSSQL_FUNCTIONS          4
#define KW_MSSQL_STORED_PROCEDURES  5
#define KW_MSSQL_OPERATORS          6
#define KW_MSSQL_M4KEYS             7
#define KW_MSSQL_INDENT_CLASS       8

static const char * const sqlWordListDesc[] = {
	"Statements",
	"Data Types",
	"System tables",
	"Global variables",
	"Functions",
	"System Stored Procedures",
	"Operators",
	0,
};

struct OptionsMSSQL {
	bool fold;
	bool foldComment;
	bool foldCompact;
	bool foldBracket;
	bool foldQuerry;
	OptionsMSSQL() {
		fold = false;
		foldComment = false;
		foldCompact = false;
		foldBracket = true;
		foldQuerry = true;
	}
};

struct OptionsSetMSSQL : public OptionSet<OptionsMSSQL> {
	OptionsSetMSSQL() {

		DefineProperty("fold", &OptionsMSSQL::fold, "");

		DefineProperty("fold.comment", &OptionsMSSQL::foldComment,
			"This option enables folding multi-line comments and explicit fold points when using the C++ lexer. "
			"Explicit fold points allows adding extra folding by placing a //{ comment at the start and a //} "
			"at the end of a section that should fold.");

		DefineProperty("fold.compact", &OptionsMSSQL::foldCompact, "");
		DefineProperty("lexer.mssql.fold.querry", &OptionsMSSQL::foldQuerry, "");
		DefineProperty("lexer.mssql.fold.bracket", &OptionsMSSQL::foldBracket, "");

		DefineWordListSets(sqlWordListDesc);

	}
};

class LexerMSSQL : public DefaultLexer {

	OptionsMSSQL options;
	OptionsSetMSSQL osMSSQL;
	WordList keywords[9];	//переданные нам вордлисты
	const std::regex reCreateFold, reRegisterMacto;
	class StyleContextEx : public StyleContext
	{
	public:
		StyleContextEx(unsigned int startPos,
			unsigned int length,
			int initStyle,
			LexAccessor &styler_, char chMask)
			: StyleContext(startPos, length, initStyle, styler_, chMask)
			, prevState(initStyle) {}
		void SetStateEx(int state_) {
			prevState = state;
			SetState(state_);
		}
		int prevState;
	};
	char classifyWordSQL(Sci_PositionU start,LexAccessor &styler, StyleContextEx &sc);


public:
	LexerMSSQL() : DefaultLexer("sql", SCLEX_MSSQL), 
		reCreateFold("^\\s*(proc\\W)|(procedure\\W)|(trigger\\W)|(view\\W)|(function\W)|(table\\s+\\w)", std::regex::ECMAScript),
		reRegisterMacto("^\\s*__REGISTER_(REPORT)|(PLUGIN)|(WIZARD)|(FORM)_", std::regex::ECMAScript)
	{}

	~LexerMSSQL() {}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvRelease4;
	}
	const char * SCI_METHOD PropertyNames() {
		return osMSSQL.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) {
		return osMSSQL.PropertyType(name);
	}
	const char * SCI_METHOD DescribeProperty(const char *name) {
		return osMSSQL.DescribeProperty(name);
	}
	int SCI_METHOD PropertySet(const char *key, const char *val);
	const char * SCI_METHOD DescribeWordListSets() {
		return osMSSQL.DescribeWordListSets();
	}
	int SCI_METHOD WordListSet(int n, const char *wl);
	void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess);
	void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess);
	const char * SCI_METHOD PropertyGet(const char *key) override;
	
	const char * SCI_METHOD GetName() override {
		return "mssql";
	}
	static ILexer5 *LexerFactoryMSSQL() {
		return new LexerMSSQL();
	}

};

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


int SCI_METHOD LexerMSSQL::PropertySet(const char *key, const char *val) {
	if (osMSSQL.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

const char * SCI_METHOD LexerMSSQL::PropertyGet(const char *key) {
	return osMSSQL.PropertyGet(key);
}

int SCI_METHOD LexerMSSQL::WordListSet(int n, const char *wl) {
	int firstModification = -1;
	if (n < 9) {
		WordList *wordListN = 0;
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

char LexerMSSQL::classifyWordSQL(Sci_PositionU start, LexAccessor &styler, StyleContextEx &sc) {

	char s[256];
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');

	sc.GetCurrentLowered(s, sizeof(s));

	char chAttr = SCE_MSSQL_IDENTIFIER;

	if (sc.state == SCE_MSSQL_GLOBAL_VARIABLE) {

        if (keywords[KW_MSSQL_GLOBAL_VARIABLES].InList(&s[2]))
            chAttr = SCE_MSSQL_GLOBAL_VARIABLE;

	} else if (wordIsNumber) {
		chAttr = SCE_MSSQL_NUMBER;

	} else if (sc.prevState == SCE_MSSQL_DEFAULT_PREF_DATATYPE) {
		// Look first in datatypes
        if (keywords[KW_MSSQL_DATA_TYPES].InList(s))
            chAttr = SCE_MSSQL_DATATYPE;
		else if (keywords[KW_MSSQL_OPERATORS].InList(s))
			chAttr = SCE_MSSQL_OPERATOR;
		else if (keywords[KW_MSSQL_STATEMENTS].InList(s))
			chAttr = SCE_MSSQL_STATEMENT;
		else if (keywords[KW_MSSQL_SYSTEM_TABLES].InList(s))
			chAttr = SCE_MSSQL_SYSTABLE;
		else if (keywords[KW_MSSQL_FUNCTIONS].InList(s))
            chAttr = SCE_MSSQL_FUNCTION;
		else if (keywords[KW_MSSQL_STORED_PROCEDURES].InList(s))
			chAttr = SCE_MSSQL_STORED_PROCEDURE;
		else if (keywords[KW_MSSQL_M4KEYS].InList(s))
			chAttr = SCE_MSSQL_M4KEYS;
	}
	else {
		if (keywords[KW_MSSQL_OPERATORS].InList(s))
			chAttr = SCE_MSSQL_OPERATOR;
		else if (keywords[KW_MSSQL_STATEMENTS].InList(s))
			chAttr = SCE_MSSQL_STATEMENT;
		else if (keywords[KW_MSSQL_SYSTEM_TABLES].InList(s))
			chAttr = SCE_MSSQL_SYSTABLE;
		else if (keywords[KW_MSSQL_FUNCTIONS].InList(s))
			chAttr = SCE_MSSQL_FUNCTION;
		else if (keywords[KW_MSSQL_STORED_PROCEDURES].InList(s))
			chAttr = SCE_MSSQL_STORED_PROCEDURE;
		else if (keywords[KW_MSSQL_DATA_TYPES].InList(s))
			chAttr = SCE_MSSQL_DATATYPE;
		else if (keywords[KW_MSSQL_M4KEYS].InList(s))
			chAttr = SCE_MSSQL_M4KEYS;
	}

	//sc.SetStateEx(chAttr);
	sc.ChangeState(chAttr);

	return chAttr;
}

void SCI_METHOD LexerMSSQL::Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	Accessor styler(pAccess, NULL);

	styler.StartAt(startPos);

	Sci_Position lineCurrent = styler.GetLine(startPos);
	int spaceFlags = 0;

	StyleContextEx sc(startPos, length, initStyle, styler, (char)(STYLE_MAX));
	Sci_PositionU lengthDoc = startPos + length;
	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) {

		// When the last char isn't part of the state (have to deal with it too)...
		if ( (sc.state == SCE_MSSQL_IDENTIFIER) ||
			(sc.state == SCE_MSSQL_STORED_PROCEDURE) ||
			(sc.state == SCE_MSSQL_DATATYPE) ||
			(sc.state == SCE_MSSQL_M4KEYS) ||
			(sc.state == SCE_MSSQL_FUNCTION) ||
			(sc.state == SCE_MSSQL_VARIABLE)) {
			if (!iswordchar(sc.ch)) {
				int stateTmp;

				if ((sc.state == SCE_MSSQL_VARIABLE) || (sc.state == SCE_MSSQL_COLUMN_NAME)) {
					sc.SetStateEx(sc.state);
					stateTmp = sc.state;
                } else
					stateTmp = classifyWordSQL(styler.GetStartSegment(), styler, sc);

				if (stateTmp == SCE_MSSQL_IDENTIFIER || stateTmp == SCE_MSSQL_VARIABLE)
					sc.SetStateEx(SCE_MSSQL_DEFAULT_PREF_DATATYPE);
				else
					sc.SetStateEx(SCE_MSSQL_DEFAULT);
			}
		} else if(sc.state == SCE_MSSQL_SYSMCONSTANTS){
			if (!iswordchar(sc.ch) || sc.ch == '.') {
				sc.SetStateEx(SCE_MSSQL_DEFAULT);
			}
		} else if (sc.state == SCE_MSSQL_LINE_COMMENT || sc.state == SCE_MSSQL_LINE_COMMENT_EX) {
			if (sc.ch == '\r' || sc.ch == '\n') {
				sc.SetStateEx(SCE_MSSQL_DEFAULT);
			}
		} else if (sc.state == SCE_MSSQL_GLOBAL_VARIABLE) {
			if ((sc.ch != '@') && !iswordchar(sc.ch)) {
				classifyWordSQL(styler.GetStartSegment(), styler, sc);
				sc.SetStateEx(SCE_MSSQL_DEFAULT);
			}
		} else if(sc.state == SCE_MSSQL_OPERATOR || sc.state == SCE_MSSQL_M4KBRASHES ){
			sc.SetStateEx(SCE_MSSQL_DEFAULT);
		}

		// If is the default or one of the above succeeded
		if (sc.state == SCE_MSSQL_DEFAULT || sc.state == SCE_MSSQL_DEFAULT_PREF_DATATYPE) {
			int stateTmp = sc.state;
			sc.ChangeState(SCE_MSSQL_DEFAULT);
			if ((sc.chPrev == '\r' || sc.chPrev == '\n' || sc.currentPos == startPos) && sc.ch == 'd' && sc.chNext == 'n' && styler.SafeGetCharAt(sc.currentPos + 2) == 'l') { 
				char c = styler.SafeGetCharAt(sc.currentPos + 3);
				if(c == ' ' || c == '\t' || c == '\r' || c == '\n') 	
					sc.SetStateEx(SCE_MSSQL_LINE_COMMENT_EX);
			}else if (sc.ch == '_' && sc.chNext == '_'){
				sc.SetStateEx(SCE_MSSQL_SYSMCONSTANTS);
			}else if (iswordstart(sc.ch)) {
				sc.SetStateEx(SCE_MSSQL_IDENTIFIER);
			} else if (sc.ch == '/' && sc.chNext == '*') {
				sc.SetStateEx(SCE_MSSQL_COMMENT);
			} else if (sc.ch == '-' && sc.chNext == '-') {
				sc.SetStateEx(SCE_MSSQL_LINE_COMMENT);
			} else if (sc.ch == '\'') {
				sc.SetStateEx(SCE_MSSQL_STRING);
			} else if (sc.ch == '"') {
				sc.SetStateEx(SCE_MSSQL_COLUMN_NAME);
			} else if (sc.ch == '[') {
				sc.SetStateEx(SCE_MSSQL_COLUMN_NAME_2);
			} else if((sc.ch == '{' || sc.ch == '}') && keywords[KW_MSSQL_M4KEYS].InList("}")) {
				sc.SetStateEx(SCE_MSSQL_M4KBRASHES);
				//sc.SetStateEx(SCE_MSSQL_DEFAULT);
			} else if (isMSSQLOperator(sc.ch)) {
				sc.SetStateEx(SCE_MSSQL_OPERATOR);
				//sc.SetStateEx(SCE_MSSQL_DEFAULT);
			} else if (sc.ch == '@') {
                if (sc.chNext == '@') {
					sc.SetStateEx(SCE_MSSQL_GLOBAL_VARIABLE);
                } else
					sc.SetStateEx(SCE_MSSQL_VARIABLE);
			} else {
				sc.ChangeState(stateTmp); //??
            }


		// When the last char is part of the state...
		} else if (sc.state == SCE_MSSQL_COMMENT) {
				if (sc.ch == '/' && sc.chPrev == '*') {
					if (((sc.currentPos > (styler.GetStartSegment() + 2)) || ((initStyle == SCE_MSSQL_COMMENT) &&
					    (styler.GetStartSegment() == startPos)))) {
						sc.SetStateEx(SCE_MSSQL_DEFAULT);
					}
				}
			} else if (sc.state == SCE_MSSQL_STRING) {
				if (sc.ch == '\'') {
					if (sc.chNext == '\'' ) {
						sc.Forward();
					} else {
						sc.SetStateEx(SCE_MSSQL_DEFAULT);
					}
				}
			} else if (sc.state == SCE_MSSQL_COLUMN_NAME) {
				if (sc.ch == '"') {
					if (sc.chNext == '"') {
						sc.Forward();
				} else {
					sc.SetStateEx(SCE_MSSQL_DEFAULT_PREF_DATATYPE);
                }
                }
		} else if (sc.state == SCE_MSSQL_COLUMN_NAME_2) {
			if (sc.ch == ']') {
				sc.SetStateEx(SCE_MSSQL_DEFAULT_PREF_DATATYPE);
			}
		}
	}
	sc.Complete();
}

void SCI_METHOD LexerMSSQL::Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	if (!options.fold)
		return;
	LexAccessor styler(pAccess);

	bool foldComment = options.foldComment;
	bool foldCompact = options.foldCompact;
	Sci_PositionU endPos = startPos + length;
	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelPrev = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelPrev = (styler.LevelAt(lineCurrent - 1) >> 16) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	int levelMinCurrent = levelPrev;
	char chNext = styler[startPos];
	bool inComment = (styler.StyleAt(startPos - 1) == SCE_MSSQL_COMMENT);
	char s[15];
	int lineState = 0;
	if (lineCurrent > 0)
		lineState = styler.GetLineState(lineCurrent - 1) & 0x1000000;
	for (Sci_PositionU i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int style = styler.StyleAt(i);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
        // Comment folding
		if (foldComment) {
			if (!inComment && (style == SCE_MSSQL_COMMENT)) {
				if (levelMinCurrent > levelCurrent) 
					levelMinCurrent = levelCurrent;
				levelCurrent++;
			}
			else if (inComment && (style != SCE_MSSQL_COMMENT))
				levelCurrent--;
			inComment = (style == SCE_MSSQL_COMMENT);
		}
		if (style == SCE_MSSQL_M4KBRASHES){
			if (ch == '{') {
				if (visibleChars) {
					std::string s = styler.GetRange(i - visibleChars, i);
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
			if (style == SCE_MSSQL_STATEMENT) {
				if (!visibleChars) {
					char lvl = 0;
					std::string s = styler.GetRangeLowered(i, styler.LineEnd(lineCurrent) + 1);
					keywords[KW_MSSQL_INDENT_CLASS].InClassificator(s.c_str(), lvl);
					if (lvl)
						lineState |= lvl << 20;

				}
				// Folding between begin or case and end
				char c = static_cast<char>(tolower(ch));
				if (c == 'b' || c == 'c' || c == 'e' || c == 'g') {
					Sci_PositionU j;
					for (j = 0; j < 6; j++) {
						if (!iswordchar(styler[i + j])) {
							break;
						}
						s[j] = static_cast<char>(tolower(styler[i + j]));
						s[j + 1] = '\0';
					}
					Sci_PositionU ii = i + j;

					if ((strcmp(s, "begin") == 0) || (strcmp(s, "case") == 0)) {
						if (levelMinCurrent > levelCurrent)
							levelMinCurrent = levelCurrent;
						levelCurrent++;
						Sci_PositionU l;
						char sNext[200];
						sNext[0] = '\0';
						for (l = 0; l < 200 && (styler[ii + l] == ' ' || styler[ii + l] == '\t' || styler[ii + l] == '\n' || styler[ii + l] == '\r'); l++); //Проматываем пробелы между словами
						if (l < 200) {
							unsigned int k;
							for (k = 0; k < 200; k++) {
								if (!iswordchar(styler[ii + k + l])) break;
								if (styler.StyleAt(ii + k + l) != SCE_MSSQL_STATEMENT) break;
								sNext[k] = styler[ii + k + l];
								sNext[k + 1] = '\0';
							}
							_strlwr_s(sNext, 200);
							if ((strcmp(sNext, "transaction") == 0)) {//не фолдим начало транзакции
								levelCurrent--;
							}
						}
					} else if (strcmp(s, "create") == 0) {
						std::string s = styler.GetRangeLowered(i, styler.LineEnd(lineCurrent) + 1);
						if (std::regex_search(s, reCreateFold)) {//не фолдим создание транзакций и временных таблиц						
							if (levelMinCurrent > levelCurrent)
								levelMinCurrent = levelCurrent;
							levelCurrent++;
						}
					} else if (strcmp(s, "end") == 0 && ((styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK) < levelCurrent)) {
						levelCurrent--;
					} else if (strcmp(s, "go") == 0) {
						levelCurrent = styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK;
					}
				}
			} else if (style == SCE_MSSQL_OPERATOR) {
				if (styler[i] == ')')
					levelCurrent--;
				else if (styler[i] == '(') {
					if (levelMinCurrent > levelCurrent)
						levelMinCurrent = levelCurrent;
					levelCurrent++;
				}
			} else if (style == SCE_MSSQL_SYSMCONSTANTS) {
				char c = static_cast<char>(tolower(ch));
				if (c == '_') {
					Sci_PositionU j;
					for (j = 0; j < 13; j++) {
						if (!iswordchar(styler[i + j])) {
							break;
						}
						s[j] = static_cast<char>(tolower(styler[i + j]));
						s[j + 1] = '\0';
					}
					if (strcmp(s, "__cmd_check_p") == 0 || strcmp(s, "__cmd_check_f") == 0 || strcmp(s, "__cmd_check_t") == 0 || strcmp(s, "__cmd_check_v") == 0) {
						levelCurrent = styler.LevelAt(0) & SC_FOLDLEVELNUMBERMASK;
					}
				}
			}
		}
		if (atEOL) {
			styler.SetLineState(lineCurrent, lineState);
			lineState &= 0x1000000;
			int lev = levelMinCurrent;
			if ((levelCurrent > lev) && (visibleChars > 0))
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			lev |= levelCurrent << 16;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev = levelCurrent;
			levelMinCurrent = levelCurrent;
			visibleChars = 0;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	// int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	// styler.SetLevel(lineCurrent, levelPrev | flagsNext);
}

LexerModule lmMSSQL(SCLEX_MSSQL, LexerMSSQL::LexerFactoryMSSQL, "mssql", sqlWordListDesc);
