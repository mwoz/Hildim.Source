#ifndef __IUPFlattabsCtrl_H 
#define __IUPFlattabsCtrl_H
#include "lua.h"
#include "iup.h"
#include "..\src\iup_class.h"
#include "../../iup/include/iuplua.h"

//class BufferListAPI1 {
//	public:
//		virtual int GetDocumentByName(FilePath filename, bool excludeCurrent = false, int forIdm = NULL) = 0;
//		virtual void* GetAt(int index) = 0;
//	};

void IupFlattabsCtrlOpen(void);
Iclass* iupFlattabsCtrlNewClass(void);

Ihandle* IupFlattabsCtrl(void);

int iupFlattabsCtrllua_open(lua_State * L);

struct _IcontrolData
{
	int xStart;
	int yStart;
	int dragTab;
	int start;
	int xFree;
	int xFreeMax;
};

#endif