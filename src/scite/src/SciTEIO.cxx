// SciTE - Scintilla based Text Editor
/** @file SciTEIO.cxx
 ** Manage input and output with the system.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <map>
#include <regex>

#if defined(GTK)

#include <unistd.h>
#include <gtk/gtk.h>

#else

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
#endif
#include <commctrl.h>

// For chdir
#ifdef _MSC_VER
#include <direct.h>
#endif

#endif

#include "Scintilla.h"

#include "GUI.h"

#include "SString.h"
#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "SciTE.h"
#include "Mutex.h"
#include "JobQueue.h"
#include "SciTEBase.h"
#include "Utf8_16.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
using namespace Scintilla;

#ifdef __unix__
const GUI::gui_char propUserFileName[] = GUI_TEXT(".SciTEUser.properties");
#else
// Windows
const GUI::gui_char propUserFileName[] = GUI_TEXT("SciTEUser.properties");
#endif
const GUI::gui_char propGlobalFileName[] = GUI_TEXT("SciTEGlobal.properties");
const GUI::gui_char propAbbrevFileName[] = GUI_TEXT("abbrev.properties");

#define PROPERTIES_EXTENSION	".properties"

static bool IsPropertiesFile(const FilePath &filename) {
	FilePath ext = filename.Extension();
	if (EqualCaseInsensitive(ext.AsUTF8().c_str(), PROPERTIES_EXTENSION + 1))
		return true;
	return false;
}

void SciTEBase::SetFileName(FilePath openName, bool fixCase, bool setCaption) {
	if (openName.AsInternal()[0] == '\"') {
		// openName is surrounded by double quotes
		GUI::gui_string pathCopy = openName.AsInternal();
		pathCopy = pathCopy.substr(1, pathCopy.size() - 2);
		filePath.Set(pathCopy);
	} else {
		filePath.Set(openName);
	}

	// Break fullPath into directory and file name using working directory for relative paths
	if (!filePath.IsAbsolute()) {
		// Relative path. Since we ran AbsolutePath, we probably are here because fullPath is empty.
		filePath.SetDirectory(filePath.Directory());
	}

	if (fixCase) {
		filePath.FixName();
	}

	props.Set("FilePath", filePath.AsUTF8().c_str());
	props.Set("FileDir", filePath.Directory().AsUTF8().c_str());
	props.Set("FileName", filePath.BaseName().AsUTF8().c_str());
	props.Set("FileExt", filePath.Extension().AsUTF8().c_str());
	props.Set("FileNameExt", FileNameExt().AsUTF8().c_str());
	SetFileProperties(props);	//!-add-[FileAttr in PROPS]
	if (setCaption)
		SetWindowName();
	if (buffers.buffers)
		buffers.buffers[buffers.Current()].Set(filePath);
	if (setCaption)
		BuffersMenu();
}

// See if path exists.
// If path is not absolute, it is combined with dir.
// If resultPath is not NULL, it receives the absolute path if it exists.
bool SciTEBase::Exists(const GUI::gui_char *dir, const GUI::gui_char *path, FilePath *resultPath) {
	FilePath copy(path);
	if (!copy.IsAbsolute() && dir) {
		copy.SetDirectory(dir);
	}
	if (!copy.Exists())
		return false;
	if (resultPath) {
		resultPath->Set(copy.AbsolutePath());
	}
	return true;
}

void SciTEBase::CountLineEnds(int &linesCR, int &linesLF, int &linesCRLF) {
	linesCR = 0;
	linesLF = 0;
	linesCRLF = 0;
	Sci_Position lengthDoc = LengthDocument();
	char chPrev = ' ';
	TextReader acc(wEditor);
	char chNext = acc.SafeGetCharAt(0);
	for (int i = 0; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = acc.SafeGetCharAt(i + 1);
		if (ch == '\r') {
			if (chNext == '\n')
				linesCRLF++;
			else
				linesCR++;
		} else if (ch == '\n') {
			if (chPrev != '\r') {
				linesLF++;
			}
		}
		chPrev = ch;
	}
}

static bool isEncodingChar(char ch) {
	return (ch == '_') || (ch == '-') || (ch == '.') ||
	       (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
	       (ch >= '0' && ch <= '9');
}

static bool isSpaceChar(char ch) {
	return (ch == ' ') || (ch == '\t');
}

static SString ExtractLine(const char *buf, size_t length) {
	unsigned int endl = 0;
	if (length > 0) {
		while ((endl < length) && (buf[endl] != '\r') && (buf[endl] != '\n')) {
			endl++;
		}
		if (((endl + 1) < length) && (buf[endl] == '\r') && (buf[endl+1] == '\n')) {
			endl++;
		}
		if (endl < length) {
			endl++;
		}
	}
	return SString(buf, 0, endl);
}

static const char codingCookie[] = "coding";

static UniMode CookieValue(const SString &s) {
	intptr_t posCoding = s.search(codingCookie);
	if (posCoding >= 0) {
		posCoding += strlen(codingCookie);
		if ((s[posCoding] == ':') || (s[posCoding] == '=')) {
			posCoding++;
			if ((s[posCoding] == '\"') || (s[posCoding] == '\'')) {
				posCoding++;
			}
			while ((unsigned(posCoding) < s.length()) &&
			        (isSpaceChar(s[posCoding]))) {
				posCoding++;
			}
			size_t endCoding = unsigned(posCoding);
			while ((endCoding < s.length()) &&
			        (isEncodingChar(s[endCoding]))) {
				endCoding++;
			}
			SString code(s.c_str(), posCoding, endCoding);
			code.lowercase();
			if (code == "utf-8") {
				return uniCookie;
			}
		}
	}
	return uni8Bit;
}

static UniMode CodingCookieValue(const char *buf, size_t length) {
	SString l1 = ExtractLine(buf, length);
	UniMode unicodeMode = CookieValue(l1);
	if (unicodeMode == uni8Bit) {
		SString l2 = ExtractLine(buf + l1.length(), length - l1.length());
		unicodeMode = CookieValue(l2);
	}
	return unicodeMode;
}

void SciTEBase::DiscoverEOLSetting() {
	SetEol();
	if (props.GetInt("eol.auto")) {
		int linesCR;
		int linesLF;
		int linesCRLF;
		CountLineEnds(linesCR, linesLF, linesCRLF);
		if (((linesLF >= linesCR) && (linesLF > linesCRLF)) || ((linesLF > linesCR) && (linesLF >= linesCRLF)))
			wEditor.SetEOLMode(EndOfLine::Lf);
		else if (((linesCR >= linesLF) && (linesCR > linesCRLF)) || ((linesCR > linesLF) && (linesCR >= linesCRLF)))
			wEditor.SetEOLMode(EndOfLine::Cr);
		else if (((linesCRLF >= linesLF) && (linesCRLF > linesCR)) || ((linesCRLF > linesLF) && (linesCRLF >= linesCR)))
			wEditor.SetEOLMode(EndOfLine::CrLf);
	}
}

// Look inside the first line for a #! clue regarding the language
SString SciTEBase::DiscoverLanguage(const char *buf, size_t length) {
	SString languageOverride = "";
	SString l1 = ExtractLine(buf, length);
	if (l1.startswith("<?xml")) {
		languageOverride = "xml";
	} else if (l1.startswith("#!")) {
		l1 = l1.substr(2);
		l1.substitute('\\', ' ');
		l1.substitute('/', ' ');
		l1.substitute("\t", " ");
		l1.substitute("  ", " ");
		l1.substitute("  ", " ");
		l1.substitute("  ", " ");
		l1.remove("\r");
		l1.remove("\n");
		if (l1.startswith(" ")) {
			l1 = l1.substr(1);
		}
		l1.substitute(' ', '\0');
		const char *word = l1.c_str();
		while (*word) {
			SString propShBang("shbang.");
			propShBang.append(word);
			SString langShBang = props.GetExpanded(propShBang.c_str());
			if (langShBang.length()) {
				languageOverride = langShBang;
			}
			word += strlen(word) + 1;
		}
	}
	if (languageOverride.length()) {
		languageOverride.insert(0, "x.");
	}
	return languageOverride;
}

void SciTEBase::DiscoverIndentSetting() {
	Sci_Position lengthDoc = LengthDocument();
	TextReader acc(wEditor);
	bool newline = true;
	int indent = 0; // current line indentation
	int tabSizes[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // number of lines with corresponding indentation (index 0 - tab)
	int prevIndent = 0; // previous line indentation
	int prevTabSize = -1; // previous line tab size
	for (int i = 0; i < lengthDoc; i++) {
		char ch = acc[i];
		if (ch == '\r' || ch == '\n') {
			indent = 0;
			newline = true;
		} else if (newline && ch == ' ') {
			indent++;
		} else if (newline) {
			if (indent) {
				if (indent == prevIndent && prevTabSize != -1) {
					tabSizes[prevTabSize]++;
				} else if (indent > prevIndent && prevIndent != -1) {
					if (indent - prevIndent <= 8) {
						prevTabSize = indent - prevIndent;
						tabSizes[prevTabSize]++;
					} else {
						prevTabSize = -1;
					}
				}
				prevIndent = indent;
			} else if (ch == '\t') {
				tabSizes[0]++;
				prevIndent = -1;
			} else {
				prevIndent = 0;
			}
			newline = false;
		}
	}
	// maximum non-zero indent
	int topTabSize = -1;
	for (int j = 0; j <= 8; j++) {
		if (tabSizes[j] && (topTabSize == -1 || tabSizes[j] > tabSizes[topTabSize])) {
			topTabSize = j;
		}
	}
	// set indentation
	if (topTabSize == 0) {
		wEditor.SetUseTabs(1);
	} else if (topTabSize != -1) {
		wEditor.SetUseTabs(0);
		wEditor.SetIndent(topTabSize);
	}
}

void SciTEBase::OpenFile(int fileSize, bool suppressMessage) {
	FILE *fp = filePath.Open(fileRead);
	wEditor.SetChangeHistory(ChangeHistoryOption::Disabled);
	if (fp) {
//!		Utf8_16_Read convert; //!-remove-[utf8.auto.check]
		CurrentBuffer()->SetTimeFromFile();
		wEditor.ClearAll();
		//char data[blockSize];
		//char* data = new char[blockSize];
		std::vector<char> data(blockSize);

		size_t lenFile = fread(data.data(), 1, data.size(), fp);
		UniMode codingCookie = CodingCookieValue(data.data(), lenFile);

		Utf8_16_Read convert(codingCookie==uni8Bit);
		convert._encoding = filePath._encoding;

		wEditor.Allocate(fileSize + 1000);
		SString languageOverride;
		bool firstBlock = true;
		bBlockTextChangeNotify = true;
		while (lenFile > 0) {
			lenFile = convert.convert(data.data(), lenFile);
			char *dataBlock = convert.getNewBuf();
			if ((firstBlock) && (language == "")) {
				languageOverride = DiscoverLanguage(dataBlock, lenFile);
			}
			firstBlock = false;
			wEditor.AddText(lenFile, dataBlock);
			lenFile = fread(data.data(), 1, data.size(), fp);
		}
		//delete[] data;
		bBlockTextChangeNotify = false;
		fclose(fp);

		if (languageOverride.length()) {
			CurrentBuffer()->overrideExtension = languageOverride;
			ReadProperties();
			SetIndentSettings();
		}
		CurrentBuffer()->unicodeMode = static_cast<UniMode>(
		            static_cast<int>(convert.getEncoding()));
		// Check the first two lines for coding cookies
		if (CurrentBuffer()->unicodeMode == uni8Bit) {
			CurrentBuffer()->unicodeMode = codingCookie;
		}
		if (CurrentBuffer()->unicodeMode != uni8Bit) {
			// Override the code page if Unicode
			codePage = SC_CP_UTF8;
		} else {
			codePage = props.GetInt("code.page");
		}
		props.SetInteger("editor.unicode.mode", CurrentBuffer()->unicodeMode + IDM_ENCODING_DEFAULT); //!-add-[EditorUnicodeMode]
		wEditor.SetCodePage(codePage);
	
		DiscoverEOLSetting();
	
		if (props.GetInt("indent.auto")) {
			DiscoverIndentSetting();
		}
	} else if (!suppressMessage&& !props.GetInt("warning.couldnotopenfile.disable")) {
		std::string cmd = "print(([[";
		cmd += "Could not open file ";
		cmd += filePath.AsUTF8();
		cmd += ".]]):from_utf8())";
		extender->DoLua(cmd.c_str());
	}
	if (!wEditor.UndoCollection()) {
		wEditor.SetUndoCollection(1);
	}
	wEditor.SetChangeHistory(static_cast<ChangeHistoryOption>((viewHisoryMarkers ? (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS) : 0) 
		                                                    | (viewHisoryIndicators ? (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_INDICATORS) : 0)));

	// Flick focus to the output window and back to
	// ensure palette realised correctly.
	WindowSetFocus(wOutput);
	WindowSetFocus(wEditor);
	wEditor.SetSavePoint();

	wEditor.GotoPos(0);
	Redraw();
}

bool SciTEBase::ReadForScript(FilePath &fileCompare, void** c, char** data, size_t &lenFile) {
	FILE *fp = fileCompare.Open(fileRead);
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		*data = new char[size];
		Utf8_16_Read** convert = (Utf8_16_Read**)c;

		lenFile = fread(*data, 1, size, fp);
		UniMode codingCookie = CodingCookieValue(*data, lenFile);

		*convert = new Utf8_16_Read(codingCookie == uni8Bit);
		(*convert)->_encoding = filePath._encoding;

		lenFile = (*convert)->convert(*data, lenFile);
		fclose(fp);
		return true;
	}
	return false;
}

int SciTEBase::CompareFile(FilePath &fileCompare, const char* txtCompare) {
	FILE *fp = fileCompare.Open(fileRead);
	int result = 2;
	if (fp) {
		result = 0;
		//char data[blockSize];
		//char *data = new char[blockSize];
		std::vector<char> data(blockSize);
		size_t lenFile = fread(data.data(), 1, blockSize, fp);
		UniMode codingCookie = CodingCookieValue(data.data(), lenFile);

		Utf8_16_Read convert(codingCookie == uni8Bit );
		convert._encoding = filePath._encoding;

		size_t lenCompare = 0;
		
		while (lenFile > 0) {
			lenFile = convert.convert(data.data(), lenFile);
			char *dataBlock = convert.getNewBuf();
			if (memcmp((void*)dataBlock, (void*)(txtCompare + lenCompare), lenFile)) {
				result = 1;
				break;
			}

			lenCompare += lenFile;
			lenFile = fread(data.data(), 1, data.size(), fp);
		}
		fclose(fp);
	}
	return result;
}

bool SciTEBase::Open(FilePath file, OpenFlags of, bool setNav) {
	FilePath absPath = file.AbsolutePath();
	int encoding = 0;
	
	absPath._encoding = encoding;
	InitialiseBuffers();

	int index = buffers.GetDocumentByName(absPath);
	if (index >= 0) {
		SetDocumentAt(index);
		if (!(of & ofForceLoad)) // Just rotate into view
			return true;
	}
	// See if we can have a buffer for the file to open
	if (!CanMakeRoom(!(of & ofNoSaveIfDirty))) {
		return false;
	}

	int size = absPath.GetFileLength();
	if (size > 0) {
		// Real file, not empty buffer
		int maxSize = props.GetInt("max.file.size");
		if (maxSize > 0 && size > maxSize) {
			GUI::gui_string sSize = GUI::StringFromInteger(size);
			GUI::gui_string sMaxSize = GUI::StringFromInteger(maxSize);
			int answer = extender->HildiAlarm("File '%1' is %2 bytes long,\nlarger than the %3 bytes limit set in the properties.\nDo you still want to open it?",
				MB_YESNO | MB_ICONWARNING, absPath.AsInternal(), sSize.c_str(), sMaxSize.c_str());
			if (answer != IDYES) {
				return false;
			}
		}
	}
	if (extender && extender->OnBeforeOpen(absPath.AsUTF8().c_str(), file.Extension().AsUTF8().c_str(), encoding)) return false;
	absPath._encoding = encoding;
	if(setNav)
		extender->OnNavigation("Open");
	
	layout.OnOpenClose(buffers.buffers[buffers.Current()].editorSide);

	bBlockRedraw = true;
	if (buffers.size == buffers.length) {
		AddFileToStack(filePath, GetSelection(), GetCurrentScrollPosition());
		ClearDocument();
		if (extender)
			extender->InitBuffer(buffers.Current());
	} else {
		if (index < 0 || !(of & ofForceLoad)) { // No new buffer, already opened
			index = buffers.length;
			New();
		}
	}
	
	SetFileName(absPath);
	CurrentBuffer()->overrideExtension = "";
	wEditor.SetBuffPointer(&absPath);
	ReadProperties();
	wEditor.SetScrollWidthTracking(1);
	wEditor.SetScrollWidth(100);
	SetIndentSettings();
	SetEol();
	UpdateBuffersCurrent();

	if (!filePath.IsUntitled()) {
		wEditor.SetReadOnly(0);
		wEditor.Cancel();
		if (of & ofPreserveUndo) {
			wEditor.BeginUndoAction();
		} else {
			wEditor.SetUndoCollection(0);
		}

		OpenFile(size, of & ofQuiet);

		if (of & ofPreserveUndo) {
			wEditor.EndUndoAction();
		} else {
			wEditor.EmptyUndoBuffer();
		}
		SString atr = props.Get("FileAttr");
		bool isReadOnly = atr.contains("H") || atr.contains("S") || atr.contains("R");
		wEditor.SetReadOnly(isReadOnly); 
	}
	RemoveFileFromStack(filePath);
	SetWindowName();
	if (lineNumbers && lineNumbersExpand)
		SetLineNumberWidth();
	bBlockRedraw = false;

	BuffersMenu();
	if (extender)
		extender->OnOpen(filePath.AsUTF8().c_str());
	if (setNav)
		extender->OnNavigation("Open-");
	return true;
}

void SciTEBase::Revert() {
	RecentFile rf = GetFilePosition();
	
	Open(filePath,ofForceLoad);
	DisplayAround(rf);
}

void SciTEBase::CheckReload() {
	if (props.GetInt("load.on.activate")) {
		// Make a copy of fullPath as otherwise it gets aliased in Open
		std::string fp = filePath.AsUTF8();
		
		if(fp.substr(0, 2) == "\\\\")
			return;
		time_t newModTime = filePath.ModifiedTime();
		DWORD attr = ::GetFileAttributesW(filePath.AsInternal());
		bool isRO = (attr & FILE_ATTRIBUTE_READONLY) || (attr & FILE_ATTRIBUTE_SYSTEM) || (attr & FILE_ATTRIBUTE_HIDDEN);

		if (filePath.Exists() && buffers.CurrentBuffer()->ROMarker != isRO) {
			wEditor.SetReadOnly(isRO);
			BuffersMenu();
		}

		if (newModTime != CurrentBuffer()->fileModTime) {
			if (newModTime != 0) {
				RecentFile rf = GetFilePosition();
				OpenFlags of = props.GetInt("reload.preserves.undo") ? ofPreserveUndo : ofNone;
				if (CurrentBuffer()->isDirty || props.GetInt("are.you.sure.on.reload") != 0) {
					if ((0 == dialogsOnScreen) && (newModTime != CurrentBuffer()->fileModLastAsk)) {
						GUI::gui_string msg;
						int decision;
						if (CurrentBuffer()->isDirty) {
							decision = extender->HildiAlarm("The file \n'%1'\n has been modified. Should it be reloaded?",
								MB_YESNO, filePath.AsInternal());
						} else {
							decision = extender->HildiAlarm("The file \n'%1'\n has been modified outside HildiM. Should it be reloaded?",
								MB_YESNO, filePath.AsInternal());
						}
						if (decision == IDYES) {
							//Open(filePath, static_cast<OpenFlags>(of | ofForceLoad));
							Open(filePath, static_cast<OpenFlags>(ofForceLoad));
							DisplayAround(rf);
						}
						CurrentBuffer()->fileModLastAsk = newModTime;
					}
				} else {
					Open(filePath, static_cast<OpenFlags>(of | ofForceLoad));
					DisplayAround(rf);
				}
			} else {
				SetWindowName();
				BuffersMenu();
			}
		}
	}
}

void SciTEBase::Activate(bool activeApp) {
	if (activeApp) {
		::PostMessage(reinterpret_cast<HWND>(GetID()), SCI_POSTPONECHECKRELOAD, 0, 0);
	} else {
		if (props.GetInt("save.on.deactivate")) {
			SaveTitledBuffers();
		}
	}
}

FilePath SciTEBase::SaveName(const char *ext) {
	GUI::gui_string savePath = filePath.AsInternal();
	if (ext) {
		size_t dot = savePath.length() - 1;
		while ((dot >= 0) && (savePath[dot] != '.')) {
			dot--;
		}
		if (dot >= 0) {
			int keepExt = props.GetInt("export.keep.ext");
			if (keepExt == 0) {
				savePath.erase(dot);
			} else if (keepExt == 2) {
				savePath[dot] = '_';
			}
		}
		savePath += GUI::StringFromUTF8(ext);
	}
	//~ fprintf(stderr, "SaveName <%s> <%s> <%s>\n", filePath.AsInternal(), savePath.c_str(), ext);
	return FilePath(savePath.c_str());
}

int SciTEBase::SaveIfUnsure(bool forceQuestion, bool forNewRoom) {
	if (buffers.CurrentBuffer()->pFriend)
		return IDOK;
	if (forNewRoom && (!CurrentBuffer()->isDirty || filePath.IsTemporaly() )) {
		int decision;
		GUI::gui_char cnt[9];
		swprintf(cnt, 9, L"%0d", buffers.length);
		decision = extender->HildiAlarm("%1 files have already been opened.\nClose '%2'?",
			MB_OKCANCEL, cnt, filePath.Name().AsInternal());
		if (decision == IDCANCEL)
			return IDCANCEL;
			
		extender->OnClose(filePath.AsUTF8().c_str());
	}
	if ((CurrentBuffer()->isDirty) && (LengthDocument() || !filePath.IsUntitled() || forceQuestion)) {
		if ((props.GetInt("are.you.sure", 1) && props.GetInt("are.you.sure.close", 1)) ||
		        filePath.IsUntitled() ||
		        forceQuestion) {
			int decision;
			if (!filePath.IsUntitled()) {
				if (forNewRoom) {
					GUI::gui_char cnt[9];
					swprintf(cnt, 9, L"%0d", buffers.length);
					decision = extender->HildiAlarm("%1 files have already been opened.\nSave changes to '%2' and close it?",
						MB_YESNOCANCEL, cnt, filePath.AsInternal());
					if (decision != IDCANCEL)
						extender->OnClose(filePath.AsUTF8().c_str());
				}
				else {
					decision = extender->HildiAlarm("Save changes to '%1'?",
						MB_YESNOCANCEL, filePath.AsInternal());
				}
			} else {
				decision = extender->HildiAlarm("Save changes to (Untitled)?",
					MB_YESNOCANCEL, filePath.AsInternal());
			}
			if (decision == IDYES) {
				if (!Save())
					decision = IDCANCEL;
			}
			return decision;
		} else if (props.GetInt("are.you.sure.close", 1)){
			if (!Save())
				return IDCANCEL;
		}
	}
	return IDYES;
}

int SciTEBase::SaveIfUnsureForBuilt() {
	if (props.GetInt("save.all.for.build")) {
		return SaveAllBuffers(false, !props.GetInt("are.you.sure.for.build"));
	}
//!	if (CurrentBuffer()->isDirty) {
	if (!props.GetInt("are.you.sure.close", 1)) return IDYES;
	if (CurrentBuffer()->DocumentNotSaved()) { //-change-[OpenNonExistent]
		if (props.GetInt("are.you.sure.for.build"))
			return SaveIfUnsure(true);

		Save();
	}
	return IDYES;
}

void SciTEBase::StripTrailingSpaces() {
	Sci_Position maxLines = wEditor.LineCount();
	for (Sci_Position line = 0; line < maxLines; line++) {
		Sci_Position lineStart = wEditor.PositionFromLine(line);
		Sci_Position lineEnd = wEditor.LineEndPosition(line);
		Sci_Position i = lineEnd - 1;
		char ch = static_cast<char>(wEditor.CharAt(i));
		while ((i >= lineStart) && ((ch == ' ') || (ch == '\t'))) {
			i--;
			ch = static_cast<char>(wEditor.CharAt(i));
		}
		if (i < (lineEnd - 1)) {
			wEditor.SetTargetStart(i + 1);
			wEditor.SetTargetEnd(lineEnd);
			wEditor.ReplaceTarget(0, "");
		}
	}
}

void SciTEBase::EnsureFinalNewLine() {
	Sci_Position maxLines = wEditor.LineCount();
	bool appendNewLine = maxLines == 1;
	Sci_Position endDocument = wEditor.PositionFromLine(maxLines);
	if (maxLines > 1) {
		appendNewLine = endDocument > wEditor.PositionFromLine(maxLines - 1);
	}
	if (appendNewLine) {
		const char *eol = "\n";
		switch (wEditor.EOLMode()) {
		case EndOfLine::CrLf:
			eol = "\r\n";
			break;
		case EndOfLine::Cr:
			eol = "\r";
			break;
		}
		wEditor.InsertText(endDocument, eol);
	}
}

/**
 * Writes the buffer to the given filename.
 */
bool SciTEBase::SaveBuffer(FilePath saveName, bool bNotSaveNotChanged) {
	bool retVal = false;
	bBlockTextChangeNotify = true;
	// Perform clean ups on text before saving
	SetFileProperties(props);	//!-add-[FileAttr in PROPS]
	wEditor.BeginUndoAction();
	if (props.GetInt("strip.trailing.spaces"))
		StripTrailingSpaces();
	if (props.GetInt("ensure.final.line.end"))
		EnsureFinalNewLine();
	if (props.GetInt("ensure.consistent.line.ends"))
		wEditor.ConvertEOLs(wEditor.EOLMode());

	wEditor.EndUndoAction();
	bool bNotSaved = buffers.CurrentBuffer()->DocumentNotSaved();

	if (extender)
		retVal = extender->OnBeforeSave(saveName.AsUTF8().c_str());

	if (!retVal) {
		Utf8_16_Write convert;
		if (CurrentBuffer()->unicodeMode != uniCookie) {	// Save file with cookie without BOM.
			convert.setEncoding(static_cast<Utf8_16::encodingType>(
				    static_cast<int>(CurrentBuffer()->unicodeMode)));
		}

		if (bNotSaved || !bNotSaveNotChanged) {
			FILE *fp = saveName.Open(fileWrite);
			if (fp) {
				convert.setfile(fp);
				convert._encoding = saveName._encoding;
				char data[blockSize + 1];
				Sci_Position lengthDoc = LengthDocument();
				retVal = true;
				Sci_Position grabSize;
				for (Sci_Position i = 0; i < lengthDoc; i += grabSize) {
					grabSize = lengthDoc - i;
					if (grabSize > blockSize)
						grabSize = blockSize;
					// Round down so only whole characters retrieved.
					grabSize = wEditor.PositionBefore(i + grabSize + 1) - i;
					GetRange(wEditor, i, i + grabSize, data);
					size_t written = convert.fwrite(data, grabSize);
					if (written == 0) {
						retVal = false;
						break;
					}
				}
				convert.fclose();
			}
		} else {
			retVal = true; //Не смогли сохранить несохраненный - наплюем
			if (!props.GetInt("save.scip.alert")) {
				std::string cmd = "print(([[";
				//cmd += "File ";
				cmd += saveName.AsUTF8();
				cmd += "]]):from_utf8()..' is not changed')";
				extender->DoLua(cmd.c_str());
			}
		}

		if (retVal && extender) {
			extender->OnSave(saveName.AsUTF8().c_str());
		}	
	}
	bBlockTextChangeNotify = false;
	return retVal;
}

void SciTEBase::ReloadProperties() {
	wEditorL.languageCurrent = "xxx";
	wEditorR.languageCurrent = "xxx"; //чтобы потом перечиталось на обоих редакторах
	ReadGlobalPropFile(L"");
	firstPropertiesRead = true;
	ReadProperties();
	SetWindowName();
	//BuffersMenu();
	Redraw();
}

// Returns false if cancelled or failed to save
bool SciTEBase::Save(bool bNotSaveNotChanged) {
	if (!filePath.IsUntitled()) {
		int decision;

		if (props.GetInt("save.deletes.first")) {
			filePath.Remove();
		} else if (props.GetInt("save.check.modified.time")) {
			time_t newModTime = filePath.ModifiedTime();
			if ((newModTime != 0) && (CurrentBuffer()->fileModTime != 0) &&
				(newModTime != CurrentBuffer()->fileModTime)) {
				decision = extender->HildiAlarm("The file \n'%1'\n has been modified outside HildiM. Should it be saved?",
					MB_YESNO | MB_ICONWARNING,  filePath.AsInternal());
				if (decision == IDNO) {
					return false;
				}
			}
		}

		if (SaveBuffer(filePath, bNotSaveNotChanged)) {
			CurrentBuffer()->SetTimeFromFile();
			wEditor.SetSavePoint();
			if (IsPropertiesFile(filePath)) {
				ReloadProperties();
			}
		} else {
			decision = extender->HildiAlarm("Could not save file\n'%1'.\nSave under a different name?",
				MB_YESNO | MB_ICONWARNING, filePath.AsInternal());
			if (decision == IDYES) {
				return SaveAsDialog();
			}
			return false;
		}
		//BuffersMenu();
		return true;
	} else {
		return SaveAsDialog();
	}
}

void SciTEBase::SaveAs(const GUI::gui_char *file, bool fixCase) {
	SetFileName(file, fixCase);
	Save();
	ReadProperties();
	wEditor.ClearDocumentStyle();
	wEditor.Colourise(0, -1);
	Redraw();
	SetWindowName();
	BuffersMenu();

}

bool SciTEBase::SaveIfNotOpen(const FilePath &destFile, bool fixCase) {
	FilePath absPath = destFile.AbsolutePath();
	int index = buffers.GetDocumentByName(absPath, true /* excludeCurrent */);
	if (index >= 0) {
		extender->HildiAlarm("File '%1' is already open in another buffer.",
			MB_OK | MB_ICONWARNING, destFile.AsInternal());
		return false;
	} else {
		SaveAs(absPath.AsInternal(), fixCase);
		return true;
	}
}

class BufferedFile {
	FILE *fp;
	bool readAll;
	bool exhausted;
	enum {bufLen = 64 * 1024};
	char buffer[bufLen];
	size_t pos;
	size_t valid;
	void EnsureData() {
		if (pos >= valid) {
			if (readAll || !fp) {
				exhausted = true;
			} else {
				valid = fread(buffer, 1, bufLen, fp);
				if (valid < bufLen) {
					readAll = true;
				}
				pos = 0;
			}
		}
	}
public:
	BufferedFile(FilePath fPath) :buffer() {
		fp = fPath.Open(fileRead);
		readAll = false;
		exhausted = fp == NULL;
		pos = 0;
		valid = 0;
	}
	~BufferedFile() {
		if (fp) {
			fclose(fp);
		}
		fp = NULL;
	}
	bool Exhausted() {
		return exhausted;
	}
	int NextByte() {
		EnsureData();
		if (pos >= valid) {
			return 0;
		}
		return buffer[pos++];
	}
	bool BufferContainsNull() {
		EnsureData();
		for (size_t i = 0;i < valid;i++) {
			if (buffer[i] == '\0')
				return true;
		}
		return false;
	}
	char *getTop(){
		EnsureData();
		if (valid > 3) return buffer;
		return 0;
	}
};

class FileReader {
	BufferedFile *bf;
	int lineNum;
	bool lastWasCR;
	enum {bufLen = 1000};
/*!
	char lineToCompare[bufLen+1];
	char lineToShow[bufLen+1];
*/
//!-start-[FileReaderUnlimitedLen]
	char buf[bufLen+1];
	SString lineToCompare;
	SString lineToShow;
//!-end-[FileReaderUnlimitedLen]
	bool caseSensitive;
	Utf8_16_Read *convert;
	Utf8_16::encodingType encType;
	bool isEncoding = false;
public:
	int m_LineLen = 0;
	FileReader(FilePath fPath, bool caseSensitive_) {
		bf = new BufferedFile(fPath);
		lineNum = 0;
		lastWasCR = false;
		caseSensitive = caseSensitive_;
		convert = new Utf8_16_Read(true);
		encType = Utf8_16::encodingType::eUnknown;
	}
	~FileReader() {
		delete bf;
		delete convert;
		bf = NULL;
	}
	void Encode(char *buf, int i){
		size_t l = 0;
		if (!lineNum || encType != Utf8_16::encodingType::eUnknown) {
			l = convert->convert(buf, i + 1, true);
			if (!isEncoding && i > 0) {
				encType = convert->getEncoding();
				isEncoding = true;
			}
		}
		if (encType == Utf8_16::encodingType::eUnknown) {
			lineToShow += buf;
			if (!caseSensitive) CharLowerBuffA(buf, i);
			lineToCompare += buf;
			return;
		}
		char *chr = convert->getNewBuf();									 
		chr[l] = '\0';
		lineToShow += GUI::ConvertFromUTF8(chr, CP_ACP).c_str();
		if (!caseSensitive) lineToCompare += GUI::ConvertFromUTF8(GUI::UTF8ToLower(chr), CP_ACP).c_str();
		else lineToCompare += lineToShow;
	}
	char *Next() {
		if (bf->Exhausted()) {
			return NULL;
		}

		lineToCompare.clear();
		lineToShow.clear();

		int i = 0;
		m_LineLen = 0;
		while (!bf->Exhausted()) {
			m_LineLen++;
			int ch = bf->NextByte();
			if (i == 0 && lastWasCR && ch == '\n') {
				lastWasCR = false;
				ch = 0;
			}
			else if (i == 0 && ch == '\0' && (m_LineLen == 1 || convert->getEncoding() == Utf8_16::encodingType::eUtf16LittleEndian)) {
				ch = 0;
			}
			else if (ch == '\r' || ch == '\n') {
				lastWasCR = ch == '\r';
				break;
			} else {
				buf[i++] = static_cast<char>(ch);
				if (i == bufLen) {
					buf[i] = '\0';
					Encode(buf, i);
					i = 0;
				}
			}
		}
		buf[i] = '\0';
		Encode(buf, i);

		lineNum++;		

		//lineToCompare = lineToShow;
		//if (!caseSensitive) {
		//	lineToCompare.lowercase();	 
		//}
		return const_cast<char *>(lineToCompare.c_str());

	}
	int LineNumber() const {
		return lineNum;
	}
	const char *Original() {

		return const_cast<char *>(lineToShow.c_str()); //!-change-[FileReaderUnlimitedLen]
	}
	bool BufferContainsNull() {
		char *top = bf->getTop();
		if (top){
			convert->convert(top, strlen(top), true);
			if (convert->getEncoding() == Utf8_16::encodingType::eUtf16BigEndian || convert->getEncoding() == Utf8_16::encodingType::eUtf16LittleEndian) return false;
		}
		return bf->BufferContainsNull();
	}
};

static bool IsWordCharacter(int ch, const char *chSet) {		
	return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || (ch == '_')) || static_cast<bool>(strchr(chSet, ch));
}

bool SciTEBase::strstrRegExp(char *text, const char *sub, void *pRegExp, GrepFlags gf, const char* charsAccented)
{//При инициализированной luaState использует поиск regex_search для определения индекса первого вхождения подстроки в текст
	if (!pRegExp) {
		char *lineEnd = text + strlen(text);
		char *match = strstr(text, sub); 
		if (!(gf & grepWholeWord)) return match ? true : false;

		size_t searchLength = strlen(sub);
		bool bLeft = !IsWordCharacter(sub[0], charsAccented);
		bool bRight = !IsWordCharacter(sub[lstrlenA(sub) - 1], charsAccented);
		while (match) {
			if (
				((match == text) || (bLeft || !IsWordCharacter(match[-1], charsAccented))) &&
				((match + searchLength == (lineEnd)) || (bRight || !IsWordCharacter(match[searchLength], charsAccented)))
				
				) {
				return true;
			}
			match = strstr(match + 1, sub);
		}
		return false;
	}
	std::regex *pReg = (std::regex*) pRegExp;
	return std::regex_search(text, *pReg);

}

void SciTEBase::CountRecursive(GrepFlags gf, FilePath baseDir, const GUI::gui_char *fileTypes, GrepOut *grepOut){
	FilePathSet directories;
	FilePathSet files;
	baseDir.List(directories, files, fileTypes);

	grepOut->iFiles += static_cast<int>(files.Length());
	if (gf & grepSubDir){
		for (size_t j = 0; (j < directories.Length() && jobQueue.ContinueSearch()); j++) {
			FilePath fPath = directories.At(j);
			if ((gf & grepDot) || (fPath.Name().AsInternal()[0] != '.')) {
				CountRecursive(gf, fPath, fileTypes, grepOut);
			}
		}
	}
}

static bool LessStdNoCase(std::string a, std::string b){
	return _stricmp(a.c_str(), b.c_str()) < 0; 
}

void SciTEBase::GrepRecursive(GrepFlags gf, FilePath baseDir, const char *searchString, const GUI::gui_char *fileTypes, size_t basePath, GrepOut *grepOut, std::regex *pRegExp){
	FilePathSet directories;
	FilePathSet files;
	baseDir.List(directories, files, fileTypes);
	size_t searchLength = strlen(searchString);
	std::vector<std::string> v;
			
			//lua_State *luaState = (lua_State*)pluaState;
	for (size_t i = 0; (i < files.Length()) && jobQueue.ContinueSearch(true); i++) {
		FilePath fPath = files.At(i);
		grepOut->iFiles += 1;
		bool bFindInFiles = false;

		std::string os;
	
		FileReader fr(fPath, gf & grepMatchCase);
		if ((gf & grepBinary) ||  !fr.BufferContainsNull()) {					  			
			while (char *line = fr.Next()) {
				if (strstrRegExp(line, searchString, (gf & grepRegExp) ? pRegExp : 0, gf, props.GetExpanded("chars.accented").c_str())) {
					grepOut->iLines += 1;
					if (!bFindInFiles) {
						if (gf & grepGroup){
							os.append(" ");	  
							os.append(GUI::ConvertFromUTF8(fPath.AsUTF8(), CP_ACP).c_str());
							os.append("\n");
						}
						grepOut->iInFiles += 1;
						bFindInFiles = true;
					}
					if (gf & grepGroup){
						os.append("\t");
					} 
					else {
						os.append(".");
						os.append(GUI::ConvertFromUTF8(fPath.AsUTF8(), CP_ACP).c_str() + basePath);
						os.append(":");
					}
					SString lNumber(fr.LineNumber());
					os.append(lNumber.c_str());
					os.append(":");

					lNumber = fr.Original();
					lNumber.substitute('\t',' ');
					lNumber.trimleft("\n\r ");
					lNumber.insert(0," ",1);
					while ( lNumber.substitute("  "," ") );
					os.append(lNumber.c_str());

					os.append("\n");
				}
			}
		}		
		if (os.length()) {
			if (gf & grepStdOut) {
				fwrite(os.c_str(), os.length(), 1, stdout);
			} else {
				v.emplace_back(os);
			}
		}	
	}
	std::string os;
	if (v.size()){
		std::sort(v.begin(), v.end(), LessStdNoCase);
		for (size_t i = 0; i < v.size(); i++)
		{
			os += v[i].c_str();
		}
		v.clear();
	}

	if (gf & grepSubDir){
		for (size_t j = 0; (j < directories.Length()) && jobQueue.ContinueSearch(); j++) {
			FilePath fPath = directories.At(j);
			if ((gf & grepDot) || (fPath.Name().AsInternal()[0] != '.')) {
				GrepRecursive(gf, fPath, searchString, fileTypes, basePath, grepOut, pRegExp);
				v.push_back(grepOut->strOut.c_str());
			}
		}
		if (v.size()){
			std::sort(v.begin(), v.end(), LessStdNoCase);
			for (size_t i = 0; i < v.size(); i++)
			{
				os += v[i].c_str();
			}
			v.clear();
		}

	}
	grepOut->strOut = os.c_str();
}

void SciTEBase::InternalGrep(GrepFlags gf, const GUI::gui_char *directory, const GUI::gui_char *fileTypes, const char *search) {
	size_t originalEnd = 0;
	GUI::ElapsedTime commandTime;
	size_t basePathLen = 0; //!-add-[FindResultListStyle]
	SString searchString;
	WideChrToMyltiBate(search, searchString); 
	SString os = "";
	if (!(gf & grepStdOut)) {
		os.append(">Search");
		if (gf & grepRegExp)
			os.append(" RE");
		if (gf & grepWholeWord)
			os.append(" WW");
		if (gf & grepMatchCase)
			os.append(" MC");
		os.append(" for \"").append(searchString.c_str()).append("\" in \"");

		std::string dir = GUI::ConvertFromUTF8( GUI::UTF8FromString(directory), CP_ACP);
		basePathLen = dir.length();
		os.append(dir.c_str());
		if (pathSepChar == os[os.length()-1]) {
			basePathLen--;
		} else {
			os.append(GUI::UTF8FromString(pathSepString).c_str());
		}

		os.append(GUI::UTF8FromString(fileTypes).c_str());
		os.append("\"");

		originalEnd = wFindRes.Send(SCI_GETCURRENTPOS);
	}
	if (!(gf & grepMatchCase) && !(gf & grepRegExp)) {
		searchString.lowercase();
	}

	std::regex regexp;
	
	if ((gf & grepRegExp)){
		try {
			//ElapsedTime et;
			std::regex::flag_type flagsRe = std::regex::ECMAScript;
			// Flags that apper to have no effect:
			// | std::regex::collate | std::regex::extended;
			if (!(gf & grepMatchCase))
				flagsRe = flagsRe | std::regex::icase;

			bool matched = false;

			regexp.assign(searchString.c_str(), flagsRe);
		}
		catch (...) {
			// Failed in some other way
			return;
		}
	}

	jobQueue.StartSearch(gf & grepProgress);

	GrepOut grepOut;
	grepOut.iLines = 0;
	grepOut.iFiles = 0;
	grepOut.iInFiles = 0;
	grepOut.strOut = "";	
	if (gf & grepProgress){
		CountRecursive(gf, FilePath(directory), fileTypes, &grepOut);
		jobQueue.SetAll(grepOut.iFiles);
		grepOut.iFiles = 0;
	}
	if (wFindRes.Send(SCI_GETLINECOUNT)) {
		wFindRes.Send(SCI_SETSEL, 0, 0);
		wFindRes.Send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>("\n"));
	}
	wFindRes.Send(SCI_SETSEL, 0, 0);
	wFindRes.Send(SCI_SETFIRSTVISIBLELINE, 0);
	wFindRes.Send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>("Wait...\n"));

	GrepRecursive(gf, FilePath(directory), searchString.c_str(), fileTypes, basePathLen, &grepOut, &regexp); 
	if (!(gf & grepStdOut)) {
		//SString sExitMessage();
		if (jobQueue.TimeCommands()) {
			os += "    Time: ";
			os += SString(commandTime.Duration(), 3);
		}
		if (!jobQueue.ContinueSearch()){
			os += " (Cancelled)";
		}
		os += "    Lines: ";
		os += SString(grepOut.iLines);
		os += " in ";
		os += SString(grepOut.iInFiles);
		os += " from ";
		os += SString(grepOut.iFiles);
		os += " files\n";
		os += grepOut.strOut;
		os.append("<\n");
	}
	wFindRes.Send(SCI_SETSEL, 0, 8);
	wFindRes.Send(SCI_SETFIRSTVISIBLELINE, 0);
	wFindRes.Send(SCI_COLOURISE, 0, -1);
	Sci_Position maxLine = wFindRes.Send(SCI_GETLINECOUNT);

	for (Sci_Position line = 0; line < maxLine; line++) {
		int level = static_cast<int>(wFindRes.Send(SCI_GETFOLDLEVEL, line));
		if ((level & SC_FOLDLEVELHEADERFLAG) &&
			(SC_FOLDLEVELBASE == (level & SC_FOLDLEVELNUMBERMASK))) {
			Sci_Position lineMaxSubord = wFindRes.Send(SCI_GETLASTCHILD, line, -1);
				wFindRes.Send(SCI_SETFOLDEXPANDED, line, 0);
				if (lineMaxSubord > line)
					wFindRes.Send(SCI_HIDELINES, line + 1, lineMaxSubord);
		}
	}
	wFindRes.Send(SCI_SETSEL, 0, 8);
	wFindRes.Send(SCI_SETLINESTATE, 0, -1);
	wFindRes.Send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(os.c_str()));
	wFindRes.Send(SCI_SETSEL, 0, 0);
	wFindRes.Send(SCI_SETFIRSTVISIBLELINE, 0);
	::PostMessage(reinterpret_cast<HWND>(GetID()), SCI_FINDPROGRESS, 2, 0);

}
static int LuaPanicFunction(lua_State *Ls) {

	lua_close(Ls);
	::MessageBox(NULL, L" Lua: error occurred in unprotected call.  This is very bad.", L"Scite", MB_OK);
	ExitThread(1);
	return 1;
}

///Поддержка запуска луа в отдельном потоке
static ExtensionAPI *host = 0;

inline void raise_error(lua_State *L, const char *errMsg = NULL) {
	luaL_where(L, 1);
	if (errMsg) {
		lua_pushstring(L, errMsg);
	}
	else {
		lua_insert(L, -2);
	}
	lua_concat(L, 2);
	lua_error(L);
}
static int lua_string_from_utf8(lua_State *L) {
	if (lua_gettop(L) != 2) raise_error(L, "Wrong arguments count for scite.ConvertFromUTF8");
	const char *s = luaL_checkstring(L, 1);
	int cp = 0;
	if (!lua_isnumber(L, 2))
		cp = GUI::CodePageFromName(lua_tostring(L, 2));
	else
		cp = static_cast<int>(lua_tointeger(L, 2));
	std::string ss = GUI::ConvertFromUTF8(s, cp);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int cf_global_print(lua_State *L) {
	int nargs = lua_gettop(L);

	lua_getglobal(L, "tostring");

	for (int i = 1; i <= nargs; ++i) {
		if (i > 1)
			host->Trace("\t");

		const char *argStr = lua_tostring(L, i);
		if (argStr) {
			host->Trace(argStr);
		}
		else {
			lua_pushvalue(L, -1); // tostring
			lua_pushvalue(L, i);
			lua_call(L, 1, 1);
			argStr = lua_tostring(L, -1);
			if (argStr) {
				host->Trace(argStr);
			}
			else {
				raise_error(L, "tostring (called from print) returned a non-string");
			}
			lua_settop(L, nargs + 1);
		}
	}

	host->Trace("\n");
	return 0;
}

int SciTEBase::internalRunLuaThread(SString strCmd, SString strDesc)	{
	lua_State  *L;
	host = this;
	L = luaL_newstate();
	lua_atpanic(L, LuaPanicFunction);
	luaL_openlibs(L);
	lua_register(L, "print", cf_global_print);
	lua_getglobal(L, "string");
	lua_pushcfunction(L, lua_string_from_utf8);
	lua_setfield(L, -2, "from_utf8");
	lua_pop(L, 1);

	if (0 == luaL_loadbuffer(L, strCmd.c_str(), strCmd.length(), strDesc.c_str())) {
		if (lua_pcall(L, 0, LUA_MULTRET, 0)){
			if (lua_isstring(L, -1)) {
				size_t len;
				const char *msg = lua_tolstring(L, -1, &len);
				char *buff = new char[len + 2];
				strncpy(buff, msg, len);
				buff[len] = '\n';
				buff[len + 1] = '\0';
				wOutput.Send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(buff));
				delete[] buff;
			}
		}
		return lua_gettop(L);
	}
	else {
		raise_error(L);
	}

	lua_close(L);

	return 0;
}
