// mblua.h : main header file for the mblua DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CmbluaApp
// See mblua.cpp for the implementation of this class
//

class CmbluaApp : public CWinApp
{
public:
	CmbluaApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};
