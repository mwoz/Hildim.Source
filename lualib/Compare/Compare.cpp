// Compare.cpp: Определяет точку входа для приложения.
//

#include "Compare.h"
#include "NPPHelpers.h"
#include "Scintilla.h"
#include "Engine.h"


extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define SETTINGOBJECT "Compare_MT_SettingOnj"
#define SETTINGMARKERS "Compare_MT_SettingMarkers"

const TCHAR PLUGIN_NAME[] = TEXT("ComparePlus");
UserSettings	Settings;
NppData nppData = { NULL, NULL };
CompareOptions cmpOptions;

static bool different = TRUE;

inline void raise_error(lua_State *L, const char *errMsg = NULL) {
	luaL_where(L, 1);
	if (errMsg) {
		lua_pushstring(L, errMsg);
	} else {
		lua_insert(L, -2);
	}
	lua_concat(L, 2);
	lua_error(L);
}

static int cf_metatable_index(lua_State *L) {
	if (lua_isstring(L, 2)) {
		const char *name = lua_tostring(L, 2);
		if (!lstrcmpA(name, "Settings")) {
			void *s = lua_newuserdata(L, sizeof(char));
			luaL_getmetatable(L, SETTINGOBJECT);
			lua_setmetatable(L, -2);
			return 1;
		} else if (!lstrcmpA(name, "Markers")) {
			void *s = lua_newuserdata(L, sizeof(char));
			luaL_getmetatable(L, SETTINGMARKERS);
			lua_setmetatable(L, -2);
			return 1;
		}
	}
	raise_error(L, "Compare internal error: invalid property");
	return -1;
}

static int cf_markers_metatable_index(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);
	if (!lstrcmpA(name, "MARKER_CHANGED_LINE")) {
		lua_pushinteger(L, MARKER_CHANGED_LINE);
	}
	else if (!lstrcmpA(name, "MARKER_ADDED_LINE")) {
		lua_pushinteger(L, MARKER_ADDED_LINE);
	}
	else if (!lstrcmpA(name, "MARKER_REMOVED_LINE")) {
		lua_pushinteger(L, MARKER_REMOVED_LINE);
	}
	else if (!lstrcmpA(name, "MARKER_MOVED_LINE")) {
		lua_pushinteger(L, MARKER_MOVED_LINE);
	}
	else if (!lstrcmpA(name, "MARKER_BLANK")) {
		lua_pushinteger(L, MARKER_BLANK);
	}
	else if (!lstrcmpA(name, "MARKER_CHANGED_SYMBOL")) {
		lua_pushinteger(L, MARKER_CHANGED_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_CHANGED_LOCAL_SYMBOL")) {
		lua_pushinteger(L, MARKER_CHANGED_LOCAL_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_ADDED_SYMBOL")) {
		lua_pushinteger(L, MARKER_ADDED_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_ADDED_LOCAL_SYMBOL")) {
		lua_pushinteger(L, MARKER_ADDED_LOCAL_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_REMOVED_SYMBOL")) {
		lua_pushinteger(L, MARKER_REMOVED_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_REMOVED_LOCAL_SYMBOL")) {
		lua_pushinteger(L, MARKER_REMOVED_LOCAL_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_MOVED_LINE_SYMBOL")) {
		lua_pushinteger(L, MARKER_MOVED_LINE_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_MOVED_BLOCK_BEGIN_SYMBOL")) {
		lua_pushinteger(L, MARKER_MOVED_BLOCK_BEGIN_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_MOVED_BLOCK_MID_SYMBOL")) {
		lua_pushinteger(L, MARKER_MOVED_BLOCK_MID_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_MOVED_BLOCK_END_SYMBOL")) {
		lua_pushinteger(L, MARKER_MOVED_BLOCK_END_SYMBOL);
	}
	else if (!lstrcmpA(name, "MARKER_ARROW_SYMBOL")) {
		lua_pushinteger(L, MARKER_ARROW_SYMBOL);
	}
	return 1;
}

static int cf_sett_metatable_index(lua_State *L) {
	if (lua_isstring(L, 2)) {
		const char *name = lua_tostring(L, 2);
		int tt = 0;
		if (!lstrcmpA(name, "AddLine")) {
			lua_pushboolean(L, Settings.AlignAllMatches);
		} else if (!lstrcmpA(name, "DetectMove")) {
			lua_pushboolean(L, Settings.DetectMoves);
		} else if (!lstrcmpA(name, "IncludeSpace")) {
			lua_pushboolean(L, Settings.IgnoreAllSpaces);
		} else if (!lstrcmpA(name, "UseSymbols")) {
			lua_pushboolean(L, false);
			//lua_pushboolean(L, Settings.UseSymbols);
		} else if (!lstrcmpA(name, "UseNavBar")) {
			lua_pushboolean(L, Settings.UseNavBar);
		} else if (!lstrcmpA(name, "Color_alpha")) {
			lua_pushinteger(L, 0);
		} else if (!lstrcmpA(name, "Color_highlight")) {
			lua_pushinteger(L, Settings.colors.add_highlight);
		} else if (!lstrcmpA(name, "Color_blank")) {
			lua_pushinteger(L, Settings.colors.blank);
		} else if (!lstrcmpA(name, "Color_moved")) {
			lua_pushinteger(L, Settings.colors.moved);
		} else if (!lstrcmpA(name, "Color_changed")) {
			lua_pushinteger(L, Settings.colors.changed);
		} else if (!lstrcmpA(name, "Color_deleted")) {
			lua_pushinteger(L, Settings.colors.removed);
		} else if (!lstrcmpA(name, "Color_added")) {
			lua_pushinteger(L, Settings.colors.moved);
		} else {
			raise_error(L, "Compare setting error: invalid property");
			return -1;
		}
	}
	return 1;
}

static int cf_sett_metatable_newindex(lua_State *L) {
	if (lua_isstring(L, 2)) {
		const char *name = lua_tostring(L, 2);
		if (false) {
		}
		else if (!lstrcmpA(name, "findUniqueMode")) {
			cmpOptions.findUniqueMode = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "alignAllMatches")) {
			cmpOptions.alignAllMatches = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "neverMarkIgnored")) {
			cmpOptions.neverMarkIgnored = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "detectMoves")) {
			cmpOptions.detectMoves = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "detectCharDiffs")) {
			cmpOptions.detectCharDiffs = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "ignoreEmptyLines")) {
			cmpOptions.ignoreEmptyLines = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "ignoreFoldedLines")) {
			cmpOptions.ignoreFoldedLines = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "ignoreChangedSpaces")) {
			cmpOptions.ignoreChangedSpaces = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "ignoreIndent")) {
			cmpOptions.ignoreIndent = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "ignoreAllSpaces")) {
			cmpOptions.ignoreAllSpaces = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "ignoreCase")) {
			cmpOptions.ignoreCase = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "recompareOnChange")) {
			cmpOptions.recompareOnChange = lua_toboolean(L, 3);
		}
		else if (!lstrcmpA(name, "selectionCompare")) {
			cmpOptions.selectionCompare = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "invertRegexp")) {
			cmpOptions.invertRegex = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "ignoreRegexp")) {
			std::string s(lua_tostring(L, 3));
			cmpOptions.setIgnoreRegex(s, cmpOptions.invertRegex);
		} else if (!lstrcmpA(name, "DetectMove")) {
			Settings.DetectMoves = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "IncludeSpace")) {
			Settings.IgnoreAllSpaces = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "UseSymbols")) {
			Settings.IconMask = lua_toboolean(L, 3) ? 0xFFFFFF : MARKER_MASK_LINE;
			//Settings.UseSymbols = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "UseNavBar")) {
			Settings.UseNavBar = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "ShowOnlyDiffs")) {
			Settings.ShowOnlyDiffs = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "Color_alpha")) {
			//Settings._colors->added = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_highlight")) {
			Settings.colors.add_highlight = lua_tointeger(L, 3);
			Settings.colors.rem_highlight = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_blank")) {
			Settings.colors.blank = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_moved")) {
			Settings.colors.moved = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_changed")) {
			Settings.colors.changed = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_deleted")) {
			Settings.colors.removed = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_added")) {
			Settings.colors.added = lua_tointeger(L, 3);
		} else {
			raise_error(L, "Compare setting error: invalid property");
			return -1;
		}
	}
	return 1;
}

static int run_Init(lua_State *L) {
	if (!nppData._scintillaMainHandle) {
		if (!lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
			return NULL;
		HWND h1 = (HWND)lua_touserdata(L, 1);
		HWND h2 = (HWND)lua_touserdata(L, 2);

		nppData._scintillaMainHandle = ::FindWindowEx(h1, NULL, L"Scintilla", NULL);
		nppData._scintillaSecondHandle = ::FindWindowEx(h2, NULL, L"Scintilla", NULL);

		nppData.pMain = (pSciCaller)GetCaller(sciLeft);
		nppData.pSecond = (pSciCaller)GetCaller(sciRight);

		nppData.pMain->AnnotationSetVisible(Scintilla::AnnotationVisible::Standard);
		nppData.pSecond->AnnotationSetVisible(Scintilla::AnnotationVisible::Standard);

	}
	if ( nppData.pMain->AnnotationGetVisible() != Scintilla::AnnotationVisible::Standard ||
	   nppData.pSecond->AnnotationGetVisible() != Scintilla::AnnotationVisible::Standard ){
			lua_pushinteger(L, 1);
			nppData._scintillaMainHandle = NULL;
		}
		else
			lua_pushinteger(L, 0);
	return 1;
}
static int run_SetStyles(lua_State *L) {
	setStyles(Settings);
	return 0;
}

void updateViewsFoldState(lua_State* L)
{
	FoldType_t foldType = NOT_SET;
	pSciCaller pM = nppData.pMain;
	pSciCaller pS = nppData.pSecond;
	if (Settings.ShowOnlyDiffs)
	{
		foldType = FOLD_MATCHES;
		hideUnmarked(pM, MARKER_MASK_LINE);
		hideUnmarked(pS, MARKER_MASK_LINE);
	}
	else if (cmpOptions.selectionCompare && Settings.ShowOnlySelections)
	{
		foldType = FOLD_OUTSIDE_SELECTIONS;
		hideOutsideRange(pM, cmpOptions.selections[MAIN_VIEW].first,
			cmpOptions.selections[MAIN_VIEW].second);
		hideOutsideRange(pS, cmpOptions.selections[SUB_VIEW].first,
			cmpOptions.selections[SUB_VIEW].second);
	}
	else if (cmpOptions.foldType != NO_FOLD)
	{
		foldType = NO_FOLD;

		auto foldedLines = getFoldedLines(pM);
		pM->FoldAll(Scintilla::FoldAction::Expand);
		setFoldedLines(pM, foldedLines);

		foldedLines = getFoldedLines(pS);
		pS->FoldAll(Scintilla::FoldAction::Expand);
		setFoldedLines(pS, foldedLines);
	}
	else
		return;

	lua_getglobal(L, "COMPARE_CLB_UpdateFoldType");
	if (lua_isfunction(L, -1)) {
		lua_pushnumber(L, foldType);
		int rez = lua_pcall(L, 1, 1, 0);
		return;
	}
	raise_error(L, "COMPARE_CLB_UpdateFoldType not found");
}

void alignDiffs(CompareSummary &summary, movedMap& moves)
{

	const AlignmentInfo_t& alignmentInfo = summary.alignmentInfo;

	const intptr_t maxSize = static_cast<intptr_t>(alignmentInfo.size());

	intptr_t mainEndLine;
	intptr_t subEndLine;
	pSciCaller pM = nppData.pMain;
	pSciCaller pS = nppData.pSecond;

	if (!cmpOptions.selectionCompare)
	{
		mainEndLine = pM->LineCount() - 1;
		subEndLine = pS->LineCount() - 1;
	}
	else
	{
		mainEndLine = cmpOptions.selections[MAIN_VIEW].second;
		subEndLine = cmpOptions.selections[SUB_VIEW].second;
	}

	bool skipFirst = false;

	intptr_t i = 0;

	// Handle zero line diffs that cannot be aligned because annotation on line 0 is not supported by Scintilla
	for (; i < maxSize && alignmentInfo[i].main.line <= mainEndLine && alignmentInfo[i].sub.line <= subEndLine; ++i)
	{
		intptr_t previousUnhiddenLine = getPreviousUnhiddenLine(pM, alignmentInfo[i].main.line);

		if (pM->AnnotationGetLines(previousUnhiddenLine) > 0)
			pM->AnnotationSetText(previousUnhiddenLine, NULL);

		previousUnhiddenLine = getPreviousUnhiddenLine(pS, alignmentInfo[i].sub.line);

		if (pS->AnnotationGetLines(previousUnhiddenLine) > 0)
			pS->AnnotationSetText(previousUnhiddenLine, NULL);

		if (alignmentInfo[i].main.line == 0 || alignmentInfo[i].sub.line == 0)
		{
			skipFirst = (alignmentInfo[i].main.line == alignmentInfo[i].sub.line);
			continue;
		}

		if (i == 0 || skipFirst)
			break;

		const intptr_t mismatchLen =
			pM->VisibleFromDocLine(alignmentInfo[i].main.line) - pS->VisibleFromDocLine(alignmentInfo[i].sub.line);

		if (cmpOptions.selectionCompare)
		{
			const intptr_t mainOffset = (mismatchLen < 0) ? -mismatchLen : 0;
			const intptr_t subOffset = (mismatchLen > 0) ? mismatchLen : 0;

			if (alignmentInfo[i - 1].main.line != 0)
			{
				addBlankSection(pM, cmpOptions.selections[MAIN_VIEW].first, mainOffset + 1, mainOffset + 1,
					"\\/\\/ Selection Compare Block Start \\/\\/");
				addBlankSection(pS, alignmentInfo[i].sub.line, subOffset + 1, subOffset + 1,
					"Lines above cannot be properly aligned.");
			}
			else if (alignmentInfo[i - 1].sub.line != 0)
			{
				addBlankSection(pM, alignmentInfo[i].main.line, mainOffset + 1, mainOffset + 1,
					"Lines above cannot be properly aligned.");
				addBlankSection(pS, cmpOptions.selections[SUB_VIEW].first, subOffset + 1, subOffset + 1,
					"\\/\\/ Selection Compare Block Start \\/\\/");
			}
			else
			{
				break;
			}
		}
		else
		{
			static const char* lineZeroAlignInfo =
				"^^^^Lines above cannot be properly aligned.^^^^^\n"
				"To see them aligned, please manually insert one empty line\n"
				"in the beginning of each file and then re-compare.\n"
				"------------------------------------------------------------";

			if (mismatchLen > 0)
			{
				addBlankSection(pM, alignmentInfo[i].main.line, 1, 1, lineZeroAlignInfo);
				addBlankSection(pS, alignmentInfo[i].sub.line, mismatchLen + 1, mismatchLen + 1,
					lineZeroAlignInfo);
			}
			else if (mismatchLen < 0)
			{
				addBlankSection(pM, alignmentInfo[i].main.line, -mismatchLen + 1, -mismatchLen + 1,
					lineZeroAlignInfo);
				addBlankSection(pS, alignmentInfo[i].sub.line, 1, 1, lineZeroAlignInfo);
			}
		}

		++i;
		break;
	}

	// Align all other diffs
	for (; i < maxSize && alignmentInfo[i].main.line <= mainEndLine && alignmentInfo[i].sub.line <= subEndLine; ++i)
	{
		intptr_t previousUnhiddenLine = getPreviousUnhiddenLine(pM, alignmentInfo[i].main.line);

		if (pM->AnnotationGetLines(previousUnhiddenLine) > 0)
			pM->AnnotationSetText(previousUnhiddenLine, NULL);

		previousUnhiddenLine = getPreviousUnhiddenLine(pS, alignmentInfo[i].sub.line);

		if (pS->AnnotationGetLines(previousUnhiddenLine) > 0)
			pS->AnnotationSetText(previousUnhiddenLine, NULL);

		if (isLineFolded(pM, alignmentInfo[i].main.line) || isLineFolded(pS, alignmentInfo[i].sub.line))
			continue;

		const intptr_t mismatchLen =
			pM->VisibleFromDocLine(alignmentInfo[i].main.line) - pS->VisibleFromDocLine(alignmentInfo[i].sub.line);

		if (mismatchLen > 0)
		{
			if ((i + 1 < maxSize) && (alignmentInfo[i].sub.line == alignmentInfo[i + 1].sub.line))
				continue;

			pS->AnnotationSetText(getPreviousUnhiddenLine(pS, alignmentInfo[i].sub.line), moves.main.text4annotation(alignmentInfo[i - 1].main.line, mismatchLen));
			//addBlankSection(pS, alignmentInfo[i].sub.line, mismatchLen, 1, moves.main.text4annotation(alignmentInfo[i - 1].main.line, mismatchLen));
		}
		else if (mismatchLen < 0)
		{
			if ((i + 1 < maxSize) && (alignmentInfo[i].main.line == alignmentInfo[i + 1].main.line))
				continue;
			pM->AnnotationSetText(getPreviousUnhiddenLine(pM, alignmentInfo[i].main.line), moves.sub.text4annotation(alignmentInfo[i - 1].sub.line, -mismatchLen));

			//addBlankSection(pM, alignmentInfo[i].main.line, -mismatchLen, 1, moves.sub.text4annotation(alignmentInfo[i - 1].sub.line, -mismatchLen));
		}
	}

	if (Settings.ShowOnlyDiffs)
	{
		mainEndLine = pM->MarkerPrevious(mainEndLine, MARKER_MASK_LINE);
		subEndLine = pM->MarkerPrevious(subEndLine, MARKER_MASK_LINE);

		if (mainEndLine < 0)
			mainEndLine = 0;
		if (subEndLine < 0)
			subEndLine = 0;
	}

	const intptr_t mainEndVisible = pM->VisibleFromDocLine(mainEndLine) + pM->WrapCount(mainEndLine) - 1;
	const intptr_t subEndVisible = pS->VisibleFromDocLine(subEndLine) + pS->WrapCount(subEndLine) - 1;

	const intptr_t mismatchLen = mainEndVisible - subEndVisible;
	const intptr_t absMismatchLen = std::abs(mismatchLen);
	const intptr_t linesOnScreen = pM->LinesOnScreen();
	const intptr_t endMisalignment = (absMismatchLen < linesOnScreen) ? absMismatchLen : linesOnScreen;

	const intptr_t mainEndLineAnnotation = pM->AnnotationGetLines(mainEndLine);
	const intptr_t subEndLineAnnotation = pS->AnnotationGetLines(subEndLine);

	if ((!cmpOptions.selectionCompare && mainEndLineAnnotation && subEndLineAnnotation) ||
		(std::abs(mainEndLineAnnotation - subEndLineAnnotation) != endMisalignment))
	{
		if (mismatchLen == 0)
		{
			pM->AnnotationSetText(mainEndLine, nullptr);
			pS->AnnotationSetText(subEndLine, nullptr);
		}
		else if (mismatchLen > 0)
		{
			pM->AnnotationSetText(mainEndLine, nullptr);
			addBlankSectionAfter(pS, subEndLine, endMisalignment);
		}
		else
		{
			addBlankSectionAfter(pM, mainEndLine, endMisalignment);
			pS->AnnotationSetText(subEndLine, nullptr);
		}
	}

	// Mark selections for clarity
	if (cmpOptions.selectionCompare)
	{
		// Line zero selections are already covered
		if (cmpOptions.selections[MAIN_VIEW].first > 0 && cmpOptions.selections[SUB_VIEW].first > 0)
		{
			intptr_t mainAnnotation = pM->AnnotationGetLines(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].first));
			intptr_t subAnnotation = pS->AnnotationGetLines(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].first));
				
			const intptr_t visibleBlockStartMismatch =
				pM->VisibleFromDocLine(cmpOptions.selections[MAIN_VIEW].first) - pS->VisibleFromDocLine(cmpOptions.selections[SUB_VIEW].first);

			++mainAnnotation;
			++subAnnotation;

			intptr_t mainAnnotPos = mainAnnotation;
			intptr_t subAnnotPos = subAnnotation;

			if (visibleBlockStartMismatch > 0)
				mainAnnotPos -= visibleBlockStartMismatch;
			else if (visibleBlockStartMismatch < 0)
				subAnnotPos += visibleBlockStartMismatch;

			addBlankSection(pM, cmpOptions.selections[MAIN_VIEW].first, mainAnnotation, mainAnnotPos,
				"\\/\\/ Selection Compare Block Start \\/\\/");
			addBlankSection(pS, cmpOptions.selections[SUB_VIEW].first, subAnnotation, subAnnotPos,
				"\\/\\/ Selection Compare Block Start \\/\\/");
		}

		{
			intptr_t mainAnnotation = pM->AnnotationGetLines(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].second + 1));
			intptr_t subAnnotation = pS->AnnotationGetLines(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].second + 1));
				
			if (mainAnnotation == 0 || subAnnotation == 0)
			{
				++mainAnnotation;
				++subAnnotation;
			}

			addBlankSection(pM, cmpOptions.selections[MAIN_VIEW].second + 1,
				mainAnnotation, mainAnnotation, "^^^ Selection Compare Block End ^^^");
			addBlankSection(pS, cmpOptions.selections[SUB_VIEW].second + 1,
				subAnnotation, subAnnotation, "^^^ Selection Compare Block End ^^^");
		}
	}
}


static int run_Compare(lua_State *L) {

	SCROLLINFO sci = {
		sizeof(sci), 0, 0, 0, 0, 0, 0
	};

	bool findUniqueMode = false;
	bool selectionCompare = false;
	int foldType = NOT_SET;
	lua_Integer s0 = -1, e0 = -1, s1 = -1, e1 = -1;

	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushliteral(L, "findUniqueMode");
		lua_gettable(L, -2);
		findUniqueMode = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushliteral(L, "selectionCompare");
		lua_gettable(L, -2);
		selectionCompare = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushliteral(L, "foldType");
		lua_gettable(L, -2);
		foldType = lua_tointeger(L, -1);
		if(foldType < 0 || foldType > 3)
			luaL_error(L, "Compare Selections: Field \"foldType\" in invalid format ");

		lua_pop(L, 1);

		if (selectionCompare) {
			lua_pushliteral(L, "selections");
			lua_gettable(L, -2);
			if (lua_type(L, -1) != LUA_TTABLE)
				luaL_error(L, "Compare Selections: Field \"selections\" not found ");
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			if (lua_type(L, -1) == LUA_TTABLE) {
				lua_pushinteger(L, 1);
				lua_gettable(L, -2);
				s0 = luaL_checkint(L, -1);
				lua_pop(L, 1);
				lua_pushinteger(L, 2);
				lua_gettable(L, -2);
				e0 = luaL_checkint(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			lua_pushinteger(L, 2);
			lua_gettable(L, -2);
			if (lua_type(L, -1) == LUA_TTABLE) {
				lua_pushinteger(L, 1);
				lua_gettable(L, -2);
				s1 = luaL_checkint(L, -1);
				lua_pop(L, 1);
				lua_pushinteger(L, 2);
				lua_gettable(L, -2);
				e1 = luaL_checkint(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 2);
		}

		cmpOptions.findUniqueMode = findUniqueMode;
		cmpOptions.selectionCompare = selectionCompare;
		cmpOptions.foldType =         static_cast<FoldType_t>(foldType);;
		cmpOptions.selections[0] = std::make_pair(s0, e0);
		cmpOptions.selections[1] = std::make_pair(s1, e1);

	}

	CompareSummary summary;
	movedMap moves;
	CompareResult rez =  compareViews(cmpOptions, summary, moves);


	updateViewsFoldState(L);

	moves.main.close_and_sort();
	moves.sub.close_and_sort();

	alignDiffs(summary, moves);

	::SendMessage(::GetParent(nppData._scintillaMainHandle), SCI_SETSCROLLINFO, SB_VERT, (WPARAM)&sci);
	::SendMessage(::GetParent(nppData._scintillaSecondHandle), SCI_SETSCROLLINFO, SB_VERT, (WPARAM)&sci);

	return 0;
}



luaL_Reg compare[] =
{
	{ "Init", run_Init },
	{ "Compare", run_Compare },
	{ "SetStyles", run_SetStyles },
	{ NULL, NULL }
};

extern "C" __declspec(dllexport) int luaopen_Compare(lua_State* L) {

	if (luaL_newmetatable(L, SETTINGOBJECT)) {
		lua_pushcfunction(L, cf_sett_metatable_index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, cf_sett_metatable_newindex);
		lua_setfield(L, -2, "__newindex");
	}
	if (luaL_newmetatable(L, SETTINGMARKERS)) {
		lua_pushcfunction(L, cf_markers_metatable_index);
		lua_setfield(L, -2, "__index");
	}


	lua_newtable(L);
	luaL_setfuncs(L, compare, 0);

	if (luaL_newmetatable(L, "Compare")) {
		lua_pushcfunction(L, cf_metatable_index);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	return 1;
}
