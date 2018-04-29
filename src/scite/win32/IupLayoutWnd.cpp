#include "SciTEWin.h"
#include "../../iup/src/iup_drvdraw.h"

std::map<std::string, IupChildWnd*> classList;

static int iScroll_CB(Ihandle *ih, int op, float posx, float posy) {
	classList[IupGetAttribute(ih, "NAME")]->Scroll_CB(op, posx, posy);
	return IUP_DEFAULT;
}

static int iVScrollFraw_CB(Ihandle*ih, IdrawCanvas* dc, int sb_size, int ymax, int pos, int d, int active, char* fgcolor_drag, char * bgcolor) {
	classList[IupGetAttribute(ih, "NAME")]->VScrollFraw_CB(ih, (void*)dc, sb_size, ymax, pos, d, active, fgcolor_drag, bgcolor);
	return IUP_DEFAULT;
}



IupChildWnd::IupChildWnd()
{
}


IupChildWnd::~IupChildWnd()
{
}
void IupChildWnd::VScrollFraw_CB(Ihandle*ih, void* c, int sb_size, int ymax, int pos, int d, int active, char* fgcolor_drag, char * bgcolor) {
	IdrawCanvas* dc = (IdrawCanvas*)c;
	iupFlatDrawBox(dc, 2, sb_size - 3, pos, pos + d, fgcolor_drag, bgcolor, active);
}

void IupChildWnd::Scroll_CB(int op, float posx, float posy) {
	switch (op) {
	case IUP_SBUP:
	case IUP_SBDN:
	case IUP_SBPGUP:
	case IUP_SBPGDN:
	case IUP_SBPOSV:
	case IUP_SBDRAGV:
		blockV = true;
		pScintilla->Call(SCI_SETFIRSTVISIBLELINE, IupGetInt(pContainer, "POSY"));
		blockV = false;
		break;
	case IUP_SBLEFT:
	case IUP_SBRIGHT:
	case IUP_SBPGLEFT:
	case IUP_SBPGRIGHT:
	case IUP_SBPOSH:
	case IUP_SBDRAGH:
		blockH = true;
		pScintilla->Call(SCI_SETXOFFSET, IupGetInt(pContainer, "POSX"));
		blockH = false;
		break;
	}
}
void IupChildWnd::Attach(HWND h, SciTEWin *pS, const char *pName, HWND hM, GUI::ScintillaWindow *pW, Ihandle *pCnt)
{
	hMainWnd = hM;
	pSciteWin = pS;
	subclassedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatWndProc)));
	SetWindowLongPtr(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowLong(h, GWL_STYLE, GetWindowLong(h, GWL_STYLE) | WS_CLIPCHILDREN);
	lstrcpynA(name, pName, 15);
	pScintilla = pW;
	pContainer = pCnt;
	IupSetCallback(pCnt, "SCROLL_CB", (Icallback)iScroll_CB);
	IupSetCallback(pCnt, "VSCROLLDRAW_CB", (Icallback)iVScrollFraw_CB);
	//IupSetCallback(pCnt, "HSCROLLDRAW_CB", (Icallback)iHScrollFraw_CB);
}

void IupChildWnd::SizeEditor() {
	int x, y;
	IupGetIntInt(pContainer, "RASTERSIZE", &x, &y);
	//::SetWindowPos((HWND)pScintilla->GetID(), HWND_TOP, 0, 0, x - (IupGetInt(pContainer, "YHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE")), y - hPx, 0);
	::SetWindowPos((HWND)pScintilla->GetID(), HWND_TOP, 0, 0, x - vPx, y - hPx, 0);

}


LRESULT PASCAL IupChildWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	LRESULT ret;
	switch (uMsg){
	case WM_SETFOCUS:
	{
		HWND h = ::FindWindowEx(hwnd, NULL, NULL, NULL);
		::SetFocus(h);
	}
		return 0;
	case SCI_GETSCROLLINFO:
	{
		if (!pContainer) return false;
		LPSCROLLINFO lpsi = (LPSCROLLINFO)lParam;
		if (wParam == SB_VERT) {
			if (lpsi->fMask & SIF_PAGE) {
				lpsi->nPage = IupGetInt(pContainer, "DY");
			}
			if (lpsi->fMask & (SIF_POS | SIF_TRACKPOS)) {
				lpsi->nPos = IupGetInt(pContainer, "POSY");
				lpsi->nTrackPos = lpsi->nPos;
			}

			if (lpsi->fMask & SIF_RANGE) {
				lpsi->nMin = IupGetInt(pContainer, "YMIN");
				lpsi->nMax = IupGetInt(pContainer, "YMAX");
			}
		} else if (wParam == SB_HORZ) {
			if (lpsi->fMask & SIF_PAGE) {
				lpsi->nPage = IupGetInt(pContainer, "DX");
			}
			if (lpsi->fMask & (SIF_POS | SIF_TRACKPOS)) {
				lpsi->nPos = IupGetInt(pContainer, "POSX");
				lpsi->nTrackPos = lpsi->nPos;
			}

			if (lpsi->fMask & SIF_RANGE) {
				lpsi->nMin = IupGetInt(pContainer, "XMIN");
				lpsi->nMax = IupGetInt(pContainer, "XMAX");
			}
		} else
			return false;
	}
		return true;
		break;
	case SCI_SETSCROLLINFO:
	{
		if (!pContainer) return false;
		LPSCROLLINFO lpsi = (LPSCROLLINFO)lParam; 
		if (wParam == SB_VERT) {
			if (lpsi->fMask & SIF_RANGE ) {
				IupSetInt(pContainer, "YMIN", lpsi->nMin);
				IupSetInt(pContainer, "YMAX", lpsi->nMax);
			}
			if (lpsi->fMask & SIF_PAGE) {
				IupSetInt(pContainer, "DY", lpsi->nPage);
				
			}
			if (lpsi->fMask & SIF_POS ) {
				IupSetInt(pContainer, "POSY", lpsi->nPos);
			}
			if (lpsi->fMask & SIF_TRACKPOS) {
				IupSetInt(pContainer, "POSY", lpsi->nTrackPos);
			}
			int v = IupGetInt(pContainer, "YHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE");
			if (v != vPx) {
				vPx = v;
				hPx = IupGetInt(pContainer, "XHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE");
				SizeEditor();
			}
		} else if(wParam == SB_HORZ) {
			if (lpsi->fMask & SIF_RANGE) {
				IupSetInt(pContainer, "XMIN", lpsi->nMin);
				IupSetInt(pContainer, "XMAX", lpsi->nMax);
			}
			if (lpsi->fMask & SIF_PAGE) {
				IupSetInt(pContainer, "DX", lpsi->nPage);
			}
			if (lpsi->fMask & SIF_POS) {
				IupSetInt(pContainer, "POSX", lpsi->nPos);
			}
			if (lpsi->fMask & SIF_TRACKPOS) {
				IupSetInt(pContainer, "POSX", lpsi->nTrackPos);
			}
			int h = IupGetInt(pContainer, "XHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE");
			if (h != hPx) {
				hPx = h;
				vPx = IupGetInt(pContainer, "YHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE");
				SizeEditor();
			}
		} else
			return false;
	}
		return true;
		break;
	case WM_NOTIFY:
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		if(::IsWindowVisible(hMainWnd) )return pSciteWin->WndProc(uMsg, wParam, lParam);
		break;
	case WM_SIZE: 	
		SizeEditor();
		break;
	case WM_CLOSE:
		ret = subclassedProc(hwnd, uMsg, wParam, lParam);
		delete(this);
		return ret;
		break;
	}

	return subclassedProc(hwnd, uMsg, wParam, lParam);
}

LRESULT PASCAL IupChildWnd::StatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	IupChildWnd* lpIupChildWnd = reinterpret_cast<IupChildWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (lpIupChildWnd)
		return lpIupChildWnd->WndProc(hwnd, uMsg, wParam, lParam);

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

IupLayoutWnd *pLayout;

IupLayoutWnd::IupLayoutWnd()
{
	pLayout = this;
}


IupLayoutWnd::~IupLayoutWnd()
{
}

Ihandle* IupLayoutWnd::Create_dialog(void)
{
	Ihandle* pTab;
	Ihandle* containers[13];

	char* fntSize = pSciteWin->Property("iup.defaultfontsize");

	if (strcmp(fntSize, "") && StrToIntA(fntSize) > 0)
		IupSetGlobal("DEFAULTFONTSIZE", fntSize);
	static char minSz[10];
	::ZeroMemory((void*)minSz, sizeof(char) * 10);
	minSz[0] = '0';
	minSz[1] = 'x';
	lstrcatA(minSz, fntSize);

	static char scrFORECOLOR[14], scrPRESSCOLOR[14], scrHIGHCOLOR[14];

	char* clr = pSciteWin->Property("iup.scroll.forecolor");
	if (!strcmp(clr, "")) {
		clr = "220 220 220";
		pSciteWin->SetProperty("iup.scroll.forecolor", clr);
	}
	lstrcpynA(scrFORECOLOR, clr, 12);

	clr = pSciteWin->Property("iup.scroll.presscolor");
	if (!strcmp(clr, "")) {
		clr = "190 190 190";
		pSciteWin->SetProperty("iup.scroll.presscolor", clr);
	}
	lstrcpynA(scrPRESSCOLOR, clr, 12);

	clr = pSciteWin->Property("iup.scroll.highcolor");
	if (!strcmp(clr, "")) {
		clr = "200 200 200";
		pSciteWin->SetProperty("iup.scroll.highcolor", clr);
	}
	lstrcpynA(scrHIGHCOLOR, clr, 12);

	static char * scrollsize = "15";

	pLeftTab = IupSetAtt(NULL, IupCreate("flattabs_ctrl"),
		"NAME", "TabCtrlLeft",
		"EXPAND", "YES",
		"TABSPADDING", "10x3",
		"EXTRABUTTONS", "1",
		"MAXSIZE", "65535x65535",
		"MINSIZE", minSz,
		"FORECOLOR", "",
		NULL);
	pRightTab = IupSetAtt(NULL, IupCreate("flattabs_ctrl"),
		"NAME", "TabCtrlRight",
		"EXPAND", "HORIZONTAL",
		"TABSPADDING", "10x3",
		"EXTRABUTTONS", "1",
		"MAXSIZE", "65535x65535",
		"MINSIZE", minSz,
		"FORECOLOR", "",
		NULL);

	pTab = IupSetAtt(NULL, IupCreatep("split",
		pLeftTab,
		IupSetAtt(NULL, IupCreatep("expander",
			pRightTab,
			NULL),
			"NAME", "RightTabExpander",
			"BARSIZE", "0",
			"EXPAND", "HORIZONTAL",
			"MINSIZE", minSz,
			"STATE", "CLOSE",
			NULL),
		NULL),
		"ORIENTATION", "VERTICAL",
		"NAME", "TabBarSplit",
		"SHOWGRIP", "NO",
		"COLOR", scrFORECOLOR,
		"BARSIZE", "0",
		"LAYOUTDRAG", "NO",
		"VALUE", "1000",
		"MINSIZE", minSz,
		"HISTORIZED", "NO",
		NULL);



	containers[3] = 
	IupSetAtt(NULL, IupCreatep("split", 
		IupSetAtt(NULL, IupCreatep("expander",
			IupSetAtt(NULL, IupCreatep("scrollbox",
				NULL),
				"NAME", "LeftBarPH",
				"SCROLLBAR", "NO",
				NULL),
			NULL),
			"NAME", "LeftBarExpander",
			"BARSIZE", "5",
			"BARPOSITION", "LEFT",
			"LAYOUTDRAG", "NO",
			"MINSIZE", "x1",
			"VALUE", "0",
		NULL),
		IupSetAtt(NULL, IupCreatep("split",
			IupSetAtt(NULL, IupCreatep("split",
						IupSetAtt(NULL, IupCreatep("split",
								IupSetAtt(NULL, IupCreate("scrollcanvas"),
									"NAME", "Source",
									"EXPAND", "YES",
									"HIGHCOLOR", scrHIGHCOLOR,
									"PRESSCOLOR", scrPRESSCOLOR,
									"FORECOLOR", scrFORECOLOR,
									"SCROLLBARSIZE", scrollsize,
								NULL),
								IupSetAtt(NULL, IupCreatep("expander",
									IupSetAtt(NULL, IupCreatep("sc_detachbox",
										IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("scrollcanvas"),
										"NAME", "CoSource",
										"EXPAND", "YES",
										"HIGHCOLOR", scrHIGHCOLOR,
										"PRESSCOLOR", scrPRESSCOLOR,
										"FORECOLOR", scrFORECOLOR,
										"SCROLLBARSIZE", scrollsize,
										NULL), NULL), "NAME", "coeditor_vbox", NULL), NULL),
									"NAME", "SourceExDetach",
									"ORIENTATION", "HORIZONTAL",
									NULL), NULL),
									"NAME", "CoSourceExpander",
									"BARSIZE", "0",
									"BARPOSITION", "LEFT",
									//"FONT", "::1",
									"MINSIZE", "0x0",
									NULL),
							NULL), 
							"NAME", "SourceSplitMiddle",
						 	"SHOWGRIP", "NO",
							"COLOR", scrFORECOLOR,
							"BARSIZE", "0",
							"VALUE", "1000",
							"LAYOUTDRAG", "NO",
							"MINSIZE", "x1",
							NULL),
							IupSetAtt(NULL, IupCreatep("expander",NULL),
								"NAME", "CoSourceExpanderBtm",
								"BARSIZE", "0",
								"BARPOSITION", "TOP",
								//"FONT", "::1",
								"MINSIZE", "0x0",
							NULL),
					NULL),
					"ORIENTATION", "HORIZONTAL",
					"NAME", "SourceSplitBtm",
				 	"SHOWGRIP", "NO",
				    "COLOR", scrFORECOLOR,
					"BARSIZE", "0",
					"VALUE", "1000",
					"LAYOUTDRAG", "NO",
					"MINSIZE", "x1",
					NULL),
				IupSetAtt(NULL, IupCreatep("expander",
				IupSetAtt(NULL, IupCreatep("scrollbox",
					NULL),
					"NAME", "RightBarPH",
					"SCROLLBAR", "NO",
					NULL),
					NULL),
					"NAME", "RightBarExpander",
					"BARSIZE", "0",
					"BARPOSITION", "LEFT",
					"MINSIZE", "x0",

					//"STATE", "CLOSE",
				NULL),
			NULL),
			"DIRECTION", "EAST",
			"NAME", "SourceSplitRight",
		 	"SHOWGRIP", "NO",
			"COLOR", scrFORECOLOR,
			"BARSIZE", "5",
			"VALUE", "1000",
			"LAYOUTDRAG", "NO",
			"MINSIZE", "x1",
		NULL),
		NULL),
		"DIRECTION", "WEST",
		"NAME", "SourceSplitLeft",
	 	"SHOWGRIP", "NO",
		"COLOR", scrFORECOLOR,
		"BARSIZE", "0",
		"VALUE", "0",
		"LAYOUTDRAG", "NO",
		//"MINSIZE", "x1",
	NULL);

	containers[2] = IupSetAtt(NULL, IupCreatep("hbox",
		containers[3],
		NULL),
		"NAME", "SourceHB",
		NULL);

	containers[7] = 
	IupSetAtt(NULL, IupCreatep("expander", 
		IupSetAtt(NULL, IupCreatep("sc_detachbox", 
			IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("scrollcanvas"),
				"NAME", "Run",
				"MINSIZE", "x20",
				"HIGHCOLOR", scrHIGHCOLOR,
				"PRESSCOLOR", scrPRESSCOLOR,
				"FORECOLOR", scrFORECOLOR,
				"SCROLLBARSIZE", scrollsize,
				NULL),
				NULL), "NAME", "concolebar_vbox", NULL), NULL),
			"NAME", "ConsoleDetach",
			"ORIENTATION", "HORIZONTAL",
			NULL),
		NULL),
		"NAME", "ConsoleExpander",
		"BARSIZE", "0",
		"BARPOSITION", "LEFT",
		"FONT", "::1",
		"MINSIZE", "0x0", 
	NULL);

	containers[10] = 
	IupSetAtt(NULL, IupCreatep("expander", 
		IupSetAtt(NULL, IupCreatep("sc_detachbox", 
			IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("scrollcanvas"),
				"NAME", "FindRes",
				"MINSIZE", "x20",
				"HIGHCOLOR", scrHIGHCOLOR,
				"PRESSCOLOR", scrPRESSCOLOR,
				"FORECOLOR", scrFORECOLOR,
				"SCROLLBARSIZE", scrollsize,
				NULL),
				NULL), "NAME", "findresbar_vbox", NULL), NULL),
			"NAME", "FindResDetach",
			"ORIENTATION", "HORIZONTAL",
			NULL),
		NULL),
		"NAME", "FindResExpander",
		"BARSIZE", "0",
		"BARPOSITION", "LEFT",
		"FONT", "::1",
		"MINSIZE", "0x0", 
	NULL);


	containers[6] = 
	IupSetAtt(NULL, IupCreatep("split",
		containers[7],
		containers[10],
		NULL),
		"NAME", "BottomSplit",
	 	"SHOWGRIP", "NO",
		"COLOR", scrFORECOLOR,
		"BARSIZE", "5",
		"LAYOUTDRAG", "NO",
	NULL);

	containers[9] = IupSetAtt(NULL, IupCreatep("scrollbox",
		NULL),
		"NAME", "FindPlaceHolder",
		"SCROLLBAR", "NO",
		NULL);

	containers[8] = 
	IupSetAtt(NULL, IupCreatep("split",
		containers[6],
		containers[9],
		NULL),
		"NAME", "BottomSplit2",
	 	"SHOWGRIP", "NO",
		"COLOR", scrFORECOLOR,
		"BARSIZE", "0",
		"LAYOUTDRAG", "NO",
		"BGCOLOR", "255 255 255",
		"VALUE", "1000",
	NULL);

	containers[5] = 
	IupSetAtt(NULL, IupCreatep("hbox",
		containers[8],
		NULL),
		"NAME", "BottomSplitParent",
		"MINSIZE", "x20",	 
		//"VISIBLE", "NO",
	NULL);


	containers[4] = IupSetAtt(NULL, IupCreatep("expander",
		containers[5],
		NULL),
		"NAME", "BottomExpander",
		"BARSIZE", "0",
		"FONT", "::1",
		"MINSIZE", "x0",
	NULL);


	containers[1] = IupSetAtt(NULL, IupCreatep("vbox",
		IupSetAtt(NULL, IupCreatep("expander",
			pTab,
			NULL),
			"NAME", "TabbarExpander",
			"BARSIZE", "0",
			"EXPAND", "HORIZONTAL",
			"MINSIZE", "x0",
		NULL),

		IupSetAtt(NULL, IupCreatep("split", containers[2],
			containers[4], NULL),
			"ORIENTATION", "HORIZONTAL",
			"NAME", "BottomBarSplit",
	 		"SHOWGRIP", "NO",
			"COLOR", scrFORECOLOR,
			"BARSIZE", "5",
			"LAYOUTDRAG", "NO",
		NULL),
	NULL),
	"NAME", "SciteVB",
	"MINSIZE", "100x100",
	//"VISIBLE", "NO",
	NULL);

	containers[0] = IupSetAtt(NULL, IupCreatep("dialog",
		containers[1],
		NULL),
		"NAME", "LAYOUT",
		"CONTROL", "YES",
		"MINSIZE", "200x200",
		"SIZE", "200x200",
		"SHRINK", "YES", 
		NULL);

	return containers[0];
}

static int cf_iup_get_layout(lua_State *L){
	iuplua_pushihandle(L, pLayout->hMain);  
	return 1;
}

void IupLayoutWnd::Fit(){
	RECT r;
	::GetClientRect((HWND)pSciteWin->GetID(), &r);
	::SetWindowPos((HWND)IupGetAttribute(hMain, "HWND"), HWND_TOP, 0, 0, r.right, r.bottom, 0);
	IupRefresh(pLeftTab);
	IupRefresh(pRightTab);
}

void IupLayoutWnd::Close(){
	HWND h = (HWND)IupGetAttribute(hMain, "HWND");
	IupDestroy(hMain);

	::CloseWindow(h);
	IupClose();
}

void IupLayoutWnd::CreateLayout(lua_State *L, SciTEWin *pS){
	pSciteWin = pS;
	hMain = Create_dialog();
	IupSetAttribute(hMain, "NATIVEPARENT", (const char*)pS->GetID());
	IupShowXY(hMain, 0, 0);
	HWND h = (HWND)IupGetAttribute(hMain, "HWND");
	//subclassedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatWndProc)));
	SetWindowLongPtr(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

HWND IupLayoutWnd::GetChildHWND(const char* name){
	if(name)
		return (HWND)IupGetAttribute(IupGetDialogChild(hMain, name), "HWND");
	return (HWND)IupGetAttribute(IupGetDialogChild(hMain, "LAYOUT"), "HWND");
}


void IupLayoutWnd::SubclassChild(const char* name, GUI::ScintillaWindow *pW){
	IupChildWnd *pICH = new IupChildWnd();
	::SetProp(GetChildHWND(name), L"iPw", (HANDLE)pW);
	Ihandle *pCnt = IupGetDialogChild(hMain, name);

	classList[name] = pICH;
	pICH->Attach(GetChildHWND(name), pSciteWin, name, (HWND)IupGetAttribute(hMain, "HWND"), pW, pCnt);
	childMap[name] = pICH;
	RECT rc;
	::GetWindowRect(GetChildHWND(name), &rc);
	if(pW) ::SetWindowPos((HWND)pW->GetID(), HWND_TOP, 0, 0, rc.right, rc.bottom, 0);
}

void IupLayoutWnd::GetPaneRect(const char *name, LPRECT pRc){
	::GetClientRect(GetChildHWND(name), pRc);		  
}

void IupLayoutWnd::SetPaneHeight(const char *name, int Height){
	Ihandle *h = IupGetDialogChild(hMain, name);
	char size[20];
	size[0] = 0;
	sprintf(size, "x%d", Height);

	IupSetAttribute(h, "RASTERSIZE", size);
	//IupSetAttribute(h, "SIZE", "x50");
}

void IupLayoutWnd::AdjustTabBar(){
	RECT rc;
	HWND hTab = ::GetWindow(GetChildHWND("SciTeTabCtrl"), GW_CHILD);
	GetPaneRect("SciTeTabCtrl", &rc);
	int width = rc.right;
	SetWindowPos(hTab,
		0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

	RECT r = { 0, 0, width - 2, 0 };
	::SendMessage(hTab, TCM_ADJUSTRECT, TRUE, LPARAM(&r));
	SetPaneHeight("SciTeTabCtrl", r.bottom - r.top - 2);
	IupRefresh(hMain);
}


LRESULT PASCAL IupLayoutWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_SIZE:
	{
		RECT rc;
		HWND hTab = ::GetWindow(GetChildHWND("SciTeTabCtrl"), GW_CHILD);
		GetPaneRect("SciTeTabCtrl", &rc);
		rc.right = LOWORD(lParam) - 2;
			SetWindowPos(hTab,
			0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

		RECT r = {0,0,LOWORD(lParam) - 2,0};
		::SendMessage(hTab, TCM_ADJUSTRECT, TRUE, LPARAM(&r));
		SetPaneHeight("SciTeTabCtrl", r.bottom - r.top -2);
		char *cc = IupGetAttribute(IupGetDialogChild(hMain, "SciteVB"), "SIZE");
		int tt = 0;
		return subclassedProc(hwnd, uMsg, wParam, lParam);

	}
	}
	return subclassedProc(hwnd, uMsg, wParam, lParam);
	
}

LRESULT PASCAL IupLayoutWnd::StatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	IupLayoutWnd* lpIupLayoutWnd = reinterpret_cast<IupLayoutWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (lpIupLayoutWnd)
		return lpIupLayoutWnd->WndProc(hwnd, uMsg, wParam, lParam);

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}