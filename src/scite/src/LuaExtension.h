// SciTE - Scintilla based Text Editor
// LuaExtension.h - Lua scripting extension
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

class LuaExtension : public Extension {

public:

	LuaExtension();
	virtual ~LuaExtension();

	virtual bool Initialise(ExtensionAPI *host_);
	virtual void PostInit(void* h);
	virtual void* GetLuaState();
	virtual bool Finalise();
	virtual bool Clear();
	virtual bool Load(const char *filename);

	virtual bool InitBuffer(int);
	virtual bool ActivateBuffer(int);
	virtual bool RemoveBuffer(int);

	virtual bool OnOpen(const char *filename);
	virtual bool OnSwitchFile(const char *filename);
	virtual bool OnBeforeSave(const char *filename);
	virtual bool OnBeforeOpen(const char *filename, const char *extension, int& encoding);
	virtual bool OnSave(const char *filename);
	virtual bool OnChar(char ch);
	virtual bool OnExecute(const char *s);
	virtual bool OnSavePointReached();
	virtual bool OnSavePointLeft();
	virtual bool OnStyle(Sci_Position startPos, Sci_Position lengthDoc, int initStyle, StyleWriter *styler);
	virtual bool OnDoubleClick(int modifiers); 
	virtual bool OnClick(int modifiers); 
	virtual bool OnMouseButtonUp(int modifiers); 
	virtual bool OnHotSpotReleaseClick(int modifiers); 
	virtual bool PaneOnUpdateUI(bool bModified, bool bSelChange, int flag); 
	virtual bool CoOnUpdateUI(bool bModified, bool bSelChange, int flag);
	virtual bool OnUpdateUI(bool bModified, bool bSelChange, int flag);
	virtual bool OnMarginClick(unsigned int margin, unsigned int modif, long line, uptr_t id);
	virtual bool OnCallTipClick(Sci_Position pos);
	virtual bool OnUserListSelection(int listType, const char *selection, Sci_Position id, int method);
	virtual bool OnNavigation(const char *item);
	virtual bool OnKey(int keyval, int modifiers, char ch); 
	virtual bool OnDwellStart(Sci_Position pos, const char *wordd, bool ctrl);
	virtual bool OnClose(const char *filename);
	virtual bool OnMacro(const char *func, uptr_t w, sptr_t l, const char *s);
	virtual bool OnMenuCommand(int, int); 
	virtual const char *OnSendEditor(unsigned int, uptr_t, const char *);
	virtual const char *OnSendEditor(unsigned int, uptr_t, sptr_t);
	virtual bool OnLindaNotify(const char*, const char* );
	virtual bool OnColorized(Sci_Position, Sci_Position);
	virtual const char *OnContextMenu(unsigned int, unsigned int, const char *);
	virtual bool OnFindProgress(uptr_t state, sptr_t all);
	virtual bool OnPostCallback(int idx);
	virtual bool OnIdle();
	virtual bool OnLayOutNotify(const char *);
	virtual bool OnGeneratedHotKey(long);  //“олько дл€ команд, посылаемых по хотке€м, сгенерированным автоматически!
	virtual void DoReboot();
	virtual void DoLua(const char *c);
	virtual void OnMouseHook(int x, int y);
	virtual bool OnMacroBlocked(int msg, intptr_t wParam, intptr_t lParam);
	virtual sptr_t OnMenuChar(int flag, const char* key);
	virtual bool OnDrawClipboard(int flag);
	virtual void OnRightEditorVisibility(bool show);
	virtual void OnTextChanged(Sci_Position position, int leg, Sci_Position linesAdded, int flag);
	virtual void BeforeTextChanged(Sci_Position position, int leg, const char* t, int type);
	virtual void OnCurrentLineFold(Sci_Position line, int leg, int foldLevelPrev, int foldLevelNow);
	virtual bool OnAutocSelection(int method, Sci_Position firstPos);
	virtual void OnCommandLine(const char* line);
	virtual void OnChangeFocus(int, int);
	virtual int HildiAlarm(const char* msg, int flag, const GUI::gui_char *p1, const GUI::gui_char *p2, const GUI::gui_char *p3);
	virtual GUI::gui_string LocalizeText(const char* msg);
	virtual void OnSize();
};
