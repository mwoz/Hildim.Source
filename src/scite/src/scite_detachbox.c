/** \file
 * \brief IupDetachBox control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "iup.h"
#include "iuplua.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "../../iup/src/iup_object.h"
#include "../../iup/src/iup_attrib.h"
#include "../../iup/src/iup_str.h"
#include "../../iup/src/iup_drv.h"
#include "../../iup/src/iup_stdcontrols.h"
#include "../../iup/src/iup_layout.h"
#include "../../iup/src/iup_childtree.h"
#include "../../iup/src/iup_draw.h"
#include "scite_detachbox.h"

struct _IcontrolData
{
	/* aux */
	int is_holding;
	Ihandle *old_parent, *old_brother;

	/* attributes */
	int layoutdrag, barsize, showgrip;
	int orientation;     /* one of the types: IDBOX_VERT, IDBOX_HORIZ */
};


int sc_ShowXY(Ihandle *ih, int x, int y)
{
	int ret;

	//iupASSERT(iupObjectCheck(ih));
	if (!iupObjectCheck(ih))
		return IUP_INVALID;

	if (ih->iclass->nativetype != IUP_TYPEDIALOG)
	{
		//iupERROR("Must be a dialog in IupShowXY.");
		return IUP_INVALID;
	}

	ret = IupMap(ih);
	if (ret == IUP_ERROR)
		return ret;

	//ret = iupDialogShowXY(ih, x, y);
	//if (ret != IUP_NOERROR)
	//{
	//	iupERROR("Error during IupShowXY.");
	//	return ret;
	//}

	return IUP_NOERROR;
}



static int iDetachBoxSetDetachAttribHidden(Ihandle* ih, const char* value)
{
	//int cur_x, cur_y;
	IFnnii detachedCB = (IFnnii)IupGetCallback(ih, "DETACHED_CB");

	/* Create new dialog */
	Ihandle *new_parent = IupDialog(NULL);
	Ihandle *old_dialog = IupGetDialog(ih);

	/* Set new dialog as child of the current application */
	IupSetAttributeHandle(new_parent, "PARENTDIALOG", old_dialog);

	//iupStrToIntInt(IupGetGlobal("CURSORPOS"), &cur_x, &cur_y, 'x');

	if (detachedCB)
	{
		int ret = detachedCB(ih, new_parent, 0, 0);
		if (ret == IUP_IGNORE)
		{
			IupDestroy(new_parent);
			return IUP_DEFAULT;
		}
	}

	/* set user size of the detachbox as the current size of the child */
	IupSetStrAttribute(ih, "RASTERSIZE", IupGetAttribute(ih->firstchild->brother, "RASTERSIZE"));

	/* Save current parent and reference child */
	ih->data->old_parent = ih->parent;
	ih->data->old_brother = ih->brother;

	IupMap(new_parent);

	/* Sets the new parent */
	IupReparent(ih, new_parent, NULL);

	/* Hide handler */
	IupSetAttribute(ih->firstchild, "VISIBLE", "No");

	/* force a dialog resize since IupMap already computed the dialog size */
	IupSetAttribute(new_parent, "RASTERSIZE", NULL);

	/* Maps and shows the new dialog */
	//sc_ShowXY(new_parent, cur_x, cur_y);
	IupMap(ih);

	/* reset user size of the detachbox */
	IupSetAttribute(ih, "USERSIZE", NULL);

	/* Updates/redraws the layout of the old dialog */
	IupRefresh(old_dialog);

	(void)value;
	return 0;
}

void Iupsc_DetachBoxOpen(void)
{
	static int run = 0;
	if (run) return;
	run = 1;
	iupRegisterClass(iupsc_DetachBoxNewClass());
}

Iclass* iupsc_DetachBoxNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("detachbox"));

  ic->name   = "sc_detachbox";
  ic->format = "h";   /* one Ihandle* */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY+2; /* canvas+child */
  ic->is_interactive = 0;

  iupClassRegisterAttribute(ic, "DETACHHIDDEN", NULL, iDetachBoxSetDetachAttribHidden, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);


  return ic;
}

Ihandle* Iupsc_DetachBox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("sc_detachbox", children);
}




static int sc_DetachBox(lua_State *L)
{
	Ihandle *ih = Iupsc_DetachBox(iuplua_checkihandleornil(L, 1));
	iuplua_plugstate(L, ih);
	iuplua_pushihandle_raw(L, ih);
	return 1;
}

int iupsc_detachboxlua_open(lua_State * L)
{
	static int run = 0;	  	  // if (!run)
	 iuplua_register(L, sc_DetachBox, "sc_DetachBox");	
	iuplua_dostring(L,
		"local ctrl = {  nick = 'sc_detachbox',  parent = iup.BOX,  subdir = 'elem',  creation = 'I',  funcname = 'sc_DetachBox', };function ctrl.createElement(class, param)  return iup.sc_DetachBox() end; iup.RegisterWidget(ctrl); iup.SetClass(ctrl, 'iupWidget')",
		"sc_detachbox.lua");
	run = 1;
	return 0;
}