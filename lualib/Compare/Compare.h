#pragma once

#define WIN32_LEAN_AND_MEAN

#include <math.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <stdio.h>
#include <assert.h>
#include "Engine.h"
#include <iostream>
#include <fstream>
#include "Resource.h"
#include <Commdlg.h>
#include "Scintilla.h"
#include "ScintillaTypes.h"
#include "ScintillaStructures.h"
#include "ILoader.h"
#include "ScintillaCall.h"
#include "diff.h"


const int nbChar = 64;


typedef LPCTSTR(__cdecl * PFUNCGETNAME)();

class NppData {
private:
	HWND _scintillaMainHandle = NULL;
	HWND _scintillaSecondHandle = NULL;
	pSciCaller _pMain = NULL;
	pSciCaller _pSecond = NULL;
public:
	void init(HWND m, HWND s, pSciCaller pM, pSciCaller pS) {
		_scintillaMainHandle = m;
		_scintillaSecondHandle = s;
		_pMain = pM;
		_pSecond = pS;
		scintillaMainHandle = m;
		scintillaSecondHandle = s;
		pMain = pM;
		pSecond = pS;

	};
	void clear() {
		_scintillaMainHandle = NULL;
		_scintillaSecondHandle = NULL;
		_pMain = NULL;
		_pSecond = NULL;
		scintillaMainHandle = NULL;
		scintillaSecondHandle = NULL;
		pMain = NULL;
		pSecond = NULL;
	};

	HWND scintillaMainHandle = NULL;
	HWND scintillaSecondHandle = NULL;
	pSciCaller pMain = NULL;
	pSciCaller pSecond = NULL;
	inline pSciCaller viewCaller(int view) {
		return view ? pSecond : pMain;
	}
};



extern const TCHAR PLUGIN_NAME[];
extern NppData		nppData;


typedef void(__cdecl * PFUNCSETINFO)(NppData);
typedef void(__cdecl * PFUNCPLUGINCMD)();
typedef void(__cdecl * PBENOTIFIED)(SCNotification *);
typedef LRESULT(__cdecl * PMESSAGEPROC)(UINT Message, WPARAM wParam, LPARAM lParam);

#define DEFAULT_ADDED_COLOR     0xE0FFE0
#define DEFAULT_DELETED_COLOR   0xE0E0FF
#define DEFAULT_CHANGED_COLOR   0x98E7E7
#define DEFAULT_MOVED_COLOR     0xB1A88D
#define DEFAULT_BLANK_COLOR     0xe4e4e4
#define DEFAULT_HIGHLIGHT_COLOR 0x010101
#define DEFAULT_HIGHLIGHT_ALPHA 100


#define LOGD_GET_TIME
#define LOGD(LOG_FILTER, STR)
#define LOGDIF(LOG_FILTER, COND, STR)
#define LOGDB(LOG_FILTER, BUFFID, STR)
#define PRINT_DIFFS(INFO, DIFFS)

