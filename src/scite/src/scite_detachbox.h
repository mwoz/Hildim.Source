#ifndef __IUPsc_DetachBox_H 
#define __IUPsc_DetachBox_H
#include "lua.h"
#include "iup.h"
#include "..\src\iup_class.h"

void Iupsc_DetachBoxOpen(void);
Iclass* iupsc_DetachBoxNewClass(void);

Ihandle* Iupsc_DetachBox(Ihandle* child);

int iupsc_detachboxlua_open(lua_State * L);

#endif