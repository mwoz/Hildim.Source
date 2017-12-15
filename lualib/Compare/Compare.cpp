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
NppData nppData;

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
		}
	}
	raise_error(L, "Compare internal error: invalid property");
	return -1;
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
		} else if (!lstrcmpA(name, "OldSymbols")) {
			lua_pushboolean(L, Settings.OldSymbols);
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
		} else if (!lstrcmpA(name, "OldSymbols")) {
			Settings.OldSymbols = lua_toboolean(L, 3);
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
	if (!lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
		return NULL;
	
	nppData._scintillaMainHandle = ::FindWindowEx((HWND)lua_touserdata(L, 1), NULL, NULL, NULL);
	nppData._scintillaSecondHandle = ::FindWindowEx((HWND)lua_touserdata(L, 2), NULL, NULL, NULL);

	::SendMessage(nppData._scintillaMainHandle, SCI_ANNOTATIONSETVISIBLE, 1, 0);
	::SendMessage(nppData._scintillaSecondHandle, SCI_ANNOTATIONSETVISIBLE, 1, 0);

	Settings.AddLine = true;
	Settings.DetectMove = true;
	Settings.IncludeSpace = true;
	Settings.OldSymbols = false;
	Settings.UseNavBar = false;

	Settings.ColorSettings.added = DEFAULT_ADDED_COLOR;
	Settings.ColorSettings.deleted = DEFAULT_DELETED_COLOR;
	Settings.ColorSettings.changed = DEFAULT_CHANGED_COLOR;
	Settings.ColorSettings.moved = DEFAULT_MOVED_COLOR;
	Settings.ColorSettings.blank = DEFAULT_BLANK_COLOR;
	Settings.ColorSettings.highlight = DEFAULT_HIGHLIGHT_COLOR;
	Settings.ColorSettings.alpha = DEFAULT_HIGHLIGHT_ALPHA;
	setStyles(Settings);
	return 0;
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
				markAsRemoved(nppData._scintillaMainHandle, doc1Changes[i].off);
				break;

			case DIFF_CHANGE1:
				markAsChanged(nppData._scintillaMainHandle, doc1Changes[i].off);
				textIndex = lineNum1[doc1Changes[i].off];

				for (int k = 0; k < doc1Changes[i].changeCount; k++) {
					struct diff_change *change = (diff_change*)varray_get(doc1Changes[i].changes, k);
					markTextAsChanged(nppData._scintillaMainHandle, textIndex + change->off, change->len);
				}
				break;

			case DIFF_MOVE:
				markAsMoved(nppData._scintillaMainHandle, doc1Changes[i].off);
				break;

			}
		}

		for (int i = 0; i < doc2Changed; i++) {
			switch (doc2Changes[i].op) {
			case DIFF_INSERT:
				markAsAdded(nppData._scintillaSecondHandle, doc2Changes[i].off);
				break;

			case DIFF_CHANGE2:
				markAsChanged(nppData._scintillaSecondHandle, doc2Changes[i].off);
				textIndex = lineNum2[doc2Changes[i].off];
				for (int k = 0; k < doc2Changes[i].changeCount; k++) {
					struct diff_change *change = (diff_change*)varray_get(doc2Changes[i].changes, k);
					markTextAsChanged(nppData._scintillaSecondHandle, textIndex + change->off, change->len);
				}
				break;

			case DIFF_MOVE:
				markAsMoved(nppData._scintillaSecondHandle, doc2Changes[i].off);
				break;
			}
		}

	if (Settings.AddLine) {
		int length = 0;
		int off = -1;
		resetPrevOffset();
		for (int i = 0; i < doc1Changed; i++) {
			switch (doc1Changes[i].op) {
			case DIFF_DELETE:
			case DIFF_MOVE:
				if (off < 0)
					off = doc1Changes[i].altLocation;
				if (doc1Changes[i].altLocation == off) {
					length++;
				} else {
					addEmptyLines(nppData._scintillaSecondHandle, off - 1, length);
					off = doc1Changes[i].altLocation;
					length = 1;
				}
				break;
			}
		}
		if (length > 0)
			addEmptyLines(nppData._scintillaSecondHandle, off - 1, length);

		length = 0;
		off = -1;
		resetPrevOffset();
		for (int i = 0; i < doc2Changed; i++) {
			switch (doc2Changes[i].op) {
			case DIFF_INSERT:
			case DIFF_MOVE:
				if (off < 0)
					off = doc2Changes[i].altLocation;
				if (doc2Changes[i].altLocation == off) {
					length++;
				} else {
					addEmptyLines(nppData._scintillaMainHandle, off - 1, length);
					off = doc2Changes[i].altLocation;
					length = 1;
				}
				break;
			}
		}
		if(length > 0)
			addEmptyLines(nppData._scintillaMainHandle, off - 1, length);
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

	::SendMessageA(nppData._scintillaMainHandle, SCI_SHOWLINES, 0, (LPARAM)1);
	::SendMessageA(nppData._scintillaSecondHandle, SCI_SHOWLINES, 0, (LPARAM)1);

	}


	return 0;
}

luaL_Reg compare[] =
{
	{ "Init", run_Init },
	{ "Compare", run_Compare },
	{ NULL, NULL }
};

extern "C" __declspec(dllexport) int luaopen_Compare(lua_State* L) {
	luaL_register(L, "Compare", compare);
	if (luaL_newmetatable(L, "Compare")) {
		lua_pushcfunction(L, cf_metatable_index);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	if (luaL_newmetatable(L, SETTINGOBJECT)) {
		lua_pushcfunction(L, cf_sett_metatable_index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, cf_sett_metatable_newindex);
		lua_setfield(L, -2, "__newindex");
	}
	return 1;
}
