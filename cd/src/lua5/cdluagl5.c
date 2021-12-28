/** \file
 * \brief OpenGL Canvas Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdgl.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdluagl.h"
#include "cdlua5_private.h"


static void *cdgl_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluaglctx = 
{
  0,
  "GL",
  cdContextGL,
  cdgl_checkdata,
  NULL,
  0
};

static const luaL_Reg funcs[] = {
  { NULL, NULL },
};

int cdluagl_open (lua_State *L)
{
  cdluaLuaState* cdL = cdlua_getstate(L);
  cdlua_register_lib(L, funcs);  /* leave cd table at the top of the stack */
  cdlua_addcontext(L, cdL, &cdluaglctx);
  return 1;
}

int luaopen_cdluagl(lua_State* L)
{
  return cdluagl_open(L);
}

