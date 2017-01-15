// SciTE - Scintilla based Text Editor
/** @file SciTEWinDlg.cxx
 ** Dialog code for the Windows version of the editor.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include "Platform.h" //!-add-[no_wornings]
#include "SciTEWin.h"
// need this header for SHBrowseForFolder
#include <shlobj.h>

/**
 * Flash the given window for the asked @a duration to visually warn the user.
 */
static void FlashThisWindow(
    HWND hWnd,    		///< Window to flash handle.
    int duration) {	///< Duration of the flash state.

	HDC hDC = ::GetDC(hWnd);
	if (hDC != NULL) {
		RECT rc;
		::GetClientRect(hWnd, &rc);
		::FillRect(hDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		::Sleep(duration);
	}
	::ReleaseDC(hWnd, hDC);
	::InvalidateRect(hWnd, NULL, true);
}

/**
 * Play the given sound, loading if needed the corresponding DLL function.
 */
static void PlayThisSound(
    const char *sound,    	///< Path to a .wav file or string with a frequency value.
    int duration,    		///< If @a sound is a frequency, gives the duration of the sound.
    HMODULE &hMM) {		///< Multimedia DLL handle.

	bool bPlayOK = false;
	int soundFreq;
	if (!sound || *sound == '\0') {
		soundFreq = -1;	// No sound at all
	} else {
		soundFreq = atoi(sound);	// May be a frequency, not a filename
	}

	if (soundFreq == 0) {	// sound is probably a path
		if (!hMM) {
			// Load the DLL only if needed (may be slow on some systems)
			hMM = ::LoadLibrary(TEXT("WINMM.DLL"));
		}

		if (hMM) {
			typedef BOOL (WINAPI *MMFn) (LPCSTR, HMODULE, DWORD);
			MMFn fnMM = (MMFn)::GetProcAddress(hMM, "PlaySoundA");
			if (fnMM) {
				bPlayOK = fnMM(sound, NULL, SND_ASYNC | SND_FILENAME);
			}
		}
	}
	if (!bPlayOK && soundFreq >= 0) {	// The sound could no be played, or user gave a frequency
		// Will use the speaker to generate a sound
		if (soundFreq < 37 || soundFreq > 32767) {
			soundFreq = 440;
		}
		if (duration < 50) {
			duration = 50;
		}
		if (duration > 5000) {	// Don't play too long...
			duration = 5000;
		}
		// soundFreq and duration are not used on Win9x.
		// On those systems, PC will either use the default sound event or
		// emit a standard speaker sound.
		::Beep(soundFreq, duration);
	}
}

static SciTEWin *Caller(HWND hDlg, UINT message, LPARAM lParam) {
	if (message == WM_INITDIALOG) {
		::SetWindowLongPtr(hDlg, DWLP_USER, lParam);
	}
	return reinterpret_cast<SciTEWin*>(::GetWindowLongPtr(hDlg, DWLP_USER));
}

//! void SciTEWin::WarnUser(int warnID) {
void SciTEWin::WarnUser(int warnID, const char *msg /* = NULL */, bool isCanBeAlerted /* = true */) { //!-change-[WarningMessage]
	SString warning;
	SString warning_msg; //!-add-[WarningMessage]
	char *warn;
	char flashDuration[10], sound[_MAX_PATH], soundDuration[10];

	switch (warnID) {
	case warnFindWrapped:
		warning = props.Get("warning.findwrapped");
		warning_msg = props.Get("warning.findwrapped.message"); //!-add-[WarningMessage]
		break;
	case warnNotFound:
		warning = props.Get("warning.notfound");
		warning_msg = props.Get("warning.notfound.message"); //!-add-[WarningMessage]
		break;
	case warnWrongFile:
		warning = props.Get("warning.wrongfile");
		warning_msg = props.Get("warning.wrongfile.message"); //!-add-[WarningMessage]
		break;
	case warnExecuteOK:
		warning = props.Get("warning.executeok");
		warning_msg = props.Get("warning.executeok.message"); //!-add-[WarningMessage]
		break;
	case warnExecuteKO:
		warning = props.Get("warning.executeko");
		warning_msg = props.Get("warning.executeko.message"); //!-add-[WarningMessage]
		break;
	case warnNoOtherBookmark:
		warning = props.Get("warning.nootherbookmark");
		warning_msg = props.Get("warning.nootherbookmark.message"); //!-add-[WarningMessage]
		break;
	default:
		warning = "";
		break;
	}
	warn = StringDup(warning.c_str());
	const char *next = GetNextPropItem(warn, flashDuration, 10);
	next = GetNextPropItem(next, sound, _MAX_PATH);
	GetNextPropItem(next, soundDuration, 10);
	delete []warn;

	int flashLen = atoi(flashDuration);
	if (flashLen) {
		FlashThisWindow(reinterpret_cast<HWND>(wEditor.GetID()), flashLen);
	}
	PlayThisSound(sound, atoi(soundDuration), hMM);
//!-start-[WarningMessage]
	if (warning_msg.length() > 0 && isCanBeAlerted) {
		warning_msg = GUI::UTF8FromString(localiser.Text(warning_msg.c_str())).c_str();
		warning_msg += "     ";
		if (msg != NULL) {
			warning_msg += "\n";
			warning_msg += GUI::UTF8FromString(localiser.Text(msg)).c_str();
			warning_msg += "     ";
		}
		WindowMessageBox(wEditor, GUI::StringFromUTF8(warning_msg.c_str()), MB_OK | MB_ICONWARNING);
	}
//!-end-[WarningMessage]
}

bool SciTEWin::DialogHandled(GUI::WindowID id, MSG *pmsg) {
	if (id) {
		if (	::IsDialogMessageW(reinterpret_cast<HWND>(id), pmsg))
			return true;
	}
	return false;
}

bool SciTEWin::ModelessHandler(MSG *pmsg) {
	if (wFindReplace.GetID() || wFindInFiles.GetID() || wParameters.GetID()) {
		//Пропускаем в основное окно все клавиатурные команды
		bool menuKey = (pmsg->message == WM_KEYDOWN || pmsg->message == WM_SYSKEYDOWN) &&
			(pmsg->wParam != VK_TAB) &&
			(pmsg->wParam != VK_ESCAPE) &&
			(pmsg->wParam != VK_RETURN);  

		if (!menuKey && wFindReplace.GetID() && DialogHandled(wFindReplace.GetID(), pmsg))
			return true;
		if (!menuKey && wFindInFiles.GetID() && DialogHandled(wFindInFiles.GetID(), pmsg))
			return true;
		if (!menuKey && DialogHandled(wParameters.GetID(), pmsg))
			return true;
	}
	if (pmsg->message == WM_KEYDOWN || pmsg->message == WM_SYSKEYDOWN) {
		if (KeyDown(pmsg->wParam))
			return true;
	} else if (pmsg->message == WM_KEYUP) {
		if (KeyUp(pmsg->wParam))
			return true;
	}

	return false;
}

//  DoDialog is a bit like something in PC Magazine May 28, 1991, page 357
int SciTEWin::DoDialog(HINSTANCE hInst, const TCHAR *resName, HWND hWnd, DLGPROC lpProc) {
	int result = ::DialogBoxParam(hInst, resName, hWnd, lpProc, reinterpret_cast<LPARAM>(this));

	if (result == -1) {
		GUI::gui_string errorNum = GUI::StringFromInteger(::GetLastError());
		GUI::gui_string msg = LocaliseMessage("Failed to create dialog box: ^0.", errorNum.c_str());
		::MessageBoxW(hWnd, msg.c_str(), appName, MB_OK | MB_SETFOREGROUND);
	}

	return result;
}


GUI::gui_string SciTEWin::DialogFilterFromProperty(const GUI::gui_char *filterProperty) {
	GUI::gui_string filter = filterProperty;
	filter.append(L"||");
	if (filter.length()) {
		std::replace(filter.begin(), filter.end(), '|', '\0');
		size_t start = 0;
		while (start < filter.length()) {
			const GUI::gui_char *filterName = filter.c_str() + start;
			if (*filterName == '#') {
				size_t next = start + wcslen(filter.c_str() + start) + 1;
				next += wcslen(filter.c_str() + next) + 1;
				filter.erase(start, next - start);
			} else {
				GUI::gui_string localised = localiser.Text(GUI::UTF8FromString(filterName).c_str(), false);
				if (localised.size()) {
					filter.erase(start, wcslen(filterName));
					filter.insert(start, localised.c_str());
				}
				start += wcslen(filter.c_str() + start) + 1;
				start += wcslen(filter.c_str() + start) + 1;
			}
		}
	}
	return filter;
}

bool SciTEWin::OpenDialog(FilePath directory, const GUI::gui_char *filter) {
	enum {maxBufferSize=2048};

	GUI::gui_string openFilter = DialogFilterFromProperty(filter);

	if (!openWhat[0]) {
		wcscpy(openWhat, localiser.Text("Custom Filter").c_str());
		openWhat[wcslen(openWhat) + 1] = '\0';
	}

	bool succeeded = false;
	GUI::gui_char openName[maxBufferSize]; // maximum common dialog buffer size (says mfc..)
	openName[0] = '\0';

//!-start-[no wornings]
#if defined(_MSC_VER) && _MSC_VER < 1300
	OPENFILENAMEW ofn = {
	       sizeof(ofn), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
#else
//!-end-[no wornings]
	OPENFILENAMEW ofn = {
	       sizeof(ofn), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
#endif //!-add-[no wornings]
	ofn.hwndOwner = MainHWND();
	ofn.hInstance = hInstance;
	ofn.lpstrFile = openName;
	ofn.nMaxFile = maxBufferSize;
	ofn.lpstrFilter = openFilter.c_str();
	ofn.lpstrCustomFilter = openWhat;
	ofn.nMaxCustFilter = ELEMENTS(openWhat);
	ofn.nFilterIndex = filterDefault;
	GUI::gui_string translatedTitle = localiser.Text("Open File");
	ofn.lpstrTitle = translatedTitle.c_str();
	if (props.GetInt("open.dialog.in.file.directory")) {
		ofn.lpstrInitialDir = directory.AsInternal();
	}
	ofn.Flags = OFN_HIDEREADONLY;

	if (buffers.size > 1) {
		ofn.Flags |=
		    OFN_EXPLORER |
		    OFN_PATHMUSTEXIST |
		    OFN_ALLOWMULTISELECT;
	}
	if (::GetOpenFileNameW(&ofn)) {
		succeeded = true;
		filterDefault = ofn.nFilterIndex;
		// if single selection then have path+file
		if (extender) extender->OnNavigation("Open");

		if (wcslen(openName) > static_cast<size_t>(ofn.nFileOffset)) {
			Open(openName);
		} else {
			FilePath directory(openName);
			GUI::gui_char *p = openName + wcslen(openName) + 1;
			while (*p) {
				// make path+file, add it to the list
				Open(FilePath(directory, FilePath(p)));
				// goto next char pos after \0
				p += wcslen(p) + 1;
			}
		}
		if (extender) extender->OnNavigation("Open-");
	}
	return succeeded;
}

FilePath SciTEWin::ChooseSaveName(FilePath directory, const char *title, const GUI::gui_char *filter, const char *ext, int *nFilter) {
	FilePath path;
	if (0 == dialogsOnScreen) {
		GUI::gui_char saveName[MAX_PATH] = GUI_TEXT("");
		FilePath savePath = SaveName(ext);
		if (!savePath.IsUntitled()) {
			GUI::gui_char saveNameSrt[MAX_PATH] = GUI_TEXT("");
			wcscpy(saveNameSrt, savePath.Name().AsInternal());
			if (saveNameSrt[0] == L'^') wcscpy(saveNameSrt, saveNameSrt + 1);
			wcscpy(saveName, savePath.Directory().AsInternal());
			wcscat(saveName, L"\\");
			wcscat(saveName, saveNameSrt);
		}
		
//!-start-[no wornings]
#if defined(_MSC_VER) && _MSC_VER < 1300
		OPENFILENAMEW ofn = {
			   sizeof(ofn), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
#else
//!-end-[no wornings]
		OPENFILENAMEW ofn;
		::ZeroMemory(&ofn, sizeof(ofn));
#endif //!-add-[no wornings]
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = MainHWND();
		ofn.hInstance = hInstance;
		ofn.lpstrFile = saveName;
		ofn.nMaxFile = ELEMENTS(saveName);
		GUI::gui_string translatedTitle = localiser.Text(title);
		ofn.lpstrTitle = translatedTitle.c_str();
		ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		ofn.lpstrFilter = filter;
		ofn.lpstrInitialDir = directory.AsInternal();

		dialogsOnScreen++;
		if (::GetSaveFileNameW(&ofn)) {
			path = saveName;
			if (nFilter) *nFilter = ofn.nFilterIndex;
		}
		dialogsOnScreen--;
	}
	return path;
}

bool SciTEWin::SaveAsDialog() {
	GUI::gui_string filter = GUI::StringFromUTF8(props.GetExpanded("save.filter").c_str()).c_str();
	GUI::gui_string ext = GUI::StringFromUTF8(props.GetExpanded("FileExt").c_str());
	if (ext != GUI::gui_string(L"")){
		filter = ext + L" (." + ext + L")|*." + ext + L"|" + filter;
	}
	filter = DialogFilterFromProperty(filter.c_str());
	int nFilter;;
	FilePath path = ChooseSaveName(filePath.Directory(), "Save File", filter.c_str(), NULL, &nFilter);
	if (path.IsSet()) {
		if (nFilter == 1 && path.Extension().AsInternal() != ext && ext != GUI::gui_string(L""))
			path = path.AsInternal() + GUI::gui_string(L".") + ext;
		return SaveIfNotOpen(path, false);
	}
	return false;
}

void SciTEWin::SaveACopy() {
	GUI::gui_string filter = GUI::StringFromUTF8(props.GetExpanded("save.filter").c_str()).c_str();
	GUI::gui_string ext = GUI::StringFromUTF8(props.GetExpanded("FileExt").c_str());  
	if (ext != GUI::gui_string(L"")){
		filter = ext + L" (." + ext + L")|*." + ext + L"|" + filter;
	}
	filter = DialogFilterFromProperty(filter.c_str());
	int nFilter;
	FilePath path = ChooseSaveName(filePath.Directory(), "Save a Copy", filter.c_str(), NULL, &nFilter);
	if (path.IsSet()) {
		if (nFilter == 1 && path.Extension().AsInternal() != ext && ext != GUI::gui_string(L""))
			path = path.AsInternal() + GUI::gui_string(L".") + ext;
		SaveBuffer(path);
	}
}

void SciTEWin::SaveAsHTML() {
	FilePath path = ChooseSaveName(filePath.Directory(), "Export File As HTML",
	                              GUI_TEXT("Web (.html;.htm)\0*.html;*.htm\0"), ".html");
	if (path.IsSet()) {
		SaveToHTML(path);
	}
}

void SciTEWin::SaveAsRTF() {
	FilePath path = ChooseSaveName(filePath.Directory(), "Export File As RTF",
	                              GUI_TEXT("RTF (.rtf)\0*.rtf\0"), ".rtf");
	if (path.IsSet()) {
		SaveToRTF(path);
	}
}

void SciTEWin::SaveAsPDF() {
	FilePath path = ChooseSaveName(filePath.Directory(), "Export File As PDF",
	                              GUI_TEXT("PDF (.pdf)\0*.pdf\0"), ".pdf");
	if (path.IsSet()) {
		SaveToPDF(path);
	}
}

void SciTEWin::SaveAsTEX() {
	FilePath path = ChooseSaveName(filePath.Directory(), "Export File As LaTeX",
	                              GUI_TEXT("TeX (.tex)\0*.tex\0"), ".tex");
	if (path.IsSet()) {
		SaveToTEX(path);
	}
}

void SciTEWin::SaveAsXML() {
	FilePath path = ChooseSaveName(filePath.Directory(), "Export File As XML",
	                              GUI_TEXT("XML (.xml)\0*.xml\0"), ".xml");
	if (path.IsSet()) {
		SaveToXML(path);
	}
}

static void DeleteFontObject(HFONT &font) {
	if (font) {
		::DeleteObject(font);
		font = 0;
	}
}

/**
 * Display the Print dialog (if @a showDialog asks it),
 * allowing it to choose what to print on which printer.
 * If OK, print the user choice, with optionally defined header and footer.
 */
void SciTEWin::Print(
    bool showDialog) {	///< false if must print silently (using default settings).

	RemoveFindMarks();
	PRINTDLG pdlg = {
	                    sizeof(PRINTDLG), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	                };
	pdlg.hwndOwner = MainHWND();
	pdlg.hInstance = hInstance;
	pdlg.Flags = PD_USEDEVMODECOPIES | PD_ALLPAGES | PD_RETURNDC;
	pdlg.nFromPage = 1;
	pdlg.nToPage = 1;
	pdlg.nMinPage = 1;
	pdlg.nMaxPage = 0xffffU; // We do not know how many pages in the
	// document until the printer is selected and the paper size is known.
	pdlg.nCopies = 1;
	pdlg.hDC = 0;
	pdlg.hDevMode = hDevMode;
	pdlg.hDevNames = hDevNames;

	// See if a range has been selected
	Sci_CharacterRange crange = GetSelection();
	int startPos = crange.cpMin;
	int endPos = crange.cpMax;

	if (startPos == endPos) {
		pdlg.Flags |= PD_NOSELECTION;
	} else {
		pdlg.Flags |= PD_SELECTION;
	}
	if (!showDialog) {
		// Don't display dialog box, just use the default printer and options
		pdlg.Flags |= PD_RETURNDEFAULT;
	}
	if (!::PrintDlg(&pdlg)) {
		return;
	}

	hDevMode = pdlg.hDevMode;
	hDevNames = pdlg.hDevNames;

	HDC hdc = pdlg.hDC;

	GUI::Rectangle rectMargins, rectPhysMargins;
	GUI::Point ptPage;
	GUI::Point ptDpi;

	// Get printer resolution
	ptDpi.x = GetDeviceCaps(hdc, LOGPIXELSX);    // dpi in X direction
	ptDpi.y = GetDeviceCaps(hdc, LOGPIXELSY);    // dpi in Y direction

	// Start by getting the physical page size (in device units).
	ptPage.x = GetDeviceCaps(hdc, PHYSICALWIDTH);   // device units
	ptPage.y = GetDeviceCaps(hdc, PHYSICALHEIGHT);  // device units

	// Get the dimensions of the unprintable
	// part of the page (in device units).
	rectPhysMargins.left = GetDeviceCaps(hdc, PHYSICALOFFSETX);
	rectPhysMargins.top = GetDeviceCaps(hdc, PHYSICALOFFSETY);

	// To get the right and lower unprintable area,
	// we take the entire width and height of the paper and
	// subtract everything else.
	rectPhysMargins.right = ptPage.x						// total paper width
	                        - GetDeviceCaps(hdc, HORZRES) // printable width
	                        - rectPhysMargins.left;				// left unprintable margin

	rectPhysMargins.bottom = ptPage.y						// total paper height
	                         - GetDeviceCaps(hdc, VERTRES)	// printable height
	                         - rectPhysMargins.top;				// right unprintable margin

	// At this point, rectPhysMargins contains the widths of the
	// unprintable regions on all four sides of the page in device units.

	// Take in account the page setup given by the user (if one value is not null)
	if (pagesetupMargin.left != 0 || pagesetupMargin.right != 0 ||
	        pagesetupMargin.top != 0 || pagesetupMargin.bottom != 0) {
		GUI::Rectangle rectSetup;

		// Convert the hundredths of millimeters (HiMetric) or
		// thousandths of inches (HiEnglish) margin values
		// from the Page Setup dialog to device units.
		// (There are 2540 hundredths of a mm in an inch.)

		TCHAR localeInfo[3];
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);

		if (localeInfo[0] == '0') {	// Metric system. '1' is US System
			rectSetup.left = MulDiv (pagesetupMargin.left, ptDpi.x, 2540);
			rectSetup.top = MulDiv (pagesetupMargin.top, ptDpi.y, 2540);
			rectSetup.right	= MulDiv(pagesetupMargin.right, ptDpi.x, 2540);
			rectSetup.bottom	= MulDiv(pagesetupMargin.bottom, ptDpi.y, 2540);
		} else {
			rectSetup.left	= MulDiv(pagesetupMargin.left, ptDpi.x, 1000);
			rectSetup.top	= MulDiv(pagesetupMargin.top, ptDpi.y, 1000);
			rectSetup.right	= MulDiv(pagesetupMargin.right, ptDpi.x, 1000);
			rectSetup.bottom	= MulDiv(pagesetupMargin.bottom, ptDpi.y, 1000);
		}

		// Dont reduce margins below the minimum printable area
		rectMargins.left	= Maximum(rectPhysMargins.left, rectSetup.left);
		rectMargins.top	= Maximum(rectPhysMargins.top, rectSetup.top);
		rectMargins.right	= Maximum(rectPhysMargins.right, rectSetup.right);
		rectMargins.bottom	= Maximum(rectPhysMargins.bottom, rectSetup.bottom);
	} else {
		rectMargins.left	= rectPhysMargins.left;
		rectMargins.top	= rectPhysMargins.top;
		rectMargins.right	= rectPhysMargins.right;
		rectMargins.bottom	= rectPhysMargins.bottom;
	}

	// rectMargins now contains the values used to shrink the printable
	// area of the page.

	// Convert device coordinates into logical coordinates
	DPtoLP(hdc, (LPPOINT) &rectMargins, 2);
	DPtoLP(hdc, (LPPOINT)&rectPhysMargins, 2);

	// Convert page size to logical units and we're done!
	DPtoLP(hdc, (LPPOINT) &ptPage, 1);

	SString headerFormat = props.Get("print.header.format");
	SString footerFormat = props.Get("print.footer.format");

	TEXTMETRIC tm;
	SString headerOrFooter;	// Usually the path, date and page number

	SString headerStyle = props.Get("print.header.style");
	StyleDefinition sdHeader(headerStyle.c_str());

	int headerLineHeight = ::MulDiv(
	                           (sdHeader.specified & StyleDefinition::sdSize) ? sdHeader.size : 9,
	                           ptDpi.y, 72);
	HFONT fontHeader = ::CreateFontA(headerLineHeight,
	                                0, 0, 0,
	                                sdHeader.bold ? FW_BOLD : FW_NORMAL,
	                                sdHeader.italics,
	                                sdHeader.underlined,
	                                0, 0, 0,
	                                0, 0, 0,
	                                (sdHeader.specified & StyleDefinition::sdFont) ? sdHeader.font.c_str() : "Arial");
	::SelectObject(hdc, fontHeader);
	::GetTextMetrics(hdc, &tm);
	headerLineHeight = tm.tmHeight + tm.tmExternalLeading;

	SString footerStyle = props.Get("print.footer.style");
	StyleDefinition sdFooter(footerStyle.c_str());

	int footerLineHeight = ::MulDiv(
	                           (sdFooter.specified & StyleDefinition::sdSize) ? sdFooter.size : 9,
	                           ptDpi.y, 72);
	HFONT fontFooter = ::CreateFontA(footerLineHeight,
	                                0, 0, 0,
	                                sdFooter.bold ? FW_BOLD : FW_NORMAL,
	                                sdFooter.italics,
	                                sdFooter.underlined,
	                                0, 0, 0,
	                                0, 0, 0,
	                                (sdFooter.specified & StyleDefinition::sdFont) ? sdFooter.font.c_str() : "Arial");
	::SelectObject(hdc, fontFooter);
	::GetTextMetrics(hdc, &tm);
	footerLineHeight = tm.tmHeight + tm.tmExternalLeading;

	DOCINFO di = {sizeof(DOCINFO), 0, 0, 0, 0};
	di.lpszDocName = windowName.c_str();
	di.lpszOutput = 0;
	di.lpszDatatype = 0;
	di.fwType = 0;
	if (::StartDoc(hdc, &di) < 0) {
		::DeleteDC(hdc);
		DeleteFontObject(fontHeader);
		DeleteFontObject(fontFooter);
		GUI::gui_string msg = LocaliseMessage("Can not start printer document.");
		WindowMessageBox(wSciTE, msg, MB_OK);
		return;
	}

	LONG lengthDoc = wEditor.Call(SCI_GETLENGTH);
	LONG lengthDocMax = lengthDoc;
	LONG lengthPrinted = 0;

	// Requested to print selection
	if (pdlg.Flags & PD_SELECTION) {
		if (startPos > endPos) {
			lengthPrinted = endPos;
			lengthDoc = startPos;
		} else {
			lengthPrinted = startPos;
			lengthDoc = endPos;
		}

		if (lengthPrinted < 0)
			lengthPrinted = 0;
		if (lengthDoc > lengthDocMax)
			lengthDoc = lengthDocMax;
	}

	// We must substract the physical margins from the printable area
	Sci_RangeToFormat frPrint;
	frPrint.hdc = hdc;
	frPrint.hdcTarget = hdc;
	frPrint.rc.left = rectMargins.left - rectPhysMargins.left;
	frPrint.rc.top = rectMargins.top - rectPhysMargins.top;
	frPrint.rc.right = ptPage.x - rectMargins.right - rectPhysMargins.left;
	frPrint.rc.bottom = ptPage.y - rectMargins.bottom - rectPhysMargins.top;
	frPrint.rcPage.left = 0;
	frPrint.rcPage.top = 0;
	frPrint.rcPage.right = ptPage.x - rectPhysMargins.left - rectPhysMargins.right - 1;
	frPrint.rcPage.bottom = ptPage.y - rectPhysMargins.top - rectPhysMargins.bottom - 1;
	if (headerFormat.size()) {
		frPrint.rc.top += headerLineHeight + headerLineHeight / 2;
	}
	if (footerFormat.size()) {
		frPrint.rc.bottom -= footerLineHeight + footerLineHeight / 2;
	}
	// Print each page
	int pageNum = 1;
	bool printPage;
	PropSetFile propsPrint;
	propsPrint.superPS = &props;
	SetFileProperties(propsPrint);

	while (lengthPrinted < lengthDoc) {
		printPage = (!(pdlg.Flags & PD_PAGENUMS) ||
		             ((pageNum >= pdlg.nFromPage) && (pageNum <= pdlg.nToPage)));

		char pageString[32];
		sprintf(pageString, "%0d", pageNum);
		propsPrint.Set("CurrentPage", pageString);

		if (printPage) {
			::StartPage(hdc);

			if (headerFormat.size()) {
				GUI::gui_string sHeader = GUI::StringFromUTF8(propsPrint.GetExpanded("print.header.format").c_str());
				::SetTextColor(hdc, sdHeader.ForeAsLong());
				::SetBkColor(hdc, sdHeader.BackAsLong());
				::SelectObject(hdc, fontHeader);
				UINT ta = ::SetTextAlign(hdc, TA_BOTTOM);
				RECT rcw = {frPrint.rc.left, frPrint.rc.top - headerLineHeight - headerLineHeight / 2,
				            frPrint.rc.right, frPrint.rc.top - headerLineHeight / 2};
				rcw.bottom = rcw.top + headerLineHeight;
				::ExtTextOutW(hdc, frPrint.rc.left + 5, frPrint.rc.top - headerLineHeight / 2,
				             ETO_OPAQUE, &rcw, sHeader.c_str(),
				             static_cast<int>(sHeader.length()), NULL);
				::SetTextAlign(hdc, ta);
				HPEN pen = ::CreatePen(0, 1, sdHeader.ForeAsLong());
				HPEN penOld = static_cast<HPEN>(::SelectObject(hdc, pen));
				::MoveToEx(hdc, frPrint.rc.left, frPrint.rc.top - headerLineHeight / 4, NULL);
				::LineTo(hdc, frPrint.rc.right, frPrint.rc.top - headerLineHeight / 4);
				::SelectObject(hdc, penOld);
				::DeleteObject(pen);
			}
		}

		frPrint.chrg.cpMin = lengthPrinted;
		frPrint.chrg.cpMax = lengthDoc;

		lengthPrinted = wEditor.Call(SCI_FORMATRANGE,
		                           printPage,
		                           reinterpret_cast<LPARAM>(&frPrint));

		if (printPage) {
			if (footerFormat.size()) {
				GUI::gui_string sFooter = GUI::StringFromUTF8(propsPrint.GetExpanded("print.footer.format").c_str());
				::SetTextColor(hdc, sdFooter.ForeAsLong());
				::SetBkColor(hdc, sdFooter.BackAsLong());
				::SelectObject(hdc, fontFooter);
				UINT ta = ::SetTextAlign(hdc, TA_TOP);
				RECT rcw = {frPrint.rc.left, frPrint.rc.bottom + footerLineHeight / 2,
				            frPrint.rc.right, frPrint.rc.bottom + footerLineHeight + footerLineHeight / 2};
				::ExtTextOutW(hdc, frPrint.rc.left + 5, frPrint.rc.bottom + footerLineHeight / 2,
				             ETO_OPAQUE, &rcw, sFooter.c_str(),
				             static_cast<int>(sFooter.length()), NULL);
				::SetTextAlign(hdc, ta);
				HPEN pen = ::CreatePen(0, 1, sdFooter.ForeAsLong());
				HPEN penOld = static_cast<HPEN>(::SelectObject(hdc, pen));
				::SetBkColor(hdc, sdFooter.ForeAsLong());
				::MoveToEx(hdc, frPrint.rc.left, frPrint.rc.bottom + footerLineHeight / 4, NULL);
				::LineTo(hdc, frPrint.rc.right, frPrint.rc.bottom + footerLineHeight / 4);
				::SelectObject(hdc, penOld);
				::DeleteObject(pen);
			}

			::EndPage(hdc);
		}
		pageNum++;

		if ((pdlg.Flags & PD_PAGENUMS) && (pageNum > pdlg.nToPage))
			break;
	}

	wEditor.Call(SCI_FORMATRANGE, FALSE, 0);

	::EndDoc(hdc);
	::DeleteDC(hdc);
	DeleteFontObject(fontHeader);
	DeleteFontObject(fontFooter);
}

void SciTEWin::PrintSetup() {
	PAGESETUPDLG pdlg = {
	                        sizeof(PAGESETUPDLG), 0, 0, 0, 0, {0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0
	                    };

	pdlg.hwndOwner = MainHWND();
	pdlg.hInstance = hInstance;

	if (pagesetupMargin.left != 0 || pagesetupMargin.right != 0 ||
	        pagesetupMargin.top != 0 || pagesetupMargin.bottom != 0) {
		pdlg.Flags = PSD_MARGINS;

		pdlg.rtMargin.left = pagesetupMargin.left;
		pdlg.rtMargin.top = pagesetupMargin.top;
		pdlg.rtMargin.right = pagesetupMargin.right;
		pdlg.rtMargin.bottom = pagesetupMargin.bottom;
	}

	pdlg.hDevMode = hDevMode;
	pdlg.hDevNames = hDevNames;

	if (!PageSetupDlg(&pdlg))
		return;

	pagesetupMargin.left = pdlg.rtMargin.left;
	pagesetupMargin.top = pdlg.rtMargin.top;
	pagesetupMargin.right = pdlg.rtMargin.right;
	pagesetupMargin.bottom = pdlg.rtMargin.bottom;

	hDevMode = pdlg.hDevMode;
	hDevNames = pdlg.hDevNames;
}

// This is a reasonable buffer size for dialog box text conversions
#define CTL_TEXT_BUF /* 512 */ 1024 //!-change-[TextSizeMax for Dialog]

class Dialog {
	HWND hDlg;
public:

	Dialog(HWND hDlg_) : hDlg(hDlg_) {
	}

	HWND Item(int id) {
		return ::GetDlgItem(hDlg, id);
	}

	void Enable(int id, bool enable) {
		::EnableWindow(Item(id), enable);
	}

	GUI::gui_string ItemTextG(int id) {
		HWND wT = Item(id);
		int len = ::GetWindowTextLengthW(wT) + 1;
		std::vector<GUI::gui_char> itemText(len);
		if (::GetDlgItemTextW(hDlg, id, &itemText[0], len)) {
			return GUI::gui_string(&itemText[0]);
		} else {
			return GUI::gui_string();
		}
	}

	void SetItemText(int id, const GUI::gui_char *s) {
		::SetDlgItemTextW(hDlg, id, s);
	}

	// Handle Unicode controls (assume strings to be UTF-8 on Windows NT)

	SString ItemTextU(int id) {
		SString s = GUI::UTF8FromString(ItemTextG(id).c_str()).c_str();
		return s;
	}

	void SetItemTextU(int id, const SString &s) {
		SetItemText(id, GUI::StringFromUTF8(s.c_str()).c_str());
	}

	void SetCheck(int id, bool value) {
		::SendMessage(::GetDlgItem(hDlg, id), BM_SETCHECK,
			value ? BST_CHECKED : BST_UNCHECKED, 0);
	}

	bool Checked(int id) {
		return BST_CHECKED == ::SendMessage(::GetDlgItem(hDlg, id), BM_GETCHECK, 0, 0);
	}

};


void SciTEWin::UIClosed() {
	SciTEBase::UIClosed();
	props.Set("Replacements", "");
	SizeSubWindows();
	WindowSetFocus(wEditor);
}

// Set a call back with the handle after init to set the path.
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/callbackfunctions/browsecallbackproc.asp

static int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM, LPARAM pData) {
	if (uMsg == BFFM_INITIALIZED) {
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
	}
	return 0;
}
int SciTEWin::PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter){
	SelectionIntoProperties();
	CollapseOutput();

	SString findInput;
	long flags = 0;
	if (props.Get("find.input").length()) {
		findInput = props.GetNewExpand("find.input");
		flags += jobHasInput;
	}

	SString findCommand = props.GetNewExpand("find.command");
	if (findCommand == "") {
		// Call InternalGrep in a new thread
		// searchParams is "(w|~)(c|~)(d|~)(b|r|~)(s|~)\0files\0text"
		// A "w" indicates whole word, "c" case sensitive, "d" dot directories, "b" binary files
		
		if (jobQueue.IsExecuting()){	 
			jobQueue.SetContinueSearch(false);
			return -1;
		}
		SString searchParams;

		searchParams.append(sParams);
		searchParams.append("\0", 1);
		searchParams.append(filter);
		searchParams.append("\0", 1);
		searchParams.append(findWhat);
		AddCommand(searchParams, directory, jobGrep, findInput, flags);
	}
	else {
		//AddCommand(findCommand,
		//	props.Get("find.directory"),
		//	jobCLI, findInput, flags);
		
		return -1;

	}
	if (jobQueue.commandCurrent > 0) {
		MakeOutputVisible(wFindRes);
		jobQueue.SetContinueSearch(true);
		Execute();
	}
	return 1;
}

BOOL SciTEWin::TabSizeMessage(HWND hDlg, UINT message, WPARAM wParam) {
	switch (message) {

	case WM_INITDIALOG: {
			LocaliseDialog(hDlg);
			::SendDlgItemMessage(hDlg, IDTABSIZE, EM_LIMITTEXT, 2, 1);
			int tabSize = wEditor.Call(SCI_GETTABWIDTH);
			if (tabSize > 99)
				tabSize = 99;
			char tmp[3];
			sprintf(tmp, "%d", tabSize);
			::SetDlgItemTextA(hDlg, IDTABSIZE, tmp);

			::SendDlgItemMessage(hDlg, IDINDENTSIZE, EM_LIMITTEXT, 2, 1);
			int indentSize = wEditor.Call(SCI_GETINDENT);
			if (indentSize > 99)
				indentSize = 99;
			sprintf(tmp, "%d", indentSize);
			::SetDlgItemTextA(hDlg, IDINDENTSIZE, tmp);

			::CheckDlgButton(hDlg, IDUSETABS, wEditor.Call(SCI_GETUSETABS));
			return TRUE;
		}

	case WM_CLOSE:
		::SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
		break;

	case WM_COMMAND:
		if (ControlIDOfCommand(wParam) == IDCANCEL) {
			::EndDialog(hDlg, IDCANCEL);
			return FALSE;
		} else if ((ControlIDOfCommand(wParam) == IDCONVERT) ||
			(ControlIDOfCommand(wParam) == IDOK)) {
			BOOL bOK;
			int tabSize = static_cast<int>(::GetDlgItemInt(hDlg, IDTABSIZE, &bOK, FALSE));
			if (tabSize > 0)
				wEditor.Call(SCI_SETTABWIDTH, tabSize);
			int indentSize = static_cast<int>(::GetDlgItemInt(hDlg, IDINDENTSIZE, &bOK, FALSE));
			if (indentSize > 0)
				wEditor.Call(SCI_SETINDENT, indentSize);
			bool useTabs = static_cast<bool>(::IsDlgButtonChecked(hDlg, IDUSETABS));
			wEditor.Call(SCI_SETUSETABS, useTabs);
			if (ControlIDOfCommand(wParam) == IDCONVERT) {
				ConvertIndentation(tabSize, useTabs);
			}
			::EndDialog(hDlg, ControlIDOfCommand(wParam));
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CALLBACK SciTEWin::TabSizeDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	return Caller(hDlg, message, lParam)->TabSizeMessage(hDlg, message, wParam);
}

void SciTEWin::TabSizeDialog() {
	DoDialog(hInstance, TEXT("TabSize"), MainHWND(), reinterpret_cast<DLGPROC>(TabSizeDlg));
	WindowSetFocus(wEditor);
}

bool SciTEWin::ParametersOpen() {
	return wParameters.Created();
}

void SciTEWin::ParamGrab() {
	if (wParameters.Created()) {
		HWND hDlg = reinterpret_cast<HWND>(wParameters.GetID());
		Dialog dlg(hDlg);
		for (int param = 0; param < maxParam; param++) {
			std::string paramVal = GUI::UTF8FromString(dlg.ItemTextG(IDPARAMSTART + param));
			SString paramText(param + 1);
			props.Set(paramText.c_str(), paramVal.c_str());
		}
	}
}

BOOL SciTEWin::ParametersMessage(HWND hDlg, UINT message, WPARAM wParam) {
	switch (message) {

	case WM_INITDIALOG: {
			LocaliseDialog(hDlg);
			wParameters = hDlg;
			Dialog dlg(hDlg);
			if (modalParameters) {
				GUI::gui_string sCommand = GUI::StringFromUTF8(parameterisedCommand.c_str());
				dlg.SetItemText(IDCMD, sCommand.c_str());
			}
			for (int param = 0; param < maxParam; param++) {
				SString paramText(param + 1);
				SString paramTextVal = props.Get(paramText.c_str());
				GUI::gui_string sVal = GUI::StringFromUTF8(paramTextVal.c_str());
				dlg.SetItemText(IDPARAMSTART + param, sVal.c_str());
			}
		}
		return TRUE;

	case WM_CLOSE:
		::SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
		break;

	case WM_COMMAND:
		if (ControlIDOfCommand(wParam) == IDCANCEL) {
			::EndDialog(hDlg, IDCANCEL);
			if (!modalParameters) {
				wParameters.Destroy();
			}
			return FALSE;
		} else if (ControlIDOfCommand(wParam) == IDOK) {
			ParamGrab();
			::EndDialog(hDlg, IDOK);
			if (!modalParameters) {
				wParameters.Destroy();
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CALLBACK SciTEWin::ParametersDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	return Caller(hDlg, message, lParam)->ParametersMessage(hDlg, message, wParam);
}

bool SciTEWin::ParametersDialog(bool modal) {
	if (wParameters.Created()) {
		ParamGrab();
		if (!modal) {
			wParameters.Destroy();
		}
		return true;
	}
	bool success = false;
	modalParameters = modal;
	if (modal) {
		success = DoDialog(hInstance,
		                   TEXT("PARAMETERS"),
		                   MainHWND(),
		                   reinterpret_cast<DLGPROC>(ParametersDlg)) == IDOK;
		wParameters = 0;
		WindowSetFocus(wEditor);
	} else {
		::CreateDialogParam(hInstance,
		                    TEXT("PARAMETERSNONMODAL"),
		                    MainHWND(),
		                    reinterpret_cast<DLGPROC>(ParametersDlg),
		                    reinterpret_cast<LPARAM>(this));
		wParameters.Show();
	}

	return success;
}

int SciTEWin::WindowMessageBox(GUI::Window &w, const GUI::gui_string &msg, int style) {
	dialogsOnScreen++;
	int ret = ::MessageBoxW(reinterpret_cast<HWND>(w.GetID()), msg.c_str(), appName, style | MB_SETFOREGROUND);
	dialogsOnScreen--;
	return ret;
}

void SciTEWin::FindMessageBox(const SString &msg, const SString *findItem) {
	if (findItem == 0) {
		GUI::gui_string msgBuf = LocaliseMessage(msg.c_str());
		WindowMessageBox(wFindReplace.Created() ? wFindReplace : wSciTE, msgBuf, MB_OK | MB_ICONWARNING);
	} else {
		GUI::gui_string findThing = GUI::StringFromUTF8(findItem->c_str());
		GUI::gui_string msgBuf = LocaliseMessage(msg.c_str(), findThing.c_str());
		WindowMessageBox(wFindReplace.Created() ? wFindReplace : wSciTE, msgBuf, MB_OK | MB_ICONWARNING);
	}
}

LRESULT CALLBACK CreditsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_GETDLGCODE)
		return DLGC_STATIC | DLGC_WANTARROWS | DLGC_WANTCHARS;

	WNDPROC lpPrevWndProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (lpPrevWndProc)
		return ::CallWindowProc(lpPrevWndProc, hwnd, uMsg, wParam, lParam);

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL SciTEWin::AboutMessage(HWND hDlg, UINT message, WPARAM wParam) {
	switch (message) {

	case WM_INITDIALOG: {
		LocaliseDialog(hDlg);
		GUI::ScintillaWindow ss;
		HWND hwndCredits = ::GetDlgItem(hDlg, IDABOUTSCINTILLA);
		LONG_PTR subclassedProc = ::SetWindowLongPtr(hwndCredits, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CreditsWndProc));
		::SetWindowLongPtr(hwndCredits, GWLP_USERDATA, subclassedProc);
		ss.SetID(hwndCredits);
		SetAboutMessage(ss, staticBuild ? "Sc1  " : "HildiM");
		}
		return TRUE;

	case WM_CLOSE:
		::SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
		break;

	case WM_COMMAND:
		if (ControlIDOfCommand(wParam) == IDOK) {
			::EndDialog(hDlg, IDOK);
			return TRUE;
		} else if (ControlIDOfCommand(wParam) == IDCANCEL) {
			::EndDialog(hDlg, IDCANCEL);
			return FALSE;
		}
	}

	return FALSE;
}

BOOL CALLBACK SciTEWin::AboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	return Caller(hDlg, message, lParam)->AboutMessage(hDlg, message, wParam);
}

void SciTEWin::AboutDialogWithBuild(int staticBuild_) {
	staticBuild = staticBuild_;
	DoDialog(hInstance, TEXT("About"), MainHWND(),
	         reinterpret_cast<DLGPROC>(AboutDlg));
	WindowSetFocus(wEditor);
}
