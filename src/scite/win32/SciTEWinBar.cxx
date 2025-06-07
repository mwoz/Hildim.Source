// SciTE - Scintilla based Text Editor
/** @file SciTEWinBar.cxx
 ** Bar and menu code for the Windows version of the editor.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include "SciTEWin.h"



/**
 * Set up properties for FileTime, FileDate, CurrentTime, CurrentDate and FileAttr.
 */
void SciTEWin::SetFileProperties(
    PropSetFile &ps) {			///< Property set to update.

	const int TEMP_LEN = 100;
	char temp[TEMP_LEN];
	HANDLE hf = ::CreateFileW(filePath.AsInternal(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		FILETIME ft;
		::GetFileTime(hf, NULL, NULL, &ft);
		::CloseHandle(hf);
		FILETIME lft;
		::FileTimeToLocalFileTime(&ft, &lft);
		SYSTEMTIME st;
		::FileTimeToSystemTime(&lft, &st);
		::GetTimeFormatA(LOCALE_USER_DEFAULT,
		                0, &st,
		                NULL, temp, TEMP_LEN);
		ps.Set("FileTime", temp);

		::GetDateFormatA(LOCALE_USER_DEFAULT,
		                DATE_SHORTDATE, &st,
		                NULL, temp, TEMP_LEN);
		ps.Set("FileDate", temp);

		DWORD attr = ::GetFileAttributesW(filePath.AsInternal());
		SString fa;
		if (attr & FILE_ATTRIBUTE_READONLY) {
			fa += "R";
		}
		if (attr & FILE_ATTRIBUTE_HIDDEN) {
			fa += "H";
		}
		if (attr & FILE_ATTRIBUTE_SYSTEM) {
			fa += "S";
		}
		ps.Set("FileAttr", fa.c_str());
	} else {
		/* Reset values for new buffers with no file */
		ps.Set("FileTime", "");
		ps.Set("FileDate", "");
		ps.Set("FileAttr", "");
	}

	::GetDateFormatA(LOCALE_USER_DEFAULT,
	                DATE_SHORTDATE, NULL,     	// Current date
	                NULL, temp, TEMP_LEN);
	ps.Set("CurrentDate", temp);			  

	::GetTimeFormatA(LOCALE_USER_DEFAULT,
	                0, NULL,     	// Current time
	                NULL, temp, TEMP_LEN);
	ps.Set("CurrentTime", temp);
}

/**
 * Manage Windows specific notifications.
 */
WCHAR stat_tips[301];	  
void SciTEWin::Notify(SCNotification *notification) {
	switch (notification->nmhdr.code) {

	case SCN_CHARADDED:
		if ((notification->nmhdr.idFrom == IDM_RUNWIN) &&
		        jobQueue.IsExecuting() &&
		        hWriteSubProcess) {
			char chToWrite = static_cast<char>(notification->ch);
			if (chToWrite != '\r') {
				DWORD bytesWrote = 0;
				::WriteFile(hWriteSubProcess, &chToWrite,
				            1, &bytesWrote, NULL);
			}
		} else {
			SciTEBase::Notify(notification);
		}
		break;
	case TTN_GETDISPINFO:
	{
	    NMTTDISPINFO  *di = reinterpret_cast<NMTTDISPINFO*>(notification);	
		GUI::gui_string  ccc = ((*(FilePath*)(&(*(RecentFile*)(&buffers.buffers[di->hdr.idFrom]))))).GetFileName();
		lstrcpyn(stat_tips, ccc.c_str(),300);
		di->lpszText = stat_tips;// (LPWSTR)ccc.c_str();
		di->hinst = NULL;
		//di->szText  str
		int iii = 1;				

	}
		break;
	default:     	// Scintilla notification, use default treatment
		SciTEBase::Notify(notification);
		break;
	}
}

void SciTEWin::ActivateWindow(const char *) {
	// This does nothing as, on Windows, you can no longer activate yourself
}



void SciTEWin::CheckMenus() {
//	!!!TODO-  notify

	SciTEBase::CheckMenus();

}

// Mingw headers do not have this:
#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT 0x0800
#endif
#ifndef TB_LOADIMAGES
#define TB_LOADIMAGES (WM_USER + 50)
#endif


static WNDPROC stDefaultTabProc = NULL;


/**
 * Create all the needed windows.
 */
SciTEWin *pSciTE;
int OnTabClick(Ihandle * ih, int new_pos, int old_pos) {

	return pSciTE->OnTab(ih, new_pos, old_pos);;
}

int OnTabShift(Ihandle * ih, int old_tab, int new_tab) {

	return pSciTE->OnShift(ih, old_tab, new_tab);
}

int SciTEWin::OnTab(Ihandle * ih, int new_pos, int old_pos) {
	extender->OnNavigation("Switch");
	SetDocumentAt(IupGetIntId(ih, "TABBUFFERID", new_pos));
	extender->OnNavigation("Switch-");
	CheckReload();
	return IUP_DEFAULT;
}

int SciTEWin::OnShift(Ihandle * ih, int old_tab, int new_tab) {

	ShiftTab(IupGetIntId(ih, "TABBUFFERID", old_tab), IupGetIntId(ih, "TABBUFFERID", new_tab), true);
	return IUP_DEFAULT;
}

void SciTEWin::Creation() {
	pSciTE = this;
	lua_State *L = (lua_State*)extender->GetLuaState();
	
	layout.CreateLayout(L, this);

	IupSetCallback(IupTab(IDM_SRCWIN), "TABCHANGEPOS_CB", (Icallback)OnTabClick);
	IupSetCallback(IupTab(IDM_COSRCWIN), "TABCHANGEPOS_CB", (Icallback)OnTabClick);

	IupSetCallback(IupTab(IDM_SRCWIN), "TAB_SHIFT_CB", (Icallback)OnTabShift);
	IupSetCallback(IupTab(IDM_COSRCWIN), "TAB_SHIFT_CB", (Icallback)OnTabShift);

	wEditorL.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("Source"),
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		100, 100,
		//reinterpret_cast<HWND>(wContent.GetID()),WS_VSCROLL | WS_HSCROLL |
		layout.GetChildHWND("Source"),
		reinterpret_cast<HMENU>(IDM_SRCWIN),
		hInstance,
		0));
	if (!wEditorL.CanCall())
		exit(FALSE);
	wEditorL.Show();
	wEditorL.UsePopUp(Scintilla::PopUp::Never);
	layout.SubclassChild("Source", &wEditorL);
	WindowSetFocus(wEditorL);
	wEditor.SetID(wEditorL.GetID());

	wEditorR.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("CoSource"),
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		100, 100,
		//reinterpret_cast<HWND>(wContent.GetID()),
		layout.GetChildHWND("CoSource"),
		reinterpret_cast<HMENU>(IDM_COSRCWIN),
		hInstance,
		0));
	if (!wEditorR.CanCall())
		exit(FALSE);
	wEditorR.Show();
	wEditorR.UsePopUp(Scintilla::PopUp::Never);
	layout.SubclassChild("CoSource", &wEditorR);
	wEditor.coEditor.SetID(wEditorR.GetID());

	wOutput.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("Run"),
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		100, 100,
		layout.GetChildHWND("Run"),
		reinterpret_cast<HMENU>(IDM_RUNWIN),
		hInstance,
		0));
	if (!wOutput.CanCall())
		exit(FALSE); 
	wOutput.Show();
	// No selection margin on output window
	wOutput.SetMarginWidthN(1, 0);
	//wOutput.Call(SCI_SETCARETPERIOD, 0);
	wOutput.UsePopUp(Scintilla::PopUp::Never);
	layout.SubclassChild("Run", &wOutput);

	wFindRes.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("FindRes"),
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		100, 100,
		layout.GetChildHWND("FindRes"),
		reinterpret_cast<HMENU>(IDM_FINDRESWIN),
		hInstance,
		0));
	if (!wFindRes.CanCall())
		exit(FALSE);
	wFindRes.Show();
	// No selection margin on output window
	wFindRes.SetMarginWidthN(1, 0);
	//wFindRes.Call(SCI_SETCARETPERIOD, 0);
	wFindRes.UsePopUp(Scintilla::PopUp::Never);
	layout.SubclassChild("FindRes", &wFindRes);
	
	::DragAcceptFiles(MainHWND(), true);

	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icce);

}

Ihandle * SciTEWin::IupTab(int id) {
	if (id == IDM_SRCWIN)
		return layout.pLeftTab;
	return layout.pRightTab;
}

