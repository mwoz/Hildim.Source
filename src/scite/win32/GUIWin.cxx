// SciTE - Scintilla based Text Editor
/** @file GUIWin.cxx
 ** Interface to platform GUI facilities.
 ** Split off from Scintilla's Platform.h to avoid SciTE depending on implementation of Scintilla.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <time.h>
#include <codecvt>
#include <string>
#include <vector>

#ifdef __MINGW_H
#define _WIN32_IE	0x0400
#endif

#undef _WIN32_WINNT
#define _WIN32_WINNT  0x0500
#ifdef _MSC_VER
// windows.h, et al, use a lot of nameless struct/unions - can't fix it, so allow it
#pragma warning(disable: 4201)
#endif
#include <windows.h>
#ifdef _MSC_VER
// okay, that's done, don't allow it in our code
#pragma warning(default: 4201)
#pragma warning(disable: 4244)
#endif

#include "Scintilla.h"
#include "GUI.h"

namespace GUI {

enum { SURROGATE_LEAD_FIRST = 0xD800 };
enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
enum { SURROGATE_TRAIL_LAST = 0xDFFF };

static size_t UTF8Len(const wchar_t *uptr, size_t tlen) {
	size_t len = 0;
	for (size_t i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			len++;
		} else if (uch < 0x800) {
			len += 2;
		} else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			len += 4;
			i++;
		} else {
			len += 3;
		}
		i++;
	}
	return len;
}

static void UTF8FromUTF16(const wchar_t *uptr, size_t tlen, char *putf, size_t len) {
	int k = 0;
	for (size_t i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			putf[k++] = static_cast<char>(uch);
		} else if (uch < 0x800) {
			putf[k++] = static_cast<char>(0xC0 | (uch >> 6));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		} else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			// Half a surrogate pair
			i++;
			unsigned int xch = 0x10000 + ((uch & 0x3ff) << 10) + (uptr[i] & 0x3ff);
			putf[k++] = static_cast<char>(0xF0 | (xch >> 18));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 12) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (xch & 0x3f));
		} else {
			putf[k++] = static_cast<char>(0xE0 | (uch >> 12));
			putf[k++] = static_cast<char>(0x80 | ((uch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		}
		i++;
	}
	putf[len] = '\0';
}

static size_t UTF16Length(const char *s, size_t len) {
	size_t ulen = 0;
	size_t charLen;
	for (size_t i=0; i<len;) {
		unsigned char ch = static_cast<unsigned char>(s[i]);
		if (ch < 0x80) {
			charLen = 1;
		} else if (ch < 0x80 + 0x40 + 0x20) {
			charLen = 2;
		} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			charLen = 3;
		} else {
			charLen = 4;
			ulen++;
		}
		i += charLen;
		ulen++;
	}
	return ulen;
}

static size_t UTF16FromUTF8(const char *s, size_t len, gui_char *tbuf, size_t tlen) {
	size_t ui=0;
	const unsigned char *us = reinterpret_cast<const unsigned char *>(s);
	size_t i=0;
	while ((i<len) && (ui<tlen)) {
		unsigned char ch = us[i++];
		if (ch < 0x80) {
			tbuf[ui] = ch;
		} else if (ch < 0x80 + 0x40 + 0x20) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0x1F) << 6);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0xF) << 12);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + ((ch & 0x7F) << 6));
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		} else {
			// Outside the BMP so need two surrogates
			int val = (ch & 0x7) << 18;
			ch = us[i++];
			val += (ch & 0x3F) << 12;
			ch = us[i++];
			val += (ch & 0x3F) << 6;
			ch = us[i++];
			val += (ch & 0x3F);
			tbuf[ui] = static_cast<wchar_t>(((val - 0x10000) >> 10) + SURROGATE_LEAD_FIRST);
			ui++;
			tbuf[ui] = static_cast<wchar_t>((val & 0x3ff) + SURROGATE_TRAIL_FIRST);
		}
		ui++;
	}
	return ui;
}

gui_string StringFromUTF8(const char *s) {
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	//return conv.from_bytes(s);

	size_t sLen = s ? strlen(s) : 0;
	size_t wideLen = UTF16Length(s, sLen);
	std::vector<gui_char> vgc(wideLen + 1);
	size_t outLen = UTF16FromUTF8(s, sLen, &vgc[0], wideLen);
	vgc[outLen] = 0;
	return gui_string(&vgc[0], outLen);
}

std::string UTF8FromString(const gui_string &s) {
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	//return conv.to_bytes(s);

	size_t sLen = s.size();
	size_t narrowLen = UTF8Len(s.c_str(), sLen);
	std::vector<char> vc(narrowLen + 1);
	UTF8FromUTF16(s.c_str(), sLen, &vc[0], narrowLen);
	return std::string(&vc[0], narrowLen);
}

//!-start-[EncodingToLua]
int CodePageFromName(const std::string &encodingName) {
	struct Encoding {
		const char *name;
		int codePage;
	} knownEncodings[] = {
		{ "ascii", SC_CP_UTF8 },
		{ "utf-8", SC_CP_UTF8 },
		{ "latin1", 1252 },
		{ "latin2", 28592 },
		{ "big5", 950 },
		{ "gbk", 936 },
		{ "shift_jis", 932 },
		{ "euc-kr", 949 },
		{ "cyrillic", 1251 },
		{ "iso-8859-5", 28595 },
		{ "iso8859-11", 874 },
		{ "1250", 1250 },
		{ "windows-1251", 1251 },
		{ 0, 0 },
	};
	for (Encoding *enc=knownEncodings; enc->name; enc++) {
		if (encodingName == enc->name) {
			return enc->codePage;
		}
	}
	return SC_CP_UTF8;
}

inline wchar_t MyCharUpper(wchar_t c)
{ return (wchar_t)(unsigned int)(UINT_PTR)CharUpperW((LPWSTR)(UINT_PTR)(unsigned int)c); }
inline wchar_t MyCharLower(wchar_t c)
{ return (wchar_t)(unsigned int)(UINT_PTR)CharLowerW((LPWSTR)(UINT_PTR)(unsigned int)c); }

std::string UTF8ToUpper(const std::string &str) {
	gui_string s = StringFromUTF8(str.c_str());
	std::transform(s.begin(), s.end(), s.begin(), MyCharUpper);
	return UTF8FromString(s);
}

std::string UTF8ToLower(const std::string &str) {
	gui_string s = StringFromUTF8(str.c_str());
	std::transform(s.begin(), s.end(), s.begin(), MyCharLower);
	return UTF8FromString(s);
}
//!-end-[EncodingToLua]
//!-start-[FixEncoding]
// from ScintillaWin.cxx
unsigned int CodePageFromCharSet(unsigned long characterSet, unsigned int documentCodePage) {
	CHARSETINFO ci = { 0, 0, { { 0, 0, 0, 0 }, { 0, 0 } } };
	intptr_t cs = characterSet;
	BOOL bci = ::TranslateCharsetInfo(reinterpret_cast<DWORD*>(cs),
	                                  &ci, TCI_SRCCHARSET);

	UINT cp;
	if (bci)
		cp = ci.ciACP;
	else if(characterSet == SC_CHARSET_OEM)
		cp = 866;
	else
		cp = documentCodePage;

	CPINFO cpi;
	if (!::IsValidCodePage(cp) && !::GetCPInfo(cp, &cpi))
		cp = CP_ACP;

	return cp;
}

std::string ConvertFromUTF8(const std::string &s, int codePage){
	if (codePage == CP_UTF8) {
		return s;
	} else {
		GUI::gui_string sWide = GUI::StringFromUTF8(s.c_str());
		int sz = static_cast<int>(sWide.length());
		int cchMulti = ::WideCharToMultiByte(codePage, 0, sWide.c_str(), sz, NULL, 0, NULL, NULL);
		char *pszMulti = new char[cchMulti + 1];
		::WideCharToMultiByte(codePage, 0, sWide.c_str(), sz, pszMulti, cchMulti + 1, NULL, NULL);
		pszMulti[cchMulti] = 0;
		std::string ret(pszMulti);
		delete []pszMulti;
		return ret;
	}
}

std::string ConvertToUTF8(const std::string &s, int codePage){
	if (codePage == CP_UTF8) {
		return s;
	} else {
		const char* original = s.c_str();
		int cchWide = ::MultiByteToWideChar(codePage, 0, original, -1, NULL, 0);
		wchar_t *pszWide = new wchar_t[cchWide + 1];
		::MultiByteToWideChar(codePage, 0, original, -1, pszWide, cchWide + 1);
		GUI::gui_string sWide(pszWide);
		std::string ret = GUI::UTF8FromString(sWide);
		delete []pszWide;
		return ret;
	}
}
//!-end-[FixEncoding]

gui_string StringFromInteger(int i) {
	gui_char number[32];
#if defined(_MSC_VER) && (_MSC_VER > 1310)
	swprintf(number, 30, L"%0d", i);
#else
	swprintf(number, L"%0d", i);
#endif
	return gui_string(number);
}

void Window::Destroy() {
	if (wid)
		::DestroyWindow(reinterpret_cast<HWND>(wid));
	wid = 0;
}

bool Window::HasFocus() {
	return ::GetFocus() == wid;
}

Rectangle Window::GetPosition() {
	RECT rc;
	::GetWindowRect(reinterpret_cast<HWND>(wid), &rc);
	return Rectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::SetPosition(Rectangle rc) {
	::SetWindowPos(reinterpret_cast<HWND>(wid),
		0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
}
void Window::MoveToLeftTop(Rectangle rc) {
//����������� ���� � ������� ����������� ��������������
	::SetWindowPos(reinterpret_cast<HWND>(wid),
		0, rc.left, rc.top, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
}

Rectangle Window::GetClientPosition() {
	RECT rc={0,0,0,0};
	if (wid)
		::GetClientRect(reinterpret_cast<HWND>(wid), &rc);
	return  Rectangle(rc.left, rc.top, rc.right, rc.bottom);
}


void Window::Show(bool show) {
	if (show)
		::ShowWindow(reinterpret_cast<HWND>(wid), SW_SHOWNOACTIVATE);
	else
		::ShowWindow(reinterpret_cast<HWND>(wid), SW_HIDE);
}

void Window::InvalidateAll() {
	::InvalidateRect(reinterpret_cast<HWND>(wid), NULL, FALSE);
}

void Window::SetTitle(const gui_char *s) {
	::SetWindowTextW(reinterpret_cast<HWND>(wid), s);
}

void Menu::CreatePopUp() {
	Destroy();
	mid = ::CreatePopupMenu();
}

void Menu::Destroy() {
	if (mid)
		::DestroyMenu(reinterpret_cast<HMENU>(mid));
	mid = 0;
}

void Menu::Show(Point pt, Window &w) {
	::TrackPopupMenu(reinterpret_cast<HMENU>(mid),
		0, pt.x - 4, pt.y, 0,
		reinterpret_cast<HWND>(w.GetID()), NULL);
	Destroy();
}

static bool initialisedET = false;
static bool usePerformanceCounter = false;
static LARGE_INTEGER frequency;

ElapsedTime::ElapsedTime() {
	if (!initialisedET) {
		usePerformanceCounter = ::QueryPerformanceFrequency(&frequency) != 0;
		initialisedET = true;
	}
	if (usePerformanceCounter) {
		LARGE_INTEGER timeVal;
		::QueryPerformanceCounter(&timeVal);
		bigBit = timeVal.HighPart;
		littleBit = timeVal.LowPart;
	} else {
		bigBit = clock();
	}
}

double ElapsedTime::Duration(bool reset) {
	double result;
	long endBigBit;
	long endLittleBit;

	if (usePerformanceCounter) {
		LARGE_INTEGER lEnd;
		::QueryPerformanceCounter(&lEnd);
		endBigBit = lEnd.HighPart;
		endLittleBit = lEnd.LowPart;
		LARGE_INTEGER lBegin;
		lBegin.HighPart = bigBit;
		lBegin.LowPart = littleBit;
		double elapsed = lEnd.QuadPart - lBegin.QuadPart;
		result = elapsed / static_cast<double>(frequency.QuadPart);
	} else {
		endBigBit = clock();
		endLittleBit = 0;
		double elapsed = endBigBit - bigBit;
		result = elapsed / CLOCKS_PER_SEC;
	}
	if (reset) {
		bigBit = endBigBit;
		littleBit = endLittleBit;
	}
	return result;
}

sptr_t ScintillaWindow::Send(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	return ::SendMessage(reinterpret_cast<HWND>(GetID()), msg, wParam, lParam);
}

sptr_t ScintillaWindow::SendPointer(unsigned int msg, uptr_t wParam, void *lParam) {
	return ::SendMessage(reinterpret_cast<HWND>(GetID()), msg, wParam, reinterpret_cast<LPARAM>(lParam));
}

bool IsDBCSLeadByte(int codePage, char ch) {
	if (SC_CP_UTF8 == codePage)
		// For lexing, all characters >= 0x80 are treated the
		// same so none is considered a lead byte.
		return false;
	else
		return ::IsDBCSLeadByteEx(codePage, ch) != 0;
}
//intptr_t ScintillaWindow::CallPointer(unsigned int msg, uintptr_t wParam, void *s) {
//	return Call(msg, wParam, reinterpret_cast<intptr_t>(s));
//}
//std::string ScintillaWindow::CallReturnString(unsigned int msg, uintptr_t wParam) {
//	size_t len = CallPointer(msg, wParam, nullptr);
//	if (len) {
//		std::string value(len, '\0');
//		CallPointer(msg, wParam, value.data());
//		return value;
//	} else {
//		return std::string();
//	}
//}
}
