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
	virtual bool OnStyle(unsigned int startPos, int lengthDoc, int initStyle, StyleWriter *styler);
//!	virtual bool OnDoubleClick();
	virtual bool OnDoubleClick(int modifiers); //!-add-[OnDoubleClick]
	virtual bool OnClick(int modifiers); //!-add-[OnClick]
	virtual bool OnMouseButtonUp(int modifiers); //!-add-[OnMouseButtonUp]
	virtual bool OnHotSpotReleaseClick(int modifiers); //!-add-[OnHotSpotReleaseClick]
	virtual bool PaneOnUpdateUI(bool bModified, bool bSelChange, int flag); 
	virtual bool CoOnUpdateUI(bool bModified, bool bSelChange, int flag);
	virtual bool OnUpdateUI(bool bModified, bool bSelChange, int flag);
	virtual bool OnMarginClick(unsigned int margin, unsigned int modif, long line, uptr_t id);
	virtual bool OnCallTipClick(int pos);
//!	virtual bool OnUserListSelection(int listType, const char *selection);
	virtual bool OnUserListSelection(int listType, const char *selection, int id, int method); //!-change-[UserListItemID]
	virtual bool OnNavigation(const char *item);
//!	virtual bool OnKey(int keyval, int modifiers);
	virtual bool OnKey(int keyval, int modifiers, char ch); //!-change-[OnKey]
	virtual bool OnDwellStart(int pos, const char *wordd, bool ctrl);
	virtual bool OnClose(const char *filename);
	virtual bool OnMacro(const char *func, unsigned int w, unsigned int l, const char *s); //!-add-[macro]
	virtual bool OnMenuCommand(int, int); //!-add-[OnMenuCommand]
	virtual const char *OnSendEditor(unsigned int, unsigned int, const char *); //!-add-[OnSendEditor]
	virtual const char *OnSendEditor(unsigned int, unsigned int, long); //!-add-[OnSendEditor]
	virtual bool OnLindaNotify(const char*, const char* );
	virtual bool OnColorized(unsigned int, unsigned int);
	virtual const char *OnContextMenu(unsigned int, unsigned int, const char *);
	virtual bool OnFindProgress(int state, int all);
	virtual bool OnPostCallback(int idx);
	virtual bool OnIdle();
	virtual bool OnLayOutNotify(const char *);
	virtual bool OnGeneratedHotKey(long);  //“олько дл€ команд, посылаемых по хотке€м, сгенерированным автоматически!
	virtual void DoReboot();
	virtual void DoLua(const char *c);
	virtual void OnMouseHook(int x, int y);
	virtual bool OnMacroBlocked(int msg, int wParam, int lParam);
	virtual int OnMenuChar(int flag, const char* key);
	virtual bool OnDrawClipboard(int flag);
	virtual void OnRightEditorVisibility(bool show);
	virtual void OnTextChanged(int position, int leg, const char* text, int linesAdded, int flag);
	virtual bool OnAutocSelection(int method, int firstPos);
	virtual void OnCommandLine(const char* line);
	virtual void OnChangeFocus(int, int);
	virtual int HildiAlarm(const char* msg, int flag, const GUI::gui_char *p1, const GUI::gui_char *p2, const GUI::gui_char *p3);
	virtual GUI::gui_string LocalizeText(const char* msg);
	virtual void OnSize();
};
