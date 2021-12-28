/** \file
 * \brief Cairo Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cddirect2d.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"


static void *cddbuf_checkdata(lua_State * L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluadbufctx = 
{
  0,
  "DIRECT2D_DBUFFER",
  cdContextDirect2DDBuffer,
  cddbuf_checkdata,
  NULL,
  0
};

static void *cdimage_checkdata(lua_State *L, int param)
{
  return cdlua_checkimage(L, param);
}

static cdluaContext cdluaimagectx = 
{
  0,
  "DIRECT2D_IMAGE",
  cdContextDirect2DImage,
  cdimage_checkdata,
  NULL,
  0
};

static void *cdimagergb_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluaimagergbctx = 
{
  0,
  "DIRECT2D_IMAGERGB",
  cdContextDirect2DImageRGB,
  cdimagergb_checkdata,
  NULL,
  0
};

static void *cdnativewindow_checkdata(lua_State *L, int param)
{
  if (lua_isnil(L,param))
    return NULL;
  if (lua_isuserdata(L,param))
    return lua_touserdata(L,param);
  if (lua_isstring(L, param))
    return (void *)lua_tostring(L,param);

  luaL_argerror(L, param, "data should be of type userdata or a string, or a nil value");
  return NULL;
}

static cdluaContext cdluanativewindowctx = 
{
  0,
  "DIRECT2D_NATIVEWINDOW",
  cdContextDirect2DNativeWindow,
  cdnativewindow_checkdata,
  NULL,
  0
};

static void *cdprinter_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluaprinterctx = 
{
  0,
  "DIRECT2D_PRINTER",
  cdContextDirect2DPrinter,
  cdprinter_checkdata,
  NULL,
  0
};

static int cdlua5_initdirect2d(lua_State *L)
{
  (void)L;
  cdInitDirect2D();
  return 0;
}

static int cdlua5_finishdirect2d(lua_State *L)
{
  (void)L;
  cdFinishDirect2D();
  return 0;
}

static const struct luaL_Reg funcs[] = {
  {"InitDirect2D", cdlua5_initdirect2d},
  {"FinishDirect2D", cdlua5_finishdirect2d},
  {NULL, NULL},
};


static int cdluadirect2d_open (lua_State *L)
{
  cdluaLuaState* cdL = cdlua_getstate(L);
  cdlua_register_lib(L, funcs);  /* leave cd table at the top of the stack */
  cdInitDirect2D();
  cdlua_addcontext(L, cdL, &cdluaimagectx);
  cdlua_addcontext(L, cdL, &cdluaimagergbctx);
  cdlua_addcontext(L, cdL, &cdluanativewindowctx);
  cdlua_addcontext(L, cdL, &cdluaprinterctx);
  cdlua_addcontext(L, cdL, &cdluadbufctx);
  return 1;
}

int luaopen_cdluadirect2d(lua_State* L)
{
  return cdluadirect2d_open(L);
}
