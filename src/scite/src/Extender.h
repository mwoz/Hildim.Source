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
enum class EditorSide : int;

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
	virtual SString PropertyNewExpand(const char * base, const char * fileNameExt)=0;
	virtual void SetProperty(const char *key, const char *val)=0;
	virtual void UnsetProperty(const char *key)=0;
	virtual uptr_t GetInstance()=0;
	virtual void ShutDown()=0;
	virtual void DoMenuCommand(int cmdID)=0;
	virtual void CheckMenus()=0; //!-add-[CheckMenus]
	virtual char *GetTranslation(const char *s, bool retainIfNotFound = true)=0; //!-add-[LocalizationFromLua]
	virtual int PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter) = 0;
	virtual void SetCoDocumentAt(int index, bool bSetBuffersMenu = true) = 0;
	virtual void SetDocumentAt(int index, bool updateStack = true, bool switchTab = true, bool bExit = false, bool setAround = true) = 0;
	virtual int GetBuffersCount() = 0;
	virtual int GetCurrentBufer() = 0;
	virtual bool GetBufferUntitled(int i) = 0;
	virtual char GetEditorSide() = 0;
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
	virtual void UiDocTabulation(EditorSide side, bool forward)=0;
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
	virtual bool NewFile(const GUI::gui_char* defName = nullptr, const GUI::gui_char* defExt = nullptr, int encoding = 0, const char* txt = nullptr, const GUI::gui_char* path = nullptr, bool unic = false) = 0;
	virtual void ChangeTabWnd(int insertIn = -1) = 0;
	virtual void CloneTab(int insertIn = -1) = 0;
};

#endif
