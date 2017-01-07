// SciTE - Scintilla based Text Editor
/** @file Extender.h
 ** SciTE extension interface.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef EXTENDER_H
#define EXTENDER_H

#include "Scintilla.h"

class StyleWriter;

class ExtensionAPI {
public:
	virtual ~ExtensionAPI() {
	}
	enum Pane { paneEditor = 1, paneOutput = 2, paneFindRes = 3 };
	virtual sptr_t Send(Pane p, unsigned int msg, uptr_t wParam=0, sptr_t lParam=0)=0;
	virtual char *Range(Pane p, int start, int end)=0;
	virtual void Remove(Pane p, int start, int end)=0;
	virtual void Insert(Pane p, int pos, const char *s)=0;
	virtual void Trace(const char *s)=0;
	virtual char *Property(const char *key)=0;
	virtual void SetProperty(const char *key, const char *val)=0;
	virtual void UnsetProperty(const char *key)=0;
	virtual uptr_t GetInstance()=0;
	virtual void ShutDown()=0;
	virtual void Perform(const char *actions)=0;
	virtual void DoMenuCommand(int cmdID)=0;
	virtual void CheckMenus()=0; //!-add-[CheckMenus]
	virtual bool ShowParametersDialog(const char *msg)=0; //!-add-[ParametersDialogFromLua]
	virtual char *GetTranslation(const char *s, bool retainIfNotFound = true)=0; //!-add-[LocalizationFromLua]
	virtual int RunLuaThread(const char *s, const char *desc) = 0;
	virtual int PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter) = 0;
	virtual void SetDocumentAt(int index, bool updateStack = true, bool switchTab = true) = 0;
	virtual int GetBuffersCount() = 0;
	virtual int GetCurrentBufer() = 0;
	virtual void GetBufferName(int i, char* c) = 0;
	virtual bool GetBuffersSavedState(int i) = 0;
	virtual void SetAcceleratorTable(void *h) = 0;
	virtual void EnsureVisible() = 0;
	virtual void HideForeReolad() = 0;
	virtual void SetOverrideLanguage(const char *lexer, bool bFireEvent) = 0;
	virtual bool SwitchMouseHook(bool bSet) = 0;
	virtual void RunInConcole() = 0;
	virtual void ExecuteHelp(const char *cmd, int hh_cmd) = 0;
	virtual void RunAsync(int idx)=0;
};

/**
 * Methods in extensions return true if they have completely handled an event and
 * false if default processing is to continue.
 */
class Extension {
public:
	virtual ~Extension() {}

	virtual bool Initialise(ExtensionAPI *host_)=0;
	virtual void PostInit(void* h) = 0;
	virtual bool Finalise()=0;
	virtual void* GetLuaState() = 0;
	virtual bool Clear()=0;
	virtual bool Load(const char *filename)=0;

	virtual bool InitBuffer(int) { return false; }
	virtual bool ActivateBuffer(int) { return false; }
	virtual bool RemoveBuffer(int) { return false; }

	virtual bool OnOpen(const char *) { return false; }
	virtual bool OnSwitchFile(const char *) { return false; }
	virtual bool OnBeforeSave(const char *) { return false; }
	virtual bool OnBeforeOpen(const char *filename, const char *extension) { return false; }
	virtual bool OnSave(const char *) { return false; }
	virtual bool OnChar(char) { return false; }
	virtual bool OnExecute(const char *) { return false; }
	virtual bool OnSavePointReached() { return false; }
	virtual bool OnSavePointLeft() { return false; }
	virtual bool OnStyle(unsigned int, int, int, StyleWriter *) {
		return false;
	}
//!	virtual bool OnDoubleClick() { return false; }
	virtual bool OnDoubleClick(int) { return false; } //!-add-[OnDoubleClick]
	virtual bool OnClick(int) { return false; } //!-add-[OnClick]
	virtual bool OnMouseButtonUp(int) { return false; } //!-add-[OnMouseButtonUp]
	virtual bool OnHotSpotReleaseClick(int) { return false; } //!-add-[OnHotSpotReleaseClick]
	virtual bool OnUpdateUI(bool bModified, bool bSelChange, int flag) { return false; }
	virtual bool OnMarginClick(unsigned int margin, unsigned int modif, long line) { return false; }
	virtual bool OnMacro(const char *, const char *) { return false; }
//!	virtual bool OnUserListSelection(int, const char *) { return false; }
	virtual bool OnUserListSelection(int, const char *,int) { return false; } //!-change-[UserListItemID]
	virtual bool OnNavigation(const char *item) { return false; }
	virtual bool OnMenuCommand(int, int) { return false; } //!-add-[OnMenuCommand]
	virtual const char *OnSendEditor(unsigned int, unsigned int, const char *) { return 0; } //!-add-[OnSendEditor]
	virtual const char *OnSendEditor(unsigned int, unsigned int, long) { return 0; } //!-add-[OnSendEditor]

	virtual bool SendProperty(const char *) { return false; }

//!	virtual bool OnKey(int, int) { return false; }
	virtual bool OnKey(int, int, char) { return false; } //!-change-[OnKey]

	virtual bool OnDwellStart(int, const char *) { return false; }
	virtual bool OnClose(const char *) { return false; }
	virtual bool OnColorized(unsigned int, unsigned int) { return false; }
	virtual const char *OnContextMenu(unsigned int msg, unsigned int wp, const char *lp){ return 0; }
	virtual bool OnFindProgress(int state, int all){ return false; }
	virtual bool OnPostCallback(int idx){ return false; }
	virtual bool OnIdle(){ return false; }
	virtual bool OnLayOutNotify(const char *){ return false; }
	virtual bool OnGeneratedHotKey(long) { return false; }
	virtual void DoReboot(){ return; };
	virtual void DoLua(const char * c){ return; };
	virtual void OnMouseHook(int x, int y){ return; };
	virtual bool OnDrawClipboard(int) { return false; }
};

#endif
