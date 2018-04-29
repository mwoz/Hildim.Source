/** \file
 * \brief Canvas Control.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include "lauxlib.h"

#include "iup.h"
#include "iupcbs.h"
#include "iupdraw.h"

#include "../../iup/src/iup_object.h"
#include "../../iup/src/iup_attrib.h"
#include "../../iup/src/iup_str.h"
#include "../../iup/src/iup_drv.h"
#include "../../iup/src/iup_drvfont.h"
#include "../../iup/src/iup_stdcontrols.h"
#include "../../iup/src/iup_layout.h"
#include "scite_scrollcanvas.h"



static int iScrollCanvasCreateMethod(Ihandle* ih, void** params)
{
	/* must be first */
	IupSetAttribute(ih, "BORDER", "NO");
	iupFlatScrollBarCreate(ih);
  return IUP_NOERROR;
}

static void iScrollCanvasSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink) {
	if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
		iupFlatScrollBarSetChildrenCurrentSize(ih, shrink);
}

static void iScrollCanvasSetChildrenPositionMethod(Ihandle* ih, int x, int y) {
	if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
		iupFlatScrollBarSetChildrenPosition(ih);

	(void)x;
	(void)y;
}

/******************************************************************************/


void IupScrollCanvasOpen(void) {
	static int run = 0;
	if (run) return;
	run = 1;
	iupRegisterClass(iupScrollCanvasNewClass());
}

Iclass* iupScrollCanvasNewClass(void)
{
	Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name = "scrollcanvas";
  ic->format = NULL;
  ic->nativetype = IUP_TYPECANVAS;

  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 1;

  /* Class functions */
  ic->New = iupScrollCanvasNewClass;
  ic->Create = iScrollCanvasCreateMethod;
  ic->SetChildrenCurrentSize = iScrollCanvasSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iScrollCanvasSetChildrenPositionMethod;

  /* Flat Scrollbar */
  iupFlatScrollBarRegister(ic);

  iupClassRegisterAttribute(ic, "FLATSCROLLBAR", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupScrollCanvas(void) {
	return IupCreatev("scrollcanvas", NULL);// (void**)children);
}

static int ScrollCanvas(lua_State *L) {
	Ihandle *ih = IupScrollCanvas();//iuplua_checkihandleornil(L, 1)
	iuplua_plugstate(L, ih);
	iuplua_pushihandle_raw(L, ih);
	return 1;
}

int iupIupScrollCanvaslua_open(lua_State * L) {
	static int run = 0;	  	  // if (!run)
	iuplua_register(L, ScrollCanvas, "ScrollCanvas");
	iuplua_dostring(L,
		"local ctrl = {  nick = 'scrollcanvas_ctrl',  parent = iup.BOX,  subdir = 'elem',  creation = 'I',  funcname = 'ScrollCanvas', };function ctrl.createElement(class, param)  return iup.ScrollCanvas() end; iup.RegisterWidget(ctrl); iup.SetClass(ctrl, 'iupWidget')",
		"scrollcanvas.lua");
	run = 1;
	return 0;
}