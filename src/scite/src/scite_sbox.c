/** \file
 * \brief IupSBox control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>

#include "lua.h"
#include "lauxlib.h"
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
#include "../../iup/src/iup_register.h"
#include "../../iup/srclua5/il.h"
#include "scite_sbox.h"

#define ISBOX_THICK 5
enum { ISBOX_NORTH, ISBOX_SOUTH, ISBOX_WEST, ISBOX_EAST };

struct _IcontrolData
{
	int w, h;
	int isholding;
	int start_x, start_y;
	int start_w, start_h;

	int direction;     /* one of the types: ISBOX_NORTH, ISBOX_SOUTH, ISBOX_WEST, ISBOX_EAST */
};

static char* iSBoxGetValueAttribute(Ihandle* ih)
{
	if (ih->data->direction == ISBOX_WEST || ih->data->direction == ISBOX_EAST)
	{
		return iupStrReturnInt(ih->data->w);
	}
	else 
	{
		return iupStrReturnInt(ih->data->h);
	}
}

static int iSBoxSetValueAttribute(Ihandle* ih, const char* value)
{
	int val;
	iupStrToInt(value, &val);
	
	if (ih->data->direction == ISBOX_WEST || ih->data->direction == ISBOX_EAST)
	{
		if (val != ih->data->w)
		{
			ih->data->w = val;
			iupLayoutApplyMinMaxSize(ih, &(ih->data->w), &(ih->data->h));
			IupRefresh(ih);  /* may affect all the elements in the dialog */
		}
	}
	else if (ih->data->direction == ISBOX_SOUTH || ih->data->direction == ISBOX_NORTH)
	{
		if (val != ih->data->h)
		{
			ih->data->h = val;
			iupLayoutApplyMinMaxSize(ih, &(ih->data->w), &(ih->data->h));
			IupRefresh(ih);  /* may affect all the elements in the dialog */
		}
	}
	return 0;
}

static int isc_SBoxMapMethod(Ihandle* ih)
{
	/* must be first */
	
	char *clr = IupGetAttribute(ih, "SCR_FORECOLOR");
	if(clr)
		IupSetAttribute(ih, "COLOR", clr);

	return IUP_NOERROR;
}

void Iupsc_SBoxOpen(void)
{
	static int run = 0;
	if (run) return;
	run = 1;
	iupRegisterClass(iupsc_SBoxNewClass());
}

Iclass* iupsc_SBoxNewClass(void)
{
    Iclass* ic = iupClassNew(iupRegisterFindClass("sbox"));
	ic->Map = isc_SBoxMapMethod;
    
    ic->name   = "sc_sbox";
    ic->format = "h";   /* one Ihandle* */
    ic->nativetype = IUP_TYPEVOID;
    ic->childtype  = IUP_CHILDMANY+2; /* canvas+child */
    ic->is_interactive = 0;
	iupClassRegisterAttribute(ic, "VALUE", iSBoxGetValueAttribute, iSBoxSetValueAttribute, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* Iupsc_SBox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("sc_sbox", children);
}

static int sc_SBox(lua_State *L)
{
	Ihandle *ih = Iupsc_SBox(iuplua_checkihandleornil(L, 1));
	iuplua_plugstate(L, ih);
	iuplua_pushihandle_raw(L, ih);
	return 1;
}

int iupsc_SBoxlua_open(lua_State * L)
{
	static int run = 0;	  	  // if (!run)
	 iuplua_register(L, sc_SBox, "sc_SBox");	
	iuplua_dostring(L,
		"local ctrl = { nick = 'sc_sbox', parent = iup.BOX, subdir = 'elem', creation = 'I',  funcname = 'sc_SBox', }; function ctrl.createElement(class, param) return iup.sc_SBox() end iup.RegisterWidget(ctrl); iup.SetClass(ctrl, 'iupWidget')",
		"sc_SBox.lua");
	run = 1;
	return 0;
}