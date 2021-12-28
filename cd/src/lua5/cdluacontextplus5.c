/** \file
 * \brief Context Plus Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua5_private.h"

static int cdlua5_finishcontextplus(lua_State *L)
{
  (void)L;
  cdFinishContextPlus();
  return 0;
}

static int cdlua5_initcontextplus(lua_State *L)
{
  (void)L;
  cdInitContextPlus();
  return 0;
}

static const luaL_Reg cdlib[] = {
  {"InitContextPlus", cdlua5_initcontextplus},
  {"FinishContextPlus", cdlua5_finishcontextplus},
  {NULL, NULL},
};


static int cdluacontextplus_open (lua_State *L)
{
  cdInitContextPlus();
  cdlua_register_lib(L, cdlib);   /* leave cd table at the top of the stack */
  return 1;
}

int luaopen_cdluacontextplus(lua_State* L)
{
  return cdluacontextplus_open(L);
}
