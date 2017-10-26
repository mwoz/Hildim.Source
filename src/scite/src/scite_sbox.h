#ifndef __IUPsc_SBox_H 
#define __IUPsc_SBox_H
#include "lua.h"
#include "iup.h"
#include "..\src\iup_class.h"

void Iupsc_SBoxOpen(void);
Iclass* iupsc_SBoxNewClass(void);

Ihandle* Iupsc_SBox(Ihandle* child);

int iupsc_SBoxlua_open(lua_State * L);

#endif