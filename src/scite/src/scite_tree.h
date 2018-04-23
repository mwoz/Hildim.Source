#ifndef __IUPsc_Tree_H 
#define __IUPsc_Tree_H
#include "lua.h"
#include "iup.h"
#include "..\src\iup_class.h"

void Iupsc_TreeOpen(void);
Iclass* iupsc_TreeNewClass(void);

Ihandle* Iupsc_Tree(Ihandle* child);

int iupsc_Treelua_open(lua_State * L);

#endif