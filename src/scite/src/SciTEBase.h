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

class Buffer : public RecentFile {
public:
	sptr_t doc;
	bool isDirty;
	GUI::gui_char *ROMarker; //!-add-[ReadOnlyTabMarker]
	bool useMonoFont;
	UniMode unicodeMode;
	time_t fileModTime;
	time_t fileModLastAsk;
	enum { fmNone, fmMarked, fmModified} findMarks;
	SString overrideExtension;	///< User has chosen to use a particular language
	std::vector<int> foldState;
	int editorSide;
	Buffer* pFriend;
	Buffer() :
//!			RecentFile(), doc(0), isDirty(false), useMonoFont(false),
			RecentFile(), doc(0), isDirty(false), ROMarker(0), useMonoFont(false),  //!-change-[ReadOnlyTabMarker]
			unicodeMode(uni8Bit), fileModTime(0), fileModLastAsk(0), findMarks(fmNone), editorSide(IDM_SRCWIN), foldState(), pFriend(NULL){}

	void Init() {
		RecentFile::Init();
		isDirty = false;
		ROMarker = NULL; //!-add-[ReadOnlyTabMarker]
		useMonoFont = false;
		unicodeMode = uni8Bit;
		fileModTime = 0;
		fileModLastAsk = 0;
		findMarks = fmNone;
		overrideExtension = "";
		foldState.clear();
		pFriend = NULL;
	}

	void SetTimeFromFile() {
		fileModTime = ModifiedTime();
		fileModLastAsk = fileModTime;
		if (pFriend){
			pFriend->fileModTime = fileModTime;
			pFriend->fileModTime = fileModTime;
		}
	}
//!-start-[OpenNonExistent]
	bool DocumentNotSaved() const {
		return (isDirty || (!IsUntitled() && (fileModTime == 0))) || (pFriend && (pFriend->isDirty || (!pFriend->IsUntitled() && (pFriend->fileModTime == 0))));
	}
//!-end-[OpenNonExistent]
};

class EditSwitcher{
public:
	virtual void SwitchTo(int wndIdm, Buffer* pBuf) = 0;
	virtual int GetWindowIdm() = 0;
	virtual void SetBuffPointer(Buffer* pBuf) = 0;
};

class BufferList {
protected:
	int current;
	int stackcurrent;
	int *stack;
public:
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
	int StackPrev();
	void CommitStackSelection();
	void MoveToStackTop(int index);
	void ShiftTo(int indexFrom, int indexTo);
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

class Localization : public PropSetFile, public ILocalize {
	SString missing;
public:
	bool read;
	Localization() : PropSetFile(true), read(false) {
	}
	GUI::gui_string Text(const char *s, bool retainIfNotFound=true);
	void SetMissing(const SString &missing_) {
		missing = missing_;
	}
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

class SciTEBase : public ExtensionAPI, public Searcher {
protected:
	GUI::gui_string windowName;
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
	enum { markerBookmark = 1 };
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

	GUI::Window wSciTE;  ///< Contains wTabBar, wContent,wIupStatus, wIupBar

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
		virtual void SwitchTo(int wndIdm, Buffer* pBuf) ;
		virtual int GetWindowIdm();
		virtual void SetBuffPointer(Buffer* pBuf);
		virtual void SetCoBuffPointer(Buffer* pBuf);
		void Switch();
		ScintillaWindowEditor coEditor;
	private:
		Buffer *buffer_L;
		Buffer *buffer_R= NULL;
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
	GUI::Window wTabBar;
	bool viewWs;
	bool viewIndent;
	bool tabMultiLine;
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
	bool fullScreen;
//!	enum { toolMax = 50 };
	enum { toolMax = 300 }; //!-change-[ToolsMax]
	Extension *extender;
	bool needReadProperties;
	bool preserveFocusOnEditor; //!-add-[GoMessageImprovement]

	int widthPanel;
	int widthPanelStartDrag;
	int prevousWidthPanel;
	int sizeSplit;
	int heightOutput;
	int heightOutputStartDrag;
	GUI::Point ptStartDrag;
	bool capturedMouse;
	int previousHeightOutput;
	bool firstPropertiesRead;
	bool bufferedDraw;
	bool twoPhaseDraw;
	bool bracesCheck;
	bool bracesSloppy;
	int bracesStyle;
	int braceCount;

	int widthFindRes;
	int widthFindResStartDrag;
	int prevousWidthFindRes;

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
	int startCalltipWord;
	int currentCallTip;
	int maxCallTips;
	SString currentCallTipWord;
	int lastPosCallTip;

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

	PropSetFile propsAbbrev;

	PropSetFile propsSession;

	FilePath pathAbbreviations;

	Localization localiser;

	PropSetFile propsStatus;	// Not attached to a file but need SetInteger method.

	enum { bufferMax = 100 };
	BufferList buffers;

	// Handle buffers
	sptr_t GetDocumentAt(int index);
	
	void UpdateBuffersCurrent();
	bool IsBufferAvailable();
	bool CanMakeRoom(bool maySaveIfDirty = true);
	void SetDocumentAt(int index, bool updateStack = true, bool switchTab = true, bool bExit = false);
	void GetBufferName(int i, char *c){lstrcpynA( c, buffers.buffers[i].AsUTF8().c_str(), 2000);};
	bool GetBuffersSavedState(int i){ return ! buffers.buffers[i].DocumentNotSaved(); };
	int GetBuffersCount(){return buffers.length; };		
	int GetCurrentBufer(){ return buffers.Current(); };		  
	Buffer *CurrentBuffer() {
		return buffers.CurrentBuffer();
	}
	void BuffersMenu();
	void Next();
	void Prev();
	void NextInStack();
	void PrevInStack();
	void EndStackedTabbing();

	virtual void TabInsert(int index, const GUI::gui_char *title) = 0;
	virtual void TabSelect(int index) = 0;
	virtual void RemoveAllTabs() = 0;
	void ShiftTab(int indexFrom, int indexTo);
	void MoveTabRight();
	void MoveTabLeft();
	void CloneTab();
	void ChangeTabWnd();
	void CheckRightEditorVisible();
	bool m_bRightEditorVisible = false;

	void ReadGlobalPropFile();
	void ReadAbbrevPropFile();
	void ReadLocalPropFile();
	void ReadDirectoryPropFile();

	sptr_t CallFocused(unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	sptr_t CallPane(int destination, unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	void CallChildren(unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	SString GetTranslationToAbout(const char * const propname, bool retainIfNotFound = true);
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
	void RestoreState(const Buffer &buffer, bool setCaption = true);
	void Close(bool updateUI = true, bool loadingSession = false, bool makingRoomForNew = false);
	bool bBlockUIUpdate = false;
	bool IsAbsolutePath(const char *path);
	bool Exists(const GUI::gui_char *dir, const GUI::gui_char *path, FilePath *resultPath);
	void DiscoverEOLSetting();
	void DiscoverIndentSetting();
	SString DiscoverLanguage(const char *buf, size_t length);
	void OpenFile(int fileSize, bool suppressMessage);
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
	bool Save();
	void SaveAs(const GUI::gui_char *file, bool fixCase);
	virtual void SaveACopy() = 0;
	void SaveToHTML(FilePath saveName);
	void StripTrailingSpaces();
	void EnsureFinalNewLine();
	bool SaveBuffer(FilePath saveName);
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
	virtual void FindMessageBox(const SString &msg, const SString *findItem = 0) = 0;
	int FindInTarget(const char *findWhat, int lenFind, int startPosition, int endPosition);
	virtual bool FindHasText() const;
	virtual void MoveBack(int distance);
	virtual void ScrollEditorIfNeeded();
	virtual void UIClosed();
	virtual void UIHasFocus();
	virtual void TabSizeDialog() = 0;
	virtual bool ParametersOpen() = 0;
	virtual void ParamGrab() = 0;
	virtual bool ParametersDialog(bool modal) = 0;
	bool HandleXml(char ch);
	SString FindOpenXmlTag(const char sel[], int nSize);
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
	virtual bool StartCallTip();
	char *GetNearestWords(const char *wordStart, int searchLen,
		const char *separators, bool ignoreCase=false, bool exactLen=false);
	virtual void FillFunctionDefinition(int pos = -1);
	void ContinueCallTip();
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
	virtual void AboutDialog() = 0;
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
	void FoldChanged(int position);
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

	void RemoveFindMarks();

	void BookmarkAdd(int lineno = -1);
	void BookmarkDelete(int lineno = -1);
	bool BookmarkPresent(int lineno = -1);
	void BookmarkToggle(int lineno = -1);
	void BookmarkNext(bool forwardScan = true, bool select = false);
	void ToggleOutputVisible();
	virtual void SizeSubWindows() = 0;

	virtual void CheckMenus();
//!	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) = 0; //!-remove-[ExtendedContextMenu]
	void ContextMenu(GUI::ScintillaWindow &wSource, GUI::Point pt, GUI::Window wCmd);

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
	void SetAboutMessage(GUI::ScintillaWindow &wsci, const char *appTitle);
	void ImportMenu(int pos);
	void SetLanguageMenu();
	void SetPropertiesInitial();
	GUI::gui_string LocaliseMessage(const char *s,
		const GUI::gui_char *param0 = 0, const GUI::gui_char *param1 = 0, const GUI::gui_char *param2 = 0);
	virtual void ReadLocalization();
	SString GetFileNameProperty(const char *name);
	virtual void ReadPropertiesInitial();
	void ReadFontProperties();
	void SetOverrideLanguage(const char *lexer, bool bFireEvent);
	StyleAndWords GetStyleAndWords(const char *base);
	SString ExtensionFileName();
	const char *GetNextPropItem(const char *pStart, char *pPropItem, int maxLen);
	void ForwardPropertyToEditor(const char *key);
	void DefineMarker(int marker, int markerType, Colour fore, Colour back);
	void ReadAPI(const SString &fileNameForExtension);
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
	int NormaliseSplit(int splitPos);

	enum { splitOut = 1, splitSidebar = 2, splitFindRes };
	void MoveSplit(GUI::Point ptNewDrag, int movedSplitter);

	void UIAvailable();
	void PerformOne(char *action);
	void StartRecordMacro();
	void StopRecordMacro();
	void StartPlayMacro();
	bool RecordMacroCommand(SCNotification *notification);
	void ExecuteMacroCommand(const char * command);
	void AskMacroList();
	bool StartMacroList(const char *words);
	void ContinueMacroList(const char *stxt);
	bool ProcessCommandLine(GUI::gui_string &args, int phase);
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
	void EnumProperties(const char *action);
	void SendOneProperty(const char *kind, const char *key, const char *val);
	void PropertyFromDirector(const char *arg);
	void PropertyToDirector(const char *arg);
	void WideChrToMyltiBate(SString strIn, SString &strOut);//ѕерекодировка дл€ последующего вывода в консоль
	int internalRunLuaThread(SString strCmd, SString strDesc);


	// ExtensionAPI
	sptr_t Send(Pane p, unsigned int msg, uptr_t wParam = 0, sptr_t lParam = 0);
	char *Range(Pane p, int start, int end);
	void Remove(Pane p, int start, int end);
	void Insert(Pane p, int pos, const char *s);
	void Trace(const char *s);
	void SetProperty(const char *key, const char *val);
	void UnsetProperty(const char *key);
	uptr_t GetInstance();
	void ShutDown();
	void Perform(const char *actions);
	void DoMenuCommand(int cmdID);
	virtual int ActiveEditor();
	bool ShowParametersDialog(const char *msg); //!-add-[ParametersDialogFromLua]
	char *GetTranslation(const char *s, bool retainIfNotFound = true); //!-add-[LocalizationFromLua]
	virtual int RunLuaThread(const char *s, const char *desc);
	virtual int PerformGrepEx(const char *sParams, const char *findWhat, const char *directory, const char *filter) = 0;
	virtual void RunInConcole();
	virtual void RunAsync(int idx)=0;

	// Valid CurrentWord characters
	bool iswordcharforsel(char ch);
	bool isfilenamecharforsel(char ch);
	bool islexerwordcharforsel(char ch);
	int OnMenuCommandCallsCount; //!-add-[OnMenuCommand]
public:
	char *Property(const char *key);

	enum { maxParam = 4 };

	SciTEBase(Extension *ext = 0);
	virtual ~SciTEBase();

	void ProcessExecute();
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