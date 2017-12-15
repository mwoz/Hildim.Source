// SciTE - Scintilla based Text Editor
/** @file SingleThreadExtension.h
 ** Extension that wraps another extension so that OnExecute calls are always
 ** seen by the wrapped extension as coming from the initial thread, despite
 ** the fact that SciTE calls it from a worker thread on Windows.
 **/
// Copyright 1998-2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SINGLETHREADEXTENSION_H
#define SINGLETHREADEXTENSION_H

#include <windows.h>
#include "Extender.h"

class SingleThreadExtension: public Extension {
public:
	SingleThreadExtension(Extension &ext_) : ext(&ext_), hwndDispatcher(NULL) {}
	virtual ~SingleThreadExtension() { Finalise(); }

	virtual bool Initialise(ExtensionAPI *host_);
	virtual void PostInit(void* h);
	virtual void* GetLuaState();
	virtual bool Finalise();
	virtual bool Clear();
	virtual bool Load(const char *filename);

	virtual bool InitBuffer(int);
	virtual bool ActivateBuffer(int);
	virtual bool RemoveBuffer(int);

	virtual bool OnOpen(const char *);
	virtual bool OnSwitchFile(const char *);
	virtual bool OnBeforeSave(const char *);
	virtual bool OnBeforeOpen(const char *, const char *);
	virtual bool OnSave(const char *);
	virtual bool OnChar(char);
	virtual bool OnExecute(const char *);
	virtual bool OnSavePointReached();
	virtual bool OnSavePointLeft();
	virtual bool OnStyle(unsigned int, int, int, StyleWriter *);
//!	virtual bool OnDoubleClick();
	virtual bool OnDoubleClick(int modifiers); //!-add-[OnDoubleClick]
	virtual bool OnClick(int modifiers); //!-add-[OnClick]
	virtual bool OnMouseButtonUp(int modifiers); //!-add-[OnMouseButtonUp]
	virtual bool OnHotSpotReleaseClick(int modifiers); //!-add-[OnHotSpotReleaseClick]
	virtual bool CoOnUpdateUI(bool bModified, bool bSelChange, int flag);
	virtual bool OnUpdateUI(bool bModified, bool bSelChange, int flag);
	virtual bool OnMarginClick(unsigned int margin, unsigned int modif, long line);
	virtual bool OnMacro(const char *, const char *);
//!	virtual bool OnUserListSelection(int, const char *);
	virtual bool OnUserListSelection(int, const char *, int); //!-change-[UserListItemID]
	virtual bool OnNavigation(const char *);
	virtual bool SendProperty(const char *);
//!-start-[OnKey]
#if !defined(GTK)
	virtual bool OnKey(int, int, char);
#else
//!-end-[OnKey]
	virtual bool OnKey(int, int);
#endif //!-add-[OnKey]
	virtual bool OnDwellStart(int, const char *);
	virtual bool OnClose(const char *);
	virtual bool OnMenuCommand(int, int); //!-add-[OnMenuCommand]
	virtual const char *OnSendEditor(unsigned int, unsigned int, const char *); //!-add-[OnSendEditor]
	virtual const char *OnSendEditor(unsigned int, unsigned int, long); //!-add-[OnSendEditor]
	virtual bool OnColorized(unsigned int, unsigned int);
	virtual const char *OnContextMenu(unsigned int msg, unsigned int wp, const char *lp);
	virtual bool OnFindProgress(int state, int all);
	virtual bool OnPostCallback(int idx);
	virtual bool OnIdle();
	virtual bool OnLayOutNotify(const char *);
	virtual bool OnGeneratedHotKey(long);
	virtual void DoReboot();
	virtual void DoLua(const char * c);
	virtual void OnMouseHook(int x, int y);
	virtual bool OnDrawClipboard(int flag);
	virtual void OnRightEditorVisibility(bool show);
protected:
	Extension *ext;

	// Since the number of extensions requiring this is never likely to be large,
	// each wrapped extension gets its own dispatcher window.
	HWND hwndDispatcher;
	static LRESULT PASCAL WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	// Copying is unsupported.
	SingleThreadExtension(const SingleThreadExtension & copy);
	SingleThreadExtension & operator=(const SingleThreadExtension & copy);
};

#endif
