// luahunspell.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#define MT "spell"

#define THIS \
	Hunspell *sp = *(Hunspell**) luaL_checkudata(L, 1, MT)


//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CluaspellApp

//BEGIN_MESSAGE_MAP(CluaspellApp, CWinApp)
//END_MESSAGE_MAP()
#define RETURN_LIST               \
	lua_createtable(L, n, 0);       \
	if (n>0) {                      \
	for (int i=0; i<n; i++) {     \
	lua_pushstring(L, list[i]); \
	lua_rawseti(L, -2, i+1);    \
	}                             \
	}                               \
	sp->free_list(&list, n);        \
	return 1

/**
h:add_dic(dic_path [, key]) -> []
load extra dictionaries (only dic files)
*/
static int l_add_dic(lua_State *L) {
	THIS;

	const char *dic = luaL_checkstring(L, 2);
	const char *key = luaL_optstring(L, 3, NULL);
	sp->add_dic(dic, key);
	return 0;
}

/**
h:spell(word) -> [boolean]
returns true, if the word is spelled correctly
*/
static int l_spell(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);
	//int info, captype;
	//int ret = sp->spell(word, &info, NULL, &captype);
	int ret = sp->spell(word);
	lua_pushboolean(L, ret);
	//lua_pushinteger(L, info);
	//lua_pushinteger(L, captype);
	return 1;
}

static int l_destroy(lua_State *L) {
	THIS;
	sp->~Hunspell();
	return 0;
}

/**
h:suggest(word) -> [table]
returns a table of suggestions for the word (or empty table)
*/
static int l_suggest(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);
	char ** list = NULL;
	int n = sp->suggest(&list, word);
	RETURN_LIST;
}

/**
h:analyze(word) -> [table]
returns a table with morphological analysis of word
*/
static int l_analyze(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);
	char **list = NULL;

	int n = sp->analyze(&list, word);
	RETURN_LIST;
}

/**
h:stem(word) -> [table]
returns a table of stems of word
*/
static int l_stem(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);
	char **list;

	int n = sp->stem(&list, word);
	RETURN_LIST;
}

/**
h:generate(word, example:string) -> [table]
generate word(s) by example

h:generate(word, desc:table) -> [table]
generate word(s) by description (dictionary dependent)
*/
static int l_generate(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);

	if (lua_type(L, 3) == LUA_TSTRING) {
		const char * example = luaL_checkstring(L, 3);
		char **list;
		int n = sp->generate(&list, word, example);
		RETURN_LIST;
	} else if (lua_type(L, 3) == LUA_TTABLE) {
		int howmany = lua_objlen(L, 3);
		char ** desc = (char**) calloc(howmany, sizeof(char*));
		for (int i=0; i<howmany; i++) {
			lua_rawgeti(L, 3, i+1);
			desc[i] = (char*) lua_tostring(L, -1); // nasty
		}

		char **list;
		int n = sp->generate(&list, word, desc, howmany);

		free(desc);
		RETURN_LIST;
	} else {
		return luaL_argerror(L, 3, "string or table expected");
	}
}

/**
h:add_word(word) -> void
adds a word to the dictionary
*/
static int l_add_word(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);
	sp->add(word);
	return 0;
}

static int l_add_with_affix(lua_State *L) {
	THIS;

	const char * word = luaL_checkstring(L, 2);
	const char * example = luaL_checkstring(L, 3);
	char str[500];
	char* flags = str;
	flags[0] = '\0';
	sp->add_with_affix(word,example,&flags);
	lua_pushstring(L, str);
	return 1;
}

static int l_get_dic_encoding(lua_State *L) {
	THIS;

	char* encoding = sp->get_dic_encoding();
	lua_pushstring(L, encoding);
	return 1;
}

luaL_Reg spell_methods[] = {
	{"add_dic", l_add_dic},
	{"analyze", l_analyze},
	{"generate", l_generate},
	{"spell", l_spell},
	{"stem", l_stem},
	{"suggest", l_suggest},
	{"add_word", l_add_word},
	{"add_with_affix", l_add_with_affix},
	{ "get_dic_encoding", l_get_dic_encoding },
	{ "destroy", l_destroy },
	{NULL, NULL}
};

static int g_spell(lua_State *L) {
	const char *aff = luaL_checkstring(L, 1);
	const char *dic = luaL_checkstring(L, 2);
	const char *key = luaL_optstring(L, 3, NULL);

	Hunspell *hs = new Hunspell(aff, dic, key);
	Hunspell **lhs = (Hunspell**) lua_newuserdata(L, sizeof(Hunspell*));
	*lhs = hs;

	luaL_getmetatable(L, MT);
	lua_setmetatable(L, -2);

	return 1;
}

static void createMetatable(lua_State *L, const char *name, luaL_Reg *methods) {
	luaL_newmetatable(L, name);
	luaL_register(L, NULL, methods);
	lua_getfield(L, -1, "__index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
}

extern "C" {
	LUA_API int luaopen_luahunspell(lua_State *L) {
		// copy locale from environment
		//setlocale(LC_ALL, "");

		createMetatable(L, MT, spell_methods);

		lua_pushcfunction(L, g_spell);
		return 1;
	}
}
