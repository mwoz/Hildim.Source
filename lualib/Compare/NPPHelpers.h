/*
This file is part of Compare plugin for Notepad++
Copyright (C)2011 Jean-Sébastien Leroy (jean.sebastien.leroy@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NPPHELPERS_H
#define NPPHELPERS_H

#include "Scintilla.h"
//#include "ScintillaMessages.h"
#include "ScintillaTypes.h"
#include "ScintillaStructures.h"
#include "ILoader.h"
#include "ScintillaCall.h"
#include "Compare.h"
#include <set>

struct ColorSettings
{
	int added;
	int removed;
	int changed;
	int moved;
	int blank;
	int _default;
	int add_highlight;
	int rem_highlight;
	int highlight_transparency;
	int caret_line_transparency;
};
struct UserSettings
{

	bool			FirstFileIsNew;
	int				NewFileViewId;
	bool			CompareToPrev;

	bool			EncodingsCheck;
	bool			AlignAllMatches;
	bool			NeverMarkIgnored;
	bool			FollowingCaret;
	bool			WrapAround;
	bool			GotoFirstDiff;
	bool			PromptToCloseOnMatch;

	bool			DetectMoves;
	bool			DetectCharDiffs;
	bool			IgnoreEmptyLines;
	bool			IgnoreFoldedLines;
	bool			IgnoreChangedSpaces;
	bool			IgnoreAllSpaces;
	bool			IgnoreCase;
	bool			IgnoreRegex;
	bool			InvertRegex;
	std::wstring	IgnoreRegexStr;

	bool			ShowOnlyDiffs;
	bool			ShowOnlySelections;
	bool			UseNavBar;

	bool			RecompareOnChange;

	int				ChangedThresholdPercent;

	bool			EnableToolbar;
	bool			SetAsFirstTB;
	bool			CompareTB;
	bool			CompareSelTB;
	bool			ClearCompareTB;
	bool			NavigationTB;
	bool			ShowOnlyDiffsTB;
	bool			NavBarTB;
	bool            ignoreComments;
	std::set<int>   commentStyles;

	ColorSettings colors;
};

extern UserSettings	Settings;

enum SciCaller {
	sciLeft = 1, sciRight, sciOutput, sciFindres
};
void markTextAsChanged(pSciCaller pc, intptr_t start, intptr_t length, int color);
void setStyles(UserSettings s);
void ready();
void wait();
void setCursor(Scintilla::CursorShape type);
void setTextStyles(ColorSettings s);
void setTextStyle(pSciCaller pc, ColorSettings s);
void setChangedStyle(pSciCaller pc, ColorSettings s);
void defineSymbol(int type,int symbol);
void defineColor(int type,int color);
char **getAllLines(pSciCaller pc,int *length, int **lineNum);

void resetPrevOffset();
__declspec(dllimport) void* GetCaller(SciCaller c);

bool getNextLineAfterFold(pSciCaller pS, intptr_t* line);
std::vector<char> getText(pSciCaller pS, intptr_t startPos, intptr_t endPos);
void toLowerCase(std::vector<char>& text, int codepage);
void clearWindow(pSciCaller pc);
std::vector<intptr_t> getFoldedLines(pSciCaller pc);
void setFoldedLines(pSciCaller pc, const std::vector<intptr_t>& foldedLines);
void clearChangedIndicator(pSciCaller pc, intptr_t start, intptr_t length);
inline intptr_t getPreviousUnhiddenLine(pSciCaller pc, intptr_t line);
void addBlankSection(pSciCaller pc, intptr_t line, intptr_t length, intptr_t textLinePos = 0, const char* text = nullptr);
void addBlankSectionAfter(pSciCaller pc, intptr_t line, intptr_t length);
bool isLineFolded(pSciCaller pc, intptr_t line);
void hideOutsideRange(pSciCaller pc, intptr_t startLine, intptr_t endLine);
void hideUnmarked(pSciCaller pc, int markMask);

enum Marker_t
{
	MARKER_CHANGED_LINE = 11,
	MARKER_ADDED_LINE,
	MARKER_REMOVED_LINE,
	MARKER_MOVED_LINE
};

constexpr int MARKER_MASK_CHANGED = (1 << MARKER_CHANGED_LINE);
constexpr int MARKER_MASK_ADDED = (1 << MARKER_ADDED_LINE);
constexpr int MARKER_MASK_REMOVED = (1 << MARKER_REMOVED_LINE);
constexpr int MARKER_MASK_MOVED_LINE = (1 << MARKER_MOVED_LINE) ;

constexpr int MARKER_MASK_MOVED = (1 << MARKER_MOVED_LINE) ;

constexpr int MARKER_MASK_DIFF_LINE = (1 << MARKER_CHANGED_LINE) |
(1 << MARKER_ADDED_LINE) |
(1 << MARKER_REMOVED_LINE);

constexpr int MARKER_MASK_NEW_LINE = (1 << MARKER_ADDED_LINE) |
(1 << MARKER_REMOVED_LINE);

constexpr int MARKER_MASK_CHANGED_LINE = (1 << MARKER_CHANGED_LINE);

constexpr int MARKER_MASK_LINE = (1 << MARKER_CHANGED_LINE) |
(1 << MARKER_ADDED_LINE) |
(1 << MARKER_REMOVED_LINE) |
(1 << MARKER_MOVED_LINE);

constexpr int MARKER_MASK_SYMBOL = 0;

constexpr int MARKER_MASK_ALL = MARKER_MASK_LINE | MARKER_MASK_SYMBOL;

constexpr int MARGIN_NUM = 4;





#endif
