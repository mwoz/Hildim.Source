// SciTE - Scintilla based Text Editor
/** @file GUI.h
 ** Interface to platform GUI facilities.
 ** Split off from Scintilla's Platform.h to avoid SciTE depending on implementation of Scintilla.
 ** Implementation in win32/GUIWin.cxx for Windows and gtk/GUIGTK.cxx for GTK+.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef GUI_H
#define GUI_H
#include "Scintilla.h"
#include "ScintillaTypes.h"
#include "ScintillaMessages.h"
#include "ScintillaStructures.h"
#include "ScintillaCall.h"
#include <algorithm>


void lab2rgb(float l_s, float a_s, float b_s, float& R, float& G, float& B);
void rgb2lab(float R, float G, float B, float & l_s, float &a_s, float &b_s);
float clr_brightness(long clr);
namespace GUI {

class Point {
public:
	int x;
	int y;

	explicit Point(int x_=0, int y_=0) : x(x_), y(y_) {
	}
};

class Rectangle {
public:
	int left;
	int top;
	int right;
	int bottom;

	Rectangle(int left_=0, int top_=0, int right_=0, int bottom_ = 0) :
		left(left_), top(top_), right(right_), bottom(bottom_) {
	}
	bool Contains(Point pt) const {
		return (pt.x >= left) && (pt.x <= right) &&
			(pt.y >= top) && (pt.y <= bottom);
	}
	int Width() const { return right - left; }
	int Height() const { return bottom - top; }
	bool operator==(const Rectangle &other) const {
		return (left == other.left) &&
			(top == other.top) &&
			(right == other.right) &&
			(bottom == other.bottom);
	}
};

#if defined(GTK)

// On GTK+ use UTF-8 char strings

typedef char gui_char;
typedef std::string gui_string;

#define GUI_TEXT(q) q

#else

// On Win32 use UTF-16 wide char strings

typedef wchar_t gui_char;
typedef std::wstring gui_string;

#define GUI_TEXT(q) L##q

#endif

//typedef std::basic_string<gui_char> gui_string;

gui_string StringFromUTF8(const char *s);
std::string UTF8FromString(const gui_string &s);
gui_string StringFromInteger(int i);

//!-start-[FixEncoding]
int CodePageFromName(const std::string &encodingName);
unsigned int CodePageFromCharSet(unsigned long characterSet, unsigned int documentCodePage);
std::string ConvertFromUTF8(const std::string &s, int codePage);
std::string ConvertToUTF8(const std::string &s, int codePage);
std::string UTF8ToUpper(const std::string &str);
std::string UTF8ToLower(const std::string &str);
//!-end-[FixEncoding]

typedef void *WindowID;
class Window {
protected:
	WindowID wid;
public:
	Window() : wid(0) {
	}
	Window &operator=(WindowID wid_) {
		wid = wid_;
		return *this;
	}
	WindowID GetID() const {
		return wid;
	}
	void SetID(WindowID wid_) {
		wid = wid_;
	}
	bool Created() const {
		return wid != 0;
	}
	void Destroy();
	bool HasFocus();
	Rectangle GetPosition();
	void SetPosition(Rectangle rc);
	void MoveToLeftTop(Rectangle rc);
	Rectangle GetClientPosition();
	void Show(bool show=true);
	void InvalidateAll();
	void SetTitle(const gui_char *s);
};

typedef void *MenuID;
class Menu {
	MenuID mid;
public:
	Menu() : mid(0) {
	}
	Menu(MenuID _id)  : mid(_id) {} //!-add-[SubMenu]
	MenuID GetID() const {
		return mid;
	}
	void CreatePopUp();
	void Destroy();
	void Show(Point pt, Window &w);
};

class ElapsedTime {
	long bigBit;
	long littleBit;
public:
	ElapsedTime();
	double Duration(bool reset=false);
};

struct ScintillaFailure {
	int status;
	ScintillaFailure(int status_) : status(status_) {
	}
};

class ScintillaWindow : public Window, public Scintilla::ScintillaCall {
	// Private so ScintillaWindow objects can not be copied
	ScintillaWindow(const ScintillaWindow &source);
	//ScintillaWindow &operator=(const ScintillaWindow &);
	SciFnDirect fn;
	sptr_t ptr;
public:
	ScintillaWindow() : fn(0), ptr(0) {
	}

	void SetID(WindowID wid_) {
		wid = wid_;
		fn = 0;
		ptr = 0;
		if (wid) {
			fn = reinterpret_cast<SciFnDirect>(
				Send(SCI_GETDIRECTFUNCTION, 0, 0));
			ptr = Send(SCI_GETDIRECTPOINTER, 0, 0);
			SetFnPtr(reinterpret_cast<Scintilla::FunctionDirect>(fn), ptr);
		}
	}
	bool CanCall() const {
		return wid && fn && ptr;
	}
//!	sptr_t Call(unsigned int msg, uptr_t wParam=0, sptr_t lParam=0) {
#define WM_SETREDRAW                    0x000B
	virtual sptr_t CallSetRedraw() {
		return static_cast<int>(fn(ptr, WM_SETREDRAW, 1, 0));
	}
	sptr_t Send(unsigned int msg, uptr_t wParam=0, sptr_t lParam=0);
	virtual sptr_t SendPointer(unsigned int msg, uptr_t wParam=0, void *lParam=0);

};

bool IsDBCSLeadByte(int codePage, char ch);

}
constexpr const float componentMaximum = 255.0f;
class ColourDesired {
	int co;
public:
	constexpr explicit ColourDesired(int co_ = 0) noexcept : co(co_) {}

	constexpr ColourDesired(unsigned int red, unsigned int green, unsigned int blue) noexcept :
		co(red | (green << 8) | (blue << 16)) {}

	constexpr bool operator==(const ColourDesired &other) const noexcept {
		return co == other.co;
	}

	void Set(long lcol) {  ///!!!TODO! - ���������������� ������������� � �� ����������� �������
		co = lcol;
	}

	void Set(unsigned int red, unsigned int green, unsigned int blue) {
		co = red | (green << 8) | (blue << 16);
	}

	constexpr int AsInteger() const noexcept {
		return co;
	}

	// Red, green and blue values as bytes 0..255
	constexpr unsigned char GetRed() const noexcept {
		return co & 0xff;
	}
	constexpr unsigned char GetGreen() const noexcept {
		return (co >> 8) & 0xff;
	}
	constexpr unsigned char GetBlue() const noexcept {
		return (co >> 16) & 0xff;
	}

	// Red, green and blue values as float 0..1.0
	constexpr float GetRedComponent() const noexcept {
		return GetRed() / componentMaximum;
	}
	constexpr float GetGreenComponent() const noexcept {
		return GetGreen() / componentMaximum;
	}
	constexpr float GetBlueComponent() const noexcept {
		return GetBlue() / componentMaximum;
	}
};
#endif
