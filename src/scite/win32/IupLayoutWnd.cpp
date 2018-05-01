#include "SciTEWin.h"
#include <assert.h>
#include "../../iup/src/iup_drvdraw.h"
#include "../../iup/srccontrols/color/iup_colorhsi.h"

std::map<std::string, IupChildWnd*> classList;

static int iScroll_CB(Ihandle *ih, int op, float posx, float posy) {
	classList[IupGetAttribute(ih, "NAME")]->Scroll_CB(op, posx, posy);
	return IUP_DEFAULT;
}

static int iVScrollFraw_CB(Ihandle*ih, IdrawCanvas* dc, int sb_size, int ymax, int pos, int d, int active, char* fgcolor_drag, char * bgcolor) {
	classList[IupGetAttribute(ih, "NAME")]->VScrollFraw_CB(ih, (void*)dc, sb_size, ymax, pos, d, active, fgcolor_drag, bgcolor);
	return IUP_DEFAULT;
}

static int iColorSettings_CB(Ihandle* ih, int side, int markerid, const char* value) {
	classList[IupGetAttribute(ih, "NAME")]->ColorSettings_CB(ih, side, markerid, value);
	return IUP_DEFAULT;
}



IupChildWnd::IupChildWnd()
{
}


IupChildWnd::~IupChildWnd()
{
}

void IupChildWnd::ColorSettings_CB(Ihandle* ih, int side, int markerid, const char* value) {
	sb_colorsetting *cs;
	if (side == 1)
		cs = &leftClr;
	else
		cs = &rightClr;
	if (markerid < 0) {
		cs->size = 0;
		cs->mask = 0;
	} else if (cs->size < 10) {
		long d = iupDrawStrToColor(value, 0);;
		unsigned char r =iupDrawRed(d);
		unsigned char g = iupDrawGreen(d);
		unsigned char b = iupDrawBlue(d);
		double h, s, i;

		iupColorRGB2HSI(r, g, b, &h, &s, &i);

		i = 0.5; s = 0.75;

		iupColorHSI2RGB(h, s, i, &r, &g, &b);

		d = (r << 16) | (g << 8) | b;
		cs->id[cs->size] = markerid;
		cs->clr[cs->size] = d;
		cs->size++;
		cs->mask |= 1 << markerid;
	}
	markerMaskAll = leftClr.mask | rightClr.mask;
}

void IupChildWnd::VScrollFraw_CB(Ihandle*ih, void* c, int sb_size, int ymax, int pos, int d, int active, char* fgcolor_drag, char * bgcolor) {
	IdrawCanvas* dc = (IdrawCanvas*)c;

	int size = pixelMap.size();

	int clrId, lineFrom;
	int clrNew, lineFromNew;
	clrId = 0; lineFrom = 0;
	for (int i = 0; i < size; i++) {
		bool nedDraw = false;
		clrNew = pixelMap[i].left;
		lineFromNew = lineFrom;
		if (clrNew) {
			if (clrId == clrNew) {
				//nothing todo
			} else {
				if (clrId)
					nedDraw = true;
				lineFromNew = sb_size + i;

			}
		} else {
			if (clrId)
				nedDraw = true;
		}
		if (nedDraw) {
			iupdrvDrawRectangle(dc, 0, lineFrom, 5, sb_size + i - 1, leftClr.clr[clrId - 1], IUP_DRAW_FILL, 1);
		}
		clrId = clrNew;
		lineFrom = lineFromNew;
	}

	clrId = 0; lineFrom = 0;
	for (int i = 0; i < size; i++) {
		bool nedDraw = false;
		clrNew = pixelMap[i].right;
		lineFromNew = lineFrom;
		if (clrNew) {
			if (clrId == clrNew) {
				//nothing todo
			} else {
				if (clrId)
					nedDraw = true;
				lineFromNew = sb_size + i;

			}
		} else {
			if (clrId)
				nedDraw = true;
		}
		if (nedDraw) {
			iupdrvDrawRectangle(dc, 6, lineFrom, sb_size-1, sb_size + i - 1, rightClr.clr[clrId - 1], IUP_DRAW_FILL, 1);
		}
		clrId = clrNew;
		lineFrom = lineFromNew;
	}

	//iupFlatDrawBox(dc, 2, sb_size - 3, pos, pos + d, fgcolor_drag, bgcolor, active);

	///iupdrvDrawRectangle(dc, xmin, ymin, xmax, ymax, color, IUP_DRAW_FILL, 1);
}

void IupChildWnd::resetPixelMap() {
	if (!vPx || !colodizedSB)
		return;
	pixelMap.clear();
	pixelMap.assign(vHeight + 1, { 0, 0 });
	int count = pS->Call(SCI_VISIBLEFROMDOCLINE, pS->Call(SCI_GETLINECOUNT)) + pS->Call(SCI_LINESONSCREEN);
	if (!count)
		return;

	float lineheightPx = (float)vHeight / (float)count;
	
	int vLine, curMark;
	for (int line = pS->Call(SCI_MARKERNEXT, 0, markerMaskAll); line != -1; line = pS->Call(SCI_MARKERNEXT,line + 1, markerMaskAll)) {
		if (pS->Call(SCI_GETLINEVISIBLE, line)) {
			curMark = pS->Call(SCI_MARKERGET, line);
			vLine = pS->Call(SCI_VISIBLEFROMDOCLINE, line);

			if (curMark & leftClr.mask) {
				int id = -1;
				for (int i = 0; i < leftClr.size; i++) {
					if (curMark & (1 << leftClr.id[i])) {
						id = i;
						break;
					}
				}
				assert(id > -1);

				int pFirst = round(vLine * lineheightPx);
				int pLast = max(pFirst, round((vLine + 1) * lineheightPx));
				for (int i = pFirst; i <= pLast; i++) {
					pixelMap[i].left = id + 1;
				}
			}
			if (curMark & rightClr.mask) {
				int id = -1;
				for (int i = 0; i < rightClr.size; i++) {
					if (curMark & (1 << rightClr.id[i])) {  
						id = i;
						break;
					}
				}
				assert(id > -1);

				int pFirst = round(vLine * lineheightPx);
				int pLast = max(pFirst, round((vLine + 1) * lineheightPx));
				for (int i = pFirst; i <= pLast; i++) {
					pixelMap[i].right = id + 1;
				}
			}
		}
	}
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
		pS->Call(SCI_SETFIRSTVISIBLELINE, IupGetInt(pContainer, "POSY"));
		blockV = false;
		break;
	case IUP_SBLEFT:
	case IUP_SBRIGHT:
	case IUP_SBPGLEFT:
	case IUP_SBPGRIGHT:
	case IUP_SBPOSH:
	case IUP_SBDRAGH:
		blockH = true;
		pS->Call(SCI_SETXOFFSET, IupGetInt(pContainer, "POSX"));
		blockH = false;
		break;
	}
}
void IupChildWnd::Attach(HWND h, SciTEWin *pScite, const char *pName, HWND hM, GUI::ScintillaWindow *pW, Ihandle *pCnt)
{
	hMainWnd = hM;
	pSciteWin = pScite;
	subclassedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatWndProc)));
	SetWindowLongPtr(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowLong(h, GWL_STYLE, GetWindowLong(h, GWL_STYLE) | WS_CLIPCHILDREN);
	lstrcpynA(name, pName, 15);
	colodizedSB = !strcmp(name, "Source") || !strcmp(name, "CoSource");
	pS = pW;
	pContainer = pCnt;
	IupSetCallback(pCnt, "SCROLL_CB", (Icallback)iScroll_CB);
	IupSetCallback(pCnt, "_COLORSETTINGS_CB", (Icallback)iColorSettings_CB);
	if(colodizedSB)
		IupSetCallback(pCnt, "VSCROLLDRAW_CB", (Icallback)iVScrollFraw_CB); 

}

void IupChildWnd::SizeEditor() {
	int x, y;
	IupGetIntInt(pContainer, "RASTERSIZE", &x, &y);
	::SetWindowPos((HWND)pS->GetID(), HWND_TOP, 0, 0, x - vPx, y - hPx, 0);

}

void IupChildWnd::OnIdle() {
	if (!resetmap)
		return;
	resetPixelMap();
	IupSetAttribute(pContainer, "REDRAWVSCROLL", "");
	resetmap = false;
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
	{
		SCNotification *notification = (SCNotification*)(lParam);
		switch (notification->nmhdr.code) {
		case SCN_MODIFIED:
			if (notification->modificationType & SC_MOD_CHANGEMARKER) {
				resetmap = true;
			}
			if ((notification->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT)) && notification->linesAdded) {
				resetmap = true;
			}
		}
	}
		if (::IsWindowVisible(hMainWnd))return pSciteWin->WndProc(uMsg, wParam, lParam);
		break;
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		if(::IsWindowVisible(hMainWnd) )return pSciteWin->WndProc(uMsg, wParam, lParam);
		break;
	case WM_SIZE:
	{
		int newVHeight = HIWORD(lParam) - 2 * vPx - hPx;
		if (vPx && (vHeight != newVHeight)) {
			vHeight = newVHeight;
			resetPixelMap();
		}
		SizeEditor();
	}
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
	if (!r.right && !r.bottom)
		return;
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
void IupLayoutWnd::OnIdle() {
	classList["Source"]->OnIdle();
	classList["CoSource"]->OnIdle();
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