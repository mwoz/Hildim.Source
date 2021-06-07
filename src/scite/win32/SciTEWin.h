// SciTE - Scintilla based Text Editor
/** @file SciTEWin.h
 ** Header of main code for the Windows version of the editor.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>

#ifdef __MINGW_H
#define _WIN32_IE	0x0400
#endif

#undef _WIN32_WINNT
#define _WIN32_WINNT  0x0501
#undef WINVER
#define WINVER 0x0501
#ifdef _MSC_VER
// windows.h, et al, use a lot of nameless struct/unions - can't fix it, so allow it
#pragma warning(disable: 4201)
#endif
#include <windows.h>
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
// Old compilers do not have Uxtheme.h
typedef HANDLE HTHEME;
#else
#include <uxtheme.h>
#endif
#ifdef _MSC_VER
// okay, that's done, don't allow it in our code
#pragma warning(default: 4201)
#endif
#include <commctrl.h>
#include <richedit.h>
#include <shlwapi.h>

#include <io.h>
#include <process.h>
#include <mmsystem.h>
#include <commctrl.h>
#ifdef _MSC_VER
#include <direct.h>
#endif
#ifdef __DMC__
#include <dir.h>
#endif

#include "ILexer.h"

#include "Scintilla.h"
#include "ScintillaTypes.h"
#include "ScintillaMessages.h"
#include "ScintillaStructures.h"
#include "Lexilla.h"
#include "../Access/LexillaAccess.h"

#include "GUI.h"

#include "SString.h"
#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "SciTE.h"
#include "Mutex.h"
#include "JobQueue.h"
#include "SciTEBase.h"
#include "SciTEKeys.h"
#include "UniqueInstance.h"
#include "Containers.h" //!-add-[user.toolbar]

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "iup.h"
#include "iuplua.h"
#include "iupcontrols.h"
#include "iupluacontrols.h"
}

class Dialog;

class SciTEWin;


inline HWND HwndOf(GUI::Window w) {
	return reinterpret_cast<HWND>(w.GetID());
}

class BaseWin : public GUI::Window {
public:
	HWND Hwnd() const {
		return reinterpret_cast<HWND>(GetID());
	}
	virtual LRESULT WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) = 0;
	static LRESULT PASCAL StWndProc(
	    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
};




/** Windows specific stuff.
 **/
class SciTEWin : public SciTEBase{

protected:
//!-start-[user.toolbar]
	HBITMAP hToolbarBitmap;
	UINT oldToolbarBitmapID;
	TArray<int,int> toolbarUsersPressableButtons;
//!-end-[user.toolbar]
	int cmdShow;
	static HINSTANCE hInstance;
	static const TCHAR *className;
	static const TCHAR *classNameInternal;
	static SciTEWin *app;
	WINDOWPLACEMENT winPlace;
	RECT rcWorkArea;
	GUI::gui_char openWhat[200];
	bool modalParameters;
	int filterDefault;
	bool staticBuild;
	int menuSource;
	std::deque<GUI::gui_string> dropFilesQueue;
	virtual GUI::Rectangle GetClientRectangle();

	// Fields also used in tool execution thread
	HANDLE hWriteSubProcess;
	DWORD subProcessGroupId;
	int outputScroll;

	HACCEL hAccTable = NULL;

	GUI::Rectangle pagesetupMargin;
	HGLOBAL hDevMode;
	HGLOBAL hDevNames;

	UniqueInstance uniqueInstance;

	/// HTMLHelp module
	HMODULE hHH;
	/// Multimedia (sound) module
	HMODULE hMM;

	// Tab Bar
	TCITEM tie;
	HFONT fontTabs;

	/// Preserve focus during deactivation
	HWND wFocus;
	HWND wActive;

	//GUI::Rectangle rFindReplace;//позиция окна поиска\замены

	virtual void GetWindowPosition(int *left, int *top, int *width, int *height, int *maximize);

	virtual void ReadProperties();

	virtual void CheckMenus();

	GUI::gui_string DialogFilterFromProperty(const GUI::gui_char *filterProperty);
	virtual bool OpenDialog(FilePath directory, const GUI::gui_char *filter);
	FilePath ChooseSaveName(FilePath directory, const char *title, const GUI::gui_char *filter=0, const char *ext=0, int *nFilter = NULL);
	virtual bool SaveAsDialog();
	virtual void SaveACopy();
	virtual void SaveAsHTML();
	virtual void SaveAsRTF();
	virtual void SaveAsPDF();
	virtual void SaveAsTEX();
	virtual void SaveAsXML();
	virtual bool PreOpenCheck(const GUI::gui_char *file);
	virtual bool IsStdinBlocked();

	/// Print the current buffer.
	virtual void Print(bool showDialog);
	/// Handle default print setup values and ask the user its preferences.
	virtual void PrintSetup();

	virtual int WindowMessageBox(GUI::Window &w, const GUI::gui_string &msg, int style);
	virtual int WindowMessageBox(const char* msg, int flag, const GUI::gui_char *p1, const GUI::gui_char *p2, const GUI::gui_char *p3);

	void DropFiles(HDROP hdrop);
	void MinimizeToTray();
	void RestoreFromTray();
	virtual void QuitProgram();

	virtual FilePath GetDefaultDirectory();
	virtual FilePath GetSciteDefaultHome();
	virtual FilePath GetSciteUserHome();

	virtual void SetFileProperties(PropSetFile &ps);

	/// Warn the user, by means defined in its properties.
//!	virtual void WarnUser(int warnID);
	virtual void WarnUser(int warnID, const char *msg = NULL, bool isCanBeAlerted = true); //!-change-[WarningMessage]

	virtual void Notify(SCNotification *notification);

	virtual void ActivateWindow(const char *timestamp);
	void ExecuteHelp(const char *cmd, int hh_cmd = 0x000d);
	void ExecuteOtherHelp(const char *cmd);
	void CopyAsRTF();
	void FullScreenToggle();
	virtual void MakeOutputVisible(GUI::ScintillaWindow &wBottom);
	void Command(WPARAM wParam, LPARAM lParam);
	HWND MainHWND();

	virtual void UIClosed();
	virtual int PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter);


	void EnsureVisible();
	virtual void HideForeReolad(int close);
	virtual void RunAsync(int idx);
	virtual void SetRestart(const char* cmdLine);
	SString restartCmdLine = "-";
	LRESULT		OnChangeCBChain(WPARAM wParam, LPARAM lParam);
	LRESULT OnDrawClipBoardMsg(WPARAM wParam);
	HWND hNextCBWnd;
	WORD cfColumnSelect;
	virtual bool IsRunAsAdmin();
	virtual bool NewInstance(const char* arg, bool asAdmin);
public:

	SciTEWin(Extension *ext, GUI::gui_string &propsExt);
	~SciTEWin();

	bool DialogHandled(GUI::WindowID id, MSG *pmsg);
	bool ModelessHandler(MSG *pmsg);

	void CreateUI();
	/// Management of the command line parameters.
	void Run(const GUI::gui_string &cmdLine, bool allowDouble);
    int EventLoop();
	void OutputAppendEncodedStringSynchronised(GUI::gui_string s, int codePage);
	DWORD ExecuteOne(const Job &jobToRun, bool &seenOutput);
	void ProcessExecute();
	void ShellExec(const SString &cmd, const char *dir);
	virtual void Execute();
	virtual void StopExecute();
	virtual void AddCommand(const SString &cmd, const SString &dir, JobSubsystem jobType, const SString &input = "", int flags=0);

	void Creation();
	LRESULT KeyDown(WPARAM wParam);
	LRESULT KeyUp(WPARAM wParam);
//!	virtual void AddToPopUp(const char *label, int cmd=0, bool enabled=true);	//!-change-[ExtendedContextMenu]
	LRESULT ContextMenuMessage(UINT iMessage, WPARAM wParam, LPARAM lParam, bool fromMargin);
	LRESULT WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam);
	int OnTab(Ihandle * ih, int new_pos, int old_pos);
	int OnShift(Ihandle * ih, int old_tab, int new_tab);

	virtual SString EncodeString(const SString &s);
	virtual SString GetRangeInUIEncoding(GUI::ScintillaWindow &wCurrent, int selStart, int selEnd);

	HACCEL GetAcceleratorTable() {
		return hAccTable;
	}

	void SetAcceleratorTable(void *h){
		if (hAccTable){
			DestroyAcceleratorTable(hAccTable);
		}
		hAccTable = (HACCEL)h;
	}

	uptr_t GetInstance();
	static void Register(HINSTANCE hInstance_);
	static LRESULT PASCAL TWndProc(
	    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	friend class UniqueInstance;
	friend class SciTEBase; //!-add-[GetApplicationProps]
	bool SwitchMouseHook(bool bSet);
	bool SwitchMacroHook(bool bSet);
	void NotifyMouseHook(int nCode, WPARAM wParam, LPARAM lParam);
	LRESULT NotifyGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
	virtual Ihandle * IupTab(int id);
	virtual void PostLoadScript();
};



inline bool IsKeyDown(int key) {
	return (::GetKeyState(key) & 0x80000000) != 0;
}


inline GUI::Point PointFromLong(long lPoint) {
	return GUI::Point(static_cast<short>(LOWORD(lPoint)), static_cast<short>(HIWORD(lPoint)));
}

