// SciTE - Scintilla based Text Editor
/** @file SciTEBuffers.cxx
 ** Buffers and jobs management.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <map>

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
#ifdef __DMC__
#include <dir.h>
#endif

#endif

#include "Scintilla.h"
#include "SciLexer.h"

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

const GUI::gui_char defaultSessionFileName[] = GUI_TEXT("SciTE.session");

BufferList::BufferList() : current(0), stackcurrent(0), stack(0), buffers(0), size(0), length(0), initialised(false) {}

BufferList::~BufferList() {
	delete []buffers;
	delete []stack;
}

void BufferList::Allocate(int maxSize) {
	length = 1;
	current = 0;
	size = maxSize;
	buffers = new Buffer[size];
	stack = new int[size];
	buffers[0].Init(this);
	pEditor->SetBuffPointer(&buffers[0].AbsolutePath());
	stack[0] = 0;
}

int BufferList::Add(sptr_t doc) {
	if (length < size) {
		length++;
	}
	buffers[length - 1].Init(this);
	if (doc)
		buffers[length - 1].doc = doc;
	stack[length - 1] = length - 1;
	buffers[length - 1].editorSide = pEditor->GetWindowIdm();
	
	MoveToStackTop(length - 1);

//!-start-[NewBufferPosition]
	switch (SciTEBase::GetProps()->GetInt("buffers.new.position", 0)) {
	case 1:
		ShiftTo(length - 1, current + 1);
		pEditor->SetBuffPointer(&buffers[current + 1].AbsolutePath());
		return current + 1;
		break;
	case 2:
		ShiftTo(length - 1, 0);
		pEditor->SetBuffPointer(&buffers[0].AbsolutePath());
		return 0;
		break;
	}
//!-end-[NewBufferPosition]
    pEditor->SetBuffPointer(&buffers[length - 1].AbsolutePath());
	return length - 1;
}

int BufferList::GetDocumentByName(FilePath filename, bool excludeCurrent, int forIdm) {
	if (!filename.IsSet()) {
		return -1;
	}
	for (int i = 0;i < length;i++) {
		if ((!excludeCurrent || i != current) && buffers[i].SameNameAs(filename) && (!forIdm || forIdm == buffers[i].editorSide)) {
			return i;
		}
	}
	return -1;
}

int BufferList::NextByIdm(int idm){
	for (int i = current + 1; i < length; i++){
		if (buffers[i].editorSide == idm)
			return i;
	}
	for (int i = 0; i < current; i++){
		if (buffers[i].editorSide == idm)
			return i;
	}
	return -1;
}

void BufferList::RemoveCurrent() {
	// Delete and move up to fill gap but ensure doc pointer is saved.
	sptr_t currentDoc = buffers[current].doc;
	for (int i = current;i < length - 1;i++) {
		buffers[i] = buffers[i + 1];
	}
	buffers[length - 1].doc = currentDoc;

	if (length > 1) {
		CommitStackSelection();
		PopStack();
		length--;

		buffers[length].Init(this);
		if (current >= length) {
			SetCurrent(length - 1);
		}
		if (current < 0) {
			SetCurrent(0);
		}
//!-start-[ZorderSwitchingOnClose]
		if (SciTEBase::GetProps()->GetInt("buffers.zorder.switching", 0)) {
			SetCurrent(stack[stackcurrent]);
		}
//!-end-[ZorderSwitchingOnClose]
	} else {
		buffers[current].Init(this);
	}
	MoveToStackTop(current);
}

int BufferList::Current() const {
	return current;
}

Buffer *BufferList::CurrentBuffer() {
	return &buffers[Current()];
}

void BufferList::SetCurrent(int index) {
	current = index;
	pEditor->SwitchTo(buffers[current].editorSide, &buffers[current].AbsolutePath());
	SciTEBase::GetProps()->SetInteger("BufferNumber", current+1); //!-add-[BufferNumber]
}

void BufferList::PopStack() {
	for (int i = 0; i < length - 1; ++i) {
		int index = stack[i + 1];
		// adjust the index for items that will move in buffers[]
		if (index > current)
			--index;
		stack[i] = index;
	}
}
int BufferList::StackNextBySide(int side, int curr){
	int stmp = stackcurrent;
	int res;
	for (int i = 0; i < length; i++) {
		if (++stackcurrent >= length)
			stackcurrent = 0;
		res = stack[stackcurrent];
		if(buffers[res].editorSide == side && curr != res)
			return res;
	}
	stackcurrent = stmp;
	return - 1;
}

int BufferList::StackNext() {
	if (++stackcurrent >= length)
		stackcurrent = 0;
	return stack[stackcurrent];
}

int BufferList::StackPrev() {
	if (--stackcurrent < 0)
		stackcurrent = length - 1;
	return stack[stackcurrent];
}

int BufferList::GetOrder(int index) {
	int res = length;
	for (int i = 0; i < length; i++) {
		if (stack[i] == index) {
			res = i;
			break;
		}
	}
	return res;
}

void BufferList::MoveToStackTop(int index) {
	// shift top chunk of stack down into the slot that index occupies
	bool move = false;
	for (int i = length - 1; i > 0; --i) {
		if (stack[i] == index)
			move = true;
		if (move)
			stack[i] = stack[i-1];
	}
	stack[0] = index;
}

void BufferList::CommitStackSelection() {
	// called only when ctrl key is released when ctrl-tabbing
	// or when a document is closed (in case of Ctrl+F4 during ctrl-tabbing)
	MoveToStackTop(stack[stackcurrent]);
	stackcurrent = 0;
}

void BufferList::ShiftTo(int indexFrom, int indexTo) {
	// shift buffer to new place in buffers array
	if (indexFrom == indexTo ||
		indexFrom < 0 || indexFrom >= length ||
		indexTo < 0 || indexTo >= length) return;
	int step = (indexFrom > indexTo) ? -1 : 1;
	Buffer tmp = buffers[indexFrom];
	int i;
	for (i = indexFrom; i != indexTo; i += step) {
		buffers[i] = buffers[i + step];
	}
	buffers[indexTo] = tmp;
	// update stack indexes
	for (i = 0; i < length; i++) {
		if (stack[i] == indexFrom) {
			stack[i] = indexTo;
		} else if (step == 1) {
			if (indexFrom < stack[i] && stack[i] <= indexTo) stack[i] -= step;
		} else {
			if (indexFrom > stack[i] && stack[i] >= indexTo) stack[i] -= step;
		}
	}
	if (indexTo >= current && current > indexFrom) current--;
	if (indexTo <= current && current < indexFrom) current++;
	if (indexFrom == current) current = indexTo;
}

void BufferList::OrderBy(std::map<int, int> &order) {
	std::map<int, Buffer> orderBuf;
	std::map<int, int> orderStack;
	bool bSetCur = true;
	for (auto& elem: order) {
		orderBuf.insert({ elem.first, buffers[elem.second] });
		if (current == elem.second && bSetCur) {
			current = elem.first;
			bSetCur = false;
		}
		for (int i = 0; i < length; i++) {
			if (stack[i] == elem.second) {
				orderStack.insert({ i, elem.first });
				break;
			}
		}
	}
	for (auto& elem : orderBuf) {
		buffers[elem.first] = elem.second;
	}
	for (auto& elem : orderStack) {
		stack[elem.first] = elem.second;
	}

}

void SciTEBase::OrderTabsBy(std::map<int, int> &order) {
	buffers.OrderBy(order);
	BuffersMenu();
}

void SciTEBase::ChangeTabWnd() {
	if (buffers.CurrentBuffer()->pFriend)
		return;
	int origStackCur = buffers.stackcurrent;
	Buffer* bPrev = buffers.CurrentBuffer();
	int iPrev = buffers.Current();
	int iPrevSide = buffers.CurrentBuffer()->editorSide;
	sptr_t d = bPrev->doc;

	int iNext = buffers.StackNextBySide(buffers.CurrentBuffer()->editorSide, iPrev);

	FilePath absPath = buffers.CurrentBuffer()->AbsolutePath();
	wEditor.coEditor.Call(SCI_ADDREFDOCUMENT, 0, d);
	wEditor.coEditor.Call(SCI_SETDOCPOINTER, 0, d);

	bPrev->editorSide = bPrev->editorSide == IDM_COSRCWIN ? IDM_SRCWIN : IDM_COSRCWIN;

	wEditor.Switch();
	bBlockRedraw = true;
	SetFileName((FilePath)(*bPrev));
	CurrentBuffer()->overrideExtension = "";

	//ReadProperties();
	//SetIndentSettings();
	//SetEol();
	UpdateBuffersCurrent();

	buffers.CurrentBuffer()->SetTimeFromFile();

	if (iNext > -1) {
		wEditor.Switch(true);
		SetDocumentAt(iNext);
		bBlockRedraw = false;
		BuffersMenu();
		bBlockRedraw = true;
	}
	else {
		sptr_t d = wEditor.coEditor.Call(SCI_CREATEDOCUMENT, 0, 0);
		wEditor.coEditor.Call(SCI_ADDREFDOCUMENT, 0, d);
		wEditor.coEditor.Call(SCI_SETDOCPOINTER, 0, d);
		wEditor.SetCoBuffPointer(NULL);
			wEditor.Switch(true);
		if (iPrevSide == IDM_SRCWIN) {
			New();
		}
	}
	bBlockRedraw = false;
	int p = props.GetInt("tabctrl.alwayssavepos");
	props.SetInteger("tabctrl.alwayssavepos", 1);
	SetDocumentAt(iPrev);
	props.SetInteger("tabctrl.alwayssavepos", p);
	buffers.stackcurrent = origStackCur;
}

void SciTEBase::CloneTab(){
	if (buffers.CurrentBuffer()->pFriend)
		return;

	Buffer* bPrev = buffers.CurrentBuffer();
	sptr_t d = bPrev->doc;
	int iPrev = buffers.Current();

	FilePath absPath = buffers.CurrentBuffer()->AbsolutePath();
	wEditor.coEditor.Call(SCI_ADDREFDOCUMENT, 0, d);
	wEditor.coEditor.Call(SCI_SETDOCPOINTER, 0, d);
	wEditor.SetCoBuffPointer(&absPath);
	wEditor.Switch();

	buffers.SetCurrent(buffers.Add(d));
	bPrev->pFriend = true;
	CurrentBuffer()->pFriend = true;

	SetFileName((FilePath)(*bPrev));
	CurrentBuffer()->overrideExtension = "";
	ReadProperties();
	SetIndentSettings();
	SetEol();
	UpdateBuffersCurrent();

	buffers.CurrentBuffer()->SetTimeFromFile();

	SetDocumentAt(iPrev + 1);

}

void SciTEBase::CheckRightEditorVisible() {
	for (int i = 0; i < buffers.length; i++) {
		if (buffers.buffers[i].editorSide != IDM_SRCWIN) {
			if (!m_bRightEditorVisible) {
				m_bRightEditorVisible = true;
				extender->OnRightEditorVisibility(true);
			}
			return;
		}
	}
	if (m_bRightEditorVisible) {
		extender->OnRightEditorVisibility(false);
		m_bRightEditorVisible = false;
	}
}

sptr_t SciTEBase::GetDocumentAt(int index) {
	if (index < 0 || index >= buffers.size) {
		return 0;
	}
	if (buffers.buffers[index].doc == 0) {
		// Create a new document buffer
		buffers.buffers[index].doc = wEditor.Call(SCI_CREATEDOCUMENT, 0, 0);
	}
	return buffers.buffers[index].doc;
}

int SciTEBase::ShiftToVisible(int index) {
	if (::IsWindowVisible((HWND)wSciTE.GetID()) && !bBlockUIUpdate && !props.GetInt("tabctrl.moved") &&
		(((!props.GetInt("tabctrl.alwayssavepos") && !(::GetAsyncKeyState(VK_SHIFT) & 0x8000))) || (props.GetInt("tabctrl.alwayssavepos") && (::GetAsyncKeyState(VK_SHIFT) & 0x8000)))) {
		int side = buffers.buffers[index].editorSide;
		int realIndex = 0;
		int firstSide;
		for (int pos = 0; pos < buffers.length; pos++) {
			if (side == buffers.buffers[pos].editorSide) {
				if (!realIndex)
					firstSide = pos;
				if (pos == index) {
					if ((int)IupGetAttribute(IupTab(side), "LASTVISIBLE") <= realIndex && index != firstSide) {
						ShiftTab(index, firstSide);
						index = firstSide;
					}
					break;
				}
				realIndex++;
			}
		}
	}
	return index;
}
void SciTEBase::SetDocumentAt(int index, bool updateStack, bool switchTab, bool bExit) {
	
	int currentbuf = buffers.Current();
	if (	index < 0 ||
	        index >= buffers.length ||
	        //index == currentbuf ||
	        currentbuf < 0 ||
	        currentbuf >= buffers.length) {
		return;
	}
	int startSide = buffers.buffers[buffers.Current()].editorSide;
	if(startSide == buffers.buffers[index].editorSide)
		layout.OnOpenClose(startSide);

	wEditor.Call(WM_SETREDRAW, 0);
	UpdateBuffersCurrent();
	buffers.SetCurrent(index);
	if (updateStack) {
		buffers.MoveToStackTop(index);
	}

	if (extender) {
		if (buffers.size > 1)
			extender->ActivateBuffer(index);
		else
			extender->InitBuffer(0);
	}

	Buffer bufferNext = buffers.buffers[buffers.Current()];
	SetFileName(bufferNext, true, switchTab);
	if (!bExit) {
		wEditor.Call(SCI_SETDOCPOINTER, 0, GetDocumentAt(buffers.Current()));
		RestoreState(bufferNext, switchTab);

		if (lineNumbers && lineNumbersExpand)
			SetLineNumberWidth();

		DisplayAround(bufferNext);

		CheckMenus();
	} else if (startSide == buffers.buffers[index].editorSide)
		ReadProperties();

	if (extender) {
		extender->OnSwitchFile(filePath.AsUTF8().c_str());
		layout.OnSwitchFile(buffers.buffers[buffers.Current()].editorSide);
	}
	if(!bBlockUIUpdate){
		wEditor.Call(WM_SETREDRAW, 1);
		RedrawWindow((HWND)wEditor.GetID(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		wEditor.coEditor.Call(WM_SETREDRAW, 1);
		RedrawWindow((HWND)wEditor.coEditor.GetID(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	
	}
}

void SciTEBase::UpdateBuffersCurrent() {
	int currentbuf = buffers.Current();

	if ((buffers.length > 0) && (currentbuf >= 0)) {
		buffers.buffers[currentbuf].Set(filePath);
		buffers.buffers[currentbuf].selection = GetSelection();
		buffers.buffers[currentbuf].scrollPosition = GetCurrentScrollPosition();

		// Retrieve fold state and store in buffer state info

		std::vector<int> *f = &buffers.buffers[currentbuf].foldState;
		f->clear();

		if (props.GetInt("fold")) {
			for (int line = 0; ; line++) {
				int lineNext = wEditor.Call(SCI_CONTRACTEDFOLDNEXT, line);
				if ((line < 0) || (lineNext < line))
					break;
				line = lineNext;
				f->push_back(line);
			}
		}
	}
}

bool SciTEBase::IsBufferAvailable() {
	return buffers.size > 1 && buffers.length < buffers.size;
}

bool SciTEBase::CanMakeRoom(bool maySaveIfDirty) {
	if (IsBufferAvailable()) {
		return true;
	} else if (maySaveIfDirty) {
		// All available buffers are taken, try and close the current one
		if (SaveIfUnsure(true) != IDCANCEL) {
			// The file isn't dirty, or the user agreed to close the current one
			return true;
		}
	} else {
		return true;	// Told not to save so must be OK.
	}
	return false;
}

void SciTEBase::ClearDocument() {
	wEditor.Call(SCI_SETREADONLY, 0);
	wEditor.Call(SCI_SETUNDOCOLLECTION, 0);
	wEditor.Call(SCI_CLEARALL);
	wEditor.Call(SCI_EMPTYUNDOBUFFER);
	wEditor.Call(SCI_SETUNDOCOLLECTION, 1);
	wEditor.Call(SCI_SETSAVEPOINT);
	wEditor.Call(SCI_SETREADONLY, isReadOnly);
}

void SciTEBase::CreateBuffers() {
	int buffersWanted = props.GetInt("buffers");
	if (buffersWanted > bufferMax) {
		buffersWanted = bufferMax;
	}
	if (buffersWanted < 1) {
		buffersWanted = 1;
	}
	buffers.Allocate(buffersWanted);
}

void SciTEBase::InitialiseBuffers() {
	if (!buffers.initialised) {
		buffers.initialised = true;
		// First document is the default from creation of control
		buffers.buffers[0].doc = wEditor.Call(SCI_GETDOCPOINTER, 0, 0);
		wEditor.Call(SCI_ADDREFDOCUMENT, 0, buffers.buffers[0].doc); // We own this reference
	}
}

FilePath SciTEBase::UserFilePath(const GUI::gui_char *name) {
	GUI::gui_string nameWithVisibility(configFileVisibilityString);
	nameWithVisibility += name;
	return FilePath(GetSciteUserHome(), nameWithVisibility.c_str());
}

static SString IndexPropKey(const char *bufPrefix, int bufIndex, const char *bufAppendix) {
	SString pKey = bufPrefix;
	pKey += '.';
	pKey += SString(bufIndex + 1);
	if (bufAppendix != NULL) {
		pKey += ".";
		pKey += bufAppendix;
	}
	return pKey;
}

void SciTEBase::SetIndentSettings() {
	// Get default values
	int useTabs = props.GetInt("use.tabs", 1);
	int tabSize = props.GetInt("tabsize");
	int indentSize = props.GetInt("indent.size");
	// Either set the settings related to the extension or the default ones
	SString fileNameForExtension = ExtensionFileName();
	SString useTabsChars = props.GetNewExpand("use.tabs.",
	        fileNameForExtension.c_str());
	if (useTabsChars.length() != 0) {
		wEditor.Call(SCI_SETUSETABS, useTabsChars.value());
	} else {
		wEditor.Call(SCI_SETUSETABS, useTabs);
	}
	SString tabSizeForExt = props.GetNewExpand("tab.size.",
	        fileNameForExtension.c_str());
	if (tabSizeForExt.length() != 0) {
		wEditor.Call(SCI_SETTABWIDTH, tabSizeForExt.value());
	} else if (tabSize != 0) {
		wEditor.Call(SCI_SETTABWIDTH, tabSize);
	}
	SString indentSizeForExt = props.GetNewExpand("indent.size.",
	        fileNameForExtension.c_str());
	if (indentSizeForExt.length() != 0) {
		wEditor.Call(SCI_SETINDENT, indentSizeForExt.value());
	} else {
		wEditor.Call(SCI_SETINDENT, indentSize);
	}
}

void SciTEBase::SetEol() {
	SString eol_mode = props.Get("eol.mode");
	if (eol_mode == "LF") {
		wEditor.Call(SCI_SETEOLMODE, SC_EOL_LF);
	} else if (eol_mode == "CR") {
		wEditor.Call(SCI_SETEOLMODE, SC_EOL_CR);
	} else if (eol_mode == "CRLF") {
		wEditor.Call(SCI_SETEOLMODE, SC_EOL_CRLF);
	}
}

void SciTEBase::New() {
	InitialiseBuffers();
	UpdateBuffersCurrent();

	if ((buffers.size == 1) && (!buffers.buffers[0].IsUntitled())) {
		AddFileToStack(buffers.buffers[0],
		        buffers.buffers[0].selection,
		        buffers.buffers[0].scrollPosition);
	}

	// If the current buffer is the initial untitled, clean buffer then overwrite it,
	// otherwise add a new buffer.
	if ((buffers.length > 1) ||
	        (buffers.Current() != 0) ||
	        (buffers.buffers[0].isDirty) ||
	        (!buffers.buffers[0].IsUntitled())) {
		if (buffers.size == buffers.length) {
			Close(false, false, true);
		}
		buffers.SetCurrent(buffers.Add());
	}

	sptr_t doc = GetDocumentAt(buffers.Current());
	wEditor.Call(SCI_SETDOCPOINTER, 0, doc);

	FilePath curDirectory(filePath.Directory());
	filePath.Set(curDirectory, GUI_TEXT(""));
	SetFileName(filePath);
	CurrentBuffer()->isDirty = false;
	jobQueue.isBuilding = false;
	jobQueue.isBuilt = false;
	isReadOnly = false;	// No sense to create an empty, read-only buffer...

	ClearDocument();

	if (extender)
		extender->InitBuffer(buffers.Current());
}

void SciTEBase::RestoreState(const Buffer &buffer, bool setCaption) {
	if (setCaption)
		SetWindowName();
	ReadProperties();
	if (CurrentBuffer()->unicodeMode != uni8Bit) {
		// Override the code page if Unicode
		codePage = SC_CP_UTF8;
		wEditor.Call(SCI_SETCODEPAGE, codePage);
	}
	props.SetInteger("editor.unicode.mode", CurrentBuffer()->unicodeMode + IDM_ENCODING_DEFAULT); //!-add-[EditorUnicodeMode]
	isReadOnly = wEditor.Call(SCI_GETREADONLY);

	// check to see whether there is saved fold state, restore
	for (size_t fold = 0; fold < buffer.foldState.size(); fold++) {
		wEditor.Call(SCI_TOGGLEFOLD, buffer.foldState[fold]);
	}
}

void SciTEBase::Close(bool updateUI, bool loadingSession, bool makingRoomForNew) {
	bool closingLast = false;
	int nextFriend = -2;
	int prevIdm = -2;
	layout.OnOpenClose(buffers.buffers[buffers.Current()].editorSide);
	if (extender) {
		extender->OnClose(filePath.AsUTF8().c_str());
		extender->OnNavigation("Close");
	}
	bBlockRedraw = true;

	if (buffers.size == 1) {
		// With no buffer list, Close means close from MRU
		closingLast = true;	  //Врядли мы будем работать в однооконном режиме !!!!!!!!!!!!
		buffers.buffers[0].Init(&buffers);
		filePath.Set(GUI_TEXT(""));
		ClearDocument(); //avoid double are-you-sure
		if (!makingRoomForNew)
			StackMenu(0); // calls New, or Open, which calls InitBuffer
	} else if (buffers.size > 1) {
		if (buffers.Current() >= 0 && buffers.Current() < buffers.length) {
			UpdateBuffersCurrent();
			Buffer buff = buffers.buffers[buffers.Current()];
			AddFileToStack(buff, buff.selection, buff.scrollPosition);
		}
		closingLast = (buffers.length == 1);
		if (closingLast) {
			buffers.buffers[0].Init(&buffers);
			if (extender)
				extender->InitBuffer(0);
		} else {
			prevIdm = buffers.CurrentBuffer()->editorSide;
			if (extender)
				extender->RemoveBuffer(buffers.Current());
			if (buffers.CurrentBuffer()->pFriend){
				if (buffers.CurrentBuffer()->isDirty)
					buffers.CurrentBuffer()->Friend()->isDirty = true;
				buffers.CurrentBuffer()->Friend()->pFriend = false;
				buffers.CurrentBuffer()->pFriend = false;
				wEditor.Call(SCI_RELEASEDOCUMENT, 0, buffers.CurrentBuffer()->doc);	
				buffers.CurrentBuffer()->doc = 0;
				sptr_t d = wEditor.Call(SCI_CREATEDOCUMENT, 0, 0);
				wEditor.Call(SCI_ADDREFDOCUMENT, 0, d);
				wEditor.Call(SCI_SETDOCPOINTER, 0, d);
			}
			ClearDocument();

			buffers.RemoveCurrent();
			if (extender && !makingRoomForNew)
				extender->ActivateBuffer(buffers.Current());
			if (prevIdm != buffers.CurrentBuffer()->editorSide){
				nextFriend = buffers.NextByIdm(prevIdm);
				if (nextFriend == -1){
					wEditor.SetCoBuffPointer(NULL);
				}
				else{
					wEditor.SetCoBuffPointer(&buffers.buffers[nextFriend].AbsolutePath());
					sptr_t d = buffers.buffers[nextFriend].doc;

					wEditor.coEditor.Call(SCI_ADDREFDOCUMENT, 0, d);
					wEditor.coEditor.Call(SCI_SETDOCPOINTER, 0, d);
				}
			}
		}
		Buffer bufferNext = buffers.buffers[buffers.Current()];

		if (updateUI)
			SetFileName(bufferNext);
		else
			filePath = bufferNext;
		wEditor.Call(SCI_SETDOCPOINTER, 0, GetDocumentAt(buffers.Current()));
		if (closingLast) {
			ClearDocument();
		}
		if (updateUI)
			CheckReload();
		if (updateUI) {
			RestoreState(bufferNext);
			DisplayAround(bufferNext);
		}
	}

	bBlockRedraw = false;
	if (updateUI) {
		BuffersMenu();
	}

	if (extender && !closingLast && !makingRoomForNew) {
		extender->OnSwitchFile(filePath.AsUTF8().c_str());
		extender->OnNavigation("Close-");
		layout.OnSwitchFile(buffers.buffers[buffers.Current()].editorSide);
	}
	if (nextFriend == -1 && prevIdm == IDM_SRCWIN){
		wEditor.Switch();
		New();
	}

	if (closingLast && props.GetInt("quit.on.close.last") && !loadingSession) {
		QuitProgram();
	}
}

void SciTEBase::CloseTab(int tab) {
	int tabCurrent = buffers.Current();
	if (tab == tabCurrent) {
		if (SaveIfUnsure() != IDCANCEL) {
			Close();
		}
	} else {
		FilePath fpCurrent = buffers.buffers[tabCurrent].AbsolutePath();
		SetDocumentAt(tab);
		if (SaveIfUnsure() != IDCANCEL) {
			Close();
			WindowSetFocus(wEditor);
			// Return to the previous buffer
			SetDocumentAt(buffers.GetDocumentByName(fpCurrent));
		}
	}
}

void SciTEBase::CloseAllBuffers(bool loadingSession) {
	if (SaveAllBuffers(false) != IDCANCEL) {
		while (buffers.length > 1)
			Close(false, loadingSession);

		Close(true, loadingSession);
	}
}

int SciTEBase::SaveAllBuffers(bool forceQuestion, bool alwaysYes) {
	int choice = IDYES;
	UpdateBuffersCurrent();
	int currentBuffer = buffers.Current();
	for (int i = 0; (i < buffers.length) && (choice != IDCANCEL); i++) {
//!		if (buffers.buffers[i].isDirty) {
		if (buffers.buffers[i].DocumentNotSaved()) { //-change-[OpenNonExistent]
			SetDocumentAt(i);
			if (alwaysYes) {
				if (!Save()) {
					choice = IDCANCEL;
				}
			} else {
				choice = SaveIfUnsure(forceQuestion);
			}
		}
	}
	SetDocumentAt(currentBuffer);
	return choice;
}

void SciTEBase::SaveTitledBuffers() {
	UpdateBuffersCurrent();
	int currentBuffer = buffers.Current();
	for (int i = 0; i < buffers.length; i++) {
//!		if (buffers.buffers[i].isDirty && !buffers.buffers[i].IsUntitled()) {
		if (buffers.buffers[i].DocumentNotSaved() && !buffers.buffers[i].IsUntitled()) { //-change-[OpenNonExistent]
			SetDocumentAt(i);
			Save();
		}
	}
	SetDocumentAt(currentBuffer);
}

void SciTEBase::Next() {
	int next = buffers.Current();
	if (++next >= buffers.length)
		next = 0;
	SetDocumentAt(next);
	CheckReload();
}

void SciTEBase::Prev() {
	int prev = buffers.Current();
	if (--prev < 0)
		prev = buffers.length - 1;

	SetDocumentAt(prev);
	CheckReload();
}

void SciTEBase::ShiftTab(int indexFrom, int indexTo, bool mouse) {
	buffers.ShiftTo(indexFrom, indexTo);
	buffers.SetCurrent(indexTo);
	BuffersMenu(mouse);

	DisplayAround(buffers.buffers[buffers.Current()]);
}

void SciTEBase::MoveTabRight() {
	if (buffers.length < 2) return;
	int indexFrom = buffers.Current();
	int indexTo = indexFrom + 1;
	if (indexTo >= buffers.length) indexTo = 0;
	ShiftTab(indexFrom, indexTo);
}

void SciTEBase::MoveTabLeft() {
	if (buffers.length < 2) return;
	int indexFrom = buffers.Current();
	int indexTo = indexFrom - 1;
	if (indexTo < 0) indexTo = buffers.length - 1;
	ShiftTab(indexFrom, indexTo);
}

void SciTEBase::NextInStack() {
	extender->OnNavigation("Switch");
	SetDocumentAt(buffers.StackNext(), false);
	extender->OnNavigation("Switch-");
	CheckReload();
}

void SciTEBase::PrevInStack() {
	extender->OnNavigation("Switch");
	SetDocumentAt(buffers.StackPrev(), false);
	extender->OnNavigation("Switch-");
	CheckReload();
}

void SciTEBase::EndStackedTabbing() {
	buffers.CommitStackSelection();
}

std::string ToAnsi(const GUI::gui_string &sWide) {

	int cchMulti = ::WideCharToMultiByte(CP_ACP, 0, sWide.c_str(), sWide.length(), NULL, 0, NULL, NULL);
	char *pszMulti = new char[cchMulti + 1];
	::WideCharToMultiByte(CP_ACP, 0, sWide.c_str(), sWide.length(), pszMulti, cchMulti + 1, NULL, NULL);
	pszMulti[cchMulti] = 0;
	std::string ret(pszMulti);
	delete[]pszMulti;
	return ret;

}
static int Ext2HUI(GUI::gui_string ext) {
	int r = 0;
	size_t len = ext.length();
	if(len > 0)
	    r += (ext.c_str()[0] % 26) * 13;
	if (len > 1)
		r += (ext.c_str()[1] % 26) * 13 + 3;
	if (len > 2)
		r += (ext.c_str()[2] % 26) * 13 + 6;
	if (len > 3)
		r += (ext.c_str()[3] % 26) * 13 + 9;
	return r % 360;
}
//static char tabROColor[15];

const char* SciTEBase::GetPropClr(const char* propName, char* buff, const char* def) {
	if (lstrcmpA(props.Get(propName).c_str(), "")) {
		return strcpy(buff, props.Get(propName).c_str());
	} else
		return def;
}



void SciTEBase::BuffersMenu(bool mousedrag) {
	UpdateBuffersCurrent();
	static char tabForeColor[16];
	static char tabROColor[16];
	static char tabActBackColor[16];
	static char tabActForeColor[16];
	static char tabActForeROColor[16];
	static char tabActForeMoviedColor[16];

	bool utf8mode = !strcmp(IupGetGlobal("UTF8MODE"), "YES");

	int pos;

	if (bBlockRedraw || bBlockUIUpdate)
		return;
	IupStoreAttribute(IupTab(IDM_COSRCWIN), "SATURATION", props.Get("tabctrl.cut.saturation").c_str());
	IupStoreAttribute(IupTab(IDM_COSRCWIN), "ILLUMINATION", props.Get("tabctrl.cut.illumination").c_str());
	IupStoreAttribute(IupTab(IDM_SRCWIN), "SATURATION", props.Get("tabctrl.cut.saturation").c_str());
	IupStoreAttribute(IupTab(IDM_SRCWIN), "ILLUMINATION", props.Get("tabctrl.cut.illumination").c_str());

	int oldLCount = (int)IupGetAttribute(IupTab(IDM_SRCWIN), "COUNT");
	int oldLCountR = (int)IupGetAttribute(IupTab(IDM_COSRCWIN), "COUNT");
	int posL = 0, posR = 0;
	if (buffers.size > 1) {

		const char* chtabForeColor = GetPropClr("tabctrl.forecolor", tabForeColor, "0 0 0");
		const char* ReadOnlyColor = GetPropClr("tabctrl.readonly.color", tabROColor, "120 120 120");
		
		const char* chtabActBackColor;

		chtabActBackColor = GetPropClr("tabctrl.active.bakcolor", tabActBackColor, "250 250 250");

		const char* chtabActForeColor = GetPropClr("tabctrl.active.forecolor", tabActForeColor, "0 0 250");
		const char* chtabActForeROColor = GetPropClr("tabctrl.active.readonly.forecolor", tabActForeROColor, "120 120 250");
		IupSetAttribute(IupTab(IDM_SRCWIN), "BGCOLOR", chtabActBackColor);
		IupSetAttribute(IupTab(IDM_COSRCWIN), "BGCOLOR", chtabActBackColor);
		
		const char* chtabActForeMoviedColor = GetPropClr("tabctrl.moved.color", tabActForeMoviedColor, "120 120 255");
		IupSetAttribute(IupTab(IDM_SRCWIN), "BGCOLORMOVIED", chtabActForeMoviedColor);
		IupSetAttribute(IupTab(IDM_COSRCWIN), "BGCOLORMOVIED", chtabActForeMoviedColor);


		for (pos = 0; pos < buffers.length; pos++) {
			int itemID = bufferCmdID + pos;
			GUI::gui_string entry;
			GUI::gui_string titleTab;
			int ihui = -1;

			if (buffers.buffers[pos].IsUntitled()) {
				GUI::gui_string untitled = extender->LocalizeText("Untitled");
				entry += untitled;
				titleTab += untitled;
			} else {
				GUI::gui_string path = buffers.buffers[pos].AsInternal();

				// Handle '&' characters in path, since they are interpreted in
				// menues and tab names.
				size_t amp = 0;
				while ((amp = path.find(GUI_TEXT("&"), amp)) != GUI::gui_string::npos) {
					path.insert(amp, GUI_TEXT("&"));
					amp += 2;
				}

				entry += path;

				size_t dirEnd = entry.rfind(pathSepChar);
				if (dirEnd != GUI::gui_string::npos) {
					titleTab += entry.substr(dirEnd + 1);
				} else {
					titleTab += entry;
				}

				size_t nameEnd = titleTab.rfind('.');
				if (props.GetInt("tabctrl.colorized")) {
					GUI::gui_string ext;
					if (nameEnd != GUI::gui_string::npos) {
						ext += titleTab.substr(nameEnd + 1);
					}
					ihui = Ext2HUI(ext);
				}
				if (props.GetInt("tabctrl.cut.ext") && nameEnd != GUI::gui_string::npos && (pos != buffers.Current())) {
					titleTab = titleTab.substr(0, nameEnd);
					if (props.GetInt("tabctrl.cut.prefix")) {
						size_t ttt = titleTab.find('^');
						bool isTmp = (titleTab.find('^') == 0);
						size_t prerEnd = titleTab.find('.');
						if (prerEnd != GUI::gui_string::npos) {
							GUI::gui_string tmp;
							if (isTmp) tmp += '^';
							tmp += titleTab.substr(prerEnd + 1);
							titleTab = tmp;
						}
					}
				}
			}

			if (buffers.buffers[pos].DocumentNotSaved()) {
				entry += GUI_TEXT("*");
				titleTab += GUI_TEXT("*");
			}
			bool ro = buffers.buffers[pos].ROMarker;

			if (buffers.buffers[pos].editorSide == IDM_COSRCWIN) {
				if (utf8mode) {
					IupStoreAttributeId(IupTab(IDM_COSRCWIN), "TABTITLE", posR, GUI::UTF8FromString(titleTab).c_str());
					IupStoreAttributeId(IupTab(IDM_COSRCWIN), "TABTIP", posR, buffers.buffers[pos].AsUTF8().c_str());
				} else {
					IupStoreAttributeId(IupTab(IDM_COSRCWIN), "TABTITLE", posR, ToAnsi(titleTab).c_str());
					IupStoreAttributeId(IupTab(IDM_COSRCWIN), "TABTIP", posR, ToAnsi(buffers.buffers[pos].AsInternal()).c_str());
				}
				IupSetIntId(IupTab(IDM_COSRCWIN), "TABBUFFERID", posR, pos);
				IupSetAttributeId(IupTab(IDM_COSRCWIN), "TABFORECOLOR", posR, ro ? ReadOnlyColor : chtabForeColor);
				IupSetIntId(IupTab(IDM_COSRCWIN), "TABBACKCOLORHUE", posR++, ihui);

				if (pos == buffers.Current()) {
					IupSetAttribute(IupTab(IDM_COSRCWIN), "VALUEPOS", (const char*)posR);
					IupSetAttribute(IupTab(IDM_COSRCWIN), "FORECOLOR", ro ? chtabActForeROColor : chtabActForeColor);
				}
			} else {
				if (utf8mode) {
					IupStoreAttributeId(IupTab(IDM_SRCWIN), "TABTITLE", posL, GUI::UTF8FromString(titleTab).c_str());
					IupStoreAttributeId(IupTab(IDM_SRCWIN), "TABTIP", posL, buffers.buffers[pos].AsUTF8().c_str());

				} else {
					IupStoreAttributeId(IupTab(IDM_SRCWIN), "TABTITLE", posL, ToAnsi(titleTab).c_str());
					IupStoreAttributeId(IupTab(IDM_SRCWIN), "TABTIP", posL, ToAnsi(buffers.buffers[pos].AsInternal()).c_str());
				}
				IupSetIntId(IupTab(IDM_SRCWIN), "TABBUFFERID", posL, pos);
				IupSetAttributeId(IupTab(IDM_SRCWIN), "TABFORECOLOR", posL, ro ? ReadOnlyColor : chtabForeColor);
				IupSetIntId(IupTab(IDM_SRCWIN), "TABBACKCOLORHUE", posL++, ihui);

				if (pos == buffers.Current()) {
					int maxP = (int)IupGetAttribute(IupTab(IDM_SRCWIN), "LASTVISIBLE");
					IupSetAttribute(IupTab(IDM_SRCWIN), "VALUEPOS", (const char*)posL);
					IupSetAttribute(IupTab(IDM_SRCWIN), "FORECOLOR", ro ? chtabActForeROColor : chtabActForeColor);
				}
			}
		}
		for (pos = oldLCount - 1; pos >= posL; pos--)
			IupStoreAttributeId(IupTab(IDM_SRCWIN), "TABTITLE", pos, NULL);
		for (pos = oldLCountR - 1; pos >= posR; pos--)
			IupStoreAttributeId(IupTab(IDM_COSRCWIN), "TABTITLE", pos, NULL);

		IupRedraw(IupGetParent(IupTab(IDM_SRCWIN)), 1);
		IupRedraw(IupGetParent(IupTab(IDM_COSRCWIN)), 1);
		if (!mousedrag) {
			IupSetAttribute(IupTab(IDM_SRCWIN), "CANCELDRAG", "");
			IupSetAttribute(IupTab(IDM_COSRCWIN), "CANCELDRAG", "");
		}

	}

	CheckMenus();
	CheckRightEditorVisible();
	ShiftToVisible(buffers.Current());
}


void SciTEBase::DropFileStackTop() {
	for (int stackPos = 0; stackPos < fileStackMax - 1; stackPos++)
		recentFileStack[stackPos] = recentFileStack[stackPos + 1];
	recentFileStack[fileStackMax - 1].Init();
}

bool SciTEBase::AddFileToBuffer(FilePath file, int pos) {
	// Return whether file loads successfully
	if (file.Exists() && Open(file, ofForceLoad)) {
		wEditor.Call(SCI_GOTOPOS, pos);
		return true;
	} else {
		return false;
	}
}

void SciTEBase::AddFileToStack(FilePath file, Sci_CharacterRange selection, int scrollPos) {
	if (!file.IsSet())
		return;
	// Only stack non-empty names
	if (file.IsSet() && !file.IsUntitled()) {
		int stackPos;
		int eqPos = fileStackMax - 1;
		for (stackPos = 0; stackPos < fileStackMax; stackPos++)
			if (recentFileStack[stackPos].SameNameAs(file))
				eqPos = stackPos;
		for (stackPos = eqPos; stackPos > 0; stackPos--)
			recentFileStack[stackPos] = recentFileStack[stackPos - 1];
		recentFileStack[0].Set(file);
		recentFileStack[0].selection = selection;
		recentFileStack[0].scrollPosition = scrollPos;
	}
}

void SciTEBase::RemoveFileFromStack(FilePath file) {
	if (!file.IsSet())
		return;
	int stackPos;
	for (stackPos = 0; stackPos < fileStackMax; stackPos++) {
		if (recentFileStack[stackPos].SameNameAs(file)) {
			for (int movePos = stackPos; movePos < fileStackMax - 1; movePos++)
				recentFileStack[movePos] = recentFileStack[movePos + 1];
			recentFileStack[fileStackMax - 1].Init();
			break;
		}
	}
}

RecentFile SciTEBase::GetFilePosition() {
	RecentFile rf;
	rf.selection = GetSelection();
	rf.scrollPosition = GetCurrentScrollPosition();
	return rf;
}

void SciTEBase::DisplayAround(const RecentFile &rf) {
	if ((rf.selection.cpMin != INVALID_POSITION) && (rf.selection.cpMax != INVALID_POSITION)) {
		int lineStart = wEditor.Call(SCI_LINEFROMPOSITION, rf.selection.cpMin);
		wEditor.Call(SCI_ENSUREVISIBLEENFORCEPOLICY, lineStart);
		int lineEnd = wEditor.Call(SCI_LINEFROMPOSITION, rf.selection.cpMax);
		wEditor.Call(SCI_ENSUREVISIBLEENFORCEPOLICY, lineEnd);
		SetSelection(rf.selection.cpMax, rf.selection.cpMin);

		int curTop = wEditor.Call(SCI_GETFIRSTVISIBLELINE);
		int lineTop = wEditor.Call(SCI_VISIBLEFROMDOCLINE, rf.scrollPosition);
		wEditor.Call(SCI_LINESCROLL, 0, lineTop - curTop);
	}
}

// Next and Prev file comments.
// Decided that "Prev" file should mean the file you had opened last
// This means "Next" file means the file you opened longest ago.
void SciTEBase::StackMenuNext() {
	for (int stackPos = fileStackMax - 1; stackPos >= 0;stackPos--) {
		if (recentFileStack[stackPos].IsSet()) {
			StackMenu(stackPos);
			return;
		}
	}
}

void SciTEBase::StackMenuPrev() {
	if (recentFileStack[0].IsSet()) {
		// May need to restore last entry if removed by StackMenu
		RecentFile rfLast = recentFileStack[fileStackMax - 1];
		StackMenu(0);	// Swap current with top of stack
		for (int checkPos = 0; checkPos < fileStackMax; checkPos++) {
			if (rfLast.SameNameAs(recentFileStack[checkPos])) {
				rfLast.Init();
			}
		}
		// And rotate the MRU
		RecentFile rfCurrent = recentFileStack[0];
		// Move them up
		for (int stackPos = 0; stackPos < fileStackMax - 1; stackPos++) {
			recentFileStack[stackPos] = recentFileStack[stackPos + 1];
		}
		recentFileStack[fileStackMax - 1].Init();
		// Copy current file into first empty
		for (int emptyPos = 0; emptyPos < fileStackMax; emptyPos++) {
			if (!recentFileStack[emptyPos].IsSet()) {
				if (rfLast.IsSet()) {
					recentFileStack[emptyPos] = rfLast;
					rfLast.Init();
				} else {
					recentFileStack[emptyPos] = rfCurrent;
					break;
				}
			}
		}
	}
}

void SciTEBase::StackMenu(int pos) {
	if (CanMakeRoom(true)) {
		if (pos >= 0) {
			if ((pos == 0) && (!recentFileStack[pos].IsSet())) {	// Empty
				New();
				SetWindowName();
				ReadProperties();
				SetIndentSettings();
				SetEol();
			} else if (recentFileStack[pos].IsSet()) {
				RecentFile rf = recentFileStack[pos];
				// Already asked user so don't allow Open to ask again.
				extender->OnNavigation("Open");
				Open(rf, ofNoSaveIfDirty);
				extender->OnNavigation("Open-");
				DisplayAround(rf);
			}
		}
	}
}


JobSubsystem SciTEBase::SubsystemType(char c) {
	if (c == '1')
		return jobGUI;
	else if (c == '2')
		return jobShell;
	else if (c == '3')
		return jobExtension;
	else if (c == '4')
		return jobHelp;
	else if (c == '5')
		return jobOtherHelp;
	return jobCLI;
}

JobSubsystem SciTEBase::SubsystemType(const char *cmd, int item) {
	SString subsysprefix = cmd;
	if (item >= 0) {
		subsysprefix += SString(item);
		subsysprefix += ".";
	}
	SString subsystem = props.GetNewExpand(subsysprefix.c_str(), FileNameExt().AsUTF8().c_str());
	return SubsystemType(subsystem[0]);
}

inline bool isdigitchar(int ch) {
	return (ch >= '0') && (ch <= '9');
}

int DecodeMessage(const char *cdoc, char *sourcePath, int format, int &column) {
	sourcePath[0] = '\0';
	column = -1; // default to not detected
	switch (format) {
	case SCE_ERR_PYTHON: {
			// Python
			const char *startPath = strchr(cdoc, '\"');
			if (startPath) {
				startPath++;
				const char *endPath = strchr(startPath, '\"');
				if (endPath) {
					int length = endPath - startPath;
					if (length > 0) {
						strncpy(sourcePath, startPath, length);
						sourcePath[length] = 0;
					}
					endPath++;
					while (*endPath && !isdigitchar(*endPath)) {
						endPath++;
					}
					int sourceNumber = atoi(endPath) - 1;
					return sourceNumber;
				}
			}
		}
	case SCE_ERR_GCC: {
			// GCC - look for number after colon to be line number
			// This will be preceded by file name.
			// Lua debug traceback messages also piggyback this style, but begin with a tab.
			if (cdoc[0] == '\t')
				++cdoc;
			for (int i = 0; cdoc[i]; i++) {
				if (cdoc[i] == ':' && isdigitchar(cdoc[i + 1])) {
					int sourceNumber = atoi(cdoc + i + 1) - 1;
					if (i > 0) {
						strncpy(sourcePath, cdoc, i);
						sourcePath[i] = 0;
					}
					return sourceNumber;
				}
			}
			break;
		}
	case SCE_ERR_MS: {
			// Visual *
			const char *start = cdoc;
			while (isspacechar(*start)) {
				start++;
			}
			const char *endPath = strchr(start, '(');
			int length = endPath - start;
			if ((length > 0) && (length < MAX_PATH)) {
				strncpy(sourcePath, start, length);
				sourcePath[length] = 0;
			}
			endPath++;
			return atoi(endPath) - 1;
		}
	case SCE_ERR_BORLAND: {
			// Borland
			const char *space = strchr(cdoc, ' ');
			if (space) {
				while (isspacechar(*space)) {
					space++;
				}
				while (*space && !isspacechar(*space)) {
					space++;
				}
				while (isspacechar(*space)) {
					space++;
				}

				const char *space2 = NULL;

				if (strlen(space) > 2) {
					space2 = strchr(space + 2, ':');
				}

				if (space2) {
					while (!isspacechar(*space2)) {
						space2--;
					}

					while (isspacechar(*(space2 - 1))) {
						space2--;
					}

					int length = space2 - space;

					if (length > 0) {
						strncpy(sourcePath, space, length);
						sourcePath[length] = '\0';
						return atoi(space2) - 1;
					}
				}
			}
			break;
		}
	case SCE_ERR_PERL: {
			// perl
			const char *at = strstr(cdoc, " at ");
			const char *line = strstr(cdoc, " line ");
			int length = line - (at + 4);
			if (at && line && length > 0) {
				strncpy(sourcePath, at + 4, length);
				sourcePath[length] = 0;
				line += 6;
				return atoi(line) - 1;
			}
			break;
		}
	case SCE_ERR_NET: {
			// .NET traceback
			const char *in = strstr(cdoc, " in ");
			const char *line = strstr(cdoc, ":line ");
			if (in && line && (line > in)) {
				in += 4;
				strncpy(sourcePath, in, line - in);
				sourcePath[line - in] = 0;
				line += 6;
				return atoi(line) - 1;
			}
		}
	case SCE_ERR_LUA: {
			// Lua 4 error looks like: last token read: `result' at line 40 in file `Test.lua'
			const char *idLine = "at line ";
			const char *idFile = "file ";
			size_t lenLine = strlen(idLine);
			size_t lenFile = strlen(idFile);
			const char *line = strstr(cdoc, idLine);
			const char *file = strstr(cdoc, idFile);
			if (line && file) {
				const char *fileStart = file + lenFile + 1;
				const char *quote = strstr(fileStart, "'");
				size_t length = quote - fileStart;
				if (quote && length > 0) {
					strncpy(sourcePath, fileStart, length);
					sourcePath[length] = '\0';
				}
				line += lenLine;
				return atoi(line) - 1;
			} else {
				// Lua 5.1 error looks like: lua.exe: test1.lua:3: syntax error
				// reuse the GCC error parsing code above!
				const char* colon = strstr(cdoc, ": ");
				if (cdoc)
					return DecodeMessage(colon + 2, sourcePath, SCE_ERR_GCC, column);
			}
			break;
		}

	case SCE_ERR_CTAG: {
			for (int i = 0; cdoc[i]; i++) {
				if ((isdigitchar(cdoc[i + 1]) || (cdoc[i + 1] == '/' && cdoc[i + 2] == '^')) && cdoc[i] == '\t') {
					int j = i - 1;
					while (j > 0 && ! strchr("\t\n\r \"$%'*,;<>?[]^`{|}", cdoc[j])) {
						j--;
					}
					if (strchr("\t\n\r \"$%'*,;<>?[]^`{|}", cdoc[j])) {
						j++;
					}
					strncpy(sourcePath, &cdoc[j], i - j);
					sourcePath[i - j] = 0;
					// Because usually the address is a searchPattern, lineNumber has to be evaluated later
					return 0;
				}
			}
		}
	case SCE_ERR_PHP: {
			// PHP error look like: Fatal error: Call to undefined function:  foo() in example.php on line 11
			const char *idLine = " on line ";
			const char *idFile = " in ";
			size_t lenLine = strlen(idLine);
			size_t lenFile = strlen(idFile);
			const char *line = strstr(cdoc, idLine);
			const char *file = strstr(cdoc, idFile);
			if (line && file && (line > file)) {
				file += lenFile;
				size_t length = line - file;
				strncpy(sourcePath, file, length);
				sourcePath[length] = '\0';
				line += lenLine;
				return atoi(line) - 1;
			}
			break;
		}

	case SCE_ERR_ELF: {
			// Essential Lahey Fortran error look like: Line 11, file c:\fortran90\codigo\demo.f90
			const char *line = strchr(cdoc, ' ');
			if (line) {
				while (isspacechar(*line)) {
					line++;
				}
				const char *file = strchr(line, ' ');
				if (file) {
					while (isspacechar(*file)) {
						file++;
					}
					while (*file && !isspacechar(*file)) {
						file++;
					}
					size_t length = strlen(file);
					strncpy(sourcePath, file, length);
					sourcePath[length] = '\0';
					return atoi(line) - 1;
				}
			}
			break;
		}

	case SCE_ERR_IFC: {
			/* Intel Fortran Compiler error/warnings look like:
			 * Error 71 at (17:teste.f90) : The program unit has no name
			 * Warning 4 at (9:modteste.f90) : Tab characters are an extension to standard Fortran 95
			 *
			 * Depending on the option, the error/warning messages can also appear on the form:
			 * modteste.f90(9): Warning 4 : Tab characters are an extension to standard Fortran 95
			 *
			 * These are trapped by the MS handler, and are identified OK, so no problem...
			 */
			const char *line = strchr(cdoc, '(');
			const char *file = strchr(line, ':');
			if (line && file) {
				file++;
				const char *endfile = strchr(file, ')');
				size_t length = endfile - file;
				strncpy(sourcePath, file, length);
				sourcePath[length] = '\0';
				line++;
				return atoi(line) - 1;
			}
			break;
		}

	case SCE_ERR_ABSF: {
			// Absoft Pro Fortran 90/95 v8.x, v9.x  errors look like: cf90-113 f90fe: ERROR SHF3D, File = shf.f90, Line = 1101, Column = 19
			const char *idFile = " File = ";
			const char *idLine = ", Line = ";
			size_t lenFile = strlen(idFile);
			size_t lenLine = strlen(idLine);
			const char *file = strstr(cdoc, idFile);
			const char *line = strstr(cdoc, idLine);
			//const char *idColumn = ", Column = ";
			//const char *column = strstr(cdoc, idColumn);
			if (line && file && (line > file)) {
				file += lenFile;
				size_t length = line - file;
				strncpy(sourcePath, file, length);
				sourcePath[length] = '\0';
				line += lenLine;
				return atoi(line) - 1;
			}
			break;
		}

	case SCE_ERR_IFORT: {
			/* Intel Fortran Compiler v8.x error/warnings look like:
			 * fortcom: Error: shf.f90, line 5602: This name does not have ...
				 */
			const char *idFile = ": Error: ";
			const char *idLine = ", line ";
			size_t lenFile = strlen(idFile);
			size_t lenLine = strlen(idLine);
			const char *file = strstr(cdoc, idFile);
			const char *line = strstr(cdoc, idLine);
			const char *lineend = strrchr(cdoc, ':');
			if (line && file && (line > file)) {
				file += lenFile;
				size_t length = line - file;
				strncpy(sourcePath, file, length);
				sourcePath[length] = '\0';
				line += lenLine;
				if ((lineend > line)) {
					return atoi(line) - 1;
				}
			}
			break;
		}

	case SCE_ERR_TIDY: {
			/* HTML Tidy error/warnings look like:
			 * line 8 column 1 - Error: unexpected </head> in <meta>
			 * line 41 column 1 - Warning: <table> lacks "summary" attribute
			 */
			const char *line = strchr(cdoc, ' ');
			if (line) {
				const char *col = strchr(line + 1, ' ');
				if (col) {
					//*col = '\0';
					int lnr = atoi(line) - 1;
					col = strchr(col + 1, ' ');
					if (col) {
						const char *endcol = strchr(col + 1, ' ');
						if (endcol) {
							//*endcol = '\0';
							column = atoi(col) - 1;
							return lnr;
						}
					}
				}
			}
			break;
		}

	case SCE_ERR_JAVA_STACK: {
			/* Java runtime stack trace
				\tat <methodname>(<filename>:<line>)
				 */
			const char *startPath = strrchr(cdoc, '(') + 1;
			const char *endPath = strchr(startPath, ':');
			int length = endPath - startPath;
			if (length > 0) {
				strncpy(sourcePath, startPath, length);
				sourcePath[length] = 0;
				int sourceNumber = atoi(endPath + 1) - 1;
				return sourceNumber;
			}
			break;
		}

	case SCE_ERR_DIFF_MESSAGE: {
			// Diff file header, either +++ <filename>\t or --- <filename>\t
			// Often followed by a position line @@ <linenumber>
			const char *startPath = cdoc + 4;
			const char *endPath = strchr(startPath, '\t');
			if (endPath) {
				int length = endPath - startPath;
				strncpy(sourcePath, startPath, length);
				sourcePath[length] = 0;
				return 0;
			}
			break;
		}
	}	// switch
	return -1;
}

//!void SciTEBase::GoMessage(int dir) {
bool SciTEBase::GoMessage(int dir, GUI::ScintillaWindow &wBottom) { //!-change-[GoMessageImprovement]
	Sci_CharacterRange crange;
	crange.cpMin = wBottom.Call(SCI_GETSELECTIONSTART);
	crange.cpMax = wBottom.Call(SCI_GETSELECTIONEND);
	int selStart = crange.cpMin;
	int curLine = wBottom.Call(SCI_LINEFROMPOSITION, selStart);
	int maxLine = wBottom.Call(SCI_GETLINECOUNT);
	int lookLine = curLine + dir;
	if (lookLine < 0)
		lookLine = maxLine - 1;
	else if (lookLine >= maxLine)
		lookLine = 0;
	TextReader acc(wBottom);
	while ((dir == 0) || (lookLine != curLine)) {
		int startPosLine = wBottom.Call(SCI_POSITIONFROMLINE, lookLine, 0);
		int lineLength = wBottom.Call(SCI_LINELENGTH, lookLine, 0);
		char style = acc.StyleAt(startPosLine);
		if (style != SCE_ERR_DEFAULT &&
		        style != SCE_ERR_CMD &&
		        style != SCE_ERR_DIFF_ADDITION &&
		        style != SCE_ERR_DIFF_CHANGED &&
		        style != SCE_ERR_DIFF_DELETION) {
			wBottom.Call(SCI_MARKERDELETEALL, static_cast<uptr_t>(-1));
			wBottom.Call(SCI_MARKERDEFINE, 0, SC_MARK_SMALLRECT);
			wBottom.Call(SCI_MARKERSETFORE, 0, ColourOfProperty(props,
			        "error.marker.fore", ColourRGB(0x7f, 0, 0)));
			wBottom.Call(SCI_MARKERSETBACK, 0, ColourOfProperty(props,
//!			        "error.marker.back", ColourRGB(0xff, 0xff, 0)));
			        "error.line.back", ColourOfProperty(props, "error.marker.back", ColourRGB(0xff, 0xff, 0)))); //!-change-[ErrorLineBack]
			wBottom.Call(SCI_MARKERADD, lookLine, 0);
			wBottom.Call(SCI_SETSEL, startPosLine, startPosLine);
			SString message = GetRange(wBottom, startPosLine, startPosLine + lineLength);
			char source[MAX_PATH];
			int column;
			int sourceLine = DecodeMessage(message.c_str(), source, style, column);
			SString findHiglight = "";
			if (style == SCE_ERR_GCC){
				int posInLine = 0; //позиция в строке редактора, с которой начинается найденное слово
				int lenSelected = 0; //длина найденного слова
				int posInFind = 0; //начало найденной строки врезультатах поиска
				//ищем позицию и длину найденного слова
				while (wBottom.Call(SCI_GETSTYLEAT, startPosLine + posInFind) == SCE_ERR_GCC && lineLength > posInFind) posInFind++;
				while (wBottom.Call(SCI_GETSTYLEAT, startPosLine + posInFind + posInLine) != SCE_ERR_FIND_VALUE && lineLength > posInFind + posInLine) posInLine++;
				while (wBottom.Call(SCI_GETSTYLEAT, startPosLine + posInFind + posInLine + lenSelected) == SCE_ERR_FIND_VALUE && lineLength > posInFind + posInLine + lenSelected) lenSelected++;
				findHiglight = message.substr(posInFind + posInLine, lenSelected);
			}
			if (sourceLine >= 0) {
//!-start-[FindResultListStyle]
				if (props.GetInt("lexer.errorlist.findliststyle", 1)&& '.' == source[0] && pathSepChar == source[1]) {
					// it can be internal search result line, so try to find the base path in topic
					int topLine = lookLine - 1;
					SString topic;
					while (topLine >= 0) {
						int startPos = wBottom.Call(SCI_POSITIONFROMLINE, topLine, 0);
						int lineLen = wBottom.Call(SCI_LINELENGTH, topLine, 0);
						topic = GetRange(wBottom, startPos, startPos + lineLen);
						if ('>' == topic[0]) {
							break;
						} else {
							topic.clear();
							topLine--;
						}
					}
					if (topic.length() > 0 && 0 == strncmp(">??Internal search",topic.c_str(),18)) {
						// get base path from topic text
						int toPos = topic.length() - 1;
						while (toPos >= 0 && pathSepChar != topic[toPos]) toPos--;
						int fromPos = toPos - 1;
						while (fromPos >= 0 && '"' != topic[fromPos]) fromPos--;
						if (fromPos > 0) {
							SString path = topic.substr(fromPos + 1, toPos - fromPos - 1);
							path.append(source + 1);
							strncpy(source, path.c_str(), MAX_PATH);
						}
					}
				}
//!-end-[FindResultListStyle]

				GUI::gui_string sourceString = GUI::StringFromUTF8(source);
				FilePath sourcePath(sourceString);
//!				if (!filePath.Name().SameNameAs(sourcePath)) {
				if (sourcePath.IsSet() && !filePath.Name().SameNameAs(sourcePath)) { //!-change-[GoMessageFix]
					FilePath messagePath;
					bool bExists = false;
					if (Exists(dirNameAtExecute.AsInternal(), sourceString.c_str(), &messagePath)) {
						bExists = true;
					} else if (Exists(dirNameForExecute.AsInternal(), sourceString.c_str(), &messagePath)) {
						bExists = true;
					} else if (Exists(filePath.Directory().AsInternal(), sourceString.c_str(), &messagePath)) {
						bExists = true;
					} else if (Exists(NULL, sourceString.c_str(), &messagePath)) {
						bExists = true;
					} else {
						// Look through buffers for name match
						for (int i = buffers.length - 1; i >= 0; i--) {
							if (sourcePath.Name().SameNameAs(buffers.buffers[i].Name())) {
								messagePath = buffers.buffers[i];
								bExists = true;
							}
						}
					}
					if (bExists) {
						if (!Open(messagePath)) {
							//!return;
							return false; //!-change-[GoMessageImprovement]
						}
						CheckReload();
					}
				}

				// If ctag then get line number after search tag or use ctag line number
				if (style == SCE_ERR_CTAG) {
					char cTag[200];
					//without following focus GetCTag wouldn't work correct
					WindowSetFocus(wBottom);
					GetCTag(cTag, 200);
					if (cTag[0] != '\0') {
						if (atol(cTag) > 0) {
							//if tag is linenumber, get line
							sourceLine = atol(cTag) - 1;
						} else {
							findWhat = cTag;
							//FindNext(false);
							//get linenumber for marker from found position
							sourceLine = wEditor.Call(SCI_LINEFROMPOSITION, wEditor.Call(SCI_GETCURRENTPOS));
						}
					}
				}
				if (extender) extender->OnNavigation("Go");
				WindowSetFocus(wEditor);

				wEditor.Call(SCI_MARKERDELETEALL, markerError);

				wEditor.Call(SCI_MARKERADD, sourceLine, markerError);
				int startSourceLine = wEditor.Call(SCI_POSITIONFROMLINE, sourceLine, 0);
				int endSourceline = wEditor.Call(SCI_POSITIONFROMLINE, sourceLine + 1, 0);
				if (column >= 0) {
					// Get the position in line according to current tab setting
					startSourceLine = wEditor.Call(SCI_FINDCOLUMN, sourceLine, column);
				}
				EnsureRangeVisible(startSourceLine, startSourceLine);
				if (props.GetInt("error.select.line") == 1) {
					//select whole source source line from column with error
					if (findHiglight != "") {
						SString findingStr = GetRange(wEditor, startSourceLine, endSourceline);
						int fPos = findingStr.search(findHiglight.c_str());
						SetSelection(startSourceLine + fPos, startSourceLine + fPos + findHiglight.length());
						wEditor.Call(SCI_SCROLLCARET);
					} else {
						SetSelection(endSourceline, startSourceLine);
					}
				} else {
					//simply move cursor to line, don't do any selection
					SetSelection(startSourceLine, startSourceLine);
				}
				if (extender) extender->OnNavigation("Go-");
				message.substitute('\t', ' ');
				message.remove("\n");
				props.Set("CurrentMessage", message.c_str());
				return false; //!-add-[GoMessageImprovement]
			}
			//!return;
			return false; //!-change-[GoMessageImprovement]
		}
		lookLine += dir;
		if (lookLine < 0)
			lookLine = maxLine - 1;
		else if (lookLine >= maxLine)
			lookLine = 0;
		if (dir == 0)
			//!return;
			return false; //!-change-[GoMessageImprovement]
	}
	return false; //!-add-[GoMessageImprovement]
}

