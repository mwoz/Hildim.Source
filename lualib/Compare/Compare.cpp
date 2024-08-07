// Compare.cpp: Определяет точку входа для приложения.
//

#include "Compare.h"
#include "NPPHelpers.h"
#include "Scintilla.h"
#include "icon_add_16.h"
#include "icon_sub_16.h"
#include "icon_warning_16.h"
#include "icon_moved_16.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define SETTINGOBJECT "Compare_MT_SettingOnj"
#define SETTINGMARKERS "Compare_MT_SettingMarkers"
NppData nppData = { NULL, NULL };

static sUserSettings Settings;
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
	if (!lstrcmpA(name, "MARKER_MOVED_LINE")) {
		lua_pushinteger(L, MARKER_MOVED_LINE);
	} else if (!lstrcmpA(name, "MARKER_BLANK_LINE")) {
		lua_pushinteger(L, MARKER_BLANK_LINE);
	} else if (!lstrcmpA(name, "MARKER_CHANGED_LINE")) {
		lua_pushinteger(L, MARKER_CHANGED_LINE);
	} else if (!lstrcmpA(name, "MARKER_ADDED_LINE")) {
		lua_pushinteger(L, MARKER_ADDED_LINE);
	} else if (!lstrcmpA(name, "MARKER_REMOVED_LINE")) {
		lua_pushinteger(L, MARKER_REMOVED_LINE);
	} else if (!lstrcmpA(name, "MARKER_CHANGED_SYMBOL")) {
		lua_pushinteger(L, MARKER_CHANGED_SYMBOL);
	} else if (!lstrcmpA(name, "MARKER_ADDED_SYMBOL")) {
		lua_pushinteger(L, MARKER_ADDED_SYMBOL);
	} else if (!lstrcmpA(name, "MARKER_REMOVED_SYMBOL")) {
		lua_pushinteger(L, MARKER_REMOVED_SYMBOL);
	} else if (!lstrcmpA(name, "MARKER_MOVED_SYMBOL")) {
		lua_pushinteger(L, MARKER_MOVED_SYMBOL);
	} else {
		raise_error(L, "Compare setting error: invalid property");
		return -1;
	}
	return 1;
}

static int cf_sett_metatable_index(lua_State *L) {
	if (lua_isstring(L, 2)) {
		const char *name = lua_tostring(L, 2);
		if (!lstrcmpA(name, "AddLine")) {
			lua_pushboolean(L, Settings.AddLine);
		} else if (!lstrcmpA(name, "DetectMove")) {
			lua_pushboolean(L, Settings.DetectMove);
		} else if (!lstrcmpA(name, "IncludeSpace")) {
			lua_pushboolean(L, Settings.IncludeSpace);
		} else if (!lstrcmpA(name, "UseSymbols")) {
			lua_pushboolean(L, Settings.UseSymbols);
		} else if (!lstrcmpA(name, "UseNavBar")) {
			lua_pushboolean(L, Settings.UseNavBar);
		} else if (!lstrcmpA(name, "Color_alpha")) {
			lua_pushinteger(L, Settings.ColorSettings.alpha);
		} else if (!lstrcmpA(name, "Color_highlight")) {
			lua_pushinteger(L, Settings.ColorSettings.highlight);
		} else if (!lstrcmpA(name, "Color_blank")) {
			lua_pushinteger(L, Settings.ColorSettings.blank);
		} else if (!lstrcmpA(name, "Color_moved")) {
			lua_pushinteger(L, Settings.ColorSettings.moved);
		} else if (!lstrcmpA(name, "Color_changed")) {
			lua_pushinteger(L, Settings.ColorSettings.changed);
		} else if (!lstrcmpA(name, "Color_deleted")) {
			lua_pushinteger(L, Settings.ColorSettings.deleted);
		} else if (!lstrcmpA(name, "Color_added")) {
			lua_pushinteger(L, Settings.ColorSettings.added);
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
		if (!lstrcmpA(name, "AddLine")) {
			Settings.AddLine = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "DetectMove")) {
			Settings.DetectMove = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "IncludeSpace")) {
			Settings.IncludeSpace = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "UseSymbols")) {
			Settings.UseSymbols = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "UseNavBar")) {
			Settings.UseNavBar = lua_toboolean(L, 3);
		} else if (!lstrcmpA(name, "Color_alpha")) {
			Settings.ColorSettings.alpha = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_highlight")) {
			Settings.ColorSettings.highlight = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_blank")) {
			Settings.ColorSettings.blank = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_moved")) {
			Settings.ColorSettings.moved = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_changed")) {
			Settings.ColorSettings.changed = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_deleted")) {
			Settings.ColorSettings.deleted = lua_tointeger(L, 3);
		} else if (!lstrcmpA(name, "Color_added")) {
			Settings.ColorSettings.added = lua_tointeger(L, 3);
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

		::SendMessage(nppData._scintillaMainHandle, SCI_ANNOTATIONSETVISIBLE, 1, 0);
		::SendMessage(nppData._scintillaSecondHandle, SCI_ANNOTATIONSETVISIBLE, 1, 0);
	}
	if (!::SendMessage(nppData._scintillaMainHandle, SCI_ANNOTATIONGETVISIBLE, 0, 0) ||
		!::SendMessage(nppData._scintillaSecondHandle, SCI_ANNOTATIONGETVISIBLE, 0, 0)) {
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
static void AddMoveNum(std::string& l, diff_edit& e) {
	if (e.op == DIFF_MOVE) {
		char buff[32];
		_itoa_s(e.matchedLine + 1, buff, 10);
		l += "-> ";
		l += buff;
	}
}
static int run_Compare(lua_State *L) {


	int doc1Length;
	int *lineNum1;

	int iii = 11;
	char **doc1 = getAllLines(nppData._scintillaMainHandle, &doc1Length, &lineNum1);

	if (doc1Length < 1)
		return true;

	int doc2Length;
	int *lineNum2;

	char **doc2 = getAllLines(nppData._scintillaSecondHandle, &doc2Length, &lineNum2);

	if (doc2Length < 1)
		return true;

	int doc1Changed = 0;
	int doc2Changed = 0;
	diff_edit *doc1Changes = NULL;
	diff_edit *doc2Changes = NULL;

	unsigned int *doc1Hashes = computeHashes(doc1, doc1Length, Settings.IncludeSpace);
	unsigned int *doc2Hashes = computeHashes(doc2, doc2Length, Settings.IncludeSpace);

	/* make diff */
	int sn;
	struct varray *ses = varray_new(sizeof(struct diff_edit), NULL);
	int result = (diff(doc1Hashes, 0, doc1Length, doc2Hashes, 0, doc2Length, (idx_fn)(getLineFromIndex), (cmp_fn)(compareLines), NULL, 0, ses, &sn, NULL));
	int changeOffset = 0;

	shift_boundries(ses, sn, doc1Hashes, doc2Hashes, doc1Length, doc2Length);
	find_moves(ses, sn, doc1Hashes, doc2Hashes, Settings.DetectMove);
	/*
	* - insert empty lines
	* - count changed lines
	*/
	doc1Changed = 0;
	doc2Changed = 0;

	for (int i = 0; i < sn; i++) {
		struct diff_edit *e = (diff_edit*)varray_get(ses, i);
		if (e->op == DIFF_DELETE) {
			e->changeCount = 0;
			doc1Changed += e->len;
			struct diff_edit *e2 = (diff_edit*)varray_get(ses, i + 1);
			e2->changeCount = 0;

			if (e2->op == DIFF_INSERT) {
				//see if the DELETE/INSERT COMBO includes changed lines or if its a completely new block
				if (compareWords(e, e2, doc1, doc2, Settings.IncludeSpace)) {
					e->op = DIFF_CHANGE1;
					e2->op = DIFF_CHANGE2;
					doc2Changed += e2->len;
				}
			}
		} else if (e->op == DIFF_INSERT) {
			e->changeCount = 0;
			doc2Changed += e->len;
		}
	}

	int doc1CurrentChange = 0;
	int doc2CurrentChange = 0;
	changeOffset = 0;
	doc1Changes = new diff_edit[doc1Changed];
	doc2Changes = new diff_edit[doc2Changed];
	int doc1Offset = 0;
	int doc2Offset = 0;

	// Switch from blocks of lines to one change per line.
	// Change CHANGE to DELETE or INSERT if there are no changes on that line
	int added;

	for (int i = 0; i < sn; i++) {
		struct diff_edit *e = (diff_edit*)varray_get(ses, i);
		e->set = i;

		switch (e->op) {
		case DIFF_CHANGE1:
		case DIFF_DELETE:
			added = setDiffLines(e, doc1Changes, &doc1CurrentChange, DIFF_DELETE, e->off + doc2Offset);
			doc2Offset -= added;
			doc1Offset += added;
			break;
		case DIFF_INSERT:
		case DIFF_CHANGE2:
			added = setDiffLines(e, doc2Changes, &doc2CurrentChange, DIFF_INSERT, e->off + doc1Offset);
			doc1Offset -= added;
			doc2Offset += added;
			break;
		}
	}

	if (result != -1) {
		int textIndex;
		different = (doc1Changed > 0) || (doc2Changed > 0);

		for (int i = 0; i < doc1Changed; i++) {
			switch (doc1Changes[i].op) {
			case DIFF_DELETE:
				markAsAdded(nppData._scintillaMainHandle, doc1Changes[i].off, Settings.UseSymbols);
				break;

			case DIFF_CHANGE1:
				markAsChanged(nppData._scintillaMainHandle, doc1Changes[i].off, Settings.UseSymbols);
				textIndex = lineNum1[doc1Changes[i].off];

				for (int k = 0; k < doc1Changes[i].changeCount; k++) {
					struct diff_change *change = (diff_change*)varray_get(doc1Changes[i].changes, k);
					markTextAsChanged(nppData._scintillaMainHandle, textIndex + change->off, change->len);
				}
				break;

			case DIFF_MOVE:
				markAsMoved(nppData._scintillaMainHandle, doc1Changes[i].off, Settings.UseSymbols);
				break;

			}
		}

		for (int i = 0; i < doc2Changed; i++) {
			switch (doc2Changes[i].op) {
			case DIFF_INSERT:
				markAsRemoved(nppData._scintillaSecondHandle, doc2Changes[i].off, Settings.UseSymbols);
				break;

			case DIFF_CHANGE2:
				markAsChanged(nppData._scintillaSecondHandle, doc2Changes[i].off, Settings.UseSymbols);
				textIndex = lineNum2[doc2Changes[i].off];
				for (int k = 0; k < doc2Changes[i].changeCount; k++) {
					struct diff_change *change = (diff_change*)varray_get(doc2Changes[i].changes, k);
					markTextAsChanged(nppData._scintillaSecondHandle, textIndex + change->off, change->len);
				}
				break;

			case DIFF_MOVE:
				markAsMoved(nppData._scintillaSecondHandle, doc2Changes[i].off, Settings.UseSymbols);
				break;
			}
		}

	if (Settings.AddLine) {
		int length = 0;
		int off = -1;
		resetPrevOffset();
		std::string lines;
		for (int i = 0; i < doc1Changed; i++) {
			switch (doc1Changes[i].op) {
			case DIFF_DELETE:
			case DIFF_MOVE:
				if (doc1Changes[i].altLocation == off) {
					length++;
					lines += "\n";
					AddMoveNum(lines, doc1Changes[i]);
					if (!off)
						lines += "   ^^^above 1-st line^^^";
				} else {
					if(length > 0)
						addEmptyLines(nppData._scintillaSecondHandle, off - 1, length, lines.c_str());
					off = doc1Changes[i].altLocation;
					length = 1;
					lines = "";
					AddMoveNum(lines, doc1Changes[i]);
					if (!off)
						lines += "   ^^^above 1-st line^^^";
				}
				break;
			}
		}
		if (length > 0)
			addEmptyLines(nppData._scintillaSecondHandle, off - 1, length, lines.c_str());

		length = 0;
		off = -1;
		resetPrevOffset();
		for (int i = 0; i < doc2Changed; i++) {
			switch (doc2Changes[i].op) {
			case DIFF_INSERT:
			case DIFF_MOVE:
				if (doc2Changes[i].altLocation == off) {
					length++;
					//if (doc2Changes[i].op == DIFF_MOVE || (i + 1 == doc2Changed) || doc2Changes[i + 1].altLocation == off)
					lines += "\n";
					AddMoveNum(lines, doc2Changes[i]);
					if (!off)
						lines += "   ^^^above 1-st line^^^";
				} else {
					if(length > 0)
						addEmptyLines(nppData._scintillaMainHandle, off - 1, length, lines.c_str());
					off = doc2Changes[i].altLocation;
					length = 1;
					lines = "";
					AddMoveNum(lines, doc2Changes[i]);
					if (!off)
						lines += "   ^^^above 1-st line^^^";
				}
				break;
			}
		}
		if(length > 0)
			addEmptyLines(nppData._scintillaMainHandle, off - 1, length, lines.c_str());
	}

	//clean up resources
#if CLEANUP

	for (int i = 0; i < doc1Length; i++) {
		if (*doc1[i] != 0) {
			delete[] doc1[i];
		}
	}

	delete[] doc1;
	delete[] lineNum1;

	for (int i = 0; i < doc2Length; i++) {
		if (*doc2[i] != 0) {
			delete[] doc2[i];
		}
	}

	delete[] doc2;
	delete lineNum2;

	delete[] doc1Hashes;
	delete[] doc2Hashes;

	clearEdits(ses, sn);

	for (int i = 0; i < doc1Changed; i++) {
		clearEdit(doc1Changes + (i));
	}

	delete[] doc1Changes;

	for (int i = 0; i < doc2Changed; i++) {
		clearEdit(doc2Changes + (i));
	}

	delete[] doc2Changes;

#endif // CLEANUP

	if (!different) {
		//::MessageBox(nppData._nppHandle, TEXT("Files Match"), TEXT("Results :"), MB_OK);
		return 0;
	}

	//::SendMessageA(nppData._scintillaMainHandle, SCI_SHOWLINES, 0, (LPARAM)1);
	//::SendMessageA(nppData._scintillaSecondHandle, SCI_SHOWLINES, 0, (LPARAM)1);
	SCROLLINFO sci = {
		sizeof(sci), 0, 0, 0, 0, 0, 0
	};

	::SendMessage(::GetParent(nppData._scintillaMainHandle), SCI_SETSCROLLINFO, SB_VERT, (WPARAM)&sci);
	::SendMessage(::GetParent(nppData._scintillaSecondHandle), SCI_SETSCROLLINFO, SB_VERT, (WPARAM)&sci);
	}


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
