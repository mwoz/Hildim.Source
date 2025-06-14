// Scintilla source code edit control
/** @file ListBox.cxx
 ** Implementation of list box on Windows.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#include <climits>

#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <iterator>
#include <memory>
#include <mutex>

// Want to use std::min and std::max so don't want Windows.h version of min and max
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#undef WINVER
#define WINVER 0x0A00
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <windowsx.h>
#include <UxTheme.h>
#include <shellscalingapi.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#if !defined(DISABLE_D2D)
#define USE_D2D 1
#endif

#if defined(USE_D2D)
#include <d2d1_1.h>
#include <d3d11_1.h>
#include <dwrite_1.h>
#endif

#include "ScintillaTypes.h"
#include "ScintillaMessages.h"
#include "ScintillaStructures.h"

#include "Debugging.h"
#include "Geometry.h"
#include "Platform.h"
#include "XPM.h"
#include "UniConversion.h"
#include "DBCS.h"

#include "WinTypes.h"
#include "PlatWin.h"
#include "../include/Scintilla.h"
#include "ListBox.h"
#if defined(USE_D2D)
#include "SurfaceD2D.h"
#endif

using namespace Scintilla;
using namespace Scintilla::Internal;

namespace {

void* PtrFromLParam(Scintilla::sptr_t lParam) noexcept {
	return reinterpret_cast<void*>(lParam);
}

struct ListItemData {
	const char *text;
	int pixId;
};

class LineToItem {
	std::vector<char> words;

	std::vector<ListItemData> data;

public:
	void Clear() noexcept {
		words.clear();
		data.clear();
	}

	[[nodiscard]] ListItemData Get(size_t index) const noexcept {
		if (index < data.size()) {
			return data[index];
		}
		ListItemData missing = {"", -1};
		return missing;
	}
	[[nodiscard]] int Count() const noexcept {
		return static_cast<int>(data.size());
	}

	void AllocItem(const char *text, int pixId) {
		const ListItemData lid = { text, pixId };
		data.push_back(lid);
	}

	char *SetWords(const char *s) {
		words = std::vector<char>(s, s+strlen(s)+1);
		return words.data();
	}
};

const TCHAR ListBoxX_ClassName[] = TEXT("ListBoxX");

ColourRGBA ColourElement(std::optional<ColourRGBA> colour, int nIndex) {
	if (colour.has_value()) {
		return colour.value();
	}
	return ColourFromSys(nIndex);
}

struct LBGraphics {
	GDIBitMap bm;
	std::unique_ptr<Surface> pixmapLine;
#if defined(USE_D2D)
	DCRenderTarget pBMDCTarget;
#endif

	void Release() noexcept {
		pixmapLine.reset();
#if defined(USE_D2D)
		pBMDCTarget = nullptr;
#endif
		bm.Release();
	}
};

}

class ListBoxX : public ListBox {
	int lineHeight = 10;
	HFONT fontCopy {};
	std::unique_ptr<FontWin> fontWin;
	Technology technology = Technology::Default;
	RGBAImageSet images;
	LineToItem lti;
	HWND lb {};
	HWND scb;
	bool unicodeMode = false;
	int codePage = 0;
	int desiredVisibleRows = 9;
	int maxItemCharacters = 0;
	unsigned int aveCharWidth = 8;
	Window *parent = nullptr;
	WNDPROC prevWndProc{};
	int ctrlID = 0;
	UINT dpi = USER_DEFAULT_SCREEN_DPI;
	IListBoxDelegate *delegate = nullptr;
	unsigned int maxCharWidth = 1;
	WPARAM resizeHit = 0;
	PRectangle rcPreSize;
	Point dragOffset;
	Point location;	// Caret location at which the list is opened
	MouseWheelDelta wheelDelta;
	ListOptions options;
	DWORD frameStyle = WS_THICKFRAME;

	LBGraphics graphics;

	HWND GetHWND() const noexcept;
	void AppendListItem(const char *text, const char *numword);
	void AdjustWindowRect(PRectangle *rc, UINT dpiAdjust) const noexcept;
	int ItemHeight() const noexcept;
	int MinClientWidth() const noexcept;
	int TextOffset() const noexcept;
	POINT GetClientExtent() const noexcept;
	POINT MinTrackSize() const noexcept;
	POINT MaxTrackSize() const noexcept;
	void SetRedraw(bool on) noexcept;
	void OnDoubleClick();
	void OnSelChange();
	void OnContextMenu();
	void ResizeToCursor();
	void StartResize(WPARAM);
	LRESULT NcHitTest(WPARAM, LPARAM) const;
	void CentreItem(int n);
	void AllocateBitMap();
	LRESULT PASCAL ListProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL ControlWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL ScrollWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	static constexpr POINT ItemInset {0, 0};	// Padding around whole item
	static constexpr POINT TextInset {2, 0};	// Padding around text
	static constexpr POINT ImageInset {1, 0};	// Padding around image

public:
	ListBoxX() = default;
	ListBoxX(const ListBoxX &) = delete;
	ListBoxX(ListBoxX &&) = delete;
	ListBoxX &operator=(const ListBoxX &) = delete;
	ListBoxX &operator=(ListBoxX &&) = delete;
	~ListBoxX() noexcept override {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = {};
		}
		graphics.Release();
	}
	void SetFont(const Font *font) override;
	void Create(Window &parent_, int ctrlID_, Point location_, int lineHeight_, bool unicodeMode_, Scintilla::Technology technology_, void* pColorInfo) override;
	void SetAverageCharWidth(int width) override;
	void SetVisibleRows(int rows) override;
	int GetVisibleRows() const override;
	PRectangle GetDesiredRect() override;
	int CaretFromEdge() override;
	void Clear() noexcept override;
	void Append(char *s, int type) override;
	int Length() override;
	void Select(int n) override;
	int GetSelection() override;
	int Find(const char *prefix) override;
	std::string GetValue(int n) override;
	void RegisterImage(int type, const char *xpm_data) override;
	void RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) override;
	void ClearRegisteredImages() override;
	void SetDelegate(IListBoxDelegate *lbDelegate) override;
	void SetList(const char *list, char separator, char typesep) override;
	void SetOptions(ListOptions options_) override;
	void Draw(DRAWITEMSTRUCT *pDrawItem);
	LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL StaticWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	Scintilla::Sci_ListColorsInfo* pListcolors = nullptr;
	void OnScrollChange();
	void sc_OnPaint(HDC hdc, RECT *rcPaint, bool bDrag, int nTrackPos);
	void sc_Paint(bool bDrag, int nTrackPos);
	bool bBlockRedraw = false;

};

std::unique_ptr<ListBox> ListBox::Allocate() {
	return std::make_unique<ListBoxX>();
}

void ListBoxX::Create(Window &parent_, int ctrlID_, Point location_, int lineHeight_, bool unicodeMode_, Scintilla::Technology technology_, void* pColorInfo) {
	parent = &parent_;
	ctrlID = ctrlID_;
	location = location_;
	lineHeight = lineHeight_;
	unicodeMode = unicodeMode_;
	codePage = unicodeMode ? CpUtf8 : 0;
	technology = technology_;
	pListcolors = (Sci_ListColorsInfo*)pColorInfo;
	HWND hwndParent = HwndFromWindow(*parent);
	HINSTANCE hinstanceParent = GetWindowInstance(hwndParent);
	// Window created as popup so not clipped within parent client area
	wid = ::CreateWindowEx(
		WS_EX_WINDOWEDGE, ListBoxX_ClassName, TEXT(""),
		WS_POPUP | frameStyle,
		100,100, 150,80, hwndParent,
		{},
		hinstanceParent,
		this);

	dpi = DpiForWindow(hwndParent);
	POINT locationw = POINTFromPoint(location);
	::MapWindowPoints(hwndParent, {}, &locationw, 1);
	location = PointFromPOINT(locationw);
}

void ListBoxX::SetFont(const Font *font) {
	const FontWin *pfm = dynamic_cast<const FontWin *>(font);
	if (pfm) {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = {};
		}
		fontCopy = pfm->HFont();
		SetWindowFont(lb, fontCopy, 0);
		fontWin = pfm->Duplicate();
		codePage = unicodeMode ? CpUtf8 : CodePageFromCharSet(fontWin->GetCharacterSet(), 1252);
		graphics.Release();
	}
}

void ListBoxX::SetAverageCharWidth(int width) {
	aveCharWidth = width;
}

void ListBoxX::SetVisibleRows(int rows) {
	desiredVisibleRows = rows;
}

int ListBoxX::GetVisibleRows() const {
	return desiredVisibleRows;
}

HWND ListBoxX::GetHWND() const noexcept {
	return HwndFromWindowID(GetID());
}

PRectangle ListBoxX::GetDesiredRect() {
	PRectangle rcDesired = GetPosition();

	int rows = Length();
	if ((rows == 0) || (rows > desiredVisibleRows))
		rows = desiredVisibleRows;
	rcDesired.bottom = rcDesired.top + ItemHeight() * rows;

	int width = MinClientWidth();
	int textSize = 0;
	int averageCharWidth = 8;

	// Make a measuring surface
	std::unique_ptr<Surface> surfaceItem(Surface::Allocate(technology));
	surfaceItem->Init(GetID());
	surfaceItem->SetMode(SurfaceMode(codePage, false));

	// Find the widest item in pixels
	const int items = lti.Count();
	for (int i = 0; i < items; i++) {
		const ListItemData item = lti.Get(i);
		const int itemTextSize = static_cast<int>(std::ceil(
			surfaceItem->WidthText(fontWin.get(), item.text)));
		textSize = std::max(textSize, itemTextSize);
	}

	maxCharWidth = static_cast<int>(std::ceil(surfaceItem->WidthText(fontWin.get(), "W")));
	averageCharWidth = static_cast<int>(surfaceItem->AverageCharWidth(fontWin.get()));

	width = std::max({ width, textSize, (maxItemCharacters + 1) * averageCharWidth });

	rcDesired.right = rcDesired.left + TextOffset() + width + (TextInset.x * 2);
	if (Length() > rows)
		rcDesired.right += SystemMetricsForDpi(SM_CXVSCROLL, dpi);

	AdjustWindowRect(&rcDesired, dpi);
	return rcDesired;
}

int ListBoxX::TextOffset() const noexcept {
	const int pixWidth = images.GetWidth();
	return pixWidth == 0 ? ItemInset.x : ItemInset.x + pixWidth + (ImageInset.x * 2);
}

int ListBoxX::CaretFromEdge() {
	PRectangle rc;
	AdjustWindowRect(&rc, dpi);
	return TextOffset() + static_cast<int>(TextInset.x + (0 - rc.left) - 1);
}

void ListBoxX::Clear() noexcept {
	ListBox_ResetContent(lb);
	maxItemCharacters = 0;
	lti.Clear();
}

void ListBoxX::Append(char *, int) {
	// This method is no longer called in Scintilla
	PLATFORM_ASSERT(false);
}

int ListBoxX::Length() {
	return lti.Count();
}

void ListBoxX::Select(int n) {
	// We are going to scroll to centre on the new selection and then select it, so disable
	// redraw to avoid flicker caused by a painting new selection twice in unselected and then
	// selected states
	SetRedraw(false);
	CentreItem(n);
	ListBox_SetCurSel(lb, n);
	OnSelChange();
	SetRedraw(true);
	OnScrollChange();
	if (pListcolors->inizialized)
		sc_Paint(false, 0);
}

int ListBoxX::GetSelection() {
	return ListBox_GetCurSel(lb);
}

// This is not actually called at present
int ListBoxX::Find(const char *) {
	return LB_ERR;
}

std::string ListBoxX::GetValue(int n) {
	const ListItemData item = lti.Get(n);
	return item.text;
}

void ListBoxX::RegisterImage(int type, const char *xpm_data) {
	XPM xpmImage(xpm_data);
	images.AddImage(type, std::make_unique<RGBAImage>(xpmImage));
}

void ListBoxX::RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) {
	images.AddImage(type, std::make_unique<RGBAImage>(width, height, 1.0f, pixelsImage));
}

void ListBoxX::ClearRegisteredImages() {
	images.Clear();
}

void ListBoxX::Draw(DRAWITEMSTRUCT *pDrawItem) {
	if ((pDrawItem->itemAction != ODA_SELECT) && (pDrawItem->itemAction != ODA_DRAWENTIRE)) {
		return;
	}
	if (!graphics.pixmapLine) {
		AllocateBitMap();
		if (!graphics.pixmapLine) {
			// Failed to allocate, so release fully and give up
			graphics.Release();
			return;
		}
	}
#if defined(USE_D2D)
	if (graphics.pBMDCTarget) {
		graphics.pBMDCTarget->BeginDraw();
	}
#endif

	const PRectangle rcItemBase = PRectangleFromRECT(pDrawItem->rcItem);
	const PRectangle rcItem(0, 0, rcItemBase.Width(), rcItemBase.Height());
	PRectangle rcBox = rcItem;
	rcBox.left += TextOffset();
	ColourRGBA colourFore;
	ColourRGBA colourBack;
	if (pDrawItem->itemState & ODS_SELECTED) {
		PRectangle rcImage = rcItem;
		rcImage.right = rcBox.left;
		// The image is not highlighted
		graphics.pixmapLine->FillRectangle(rcImage, ColourElement(options.back, COLOR_WINDOW));
		colourBack = ColourElement(options.backSelected, COLOR_HIGHLIGHT);
		graphics.pixmapLine->FillRectangle(rcBox, colourBack);
		colourFore = ColourElement(options.foreSelected, COLOR_HIGHLIGHTTEXT);
	} else {
		colourBack = ColourElement(options.back, COLOR_WINDOW);
		graphics.pixmapLine->FillRectangle(rcItem, colourBack);
		colourFore = ColourElement(options.fore, COLOR_WINDOWTEXT);
	}


	const ListItemData item = lti.Get(pDrawItem->itemID);
	const int pixId = item.pixId;
	const char *text = item.text;

	const PRectangle rcText = rcBox.Inset(Point(TextInset.x, TextInset.y));

	const double ascent = graphics.pixmapLine->Ascent(fontWin.get());
	graphics.pixmapLine->DrawTextClipped(rcText, fontWin.get(), rcText.top + ascent, text, colourFore, colourBack);

	// Draw the image, if any
	const RGBAImage *pimage = images.Get(pixId);
	if (pimage) {
		const XYPOSITION left = rcItem.left + ItemInset.x + ImageInset.x;
		PRectangle rcImage = rcItem;
		rcImage.left = left;
		rcImage.right = rcImage.left + images.GetWidth();
		graphics.pixmapLine->DrawRGBAImage(rcImage,
			pimage->GetWidth(), pimage->GetHeight(), pimage->Pixels());
	}

#if defined(USE_D2D)
	if (graphics.pBMDCTarget) {
		const HRESULT hrEnd = graphics.pBMDCTarget->EndDraw();
		if (FAILED(hrEnd)) {
			return;
		}
	}
#endif

	// Blit from hMemDC to hDC
	const SIZE extent = SizeOfRect(pDrawItem->rcItem);
	::BitBlt(pDrawItem->hDC, pDrawItem->rcItem.left, pDrawItem->rcItem.top, extent.cx, extent.cy, graphics.bm.DC(), 0, 0, SRCCOPY);
}

void ListBoxX::AppendListItem(const char *text, const char *numword) {
	int pixId = -1;
	if (numword) {
		pixId = 0;
		char ch;
		while ((ch = *++numword) != '\0') {
			pixId = 10 * pixId + (ch - '0');
		}
	}

	lti.AllocItem(text, pixId);
	maxItemCharacters = std::max(maxItemCharacters, static_cast<int>(strlen(text)));
}

void ListBoxX::SetDelegate(IListBoxDelegate *lbDelegate) {
	delegate = lbDelegate;
}

void ListBoxX::SetList(const char *list, char separator, char typesep) {
	// Turn off redraw while populating the list - this has a significant effect, even if
	// the listbox is not visible.
	SetRedraw(false);
	Clear();
	const size_t size = strlen(list);
	char *words = lti.SetWords(list);
	const char *startword = words;
	char *numword = nullptr;
	for (size_t i=0; i < size; i++) {
		if (words[i] == separator) {
			words[i] = '\0';
			if (numword)
				*numword = '\0';
			AppendListItem(startword, numword);
			startword = words + i + 1;
			numword = nullptr;
		} else if (words[i] == typesep) {
			numword = words + i;
		}
	}
	if (startword) {
		if (numword)
			*numword = '\0';
		AppendListItem(startword, numword);
	}

	// Finally populate the listbox itself with the correct number of items
	const int count = lti.Count();
	::SendMessage(lb, LB_INITSTORAGE, count, 0);
	for (intptr_t j=0; j<count; j++) {
		ListBox_AddItemData(lb, j+1);
	}
	SetRedraw(true);
}

void ListBoxX::SetOptions(ListOptions options_) {
	options = options_;
	frameStyle = FlagSet(options.options, AutoCompleteOption::FixedSize) ? WS_BORDER : WS_THICKFRAME;
}

void ListBoxX::AdjustWindowRect(PRectangle *rc, UINT dpiAdjust) const noexcept {
	RECT rcw = RectFromPRectangle(*rc);
	AdjustWindowRectForDpi(&rcw, frameStyle, dpiAdjust);
	*rc = PRectangleFromRECT(rcw);
}

int ListBoxX::ItemHeight() const noexcept {
	int itemHeight = lineHeight + (TextInset.y * 2);
	const int pixHeight = images.GetHeight() + (ImageInset.y * 2);
	if (itemHeight < pixHeight) {
		itemHeight = pixHeight;
	}
	return itemHeight;
}

int ListBoxX::MinClientWidth() const noexcept {
	return 12 * (aveCharWidth+aveCharWidth/3);
}

POINT ListBoxX::MinTrackSize() const noexcept {
	PRectangle rc = PRectangle::FromInts(0, 0, MinClientWidth(), ItemHeight());
	AdjustWindowRect(&rc, dpi);
	POINT ret = {static_cast<LONG>(rc.Width()), static_cast<LONG>(rc.Height())};
	return ret;
}

POINT ListBoxX::MaxTrackSize() const noexcept {
	PRectangle rc = PRectangle::FromInts(0, 0,
		std::max<int>(static_cast<unsigned int>(MinClientWidth()),
		maxCharWidth * maxItemCharacters + TextInset.x * 2 +
		 TextOffset() + SystemMetricsForDpi(SM_CXVSCROLL, dpi)),
		ItemHeight() * lti.Count());
	AdjustWindowRect(&rc, dpi);
	POINT ret = {static_cast<LONG>(rc.Width()), static_cast<LONG>(rc.Height())};
	return ret;
}

void ListBoxX::SetRedraw(bool on) noexcept {
	::SendMessage(lb, WM_SETREDRAW, on, 0);
	if (on) {
		::RedrawWindow(lb, {}, {}, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

void ListBoxX::ResizeToCursor() {
	PRectangle rc = GetPosition();
	POINT ptw;
	::GetCursorPos(&ptw);
	const Point pt = PointFromPOINT(ptw) + dragOffset;

	switch (resizeHit) {
		case HTLEFT:
			rc.left = pt.x;
			break;
		case HTRIGHT:
			rc.right = pt.x;
			break;
		case HTTOP:
			rc.top = pt.y;
			break;
		case HTTOPLEFT:
			rc.top = pt.y;
			rc.left = pt.x;
			break;
		case HTTOPRIGHT:
			rc.top = pt.y;
			rc.right = pt.x;
			break;
		case HTBOTTOM:
			rc.bottom = pt.y;
			break;
		case HTBOTTOMLEFT:
			rc.bottom = pt.y;
			rc.left = pt.x;
			break;
		case HTBOTTOMRIGHT:
			rc.bottom = pt.y;
			rc.right = pt.x;
			break;
		default:
			break;
	}

	const POINT ptMin = MinTrackSize();
	const POINT ptMax = MaxTrackSize();
	// We don't allow the left edge to move at present, but just in case
	rc.left = std::clamp(rc.left, rcPreSize.right - ptMax.x, rcPreSize.right - ptMin.x);
	rc.top = std::clamp(rc.top, rcPreSize.bottom - ptMax.y, rcPreSize.bottom - ptMin.y);
	rc.right = std::clamp(rc.right, rcPreSize.left + ptMin.x, rcPreSize.left + ptMax.x);
	rc.bottom = std::clamp(rc.bottom, rcPreSize.top + ptMin.y, rcPreSize.top + ptMax.y);

	SetPosition(rc);
}

void ListBoxX::StartResize(WPARAM hitCode) {
	rcPreSize = GetPosition();
	POINT cursorPos;
	::GetCursorPos(&cursorPos);

	switch (hitCode) {
		case HTRIGHT:
		case HTBOTTOM:
		case HTBOTTOMRIGHT:
			dragOffset.x = rcPreSize.right - cursorPos.x;
			dragOffset.y = rcPreSize.bottom - cursorPos.y;
			break;

		case HTTOPRIGHT:
			dragOffset.x = rcPreSize.right - cursorPos.x;
			dragOffset.y = rcPreSize.top - cursorPos.y;
			break;

		// Note that the current hit test code prevents the left edge cases ever firing
		// as we don't want the left edge to be movable
		case HTLEFT:
		case HTTOP:
		case HTTOPLEFT:
			dragOffset.x = rcPreSize.left - cursorPos.x;
			dragOffset.y = rcPreSize.top - cursorPos.y;
			break;
		case HTBOTTOMLEFT:
			dragOffset.x = rcPreSize.left - cursorPos.x;
			dragOffset.y = rcPreSize.bottom - cursorPos.y;
			break;

		default:
			return;
	}

	::SetCapture(GetHWND());
	resizeHit = hitCode;
}

LRESULT ListBoxX::NcHitTest(WPARAM wParam, LPARAM lParam) const {
	const PRectangle rc = GetPosition();

	LRESULT hit = ::DefWindowProc(GetHWND(), WM_NCHITTEST, wParam, lParam);
	// There is an apparent bug in the DefWindowProc hit test code whereby it will
	// return HTTOPXXX if the window in question is shorter than the default
	// window caption height + frame, even if one is hovering over the bottom edge of
	// the frame, so workaround that here
	if (hit >= HTTOP && hit <= HTTOPRIGHT) {
		const int minHeight = SystemMetricsForDpi(SM_CYMINTRACK, dpi);
		const int yPos = GET_Y_LPARAM(lParam);
		if ((rc.Height() < minHeight) && (yPos > ((rc.top + rc.bottom)/2))) {
			hit += HTBOTTOM - HTTOP;
		}
	}

	// Never permit resizing that moves the left edge. Allow movement of top or bottom edge
	// depending on whether the list is above or below the caret
	switch (hit) {
		case HTLEFT:
		case HTTOPLEFT:
		case HTBOTTOMLEFT:
			hit = HTERROR;
			break;

		case HTTOP:
		case HTTOPRIGHT: {
				// Valid only if caret below list
				if (location.y < rc.top)
					hit = HTERROR;
			}
			break;

		case HTBOTTOM:
		case HTBOTTOMRIGHT: {
				// Valid only if caret above list
				if (rc.bottom <= location.y)
					hit = HTERROR;
			}
			break;
		default:
			break;
	}

	return hit;
}

void ListBoxX::OnDoubleClick() {
	if (delegate) {
		ListBoxEvent event(ListBoxEvent::EventType::doubleClick);
		delegate->ListNotify(&event);
	}
}

void ListBoxX::OnSelChange() {
	if (delegate) {
		ListBoxEvent event(ListBoxEvent::EventType::selectionChange);
		delegate->ListNotify(&event);
	}
}

void ListBoxX::OnContextMenu() {
	if (delegate) {
		ListBoxEvent event(ListBoxEvent::EventType::contextMenu);
		delegate->ListNotify(&event);
	}
}

POINT ListBoxX::GetClientExtent() const noexcept {
	RECT rc;
	::GetWindowRect(HwndFromWindowID(wid), &rc);
	POINT ret { rc.right - rc.left, rc.bottom - rc.top };
	return ret;
}

void ListBoxX::CentreItem(int n) {
	// If below mid point, scroll up to centre, but with more items below if uneven
	if (n >= 0) {
		const POINT extent = GetClientExtent();
		const int visible = extent.y/ItemHeight();
		if (visible < Length()) {
			const int top = ListBox_GetTopIndex(lb);
			const int half = (visible - 1) / 2;
			if (n > (top + half))
				ListBox_SetTopIndex(lb, n - half);
		}
	}
}

void ListBoxX::AllocateBitMap() {
	const SIZE extent { GetClientExtent().x, lineHeight };

	graphics.bm.Create({}, extent.cx, -extent.cy, nullptr);
	if (!graphics.bm) {
		return;
	}

	// Make a surface
	graphics.pixmapLine = Surface::Allocate(technology);
	graphics.pixmapLine->SetMode(SurfaceMode(codePage, false));

#if defined(USE_D2D)
	if (technology != Technology::Default) {
		if (!LoadD2D()) {
			return;
		}

		const D2D1_RENDER_TARGET_PROPERTIES drtp = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			{ DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED });

		HRESULT hr = CreateDCRenderTarget(&drtp, graphics.pBMDCTarget);
		if (FAILED(hr) || !graphics.pBMDCTarget) {
			return;
		}

		const RECT rcExtent = { 0, 0, extent.cx, extent.cy };
		hr = graphics.pBMDCTarget->BindDC(graphics.bm.DC(), &rcExtent);
		if (SUCCEEDED(hr)) {
			graphics.pixmapLine->Init(graphics.pBMDCTarget.Get(), GetID());
		}
		return;
	}
#endif

	// Either technology == Technology::Default or USE_D2D turned off
	graphics.pixmapLine->Init(graphics.bm.DC(), GetID());
}


LRESULT PASCAL ListBoxX::ListProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	try {
		ListBoxX *lbx = static_cast<ListBoxX *>(PointerFromWindow(::GetParent(hWnd)));
		switch (iMessage) {
		case WM_MOUSEWHEEL:
			return lbx->WndProc(GetParent(hWnd), iMessage, wParam, lParam);
		case WM_ERASEBKGND:
		{
			HBRUSH brush;
			RECT rect;
			brush = CreateSolidBrush(lbx->options.back->OpaqueRGB());
			SelectObject((HDC)wParam, brush);
			GetClientRect(hWnd, &rect) ;
			FillRect((HDC)wParam, &rect, brush);
			DeleteObject(brush);
		}
			return TRUE;

		case WM_MOUSEACTIVATE:
			// This prevents the view activating when the scrollbar is clicked
			return MA_NOACTIVATE;

		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN: {
				// We must take control of selection to prevent the ListBox activating
				// the popup
				const LRESULT lResult = ::SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
				if (HIWORD(lResult) == 0) {
					ListBox_SetCurSel(hWnd, LOWORD(lResult));
					OnSelChange();
					}
				}
			return 0;

		case WM_LBUTTONUP:
			return 0;

		case WM_LBUTTONDBLCLK: {
				OnDoubleClick();
			}
			return 0;
		case WM_RBUTTONUP:{
				OnContextMenu();
			}
		return 0;
		
		case WM_MBUTTONDOWN:
			// disable the scroll wheel button click action
			return 0;

		case WM_VSCROLL:
		case LB_SETCURSEL:
		{
			LRESULT res = ::CallWindowProc(prevWndProc, hWnd, iMessage, wParam, lParam);
			OnScrollChange();
			return res;
		}
		
			break;

		default:
			break;
		}

		if (prevWndProc) {
			return ::CallWindowProc(prevWndProc, hWnd, iMessage, wParam, lParam);
		}
	} catch (...) {
	}
	return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
}

LRESULT PASCAL ListBoxX::ControlWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	if (ListBoxX* lbx = static_cast<ListBoxX*>(PointerFromWindow(::GetParent(hWnd)))) {
		return lbx->ListProc(hWnd, iMessage, wParam, lParam);
	}
	return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
}

LRESULT ListBoxX::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE: {
			HINSTANCE hinstanceParent = GetWindowInstance(HwndFromWindow(*parent));
			// Note that LBS_NOINTEGRALHEIGHT is specified to fix cosmetic issue when resizing the list
			// but has useful side effect of speeding up list population significantly
			lb = ::CreateWindowEx(
				0, TEXT("listbox"), TEXT(""),
				WS_CHILD | WS_VISIBLE | //WS_VSCROLL |
				LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_NOINTEGRALHEIGHT,
				0, 0, 150,80, hWnd,
				reinterpret_cast<HMENU>(static_cast<ptrdiff_t>(ctrlID)),
				hinstanceParent,
				nullptr);
			scb = CreateWindowEx(
				0, TEXT("Scrollbar"), TEXT(""),
				WS_CHILD | WS_VISIBLE | SBS_VERT | SBS_RIGHTALIGN,
				0, 0, 150, 65, hWnd,
				reinterpret_cast<HMENU>(static_cast<ptrdiff_t>(ctrlID)),
				hinstanceParent,
				0); 
			prevWndProc = SubclassWindow(lb, ControlWndProc);
	
			WNDPROC pWP = SubclassWindow(scb, ScrollWndProc);
			::SetWindowLongPtr(scb, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWP));
			
			SetWindowTheme(scb, L"", L"");
		}
		break;

	case WM_SIZE:
		if (lb) {
			graphics.Release();	// Bitmap must be reallocated to new size.
			SetRedraw(false);
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE | SIF_RANGE;
			si.nMin = 0;
			si.nMax = static_cast<int>(ControlWndProc(lb, LB_GETCOUNT, 0, 0)) - 1;
			si.nPage = HIWORD(lParam) / ItemHeight();
			::SetScrollInfo(scb, SB_CTL, &si, false);

			int sWidth = si.nMax > (int)si.nPage ? 15 : 0;
			::SetWindowPos(lb, 0, 0, 0, LOWORD(lParam) - sWidth, HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
			::SetWindowPos(scb, 0, LOWORD(lParam) - sWidth,0, sWidth, HIWORD(lParam), SWP_NOZORDER|SWP_NOACTIVATE );
			// Ensure the selection remains visible
			CentreItem(GetSelection());
			SetRedraw(true);
		}
		break;

	case WM_PAINT: {
			Painter painter(hWnd);
		}
		break;

	case WM_COMMAND:
		// This is not actually needed now - the registered double click action is used
		// directly to action a choice from the list.
		::SendMessage(HwndFromWindow(*parent), iMessage, wParam, lParam);
		break;

	case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *pMeasureItem = static_cast<MEASUREITEMSTRUCT *>(PtrFromLParam(lParam));
			pMeasureItem->itemHeight = ItemHeight();
		}
		break;

	case WM_DRAWITEM:
		Draw(static_cast<DRAWITEMSTRUCT *>(PtrFromLParam(lParam)));
		break;

	case WM_DESTROY:
		lb = {};
		SetWindowPointer(hWnd, nullptr);
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);

	case WM_ERASEBKGND:
	{
		HBRUSH brush;
		RECT rect;
		brush = CreateSolidBrush(options.back->OpaqueRGB());
		SelectObject((HDC)wParam, brush);
		GetClientRect(hWnd, &rect);
		FillRect((HDC)wParam, &rect, brush);
		DeleteObject(brush);
	}
		// To reduce flicker we can elide background erasure since this window is
		// completely covered by its child.
		return TRUE;

	case WM_GETMINMAXINFO: {
			MINMAXINFO *minMax = static_cast<MINMAXINFO*>(PtrFromLParam(lParam));
			minMax->ptMaxTrackSize = MaxTrackSize();
			minMax->ptMinTrackSize = MinTrackSize();
		}
		break;

	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;

	case WM_NCHITTEST:
		return NcHitTest(wParam, lParam);

	case WM_NCLBUTTONDOWN:
		// We have to implement our own window resizing because the DefWindowProc
		// implementation insists on activating the resized window
		StartResize(wParam);
		return 0;

	case WM_MOUSEMOVE: {
			if (resizeHit == 0) {
				return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
			}
			ResizeToCursor();
		}
		break;

	case WM_LBUTTONUP:
	case WM_CANCELMODE:
		if (resizeHit != 0) {
			resizeHit = 0;
			::ReleaseCapture();
		}
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	case WM_MOUSEWHEEL:
		if (wheelDelta.Accumulate(wParam)) {
			const int nRows = GetVisibleRows();
			int linesToScroll = std::clamp(nRows - 1, 1, 3);
			linesToScroll *= wheelDelta.Actions();
			int top = ListBox_GetTopIndex(lb) + linesToScroll;
			if (top < 0) {
				top = 0;
			}
			ListBox_SetTopIndex(lb, top);
			OnScrollChange();
			if (pListcolors->inizialized)
				sc_Paint(false, 0);
		}
		break;
	case WM_NCPAINT:
	{
		if (!pListcolors->inizialized)
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);

		RECT rect, rdraw;
		HDC hdc = ::GetWindowDC(hWnd);

		GetWindowRect(hWnd, &rect);

		HRGN hrgn = CreateRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top);
		::SetWindowRgn(hWnd, hrgn, false);
		DeleteObject(hrgn);
		COLORREF penCl = pListcolors->border;
		HPEN p = ::CreatePen(PS_SOLID, 1, penCl);
		HPEN penOld = static_cast<HPEN>(SelectObject(hdc, p));
		::Rectangle(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
		SelectObject(hdc, penOld);
		DeleteObject(p);

		HBRUSH b = CreateSolidBrush(pListcolors->borderbak);
		int borderX = ::GetSystemMetrics( SM_CXSIZEFRAME) + (frameStyle = WS_BORDER ? ::GetSystemMetrics(SM_CXDLGFRAME) : 0);
		int borderY = ::GetSystemMetrics( SM_CYSIZEFRAME) + (frameStyle = WS_BORDER ? ::GetSystemMetrics(SM_CYDLGFRAME) : 0);

		rdraw = { 1, 1, rect.right - rect.left - 2, borderY };
		FillRect(hdc, &rdraw, b);
		rdraw = { 1, 1, borderX , rect.bottom - rect.top - 2 };
		FillRect(hdc, &rdraw, b);
		rdraw = { rect.right - rect.left - borderX , 1, rect.right - rect.left - 1, rect.bottom - rect.top - 1 };
		FillRect(hdc, &rdraw, b);
		rdraw = { 1, rect.bottom - rect.top - borderY , rect.right - rect.left - 1, rect.bottom - rect.top - 1 };
		FillRect(hdc, &rdraw, b);

		DeleteObject(b);

		ReleaseDC(hWnd, hdc);
		return 0;
		}
	case WM_VSCROLL:
	{
		SHORT pos = HIWORD(wParam);
		SHORT flag = LOWORD(wParam);
		sptr_t r = ControlWndProc(lb, iMessage, wParam, lParam);

		if (!pListcolors->inizialized)
			return r;

		if(bBlockRedraw)
			::DefWindowProcW(scb, WM_SETREDRAW, 1, 0);
		sc_Paint((flag == SB_THUMBPOSITION || flag == SB_THUMBTRACK), pos);
		
		if(bBlockRedraw)
			::DefWindowProcW(scb, WM_SETREDRAW, 0, 0);
		return r;

		}
		break;
	default:
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}

	return 0;
}



LRESULT PASCAL ListBoxX::StaticWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	if (iMessage == WM_CREATE) {
		CREATESTRUCT *pCreate = static_cast<CREATESTRUCT *>(PtrFromLParam(lParam));
		SetWindowPointer(hWnd, pCreate->lpCreateParams);
	}
	// Find C++ object associated with window.

	if (ListBoxX* lbx = static_cast<ListBoxX*>(PointerFromWindow(hWnd))) {
		return lbx->WndProc(hWnd, iMessage, wParam, lParam);
	}
	return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
}

void ListBoxX::OnScrollChange() {
	sptr_t ind = ControlWndProc(lb, LB_GETTOPINDEX, 0, 0);
	::SetScrollPos(scb, SB_CTL, static_cast<int>(ind), !pListcolors->inizialized);
}

void ListBoxX::sc_Paint(bool bDrag, int nTrackPos) {
	HDC hdc = ::GetDC(scb);
	RECT r;
	::GetClientRect(scb, &r);
	sc_OnPaint(hdc, &r, bDrag, nTrackPos);

	ReleaseDC(scb, hdc);
}

bool PointInRect(POINT* p, RECT* r) {
	return (r->left <= p->x) && (p->x <= r->right) && (r->top <= p->y) && (p->y <= r->bottom);
}

void ListBoxX::sc_OnPaint(HDC hDC, RECT* rcPaint, bool bDrag, int nTrackPos) {
	int w = rcPaint->right - rcPaint->left;
	int h = rcPaint->bottom - rcPaint->top;

	HDC hdc = ::CreateCompatibleDC(hDC);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, w, h);
	HBITMAP hBitmapOld = SelectBitmap(hdc, hBitmap);

	HBRUSH b = CreateSolidBrush(pListcolors->scrollbak);
	::FillRect(hdc, rcPaint, b);
	::DeleteObject(b);

	HBRUSH b_fore = CreateSolidBrush(pListcolors->scroll);
	HBRUSH b_hl = CreateSolidBrush(pListcolors->scrollhl);
	HBRUSH b_press = CreateSolidBrush(pListcolors->scrollpress);

	RECT r;
	r.left = rcPaint->left + 2;
	r.right = rcPaint->right - 2;


	int count = static_cast<int>(ControlWndProc(lb, LB_GETCOUNT, 0, 0));
	int hs = h - 2 * w;
	int lenT = hs * (rcPaint->bottom - rcPaint->top) / (count * ItemHeight()); ;
	int hTop = hs * static_cast<int>(ControlWndProc(lb, LB_GETTOPINDEX, 0, 0)) / count;
	r.top = rcPaint->top + w + hTop;
	r.bottom = rcPaint->top + w + hTop + lenT;

	POINT curPos;
	::GetCursorPos(&curPos);
	SHORT bPress = ::GetAsyncKeyState(VK_LBUTTON);

	::MapWindowPoints(NULL, scb, &curPos, 1);

	b = b_fore;
	if (PointInRect(&curPos, &r) || bDrag) {
		b = (bDrag || bPress) ? b_press : b_hl;
	}

	::FillRect(hdc, &r, b);

	int w2 = w / 2;
	POINT line_poly[3] = { {rcPaint->left + 1, rcPaint->top + w2 + 2},
	{ rcPaint->right - 1, rcPaint->top + w2 + 2 },
	{ rcPaint->left + w2, rcPaint->top + 2} };

	r.top = 0;
	r.bottom = w;
	b = b_fore;
	if (PointInRect(&curPos, &r)) {
		b = (bPress) ? b_press : b_hl;
	}

	HBRUSH hBrushOld = (HBRUSH)SelectObject(hdc, b);
	BeginPath(hdc);
	Polygon(hdc, line_poly, 3);
	EndPath(hdc);
	FillPath(hdc);

	POINT line_poly2[3] = { { rcPaint->left + 2, rcPaint->bottom - w2 - 2 },
	{ rcPaint->right - 2, rcPaint->bottom - w2 - 2 },
	{ rcPaint->left + w2, rcPaint->bottom - 3 } };

	r.top = rcPaint->bottom - w;
	r.bottom = rcPaint->bottom;
	b = b_fore;
	if (PointInRect(&curPos, &r)) {
		b = (bPress) ? b_press : b_hl;
	}

	BeginPath(hdc);
	Polygon(hdc, line_poly2, 3);
	EndPath(hdc);
	FillPath(hdc);

	SelectObject(hdc, hBrushOld);
	::DeleteObject(b_fore);
	::DeleteObject(b_hl);
	::DeleteObject(b_press);

	::BitBlt(hDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);

	::SelectObject(hdc, GetStockFont(WHITE_BRUSH));
	SelectBitmap(hdc, hBitmapOld);
	::DeleteObject(hBitmap);
	::DeleteDC(hdc);

}

LRESULT PASCAL ListBoxX::ScrollWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	WNDPROC prevWndProc_Scroll = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	ListBoxX* lbx = static_cast<ListBoxX*>(PointerFromWindow(::GetParent(hWnd)));
	switch (iMessage) {
	case WM_ERASEBKGND:
	return TRUE;
	case WM_PAINT:
	{
		if (!lbx->pListcolors->inizialized)
			return ::CallWindowProc(prevWndProc_Scroll, hWnd, iMessage, wParam, lParam);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		lbx->sc_OnPaint(hdc, &ps.rcPaint, false, 0);

		EndPaint(hWnd, &ps);
		return TRUE;
	}
	break;
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	{
		if (!lbx->pListcolors->inizialized)
			return ::CallWindowProc(prevWndProc_Scroll, hWnd, iMessage, wParam, lParam);
		if (iMessage == WM_MOUSEMOVE && !::GetAsyncKeyState(VK_LBUTTON)) {
			LRESULT res = ::CallWindowProc(prevWndProc_Scroll, hWnd, iMessage, wParam, lParam);
			lbx->sc_Paint(false, 0);
			return res;
		}

		::DefWindowProcW(hWnd, WM_SETREDRAW, 0, 0);
		lbx->bBlockRedraw = true;
		LRESULT res = ::CallWindowProc(prevWndProc_Scroll, hWnd, iMessage, wParam, lParam);
		::DefWindowProcW(hWnd, WM_SETREDRAW, 1, 0);
		lbx->bBlockRedraw = false;
		if (!::GetAsyncKeyState(VK_LBUTTON))
			lbx->sc_Paint(false, 0);

		return res;
	}
	}
	return ::CallWindowProc(prevWndProc_Scroll, hWnd, iMessage, wParam, lParam);
}

namespace Scintilla::Internal {

bool ListBoxX_Register() noexcept {
	WNDCLASSEX wndclassc {};
	wndclassc.cbSize = sizeof(wndclassc);
	// We need CS_HREDRAW and CS_VREDRAW because of the ellipsis that might be drawn for
	// truncated items in the list and the appearance/disappearance of the vertical scroll bar.
	// The list repaint is double-buffered to avoid the flicker this would otherwise cause.
	wndclassc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclassc.cbWndExtra = sizeof(ListBoxX *);
	wndclassc.hInstance = hinstPlatformRes;
	wndclassc.lpfnWndProc = ListBoxX::StaticWndProc;
	wndclassc.hCursor = ::LoadCursor({}, IDC_ARROW);
	wndclassc.lpszClassName = ListBoxX_ClassName;

	return ::RegisterClassEx(&wndclassc) != 0;
}

void ListBoxX_Unregister() noexcept {
	if (hinstPlatformRes) {
		::UnregisterClass(ListBoxX_ClassName, hinstPlatformRes);
	}
}


ListBox::ListBox() noexcept = default;

ListBox::~ListBox() noexcept = default;

}
