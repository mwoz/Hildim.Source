#include "SciTEWin.h"

IupChildWnd::IupChildWnd()
{
}


IupChildWnd::~IupChildWnd()
{
}

void IupChildWnd::Attach(HWND h, SciTEWin *pS, const char *pName, HWND hM)
{
	hMainWnd = hM;
	pSciteWin = pS;
	subclassedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatWndProc)));
	SetWindowLongPtr(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowLong(h, GWL_STYLE, GetWindowLong(h, GWL_STYLE) | WS_CLIPCHILDREN);
	lstrcpynA(name, pName, 15);
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
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		if(::IsWindowVisible(hMainWnd) )return pSciteWin->WndProc(uMsg, wParam, lParam);
		break;
	case WM_SIZE: 	{
		::SetWindowPos(::GetWindow(hwnd, GW_CHILD), HWND_TOP, 0, 0, LOWORD(lParam), HIWORD(lParam), 0);
		//pSciteWin->SizeSubWindows();
						
	}
		break;
	case WM_LBUTTONDBLCLK:
	{
		if (!strcmp(pSciteWin->Property("tabbar.tab.close.on.doubleclick"),"1") )
			pSciteWin->WndProc(WM_COMMAND, IDM_NEW, 0);
	}
		break;
	case WM_CLOSE:
		LRESULT r = subclassedProc(hwnd, uMsg, wParam, lParam);
		delete(this);
		return r;
	//case WM_SIZE:
	//	subclassedProc(hwnd, uMsg, wParam, lParam);
	//	pSciteWin->WndProc(uMsg, wParam, lParam);
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
	Ihandle* containers[13];

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
								IupSetAtt(NULL, IupCreate("canvas"),
									"NAME", "Source",
									"EXPAND", "YES",
								NULL),
								IupSetAtt(NULL, IupCreatep("expander",
									IupSetAtt(NULL, IupCreatep("sc_detachbox",
										IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("canvas"),
										"NAME", "CoSource",
										"EXPAND", "YES",
										NULL), NULL), "NAME", "coeditor_vbox", NULL), NULL),
									"NAME", "SourceExDetach",
									"ORIENTATION", "HORIZONTAL",
									NULL), NULL),
									"NAME", "CoSourceExpander",
									"BARSIZE", "0",
									"BARPOSITION", "LEFT",
									"FONT", "::1",
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
								"FONT", "::1",
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
			IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("canvas"),
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
			IupSetAtt(NULL, IupCreatep("vbox", IupSetAtt(NULL, IupCreate("canvas"),
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
			IupSetAtt(NULL, IupCreate("canvas"),
				//"MAXSIZE", "x20",
				"EXPAND", "HORIZONTAL",
				"NAME", "SciTeTabCtrl",
				"MINSIZE", "x20",
			NULL),
			NULL),
			"NAME", "TabbarExpander",
			"BARSIZE", "0",
			"FONT", "::1",
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
}

void IupLayoutWnd::Close(){
	HWND h = (HWND)IupGetAttribute(hMain, "HWND");
	IupDestroy(hMain);

	::CloseWindow(h);
	IupClose();
}

void IupLayoutWnd::CreateLayout(lua_State *L, SciTEWin *pS){
	hMain = Create_dialog();
	pSciteWin = pS;
	IupSetAttribute(hMain, "NATIVEPARENT", (const char*)pS->GetID());
	IupShowXY(hMain, 0, 0);
	HWND h = (HWND)IupGetAttribute(hMain, "HWND");
	subclassedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatWndProc)));
	SetWindowLongPtr(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

HWND IupLayoutWnd::GetChildHWND(const char* name){
	return (HWND)IupGetAttribute(IupGetDialogChild(hMain, name), "HWND");
}


void IupLayoutWnd::SubclassChild(const char* name, const GUI::Window *pW){
	IupChildWnd *pICH = new IupChildWnd();
	::SetProp(GetChildHWND(name), L"iPw", (HANDLE)pW);
	pICH->Attach(GetChildHWND(name), pSciteWin, name, (HWND)IupGetAttribute(hMain, "HWND"));
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