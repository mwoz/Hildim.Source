// SciTE - Scintilla based Text Editor
/** @file SciTEWin.cxx
 ** Main code for the Windows version of the editor.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <time.h>

//#include "Platform.h" //!-add-[no_wornings]
#include "SciTEWin.h"
#include "Windowsx.h"
#include "versionhelpers.h"

#ifdef DTBG_CLIPRECT
#define THEME_AVAILABLE
#endif

// Since Vsstyle.h and Vssym32.h are not available from all compilers just define the used symbols
#define CBS_NORMAL 1
#define CBS_HOT 2
#define CBS_PUSHED 3
#define WP_SMALLCLOSEBUTTON 19
#define TS_NORMAL 1
#define TS_HOT 2
#define TS_PRESSED 3
#define TS_CHECKED 5
#define TS_HOTCHECKED 6
#define TP_BUTTON 1

#ifndef WM_UPDATEUISTATE
#define WM_UPDATEUISTATE 0x0128
#endif

#ifndef UISF_HIDEACCEL
#define UISF_HIDEACCEL 2
#define UISF_HIDEFOCUS 1
#define UIS_CLEAR 2
#define UIS_SET 1
#endif

#include "LuaExtension.h"
#include <sstream>
#include "../../iup/src/win/iupwin_drv.h"

const GUI::gui_char appName[] = GUI_TEXT("HildiM");

int bIsPopUpMenuItem = 0;
HHOOK macroHook = NULL;
HHOOK mouseHook = NULL;
HHOOK keyBoardHook = NULL;
HMENU topLevelPopUp = NULL;
bool kbdHookOn = false;
bool bAltReleased = false;

static GUI::gui_string GetErrorMessage(DWORD nRet) {
	LPWSTR lpMsgBuf = NULL;
	::FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    nRet,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   // Default language
	    reinterpret_cast<LPWSTR>(&lpMsgBuf),
	    0,
	    NULL
	);
	GUI::gui_string s= lpMsgBuf;
	::LocalFree(lpMsgBuf);
	return s;
}

//!-start-[GetApplicationProps]
SciTEBase *SciTEBase::GetApplicationInstance() {
	return SciTEWin::app;
}
//!-end-[GetApplicationProps]

int SciTEKeys::GetVK(SString sKey){
	int keyval = -1;
	if (sKey.length() == 1) {
		keyval = VkKeyScan(sKey[0]);
		if (keyval == -1)
		{
			if (sKey[0] >= 'A' &&  sKey[0] <= 'Z')
				keyval = sKey[0];
		}
		else
			keyval &= 0xFF;
	}
	else if (sKey.length() > 1) {
		if ((sKey[0] == 'F') && (isdigit(sKey[1]))) {
			sKey.remove("F");
			int fkeyNum = sKey.value();
			if (fkeyNum >= 1 && fkeyNum <= 12)
				keyval = fkeyNum - 1 + VK_F1;
		}
		else if ((sKey[0] == 'V') && (isdigit(sKey[1]))) {
			sKey.remove("V");
			int vkey = sKey.value();
			if (vkey > 0 && vkey <= 0x7FFF)
				keyval = vkey;
		}
		else if (sKey.search("Keypad") == 0) {
			sKey.remove("Keypad");
			if (isdigit(sKey[0])) {
				int keyNum = sKey.value();
				if (keyNum >= 0 && keyNum <= 9)
					keyval = keyNum + VK_NUMPAD0;
			}
			else if (sKey == "Plus") {
				keyval = VK_ADD;
			}
			else if (sKey == "Minus") {
				keyval = VK_SUBTRACT;
			}
			else if (sKey == "Decimal") {
				keyval = VK_DECIMAL;
			}
			else if (sKey == "Divide") {
				keyval = VK_DIVIDE;
			}
			else if (sKey == "Multiply") {
				keyval = VK_MULTIPLY;
			}
		}
		else if (sKey == "Left") {
			keyval = VK_LEFT;
		}
		else if (sKey == "Right") {
			keyval = VK_RIGHT;
		}
		else if (sKey == "Up") {
			keyval = VK_UP;
		}
		else if (sKey == "Down") {
			keyval = VK_DOWN;
		}
		else if (sKey == "Insert") {
			keyval = VK_INSERT;
		}
		else if (sKey == "End") {
			keyval = VK_END;
		}
		else if (sKey == "Home") {
			keyval = VK_HOME;
		}
		else if (sKey == "Enter") {
			keyval = VK_RETURN;
		}
		else if (sKey == "Space") {
			keyval = VK_SPACE;
		}
		else if (sKey == "Tab") {
			keyval = VK_TAB;
		}
		else if (sKey == "Escape") {
			keyval = VK_ESCAPE;
		}
		else if (sKey == "Delete") {
			keyval = VK_DELETE;
		}
		else if (sKey == "PageUp") {
			keyval = VK_PRIOR;
		}
		else if (sKey == "PageDown") {
			keyval = VK_NEXT;
		}
		else if (sKey == "Win") {
			keyval = VK_LWIN;
		}
		else if (sKey == "Menu") {
			keyval = VK_APPS;
		}
	}
	return keyval;
}

void SciTEKeys::FillAccel(void *p, const char *mnemonic, int cmd){
	LPACCEL pAcc = reinterpret_cast<LPACCEL>(p);
	pAcc->fVirt = 3;
	if (mnemonic && *mnemonic) {
		SString sKey = mnemonic;

		if (sKey.contains("Ctrl+")) {
			pAcc->fVirt |= 0x08;
			sKey.remove("Ctrl+");
		}
		if (sKey.contains("Shift+")) {
			pAcc->fVirt |= 0x04;
			sKey.remove("Shift+");
		}
		if (sKey.contains("Alt+")) {
			pAcc->fVirt |= 0x10;
			sKey.remove("Alt+");
		}
		pAcc->key = GetVK(sKey);
		pAcc->cmd = cmd;
	}
}

long SciTEKeys::ParseKeyCode(const char *mnemonic) {
	int modsInKey = 0;
	int keyval = -1;

	if (mnemonic && *mnemonic) {
		SString sKey = mnemonic;

		if (sKey.contains("Ctrl+")) {
			modsInKey |= SCMOD_CTRL;
			sKey.remove("Ctrl+");
		}
		if (sKey.contains("Shift+")) {
			modsInKey |= SCMOD_SHIFT;
			sKey.remove("Shift+");
		}
		if (sKey.contains("Alt+")) {
			modsInKey |= SCMOD_ALT;
			sKey.remove("Alt+");
		}
		keyval = GetVK(sKey);
	}

	return (keyval > 0) ? (keyval | (modsInKey<<16)) : 0;
}
long SciTEKeys::ParseKeyCodeWin(const char *mnemonic){
	int modsInKey = 0;
	int keyval = -1;

	if (mnemonic && *mnemonic) {
		SString sKey = mnemonic;

		if (sKey.contains("Ctrl+")) {
			modsInKey |= MOD_CONTROL;
			sKey.remove("Ctrl+");
		}
		if (sKey.contains("Shift+")) {
			modsInKey |= MOD_SHIFT;
			sKey.remove("Shift+");
		}
		if (sKey.contains("Alt+")) {
			modsInKey |= MOD_ALT;
			sKey.remove("Alt+");
		}
		if (sKey.contains("Win+")) {
			modsInKey |= MOD_WIN;
			sKey.remove("Win+");
		}
		keyval = GetVK(sKey);
	}

	return (keyval > 0) ? (keyval | (modsInKey << 16)) : 0;
}

bool SciTEKeys::MatchKeyCode(long parsedKeyCode, int keyval, int modifiers) {
	// TODO: are the 0x11 and 0x10 special cases needed, or are they
	// just short-circuits?  If not needed, this test could removed.
	if (keyval == 0x11 || keyval == 0x10)
		return false;
	return parsedKeyCode && !(0xFFFF0000 & (keyval | modifiers)) && (parsedKeyCode == (keyval | (modifiers<<16)));
}

HINSTANCE SciTEWin::hInstance = 0;
const TCHAR *SciTEWin::className = NULL;
const TCHAR *SciTEWin::classNameInternal = NULL;
SciTEWin *SciTEWin::app = NULL;
SciTEWin *pSciTEWin;

LRESULT CALLBACK KeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	bAltReleased = (wParam == VK_MENU && lParam < 0 && !kbdHookOn);
	if (wParam == VK_MENU && lParam < 0)
		pSciTEWin->NotifyMouseHook(nCode, WM_KEYDOWN, -1);
	else {
		if (kbdHookOn) {
			if (!(bIsPopUpMenuItem & 2) && nCode && wParam == VK_LEFT && (::GetAsyncKeyState(VK_LEFT) < 0)) {
				pSciTEWin->NotifyMouseHook(nCode, WM_KEYUP, -1);
			} else if (!(bIsPopUpMenuItem & 1) && nCode && wParam == VK_RIGHT && (::GetAsyncKeyState(VK_RIGHT) < 0)) {
				pSciTEWin->NotifyMouseHook(nCode, WM_KEYUP, 1);
			}
		}
	}

	return CallNextHookEx(keyBoardHook, nCode, wParam, lParam);
}

SciTEWin::SciTEWin(Extension *ext, GUI::gui_string &propsExt) : SciTEBase(ext) {
	app = this;
	pSciTEWin = this;

	cmdShow = 0;
	heightBar = 7;
	fontTabs = 0;
	wFocus = 0;
	wActive = NULL;

	winPlace.length = 0;

	openWhat[0] = '\0';
	modalParameters = false;
	filterDefault = 1;
	menuSource = 0;

	hWriteSubProcess = NULL;
	subProcessGroupId = 0;
	outputScroll = 1;

	// Read properties resource into propsEmbed
	// The embedded properties file is to allow distributions to be sure
	// that they have a sensible default configuration even if the properties
	// files are missing. Also allows a one file distribution of Sc1.EXE.
	propsEmbed.Clear();
	// System type properties are also stored in the embedded properties.
	propsEmbed.Set("PLAT_WIN", "1");
	propsEmbed.Set("PLAT_WINNT", "1");

	HRSRC handProps = ::FindResource(hInstance, TEXT("Embedded"), TEXT("Properties"));
	if (handProps) {
		DWORD size = ::SizeofResource(hInstance, handProps);
		HGLOBAL hmem = ::LoadResource(hInstance, handProps);
		if (hmem) {
			const void *pv = ::LockResource(hmem);
			if (pv) {
				propsEmbed.ReadFromMemory(
				    reinterpret_cast<const char *>(pv), size, FilePath());
			}
		}
		::FreeResource(handProps);
	}

	pathAbbreviations = GetAbbrevPropertiesFileName();

	ReadGlobalPropFile(propsExt);
	/// Need to copy properties to variables before setting up window
	SetPropertiesInitial();

	hDevMode = 0;
	hDevNames = 0;
	::ZeroMemory(&pagesetupMargin, sizeof(pagesetupMargin));

	hHH = 0;
	hMM = 0;
	uniqueInstance.Init(this);

	hAccTable = ::LoadAccelerators(hInstance, TEXT("ACCELS")); // md

	hToolbarBitmap = 0; //!-add-[user.toolbar]
	oldToolbarBitmapID = 0; //!-add-[user.toolbar]

}

SciTEWin::~SciTEWin() {
	if (hDevMode)
		::GlobalFree(hDevMode);
	if (hDevNames)
		::GlobalFree(hDevNames);
	if (hHH)
		::FreeLibrary(hHH);
	if (hMM)
		::FreeLibrary(hMM);
	if (fontTabs)
		::DeleteObject(fontTabs);
}

uptr_t SciTEWin::GetInstance() {
	return reinterpret_cast<uptr_t>(hInstance);
}

void SciTEWin::Register(HINSTANCE hInstance_) {
	const TCHAR resourceName[] = TEXT("SciTE");

	hInstance = hInstance_;

	WNDCLASS wndclass;
	// Register the frame window
	className = TEXT("HildiMWindow");
//!	wndclass.style = 0;
	wndclass.style = CS_DBLCLKS;	//!-change-[new_on_dbl_clk]
	wndclass.lpfnWndProc = SciTEWin::TWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(SciTEWin*);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = ::LoadIcon(hInstance, resourceName);
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = className;
	if (!::RegisterClass(&wndclass))
		exit(FALSE);

	// Register the window that holds the two Scintilla edit windows and the separator
	classNameInternal = TEXT("SciTEWindowContent");
	wndclass.lpfnWndProc = BaseWin::StWndProc;
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = classNameInternal;
	if (!::RegisterClass(&wndclass))
		exit(FALSE);
}

void SciTEWin::GetWindowPosition(int *left, int *top, int *width, int *height, int *maximize) {
	winPlace.length = sizeof(winPlace);
	::GetWindowPlacement(MainHWND(), &winPlace);

	*left = winPlace.rcNormalPosition.left;
	*top = winPlace.rcNormalPosition.top;
	*width =  winPlace.rcNormalPosition.right - winPlace.rcNormalPosition.left;
	*height = winPlace.rcNormalPosition.bottom - winPlace.rcNormalPosition.top;
	*maximize = (winPlace.showCmd == SW_MAXIMIZE) ? 1 : 0;
}

// Allow UTF-8 file names and command lines to be used in calls to io.open and io.popen in Lua scripts.
// The scite_lua_win.h header redirects fopen and _popen to these functions.

extern "C" {

FILE *scite_lua_fopen(const char *filename, const char *mode) {
	GUI::gui_string sFilename = GUI::StringFromUTF8(filename);
	GUI::gui_string sMode = GUI::StringFromUTF8(mode);
	FILE *f = _wfopen(sFilename.c_str(), sMode.c_str());
	if (f == NULL)
		// Fallback on narrow string in case already in CP_ACP
		f = fopen(filename, mode);
	return f;
}

FILE *scite_lua_popen(const char *filename, const char *mode) {
	GUI::gui_string sFilename = GUI::StringFromUTF8(filename);
	GUI::gui_string sMode = GUI::StringFromUTF8(mode);
	FILE *f = _wpopen(sFilename.c_str(), sMode.c_str());
	if (f == NULL)
		// Fallback on narrow string in case already in CP_ACP
		f = _popen(filename, mode);
	return f;
}

}

void SciTEWin::ReadProperties() {
	SciTEBase::ReadProperties();

	outputScroll = props.GetInt("output.scroll", 1);
}

static FilePath GetSciTEPath(FilePath home) {
	if (home.IsSet()) {
		return FilePath(home);
	} else {
		GUI::gui_char path[MAX_PATH];
		::GetModuleFileNameW(0, path, ELEMENTS(path));
		// Remove the SciTE.exe
		GUI::gui_char *lastSlash = wcsrchr(path, pathSepChar);
		if (lastSlash)
			*lastSlash = '\0';
		return FilePath(path);
	}
}

FilePath SciTEWin::GetDefaultDirectory() {
	GUI::gui_char *home = _wgetenv(GUI_TEXT("SciTE_HOME"));
	return GetSciTEPath(home);
}

FilePath SciTEWin::GetSciteDefaultHome() {
	GUI::gui_char *home = _wgetenv(GUI_TEXT("SciTE_HOME"));
	return GetSciTEPath(home);
}

FilePath SciTEWin::GetSciteUserHome() {
	GUI::gui_char *home = _wgetenv(GUI_TEXT("SciTE_HOME"));
//!-start-[scite.userhome]
	GUI::gui_string userhome;
	if (!home) {
		userhome = GUI::StringFromUTF8(props.GetExpanded("scite.userhome").c_str());
		if (userhome.length())
			home = const_cast<GUI::gui_char*>(userhome.c_str());
	}
//!-end-[scite.userhome]
	if (!home)
		home = _wgetenv(GUI_TEXT("USERPROFILE"));
	return GetSciTEPath(home);
}

// Help command lines contain topic!path
void SciTEWin::ExecuteOtherHelp(const char *cmd) {
	GUI::gui_string s = GUI::StringFromUTF8(cmd);
	size_t pos = s.find_first_of('!');
	if (pos != GUI::gui_string::npos) {
		GUI::gui_string topic = s.substr(0, pos);
		GUI::gui_string path = s.substr(pos+1);
		::WinHelpW(MainHWND(),
			path.c_str(),
			HELP_KEY,
			reinterpret_cast<ULONG_PTR>(topic.c_str()));
 	}
}

// HH_AKLINK not in mingw headers
struct XHH_AKLINK {
	long cbStruct;
	BOOL fReserved;
	const wchar_t *pszKeywords;
	wchar_t *pszUrl;
	wchar_t *pszMsgText;
	wchar_t *pszMsgTitle;
	wchar_t *pszWindow;
	BOOL fIndexOnFail;
};

// Help command lines contain topic!path
void SciTEWin::ExecuteHelp(const char *cmd, int hh_cmd) {
	if (!hHH)
		hHH = ::LoadLibrary(TEXT("HHCTRL.OCX"));

	if (hHH) {
		GUI::gui_string s = GUI::StringFromUTF8(cmd);
		typedef HWND(WINAPI *HelpFn) (HWND, const wchar_t *, UINT, DWORD_PTR);
		HelpFn fnHHW = (HelpFn)::GetProcAddress(hHH, "HtmlHelpW");
		if (fnHHW) {
			switch (hh_cmd){
			case 0x000d:
			{
				size_t pos = s.find_first_of('!');
				if (pos != GUI::gui_string::npos) {
					GUI::gui_string topic = s.substr(0, pos);
					GUI::gui_string path = s.substr(pos + 1);
					XHH_AKLINK ak;
					ak.cbStruct = sizeof(ak);
					ak.fReserved = FALSE;
					ak.pszKeywords = topic.c_str();
					ak.pszUrl = NULL;
					ak.pszMsgText = NULL;
					ak.pszMsgTitle = NULL;
					ak.pszWindow = NULL;
					ak.fIndexOnFail = TRUE;
					fnHHW(NULL,
						path.c_str(),
						0x000d,          	// HH_KEYWORD_LOOKUP
						reinterpret_cast<DWORD_PTR>(&ak)
						);
				}
			}
				return;
			}
			fnHHW(NULL,
				s.c_str(),
				hh_cmd,
				NULL
				);
		}
	}
}

void SciTEWin::CopyWithColors(CopyColorsType clrType){
	const Scintilla::Sci_CharacterRange cr = GetSelection();

	bool bOpen = false;

	char* text = new char[(cr.cpMax - cr.cpMin + 1)];

	size_t len;
	HGLOBAL hand;
	Sci_TextRange tr;
	tr.chrg.cpMin = cr.cpMin;
	tr.chrg.cpMax = cr.cpMax;
	tr.lpstrText = text;

	wEditor.GetTextRange(&tr);

	GUI::gui_string uText;
	const void* t;

	if (clrType != CopyColorsType::htmlText) {
		len = strlen(text);
		if (codePage) {
			uText = GUI::StringFromUTF8(text);
			len = uText.length() * 2 + 2;
			t = uText.c_str();
		}
		else {
			t = text;
			len = strlen(text) + 1;

		}
		hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len);
		if (hand) {
			if (!bOpen) {
				::OpenClipboard(MainHWND());
				::EmptyClipboard();
			}
			bOpen = true;
			char* ptr = static_cast<char*>(::GlobalLock(hand));
			if (ptr) {
				memcpy(ptr, t, len);
				::GlobalUnlock(hand);
			}
			::SetClipboardData(codePage ? CF_UNICODETEXT : CF_TEXT, hand);

		}
	}
	std::ostringstream oss; 
	switch (clrType) {
	case CopyColorsType::rtf:
		SaveToStreamRTF(oss, cr.cpMin, cr.cpMax);
		break;
	case CopyColorsType::html:
		SaveToStreamHTML(oss, cr.cpMin, cr.cpMax);
		break;
	case CopyColorsType::htmlText:
		SaveToStreamHTMLText(oss, cr.cpMin, cr.cpMax);
		break;
	}

	std::string rtf = oss.str(); 
	len = rtf.length() + 1;	// +1 for NUL
	if (clrType == CopyColorsType::html) {
		size_t l = len - 2;
		if (l > 0) {
			std::string num = std::to_string(l);   //"EndHTML:0000000000\n"
			size_t p1 = rtf.find("EndHTML:") + 8;
			
			//str3_Iter = rtf.erase( (p1, (size_t)num.length());
			rtf = rtf.erase(p1, num.length());
			rtf = rtf.insert(p1 + 10 - num.length(), num);

		}
		l = rtf.find("<html>");
		if (l > 0) {
			std::string num = std::to_string(l);   //"EndHTML:0000000000\n"
			size_t p1 = rtf.find("StartHTML:") + 10;

			//str3_Iter = rtf.erase( (p1, (size_t)num.length());
			rtf = rtf.erase(p1, num.length());
			rtf = rtf.insert(p1 + 10 - num.length(), num);
		}
		l = rtf.find("<!--StartFragment-->") + 20;
		if (l > 0) {
			std::string num = std::to_string(l);   //"EndHTML:0000000000\n"
			size_t p1 = rtf.find("StartFragment:") + 14;

			//str3_Iter = rtf.erase( (p1, (size_t)num.length());
			rtf = rtf.erase(p1, num.length());
			rtf = rtf.insert(p1 + 10 - num.length(), num);
		}
		l = rtf.find("<!--EndFragment-->");
		if (l > 0) {
			std::string num = std::to_string(l);   //"EndHTML:0000000000\n"
			size_t p1 = rtf.find("EndFragment:") + 12;

			//str3_Iter = rtf.erase( (p1, (size_t)num.length());
			rtf = rtf.erase(p1, num.length());
			rtf = rtf.insert(p1 + 10 - num.length(), num);
		}
	}
	else if (codePage && clrType == CopyColorsType::htmlText) {
		rtf = GUI::ConvertFromUTF8(rtf, CP_ACP);
	}

	hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len);
	if (hand) {
		if (!bOpen) {
			::OpenClipboard(MainHWND());
			::EmptyClipboard();
		}
		bOpen = true;
		char *ptr = static_cast<char *>(::GlobalLock(hand));
		if (ptr) {
			memcpy(ptr, rtf.c_str(), len);
			::GlobalUnlock(hand);
		}
		switch (clrType) {
		case CopyColorsType::rtf:
			::SetClipboardData(::RegisterClipboardFormat(CF_RTF), hand);
			break;
		case CopyColorsType::html:
			::SetClipboardData(::RegisterClipboardFormat(L"HTML Format"), hand);
			break;
		case CopyColorsType::htmlText:
			::SetClipboardData(CF_TEXT, hand);
			break;
		}
		hand = GlobalAlloc(GMEM_DDESHARE, sizeof(LCID));
		LCID* lcid = (LCID*)GlobalLock(hand);
		*lcid = GetSystemDefaultLCID();
		GlobalUnlock(hand);
		SetClipboardData(CF_LOCALE, hand);
	}

	if (bOpen)
		::CloseClipboard();
	delete[] text;
}
void SciTEWin::CopyAsHTMLText() {
	CopyWithColors(CopyColorsType::htmlText);
}
void SciTEWin::CopyAsHTML() {
	CopyWithColors(CopyColorsType::html);
}
void SciTEWin::CopyAsRTF() {
	CopyWithColors(CopyColorsType::rtf);
}

void SciTEWin::MakeOutputVisible(GUI::ScintillaWindow &wBottom)
{
	if (wFindRes.GetID() == wBottom.GetID()){
		extender->OnLayOutNotify("SHOW_FINDRES");
	}
	else if (wOutput.GetID() == wBottom.GetID()){
		extender->OnLayOutNotify("SHOW_OUTPUT");
	}
}

HWND SciTEWin::MainHWND() {
	return HwndOf(wSciTE);
}

void SciTEWin::Command(WPARAM wParam, LPARAM lParam) {
	int cmdID = ControlIDOfCommand(static_cast<unsigned long>(wParam));
	if (wParam & 0x10000) {
		// From accelerator -> goes to focused pane.
		menuSource = 0;
	}

	switch (cmdID) {
	case IDM_SRCWIN:
	case IDM_RUNWIN:
	case IDM_COSRCWIN:
	case IDM_FINDRESWIN:
		if (HIWORD(wParam) == SCEN_SETFOCUS) {
			wFocus = reinterpret_cast<HWND>(lParam);
			CheckMenus();
			extender->OnChangeFocus(cmdID, 1);
		}
		if (HIWORD(wParam) == SCEN_KILLFOCUS) {
			CheckMenus();
			extender->OnChangeFocus(cmdID, 0);
		}
		break;

	case IDM_ACTIVATE:
		Activate(lParam);
		break;

	case IDM_FINISHEDEXECUTE: {
			jobQueue.SetExecuting(false);
			//if (needReadProperties) !!TODO - видимо переменную needReadProperties можно удалить
			//	ReadProperties();
			CheckMenus();
			for (int icmd = 0; icmd < jobQueue.commandMax; icmd++) {
				jobQueue.jobQueue[icmd].Clear();
			}
			jobQueue.commandCurrent = 0;
			CheckReload();
		}
		break;

	case IDM_ONTOP:
		topMost = (topMost ? false : true);
		::SetWindowPos(MainHWND(), (topMost ? HWND_TOPMOST : HWND_NOTOPMOST ), 0, 0, 0, 0, SWP_NOMOVE + SWP_NOSIZE);
		break;

	case IDM_OPENFILESHERE:
		uniqueInstance.ToggleOpenFilesHere();
		break;

	case IDC_TABCLOSE:
		CloseTab((int)lParam);
		break;

//!-start-[close_on_dbl_clk]
	case IDC_TABDBLCLK:
		if (props.GetInt("tabbar.tab.close.on.doubleclick") > 0) {
			CloseTab((int)lParam);
		}
		break;
//!-end-[close_on_dbl_clk]

	case IDC_SHIFTTAB:
		ShiftTab(LOWORD(lParam), HIWORD(lParam));
		break;

	default:
		SciTEBase::MenuCommand(cmdID, menuSource);
	}
}

void SciTEWin::OutputAppendEncodedStringSynchronised(GUI::gui_string s, int codePage) {
	int sz = static_cast<int>(s.size());
	int cchMulti = ::WideCharToMultiByte(codePage, 0, s.c_str(), sz, NULL, 0, NULL, NULL);
	char *pszMulti = new char[cchMulti + 1];
	::WideCharToMultiByte(codePage, 0, s.c_str(), sz, pszMulti, cchMulti + 1, NULL, NULL);
	pszMulti[cchMulti] = 0;
	OutputAppendStringSynchronised(pszMulti);
	delete []pszMulti;
}

/**
 * Run a command with redirected input and output streams
 * so the output can be put in a window.
 * It is based upon several usenet posts and a knowledge base article.
 * This is running in a separate thread to the user interface so should always
 * use ScintillaWindow::Send rather than a one of the direct function calls.
 */
DWORD SciTEWin::ExecuteOne(const Job &jobToRun, bool &seenOutput) {
	DWORD exitcode = 0;
	GUI::ElapsedTime commandTime;

	if (jobToRun.jobType == jobShell) {
		ShellExec(jobToRun.command, jobToRun.directory.AsUTF8().c_str());
		return exitcode;
	}

	if (jobToRun.jobType == jobExtension) {
		if (extender) {
			// Problem: we are in the wrong thread!  That is the cause of the cursed PC.
			// It could also lead to other problems.

			if (jobToRun.flags & jobGroupUndo)
				wEditor.BeginUndoAction();

			extender->OnExecute(jobToRun.command.c_str());

			if (jobToRun.flags & jobGroupUndo)
				wEditor.EndUndoAction();

			Redraw();
			// A Redraw "might" be needed, since Lua and Director
			// provide enough low-level capabilities to corrupt the
			// display.
			// (That might have been due to a race condition, and might now be
			// corrected by SingleThreadExtension.  Should check it some time.)
		}
		return exitcode;
	}

	if (jobToRun.jobType == jobHelp) {
		ExecuteHelp(jobToRun.command.c_str());
		return exitcode;
	}

	if (jobToRun.jobType == jobOtherHelp) {
		ExecuteOtherHelp(jobToRun.command.c_str());
		return exitcode;
	}

	if (jobToRun.jobType == jobGrep) {
		// jobToRun.command is "(w|~)(c|~)(d|~)(b|r|~)(s|~)(g|~)(p|~)\0files\0text"
		const char *grepCmd = jobToRun.command.c_str();
		GrepFlags gf = grepNone;
		if (*grepCmd == 'w')
			gf = static_cast<GrepFlags>(gf | grepWholeWord);
		grepCmd++;
		if (*grepCmd == 'c')
			gf = static_cast<GrepFlags>(gf | grepMatchCase);
		grepCmd++;
		if ((*grepCmd == 'd'))
			gf = static_cast<GrepFlags>(gf | grepDot);
		grepCmd++;
		if (*grepCmd == 'b')
			gf = static_cast<GrepFlags>(gf | grepBinary);
		if (*grepCmd == 'r')
			gf = static_cast<GrepFlags>(gf | grepRegExp);
		grepCmd++;
		if (*grepCmd == 's')
			gf = static_cast<GrepFlags>(gf | grepSubDir);
		grepCmd++;
		if (*grepCmd == 'g')
			gf = static_cast<GrepFlags>(gf | grepGroup);
		grepCmd++;
		if (*grepCmd == 'p')
			gf = static_cast<GrepFlags>(gf | grepProgress);
		grepCmd++;
		const char *findFiles = grepCmd + 1;
		const char *findWhat = findFiles + strlen(findFiles) + 1;
		InternalGrep(gf, jobToRun.directory.AsInternal(), GUI::StringFromUTF8(findFiles).c_str(), findWhat);
		return exitcode;
	}

	UINT codePage = static_cast<UINT>(wOutput.Send(SCI_GETCODEPAGE));
	if (codePage != SC_CP_UTF8) {
//!		codePage = CodePageFromCharSet(characterSet, codePage);
		codePage = GUI::CodePageFromCharSet(characterSet, codePage); //!-change-[FixEncoding]
	}

	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), 0, 0};
	char buffer[16384];
	OutputAppendStringSynchronised(">");
	OutputAppendEncodedStringSynchronised(GUI::StringFromUTF8(jobToRun.command.c_str()), codePage);
	OutputAppendStringSynchronised("\n");

	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	SECURITY_DESCRIPTOR sd;
	// Make a real security thing to allow inheriting handles
	::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	::SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;

	HANDLE hPipeWrite = NULL;
	HANDLE hPipeRead = NULL;
	// Create pipe for output redirection
	// read handle, write handle, security attributes,  number of bytes reserved for pipe - 0 default
	::CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0);

	// Create pipe for input redirection. In this code, you do not
	// redirect the output of the child process, but you need a handle
	// to set the hStdInput field in the STARTUP_INFO struct. For safety,
	// you should not set the handles to an invalid handle.

	hWriteSubProcess = NULL;
	subProcessGroupId = 0;
	HANDLE hRead2 = NULL;
	// read handle, write handle, security attributes,  number of bytes reserved for pipe - 0 default
	::CreatePipe(&hRead2, &hWriteSubProcess, &sa, 0);

	::SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);
	::SetHandleInformation(hWriteSubProcess, HANDLE_FLAG_INHERIT, 0);

	// Make child process use hPipeWrite as standard out, and make
	// sure it does not show on screen.
	STARTUPINFOW si = {
			     sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			 };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	if (jobToRun.jobType == jobCLI)
		si.wShowWindow = SW_HIDE;
	else
		si.wShowWindow = SW_SHOW;
	si.hStdInput = hRead2;
	si.hStdOutput = hPipeWrite;
	si.hStdError = hPipeWrite;

	FilePath startDirectory = jobToRun.directory.AbsolutePath();

	PROCESS_INFORMATION pi = {0, 0, 0, 0};

	bool running = ::CreateProcessW(
			  NULL,
			  const_cast<wchar_t *>(GUI::StringFromUTF8(jobToRun.command.c_str()).c_str()),
			  NULL, NULL,
			  TRUE, CREATE_NEW_PROCESS_GROUP,
			  NULL,
			  startDirectory.IsSet() ?
			  startDirectory.AsInternal() : NULL,
			  &si, &pi);

	// if jobCLI "System cant find" - try calling with command processor
	if ((!running) && (jobToRun.jobType == jobCLI) && (::GetLastError() == ERROR_FILE_NOT_FOUND)) {

		SString runComLine = "cmd.exe /c ";
		runComLine = runComLine.append(jobToRun.command.c_str());

		running = ::CreateProcessW(
			  NULL,
			  const_cast<wchar_t*>(GUI::StringFromUTF8(runComLine.c_str()).c_str()),
			  NULL, NULL,
			  TRUE, CREATE_NEW_PROCESS_GROUP,
			  NULL,
			  startDirectory.IsSet() ?
			  startDirectory.AsInternal() : NULL,
			  &si, &pi);
	}

	if (running) {
		subProcessGroupId = pi.dwProcessId;

		bool cancelled = false;

		SString repSelBuf;

		size_t totalBytesToWrite = 0;
		if (jobToRun.flags & jobHasInput) {
			totalBytesToWrite = jobToRun.input.length();
		}

		if (totalBytesToWrite > 0 && !(jobToRun.flags & jobQuiet)) {
			SString input = jobToRun.input;
			input.substitute("\n", "\n>> ");

			OutputAppendStringSynchronised(">> ");
			OutputAppendStringSynchronised(input.c_str());
			OutputAppendStringSynchronised("\n");
		}
//!-start-[InputErr]
		if (totalBytesToWrite <= 0)
		{
			if (jobToRun.flags & jobHasInput){
				totalBytesToWrite = 1;
				if(!(jobToRun.flags & jobQuiet)) {
					OutputAppendStringSynchronised("\x4");
				}
			}
		}
//!-end-[InputErr]

		unsigned writingPosition = 0;

		while (running) {
			if (writingPosition >= totalBytesToWrite) {
				::Sleep(100L);
			}

			DWORD bytesRead = 0;
			DWORD bytesAvail = 0;

			if (!::PeekNamedPipe(hPipeRead, buffer,
					     sizeof(buffer), &bytesRead, &bytesAvail, NULL)) {
				bytesAvail = 0;
			}

			if ((bytesAvail < 1000) && (hWriteSubProcess != INVALID_HANDLE_VALUE) && (writingPosition < totalBytesToWrite)) {
				// There is input to transmit to the process.  Do it in small blocks, interleaved
				// with reads, so that our hRead buffer will not be overrun with results.

				size_t bytesToWrite;
				intptr_t eol_pos = jobToRun.input.search("\n", writingPosition);
				if (eol_pos == -1) {
					bytesToWrite = totalBytesToWrite - writingPosition;
				} else {
					bytesToWrite = eol_pos + 1 - writingPosition;
				}
				if (bytesToWrite > 250) {
					bytesToWrite = 250;
				}

				DWORD bytesWrote = 0;

				int bTest = ::WriteFile(hWriteSubProcess,
					    const_cast<char *>(jobToRun.input.c_str() + writingPosition),
					static_cast<DWORD>(bytesToWrite), &bytesWrote, NULL);

				if (bTest) {
					if ((writingPosition + bytesToWrite) / 1024 > writingPosition / 1024) {
						// sleep occasionally, even when writing
						::Sleep(100L);
					}

					writingPosition += bytesWrote;

					if (writingPosition >= totalBytesToWrite) {
						::CloseHandle(hWriteSubProcess);
						hWriteSubProcess = INVALID_HANDLE_VALUE;
					}

				} else {
					// Is this the right thing to do when writing to the pipe fails?
					::CloseHandle(hWriteSubProcess);
					hWriteSubProcess = INVALID_HANDLE_VALUE;
					OutputAppendStringSynchronised("\n>Input pipe closed due to write failure.\n");
				}

			} else if (bytesAvail > 0) {
				int bTest = ::ReadFile(hPipeRead, buffer,
						       sizeof(buffer), &bytesRead, NULL);

				if (bTest && bytesRead) {

					if (jobToRun.flags & jobRepSelMask) {
						repSelBuf.append(buffer, bytesRead);
					}

					if (!(jobToRun.flags & jobQuiet)) {
						if (!seenOutput) {
							MakeOutputVisible(wOutput);
							seenOutput = true;
						}
//!-start-[oem2ansi]
						// Convert OEM output to ANSI
						if (props.GetInt("output.code.page.oem2ansi")) {
							if (props.GetInt("character.set") == 204) {
								::OemToCharBuffA(buffer, buffer, bytesRead);
							}
						}
//!-end-[oem2ansi]
						// Display the data
						//buffer[bytesRead] = 0;
						char *tmpbuff = new char[bytesRead+ 1];
						StrCpyNA(tmpbuff, buffer, bytesRead);
						tmpbuff[bytesRead] = 0;
						//Поскольку мы находимся в отдельном потоке, напрямую нельзя вызывать OnSendEditor
						//Пошлем сообщение. Если скрипт вернет ненулевое значение, не будем ничего писать в консоль
						if (!::SendMessage(MainHWND(), SCITE_NOTIYCMD, bytesRead, (WPARAM)(tmpbuff))){
							OutputAppendStringSynchronised(tmpbuff, bytesRead);
						}
						delete tmpbuff;
					}

					::UpdateWindow(MainHWND());
				} else {
					running = false;
				}
			} else {
				if (::GetExitCodeProcess(pi.hProcess, &exitcode)) {
					if (STILL_ACTIVE != exitcode) {
						// Already dead
						running = false;
					}
				}
			}

			if (jobQueue.SetCancelFlag(0)) {
				if (WAIT_OBJECT_0 != ::WaitForSingleObject(pi.hProcess, 500)) {
					// We should use it only if the GUI process is stuck and
					// don't answer to a normal termination command.
					// This function is dangerous: dependant DLLs don't know the process
					// is terminated, and memory isn't released.
					OutputAppendStringSynchronised("\n>Process failed to respond; forcing abrupt termination...\n");
					::TerminateProcess(pi.hProcess, 1);
				}
				running = false;
				cancelled = true;
			}
		}

		if (WAIT_OBJECT_0 != ::WaitForSingleObject(pi.hProcess, 1000)) {
			OutputAppendStringSynchronised("\n>Process failed to respond; forcing abrupt termination...");
			::TerminateProcess(pi.hProcess, 2);
		}
		::GetExitCodeProcess(pi.hProcess, &exitcode);
		SString sExitMessage(exitcode);
		sExitMessage.insert(0, ">Exit code: ");
		if (jobQueue.TimeCommands()) {
			sExitMessage += "    Time: ";
			sExitMessage += SString(commandTime.Duration(), 3);
		}
		sExitMessage += "\n";
		OutputAppendStringSynchronised(sExitMessage.c_str());

		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);

		if (!cancelled) {
			bool doRepSel = false;
			if (jobToRun.flags & jobRepSelYes)
				doRepSel = true;
			else if (jobToRun.flags & jobRepSelAuto)
				doRepSel = (0 == exitcode);

			if (doRepSel) {
				Sci_Position cpMin = wEditor.SelectionStart();
				wEditor.ReplaceSel(repSelBuf.c_str());
				wEditor.SetSel(cpMin, cpMin + repSelBuf.length());
			}
		}

		WarnUser(warnExecuteOK);

	} else {
		DWORD nRet = ::GetLastError();
		OutputAppendStringSynchronised(">");
		OutputAppendEncodedStringSynchronised(GetErrorMessage(nRet), codePage);
		WarnUser(warnExecuteKO);
	}
	::CloseHandle(hPipeRead);
	::CloseHandle(hPipeWrite);
	::CloseHandle(hRead2);
	::CloseHandle(hWriteSubProcess);
	hWriteSubProcess = NULL;
	subProcessGroupId = 0;
	return exitcode;
}

/**
 * Run the commands in the job queue, stopping if one fails.
 * This is running in a separate thread to the user interface so must be
 * careful when reading and writing shared state.
 */
void SciTEWin::ProcessExecute() {
	DWORD exitcode = 0;
	if (scrollOutput)
		wOutput.Send(SCI_GOTOPOS, wOutput.Send(SCI_GETTEXTLENGTH));
	Sci_Position originalEnd = wOutput.Send(SCI_GETCURRENTPOS);
	bool seenOutput = false;

	JobSubsystem initJobType = jobQueue.jobQueue[0].jobType;
	for (int icmd = 0; icmd < jobQueue.commandCurrent && icmd < jobQueue.commandMax && exitcode == 0; icmd++) {
		exitcode = ExecuteOne(jobQueue.jobQueue[icmd], seenOutput);
		if (jobQueue.isBuilding) {
			// The build command is first command in a sequence so it is only built if
			// that command succeeds not if a second returns after document is modified.
			jobQueue.isBuilding = false;
			if (exitcode == 0)
				jobQueue.isBuilt = true;
		}
	}

	// Move selection back to beginning of this run so that F4 will go
	// to first error of this run.
	// scroll and return only if output.scroll equals
	// one in the properties file
	if ((outputScroll == 1) && returnOutputToCommand && initJobType != jobGrep)
		wOutput.Send(SCI_GOTOPOS, originalEnd, 0);
	returnOutputToCommand = true;
	//::SendMessage(MainHWND(), WM_COMMAND, IDM_FINISHEDEXECUTE, 0);
	::PostMessage(MainHWND(), WM_COMMAND, IDM_FINISHEDEXECUTE, 0);
	//ПОСЫЛАЕМ нотификацию в главное окно, чтобы она смогла вызвать эвент у скрипта. по этому эвенту можно будет создать новый джоб
	//(выполнитькомманду меню и пр) - изнутри этого потока этого деолать НЕЛЬЗЯ!
	if (initJobType == jobCLI) ::PostMessage(MainHWND(), SCITE_NOTIFYCMDEXIT, exitcode, 0);//Уведомим скрипт, что команда окончилась
}

void ExecThread(void *ptw) {
	SciTEWin *tw = reinterpret_cast<SciTEWin *>(ptw);
	tw->ProcessExecute();
}

void SciTEWin::ShellExec(const SString &cmd, const char *dir) {
	char *mycmd;

	// guess if cmd is an executable, if this succeeds it can
	// contain spaces without enclosing it with "
	char *mycmdcopy = StringDup(cmd.c_str());
	_strlwr(mycmdcopy);

	char *mycmd_end = NULL;
	char *myparams = NULL;

	char *s = strstr(mycmdcopy, ".exe");
	if (s == NULL)
		s = strstr(mycmdcopy, ".cmd");
	if (s == NULL)
		s = strstr(mycmdcopy, ".bat");
	if (s == NULL)
		s = strstr(mycmdcopy, ".com");
	if ((s != NULL) && ((*(s + 4) == '\0') || (*(s + 4) == ' '))) {
		size_t len_mycmd = s - mycmdcopy + 4;
		delete []mycmdcopy;
		mycmdcopy = StringDup(cmd.c_str());
		mycmd = mycmdcopy;
		mycmd_end = mycmdcopy + len_mycmd;
	} else {
		delete []mycmdcopy;
		mycmdcopy = StringDup(cmd.c_str());
		if (*mycmdcopy != '"') {
			// get next space to separate cmd and parameters
			mycmd_end = strchr(mycmdcopy, ' ');
			mycmd = mycmdcopy;
		} else {
			// the cmd is surrounded by ", so it can contain spaces, but we must
			// strip the " for ShellExec
			mycmd = mycmdcopy + 1;
			char *s = strchr(mycmdcopy + 1, '"');
			if (s != NULL) {
				*s = '\0';
				mycmd_end = s + 1;
			}
		}
	}

	if ((mycmd_end != NULL) && (*mycmd_end != '\0')) {
		*mycmd_end = '\0';
		// test for remaining params after cmd, they may be surrounded by " but
		// we give them as-is to ShellExec
		++mycmd_end;
		while (*mycmd_end == ' ')
			++mycmd_end;

		if (*mycmd_end != '\0')
			myparams = mycmd_end;
	}

	GUI::gui_string sMycmd = GUI::StringFromUTF8(mycmd);
	GUI::gui_string sMyparams = GUI::StringFromUTF8(myparams);
	GUI::gui_string sDir = GUI::StringFromUTF8(dir);

	SHELLEXECUTEINFO exec= { sizeof (exec), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	exec.fMask= SEE_MASK_FLAG_NO_UI; // own msg box on return
	exec.hwnd= MainHWND();
	exec.lpVerb= L"open";  // better for executables to use "open" instead of NULL
	exec.lpFile= sMycmd.c_str();   // file to open
	exec.lpParameters= sMyparams.c_str(); // parameters
	exec.lpDirectory= sDir.c_str(); // launch directory
	exec.nShow= SW_SHOWNORMAL; //default show cmd

	if (::ShellExecuteEx(&exec)) {
		// it worked!
		delete []mycmdcopy;
		return;
	}
	DWORD rc = GetLastError();

	SString errormsg("Error while launching:\n\"");
	errormsg += mycmdcopy;
	if (myparams != NULL) {
		errormsg += "\" with Params:\n\"";
		errormsg += myparams;
	}
	errormsg += "\"\n";
	GUI::gui_string sErrorMsg = GUI::StringFromUTF8(errormsg.c_str()) + GetErrorMessage(rc);
	WindowMessageBox(wSciTE, sErrorMsg, MB_OK);

	delete []mycmdcopy;
}

void SciTEWin::Execute() {
	SciTEBase::Execute();

	_beginthread(ExecThread, 1024 * 1024, reinterpret_cast<void *>(this));
}

void SciTEWin::StopExecute() {
	if (hWriteSubProcess && (hWriteSubProcess != INVALID_HANDLE_VALUE)) {
		char stop[] = "\032";
		DWORD bytesWrote = 0;
		::WriteFile(hWriteSubProcess, stop, static_cast<DWORD>(strlen(stop)), &bytesWrote, NULL);
		Sleep(500L);
	}

#ifdef USE_CONSOLE_EVENT
	if (subProcessGroupId) {
		// this also doesn't work
		OutputAppendStringSynchronised("\n>Attempting to cancel process...");

		if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, subProcessGroupId)) {
			LONG errCode = GetLastError();
			OutputAppendStringSynchronised("\n>BREAK Failed ");
			OutputAppendStringSynchronised(SString(errCode).c_str());
			OutputAppendStringSynchronised("\n");
		}
		Sleep(100L);
	}
#endif

	jobQueue.SetCancelFlag(1);
}

void SciTEWin::AddCommand(const SString &cmd, const SString &dir, JobSubsystem jobType, const SString &input, int flags) {
	if (cmd.length()) {
		if ((jobType == jobShell) && ((flags & jobForceQueue) == 0)) {
			SString pCmd = cmd;
			parameterisedCommand = "";
			pCmd = props.Expand(pCmd.c_str());
			ShellExec(pCmd, dir.c_str());
		} else {
			SciTEBase::AddCommand(cmd, dir, jobType, input, flags);
		}
	}
}

void SciTEWin::QuitProgram() {
	::PostQuitMessage(0);
	ChangeClipboardChain(MainHWND(), hNextCBWnd);
	wSciTE.Destroy();
}

void SciTEWin::CreateUI() {
	CreateBuffers();

	int left = props.GetInt("position.left", CW_USEDEFAULT);
	int top = props.GetInt("position.top", CW_USEDEFAULT);
	int width = props.GetInt("position.width", CW_USEDEFAULT);
	int height = props.GetInt("position.height", CW_USEDEFAULT);
	cmdShow = props.GetInt("position.maximize", 0) ? SW_MAXIMIZE : SW_NORMAL;
	if (width == -1 || height == -1) {
		cmdShow = SW_MAXIMIZE;
		width = CW_USEDEFAULT;
		height = CW_USEDEFAULT;
	}

	if (props.GetInt("position.tile") && ::FindWindow(TEXT("HildiMWindow"), NULL) &&
	        (left != static_cast<int>(CW_USEDEFAULT))) {
		left += width;
	}
	UIAvailable();
	extender->Initialise(this);

	// Pass 'this' pointer in lpParam of CreateWindow().
	wSciTE = ::CreateWindowEx(
	             0,
	             className,
		windowName.c_str(),
		WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
		WS_CLIPCHILDREN,
	             left, top, width, height,
	             NULL,
	             NULL,
	             hInstance,
	             reinterpret_cast<LPSTR>(this));
	if (!wSciTE.Created())
		exit(FALSE);

	if(!IsWindows8OrGreater())
	    SetWindowLongPtr(MainHWND(), GWL_STYLE, WS_SYSMENU | WS_THICKFRAME |
	    	WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
	    	WS_CLIPCHILDREN);
	::SetWindowPos(MainHWND(), NULL, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);


	jobQueue.hwnd = (HWND)wSciTE.GetID();
	IupSetGlobal("DEFAULTNATUVEPARENT", (const char *)wSciTE.GetID());

	SString pageSetup = props.Get("print.margins");
	char val[32];
	char *ps = StringDup(pageSetup.c_str());
	const char *next = GetNextPropItem(ps, val, 32);
	pagesetupMargin.left = atol(val);
	next = GetNextPropItem(next, val, 32);
	pagesetupMargin.right = atol(val);
	next = GetNextPropItem(next, val, 32);
	pagesetupMargin.top = atol(val);
	GetNextPropItem(next, val, 32);
	pagesetupMargin.bottom = atol(val);
	delete []ps;
	BOOL b = FALSE;
					 	
	extender->PostInit((void*)layout.hMain);

}

static bool IsASpace(int ch) {
	return (ch == ' ') || (ch == '\t');
}


/**
 * Process the command line, check for other instance wanting to open files,
 * create the SciTE window, perform batch processing (print) or transmit command line
 * to other instance and exit or just show the window and open files.
 */
void SciTEWin::Run(const GUI::gui_string &cmdLine, bool allowDouble) {

	// No need to check for other instances when doing a batch job:
	// perform some tasks and exit immediately.
	if (props.GetInt("check.if.already.open") != 0 && !allowDouble) {
		uniqueInstance.CheckOtherInstance();
		
		//При нажатом CTRL всегда разрешаем открывать новый инстанс - иногда открывается и при нажаотм почему-то - отменили...&& !::GetAsyncKeyState(VK_CONTROL)
		if (uniqueInstance.FindOtherInstance()) {
			uniqueInstance.SendCommands(GUI::UTF8FromString(cmdLine).c_str());

			// Kill itself, leaving room to the previous instance
			::PostQuitMessage(0);
			wSciTE.Destroy();
			return;	// Don't do anything else
		}
	}

	props.Set("hildim.command.line", GUI::UTF8FromString(cmdLine).c_str());

	CreateUI();

	cfColumnSelect = ::RegisterClipboardFormat(TEXT("MSDEVColumnSelect"));		
	hNextCBWnd = ::SetClipboardViewer(MainHWND());
	//Redraw();
}

/**
 * Draw the split bar.
 */

void SciTEWin::EnsureVisible(){
	if (cmdShow) {	// assume SW_MAXIMIZE only
		if (cmdShow != SW_MAXIMIZE) {
			RECT rcw;
			MONITORINFO mi;
			::GetWindowRect(MainHWND(), &rcw);
			HMONITOR hMonitor = MonitorFromRect(&rcw, MONITOR_DEFAULTTONEAREST);
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			if (rcw.right < mi.rcMonitor.left || rcw.left > mi.rcMonitor.right || rcw.top < mi.rcMonitor.top || rcw.top > mi.rcMonitor.bottom) {
				rcw.left = mi.rcMonitor.left + 20;
				rcw.right = mi.rcMonitor.right - 20;
				rcw.top = mi.rcMonitor.top + 20;
				rcw.bottom = mi.rcMonitor.bottom - 20;
				::MoveWindow(MainHWND(), rcw.left, rcw.top, rcw.right - rcw.left, rcw.bottom - rcw.top, false);
			}
		}
		::ShowWindow(MainHWND(), cmdShow);	
		cmdShow = 0;	
		Redraw();	 
	}
	macro1stLoaded = true;
}
void KillThreadProc(void *p) {
	::Sleep(60000);
	::TerminateProcess(GetCurrentProcess(), 2);
}

void SciTEWin::HideForeReolad(int close){
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	::GetWindowPlacement(MainHWND(), &wp);
	cmdShow = wp.showCmd;
	::ShowWindow(MainHWND(), SW_HIDE);

}

void SciTEWin::RunAsync(int idx)	{
	PostMessage((HWND)GetID(), SCI_POSTCALBACK, idx, 0);
}

void SciTEWin::SetRestart(const char* cmdLine) {
	restartCmdLine = cmdLine;
}

/**
 * Open files dropped on the SciTE window.
 */
void SciTEWin::DropFiles(HDROP hdrop) {
	// If drag'n'drop inside the SciTE window but outside
	// Scintilla, hdrop is null, and an exception is generated!
	if (hdrop) {
		int filesDropped = ::DragQueryFile(hdrop, 0xffffffff, NULL, 0);
		// Append paths to dropFilesQueue, to finish drag operation soon
		for (int i = 0; i < filesDropped; ++i) {
			GUI::gui_char pathDropped[MAX_PATH];
			::DragQueryFileW(hdrop, i, pathDropped, ELEMENTS(pathDropped));
			dropFilesQueue.push_back(pathDropped);
		}
		::DragFinish(hdrop);
		// Put SciTE to forefront
		// May not work for Win2k, but OK for lower versions
		// Note: how to drop a file to an iconic window?
		// Actually, it is the Send To command that generates a drop.
		if (::IsIconic(MainHWND())) {
			::ShowWindow(MainHWND(), SW_RESTORE);
		}
		::SetForegroundWindow(MainHWND());
		// Post message to ourself for opening the files so we can finish the drop message and
		// the drop source will respond when open operation takes long time (opening big files...)
		if (filesDropped > 0) {
			::PostMessage(MainHWND(), SCITE_DROP, 0, 0);
		}
	}
}

/**
 * Handle simple wild-card file patterns and directory requests.
 */

GUI::Rectangle SciTEWin::GetClientRectangle() {
	RECT rc = { 0, 0, 0, 0 };
	::GetClientRect(MainHWND(), &rc);
	return  GUI::Rectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void SciTEWin::MinimizeToTray() {
	TCHAR n[64] = TEXT("SciTE");
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = MainHWND();
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = SCITE_TRAY;
	nid.hIcon = static_cast<HICON>(
	                ::LoadImage(hInstance, TEXT("SCITE"), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE));
	lstrcpy(nid.szTip, n);
	::ShowWindow(MainHWND(), SW_MINIMIZE);
	if (::Shell_NotifyIcon(NIM_ADD, &nid)) {
		::ShowWindow(MainHWND(), SW_HIDE);
	}
}

void SciTEWin::RestoreFromTray() {
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = MainHWND();
	nid.uID = 1;
	::ShowWindow(MainHWND(), SW_SHOW);
	::Sleep(100);
	::Shell_NotifyIcon(NIM_DELETE, &nid);
}

#ifndef VK_OEM_2
static const int VK_OEM_2=0xbf;
static const int VK_OEM_3=0xc0;
static const int VK_OEM_4=0xdb;
static const int VK_OEM_5=0xdc;
static const int VK_OEM_6=0xdd;
#endif
#ifndef VK_OEM_PLUS
static const int VK_OEM_PLUS=0xbb;
#endif

inline bool KeyMatch(const SString &sKey, int keyval, int modifiers) {
	return SciTEKeys::MatchKeyCode(
		SciTEKeys::ParseKeyCode(sKey.c_str()), keyval, modifiers);
}

LRESULT SciTEWin::KeyDown(WPARAM wParam) {
	// Look through lexer menu
	int modifiers =
	    (IsKeyDown(VK_SHIFT) ? SCMOD_SHIFT : 0) |
	    (IsKeyDown(VK_CONTROL) ? SCMOD_CTRL : 0) |
	    (IsKeyDown(VK_MENU) ? SCMOD_ALT : 0);

/*!
	if (extender && extender->OnKey(wParam, modifiers))
		return 1l;
*/
//!-start-[OnKey]
	if ( extender ) {
		char ch[4] = { 0 };

		static bool sPrevIsDeadKey = false;

		bool bIsDeadKey = ::MapVirtualKeyA(static_cast<UINT>(wParam), 2 ) & 0x80008000;
		if ( bIsDeadKey ) {
			sPrevIsDeadKey == true ? sPrevIsDeadKey = false : sPrevIsDeadKey = true;
		}
		else {
			if ( sPrevIsDeadKey == false ) {
				UINT uFlags = 1;
				if ( IsKeyDown( VK_MENU ) && wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9 ) {
					uFlags = 0;
				}

				unsigned char masKS[256] = { 0 }; 

 				if (::GetKeyboardState(masKS) && ::ToAscii(static_cast<UINT>(wParam), ::MapVirtualKeyA(static_cast<UINT>(wParam), 0 ), masKS, (LPWORD)ch, uFlags ) != 1 ) {
 					ch[0] = 0;
				}
 			}
			sPrevIsDeadKey = false;
		}

		if ( extender->OnKey( (int)wParam, modifiers, ch[0] ) ) {
			return 1l;	  
		}
	}

	return 0l;
}

LRESULT SciTEWin::KeyUp(WPARAM wParam) {
	if (wParam == VK_CONTROL) {
		EndStackedTabbing();
		extender->OnDwellStart(0, "", false);
	}
	return 0l;
}

LRESULT SciTEWin::ContextMenuMessage(UINT iMessage, WPARAM wParam, LPARAM lParam, bool fromMargin) {
	GUI::ScintillaWindow *w = &wEditor;
	if (wOutput.Focus()) w = &wOutput;
	else if (wFindRes.Focus()) w = &wFindRes;
	if ((WPARAM)w->GetID() != wParam) return 0;		//Мы можем сюда попасть, когда закрывается без выбора другое меню иуп - отсекаем этот вариант
	GUI::Point pt = PointFromLong(static_cast<long>(lParam));
	if ((pt.x == -1) && (pt.y == -1)) {
		// Caused by keyboard so display menu near caret
		if (wOutput.HasFocus())
			w = &wOutput;
		else if (wFindRes.HasFocus())
			w = &wFindRes;
		Sci_Position position = w->CurrentPos();
		pt.x = static_cast<int>(w->PointXFromPosition(position));
		pt.y = static_cast<int>(w->PointYFromPosition(position) + w->TextHeight(w->LineFromPosition(position)));
		POINT spt = {pt.x, pt.y};
		::ClientToScreen(static_cast<HWND>(w->GetID()), &spt);
		pt = GUI::Point(spt.x, spt.y);
	}

	menuSource = ::GetDlgCtrlID(HwndOf(*w));
	ContextMenu(*w, pt, wSciTE, fromMargin);
	return 0;
}


LRESULT SciTEWin::WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) {
	int statusFailure = 0;
	static int boxesVisible = 0;
	static int prevShowCmd = -1;

	static bool sysminimized = false;
	try {
		LRESULT uim = uniqueInstance.CheckMessage(iMessage, wParam, lParam);
		if (uim != 0) {
			return uim;
		}

		switch (iMessage) {
		case WM_NCCALCSIZE:
			return layout.OnNcCalcSize(MainHWND(), (BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);
			break;
		case WM_NCHITTEST:
		{
			LRESULT r = ::DefWindowProcW(MainHWND(), iMessage, wParam, lParam);
			POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if ((r == HTCLOSE || r == HTMINBUTTON || r == HTMAXBUTTON || r == HTCAPTION)) {
				r = HTCLIENT;
			}
			else if (r == HTCLIENT)
				r = layout.OnNcHitTestClient(MainHWND(), p);
			return r;
		}

		case WM_NCACTIVATE:
		{
			extender->OnShowActivate(wParam ? 1 : 0, layout.ShowBorder(wParam));
			return ::DefWindowProc(MainHWND(), iMessage, wParam, -1);
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{
			LRESULT r = ::DefWindowProcW(MainHWND(), iMessage, wParam, lParam);
			extender->OnShowActivate(-1, layout.ShowBorder(true));
			return r;
		}

		case WM_CREATE:
			Creation();
			keyBoardHook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardProc, hInstance, GetCurrentThreadId());
			break;
		case WM_SIZE:
		{
			LRESULT r = ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
			layout.Fit();
			extender->OnShowActivate(-1, -1);
			return r;
		}
		break;
		case WM_COMMAND:
			Command(wParam, lParam);
			break;

		case WM_CONTEXTMENU:
			return ContextMenuMessage(iMessage, wParam, lParam, false);
		case SCI_MARGINCONTEXTMENU:
			return ContextMenuMessage(iMessage, wParam, lParam, true);

		case WM_ENTERMENULOOP:
			if (!wParam){
				menuSource = 0;
				::EndMenu();
				if (bAltReleased)
					extender->OnMenuChar(1, "");
			}
			break;
		case WM_INITMENUPOPUP:
		case WM_UNINITMENUPOPUP:
			iupwinMenuDialogProc(NULL, iMessage, wParam, lParam);

			break;
		case WM_MENUCHAR:
		{
			WORD wp = HIWORD(wParam);
			switch (wp) {
			case MF_SYSMENU:
			case 0x00000010L:
			{
				char key[2];
				key[1] = 0;
				key[0] = static_cast<char>(LOWORD(wParam));
				return extender->OnMenuChar(wp & 0xff, key);
			}
			break;

			}
		}
		break;
		case WM_SYSCHAR:
			return ::DefWindowProcW(MainHWND(), iMessage, wParam, lParam);

		case WM_SYSCOMMAND:
			if (wParam == SC_MINIMIZE)
				sysminimized = true;
			if ((wParam == SC_MINIMIZE) && props.GetInt("minimize.to.tray")) {
				MinimizeToTray();
				return 0;
			} else if(wParam == SC_CLOSE){
				::SendMessage(MainHWND(), WM_COMMAND, IDM_QUIT, 0);
				return 0;
			}
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

		case SCITE_TRAY:
			if (lParam == WM_LBUTTONDOWN) {
				RestoreFromTray();
				::ShowWindow(MainHWND(), SW_RESTORE);
				::FlashWindow(MainHWND(), FALSE);
			}
			break;
		case SCITE_NOTIFYTREAD:
		{
			extender->OnLindaNotify((const char*)wParam, (const char*)lParam);
			return 0;
		}
		case SCITE_NOTIYCMD:
			//Пришли даные из консольного потока  - выполним скрипт в луа
			return (LRESULT)extender->OnSendEditor(SCN_NOTYFY_OUTPUTCMD, wParam, reinterpret_cast<const char *>(lParam));

		case SCITE_NOTIFYCMDEXIT:
			//закончился консольный поток - выполним скрипт в луа
			return (LRESULT)extender->OnSendEditor(SCN_NOTYFY_OUTPUTEXIT, lParam, wParam);
		case SCI_POSTCALBACK:
			return (LRESULT)extender->OnPostCallback(static_cast<int>(wParam));
		case SCI_FINDPROGRESS:
			return (LRESULT)extender->OnFindProgress(wParam, lParam);
		case SCI_POSTPONECHECKRELOAD:
			CheckReload();
			break;

		case WM_MOUSEMOVE:
			::SetCursor(::LoadCursor(NULL, IDC_ARROW)); //Сюда попадаем только при открытом модальном диалоге
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			HWND h = GetActiveWindow();
			for (HWND h = ::FindWindowExA(NULL, NULL, "IupDialogSaveBits", NULL); h; h = ::FindWindowExA(NULL, h, "IupDialogSaveBits", NULL)) {
				if (!(GetWindowLongA(h, GWL_STYLE) & WS_DISABLED)) {
					FlashWindow(h, true);
					break;
				}
			}
		}
		break;

		case SCITE_DROP:
			// Open the files
			extender->OnNavigation("_openSet");
			while (!dropFilesQueue.empty()) {
				FilePath file(dropFilesQueue.front());
				if (dropFilesQueue.size() == 1){
					extender->OnNavigation("_openSetLast");
				}
				dropFilesQueue.pop_front();
				if (file.Exists()) {
					Open(file.AsInternal());
				} else {
					extender->HildiAlarm("Could not open file\n'%1'.",
						MB_OK | MB_ICONWARNING, file.AsInternal());
				}
			} 
			extender->OnNavigation("_-openSet");
			break;

		case WM_NOTIFY:
			Notify(reinterpret_cast<SCNotification *>(lParam));
			break;

		case WM_KEYDOWN:
			return KeyDown(wParam);

		case WM_KEYUP:
			return KeyUp(wParam);

		case WM_MOVE:
			wEditor.CallTipCancel();
			break;

		case WM_GETMINMAXINFO:
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
			break;

		case WM_INITMENU:
			CheckMenus();
			break;

		case WM_MENUSELECT:
			if (!topLevelPopUp) topLevelPopUp = (HMENU)lParam;	   //При снлнкте пераой строет после установеи хука запоминаем меню
			bIsPopUpMenuItem = ((HIWORD(wParam) & MF_POPUP) ? 1 : 0) +(( topLevelPopUp != (HMENU)lParam) ? 2 : 0); //Не будем посылать нотификацию на метках открытия сабменб=ю и на сбменю
			break;

		case WM_CLOSE:
			QuitProgram();
			return 0;

		case WM_QUERYENDSESSION:
			QuitProgram();
			return 1;

		case WM_DESTROY:
			layout.Close();
			UnhookWindowsHookEx(keyBoardHook);
			keyBoardHook = NULL;
			PostMessage(MainHWND(), WM_QUIT, 0, 0);
			break;

		case WM_SETTINGCHANGE:
			wEditor.Send(WM_SETTINGCHANGE, wParam, lParam);
			wOutput.Send(WM_SETTINGCHANGE, wParam, lParam);
			break;

		case WM_SYSCOLORCHANGE:
			wEditor.Send(WM_SYSCOLORCHANGE, wParam, lParam);
			wOutput.Send(WM_SYSCOLORCHANGE, wParam, lParam);
			break;

		case WM_PALETTECHANGED:
			if (wParam != reinterpret_cast<WPARAM>(MainHWND())) {
				wEditor.Send(WM_PALETTECHANGED, wParam, lParam);
				//wOutput.Call(WM_PALETTECHANGED, wParam, lParam);
			}

			break;

		case WM_QUERYNEWPALETTE:
			wEditor.Send(WM_QUERYNEWPALETTE, wParam, lParam);
			return TRUE;

		case WM_ACTIVATEAPP:
			if (props.GetInt("selection.hide.on.deactivate", 1)) {
				wEditor.HideSelection(!wParam);
			}
			extender->OnMenuChar(3, "NO");
			// Do not want to display dialog yet as may be in middle of system mouse capture
			::PostMessage(MainHWND(), WM_COMMAND, IDM_ACTIVATE, wParam);
			if(wParam == WA_INACTIVE)
			{
				wActive = ::GetFocus();
			}
			else
			{
				if(wActive)::SetFocus(wActive);
			}
			break;

		case WM_ACTIVATE:
			if (wParam != WA_INACTIVE) {
				::SetFocus(wFocus);
			} 
			break;

		case WM_DROPFILES:
			DropFiles(reinterpret_cast<HDROP>(wParam));
			break;

		case WM_COPYDATA:
			return uniqueInstance.CopyData(reinterpret_cast<COPYDATASTRUCT *>(lParam));

		case WM_DRAWCLIPBOARD:
			return OnDrawClipBoardMsg(wParam);
		case WM_CHANGECBCHAIN:
			return OnChangeCBChain(wParam, lParam);
		case WM_DRAWITEM:
		case WM_MEASUREITEM:
			if (!wParam)
				return SendMessage(layout.GetChildHWND("LAYOUT"), iMessage, wParam, lParam);
			break;
		default:
			return ::DefWindowProcW(MainHWND(), iMessage, wParam, lParam);
		}
	} catch (GUI::ScintillaFailure &sf) {
		statusFailure = sf.status;
	}
	if ((statusFailure > 0) && (boxesVisible == 0)) {
		boxesVisible++;
		char buff[200];
		if (statusFailure == SC_STATUS_BADALLOC) {
			strcpy(buff, "Memory exhausted.");
		} else {
			sprintf(buff, "Scintilla failed with status %d.", statusFailure);
		}
		strcat(buff, " SciTE will now close.");
		GUI::gui_string sMessage = GUI::StringFromUTF8(buff);
		::MessageBox(MainHWND(), sMessage.c_str(), TEXT("Failure in Scintilla"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		exit(FALSE);
	}
	return 0l;
}

bool SciTEWin::IsRunAsAdmin() {
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup)) {
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup) {
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError) {
		throw dwError;
	}

	return fIsRunAsAdmin;
}

bool SciTEWin::NewInstance(const char* arg, bool asAdmin) {
	wchar_t szPath[MAX_PATH];
	GUI::gui_string a = GUI::StringFromUTF8(arg);
	if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
		// Launch itself as administrator.
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		if(asAdmin)
			sei.lpVerb = L"runas";
		sei.lpFile = szPath;
		sei.lpParameters = a.c_str(); 
		sei.hwnd = (HWND)wSciTE.GetID();
		sei.nShow = SW_NORMAL;

		return ShellExecuteEx(&sei);
	}
	return false;
}

// Take care of 32/64 bit pointers
#ifdef GetWindowLongPtr
static void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, 0));
}
static void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(ptr));
}
#else
static void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLong(hWnd, 0));
}
static void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(ptr));
}
#endif

LRESULT PASCAL SciTEWin::TWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {

	// Find C++ object associated with window.
	SciTEWin *scite = reinterpret_cast<SciTEWin *>(PointerFromWindow(hWnd));
	// scite will be zero if WM_CREATE not seen yet
	if (scite == 0) {
		if (iMessage == WM_CREATE) {
			LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			scite = reinterpret_cast<SciTEWin *>(cs->lpCreateParams);
			scite->wSciTE = hWnd;
			SetWindowPointer(hWnd, scite);
			return scite->WndProc(iMessage, wParam, lParam);
		} else
			return ::DefWindowProcW(hWnd, iMessage, wParam, lParam);
	} else
		return scite->WndProc(iMessage, wParam, lParam);
}


static void SetFontHandle(GUI::Window &w, HFONT hfont) {
	::SendMessage(HwndOf(w),
		WM_SETFONT, reinterpret_cast<WPARAM>(hfont),
		0);    // redraw option
}

static int WidthText(HFONT hfont, const GUI::gui_char *text) {
	HDC hdcMeasure = ::CreateCompatibleDC(NULL);
	HFONT hfontOriginal = static_cast<HFONT>(::SelectObject(hdcMeasure, hfont));
	RECT rcText = {0,0, 2000, 2000};
	::DrawText(hdcMeasure, const_cast<LPWSTR>(text), -1, &rcText, DT_CALCRECT);
	int width = rcText.right - rcText.left;
	::SelectObject(hdcMeasure, hfontOriginal);
	::DeleteDC(hdcMeasure);
	return width;
}

static int WidthControl(GUI::Window &w) {
	GUI::Rectangle rc = w.GetPosition();
	return rc.Width();
}

static SString ControlText(GUI::Window w) {
	HWND wT = HwndOf(w);
	int len = ::GetWindowTextLengthW(wT) + 1;
	std::vector<GUI::gui_char> itemText(len);
	GUI::gui_string gsFind;
	if (::GetWindowText(wT, &itemText[0], len)) {
		gsFind = GUI::gui_string(&itemText[0]);
	}
	return GUI::UTF8FromString(gsFind.c_str()).c_str();
}

static const char *textFindPrompt = "Fi&nd:";
static const char *textReplacePrompt = "Rep&lace:";
static const char *textFindNext = "&Find Next";
static const char *textMarkAll = "&Mark All";

//~ static const char *textFind = "F&ind";
static const char *textReplace = "&Replace";
static const char *textReplaceAll = "Replace &All";
static const char *textInSelection = "In &Selection";
//~ static const char *textReplaceInBuffers = "Replace In &Buffers";
//~ static const char *textClose = "Close";

struct Toggle {
	enum { tWord, tCase, tRegExp, tBackslash, tWrap, tUp };
	const char *label;
	int cmd;
	int id;
};

static Toggle toggles[] = {
	{"Match &whole word only", IDM_WHOLEWORD, IDWHOLEWORD},
	{"Case sensiti&ve", IDM_MATCHCASE, IDMATCHCASE},
	{"Regular &expression", IDM_REGEXP, IDREGEXP},
	{"Transform &backslash expressions", IDM_UNSLASH, IDUNSLASH},
	{"Wrap ar&ound", IDM_WRAPAROUND, IDWRAP},
	{"&Up", IDM_DIRECTIONUP, IDDIRECTIONUP},
	{0, 0, 0},
};

#define PACKVERSION(major,minor) MAKELONG(minor,major)

static DWORD GetVersion(LPCTSTR lpszDllName) {
    DWORD dwVersion = 0;
    HINSTANCE hinstDll = ::LoadLibrary(lpszDllName);
    if (hinstDll) {
        DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");

        if (pDllGetVersion) {
            DLLVERSIONINFO dvi;
            ::ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            HRESULT hr = (*pDllGetVersion)(&dvi);
            if (SUCCEEDED(hr)) {
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }
        ::FreeLibrary(hinstDll);
    }
    return dwVersion;
}


static bool HideKeyboardCues() {
	BOOL b;
	::SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &b, 0);
	return !b;
}

LRESULT PASCAL BaseWin::StWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	// Find C++ object associated with window.
	BaseWin *base = reinterpret_cast<BaseWin *>(::PointerFromWindow(hWnd));
	// scite will be zero if WM_CREATE not seen yet
	if (base == 0) {
		if (iMessage == WM_CREATE) {
			LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			base = reinterpret_cast<BaseWin *>(cs->lpCreateParams);
			SetWindowPointer(hWnd, base);
			base->SetID(hWnd);
			return base->WndProc(iMessage, wParam, lParam);
		} else
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	} else
		return base->WndProc(iMessage, wParam, lParam);
}

// Convert String from UTF-8 to doc encoding
SString SciTEWin::EncodeString(const SString &s) {
	//::MessageBox(GetFocus(),SString(s).c_str(),"EncodeString:in",0);

	UINT codePage = static_cast<UINT>(wEditor.CodePage());

	if (codePage != SC_CP_UTF8) {
		codePage = GUI::CodePageFromCharSet(characterSet, codePage); 

		return SString(GUI::ConvertFromUTF8(s.c_str(), codePage).c_str()); 
	}
	return SciTEBase::EncodeString(s);
}

// Convert String from doc encoding to UTF-8
SString SciTEWin::GetRangeInUIEncoding(GUI::ScintillaWindow &win, int selStart, int selEnd) {
	SString s = SciTEBase::GetRangeInUIEncoding(win, selStart, selEnd);

	UINT codePage = static_cast<UINT>(wEditor.CodePage());

	if (codePage != SC_CP_UTF8) {
		codePage = GUI::CodePageFromCharSet(characterSet, codePage);

		return GUI::ConvertToUTF8(s.c_str(), codePage).c_str(); 
	}
	return s;
}

LRESULT SciTEWin::OnDrawClipBoardMsg(WPARAM wParam)
{
	if (wParam == 1){
		extender->OnDrawClipboard(::IsClipboardFormatAvailable(CF_TEXT) ? (::IsClipboardFormatAvailable(cfColumnSelect) ? 2 : 1 ) : 0);
	}
	else
	{
		//extender->OnDrawClipboard(::IsClipboardFormatAvailable(CF_TEXT) ? (::IsClipboardFormatAvailable(cfColumnSelect) ? 2 : 1) : 0);
		if (hNextCBWnd&&IsWindow(hNextCBWnd))
		{
			SendMessage(hNextCBWnd, WM_DRAWCLIPBOARD, 0, 0);
		}
		else hNextCBWnd = 0;
		//PostMessage(MainHWND(), WM_DRAWCLIPBOARD, 1, 1);
		SendMessageTimeout(MainHWND(), WM_DRAWCLIPBOARD, 1, 1, SMTO_ABORTIFHUNG, 100, NULL);
	}
	return 0;
}

LRESULT SciTEWin::OnChangeCBChain(WPARAM wParam, LPARAM lParam)
{
	//Удалился какой-то вьювер
	if (hNextCBWnd == (HWND)wParam)
	{//Он следующий в очереди: изменяем нашу глобальную переменную
		hNextCBWnd = (HWND)lParam;
		return 0;
	}
	else
	{//Иначе пересылаем сообщение дальше
		return::SendMessage(hNextCBWnd, WM_CHANGECBCHAIN, wParam, lParam);
	}
}

int SciTEWin::EventLoop() {
	MSG msg;
	msg.wParam = 0;

	while (TRUE)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message != WM_QUIT)
			{
				switch (msg.message) {
				case WM_SYSKEYDOWN:
					switch (msg.wParam) {
					case VK_MENU:
						if (!kbdHookOn) {
							extender->OnMenuChar(3, "YES");
						}
						break;
					}
					break;
				}
				//if (!ModelessHandler(&msg) && (!::TranslateAccelerator(reinterpret_cast<HWND>(GetID()), GetAcceleratorTable(), &msg)))
				//Изменен порядок выполнения для легкой блокировки пользовательского ввода(findres), не нарушающей работы акселераторов
				if ((!::TranslateAccelerator(reinterpret_cast<HWND>(GetID()), GetAcceleratorTable(), &msg)) && (!ModelessHandler(&msg)))
				{
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}

			}
			else goto BRK;
		}
		layout.OnIdle();
		extender->OnIdle();
		WaitMessage();
	}
BRK:
	if (restartCmdLine != "-") {
		hWriteSubProcess = NULL;
		subProcessGroupId = 0;

		// Make child process use hPipeWrite as standard out, and make
		// sure it does not show on screen.
		STARTUPINFOW si = {
			sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		si.wShowWindow = SW_SHOW;
		WCHAR fName[500]; 
		::GetModuleFileNameW(NULL, fName, 499);

		PROCESS_INFORMATION pi = { 0, 0, 0, 0 };

		::CreateProcessW(
			fName,
			const_cast<wchar_t *>(restartCmdLine == "" ? NULL: GUI::StringFromUTF8(restartCmdLine.c_str()).c_str()),
			NULL, NULL,
			TRUE, CREATE_NEW_PROCESS_GROUP,
			NULL,
			NULL,
			&si, &pi);

	}
	return static_cast<int>(msg.wParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

	typedef BOOL (WINAPI *SetDllDirectorySig)(LPCTSTR lpPathName);
	SetDllDirectorySig SetDllDirectoryFn = (SetDllDirectorySig)::GetProcAddress(
		::GetModuleHandle(TEXT("kernel32.dll")), "SetDllDirectoryW");
	if (SetDllDirectoryFn) {
		// For security, remove current directory from the DLL search path
		SetDllDirectoryFn(TEXT(""));
	}
	 
	LuaExtension multiExtender;
	Extension *extender = &multiExtender;

	GUI::gui_string cmdLine = GetCommandLine();
	if (cmdLine[0] == '\"') {
		cmdLine = cmdLine.substr(cmdLine.substr(1).find('\"') + 2);
	} else if (cmdLine.find(' ') != std::string::npos) {
		cmdLine = cmdLine.substr(cmdLine.substr(1).find(' ') + 1);
	} else
		cmdLine = L"";	
	GUI::gui_string advProps;
	bool allowDouble = false;
	if (cmdLine != L"") {
		std::wregex re(L"^((?![-/]cmd).)*?[-/]d[-/ ]", std::regex::ECMAScript); //ищем -d ДО которого нет -cmd
		allowDouble = std::regex_search(cmdLine + L" ", re);
		re = L"^((?![-/]cmd).)*?[-/]props=\"([^\"]+)\".*";

		std::wsmatch mtch;

		if (std::regex_match(cmdLine, mtch, re)) {
			advProps = mtch[2];
		}
	}

	SciTEWin MainWind(extender, advProps);
	SciTEWin::Register(hInstance);

	Lexilla::SetDefaultDirectory(GetSciTEPath(FilePath()).AsUTF8());

	HMODULE hmod = ::LoadLibrary(TEXT("SciLexer.DLL"));
	if (hmod == NULL) {
		::MessageBox(NULL, TEXT("The Scintilla DLL could not be loaded.  HoldiM will now close"),
			TEXT("Error loading Scintilla"), MB_OK | MB_ICONERROR);
		return 1;
	}

	pSciTEWin = &MainWind;

	MainWind.Run(cmdLine, allowDouble);
	int result = MainWind.EventLoop();

	_beginthread(KillThreadProc, 1024 * 1024, NULL);
	OleUninitialize();
	::FreeLibrary(hmod);

	return result;
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	LRESULT r = pSciTEWin->NotifyGetMsgProc(nCode, wParam, lParam);
	return CallNextHookEx(macroHook, nCode, wParam, lParam) || r;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	pSciTEWin->NotifyMouseHook(nCode, wParam, lParam);
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

bool SciTEWin::SwitchMacroHook(bool bSet) {
	bool result = false;
	if (bSet && !macroHook) {
		macroHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hInstance, GetCurrentThreadId());
		result = (macroHook != NULL);
	} else if (!bSet && macroHook) {
		result = UnhookWindowsHookEx(macroHook);
		macroHook = NULL;
	}
	return result;
}
bool SciTEWin::SwitchMouseHook(bool bSet){
	bool result = false;
	if (bSet && !mouseHook){
		bIsPopUpMenuItem = false;
		topLevelPopUp = NULL;
		mouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, hInstance, GetCurrentThreadId());
		kbdHookOn = true;
		result = (mouseHook != NULL);
	}
	else if (!bSet && mouseHook){
		result = UnhookWindowsHookEx(mouseHook);
		kbdHookOn = false;
		
		::EndMenu();
		mouseHook = NULL;
	}
	return result;
}
LRESULT SciTEWin::NotifyGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	MSG * pMessage = (MSG*)lParam;
	if (pMessage->hwnd == wEditor.GetID()) {
		switch (pMessage->message) {
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			if(extender->OnMacroBlocked(pMessage->message, reinterpret_cast<intptr_t>(pMessage) - wParam, static_cast<intptr_t>(pMessage->lParam)))
				pMessage->message = 0;
			break;
		}
	}
	return 0;
}
void SciTEWin::NotifyMouseHook(int nCode, WPARAM wParam, LPARAM lParam){

	if (wParam == WM_MOUSEMOVE){
		LPMOUSEHOOKSTRUCT mh = (LPMOUSEHOOKSTRUCT)lParam;
		RECT area;
		SystemParametersInfoA(SPI_GETWORKAREA, 0, &area, 0);
		extender->OnMouseHook(mh->pt.x - area.left, mh->pt.y - area.top);
	} else if (wParam == WM_RBUTTONUP) { //|| wParam == WM_RBUTTONDOWN
		extender->OnMouseHook(-7000, -7000);
	} else if (wParam == WM_KEYUP) {
		extender->OnMenuChar(2, lParam > 0 ? "1" : "-1");
	} else if (wParam == WM_KEYDOWN) {
		extender->OnMenuChar(3, lParam > 0 ? "YES" : "NO");
	}
}
void SciTEWin::PostLoadScript() {
}
__declspec(dllexport) void* GetCaller(SciCaller c) {
	return reinterpret_cast<void*>(pSciTEWin->GetCaler(c));
}

