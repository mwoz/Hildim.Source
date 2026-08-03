// SciTE - Scintilla based Text Editor
// LuaExtension.h - Lua scripting extension
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef LUAEXTENSION_H
#define LUAEXTENSION_H

class SciTEBase;

class LuaExtension {

public:

	LuaExtension();
	virtual ~LuaExtension();

	bool Initialise(SciTEBase *host_);
	void PostInit(void* h);
	void* GetLuaState();
	bool Finalise();
	bool Clear();
	bool Load(const char *filename);

	bool InitBuffer(int);
	bool ActivateBuffer(int);
	bool RemoveBuffer(int);

	bool OnOpen(const char *filename);
	bool OnSwitchFile(const char *filename);
	bool OnBeforeSave(const char *filename);
	bool OnBeforeOpen(const char *filename, const char *extension, int& encoding);
	bool OnSave(const char *filename);
	bool OnChar(char ch);
	bool OnExecute(const char *s);
	bool OnSavePointReached();
	bool OnSavePointLeft();
	bool OnStyle(Sci_Position startPos, Sci_Position lengthDoc, int initStyle, StyleWriter *styler);
	bool OnDoubleClick(int modifiers); 
	bool OnClick(int modifiers); 
	bool OnMouseButtonUp(int modifiers); 
	bool OnHotSpotReleaseClick(int modifiers); 
	bool PaneOnUpdateUI(bool bModified, bool bSelChange, int flag); 
	bool CoOnUpdateUI(bool bModified, bool bSelChange, int flag);
	bool OnUpdateUI(bool bModified, bool bSelChange, int flag);
	bool OnMarginClick(unsigned int margin, unsigned int modif, long line, uptr_t id);
	bool OnCallTipClick(Sci_Position pos);
	bool OnUserListSelection(int listType, const char *selection, Sci_Position id, int method);
	bool OnNavigation(const char *item);
	bool OnKey(int keyval, int modifiers, char ch); 
	bool OnDwellStart(Sci_Position pos, const char *wordd, bool ctrl);
	bool OnClose(const char *filename);
	bool OnMacro(const char *func, uptr_t w, sptr_t l, const char *s);
	bool OnMenuCommand(int, int); 
	const char *OnSendEditor(unsigned int, uptr_t, const char *);
	const char *OnSendEditor(unsigned int, uptr_t, sptr_t);
	bool OnLindaNotify(const char*);
	bool OnColorized(Sci_Position, Sci_Position);
	const char *OnContextMenu(unsigned int, unsigned int, const char *);
	bool OnFindProgress(uptr_t state, sptr_t all);
	bool OnPostCallback(int idx);
	bool OnIdle();
	bool OnLayOutNotify(const char *);
	bool OnGeneratedHotKey(long);  //Только для команд, посылаемых по хоткеям, сгенерированным автоматически!
	void DoReboot();
	void DoLua(const char *c);
	void OnMouseHook(int x, int y);
	bool OnMacroBlocked(int msg, intptr_t wParam, intptr_t lParam);
	sptr_t OnMenuChar(int flag, const char* key);
	bool OnDrawClipboard(int flag);
	void OnRightEditorVisibility(bool show);
	void OnTextChanged(Sci_Position position, int leg, Sci_Position linesAdded, int flag);
	void BeforeTextChanged(Sci_Position position, int leg, const char* t, int type);
	void OnCurrentLineFold(Sci_Position line, int leg, int foldLevelPrev, int foldLevelNow);
	bool OnAutocSelection(int method, Sci_Position firstPos);
	void OnCommandLine(const char* line);
	void OnChangeFocus(int, int);
	int HildiAlarm(const char* msg, int flag, const GUI::gui_char *p1 = nullptr, const GUI::gui_char *p2 = nullptr, const GUI::gui_char *p3 = nullptr);
	bool SendProperty(const char *) { return false; }
	GUI::gui_string LocalizeText(const char* msg);
	void OnShowActivate(int bActivate, int cmdShow, int SavePoint = 0);
	void raiseLuaError(const char* msg);
};

#endif
