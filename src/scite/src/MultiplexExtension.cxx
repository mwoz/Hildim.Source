// SciTE - Scintilla based Text Editor
/** @file MultiplexExtension.cxx
 ** Extension that manages / dispatches messages to multiple extensions.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <string>
#include "Scintilla.h"
#include "GUI.h"
#include "MultiplexExtension.h"

MultiplexExtension::MultiplexExtension(): extensions(0), extensionCount(0), host(0) {}

MultiplexExtension::~MultiplexExtension() {
	Finalise();
	delete [] extensions;
}

bool MultiplexExtension::RegisterExtension(Extension &ext_) {
	for (int i = 0; i < extensionCount; ++i)
		if (extensions[i] == &ext_)
			return true;

	Extension **newExtensions = new Extension *[extensionCount+1];

	if (newExtensions) {
		if (extensions) {
			for (int i = 0; i < extensionCount; ++i)
				newExtensions[i] = extensions[i];
			delete[] extensions;
		}

		extensions = newExtensions;
		extensions[extensionCount++] = &ext_;

		if (host)
			ext_.Initialise(host);

		return true;
	} else {
		return false;
	}
}


// Initialise, Finalise, Clear, and SetProperty get broadcast to all extensions,
// regardless of return code.  This does not strictly match the documentation, but
// seems like the right thing to do.  The others methods stop processing once one
// Extension returns true.
//
// Load will eventually be changed to be smarter, so that each Extension can have
// something different loaded into it.  OnExecute and OnMacro might also be made
// smarter with a syntax to indicate to which extension the command should be sent.

bool MultiplexExtension::Initialise(ExtensionAPI *host_) {
	if (host)
		Finalise(); // shouldn't happen.

	host = host_;
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->Initialise(host_);

	return false;
}

void MultiplexExtension::PostInit(void* h){
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->PostInit(h);
}
void* MultiplexExtension::GetLuaState(){
	void *L;
	for (int i = 0; i < extensionCount; ++i){
		L = extensions[i]->GetLuaState();
		if (L) return L;
	}
	return NULL;
}

bool MultiplexExtension::Finalise() {
	if (host) {
		for (int i = extensionCount - 1; i >= 0; --i)
			extensions[i]->Finalise();

		host = 0;
	}
	return false;
}

bool MultiplexExtension::Clear() {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->Clear();
	return false;
}

bool MultiplexExtension::Load(const char *filename) {
	bool handled = false;

	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->Load(filename))
			handled = true;

	return handled;
}

bool MultiplexExtension::InitBuffer(int index) {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->InitBuffer(index);
	return false;
}

bool MultiplexExtension::ActivateBuffer(int index) {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->ActivateBuffer(index);
	return false;
}

bool MultiplexExtension::RemoveBuffer(int index) {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->RemoveBuffer(index);
	return false;
}

bool MultiplexExtension::OnOpen(const char *filename) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnOpen(filename))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnSwitchFile(const char *filename) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnSwitchFile(filename))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnBeforeSave(const char *filename) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
	if (extensions[i]->OnBeforeSave(filename))
		handled = true;
	return handled;
}

bool MultiplexExtension::OnBeforeOpen(const char *filename, const char *extension) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
	if (extensions[i]->OnBeforeOpen(filename, extension))
		handled = true;
	return handled;
}

bool MultiplexExtension::OnSave(const char *filename) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnSave(filename))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnChar(char c) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnChar(c))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnExecute(const char *cmd) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnExecute(cmd))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnSavePointReached() {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnSavePointReached())
			handled = true;
	return handled;
}

bool MultiplexExtension::OnSavePointLeft() {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnSavePointLeft())
			handled = true;
	return handled;
}

bool MultiplexExtension::OnStyle(unsigned int p, int q, int r, StyleWriter *s) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnStyle(p, q, r, s))
			handled = true;
	return handled;
}

/*!
bool MultiplexExtension::OnDoubleClick() {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnDoubleClick())
			handled = true;
	return handled;
}
*/
//!-start-[OnDoubleClick]
bool MultiplexExtension::OnDoubleClick(int modifiers) {
	bool handled = false;
	for (int i=0; i<extensionCount && !handled; ++i)
                if (extensions[i]->OnDoubleClick(modifiers))
                        handled = true;
	return handled;
}
//!-end-[OnDoubleClick]

//!-start-[OnClick]
bool MultiplexExtension::OnClick(int modifiers) {
	bool handled = false;
	for (int i=0; i<extensionCount && !handled; ++i)
		if (extensions[i]->OnClick(modifiers))
			handled = true;
		return handled;
}
//!-end-[OnClick]

//!-start-[OnHotSpotReleaseClick]
bool MultiplexExtension::OnHotSpotReleaseClick(int modifiers) {
	bool handled = false;
	for (int i=0; i<extensionCount && !handled; ++i)
		if (extensions[i]->OnHotSpotReleaseClick(modifiers))
			handled = true;
		return handled;
}
//!-end-[OnHotSpotReleaseClick]

//!-start-[OnMouseButtonUp]
bool MultiplexExtension::OnMouseButtonUp(int modifiers) {
	bool handled = false;
	for (int i=0; i<extensionCount && !handled; ++i)
		if (extensions[i]->OnMouseButtonUp(modifiers))
			handled = true;
		return handled;
}
//!-end-[OnMouseButtonUp]

bool MultiplexExtension::OnUpdateUI(bool bModified, bool bSelChange, int flag) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
	if (extensions[i]->OnUpdateUI(bModified, bSelChange, flag))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnMarginClick(unsigned int margin, unsigned int modif, long line) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
	if (extensions[i]->OnMarginClick(margin, modif, line))
			handled = true;
	return handled;
}

bool MultiplexExtension::OnMacro(const char *p, const char *q) {
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnMacro(p, q))
			handled = true;
	return handled;
}

//!bool MultiplexExtension::OnUserListSelection(int listType, const char *selection) {
bool MultiplexExtension::OnUserListSelection(int listType, const char *selection, int id) { //!-change-[UserListItemID]
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
//!		if (extensions[i]->OnUserListSelection(listType, selection))
		if (extensions[i]->OnUserListSelection(listType, selection, id)) //!-change-[UserListItemID]
			handled = true;
	return handled;
}
bool MultiplexExtension::OnNavigation(const char *item){
	bool handled = false;
	for (int i = 0; i < extensionCount && !handled; ++i)
		if (extensions[i]->OnNavigation(item))
			handled = true;
	return handled;
}
bool MultiplexExtension::SendProperty(const char *prop) {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->SendProperty(prop);
	return false;
}

/*
bool MultiplexExtension::OnKey(int keyval, int modifiers) {
	bool handled = false;
	for (int i = 0; i < extensionCount; ++i)
		if (extensions[i]->OnKey(keyval, modifiers))
			handled = true;
	return handled;
}
*/
//!-start-[OnKey]
bool MultiplexExtension::OnKey(int keyval, int modifiers, char ch) {
	bool handled = false;
	for (int i = 0; i < extensionCount; ++i)
		if (extensions[i]->OnKey(keyval, modifiers, ch))
			handled = true;
	return handled;
}
//!-end-[OnKey]

bool MultiplexExtension::OnDwellStart(int pos, const char *word) {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->OnDwellStart(pos, word);
	return false;
}

bool MultiplexExtension::OnClose(const char *filename) {
	for (int i = 0; i < extensionCount; ++i)
		extensions[i]->OnClose(filename);
	return false;
}

//!-start-[OnMenuCommand]
bool MultiplexExtension::OnMenuCommand(int cmd, int source) {
	bool handled = false;
	for (int i=0; i<extensionCount && !handled; ++i)
                if (extensions[i]->OnMenuCommand(cmd,source))
                        handled = true;
	return handled;
}
//!-end-[OnMenuCommand]

//!-start-[OnSendEditor]
const char *MultiplexExtension::OnSendEditor(unsigned int msg, unsigned int wp, const char *lp) {
	const char *result = 0;
	for (int i=0; i<extensionCount; ++i) {
		result = extensions[i]->OnSendEditor(msg,wp,lp);
		if (result != 0) break;
	}
	return result;
}

const char *MultiplexExtension::OnSendEditor(unsigned int msg, unsigned int wp, long lp) {
	const char *result = 0;
	for (int i=0; i<extensionCount; ++i) {
		result = extensions[i]->OnSendEditor(msg,wp,lp);
		if (result != 0) break;
	}
	return result;
}
bool MultiplexExtension::OnColorized(unsigned int wp, unsigned int lp) {
	for (int i=0; i<extensionCount; ++i) {
		if(extensions[i]->OnColorized(wp,lp)) return true;
	}
	return false;
}

const char *MultiplexExtension::OnContextMenu(unsigned int msg, unsigned int wp, const char *lp) {
	const char *result = 0;
	for (int i = 0; i<extensionCount; ++i) {
		result = extensions[i]->OnContextMenu(msg, wp, lp);
		if (result != 0) break;
	}
	return result;
}
bool MultiplexExtension::OnFindProgress(int state, int all) {
	for (int i = 0; i<extensionCount; ++i) {
		if (extensions[i]->OnFindProgress(state, all)) return true;
	}
	return false;
}

bool MultiplexExtension::OnPostCallback(int idx) {
	for (int i = 0; i<extensionCount; ++i) {
		if (extensions[i]->OnPostCallback(idx)) return true;
	}
	return false;
}

bool MultiplexExtension::OnIdle() {
	for (int i = 0; i<extensionCount; ++i) {
		if (extensions[i]->OnIdle()) return true;
	}
	return false;
}

bool MultiplexExtension::OnLayOutNotify(const char *command) {
	for (int i = 0; i<extensionCount; ++i) {
		if (extensions[i]->OnLayOutNotify(command)) return true;
	}
	return false;
}

bool MultiplexExtension::OnGeneratedHotKey(long hotkey) {
	for (int i = 0; i<extensionCount; ++i) {
		if (extensions[i]->OnGeneratedHotKey(hotkey)) return true;
	}
	return false;
}

void MultiplexExtension::DoReboot(){
	if (extensionCount > 0) extensions[0]->DoReboot();
}

void MultiplexExtension::DoLua(const char * c){
	if (extensionCount > 0) extensions[0]->DoLua(c);
}
void MultiplexExtension::OnMouseHook(int x, int y){
	for (int i = 0; i<extensionCount; ++i) {
		extensions[i]->OnMouseHook(x,y);
	}
}
bool MultiplexExtension::OnDrawClipboard(int flag) {
	for (int i = 0; i<extensionCount; ++i) {
		if (extensions[i]->OnDrawClipboard(flag)) return true;
	}
	return false;
}

//!-end-[OnSendEditor]
