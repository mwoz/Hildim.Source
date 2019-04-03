/** \file
 * \brief IupSBox control
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <commctrl.h>

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
#include "../../iup/src/win/iupwin_handle.h"
#include "../../iup/src/win/iupwin_drv.h"
#include "scite_tree.h"

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

static int sctree_branchclose_cb(Ihandle *self, int p0) {
	lua_State *L = iuplua_call_start(self, "flat_branchclose_cb");
	lua_pushinteger(L, p0);
	return iuplua_call(L, 1);
}

static int sctree_branchopen_cb(Ihandle *self, int p0) {
	lua_State *L = iuplua_call_start(self, "flat_branchopen_cb");
	lua_pushinteger(L, p0);
	return iuplua_call(L, 1);
}

static int sctree_selection_cb(Ihandle *self, int p0, int p1) {
	lua_State *L = iuplua_call_start(self, "flat_selection_cb");
	lua_pushinteger(L, p0);
	lua_pushinteger(L, p1);
	return iuplua_call(L, 2);
}

static int winTreeProc(Ihandle* ih, HWND cbedit, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result) {
	switch (msg) {
	case WM_HSCROLL:
		break;
	case WM_VSCROLL:
		break;
	case WM_MOUSEWHEEL:
	{
		short delta = HIWORD(wp);// > 120 ? 1 : -1;
		IupSetInt(ih->parent, "POSY", IupGetInt(ih->parent, "POSY") - delta);
		return 1;
	}
	break;
	}

	(void)wp;
	(void)cbedit;
	(void)ih;
	return 0;
}

static int bRefresh = FALSE;

static LRESULT CALLBACK winTreeWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	int ret = 0;
	LRESULT result = 0;
	WNDPROC oldProc;
	Ihandle *ih;

	ih = iupwinHandleGet(hwnd);
	if (!ih)
		return DefWindowProc(hwnd, msg, wp, lp);  /* should never happen */

												  /* retrieve the control previous procedure for subclassing */
	oldProc = (WNDPROC)IupGetCallback(ih, "_IUPWIN_TREEOLDWNDPROC_CB");

	ret = winTreeProc(ih, hwnd, msg, wp, lp, &result);

	if (ret)
		return result;
	else
		ret = CallWindowProc(oldProc, hwnd, msg, wp, lp);
	if (bRefresh) {
		bRefresh = FALSE;
		IupRefresh(ih);
	}
	return ret;

}

static int isc_SetNoScrollSize_recr(Ihandle* ih, HDC hdc, int item, long *pWidth, int level, int h, int iAct, int bOpen) {
	SIZE size;
	int dLevel = SendMessage(ih->handle, TVM_GETINDENT, 0, 0) + 1;
	for (; ; ) {
		char *str = IupGetAttributeId(ih, "TITLE", item);
		if (!str)
			return h;
		GetTextExtentPoint32A(hdc, str, strlen(str), &size);
		
		h++;
		if (*pWidth < size.cx + level)
			*pWidth = size.cx + level;
		if (!strcmp(IupGetAttributeId(ih, "KIND", item), "BRANCH") &&
			((!strcmp(IupGetAttributeId(ih, "STATE", item),"EXPANDED") && (iAct != item)) || (iAct == item && bOpen )) ) {
			h = isc_SetNoScrollSize_recr(ih, hdc, item + 1, pWidth, level + dLevel, h, iAct, bOpen);
		}
		int next;
		if (!iupStrToInt(IupGetAttributeId(ih, "NEXT", item), &next))
			break;
		item = next;
	}
	return h;
}

static void isc_SetNoScrollSize(Ihandle* ih, int iAct, int bOpen) {
	int cnt = IupGetInt(ih, "COUNT");
	int w = 0;
	HDC hdc = GetDC(ih->handle);
	int iH = SendMessage(ih->handle, TVM_GETITEMHEIGHT, 0, 0);
	int l1 = SendMessage(ih->handle, TVM_GETINDENT, 0, 0) + 21;
	
	HFONT hOldFont, hFont = (HFONT)iupwinGetHFont(IupGetAttribute(ih, "FONT"));
	hOldFont = SelectObject(hdc, hFont);

	int h = isc_SetNoScrollSize_recr(ih, hdc, 0 , &w, l1, 1, iAct, bOpen) * iH;

	SelectObject(hdc, hOldFont);
	ReleaseDC(ih->handle, hdc);

	IupSetAttribute(ih, "RASTERSIZE", iupStrReturnIntInt(w, h, 'x'));
	if(bOpen)
		IupRefresh(ih);
	else
		bRefresh = TRUE;
}

static int isc_GetTopItem_Recr(Ihandle* ih,  int y, int item, int ItemFound, BOOL * bFind, int iH) {
	for (; ; ) {
		if (item >= ItemFound){
			*bFind = TRUE;
			return item == ItemFound ? y : -1;
		}
		y += iH;
		if (!strcmp(IupGetAttributeId(ih, "KIND", item), "BRANCH") && (!strcmp(IupGetAttributeId(ih, "STATE", item), "EXPANDED")))
		{
			y = isc_GetTopItem_Recr(ih, y , item + 1, ItemFound, bFind, iH);
			if (*bFind)
				return y;

		}
		int next;
		iupStrToInt(IupGetAttributeId(ih, "NEXT", item), &next);
		if (next <= item)
			break;
		item = next;
	}
	*bFind = 0;
	return y;
}

static void iscTreeFlatTopitemAttrib_int(Ihandle* ih, int item) {

	BOOL bFound = FALSE;

	int x, y, delta = 0;
	int iH = (int)SendMessage(ih->handle, TVM_GETITEMHEIGHT, 0, 0);
	IupGetIntInt(ih->parent,"RASTERSIZE", &x, &y);
	if (y >= iH * 2)
		delta = iH/2;

	int curY = isc_GetTopItem_Recr(ih, 0, 0, item, &bFound, iH);
	if (bFound) {
		int posy = IupGetInt(ih->parent, "POSY");
		int dy = IupGetInt(ih->parent, "DY");
		if (curY < posy) {
			IupSetInt(ih->parent, "POSY", curY - 10);

		} else if (curY + iH > posy + dy) {
			IupSetInt(ih->parent, "POSY", curY + iH - dy + 10);
		}
	}
}

static int iscTreeFlatTopitemAttrib(Ihandle* ih, const char* value) {
	
	int item;
	if (iupStrToInt(value, &item)) {
		IupSetAttribute(ih, "TOPITEM", value);
		iscTreeFlatTopitemAttrib_int(ih, item);
	}
	return 1;
}

static int isc_TreeBranchOpen_CB(Ihandle* ih, int item) {
	IFni cbFlatBranchOpen = (IFni)IupGetCallback(ih, "FLAT_BRANCHOPEN_CB");
	int res = IUP_DEFAULT;
	if (cbFlatBranchOpen)
		res = cbFlatBranchOpen(ih, item);
	if (res != IUP_IGNORE) {
		isc_SetNoScrollSize(ih, item, 1);
		iscTreeFlatTopitemAttrib_int(ih, item);
	}
	return res;
}

static int isc_TreeBranchClose_CB(Ihandle* ih, int item) {
	IFni cbFlatBranchClose = (IFni)IupGetCallback(ih, "FLAT_BRANCHCLOSE_CB");
	int res = IUP_DEFAULT;
	if (cbFlatBranchClose)
		res = cbFlatBranchClose(ih, item);
	if (res != IUP_IGNORE) {
		isc_SetNoScrollSize(ih, item, 0);
		iscTreeFlatTopitemAttrib_int(ih, item);
	}
	return res;
}

static int isc_Selection_CB(Ihandle* ih, int id, int status) {
	IFnii cbSelection = (IFnii)IupGetCallback(ih, "FLAT_SELECTION_CB");
	int res = IUP_DEFAULT;
	if (cbSelection)
		res = cbSelection(ih, id, status);

	iscTreeFlatTopitemAttrib_int(ih, IupGetInt(ih, "VALUE"));

	return res;
}

static int iscTreeSetResetScrollAttrib(Ihandle* ih, const char* value) {
	isc_SetNoScrollSize(ih, -1, TRUE);
	return 1;
}

static int isc_TreeCreateMethod(Ihandle* ih, void** params) {
	IupSetCallback(ih, "BRANCHOPEN_CB", (Icallback)isc_TreeBranchOpen_CB);
	IupSetCallback(ih, "BRANCHCLOSE_CB", (Icallback)isc_TreeBranchClose_CB);
	IupSetCallback(ih, "SELECTION_CB", (Icallback)isc_Selection_CB);
	return IUP_NOERROR;
}



static int isc_TreeMapMethod(Ihandle* ih) {
	SetWindowLongPtr(ih->handle, GWL_STYLE, GetWindowLongPtr(ih->handle, GWL_STYLE) - WS_BORDER);
	IupSetCallback(ih, "_IUPWIN_TREEOLDWNDPROC_CB", (Icallback)GetWindowLongPtr(ih->handle, GWLP_WNDPROC));
	SetWindowLongPtr(ih->handle, GWLP_WNDPROC, (LONG_PTR)winTreeWndProc);
	IupSetAttribute(ih, "FGCOLOR", IupGetAttribute(ih, "TXTFGCOLOR"));
	IupSetAttribute(ih, "BGCOLOR", IupGetAttribute(ih, "TXTBGCOLOR"));
	return IUP_NOERROR;
}

void Iupsc_TreeOpen(void)
{
	static int run = 0;
	if (run) return;
	run = 1;
	iupRegisterClass(iupsc_TreeNewClass());
}

Iclass* iupsc_TreeNewClass(void)
{
    Iclass* ic = iupClassNew(iupRegisterFindClass("tree"));
    
    ic->name   = "sc_tree";
	ic->format = NULL; /* no parameters */
	ic->nativetype = IUP_TYPECONTROL;
	ic->childtype = IUP_CHILDNONE;
	ic->is_interactive = 1;
	ic->has_attrib_id = 1;   /* has attributes with IDs that must be parsed */

	iupClassRegisterCallback(ic, "FLAT_BRANCHCLOSE_CB", "i");
	iupClassRegisterCallback(ic, "FLAT_BRANCHOPEN_CB", "i");
	iupClassRegisterCallback(ic, "FLAT_SELECTION_CB", "ii");

	iupClassRegisterAttribute(ic, "RESETSCROLL", NULL, iscTreeSetResetScrollAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "FLAT_TOPITEM", NULL, iscTreeFlatTopitemAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

	ic->Map = isc_TreeMapMethod;
	ic->Create = isc_TreeCreateMethod;
  return ic;
}

Ihandle* Iupsc_Tree(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("sc_tree", children);
}

static int sc_Tree(lua_State *L)
{
	Ihandle *ih = Iupsc_Tree(iuplua_checkihandleornil(L, 1));
	iuplua_plugstate(L, ih);
	iuplua_pushihandle_raw(L, ih);
	return 1;
}

int iupsc_Treelua_open(lua_State * L)
{
	static int run = 0;	  	  // if (!run)
	iuplua_register(L, sc_Tree, "sc_Tree");
	iuplua_register_cb(L, "FLAT_BRANCHCLOSE_CB", (lua_CFunction)sctree_branchclose_cb, NULL);
	iuplua_register_cb(L, "FLAT_BRANCHOPEN_CB", (lua_CFunction)sctree_branchopen_cb, NULL);
	iuplua_register_cb(L, "FLAT_SELECTION_CB", (lua_CFunction)sctree_selection_cb, NULL);
	iuplua_dostring(L,
		"local ctrl = { nick = 'sc_tree', parent = iup.BOX, subdir = 'elem', creation = 'I',  funcname = 'sc_Tree', }; function ctrl.createElement(class, param) return iup.sc_Tree() end iup.RegisterWidget(ctrl); iup.SetClass(ctrl, 'iupWidget')",
		"sc_tree.lua");
	run = 1;
	return 0;
}