// SciTE - Scintilla based Text Editor
// LuaExtension.h - Lua scripting extension
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

class LuaExtension : public Extension {
private:
	LuaExtension(); // Singleton
	LuaExtension(const LuaExtension &);   // Disable copy ctor
	void operator=(const LuaExtension &); // Disable operator=

public:
	static LuaExtension &Instance();

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
	virtual bool OnUpdateUI();
	virtual bool OnMarginClick();
//!	virtual bool OnUserListSelection(int listType, const char *selection);
	virtual bool OnUserListSelection(int listType, const char *selection, int id); //!-change-[UserListItemID]
	virtual bool OnNavigation(const char *item);
//!	virtual bool OnKey(int keyval, int modifiers);
	virtual bool OnKey(int keyval, int modifiers, char ch); //!-change-[OnKey]
	virtual bool OnDwellStart(int pos, const char *word);
	virtual bool OnClose(const char *filename);
	virtual bool OnMacro(const char *p, const char *q); //!-add-[macro]
	virtual bool OnMenuCommand(int, int); //!-add-[OnMenuCommand]
	virtual const char *OnSendEditor(unsigned int, unsigned int, const char *); //!-add-[OnSendEditor]
	virtual const char *OnSendEditor(unsigned int, unsigned int, long); //!-add-[OnSendEditor]
	virtual bool OnColorized(unsigned int, unsigned int);
	virtual const char *OnContextMenu(unsigned int, unsigned int, const char *);
	virtual bool OnFindCompleted();
	virtual bool OnIdle();
	virtual bool OnLayOutNotify(const char *);
	virtual bool OnGeneratedHotKey(long);  //������ ��� ������, ���������� �� �������, ��������������� �������������!
	virtual void DoReboot();
	virtual void DoLua(const char *c);
	virtual void OnMouseHook(int x, int y);

};
