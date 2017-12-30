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


void SciTEWin::LocaliseDialog(HWND wDialog) {

}

// Mingw headers do not have this:
#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT 0x0800
#endif
#ifndef TB_LOADIMAGES
#define TB_LOADIMAGES (WM_USER + 50)
#endif


static WNDPROC stDefaultTabProc = NULL;
static LRESULT PASCAL TabWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {

	static BOOL st_bDragBegin = FALSE;
	static int st_iDraggingTab = -1;
	static int st_iLastClickTab = -1;
	static HWND st_hwndLastFocus = NULL;

	switch (iMessage) {

	case WM_LBUTTONDOWN: {
			GUI::Point pt = PointFromLong(lParam);
			TCHITTESTINFO thti;
			thti.pt.x = pt.x;
			thti.pt.y = pt.y;
			thti.flags = 0;
			st_iLastClickTab = ::SendMessage(hWnd, TCM_HITTEST, (WPARAM)0, (LPARAM) & thti);
		}
		break;
	}

	LRESULT retResult;
	if (stDefaultTabProc != NULL) {
		retResult = CallWindowProc(stDefaultTabProc, hWnd, iMessage, wParam, lParam);
	} else {
		retResult = ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}

	switch (iMessage) {
//!-start-[close_on_dbl_clk]
	case WM_LBUTTONDBLCLK: {
			GUI::Point pt = PointFromLong(lParam);
			TCHITTESTINFO thti;
			thti.pt.x = pt.x;
			thti.pt.y = pt.y;
			thti.flags = 0;
			int tab = ::SendMessage(hWnd, TCM_HITTEST, (WPARAM)0, (LPARAM) & thti);
			if (tab >= 0) {
				::SendMessage(::GetParent(hWnd), WM_COMMAND, IDC_TABDBLCLK, (LPARAM)tab);
			}
		}
		break;
//!-end-[close_on_dbl_clk]

	case WM_MBUTTONDOWN: {
			// Check if on tab bar
			GUI::Point pt = PointFromLong(lParam);
			TCHITTESTINFO thti;
			thti.pt.x = pt.x;
			thti.pt.y = pt.y;
			thti.flags = 0;
			int tab = ::SendMessage(hWnd, TCM_HITTEST, (WPARAM)0, (LPARAM) & thti);
			if (tab >= 0) {
				::SendMessage(::GetParent(hWnd), WM_COMMAND, IDC_TABCLOSE, (LPARAM)tab);
			}
		}
		break;

	case WM_LBUTTONUP: {
			st_iLastClickTab = -1;
			if (st_bDragBegin == TRUE) {
				if (st_hwndLastFocus != NULL) ::SetFocus(st_hwndLastFocus);
				::ReleaseCapture();
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				st_bDragBegin = FALSE;
				GUI::Point pt = PointFromLong(lParam);
				TCHITTESTINFO thti;
				thti.pt.x = pt.x;
				thti.pt.y = pt.y;
				thti.flags = 0;
				int tab = ::SendMessage(hWnd, TCM_HITTEST, (WPARAM)0, (LPARAM) & thti);
				if (tab > -1 && st_iDraggingTab > -1 && st_iDraggingTab != tab) {
					::SendMessage(::GetParent(hWnd),
					        WM_COMMAND,
					        IDC_SHIFTTAB,
					        MAKELPARAM(st_iDraggingTab, tab));
				}
				st_iDraggingTab = -1;
			}
		}
		break;

	case WM_KEYDOWN: {
			if (wParam == VK_ESCAPE) {
				if (st_bDragBegin == TRUE) {
					if (st_hwndLastFocus != NULL) ::SetFocus(st_hwndLastFocus);
					::ReleaseCapture();
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
					st_bDragBegin = FALSE;
					st_iDraggingTab = -1;
					st_iLastClickTab = -1;
					::InvalidateRect(hWnd, NULL, FALSE);
				}
			}
		}
		break;

	case WM_MOUSEMOVE: {

			GUI::Point pt = PointFromLong(lParam);
			TCHITTESTINFO thti;
			thti.pt.x = pt.x;
			thti.pt.y = pt.y;
			thti.flags = 0;
			int tab = ::SendMessage(hWnd, TCM_HITTEST, (WPARAM)0, (LPARAM) & thti);
			int tabcount = ::SendMessage(hWnd, TCM_GETITEMCOUNT, (WPARAM)0, (LPARAM)0);

			if (wParam == MK_LBUTTON &&
			        tabcount > 1 &&
			        tab > -1 &&
			        st_iLastClickTab == tab &&
			        st_bDragBegin == FALSE) {
				st_iDraggingTab = tab;
				::SetCapture(hWnd);
				st_hwndLastFocus = ::SetFocus(hWnd);
				st_bDragBegin = TRUE;
				HCURSOR hcursor = ::LoadCursor(::GetModuleHandle(NULL),
				        MAKEINTRESOURCE(IDC_DRAGDROP));
				if (hcursor) ::SetCursor(hcursor);
			} else {
				if (st_bDragBegin == TRUE) {
					if (tab > -1 && st_iDraggingTab > -1 /*&& st_iDraggingTab != tab*/) {
						HCURSOR hcursor = ::LoadCursor(::GetModuleHandle(NULL),
						        MAKEINTRESOURCE(IDC_DRAGDROP));
						if (hcursor) ::SetCursor(hcursor);
					} else {
						::SetCursor(::LoadCursor(NULL, IDC_NO));
					}
				}
			}
		}
		break;

	case WM_PAINT: {
			if (st_bDragBegin == TRUE && st_iDraggingTab != -1) {

				GUI::Point ptCursor;
				::GetCursorPos(reinterpret_cast<POINT*>(&ptCursor));
				GUI::Point ptClient = ptCursor;
				::ScreenToClient(hWnd, reinterpret_cast<POINT*>(&ptClient));
				TCHITTESTINFO thti;
				thti.pt.x = ptClient.x;
				thti.pt.y = ptClient.y;
				thti.flags = 0;
				int tab = ::SendMessage(hWnd, TCM_HITTEST, (WPARAM)0, (LPARAM) & thti);

				RECT tabrc;
				if (tab != -1 &&
				        tab != st_iDraggingTab &&
				        TabCtrl_GetItemRect(hWnd, tab, &tabrc)) {

					HDC hDC = ::GetDC(hWnd);
					if (hDC) {

						int xLeft = tabrc.left + 8;
						int yLeft = tabrc.top + (tabrc.bottom - tabrc.top) / 2;
						POINT ptsLeftArrow[] = {
							{xLeft, yLeft - 2},
							{xLeft - 2, yLeft - 2},
							{xLeft - 2, yLeft - 5},
							{xLeft - 7, yLeft},
							{xLeft - 2, yLeft + 5},
							{xLeft - 2, yLeft + 2},
							{xLeft, yLeft + 2}
						};

						int xRight = tabrc.right - 10;
						int yRight = tabrc.top + (tabrc.bottom - tabrc.top) / 2;
						POINT ptsRightArrow[] = {
							{xRight, yRight - 2},
							{xRight + 2, yRight - 2},
							{xRight + 2, yRight - 5},
							{xRight + 7, yRight},
							{xRight + 2, yRight + 5},
							{xRight + 2, yRight + 2},
							{xRight, yRight + 2}
						};

						HPEN pen = ::CreatePen(0,1,RGB(255, 0, 0));
						HPEN penOld = static_cast<HPEN>(::SelectObject(hDC, pen));
						COLORREF colourNearest = ::GetNearestColor(hDC, RGB(255, 0, 0));
						HBRUSH brush = ::CreateSolidBrush(colourNearest);
						HBRUSH brushOld = static_cast<HBRUSH>(::SelectObject(hDC, brush));
						::Polygon(hDC, tab < st_iDraggingTab ? ptsLeftArrow : ptsRightArrow, 7);
						::SelectObject(hDC, brushOld);
						::DeleteObject(brush);
						::SelectObject(hDC, penOld);
						::DeleteObject(pen);
					}
					::ReleaseDC(hWnd, hDC);
				}
			}
		}
		break;
	}

	return retResult;
}

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
	SetDocumentAt(IupGetIntId(ih, "TABBUFFERID", new_pos));
	CheckReload();
	return IUP_DEFAULT;
}

int SciTEWin::OnShift(Ihandle * ih, int old_tab, int new_tab) {

	ShiftTab(IupGetIntId(ih, "TABBUFFERID", old_tab), IupGetIntId(ih, "TABBUFFERID", new_tab));
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
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		100, 100,
		//reinterpret_cast<HWND>(wContent.GetID()),
		layout.GetChildHWND("Source"),
		reinterpret_cast<HMENU>(IDM_SRCWIN),
		hInstance,
		0));
	if (!wEditorL.CanCall())
		exit(FALSE);
	wEditorL.Show();
	wEditorL.Call(SCI_USEPOPUP, 0);
	layout.SubclassChild("Source", &wEditorL);
	WindowSetFocus(wEditorL);
	wEditor.SetID(wEditorL.GetID());

	wEditorR.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("CoSource"),
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
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
	wEditorR.Call(SCI_USEPOPUP, 0);
	layout.SubclassChild("CoSource", &wEditorR);
	wEditor.coEditor.SetID(wEditorR.GetID());

	wOutput.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("Run"),
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
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
	wOutput.Call(SCI_SETMARGINWIDTHN, 1, 0);
	//wOutput.Call(SCI_SETCARETPERIOD, 0);
	wOutput.Call(SCI_USEPOPUP, 0);
	layout.SubclassChild("Run", &wOutput);

	wFindRes.SetID(::CreateWindowEx(
		0,
		TEXT("Scintilla"),
		TEXT("FindRes"),
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
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
	wFindRes.Call(SCI_SETMARGINWIDTHN, 1, 0);
	//wFindRes.Call(SCI_SETCARETPERIOD, 0);
	wFindRes.Call(SCI_USEPOPUP, 0);
	layout.SubclassChild("FindRes", &wFindRes);
	
	::DragAcceptFiles(MainHWND(), true);

	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icce);

	WNDCLASS wndClass = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	GetClassInfo(NULL, WC_TABCONTROL, &wndClass);
	stDefaultTabProc = wndClass.lpfnWndProc;
	wndClass.lpfnWndProc = TabWndProc;
	wndClass.style = wndClass.style | CS_DBLCLKS;
	wndClass.lpszClassName = TEXT("SciTeTabCtrl");
	wndClass.hInstance = hInstance;
	if (RegisterClass(&wndClass) == 0)
		exit(FALSE);

}

Ihandle * SciTEWin::IupTab(int id) {
	if (id == IDM_SRCWIN)
		return layout.pLeftTab;
	return layout.pRightTab;
}

