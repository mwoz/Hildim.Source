#ifndef __IUPFlattabsCtrl_H 
#define __IUPFlattabsCtrl_H
#include "lua.h"
#include "iup.h"
#include "..\src\iup_class.h"

void IupFlattabsCtrlOpen(void);
Iclass* iupFlattabsCtrlNewClass(void);

Ihandle* IupFlattabsCtrl(void);

int iupFlattabsCtrllua_open(lua_State * L);
typedef struct _ITabData
{
	char tabname[256];
	void* userdata;
} ITabData;
struct _IcontrolData
{
	ITabData *tab_cache;
	int tab_count;
};

#endif