/******************************************************************************
 * Automatically generated file. Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


 
static int FontIcon (lua_State * L)
{
	Ihandle* ih = IupFontIcon((char*)luaL_checkstring(L, 1));
	iuplua_plugstate(L, ih);
	iuplua_pushihandle_raw(L, ih);
	return 1;
} 
 
int iupfonticonlua_open(lua_State * L)
{
  iuplua_register(L, FontIcon, "FontIcon");


#ifdef IUPLUA_USELOH
#include "fonticon.loh"
#else
#ifdef IUPLUA_USELH
#include "fonticon.lh"
#else
  iuplua_dofile(L, "fonticon.lua");
#endif
#endif

  return 0;
}

