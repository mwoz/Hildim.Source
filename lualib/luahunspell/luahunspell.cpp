// luahunspell.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <cstring>
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#define LUASPELL "spell"

static Hunspell* CheckH(lua_State* L) {
	return *reinterpret_cast<Hunspell**>(luaL_checkudata(L, 1, LUASPELL));
}

/**
h:add_dic(dic_path [, key]) -> []
load extra dictionaries (only dic files)
*/
static int l_add_dic(lua_State *L) {

	const char *dic = luaL_checkstring(L, 2);
	const char *key = luaL_optstring(L, 3, NULL);
	CheckH(L)->add_dic(dic, key);
	return 0;
}

/**
h:spell(word) -> [boolean]
returns true, if the word is spelled correctly
*/
static int l_spell(lua_State *L) {

	const std::string word = luaL_checkstring(L, 2);
	int ret = CheckH(L)->spell(word);
	lua_pushboolean(L, ret);
	return 1;
}

static int l_destroy(lua_State *L) {
	CheckH(L)->~Hunspell();
	return 0;
}

/**
h:suggest(word) -> [table]
returns a table of suggestions for the word (or empty table)
*/
static int l_suggest(lua_State* L) {

	const std::string word = luaL_checkstring(L, 2);
	Hunspell* sp = CheckH(L);
	std::vector<std::string> vs = sp->suggest(word);
	int n = vs.size();
	lua_createtable(L, n, 0);
	if (n > 0) {
		for (int i = 0; i < n; i++) {
			lua_pushstring(L, vs[i].c_str());
			lua_rawseti(L, -2, i + 1);
		}		
	}
	return 1;
}
/**
h:stem(word)->[table]
returnsatableofstemsofword
*/

static int l_stem(lua_State *L) {

	const std::string word = luaL_checkstring(L, 2);
	Hunspell* sp = CheckH(L);
	std::vector<std::string> vs = sp->stem(word);
	int n = vs.size();
	lua_createtable(L, n, 0);
	if (n > 0) {
		for (int i = 0; i < n; i++) {
			lua_pushstring(L, vs[i].c_str());
			lua_rawseti(L, -2, i + 1);
		}

		
	}
	return 1;
}

/**
h:add_word(word) -> void
adds a word to the dictionary
*/
static int l_add_word(lua_State *L) {
	const std::string word = luaL_checkstring(L, 2);
	CheckH(L)->add(word);
	return 0;
}

static int l_add_with_affix(lua_State *L) {
	const std::string word = luaL_checkstring(L, 2);
	const std::string example = luaL_checkstring(L, 3);
	std::string flags;
	CheckH(L)->add_with_affix(word,example,flags);
	lua_pushstring(L, flags.c_str());
	return 1;
}


luaL_Reg spell_methods[] = {
	{"add_dic", l_add_dic},
	{"spell", l_spell},
	{"stem", l_stem},
	{"suggest", l_suggest},
	{"add_word", l_add_word},
	{"add_with_affix", l_add_with_affix},
	{"destroy", l_destroy},
	{NULL, NULL}
};



static int g_spell(lua_State *L) {
	const char *aff = luaL_checkstring(L, 1);
	const char *dic = luaL_checkstring(L, 2);
	const char *key = luaL_optstring(L, 3, NULL);

	Hunspell *hs = new Hunspell(aff, dic, key);
	Hunspell **lhs = (Hunspell**) lua_newuserdata(L, sizeof(Hunspell*));
	*lhs = hs;

	luaL_getmetatable(L, LUASPELL);
	lua_setmetatable(L, -2);

	return 1;
}

luaL_Reg luaspell_methods[] = {
	{"Create", g_spell},
	{NULL, NULL}
};

static void createMetatable(lua_State *L, const char *name, luaL_Reg *methods) {
	luaL_newmetatable(L, name);
	luaL_setfuncs(L, methods, 0);
	lua_getfield(L, -1, "__index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
}

 
extern "C" __declspec(dllexport) int luaopen_luahunspell(lua_State *L) {
		// copy locale from environment
		//setlocale(LC_ALL, "");

	createMetatable(L, LUASPELL, spell_methods);
	luaL_setfuncs(L, luaspell_methods, 0);

		///lua_pushcfunction(L, g_spell);
		return 1;
	}
