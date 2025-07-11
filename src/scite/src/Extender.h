// SciTE - Scintilla based Text Editor
/** @file Extender.h
 ** SciTE extension interface.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef EXTENDER_H
#define EXTENDER_H

#include "Scintilla.h"
#include <map>
class StyleWriter;

class ExtensionAPI {
public:
	virtual ~ExtensionAPI() {
	}
	enum Pane { paneEditor = 1, paneCoEditor = 2, paneOutput = 3, paneFindRes = 4 };
	virtual sptr_t Send(Pane p, Scintilla::Message, uptr_t wParam=0, sptr_t lParam=0)=0;
	virtual char *Range(Pane p, Sci_Position start, Sci_Position end)=0;
	virtual char *Line(Pane p, Sci_Position line, int bNeedEnd)=0;
	virtual void Remove(Pane p, Sci_Position start, Sci_Position end)=0;
	virtual void Insert(Pane p, Sci_Position pos, const char *s)=0;
	virtual void Trace(const char *s)=0;
	virtual char *Property(const char *key)=0;
	virtual void SetProperty(const char *key, const char *val)=0;
	virtual void UnsetProperty(const char *key)=0;
	virtual uptr_t GetInstance()=0;
	virtual void ShutDown()=0;
	virtual void DoMenuCommand(int cmdID)=0;
	virtual void CheckMenus()=0; //!-add-[CheckMenus]
	virtual char *GetTranslation(const char *s, bool retainIfNotFound = true)=0; //!-add-[LocalizationFromLua]
	virtual int PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter) = 0;
	virtual void SetCoDocumentAt(int index, bool bSetBuffersMenu = true) = 0;
	virtual void SetDocumentAt(int index, bool updateStack = true, bool switchTab = true, bool bExit = false) = 0;
	virtual int GetBuffersCount() = 0;
	virtual int GetCurrentBufer() = 0;
	virtual std::string GetCoBufferName() = 0;
	virtual std::string GetBufferName(int i) = 0;
	virtual void SetBufferEncoding(int i, int e) = 0;
	virtual int GetBufferEncoding(int i) = 0;
	virtual void ClearBufferFileTime(int i) = 0;
	virtual int GetBufferFileTime(int i) = 0;
	virtual bool GetBuffersSavedState(int i) = 0;
	virtual void UpdateBuffersCoCurrent(void) = 0;
	virtual void SetAcceleratorTable(void *h) = 0;
	virtual void EnsureVisible() = 0;
	virtual void HideForeReolad(int close) = 0;
	virtual void SetOverrideLanguage(const char *lexer, bool bFireEvent) = 0;
	virtual bool SwitchMouseHook(bool bSet) = 0;
	virtual void RunInConcole() = 0;
	virtual void ExecuteHelp(const char *cmd, int hh_cmd) = 0;
	virtual void RunAsync(int idx)=0;
	virtual int ActiveEditor()=0;
	virtual int GetBufferSide(int index) = 0;
	virtual time_t GetBufferModTime(int index) = 0;
	virtual void UiDocTabulation(int side, bool forward)=0;
	virtual bool GetBufferReadOnly(int index) = 0;
	virtual int GetBufferUnicMode(int index) = 0;
	virtual int GetBufferOrder(int index) = 0;
	virtual int SecondEditorActive() = 0;
	virtual int Cloned(int index) = 0;
	virtual int IndexOfClone(int index) = 0;
	virtual int BufferByName(const char* c) = 0;
	virtual bool Open_script(const char* path) = 0;
	virtual void ReloadProperties() = 0;
	virtual void Close_script() = 0;
	virtual void SavePositions() = 0;
	virtual void BlockUpdate(int cmd) = 0;
	virtual void SetRestart(const char* cmdLine)=0;
	virtual int WindowMessageBox(const char* msg, int flag, const GUI::gui_char *p1, const GUI::gui_char *p2, const GUI::gui_char *p3)=0;
	virtual int CompareFile(FilePath &fileCompare, const char* txtCompare) = 0;
	virtual void OrderTabsBy(std::map<int, int> &order) = 0;
	virtual void PostLoadScript() = 0;
	virtual bool IsRunAsAdmin()=0;
	virtual bool NewInstance(const char* arg, bool asAdmin) =0;
	virtual bool ReadForScript(FilePath &fileCompare, void** convert, char** data, size_t &lenFile) = 0;
	virtual unsigned long InvertColor(unsigned long clr) = 0;
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
	virtual bool OnBeforeOpen(const char *filename, const char *extension, int& encoding) { return false; }
	virtual bool OnSave(const char *) { return false; }
	virtual bool OnChar(char) { return false; }
	virtual bool OnExecute(const char *) { return false; }
	virtual bool OnSavePointReached() { return false; }
	virtual bool OnSavePointLeft() { return false; }
	virtual bool OnStyle(Sci_Position, Sci_Position, int, StyleWriter *) {
		return false;
	}
//!	virtual bool OnDoubleClick() { return false; }
	virtual bool OnDoubleClick(int) { return false; } //!-add-[OnDoubleClick]
	virtual bool OnClick(int) { return false; } //!-add-[OnClick]
	virtual bool OnMouseButtonUp(int) { return false; } //!-add-[OnMouseButtonUp]
	virtual bool OnHotSpotReleaseClick(int) { return false; } //!-add-[OnHotSpotReleaseClick]
	virtual bool PaneOnUpdateUI(bool bModified, bool bSelChange, int flag) { return false; }
	virtual bool CoOnUpdateUI(bool bModified, bool bSelChange, int flag) { return false; }
	virtual bool OnUpdateUI(bool bModified, bool bSelChange, int flag) { return false; }
	virtual bool OnMarginClick(unsigned int margin, unsigned int modif, long line, uptr_t id) { return false; }
	virtual bool OnMacro(const char *, uptr_t, sptr_t, const char *) { return false; }
	virtual bool OnUserListSelection(int, const char *, Sci_Position, int) { return false; } //!-change-[UserListItemID]
	virtual bool OnNavigation(const char *item) { return false; }
	virtual bool OnMenuCommand(int, int) { return false; } //!-add-[OnMenuCommand]
	virtual const char *OnSendEditor(unsigned int, uptr_t, const char *) { return 0; } //!-add-[OnSendEditor]
	virtual const char *OnSendEditor(unsigned int, uptr_t, sptr_t) { return 0; } //!-add-[OnSendEditor]
	virtual bool OnLindaNotify(const char*, const char*) { return false; }
	virtual bool OnCallTipClick(Sci_Position pos) { return false; }

	virtual bool SendProperty(const char *) { return false; }

//!	virtual bool OnKey(int, int) { return false; }
	virtual bool OnKey(int, int, char) { return false; } //!-change-[OnKey]

	virtual bool OnDwellStart(Sci_Position, const char *, bool) { return false; }
	virtual bool OnClose(const char *) { return false; }
	virtual bool OnColorized(Sci_Position, Sci_Position) { return false; }
	virtual const char *OnContextMenu(unsigned int msg, unsigned int wp, const char *lp){ return 0; }
	virtual bool OnFindProgress(uptr_t state, sptr_t all){ return false; }
	virtual bool OnPostCallback(int idx){ return false; }
	virtual bool OnIdle(){ return false; }
	virtual bool OnLayOutNotify(const char *){ return false; }
	virtual bool OnGeneratedHotKey(long) { return false; }
	virtual void DoReboot(){ return; };
	virtual void DoLua(const char * c){ return; };
	virtual void OnMouseHook(int x, int y){ return; };
	virtual bool OnMacroBlocked(int msg, intptr_t wParam, intptr_t lParam){ return false; };
	virtual sptr_t  OnMenuChar(int flag, const char* key) { return 0; };
	virtual bool OnDrawClipboard(int) { return false; }
	virtual void OnRightEditorVisibility(bool) {}
	virtual void OnTextChanged(Sci_Position position, int leg, Sci_Position linesAdded, int flag) { return; };
	virtual void BeforeTextChanged(Sci_Position position, int leg, const char * t, int type) { return; };
	virtual void OnCurrentLineFold(Sci_Position line, int leg, int foldLevelPrev, int foldLevelNow) { return; };
	virtual bool OnAutocSelection(int method, Sci_Position firstPos) { return false; };
	virtual void OnCommandLine(const char* line) { };
	virtual void OnChangeFocus(int, int) {};
	virtual int HildiAlarm(const char* msg, int flag, const GUI::gui_char *p1 = NULL, const GUI::gui_char *p2 = NULL, const GUI::gui_char *p3 = NULL) { return 0; }
	virtual GUI::gui_string LocalizeText(const char* msg) { GUI::gui_string s; return s; };
	virtual void OnShowActivate(int bActivate, int cmdShow, int SavePoint = 0) {};
};

#endif
