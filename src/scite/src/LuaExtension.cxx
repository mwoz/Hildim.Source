// SciTE - Scintilla based Text Editor
// LuaExtension.cxx - Lua scripting extension
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include <string>

#include "Scintilla.h"

#include "GUI.h"
#include "SString.h"
#include "FilePath.h"
#include "Utf8_16.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "LuaExtension.h"
//#include "uiautomationclient.h"
//#include "winerror.h"

#include "IFaceTable.h"
#include "SciTEKeys.h"

#include <regex>
#include <map>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "iup.h"
#include "iuplua.h"
#include "iupcontrols.h"
#include "iupluacontrols.h"
#include "iupluaweb.h"
#include "iupluaole.h"
#include "..\..\iup\src\iup_key.h"
#include "scite_detachbox.h"
#include "scite_sbox.h"
#include "scite_flattabs.h"
#include "scite_scrollcanvas.h"
//#include "scite_images.h"
#include "../../iup/include/iup.h"
#include "../../iup/include/iuplua.h"
#include "../../iup/srccontrols/matrix/iupmat_getset.h"
#include "../../cd/include/cd.h"
#include "../../cd/include/cdlua.h"
#include "../../cd/include/cdluaiup.h"
}
 
using namespace Scintilla;

#if !defined(GTK)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
// MSVC looks deeper into the code than other compilers, sees that
// lua_error calls longjmp, and complains about unreachable code.
#pragma warning(disable: 4702)
#endif

#else

#include <limits.h>
#ifdef PATH_MAX
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 260
#endif

#endif


// A note on naming conventions:
// I've gone back and forth on this a bit, trying different styles.
// It isn't easy to get something that feels consistent, considering
// that the Lua API uses lower case, underscore-separated words and
// Scintilla of course uses mixed case with no underscores.

// What I've settled on is that functions that require you to think
// about the Lua stack are likely to be mixed with Lua API functions,
// so these should using a naming convention similar to Lua itself.
// Functions that don't manipulate Lua at a low level should follow
// the normal SciTE convention.  There is some grey area of course,
// and for these I just make a judgement call


static ExtensionAPI *host = 0;
static lua_State *luaState = 0;
static bool luaDisabled = false;

static char *startupScript = NULL;
static SString extensionScript;

static bool tracebackEnabled = true;

static int maxBufferIndex = -1;
static int curBufferIndex = -1;

static int GetPropertyInt(const char *propName) {
	int propVal = 0;
	if (host) {
		char *pszPropVal = host->Property(propName);
		if (pszPropVal) {
			propVal = atoi(pszPropVal);
			delete [] pszPropVal;
		}
	}
	return propVal;
}

LuaExtension::LuaExtension() {}

LuaExtension::~LuaExtension() {}

// Forward declarations
static ExtensionAPI::Pane check_pane_object(lua_State *L, int index);
static void push_pane_object(lua_State *L, ExtensionAPI::Pane p);
static int iface_function_helper(lua_State *L, const IFaceFunction &func, bool bSetter = false);

inline bool IFaceTypeIsScriptable(IFaceType t, int index) {
	return t < iface_stringresult || (index==1 && t == iface_stringresult);
}

inline bool IFaceTypeIsNumeric(IFaceType t) {
	return (t > iface_void && t < iface_bool);
}

inline bool IFaceFunctionIsScriptable(const IFaceFunction &f) {
	return IFaceTypeIsScriptable(f.paramType[0], 0) && IFaceTypeIsScriptable(f.paramType[1], 1);
}

inline bool IFacePropertyIsScriptable(const IFaceProperty &p) {
	return (((p.valueType > iface_void) && (p.valueType <= iface_stringresult) && (p.valueType != iface_keymod)) &&
		((p.paramType < iface_colour) || (p.paramType == iface_string) ||
		(p.paramType == iface_bool)) && (p.getter || p.setter));
}

inline void raise_error(lua_State *L, const char *errMsg=NULL) {
	luaL_where(L, 1);
	if (errMsg) {
		lua_pushstring(L, errMsg);
	} else {
		lua_insert(L, -2);
	}
	lua_concat(L, 2);
	lua_error(L);
}

inline int absolute_index(lua_State *L, int index) {
	if (index > LUA_REGISTRYINDEX && index < 0)
		return lua_gettop(L) + index + 1;
	else
		return index;
}

// copy the contents of one table into another returning the size
static int merge_table(lua_State *L, int destTableIdx, int srcTableIdx, bool copyMetatable = false) {
	int count = 0;
	if (lua_istable(L, destTableIdx) && lua_istable(L, srcTableIdx)) {
		srcTableIdx = absolute_index(L, srcTableIdx);
		destTableIdx = absolute_index(L, destTableIdx);
		if (copyMetatable) {
			lua_getmetatable(L, srcTableIdx);
			lua_setmetatable(L, destTableIdx);
		}
		lua_pushnil(L); // first key
		while (lua_next(L, srcTableIdx) != 0) {
			// key is at index -2 and value at index -1
			lua_pushvalue(L, -2); // the key
			lua_insert(L, -2); // leaving value (-1), key (-2), key (-3)
			lua_rawset(L, destTableIdx);
			++count;
		}
	}
	return count;
}

// make a copy of a table (not deep), leaving it at the top of the stack
static bool clone_table(lua_State *L, int srcTableIdx, bool copyMetatable = false) {
	if (lua_istable(L, srcTableIdx)) {
		srcTableIdx = absolute_index(L, srcTableIdx);
		lua_newtable(L);
		merge_table(L, -1, srcTableIdx, copyMetatable);
		return true;
	} else {
		lua_pushnil(L);
		return false;
	}
}

// loop through each key in the table and set its value to nil
static void clear_table(lua_State *L, int tableIdx, bool clearMetatable = true) {
	if (lua_istable(L, tableIdx)) {
		tableIdx = absolute_index(L, tableIdx);
		if (clearMetatable) {
			lua_pushnil(L);
			lua_setmetatable(L, tableIdx);
		}
		lua_pushnil(L); // first key
		while (lua_next(L, tableIdx) != 0) {
			// key is at index -2 and value at index -1
			lua_pop(L, 1); // discard value
			lua_pushnil(L);
			lua_rawset(L, tableIdx); // table[key] = nil
			lua_pushnil(L); // get 'new' first key
		}
	}
}

// Lua 5.1's checkudata throws an error on failure, we don't want that, we want NULL
static void *checkudata(lua_State *L, int ud, const char *tname) {
	void *p = lua_touserdata(L, ud);
	if (p != NULL) { // value is a userdata?
		if (lua_getmetatable(L, ud)) { // does it have a metatable?
			lua_getfield(L, LUA_REGISTRYINDEX, tname); // get correct metatable
			if (lua_rawequal(L, -1, -2)) { // does it have correct mt?
				lua_pop(L, 2);
				return p;
			}
		}
	}
	return NULL;
}

static int cf_scite_send(lua_State *L) {
	// This is reinstated as a replacement for the old <pane>:send, which was removed
	// due to safety concerns.  Is now exposed as scite.SendEditor / scite.SendOutput.
	// It is rewritten to be typesafe, checking the arguments against the metadata in
	// IFaceTable in the same way that the object interface does.

	int paneIndex = lua_upvalueindex(1);
	check_pane_object(L, paneIndex);
	int message = static_cast<int>(luaL_checkinteger(L, 1));

	lua_pushvalue(L, paneIndex);
	lua_replace(L, 1);

	IFaceFunction func = { "", 0, iface_void, {iface_void, iface_void} };
	for (int funcIdx = 0; funcIdx < IFaceTable::functionCount; ++funcIdx) {
		if (IFaceTable::functions[funcIdx].value == message) {
			func = IFaceTable::functions[funcIdx];
			break;
		}
	}

	if (func.value == 0) {
		for (int propIdx = 0; propIdx < IFaceTable::propertyCount; ++propIdx) {
			if (IFaceTable::properties[propIdx].getter == message) {
				func = IFaceTable::properties[propIdx].GetterFunction();
				break;
			} else if (IFaceTable::properties[propIdx].setter == message) {
				func = IFaceTable::properties[propIdx].SetterFunction();
				break;
			}
		}
	}

	if (func.value != 0) {
		if (IFaceFunctionIsScriptable(func)) {
			return iface_function_helper(L, func);
		} else {
			raise_error(L, "Cannot call send for this function: not scriptable.");
			return 0;
		}
	} else {
		raise_error(L, "Message number does not match any published Scintilla function or property");
		return 0;
	}
}

static int cf_scite_open(lua_State *L) {
	lua_pushboolean(L, host->Open_script(luaL_checkstring(L, 1)));
	return 1;
}

static int cf_scite_menu_command(lua_State *L) {
	int cmdID = static_cast<int>(luaL_checkinteger(L, 1));
	if (cmdID) {
		host->DoMenuCommand(cmdID);
	}
	return 0;
}

static int cf_scite_get_editor(lua_State *L) {
	lua_pushinteger(L, host->ActiveEditor());
	return 1;
}

static int cf_scite_save_encoding_file(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	const char *body = luaL_checkstring(L, 2);
	int encoding = static_cast<int>(lua_tointeger(L, 3));


	FilePath fPath = GUI::StringFromUTF8(path);
	std::string err;
	if (!fPath.Directory().IsDirectory()) {
		err = "SaveEncodingFile: Not found directory ";
		err += fPath.Directory().AsUTF8();
		goto errr;
	}
	{
		FILE* f = fPath.Open(fileWrite);

		if (f) {
			Utf8_16_Write convert;
			convert.setEncoding(static_cast<Utf8_16::encodingType>(encoding));
			convert.setfile(f);
			size_t written = convert.fwrite(body, lstrlenA(body));
			convert.fclose();
			if (written) {
				lua_pushboolean(L, true);
				return 1;
			}
			err = "SaveEncodingFile: Error when save ";
			err += fPath.AsUTF8();
		}
		err = "SaveEncodingFile: Can't open ";
		err += fPath.AsUTF8();
	}
errr:
	lua_pushboolean(L, false);
	lua_pushstring(L, err.c_str());
	return 2;

}


static int cf_scite_Read(lua_State *L) {
	FilePath filePath = GUI::StringFromUTF8(luaL_checkstring(L, 1));
	Utf8_16_Read* convert;
	char* data;
	size_t len;
	if (host->ReadForScript(filePath,(void**) &convert, &data, len)) {
		char* c = convert->getNewBuf();
		char* cc = new char[len + 1];
		memcpy((void*)cc, (void*)c, len);
		cc[len] = 0;
		lua_pushstring(L, cc);
		lua_pushnumber(L, convert->getEncoding()  );
		delete[] cc;
		delete convert;
		delete[] data;
		return 2;
	}

	return 0;	
}

// using http://www.easyrgb.com/index.php?X=MATH&H=01#text1
static int cf_scite_RGB2LAB(lua_State *L) {
	std::string err;
	float R, G, B, l_s, a_s, b_s;
	R = static_cast<float>(luaL_checkinteger(L, 1));
	G = static_cast<float>(luaL_checkinteger(L, 2));
	B = static_cast<float>(luaL_checkinteger(L, 3));
	if ((R < 0 || R > 255) || (G < 0 || G > 255) || (B < 0 || B > 255)) {
		err = "One from arguments (r,g,b) is negative or more then 255";
		goto errr;
	}

	rgb2lab(R, G, B, l_s, a_s, b_s);

	lua_pushnumber(L, l_s);
	lua_pushnumber(L, a_s);
	lua_pushnumber(L, b_s);
	return 3;
errr:
	lua_pushboolean(L, false);
	lua_pushstring(L, err.c_str());
	return 2;
}

static int cf_scite_InvertColor(lua_State* L) {
	lua_pushnumber(L, 0);
	if (host) {
		lua_pushnumber(L, host->InvertColor(static_cast<int>(luaL_checkinteger(L, 1))));
		return 1;
	}
	return 0;
}
static int cf_get_is64bit(lua_State* L) {
	lua_pushboolean(L, sizeof(size_t) == 8);
	return 1;
}

static int cf_switch_tabs(lua_State* L) {
	int side = static_cast<int>(luaL_checkinteger(L, 1));
	if (side < -1 || side > 1)
		luaL_argerror(L, 1, " allowed values: -1, 0, 1");
	bool forward = lua_toboolean(L, 2);
	host->UiDocTabulation(side, forward);
	return 0;
}

static int cf_get_keyboard_layout(lua_State* L) {
	
	char buff[KL_NAMELENGTH];
	if (GetKeyboardLayoutNameA(buff)) {
		lua_pushstring(L, buff);
		return 1;
	}
	return 0;
}

static int cf_scite_LAB2RGB(lua_State *L) {
	std::string err;
	float R, G, B, l_s, a_s, b_s;

	l_s = static_cast<float>(luaL_checknumber(L, 1));
	a_s = static_cast<float>(luaL_checknumber(L, 2));
	b_s = static_cast<float>(luaL_checknumber(L, 3));

	if (abs(a_s) > 128 || abs(b_s) > 128 ) {
		err = "One from arguments (a_s, b_s) is less then -128 or more then 128";
		goto errr;
	}
	if (abs(l_s - 50) > 50) {
		err = "One from arguments (a_s, b_s) is negative or more then 100";
		goto errr;
	}

	lab2rgb(l_s, a_s, b_s, R, G, B);

	lua_pushinteger(L, static_cast<lua_Integer>(R));
	lua_pushinteger(L, static_cast<lua_Integer>(G));
	lua_pushinteger(L, static_cast<lua_Integer>(B));
	return 3;
errr:
	lua_pushboolean(L, false);
	lua_pushstring(L, err.c_str());
	return 2;
}

static int cf_scite_compare_encoding_file(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	const char *body = luaL_checkstring(L, 2);
	int encoding = static_cast<int>(lua_tointeger(L, 3));


	FilePath fPath = GUI::StringFromUTF8(path);
	std::string err;
	if (!fPath.Directory().IsDirectory()) {
		err = "CompareEncodingFile: Not found directory ";
		err += fPath.Directory().AsUTF8();
		goto errr;
	}

	if (!fPath.Exists()) {
		err = "CompareEncodingFile: File not found ";
		err += fPath.Directory().AsUTF8();
		goto errr;
	}
	{
		int result = host->CompareFile(fPath, body);

		if (!result) {
			lua_pushboolean(L, true);
			lua_pushstring(L, err.c_str());
			return 2;
		}
		if (result == 2) {
			err = "CompareEncodingFile: Can't open ";
			err += fPath.AsUTF8();
		}
		else {
			err = "Different";
		}
	}
errr:
	lua_pushboolean(L, false);
	lua_pushstring(L, err.c_str());
	return 2;

}

//!-start-[LocalizationFromLua]
static int cf_editor_get_translation(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	char *r = NULL;
	if (lua_gettop(L) > 1) {
		r = host->GetTranslation(s, (lua_toboolean(L, 2) != 0));
	} else {
		r = host->GetTranslation(s);
	}
	lua_pushstring(L, r);
	delete []r;
	return 1;
}
//!-end-[LocalizationFromLua]

//!-start-[CheckMenus]
static int cf_scite_check_menus(lua_State *) {
	host->CheckMenus();
	return 0;
}
//!-end-[CheckMenus]

//!-start-[EncodingToLua]
static int cf_pane_get_codepage(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	UINT codePage = static_cast<UINT>(host->Send(p, Message::GetCodePage));
	if(codePage != SC_CP_UTF8) {
		unsigned int cs = SC_CHARSET_DEFAULT;
		char* charSet = host->Property("character.set");
		if(strcmp(charSet, "") != 0)
			cs = atoi(charSet);
		codePage = GUI::CodePageFromCharSet(cs, codePage);
	} else { // temporary solution
		unsigned int unimode = atoi(host->Property("editor.unicode.mode"));
		switch(unimode) {
			case 152: codePage = 1200; break; // UTF-16 LE
			case 151: codePage = 1201; break; // UTF-16 BE
			case 153: codePage = 65001; break; // UTF-8 BOM
			case 154: codePage = 65001; break; // UTF-8 without BOM
			default: codePage = SC_CP_UTF8;
		}
	}
	lua_pushinteger(L, codePage);
	return 1;
}

static int lua_string_from_utf8(lua_State *L) {
	int cp = 0;
	if (lua_gettop(L) == 1)
		cp = ::GetACP();
	const char *s = luaL_checkstring(L, 1);

	if (!cp) {
		if (!lua_isnumber(L, 2))
			cp = GUI::CodePageFromName(lua_tostring(L, 2));
		else
			cp = static_cast<int>(lua_tointeger(L, 2));
	}
	std::string ss = GUI::ConvertFromUTF8(s, cp);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_to_utf8(lua_State *L) {
	int cp = 0;
	if (lua_gettop(L) == 1)
		cp = ::GetACP();
	const char *s = luaL_checkstring(L, 1);
	if (!cp) {
		if (!lua_isnumber(L, 2))
			cp = GUI::CodePageFromName(lua_tostring(L, 2));
		else
			cp = static_cast<int>(lua_tointeger(L, 2));
	}
	std::string ss = GUI::ConvertToUTF8(s, cp);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_utf8_to_upper(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	std::string ss = GUI::UTF8ToUpper(s);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_utf8_to_lower(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	std::string ss = GUI::UTF8ToLower(s);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_utf8len(lua_State *L) {
	const char *str = luaL_checkstring(L, 1);
	GUI::gui_string wstr = GUI::StringFromUTF8(str);
	lua_pushinteger(L, wstr.length());
	return 1;
}
//!-end-[EncodingToLua]

static ExtensionAPI::Pane check_pane_object(lua_State *L, int index) {
	ExtensionAPI::Pane *pPane = reinterpret_cast<ExtensionAPI::Pane*>(checkudata(L, index, "SciTE_MT_Pane"));

	if ((!pPane) && lua_istable(L, index)) {
		// so that nested objects have a convenient way to do a back reference
		int absIndex = absolute_index(L, index);
		lua_pushliteral(L, "pane");
		lua_gettable(L, absIndex);
		pPane = reinterpret_cast<ExtensionAPI::Pane*>(checkudata(L, -1, "SciTE_MT_Pane"));
	}

	if (pPane) {
		if ((*pPane == ExtensionAPI::paneEditor) && (curBufferIndex < 0))
			raise_error(L, "Editor pane is not accessible at this time.");

		return *pPane;
	}

	if (index == 1)
		lua_pushliteral(L, "Self object is missing in pane method or property access.");
	else if (index == lua_upvalueindex(1))
		lua_pushliteral(L, "Internal error: pane object expected in closure.");
	else
		lua_pushliteral(L, "Pane object expected.");

	raise_error(L);
	return ExtensionAPI::paneOutput; // this line never reached
}

static int cf_pane_getU(lua_State* L, DWORD msg) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	if (lua_gettop(L) >= 2) {
		UINT cpPos = static_cast<UINT>(luaL_checknumber(L, 2));

		if (cpPos >= 0) {
			sptr_t st = host->Send(p, static_cast<Message>(msg), cpPos, cpPos);
			if (st < 0)
				st = st + 256;
			lua_pushinteger(L, st);
			return 1;
		}
		else {
			raise_error(L, "Invalid argument 1 for <pane>:getstyle.  Positive number or zero expected.");
		}
	}
	else {
		raise_error(L, "Not enough arguments for <pane>:getstyle");
	}
	return 0;
}

static int cf_pane_getstyle(lua_State *L) {
	return cf_pane_getU(L, SCI_GETSTYLEAT);
}
static int cf_pane_getchar(lua_State *L) {
	return cf_pane_getU(L, SCI_GETCHARAT);
}
static int cf_pane_getutf8text(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	int np = lua_gettop(L);
	int cp = static_cast<int>(host->Send(p, Message::GetCodePage, 0, 0));
	if (np == 1) {
		sptr_t size = host->Send(p, Message::GetLength, 0, 0);
		char* buff = new char[size + 1];
		host->Send(p, Message::GetText, size, reinterpret_cast<sptr_t>(buff));
		std::string ss = GUI::ConvertToUTF8(buff, cp);
		lua_pushstring(L, ss.c_str());
		delete[] buff;
		return 1;
	}
	else {
		const char* str = luaL_checkstring(L, 2);
		std::string ss = GUI::ConvertFromUTF8(str, cp);
		host->Send(p, Message::SetText, 0, reinterpret_cast<sptr_t>(ss.c_str()));
		return 0;
	}
}
static int cf_pane_line(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	if (lua_gettop(L) >= 2) {
		int cpLine = static_cast<int>(luaL_checknumber(L, 2));

		if (cpLine >= 0) {
			int bNeedEnd = 0;
			if (lua_gettop(L) >= 3){
				if(lua_isboolean(L, 3))
					bNeedEnd = lua_toboolean(L, 3);
				else
					raise_error(L, "Invalid argument 2 for <pane>:line.  Boolean expected.");
			}
			char *range = host->Line(p, cpLine, bNeedEnd);
			if (range) {
				lua_pushstring(L, range);
				delete[]range;
				return 1;
			}
		} else {
			raise_error(L, "Invalid argument 1 for <pane>:line.  Positive number or zero expected.");
		}
	} else {
		raise_error(L, "Not enough arguments for <pane>:line");
	}
	return 0;
}

static int cf_pane_textrange(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);

	if (lua_gettop(L) >= 3) {
		int cpMin = static_cast<int>(luaL_checknumber(L, 2));
		int cpMax = static_cast<int>(luaL_checknumber(L, 3));

		if (cpMax >= 0) {
			if(cpMax < cpMin){
				lua_pushstring(L, "");
			}else{
				char *range = host->Range(p, cpMin, cpMax);
				if (range) {
					lua_pushstring(L, range);
					delete []range;
					return 1;
				}
			}
		} else {
			raise_error(L, "Invalid argument 2 for <pane>:textrange.  Positive number or zero expected.");
		}
	} else {
		raise_error(L, "Not enough arguments for <pane>:textrange");
	}

	return 0;
}

static int cf_pane_insert(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	int pos = static_cast<int>(luaL_checkinteger(L, 2));
	const char *s = luaL_checkstring(L, 3);
	host->Insert(p, pos, s);
	return 0;
}

static int cf_pane_remove(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	int cpMin = static_cast<int>(luaL_checknumber(L, 2));
	int cpMax = static_cast<int>(luaL_checknumber(L, 3));
	host->Remove(p, cpMin, cpMax);
	return 0;
}

static int cf_pane_append(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	const char *s = luaL_checkstring(L, 2);
	host->Insert(p, host->Send(p, Message::GetLength, 0, 0), s);
	return 0;
}

static int cf_pane_eol_annotation_set(lua_State* L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);
	lua_Number line = luaL_checknumber(L, 2);

	switch (lua_type(L, 3)) {
	case LUA_TNIL:
		host->Send(p, Message::EOLAnnotationSetText, line, NULL);
		break;
	case LUA_TSTRING:
	{
		std::string ss = GUI::ConvertToUTF8(lua_tostring(L, 3), 0);
		host->Send(p, Message::EOLAnnotationSetText, line, reinterpret_cast<sptr_t>(ss.c_str()));
	}
		break;
	default:
		luaL_argerror(L, 3, "string or nil expected");
	}
	
	return 0;
}

static int cf_pane_findtext(lua_State *L) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);

	int nArgs = lua_gettop(L);

	const char *t = luaL_checkstring(L, 2);
	bool hasError = (!t);

	if (!hasError) {
		::Sci_TextToFind ft = {{0, 0}, 0, {0, 0}};

		ft.lpstrText = const_cast<char*>(t);

		int flags = (nArgs > 2) ? static_cast<int>(luaL_checkinteger(L, 3)) : 0;
		hasError = (flags == 0 && lua_gettop(L) > nArgs);

		if (!hasError) {
			if (nArgs > 3) {
				ft.chrg.cpMin = static_cast<int>(luaL_checkinteger(L,4));
				hasError = (lua_gettop(L) > nArgs);
			}
		}

		if (!hasError) {
			if (nArgs > 4) {
				ft.chrg.cpMax = static_cast<int>(luaL_checkinteger(L,5));
				hasError = (lua_gettop(L) > nArgs);
			} else {
				ft.chrg.cpMax = static_cast<Sci_PositionCR>(host->Send(p, Message::GetLength, 0, 0));
			}
		}

		if (!hasError) {
			sptr_t result = host->Send(p, Message::FindText, static_cast<uptr_t>(flags), reinterpret_cast<sptr_t>(&ft));
			if (result >= 0) {
				lua_pushinteger(L, ft.chrgText.cpMin);
				lua_pushinteger(L, ft.chrgText.cpMax);
				return 2;
			} else {
				lua_pushnil(L);
				return 1;
			}
		}
	}

	if (hasError) {
		raise_error(L, "Invalid arguments for <pane>:findtext");
	}

	return 0;
}

static int cf_Reg_HotKey(lua_State* L){
	int count = 0;

	if (lua_istable(L, 1)){
		int i = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0) {
			++count;
			lua_pop(L, 1);
		}
		LPACCEL  acc = (LPACCEL)LocalAlloc(LPTR, count * sizeof(ACCEL));
		lua_pushnil(L);
		while (lua_next(L, 1) != 0) {
			// key is at index -2 and value at index -1
			const char* ccc = luaL_checkstring(L, -2);
			int kk = luaL_checkinteger(L, -1);
			SciTEKeys::FillAccel((void*)(acc + i), luaL_checkstring(L, -2), static_cast<int>(luaL_checkinteger(L, -1)));
			lua_pop(L, 1);
			i++;
		}
		host->SetAcceleratorTable((void*) ::CreateAcceleratorTable(acc, count));
		//delete acc;
	}
	return 1;
}

static int sf_findfiles(lua_State* L) {
	WIN32_FIND_DATAW findFileData;
	HANDLE hFind = ::FindFirstFileW(GUI::StringFromUTF8(luaL_checkstring(L, 1)).c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		// create table for result
		lua_createtable(L, 1, 0);

		lua_Integer num = 1;
		BOOL isFound = TRUE;
		while (isFound != FALSE) {
			// store file info
			lua_pushinteger(L, num);
			lua_createtable(L, 0, 4);

			lua_pushstring(L, GUI::UTF8FromString(findFileData.cFileName).c_str());
			lua_setfield(L, -2, "name");

			lua_pushboolean(L, findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			lua_setfield(L, -2, "isdirectory");

			lua_pushinteger(L, findFileData.dwFileAttributes);
			lua_setfield(L, -2, "attributes");

			lua_pushinteger(L, findFileData.nFileSizeHigh * ((lua_Integer)MAXDWORD + 1) +
				findFileData.nFileSizeLow);
			lua_setfield(L, -2, "size");


			__int64 t = findFileData.ftLastWriteTime.dwLowDateTime + ((__int64)findFileData.ftLastWriteTime.dwHighDateTime << 32);
			lua_pushnumber(L, (lua_Number)t);
			lua_setfield(L, -2, "writetime");

			lua_settable(L, -3);
			num++;

			// next
			isFound = ::FindNextFileW(hFind, &findFileData);
		}

		::FindClose(hFind);

		return 1;
	}

	// files not found
	return 0;
}


static int sf_IsRunAsAdmin(lua_State* L) {
	BOOL res = host->IsRunAsAdmin();
	lua_pushboolean(L, res);
	return 1;
}
static int sf_NewInstance(lua_State* L) {
	BOOL res = host->NewInstance(luaL_checkstring(L, 1), luaL_checkinteger(L, 2));
	lua_pushboolean(L, res);
	return 1;
}
static int cf_EnsureVisible(lua_State* L){
	host->EnsureVisible();
	return 0;
}

static int cf_HideForeReolad(lua_State* L){
	int close =0;
	if(lua_isinteger(L, 1))
		close = static_cast<int>(luaL_checkinteger(L, 1));
	host->HideForeReolad(close);
	return 0;
}

static int sf_SwitchMouseHook(lua_State* L){
	bool bSet = lua_toboolean(L, 1);
	bSet = host->SwitchMouseHook(bSet);
	lua_pushboolean(L, bSet);
	return 1;
}

static int sf_RunInConcole(lua_State* L) {
	host->RunInConcole();
	return 0;
}

static int sf_Close(lua_State* L) {
	host->Close_script();
	return 0;
}

static int sf_SavePosition(lua_State* L) {
	host->SavePositions();
	return 0;
}

static int sf_BlockUpdate(lua_State* L) {
	int cmd = static_cast<int>(luaL_checkinteger(L, 1));
	host->BlockUpdate(cmd);
	return 0;
}

static int cf_editor_set_foreground(lua_State* L) {

	HWND hChild = NULL;
	hChild = ::FindWindowEx(NULL, hChild, L"HildiMWindow", NULL);
	WINDOWPLACEMENT placement{};
	placement.length = sizeof(placement);
	
	if (!GetWindowPlacement(hChild, &placement)) {
		lua_pushstring(L, "Failed to window placement!");
		return 1;
	}	
		
	
	
	bool minimized = placement.showCmd == SW_SHOWMINIMIZED;
	if (minimized) {
		ShowWindow(hChild, SW_RESTORE);
		placement.showCmd = SW_RESTORE;
	}
	HWND hCurrWnd;
	int iMyTID;
	int iCurrTID;
	const char* er = NULL;

	hCurrWnd = ::GetForegroundWindow();
	if (!hCurrWnd) {
		er = "Coud not GetForegroundWindow";
		goto error;
	}
	iMyTID = GetCurrentThreadId();
	iCurrTID = GetWindowThreadProcessId(hCurrWnd, 0);
	if (!iCurrTID){
		er = "Coud not GetWindowThreadProcessId";
		goto error;
	}

	BOOL b = AttachThreadInput(iMyTID, iCurrTID, TRUE);
	if (!iCurrTID){
		er = "Coud not AttachThreadInput";
		goto error;
	}

	// hWnd - ���������� ����.
	int i;
	for (i = 0; i < 10 && hChild != ::GetForegroundWindow(); i++) {
		Sleep(2);
		SwitchToThisWindow(hChild, true);
	}
	if (!er && hChild != ::GetForegroundWindow())
		er = "Coud not SwitchToThisWindow";

	AttachThreadInput(iMyTID, iCurrTID, FALSE);

	
error:
	if (er) {
		lua_pushstring(L, er);
		return 1;
	}
	return 0;
}

static int sf_SetOverrideLanguage(lua_State* L){
	const char * lexer = luaL_checkstring(L, 1);
	host->SetOverrideLanguage(lexer, true);
	return 0;
}

static int sf_ExecuteHelp(lua_State* L){
	const char * cmd = luaL_checkstring(L, 1);
	int hh_cmd = static_cast<int>(luaL_checkinteger(L, 2));
	host->ExecuteHelp(cmd, hh_cmd);
	return 0;
}

static int sf_CheckRegexp(lua_State* L){
	const char * s = luaL_checkstring(L, 1);
	try{
		//ElapsedTime et;
		std::regex::flag_type flagsRe = std::regex::ECMAScript;

		bool matched = false;

		std::regex regexp;
		regexp.assign(s, flagsRe);

	}
	catch (std::regex_error & rerr) {
		// Failed to create regular expression
		//throw RegexError();
		lua_pushstring(L, rerr.what());
		return 1;
	}
	catch (...) {
		// Failed in some other way
		lua_pushstring(L, "No Regex Error");
		return 1;
	}
	return 0;
}

// Pane match generator.  This was prototyped in about 30 lines of Lua.
// I hope the C++ version is more robust at least, e.g. prevents infinite
// loops and is more tamper-resistant.

struct PaneMatchObject {
	ExtensionAPI::Pane pane;
	Sci_Position startPos;
	Sci_Position endPos;
	int flags; // this is really part of the state, but is kept here for convenience
	Sci_Position endPosOrig; // has to do with preventing infinite loop on a 0-length match
};

static int cf_match_replace(lua_State *L) {
	PaneMatchObject *pmo = reinterpret_cast<PaneMatchObject*>(checkudata(L, 1, "SciTE_MT_PaneMatchObject"));
	if (!pmo) {
		raise_error(L, "Self argument for match:replace() should be a pane match object.");
		return 0;
	} else if ((pmo->startPos < 0) || (pmo->endPos < pmo->startPos) || (pmo->endPos < 0)) {
		raise_error(L, "Blocked attempt to use invalidated pane match object.");
		return 0;
	}
	const char *replacement = luaL_checkstring(L, 2);

	// If an option were added to process \d back-references, it would just
	// be an optional boolean argument, i.e. m:replace([[\1]], true), and
	// this would just change SCI_REPLACETARGET to SCI_REPLACETARGETRE.
	// The problem is, even if SCFIND_REGEXP was used, it's hard to know
	// whether the back references are still valid.  So for now this is
	// left out.

	host->Send(pmo->pane, Message::SetTargetStart, pmo->startPos, 0);
	host->Send(pmo->pane, Message::SetTargetEnd, pmo->endPos, 0);
	host->Send(pmo->pane, Message::ReplaceTarget, static_cast<uptr_t>(lua_rawlen(L, 2)), reinterpret_cast<sptr_t>(replacement));
	pmo->endPos = host->Send(pmo->pane, Message::SetTargetEnd, 0, 0);
	return 0;
}

static int cf_match_metatable_index(lua_State *L) {
	PaneMatchObject *pmo = reinterpret_cast<PaneMatchObject*>(checkudata(L, 1, "SciTE_MT_PaneMatchObject"));
	if (!pmo) {
		raise_error(L, "Internal error: pane match object is missing.");
		return 0;
	} else if ((pmo->startPos < 0) || (pmo->endPos < pmo->startPos) || (pmo->endPos < 0)) {
		raise_error(L, "Blocked attempt to use invalidated pane match object.");
		return 0;
	}

	if (lua_isstring(L, 2)) {
		const char *key = lua_tostring(L, 2);

		if (0 == strcmp(key, "pos")) {
			lua_pushinteger(L, pmo->startPos);
			return 1;
		} else if (0 == strcmp(key, "len")) {
			lua_pushinteger(L, pmo->endPos - pmo->startPos); 
			return 1;
		} else if (0 == strcmp(key, "text")) {
			// If the document is changed while in the match loop, this will be broken. 
			// Exception: if the changes are made exclusively through match:replace,
			// everything will be fine.
			char *range = host->Range(pmo->pane, pmo->startPos, pmo->endPos);
			if (range) {
				lua_pushstring(L, range);
				delete []range;
				return 1;
			} else {
				return 0;
			}
		} else if (0 == strcmp(key, "replace")) {
			int replaceMethodIndex = lua_upvalueindex(1);
			if (lua_iscfunction(L, replaceMethodIndex)) {
				lua_pushvalue(L, replaceMethodIndex);
				return 1;
			} else {
				return 0;
			}
		}
	}

	raise_error(L, "Invalid property / method name for pane match object.");
	return 0;
}

static int cf_match_metatable_tostring(lua_State *L) {
	PaneMatchObject *pmo = reinterpret_cast<PaneMatchObject*>(checkudata(L, 1, "SciTE_MT_PaneMatchObject"));
	if (!pmo) {
		raise_error(L, "Internal error: pane match object is missing.");
		return 0;
	} else if ((pmo->startPos < 0) || (pmo->endPos < pmo->startPos) || (pmo->endPos < 0)) {
		lua_pushliteral(L, "match(invalidated)");
		return 1;
	} else {
		lua_pushfstring(L, "match{pos=%d,len=%d}", pmo->startPos, pmo->endPos - pmo->startPos);
		return 1;
	}
}

static int cf_pane_match(lua_State *L) {
	int nargs = lua_gettop(L);

	ExtensionAPI::Pane p = check_pane_object(L, 1);
	luaL_checkstring(L, 2);

	int generatorIndex = lua_upvalueindex(1);
	if (!lua_isfunction(L, generatorIndex)) {
		raise_error(L, "Internal error: match generator is missing.");
		return 0;
	}

	lua_pushvalue(L, generatorIndex);

	// I'm putting some of the state in the match userdata for more convenient
	// access.  But, the search string is going in state because that part is
	// more convenient to leave in Lua form.
	lua_pushvalue(L, 2);

	PaneMatchObject *pmo = reinterpret_cast<PaneMatchObject*>(lua_newuserdata(L, sizeof(PaneMatchObject)));
	if (pmo) {
		pmo->pane = p;
		pmo->startPos = -1;
		pmo->endPos = pmo->endPosOrig = 0;
		pmo->flags = 0;
		if (nargs >= 3) {
			pmo->flags = static_cast<int>(luaL_checkinteger(L, 3));
			if (nargs >= 4) {
				pmo->endPos = pmo->endPosOrig = static_cast<long>(luaL_checkinteger(L, 4));
				if (pmo->endPos < 0) {
					raise_error(L, "Invalid argument 3 for <pane>:match.  Positive number or zero expected.");
					return 0;
				}
			}
		}
		if (luaL_newmetatable(L, "SciTE_MT_PaneMatchObject")) {
			lua_pushliteral(L, "__index");
			lua_pushcfunction(L, cf_match_replace);
			lua_pushcclosure(L, cf_match_metatable_index, 1);
			lua_settable(L, -3);

			lua_pushliteral(L, "__tostring");
			lua_pushcfunction(L, cf_match_metatable_tostring);
			lua_settable(L, -3);
		}
		lua_setmetatable(L, -2);

		return 3;
	} else {
		raise_error(L, "Internal error: could not create match object.");
		return 0;
	}
}

static int cf_pane_match_generator(lua_State *L) {
	const char *text = lua_tostring(L, 1);
	PaneMatchObject *pmo = reinterpret_cast<PaneMatchObject*>(checkudata(L, 2, "SciTE_MT_PaneMatchObject"));

	if (!(text)) {
		raise_error(L, "Internal error: invalid state for <pane>:match generator.");
		return 0;
	} else if (!pmo) {
		raise_error(L, "Internal error: invalid match object initializer for <pane>:match generator");
		return 0;
	}

	if ((pmo->endPos < 0) || (pmo->endPos < pmo->startPos)) {
		raise_error(L, "Blocked attempt to use invalidated pane match object.");  
		return 0;
	}

	Sci_Position searchPos = pmo->endPos;
	if ((pmo->startPos == pmo->endPosOrig) && (pmo->endPos == pmo->endPosOrig)) {
		// prevent infinite loop on zero-length match by stepping forward
		searchPos++;
	}

	::Sci_TextToFind ft = { {0,0}, 0, {0,0} };
	ft.chrg.cpMin = static_cast<Sci_PositionCR>(searchPos);
	ft.chrg.cpMax = static_cast<Sci_PositionCR>(host->Send(pmo->pane, Message::GetLength, 0, 0));
	ft.lpstrText = const_cast<char*>(text);

	if (ft.chrg.cpMax > ft.chrg.cpMin) {
		sptr_t result = host->Send(pmo->pane, Message::FindText, static_cast<uptr_t>(pmo->flags), reinterpret_cast<sptr_t>(&ft));
		if (result >= 0) {
			pmo->startPos = ft.chrgText.cpMin;
			pmo->endPos = pmo->endPosOrig = ft.chrgText.cpMax; 
			lua_pushvalue(L, 2);
			return 1;
		}
	}

	// One match object is used throughout the entire iteration.
	// This means it's bad to try to save the match object for later
	// reference.
	pmo->startPos = pmo->endPos = pmo->endPosOrig = -1;
	lua_pushnil(L);
	return 1;
}

static int cf_props_metatable_index(lua_State *L) {
	int selfArg = lua_isuserdata(L, 1) ? 1 : 0;

	if (lua_isstring(L, selfArg + 1)) {
		char *value = host->Property(lua_tostring(L, selfArg + 1));
		if (value) {
			lua_pushstring(L, value);
			delete []value;
			return 1;
		} else {
			lua_pushliteral(L, "");
			return 1;
		}
	} else {
		raise_error(L, "String argument required for property access");
	}
	return 0;
}

static int cf_props_metatable_newindex(lua_State *L) {
	int selfArg = lua_isuserdata(L, 1) ? 1 : 0;

	const char *key = lua_isstring(L, selfArg + 1) ? lua_tostring(L, selfArg + 1) : 0;
	const char *val = lua_tostring(L, selfArg + 2);

	if (key && *key) {
		if (val) {
			host->SetProperty(key, val);
		} else if (lua_isnil(L, selfArg + 2)) {
			host->UnsetProperty(key);
		} else {
			raise_error(L, "Expected string or nil for property assignment.");
		}
	} else {
		raise_error(L, "Property name must be a non-empty string.");
	}
	return 0;
}

/*
static int cf_os_execute(lua_State *L) {
	// The SciTE version of os.execute would pipe its stdout and stderr
	// to the output pane.  This can be implemented in terms of popen
	// on GTK and in terms of CreateProcess on Windows.  Either way,
	// stdin should be null, and the Lua script should wait for the
	// subprocess to finish before continuing.  (What if it takes
	// a very long time?  Timeout?)

	raise_error(L, "Not implemented.");
	return 0;
}
*/

static int cf_global_print(lua_State *L) {
	int nargs = lua_gettop(L);

	lua_getglobal(L, "tostring");

	for (int i = 1; i <= nargs; ++i) {
		if (i > 1)
			host->Trace("\t");

		const char *argStr = lua_tostring(L, i);
		if (argStr) {
			host->Trace(argStr);
		} else {
			lua_pushvalue(L, -1); // tostring
			lua_pushvalue(L, i);
			lua_call(L, 1, 1);
			argStr = lua_tostring(L, -1);
			if (argStr) {
				host->Trace(argStr);
			} else {
				raise_error(L, "tostring (called from print) returned a non-string");
			}
			lua_settop(L, nargs + 1);
		}
	}

	host->Trace("\n");
	return 0;
}

static int cf_perform_grep_ex(lua_State *L){
	const char *p = luaL_checkstring(L, 1);
	const char *w = luaL_checkstring(L, 2);
	const char *d = luaL_checkstring(L, 3);
	const char *f = luaL_checkstring(L, 4);
	host->PerformGrepEx(p, w, d, f);
	return 1;
}

static int bf_current(lua_State *L) {

	lua_pushinteger(L, curBufferIndex < 0 ? -1 : host->GetCurrentBufer());
	return 1;
}

static int bf_get_buffer_side(lua_State *L) {

	lua_pushinteger(L, curBufferIndex < 0 ? 0 : host->GetBufferSide(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}
static int bf_get_buffer_order(lua_State *L) {

	lua_pushinteger(L, host->GetBufferOrder(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}
static int bf_get_buffer_modtime(lua_State *L) {

	lua_pushinteger(L, host->GetBufferModTime(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}

static int bf_get_buffer_unicmode(lua_State *L) {
	
	lua_pushinteger(L, host->GetBufferUnicMode(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}

static int bf_get_buffer_redonly(lua_State *L) {

	lua_pushboolean(L, host->GetBufferReadOnly(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}

static int bf_is_cloned(lua_State *L) {
	lua_pushinteger(L, curBufferIndex < 0 ? 0 : host->Cloned(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}
static int bf_index_of_clone(lua_State *L) {
	lua_pushinteger(L, curBufferIndex < 0 ? -1 : host->IndexOfClone(static_cast<int>(luaL_checkinteger(L, 1))));
	return 1;
}
static int bf_buffer_by_name(lua_State *L) {
	lua_pushinteger(L, curBufferIndex < 0 ? -1 : host->BufferByName(luaL_checkstring(L, 1)));
	return 1;
}
static int bf_second_editor_active(lua_State *L) {

	lua_pushinteger(L, curBufferIndex < 0 ? false : host->SecondEditorActive());
	return 1;
}

static int bf_get_count(lua_State *L){
	lua_pushinteger(L, host->GetBuffersCount());
	return 1;
}

static int bf_get_buffer_name(lua_State *L){
	int i = static_cast<int>(luaL_checkinteger(L, 1));
	lua_pushstring(L, host->GetBufferName(i).c_str());
	return 1;
}

static int bf_save_cobuffer_pos(lua_State *L){
	host->UpdateBuffersCoCurrent();
	return 1;
}

static int bf_get_cobuffer_name(lua_State *L){
	lua_pushstring(L, host->GetCoBufferName().c_str());
	return 1;
}

static int bf_get_buffer_saved(lua_State *L){
	int i = static_cast<int>(luaL_checkinteger(L, 1));
	lua_pushboolean(L, host->GetBuffersSavedState(i));
	return 1;
}

static int bf_get_buffer_encoding(lua_State *L) {
	int i = static_cast<int>(luaL_checkinteger(L, 1));
	lua_pushinteger(L, host->GetBufferEncoding(i));
	return 1;
}

static int bf_set_buffer_encoding(lua_State *L) {
	int i = static_cast<int>(luaL_checkinteger(L, 1));
	int e = static_cast<int>(luaL_checkinteger(L, 2));
	host->SetBufferEncoding(i, e);
	return 0;
}

static int bf_get_filetime(lua_State *L) {
	int i = static_cast<int>(luaL_checkinteger(L, 1));
	lua_pushinteger(L, host->GetBufferFileTime(i));
	return 1;
}

static int bf_set_filetime(lua_State *L) {
	int i = static_cast<int>(luaL_checkinteger(L, 1));
	host->ClearBufferFileTime(i);
	return 0;
}



static int cf_set_co_document_at(lua_State* L) {
	int index = static_cast<int>(luaL_checkinteger(L, 1));
	host->SetCoDocumentAt(index);
	return 0;
}

static int cf_set_document_at(lua_State *L){
	int index = static_cast<int>(luaL_checkinteger(L, 1));
	int updateStack = 1;
	if (lua_isboolean(L, 2))
		updateStack = lua_toboolean(L, 2);
	int switchTab = 1;
	if (lua_isboolean(L, 3))
		switchTab = lua_toboolean(L, 3);
	host->SetDocumentAt(index, updateStack == 1, switchTab == 1);
	return 0;
}

static int cf_global_trace(lua_State *L) {
	const char *s = lua_tostring(L,1);
	if (s) {
		host->Trace(s);
	}
	return 0;
}

static int cf_global_dostring(lua_State *L) {
	int nargs = lua_gettop(L);
	const char *code = luaL_checkstring(L, 1);
	const char *name = luaL_optstring(L, 2, code);
	if (0 == luaL_loadbuffer(L, code, static_cast<size_t>(lua_rawlen(L, 1)), name)) {
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L) - nargs;
	} else {
		raise_error(L);
	}
	return 0;
}
static void procrssError(lua_State* L, int result) {
	if (result == LUA_ERRRUN) {
		lua_getglobal(L, "print");
		lua_insert(L, -2); // use pushed error message
		lua_pcall(L, 1, 0, 0);
	}
	else {
		lua_pop(L, 1);
		if (result == LUA_ERRMEM) {
			host->Trace("> Lua: memory allocation error\n");
		}
		else if (result == LUA_ERRERR) {
			host->Trace("> Lua: an error occurred, but cannot be reported due to failure in _TRACEBACK\n");
		}
		else {
			host->Trace("> Lua: unexpected error\n");
		}
	}
}

static bool call_function(lua_State *L, int nargs, int nresult = 1) {
	bool handled = false;
	if (L) {
		int traceback = 0;
		if (tracebackEnabled) {
			lua_getglobal(L, "debug");
			lua_getfield(L, -1, "traceback");
			lua_remove(L, -2);
			if (lua_isfunction(L, -1)) {
				traceback = lua_gettop(L) - nargs - 1;
				lua_insert(L, traceback);
			} else {
				lua_pop(L, 1);
			}
		}

		int result = lua_pcall(L, nargs, nresult, traceback);
		if (traceback) {
			lua_remove(L, traceback);
		}

		if (0 == result) {
			if (!nresult) {
				handled = true;
			} else if (nresult == 1){
				handled = (0 != lua_toboolean(L, -1));
				lua_pop(L, 1);
			}
		} else {
			procrssError(L, result);
		}
	}
	return handled;
}

static int sf_RunAsync(lua_State* L) {
	int idx = luaL_ref(L, LUA_REGISTRYINDEX);
	host->RunAsync(idx);
	return 0;
}

static int sf_SetRestart(lua_State* L) {
	const char *cmdline = luaL_checkstring(L, 1);
	host->SetRestart(cmdline);
	return 0;
}

static void sf_FileVersionInfo_value(lua_State* L, BYTE* pbVersionInfo, const char* field) {
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;
	GUI::gui_string block = L"\\StringFileInfo\\%04x%04x\\";
	block += GUI::StringFromUTF8(field);
	UINT len;
	if (VerQueryValue(pbVersionInfo,
		TEXT("\\VarFileInfo\\Translation"),
		(LPVOID*)&lpTranslate,
		&len)) {
		TCHAR SubBlock[128];
		swprintf(SubBlock, 128, block.c_str(),
			lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
		LPTSTR ret;
		if (VerQueryValue(pbVersionInfo, SubBlock, (LPVOID*)&ret, &len) && len>0) {
			GUI::gui_string s = ret;
			lua_pushstring(L, GUI::UTF8FromString(s).c_str());
			lua_setfield(L, -2, field);
		}
	}
}

static int sf_OrderTab(lua_State* L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	std::map<int, int> order;
	lua_pushnil(L);  /* first key (������ ����) */
	while (lua_next(L, 1) != 0) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		/* ���������� 'key' (�� ������� -2) � 'value' (�� ������� -1) */
		int key = static_cast<int>(luaL_checkinteger(L, -2));
		int val = static_cast<int>(luaL_checkinteger(L, -1));
		order.insert({ key, val });

		/* removes 'value'; keeps 'key' for next iteration */
		/* ������� 'value'; �������� 'key' ��� ��������� �������� */
		lua_pop(L, 1);
	}
	host->OrderTabsBy(order);
	return 0;
}

static int sf_FileVersionInfo(lua_State* L) {
	DWORD               dwSize = 0;
	BYTE                *pbVersionInfo = NULL;
	VS_FIXEDFILEINFO    *pFileInfo = NULL;
	UINT                puLenFileInfo = 0;
	GUI::gui_string path = GUI::StringFromUTF8(luaL_checkstring(L, 1));

	// Get the version information for the file requested
	dwSize = GetFileVersionInfoSizeEx(FILE_VER_GET_NEUTRAL | FILE_VER_GET_LOCALISED, path.c_str(), NULL);
	if (dwSize == 0) {
		printf("Error in GetFileVersionInfoSize: %d\n", GetLastError());
		return 0;
	}

	pbVersionInfo = new BYTE[dwSize];

	if (!GetFileVersionInfoEx(FILE_VER_GET_NEUTRAL | FILE_VER_GET_LOCALISED, path.c_str(), 0, dwSize, pbVersionInfo)) {
		printf("Error in GetFileVersionInfo: %d\n", GetLastError());
		delete[] pbVersionInfo;
		return 0;
	}

	if (!VerQueryValue(pbVersionInfo, TEXT("\\"), (LPVOID*)&pFileInfo, &puLenFileInfo)) {
		printf("Error in VerQueryValue: %d\n", GetLastError());
		delete[] pbVersionInfo;
		return 0;
	}
	lua_createtable(L, 1, 0);
	 //pFileInfo->dwFileVersionMS is usually zero. However, you should check
	// this if your version numbers seem to be wrong


	lua_pushnumber(L, (pFileInfo->dwFileVersionMS >> 16) & 0xff);
	lua_setfield(L, -2, "FVMS1");
	lua_pushnumber(L, (pFileInfo->dwFileVersionMS >> 0) & 0xff);
	lua_setfield(L, -2, "FVMS2");
	lua_pushnumber(L, (pFileInfo->dwFileVersionLS >> 16) & 0xff);
	lua_setfield(L, -2, "FVLS1");
	lua_pushnumber(L, (pFileInfo->dwFileVersionLS >> 0) & 0xff);
	lua_setfield(L, -2, "FVLS2");
	lua_pushnumber(L, (pFileInfo->dwProductVersionMS >> 16) & 0xff);
	lua_setfield(L, -2, "PVMS1");
	lua_pushnumber(L, (pFileInfo->dwProductVersionMS >> 0) & 0xff);
	lua_setfield(L, -2, "PVMS2");
	lua_pushnumber(L, (pFileInfo->dwProductVersionLS >> 16) & 0xff);
	lua_setfield(L, -2, "PVLS1");
	lua_pushnumber(L, (pFileInfo->dwProductVersionLS >> 0) & 0xff);
	lua_setfield(L, -2, "PVLS2");

	sf_FileVersionInfo_value(L, pbVersionInfo, "ProductVersion");
	sf_FileVersionInfo_value(L, pbVersionInfo, "CompanyName");
	sf_FileVersionInfo_value(L, pbVersionInfo, "FileDescription");
	sf_FileVersionInfo_value(L, pbVersionInfo, "FileVersion");
	sf_FileVersionInfo_value(L, pbVersionInfo, "InternalName");
	sf_FileVersionInfo_value(L, pbVersionInfo, "LegalCopyright");
	sf_FileVersionInfo_value(L, pbVersionInfo, "OriginalFilename");
	sf_FileVersionInfo_value(L, pbVersionInfo, "ProductName");
	sf_FileVersionInfo_value(L, pbVersionInfo, "IUPVersion");

	lua_pushstring(L, sizeof(intptr_t)==4 ? "32":"64");
	lua_setfield(L, -2, "BITS");
	
	delete[] pbVersionInfo;
	return 1;
}

bool LuaExtension::OnPostCallback(int idx) {
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, idx);
	call_function(luaState, 0);
	luaL_unref(luaState, LUA_REGISTRYINDEX, idx);
	return false;
}

//!-start-[macro] [OnSendEditor]
static const char *call_sfunction(lua_State *L, int nargs, bool ignoreFunctionReturnValue=false) {
	const char *handled = NULL;
	if (L) {
		int traceback = 0;
		if (tracebackEnabled) {
			lua_getglobal(L, "print");
			if (lua_isfunction(L, -1)) {
				traceback = lua_gettop(L) - nargs - 1;
				lua_insert(L, traceback);
			} else {
				lua_pop(L, 1);
			}
		}

		int result = lua_pcall(L, nargs, ignoreFunctionReturnValue ? 0 : 1, traceback);

		if (traceback) {
			lua_remove(L, traceback);
		}

		if (0 == result) {
			if (ignoreFunctionReturnValue) {
				handled = "";
			} else {
				handled = lua_tostring(L, -1);
				lua_pop(L, 1);
			}
		} else {
			procrssError(L, result);
		}
	}
	return handled;
}
//!-end-[macro] [OnSendEditor]

static bool CallNamedFunction(const char *name) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			handled = call_function(luaState, 0);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static bool CallNamedFunction(const char *name, const char *arg) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushstring(luaState, arg);
			handled = call_function(luaState, 1);
		} else {
			lua_pop(luaState, 1);
	}	
	}
	return handled;
}

static bool CallNamedFunction(const char *name, LUA_INTEGER numberArg, const char *stringArg) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushstring(luaState, stringArg);
			handled = call_function(luaState, 2);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static bool CallNamedFunction(const char *name, LUA_INTEGER numberArg, const char *stringArg, LUA_INTEGER numberArg2, LUA_INTEGER numberArg3 ) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushstring(luaState, stringArg);
			lua_pushinteger(luaState, numberArg2);
			lua_pushinteger(luaState, numberArg3);
			handled = call_function(luaState, 4);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

//!-start-[macro]
static bool CallNamedFunction(const char *name, const char *stringArg, const char *stringArg2) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushstring(luaState, stringArg);
			lua_pushstring(luaState, stringArg2);
			handled = call_function(luaState, 2);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}
//!-end-[macro]

//!-start-[OnMenuCommand]
static bool CallNamedFunction(const char *name, LUA_INTEGER numberArg, LUA_INTEGER numberArg2) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushinteger(luaState, numberArg2);
			handled = call_function(luaState, 2);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static const char *CallNamedFunction(const char *name, LUA_INTEGER numberArg, LUA_INTEGER numberArg2, const char *stringArg) {
	const char *handled = NULL;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushinteger(luaState, numberArg2);
			lua_pushstring(luaState, stringArg);
			handled = call_sfunction(luaState, 3);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static const char *CallNamedFunction(const char *name, LUA_INTEGER numberArg, const char *stringArg, int numberArg2) {
	const char *handled = NULL;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushstring(luaState, stringArg);
			lua_pushinteger(luaState, numberArg2);
			handled = call_sfunction(luaState, 3);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static const char *CallNamedFunction(const char *name, LUA_INTEGER numberArg, LUA_INTEGER numberArg2, LUA_INTEGER numberArg3, LUA_INTEGER numberArg4) {
	const char *handled = NULL;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushinteger(luaState, numberArg2);
			lua_pushinteger(luaState, numberArg3);
			lua_pushinteger(luaState, numberArg4);
			handled = call_sfunction(luaState, 4);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static const char *CallNamedFunction(const char *name, LUA_INTEGER numberArg, LUA_INTEGER numberArg2, LUA_INTEGER numberArg3) {
	const char *handled = NULL;
	if (luaState) {
		lua_getglobal(luaState, name);
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, numberArg);
			lua_pushinteger(luaState, numberArg2);
			lua_pushinteger(luaState, numberArg3);
			handled = call_sfunction(luaState, 3);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

static int sf_ReloadProperties(lua_State* L) {
	host->ReloadProperties();
	CallNamedFunction("OnReloadProperties");
	return 0;
}


static int iface_function_helper(lua_State *L, const IFaceFunction &func, bool bSetter) {
	ExtensionAPI::Pane p = check_pane_object(L, 1);

	int arg = 2;

	intptr_t params[2] = {0,0};

	std::string stringResult;

	bool needStringResult = false;

	int loopParamCount = 2;

	if (func.paramType[0] == iface_length && func.paramType[1] == iface_string) {
		params[0] = static_cast<intptr_t>(lua_rawlen(L, arg));
		params[1] = reinterpret_cast<intptr_t>(params[0] ? lua_tostring(L, arg) : "");
		loopParamCount = 0;
	} else if ((func.paramType[1] == iface_stringresult) || (func.returnType == iface_stringresult)) {
		needStringResult = true;
		// The buffer will be allocated later, so it won't leak if Lua does
		// a longjmp in response to a bad arg.
		if (func.paramType[0] == iface_length) {
			loopParamCount = 0;
		} else {
			if (!bSetter)
				loopParamCount = 1;
			else
				needStringResult = false;
		}
	}

	for (int i=0; i<loopParamCount; ++i) {
		if (func.paramType[i] == iface_string || func.paramType[i] == iface_stringresult) {
			const char *s = lua_tostring(L, arg++);
			params[i] = reinterpret_cast<intptr_t>(s ? s : "");
		} else if (func.paramType[i] == iface_keymod) {
			int keycode = static_cast<int>(luaL_checknumber(L, arg++)) & 0xFFFF;
			int modifiers = static_cast<int>(luaL_checknumber(L, arg++)) & (SCMOD_SHIFT|SCMOD_CTRL|SCMOD_ALT);
			params[i] = keycode | (modifiers<<16);
		} else if (func.paramType[i] == iface_bool) {
			params[i] = lua_toboolean(L, arg++);
		} else if (IFaceTypeIsNumeric(func.paramType[i])) {
			params[i] = static_cast<long>(luaL_checknumber(L, arg++));
		}
	}
	if (bSetter && func.paramType[0] == iface_stringresult)
		params[1] = params[0];

	if (needStringResult) {
		const intptr_t stringResultLen = host->Send(p, static_cast<Message>(func.value), params[0], 0);
		stringResult.assign(stringResultLen, '\0');
		params[1] = reinterpret_cast<intptr_t>(stringResult.data());

		if (func.paramType[0] == iface_length) {
			params[0] = stringResultLen;
		}
	}

	// Now figure out what to do with the param types and return type.
	// - stringresult gets inserted at the start of return tuple.
	// - numeric return type gets returned to lua as a number (following the stringresult)
	// - other return types e.g. void get dropped.

	lua_Integer result = host->Send(p, static_cast<Message>(func.value), params[0], params[1]);

	int resultCount = 0;

	if (needStringResult) {
		//push_string(L, stringResult);
		lua_pushlstring(L, stringResult.data(), stringResult.length());
		resultCount++;
	}

	if (func.returnType == iface_bool) {
		lua_pushboolean(L, static_cast<int>(result));
		resultCount++;
	}	if (func.returnType == iface_bool) {
		lua_pushboolean(L, static_cast<int>(result));
		resultCount++;
	} else if (IFaceTypeIsNumeric(func.returnType)) {
		lua_pushinteger(L, result);
		resultCount++;
	}

	return resultCount;
}

struct IFacePropertyBinding {
	ExtensionAPI::Pane pane;
	const IFaceProperty *prop;
};

static int cf_ifaceprop_metatable_index(lua_State *L) {
	// if there is a getter, __index calls it
	// otherwise, __index raises "property 'name' is write-only".
	IFacePropertyBinding *ipb = reinterpret_cast<IFacePropertyBinding*>(checkudata(L, 1, "SciTE_MT_IFacePropertyBinding"));
	if (!(ipb && IFacePropertyIsScriptable(*(ipb->prop)))) {
		raise_error(L, "Internal error: property binding is improperly set up");
		return 0;
	}
	if (ipb->prop->getter == 0) {
		raise_error(L, "Attempt to read a write-only indexed property");
		return 0;
	}
	IFaceFunction func = ipb->prop->GetterFunction();

	// rewrite the stack to match what the function expects.  put pane at index 1; param is already at index 2.
	push_pane_object(L, ipb->pane);
	lua_replace(L, 1);
	lua_settop(L, 2);
	return iface_function_helper(L, func);
}

static int cf_ifaceprop_metatable_newindex(lua_State *L) {
	IFacePropertyBinding *ipb = reinterpret_cast<IFacePropertyBinding*>(checkudata(L, 1, "SciTE_MT_IFacePropertyBinding"));
	if (!(ipb && IFacePropertyIsScriptable(*(ipb->prop)))) {
		raise_error(L, "Internal error: property binding is improperly set up");
		return 0;
	}
	if (ipb->prop->setter == 0) {
		raise_error(L, "Attempt to write a read-only indexed property");
		return 0;
	}
	IFaceFunction func = ipb->prop->SetterFunction();

	// rewrite the stack to match what the function expects.
	// pane at index 1; param at index 2, value at index 3
	push_pane_object(L, ipb->pane);
	lua_replace(L, 1);
	lua_settop(L, 3);
	return iface_function_helper(L, func, true);
}

static int cf_pane_iface_function(lua_State *L) {
	int funcidx = lua_upvalueindex(1);
	const IFaceFunction *func = reinterpret_cast<IFaceFunction *>(lua_touserdata(L, funcidx));
	if (func) {
		return iface_function_helper(L, *func);
	} else {
		raise_error(L, "Internal error - bad upvalue in iface function closure");
		return 0;
	}
}

static int push_iface_function(lua_State *L, const char *name) {
	int i = IFaceTable::FindFunction(name);
	if (i >= 0) {
		if (IFaceFunctionIsScriptable(IFaceTable::functions[i])) {
			lua_pushlightuserdata(L, const_cast<IFaceFunction*>(IFaceTable::functions+i));
			lua_pushcclosure(L, cf_pane_iface_function, 1);

			// Since Lua experts say it is inefficient to create closures / cfunctions
			// in an inner loop, I tried caching the closures in the metatable, and looking
			// for them there first.  However, it made very little difference and did not
			// seem worth the added complexity. - WBD

			return 1;
		}
	}
	return -1; // signal to try next pane index handler
}

static int push_iface_propval(lua_State *L, const char *name) {
	// this function doesn't raise errors, but returns 0 if the function is not handled.

	int propidx = IFaceTable::FindProperty(name);
	if (propidx >= 0) {
		const IFaceProperty &prop = IFaceTable::properties[propidx];
		if (!IFacePropertyIsScriptable(prop)) {
			raise_error(L, "Error: iface property is not scriptable.");
			return -1;
		}

		if (prop.paramType == iface_void) {
			if (prop.getter) {
				lua_settop(L, 1);
				return iface_function_helper(L, prop.GetterFunction());
			}
		} else if (prop.paramType == iface_bool) {
			// The bool getter is untested since there are none in the iface.
			// However, the following is suggested as a reference protocol.
			ExtensionAPI::Pane p = check_pane_object(L, 1);

			if (prop.getter) {
				if (host->Send(p, static_cast<Message>(prop.getter), 1, 0)) {
					lua_pushnil(L);
					return 1;
				} else {
					lua_settop(L, 1);
					lua_pushboolean(L, 0);
					return iface_function_helper(L, prop.GetterFunction());
				}
			}
		} else {
			// Indexed property.  These return an object with the following behavior:
			// if there is a getter, __index calls it
			// otherwise, __index raises "property 'name' is write-only".
			// if there is a setter, __newindex calls it
			// otherwise, __newindex raises "property 'name' is read-only"

			IFacePropertyBinding *ipb = reinterpret_cast<IFacePropertyBinding*>(lua_newuserdata(L, sizeof(IFacePropertyBinding)));
			if (ipb) {
				ipb->pane = check_pane_object(L, 1);
				ipb->prop = &prop;
				if (luaL_newmetatable(L, "SciTE_MT_IFacePropertyBinding")) {
					lua_pushliteral(L, "__index");
					lua_pushcfunction(L, cf_ifaceprop_metatable_index);
					lua_settable(L, -3);
					lua_pushliteral(L, "__newindex");
					lua_pushcfunction(L, cf_ifaceprop_metatable_newindex);
					lua_settable(L, -3);
				}
				lua_setmetatable(L, -2);
				return 1;
			} else {
				raise_error(L, "Internal error: failed to allocate userdata for indexed property");
				return -1;
			}
		}
	}

	return -1; // signal to try next pane index handler
}

static int cf_pane_metatable_index(lua_State *L) {
	if (lua_isstring(L, 2)) {
		const char *name = lua_tostring(L, 2);

		// these return the number of values pushed (possibly 0), or -1 if no match
		int results = push_iface_function(L, name);
		if (results < 0)
			results = push_iface_propval(L, name);

		if (results >= 0) {
			return results;
		} else if (name[0] != '_') {
			lua_getmetatable(L, 1);
			if (lua_istable(L, -1)) {
				lua_pushvalue(L, 2);
				lua_gettable(L, -2);
				if (!lua_isnil(L, -1))
					return 1;
			}
		}
	}

	raise_error(L, "Pane function / readable property / indexed writable property name expected");
	return 0;
}

static int cf_pane_metatable_newindex(lua_State *L) {
	if (lua_isstring(L, 2)) {
		int propidx = IFaceTable::FindProperty(lua_tostring(L, 2));
		if (propidx >= 0) {
			const IFaceProperty &prop = IFaceTable::properties[propidx];
			if (IFacePropertyIsScriptable(prop)) {
				if (prop.setter) {
					// stack needs to be rearranged to look like an iface function call
					lua_remove(L, 2);

					if (prop.paramType == iface_void) {
						return iface_function_helper(L, prop.SetterFunction(), true);

					} else if ((prop.paramType == iface_bool)) {
						if (!lua_isnil(L, 3)) {
							lua_pushboolean(L, 1);
							lua_insert(L, 2);
						} else {
							// the nil will do as a false value.
							// just push an arbitrary numeric value that Scintilla will ignore
							lua_pushinteger(L, 0);
						}
						return iface_function_helper(L, prop.SetterFunction(), true);

					} else {
						raise_error(L, "Error - (pane object) cannot assign directly to indexed property");
					}
				} else {
					raise_error(L, "Error - (pane object) cannot assign to a read-only property");
				}
			}
		}
	}

	raise_error(L, "Error - (pane object) expected the name of a writable property");
	return 0;
}

void push_pane_object(lua_State *L, ExtensionAPI::Pane p) {
	*reinterpret_cast<ExtensionAPI::Pane*>(lua_newuserdata(L, sizeof(p))) = p;
	if (luaL_newmetatable(L, "SciTE_MT_Pane")) {
		lua_pushcfunction(L, cf_pane_metatable_index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, cf_pane_metatable_newindex);
		lua_setfield(L, -2, "__newindex");

		// Push built-in functions into the metatable, where the custom
		// __index metamethod will find them.

		lua_pushcfunction(L, cf_pane_eol_annotation_set);
		lua_setfield(L, -2, "eolAnnotationSet");
		lua_pushcfunction(L, cf_pane_findtext);
		lua_setfield(L, -2, "findtext");
		lua_pushcfunction(L, cf_pane_textrange);
		lua_setfield(L, -2, "textrange");
		lua_pushcfunction(L, cf_pane_insert);
		lua_setfield(L, -2, "insert");
		lua_pushcfunction(L, cf_pane_remove);
		lua_setfield(L, -2, "remove");
		lua_pushcfunction(L, cf_pane_append);
		lua_setfield(L, -2, "append");
		lua_pushcfunction(L, cf_pane_line);
		lua_setfield(L, -2, "line");
		lua_pushcfunction(L, cf_pane_getstyle);
		lua_setfield(L, -2, "ustyle");
		lua_pushcfunction(L, cf_pane_getchar);
		lua_setfield(L, -2, "uchar");
		lua_pushcfunction(L, cf_pane_getutf8text);
		lua_setfield(L, -2, "utf8text");
		lua_pushcfunction(luaState, cf_pane_get_codepage);
		lua_setfield(luaState, -2, "codepage");
		lua_pushcfunction(L, cf_pane_match_generator);
		lua_pushcclosure(L, cf_pane_match, 1);
		lua_setfield(L, -2, "match");
	}
	lua_setmetatable(L, -2);
}

static int cf_global_metatable_index(lua_State *L) {
	if (lua_isstring(L, 2)) {
		const char *name = lua_tostring(L, 2);
		if ((name[0] < 'A') || (name[0] > 'Z') || ((name[1] >= 'a') && (name[1] <= 'z'))) {
			// short circuit; iface constants are always upper-case and start with a letter
			return 0;
		}

		int i = IFaceTable::FindConstant(name);
		if (i >= 0) {
			lua_pushinteger(L, IFaceTable::constants[i].value);
			return 1;
		} else {
			i = IFaceTable::FindFunctionByConstantName(name);
			if (i >= 0) {
				lua_pushinteger(L, IFaceTable::functions[i].value);

				// FindFunctionByConstantName is slow, so cache the result into the
				// global table.  My tests show this gives an order of magnitude
				// improvement.
				lua_pushvalue(L, 2);
				lua_pushvalue(L, -2);
				lua_rawset(L, 1);

				return 1;
			}
		}
	}

	return 0; // global namespace access should not raise errors
}

static int LuaPanicFunction(lua_State *L) {
	if (L == luaState) {
		iuplua_close(luaState);
		lua_close(luaState);
		luaState = NULL;
		luaDisabled = true;
	}
	host->Trace("\n> Lua: error occurred in unprotected call.  This is very bad.\n");
	return 1;
}

// Don't initialise Lua in LuaExtension::Initialise.  Wait and initialise Lua the
// first time Lua is used, e.g. when a Load event is called with an argument that
// appears to be the name of a Lua script.  This just-in-time initialisation logic
// does add a little extra complexity but not a lot.  It's probably worth it,
// since it means a user who is having trouble with Lua can just refrain from
// using it.

static char *CheckStartupScript() {
	delete[] startupScript;

	startupScript = host->Property("ext.lua.startup.script");

	if (startupScript && startupScript[0] == '\0') {
		delete[] startupScript;
		startupScript = NULL;
		//Startup path not found in ext.lua.startup.script";
		//Global properties is absent?
		::exit(111222333);
	}

	return startupScript;
}
Ihandle *hLayout;

static int cf_iup_get_layout(lua_State *L){
	iuplua_pushihandle(L, hLayout);
	return 1;
}
static int cf_iup_KeyCodeToName(lua_State *L){
	int k = static_cast<int>(luaL_checkinteger(L, 1));
	lua_pushfstring(L, iupKeyCodeToName(k));
	return 1;
}

void ContinueInit(void* h){
	if (h) hLayout = (Ihandle*)h;
	lua_getglobal(luaState, "iup");
	lua_pushcfunction(luaState, cf_iup_get_layout);
	lua_setfield(luaState, -2, "GetLayout");
	lua_pushcfunction(luaState, cf_iup_KeyCodeToName);
	lua_setfield(luaState, -2, "KeyCodeToName");
	lua_pop(luaState, 1);

	if (startupScript) {
		// TODO: Should buffer be deactivated temporarily, so editor iface
		// functions won't be available during a reset, just as they are not
		// available during a normal startup?  Are there any other functions
		// that should be blocked during startup, e.g. the ones that allow
		// you to add or switch buffers?

		FilePath fpTest(GUI::StringFromUTF8(startupScript));
		if (fpTest.Exists()) {
			luaL_loadfile(luaState, startupScript);
			if (!call_function(luaState, 0, 0)) {
				host->Trace(">Lua: error occurred while loading startup script\n");
			}
			host->PostLoadScript();
		}
	}

	// Clone the initial state (including metatable) in the registry so that it can be restored.
	// (If reset==1 this will not be used, but this is a shallow copy, not very expensive, and
	// who knows what the value of reset will be the next time InitGlobalScope runs.)
	clone_table(luaState, -1, true);
	lua_setfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialState");

	// Clone loaded packages (package.loaded) state in the registry so that it can be restored.
	lua_getfield(luaState, LUA_REGISTRYINDEX, "_LOADED");
	clone_table(luaState, -1);
	lua_setfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialPackageState");
	lua_pop(luaState, 1);


}

void PublishGlobalBufferData() {
	lua_pushliteral(luaState, "buffer");
	if (curBufferIndex >= 0) {
		lua_pushliteral(luaState, "SciTE_BufferData_Array");
		lua_rawget(luaState, LUA_REGISTRYINDEX);
		if (!lua_istable(luaState, -1)) {
			lua_pop(luaState, 1);

			lua_newtable(luaState);
			lua_pushliteral(luaState, "SciTE_BufferData_Array");
			lua_pushvalue(luaState, -2);
			lua_rawset(luaState, LUA_REGISTRYINDEX);
		}
		lua_rawgeti(luaState, -1, curBufferIndex);
		if (!lua_istable(luaState, -1)) {
			// create new buffer-data
			lua_pop(luaState, 1);
			lua_newtable(luaState);
			// remember it
			lua_pushvalue(luaState, -1);
			lua_rawseti(luaState, -3, curBufferIndex);
		}
		// Replace SciTE_BufferData_Array in the stack, leaving (buffer=-1, 'buffer'=-2)
		lua_replace(luaState, -2);
	} else {
		// for example, during startup, before any InitBuffer / ActivateBuffer
		lua_pushnil(luaState);
	}
	//lua_rawset(luaState, LUA_GLOBALSINDEX);
	lua_setglobal(luaState, "buffer");
}

int OnParamKeyPress(int c, int press) {
	if ((press == 1) && (c == K_F1)) {
		return CallNamedFunction("OnParamKeyPress");
	}
	return 0;
}


void setListVal(void* IH, int lin, int col, const char* value) {
	iupMatrixSetValue((Ihandle*)IH, lin, col, value, 0);
}

int sf_GetListHandlers(lua_State *L) {
	lua_pushlightuserdata(L, (void*)setListVal);
	lua_pushlightuserdata(L, (void*)iuplua_checkihandle(L, 1));
	return 2;
}

static int cf_editor_reload_startup_script(lua_State*); //!-add-[StartupScriptReload]

static bool InitGlobalScope(bool checkProperties, bool forceReload = false) {
	bool reload = forceReload;
	if (checkProperties) {
		int resetMode = GetPropertyInt("ext.lua.reset");
		if (resetMode >= 1) {
			reload = true;
		}
	}

	tracebackEnabled = (GetPropertyInt("ext.lua.debug.traceback") == 1);

	if (luaState) {
		// The Clear / Load used to use metatables to setup without having to re-run the scripts,
		// but this was unreliable e.g. a few library functions and some third-party code use
		// rawget to access functions in the global scope.  So the new method makes a shallow
		// copy of the initialized global environment, and uses that to re-init the scope.

		if (!reload) {
			lua_getfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialState");
			if (lua_istable(luaState, -1)) {
				clear_table(luaState, -2, true);
				merge_table(luaState, -2, -1, true);
				lua_pop(luaState, 1);

				// restore initial package.loaded state
				lua_getfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialPackageState");
				lua_getfield(luaState, LUA_REGISTRYINDEX, "_LOADED");
				clear_table(luaState, -1, false);
				merge_table(luaState, -1, -2, false);
				lua_pop(luaState, 2);

				PublishGlobalBufferData();

				return true;
			} else {
				lua_pop(luaState, 1);
			}
		}

		// reload mode is enabled, or else the inital state has been broken.
		// either way, we're going to need a "new" initial state.

		lua_pushnil(luaState);
		lua_setfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialState");

		// Also reset buffer data, since scripts might depend on this to know
		// whether they need to re-initialize something.
		lua_pushnil(luaState);
		lua_setfield(luaState, LUA_REGISTRYINDEX, "SciTE_BufferData_Array");

		// Don't replace global scope using new_table, because then startup script is
		// bound to a different copy of the globals than the extension script.
		clear_table(luaState, -1, true);

		// Lua 5.1: _LOADED is in LUA_REGISTRYINDEX, so it must be cleared before
		// loading libraries or they will not load because Lua's package system
		// thinks they are already loaded
		lua_pushnil(luaState);
		lua_setfield(luaState, LUA_REGISTRYINDEX, "_LOADED");

	} else if (!luaDisabled) {
		luaState = luaL_newstate();
		if (!luaState) {
			luaDisabled = true;
			host->Trace("> Lua: scripting engine failed to initalise\n");
			return false;
		}
		lua_atpanic(luaState, LuaPanicFunction);

	} else {
		return false;
	}
	// ...register standard libraries
	luaL_openlibs(luaState);
	iuplua_open(luaState);
	iupolelua_open(luaState);
	iupweblua_open(luaState);
	Iupsc_DetachBoxOpen();
	iupsc_detachboxlua_open(luaState);
	Iupsc_SBoxOpen();
	iupsc_SBoxlua_open(luaState);
	IupFlattabsCtrlOpen();
	iupFlattabsCtrllua_open(luaState);
	IupScrollCanvasOpen();
	iupIupScrollCanvaslua_open(luaState);
	iupcontrolslua_open(luaState);
	cdlua_open(luaState);
	cdluaiup_open(luaState);
	//load_all_images_Images();

	lua_register(luaState, "_ALERT", cf_global_print);

	// although this is mostly redundant with output:append
	// it is still included for now
	lua_register(luaState, "trace", cf_global_trace);

	// emulate a Lua 4 function that is useful in menu commands
	lua_register(luaState, "dostring", cf_global_dostring);

	// override a library function whose default impl uses stdout
	lua_register(luaState, "print", cf_global_print);

	// props object - provides access to Property and SetProperty
	lua_newuserdata(luaState, 1); // the value doesn't matter.
	if (luaL_newmetatable(luaState, "SciTE_MT_Props")) {
		lua_pushcfunction(luaState, cf_props_metatable_index);
		lua_setfield(luaState, -2, "__index");
		lua_pushcfunction(luaState, cf_props_metatable_newindex);
		lua_setfield(luaState, -2, "__newindex");
	}
	lua_setmetatable(luaState, -2);
	lua_setglobal(luaState, "props");

	// pane objects
	push_pane_object(luaState, ExtensionAPI::paneEditor);
	lua_setglobal(luaState, "editor");

	push_pane_object(luaState, ExtensionAPI::paneCoEditor);
	lua_setglobal(luaState, "coeditor");

	push_pane_object(luaState, ExtensionAPI::paneOutput);
	lua_setglobal(luaState, "output");

	push_pane_object(luaState, ExtensionAPI::paneFindRes); 
	lua_setglobal(luaState, "findres");

	// scite
	lua_newtable(luaState);

	lua_getglobal(luaState, "editor");
	lua_pushcclosure(luaState, cf_scite_send, 1);
	lua_setfield(luaState, -2, "SendEditor");

	lua_getglobal(luaState, "output");
	lua_pushcclosure(luaState, cf_scite_send, 1);
	lua_setfield(luaState, -2, "SendOutput");

	lua_getglobal(luaState, "findres");
	lua_pushcclosure(luaState, cf_scite_send, 1);
	lua_setfield(luaState, -2, "SendFindRes");

	lua_pushcfunction(luaState, cf_scite_open);
	lua_setfield(luaState, -2, "Open");

	lua_pushcfunction(luaState, cf_scite_menu_command);
	lua_setfield(luaState, -2, "MenuCommand");

	//!-start-[CheckMenus]
	lua_pushcfunction(luaState, cf_scite_check_menus);
	lua_setfield(luaState, -2, "CheckMenus");
	//!-end-[CheckMenus]

//!-start-[ReloadStartupScript]
	lua_pushcfunction(luaState, cf_editor_reload_startup_script);
	lua_setfield(luaState, -2, "ReloadStartupScript");
//!-end-[ReloadStartupScript]

//!-start-[LocalizationFromLua]
	lua_pushcfunction(luaState, cf_editor_get_translation);
	lua_setfield(luaState, -2, "GetTranslation");
//!-end-[LocalizationFromLua]

	lua_pushcfunction(luaState, cf_Reg_HotKey);
	lua_setfield(luaState, -2, "RegistryHotKeys");

	lua_pushcfunction(luaState, cf_perform_grep_ex);
	lua_setfield(luaState, -2, "PerformGrepEx");

	lua_pushcfunction(luaState, cf_EnsureVisible);
	lua_setfield(luaState, -2, "EnsureVisible");

	lua_pushcfunction(luaState, cf_HideForeReolad);
	lua_setfield(luaState, -2, "HideForeReolad");

	lua_pushcfunction(luaState, sf_SetOverrideLanguage);
	lua_setfield(luaState, -2, "SetLexer");

	lua_pushcfunction(luaState, sf_SwitchMouseHook);
	lua_setfield(luaState, -2, "SwitchMouseHook");

	lua_pushcfunction(luaState, sf_RunInConcole);
	lua_setfield(luaState, -2, "RunInConcole");

	lua_pushcfunction(luaState, sf_ExecuteHelp);
	lua_setfield(luaState, -2, "ExecuteHelp");

	lua_pushcfunction(luaState, sf_CheckRegexp);
	lua_setfield(luaState, -2, "CheckRegexp");

	lua_pushcfunction(luaState, sf_RunAsync);
	lua_setfield(luaState, -2, "RunAsync");

	lua_pushcfunction(luaState, cf_scite_get_editor);
	lua_setfield(luaState, -2, "ActiveEditor");

	lua_pushcfunction(luaState, cf_scite_save_encoding_file);
	lua_setfield(luaState, -2, "SaveEncodingFile");

	lua_pushcfunction(luaState, cf_scite_compare_encoding_file);
	lua_setfield(luaState, -2, "CompareEncodingFile");

	lua_pushcfunction(luaState, sf_ReloadProperties);
	lua_setfield(luaState, -2, "ReloadProperties");

	lua_pushcfunction(luaState, sf_Close);
	lua_setfield(luaState, -2, "Close");

	lua_pushcfunction(luaState, sf_SavePosition);
	lua_setfield(luaState, -2, "SavePosition");

	lua_pushcfunction(luaState, sf_BlockUpdate);
	lua_setfield(luaState, -2, "BlockUpdate");

	lua_pushcfunction(luaState, cf_editor_set_foreground);
	lua_setfield(luaState, -2, "SetForeground");

	lua_pushcfunction(luaState, sf_SetRestart);
	lua_setfield(luaState, -2, "SetRestart");

	lua_pushcfunction(luaState, sf_FileVersionInfo);
	lua_setfield(luaState, -2, "FileVersionInfo");

	lua_pushcfunction(luaState, sf_OrderTab);
	lua_setfield(luaState, -2, "OrderTab");

	lua_pushcfunction(luaState, sf_findfiles);
	lua_setfield(luaState, -2, "findfiles");

	lua_pushcfunction(luaState, sf_IsRunAsAdmin);
	lua_setfield(luaState, -2, "IsRunAsAdmin");

	lua_pushcfunction(luaState, sf_NewInstance);
	lua_setfield(luaState, -2, "NewInstance");

	lua_pushcfunction(luaState, sf_GetListHandlers);
	lua_setfield(luaState, -2, "GetListHandlers");

	lua_pushcfunction(luaState, cf_scite_Read);
	lua_setfield(luaState, -2, "Read");

	lua_pushcfunction(luaState, cf_scite_LAB2RGB);
	lua_setfield(luaState, -2, "LAB2RGB");

	lua_pushcfunction(luaState, cf_scite_RGB2LAB);
	lua_setfield(luaState, -2, "RGB2LAB");

	lua_pushcfunction(luaState, cf_scite_InvertColor);
	lua_setfield(luaState, -2, "InvertColor");

	lua_pushcfunction(luaState, cf_get_keyboard_layout);
	lua_setfield(luaState, -2, "GetKeyboardLayout");

	lua_pushcfunction(luaState, cf_get_is64bit);
	lua_setfield(luaState, -2, "Is64Bit");

	lua_pushcfunction(luaState, cf_switch_tabs);
	lua_setfield(luaState, -2, "SwitchTabs");

	// buffers
	lua_newtable(luaState);
	lua_pushcfunction(luaState, bf_get_count);
	lua_setfield(luaState, -2, "GetCount");
	lua_pushcfunction(luaState, cf_set_co_document_at);
	lua_setfield(luaState, -2, "SetCoDocumentAt");
	lua_pushcfunction(luaState, cf_set_document_at);
	lua_setfield(luaState, -2, "SetDocumentAt");
	lua_pushcfunction(luaState, bf_get_buffer_name);
	lua_setfield(luaState, -2, "NameAt");
	lua_pushcfunction(luaState, bf_get_cobuffer_name);
	lua_setfield(luaState, -2, "CoName");
	lua_pushcfunction(luaState, bf_get_buffer_saved);
	lua_setfield(luaState, -2, "SavedAt");
	lua_pushcfunction(luaState, bf_get_buffer_encoding);
	lua_setfield(luaState, -2, "EncodingAt");
	lua_pushcfunction(luaState, bf_set_buffer_encoding);
	lua_setfield(luaState, -2, "SetEncodingAt");
	lua_pushcfunction(luaState, bf_get_filetime);
	lua_setfield(luaState, -2, "FileTimeAt");
	lua_pushcfunction(luaState, bf_set_filetime);
	lua_setfield(luaState, -2, "ClearFileTimeAt");
	lua_pushcfunction(luaState, bf_current);
	lua_setfield(luaState, -2, "GetCurrent");
	lua_pushcfunction(luaState, bf_save_cobuffer_pos);
	lua_setfield(luaState, -2, "SaveCoBuffPos");

	lua_pushcfunction(luaState, bf_get_buffer_side);
	lua_setfield(luaState, -2, "GetBufferSide");

	lua_pushcfunction(luaState, bf_get_buffer_order);
	lua_setfield(luaState, -2, "GetBufferOrder");

	lua_pushcfunction(luaState, bf_get_buffer_redonly);
	lua_setfield(luaState, -2, "GetBufferReadOnly");

	lua_pushcfunction(luaState, bf_get_buffer_modtime);
	lua_setfield(luaState, -2, "GetBufferModTime");

	lua_pushcfunction(luaState, bf_get_buffer_unicmode);
	lua_setfield(luaState, -2, "GetBufferUnicMode");

	lua_pushcfunction(luaState, bf_is_cloned);
	lua_setfield(luaState, -2, "IsCloned");

	lua_pushcfunction(luaState, bf_index_of_clone);
	lua_setfield(luaState, -2, "IndexOfClone");

	lua_pushcfunction(luaState, bf_buffer_by_name);
	lua_setfield(luaState, -2, "BufferByName");

	lua_pushcfunction(luaState, bf_second_editor_active);
	lua_setfield(luaState, -2, "SecondEditorActive");
	lua_setfield(luaState, -2, "buffers");
	
	lua_setglobal(luaState, "scite");


	
//!-start-[EncodingToLua]
	lua_getglobal(luaState, "string");
	lua_pushcfunction(luaState, lua_string_to_utf8);
	lua_setfield(luaState, -2, "to_utf8");
	lua_pushcfunction(luaState, lua_string_from_utf8);
	lua_setfield(luaState, -2, "from_utf8");
	lua_pushcfunction(luaState, lua_string_utf8_to_upper);
	lua_setfield(luaState, -2, "utf8upper");
	lua_pushcfunction(luaState, lua_string_utf8_to_lower);
	lua_setfield(luaState, -2, "utf8lower");
	lua_pushcfunction(luaState, lua_string_utf8len);
	lua_setfield(luaState, -2, "utf8len");
	lua_pop(luaState, 1);
//!-end-[EncodingToLua]

	// Metatable for global namespace, to publish iface constants
	lua_pushglobaltable(luaState);
	if (luaL_newmetatable(luaState, "SciTE_MT_GlobalScope")) {
		lua_pushcfunction(luaState, cf_global_metatable_index);
		lua_setfield(luaState, -2, "__index");
	}

	lua_setmetatable(luaState, -2);

	IupSetFunction("GLOBALKEYPRESS_CB", (Icallback)OnParamKeyPress);
	if (checkProperties && reload) {
		CheckStartupScript();
	}
	lua_pushliteral(luaState, LUA_VERSION_RELEASE);
	lua_setfield(luaState, -2, "_VERSION_RELEASE");

	//if (startupScript) {
	//	// TODO: Should buffer be deactivated temporarily, so editor iface
	//	// functions won't be available during a reset, just as they are not
	//	// available during a normal startup?  Are there any other functions
	//	// that should be blocked during startup, e.g. the ones that allow
	//	// you to add or switch buffers?
	//
	//	FilePath fpTest(GUI::StringFromUTF8(startupScript));
	//	if (fpTest.Exists()) {
	//		luaL_loadfile(luaState, startupScript);
	//		if (!call_function(luaState, 0, 0)) {
	//			host->Trace(">Lua: error occurred while loading startup script\n");
	//		}
	//	}
	//}
	//
	//// Clone the initial state (including metatable) in the registry so that it can be restored.
	//// (If reset==1 this will not be used, but this is a shallow copy, not very expensive, and
	//// who knows what the value of reset will be the next time InitGlobalScope runs.)
	//clone_table(luaState, LUA_GLOBALSINDEX, true);
	//lua_setfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialState");
	//
	//// Clone loaded packages (package.loaded) state in the registry so that it can be restored.
	//lua_getfield(luaState, LUA_REGISTRYINDEX, "_LOADED");
	//clone_table(luaState, -1);
	//lua_setfield(luaState, LUA_REGISTRYINDEX, "SciTE_InitialPackageState");
	//lua_pop(luaState, 1);

	//PublishGlobalBufferData();

	if (reload) ContinueInit(NULL);

	return true;
}

bool LuaExtension::Initialise(ExtensionAPI *host_) {
	host = host_;

	if (CheckStartupScript()) {
		InitGlobalScope(false);
	}
	return false;
}

void LuaExtension::PostInit(void* h){
	ContinueInit(h);
	PublishGlobalBufferData();

}
void* LuaExtension::GetLuaState(){
	return (void*) luaState;
}
bool LuaExtension::Finalise() {
	if (luaState) {
		CallNamedFunction("OnFinalise"); //!-add-[OnFinalise]
		iuplua_close(luaState);
		//try{
			lua_close(luaState);
		//} catch (...) {} // ��������� ������ ���� ���� ������������ �������
	}

	luaState = NULL;
	host = NULL;

	// The rest don't strictly need to be cleared since they
	// are never accessed except when luaState and host are set

	delete [] startupScript;
	startupScript = NULL;

	return false;
}

bool LuaExtension::Clear() {
	if (luaState) {
		CallNamedFunction("OnClear");
	}
	if (luaState) {
		InitGlobalScope(true);
		extensionScript.clear();
	} else if ((GetPropertyInt("ext.lua.reset") >= 1) && CheckStartupScript()) {
		InitGlobalScope(false);
	}
	return false;
}

bool LuaExtension::Load(const char *filename) {
	bool loaded = false;

	if (!luaDisabled) {
		size_t sl = strlen(filename);
		if (sl >= 4 && strcmp(filename+sl-4, ".lua")==0) {
			if (luaState || InitGlobalScope(false)) {
				extensionScript = filename;
				luaL_loadfile(luaState, filename);
				if (!call_function(luaState, 0, 0)) {
					host->Trace(">Lua: error occurred while loading extension script\n");
				}
				loaded = true;
			}
		}
	}
	return loaded;
}


bool LuaExtension::InitBuffer(int index) {
	//char msg[100];
	//sprintf(msg, "InitBuffer(%d)\n", index);
	//host->Trace(msg);

	//if (index > maxBufferIndex)	- �� �������� � ������, ����� ������� ���� ����������  �� � ����� - ����� ��������� ����� ���������
	//	maxBufferIndex = index;
	maxBufferIndex = host->GetBuffersCount();

	if (luaState) {
		// This buffer might be recycled.  Clear the data associated
		// with the old file.

		lua_getfield(luaState, LUA_REGISTRYINDEX, "SciTE_BufferData_Array");
		if (lua_istable(luaState, -1)) {
			lua_pushnil(luaState);
			lua_rawseti(luaState, -2, index);
		}
		lua_pop(luaState, 1);
		// We also need to handle cases where Lua initialization is
		// delayed (e.g. no startup script).  For that we'll just
		// explicitly call InitBuffer(curBufferIndex)
	}

	curBufferIndex = index;
	return false;
}

bool LuaExtension::ActivateBuffer(int index) {
	//char msg[100];
	//sprintf(msg, "ActivateBuffer(%d)\n", index);
	//host->Trace(msg);

	// Probably don't need to do anything with Lua here.  Setting
	// curBufferIndex is important so that InitGlobalScope knows
	// which buffer is active, in order to populate the 'buffer'
	// global with the right data.

	curBufferIndex = index;

	return false;
}

bool LuaExtension::RemoveBuffer(int index) {
	//char msg[100];
	//sprintf(msg, "RemoveBuffer(%d)\n", index);
	//host->Trace(msg);

	if (luaState) {
		// Remove the bufferdata element at index, and move
		// the other elements down.  The element at the
		// current maxBufferIndex can be discarded after
		// it gets copied to maxBufferIndex-1.

		lua_getfield(luaState, LUA_REGISTRYINDEX, "SciTE_BufferData_Array");
		if (lua_istable(luaState, -1)) {
			for (int i = index; i < maxBufferIndex; ++i) {
				lua_rawgeti(luaState, -1, i+1);
				lua_rawseti(luaState, -2, i);
			}

			lua_pushnil(luaState);
			lua_rawseti(luaState, -2, maxBufferIndex);

			lua_pop(luaState, 1); // the bufferdata table
		} else {
			lua_pop(luaState, 1);
		}
	}

	if (maxBufferIndex > 0)
		maxBufferIndex--;

	// Invalidate current buffer index; Activate or Init will follow.
	curBufferIndex = -1;

	return false;
}

bool LuaExtension::OnExecute(const char *s) {
	bool handled = false;

	if (luaState || InitGlobalScope(false)) {
		// May as well use Lua's pattern matcher to parse the command.
		// Scintilla's RESearch was the other option.
		int stackBase = lua_gettop(luaState);
		lua_pushglobaltable(luaState);
		lua_pushliteral(luaState, "string");
		lua_rawget(luaState, -2);
		if (lua_istable(luaState, -1)) {
			lua_pushliteral(luaState, "find");
			lua_rawget(luaState, -2);
			if (lua_isfunction(luaState, -1)) {
				lua_pushstring(luaState, s);
				lua_pushliteral(luaState, "^%s*([%a_][%a%d_]*)%s*(.-)%s*$");
				int status = lua_pcall(luaState, 2, 4, 0);
				if (status==0) {
					const char* fn = lua_tolstring(luaState, -2, 0);
					const char* arg = lua_tolstring(luaState, -1, 0);
					lua_pushglobaltable(luaState);
					//lua_insert(luaState, stackBase+1);
					//lua_pushliteral(luaState, fn);
					lua_pushstring(luaState, fn);
					lua_gettable(luaState, -2);
					//lua_gettable(luaState, LUA_RIDX_GLOBALS);
					if (!lua_isnil(luaState, -1)) {
						if (lua_isfunction(luaState, -1)) {
							// Try calling it and, even if it fails, short-circuit Filerx
							handled = true;
							lua_insert(luaState, stackBase+1);
							lua_pushstring(luaState, arg);
							lua_insert(luaState, stackBase + 2);
							lua_settop(luaState, stackBase+2);
							if (!call_function(luaState, 1, 0)) {
								host->Trace(">Lua: error occurred while processing command\n");
							}
						}
					} else {
						host->Trace("> Lua: error checking global scope for command\n");
					}
				}
			}
		} else {
			host->Trace("> Lua: string library not loaded\n");
		}

		lua_settop(luaState, stackBase);
	}

	return handled;
}

bool LuaExtension::OnOpen(const char *filename) {
	return CallNamedFunction("OnOpen", filename);
}

bool LuaExtension::OnSwitchFile(const char *filename) {
	return CallNamedFunction("OnSwitchFile", filename);
}

bool LuaExtension::OnBeforeSave(const char *filename) {
	return CallNamedFunction("OnBeforeSave", filename);
}

bool LuaExtension::OnBeforeOpen(const char *filename, const char *extension, int& encoding) {
	//bool handled = CallNamedFunction("OnBeforeOpen", filename, extension);
	//if (!handled) {
	//	if(lua_isinteger(luaState, -1))
	//		encoding = lua_tointeger(luaState, -1);
	//}
	//return handled;
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnBeforeOpen");
		if (lua_isfunction(luaState, -1)) {
			lua_pushstring(luaState, filename);
			lua_pushstring(luaState, extension);
			call_function(luaState, 2, 2);

			if (lua_isinteger(luaState, -2)) {
				encoding = static_cast<int>(lua_tointeger(luaState, -2));
			} else {
				handled = lua_toboolean(luaState, -2);
			}
			lua_pop(luaState, 2);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

bool LuaExtension::OnSave(const char *filename) {
	bool result = CallNamedFunction("OnSave", filename);

	// should this be case insensitive on windows?  check for different
	// ways of spelling a filename, e.g. short vs. long?
	if (startupScript && 0 == strcmp(filename, startupScript)) {
		if (GetPropertyInt("ext.lua.auto.reload") > 0) {
			InitGlobalScope(false, true);
			if (extensionScript.length()) {
				Load(extensionScript.c_str());
			}
		}
	} else if (extensionScript.length() && 0 == strcmp(filename, extensionScript.c_str())) {
		if (GetPropertyInt("ext.lua.auto.reload") > 0) {
			InitGlobalScope(false, false);
			Load(extensionScript.c_str());
		}
	}

	return result;
}

bool LuaExtension::OnChar(char ch) {
	char chs[2] = {ch, '\0'};
	return CallNamedFunction("OnChar", chs);
}

void LuaExtension::OnShowActivate(int bActivate, int cmdShow, int SavePoint) {
	CallNamedFunction("OnShowActivate", bActivate, cmdShow, SavePoint);
}

bool LuaExtension::OnSavePointReached() {
	return CallNamedFunction("OnSavePointReached");
}

bool LuaExtension::OnSavePointLeft() {
	return CallNamedFunction("OnSavePointLeft");
}

// Similar to StyleContext class in Scintilla
struct StylingContext {
	unsigned int startPos;
	int lengthDoc;
	int initStyle;
	StyleWriter *styler;

	Sci_Position endPos;
	Sci_Position endDoc;

	Sci_Position currentPos;
	bool atLineStart;
	bool atLineEnd;
	int state;

	char cursor[3][8];
	int cursorPos;
	int codePage;
	int lenCurrent;
	int lenNext;

	static StylingContext *Context(lua_State *L) {
		return reinterpret_cast<StylingContext *>(
			lua_touserdata(L, lua_upvalueindex(1)));
	}

	void Colourize() {
		Sci_Position end = currentPos - 1;
		if (end >= static_cast<int>(endDoc))
			end = static_cast<int>(endDoc)-1;
		styler->ColourTo(end, state);
	}

	static int Line(lua_State *L) {
		StylingContext *context = Context(L);
		int position = static_cast<int>(luaL_checkinteger(L, 2));
		lua_pushinteger(L, context->styler->GetLine(position));
		return 1;
	}

	static int CharAt(lua_State *L) {
		StylingContext *context = Context(L);
		int position = static_cast<int>(luaL_checkinteger(L, 2));
		lua_pushinteger(L, context->styler->SafeGetCharAt(position));
		return 1;
	}

	static int StyleAt(lua_State *L) {
		StylingContext *context = Context(L);
		int position = static_cast<int>(luaL_checkinteger(L, 2));
		lua_pushinteger(L, context->styler->StyleAt(position));
		return 1;
	}

	static int LevelAt(lua_State *L) {
		StylingContext *context = Context(L);
		int line = static_cast<int>(luaL_checkinteger(L, 2));
		lua_pushinteger(L, context->styler->LevelAt(line));
		return 1;
	}

	static int SetLevelAt(lua_State *L) {
		StylingContext *context = Context(L);
		int line = static_cast<int>(luaL_checkinteger(L, 2));
		int level = static_cast<int>(luaL_checkinteger(L, 3));
		context->styler->SetLevel(line, level);
		return 0;
	}

	static int LineState(lua_State *L) {
		StylingContext *context = Context(L);
		int line = static_cast<int>(luaL_checkinteger(L, 2));
		lua_pushinteger(L, context->styler->GetLineState(line));
		return 1;
	}

	static int SetLineState(lua_State *L) {
		StylingContext *context = Context(L);
		int line = static_cast<int>(luaL_checkinteger(L, 2));
		int state = static_cast<int>(luaL_checkinteger(L, 3));
		context->styler->SetLineState(line, state);
		return 0;
	}


	void GetNextChar() {
		lenCurrent = lenNext;
		lenNext = 1;
		Sci_Position nextPos = currentPos + lenCurrent;
		unsigned char byteNext = static_cast<unsigned char>(styler->SafeGetCharAt(nextPos));
		unsigned int nextSlot = (cursorPos + 1) % 3;
		memcpy(cursor[nextSlot], "\0\0\0\0\0\0\0\0", 8);
		cursor[nextSlot][0] = byteNext;
		if (codePage) {
			if (codePage == SC_CP_UTF8) {
				if (byteNext >= 0x80) {
					cursor[nextSlot][1] = styler->SafeGetCharAt(nextPos+1);
					lenNext = 2;
					if (byteNext >= 0x80 + 0x40 + 0x20) {
						lenNext = 3;
						cursor[nextSlot][2] = styler->SafeGetCharAt(nextPos+2);
						if (byteNext >= 0x80 + 0x40 + 0x20 + 0x10) {
							lenNext = 4;
							cursor[nextSlot][3] = styler->SafeGetCharAt(nextPos+3);
						}
					}
				}
			} else {
				if (styler->IsLeadByte(byteNext)) {
					lenNext = 2;
					cursor[nextSlot][1] = styler->SafeGetCharAt(nextPos+1);
				}
			}
		}

		// End of line?
		// Trigger on CR only (Mac style) or either on LF from CR+LF (Dos/Win)
		// or on LF alone (Unix). Avoid triggering two times on Dos/Win.
		char ch = cursor[(cursorPos) % 3][0];
		atLineEnd = (ch == '\r' && cursor[nextSlot][0] != '\n') ||
					(ch == '\n') ||
					(currentPos >= endPos);
	}

	void StartStyling(unsigned int startPos, unsigned int length, int initStyle) {
		endDoc = styler->Length();
		endPos = startPos + length;
		if (endPos == endDoc)
			endPos = endDoc + 1;
		currentPos = startPos;
		atLineStart = true;
		atLineEnd = false;
		state = initStyle; 
		cursorPos = 0;
		lenCurrent = 0;
		lenNext = 0; 
		memcpy(cursor[0], "\0\0\0\0\0\0\0\0", 8);
		memcpy(cursor[1], "\0\0\0\0\0\0\0\0", 8);
		memcpy(cursor[2], "\0\0\0\0\0\0\0\0", 8);
		styler->StartAt(startPos, static_cast<char>(0xffu));
		styler->StartSegment(startPos);

		GetNextChar();
		cursorPos++;
		GetNextChar();
	}

	static int EndStyling(lua_State *L) {
		StylingContext *context = Context(L);
		context->Colourize();
		return 0;
	}

	static int StartStyling(lua_State *L) {
		StylingContext *context = Context(L);
		unsigned int startPos = static_cast<int>(luaL_checkinteger(L, 2));
		unsigned int length = static_cast<int>(luaL_checkinteger(L, 3));
        int initStyle = static_cast<int>(luaL_checkinteger(L, 4));
		context->StartStyling(startPos, length, initStyle);
		return 0;
	}

	static int More(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushboolean(L, context->currentPos < context->endPos);
		return 1;
	}

	void Forward() {
		if (currentPos < endPos) {
			atLineStart = atLineEnd;
			currentPos += lenCurrent;
			cursorPos++;
			GetNextChar();
		} else {
			atLineStart = false;
			memcpy(cursor[0], "\0\0\0\0\0\0\0\0", 8);
			memcpy(cursor[1], "\0\0\0\0\0\0\0\0", 8);
			memcpy(cursor[2], "\0\0\0\0\0\0\0\0", 8);
			atLineEnd = true;
		}
	}

	static int Forward(lua_State *L) {
		StylingContext *context = Context(L);
		context->Forward();
		return 0;
	}

	static int Position(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushinteger(L, context->currentPos);
		return 1;
	}

	static int AtLineStart(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushboolean(L, context->atLineStart);
		return 1;
	}

	static int AtLineEnd(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushboolean(L, context->atLineEnd);
		return 1;
	}

	static int State(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushinteger(L, context->state);
		return 1;
	}

	static int SetState(lua_State *L) {
		StylingContext *context = Context(L);
		context->Colourize();
		context->state = static_cast<int>(luaL_checkinteger(L, 2));
		return 0;
	}

	static int ForwardSetState(lua_State *L) {
		StylingContext *context = Context(L);
		context->Forward();
		context->Colourize();
		context->state = static_cast<int>(luaL_checkinteger(L, 2));
		return 0;
	}

	static int ChangeState(lua_State *L) {
		StylingContext *context = Context(L);
		context->state = static_cast<int>(luaL_checkinteger(L, 2));
		return 0;
	}

	static int Current(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushstring(L, context->cursor[context->cursorPos % 3]);
		return 1;
	}

	static int Next(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushstring(L, context->cursor[(context->cursorPos + 1) % 3]);
		return 1;
	}

	static int Previous(lua_State *L) {
		StylingContext *context = Context(L);
		lua_pushstring(L, context->cursor[(context->cursorPos + 2) % 3]);
		return 1;
	}

	static int Token(lua_State *L) {
		StylingContext *context = Context(L);
		Sci_Position start = context->styler->GetStartSegment();
		Sci_Position end = context->currentPos - 1;
		Sci_Position len = end - start + 1;
		if (len <= 0)
			len = 1;
		char *sReturn = new char[len+1];
		for (int i = 0; i < len; i++) {
			sReturn[i] = context->styler->SafeGetCharAt(start + i);
		}
		sReturn[len] = '\0';
		lua_pushstring(L, sReturn);
		delete []sReturn;
		return 1;
	}

	bool Match(const char *s) {
		for (int n=0; *s; n++) {
			if (*s != styler->SafeGetCharAt(currentPos+n))
				return false;
			s++;
		}
		return true;
	}

	static int Match(lua_State *L) {
		StylingContext *context = Context(L);
		const char *s = luaL_checkstring(L, 2);
		lua_pushboolean(L, context->Match(s));
		return 1;
	}

	void PushMethod(lua_State *L, lua_CFunction fn, const char *name) {
		lua_pushlightuserdata(L, this);
		lua_pushcclosure(L, fn, 1);
		lua_setfield(luaState, -2, name);
	}
};

bool LuaExtension::OnStyle(Sci_Position startPos, Sci_Position lengthDoc, int initStyle, StyleWriter *styler) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnStyle");
		if (lua_isfunction(luaState, -1)) {

			StylingContext sc; 
			sc.startPos = static_cast<unsigned int>(startPos);
			sc.lengthDoc = static_cast<int>(lengthDoc);
			sc.initStyle = initStyle;
			sc.styler = styler;
			sc.codePage = static_cast<int>(host->Send(ExtensionAPI::paneEditor, Message::GetCodePage));

			lua_newtable(luaState);

			lua_pushstring(luaState, "startPos");
			lua_pushinteger(luaState, startPos);
			lua_settable(luaState, -3);

			lua_pushstring(luaState, "lengthDoc");
			lua_pushinteger(luaState, lengthDoc);
			lua_settable(luaState, -3);

			lua_pushstring(luaState, "initStyle");
			lua_pushinteger(luaState, initStyle);
			lua_settable(luaState, -3);

			lua_pushstring(luaState, "language");
			char *lang = host->Property("Language");
			lua_pushstring(luaState, lang);
			delete []lang;
			lua_settable(luaState, -3);

			sc.PushMethod(luaState, StylingContext::Line, "Line");
			sc.PushMethod(luaState, StylingContext::CharAt, "CharAt");
			sc.PushMethod(luaState, StylingContext::StyleAt, "StyleAt");
			sc.PushMethod(luaState, StylingContext::LevelAt, "LevelAt");
			sc.PushMethod(luaState, StylingContext::SetLevelAt, "SetLevelAt");
			sc.PushMethod(luaState, StylingContext::LineState, "LineState");
			sc.PushMethod(luaState, StylingContext::SetLineState, "SetLineState");

			sc.PushMethod(luaState, StylingContext::StartStyling, "StartStyling");
			sc.PushMethod(luaState, StylingContext::EndStyling, "EndStyling");
			sc.PushMethod(luaState, StylingContext::More, "More");
			sc.PushMethod(luaState, StylingContext::Forward, "Forward");
			sc.PushMethod(luaState, StylingContext::Position, "Position");
			sc.PushMethod(luaState, StylingContext::AtLineStart, "AtLineStart");
			sc.PushMethod(luaState, StylingContext::AtLineEnd, "AtLineEnd");
			sc.PushMethod(luaState, StylingContext::State, "State");
			sc.PushMethod(luaState, StylingContext::SetState, "SetState");
			sc.PushMethod(luaState, StylingContext::ForwardSetState, "ForwardSetState");
			sc.PushMethod(luaState, StylingContext::ChangeState, "ChangeState");
			sc.PushMethod(luaState, StylingContext::Current, "Current");
			sc.PushMethod(luaState, StylingContext::Next, "Next");
			sc.PushMethod(luaState, StylingContext::Previous, "Previous");
			sc.PushMethod(luaState, StylingContext::Token, "Token");
			sc.PushMethod(luaState, StylingContext::Match, "Match");

			handled = call_function(luaState, 1);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

/*!
bool LuaExtension::OnDoubleClick() {
	return CallNamedFunction("OnDoubleClick");
}
*/
//!-start-[OnDoubleClick]
bool LuaExtension::OnDoubleClick(int modifiers) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnDoubleClick");
		if (lua_isfunction(luaState, -1)) {
			lua_pushboolean(luaState, (SCMOD_SHIFT & modifiers) != 0 ? 1 : 0); // shift/lock
			lua_pushboolean(luaState, (SCMOD_CTRL  & modifiers) != 0 ? 1 : 0); // control
			lua_pushboolean(luaState, (SCMOD_ALT   & modifiers) != 0 ? 1 : 0); // alt
			lua_pushboolean(luaState, (SCMOD_META   & modifiers) != 0 ? 1 : 0); // middle
			handled = call_function(luaState, 4);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}
//!-end-[OnDoubleClick]

//!-start-[OnClick]
bool LuaExtension::OnClick(int modifiers) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnClick");
		if (lua_isfunction(luaState, -1)) {
			lua_pushboolean(luaState, (SCMOD_SHIFT & modifiers) != 0 ? 1 : 0); // shift/lock
			lua_pushboolean(luaState, (SCMOD_CTRL  & modifiers) != 0 ? 1 : 0); // control
			lua_pushboolean(luaState, (SCMOD_ALT   & modifiers) != 0 ? 1 : 0); // alt
			lua_pushboolean(luaState, (SCMOD_META   & modifiers) != 0 ? 1 : 0); // middle
			handled = call_function(luaState, 4);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}
//!-end-[OnClick]

//!-start-[OnHotSpotReleaseClick]
bool LuaExtension::OnHotSpotReleaseClick(int modifiers) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnHotSpotReleaseClick");
		if (lua_isfunction(luaState, -1)) {
			lua_pushboolean(luaState, (SCMOD_CTRL  & modifiers) != 0 ? 1 : 0); // control
			handled = call_function(luaState, 1);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}
//!-end-[OnHotSpotReleaseClick]

//!-start-[OnMouseButtonUp]
bool LuaExtension::OnMouseButtonUp(int modifiers) {
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnMouseButtonUp");
		if (lua_isfunction(luaState, -1)) {
			lua_pushnumber(luaState, modifiers); // control
			handled = call_function(luaState, 1);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}
//!-end-[OnMouseButtonUp]
static int prevBuffer = -1;
bool LuaExtension::OnUpdateUI(bool bModified, bool bSelChange, int flag) {
	if (curBufferIndex < 0) return false;//���� ��� �������� ����������, ������� ������� ���� �� ������
	const char *res = CallNamedFunction("OnUpdateUI", bModified, bSelChange, flag, prevBuffer != curBufferIndex);
	prevBuffer = curBufferIndex;
	return res;
}

bool LuaExtension::CoOnUpdateUI(bool bModified, bool bSelChange, int flag) {
	if (curBufferIndex < 0) return false;//���� ��� �������� ����������, ������� ������� ���� �� ������
	return CallNamedFunction("CoOnUpdateUI", bModified, bSelChange, flag);
}
bool LuaExtension::PaneOnUpdateUI(bool bModified, bool bSelChange, int flag) {
	if (curBufferIndex < 0) return false;//���� ��� �������� ����������, ������� ������� ���� �� ������
	return CallNamedFunction("PaneOnUpdateUI", bModified, bSelChange, flag);
}

bool LuaExtension::OnMarginClick(unsigned int margin, unsigned int modif, long line, uptr_t id) {
	return CallNamedFunction("OnMarginClick", margin, modif, line, static_cast<unsigned int>(id));
}
bool LuaExtension::OnCallTipClick(Sci_Position pos) { 
	return CallNamedFunction("OnCallTipClick", pos, 0, 0);
}

bool LuaExtension::OnUserListSelection(int listType, const char *selection, Sci_Position id, int method) {
	return CallNamedFunction("OnUserListSelection", listType, selection, id, method) != NULL;
}

bool LuaExtension::OnNavigation(const char *item){ 
	return CallNamedFunction("OnNavigation" ,item);
}
//! bool LuaExtension::OnKey(int keyval, int modifiers) {
bool LuaExtension::OnKey(int keyval, int modifiers, char ch) { //!-chage-[OnKey]
	bool handled = false;
	if (luaState) {
		lua_getglobal(luaState, "OnKey");
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, keyval);
			lua_pushboolean(luaState, (SCMOD_SHIFT & modifiers) != 0 ? 1 : 0); // shift/lock
			lua_pushboolean(luaState, (SCMOD_CTRL  & modifiers) != 0 ? 1 : 0); // control
			lua_pushboolean(luaState, (SCMOD_ALT   & modifiers) != 0 ? 1 : 0); // alt
//!			handled = call_function(luaState, 4);
//!-start-[OnKey]
			char str[2] = {ch, 0};
			lua_pushstring(luaState, str);
			handled = call_function(luaState, 5);
//!-end-[OnKey]
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}

bool LuaExtension::OnDwellStart(Sci_Position pos, const char *word, bool ctrl) {
	return CallNamedFunction("OnDwellStart", pos, word, ctrl); 
}

bool LuaExtension::OnClose(const char *filename) {
	return CallNamedFunction("OnClose", filename);
}

//!-start-[macro]
bool LuaExtension::OnMacro(const char *func, uptr_t w, sptr_t l, const char *s){
	const char *handled = NULL;
	if (luaState) {
		lua_getglobal(luaState, "OnMacro"); 
		if (lua_isfunction(luaState, -1)) {
			lua_pushstring(luaState, "F");
			lua_pushstring(luaState, func);

			if (w) {
				lua_pushinteger(luaState, w - 1);
			} else {
				lua_pushnil(luaState);
			}

			if (l) {
				lua_pushinteger(luaState, l - 1);
			} else {
				lua_pushnil(luaState);
			}

			lua_pushstring(luaState, s);
			handled = call_sfunction(luaState, 5);
		} else {
			lua_pop(luaState, 1);
		}
	}
	return handled;
}
//!-end-[macro]

//!-start-[OnMenuCommand]
bool LuaExtension::OnMenuCommand(int cmd, int source) {
	return CallNamedFunction("OnMenuCommand", cmd, source);
}
//!-end-[OnMenuCommand]

//!-start-[OnSendEditor]
const char *LuaExtension::OnSendEditor(unsigned int msg, uptr_t wp, const char *lp) {
	return CallNamedFunction("OnSendEditor", msg, static_cast<unsigned int>(wp), lp);
}

const char *LuaExtension::OnSendEditor(unsigned int msg, uptr_t wp, sptr_t lp) { 
	return CallNamedFunction("OnSendEditor", msg, static_cast<unsigned int>(wp), static_cast<unsigned int>(lp));
}
//!-end-[OnSendEditor]
bool LuaExtension::OnLindaNotify(const char* key, const char* msg) {
	if (!strcmp(key, "print")) {
		host->Trace(msg);
		host->Trace("\n");
	}
	return CallNamedFunction("OnLindaNotify", key);
}

bool LuaExtension::OnColorized(Sci_Position wp, Sci_Position lp) {
	return CallNamedFunction("OnColorized", wp, lp);
}

const char *LuaExtension::OnContextMenu(unsigned int msg, unsigned int wp, const char *lp) {
	return CallNamedFunction("OnContextMenu", msg, wp, lp);
}

bool LuaExtension::OnFindProgress(uptr_t state, sptr_t all) {
	return CallNamedFunction("OnFindProgress", state, all);
}

bool LuaExtension::OnIdle() {
	return CallNamedFunction("OnIdle");
}

bool LuaExtension::OnLayOutNotify(const char *command) {
	return CallNamedFunction("OnLayOutNotify", command);
}
bool LuaExtension::OnGeneratedHotKey(long hotkey) {
	return CallNamedFunction("event_MenuHotKey", hotkey, (LUA_INTEGER)0);
}

void LuaExtension::DoReboot(){
	
cf_editor_reload_startup_script(luaState);
}

void LuaExtension::DoLua(const char *c) {
	CallNamedFunction("dostring", c);
}

bool LuaExtension::OnMacroBlocked(int msg, intptr_t wParam, intptr_t lParam) {
	return CallNamedFunction("OnMacroBlockedEvents", msg, wParam, lParam);
}

sptr_t  LuaExtension::OnMenuChar(int flag, const char* key) {
	//return CallNamedFunction("event_MenuChar", flag, key);

	sptr_t r = 0;
	if (luaState) {
		lua_getglobal(luaState, "event_MenuChar");
		if (lua_isfunction(luaState, -1)) {
			lua_pushinteger(luaState, flag);
			lua_pushstring(luaState, key);

			int result = lua_pcall(luaState, 2, 1, false); 
			if (0 == result) {
				r = (sptr_t)lua_tointeger(luaState, -1);
				lua_pop(luaState, 1);
			}
			else  {
				procrssError(luaState, result);
			}

		}
		else {
			lua_pop(luaState, 1);
		}
	}
	return r;
}

void LuaExtension::OnMouseHook(int x, int y) {
	CallNamedFunction("event_MenuMouseHook", x, y);
}
void LuaExtension::OnChangeFocus(int src, int focus) {
	CallNamedFunction("OnChangeFocus", src, focus);
}

bool LuaExtension::OnDrawClipboard(int flag){
	return CallNamedFunction("OnDrawClipboard", flag, (LUA_INTEGER)0);
}

void LuaExtension::OnRightEditorVisibility(bool show) {
	CallNamedFunction("OnRightEditorVisibility", show, (LUA_INTEGER)0);
}

void LuaExtension::OnTextChanged(Sci_Position position, int leg, Sci_Position linesAdded, int flag) {
	CallNamedFunction("OnTextChanged", position, flag, linesAdded, leg); 
}

void LuaExtension::BeforeTextChanged(Sci_Position position, int leg, const char* t, int type) {
	CallNamedFunction("BeforeTextChanged", leg, t, position, type) ;
}

void LuaExtension::OnCurrentLineFold(Sci_Position line, int leg, int foldLevelPrev, int foldLevelNow) {
	CallNamedFunction("OnCurrentLineFold", line, foldLevelPrev, foldLevelNow);
}

bool LuaExtension::OnAutocSelection(int method, Sci_Position firstPos) { 
	return CallNamedFunction("OnAutocSelection", method, firstPos);
}

void LuaExtension::OnCommandLine(const char* line) { 
	CallNamedFunction("OnCommandLine", line);
}

GUI::gui_string LuaExtension::LocalizeText(const char* msg) {
	if (luaState) {
		lua_getglobal(luaState, "_LocalizeText");
		if (lua_isfunction(luaState, -1)) {
			lua_pushstring(luaState, msg);
			const char* handled = call_sfunction(luaState, 1);
			if(handled)
				return GUI::StringFromUTF8(handled);
		}
	} 
	return GUI::StringFromUTF8(msg);
}

int LuaExtension::HildiAlarm(const char* msg, int flag, const GUI::gui_char *p1 = NULL, const GUI::gui_char *p2 = NULL, const GUI::gui_char *p3 = NULL) {
	int handled = NULL; 
	if (luaState) {
		lua_getglobal(luaState, "_HildiAlarm");
		if (lua_isfunction(luaState, -1)) {
			//GUI::ConvertToUTF8(p1.c_str(), CP_ACP);
			std::string ps1, ps2, ps3;
			if(p1)
				ps1 = GUI::UTF8FromString(p1);
			if(p2)
				ps2 = GUI::UTF8FromString(p2);
			if(p3)
				ps3 = GUI::UTF8FromString(p3);
			lua_pushstring(luaState, msg);
			lua_pushinteger(luaState, flag);
			int cnt = 2;
			if (p1) {
				lua_pushstring(luaState, ps1.c_str());
				cnt++;
			}
			if (p2) {
				lua_pushstring(luaState, ps2.c_str());
				cnt++;
			}
			if (p3) {
				lua_pushstring(luaState, ps3.c_str());
				cnt++;
			}
			if (!lua_pcall(luaState, cnt, 1, 0)) {
				handled = static_cast<int>(lua_tointeger(luaState, -1));
				lua_pop(luaState, 1);
				return handled;
			}
		} else {
			lua_pop(luaState, 1);
		}
	}

	return host->WindowMessageBox(msg, flag, p1, p2, p3);
}

static int cf_editor_reload_startup_script(lua_State*) {

	FilePath fpTest(GUI::StringFromUTF8(startupScript));
	if (fpTest.Exists()) {
		luaL_loadfile(luaState, startupScript);
		if (!call_function(luaState, 0, 0)) {
			host->Trace(">Lua: error occurred while loading startup script\n");
		}
		host->PostLoadScript();
	}
	return 0;
}

#ifdef _MSC_VER
// Unreferenced inline functions are OK
#pragma warning(disable: 4514)
#endif
