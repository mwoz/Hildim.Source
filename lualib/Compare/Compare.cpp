// Compare.cpp: Определяет точку входа для приложения.
//

#include "Compare.h"
#include "NPPHelpers.h"
#include "Scintilla.h"
#include "Engine.h"
#include <map>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define SETTINGOBJECT "Compare_MT_SettingOnj"
#define SETTINGMARKERS "Compare_MT_SettingMarkers"

#define LUA_TOINT(L,i)	static_cast<int>(lua_tointegerx(L,(i),NULL))

const TCHAR PLUGIN_NAME[] = TEXT("ComparePlus");
UserSettings	Settings;
NppData nppData;
CompareOptions cmpOptions;

std::string _linesAbove = "!^^^ ";
std::string _selStart = "vvv ";
std::string _selEnd = "^^^ ";
std::string _toAlign = "";

intptr_t line1stAlign = 1;

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
		std::string er = "Compare.lua: ""Compare"" object does not have the """;
		er += name;
		er += """ property";
		raise_error(L, er.c_str());
	}
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
			std::string er = "Compare.lua, Compare.Settings: invalid property get: ";
			er += name;
			raise_error(L, er.c_str());
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
		else if (!lstrcmpA(name, "selectionCompare")) {
			cmpOptions.selectionCompare = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "invertRegexp")) {
			cmpOptions.invertRegex = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "ignoreRegexp")) {
			std::string s(lua_tostring(L, 3));
			cmpOptions.setIgnoreRegex(s, cmpOptions.invertRegex);
		} else if (!lstrcmpA(name, "bigBlockFactor")) {
			cmpOptions.bigBlockFactor = static_cast<short>(luaL_checkinteger(L, 3));
		} else if (!lstrcmpA(name, "inBlock_Percent")) {
			cmpOptions.inBlock_Percent = static_cast<int>(luaL_checkinteger(L, 3));
		} else if (!lstrcmpA(name, "inLine_Percent")) {
			cmpOptions.inLine_Percent = static_cast<int>(luaL_checkinteger(L, 3));
		} else if (!lstrcmpA(name, "DetectMove")) {
			Settings.DetectMoves = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "IncludeSpace")) {
			Settings.IgnoreAllSpaces = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "UseNavBar")) {
			Settings.UseNavBar = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "ShowOnlyDiffs")) {
			Settings.ShowOnlyDiffs = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "ShowDiffsContext")) {
			Settings.ShowDiffsContext = LUA_TOINT(L, 3);
		} else if (!lstrcmpA(name, "ignoreComments")) {
			Settings.ignoreComments = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "Color_alpha")) {
			//Settings._colors->added = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_highlight")) {
			Settings.colors.add_highlight = LUA_TOINT(L, 3);
			Settings.colors.rem_highlight = LUA_TOINT(L, 3);
		} else if (!lstrcmpA(name, "Color_blank")) {
			Settings.colors.blank = LUA_TOINT(L, 3);
		} else if (!lstrcmpA(name, "Color_moved")) {
			Settings.colors.moved = LUA_TOINT(L, 3);
		} else if (!lstrcmpA(name, "Color_changed")) {
			Settings.colors.changed = LUA_TOINT(L, 3);
		} else if (!lstrcmpA(name, "Color_deleted")) {
			Settings.colors.removed = LUA_TOINT(L, 3);
		} else if (!lstrcmpA(name, "Color_added")) {
			Settings.colors.added = LUA_TOINT(L, 3);
		} else {
			std::string er = "Compare.lua, Compare.Settings: invalid property set: ";
			er += name;
			raise_error(L, er.c_str());
			return -1;
		}
	}
	return 1;
}

static int run_Init(lua_State *L) {
	if (!nppData.scintillaMainHandle) {
		if (!lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
			return NULL;
		HWND h1 = (HWND)lua_touserdata(L, 1);
		HWND h2 = (HWND)lua_touserdata(L, 2);
		nppData.init(::FindWindowEx(h1, NULL, L"Scintilla", NULL), ::FindWindowEx(h2, NULL, L"Scintilla", NULL), (pSciCaller)GetCaller(sciLeft), (pSciCaller)GetCaller(sciRight));


		nppData.pMain->AnnotationSetVisible(Scintilla::AnnotationVisible::Standard);
		nppData.pSecond->AnnotationSetVisible(Scintilla::AnnotationVisible::Standard);

	}
	if ( nppData.pMain->AnnotationGetVisible() != Scintilla::AnnotationVisible::Standard ||
	   nppData.pSecond->AnnotationGetVisible() != Scintilla::AnnotationVisible::Standard ){
			lua_pushinteger(L, 1);
			nppData.clear();
		}
		else
			lua_pushinteger(L, 0);
	return 1;
}
static int run_SetStyles(lua_State *L) {

		lua_pushliteral(L, "linesAbove");
		lua_gettable(L, -2);
		_linesAbove += luaL_checkstring(L, -1);
		lua_pop(L, 1);
		_linesAbove += " ^^^!";

		lua_pushliteral(L, "toAlign");
		lua_gettable(L, -2);
		_toAlign = _linesAbove;
		_toAlign += '\n';
		_toAlign += luaL_checkstring(L, -1);
		lua_pop(L, 1);
		_toAlign += "\n----------------------------------";

		lua_pushliteral(L, "selStart");
		_selStart = "vvv ";
		lua_gettable(L, -2);
		_selStart += luaL_checkstring(L, -1);
		lua_pop(L, 1);
		_selStart += " vvv";

		lua_pushliteral(L, "selEnd");
		_selEnd = "^^^ ";
		lua_gettable(L, -2);
		_selEnd += luaL_checkstring(L, -1);
		lua_pop(L, 1);
		_selEnd += " ^^^";

		lua_pushliteral(L, "hiddehLineHeader");
		lua_gettable(L, -2);
		Settings.hiddehLineHeader = luaL_checkstring(L, -1);
		lua_pop(L, 1);



	setStyles(Settings);
	return 0;
}

void updateViewsFoldState(lua_State* L)
{
	FoldType_t foldType = NOT_SET;
	pSciCaller pM = nppData.pMain;
	pSciCaller pS = nppData.pSecond;
	if (Settings.ShowOnlyDiffs && !Settings.ShowDiffsContext && !cmpOptions.selectionCompare)
	{
		foldType = FOLD_MATCHES;
		hideUnmarked(pM, MARKER_MASK_LINE);
		hideUnmarked(pS, MARKER_MASK_LINE);
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
	raise_error(L, "Compare.lua: ""COMPARE_CLB_UpdateFoldType"" not implemented");
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
	pM->AnnotationClearAll();
	pS->AnnotationClearAll();

	intptr_t i = 0; 
	intptr_t mismatchLenTotal = 0;
	// Handle zero line diffs that cannot be aligned because annotation on line 0 is not supported by Scintilla

	std::map<intptr_t, std::string> annotationMainMap, annotationSecondMap;

	line1stAlign = 1;

	for (; i < maxSize && alignmentInfo[i].main.line <= mainEndLine && alignmentInfo[i].sub.line <= subEndLine; ++i)
	{

		if (isLineFolded(pM, alignmentInfo[i].main.line) || isLineFolded(pS, alignmentInfo[i].sub.line))
			continue;

		const intptr_t mismatchLen =
			alignmentInfo[i].main.line - alignmentInfo[i].sub.line - mismatchLenTotal;


		intptr_t lSrc, lTgt;
  		if (mismatchLen > 0)
		{
			if ((i + 1 < maxSize) && (alignmentInfo[i].sub.line == alignmentInfo[i + 1].sub.line))
				continue;

			lSrc = alignmentInfo[i].sub.line - 1;
			lTgt = alignmentInfo[i].sub.line + mismatchLenTotal;

			std::map<intptr_t, bool> emptyLines;
			if (cmpOptions.ignoreEmptyLines && Settings.ShowOnlyDiffs && !Settings.ShowDiffsContext && !cmpOptions.selectionCompare) {
				for (int j = 0; j < mismatchLen; j++) {
					if (pM->LineIndentPosition(lTgt + j) == pM->LineEndPosition(lTgt + j))
						emptyLines[j] = true;
				}
			}

			std::string sOut = moves.main.text4annotation(lTgt, mismatchLen, lSrc, emptyLines);
			 
			if (lSrc == -1) {
				lSrc = 0;
				sOut += '\n';
				sOut += _toAlign;
				annotationMainMap[alignmentInfo[i].main.line] += _toAlign;
				line1stAlign = alignmentInfo[i].main.line + 1;
			}

			if (!sOut.empty()) {
				if (annotationSecondMap[lSrc].empty())
					annotationSecondMap[lSrc] = sOut;
				else {
					annotationSecondMap[lSrc] += '\n';
					annotationSecondMap[lSrc] += sOut;
				}
			}
		}
		else if (mismatchLen < 0)
		{
			if ((i + 1 < maxSize) && (alignmentInfo[i].main.line == alignmentInfo[i + 1].main.line))
				continue;

			lSrc = alignmentInfo[i].main.line - 1;
			lTgt = alignmentInfo[i].main.line - mismatchLenTotal;

			std::map<intptr_t, bool> emptyLines;
			if (cmpOptions.ignoreEmptyLines && Settings.ShowOnlyDiffs && !Settings.ShowDiffsContext && !cmpOptions.selectionCompare) {
				for (int j = 0; j < (-mismatchLen); j++) {
					if (pS->LineIndentPosition(lTgt + j) == pS->LineEndPosition(lTgt + j))
						emptyLines[j] = true;
				}
			}

			std::string sOut = moves.sub.text4annotation(lTgt, -mismatchLen, lSrc, emptyLines);
			
			if (lSrc == -1) {
				lSrc = 0;
				sOut += '\n';
				sOut += _toAlign;
				annotationSecondMap[alignmentInfo[i].sub.line] += _toAlign;
				line1stAlign = -alignmentInfo[i].sub.line - 1;
			}

			if (!sOut.empty()) {
				if (annotationMainMap[lSrc].empty())
					annotationMainMap[lSrc] = sOut;
				else {
					annotationMainMap[lSrc] += '\n';
					annotationMainMap[lSrc] += sOut;
				}
			}

		}
		mismatchLenTotal += mismatchLen;
	}
	if (Settings.ShowOnlyDiffs && !Settings.ShowDiffsContext && !cmpOptions.selectionCompare) {  //
		std::string sTotal("");	

		
		for (i = pM->LineCount() - 1; i >= 0; i--) {
			if (annotationMainMap.count(i)) {
				if (!sTotal.empty())
					sTotal = '\n' + sTotal;
				sTotal = annotationMainMap[i] + sTotal;
				annotationMainMap.erase(i);
			}
			if (pM->LineVisible(i) && !sTotal.empty()) {
				if (sTotal[0] == '!')
					sTotal = '\n' + sTotal;
				annotationMainMap[i] = sTotal;
				sTotal = "";
			}
		}
		sTotal = "";
		for (i = pS->LineCount() - 1; i >= 0; i--) {
			if (annotationSecondMap.count(i)) {
				if (!sTotal.empty())
					sTotal = '\n' + sTotal;
				sTotal = annotationSecondMap[i] + sTotal;
				annotationSecondMap.erase(i);
			}
			if (pS->LineVisible(i) && !sTotal.empty()) {
				if (sTotal[0] == '!')
					sTotal = '\n' + sTotal;
				annotationSecondMap[i] = sTotal;
				sTotal = "";
			}
		}

	}

	for (auto p : annotationMainMap) {
		pM->AnnotationSetText(p.first, p.second.c_str());
	}
	for (auto p : annotationSecondMap) {
		pS->AnnotationSetText(p.first, p.second.c_str());
	}

	if (Settings.ShowOnlyDiffs && !Settings.ShowDiffsContext && !cmpOptions.selectionCompare)
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

	if (!cmpOptions.findUniqueMode && ((!cmpOptions.selectionCompare && mainEndLineAnnotation && subEndLineAnnotation) ||
		(std::abs(mainEndLineAnnotation - subEndLineAnnotation) != endMisalignment)))
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
		std::string blank("");
		// Line zero selections are already covered
		if (cmpOptions.selections[MAIN_VIEW].first > 0 && cmpOptions.selections[SUB_VIEW].first > 0)
		{

			if (cmpOptions.findUniqueMode) {
				pM->AnnotationSetText(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].first), _selStart.c_str());
				pS->AnnotationSetText(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].first), _selStart.c_str());

			}
			else {
				intptr_t mainAnnotation = pM->AnnotationGetLines(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].first));
				intptr_t subAnnotation = pS->AnnotationGetLines(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].first));

				
				blank = "";
				if (subAnnotation > 0) 
					blank += std::string(subAnnotation, '\n');
				blank += _selStart;

				pS->AnnotationSetText(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].first), blank.c_str());

			
				blank = "";

				if (mainAnnotation > 0)
					blank += std::string(mainAnnotation , '\n');
				blank += _selStart;

				pM->AnnotationSetText(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].first), blank.c_str());
			}
		}

		{
				
			if (cmpOptions.findUniqueMode) {
				pM->AnnotationSetText(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].second + 1), _selEnd.c_str());
				pS->AnnotationSetText(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].second + 1), _selEnd.c_str());
			}
			else {
				intptr_t mainAnnotation = pM->AnnotationGetLines(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].second + 1));
				intptr_t subAnnotation = pS->AnnotationGetLines(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].second + 1));

				blank = "";
				blank += _selEnd;
				if (mainAnnotation > 0)
					blank += std::string(mainAnnotation, '\n');
				pM->AnnotationSetText(getPreviousUnhiddenLine(pM, cmpOptions.selections[MAIN_VIEW].second + 1), blank.c_str());

				blank = "";
				blank += _selEnd;
				if (subAnnotation > 0)
					blank += std::string(subAnnotation, '\n');
				pS->AnnotationSetText(getPreviousUnhiddenLine(pS, cmpOptions.selections[SUB_VIEW].second + 1), blank.c_str());



				//addBlankSection(pM, cmpOptions.selections[MAIN_VIEW].second + 1, mainAnnotation, mainAnnotation, _selEnd.c_str());

				//addBlankSection(pS, cmpOptions.selections[SUB_VIEW].second + 1, subAnnotation, subAnnotation, _selEnd.c_str());
			}
				
		}
	}
}


static int run_Compare(lua_State *L) {

	SCROLLINFO sci = {
		sizeof(sci), 0, 0, 0, 0, 0, 0
	};

	bool selectionCompare = false;
	int newFileViewId = 1;
	int foldType = NOT_SET;
	lua_Integer s0 = -1, e0 = -1, s1 = -1, e1 = -1;

	if (lua_type(L, -1) == LUA_TTABLE) {

		lua_pushliteral(L, "newFileViewId");
		lua_gettable(L, -2);
		newFileViewId = LUA_TOINT(L, -1);
		lua_pop(L, 1);

		lua_pushliteral(L, "selectionCompare");
		lua_gettable(L, -2);
		selectionCompare = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushliteral(L, "foldType");
		lua_gettable(L, -2);
		foldType = LUA_TOINT(L, -1);
		if (foldType < 0 || foldType > 3)
		{
			std::string er = "Compare.lua, Compare: Field \"foldType\" in invalid format: ";
			er += std::to_string(foldType);
			raise_error(L, er.c_str());
		}

		lua_pop(L, 1);

		if (selectionCompare) {
			lua_pushliteral(L, "selections");
			lua_gettable(L, -2);
			if (lua_type(L, -1) != LUA_TTABLE)
			{
				std::string er = "Compare.lua, Compare selected: Field \"selections\" not found";
				er += std::to_string(foldType);
				raise_error(L, er.c_str());
			}
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
		Settings.commentStyles.clear();
		lua_pushliteral(L, "commentStyles");
		lua_gettable(L, -2);
		if (lua_type(L, -1) == LUA_TTABLE) {
			for (int i = 1; i < 100; i++) {
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				int id = LUA_TOINT(L, -1);
				lua_pop(L, 1);
				if (id) {
					Settings.commentStyles.insert(id);
				}
				else
					break;
			}
		}


		cmpOptions.newFileViewId = newFileViewId;
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

	//if (!cmpOptions.findUniqueMode) {
	alignDiffs(summary, moves);


	if (Settings.ShowOnlyDiffs && Settings.ShowDiffsContext && !cmpOptions.selectionCompare) {
		pSciCaller pM = nppData.pMain;
		pSciCaller pS = nppData.pSecond;
		intptr_t lMax = pM->VisibleFromDocLine(pM->LineCount() + 5);

		if(line1stAlign > 0)
			line1stAlign = pM->VisibleFromDocLine(line1stAlign);
		else
			line1stAlign = pS->VisibleFromDocLine(-line1stAlign);

		intptr_t lHideEnd = lMax;
		for (intptr_t l = lMax; l >= line1stAlign; l--) {
			intptr_t lM = pM->DocLineFromVisible(l);
			intptr_t lS = pS->DocLineFromVisible(l);

			if (lS == 179)
				int ttt = 0;
			//break;
			
			if (!(pM->MarkerGet(lM) & MARKER_MASK_LINE) && !(pS->MarkerGet(lS) & MARKER_MASK_LINE)) {
				if (!lHideEnd)
					lHideEnd = l;
				continue;
			}
			//if (!lHideStart)
			//	lHideStart = l;

			if (lHideEnd - l > Settings.ShowDiffsContext * 2) {
				intptr_t l1st = l + Settings.ShowDiffsContext + 1;
				intptr_t lLastst = lHideEnd - Settings.ShowDiffsContext;
				intptr_t cnt = 0;
				if (cmpOptions.ignoreEmptyLines) {
					intptr_t cnt = Settings.ShowDiffsContext;
					for (l1st = l + 1; cnt && l1st < lHideEnd; l1st++)
						if(!pM->AnnotationGetLines(pM->DocLineFromVisible(l1st)) && !pS->AnnotationGetLines(pS->DocLineFromVisible(l1st)))
							cnt--;
					if (!cnt) {
						cnt = Settings.ShowDiffsContext;
						for (lLastst = lHideEnd; cnt && lLastst > l1st; lLastst--)
							if (!pM->AnnotationGetLines(pM->DocLineFromVisible(lLastst)) && !pS->AnnotationGetLines(pS->DocLineFromVisible(lLastst)))
								cnt--;
					}

				}
				else {
					l1st = l + Settings.ShowDiffsContext + 1;
					lLastst = lHideEnd - Settings.ShowDiffsContext;
				}
				//if (lLastst > l1st) {
				if (!cnt && l1st < lLastst) {
					intptr_t ttt = pS->DocLineFromVisible(l1st)-1;
					hideLinesHildim(pM, pM->DocLineFromVisible(l1st), pM->DocLineFromVisible(lLastst));
					hideLinesHildim(pS, pS->DocLineFromVisible(l1st), pS->DocLineFromVisible(lLastst));
				}
				
			}
			lHideEnd = 0;
		}
		if (lHideEnd > Settings.ShowDiffsContext + line1stAlign) {
			hideLinesHildim(pM, pM->DocLineFromVisible(line1stAlign), pM->DocLineFromVisible(lHideEnd - Settings.ShowDiffsContext));
			hideLinesHildim(pS, pS->DocLineFromVisible(line1stAlign), pS->DocLineFromVisible(lHideEnd - Settings.ShowDiffsContext));
		}

	}

	::SendMessage(::GetParent(nppData.scintillaMainHandle), SCI_SETSCROLLINFO, SB_VERT, (WPARAM)&sci);
	::SendMessage(::GetParent(nppData.scintillaSecondHandle), SCI_SETSCROLLINFO, SB_VERT, (WPARAM)&sci);
	//}
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
