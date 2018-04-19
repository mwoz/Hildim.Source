#include "SciTEWin.h"

std::map<std::string, IupChildWnd*> classList;

static int iScroll_CB(Ihandle *ih, int op, float posx, float posy) {
	classList[IupGetAttribute(ih, "NAME")]->Scroll_CB(op, posx, posy);
	return IUP_DEFAULT;
}

IupChildWnd::IupChildWnd()
{
}


IupChildWnd::~IupChildWnd()
{
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
	pScintilla->Call(SCI_SETVSCROLLBAR, false);
	pScintilla->Call(SCI_SETHSCROLLBAR, false);
	pContainer = pCnt;
	IupSetCallback(pCnt, "SCROLL_CB", (Icallback)iScroll_CB);
}

void IupChildWnd::SizeEditor() {
	int x, y;
	IupGetIntInt(pContainer, "RASTERSIZE", &x, &y);
	::SetWindowPos((HWND)pScintilla->GetID(), HWND_TOP, 0, 0, x - vPx, y - hPx, 0);
}

void IupChildWnd::RecetHScrollBar() {
	IupSetDouble(pContainer, "XMAX", pScintilla->Call(SCI_GETSCROLLWIDTH));
	IupSetDouble(pContainer, "DX", pScintilla->Call(SCI_H_TEXTRECTWIDTH));
	IupSetDouble(pContainer, "POSX", pScintilla->Call(SCI_GETXOFFSET));
	int newSize = IupGetInt(pContainer, "XHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE");
	if (newSize != hPx) {
		hPx = newSize;
		SizeEditor();
	}
}

void IupChildWnd::RecetVScrollBar() {
	int dy = pScintilla->Call(SCI_LINESONSCREEN);

	IupSetDouble(pContainer, "YMAX", pScintilla->Call(SCI_VISIBLEFROMDOCLINE, pScintilla->Call(SCI_GETLINECOUNT) + 1) - 1 + (pScintilla->Call(SCI_GETENDATLASTLINE) ? 0 : dy));
	IupSetDouble(pContainer, "DY", dy);
	IupSetDouble(pContainer, "POSY", pScintilla->Call(SCI_GETFIRSTVISIBLELINE));
	int newSize = IupGetInt(pContainer, "YHIDDEN") ? 0 : IupGetInt(pContainer, "SCROLLBARSIZE");
	if (newSize != vPx) {
		vPx = newSize;
		SizeEditor();
	}
}

LRESULT PASCAL IupChildWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_SETFOCUS:
	{
		HWND h = ::FindWindowEx(hwnd, NULL, NULL, NULL);
		::SetFocus(h);
	}
		return 0;
	case WM_NOTIFY:
		if (::IsWindowVisible(hMainWnd)) {
			SCNotification *notification = reinterpret_cast<SCNotification *>(lParam);
			switch(notification->nmhdr.code){
			case SCN_UPDATEUI: 
				if (notification->updated & SC_UPDATE_V_SCROLL) {
					if (pScintilla->Call(SCI_GETVSCROLLBAR))
						pScintilla->Call(SCI_SETVSCROLLBAR, false);
					if (blockV)
						break;
					RecetVScrollBar();
				}
				if (notification->updated & (SC_UPDATE_H_SCROLL)) {
					//if (pScintilla->Call(SCI_GETHSCROLLBAR))
					//	pScintilla->Call(SCI_SETHSCROLLBAR, false);
					if (blockH)
						break;
					RecetHScrollBar();

				}
				break;
			}
			//if(::IsWindowVisible(hMainWnd) )return pSciteWin->WndProc(uMsg, wParam, lParam);
			return pSciteWin->WndProc(uMsg, wParam, lParam);
		}
		break;
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		if(::IsWindowVisible(hMainWnd) )return pSciteWin->WndProc(uMsg, wParam, lParam);
		break;
	case WM_SIZE: 	
		SizeEditor();
		break;
	case WM_CLOSE:
		LRESULT r = subclassedProc(hwnd, uMsg, wParam, lParam);
		delete(this);
		return r;
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
			"BARSIZE", "3",
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
								NULL),
								IupSetAtt(NULL, IupCreatep("expander",
									IupSetAtt(NULL, IupCreatep("sc_detachbox",
										IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("scrollcanvas"),
										"NAME", "CoSource",
										"EXPAND", "YES",
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
			"BARSIZE", "3",
			"VALUE", "1000",
			"LAYOUTDRAG", "NO",
			"MINSIZE", "x1",
		NULL),
		NULL),
		"DIRECTION", "WEST",
		"NAME", "SourceSplitLeft",
		"SHOWGRIP", "NO",
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
		"BARSIZE", "3",
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
			"BARSIZE", "3",
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
	subclassedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatWndProc)));
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