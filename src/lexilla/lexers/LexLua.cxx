// Scintilla source code edit control
/** @file LexLua.cxx
 ** Lexer for Lua language.
 **
 ** Written by Paul Winwood.
 ** Folder by Alexey Yutkin.
 ** Modified by Marcos E. Wurzius & Philippe Lhoste
 **/

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

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "StringCopy.h"
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

static const char * const luaWordListDesc[] = {
	"Keywords",
	"Basic functions",
	"String, (table) & math functions",
	"(coroutines), I/O & system facilities",
	"user1",
	"user2",
	"user3",
	"user4",
	"user5",
	"Attributes",
	0
};

struct OptionsLua {
	bool fold;
	bool foldComment;
	bool foldCompact;
	bool foldAtElse;
	OptionsLua() {
		fold = false;
		foldComment = false;
		foldCompact = false;
		foldAtElse = false;
	}
};

struct OptionsSetLua : public OptionSet<OptionsLua> {
	OptionsSetLua() {

		DefineProperty("fold", &OptionsLua::fold, "");

		DefineProperty("fold.comment", &OptionsLua::foldComment,
			"This option enables folding multi-line comments and explicit fold points when using the C++ lexer. "
			"Explicit fold points allows adding extra folding by placing a //{ comment at the start and a //} "
			"at the end of a section that should fold.");

		DefineProperty("fold.compact", &OptionsLua::foldCompact, "");

		DefineProperty("fold.at.else", &OptionsLua::foldAtElse,
			"This option enables C++ folding on a \"} else {\" line of an if statement.");

		DefineWordListSets(luaWordListDesc);

	}
};

class LexerLua : public DefaultLexer {

	OptionsLua options;
	OptionsSetLua osLua;
	WordList keywords[10];	//переданные нам вордлисты
public:
	
	LexerLua() : DefaultLexer("lua", SCLEX_LUA) {}
	~LexerLua() {}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvRelease4;
	}
	const char * SCI_METHOD PropertyNames() {
		return osLua.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) {
		return osLua.PropertyType(name);
	}
	const char * SCI_METHOD DescribeProperty(const char *name) {
		return osLua.DescribeProperty(name);
	}
	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val);
	const char * SCI_METHOD DescribeWordListSets() {
		return osLua.DescribeWordListSets();
	}
	Sci_Position SCI_METHOD WordListSet(int n, const char *wl);
	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess);
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess);
	const char * SCI_METHOD PropertyGet(const char *key) override;

	static ILexer5 *LexerFactoryLua() {
		return new LexerLua();
	}

};

// Test for [=[ ... ]=] delimiters, returns 0 if it's only a [ or ],
// return 1 for [[ or ]], returns >=2 for [=[ or ]=] and so on.
// The maximum number of '=' characters allowed is 254.
static int LongDelimCheck(StyleContext &sc) {
	int sep = 1;
	while (sc.GetRelative(sep) == '=' && sep < 0xFF)
		sep++;
	if (sc.GetRelative(sep) == sc.ch)
		return sep;
	return 0;
}

Sci_Position SCI_METHOD LexerLua::PropertySet(const char *key, const char *val) {
	if (osLua.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

const char * SCI_METHOD LexerLua::PropertyGet(const char *key) {
	return osLua.PropertyGet(key);
}

Sci_Position SCI_METHOD LexerLua::WordListSet(int n, const char *wl) {
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

void SCI_METHOD LexerLua::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess){
	LexAccessor styler(pAccess);

	// Accepts accented characters
	CharacterSet setWordStart(CharacterSet::setAlpha, "_", 0x80, true);
	CharacterSet setWord(CharacterSet::setAlphaNum, "_", 0x80, true);
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases. [pP] is for hex floats.
	CharacterSet setNumber(CharacterSet::setDigits, ".-+abcdefpABCDEFP");
	CharacterSet setExponent("eEpP");
	CharacterSet setLuaOperator("*/-+()={}~[];<>,.^%:#&|");
	CharacterSet setEscapeSkip("\"'\\");

	// if stay on identifier or operator then go back to start of object
	while (startPos > 1 && (
		initStyle == SCE_LUA_WORD ||
		initStyle == SCE_LUA_OPERATOR ||
		initStyle == SCE_LUA_IDENTIFIER ||
		(initStyle >= SCE_LUA_WORD2 && initStyle <= SCE_LUA_WORD8)))
	{
		startPos--;
		initStyle = styler.StyleAt(startPos-1);
	}

	Sci_Position currentLine = styler.GetLine(startPos);
	// Initialize long string [[ ... ]] or block comment --[[ ... ]] nesting level,
	// if we are inside such a string. Block comment was introduced in Lua 5.0,
	// blocks with separators [=[ ... ]=] in Lua 5.1.
	// Continuation of a string (\z whitespace escaping) is controlled by stringWs.
	int nestLevel = 0;
	int sepCount = 0;
	int stringWs = 0;
	if (initStyle == SCE_LUA_LITERALSTRING || initStyle == SCE_LUA_COMMENT ||
		initStyle == SCE_LUA_STRING || initStyle == SCE_LUA_CHARACTER) {
		const int lineState = styler.GetLineState(currentLine - 1);
		nestLevel = lineState >> 9;
		sepCount = lineState & 0xFF;
		stringWs = lineState & 0x100;
	}

	// Do not leak onto next line
	if (initStyle == SCE_LUA_STRINGEOL || initStyle == SCE_LUA_COMMENTLINE || initStyle == SCE_LUA_PREPROCESSOR) {
		initStyle = SCE_LUA_DEFAULT;
	}

	unsigned int objectPartEndPos = 0;
	bool isObject = false;
	bool isObjectStart = false;
	bool isSubObject = false;
	char sChar = 0;
	class StyleContextEx : public StyleContext
	{
	public:
		StyleContextEx( unsigned int startPos,
						unsigned int length,
						int initStyle,
						LexAccessor &styler_ )
		: StyleContext( startPos, length, initStyle, styler_)
		, endPosEx( startPos + length )
		, stylerEx( styler_ )
		{
		}
		void MoveTo(unsigned int pos) {
			if (pos < endPosEx) {
				pos--;
				currentPos = pos;
				chPrev = 0;
				ch = static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos));
				if (stylerEx.IsLeadByte(static_cast<char>(ch))) {
					pos++;
					ch = ch << 8;
					ch |= static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos));
				}
				chNext = static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos+1));
				if (stylerEx.IsLeadByte(static_cast<char>(chNext))) {
					chNext = chNext << 8;
					chNext |= static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos+2));
				}
				// End of line?
				// Trigger on CR only (Mac style) or either on LF from CR+LF (Dos/Win)
				// or on LF alone (Unix). Avoid triggering two times on Dos/Win.
				atLineEnd = (ch == '\r' && chNext != '\n') ||
							(ch == '\n') ||
							(currentPos >= endPosEx);
				Forward();
			} else {
				currentPos = endPosEx;
				atLineStart = false;
				chPrev = ' ';
				ch = ' ';
				chNext = ' ';
				atLineEnd = true;
			}
		}
	private:
		unsigned int endPosEx;
		LexAccessor &stylerEx;
	};
	StyleContextEx sc(startPos, length, initStyle, styler);

	if (startPos == 0 && sc.ch == '#') {
		// shbang line: # is a comment only if first char of the script
		sc.SetState(SCE_LUA_COMMENTLINE);
	}

	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) { 
		if (sc.atLineEnd) {
			// Update the line state, so it can be seen by next line
			currentLine = styler.GetLine(sc.currentPos);
			switch (sc.state) {
			case SCE_LUA_LITERALSTRING:
			case SCE_LUA_COMMENT:
			case SCE_LUA_STRING:
			case SCE_LUA_CHARACTER:
				// Inside a literal string, block comment or string, we set the line state
				styler.SetLineState(currentLine, (nestLevel << 9) | stringWs | sepCount);
				break;
			default:
				// Reset the line state
				styler.SetLineState(currentLine, 0);
				break;
			}
		}
		if (sc.atLineStart && (sc.state == SCE_LUA_STRING)) {
			// Prevent SCE_LUA_STRINGEOL from leaking back to previous line
			sc.SetState(SCE_LUA_STRING);
		}

		// Handle string line continuation
		if ((sc.state == SCE_LUA_STRING || sc.state == SCE_LUA_CHARACTER) &&
				sc.ch == '\\') {
			if (sc.chNext == '\n' || sc.chNext == '\r') {
				sc.Forward();
				if (sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}

		// Determine if the current state should terminate.
		if (sc.state == SCE_LUA_OPERATOR) {
			if (sc.ch == ':' && sc.chPrev == ':') {	// :: <label> :: forward scan
				sc.Forward();
				Sci_Position ln = 0;
				while (IsASpaceOrTab(sc.GetRelative(ln)))	// skip over spaces/tabs
					ln++;
				Sci_Position ws1 = ln;
				if (setWordStart.Contains(sc.GetRelative(ln))) {
					int c, i = 0;
					char s[100];
					while (setWord.Contains(c = sc.GetRelative(ln))) {	// get potential label
						if (i < 90)
							s[i++] = static_cast<char>(c);
						ln++;
					}
					s[i] = '\0'; Sci_Position lbl = ln;
					if (!keywords[0].InList(s)) {
						while (IsASpaceOrTab(sc.GetRelative(ln)))	// skip over spaces/tabs
							ln++;
						Sci_Position ws2 = ln - lbl;
						if (sc.GetRelative(ln) == ':' && sc.GetRelative(ln + 1) == ':') {
							// final :: found, complete valid label construct
							sc.ChangeState(SCE_LUA_LABEL);
							if (ws1) {
								sc.SetState(SCE_LUA_DEFAULT);
								sc.ForwardBytes(ws1);
							}
							sc.SetState(SCE_LUA_LABEL);
							sc.ForwardBytes(lbl - ws1);
							if (ws2) {
								sc.SetState(SCE_LUA_DEFAULT);
								sc.ForwardBytes(ws2);
							}
							sc.SetState(SCE_LUA_LABEL);
							sc.ForwardBytes(2);
						}
					}
				}
			}
			sc.SetState(SCE_LUA_DEFAULT);
		} else if (sc.state == SCE_LUA_NUMBER) {
			// We stop the number definition on non-numerical non-dot non-eEpP non-sign non-hexdigit char
			if (!setNumber.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_DEFAULT);
			} else if (sc.ch == '-' || sc.ch == '+') {
				if (!setExponent.Contains(sc.chPrev))
					sc.SetState(SCE_LUA_DEFAULT);
			}

		} else if (sc.state == SCE_LUA_IDENTIFIER
				|| sc.state == SCE_LUA_ATTRIBUTE
				|| sc.state == SCE_LUA_WORD
				|| sc.state == SCE_LUA_WORD2
				|| sc.state == SCE_LUA_WORD3
				|| sc.state == SCE_LUA_WORD4
				|| sc.state == SCE_LUA_WORD5
				|| sc.state == SCE_LUA_WORD6
				|| sc.state == SCE_LUA_WORD7
				|| sc.state == SCE_LUA_WORD8) {
			if (!setWord.Contains(sc.ch)) {
				bool isFin;
				if ((sc.ch == ':' || sc.ch == '.') && setWordStart.Contains(sc.chNext)) {
					// continue with object fields
					if (!isObject) {
						isObject = true;
						isObjectStart = true;
						objectPartEndPos = sc.currentPos;
						continue;
				}
					isFin = false;
			} else {
					isFin = true;
			}

				char s[100];
				sc.GetCurrent(s, sizeof(s));
				if (keywords[0].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD);
					if (strcmp(s, "goto") == 0) {	// goto <label> forward scan
			sc.SetState(SCE_LUA_DEFAULT);
				while (IsASpaceOrTab(sc.ch) && !sc.atLineEnd)
					sc.Forward();
				if (setWordStart.Contains(sc.ch)) {
					sc.SetState(SCE_LUA_LABEL);
					sc.Forward();
					while (setWord.Contains(sc.ch))
						sc.Forward();
					sc.GetCurrent(s, sizeof(s));
							if (keywords[0].InList(s))
						sc.ChangeState(SCE_LUA_WORD);
				}
				sc.SetState(SCE_LUA_DEFAULT);
			}
				} else if (keywords[1].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD2);
				} else if (keywords[2].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD3);
				} else if (keywords[3].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD4);
				} else if (keywords[4].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD5);
				} else if (keywords[5].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD6);
				} else if (keywords[6].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD7);
				} else if (keywords[7].InList(s)) {
					sc.ChangeState(SCE_LUA_WORD8);
				} else if (isObject || isSubObject) {
					// colourise objects part separately&& sc.currentPos <= objectPartEndPos
					if (isObject ) {
						int currPos = sc.currentPos;
						sc.MoveTo(objectPartEndPos);
						if (isObjectStart && sc.currentPos <= objectPartEndPos) {
							sc.GetCurrent(s, sizeof(s));
							if (keywords[0].InList(s)) {
						        sc.ChangeState(SCE_LUA_WORD);
							} else if (keywords[1].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD2);
							} else if (keywords[2].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD3);
							} else if (keywords[3].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD4);
							} else if (keywords[4].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD5);
							} else if (keywords[5].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD6);
							} else if (keywords[6].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD7);
							} else if (keywords[7].InList(s)) {
								sc.ChangeState(SCE_LUA_WORD8);
							}
							isObjectStart = false;
						}
						sc.SetState(SCE_LUA_OPERATOR);
						s[0] = static_cast<char>(sc.ch);
						sc.Forward();
						sc.SetState(SCE_LUA_IDENTIFIER);
						sc.MoveTo(currPos);
					} else {
						s[0] = sChar;
					}
					sc.GetCurrent(s + 1, sizeof(s) - 1);
					if (keywords[0].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD);
					} else if (keywords[1].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD2);
					} else if (keywords[2].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD3);
					} else if (keywords[3].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD4);
					} else if (keywords[4].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD5);
					} else if (keywords[5].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD6);
					} else if (keywords[6].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD7);
					} else if (keywords[7].InList(s)) {
						sc.ChangeState(SCE_LUA_WORD8);
					}
					objectPartEndPos = sc.currentPos;
				}
				if (isFin) {
					isObject = false;
				}
				sc.SetState(SCE_LUA_DEFAULT);
			}
		} else if (sc.state == SCE_LUA_COMMENTLINE || sc.state == SCE_LUA_PREPROCESSOR) {
			if (sc.atLineEnd) {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		} else if (sc.state == SCE_LUA_STRING) {
			if (stringWs) {
				if (!IsASpace(sc.ch))
					stringWs = 0;
			}
			if (sc.ch == '\\') {
				if (setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				} else if (sc.chNext == 'z') {
					sc.Forward();
					stringWs = 0x100;
				}
			} else if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			} else if (stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_LUA_STRINGEOL);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		} else if (sc.state == SCE_LUA_CHARACTER) {
			if (stringWs) {
				if (!IsASpace(sc.ch))
					stringWs = 0;
			}
			if (sc.ch == '\\') {
				if (setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				} else if (sc.chNext == 'z') {
					sc.Forward();
					stringWs = 0x100;
				}
			} else if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			} else if (stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_LUA_STRINGEOL);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		} else if (sc.state == SCE_LUA_LITERALSTRING || sc.state == SCE_LUA_COMMENT) {
			if (sc.ch == '[') {
			const int sep = LongDelimCheck(sc);
				if (sep == 1 && sepCount == 1) {    // [[-only allowed to nest
					nestLevel++;
					sc.Forward();
				}
			} else if (sc.ch == ']') {
				int sep = LongDelimCheck(sc);
				if (sep == 1 && sepCount == 1) {    // un-nest with ]]-only
					nestLevel--;
					sc.Forward();
					if (nestLevel == 0) {
						sc.ForwardSetState(SCE_LUA_DEFAULT);
					}
				} else if (sep > 1 && sep == sepCount) {   // ]=]-style delim
				sc.Forward(sep);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		}
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_LUA_DEFAULT) {
			if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
				sc.SetState(SCE_LUA_NUMBER);
				if (sc.ch == '0' && toupper(sc.chNext) == 'X') {
					sc.Forward();
				}
			} else if (setWordStart.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_IDENTIFIER);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_LUA_STRING);
				stringWs = 0;
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_LUA_CHARACTER);
				stringWs = 0;
			} else if (sc.ch == '[') {
				sepCount = LongDelimCheck(sc);
				if (sepCount == 0) {
					sc.SetState(SCE_LUA_OPERATOR);
				} else {
					nestLevel = 1;
					sc.SetState(SCE_LUA_LITERALSTRING);
					sc.Forward(sepCount);
				}
			} else if (sc.Match('-', '-')) {
				sc.SetState(SCE_LUA_COMMENTLINE);
				if (sc.Match("--[")) {
					sc.Forward(2);
					sepCount = LongDelimCheck(sc);
					if (sepCount > 0) {
						nestLevel = 1;
						sc.ChangeState(SCE_LUA_COMMENT);
						sc.Forward(sepCount);
					}
				} else {
					sc.Forward();
				}
			} else if (sc.atLineStart && sc.Match('$')) {
				sc.SetState(SCE_LUA_PREPROCESSOR);	// Obsolete since Lua 4.0, but still in old code
			} else if (setLuaOperator.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_OPERATOR);
				if (sc.ch == '<') {
					Sci_PositionU pc = sc.currentPos;
					int d = 0;
					do {
						sc.Forward();
						d++;
					} while (sc.ch == ' ' || sc.ch == '\t');
					do {
						sc.Forward();
					} while (sc.ch >= 'a' && sc.ch <= 'z');
					char s[100];
					sc.GetCurrent(s, sizeof(s));
					if (keywords[8].InList(s + d)) {
						while (sc.ch == ' ' || sc.ch == '\t')
							sc.Forward();
						
						if (sc.ch == '>') {
							sc.ChangeState(SCE_LUA_ATTRIBUTE);
							pc = NULL;
						}
					}
					if(pc)
						sc.MoveTo(pc);
				}
			}
			if (sc.ch == ')' || sc.ch == ']') {
				isSubObject = true;
			}
			else {
				if (isSubObject && sc.state != SCE_LUA_IDENTIFIER) {
					if (setWordStart.Contains(sc.chNext) && (sc.ch == '.' || sc.ch == ':'))
						sChar = static_cast<char>(sc.ch);
					else
						isSubObject = false;
				}
			}
		}
	}

	sc.Complete();
}

void SCI_METHOD LexerLua::Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess){
	if (!options.fold) 
		return;

	LexAccessor styler(pAccess);

	const Sci_PositionU lengthDoc = startPos + length;
	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelPrev = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelPrev = (styler.LevelAt(lineCurrent - 1) >> 16) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	int levelMinCurrent = levelCurrent;
	char chNext = styler[startPos];
	const bool foldCompact = options.foldCompact;
	int styleNext = styler.StyleAt(startPos);

	for (Sci_PositionU i = startPos; i < lengthDoc; i++) {
		const char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		const int style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		if (style == SCE_LUA_WORD) {
			//if (ch == 'i' || ch == 'd' || ch == 'f' || ch == 'e' || ch == 'r' || ch == 'u') {
				char s[100] = "";
				s[99] = '\0';
				Sci_PositionU j = i + 98;
				Sci_PositionU i0 = i;
				for (; i < j; i++) {
					if (!iswordchar(styler[i])) {
						s[i - i0] = '\0';
						i--;
						chNext = styler.SafeGetCharAt(i + 1);
						styleNext = styler.StyleAt(i + 1);
						break;
					}
					s[i - i0] = styler[i];					
				}

				if ((strcmp(s, "if") == 0) || (strcmp(s, "do") == 0) || (strcmp(s, "function") == 0) || (strcmp(s, "repeat") == 0)) {
					if (options.foldAtElse && levelMinCurrent > levelCurrent) {
						levelMinCurrent = levelCurrent;
					}
					levelCurrent++;
				}else if ((strcmp(s, "end") == 0) || (strcmp(s, "until") == 0)) {
					levelCurrent--;
				} if ((strcmp(s, "else") == 0) || (strcmp(s, "elseif") == 0)) {
					if (options.foldAtElse && levelMinCurrent >= levelCurrent) {
						levelMinCurrent = levelCurrent - 1;
				}
			}
			//}
		} else if (style == SCE_LUA_OPERATOR) {
			if (ch == '{' || ch == '(') {
				if (options.foldAtElse && levelMinCurrent > levelCurrent) {
					levelMinCurrent = levelCurrent;
				}
				levelCurrent++;
			} else if (ch == '}' || ch == ')') {
				levelCurrent--;
			}
		} 
		if ((styleNext == SCE_LUA_LITERALSTRING && style != SCE_LUA_LITERALSTRING) 
			|| (styleNext == SCE_LUA_COMMENT && style != SCE_LUA_COMMENT)) {
			if (options.foldAtElse && levelMinCurrent > levelCurrent) {
				levelMinCurrent = levelCurrent;
			}
				levelCurrent++;
		} else  if ((styleNext != SCE_LUA_LITERALSTRING && style == SCE_LUA_LITERALSTRING && ch ==']') 
			|| (styleNext != SCE_LUA_COMMENT && style == SCE_LUA_COMMENT && ch == ']')) {
				levelCurrent--;
			}

		if (atEOL) {
			int lev = levelPrev;
			if (options.foldAtElse) {
				lev = levelMinCurrent;
			}
			if ((levelCurrent > lev) && (visibleChars > 0)) {
				lev |= SC_FOLDLEVELHEADERFLAG;
			}
			if (visibleChars == 0 && foldCompact) {
				lev |= SC_FOLDLEVELWHITEFLAG;
			}

			if ((levelCurrent & SC_FOLDLEVELNUMBERMASK) < SC_FOLDLEVELBASE)
				levelCurrent = SC_FOLDLEVELBASE;

			lev |= (levelCurrent & SC_FOLDLEVELNUMBERMASK) << 16;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev = levelCurrent;
			levelMinCurrent = levelPrev;
			visibleChars = 0;
		}
		if (!isspacechar(ch)) {
			visibleChars++;
		}
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later

	//int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	//if ((levelCurrent & SC_FOLDLEVELNUMBERMASK) < SC_FOLDLEVELBASE)
	//	levelCurrent = SC_FOLDLEVELBASE;
	//styler.SetLevel(lineCurrent, (options.foldAtElse ? levelMinCurrent : levelPrev) | ((levelCurrent & SC_FOLDLEVELNUMBERMASK) << 16) | flagsNext);
}

namespace {

LexicalClass lexicalClasses[] = {
	// Lexer Lua SCLEX_LUA SCE_LUA_:
	0, "SCE_LUA_DEFAULT", "default", "White space: Visible only in View Whitespace mode (or if it has a back colour)",
	1, "SCE_LUA_COMMENT", "comment", "Block comment (Lua 5.0)",
	2, "SCE_LUA_COMMENTLINE", "comment line", "Line comment",
	3, "SCE_LUA_COMMENTDOC", "comment documentation", "Doc comment -- Not used in Lua (yet?)",
	4, "SCE_LUA_NUMBER", "literal numeric", "Number",
	5, "SCE_LUA_WORD", "keyword", "Keyword",
	6, "SCE_LUA_STRING", "literal string", "(Double quoted) String",
	7, "SCE_LUA_CHARACTER", "literal string character", "Character (Single quoted string)",
	8, "SCE_LUA_LITERALSTRING", "literal string", "Literal string",
	9, "SCE_LUA_PREPROCESSOR", "preprocessor", "Preprocessor (obsolete in Lua 4.0 and up)",
	10, "SCE_LUA_OPERATOR", "operator", "Operators",
	11, "SCE_LUA_IDENTIFIER", "identifier", "Identifier (everything else...)",
	12, "SCE_LUA_STRINGEOL", "error literal string", "End of line where string is not closed",
	13, "SCE_LUA_WORD2", "identifier", "Other keywords",
	14, "SCE_LUA_WORD3", "identifier", "Other keywords",
	15, "SCE_LUA_WORD4", "identifier", "Other keywords",
	16, "SCE_LUA_WORD5", "identifier", "Other keywords",
	17, "SCE_LUA_WORD6", "identifier", "Other keywords",
	18, "SCE_LUA_WORD7", "identifier", "Other keywords",
	19, "SCE_LUA_WORD8", "identifier", "Other keywords",
	20, "SCE_LUA_LABEL", "label", "Labels",
};

}

LexerModule lmLua(SCLEX_LUA, LexerLua::LexerFactoryLua, "lua", luaWordListDesc);