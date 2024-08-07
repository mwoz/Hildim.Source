// Scintilla source code edit control
/** @file LexOthers.cxx
 ** Lexers for batch files, diff results, properties files, make files and error lists.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "PropSetSimple.h"  //!-add-[FindResultListStyle]
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

using namespace Lexilla;

static bool strstart(const char *haystack, const char *needle) {
	return strncmp(haystack, needle, strlen(needle)) == 0;
}

static bool Is0To9(char ch) {
	return (ch >= '0') && (ch <= '9');
}

static bool Is1To9(char ch) {
	return (ch >= '1') && (ch <= '9');
}

static bool IsAlphabetic(int ch) {
	return IsASCII(ch) && isalpha(ch);
}

static inline bool AtEOL(Accessor &styler, unsigned int i) {
	return (styler[i] == '\n') ||
	       ((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}

// Tests for BATCH Operators
static bool IsBOperator(char ch) {
	return (ch == '=') || (ch == '+') || (ch == '>') || (ch == '<') ||
//!		(ch == '|') || (ch == '?') || (ch == '*');
		(ch == '|') || (ch == '?') || (ch == '*') || (ch == '(') || (ch == ')'); //!-change-[BatchLexerImprovement]
}

// Tests for BATCH Separators
static bool IsBSeparator(char ch) {
	return (ch == '\\') || (ch == '.') || (ch == ';') ||
		(ch == '\"') || (ch == '\'') || (ch == '/');
}
//!-start-[BatchLexerImprovement]
// Tests for Environment Variable simbol
static inline bool IsEnvironmentVar(char ch) {
	return isalpha(ch) || Is0To9(ch) || (ch == '_');
}

// Tests for BATCH Variable simbol
static inline bool IsBatchVar(char ch) {
	return isalpha(ch) || Is0To9(ch);
}

// Find length of BATCH Variable with modifier (%~...) or return 0
static unsigned int GetBatchVarLen(char *wordBuffer, unsigned int wbl)
{
	if (wbl > 2 && wordBuffer[0] == '%' && wordBuffer[1] == '~') {
		wordBuffer += 2;
		if (wbl > 5 && wordBuffer[0] == '$' && isalpha(wordBuffer[1])) {
			unsigned int l = 2;
			while (IsEnvironmentVar(wordBuffer[l])) l++;
			if (wordBuffer[l] == ':' && IsBatchVar(wordBuffer[l+1]))
				return l + 4;
		} else
		if (wbl > 7 && 0 == CompareNCaseInsensitive(wordBuffer, "dp$", 3) &&
			isalpha(wordBuffer[3])) {
			unsigned int l = 4;
			while (IsEnvironmentVar(wordBuffer[l])) l++;
			if (wordBuffer[l] == ':' && IsBatchVar(wordBuffer[l+1]))
				return l + 4;
		} else
		if (wbl > 6 && 0 == CompareNCaseInsensitive(wordBuffer, "ftza", 4) &&
			IsBatchVar(wordBuffer[4])) {
			return 7;
		} else
		if (wbl > 4 &&
			(0 == CompareNCaseInsensitive(wordBuffer, "dp", 2) ||
			0 == CompareNCaseInsensitive(wordBuffer, "nx", 2) ||
			0 == CompareNCaseInsensitive(wordBuffer, "fs", 2)) &&
			IsBatchVar(wordBuffer[2])) {
			return 5;
		} else
		if (wbl > 3 &&
			(wordBuffer[0] == 'f' || wordBuffer[0] == 'F' ||
			wordBuffer[0] == 'd' || wordBuffer[0] == 'D' ||
			wordBuffer[0] == 'p' || wordBuffer[0] == 'P' ||
			wordBuffer[0] == 'n' || wordBuffer[0] == 'N' ||
			wordBuffer[0] == 'x' || wordBuffer[0] == 'X' ||
			wordBuffer[0] == 's' || wordBuffer[0] == 'S' ||
			wordBuffer[0] == 'a' || wordBuffer[0] == 'A' ||
			wordBuffer[0] == 't' || wordBuffer[0] == 'T' ||
			wordBuffer[0] == 'z' || wordBuffer[0] == 'Z') &&
			IsBatchVar(wordBuffer[1])) {
			return 4;
		} else
		if (IsBatchVar(wordBuffer[0])) {
			return 3;
		}
	}
	return 0;
}
//!-end-[BatchLexerImprovement]

static void ColouriseBatchLine(
    char *lineBuffer,
    unsigned int lengthLine,
    unsigned int startLine,
    unsigned int endPos,
    WordList *keywordlists[],
    Accessor &styler) {

	unsigned int offset = 0;	// Line Buffer Offset
	unsigned int cmdLoc;		// External Command / Program Location
	char wordBuffer[81];		// Word Buffer - large to catch long paths
	unsigned int wbl;		// Word Buffer Length
	unsigned int wbo;		// Word Buffer Offset - also Special Keyword Buffer Length
	WordList &keywords = *keywordlists[0];      // Internal Commands
	WordList &keywords2 = *keywordlists[1];     // External Commands (optional)
	bool isDelayedExpansion = styler.GetPropertyInt("lexer.batch.enabledelayedexpansion") != 0; //!-add-[BatchLexerImprovement]

	// CHOICE, ECHO, GOTO, PROMPT and SET have Default Text that may contain Regular Keywords
	//   Toggling Regular Keyword Checking off improves readability
	// Other Regular Keywords and External Commands / Programs might also benefit from toggling
	//   Need a more robust algorithm to properly toggle Regular Keyword Checking
	bool continueProcessing = true;	// Used to toggle Regular Keyword Checking
	// Special Keywords are those that allow certain characters without whitespace after the command
	// Examples are: cd. cd\ md. rd. dir| dir> echo: echo. path=
	bool inString = false; // Used for processing while "" //!-add-[BatchLexerImprovement]
	// Special Keyword Buffer used to determine if the first n characters is a Keyword
	char sKeywordBuffer[10];	// Special Keyword Buffer
	bool sKeywordFound;		// Exit Special Keyword for-loop if found

	// Skip initial spaces
	while ((offset < lengthLine) && (isspacechar(lineBuffer[offset]))) {
		offset++;
	}
	// Colorize Default Text
	styler.ColourTo(startLine + offset - 1, SCE_BAT_DEFAULT);
	// Set External Command / Program Location
	cmdLoc = offset;

	// Check for Fake Label (Comment) or Real Label - return if found
	if (lineBuffer[offset] == ':') {
		if (lineBuffer[offset + 1] == ':') {
			// Colorize Fake Label (Comment) - :: is similar to REM, see http://content.techweb.com/winmag/columns/explorer/2000/21.htm
			styler.ColourTo(endPos, SCE_BAT_COMMENT);
		} else {
			// Colorize Real Label
			styler.ColourTo(endPos, SCE_BAT_LABEL);
		}
		return;
//!-start-[BatchLexerImprovement]
	// Check for Comment - return if found
	} else if (CompareNCaseInsensitive(lineBuffer+offset, "rem", 3) == 0 &&
	           isspacechar(lineBuffer[offset + 3])) {
			styler.ColourTo(endPos, SCE_BAT_COMMENT);
			return;
//!-end-[BatchLexerImprovement]
	// Check for Drive Change (Drive Change is internal command) - return if found
	} else if ((IsAlphabetic(lineBuffer[offset])) &&
		(lineBuffer[offset + 1] == ':') &&
		((isspacechar(lineBuffer[offset + 2])) ||
		(((lineBuffer[offset + 2] == '\\')) &&
		(isspacechar(lineBuffer[offset + 3]))))) {
		// Colorize Regular Keyword
		styler.ColourTo(endPos, SCE_BAT_WORD);
		return;
	}

	// Check for Hide Command (@ECHO OFF/ON)
	if (lineBuffer[offset] == '@') {
		styler.ColourTo(startLine + offset, SCE_BAT_HIDE);
		offset++;
	}
	// Skip next spaces
	while ((offset < lengthLine) && (isspacechar(lineBuffer[offset]))) {
		offset++;
	}

	// Read remainder of line word-at-a-time or remainder-of-word-at-a-time
	while (offset < lengthLine) {
		if (offset > startLine) {
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1, SCE_BAT_DEFAULT);
		}
		// Copy word from Line Buffer into Word Buffer
		wbl = 0;
		for (; offset < lengthLine && wbl < 80 &&
		        !isspacechar(lineBuffer[offset]); wbl++, offset++) {
			wordBuffer[wbl] = static_cast<char>(tolower(lineBuffer[offset]));
		}
		wordBuffer[wbl] = '\0';
		wbo = 0;

/*!-remove-[BatchLexerImprovement]
		// Check for Comment - return if found
		if (CompareCaseInsensitive(wordBuffer, "rem") == 0) {
			styler.ColourTo(endPos, SCE_BAT_COMMENT);
			return;
		}
*/
		// Check for Separator
		if (IsBSeparator(wordBuffer[0])) {
			// Check for External Command / Program
			if ((cmdLoc == offset - wbl) &&
				((wordBuffer[0] == ':') ||
				(wordBuffer[0] == '\\') ||
				(wordBuffer[0] == '.'))) {
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 1);
				// Colorize External Command / Program
				if (!keywords2) {
					styler.ColourTo(startLine + offset - 1, SCE_BAT_COMMAND);
				} else if (keywords2.InList(wordBuffer)) {
					styler.ColourTo(startLine + offset - 1, SCE_BAT_COMMAND);
				} else {
					styler.ColourTo(startLine + offset - 1, SCE_BAT_DEFAULT);
				}
				// Reset External Command / Program Location
				cmdLoc = offset;
			} else {
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 1);
				// Colorize Default Text
				styler.ColourTo(startLine + offset - 1, SCE_BAT_DEFAULT);
//!-start-[BatchLexerImprovement]
				if (wordBuffer[0] == '"')
					inString = !inString;
//!-end-[BatchLexerImprovement]
			}
//!-start-[BatchLexerImprovement]
		// Check for Labels in text (... :label)
		} else if (wordBuffer[0] == ':' && isspacechar(lineBuffer[offset - wbl - 1])) {
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1 - wbl, SCE_BAT_DEFAULT);
			// Colorize Label
			styler.ColourTo(startLine + offset - 1, SCE_BAT_CLABEL);
			// No need to Reset Offset
		// Check for SetLocal Variable (!x...!)
		} else if (isDelayedExpansion && wordBuffer[0] == '!') {
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1 - wbl, SCE_BAT_DEFAULT);
			wbo++;
			// Search to end of word for second !
			while ((wbo < wbl) &&
				(wordBuffer[wbo] != '!') &&
				(!IsBOperator(wordBuffer[wbo])) &&
				(!IsBSeparator(wordBuffer[wbo]))) {
				wbo++;
			}
			if (wordBuffer[wbo] == '!') {
				wbo++;
				// Colorize Environment Variable
				styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_EXPANSION);
			} else {
				wbo = 1;
				// Colorize Simbol
				styler.ColourTo(startLine + offset - 1 - (wbl - 1), SCE_BAT_DEFAULT);
			}
			// Check for External Command / Program
			if (cmdLoc == offset - wbl) {
				cmdLoc = offset - (wbl - wbo);
			}
			// Reset Offset to re-process remainder of word
			offset -= (wbl - wbo);
//!-end-[BatchLexerImprovement]
		// Check for Regular Keyword in list
		} else if ((keywords.InList(wordBuffer)) &&
			(!inString) && //!-add-[BatchLexerImprovement]
			(continueProcessing)) {
			// ECHO, GOTO, PROMPT and SET require no further Regular Keyword Checking
			if ((CompareCaseInsensitive(wordBuffer, "echo") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "goto") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "prompt") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "set") == 0)) {
				continueProcessing = false;
			}
			// Identify External Command / Program Location for ERRORLEVEL, and EXIST
			if ((CompareCaseInsensitive(wordBuffer, "errorlevel") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "exist") == 0)) {
				// Reset External Command / Program Location
				cmdLoc = offset;
				// Skip next spaces
				while ((cmdLoc < lengthLine) &&
					(isspacechar(lineBuffer[cmdLoc]))) {
					cmdLoc++;
				}
				// Skip comparison
				while ((cmdLoc < lengthLine) &&
					(!isspacechar(lineBuffer[cmdLoc]))) {
					cmdLoc++;
				}
				// Skip next spaces
				while ((cmdLoc < lengthLine) &&
					(isspacechar(lineBuffer[cmdLoc]))) {
					cmdLoc++;
				}
			// Identify External Command / Program Location for CALL, DO, LOADHIGH and LH
			} else if ((CompareCaseInsensitive(wordBuffer, "call") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "do") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "loadhigh") == 0) ||
				(CompareCaseInsensitive(wordBuffer, "lh") == 0)) {
				// Reset External Command / Program Location
				cmdLoc = offset;
				// Skip next spaces
				while ((cmdLoc < lengthLine) &&
					(isspacechar(lineBuffer[cmdLoc]))) {
					cmdLoc++;
				}
			}
			// Colorize Regular keyword
			styler.ColourTo(startLine + offset - 1, SCE_BAT_WORD);
			// No need to Reset Offset
		// Check for Special Keyword in list, External Command / Program, or Default Text
		} else if ((wordBuffer[0] != '%') &&
				   (wordBuffer[0] != '!') &&
			(!IsBOperator(wordBuffer[0])) &&
			(!inString) && //!-add-[BatchLexerImprovement]
			(continueProcessing)) {
			// Check for Special Keyword
			//     Affected Commands are in Length range 2-6
			//     Good that ERRORLEVEL, EXIST, CALL, DO, LOADHIGH, and LH are unaffected
			sKeywordFound = false;
			for (unsigned int keywordLength = 2; keywordLength < wbl && keywordLength < 7 && !sKeywordFound; keywordLength++) {
				wbo = 0;
				// Copy Keyword Length from Word Buffer into Special Keyword Buffer
				for (; wbo < keywordLength; wbo++) {
					sKeywordBuffer[wbo] = static_cast<char>(wordBuffer[wbo]);
				}
				sKeywordBuffer[wbo] = '\0';
				// Check for Special Keyword in list
				if ((keywords.InList(sKeywordBuffer)) &&
					((IsBOperator(wordBuffer[wbo])) ||
					(IsBSeparator(wordBuffer[wbo])))) {
					sKeywordFound = true;
					// ECHO requires no further Regular Keyword Checking
					if (CompareCaseInsensitive(sKeywordBuffer, "echo") == 0) {
						continueProcessing = false;
					}
					// Colorize Special Keyword as Regular Keyword
					styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_WORD);
					// Reset Offset to re-process remainder of word
					offset -= (wbl - wbo);
				}
			}
			// Check for External Command / Program or Default Text
			if (!sKeywordFound) {
				wbo = 0;
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					// Read up to %, Operator or Separator
					while ((wbo < wbl) &&
						(wordBuffer[wbo] != '%') &&
//!						(wordBuffer[wbo] != '!') &&
						(!isDelayedExpansion || wordBuffer[wbo] != '!') && //!-change-[BatchLexerImprovement]
						(!IsBOperator(wordBuffer[wbo])) &&
						(!IsBSeparator(wordBuffer[wbo]))) {
						wbo++;
					}
					// Reset External Command / Program Location
					cmdLoc = offset - (wbl - wbo);
					// Reset Offset to re-process remainder of word
					offset -= (wbl - wbo);
					// CHOICE requires no further Regular Keyword Checking
					if (CompareCaseInsensitive(wordBuffer, "choice") == 0) {
						continueProcessing = false;
					}
					// Check for START (and its switches) - What follows is External Command \ Program
					if (CompareCaseInsensitive(wordBuffer, "start") == 0) {
						// Reset External Command / Program Location
						cmdLoc = offset;
						// Skip next spaces
						while ((cmdLoc < lengthLine) &&
							(isspacechar(lineBuffer[cmdLoc]))) {
							cmdLoc++;
						}
						// Reset External Command / Program Location if command switch detected
						if (lineBuffer[cmdLoc] == '/') {
							// Skip command switch
							while ((cmdLoc < lengthLine) &&
								(!isspacechar(lineBuffer[cmdLoc]))) {
								cmdLoc++;
							}
							// Skip next spaces
							while ((cmdLoc < lengthLine) &&
								(isspacechar(lineBuffer[cmdLoc]))) {
								cmdLoc++;
							}
						}
					}
					// Colorize External Command / Program
					if (!keywords2) {
						styler.ColourTo(startLine + offset - 1, SCE_BAT_COMMAND);
					} else if (keywords2.InList(wordBuffer)) {
						styler.ColourTo(startLine + offset - 1, SCE_BAT_COMMAND);
					} else {
						styler.ColourTo(startLine + offset - 1, SCE_BAT_DEFAULT);
					}
					// No need to Reset Offset
				// Check for Default Text
				} else {
					// Read up to %, Operator or Separator
					while ((wbo < wbl) &&
						(wordBuffer[wbo] != '%') &&
//!						(wordBuffer[wbo] != '!') &&
						(!isDelayedExpansion || wordBuffer[wbo] != '!') && //!-change-[BatchLexerImprovement]
						(!IsBOperator(wordBuffer[wbo])) &&
						(!IsBSeparator(wordBuffer[wbo]))) {
						wbo++;
					}
					// Colorize Default Text
					styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_DEFAULT);
					// Reset Offset to re-process remainder of word
					offset -= (wbl - wbo);
				}
			}
		// Check for Argument  (%n), Environment Variable (%x...%) or Local Variable (%%a)
		} else if (wordBuffer[0] == '%') {
			unsigned int varlen; //!-add-[BatchLexerImprovement]
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1 - wbl, SCE_BAT_DEFAULT);
			wbo++;
			// Search to end of word for second % (can be a long path)
			while ((wbo < wbl) &&
				(wordBuffer[wbo] != '%') &&
				(!IsBOperator(wordBuffer[wbo])) &&
				(!IsBSeparator(wordBuffer[wbo]))) {
				wbo++;
			}
			// Check for Argument (%n) or (%*)
			if (((Is0To9(wordBuffer[1])) || (wordBuffer[1] == '*')) &&
				(wordBuffer[wbo] != '%')) {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - 2);
				}
				// Colorize Argument
				styler.ColourTo(startLine + offset - 1 - (wbl - 2), SCE_BAT_IDENTIFIER);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 2);
			// Check for Expanded Argument (%~...) / Variable (%%~...)
			} else if (((wbl > 1) && (wordBuffer[1] == '~')) ||
				((wbl > 2) && (wordBuffer[1] == '%') && (wordBuffer[2] == '~'))) {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - wbo);
				}
				// Colorize Expanded Argument / Variable
				styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_IDENTIFIER);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - wbo);
			// Check for Environment Variable (%x...%)
			} else if ((wordBuffer[1] != '%') &&
				(wordBuffer[wbo] == '%')) {
				wbo++;
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - wbo);
				}
				// Colorize Environment Variable
//!				styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_IDENTIFIER);
				styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_ENVIRONMENT); //!-change-[BatchLexerImprovement]
				// Reset Offset to re-process remainder of word
				offset -= (wbl - wbo);
//!-start-[BatchLexerImprovement]
			// Check for Variable with modifiers (%~...)
			} else if ((varlen = GetBatchVarLen(wordBuffer, wbl)) != 0) {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - varlen);
				}
				// Colorize Variable
				styler.ColourTo(startLine + offset - 1 - (wbl - varlen), SCE_BAT_IDENTIFIER);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - varlen);
			// Check for Local Variable with modifiers (%%~...)
			} else if ((wordBuffer[1] == '%') &&
				((varlen = GetBatchVarLen(wordBuffer+1, wbl-1)) != 0)) {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - varlen - 1);
				}
				// Colorize Local Variable
				styler.ColourTo(startLine + offset - 1 - (wbl - varlen - 1), SCE_BAT_IDENTIFIER);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - varlen - 1);
//!-end-[BatchLexerImprovement]
			// Check for Local Variable (%%a)
			} else if (
				(wbl > 2) &&
				(wordBuffer[1] == '%') &&
				(wordBuffer[2] != '%') &&
				(!IsBOperator(wordBuffer[2])) &&
				(!IsBSeparator(wordBuffer[2]))) {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - 3);
				}
				// Colorize Local Variable
				styler.ColourTo(startLine + offset - 1 - (wbl - 3), SCE_BAT_IDENTIFIER);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 3);
//!-start-[BatchLexerImprovement]
			// Check for %%
			} else if (
				(wbl > 1) &&
				(wordBuffer[1] == '%')) {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - 2);
				}
				// Colorize Simbols
				styler.ColourTo(startLine + offset - 1 - (wbl - 2), SCE_BAT_DEFAULT);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 2);
			} else {
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - 1);
				}
				// Colorize Simbol
				styler.ColourTo(startLine + offset - 1 - (wbl - 1), SCE_BAT_DEFAULT);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 1);
//!-end-[BatchLexerImprovement]
			}
		// Check for Environment Variable (!x...!)
		} else if (wordBuffer[0] == '!') {
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1 - wbl, SCE_BAT_DEFAULT);
			wbo++;
			// Search to end of word for second ! (can be a long path)
			while ((wbo < wbl) &&
				(wordBuffer[wbo] != '!') &&
				(!IsBOperator(wordBuffer[wbo])) &&
				(!IsBSeparator(wordBuffer[wbo]))) {
				wbo++;
			}
			if (wordBuffer[wbo] == '!') {
				wbo++;
				// Check for External Command / Program
				if (cmdLoc == offset - wbl) {
					cmdLoc = offset - (wbl - wbo);
				}
				// Colorize Environment Variable
				styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_IDENTIFIER);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - wbo);
			}
		// Check for Operator
		} else if (IsBOperator(wordBuffer[0])) {
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1 - wbl, SCE_BAT_DEFAULT);
			// Check for Comparison Operator
			if ((wordBuffer[0] == '=') && (wordBuffer[1] == '=')) {
				// Identify External Command / Program Location for IF
				cmdLoc = offset;
				// Skip next spaces
				while ((cmdLoc < lengthLine) &&
					(isspacechar(lineBuffer[cmdLoc]))) {
					cmdLoc++;
				}
				// Colorize Comparison Operator
				styler.ColourTo(startLine + offset - 1 - (wbl - 2), SCE_BAT_OPERATOR);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 2);
			// Check for Pipe Operator
			} else if (wordBuffer[0] == '|') {
				// Reset External Command / Program Location
				cmdLoc = offset - wbl + 1;
				// Skip next spaces
				while ((cmdLoc < lengthLine) &&
					(isspacechar(lineBuffer[cmdLoc]))) {
					cmdLoc++;
				}
				// Colorize Pipe Operator
				styler.ColourTo(startLine + offset - 1 - (wbl - 1), SCE_BAT_OPERATOR);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 1);
			// Check for Other Operator
			} else {
				// Check for > Operator
				if (wordBuffer[0] == '>') {
					// Turn Keyword and External Command / Program checking back on
					continueProcessing = true;
				}
				// Colorize Other Operator
				if (!inString || !(wordBuffer[0] == '(' || wordBuffer[0] == ')')) //!-add-[BatchLexerImprovement]
				styler.ColourTo(startLine + offset - 1 - (wbl - 1), SCE_BAT_OPERATOR);
				// Reset Offset to re-process remainder of word
				offset -= (wbl - 1);
			}
		// Check for Default Text
		} else {
			// Read up to %, Operator or Separator
			while ((wbo < wbl) &&
				(wordBuffer[wbo] != '%') &&
//!				(wordBuffer[wbo] != '!') &&
				(!isDelayedExpansion || wordBuffer[wbo] != '!') && //!-change-[BatchLexerImprovement]
				(!IsBOperator(wordBuffer[wbo])) &&
				(!IsBSeparator(wordBuffer[wbo]))) {
				wbo++;
			}
			// Colorize Default Text
			styler.ColourTo(startLine + offset - 1 - (wbl - wbo), SCE_BAT_DEFAULT);
			// Reset Offset to re-process remainder of word
			offset -= (wbl - wbo);
		}
		// Skip next spaces - nothing happens if Offset was Reset
		while ((offset < lengthLine) && (isspacechar(lineBuffer[offset]))) {
			offset++;
		}
	}
	// Colorize Default Text for remainder of line - currently not lexed
	styler.ColourTo(endPos, SCE_BAT_DEFAULT);
}

static void ColouriseBatchDoc(
    unsigned int startPos,
    int length,
    int /*initStyle*/,
    WordList *keywordlists[],
    Accessor &styler) {

	char lineBuffer[1024];

	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	unsigned int startLine = startPos;
	for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i];
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
			ColouriseBatchLine(lineBuffer, linePos, startLine, i, keywordlists, styler);
			linePos = 0;
			startLine = i + 1;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
		lineBuffer[linePos] = '\0';
		ColouriseBatchLine(lineBuffer, linePos, startLine, startPos + length - 1,
		                   keywordlists, styler);
	}
}
//!-start-[BatchLexerImprovement]
static void FoldBatchDoc(unsigned int startPos, int length, int,
    WordList *[], Accessor &styler)
{
	int line = styler.GetLine(startPos);
	int level = styler.LevelAt(line);
	int levelIndent = 0;
	unsigned int endPos = startPos + length;
	// Scan for ( and )
	for (unsigned int i = startPos; i < endPos; i++) {
		int c = styler.SafeGetCharAt(i, '\n');
		int style = styler.StyleAt(i);
		if (style == SCE_BAT_OPERATOR) {
			// CheckFoldPoint
			if (c == '(') {
				levelIndent += 1;
			} else
			if (c == ')') {
					levelIndent -= 1;
			}
		}
		if (c == '\n') { // line end
				if (levelIndent > 0) {
						level |= SC_FOLDLEVELHEADERFLAG;
				}
				if (level != styler.LevelAt(line))
						styler.SetLevel(line, level);
				level += levelIndent;
				if ((level & SC_FOLDLEVELNUMBERMASK) < SC_FOLDLEVELBASE)
						level = SC_FOLDLEVELBASE;
				line++;
				// reset state
				levelIndent = 0;
				level &= ~SC_FOLDLEVELHEADERFLAG;
				level &= ~SC_FOLDLEVELWHITEFLAG;
		}
	}
}
//!-end-[BatchLexerImprovement]

#define DIFF_BUFFER_START_SIZE 16
// Note that ColouriseDiffLine analyzes only the first DIFF_BUFFER_START_SIZE
// characters of each line to classify the line.

static void ColouriseDiffLine(char *lineBuffer, int endLine, Accessor &styler) {
	// It is needed to remember the current state to recognize starting
	// comment lines before the first "diff " or "--- ". If a real
	// difference starts then each line starting with ' ' is a whitespace
	// otherwise it is considered a comment (Only in..., Binary file...)
	if (0 == strncmp(lineBuffer, "diff ", 5)) {
		styler.ColourTo(endLine, SCE_DIFF_COMMAND);
	} else if (0 == strncmp(lineBuffer, "Index: ", 7)) {  // For subversion's diff
		styler.ColourTo(endLine, SCE_DIFF_COMMAND);
	} else if (0 == strncmp(lineBuffer, "---", 3) && lineBuffer[3] != '-') {
		// In a context diff, --- appears in both the header and the position markers
		if (lineBuffer[3] == ' ' && atoi(lineBuffer + 4) && !strchr(lineBuffer, '/'))
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else if (lineBuffer[3] == '\r' || lineBuffer[3] == '\n')
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else
			styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (0 == strncmp(lineBuffer, "+++ ", 4)) {
		// I don't know of any diff where "+++ " is a position marker, but for
		// consistency, do the same as with "--- " and "*** ".
		if (atoi(lineBuffer+4) && !strchr(lineBuffer, '/'))
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else
			styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (0 == strncmp(lineBuffer, "====", 4)) {  // For p4's diff
		styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (0 == strncmp(lineBuffer, "***", 3)) {
		// In a context diff, *** appears in both the header and the position markers.
		// Also ******** is a chunk header, but here it's treated as part of the
		// position marker since there is no separate style for a chunk header.
		if (lineBuffer[3] == ' ' && atoi(lineBuffer+4) && !strchr(lineBuffer, '/'))
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else if (lineBuffer[3] == '*')
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else
			styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (0 == strncmp(lineBuffer, "? ", 2)) {    // For difflib
		styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (lineBuffer[0] == '@') {
		styler.ColourTo(endLine, SCE_DIFF_POSITION);
	} else if (lineBuffer[0] >= '0' && lineBuffer[0] <= '9') {
		styler.ColourTo(endLine, SCE_DIFF_POSITION);
	} else if (lineBuffer[0] == '-' || lineBuffer[0] == '<') {
		styler.ColourTo(endLine, SCE_DIFF_DELETED);
	} else if (lineBuffer[0] == '+' || lineBuffer[0] == '>') {
		styler.ColourTo(endLine, SCE_DIFF_ADDED);
	} else if (lineBuffer[0] == '!') {
		styler.ColourTo(endLine, SCE_DIFF_CHANGED);
	} else if (lineBuffer[0] != ' ') {
		styler.ColourTo(endLine, SCE_DIFF_COMMENT);
	} else {
		styler.ColourTo(endLine, SCE_DIFF_DEFAULT);
	}
}

static void ColouriseDiffDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	char lineBuffer[DIFF_BUFFER_START_SIZE] = "";
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	for (unsigned int i = startPos; i < startPos + length; i++) {
		if (AtEOL(styler, i)) {
			if (linePos < DIFF_BUFFER_START_SIZE) {
				lineBuffer[linePos] = 0;
			}
			ColouriseDiffLine(lineBuffer, i, styler);
			linePos = 0;
		} else if (linePos < DIFF_BUFFER_START_SIZE - 1) {
			lineBuffer[linePos++] = styler[i];
		} else if (linePos == DIFF_BUFFER_START_SIZE - 1) {
			lineBuffer[linePos++] = 0;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
		if (linePos < DIFF_BUFFER_START_SIZE) {
			lineBuffer[linePos] = 0;
		}
		ColouriseDiffLine(lineBuffer, startPos + length - 1, styler);
	}
}

static void FoldDiffDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	int curLine = styler.GetLine(startPos);
	int curLineStart = styler.LineStart(curLine);
	int prevLevel = curLine > 0 ? styler.LevelAt(curLine - 1) : SC_FOLDLEVELBASE;
	int nextLevel;

	do {
		int lineType = styler.StyleAt(curLineStart);
		if (lineType == SCE_DIFF_COMMAND)
			nextLevel = SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG;
		else if (lineType == SCE_DIFF_HEADER)
			nextLevel = (SC_FOLDLEVELBASE + 1) | SC_FOLDLEVELHEADERFLAG;
		else if (lineType == SCE_DIFF_POSITION && styler[curLineStart] != '-')
			nextLevel = (SC_FOLDLEVELBASE + 2) | SC_FOLDLEVELHEADERFLAG;
		else if (prevLevel & SC_FOLDLEVELHEADERFLAG)
			nextLevel = (prevLevel & SC_FOLDLEVELNUMBERMASK) + 1;
		else
			nextLevel = prevLevel;

		if ((nextLevel & SC_FOLDLEVELHEADERFLAG) && (nextLevel == prevLevel))
			styler.SetLevel(curLine-1, prevLevel & ~SC_FOLDLEVELHEADERFLAG);

		styler.SetLevel(curLine, nextLevel);
		prevLevel = nextLevel;

		curLineStart = styler.LineStart(++curLine);
	} while (static_cast<int>(startPos) + length > curLineStart);
}

static inline bool isassignchar(unsigned char ch) {
	return (ch == '=') || (ch == ':');
}

//!-start-[PropsKeywords]
static bool isprefix(const char *target, const char *prefix) {
	while (*target && *prefix) {
		if (*target != *prefix)
			return false;
		target++;
		prefix++;
	}
	if (*prefix)
		return false;
	else
		return true;
}
//!-end-[PropsKeywords]
//!static void ColourisePropsLine(
static char ColourisePropsLine( // return last style //!-change-[PropsColouriseFix]
    char *lineBuffer,
    unsigned int lengthLine,
    unsigned int startLine,
    unsigned int endPos,
    WordList *keywordlists[], //!-add-[PropsKeysSets]
    Accessor &styler,
    bool allowInitialSpaces) {

	unsigned int i = 0;
	if (allowInitialSpaces) {
		while ((i < lengthLine) && isspacechar(lineBuffer[i]))	// Skip initial spaces
			i++;
	} else {
		if (isspacechar(lineBuffer[i])) // don't allow initial spaces
			i = lengthLine;
	}

	if (i < lengthLine) {
		if (lineBuffer[i] == '#' || lineBuffer[i] == '!' || lineBuffer[i] == ';') {
			styler.ColourTo(endPos, SCE_PROPS_COMMENT);
			return SCE_PROPS_COMMENT; //!-add-[PropsColouriseFix]
		} else if (lineBuffer[i] == '[') {
			styler.ColourTo(endPos, SCE_PROPS_SECTION);
			return SCE_PROPS_SECTION; //!-add-[PropsColouriseFix]
		} else if (lineBuffer[i] == '@') {
			styler.ColourTo(startLine + i, SCE_PROPS_DEFVAL);
			if (isassignchar(lineBuffer[i++]))
				styler.ColourTo(startLine + i, SCE_PROPS_ASSIGNMENT);
			styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
//!-start-[PropsKeywords]
		} else if (isprefix(lineBuffer, "import ")) {
			styler.ColourTo(startLine + 6, SCE_PROPS_KEYWORD);
			styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
		} else if (isprefix(lineBuffer, "if ")) {
			styler.ColourTo(startLine + 2, SCE_PROPS_KEYWORD);
			styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
//!-end-[PropsKeywords]
		} else {
			// Search for the '=' character
			while ((i < lengthLine) && !isassignchar(lineBuffer[i]))
				i++;
			if ((i < lengthLine) && isassignchar(lineBuffer[i])) {
//!				styler.ColourTo(startLine + i - 1, SCE_PROPS_KEY);
//!-start-[PropsKeysSets]
				if (i > 0) {
					int chAttr;
					lineBuffer[i] = '\0';
					// remove trailing spaces
					int indent = 0;
					while (lineBuffer[0] == ' ' || lineBuffer[0] == '\t') {
						lineBuffer++;
						indent++;
					}
					int len=0, fin=0;
					if ((*keywordlists[0]).InListPartly(lineBuffer, '~', len, fin)) {
						chAttr = SCE_PROPS_KEYSSET0; 
					}
					else if ((*keywordlists[1]).InListPartly(lineBuffer, '~', len, fin)) {
						chAttr = SCE_PROPS_KEYSSET1;
					}
					else if ((*keywordlists[2]).InListPartly(lineBuffer, '~', len, fin)) {
						chAttr = SCE_PROPS_KEYSSET2;
					}
					else if ((*keywordlists[3]).InListPartly(lineBuffer, '~', len, fin)) {
						chAttr = SCE_PROPS_KEYSSET3;
					} else {
						chAttr = SCE_PROPS_KEY;
					}
					styler.ColourTo(startLine + indent + len, chAttr);
					styler.ColourTo(startLine + i - 1 - fin, SCE_PROPS_KEY);
					styler.ColourTo(startLine + i - 1, chAttr);
				}
//!-end-[PropsKeysSets]
				styler.ColourTo(startLine + i, SCE_PROPS_ASSIGNMENT);
				styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
			} else {
				styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
			}
		}
	} else {
		styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
	}
	return SCE_PROPS_DEFAULT; //!-add-[PropsColouriseFix]
}

//!static void ColourisePropsDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
static void ColourisePropsDoc(unsigned int startPos, int length, int, WordList *keywordlists[], Accessor &styler) { //!-change-[PropsKeysSets]
	char lineBuffer[1024];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	unsigned int startLine = startPos;

	// property lexer.props.allow.initial.spaces
	//	For properties files, set to 0 to style all lines that start with whitespace in the default style.
	//	This is not suitable for SciTE .properties files which use indentation for flow control but
	//	can be used for RFC2822 text where indentation is used for continuation lines.
	bool allowInitialSpaces = styler.GetPropertyInt("lexer.props.allow.initial.spaces", 1) != 0;
//!-start-[PropsColouriseFix]
	char style = 0;
	bool continuation = false;
	if (startPos >= 3)
		continuation = styler.StyleAt(startPos-2) != SCE_PROPS_COMMENT && ((styler[startPos-2] == '\\')
			|| (styler[startPos-3] == '\\' && styler[startPos-2] == '\r'));
//!-end-[PropsColouriseFix]

	for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i];
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
//!			ColourisePropsLine(lineBuffer, linePos, startLine, i, styler, allowInitialSpaces);
//!-start-[PropsKeysSets]
			if (continuation)
				styler.ColourTo(i, SCE_PROPS_DEFAULT);
			else
			style = ColourisePropsLine(lineBuffer, linePos, startLine, i, keywordlists, styler, allowInitialSpaces);
			// test: is next a continuation of line
			continuation = (linePos >= sizeof(lineBuffer) - 1) ||
				(style != SCE_PROPS_COMMENT &&  ((lineBuffer[linePos-2] == '\\')
				|| (lineBuffer[linePos-3] == '\\' && lineBuffer[linePos-2] == '\r')));
//!-end-[PropsKeysSets]
			linePos = 0;
			startLine = i + 1;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
//!-start-[PropsColouriseFix]
		if (continuation)
			styler.ColourTo(startPos + length - 1, SCE_PROPS_DEFAULT);
		else
//!-end-[PropsColouriseFix]
//!		ColourisePropsLine(lineBuffer, linePos, startLine, startPos + length - 1, styler, allowInitialSpaces);
		ColourisePropsLine(lineBuffer, linePos, startLine, startPos + length - 1, keywordlists, styler, allowInitialSpaces); //!-change-[PropsKeysSets]
	}
}

// adaption by ksc, using the "} else {" trick of 1.53
// 030721
static void FoldPropsDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;

	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);

	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	bool headerPoint = false;
	int lev;

	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler[i+1];

		int style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (style == SCE_PROPS_SECTION) {
			headerPoint = true;
		}

		if (atEOL) {
			lev = SC_FOLDLEVELBASE;

			if (lineCurrent > 0) {
				int levelPrevious = styler.LevelAt(lineCurrent - 1);

				if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
					lev = SC_FOLDLEVELBASE + 1;
				} else {
					lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
				}
			}

			if (headerPoint) {
				lev = SC_FOLDLEVELBASE;
			}
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;

			if (headerPoint) {
				lev |= SC_FOLDLEVELHEADERFLAG;
			}
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}

			lineCurrent++;
			visibleChars = 0;
			headerPoint = false;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}

	if (lineCurrent > 0) {
		int levelPrevious = styler.LevelAt(lineCurrent - 1);
		if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
			lev = SC_FOLDLEVELBASE + 1;
		} else {
			lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
		}
	} else {
		lev = SC_FOLDLEVELBASE;
	}
	int flagsNext = styler.LevelAt(lineCurrent);
	styler.SetLevel(lineCurrent, lev | (flagsNext & ~SC_FOLDLEVELNUMBERMASK));
}

static void ColouriseMakeLine(
    char *lineBuffer,
    unsigned int lengthLine,
    unsigned int startLine,
    unsigned int endPos,
    Accessor &styler) {

	unsigned int i = 0;
	int lastNonSpace = -1;
	unsigned int state = SCE_MAKE_DEFAULT;
	bool bSpecial = false;

	// check for a tab character in column 0 indicating a command
	bool bCommand = false;
	if ((lengthLine > 0) && (lineBuffer[0] == '\t'))
		bCommand = true;

	// Skip initial spaces
	while ((i < lengthLine) && isspacechar(lineBuffer[i])) {
		i++;
	}
	if (i < lengthLine) {
		if (lineBuffer[i] == '#') {	// Comment
			styler.ColourTo(endPos, SCE_MAKE_COMMENT);
			return;
		}
		if (lineBuffer[i] == '!') {	// Special directive
			styler.ColourTo(endPos, SCE_MAKE_PREPROCESSOR);
			return;
		}
	}
	int varCount = 0;
	while (i < lengthLine) {
		if (((i + 1) < lengthLine) && (lineBuffer[i] == '$' && lineBuffer[i + 1] == '(')) {
			styler.ColourTo(startLine + i - 1, state);
			state = SCE_MAKE_IDENTIFIER;
			varCount++;
		} else if (state == SCE_MAKE_IDENTIFIER && lineBuffer[i] == ')') {
			if (--varCount == 0) {
				styler.ColourTo(startLine + i, state);
				state = SCE_MAKE_DEFAULT;
			}
		}

		// skip identifier and target styling if this is a command line
		if (!bSpecial && !bCommand) {
			if (lineBuffer[i] == ':') {
				if (((i + 1) < lengthLine) && (lineBuffer[i + 1] == '=')) {
					// it's a ':=', so style as an identifier
					if (lastNonSpace >= 0)
						styler.ColourTo(startLine + lastNonSpace, SCE_MAKE_IDENTIFIER);
					styler.ColourTo(startLine + i - 1, SCE_MAKE_DEFAULT);
					styler.ColourTo(startLine + i + 1, SCE_MAKE_OPERATOR);
				} else {
					// We should check that no colouring was made since the beginning of the line,
					// to avoid colouring stuff like /OUT:file
					if (lastNonSpace >= 0)
						styler.ColourTo(startLine + lastNonSpace, SCE_MAKE_TARGET);
					styler.ColourTo(startLine + i - 1, SCE_MAKE_DEFAULT);
					styler.ColourTo(startLine + i, SCE_MAKE_OPERATOR);
				}
				bSpecial = true;	// Only react to the first ':' of the line
				state = SCE_MAKE_DEFAULT;
			} else if (lineBuffer[i] == '=') {
				if (lastNonSpace >= 0)
					styler.ColourTo(startLine + lastNonSpace, SCE_MAKE_IDENTIFIER);
				styler.ColourTo(startLine + i - 1, SCE_MAKE_DEFAULT);
				styler.ColourTo(startLine + i, SCE_MAKE_OPERATOR);
				bSpecial = true;	// Only react to the first '=' of the line
				state = SCE_MAKE_DEFAULT;
			}
		}
		if (!isspacechar(lineBuffer[i])) {
			lastNonSpace = i;
		}
		i++;
	}
	if (state == SCE_MAKE_IDENTIFIER) {
		styler.ColourTo(endPos, SCE_MAKE_IDEOL);	// Error, variable reference not ended
	} else {
		styler.ColourTo(endPos, SCE_MAKE_DEFAULT);
	}
}

static void ColouriseMakeDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	char lineBuffer[1024];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	unsigned int startLine = startPos;
	for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i];
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
			ColouriseMakeLine(lineBuffer, linePos, startLine, i, styler);
			linePos = 0;
			startLine = i + 1;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
		ColouriseMakeLine(lineBuffer, linePos, startLine, startPos + length - 1, styler);
	}
}

static int RecogniseErrorListLine(const char *lineBuffer, unsigned int lengthLine, int &startValue) {
	if (lineBuffer[0] == '>') {
		// Command or return status
		return SCE_ERR_CMD;
	} else if (strstr(lineBuffer, "<<<!") &&
		strstr(lineBuffer, "!>>>") &&
		strstr(lineBuffer, "<<<!") < strstr(lineBuffer, "!>>>")) {
		startValue = (int)(strstr(lineBuffer, "<<<!") - lineBuffer);
		return SCE_ERR_ATRIUM_WARNING;
	} else if (lineBuffer[0] == '<') {
		// Diff removal.
		return SCE_ERR_DIFF_DELETION;
	} else if (lineBuffer[0] == '!') {
		return SCE_ERR_DIFF_CHANGED;
	} else if (lineBuffer[0] == '+') {
		if (strstart(lineBuffer, "+++ ")) {
			return SCE_ERR_DIFF_MESSAGE;
		} else {
			return SCE_ERR_DIFF_ADDITION;
		}
	} else if (lineBuffer[0] == '-') {
		if (strstart(lineBuffer, "--- ")) {
			return SCE_ERR_DIFF_MESSAGE;
		} else {
			return SCE_ERR_DIFF_DELETION;
		}
	} else if (strstart(lineBuffer, "cf90-")) {
		// Absoft Pro Fortran 90/95 v8.2 error and/or warning message
		return SCE_ERR_ABSF;
	} else if (strstart(lineBuffer, "fortcom:")) {
		// Intel Fortran Compiler v8.0 error/warning message
		return SCE_ERR_IFORT;
	} else if (strstr(lineBuffer, "File \"") && strstr(lineBuffer, ", line ")) {
		return SCE_ERR_PYTHON;
	} else if (strstr(lineBuffer, " in ") && strstr(lineBuffer, " on line ")) {
		return SCE_ERR_PHP;
	} else if ((strstart(lineBuffer, "Error ") ||
		strstart(lineBuffer, "Warning ")) &&
		strstr(lineBuffer, " at (") &&
		strstr(lineBuffer, ") : ") &&
		(strstr(lineBuffer, " at (") < strstr(lineBuffer, ") : "))) {
		// Intel Fortran Compiler error/warning message
		return SCE_ERR_IFC;
	} else if (strstart(lineBuffer, "Error ")) {
		// Borland error message
		return SCE_ERR_BORLAND;
	} else if (strstart(lineBuffer, "Warning ")) {
		// Borland warning message
		return SCE_ERR_BORLAND;
	} else if (strstr(lineBuffer, "at line ") &&
	        (strstr(lineBuffer, "at line ") < (lineBuffer + lengthLine)) &&
	           strstr(lineBuffer, "file ") &&
	           (strstr(lineBuffer, "file ") < (lineBuffer + lengthLine))) {
		// Lua 4 error message
		return SCE_ERR_LUA;
	} else if (strstr(lineBuffer, " at ") &&
	        (strstr(lineBuffer, " at ") < (lineBuffer + lengthLine)) &&
	           strstr(lineBuffer, " line ") &&
	           (strstr(lineBuffer, " line ") < (lineBuffer + lengthLine)) &&
	        (strstr(lineBuffer, " at ") + 4 < (strstr(lineBuffer, " line ")))) {
		// perl error message:
		// <message> at <file> line <line>
		return SCE_ERR_PERL;
	} else if ((memcmp(lineBuffer, "   at ", 6) == 0) &&
	           strstr(lineBuffer, ":line ")) {
		// A .NET traceback
		return SCE_ERR_NET;
	} else if (strstart(lineBuffer, "Line ") &&
	           strstr(lineBuffer, ", file ")) {
		// Essential Lahey Fortran error message
		return SCE_ERR_ELF;
	} else if (strstart(lineBuffer, "line ") &&
	           strstr(lineBuffer, " column ")) {
		// HTML tidy style: line 42 column 1
		return SCE_ERR_TIDY;
	} else if (strstart(lineBuffer, "\tat ") &&
	           strstr(lineBuffer, "(") &&
	           strstr(lineBuffer, ".java:")) {
		// Java stack back trace
		return SCE_ERR_JAVA_STACK;
	} else if (strstart(lineBuffer, "In file included from ") ||
	           strstart(lineBuffer, "                 from ")) {
		// GCC showing include path to following error
		return SCE_ERR_GCC_INCLUDED_FROM;
	} else {
		// Look for one of the following formats:
		// GCC: <filename>:<line>:<message>
		// Microsoft: <filename>(<line>) :<message>
		// Common: <filename>(<line>): warning|error|note|remark|catastrophic|fatal
		// Common: <filename>(<line>) warning|error|note|remark|catastrophic|fatal
		// Microsoft: <filename>(<line>,<column>)<message>
		// CTags: <identifier>\t<filename>\t<message>
		// Lua 5 traceback: \t<filename>:<line>:<message>
		// Lua 5.1: <exe>: <filename>:<line>:<message>
		bool initialTab = (lineBuffer[0] == '\t');
		bool initialColonPart = false;
		bool canBeCtags = !initialTab;	// For ctags must have an identifier with no spaces then a tab
		enum { stInitial,
			stGccStart, stGccDigit, stGccColumn, stGcc,
			stMsStart, stMsDigit, stMsBracket, stMsVc, stMsDigitComma, stMsDotNet,
			stCtagsStart, stCtagsFile, stCtagsStartString, stCtagsStringDollar, stCtags,
			stUnrecognized
		} state = stInitial;
		for (unsigned int i = 0; i < lengthLine; i++) {
			char ch = lineBuffer[i];
			char chNext = ' ';
			if ((i + 1) < lengthLine)
				chNext = lineBuffer[i + 1];
			if (state == stInitial) {
				if (ch == ':') {
					// May be GCC, or might be Lua 5 (Lua traceback same but with tab prefix)
					if ((chNext != '\\') && (chNext != '/') && (chNext != ' ')) {
						// This check is not completely accurate as may be on
						// GTK+ with a file name that includes ':'.
						state = stGccStart;
					} else if (chNext == ' ') { // indicates a Lua 5.1 error message
						initialColonPart = true;
					}
				} else if ((ch == '(') && Is1To9(chNext) && (!initialTab)) {
					// May be Microsoft
					// Check against '0' often removes phone numbers
					state = stMsStart;
				} else if ((ch == '\t') && canBeCtags) {
					// May be CTags
					state = stCtagsStart;
				} else if (ch == ' ') {
					canBeCtags = false;
				}
			} else if (state == stGccStart) {	// <filename>:
				state = Is1To9(ch) ? stGccDigit : stUnrecognized;
			} else if (state == stGccDigit) {	// <filename>:<line>
				if (ch == ':') {
					state = stGccColumn;	// :9.*: is GCC
					startValue = i + 1;
				} else if (!Is0To9(ch)) {
					state = stUnrecognized;
				}
			} else if (state == stGccColumn) {	// <filename>:<line>:<column>
				if (!Is0To9(ch)) {
					state = stGcc;
					if (ch == ':')
						startValue = i + 1;
					break;
				}
			} else if (state == stMsStart) {	// <filename>(
				state = Is0To9(ch) ? stMsDigit : stUnrecognized;
			} else if (state == stMsDigit) {	// <filename>(<line>
				if (ch == ',') {
					state = stMsDigitComma;
				} else if (ch == ')') {
					state = stMsBracket;
				} else if ((ch != ' ') && !Is0To9(ch)) {
					state = stUnrecognized;
				}
			} else if (state == stMsBracket) {	// <filename>(<line>)
				if ((ch == ' ') && (chNext == ':')) {
					state = stMsVc;
				} else if ((ch == ':' && chNext == ' ') || (ch == ' ')) {
					// Possibly Delphi.. don't test against chNext as it's one of the strings below.
					char word[512];
					unsigned int j, chPos;
					unsigned numstep;
					chPos = 0;
					if (ch == ' ')
						numstep = 1; // ch was ' ', handle as if it's a delphi errorline, only add 1 to i.
					else
						numstep = 2; // otherwise add 2.
					for (j = i + numstep; j < lengthLine && IsAlphabetic(lineBuffer[j]) && chPos < sizeof(word) - 1; j++)
						word[chPos++] = lineBuffer[j];
					word[chPos] = 0;
					if (!CompareCaseInsensitive(word, "error") || !CompareCaseInsensitive(word, "warning") ||
						!CompareCaseInsensitive(word, "fatal") || !CompareCaseInsensitive(word, "catastrophic") ||
						!CompareCaseInsensitive(word, "note") || !CompareCaseInsensitive(word, "remark")) {
						state = stMsVc;
					} else {
						state = stUnrecognized;
					}
				} else {
					state = stUnrecognized;
				}
			} else if (state == stMsDigitComma) {	// <filename>(<line>,
				if (ch == ')') {
					state = stMsDotNet;
					break;
				} else if ((ch != ' ') && !Is0To9(ch)) {
					state = stUnrecognized;
				}
			} else if (state == stCtagsStart) {
				if (ch == '\t') {
					state = stCtagsFile;
				}
			} else if (state == stCtagsFile) {
				if ((lineBuffer[i - 1] == '\t') &&
				        ((ch == '/' && chNext == '^') || Is0To9(ch))) {
					state = stCtags;
					break;
				} else if ((ch == '/') && (chNext == '^')) {
					state = stCtagsStartString;
				}
			} else if ((state == stCtagsStartString) && ((lineBuffer[i] == '$') && (lineBuffer[i + 1] == '/'))) {
				state = stCtagsStringDollar;
				break;
			}
		}
		if (state == stGcc) {
			return initialColonPart ? SCE_ERR_LUA : SCE_ERR_GCC;
		} else if ((state == stMsVc) || (state == stMsDotNet)) {
			return SCE_ERR_MS;
		} else if ((state == stCtagsStringDollar) || (state == stCtags)) {
			return SCE_ERR_CTAG;
		} else {
			return SCE_ERR_DEFAULT;
		}
	}
}
//!-start-[FindResultListStyle]
// Find part of the string lineBuffer beetwen substrings beginT and endT
// write results substring to the findValue
static bool GetPartOf(const char *lineBuffer,
		const char *beginT, const char *endT, char *findValue, unsigned int maxLen) {
	if (strstart(lineBuffer, beginT)) {
		const char *buff = lineBuffer + strlen(beginT);
		unsigned int len = strlen(endT);
		unsigned int p = 0;
		while (0 != strncmp(buff + p, endT, len)) {
			p++;
			if (p > maxLen) return false;
		}
		strncpy(findValue, buff, p);
		findValue[p] = '\0';
		return true;
	}
	return false;
}

static void ColouriseFindListLine(
		const char *lineBuffer,
		unsigned int lengthLine,
		unsigned int startPos,
		unsigned int endPos,
		char *findValue,
		Accessor &styler) {
	int len = strlen(findValue);
	if (len > 0) {
		unsigned int p = 0;
		while (p < lengthLine - len) {
			if (0 == CompareNCaseInsensitive(lineBuffer + p, findValue, len)) {
				styler.ColourTo(startPos + p, SCE_ERR_VALUE);
				styler.ColourTo(startPos + p + len, SCE_ERR_FIND_VALUE);
				p += len - 1;
			}
			p++;
		}
	}
	styler.ColourTo(endPos, SCE_ERR_VALUE);
}
//!-end-[FindResultListStyle]
static void ColouriseErrorListLine(
    char *lineBuffer,
    unsigned int lengthLine,
    unsigned int endPos,
//!-start-[FindResultListStyle]
    const char *findTitleB,
    const char *findTitleE,
//!-end-[FindResultListStyle]
    Accessor &styler,
	bool valueSeparate) {
//!-start-[FindResultListStyle]
	static bool isFindList;
	static char findValue[1000];
//!-end-[FindResultListStyle]
	int startValue = -1;
	int style = RecogniseErrorListLine(lineBuffer, lengthLine, startValue);
	if (valueSeparate && (startValue >= 0)) {
		if (style == SCE_ERR_ATRIUM_WARNING) {
			styler.ColourTo(endPos - (lengthLine - startValue), SCE_ERR_DEFAULT);
			startValue = (int)(strstr(lineBuffer, "!>>>") - lineBuffer) + 4;
			styler.ColourTo(endPos - (lengthLine - startValue), style);
		} else {
			styler.ColourTo(endPos - (lengthLine - startValue), style);
			//!-start-[FindResultListStyle]
			if (isFindList) {
				ColouriseFindListLine(lineBuffer + startValue, lengthLine - startValue + 1, endPos - lengthLine + startValue, endPos, findValue, styler);
			} else
				//!-end-[FindResultListStyle]
				styler.ColourTo(endPos, SCE_ERR_VALUE);
		}
	} else {
//!-start-[FindResultListStyle]
		if (valueSeparate && style == SCE_ERR_CMD) {
			isFindList = GetPartOf(lineBuffer, ">Internal search for \"", "\" in \"", findValue, 1000);
			if (!isFindList && findTitleB)
				isFindList = GetPartOf(lineBuffer, findTitleB, findTitleE, findValue, 1000);
			if (!isFindList) findValue[0] = '\0';
		}
//!-end-[FindResultListStyle]
		styler.ColourTo(endPos, style);
	}
}

static void ColouriseErrorListDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	char lineBuffer[10000];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;

	// property lexer.errorlist.value.separate
	//	For lines in the output pane that are matches from Find in Files or GCC-style
	//	diagnostics, style the path and line number separately from the rest of the
	//	line with style 21 used for the rest of the line.
	//	This allows matched text to be more easily distinguished from its location.
//!	bool valueSeparate = styler.GetPropertyInt("lexer.errorlist.value.separate", 0) != 0;
//!-start-[FindResultListStyle]

	// property lexer.errorlist.value.separate
	//	For lines in the output pane that are matches from Find in Files or GCC-style
	//	diagnostics, style the path and line number separately from the rest of the
	//	line with style 21 used for the rest of the line.
	//	This allows matched text to be more easily distinguished from its location.
	bool valueSeparate = styler.GetPropertyInt("lexer.errorlist.value.separate", 1) > 0;
	const char *findTitleB = styler.pprops->Get("lexer.errorlist.findtitle.begin");
	const char *findTitleE = styler.pprops->Get("lexer.errorlist.findtitle.end");
//!-end-[FindResultListStyle]
	for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i];
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
//!			ColouriseErrorListLine(lineBuffer, linePos, i, styler, valueSeparate);
			ColouriseErrorListLine(lineBuffer, linePos, i, findTitleB, findTitleE, styler, valueSeparate); //!-change-[FindResultListStyle]
			linePos = 0;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
//!		ColouriseErrorListLine(lineBuffer, linePos, startPos + length - 1, styler, valueSeparate);
		ColouriseErrorListLine(lineBuffer, linePos, startPos + length - 1, findTitleB, findTitleE, styler, valueSeparate); //!-change-[FindResultListStyle]
	}
}

static bool latexIsSpecial(int ch) {
	return (ch == '#') || (ch == '$') || (ch == '%') || (ch == '&') || (ch == '_') ||
	       (ch == '{') || (ch == '}') || (ch == ' ');
}

static bool latexIsBlank(int ch) {
	return (ch == ' ') || (ch == '\t');
}

static bool latexIsBlankAndNL(int ch) {
	return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
}

static bool latexIsLetter(int ch) {
	return isascii(ch) && isalpha(ch);
}

static bool latexIsTagValid(int &i, int l, Accessor &styler) {
	while (i < l) {
		if (styler.SafeGetCharAt(i) == '{') {
			while (i < l) {
				i++;
				if (styler.SafeGetCharAt(i) == '}') {
					return true;
				}	else if (!latexIsLetter(styler.SafeGetCharAt(i)) &&
                   styler.SafeGetCharAt(i)!='*') {
					return false;
				}
			}
		} else if (!latexIsBlank(styler.SafeGetCharAt(i))) {
			return false;
		}
		i++;
	}
	return false;
}

static bool latexNextNotBlankIs(int i, int l, Accessor &styler, char needle) {
  char ch;
	while (i < l) {
    ch = styler.SafeGetCharAt(i);
		if (!latexIsBlankAndNL(ch) && ch != '*') {
      if (ch == needle)
        return true;
      else
        return false;
		}
		i++;
	}
	return false;
}

static bool latexLastWordIs(int start, Accessor &styler, const char *needle) {
  unsigned int i = 0;
	unsigned int l = static_cast<unsigned int>(strlen(needle));
	int ini = start-l+1;
	char s[32];

	while (i < l && i < 32) {
		s[i] = styler.SafeGetCharAt(ini + i);
    i++;
	}
	s[i] = '\0';

	return (strcmp(s, needle) == 0);
}

static void ColouriseLatexDoc(unsigned int startPos, int length, int initStyle,
                              WordList *[], Accessor &styler) {

	styler.StartAt(startPos);

	int state = initStyle;
	char chNext = styler.SafeGetCharAt(startPos);
	styler.StartSegment(startPos);
	int lengthDoc = startPos + length;
  char chVerbatimDelim = '\0';

	for (int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if (styler.IsLeadByte(ch)) {
			i++;
			chNext = styler.SafeGetCharAt(i + 1);
			continue;
		}

		switch (state) {
		case SCE_L_DEFAULT :
			switch (ch) {
			case '\\' :
				styler.ColourTo(i - 1, state);
				if (latexIsSpecial(chNext)) {
					state = SCE_L_SPECIAL;
				} else {
					if (latexIsLetter(chNext)) {
						state = SCE_L_COMMAND;
					}	else {
						if (chNext == '(' || chNext == '[') {
							styler.ColourTo(i-1, state);
							styler.ColourTo(i+1, SCE_L_SHORTCMD);
							state = SCE_L_MATH;
							if (chNext == '[')
								state = SCE_L_MATH2;
							i++;
							chNext = styler.SafeGetCharAt(i+1);
						} else {
							state = SCE_L_SHORTCMD;
						}
					}
				}
				break;
			case '$' :
				styler.ColourTo(i - 1, state);
				state = SCE_L_MATH;
				if (chNext == '$') {
					state = SCE_L_MATH2;
					i++;
					chNext = styler.SafeGetCharAt(i + 1);
				}
				break;
			case '%' :
				styler.ColourTo(i - 1, state);
				state = SCE_L_COMMENT;
				break;
			}
			break;
		case SCE_L_ERROR:
			styler.ColourTo(i-1, state);
			state = SCE_L_DEFAULT;
			break;
		case SCE_L_SPECIAL:
		case SCE_L_SHORTCMD:
			styler.ColourTo(i, state);
			state = SCE_L_DEFAULT;
			break;
		case SCE_L_COMMAND :
			if (!latexIsLetter(chNext)) {
				styler.ColourTo(i, state);
				state = SCE_L_DEFAULT;
        if (latexNextNotBlankIs(i+1, lengthDoc, styler, '[' )) {
          state = SCE_L_CMDOPT;
				} else if (latexLastWordIs(i, styler, "\\begin")) {
					state = SCE_L_TAG;
				} else if (latexLastWordIs(i, styler, "\\end")) {
					state = SCE_L_TAG2;
				} else if (latexLastWordIs(i, styler, "\\verb") &&
                   chNext != '*' && chNext != ' ') {
          chVerbatimDelim = chNext;
					state = SCE_L_VERBATIM;
				}
			}
			break;
		case SCE_L_CMDOPT :
      if (ch == ']') {
        styler.ColourTo(i, state);
        state = SCE_L_DEFAULT;
      }
			break;
		case SCE_L_TAG :
			if (latexIsTagValid(i, lengthDoc, styler)) {
				styler.ColourTo(i, state);
				state = SCE_L_DEFAULT;
				if (latexLastWordIs(i, styler, "{verbatim}")) {
					state = SCE_L_VERBATIM;
				} else if (latexLastWordIs(i, styler, "{comment}")) {
					state = SCE_L_COMMENT2;
				} else if (latexLastWordIs(i, styler, "{math}")) {
					state = SCE_L_MATH;
				} else if (latexLastWordIs(i, styler, "{displaymath}")) {
					state = SCE_L_MATH2;
				} else if (latexLastWordIs(i, styler, "{equation}")) {
					state = SCE_L_MATH2;
				}
			} else {
				state = SCE_L_ERROR;
				styler.ColourTo(i, state);
				state = SCE_L_DEFAULT;
			}
			chNext = styler.SafeGetCharAt(i+1);
			break;
		case SCE_L_TAG2 :
			if (latexIsTagValid(i, lengthDoc, styler)) {
				styler.ColourTo(i, state);
				state = SCE_L_DEFAULT;
			} else {
				state = SCE_L_ERROR;
			}
			chNext = styler.SafeGetCharAt(i+1);
			break;
		case SCE_L_MATH :
			if (ch == '$') {
				styler.ColourTo(i, state);
				state = SCE_L_DEFAULT;
			} else if (ch == '\\' && chNext == ')') {
				styler.ColourTo(i-1, state);
				styler.ColourTo(i+1, SCE_L_SHORTCMD);
				i++;
				chNext = styler.SafeGetCharAt(i+1);
				state = SCE_L_DEFAULT;
			} else if (ch == '\\') {
				int match = i + 3;
				if (latexLastWordIs(match, styler, "\\end")) {
					match++;
					if (latexIsTagValid(match, lengthDoc, styler)) {
						if (latexLastWordIs(match, styler, "{math}")) {
							styler.ColourTo(i-1, state);
							state = SCE_L_COMMAND;
						}
					}
				}
			}

			break;
		case SCE_L_MATH2 :
			if (ch == '$') {
        if (chNext == '$') {
          i++;
          chNext = styler.SafeGetCharAt(i + 1);
          styler.ColourTo(i, state);
          state = SCE_L_DEFAULT;
        } else {
          styler.ColourTo(i, SCE_L_ERROR);
          state = SCE_L_DEFAULT;
        }
			} else if (ch == '\\' && chNext == ']') {
				styler.ColourTo(i-1, state);
				styler.ColourTo(i+1, SCE_L_SHORTCMD);
				i++;
				chNext = styler.SafeGetCharAt(i+1);
				state = SCE_L_DEFAULT;
			} else if (ch == '\\') {
				int match = i + 3;
				if (latexLastWordIs(match, styler, "\\end")) {
					match++;
					if (latexIsTagValid(match, lengthDoc, styler)) {
						if (latexLastWordIs(match, styler, "{displaymath}")) {
							styler.ColourTo(i-1, state);
							state = SCE_L_COMMAND;
						} else if (latexLastWordIs(match, styler, "{equation}")) {
							styler.ColourTo(i-1, state);
							state = SCE_L_COMMAND;
						}
					}
				}
			}
			break;
		case SCE_L_COMMENT :
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_L_DEFAULT;
			}
			break;
		case SCE_L_COMMENT2 :
			if (ch == '\\') {
				int match = i + 3;
				if (latexLastWordIs(match, styler, "\\end")) {
					match++;
					if (latexIsTagValid(match, lengthDoc, styler)) {
						if (latexLastWordIs(match, styler, "{comment}")) {
							styler.ColourTo(i-1, state);
							state = SCE_L_COMMAND;
						}
					}
				}
			}
			break;
		case SCE_L_VERBATIM :
			if (ch == '\\') {
				int match = i + 3;
				if (latexLastWordIs(match, styler, "\\end")) {
					match++;
					if (latexIsTagValid(match, lengthDoc, styler)) {
						if (latexLastWordIs(match, styler, "{verbatim}")) {
							styler.ColourTo(i-1, state);
							state = SCE_L_COMMAND;
						}
					}
				}
			} else if (chNext == chVerbatimDelim) {
        styler.ColourTo(i+1, state);
				state = SCE_L_DEFAULT;
        chVerbatimDelim = '\0';
      } else if (chVerbatimDelim != '\0' && (ch == '\n' || ch == '\r')) {
        styler.ColourTo(i, SCE_L_ERROR);
				state = SCE_L_DEFAULT;
        chVerbatimDelim = '\0';
      }
			break;
		}
	}
	styler.ColourTo(lengthDoc-1, state);
}

static const char *const batchWordListDesc[] = {
	"Internal Commands",
	"External Commands",
	0
};
//!-start-[PropsKeysSets]
static const char * const propsWordListDesc[] = {
	"Keys set 0",
	"Keys set 1",
	"Keys set 2",
	"Keys set 3",
	0
};
//!-end-[PropsKeysSets]

static const char *const emptyWordListDesc[] = {
	0
};

static void ColouriseNullDoc(unsigned int startPos, int length, int, WordList *[],
                            Accessor &styler) {
	// Null language means all style bytes are 0 so just mark the end - no need to fill in.
	if (length > 0) {
		styler.StartAt(startPos + length - 1);
		styler.StartSegment(startPos + length - 1);
		styler.ColourTo(startPos + length - 1, 0);
	}
}

//!LexerModule lmBatch(SCLEX_BATCH, ColouriseBatchDoc, "batch", 0, batchWordListDesc);
//LexerModule lmBatch(SCLEX_BATCH, ColouriseBatchDoc, "batch", FoldBatchDoc, batchWordListDesc); //!-change-[BatchLexerImprovement]
//LexerModule lmDiff(SCLEX_DIFF, ColouriseDiffDoc, "diff", FoldDiffDoc, emptyWordListDesc);
//!LexerModule lmProps(SCLEX_PROPERTIES, ColourisePropsDoc, "props", FoldPropsDoc, emptyWordListDesc);
//LexerModule lmProps(SCLEX_PROPERTIES, ColourisePropsDoc, "props", FoldPropsDoc, propsWordListDesc); //!-change-[PropsKeysSets]
//LexerModule lmMakeOters(SCLEX_MAKEFILE, ColouriseMakeDoc, "makefile", 0, emptyWordListDesc);
//LexerModule lmErrorList(SCLEX_ERRORLIST, ColouriseErrorListDoc, "errorlist", 0, emptyWordListDesc);
//LexerModule lmNull(SCLEX_NULL, ColouriseNullDoc, "null");
