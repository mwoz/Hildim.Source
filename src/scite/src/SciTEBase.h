// SciTE - Scintilla based Text Editor
/** @file SciTEBase.h
 ** Definition of platform independent base class of editor.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef __SCITEBASE_H
#define	 __SCITEBASE_H
extern const GUI::gui_char appName[];

extern const GUI::gui_char propUserFileName[];
extern const GUI::gui_char propGlobalFileName[];
extern const GUI::gui_char propAbbrevFileName[];

#ifdef WIN32
#ifdef _MSC_VER
// Shut up level 4 warning:
// warning C4710: function 'void whatever(...)' not inlined
// warning C4800: forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4710 4800)
#endif
#ifdef __DMC__
#include <time.h>
#endif
#endif
#include <regex>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "iup.h"
#include "iuplua.h"
#include "iupcontrols.h"
#include "iupluacontrols.h"
#include "scite_flattabs.h"
}

#define ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

inline int Minimum(int a, int b) {
	return (a < b) ? a : b;
}

inline int Maximum(int a, int b) {
	return (a > b) ? a : b;
}

inline long LongFromTwoShorts(short a,short b) {
	return (a) | ((b) << 16);
}

typedef long Colour;
inline Colour ColourRGB(unsigned int red, unsigned int green, unsigned int blue) {
	return red | (green << 8) | (blue << 16);
}

/**
 * The order of menus on Windows - the Buffers menu may not be present
 * and there is a Help menu at the end.
 */
enum {
    menuFile = 0, menuEdit = 1, menuSearch = 2, menuView = 3,
    menuTools = 4, menuOptions = 5, menuLanguage = 6, menuBuffers = 7,
    menuHelp = 8
};


class RecentFile : public FilePath {
public:
	Sci_CharacterRange selection;
	int scrollPosition;
	RecentFile() {
		selection.cpMin = INVALID_POSITION;
		selection.cpMax = INVALID_POSITION;
		scrollPosition = 0;
	}
	void Init() {
		FilePath::Init();
		selection.cpMin = INVALID_POSITION;
		selection.cpMax = INVALID_POSITION;
		scrollPosition = 0;
	}
};

// Related to Utf8_16::encodingType but with additional values at end
enum UniMode {
    uni8Bit = 0, uni16BE = 1, uni16LE = 2, uniUTF8 = 3,
    uniCookie = 4
};

class BufferListAPI {
public:
	virtual int GetDocumentByName(FilePath filename, bool excludeCurrent = false, int forIdm = NULL) = 0;
	virtual void* GetAt(int index) = 0;
};

class Buffer : public RecentFile {
public:
	sptr_t doc;
	bool isDirty;
	bool ROMarker; 
	bool useMonoFont;
	UniMode unicodeMode;
	time_t fileModTime;
	time_t fileModLastAsk;
	enum { fmNone, fmMarked, fmModified} findMarks;
	SString overrideExtension;	///< User has chosen to use a particular language
	std::vector<int> foldState;
	int editorSide;
	bool pFriend;
	BufferListAPI* pBase;
	int FriendIndex() {
		return  pBase->GetDocumentByName(this->AbsolutePath(), false, editorSide == IDM_SRCWIN ? IDM_COSRCWIN : IDM_SRCWIN);
	}

	Buffer* Friend() {
		int i = pBase->GetDocumentByName(this->AbsolutePath(), false, editorSide == IDM_SRCWIN ? IDM_COSRCWIN : IDM_SRCWIN );
		if (i == -1) 
			return NULL;
		return (Buffer*)pBase->GetAt(i);
	}
	Buffer() :
//!			RecentFile(), doc(0), isDirty(false), useMonoFont(false),
			RecentFile(), doc(0), isDirty(false), ROMarker(false), useMonoFont(false),  //!-change-[ReadOnlyTabMarker]
			unicodeMode(uni8Bit), fileModTime(0), fileModLastAsk(0), findMarks(fmNone), editorSide(IDM_SRCWIN), foldState(), pFriend(false){}

	void Init(BufferListAPI* pB) {
		RecentFile::Init();
		isDirty = false;
		ROMarker = false; //!-add-[ReadOnlyTabMarker]
		useMonoFont = false;
		unicodeMode = uni8Bit;
		fileModTime = 0;
		fileModLastAsk = 0;
		findMarks = fmNone;
		overrideExtension = "";
		foldState.clear();
		pFriend = false;
		pBase = pB;
	}

	void SetTimeFromFile() {
		fileModTime = ModifiedTime();
		fileModLastAsk = fileModTime;
		if (pFriend){
			Buffer* b = Friend();
			if (b) b->fileModTime = fileModTime;
		}
	}
//!-start-[OpenNonExistent]
	bool DocumentNotSaved()  {
		bool rez = (isDirty || (!IsUntitled() && (fileModTime == 0)));
		if (rez || !pFriend)
			return rez;
		Buffer* b = Friend();
		return b && (b->isDirty || (!b->IsUntitled() && (b->fileModTime == 0)));
	}
//!-end-[OpenNonExistent]
};

class EditSwitcher{
public:
	virtual void SwitchTo(int wndIdm, FilePath* pBuf) = 0;
	virtual int GetWindowIdm() = 0;
	virtual void SetBuffPointer(FilePath* pBuf) = 0;
	virtual void SetBuffEncoding(int e) = 0;
};

class BufferList: public BufferListAPI {
protected:
	int current;
	int *stack; 
public:
	int stackcurrent;
	Buffer *buffers;
	EditSwitcher * pEditor;
	int size;
	int length;
	bool initialised;
	BufferList();
	~BufferList();
	void Allocate(int maxSize);
	int Add(sptr_t doc = NULL);
	int GetDocumentByName(FilePath filename, bool excludeCurrent=false, int forIdm = NULL);
	void RemoveCurrent();
	int NextByIdm(int idm);
	int Current() const;
	Buffer *CurrentBuffer();
	void SetCurrent(int index);
	int StackNext();
	int StackNextBySide(int side, int curr);
	int StackPrev();
	void CommitStackSelection();
	void MoveToStackTop(int index);
	void ShiftTo(int indexFrom, int indexTo);
	int GetOrder(int index);
	virtual void* GetAt(int index) {
		return (void*) &buffers[index];
	}
	void OrderBy(std::map<int, int> &order);
private:
	void PopStack();
};

enum {
    heightTools = 24,
    heightTab = 24,
    heightStatus = 20,
    statusPosWidth = 256
};

/// Warning IDs.
enum {
    warnFindWrapped = 1,
    warnNotFound,
    warnNoOtherBookmark,
    warnWrongFile,
    warnExecuteOK,
    warnExecuteKO
};

/// Codes representing the effect a line has on indentation.
enum IndentationStatus {
    isNone,		// no effect on indentation
    isBlockStart,	// indentation block begin such as "{" or VB "function"
    isBlockEnd,	// indentation end indicator such as "}" or VB "end"
    isKeyWordStart	// Keywords that cause indentation
};

int IntFromHexDigit(int ch);
int IntFromHexByte(const char *hexByte);

class StyleDefinition {
public:
	SString font;
	int size;
	SString fore;
	SString back;
	bool bold;
	bool italics;
	bool eolfilled;
	bool underlined;
	int caseForce;
	bool visible;
	bool changeable;
	bool hotspot; //!-add-[StyleDefHotspot]
	enum flags { sdNone = 0, sdFont = 0x1, sdSize = 0x2, sdFore = 0x4, sdBack = 0x8,
	        sdBold = 0x10, sdItalics = 0x20, sdEOLFilled = 0x40, sdUnderlined = 0x80,
//!	        sdCaseForce = 0x100, sdVisible = 0x200, sdChangeable = 0x400} specified;
	        sdCaseForce = 0x100, sdVisible = 0x200, sdChangeable = 0x400, sdHotspot = 0x800} specified; //!-change-[StyleDefHotspot]
	StyleDefinition(const char *definition);
	bool ParseStyleDefinition(const char *definition);
	long ForeAsLong() const;
	long BackAsLong() const;
};

struct StyleAndWords {
	int styleNumber;
	SString words;
	bool IsEmpty() { return words.length() == 0; }
	bool IsSingleChar() { return words.length() == 1; }
};

// Interface between SciTE and dialogs and strips for find and replace
class Searcher {
public:
	SString findWhat;
	SString replaceWhat;

	bool wholeWord;
	bool matchCase;
	bool regExp;
	bool unSlash;
	bool wrapFind;
	bool reverseFind;
	bool subDirSearch;

	bool replacing;
	bool havefound;
	bool findInStyle;
	int findStyle;

	bool focusOnReplace;

	Searcher();

	virtual bool FindHasText() const = 0;
	virtual void MoveBack(int distance) = 0;
	virtual void ScrollEditorIfNeeded() = 0;

	virtual void UIClosed() = 0;
	virtual void UIHasFocus() = 0;
	bool &FlagFromCmd(int cmd);
};

class SearchUI {
protected:
	Searcher *pSearcher;
public:
	SearchUI() : pSearcher(0) {
	}
	void SetSearcher(Searcher *pSearcher_) {
		pSearcher = pSearcher_;
	}
};
typedef struct sb_colors { long left; long right; } sb_colors;
struct sb_colorsetting {
	int size;
	int id[10];
	long clr[10];
	DWORD mask;
	long annotation;
};
class IupChildWnd
{
public:
	IupChildWnd();
	~IupChildWnd();
	void Attach(HWND h, void *pScite, const char *pName, HWND hM, GUI::ScintillaWindow *pW, Ihandle *pCnt);
	void Scroll_CB(int op, float posx, float posy);
	void FlatScroll_CB();
	void VScrollDraw_CB(Ihandle*ih, void* c, int sb_size, int ymax, int pos, int d, int active, char* fgcolor_drag, char * bgcolor);
	void ColorSettings_CB(Ihandle* ih, int side, int markerid, const char* value);
	void HideScrolls();
	void OnIdle();
	void resetPixelMap();
	void setCurLine(int l);
private:
	int curLine = -1;
	bool lineChanged = false;
	float lineheightPx;
	char name[16];
	bool bNeedSize = false;
	HWND hMainWnd;
	void *pSciteWin;
	WNDPROC subclassedProc;
	LRESULT PASCAL WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL StatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	GUI::ScintillaWindow *pS;
	Ihandle *pContainer = 0;
	void SizeEditor();
	bool blockV = false;
	bool blockH = false;
	int hPx = 0; //высота горизонтального бара
	int vPx = 0;  //ширина вертикального бара
	UINT vHeight = 0; //текущая высота вертикального бара
	bool colodizedSB = false;
	int resetmap = false;
	std::vector<sb_colors> pixelMap;

	sb_colorsetting leftClr = { 0,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0, 0 };
	sb_colorsetting rightClr = { 0,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0, 0 };
	DWORD markerMaskAll = 0;
	bool bBlockFlatCollback = false;

};
typedef std::map<const char*, IupChildWnd*> mapsICW;
class IupLayoutWnd
{
public:
	IupLayoutWnd();
	~IupLayoutWnd();
	void CreateLayout(lua_State *L, void *pS);
	HWND GetChildHWND(const char* name);
	void SubclassChild(const char* name, GUI::ScintillaWindow *pW);
	void GetPaneRect(const char *name, LPRECT pRc);
	void SetPaneHeight(const char *name, int Height);
	void AdjustTabBar();
	Ihandle* hMain;
	void Fit();
	void Close();
	void OnIdle();
	void OnSwitchFile(int editorSide);
	void OnOpenClose(int editorSide);
	COLORREF GetColorRef(char* name);
	Ihandle *pLeftTab;
	Ihandle *pRightTab;
	LRESULT OnNcCalcSize(HWND hwnd, BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	LRESULT OnNcPaint(HWND hwnd, BOOL bActiv, BOOL bOnlyBtns = false);
	LRESULT OnNcHitTest(HWND hwnd, POINT cursor);
	LRESULT OnNcMouseMove(HWND hwnd, int iBtn);
	LRESULT OnNcLMouseDown(HWND hwnd, int iBtn);
	LRESULT OnNcLMouseUp(HWND hwnd, int iBtn);
	bool FullScreen = false;
	bool StandartWindowDecoration = false;
	void SetTitle(const GUI::gui_char *s);
	mapsICW childMap;
private:
	void * pSciteWin;
	IupChildWnd ichFindRes;
	Ihandle* Create_dialog();
	
	WNDPROC subclassedProc;
	LRESULT PASCAL WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL StatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void PropGet(const char *name, const char *defoult, char* buff);
	bool HilightBtn(RECT *rect, POINT *p, HDC hdc, int iBtn, COLORREF clr, int rx, int ry);
	int captWidth = 32;
	int nBtn = 0;
	int nBtnPressed = 0;
	int borderX = 0;
	int borderY = 0;
	GUI::gui_string title = L"";
	HICON hicon = NULL;
};

class SciTEBase : public ExtensionAPI, public Searcher {
protected:
	GUI::gui_string windowName = L"HildiM";
	FilePath filePath;
	FilePath dirNameAtExecute;
	FilePath dirNameForExecute;

//!	enum { fileStackMax = 10 };
	enum { fileStackMax = 30 }; //!-change-[MoreRecentFiles]
	enum { fileStackMaxDefault = 10 }; //!-add-[MoreRecentFiles]
	RecentFile recentFileStack[fileStackMax];
	enum { fileStackCmdID = IDM_MRUFILE, bufferCmdID = IDM_BUFFER };

	enum { importMax = 50 };
	FilePath importFiles[importMax];
	enum { importCmdID = IDM_IMPORT };

	enum { indicatorMatch = INDIC_CONTAINER };
	enum {markerSaved  = 0, markerNotSaved, markerBookmark, markerError};
	SString parameterisedCommand;
	char abbrevInsert[200];

	enum { languageCmdID = IDM_LANGUAGE };

	int codePage;
	int characterSet;
	SString language;
	int lexLanguage;
	int lexLPeg;
	StringList apis;
	SString apisFileNames;
	SString functionDefinition;

	bool indentOpening;
	bool indentClosing;
	bool indentMaintain;
	int statementLookback;
	StyleAndWords statementIndent;
	StyleAndWords statementEnd;
	StyleAndWords blockStart;
	StyleAndWords blockEnd;
	enum { noPPC, ppcStart, ppcMiddle, ppcEnd, ppcDummy };	///< Indicate the kind of preprocessor condition line
	char preprocessorSymbol;	///< Preprocessor symbol (in C: #)
	StringList preprocCondStart;	///< List of preprocessor conditional start keywords (in C: if ifdef ifndef)
	StringList preprocCondMiddle;	///< List of preprocessor conditional middle keywords (in C: else elif)
	StringList preprocCondEnd;	///< List of preprocessor conditional end keywords (in C: endif)

	GUI::Window wSciTE;  

//!	GUI::ScintillaWindow wEditor;
//!-start-[OnSendEditor]
	class ScintillaWindowEditor : public GUI::ScintillaWindow
	{
	public:
		virtual sptr_t Call(unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
		SciTEBase* pBase;
	};
	class ScintillaWindowSwitcher : public ScintillaWindowEditor, public EditSwitcher {
	public:	
		virtual void SwitchTo(int wndIdm, FilePath* pBuf) ;
		virtual int GetWindowIdm();
		virtual void SetBuffPointer(FilePath* pBuf);
		virtual void SetCoBuffPointer(FilePath* pBuf);
		virtual void SetBuffEncoding(int e);
		FilePath GetCoBuffPointer();
		void Switch(bool ignorebuff = false);
		ScintillaWindowEditor coEditor;
	private:
		FilePath buffer_L;
		FilePath buffer_R= NULL;
	};
	
	ScintillaWindowSwitcher wEditor;
	ScintillaWindowEditor wEditorR;
	ScintillaWindowEditor wEditorL;
	virtual sptr_t CallAll(unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	sptr_t CallStringAll(unsigned int msg, uptr_t wParam, const char *s);
	friend class ScintillaWindowEditor;
//!-end-[OnSendEditor]
	GUI::ScintillaWindow wOutput;
	GUI::ScintillaWindow wFindRes;
	GUI::Window wIncrement;
	bool viewWs;
	bool viewIndent;
	bool iuptbVisible;
	SString sbValue;	///< Status bar text.
	int sbNum;	///< Number of the currenly displayed status bar information.
	int visHeightTools;
	int visHeightTab;
	int visHeightEditor;
	int visHeightIuptool;
	int heightBar;
	// Prevent automatic load dialog appearing at the same time as
	// other dialogs as this can leads to reentry errors.
	int dialogsOnScreen;
	bool topMost;
	bool wrap;
	bool wrapOutput;
	bool wrapFindRes;
	int wrapStyle;
	bool isReadOnly;
	bool openFilesHere;
//!	enum { toolMax = 50 };
	enum { toolMax = 300 }; //!-change-[ToolsMax]
	Extension *extender;
	bool needReadProperties;
	bool preserveFocusOnEditor; //!-add-[GoMessageImprovement]

	GUI::Point ptStartDrag;
	bool capturedMouse;
	bool firstPropertiesRead;
	bool bufferedDraw;
	bool twoPhaseDraw;
	bool bracesCheck;
	bool bracesSloppy;
	int bracesStyle;
	int braceCount;

	bool indentationWSVisible;
	int indentExamine;

	bool autoCompleteIgnoreCase;
	bool autoCompleteIncremental;
	bool callTipAutomatic; //!-add-[BetterCalltips]
	bool callTipIgnoreCase;
	int calltipShowPerPage; //!-add-[BetterCalltips]
	bool autoCCausedByOnlyOne;
	SString calltipWordCharacters;
	SString calltipParametersStart;
	SString calltipParametersEnd;
	SString calltipParametersSeparators;
	SString calltipEndDefinition;
	SString autoCompleteStartCharacters;
	SString autoCompleteFillUpCharacters;
	SString wordCharacters;
	SString whitespaceCharacters;

	bool macro1stLoaded = false;

	bool margin;
	int marginWidth;
	enum { marginWidthDefault = 20};

	bool foldMargin;
	int foldMarginWidth;
	enum { foldMarginWidthDefault = 14};

	bool lineNumbers;
	int lineNumbersWidth;
	enum { lineNumbersWidthDefault = 4 };
	bool lineNumbersExpand;

	bool usePalette;
	bool allowMenuActions;
	int scrollOutput;
	bool returnOutputToCommand;
	JobQueue jobQueue;

	bool macrosEnabled;
	SString currentMacro;
	bool recording;

	PropSetFile propsEmbed;
	PropSetFile propsBase;
	PropSetFile propsUser;
	PropSetFile propsDirectory;
	PropSetFile propsLocal;
	PropSetFile props;

	PropSetFile propsSession;

	FilePath pathAbbreviations;

	IupLayoutWnd layout;

	PropSetFile propsStatus;	// Not attached to a file but need SetInteger method.

	enum { bufferMax = 100 };
	BufferList buffers;

	// Handle buffers
	sptr_t GetDocumentAt(int index);
	
	void UpdateBuffersCurrent();
	void UpdateBuffersCoCurrent();
	bool IsBufferAvailable();
	bool CanMakeRoom(bool maySaveIfDirty = true);
	void SetDocumentAt(int index, bool updateStack = true, bool switchTab = true, bool bExit = false);
	int ShiftToVisible(int index);
	void GetBufferName(int i, char *c) { lstrcpynA(c, buffers.buffers[i].AsUTF8().c_str(), 2000); };
	void GetCoBufferName(char *c){lstrcpynA( c, wEditor.GetCoBuffPointer().AsUTF8().c_str(), 2000);};
	int GetBufferEncoding(int i) { return buffers.buffers[i]._encoding; };
	int GetBufferFileTime(int i) { return (int)buffers.buffers[i].fileModTime;};
	void ClearBufferFileTime(int i) { buffers.buffers[i].SetTimeFromFile();	SetWindowName();BuffersMenu();	};
	void SetBufferEncoding(int i, int e);
	bool GetBuffersSavedState(int i){ return ! buffers.buffers[i].DocumentNotSaved(); };
	int GetBuffersCount(){return buffers.length; };		
	int GetCurrentBufer(){ return buffers.Current(); };	
	virtual int GetBufferSide(int index) { return buffers.buffers[index].editorSide == IDM_SRCWIN ? 0 : 1;  };
	virtual int GetBufferOrder(int index) { return buffers.GetOrder(index); };
	virtual int GetBufferModTime(int index) { return buffers.buffers[index].fileModTime; }
	virtual int GetBufferUnicMode(int index) { return buffers.buffers[index].unicodeMode + IDM_ENCODING_DEFAULT; };
	virtual int SecondEditorActive();
	virtual void Open_script(const char* path);
	virtual void SavePositions();
	virtual void BlockUpdate(int cmd);
	virtual void Close_script();
		
	virtual int Cloned(int index) {
		return buffers.buffers[index].pFriend;
	}
	virtual int IndexOfClone(int index) {
		return buffers.buffers[index].FriendIndex();
	}
	virtual int BufferByName(const char* c) {
		return buffers.GetDocumentByName(FilePath(GUI::StringFromUTF8(c)));
	}
	Buffer *CurrentBuffer() {
		return buffers.CurrentBuffer();
	}
	void BuffersMenu(bool mousedrag = false);
	const char* GetPropClr(const char* propName, char* buff, const char* def);
	void Next();
	void Prev();
	void NextInStack();
	void PrevInStack();
	void EndStackedTabbing();

	void ShiftTab(int indexFrom, int indexTo, bool mose = false);
	void MoveTabRight();
	void MoveTabLeft();
	void CloneTab();
	void ChangeTabWnd();
	virtual void OrderTabsBy(std::map<int, int> &order);
	virtual void PostLoadScript() = 0;
	void CheckRightEditorVisible();
	bool m_bRightEditorVisible = false;

	void ReadGlobalPropFile(GUI::gui_string adv);

	sptr_t CallFocused(unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	sptr_t CallPane(int destination, unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	void CallChildren(unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);

	int LengthDocument();
	int GetCaretInLine();
	void GetLine(char *text, int sizeText, int line = -1);
	SString GetLine(int line = -1);
	void GetRange(GUI::ScintillaWindow &win, int start, int end, char *text);
	int IsLinePreprocessorCondition(char *line);
	bool FindMatchingPreprocessorCondition(int &curLine, int direction, int condEnd1, int condEnd2);
	bool FindMatchingPreprocCondPosition(bool isForward, int &mppcAtCaret, int &mppcMatch);
	bool FindMatchingBracePosition(bool editor, int &braceAtCaret, int &braceOpposite, bool sloppy);
	void BraceMatch(bool editor);

//!	virtual void WarnUser(int warnID) = 0;
	virtual void WarnUser(int warnID, const char *msg = NULL, bool isCanBeAlerted = true) = 0; //!-change-[WarningMessage]
	void SetWindowName();
	void SetFileName(FilePath openName, bool fixCase = true, bool setCaption = true);
	FilePath FileNameExt() const {
		return filePath.Name();
	}
	void ClearDocument();
	void CreateBuffers();
	void InitialiseBuffers();
	FilePath UserFilePath(const GUI::gui_char *name);
	virtual void GetWindowPosition(int *left, int *top, int *width, int *height, int *maximize) = 0;
	void SetIndentSettings();
	void SetEol();
	void New();
	void RestoreState(const Buffer &buffer, bool setCaption = true, bool scipCollapse= false);
	void Close(bool updateUI = true, bool loadingSession = false, bool makingRoomForNew = false);
	bool bBlockUIUpdate = false;
	bool bBlockRedraw = false;
	bool bBlockTextChangeNotify = false;
	bool Exists(const GUI::gui_char *dir, const GUI::gui_char *path, FilePath *resultPath);
	void DiscoverEOLSetting();
	void DiscoverIndentSetting();
	SString DiscoverLanguage(const char *buf, size_t length);
	void OpenFile(int fileSize, bool suppressMessage);
	virtual bool ReadForScript(FilePath &fileCompare, void** convert, char** data, size_t &lenFile);
	virtual void OpenUriList(const char *) {}
	virtual bool OpenDialog(FilePath directory, const GUI::gui_char *filter) = 0;
	virtual bool SaveAsDialog() = 0;

	void CountLineEnds(int &linesCR, int &linesLF, int &linesCRLF);
	enum OpenFlags {
	    ofNone = 0, 		// Default
	    ofNoSaveIfDirty = 1, 	// Suppress check for unsaved changes
	    ofForceLoad = 2,	// Reload file even if already in a buffer
	    ofPreserveUndo = 4,	// Do not delete undo history
	    ofQuiet = 8		// Avoid "Could not open file" message
	};
	virtual bool PreOpenCheck(const GUI::gui_char *file);
	bool Open(FilePath file, OpenFlags of = ofNone);
	void Revert();
	FilePath SaveName(const char *ext);
	int SaveIfUnsure(bool forceQuestion = false);
	int SaveIfUnsureForBuilt();
	bool SaveIfNotOpen(const FilePath &destFile, bool fixCase);
	bool Save(bool bNotSaveNotChanged = false);
	void SaveAs(const GUI::gui_char *file, bool fixCase);
	virtual void SaveACopy() = 0;
	void SaveToHTML(FilePath saveName);
	void StripTrailingSpaces();
	void EnsureFinalNewLine();
	bool SaveBuffer(FilePath saveName, bool bNotSaveNotChanged = false);
	virtual void SaveAsHTML() = 0;
	void SaveToRTF(FilePath saveName, int start = 0, int end = -1);
	virtual void SaveAsRTF() = 0;
	void SaveToPDF(FilePath saveName);
	virtual void SaveAsPDF() = 0;
	void SaveToTEX(FilePath saveName);
	virtual void SaveAsTEX() = 0;
	void SaveToXML(FilePath saveName);
	virtual void SaveAsXML() = 0;
	virtual FilePath GetDefaultDirectory() = 0;
	virtual FilePath GetSciteDefaultHome() = 0;
	virtual FilePath GetSciteUserHome() = 0;
	FilePath GetDefaultPropertiesFileName();
	FilePath GetUserPropertiesFileName();
	FilePath GetDirectoryPropertiesFileName();
	FilePath GetLocalPropertiesFileName();
	FilePath GetAbbrevPropertiesFileName();
	void OpenProperties(int propsFile);
	int GetMenuCommandAsInt(SString commandName);
	virtual void Print(bool) {}
	virtual void PrintSetup() {}
	Sci_CharacterRange GetSelection();
	void SetSelection(int anchor, int currentPos);
	//	void SelectionExtend(char *sel, int len, char *notselchar);
	void GetCTag(char *sel, int len);
	SString GetRange(GUI::ScintillaWindow &win, int selStart, int selEnd);
	virtual SString GetRangeInUIEncoding(GUI::ScintillaWindow &win, int selStart, int selEnd);
	SString GetLine(GUI::ScintillaWindow &win, int line);
	SString RangeExtendAndGrab(GUI::ScintillaWindow &wCurrent, int &selStart, int &selEnd,
	        bool (SciTEBase::*ischarforsel)(char ch), bool stripEol = true);
	SString SelectionExtend(bool (SciTEBase::*ischarforsel)(char ch), bool stripEol = true);
	void FindWordAtCaret(int &start, int &end);
	bool SelectWordAtCaret();
	SString SelectionWord(bool stripEol = true);
	SString SelectionFilename();
	void SelectionIntoProperties();
	virtual SString EncodeString(const SString &s);
	virtual int WindowMessageBox(GUI::Window &w, const GUI::gui_string &msg, int style) = 0;
	virtual int WindowMessageBox(const char* msg, int flag, const GUI::gui_char *p1, const GUI::gui_char *p2, const GUI::gui_char *p3) = 0;
	int FindInTarget(const char *findWhat, int lenFind, int startPosition, int endPosition);
	virtual bool FindHasText() const;
	virtual void MoveBack(int distance);
	virtual void ScrollEditorIfNeeded();
	virtual void UIClosed();
	virtual void UIHasFocus();
	void GoMatchingBrace(bool select);
	void GoMatchingPreprocCond(int direction, bool select);
	void OutputAppendString(const char *s, int len = -1);
	void OutputAppendStringSynchronised(const char *s, int len = -1);
	void FindResAppendString(const char *s, int len = -1);
	void FindResAppendStringSynchronised(const char *s, int len = -1);
	virtual void MakeOutputVisible(GUI::ScintillaWindow &wBottom);
	void ClearJobQueue();
	virtual void Execute();
	virtual void StopExecute() = 0;

	bool GoMessage(int dir, GUI::ScintillaWindow &wBottom);
	char *GetNearestWords(const char *wordStart, int searchLen,
		const char *separators, bool ignoreCase=false, bool exactLen=false);
	virtual void EliminateDuplicateWords(char *words);
	virtual bool StartAutoComplete();
	virtual bool StartAutoCompleteWord(bool onlyOneWord);
	virtual bool StartBlockComment();
	virtual bool StartBoxComment();
	virtual bool StartStreamComment();
	unsigned int GetLinePartsInStyle(int line, int style1, int style2, SString sv[], int len);
	void SetLineIndentation(int line, int indent);
	int GetLineIndentation(int line);
	int GetLineIndentPosition(int line);
	void ConvertIndentation(int tabSize, int useTabs);
	bool RangeIsAllWhitespace(int start, int end);
	IndentationStatus GetIndentState(int line);
	int IndentOfBlock(int line);
	void MaintainIndentation(char ch);
	void AutomaticIndentation(char ch);
	void CharAdded(char ch);
	void CharAddedOutput(int ch);
	void SetTextProperties(PropSetFile &ps);
	virtual void SetFileProperties(PropSetFile &ps) = 0;
	int GetLineLength(int line);
	int GetCurrentLineNumber();
	int GetCurrentScrollPosition();
	virtual void AddCommand(const SString &cmd, const SString &dir,
	        JobSubsystem jobType, const SString &input = "",
	        int flags = 0);

	virtual void QuitProgram() = 0;
	void CloseTab(int tab);
	void CloseAllBuffers(bool loadingSession = false);
	int SaveAllBuffers(bool forceQuestion, bool alwaysYes = false);
	void SaveTitledBuffers();
	virtual void CopyAsRTF() {}
	void SetLineNumberWidth();
	virtual void Command(WPARAM wParam, LPARAM lParam) = 0;
	void MenuCommand(int cmdID, int source = 0);
	void FoldChanged(int line, int levelNow, int levelPrev, GUI::ScintillaWindow *w);
	//void FoldChanged(int position);
	void Expand(int &line, bool doExpand, bool force = false,
		int visLevels = 0, int level = -1);
	void Expand(GUI::ScintillaWindow *w, int &line, bool doExpand, bool force = false,
		int visLevels = 0, int level = -1);
	void FoldAll();
	void CollapseOutput();
	void ToggleFoldRecursive(int line, int level);
	void EnsureAllChildrenVisible(int line, int level);
	void EnsureRangeVisible(int posStart, int posEnd, bool enforcePolicy = true);
	void GotoLineEnsureVisible(int line);
	bool MarginClick(int position, int modifiers, GUI::ScintillaWindow *w);
	void NewLineInOutput();
	virtual void Notify(SCNotification *notification);

	virtual void ActivateWindow(const char *timestamp) = 0;

	void BookmarkAdd(int lineno = -1);
	void BookmarkDelete(int lineno = -1);
	bool BookmarkPresent(int lineno = -1);
	void BookmarkToggle(int lineno = -1);
	void BookmarkNext(bool forwardScan = true, bool select = false);

	virtual void CheckMenus();
//!	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) = 0; //!-remove-[ExtendedContextMenu]
	void ContextMenu(GUI::ScintillaWindow &wSource, GUI::Point pt, GUI::Window wCmd, int isMargin = 0);

	void DropFileStackTop();

//!-end-[ExtendedContextMenu]
	bool AddFileToBuffer(FilePath file, int pos);
	void AddFileToStack(FilePath file, Sci_CharacterRange selection, int scrollPos);
	void RemoveFileFromStack(FilePath file);
	RecentFile GetFilePosition();
	void DisplayAround(const RecentFile &rf);
	void StackMenu(int pos);
	void StackMenuNext();
	void StackMenuPrev();

	JobSubsystem SubsystemType(char c);
	JobSubsystem SubsystemType(const char *cmd, int item = -1);

	void AssignKey(int key, int mods, int cmd);
	void ViewWhitespace(bool view);

	//void ImportMenu(int pos);
	//void SetLanguageMenu();
	void SetPropertiesInitial();
	SString GetFileNameProperty(const char *name);
	virtual void ReadPropertiesInitial();
	void ReadFontProperties();
	void SetOverrideLanguage(const char *lexer, bool bFireEvent);
	StyleAndWords GetStyleAndWords(const char *base);
	SString ExtensionFileName();
	const char *GetNextPropItem(const char *pStart, char *pPropItem, int maxLen);
	void ForwardPropertyToEditor(const char *key);
	void DefineMarker(int marker, int markerType, Colour fore, Colour back);
	SString FindLanguageProperty(const char *pattern, const char *defaultValue = "");
	int FindIntLanguageProperty(const char *pattern, int defaultValue = 0); //!-add-[BetterCalltips]
	virtual void ReadProperties();
	void SetOneStyle(GUI::ScintillaWindow &win, int style, const StyleDefinition &sd);
	void SetStyleFor(GUI::ScintillaWindow &win, const char *language);
	void ReloadProperties();

	void CheckReload();
	void Activate(bool activeApp);
	virtual GUI::Rectangle GetClientRectangle()=0;
	void Redraw();

	enum { splitOut = 1, splitSidebar = 2, splitFindRes };

	void UIAvailable();
	void StartRecordMacro();
	void StopRecordMacro();

	bool RecordMacroCommand(SCNotification *notification);

	virtual bool IsStdinBlocked();
	void OpenFromStdin(bool UseOutputPane);
	void OpenFilesFromStdin();
	enum GrepFlags {
	    grepNone = 0, grepWholeWord = 1, grepMatchCase = 2, grepStdOut = 4,
		grepDot = 8, grepBinary = 16, grepRegExp = 32, grepSubDir = 64, grepGroup = 128, grepProgress = 256
	};
	typedef struct GrepOut{
		SString strOut;
		int iLines;
		int iFiles;
		int iInFiles;
	}GrepOut;
	void GrepRecursive(GrepFlags gf, FilePath baseDir, const char *searchString, const GUI::gui_char *fileTypes, unsigned int basePath, GrepOut *grepOut, std::regex *pRegExp); //!-change-[FindResultListStyle]
	void CountRecursive(GrepFlags gf, FilePath baseDir, const GUI::gui_char *fileTypes, GrepOut *grepOut);
	bool strstrRegExp(char *text, const char *sub, void *pRegExp, GrepFlags gf);
	void InternalGrep(GrepFlags gf, const GUI::gui_char *directory, const GUI::gui_char *files, const char *search);

	void SendOneProperty(const char *kind, const char *key, const char *val);
	void PropertyFromDirector(const char *arg);
	void WideChrToMyltiBate(SString strIn, SString &strOut);//Перекодировка для последующего вывода в консоль
	int internalRunLuaThread(SString strCmd, SString strDesc);


	// ExtensionAPI
	sptr_t Send(Pane p, unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	char *Range(Pane p, int start, int end);
	char *Line(Pane p, int line, int bNeedEnd);
	void Remove(Pane p, int start, int end);
	void Insert(Pane p, int pos, const char *s);
	void Trace(const char *s);
	void UnsetProperty(const char *key);
	uptr_t GetInstance();
	void ShutDown();
public:
	void DoMenuCommand(int cmdID);
protected:
	virtual int ActiveEditor();
	char *GetTranslation(const char *s, bool retainIfNotFound = true); //!-add-[LocalizationFromLua]
	virtual int PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter) = 0;
	virtual void RunInConcole();
	virtual void RunAsync(int idx)=0;
	virtual void SetRestart(const char* cmdLine)=0;

	// Valid CurrentWord characters
	bool iswordcharforsel(char ch);
	bool isfilenamecharforsel(char ch);
	bool islexerwordcharforsel(char ch);
	int OnMenuCommandCallsCount; //!-add-[OnMenuCommand]
	virtual bool SwitchMacroHook(bool bSet) = 0;
public:
	char *Property(const char *key);

	enum { maxParam = 4 };

	SciTEBase(Extension *ext = 0);
	virtual ~SciTEBase();

	void SetProperty(const char *key, const char *val);
	
	GUI::WindowID GetID() { return wSciTE.GetID(); }

//!-start-[GetApplicationProps]
	static SciTEBase *GetApplicationInstance();
	static PropSetFile *GetProps() {
		SciTEBase *app = GetApplicationInstance();
		if (app != NULL) return &(app->props);
		return NULL;
	}
//!-end-[GetApplicationProps]
	enum OutputMode{ outConsole = 1, outLua = 2, outInterface = 3, outluaPrint = 4, outNull = 0 };
	OutputMode curOutMode = outNull;
	virtual Ihandle * IupTab(int id) = 0;
	bool bFinalise = false;

	virtual int CompareFile(FilePath &fileCompare, const char* txtCompare);
	virtual bool IsRunAsAdmin() = 0;
	virtual bool NewInstance(const char* arg, bool asAdmin)= 0;
private:
	// un-implemented copy-constructor and assignment operator
	SciTEBase(const SciTEBase&);
	void operator=(const SciTEBase&);
};



/// Base size of file I/O operations.
const int blockSize = 131072;

#if defined(GTK)
// MessageBox
#define MB_OK	(0L)
#define MB_YESNO	(0x4L)
#define MB_YESNOCANCEL	(0x3L)
#define MB_ICONWARNING	(0x30L)
#define MB_ICONQUESTION (0x20L)
#define IDOK	(1)
#define IDCANCEL	(2)
#define IDYES	(6)
#define IDNO	(7)
#endif

int ControlIDOfCommand(unsigned long);
void LowerCaseString(char *s);
long ColourOfProperty(PropSetFile &props, const char *key, Colour colourDefault);
void WindowSetFocus(GUI::ScintillaWindow &w);

inline bool isspacechar(unsigned char ch) {
    return (ch == ' ') || ((ch >= 0x09) && (ch <= 0x0d));
}
#endif

static UniMode CodingCookieValue(const char *buf, size_t length);