#include "SciTEWin.h"
#include <assert.h>
#include "../../iup/src/iup_drvdraw.h"
#include "../../iup/src/iup_draw.h"
#include "../../iup/src/iup_image.h"
#include "../../iup/src/iup_str.h"
#include "../../iup/src/win/iupwin_drv.h"
#include "../../iup/srccontrols/color/iup_colorhsi.h"

std::map<std::string, IupChildWnd*> classList;

static Ihandle* load_image_property_WW(const char *fore) {
	unsigned char imgdata[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	Ihandle* image = IupImage(16, 16, imgdata);

	IupSetAttribute(image, "0", fore);
	IupSetAttribute(image, "1", "BGCOLOR");

	return image;
}

static Ihandle* load_image_uncheck(const char *back, const char *border) {
	unsigned char imgdata[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	Ihandle* image = IupImage(13, 13, imgdata);

	IupSetAttribute(image, "0", back);
	IupSetAttribute(image, "1", border);

	return image;
}

static Ihandle* load_image_check(const char *back, const char *border, const char* forward) {
	unsigned char imgdata[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 1,
		1, 0, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 1,
		1, 0, 2, 2, 0, 0, 0, 2, 2, 2, 0, 0, 1,
		1, 0, 2, 2, 2, 0, 2, 2, 2, 0, 0, 0, 1,
		1, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	Ihandle* image = IupImage(13, 13, imgdata);

	IupSetAttribute(image, "0", back);
	IupSetAttribute(image, "1", border);
	IupSetAttribute(image, "2", forward);

	return image;
}
static Ihandle* load_image_MINIMISE(const char* forward, const char* bg) {
	unsigned char imgdata[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	Ihandle* image = IupImage(10, 10, imgdata);

	IupSetAttribute(image, "0", bg);
	IupSetAttribute(image, "1", forward);

	return image;
}

static Ihandle* load_image_NORMAL(const char* forward, const char* bg) {
	unsigned char imgdata[] = {
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 0 };

	Ihandle* image = IupImage(10, 10, imgdata);

	IupSetAttribute(image, "0", bg);
	IupSetAttribute(image, "1", forward);

	return image;
}

static Ihandle* load_image_CLOSE(const char* forward, const char* bg) {
	unsigned char imgdata[] = {
		0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
		1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
		0, 1, 1, 1, 0, 0, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 0, 0, 1, 1, 1, 0,
		1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
		0, 1, 0, 0, 0, 0, 0, 0, 1, 0 };

	Ihandle* image = IupImage(10, 10, imgdata);

	IupSetAttribute(image, "0", bg);
	IupSetAttribute(image, "1", forward);

	return image;
}

static Ihandle* load_image_MAXIMISE(const char* forward, const char* bg) {
	unsigned char imgdata[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	Ihandle* image = IupImage(10, 10, imgdata);

	IupSetAttribute(image, "0", bg);
	IupSetAttribute(image, "1", forward);

	return image;
}


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
		cs->annotation = 0;
	} else if (cs->size < 10) {
		if (markerid > MARKER_MAX) {
			int i = 0;
			iupStrToInt(value, &i);
			cs->annotation = i;
		} else {
			long d = iupDrawStrToColor(value, 0);;
			unsigned char r = iupDrawRed(d);
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
	}
	markerMaskAll = leftClr.mask | rightClr.mask;
}

void IupChildWnd::VScrollFraw_CB(Ihandle*ih, void* c, int sb_size, int ymax, int pos, int pos2, int highlight, char* fgcolor_drag, char * bgcolor) {
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
			int lineTo = sb_size + i - 1;
			if (lineTo - lineFrom < 3) {
				int idStart = lineFrom - sb_size;
				if ((idStart > 0) && !pixelMap[idStart - 1].left)
					lineFrom--;
				if ((lineTo - lineFrom < 3) && (i < size - 1) && !pixelMap[i].left)
					lineTo++;
			}

			iupdrvDrawRectangle(dc, 0, lineFrom, 5, lineTo, leftClr.clr[clrId - 1], IUP_DRAW_FILL, 1);
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
			int lineTo = sb_size + i - 1;
			if (lineTo - lineFrom < 3) {
				int idStart = lineFrom - sb_size;
				if ((idStart > 0) && !pixelMap[idStart - 1].right)
					lineFrom--;
				if ((lineTo - lineFrom < 3) && (i < size - 1) && !pixelMap[i].right)
					lineTo++;
			}
			iupdrvDrawRectangle(dc, 6, lineFrom, sb_size-1, lineTo, rightClr.clr[clrId - 1], IUP_DRAW_FILL, 1);
		}
		clrId = clrNew;
		lineFrom = lineFromNew;
	}

	int dL, dR;
	dL = 0, dR = 0;
	if (highlight < 4) {
		for (int i = pos; (i <= pos2) && i < size; i++) {
			if (pixelMap[i].left) {
				dL = 1;
				break;
			}
		}
		for (int i = pos; (i <= pos2) && i < size; i++) {
			if (pixelMap[i].right) {
				dR = 1;
				break;
			}
		}

	} else 		{
		int uuu = 0;
	}

	iupFlatDrawBox(dc, 2 + dL, sb_size - 3 - dR, pos, pos2, fgcolor_drag, bgcolor, 1);

}

void IupChildWnd::resetPixelMap() {
	if (!vPx || !colodizedSB)
		return;
	
	pixelMap.assign(vHeight + 1, { 0, 0 });
	int docCount = pS->Call(SCI_GETLINECOUNT);
	int count = pS->Call(SCI_VISIBLEFROMDOCLINE, docCount) + pS->Call(SCI_LINESONSCREEN);
	if (!count)
		return;

	float lineheightPx = (float)vHeight / (float)count;
	
	int vLine, curMark;
	for(int line = 0; line <= docCount; line ++){
		if (pS->Call(SCI_GETLINEVISIBLE, line)) {
			curMark = pS->Call(SCI_MARKERGET, line);
			if (curMark & leftClr.mask) {
				vLine = pS->Call(SCI_VISIBLEFROMDOCLINE, line);
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
					if(!pixelMap[i].left || pixelMap[i].left > id + 1) //îò ïåðâûõ ïî ñïèñêó öâåòîâ âñåãäà ÷òî-òî îñòàíåòñÿ
						pixelMap[i].left = id + 1;
				}
			}
			if (curMark & rightClr.mask) {
				vLine = pS->Call(SCI_VISIBLEFROMDOCLINE, line);
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
					if (!pixelMap[i].right || pixelMap[i].right > id + 1)
						pixelMap[i].right = id + 1;
				}
			}
			int annotLines;
			if ((rightClr.annotation || leftClr.annotation) && (annotLines = pS->Call(SCI_ANNOTATIONGETLINES, line))) {
				vLine = pS->Call(SCI_VISIBLEFROMDOCLINE, line);
				int pFirst = round((vLine + 1) * lineheightPx);
				int pLast = max(pFirst, round((vLine + annotLines + 1) * lineheightPx));
				for (int i = pFirst; i <= pLast; i++) {
					if (rightClr.annotation && (!pixelMap[i].right || pixelMap[i].right > rightClr.annotation))
						pixelMap[i].right = rightClr.annotation;
					if (leftClr.annotation && (!pixelMap[i].left || pixelMap[i].left > leftClr.annotation))
						pixelMap[i].left = leftClr.annotation;
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
void IupChildWnd::Attach(HWND h, void *pScite, const char *pName, HWND hM, GUI::ScintillaWindow *pW, Ihandle *pCnt)
{
	hMainWnd = hM;
	pSciteWin = (SciTEWin*)pScite;
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
	bNeedSize = false;
	int x, y;
	IupGetIntInt(pContainer, "RASTERSIZE", &x, &y);
	RECT r;
	::GetWindowRect((HWND)pS->GetID(), &r);
	if((r.right - r.left != x - vPx) || (r.bottom - r.top != y - hPx))
		::SetWindowPos((HWND)pS->GetID(), HWND_TOP, 0, 0, x - vPx, y - hPx, 0);

}

void IupChildWnd::HideScrolls() {
	vPx = 0;
	hPx = 0;
    SizeEditor();
	return;
}

void IupChildWnd::OnIdle() {
	if (resetmap) {
		resetPixelMap();
		IupSetAttribute(pContainer, "REDRAWVSCROLL", "");
	}
	resetmap = false;
	if (bNeedSize)
		SizeEditor();
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
				resetmap = true;
			}
			if (lpsi->fMask & SIF_PAGE) {
				IupSetInt(pContainer, "DY", lpsi->nPage);
				resetmap = true;
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
				RECT r;
				::GetWindowRect((HWND)IupGetAttribute(pContainer, "HWND"), &r);
				int newVHeight = r.bottom - r.top - 2 * vPx - hPx;
				if (vPx && (vHeight != newVHeight)) {
					vHeight = newVHeight;
					resetmap = true;
				}
				//SizeEditor();
				bNeedSize = false;
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
				RECT r;
				::GetWindowRect((HWND)IupGetAttribute(pContainer, "HWND"), &r);
				int newVHeight = r.bottom - r.top - 2 * vPx - hPx;
				if (vPx && (vHeight != newVHeight)) {
					vHeight = newVHeight;
					resetmap = true;
				}
				//SizeEditor();
				bNeedSize = true;
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
		case SCN_MARGINCLICK:
		case SCN_MODIFIED:
				resetmap = true;
			break;
		}
	}
		if (::IsWindowVisible(hMainWnd))return ((SciTEWin*)pSciteWin)->WndProc(uMsg, wParam, lParam);
		break;
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		if(::IsWindowVisible(hMainWnd) )return ((SciTEWin*)pSciteWin)->WndProc(uMsg, wParam, lParam);
		break;
	case WM_SIZE:
	{
		int newVHeight = HIWORD(lParam) - 2 * vPx - hPx;
		if (vPx && (vHeight != newVHeight)) {
			vHeight = newVHeight;
			resetmap = true;
			//resetPixelMap();
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

void IupLayoutWnd::PropGet(const char *name, const char *defoult, char* buff) {
	const char* clr = ((SciTEWin*)pSciteWin)->Property(name);
	if (!strcmp(clr, "")) {
		clr = defoult;
		((SciTEWin*)pSciteWin)->SetProperty(name, clr);
	}
	lstrcpynA(buff, clr, 12);
}

Ihandle* IupLayoutWnd::Create_dialog()
{
	Ihandle* pTab;
	Ihandle* containers[13];

	char* fntSize = ((SciTEWin*)pSciteWin)->Property("iup.defaultfontsize");

	captWidth = 20;
	if (strcmp(fntSize, "") && StrToIntA(fntSize) > 0) {
		IupSetGlobal("DEFAULTFONTSIZE", fntSize);
		if (atoi(fntSize) > 10)
			captWidth = 30;
	}
	IupSetGlobal("ICON", "SCITE");
	static char minSz[10];
	::ZeroMemory((void*)minSz, sizeof(char) * 10);
	minSz[0] = '0';
	minSz[1] = 'x';
	lstrcatA(minSz, fntSize);
	
	static char scrFORECOLOR[14], scrPRESSCOLOR[14], scrHIGHCOLOR[14], scrBACKCOLOR[14],
		scrHLCOLOR[14], scrBORDERHLCOLOR[14], scrBORDERCOLOR[14],
		scrBGCOLOR[14], scrTXTBGCOLOR[14], scrFGCOLOR[14],
		scrTXTFGCOLOR[14], scrTXTHLCOLOR[14], scrTXTINACTIVCOLOR[14], scrSPLITCOLOR[14], scrollsize[4], framesize[4];
	_itoa(::GetSystemMetrics(SM_CYSIZEFRAME), framesize, 10);

	PropGet("layout.splittercolor", "220 220 220", scrSPLITCOLOR);
	PropGet("layout.scroll.forecolor", "190 190 190", scrFORECOLOR);
	PropGet("layout.scroll.presscolor", "150 150 150", scrPRESSCOLOR);
	PropGet("layout.scroll.highcolor", "170 170 170", scrHIGHCOLOR);
	PropGet("layout.scroll.backcolor", "240 240 240", scrBACKCOLOR);

	PropGet("layout.hlcolor", "200 225 245", scrHLCOLOR);
	PropGet("layout.borderhlcolor", "50 150 255", scrBORDERHLCOLOR);
	PropGet("layout.bordercolor", "200 200 200", scrBORDERCOLOR);
	PropGet("layout.bgcolor", "240 240 240", scrBGCOLOR);
	PropGet("layout.txtbgcolor", "255 255 255", scrTXTBGCOLOR);
	PropGet("layout.fgcolor", "0 0 0", scrFGCOLOR);
	PropGet("layout.txtfgcolor", "0 0 0", scrTXTFGCOLOR);
	PropGet("layout.txthlcolor", "15 60 195", scrTXTHLCOLOR);
	PropGet("layout.txtinactivcolor", "70 70 70", scrTXTINACTIVCOLOR);
	PropGet("iup.scrollbarsize", "15", scrollsize);
	((SciTEWin*)pSciteWin)->SetProperty("layout.wndframesize", framesize);

	IupSetHandle("property_µ", load_image_property_WW(scrFGCOLOR));
	IupSetHandle("check_µ", load_image_check(scrBGCOLOR, scrBORDERCOLOR, scrFGCOLOR));
	IupSetHandle("uncheck_µ", load_image_uncheck(scrBGCOLOR, scrBORDERCOLOR));
	IupSetHandle("check_t_µ", load_image_check(scrTXTBGCOLOR, scrBORDERCOLOR, scrTXTFGCOLOR));
	IupSetHandle("uncheck_t_µ", load_image_uncheck(scrTXTBGCOLOR, scrBORDERCOLOR));
	IupSetHandle("uncheck_inactive_µ", load_image_uncheck(scrTXTINACTIVCOLOR, scrBORDERCOLOR));
	
	IupSetHandle("MINIMISE_µ", load_image_MINIMISE(scrFGCOLOR, scrSPLITCOLOR));
	IupSetHandle("NORMAL_µ", load_image_NORMAL(scrFGCOLOR, scrSPLITCOLOR));
	IupSetHandle("CLOSE_µ", load_image_CLOSE(scrFGCOLOR, scrSPLITCOLOR));
	IupSetHandle("MAXIMISE_µ", load_image_MAXIMISE(scrFGCOLOR, scrSPLITCOLOR));
	IupSetHandle("MINIMISE_H_µ", load_image_MINIMISE(scrFGCOLOR, scrHLCOLOR));
	IupSetHandle("NORMAL_H_µ", load_image_NORMAL(scrFGCOLOR, scrHLCOLOR));
	IupSetHandle("CLOSE_H_µ", load_image_CLOSE("255 0 0", scrHLCOLOR));
	IupSetHandle("MAXIMISE_H_µ", load_image_MAXIMISE(scrFGCOLOR, scrHLCOLOR));

	pLeftTab = IupSetAtt(NULL, IupCreate("flattabs_ctrl"),
		"NAME", "TabCtrlLeft",
		"EXPAND", "YES",
		"TABSPADDING", "10x3",
		"EXTRABUTTONS", "1",
		"MAXSIZE", "65535x65535",
		"MINSIZE", minSz,
		"TABSFORECOLOR", scrFGCOLOR,
		"TABSLINECOLOR", scrBORDERCOLOR,
		NULL);
	pRightTab = IupSetAtt(NULL, IupCreate("flattabs_ctrl"),
		"NAME", "TabCtrlRight",
		"EXPAND", "HORIZONTAL",
		"TABSPADDING", "10x3",
		"EXTRABUTTONS", "1",
		"MAXSIZE", "65535x65535",
		"MINSIZE", minSz,
		"TABSFORECOLOR", scrFGCOLOR,
		"TABSLINECOLOR", scrBORDERCOLOR,
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
		"COLOR", scrSPLITCOLOR,
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
				"BARSIZE", "0",
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
							"BACKCOLOR", scrBACKCOLOR,
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
									"BACKCOLOR", scrBACKCOLOR,
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
						"COLOR", scrSPLITCOLOR,
						"BARSIZE", "0",
						"VALUE", "1000",
						"LAYOUTDRAG", "NO",
						"MINSIZE", "x1",
						NULL),
					IupSetAtt(NULL, IupCreatep("expander", NULL),
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
					"COLOR", scrSPLITCOLOR,
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
				"COLOR", scrSPLITCOLOR,
				"BARSIZE", "5",
				"VALUE", "1000",
				"LAYOUTDRAG", "NO",
				"MINSIZE", "x1",
				NULL),
			NULL),
			"DIRECTION", "WEST",
			"NAME", "SourceSplitLeft",
			"SHOWGRIP", "NO",
			"COLOR", scrSPLITCOLOR,
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
					"BACKCOLOR", scrBACKCOLOR,
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
					"BACKCOLOR", scrBACKCOLOR,
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
			"COLOR", scrSPLITCOLOR,
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
			"COLOR", scrSPLITCOLOR,
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
			"COLOR", scrSPLITCOLOR,
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
		"FLAT", "YES",
		"HLCOLOR", scrHLCOLOR,
		"BORDERHLCOLOR", scrBORDERHLCOLOR,
		"BORDERCOLOR", scrBORDERCOLOR,
		"BGCOLOR", scrBGCOLOR,
		"TXTBGCOLOR", scrTXTBGCOLOR,
		"FGCOLOR", scrFGCOLOR,
		"TXTFGCOLOR", scrTXTFGCOLOR,
		"TXTHLCOLOR", scrTXTHLCOLOR,
		"TXTINACTIVCOLOR", scrTXTINACTIVCOLOR,
		"CAPTBGCOLOR", scrSPLITCOLOR,
		"SCR_FORECOLOR", scrFORECOLOR,
		"SCR_PRESSCOLOR", scrPRESSCOLOR,
		"SCR_HIGHCOLOR", scrHIGHCOLOR,
		"SCR_BACKCOLOR", scrBACKCOLOR,
		NULL);
	//IupSetGlobal("DLGBGCOLOR", "0 0 250");
	//IupSetGlobal("MENUBGCOLOR", "0 255 255");
	
	return containers[0];
}

static int cf_iup_get_layout(lua_State *L){
	iuplua_pushihandle(L, pLayout->hMain);  
	return 1;
}

void IupLayoutWnd::Fit(){
	RECT r;
	::GetClientRect((HWND)((SciTEWin*)pSciteWin)->GetID(), &r);
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

void IupLayoutWnd::CreateLayout(lua_State *L, void *pS){
	pSciteWin = (SciTEBase*)pS;
	hMain = Create_dialog();
	IupSetAttribute(hMain, "NATIVEPARENT", (const char*)((SciTEWin*)pSciteWin)->GetID());
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

void IupLayoutWnd::OnSwitchFile(int editorSide) {
	if (editorSide == IDM_SRCWIN)
		classList["Source"]->resetPixelMap();
	else
		classList["CoSource"]->resetPixelMap();
}

void IupLayoutWnd::OnOpenClose(int editorSide) {
	if (editorSide == IDM_SRCWIN)
		classList["Source"]->HideScrolls();
	else
		classList["CoSource"]->HideScrolls();
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

COLORREF IupLayoutWnd::GetColorRef(char* name) {
	COLORREF cr;
	iupwinGetColorRef(hMain, name, &cr);
	return cr;
}

LRESULT PASCAL IupLayoutWnd::StatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	IupLayoutWnd* lpIupLayoutWnd = reinterpret_cast<IupLayoutWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (lpIupLayoutWnd)
		return lpIupLayoutWnd->WndProc(hwnd, uMsg, wParam, lParam); 

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT IupLayoutWnd::OnNcCalcSize(HWND hwnd, BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp) {
	LRESULT r =::DefWindowProc(hwnd, WM_NCCALCSIZE, (WPARAM)bCalcValidRects, (LPARAM)lpncsp);
	if (bCalcValidRects && !FullScreen) {
		int ibrd = ::GetSystemMetrics(SM_CYSIZEFRAME) + 1;
		lpncsp->rgrc[0].top = lpncsp->rgrc[1].top + captWidth;
		lpncsp->rgrc[0].bottom = lpncsp->rgrc[1].bottom - ibrd;
		lpncsp->rgrc[0].left = lpncsp->rgrc[1].left + ibrd;
		lpncsp->rgrc[0].right = lpncsp->rgrc[1].right - ibrd;
	}
	return r;
}

void drawWinBtn(HDC hDC, HBITMAP hBitmap, int x, int y) {
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	BitBlt(hDC, x, y, 15, 15, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, oldBitmap);
	DeleteDC(hMemDC);
}

bool IupLayoutWnd::HilightBtn(RECT *rect, POINT *p, HDC hdc, int iBtn, COLORREF clr) {
	RECT r = { rect->right - rect->left - (captWidth + 10) * iBtn , 0, 
		rect->right - rect->left - (captWidth + 10) * (iBtn - 1) , captWidth };
	bool rez = (r.left <= p->x) && (r.right > p->x) && (r.top <= p->y) && (r.bottom >= p->y);
	if (rez && clr) {
		rez = ((iBtn == 1 && nBtn == HTCLOSE) || (iBtn == 2 && nBtn == HTMAXBUTTON) || (iBtn == 3 && nBtn == HTMINBUTTON));
		if (rez) {
			HBRUSH b = CreateSolidBrush(clr);
			FillRect(hdc, &r, b);
			DeleteObject(b);
		}
	}
	return rez;
}

LRESULT IupLayoutWnd::OnNcHitTest(HWND hwnd, POINT mouse) {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	mouse.x -= rect.left;
	mouse.y -= rect.top;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_NONCLIENT | TME_LEAVE;
	tme.hwndTrack = hwnd;
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);

	if (HilightBtn(&rect, &mouse, NULL, 1, NULL))
		return HTCLOSE;
	if (HilightBtn(&rect, &mouse, NULL, 2, NULL))
		return HTMAXBUTTON;
	if (HilightBtn(&rect, &mouse, NULL, 3, NULL))
		return HTMINBUTTON;
	return HTCAPTION;

}
LRESULT IupLayoutWnd::OnNcMouseMove(HWND hwnd, int iBtn) {
	if (iBtn != nBtn) {
		nBtn = iBtn;
		if (nBtnPressed != iBtn)
			nBtnPressed = 0;
		OnNcPaint(hwnd, true);
	}
	return 0;
}
LRESULT IupLayoutWnd::OnNcLMouseDown(HWND hwnd, int iBtn) {
	nBtnPressed = iBtn;
	return 0;
}
LRESULT IupLayoutWnd::OnNcLMouseUp(HWND hwnd, int iBtn) {
	if (nBtnPressed == iBtn) {
		switch (iBtn) {
		case HTMINBUTTON:
			::ShowWindow(hwnd, SW_MINIMIZE);
			break;
		case HTMAXBUTTON:
			{
				WINDOWPLACEMENT wp;
				::GetWindowPlacement(hwnd, &wp);
				if (wp.showCmd == SW_SHOWNORMAL)
					::ShowWindow(hwnd, SW_MAXIMIZE);
				else
					::ShowWindow(hwnd, SW_NORMAL);
			}
			break;
		case HTCLOSE:
			((SciTEWin*)pSciteWin)->DoMenuCommand(IDM_QUIT);
		}
	}
	return 0;
}
LRESULT IupLayoutWnd::OnNcPaint(HWND hwnd, BOOL bActiv) {
	if (FullScreen)
		return 0;
	WINDOWPLACEMENT wp;
	::GetWindowPlacement(hwnd, &wp);
	if (wp.showCmd == SW_SHOWMINIMIZED)
		return 0;	
	HDC hdc, hdcw;
	RECT rect;
	HBRUSH b;

	hdc = ::GetWindowDC(hwnd);
	bool bActive = ((hwnd == ::GetForegroundWindow()) && bActiv);

	GetWindowRect(hwnd, &rect);
	HRGN hrgn = CreateRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top);
	b = CreateSolidBrush(GetColorRef("CAPTBGCOLOR"));
	::SetWindowRgn(hwnd, hrgn, false);

	POINT mouse;
	::GetCursorPos(&mouse);
	mouse.x -= rect.left;
	mouse.y -= rect.top;

	int ibrd = ::GetSystemMetrics(SM_CYSIZEFRAME) + 1;

	RECT rdraw;
	if (wp.showCmd == SW_SHOWNORMAL) {
		rdraw = { 0, 0, ibrd, rect.bottom - rect.top };
		FillRect(hdc, &rdraw, b);
		rdraw = { rect.right -rect.left - ibrd, 0, rect.right - rect.left, rect.bottom - rect.top };
		FillRect(hdc, &rdraw, b);
		rdraw = { 0, rect.bottom - rect.top - ibrd, rect.right - rect.left, rect.bottom - rect.top };
		FillRect(hdc, &rdraw, b);
	}
	rdraw = { 0, 0, rect.right - rect.left, captWidth };
	FillRect(hdc, &rdraw, b);

	DeleteObject(b); 
	
	HICON hicon;
	if (captWidth > 20) {
		hicon = (HICON)::LoadImage(::GetModuleHandle(NULL), L"SCITE", IMAGE_ICON, 24, 24, 0);
		::DrawIconEx(hdc, 3, 3, hicon, 24, 24, 0, NULL, DI_NORMAL);
	} else {
		hicon = (HICON)::LoadImage(::GetModuleHandle(NULL), L"SCITE", IMAGE_ICON, 16, 16, 0);
		::DrawIconEx(hdc, 2, 2, hicon, 16, 16, 0, NULL, DI_NORMAL);

	}
	
	char* font;
	font = IupGetGlobal("DEFAULTFONT");
	
	HFONT hFont = (HFONT)iupwinGetHFont(font);
	SelectObject(hdc, hFont);
	SetBkColor(hdc, GetColorRef("CAPTBGCOLOR"));
	SetTextColor(hdc, bActive ? GetColorRef("FGCOLOR") : GetColorRef("TXTINACTIVCOLOR"));
	
	WCHAR cap[500];
	GetWindowText((HWND)((SciTEWin*)pSciteWin)->GetID(), cap, 499);
	rdraw = {40, 0, rect.right - rect.left - captWidth * 3 - 30, captWidth};
	
	::DrawText(hdc, cap, -1, &rdraw, DT_SINGLELINE | DT_VCENTER) ; 

	rdraw = { rect.right - rect.left - (captWidth + 10) + captWidth / 2 , 0, rect.right - rect.left , captWidth };

	COLORREF hl = GetColorRef("HLCOLOR");

	bool bHl = HilightBtn(&rect, &mouse, hdc, 1, hl);
	HBITMAP hb = (HBITMAP)iupImageGetImage(bHl ? "CLOSE_H_µ": "CLOSE_µ", NULL, FALSE, NULL);
	drawWinBtn(hdc, hb, rect.right - rect.left - (captWidth + 10) + captWidth / 2, (captWidth - 10) / 2);

	bHl = HilightBtn(&rect, &mouse, hdc, 2, hl);
	hb = (HBITMAP)iupImageGetImage(wp.showCmd == SW_SHOWNORMAL ? (bHl ? "MAXIMISE_H_µ" : "MAXIMISE_µ") : (bHl ? "NORMAL_H_µ" : "NORMAL_µ"), NULL, FALSE, NULL);
	drawWinBtn(hdc, hb, rect.right - rect.left - (captWidth + 10) * 2 + captWidth / 2, (captWidth - 10) / 2);

	bHl = HilightBtn(&rect, &mouse, hdc, 3, hl);
	hb = (HBITMAP)iupImageGetImage(bHl ? "MINIMISE_H_µ" : "MINIMISE_µ", NULL, FALSE, NULL);
	drawWinBtn(hdc, hb, rect.right - rect.left - (captWidth + 10) * 3 + captWidth / 2, (captWidth - 10) / 2);


	if (bActive && wp.showCmd == SW_SHOWNORMAL) {
		HPEN hPen, hPenOld;
		hPen = CreatePen(PS_SOLID, 2, GetColorRef("BORDERCOLOR"));
		hPenOld = (HPEN)SelectObject(hdc, hPen);
		POINT line_poly[5] = {1, 1, 1, rect.bottom - rect.top -1, rect.right - rect.left -1, rect.bottom - rect.top -1, rect.right - rect.left -1, 1, 1, 1};
		BOOL b = Polyline(hdc, line_poly, 5);
		SelectObject(hdc, hPenOld);
		DeleteObject(hPen);
	}

	//Rectangle(hdc, 0, 0, 6000, ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYSIZEFRAME));
	//int tt = ::BitBlt(hdcw, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdc, 0, 0, SRCCOPY);
	ReleaseDC(hwnd, hdc);
	RedrawWindow(hwnd, &rect, NULL, RDW_UPDATENOW);
	return 0;
}