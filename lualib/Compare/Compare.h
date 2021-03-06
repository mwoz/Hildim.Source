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
extern "C" {

#include "diff.h"

}

const int nbChar = 64;


typedef LPCTSTR(__cdecl * PFUNCGETNAME)();

struct NppData {
	HWND _scintillaMainHandle;
	HWND _scintillaSecondHandle;
};

typedef void(__cdecl * PFUNCSETINFO)(NppData);
typedef void(__cdecl * PFUNCPLUGINCMD)();
typedef void(__cdecl * PBENOTIFIED)(SCNotification *);
typedef LRESULT(__cdecl * PMESSAGEPROC)(UINT Message, WPARAM wParam, LPARAM lParam);

struct ShortcutKey {
	bool _isCtrl;
	bool _isAlt;
	bool _isShift;
	UCHAR _key;
};

struct FuncItem {
	TCHAR _itemName[nbChar];
	PFUNCPLUGINCMD _pFunc;
	int _cmdID;
	bool _init2Check;
	ShortcutKey *_pShKey;
};

typedef FuncItem * (__cdecl * PFUNCGETFUNCSARRAY)(int *);

#define DEFAULT_ADDED_COLOR     0xE0FFE0
#define DEFAULT_DELETED_COLOR   0xE0E0FF
#define DEFAULT_CHANGED_COLOR   0x98E7E7
#define DEFAULT_MOVED_COLOR     0xB1A88D
#define DEFAULT_BLANK_COLOR     0xe4e4e4
#define DEFAULT_HIGHLIGHT_COLOR 0x010101
#define DEFAULT_HIGHLIGHT_ALPHA 100

enum MARKER_ID
{
	MARKER_MOVED_LINE = 11,
	MARKER_BLANK_LINE,
	MARKER_CHANGED_LINE,
	MARKER_ADDED_LINE,
	MARKER_REMOVED_LINE,
	MARKER_CHANGED_SYMBOL,
	MARKER_ADDED_SYMBOL,
	MARKER_REMOVED_SYMBOL,
	MARKER_MOVED_SYMBOL
};

enum MENU_COMMANDS
{
	CMD_COMPARE = 0,
	CMD_CLEAR_RESULTS,
	CMD_SEPARATOR_1,
	CMD_COMPARE_LAST_SAVE,
	CMD_COMAPRE_SVN_BASE,
	CMD_SEPARATOR_2,
	CMD_ALIGN_MATCHES,
	CMD_IGNORE_SPACING,
	CMD_DETECT_MOVES,
	CMD_USE_NAV_BAR,
	CMD_SEPARATOR_3,
	CMD_PREV,
	CMD_NEXT,
	CMD_FIRST,
	CMD_LAST,
	CMD_SEPARATOR_4,
	CMD_OPTION,
	CMD_ABOUT,
	NB_MENU_COMMANDS
};

struct sColorSettings
{
	int added;
	int deleted;
	int changed;
	int moved;
	int blank;
	int highlight;
	int alpha;
};

struct sUserSettings
{
	bool           UseNavBar;
	bool           AddLine;
	bool           IncludeSpace;
	bool           DetectMove;
	bool           UseSymbols;
	sColorSettings ColorSettings;
};

enum eEOL
{
	EOF_WIN,
	EOF_LINUX,
	EOF_MAC
};

const CHAR strEOL[3][3] =
{
	"\r\n",
	"\r",
	"\n"
};

const UINT lenEOL[3] = { 2,1,1 };
