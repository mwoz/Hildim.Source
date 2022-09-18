// SciTE - Scintilla based Text Editor
/** @file SciTEBase.cxx
 ** Platform independent base class of editor.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#if defined(GTK)

#include <unistd.h>
#include <gtk/gtk.h>

#else

#undef _WIN32_WINNT
#define _WIN32_WINNT  0x0500
#ifdef _MSC_VER
// windows.h, et al, use a lot of nameless struct/unions - can't fix it, so allow it
#pragma warning(disable: 4201)
#endif
#include <windows.h>
#ifdef _MSC_VER
// okay, that's done, don't allow it in our code
#pragma warning(default: 4201)
#endif
#include <commctrl.h>

#ifdef _MSC_VER
#include <direct.h>
#endif
#ifdef __DMC__
#include <dir.h>
#endif

#endif

#include "Scintilla.h"
#include "SciLexer.h"

#include "IFaceTable.h"
#include "GUI.h"
#include "SString.h"
#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "SciTE.h"
#include "Mutex.h"
#include "JobQueue.h"
#include "SciTEBase.h"

#define _MAX_EXTENSION_RECURSIVE_CALL 100 //!-add-[OnSendEditor][OnMenuCommand]

// Contributor names are in UTF-8
const char *contributors[] = {
            "Atsuo Ishimoto",
            "Mark Hammond",
            "Francois Le Coguiec",
            "Dale Nagata",
            "Ralf Reinhardt",
            "Philippe Lhoste",
            "Andrew McKinlay",
            "Stephan R. A. Deibel",
            "Hans Eckardt",
            "Vassili Bourdo",
            "Maksim Lin",
            "Robin Dunn",
            "John Ehresman",
            "Steffen Goeldner",
            "Deepak S.",
            "DevelopMentor http://www.develop.com",
            "Yann Gaillard",
            "Aubin Paul",
            "Jason Diamond",
            "Ahmad Baitalmal",
            "Paul Winwood",
            "Maxim Baranov",
#if defined(GTK)
            "Icons Copyright(C) 1998 by Dean S. Jones",
            "    http://jfa.javalobby.org/projects/icons/",
#endif
            "Ragnar H\xc3\xb8jland",
            "Christian Obrecht",
            "Andreas Neukoetter",
            "Adam Gates",
            "Steve Lhomme",
            "Ferdinand Prantl",
            "Jan Dries",
            "Markus Gritsch",
            "Tahir Karaca",
            "Ahmad Zawawi",
            "Laurent le Tynevez",
            "Walter Braeu",
            "Ashley Cambrell",
            "Garrett Serack",
            "Holger Schmidt",
            "ActiveState http://www.activestate.com",
            "James Larcombe",
            "Alexey Yutkin",
            "Jan Hercek",
            "Richard Pecl",
            "Edward K. Ream",
            "Valery Kondakoff",
            "Sm\xc3\xa1ri McCarthy",
            "Clemens Wyss",
            "Simon Steele",
            "Serge A. Baranov",
            "Xavier Nodet",
            "Willy Devaux",
            "David Clain",
            "Brendon Yenson",
            "Vamsi Potluru http://www.baanboard.com",
            "Praveen Ambekar",
            "Alan Knowles",
            "Kengo Jinno",
            "Valentin Valchev",
            "Marcos E. Wurzius",
            "Martin Alderson",
            "Robert Gustavsson",
            "Jos\xc3\xa9 Fonseca",
            "Holger Kiemes",
            "Francis Irving",
            "Scott Kirkwood",
            "Brian Quinlan",
            "Ubi",
            "Michael R. Duerig",
            "Deepak T",
            "Don Paul Beletsky",
            "Gerhard Kalab",
            "Olivier Dagenais",
            "Josh Wingstrom",
            "Bruce Dodson",
            "Sergey Koshcheyev",
            "Chuan-jian Shen",
            "Shane Caraveo",
            "Alexander Scripnik",
            "Ryan Christianson",
            "Martin Steffensen",
            "Jakub Vr\xc3\xa1na",
            "The Black Horus",
            "Bernd Kreuss",
            "Thomas Lauer",
            "Mike Lansdaal",
            "Yukihiro Nakai",
            "Jochen Tucht",
            "Greg Smith",
            "Steve Schoettler",
            "Mauritius Thinnes",
            "Darren Schroeder",
            "Pedro Guerreiro",
            "Steven te Brinke",
            "Dan Petitt",
            "Biswapesh Chattopadhyay",
            "Kein-Hong Man",
            "Patrizio Bekerle",
            "Nigel Hathaway",
            "Hrishikesh Desai",
            "Sergey Puljajev",
            "Mathias Rauen",
            "Angelo Mandato http://www.spaceblue.com",
            "Denis Sureau",
            "Kaspar Schiess",
            "Christoph H\xc3\xb6sler",
            "Jo\xc3\xa3o Paulo F Farias",
            "Ron Schofield",
            "Stefan Wosnik",
            "Marius Gheorghe",
            "Naba Kumar",
            "Sean O'Dell",
            "Stefanos Togoulidis",
            "Hans Hagen",
            "Jim Cape",
            "Roland Walter",
            "Brian Mosher",
            "Nicholas Nemtsev",
            "Roy Wood",
            "Peter-Henry Mander",
            "Robert Boucher",
            "Christoph Dalitz",
            "April White",
            "S. Umar",
            "Trent Mick",
            "Filip Yaghob",
            "Avi Yegudin",
            "Vivi Orunitia",
            "Manfred Becker",
            "Dimitris Keletsekis",
            "Yuiga",
            "Davide Scola",
            "Jason Boggs",
            "Reinhold Niesner",
            "Jos van der Zande",
            "Pescuma",
            "Pavol Bosik",
            "Johannes Schmid",
            "Blair McGlashan",
            "Mikael Hultgren",
            "Florian Balmer",
            "Hadar Raz",
            "Herr Pfarrer",
            "Ben Key",
            "Gene Barry",
            "Niki Spahiev",
            "Carsten Sperber",
            "Phil Reid",
            "Iago Rubio",
            "R\xc3\xa9gis Vaquette",
            "Massimo Cor\xc3\xa0",
            "Elias Pschernig",
            "Chris Jones",
            "Josiah Reynolds",
            "Robert Roessler http://www.rftp.com",
            "Steve Donovan",
            "Jan Martin Pettersen",
            "Sergey Philippov",
            "Borujoa",
            "Michael Owens",
            "Franck Marcia",
            "Massimo Maria Ghisalberti",
            "Frank Wunderlich",
            "Josepmaria Roca",
            "Tobias Engvall",
            "Suzumizaki Kimitaka",
            "Michael Cartmell",
            "Pascal Hurni",
            "Andre",
            "Randy Butler",
            "Georg Ritter",
            "Michael Goffioul",
            "Ben Harper",
            "Adam Strzelecki",
            "Kamen Stanev",
            "Steve Menard",
            "Oliver Yeoh",
            "Eric Promislow",
            "Joseph Galbraith",
            "Jeffrey Ren",
            "Armel Asselin",
            "Jim Pattee",
            "Friedrich Vedder",
            "Sebastian Pipping",
            "Andre Arpin",
            "Stanislav Maslovski",
            "Martin Stone",
            "Fabien Proriol",
            "mimir",
            "Nicola Civran",
            "Snow",
            "Mitchell Foral",
            "Pieter Holtzhausen",
            "Waldemar Augustyn",
            "Jason Haslam",
            "Sebastian Steinlechner",
            "Chris Rickard",
            "Rob McMullen",
            "Stefan Schwendeler",
            "Cristian Adam",
            "Nicolas Chachereau",
            "Istvan Szollosi",
            "Xie Renhui",
            "Enrico Tr\xc3\xb6ger",
            "Todd Whiteman",
            "Yuval Papish",
            "instanton",
            "Sergio Lucato",
            "VladVRO",
            "Dmitry Maslov",
            "chupakabra",
            "Juan Carlos Arevalo Baeza",
            "Nick Treleaven",
            "Stephen Stagg",
            "Jean-Paul Iribarren",
            "Tim Gerundt",
            "Sam Harwell",
            "Boris",
            "Jason Oster",
            "Gertjan Kloosterman",
            "alexbodn",
            "Sergiu Dotenco",
            "Anders Karlsson",
            "ozlooper",
            "Marko Njezic",
            "Eugen Bitter",
            "Christoph Baumann",
            "Christopher Bean",
            "Sergey Kishchenko",
            "Kai Liu",
            "Andreas Rumpf",
            "James Moffatt",
            "Yuzhou Xin",
            "Nic Jansma",
            "Evan Jones",
            "Mike Lischke",
            "Eric Kidd",
            "maXmo",
            "David Severwright",
            "Jon Strait",
            "Oliver Kiddle",
            "Etienne Girondel",
            "Haimag Ren",
            "Andrey Moskalyov",
            "Xavi",
            "Toby Inkster",
            "Eric Forgeot",
            "Colomban Wendling",
            "Neo",
            "Jordan Russell",
            "Farshid Lashkari",
            "Sam Rawlins",
            "Michael Mullin",
            "Carlos SS",
            "vim",
            "Martial Demolins",
        };

// AddStyledText only called from About so static size buffer is OK
void AddStyledText(GUI::ScintillaWindow &wsci, const char *s, int attr) {
	char buf[1000];
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		buf[i*2] = s[i];
		buf[i*2 + 1] = static_cast<char>(attr);
	}
	wsci.SendPointer(SCI_ADDSTYLEDTEXT,
	        static_cast<int>(len*2), const_cast<char *>(buf));
}

void SetAboutStyle(GUI::ScintillaWindow &wsci, int style, Colour fore) {
	wsci.Send(SCI_STYLESETFORE, style, fore);
}

static void HackColour(int &n) {
	n += (rand() % 100) - 50;
	if (n > 0xE7)
		n = 0x60;
	if (n < 0)
		n = 0x80;
}

Searcher::Searcher() {
	wholeWord = false;
	matchCase = false;
	regExp = false;
	unSlash = false;
	wrapFind = true;
	reverseFind = false;

	replacing = false;
	havefound = false;
	findInStyle = false;
	findStyle = 0;

	focusOnReplace = false;
}

// The find and replace dialogs and strips often manipulate boolean
// flags based on dialog control IDs and menu IDs.
bool &Searcher::FlagFromCmd(int cmd) {
	static bool notFound;
	switch (cmd) {
		case IDWHOLEWORD:
		case IDM_WHOLEWORD:
			return wholeWord;
		case IDMATCHCASE:
		case IDM_MATCHCASE:
			return matchCase;
		case IDREGEXP:
		case IDM_REGEXP:
			return regExp;
		case IDUNSLASH:
		case IDM_UNSLASH:
			return unSlash;
		case IDWRAP:
		case IDM_WRAPAROUND:
			return wrapFind;
		case IDDIRECTIONUP:
		case IDM_DIRECTIONUP:
			return reverseFind;
	}
	return notFound;
}

SciTEBase::SciTEBase(Extension *ext) : apis(true), extender(ext) {
	codePage = 0;
	characterSet = 0;
	language = "java";
	lexLanguage = SCLEX_CPP;
	lexLPeg = -1;
	functionDefinition = 0;
	indentOpening = true;
	indentClosing = true;
	indentMaintain = false;
	statementLookback = 10;
	preprocessorSymbol = '\0';

	iuptbVisible = false;
	sbNum = 1;
	visHeightTools = 0;
	visHeightTab = 0;
	visHeightEditor = 1;
	visHeightIuptool = 0;
	heightBar = 7;
	dialogsOnScreen = 0;
	topMost = false;
	wrap = false;
	wrapOutput = false;
	wrapFindRes = false;
	wrapStyle = SC_WRAP_WORD;
	isReadOnly = false;
	openFilesHere = false;

	allowMenuActions = true;
	scrollOutput = 1;
	returnOutputToCommand = true;

	ptStartDrag.x = 0;
	ptStartDrag.y = 0;
	capturedMouse = false;
	firstPropertiesRead = true;
	bufferedDraw = true;
	twoPhaseDraw = true;
	bracesCheck = true;
	bracesSloppy = false;
	bracesStyle = 0;
	braceCount = 0;

	indentationWSVisible = true;
	indentExamine = SC_IV_LOOKBOTH;

	autoCompleteIgnoreCase = false;
	autoCompleteIncremental = false;
	callTipIgnoreCase = false;
	calltipShowPerPage = 1; //!-add-[BetterCalltips]
	autoCCausedByOnlyOne = false;

	margin = false;
	marginWidth = marginWidthDefault;
	foldMargin = true;
	foldMarginWidth = foldMarginWidthDefault;
	lineNumbers = false;
	lineNumbersWidth = lineNumbersWidthDefault;
	lineNumbersExpand = false;
	usePalette = false;

	abbrevInsert[0] = '\0';

	macrosEnabled = false;
	recording = false;

	propsBase.superPS = &propsEmbed;
	propsUser.superPS = &propsBase;
	propsDirectory.superPS = &propsUser;
	propsLocal.superPS = &propsDirectory;
	props.superPS = &propsLocal;

	propsStatus.superPS = &props;

	needReadProperties = false;
	preserveFocusOnEditor = false; //!-add-[GoMessageImprovement]
	wEditor.pBase = this; //!-add-[OnSendEditor]
	wEditor.coEditor.pBase = this; //!-add-[OnSendEditor]
	wEditorR.pBase = this;
	wEditorL.pBase = this;
	buffers.pEditor = &wEditor;
	OnMenuCommandCallsCount = 0;	//!-add-[OnMenuCommand]
}

SciTEBase::~SciTEBase() {
	bFinalise = true;
	if (extender)
		extender->Finalise();
//!	popup.Destroy();	//!-remove-[ExtendedContextMenu]
}

//!-start-[OnSendEditor]
static bool isInterruptableMessage(unsigned int msg) {
	switch (msg) {
	// Enumerates all macroable messages
	// list copied from /scintilla/src/Editor.cxx
		case SCI_CUT:
		case SCI_COPY:
		case SCI_PASTE:
		case SCI_CLEAR:
		case SCI_REPLACESEL:
		case SCI_ADDTEXT:
		case SCI_INSERTTEXT:
		case SCI_APPENDTEXT:
		case SCI_CLEARALL:
		case SCI_SELECTALL:
		case SCI_GOTOLINE:
		case SCI_GOTOPOS:
		case SCI_SEARCHANCHOR:
		case SCI_SEARCHNEXT:
		case SCI_SEARCHPREV:
		case SCI_LINEDOWN:
		case SCI_LINEDOWNEXTEND:
		case SCI_PARADOWN:
		case SCI_PARADOWNEXTEND:
		case SCI_LINEUP:
		case SCI_LINEUPEXTEND:
		case SCI_PARAUP:
		case SCI_PARAUPEXTEND:
		case SCI_CHARLEFT:
		case SCI_CHARLEFTEXTEND:
		case SCI_CHARRIGHT:
		case SCI_CHARRIGHTEXTEND:
		case SCI_WORDLEFT:
		case SCI_WORDLEFTEXTEND:
		case SCI_WORDRIGHT:
		case SCI_WORDRIGHTEXTEND:
		case SCI_WORDPARTLEFT:
		case SCI_WORDPARTLEFTEXTEND:
		case SCI_WORDPARTRIGHT:
		case SCI_WORDPARTRIGHTEXTEND:
		case SCI_WORDLEFTEND:
		case SCI_WORDLEFTENDEXTEND:
		case SCI_WORDRIGHTEND:
		case SCI_WORDRIGHTENDEXTEND:
		case SCI_HOME:
		case SCI_HOMEEXTEND:
		case SCI_LINEEND:
		case SCI_LINEENDEXTEND:
		case SCI_HOMEWRAP:
		case SCI_HOMEWRAPEXTEND:
		case SCI_LINEENDWRAP:
		case SCI_LINEENDWRAPEXTEND:
		case SCI_DOCUMENTSTART:
		case SCI_DOCUMENTSTARTEXTEND:
		case SCI_DOCUMENTEND:
		case SCI_DOCUMENTENDEXTEND:
		case SCI_STUTTEREDPAGEUP:
		case SCI_STUTTEREDPAGEUPEXTEND:
		case SCI_STUTTEREDPAGEDOWN:
		case SCI_STUTTEREDPAGEDOWNEXTEND:
		case SCI_PAGEUP:
		case SCI_PAGEUPEXTEND:
		case SCI_PAGEDOWN:
		case SCI_PAGEDOWNEXTEND:
		case SCI_EDITTOGGLEOVERTYPE:
		case SCI_CANCEL:
		case SCI_DELETEBACK:
		case SCI_TAB:
		case SCI_BACKTAB:
		case SCI_FORMFEED:
		case SCI_VCHOME:
		case SCI_VCHOMEEXTEND:
		case SCI_VCHOMEWRAP:
		case SCI_VCHOMEWRAPEXTEND:
		case SCI_DELWORDLEFT:
		case SCI_DELWORDRIGHT:
		case SCI_DELLINELEFT:
		case SCI_DELLINERIGHT:
		case SCI_LINECOPY:
		case SCI_LINECUT:
		case SCI_LINEDELETE:
		case SCI_LINETRANSPOSE:
		case SCI_LINEDUPLICATE:
		case SCI_LOWERCASE:
		case SCI_UPPERCASE:
		case SCI_LINESCROLLDOWN:
		case SCI_LINESCROLLUP:
		case SCI_DELETEBACKNOTLINE:
		case SCI_HOMEDISPLAY:
		case SCI_HOMEDISPLAYEXTEND:
		case SCI_LINEENDDISPLAY:
		case SCI_LINEENDDISPLAYEXTEND:
		case SCI_SETSELECTIONMODE:
		case SCI_LINEDOWNRECTEXTEND:
		case SCI_LINEUPRECTEXTEND:
		case SCI_CHARLEFTRECTEXTEND:
		case SCI_CHARRIGHTRECTEXTEND:
		case SCI_HOMERECTEXTEND:
		case SCI_VCHOMERECTEXTEND:
		case SCI_LINEENDRECTEXTEND:
		case SCI_PAGEUPRECTEXTEND:
		case SCI_PAGEDOWNRECTEXTEND:
		case SCI_SELECTIONDUPLICATE:
	// One more interruptable messages
		case SCI_SETREADONLY:
		case SCI_MARKERADD:
		case SCI_MARKERDELETE:
		case SCI_MARKERDELETEALL:
			return true;
	}
	return false;
}

// messages list witch has not string parameter
static bool isNotStringParams(unsigned int msg) {
	switch (msg) {
		case SCI_MARKERADD:
		case SCI_MARKERDELETE:
			return true;
	}
	return false;
}

#define _MAX_SEND_RECURSIVE_CALL 100
static int static_iOnSendEditorCallsCount = 0;

sptr_t SciTEBase::ScintillaWindowEditor::Call( unsigned int msg, uptr_t wParam, sptr_t lParam )
{
	const char *result = NULL;
	if (pBase->extender && isInterruptableMessage(msg) && static_iOnSendEditorCallsCount < _MAX_SEND_RECURSIVE_CALL) {
		static_iOnSendEditorCallsCount++;
		if (!isNotStringParams(msg))
			result = pBase->extender->OnSendEditor(msg, wParam, reinterpret_cast<const char *>(lParam));
		else
			result = pBase->extender->OnSendEditor(msg, wParam, static_cast<long>(lParam));
		static_iOnSendEditorCallsCount--;
	}
	if (result != NULL) {
		if (pBase->recording && static_iOnSendEditorCallsCount == 0) {
			// send record macro notification
			SCNotification notification;
			notification.message = msg;
			notification.wParam = wParam;
			notification.lParam = lParam;
			pBase->RecordMacroCommand(&notification);
		}
		return reinterpret_cast<sptr_t>(result);
	} else {
		if (msg == SCI_SETREADONLY) {
			pBase->buffers.buffers[pBase->buffers.Current()].ROMarker = wParam ? true : false;
		}
		return ScintillaWindow::Call( msg, wParam, lParam);
	}
}
//!-end-[OnSendEditor]

void SciTEBase::ScintillaWindowSwitcher::Switch(bool ignorebuff){
	FilePath f = buffer_L;
	if (GetWindowIdm() == IDM_SRCWIN){
		SetID(pBase->wEditorR.GetID());
		coEditor.SetID(pBase->wEditorL.GetID());
	}
	else{
		SetID(pBase->wEditorL.GetID());
		coEditor.SetID(pBase->wEditorR.GetID());
	}
	if (ignorebuff)
		return;
	buffer_L = buffer_R;
	buffer_R = f;
}

void SciTEBase::ScintillaWindowSwitcher::SwitchTo(int wndIdm, FilePath* pBuff){
	bool needGrab =  (GetWindowIdm() != wndIdm);
	if (!needGrab && !pBuff) return;
	if (wndIdm == IDM_SRCWIN ){
		SetID(pBase->wEditorL.GetID());	
		coEditor.SetID(pBase->wEditorR.GetID());
		if (pBuff){
			buffer_L.Set(pBuff->AsInternal());
		}
		else if (lstrcmpW(buffer_L.AsInternal() ,L"") )
		{
			int id = pBase->buffers.GetDocumentByName(buffer_L, false, IDM_SRCWIN);
			if (id > -1) {
				pBase->SetDocumentAt(id, true, true, true);
				return;
			}
			//throw std::runtime_error("Buffers Filed");
			return;
		}
	}
	else if (wndIdm == IDM_COSRCWIN){
		if (!pBuff && !lstrcmpW(buffer_R.AsInternal(), L""))
			return;
		SetID(pBase->wEditorR.GetID());	
		coEditor.SetID(pBase->wEditorL.GetID());
		if (pBuff){
			buffer_R.Set(pBuff->AsInternal());
		}
		else
		{
			int id = pBase->buffers.GetDocumentByName(buffer_R, false, IDM_COSRCWIN);
			if (id > -1) {
				pBase->SetDocumentAt(id, true, true, true);
				return;
			}
			//throw std::runtime_error("Buffers Filed");
			return;
		}
	}
	else throw std::runtime_error("ScintillaWindowSwitcher: unknown idm");
	coEditor.Call(SCI_SETFOCUS, false);
}
void SciTEBase::ScintillaWindowSwitcher::SetBuffPointer(FilePath* pBuf) {
	if (GetWindowIdm() == IDM_SRCWIN) {
		buffer_L.Set(pBuf ? pBuf->AsInternal() : L"");
	} else {
		buffer_R.Set(pBuf ? pBuf->AsInternal() : L"");
	}
}

void SciTEBase::ScintillaWindowSwitcher::SetBuffEncoding(int e) {
	if (GetWindowIdm() == IDM_SRCWIN) {
		buffer_L._encoding = e;
	} else {
		buffer_R._encoding = e;
	}
}

void SciTEBase::ScintillaWindowSwitcher::SetCoBuffPointer(FilePath* pBuf){
	if (GetWindowIdm() == IDM_SRCWIN){
		buffer_R.Set(pBuf ? pBuf->AsInternal() : L"");
	}
	else{
		buffer_L.Set(pBuf ? pBuf->AsInternal(): L"");
	}
}

FilePath SciTEBase::ScintillaWindowSwitcher::GetCoBuffPointer() {
	if (GetWindowIdm() == IDM_SRCWIN) {
		return buffer_R;
	} else {
		return buffer_L;
	}
}

int SciTEBase::ScintillaWindowSwitcher::GetWindowIdm(){
	if (GetID() == pBase->wEditorL.GetID())	return IDM_SRCWIN;
	return IDM_COSRCWIN;
}

sptr_t SciTEBase::CallStringAll(unsigned int msg, uptr_t wParam, const char *s){
	wEditorR.CallString(msg, wParam, s);
	return wEditorL.CallString(msg, wParam, s);
}

sptr_t SciTEBase::CallAll(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	wEditorR.Call(msg, wParam, lParam);
	return wEditorL.Call(msg, wParam, lParam);
}
sptr_t SciTEBase::CallFocused(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	if (wOutput.HasFocus())
		return wOutput.Call(msg, wParam, lParam);
	else if (wFindRes.HasFocus())
		return wFindRes.Call(msg, wParam, lParam);
	else
		return wEditor.Call(msg, wParam, lParam);
}

sptr_t SciTEBase::CallPane(int destination, unsigned int msg, uptr_t wParam, sptr_t lParam) {
	if (destination == IDM_SRCWIN)
		return wEditor.Call(msg, wParam, lParam);
	else if (destination == IDM_RUNWIN)
		return wOutput.Call(msg, wParam, lParam);
	else if (destination == IDM_FINDRESWIN)
		return wFindRes.Call(msg, wParam, lParam);
	else
		return CallFocused(msg, wParam, lParam);
}

void SciTEBase::CallChildren(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	wEditor.Call(msg, wParam, lParam);
	wOutput.Call(msg, wParam, lParam);
	wFindRes.Call(msg, wParam, lParam);
}

void SciTEBase::ViewWhitespace(bool view) {
	if (view && indentationWSVisible) {
		wEditorL.Call(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS);
		wEditorR.Call(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS);
	} else if (view){
		wEditorL.Call(SCI_SETVIEWWS, SCWS_VISIBLEAFTERINDENT);
		wEditorR.Call(SCI_SETVIEWWS, SCWS_VISIBLEAFTERINDENT);
	} else {
		wEditorL.Call(SCI_SETVIEWWS, SCWS_INVISIBLE);
		wEditorR.Call(SCI_SETVIEWWS, SCWS_INVISIBLE);
	}
}

StyleAndWords SciTEBase::GetStyleAndWords(const char *base) {
	StyleAndWords sw;
	SString fileNameForExtension = ExtensionFileName();
	SString sAndW = props.GetNewExpand(base, fileNameForExtension.c_str());
	sw.styleNumber = sAndW.value();
	const char *space = strchr(sAndW.c_str(), ' ');
	if (space)
		sw.words = space + 1;
	return sw;
}

void SciTEBase::AssignKey(int key, int mods, int cmd) {
	wEditor.Call(SCI_ASSIGNCMDKEY,
	        LongFromTwoShorts(static_cast<short>(key),
	                static_cast<short>(mods)), cmd);
}

/**
 * Override the language of the current file with the one indicated by @a cmdID.
 * Mostly used to set a language on a file of unknown extension.
 */

void SciTEBase::SetOverrideLanguage(const char* lexer, bool bFireEvent) {
	RecentFile rf = GetFilePosition();
	EnsureRangeVisible(0, wEditor.Call(SCI_GETLENGTH), false);
	// Zero all the style bytes
	wEditor.Call(SCI_CLEARDOCUMENTSTYLE);

	CurrentBuffer()->overrideExtension = "x.";
	CurrentBuffer()->overrideExtension += lexer;
	ReadProperties();
	SetIndentSettings();
	wEditor.Call(SCI_COLOURISE, 0, -1);
	Redraw();
	DisplayAround(rf);
	if (bFireEvent) {
		extender->OnSwitchFile(props.GetString("FilePath"));
		layout.OnSwitchFile(buffers.buffers[buffers.Current()].editorSide);
	}

}


int SciTEBase::LengthDocument() {
	return wEditor.Call(SCI_GETLENGTH);
}

int SciTEBase::GetCaretInLine() {
	int caret = wEditor.Call(SCI_GETCURRENTPOS);
	int line = wEditor.Call(SCI_LINEFROMPOSITION, caret);
	int lineStart = wEditor.Call(SCI_POSITIONFROMLINE, line);
	return caret - lineStart;
}

void SciTEBase::GetLine(char *text, int sizeText, int line) {
	if (line < 0)
		line = GetCurrentLineNumber();
	int lineStart = wEditor.Call(SCI_POSITIONFROMLINE, line);
	int lineEnd = wEditor.Call(SCI_GETLINEENDPOSITION, line);
	int lineMax = lineStart + sizeText - 1;
	if (lineEnd > lineMax)
		lineEnd = lineMax;
	GetRange(wEditor, lineStart, lineEnd, text);
	text[lineEnd - lineStart] = '\0';
}

SString SciTEBase::GetLine(int line) {
	int len;
	// Get needed buffer size
	if (line < 0) {
		len = wEditor.Send(SCI_GETCURLINE, 0, 0);
	} else {
		len = wEditor.Send(SCI_GETLINE, line, 0);
	}
	// Allocate buffer
	SBuffer text(len);
	// And get the line
	if (line < 0) {
		wEditor.SendPointer(SCI_GETCURLINE, len, text.ptr());
	} else {
		wEditor.SendPointer(SCI_GETLINE, line, text.ptr());
	}
	return SString(text);
}

void SciTEBase::GetRange(GUI::ScintillaWindow &win, int start, int end, char *text) {
	Sci_TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = text;
	win.SendPointer(SCI_GETTEXTRANGE, 0, &tr);
}

/**
 * Check if the given line is a preprocessor condition line.
 * @return The kind of preprocessor condition (enum values).
 */
int SciTEBase::IsLinePreprocessorCondition(char *line) {
	char *currChar = line;
	char word[32];

	if (!currChar) {
		return false;
	}
	while (isspacechar(*currChar) && *currChar) {
		currChar++;
	}
	if (preprocessorSymbol && (*currChar == preprocessorSymbol)) {
		currChar++;
		while (isspacechar(*currChar) && *currChar) {
			currChar++;
		}
		size_t i = 0;
		while (!isspacechar(*currChar) && *currChar && (i < (sizeof(word) - 1))) {
			word[i++] = *currChar++;
		}
		word[i] = '\0';
		if (preprocCondStart.InList(word)) {
			return ppcStart;
		}
		if (preprocCondMiddle.InList(word)) {
			return ppcMiddle;
		}
		if (preprocCondEnd.InList(word)) {
			return ppcEnd;
		}
	}
	return noPPC;
}

/**
 * Search a matching preprocessor condition line.
 * @return @c true if the end condition are meet.
 * Also set curLine to the line where one of these conditions is mmet.
 */
bool SciTEBase::FindMatchingPreprocessorCondition(
    int &curLine,   		///< Number of the line where to start the search
    int direction,   		///< Direction of search: 1 = forward, -1 = backward
    int condEnd1,   		///< First status of line for which the search is OK
    int condEnd2) {		///< Second one

	bool isInside = false;
	char line[800];	// No need for full line
	int status, level = 0;
	int maxLines = wEditor.Call(SCI_GETLINECOUNT) - 1;

	while (curLine < maxLines && curLine > 0 && !isInside) {
		curLine += direction;	// Increment or decrement
		GetLine(line, sizeof(line), curLine);
		status = IsLinePreprocessorCondition(line);

		if ((direction == 1 && status == ppcStart) || (direction == -1 && status == ppcEnd)) {
			level++;
		} else if (level > 0 && ((direction == 1 && status == ppcEnd) || (direction == -1 && status == ppcStart))) {
			level--;
		} else if (level == 0 && (status == condEnd1 || status == condEnd2)) {
			isInside = true;
		}
	}

	return isInside;
}

/**
 * Find if there is a preprocessor condition after or before the caret position,
 * @return @c true if inside a preprocessor condition.
 */
bool SciTEBase::FindMatchingPreprocCondPosition(
    bool isForward,   		///< @c true if search forward
    int &mppcAtCaret,   	///< Matching preproc. cond.: current position of caret
    int &mppcMatch) {		///< Matching preproc. cond.: matching position

	bool isInside = false;
	int curLine;
	char line[800];	// Probably no need to get more characters, even if the line is longer, unless very strange layout...
	int status;

	// Get current line
	curLine = wEditor.Call(SCI_LINEFROMPOSITION, mppcAtCaret);
	GetLine(line, sizeof(line), curLine);
	status = IsLinePreprocessorCondition(line);

	switch (status) {
	case ppcStart:
		if (isForward) {
			isInside = FindMatchingPreprocessorCondition(curLine, 1, ppcMiddle, ppcEnd);
		} else {
			mppcMatch = mppcAtCaret;
			return true;
		}
		break;
	case ppcMiddle:
		if (isForward) {
			isInside = FindMatchingPreprocessorCondition(curLine, 1, ppcMiddle, ppcEnd);
		} else {
			isInside = FindMatchingPreprocessorCondition(curLine, -1, ppcStart, ppcMiddle);
		}
		break;
	case ppcEnd:
		if (isForward) {
			mppcMatch = mppcAtCaret;
			return true;
		} else {
			isInside = FindMatchingPreprocessorCondition(curLine, -1, ppcStart, ppcMiddle);
		}
		break;
	default:   	// Should be noPPC

		if (isForward) {
			isInside = FindMatchingPreprocessorCondition(curLine, 1, ppcMiddle, ppcEnd);
		} else {
			isInside = FindMatchingPreprocessorCondition(curLine, -1, ppcStart, ppcMiddle);
		}
		break;
	}

	if (isInside) {
		mppcMatch = wEditor.Call(SCI_POSITIONFROMLINE, curLine);
	}
	return isInside;
}

static bool IsBrace(char ch) {
	return ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '{' || ch == '}';
}

/**
 * Find if there is a brace next to the caret, checking before caret first, then
 * after caret. If brace found also find its matching brace.
 * @return @c true if inside a bracket pair.
 */
bool SciTEBase::FindMatchingBracePosition(bool editor, int &braceAtCaret, int &braceOpposite, bool sloppy) {
	//int maskStyle = (1 << wEditor.Call(SCI_GETSTYLEBITSNEEDED)) - 1;
	bool isInside = false;
//!	GUI::ScintillaWindow &win = editor ? wEditor : wOutput;			  !!!TODO!!! -
	GUI::ScintillaWindow &win = editor ? reinterpret_cast<GUI::ScintillaWindow&>(wEditor) : wOutput; //!-change-[OnSendEditor]

	int mainSel = win.Send(SCI_GETMAINSELECTION, 0, 0);
	if (win.Send(SCI_GETSELECTIONNCARETVIRTUALSPACE, mainSel, 0) > 0)
		return false;

	int bracesStyleCheck = editor ? bracesStyle : 0; 
	int caretPos = win.Send(SCI_GETCURRENTPOS, 0, 0);
	braceAtCaret = -1;
	braceOpposite = -1;
	char charBefore = '\0';
	char styleBefore = '\0';
	int lengthDoc = win.Send(SCI_GETLENGTH, 0, 0);
	TextReader acc(win);
	if ((lengthDoc > 0) && (caretPos > 0)) {
		// Check to ensure not matching brace that is part of a multibyte character
		if (win.Send(SCI_POSITIONBEFORE, caretPos) == (caretPos - 1)) {
			charBefore = acc[caretPos - 1];
			styleBefore = static_cast<char>(acc.StyleAt(caretPos - 1) ); //& maskStyle
		}
	}
	// Priority goes to character before caret
	if (charBefore && IsBrace(charBefore) &&
	        ((styleBefore == bracesStyleCheck) || (!bracesStyle))) {
		braceAtCaret = caretPos - 1;
	}
	bool colonMode = false;
	if ((lexLanguage == SCLEX_PYTHON) &&
	        (':' == charBefore) && (SCE_P_OPERATOR == styleBefore)) {
		braceAtCaret = caretPos - 1;
		colonMode = true;
	}
	bool isAfter = true;
	if (lengthDoc > 0 && sloppy && (braceAtCaret < 0) && (caretPos < lengthDoc)) {
		// No brace found so check other side
		// Check to ensure not matching brace that is part of a multibyte character
		if (win.Send(SCI_POSITIONAFTER, caretPos) == (caretPos + 1)) {
			char charAfter = acc[caretPos];
			char styleAfter = static_cast<char>(acc.StyleAt(caretPos)); // & maskStyle
			if (charAfter && IsBrace(charAfter) && ((styleAfter == bracesStyleCheck) || (!bracesStyle))) {
				braceAtCaret = caretPos;
				isAfter = false;
			}
			if ((lexLanguage == SCLEX_PYTHON) &&
			        (':' == charAfter) && (SCE_P_OPERATOR == styleAfter)) {
				braceAtCaret = caretPos;
				colonMode = true;
			}
		}
	}
	if (braceAtCaret >= 0) {
		if (colonMode) {
			int lineStart = win.Send(SCI_LINEFROMPOSITION, braceAtCaret);
			int lineMaxSubord = win.Send(SCI_GETLASTCHILD, lineStart, -1);
			braceOpposite = win.Send(SCI_GETLINEENDPOSITION, lineMaxSubord);
		} else {
			braceOpposite = win.Send(SCI_BRACEMATCH, braceAtCaret, 0);
		}
		if (braceOpposite > braceAtCaret) {
			isInside = isAfter;
		} else {
			isInside = !isAfter;
		}
	}
	return isInside;
}

void SciTEBase::BraceMatch(bool editor) {
	if (!bracesCheck)
		return;
	int braceAtCaret = -1;
	int braceOpposite = -1;
	FindMatchingBracePosition(editor, braceAtCaret, braceOpposite, bracesSloppy);
//!	GUI::ScintillaWindow &win = editor ? wEditor : wOutput;		 !!!TODO!!!
	GUI::ScintillaWindow &win = editor ? reinterpret_cast<GUI::ScintillaWindow&>(wEditor) : wOutput; //!-change-[OnSendEditor]
	if ((braceAtCaret != -1) && (braceOpposite == -1)) {
		win.Send(SCI_BRACEBADLIGHT, braceAtCaret, 0);
		wEditor.Call(SCI_SETHIGHLIGHTGUIDE, 0);
	} else {
		char chBrace = 0;
		if (braceAtCaret >= 0)
			chBrace = static_cast<char>(win.Send(
			            SCI_GETCHARAT, braceAtCaret, 0));
		win.Send(SCI_BRACEHIGHLIGHT, braceAtCaret, braceOpposite);
		int columnAtCaret = win.Send(SCI_GETCOLUMN, braceAtCaret, 0);
		int columnOpposite = win.Send(SCI_GETCOLUMN, braceOpposite, 0);
		if (chBrace == ':') {
			int lineStart = win.Send(SCI_LINEFROMPOSITION, braceAtCaret);
			int indentPos = win.Send(SCI_GETLINEINDENTPOSITION, lineStart, 0);
			int indentPosNext = win.Send(SCI_GETLINEINDENTPOSITION, lineStart + 1, 0);
			columnAtCaret = win.Send(SCI_GETCOLUMN, indentPos, 0);
			int columnAtCaretNext = win.Send(SCI_GETCOLUMN, indentPosNext, 0);
			int indentSize = win.Send(SCI_GETINDENT);
			if (columnAtCaretNext - indentSize > 1)
				columnAtCaret = columnAtCaretNext - indentSize;
			if (columnOpposite == 0)	// If the final line of the structure is empty
				columnOpposite = columnAtCaret;
		} else {
			if (win.Send(SCI_LINEFROMPOSITION, braceAtCaret) == win.Send(SCI_LINEFROMPOSITION, braceOpposite)) {
				// Avoid attempting to draw a highlight guide
				columnAtCaret = 0;
				columnOpposite = 0;
			}
		}

		if (props.GetInt("highlight.indentation.guides"))
			win.Send(SCI_SETHIGHLIGHTGUIDE, Minimum(columnAtCaret, columnOpposite), 0);
	}
}

void SciTEBase::SetWindowName() {
	if (bBlockUIUpdate)
		return;
	if (filePath.IsUntitled()) {
		windowName = extender->LocalizeText("Untitled");
		windowName.insert(0, GUI_TEXT("("));
		windowName += GUI_TEXT(")");
	} else if (props.GetInt("title.full.path") == 2) {
		windowName = FileNameExt().AsInternal();
		windowName += GUI_TEXT(" ");
		windowName += extender->LocalizeText("in");
		windowName += GUI_TEXT(" ");
		windowName += filePath.Directory().AsInternal();
	} else if (props.GetInt("title.full.path") == 1) {
		windowName = filePath.AsInternal();
	} else {
		windowName = FileNameExt().AsInternal();
	}
//!	if (CurrentBuffer()->isDirty)
	if (CurrentBuffer()->DocumentNotSaved()) //!-change-[OpenNonExistent]
		windowName += GUI_TEXT(" * ");
	else
		windowName += GUI_TEXT(" - ");
	windowName += appName;

	if (buffers.length > 1 && props.GetInt("title.show.buffers")) {
		windowName += GUI_TEXT(" [");
		windowName += GUI::StringFromInteger(buffers.Current() + 1);
		windowName += GUI_TEXT(" ");
		windowName += extender->LocalizeText("of");
		windowName += GUI_TEXT(" ");
		windowName += GUI::StringFromInteger(buffers.length);
		windowName += GUI_TEXT("]");
	}
	

	layout.SetTitle(windowName.c_str());
}

Sci_CharacterRange SciTEBase::GetSelection() {
	Sci_CharacterRange crange;
	crange.cpMin = wEditor.Call(SCI_GETSELECTIONSTART);
	crange.cpMax = wEditor.Call(SCI_GETSELECTIONEND);
	return crange;
}

void SciTEBase::SetSelection(int anchor, int currentPos) {
	wEditor.Call(SCI_SETSEL, anchor, currentPos);
}

void SciTEBase::GetCTag(char *sel, int len) {
	int lengthDoc, selStart, selEnd;
	int mustStop = 0;
	char c;
//!	GUI::ScintillaWindow &wCurrent = wOutput.HasFocus() ? wOutput : wEditor;
	GUI::ScintillaWindow &wCurrent = wOutput.HasFocus() ? wOutput : reinterpret_cast<GUI::ScintillaWindow&>(wEditor); //!-change-[OnSendEditor]

	lengthDoc = wCurrent.Call(SCI_GETLENGTH);
	selStart = selEnd = wCurrent.Call(SCI_GETSELECTIONEND);
	TextReader acc(wCurrent);
	while (!mustStop) {
		if (selStart < lengthDoc - 1) {
			selStart++;
			c = acc[selStart];
			if (c == '\r' || c == '\n') {
				mustStop = -1;
			} else if (c == '\t' && ((acc[selStart + 1] == '/' && acc[selStart + 2] == '^') || isdigit(acc[selStart + 1]))) {
				mustStop = 1;
			}
		} else {
			mustStop = -1;
		}
	}
	if (mustStop == 1 && (acc[selStart + 1] == '/' && acc[selStart + 2] == '^')) {	// Found
		selEnd = selStart += 3;
		mustStop = 0;
		while (!mustStop) {
			if (selEnd < lengthDoc - 1) {
				selEnd++;
				c = acc[selEnd];
				if (c == '\r' || c == '\n') {
					mustStop = -1;
				} else if (c == '$' && acc[selEnd + 1] == '/') {
					mustStop = 1;	// Found!
				}

			} else {
				mustStop = -1;
			}
		}
	} else if (mustStop == 1 && isdigit(acc[selStart + 1])) {
		// a Tag can be referenced by line Number also
		selEnd = selStart += 1;
		while (isdigit(acc[selEnd]) && (selEnd < lengthDoc)) {
			selEnd++;
		}
	}

	sel[0] = '\0';
	if ((selStart < selEnd) && ((selEnd - selStart + 1) < len)) {
		GetRange(wCurrent, selStart, selEnd, sel);
	}
}

// Default characters that can appear in a word
bool SciTEBase::iswordcharforsel(char ch) {
	return !strchr("\t\n\r !\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", ch);
}

// Accept slightly more characters than for a word
// Doesn't accept all valid characters, as they are rarely used in source filenames...
// Accept path separators '/' and '\', extension separator '.', and ':', MS drive unit
// separator, and also used for separating the line number for grep. Same for '(' and ')' for cl.
bool SciTEBase::isfilenamecharforsel(char ch) {
	return !strchr("\t\n\r \"$%'*,;<>?[]^`{|}", ch);
}

bool SciTEBase::islexerwordcharforsel(char ch) {
	// If there are no word.characters defined for the current file, fall back on the original function
	if (wordCharacters.length())
		return wordCharacters.contains(ch);
	else
		return iswordcharforsel(ch);
}

SString SciTEBase::GetRange(GUI::ScintillaWindow &win, int selStart, int selEnd) {
	SBuffer sel(selEnd - selStart);
	Sci_TextRange tr;
	tr.chrg.cpMin = selStart;
	tr.chrg.cpMax = selEnd;
	tr.lpstrText = sel.ptr();
	win.SendPointer(SCI_GETTEXTRANGE, 0, &tr);
	return SString(sel);
}

SString SciTEBase::GetRangeInUIEncoding(GUI::ScintillaWindow &win, int selStart, int selEnd) {
	return GetRange(win, selStart, selEnd);
}

SString SciTEBase::GetLine(GUI::ScintillaWindow &win, int line) {
	int lineStart = win.Call(SCI_POSITIONFROMLINE, line);
	int lineEnd = win.Call(SCI_GETLINEENDPOSITION, line);
	return GetRange(win, lineStart, lineEnd);
}

SString SciTEBase::RangeExtendAndGrab(
    GUI::ScintillaWindow &wCurrent,
    int &selStart,
    int &selEnd,
    bool (SciTEBase::*ischarforsel)(char ch),	///< Function returning @c true if the given char. is part of the selection.
    bool stripEol /*=true*/) {

	if (selStart == selEnd && ischarforsel) {
		// Empty range and have a function to extend it
		int lengthDoc = wCurrent.Call(SCI_GETLENGTH);
		TextReader acc(wCurrent);
		// Try and find a word at the caret
		// On the left...
		while ((selStart > 0) && ((this->*ischarforsel)(acc[selStart - 1]))) {
			selStart--;
		}
		// and on the right
		while ((selEnd < lengthDoc) && ((this->*ischarforsel)(acc[selEnd]))) {
			selEnd++;
		}
	}
	SString selected;
	if (selStart != selEnd) {
		selected = GetRangeInUIEncoding(wCurrent, selStart, selEnd);
	}
	if (stripEol) {
		// Change whole line selected but normally end of line characters not wanted.
		// Remove possible terminating \r, \n, or \r\n.
		size_t sellen = selected.length();
		if (sellen >= 2 && (selected[sellen - 2] == '\r' && selected[sellen - 1] == '\n')) {
			selected.remove(sellen - 2, 0);
		} else if (sellen >= 1 && (selected[sellen - 1] == '\r' || selected[sellen - 1] == '\n')) {
			selected.remove(sellen - 1, 0);
		}
	}

	return selected;
}

/**
 * If there is selected text, either in the editor or the output pane,
 * put the selection in the @a sel buffer, up to @a len characters.
 * Otherwise, try and select characters around the caret, as long as they are OK
 * for the @a ischarforsel function.
 * Remove the last two character controls from the result, as they are likely
 * to be CR and/or LF.
 */
SString SciTEBase::SelectionExtend(
    bool (SciTEBase::*ischarforsel)(char ch),	///< Function returning @c true if the given char. is part of the selection.
    bool stripEol /*=true*/) {

//!	GUI::ScintillaWindow &wCurrent = wOutput.HasFocus() ? wOutput : wEditor;		!!!TODO!!!
	GUI::ScintillaWindow &wCurrent = wOutput.HasFocus() ? wOutput : reinterpret_cast<GUI::ScintillaWindow&>(wEditor); //!-change-[OnSendEditor]

	int selStart = wCurrent.Call(SCI_GETSELECTIONSTART);
	int selEnd = wCurrent.Call(SCI_GETSELECTIONEND);
	return RangeExtendAndGrab(wCurrent, selStart, selEnd, ischarforsel, stripEol);
}

void SciTEBase::FindWordAtCaret(int &start, int &end) {

//!	GUI::ScintillaWindow &wCurrent = wOutput.HasFocus() ? wOutput : wEditor;
	GUI::ScintillaWindow &wCurrent = wOutput.HasFocus() ? wOutput : reinterpret_cast<GUI::ScintillaWindow&>(wEditor); //!-change-[OnSendEditor]

	start = wCurrent.Call(SCI_GETSELECTIONSTART);
	end = wCurrent.Call(SCI_GETSELECTIONEND);
	// Call just to update start & end
	RangeExtendAndGrab(wCurrent, start, end, &SciTEBase::iswordcharforsel, false);
}

bool SciTEBase::SelectWordAtCaret() {
	int selStart = 0;
	int selEnd = 0;
	FindWordAtCaret(selStart, selEnd);
	SetSelection(selStart, selEnd);
	return selStart != selEnd;
}

SString SciTEBase::SelectionWord(bool stripEol /*=true*/) {
	return SelectionExtend(&SciTEBase::islexerwordcharforsel, stripEol);
}

SString SciTEBase::SelectionFilename() {
	return SelectionExtend(&SciTEBase::isfilenamecharforsel);
}

void SciTEBase::SelectionIntoProperties() {
	SString currentSelection = SelectionExtend(0, false);
	props.Set("CurrentSelection", currentSelection.c_str());

	SString word = SelectionWord();
	props.Set("CurrentWord", word.c_str());

	int selStart = CallFocused(SCI_GETSELECTIONSTART);
	int selEnd = CallFocused(SCI_GETSELECTIONEND);
	props.SetInteger("SelectionStartLine", CallFocused(SCI_LINEFROMPOSITION, selStart) + 1);
	props.SetInteger("SelectionStartColumn", CallFocused(SCI_GETCOLUMN, selStart) + 1);
	props.SetInteger("SelectionEndLine", CallFocused(SCI_LINEFROMPOSITION, selEnd) + 1);
	props.SetInteger("SelectionEndColumn", CallFocused(SCI_GETCOLUMN, selEnd) + 1);
}

SString SciTEBase::EncodeString(const SString &s) {
	return SString(s);
}

static int UnSlashAsNeeded(SString &s, bool escapes, bool regularExpression) {
	if (escapes) {
		char *sUnslashed = StringDup(s.c_str(), s.length());
		size_t len;
		if (regularExpression) {
			// For regular expressions, the only escape sequences allowed start with \0
			// Other sequences, like \t, are handled by the RE engine.
			len = UnSlashLowOctal(sUnslashed);
		} else {
			// C style escapes allowed
			len = UnSlash(sUnslashed);
		}
		s = sUnslashed;
		delete []sUnslashed;
		return static_cast<int>(len);
	} else {
		return s.length();
	}
}

int SciTEBase::FindInTarget(const char *findWhat, int lenFind, int startPosition, int endPosition) {
	wEditor.Call(SCI_SETTARGETSTART, startPosition);
	wEditor.Call(SCI_SETTARGETEND, endPosition);
	int posFind = wEditor.CallString(SCI_SEARCHINTARGET, lenFind, findWhat);
	while (findInStyle && posFind != -1 && findStyle != wEditor.Call(SCI_GETSTYLEAT, posFind)) {
		if (startPosition < endPosition) {
			wEditor.Call(SCI_SETTARGETSTART, posFind + 1);
			wEditor.Call(SCI_SETTARGETEND, endPosition);
		} else {
			wEditor.Call(SCI_SETTARGETSTART, startPosition);
			wEditor.Call(SCI_SETTARGETEND, posFind + 1);
		}
		posFind = wEditor.CallString(SCI_SEARCHINTARGET, lenFind, findWhat);
	}
	return posFind;
}

bool SciTEBase::FindHasText() const {
	return findWhat[0];
}

void SciTEBase::MoveBack(int distance) {
	Sci_CharacterRange cr = GetSelection();
	SetSelection(cr.cpMin - distance, cr.cpMin - distance);
}

void SciTEBase::ScrollEditorIfNeeded() {
	GUI::Point ptCaret;
	int caret = wEditor.Call(SCI_GETCURRENTPOS);
	ptCaret.x = wEditor.Call(SCI_POINTXFROMPOSITION, 0, caret);
	ptCaret.y = wEditor.Call(SCI_POINTYFROMPOSITION, 0, caret);
	ptCaret.y += wEditor.Call(SCI_TEXTHEIGHT, 0, 0) - 1;

	GUI::Rectangle rcEditor = wEditor.GetClientPosition();
	if (!rcEditor.Contains(ptCaret))
		wEditor.Call(SCI_SCROLLCARET);
}

void SciTEBase::UIClosed() {
}

void SciTEBase::UIHasFocus() {
}

void SciTEBase::OutputAppendString(const char *s, int len) {
	if (len == -1)
		len = static_cast<int>(strlen(s));
	wOutput.Call(SCI_APPENDTEXT, len, reinterpret_cast<sptr_t>(s));
	if (scrollOutput) {
		int line = wOutput.Call(SCI_GETLINECOUNT, 0, 0);
		int lineStart = wOutput.Call(SCI_POSITIONFROMLINE, line);
		wOutput.Call(SCI_GOTOPOS, lineStart);
	}
}

void SciTEBase::OutputAppendStringSynchronised(const char *s, int len) {				 //
	if (len == -1)
		len = static_cast<int>(strlen(s));
	wOutput.Send(SCI_APPENDTEXT, len, reinterpret_cast<sptr_t>(s));
	if (scrollOutput) {
		int line = wOutput.Send(SCI_GETLINECOUNT);
		int lineStart = wOutput.Send(SCI_POSITIONFROMLINE, line);
		wOutput.Send(SCI_GOTOPOS, lineStart);
	}
}

void SciTEBase::FindResAppendString(const char *s, int len) {
	if (len == -1)
		len = static_cast<int>(strlen(s));
	wFindRes.Call(SCI_APPENDTEXT, len, reinterpret_cast<sptr_t>(s));
	if (scrollOutput) {
		int line = wFindRes.Call(SCI_GETLINECOUNT, 0, 0);
		int lineStart = wFindRes.Call(SCI_POSITIONFROMLINE, line);
		wFindRes.Call(SCI_GOTOPOS, lineStart);
	}
}

void SciTEBase::FindResAppendStringSynchronised(const char *s, int len) {				 //		!!!TODO!!! посмотреть, где это должно использоватьс€
	if (len == -1)
		len = static_cast<int>(strlen(s));
	wFindRes.Send(SCI_APPENDTEXT, len, reinterpret_cast<sptr_t>(s));
	if (scrollOutput) {
		int line = wFindRes.Send(SCI_GETLINECOUNT);
		int lineStart = wFindRes.Send(SCI_POSITIONFROMLINE, line);
		wFindRes.Send(SCI_GOTOPOS, lineStart);
	}
}

void SciTEBase::MakeOutputVisible(GUI::ScintillaWindow &wBottom) {
 //TODO
}

void SciTEBase::ClearJobQueue() {
	for (int ic = 0; ic < jobQueue.commandMax; ic++) {
		jobQueue.jobQueue[ic].Clear();
	}
	jobQueue.commandCurrent = 0;
}

void SciTEBase::Execute() {
	props.Set("CurrentMessage", "");
	dirNameForExecute = FilePath();

	int ic;
	parameterisedCommand = "";
	for (ic = 0; ic < jobQueue.commandMax; ic++) {
		if (jobQueue.jobQueue[ic].directory.IsSet()) {
			dirNameForExecute = jobQueue.jobQueue[ic].directory;
		}
	}

	for (ic = 0; ic < jobQueue.commandMax; ic++) {
		jobQueue.jobQueue[ic].command = props.Expand(jobQueue.jobQueue[ic].command.c_str());
	}

	if (jobQueue.ClearBeforeExecute()) {
		wOutput.Send(SCI_CLEARALL);
	}

	wOutput.Call(SCI_MARKERDELETEALL, static_cast<uptr_t>(-1));
	wEditor.Call(SCI_MARKERDELETEALL, markerError);
	// Ensure the output pane is visible
	if (jobQueue.ShowOutputPane()) {
		MakeOutputVisible(wOutput);
	}

	jobQueue.cancelFlag = 0L;
	jobQueue.SetExecuting(true);
	CheckMenus();
	filePath.Directory().SetWorkingDirectory();
	dirNameAtExecute = filePath.Directory();
}

void SciTEBase::BookmarkAdd(int lineno) {
	if (lineno == -1)
		lineno = GetCurrentLineNumber();
	if (!BookmarkPresent(lineno))
		wEditor.Call(SCI_MARKERADD, lineno, markerBookmark);
}

void SciTEBase::BookmarkDelete(int lineno) {
	if (lineno == -1)
		lineno = GetCurrentLineNumber();
	if (BookmarkPresent(lineno))
		wEditor.Call(SCI_MARKERDELETE, lineno, markerBookmark);
}

bool SciTEBase::BookmarkPresent(int lineno) {
	if (lineno == -1)
		lineno = GetCurrentLineNumber();
	int state = wEditor.Call(SCI_MARKERGET, lineno);
	return state & (1 << markerBookmark);
}

void SciTEBase::BookmarkToggle(int lineno) {
	if (lineno == -1)
		lineno = GetCurrentLineNumber();
	if (BookmarkPresent(lineno)) {
		BookmarkDelete(lineno);
	} else {
		BookmarkAdd(lineno);
	}
}

void SciTEBase::BookmarkNext(bool forwardScan, bool select) {
	int lineno = GetCurrentLineNumber();
	int sci_marker = SCI_MARKERNEXT;
	int lineStart = lineno + 1;	//Scan starting from next line
	int lineRetry = 0;				//If not found, try from the beginning
	int anchor = wEditor.Call(SCI_GETANCHOR);
	if (!forwardScan) {
		lineStart = lineno - 1;		//Scan starting from previous line
		lineRetry = wEditor.Call(SCI_GETLINECOUNT, 0, 0L);	//If not found, try from the end
		sci_marker = SCI_MARKERPREVIOUS;
	}
	int nextLine = wEditor.Call(sci_marker, lineStart, 1 << markerBookmark);
	if (nextLine < 0)
		nextLine = wEditor.Call(sci_marker, lineRetry, 1 << markerBookmark);
	if (nextLine < 0 || nextLine == lineno)	// No bookmark (of the given type) or only one, and already on it
		WarnUser(warnNoOtherBookmark);
	else {
		if (extender) extender->OnNavigation("Bkmk");
		GotoLineEnsureVisible(nextLine);
		if (select) {
			wEditor.Call(SCI_SETANCHOR, anchor);
		}
		if (extender) extender->OnNavigation("Bkmk-");
	}
}

//GUI::Rectangle SciTEBase::GetClientRectangle() {
//	RECT rc={0,0,0,0};
//	if (wid)
//		::GetClientRect(reinterpret_cast<HWND>(wid), &rc);
//	return  Rectangle(rc.left, rc.top, rc.right, rc.bottom);
//}

void SciTEBase::Redraw() {
	wSciTE.InvalidateAll();
	wEditorL.InvalidateAll();
	wEditorR.InvalidateAll();
	wOutput.InvalidateAll();
	wFindRes.InvalidateAll();
}

char *SciTEBase::GetNearestWords(const char *wordStart, int searchLen,
		const char *separators, bool ignoreCase /*=false*/, bool exactLen /*=false*/) {
	char *words = 0;
	while (!words && *separators) {
		words = apis.GetNearestWords(wordStart, searchLen, ignoreCase, *separators, exactLen);
		separators++;
	}
	return words;
}


//!-start-[BetterCalltips]
static inline char MakeUpperCase(char ch) {
	if (ch < 'a' || ch > 'z')
		return ch;
	else
		return static_cast<char>(ch - 'a' + 'A');
}

static int CompareNCaseInsensitive(const char *a, const char *b, size_t len) {
	while (*a && *b && len) {
		if (*a != *b) {
			char upperA = MakeUpperCase(*a);
			char upperB = MakeUpperCase(*b);
			if (upperA != upperB)
				return upperA - upperB;
		}
		a++;
		b++;
		len--;
	}
	if (len == 0)
		return 0;
	else
		// Either *a or *b is nul
		return *a - *b;
}


void SciTEBase::EliminateDuplicateWords(char *words) {
	char *firstWord = words;
	char *firstSpace = strchr(firstWord, ' ');
	char *secondWord;
	char *secondSpace;
	int firstLen, secondLen;

	while (firstSpace) {
		firstLen = firstSpace - firstWord;
		secondWord = firstWord + firstLen + 1;
		secondSpace = strchr(secondWord, ' ');

		if (secondSpace)
			secondLen = secondSpace - secondWord;
		else
			secondLen = strlen(secondWord);

		if (firstLen == secondLen &&
		        !strncmp(firstWord, secondWord, firstLen)) {
			strcpy(firstWord, secondWord);
			firstSpace = strchr(firstWord, ' ');
		} else {
			firstWord = secondWord;
			firstSpace = secondSpace;
		}
	}
}

bool SciTEBase::StartAutoComplete() {
	SString line = GetLine();
	int current = GetCaretInLine();

	int startword = current;

	while ((startword > 0) &&
	        (calltipWordCharacters.contains(line[startword - 1]) ||
	         autoCompleteStartCharacters.contains(line[startword - 1]))) {
		startword--;
	}

	SString root = line.substr(startword, current - startword);
	if (apis) {
		char *words = GetNearestWords(root.c_str(), root.length(),
			calltipParametersStart.c_str(), autoCompleteIgnoreCase);
		if (words) {
			EliminateDuplicateWords(words);
			wEditor.Call(SCI_AUTOCSETSEPARATOR, ' ');
			wEditor.CallString(SCI_AUTOCSHOW, root.length(), words);
			delete []words;
		}
	}
	return true;
}

bool SciTEBase::StartAutoCompleteWord(bool onlyOneWord) {
	if (props.GetInt("autocompleteword.automatic.blocked"))
		return true;
	int len;
	// Get needed buffer size
	int curr_position = wEditor.Call(SCI_GETCURRENTPOS);
	int linestart = wEditor.Call(SCI_POSITIONFROMLINE, wEditor.Call(SCI_LINEFROMPOSITION, curr_position));
	len = curr_position - linestart + 1;
	char* text = new char[len];

	Sci_TextRange tr;
	tr.chrg.cpMin = linestart;
	tr.chrg.cpMax = curr_position;
	tr.lpstrText = text;

	wEditor.SendPointer(SCI_GETTEXTRANGE, 0, &tr);

	SString line = text;
	delete[] text;

	if (CurrentBuffer()->unicodeMode) {
		std::string enc = GUI::ConvertFromUTF8(line.c_str(), ::GetACP());
		line = enc.c_str();
	}
	int current = line.length();
	autoCompleteIncremental = (props.GetInt("autocompleteword.incremental") == 1);

	if (!current) wEditor.Call(SCI_AUTOCCANCEL); //!-add-[autocompleteword.incremental]
	int startword = current;
	// Autocompletion of pure numbers is mostly an annoyance
	bool allNumber = true;
	while (startword > 0 && wordCharacters.contains(line[startword - 1])) {
		startword--;
		if (allNumber && (line[startword] < '0' || line[startword] > '9')) {
			allNumber = false;
		}
	}
	//if (startword == current || allNumber) {
	if (current - startword < props.GetInt("autocompleteword.startcount", 1) || allNumber) {
		if (wEditor.Call(SCI_AUTOCACTIVE))
			wEditor.Call(SCI_AUTOCCANCEL);
		return true;
	}
	SString root = line.substr(startword, current - startword);
	//SString rootU = root;
	if (CurrentBuffer()->unicodeMode) {
		std::string enc = GUI::ConvertToUTF8(root.c_str(), ::GetACP());
		root = enc.c_str();
	}
	if (wEditor.Call(SCI_AUTOCACTIVE)) {
		wEditor.CallString(SCI_AUTOCSELECT, NULL, root.c_str());
		return true;
	} 
	int doclen = LengthDocument();
	Sci_TextToFind ft = {{0, 0}, 0, {0, 0}};
	ft.lpstrText = const_cast<char*>(root.c_str());
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = doclen;
	ft.chrgText.cpMin = 0;
	ft.chrgText.cpMax = 0;
	const int flags = SCFIND_WORDSTART | (autoCompleteIgnoreCase ? 0 : SCFIND_MATCHCASE);

	SString templateEnd = "[" + wordCharacters + "]*";
	if (CurrentBuffer()->unicodeMode) {
		std::string enc = GUI::ConvertToUTF8(templateEnd.c_str(), ::GetACP());
		templateEnd = enc.c_str();
	}
	Sci_TextToFind ftE = { { 0, 0 }, 0,{ 0, 0 } };
	ftE.lpstrText = const_cast<char*>(templateEnd.c_str());
	ftE.chrg.cpMin = 0;
	ftE.chrg.cpMax = doclen;
	ftE.chrgText.cpMin = 0;
	ftE.chrgText.cpMax = 0;
	const int flagsE = SCFIND_REGEXP | SCFIND_CXX11REGEX;


	//int posCurrentWord = wEditor.Call(SCI_GETCURRENTPOS) - root.length();
	unsigned int minWordLength = 0;
	unsigned int nwords = 0;

	// wordsNear contains a list of words separated by single spaces and with a space
	// at the start and end. This makes it easy to search for words.
	SString wordsNear;
	wordsNear.setsizegrowth(1000);
	wordsNear.append("\n");

	int posFind = wEditor.CallString(SCI_FINDTEXT, flags, reinterpret_cast<char *>(&ft));
	TextReader acc(wEditor);
	while (posFind >= 0 && posFind < doclen) {	// search all the document
		ftE.chrg.cpMin = ft.chrgText.cpMax;
		//ftE.chrg.cpMax = wEditor.Call(SCI_POSITIONFROMLINE, wEditor.Call(SCI_LINEFROMPOSITION, ft.chrgText.cpMax) + 1);
		int wordEnd = wEditor.CallString(SCI_FINDTEXT, flagsE, reinterpret_cast<char *>(&ftE));
		if (wordEnd < 0)
			break;
		wordEnd = ftE.chrgText.cpMax;
		size_t wordLength = wordEnd - posFind;
		if (wordLength > root.length()) {
			SString word = GetRange(wEditor, posFind, wordEnd);
			//word.substitute("\n", "");
			//word.substitute("\r", "");
			if (word.length() > root.length() && !(posFind < curr_position && curr_position <= wordEnd)) {
				word.insert(0, "\n");
				word.append("\n");
				if (!wordsNear.contains(word.c_str())) {	// add a new entry
					wordsNear += word.c_str() + 1;
					if (minWordLength < wordLength)
						minWordLength = wordLength;

					nwords++;
					if (onlyOneWord && nwords > 1) {
						return true;
					}
				}
			}
		}
		ft.chrg.cpMin = wordEnd;
		posFind = wEditor.CallString(SCI_FINDTEXT, flags, reinterpret_cast<char *>(&ft));
	}

	size_t length = wordsNear.length();
	if ((length > 2) && (!onlyOneWord || (minWordLength > root.length()))) {
		// Protect spaces by temporrily transforming to \001
		wordsNear.substitute(' ', '\001');
		StringList wl(true);
		wl.Set(wordsNear.c_str());
		char *words = wl.GetNearestWords("", 0, autoCompleteIgnoreCase);
		SString acText(words);
		// Use \n as word separator
		char sep = (props.GetInt("autocompleteword.startcount", 1) == 1 && props.GetInt("autocompleteword.useabrev")) ? '\t' : '\n';
		acText.substitute(' ', sep);
		// Return spaces from \001
		acText.substitute('\001', ' ');

		wEditor.Call(SCI_AUTOCSETSEPARATOR, sep);
		wEditor.CallString(SCI_AUTOCSHOW, root.length(), acText.c_str());

		delete []words;
	} else {
		wEditor.Call(SCI_AUTOCCANCEL);
	}
	return true;
}


bool SciTEBase::StartBlockComment() {
	SString fileNameForExtension = ExtensionFileName();
	SString lexerName = props.GetNewExpand("lexer.", fileNameForExtension.c_str());
	SString base("comment.block.");
	SString comment_at_line_start("comment.block.at.line.start.");
	base += lexerName;
	comment_at_line_start += lexerName;
	bool placeCommentsAtLineStart = props.GetInt(comment_at_line_start.c_str()) != 0;

	SString comment = props.Get(base.c_str());
	if (comment == "") { // user friendly error message box
		GUI::gui_string sBase = GUI::StringFromUTF8(base.c_str());
		extender->HildiAlarm("Block comment variable '%1' is not defined in SciTE *.properties!",
			MB_OK | MB_ICONWARNING, sBase.c_str());
		return true;
	}
	SString long_comment = comment;
	long_comment.append(" ");
	int selectionStart = wEditor.Call(SCI_GETSELECTIONSTART);
	int selectionEnd = wEditor.Call(SCI_GETSELECTIONEND);
	int caretPosition = wEditor.Call(SCI_GETCURRENTPOS);
	// checking if caret is located in _beginning_ of selected block
	bool move_caret = caretPosition < selectionEnd;
	int selStartLine = wEditor.Call(SCI_LINEFROMPOSITION, selectionStart);
	int selEndLine = wEditor.Call(SCI_LINEFROMPOSITION, selectionEnd);
	int lines = selEndLine - selStartLine;
	int firstSelLineStart = wEditor.Call(SCI_POSITIONFROMLINE, selStartLine);
	// "caret return" is part of the last selected line
	if ((lines > 0) &&
	        (selectionEnd == wEditor.Call(SCI_POSITIONFROMLINE, selEndLine)))
		selEndLine--;
	wEditor.Call(SCI_BEGINUNDOACTION);
	for (int i = selStartLine; i <= selEndLine; i++) {
		int lineStart = wEditor.Call(SCI_POSITIONFROMLINE, i);
		int lineIndent = lineStart;
		int lineEnd = wEditor.Call(SCI_GETLINEENDPOSITION, i);
		if (!placeCommentsAtLineStart) {
			lineIndent = GetLineIndentPosition(i);
		}
		SString linebuf = GetRange(wEditor, lineIndent, lineEnd);
		// empty lines are not commented
		if (linebuf.length() < 1)
			continue;
		if (linebuf.startswith(comment.c_str())) {
			int commentLength = comment.length();
			if (linebuf.startswith(long_comment.c_str())) {
				// Removing comment with space after it.
				commentLength = long_comment.length();
			}
			wEditor.Call(SCI_SETSEL, lineIndent, lineIndent + commentLength);
			wEditor.CallString(SCI_REPLACESEL, 0, "");
			if (i == selStartLine) // is this the first selected line?
				selectionStart -= commentLength;
			selectionEnd -= commentLength; // every iteration
			continue;
		}
		if (i == selStartLine) // is this the first selected line?
			selectionStart += long_comment.length();
		selectionEnd += long_comment.length(); // every iteration
		wEditor.CallString(SCI_INSERTTEXT, lineIndent, long_comment.c_str());
	}
	// after uncommenting selection may promote itself to the lines
	// before the first initially selected line;
	// another problem - if only comment symbol was selected;
	if (selectionStart < firstSelLineStart) {
		if (selectionStart >= selectionEnd - (static_cast<int>(long_comment.length()) - 1))
			selectionEnd = firstSelLineStart;
		selectionStart = firstSelLineStart;
	}
	if (move_caret) {
		// moving caret to the beginning of selected block
		wEditor.Call(SCI_GOTOPOS, selectionEnd);
		wEditor.Call(SCI_SETCURRENTPOS, selectionStart);
	} else {
		wEditor.Call(SCI_SETSEL, selectionStart, selectionEnd);
	}
	wEditor.Call(SCI_ENDUNDOACTION);
	return true;
}

static const char *LineEndString(int eolMode) {
	switch (eolMode) {
		case SC_EOL_CRLF:
			return "\r\n";
		case SC_EOL_CR:
			return "\r";
		case SC_EOL_LF:
		default:
			return "\n";
	}
}

bool SciTEBase::StartBoxComment() {
	// Get start/middle/end comment strings from options file(s)
	SString fileNameForExtension = ExtensionFileName();
	SString lexerName = props.GetNewExpand("lexer.", fileNameForExtension.c_str());
	SString start_base("comment.box.start.");
	SString middle_base("comment.box.middle.");
	SString end_base("comment.box.end.");
	SString white_space(" ");
	SString eol(LineEndString(wEditor.Call(SCI_GETEOLMODE)));
	start_base += lexerName;
	middle_base += lexerName;
	end_base += lexerName;
	SString start_comment = props.Get(start_base.c_str());
	SString middle_comment = props.Get(middle_base.c_str());
	SString end_comment = props.Get(end_base.c_str());
	if (start_comment == "" || middle_comment == "" || end_comment == "") {
		GUI::gui_string sStart = GUI::StringFromUTF8(start_base.c_str());
		GUI::gui_string sMiddle = GUI::StringFromUTF8(middle_base.c_str());
		GUI::gui_string sEnd = GUI::StringFromUTF8(end_base.c_str());
		extender->HildiAlarm("Box comment variables '%1', '%2' and '%3' are not defined in SciTE *.properties!",
			MB_OK | MB_ICONWARNING, sStart.c_str(), sMiddle.c_str(), sEnd.c_str());
		return true;
	}

	// Note selection and cursor location so that we can reselect text and reposition cursor after we insert comment strings
	size_t selectionStart = wEditor.Call(SCI_GETSELECTIONSTART);
	size_t selectionEnd = wEditor.Call(SCI_GETSELECTIONEND);
	size_t caretPosition = wEditor.Call(SCI_GETCURRENTPOS);
	bool move_caret = caretPosition < selectionEnd;
	size_t selStartLine = wEditor.Call(SCI_LINEFROMPOSITION, selectionStart);
	size_t selEndLine = wEditor.Call(SCI_LINEFROMPOSITION, selectionEnd);
	size_t lines = selEndLine - selStartLine + 1;

	// If selection ends at start of last selected line, fake it so that selection goes to end of second-last selected line
	if (lines > 1 && selectionEnd == static_cast<size_t>(wEditor.Call(SCI_POSITIONFROMLINE, selEndLine))) {
		selEndLine--;
		lines--;
		selectionEnd = wEditor.Call(SCI_GETLINEENDPOSITION, selEndLine);
	}

	// Pad comment strings with appropriate whitespace, then figure out their lengths (end_comment is a bit special-- see below)
	start_comment += white_space;
	middle_comment += white_space;
	size_t start_comment_length = start_comment.length();
	size_t middle_comment_length = middle_comment.length();
	size_t end_comment_length = end_comment.length();
	size_t whitespace_length = white_space.length();

	// Calculate the length of the longest comment string to be inserted, and allocate a null-terminated char buffer of equal size
	size_t maxCommentLength = start_comment_length;
	if (middle_comment_length > maxCommentLength)
		maxCommentLength = middle_comment_length;
	if (end_comment_length + whitespace_length > maxCommentLength)
		maxCommentLength = end_comment_length + whitespace_length;
	char *tempString = new char[maxCommentLength + 1];

	wEditor.Call(SCI_BEGINUNDOACTION);

	// Insert start_comment if needed
	int lineStart = wEditor.Call(SCI_POSITIONFROMLINE, selStartLine);
	GetRange(wEditor, lineStart, lineStart + start_comment_length, tempString);
	tempString[start_comment_length] = '\0';
	if (start_comment != tempString) {
		wEditor.CallString(SCI_INSERTTEXT, lineStart, start_comment.c_str());
		selectionStart += start_comment_length;
		selectionEnd += start_comment_length;
	}

	if (lines <= 1) {
		// Only a single line was selected, so just append whitespace + end-comment at end of line if needed
		int lineEnd = wEditor.Call(SCI_GETLINEENDPOSITION, selEndLine);
		GetRange(wEditor, lineEnd - end_comment_length, lineEnd, tempString);
		tempString[end_comment_length] = '\0';
		if (end_comment != tempString) {
			end_comment.insert(0, white_space.c_str());
			wEditor.CallString(SCI_INSERTTEXT, lineEnd, end_comment.c_str());
		}
	} else {
		// More than one line selected, so insert middle_comments where needed
		for (size_t i = selStartLine + 1; i < selEndLine; i++) {
			lineStart = wEditor.Call(SCI_POSITIONFROMLINE, i);
			GetRange(wEditor, lineStart, lineStart + middle_comment_length, tempString);
			tempString[middle_comment_length] = '\0';
			if (middle_comment != tempString) {
				wEditor.CallString(SCI_INSERTTEXT, lineStart, middle_comment.c_str());
				selectionEnd += middle_comment_length;
			}
		}

		// If last selected line is not middle-comment or end-comment, we need to insert
		// a middle-comment at the start of last selected line and possibly still insert
		// and end-comment tag after the last line (extra logic is necessary to
		// deal with the case that user selected the end-comment tag)
		lineStart = wEditor.Call(SCI_POSITIONFROMLINE, selEndLine);
		GetRange(wEditor, lineStart, lineStart + end_comment_length, tempString);
		tempString[end_comment_length] = '\0';
		if (end_comment != tempString) {
			GetRange(wEditor, lineStart, lineStart + middle_comment_length, tempString);
			tempString[middle_comment_length] = '\0';
			if (middle_comment != tempString) {
				wEditor.CallString(SCI_INSERTTEXT, lineStart, middle_comment.c_str());
				selectionEnd += middle_comment_length;
			}

			// And since we didn't find the end-comment string yet, we need to check the *next* line
			//  to see if it's necessary to insert an end-comment string and a linefeed there....
			lineStart = wEditor.Call(SCI_POSITIONFROMLINE, selEndLine + 1);
			GetRange(wEditor, lineStart, lineStart + (int) end_comment_length, tempString);
			tempString[end_comment_length] = '\0';
			if (end_comment != tempString) {
				end_comment += eol;
				wEditor.CallString(SCI_INSERTTEXT, lineStart, end_comment.c_str());
			}
		}
	}

	if (move_caret) {
		// moving caret to the beginning of selected block
		wEditor.Call(SCI_GOTOPOS, selectionEnd);
		wEditor.Call(SCI_SETCURRENTPOS, selectionStart);
	} else {
		wEditor.Call(SCI_SETSEL, selectionStart, selectionEnd);
	}

	wEditor.Call(SCI_ENDUNDOACTION);

	delete[] tempString;

	return true;
}

bool SciTEBase::StartStreamComment() {
	SString fileNameForExtension = ExtensionFileName();
	SString lexerName = props.GetNewExpand("lexer.", fileNameForExtension.c_str());
	SString start_base("comment.stream.start.");
	SString end_base("comment.stream.end.");
	SString white_space(" ");
	start_base += lexerName;
	end_base += lexerName;
	SString start_comment = props.Get(start_base.c_str());
	SString end_comment = props.Get(end_base.c_str());
	if (start_comment == "" || end_comment == "") {
		GUI::gui_string sStart = GUI::StringFromUTF8(start_base.c_str());
		GUI::gui_string sEnd = GUI::StringFromUTF8(end_base.c_str());
		extender->HildiAlarm("Stream comment variables '%1' and '%2' are not defined in SciTE *.properties!",
			MB_OK | MB_ICONWARNING, sStart.c_str(), sEnd.c_str());
		return true;
	}
	start_comment += white_space;
	white_space += end_comment;
	end_comment = white_space;
	size_t start_comment_length = start_comment.length();
	size_t selectionStart = wEditor.Call(SCI_GETSELECTIONSTART);
	size_t selectionEnd = wEditor.Call(SCI_GETSELECTIONEND);
	size_t caretPosition = wEditor.Call(SCI_GETCURRENTPOS);
	// checking if caret is located in _beginning_ of selected block
	bool move_caret = caretPosition < selectionEnd;
	// if there is no selection?
	if (selectionEnd - selectionStart <= 0) {
		int selLine = wEditor.Call(SCI_LINEFROMPOSITION, selectionStart);
		int lineIndent = GetLineIndentPosition(selLine);
		int lineEnd = wEditor.Call(SCI_GETLINEENDPOSITION, selLine);
		if (RangeIsAllWhitespace(lineIndent, lineEnd))
			return true; // we are not dealing with empty lines
		char linebuf[1000];
		GetLine(linebuf, sizeof(linebuf));
		int current = GetCaretInLine();
		// checking if we are not inside a word
		if (!wordCharacters.contains(linebuf[current]))
			return true; // caret is located _between_ words
		int startword = current;
		int endword = current;
		int start_counter = 0;
		int end_counter = 0;
		while (startword > 0 && wordCharacters.contains(linebuf[startword - 1])) {
			start_counter++;
			startword--;
		}
		// checking _beginning_ of the word
		if (startword == current)
			return true; // caret is located _before_ a word
		while (linebuf[endword + 1] != '\0' && wordCharacters.contains(linebuf[endword + 1])) {
			end_counter++;
			endword++;
		}
		selectionStart -= start_counter;
		selectionEnd += (end_counter + 1);
	}
	wEditor.Call(SCI_BEGINUNDOACTION);
	wEditor.CallString(SCI_INSERTTEXT, selectionStart, start_comment.c_str());
	selectionEnd += start_comment_length;
	selectionStart += start_comment_length;
	wEditor.CallString(SCI_INSERTTEXT, selectionEnd, end_comment.c_str());
	if (move_caret) {
		// moving caret to the beginning of selected block
		wEditor.Call(SCI_GOTOPOS, selectionEnd);
		wEditor.Call(SCI_SETCURRENTPOS, selectionStart);
	} else {
		wEditor.Call(SCI_SETSEL, selectionStart, selectionEnd);
	}
	wEditor.Call(SCI_ENDUNDOACTION);
	return true;
}

/**
 * Return the length of the given line, not counting the EOL.
 */
int SciTEBase::GetLineLength(int line) {
	return wEditor.Call(SCI_GETLINEENDPOSITION, line) - wEditor.Call(SCI_POSITIONFROMLINE, line);
}

int SciTEBase::GetCurrentLineNumber() {
	return wEditor.Call(SCI_LINEFROMPOSITION,
	        wEditor.Call(SCI_GETCURRENTPOS));
}

int SciTEBase::GetCurrentScrollPosition() {
	int lineDisplayTop = wEditor.Call(SCI_GETFIRSTVISIBLELINE);
	return wEditor.Call(SCI_DOCLINEFROMVISIBLE, lineDisplayTop);
}

/**
 * Set up properties for ReadOnly, EOLMode, BufferLength, NbOfLines, SelLength, SelHeight.
 */
void SciTEBase::SetTextProperties(
    PropSetFile &ps) {			///< Property set to update.

	const int TEMP_LEN = 100;
	char temp[TEMP_LEN];

	std::string ro = GUI::UTF8FromString(extender->LocalizeText("READ"));
	ps.Set("ReadOnly", isReadOnly ? ro.c_str() : "");

	int eolMode = wEditor.Call(SCI_GETEOLMODE);
	ps.Set("EOLMode", eolMode == SC_EOL_CRLF ? "CR+LF" : (eolMode == SC_EOL_LF ? "LF" : "CR"));

	sprintf(temp, "%d", LengthDocument());
	ps.Set("BufferLength", temp);

	ps.SetInteger("NbOfLines", wEditor.Call(SCI_GETLINECOUNT));

	Sci_CharacterRange crange = GetSelection();
	int selFirstLine = wEditor.Call(SCI_LINEFROMPOSITION, crange.cpMin);
	int selLastLine = wEditor.Call(SCI_LINEFROMPOSITION, crange.cpMax);
	if (wEditor.Call(SCI_GETSELECTIONMODE) == SC_SEL_RECTANGLE) {
		long charCount = 0;
		for (int line = selFirstLine; line <= selLastLine; line++) {
			int startPos = wEditor.Call(SCI_GETLINESELSTARTPOSITION, line);
			int endPos = wEditor.Call(SCI_GETLINESELENDPOSITION, line);
			charCount += endPos - startPos;
		}
		sprintf(temp, "%ld", charCount);
	} else {
		sprintf(temp, "%ld", crange.cpMax - crange.cpMin);
	}
	ps.Set("SelLength", temp);
	int caretPos = wEditor.Call(SCI_GETCURRENTPOS);
	int selAnchor = wEditor.Call(SCI_GETANCHOR);
	if (0 == (crange.cpMax - crange.cpMin)) {
		sprintf(temp, "%d", 0);
	} else if (selLastLine == selFirstLine) {
		sprintf(temp, "%d", 1);
	} else if ((wEditor.Call(SCI_GETCOLUMN, caretPos) == 0 && (selAnchor <= caretPos)) ||
	        ((wEditor.Call( SCI_GETCOLUMN, selAnchor) == 0) && (selAnchor > caretPos ))) {
		sprintf(temp, "%d", selLastLine - selFirstLine);
	} else {
		sprintf(temp, "%d", selLastLine - selFirstLine + 1);
	}
	ps.Set("SelHeight", temp);
}

void SciTEBase::SetLineIndentation(int line, int indent) {
	if (indent < 0)
		return;
	Sci_CharacterRange crange = GetSelection();
	int posBefore = GetLineIndentPosition(line);
	wEditor.Call(SCI_SETLINEINDENTATION, line, indent);
	int posAfter = GetLineIndentPosition(line);
	int posDifference = posAfter - posBefore;
	if (posAfter > posBefore) {
		// Move selection on
		if (crange.cpMin >= posBefore) {
			crange.cpMin += posDifference;
		}
		if (crange.cpMax >= posBefore) {
			crange.cpMax += posDifference;
		}
	} else if (posAfter < posBefore) {
		// Move selection back
		if (crange.cpMin >= posAfter) {
			if (crange.cpMin >= posBefore)
				crange.cpMin += posDifference;
			else
				crange.cpMin = posAfter;
		}
		if (crange.cpMax >= posAfter) {
			if (crange.cpMax >= posBefore)
				crange.cpMax += posDifference;
			else
				crange.cpMax = posAfter;
		}
	}
	SetSelection(crange.cpMin, crange.cpMax);
}

int SciTEBase::GetLineIndentation(int line) {
	return wEditor.Call(SCI_GETLINEINDENTATION, line);
}

int SciTEBase::GetLineIndentPosition(int line) {
	return wEditor.Call(SCI_GETLINEINDENTPOSITION, line);
}

static SString CreateIndentation(int indent, int tabSize, bool insertSpaces) {
	SString indentation;
	if (!insertSpaces) {
		while (indent >= tabSize) {
			indentation.append("\t", 1);
			indent -= tabSize;
		}
	}
	while (indent > 0) {
		indentation.append(" ", 1);
		indent--;
	}
	return indentation;
}

void SciTEBase::ConvertIndentation(int tabSize, int useTabs) {
	wEditor.Call(SCI_BEGINUNDOACTION);
	int maxLine = wEditor.Call(SCI_GETLINECOUNT);
	for (int line = 0; line < maxLine; line++) {
		int lineStart = wEditor.Call(SCI_POSITIONFROMLINE, line);
		int indent = GetLineIndentation(line);
		int indentPos = GetLineIndentPosition(line);
		const int maxIndentation = 1000;
		if (indent < maxIndentation) {
			SString indentationNow = GetRange(wEditor, lineStart, indentPos);
			SString indentationWanted = CreateIndentation(indent, tabSize, !useTabs);
			if (indentationNow != indentationWanted) {
				wEditor.Call(SCI_SETTARGETSTART, lineStart);
				wEditor.Call(SCI_SETTARGETEND, indentPos);
				wEditor.CallString(SCI_REPLACETARGET, indentationWanted.length(),
					indentationWanted.c_str());
			}
		}
	}
	wEditor.Call(SCI_ENDUNDOACTION);
}

bool SciTEBase::RangeIsAllWhitespace(int start, int end) {
	TextReader acc(wEditor);
	for (int i = start;i < end;i++) {
		if ((acc[i] != ' ') && (acc[i] != '\t'))
			return false;
	}
	return true;
}

unsigned int SciTEBase::GetLinePartsInStyle(int line, int style1, int style2, SString sv[], int len) {
	for (int i = 0; i < len; i++)
		sv[i] = "";
	TextReader acc(wEditor);
	SString s;
	int part = 0;
	int thisLineStart = wEditor.Call(SCI_POSITIONFROMLINE, line);
	int nextLineStart = wEditor.Call(SCI_POSITIONFROMLINE, line + 1);
	for (int pos = thisLineStart; pos < nextLineStart; pos++) {
		if ((acc.StyleAt(pos) == style1) || (acc.StyleAt(pos) == style2)) {
			char c[2];
			c[0] = acc[pos];
			c[1] = '\0';
			s += c;
		} else if (s.length() > 0) {
			if (part < len) {
				sv[part++] = s;
			}
			s = "";
		}
	}
	if ((s.length() > 0) && (part < len)) {
		sv[part++] = s;
	}
	return part;
}

inline bool IsAlphabetic(unsigned int ch) {
	return ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z'));
}

static bool includes(const StyleAndWords &symbols, const SString &value) {
	if (symbols.words.length() == 0) {
		return false;
	} else if (IsAlphabetic(symbols.words[0])) {
		// Set of symbols separated by spaces
		size_t lenVal = value.length();
		const char *symbol = symbols.words.c_str();
		while (symbol) {
			const char *symbolEnd = strchr(symbol, ' ');
			size_t lenSymbol = strlen(symbol);
			if (symbolEnd)
				lenSymbol = symbolEnd - symbol;
			if (lenSymbol == lenVal) {
				if (strncmp(symbol, value.c_str(), lenSymbol) == 0) {
					return true;
				}
			}
			symbol = symbolEnd;
			if (symbol)
				symbol++;
		}
	} else {
		// Set of individual characters. Only one character allowed for now
		char ch = symbols.words[0];
		return strchr(value.c_str(), ch) != 0;
	}
	return false;
}

IndentationStatus SciTEBase::GetIndentState(int line) {
	// C like language indentation defined by braces and keywords
	IndentationStatus indentState = isNone;
	SString controlWords[20];
	unsigned int parts = GetLinePartsInStyle(line, statementIndent.styleNumber,
	        -1, controlWords, ELEMENTS(controlWords));
	unsigned int i;
	for (i = 0; i < parts; i++) {
		if (includes(statementIndent, controlWords[i]))
			indentState = isKeyWordStart;
	}
	parts = GetLinePartsInStyle(line, statementEnd.styleNumber,
	        -1, controlWords, ELEMENTS(controlWords));
	for (i = 0; i < parts; i++) {
		if (includes(statementEnd, controlWords[i]))
			indentState = isNone;
	}
	// Braces override keywords
	SString controlStrings[20];
	parts = GetLinePartsInStyle(line, blockEnd.styleNumber,
	        -1, controlStrings, ELEMENTS(controlStrings));
	for (unsigned int j = 0; j < parts; j++) {
		if (includes(blockEnd, controlStrings[j]))
			indentState = isBlockEnd;
		if (includes(blockStart, controlStrings[j]))
			indentState = isBlockStart;
	}
	return indentState;
}

int SciTEBase::IndentOfBlock(int line) {
	if (line < 0)
		return 0;
	int indentSize = wEditor.Call(SCI_GETINDENT);
	int indentBlock = GetLineIndentation(line);
	int backLine = line;
	IndentationStatus indentState = isNone;
	if (statementIndent.IsEmpty() && blockStart.IsEmpty() && blockEnd.IsEmpty())
		indentState = isBlockStart;	// Don't bother searching backwards

	int lineLimit = line - statementLookback;
	if (lineLimit < 0)
		lineLimit = 0;
	while ((backLine >= lineLimit) && (indentState == 0)) {
		indentState = GetIndentState(backLine);
		if (indentState != 0) {
			indentBlock = GetLineIndentation(backLine);
			if (indentState == isBlockStart) {
				if (!indentOpening)
					indentBlock += indentSize;
			}
			if (indentState == isBlockEnd) {
				if (indentClosing)
					indentBlock -= indentSize;
				if (indentBlock < 0)
					indentBlock = 0;
			}
			if ((indentState == isKeyWordStart) && (backLine == line))
				indentBlock += indentSize;
		}
		backLine--;
	}
	return indentBlock;
}

void SciTEBase::MaintainIndentation(char ch) {
	int eolMode = wEditor.Call(SCI_GETEOLMODE);
	int curLine = GetCurrentLineNumber();
	int lastLine = curLine - 1;

	if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
	        (eolMode == SC_EOL_CR && ch == '\r')) {
		if (props.GetInt("indent.automatic")) {
			while (lastLine >= 0 && GetLineLength(lastLine) == 0)
				lastLine--;
		}
		int indentAmount = 0;
		if (lastLine >= 0) {
			indentAmount = GetLineIndentation(lastLine);
		}
		if (indentAmount > 0) {
			SetLineIndentation(curLine, indentAmount);
		}
	}
}

void SciTEBase::AutomaticIndentation(char ch) {
	Sci_CharacterRange crange = GetSelection();
	int selStart = crange.cpMin;
	int curLine = GetCurrentLineNumber();
	int thisLineStart = wEditor.Call(SCI_POSITIONFROMLINE, curLine);
	int indentSize = wEditor.Call(SCI_GETINDENT);
	int indentBlock = IndentOfBlock(curLine - 1);

	if (blockEnd.IsSingleChar() && ch == blockEnd.words[0]) {	// Dedent maybe
		if (!indentClosing) {
			if (RangeIsAllWhitespace(thisLineStart, selStart - 1)) {
				SetLineIndentation(curLine, indentBlock - indentSize);
			}
		}
	} else if (!blockEnd.IsSingleChar() && (ch == ' ')) {	// Dedent maybe
		if (!indentClosing && (GetIndentState(curLine) == isBlockEnd)) {}
	} else if (blockStart.IsSingleChar() && (ch == blockStart.words[0])) {
		// Dedent maybe if first on line and previous line was starting keyword
		if (!indentOpening && (GetIndentState(curLine - 1) == isKeyWordStart)) {
			if (RangeIsAllWhitespace(thisLineStart, selStart - 1)) {
				SetLineIndentation(curLine, indentBlock - indentSize);
			}
		}
	} else if ((ch == '\r' || ch == '\n') && (selStart == thisLineStart)) {
		if (!indentClosing && !blockEnd.IsSingleChar()) {	// Dedent previous line maybe
			SString controlWords[1];
			if (GetLinePartsInStyle(curLine - 1, blockEnd.styleNumber,
			        -1, controlWords, ELEMENTS(controlWords))) {
				if (includes(blockEnd, controlWords[0])) {
					// Check if first keyword on line is an ender
					SetLineIndentation(curLine - 1, IndentOfBlock(curLine - 2) - indentSize);
					// Recalculate as may have changed previous line
					indentBlock = IndentOfBlock(curLine - 1);
				}
			}
		}

		SString s = "";
		for(int i = 0; i<indentBlock; i++) s.append(" ");
		wEditor.CallString(SCI_REPLACESEL, 0, s.c_str());
		//SetLineIndentation(curLine, indentBlock);
	}
}

/**
 * Upon a character being added, SciTE may decide to perform some action
 * such as displaying a completion list or auto-indentation.
 */

void SciTEBase::CharAdded(char ch) {
	static int prevAutoCHooseSingle = 0;
	if (recording)
		return;	
	Sci_CharacterRange crange = GetSelection();
	int selStart = crange.cpMin;
	int selEnd = crange.cpMax;
	if ((selEnd == selStart) && (selStart > 0)) {
		//if (wEditor.Call(SCI_CALLTIPACTIVE)) {
		//	if (calltipParametersEnd.contains(ch)) {
		//		braceCount--;
		//		if (braceCount < 1)
		//			wEditor.Call(SCI_CALLTIPCANCEL);
		//	} else if (calltipParametersStart.contains(ch)) {
		//		braceCount++;
		//	}
		//} else 
		if (wEditor.Call(SCI_AUTOCACTIVE)) {
			if (calltipParametersStart.contains(ch)) {
				braceCount++;
			} else if (calltipParametersEnd.contains(ch)) {
				braceCount--;
			} else if (!wordCharacters.contains(ch)) {
				if (!props.GetInt("autocompleteword.automatic.blocked")) {
					wEditor.Call(SCI_AUTOCCANCEL);
					if (autoCompleteStartCharacters.contains(ch)) {
						StartAutoComplete();
					}
				}
			} else if (autoCCausedByOnlyOne) {
//!				StartAutoCompleteWord(true);
				StartAutoCompleteWord(!props.GetInt("autocompleteword.incremental")); //!-change-[autocompleteword.incremental]
			}
		} else {
			if (prevAutoCHooseSingle) {
				prevAutoCHooseSingle = 0;
				wEditor.Call(SCI_AUTOCSETCHOOSESINGLE, 1);
			}
			if (calltipParametersStart.contains(ch)) {
				braceCount = 1;
			} else {
				autoCCausedByOnlyOne = false;
				if (indentMaintain)
					MaintainIndentation(ch);
				else if (props.GetInt("indent.automatic"))
					AutomaticIndentation(ch);
				if (autoCompleteStartCharacters.contains(ch)) {
					StartAutoComplete();
				} else if (props.GetInt("autocompleteword.automatic") && wordCharacters.contains(ch)) {
					prevAutoCHooseSingle = wEditor.Call(SCI_AUTOCGETCHOOSESINGLE);
					wEditor.Call(SCI_AUTOCSETCHOOSESINGLE, 0);
					StartAutoCompleteWord(!props.GetInt("autocompleteword.incremental")); 
					autoCCausedByOnlyOne = wEditor.Call(SCI_AUTOCACTIVE);
				}
			}
			if (!wEditor.Call(SCI_AUTOCACTIVE))
				props.SetInteger("autocompleteword.automatic.blocked", 0);
		}
	}
}

/**
 * Upon a character being added to the output, SciTE may decide to perform some action
 * such as displaying a completion list or running a shell command.
 */
void SciTEBase::CharAddedOutput(int ch) {
	if (ch == '\n') {
		NewLineInOutput();
	} else if (ch == '(') {
		// Potential autocompletion of symbols when $( typed
		int selStart = wOutput.Call(SCI_GETSELECTIONSTART);
		if ((selStart > 1) && (wOutput.Call(SCI_GETCHARAT, selStart - 2, 0) == '$')) {
			SString symbols;
			const char *key = NULL;
			const char *val = NULL;
			bool b = props.GetFirst(key, val);
			while (b) {
				symbols.append(key);
				symbols.append(") ");
				b = props.GetNext(key, val);
			}
			StringList symList;
			symList.Set(symbols.c_str());
			char *words = symList.GetNearestWords("", 0, true);
			if (words) {
				wEditor.Call(SCI_AUTOCSETSEPARATOR, ' ');
				wOutput.CallString(SCI_AUTOCSHOW, 0, words);
				delete []words;
			}
		}
	}
}

void SciTEBase::GoMatchingBrace(bool select) {
	int braceAtCaret = -1;
	int braceOpposite = -1;
	bool isInside = FindMatchingBracePosition(true, braceAtCaret, braceOpposite, true);
	// Convert the character positions into caret positions based on whether
	// the caret position was inside or outside the braces.
	if (isInside) {
		if (braceOpposite > braceAtCaret) {
			braceAtCaret++;
		} else {
			braceOpposite++;
		}
	} else {    // Outside
		if (braceOpposite > braceAtCaret) {
			braceOpposite++;
		} else {
			braceAtCaret++;
		}
	}
	if (braceOpposite >= 0) {
		EnsureRangeVisible(braceOpposite, braceOpposite);
		if (select) {
			SetSelection(braceAtCaret, braceOpposite);
		} else {
			SetSelection(braceOpposite, braceOpposite);
		}
	}
}

// Text	ConditionalUp	Ctrl+J	Finds the previous matching preprocessor condition
// Text	ConditionalDown	Ctrl+K	Finds the next matching preprocessor condition
void SciTEBase::GoMatchingPreprocCond(int direction, bool select) {
	int mppcAtCaret = wEditor.Call(SCI_GETCURRENTPOS);
	int mppcMatch = -1;
	int forward = (direction == IDM_NEXTMATCHPPC);
	bool isInside = FindMatchingPreprocCondPosition(forward, mppcAtCaret, mppcMatch);

	if (isInside && mppcMatch >= 0) {
		EnsureRangeVisible(mppcMatch, mppcMatch);
		if (select) {
			// Selection changes the rules a bit...
			int selStart = wEditor.Call(SCI_GETSELECTIONSTART);
			int selEnd = wEditor.Call(SCI_GETSELECTIONEND);
			// pivot isn't the caret position but the opposite (if there is a selection)
			int pivot = (mppcAtCaret == selStart ? selEnd : selStart);
			if (forward) {
				// Caret goes one line beyond the target, to allow selecting the whole line
				int lineNb = wEditor.Call(SCI_LINEFROMPOSITION, mppcMatch);
				mppcMatch = wEditor.Call(SCI_POSITIONFROMLINE, lineNb + 1);
			}
			SetSelection(pivot, mppcMatch);
		} else {
			SetSelection(mppcMatch, mppcMatch);
		}
	} else {
		WarnUser(warnNotFound);
	}
}

void SciTEBase::AddCommand(const SString &cmd, const SString &dir, JobSubsystem jobType, const SString &input, int flags) {
	if (jobQueue.commandCurrent >= jobQueue.commandMax)
		return;
	if (jobQueue.commandCurrent == 0)
		jobQueue.jobUsesOutputPane = false;
	if (cmd.length()) {
		jobQueue.jobQueue[jobQueue.commandCurrent].command = cmd;
		jobQueue.jobQueue[jobQueue.commandCurrent].directory.Set(GUI::StringFromUTF8(dir.c_str()));
		jobQueue.jobQueue[jobQueue.commandCurrent].jobType = jobType;
		jobQueue.jobQueue[jobQueue.commandCurrent].input = input;
		jobQueue.jobQueue[jobQueue.commandCurrent].flags = flags;
		jobQueue.commandCurrent++;
		if (jobType == jobCLI)
			jobQueue.jobUsesOutputPane = true;
		// For jobExtension, the Trace() method shows output pane on demand.
	}
}

int ControlIDOfCommand(unsigned long wParam) {
	return wParam & 0xffff;
}

void WindowSetFocus(GUI::ScintillaWindow &w) {
	w.Send(SCI_GRABFOCUS, 0, 0);
}

void SciTEBase::Close_script() { 
	Close(); 
	WindowSetFocus(wEditor); 
}

void SciTEBase::SetLineNumberWidth(ScintillaWindowEditor *pE) {
	if (!pE)
		pE = &wEditor;

	if (lineNumbers) {
		int pixelWidth = 0;
		DWORD foldMode = props.GetInt("fold.flags");
		if ((foldMode & SC_FOLDFLAG_LEVELNUMBERS) || (foldMode & SC_FOLDFLAG_LINESTATE)) {
			pixelWidth = pE->CallString(SCI_TEXTWIDTH, STYLE_LINENUMBER, " HW9999999");
		} 
		int lineNumWidth = lineNumbersWidth;

		if (lineNumbersExpand) {
			// The margin size will be expanded if the current buffer's maximum
			// line number would overflow the margin.

			int lineCount = pE->Call(SCI_GETLINECOUNT);

			lineNumWidth = 1;
			while (lineCount >= 10) {
				lineCount /= 10;
				++lineNumWidth;
			}

			if (lineNumWidth < lineNumbersWidth) {
				lineNumWidth = lineNumbersWidth;
			}
		}

		// The 4 here allows for spacing: 1 pixel on left and 3 on right.
		pixelWidth += 4 + lineNumWidth * pE->CallString(SCI_TEXTWIDTH, STYLE_LINENUMBER, "9");
		
		pE->Call(SCI_SETMARGINWIDTHN, 0, pixelWidth);
	} else {
		pE->Call(SCI_SETMARGINWIDTHN, 0, 0);
	}
}

void SciTEBase::MenuCommand(int cmdID, int source) {
//!-start-[OnMenuCommand]
	if (extender && OnMenuCommandCallsCount < _MAX_EXTENSION_RECURSIVE_CALL) {
		OnMenuCommandCallsCount++;
		bool result = extender->OnMenuCommand(cmdID,source);
		OnMenuCommandCallsCount--;
		if (result) return;
	}
//!-end-[OnMenuCommand]
	switch (cmdID) {
	case IDM_NEW:
		// For the New command, the "are you sure" question is always asked as this gives
		// an opportunity to abandon the edits made to a file when are.you.sure is turned off.
		if (CanMakeRoom()) {
			New();
			ReadProperties();
			SetIndentSettings();
			SetEol();
			WindowSetFocus(wEditor);
		}
		break;
	case IDM_OPEN:
		// No need to see if can make room as that will occur
		// when doing the opening. Must be done there as user
		// may decide to open multiple files so do not know yet
		// how much room needed.
		OpenDialog(filePath.Directory(), GUI::StringFromUTF8(props.GetExpanded("open.filter").c_str()).c_str());
		WindowSetFocus(wEditor);
		break;
	case IDM_OPENSELECTED:
		break;
	case IDM_REVERT:
		Revert();
		WindowSetFocus(wEditor);
		break;
	case IDM_CLOSE:
		if (SaveIfUnsure() != IDCANCEL) {
			Close();
			WindowSetFocus(wEditor);
		}
		break;
	case IDM_CLOSEALL:
		CloseAllBuffers();
		break;
	case IDM_SAVE:
		Save(true);
		WindowSetFocus(wEditor);
		break;
	case IDM_SAVEALL:
		SaveAllBuffers(false, true);
		break;
	case IDM_SAVEAS:
		SaveAsDialog();
		WindowSetFocus(wEditor);
		break;
	case IDM_SAVEACOPY:
		SaveACopy();
		WindowSetFocus(wEditor);
		break;
	case IDM_COPYPATH:
		break;
	case IDM_SAVEASHTML:
		SaveAsHTML();
		WindowSetFocus(wEditor);
		break;
	case IDM_SAVEASRTF:
		SaveAsRTF();
		WindowSetFocus(wEditor);
		break;
	case IDM_SAVEASPDF:
		SaveAsPDF();
		WindowSetFocus(wEditor);
		break;
	case IDM_SAVEASTEX:
		SaveAsTEX();
		WindowSetFocus(wEditor);
		break;
	case IDM_SAVEASXML:
		SaveAsXML();
		WindowSetFocus(wEditor);
		break;
	case IDM_PRINT:
		Print(true);
		break;
	case IDM_PRINTSETUP:
		PrintSetup();
		break;
	case IDM_QUIT:
		QuitProgram();
		break;
	case IDM_ENCODING_DEFAULT:
	case IDM_ENCODING_UCS2BE:
	case IDM_ENCODING_UCS2LE:
	case IDM_ENCODING_UTF8:
	case IDM_ENCODING_UCOOKIE:
		CurrentBuffer()->unicodeMode = static_cast<UniMode>(cmdID - IDM_ENCODING_DEFAULT);
		if (CurrentBuffer()->unicodeMode != uni8Bit) {
			// Override the code page if Unicode
			codePage = SC_CP_UTF8;
		} else {
			codePage = props.GetInt("code.page");
		}
		props.SetInteger("editor.unicode.mode", CurrentBuffer()->unicodeMode + IDM_ENCODING_DEFAULT); //!-add-[EditorUnicodeMode]
		wEditor.Call(SCI_SETCODEPAGE, codePage);
//-start-[fix_invalid_codepage]
		if ( wEditor.Call(SCI_GETCODEPAGE) != codePage ) {
			codePage = 0;
			wEditor.Call(SCI_SETCODEPAGE, codePage);
		}
//!-end-[fix_invalid_codepage]
		break;

	case IDM_NEXTFILESTACK:
		if (buffers.size > 1 && props.GetInt("buffers.zorder.switching")) {
			NextInStack(); // next most recently selected buffer
			WindowSetFocus(wEditor);
			CheckReload();
			break;
		}
		// else fall through and do NEXTFILE behaviour...
	case IDM_NEXTFILE:
		if (buffers.size > 1) {
			Next(); // Use Next to tabs move left-to-right
			WindowSetFocus(wEditor);
		} else {
			// Not using buffers - switch to next file on MRU
			StackMenuNext();
		}
		break;

	case IDM_PREVFILESTACK:
		if (buffers.size > 1 && props.GetInt("buffers.zorder.switching")) {
			PrevInStack(); // next least recently selected buffer
			WindowSetFocus(wEditor);
			CheckReload();
			break;
		}
		// else fall through and do PREVFILE behaviour...
	case IDM_PREVFILE:
		if (buffers.size > 1) {
			Prev(); // Use Prev to tabs move right-to-left
			WindowSetFocus(wEditor);
		} else {
			// Not using buffers - switch to previous file on MRU
			StackMenuPrev();
		}
		break;

	case IDM_MOVETABRIGHT:
		MoveTabRight();
		WindowSetFocus(wEditor);
		break;

	case IDM_MOVETABLEFT:
		MoveTabLeft();
		WindowSetFocus(wEditor);
		break;

	case IDM_CLONETAB:
		CloneTab();
		break;

	case IDM_CHANGETAB:
		ChangeTabWnd();
		break;

	case IDM_UNDO:
		CallPane(source, SCI_UNDO);
		CheckMenus();
		break;

	case IDM_REDO:
		CallPane(source, SCI_REDO);
		CheckMenus();
		break;

	case IDM_CUT:
		if (CallPane(source, SCI_GETSELECTIONSTART) != CallPane(source, SCI_GETSELECTIONEND)) {
			CallPane(source, SCI_CUT);
		}
		break;
	case IDM_COPY:
		if (CallPane(source, SCI_GETSELECTIONSTART) != CallPane(source, SCI_GETSELECTIONEND)) {
			//fprintf(stderr, "Copy from %d\n", source);
			CallPane(source, SCI_COPY);
		}
		break;
	case IDM_PASTE:
		CallPane(source, SCI_PASTE);
		break;
	case IDM_DUPLICATE:
		CallPane(source, SCI_SELECTIONDUPLICATE);
		break;
	case IDM_PASTEANDDOWN: {
			int pos = CallFocused(SCI_GETCURRENTPOS);
			CallFocused(SCI_PASTE);
			CallFocused(SCI_SETCURRENTPOS, pos);
			CallFocused(SCI_CHARLEFT);
			CallFocused(SCI_LINEDOWN);
		}
		break;
	case IDM_CLEAR:
		CallPane(source, SCI_CLEAR);
		break;
	case IDM_SELECTALL:
		CallPane(source, SCI_SELECTALL);
		break;
	case IDM_COPYASRTF:
		CopyAsRTF();
		break;

	case IDM_FIND:
		break;

	case IDM_FINDNEXT:
		break;

	case IDM_FINDNEXTBACK:
		break;

	case IDM_FINDNEXTSEL:
		break;

	case IDM_FINDNEXTBACKSEL:
		break;

	case IDM_FINDINFILES:
		break;

	case IDM_REPLACE:
		break;

	case IDM_GOTO:
		break;

	case IDM_MATCHBRACE:
		GoMatchingBrace(false);
		break;

	case IDM_SELECTTOBRACE:
		GoMatchingBrace(true);
		break;

	case IDM_PREVMATCHPPC:
		GoMatchingPreprocCond(IDM_PREVMATCHPPC, false);
		break;

	case IDM_SELECTTOPREVMATCHPPC:
		GoMatchingPreprocCond(IDM_PREVMATCHPPC, true);
		break;

	case IDM_NEXTMATCHPPC:
		GoMatchingPreprocCond(IDM_NEXTMATCHPPC, false);
		break;

	case IDM_SELECTTONEXTMATCHPPC:
		GoMatchingPreprocCond(IDM_NEXTMATCHPPC, true);
		break;

	case IDM_COMPLETE:
		autoCCausedByOnlyOne = false;
		StartAutoComplete();
		break;

	case IDM_COMPLETEWORD:
		autoCCausedByOnlyOne = false;
		StartAutoCompleteWord(false);
		if(!props.GetInt("autocompleteword.automatic.blocked") && wEditor.Call(SCI_AUTOCACTIVE))
			props.SetInteger("autocomleted.from.menu", 1);
		break;

	case IDM_ABBREV:
		wEditor.Call(SCI_CANCEL);
		break;

	case IDM_INS_ABBREV:
		wEditor.Call(SCI_CANCEL);
		break;

	case IDM_BLOCK_COMMENT:
		StartBlockComment();
		break;

	case IDM_BOX_COMMENT:
		StartBoxComment();
		break;

	case IDM_STREAM_COMMENT:
		StartStreamComment();
		break;

	case IDM_TOGGLE_FOLDALL:
		FoldAll();
		break;

	case IDM_UPRCASE:
		CallFocused(SCI_UPPERCASE);
		break;

	case IDM_LWRCASE:
		CallFocused(SCI_LOWERCASE);
		break;

	case IDM_JOIN:
		CallFocused(SCI_TARGETFROMSELECTION);
		CallFocused(SCI_LINESJOIN);
		break;

	case IDM_SPLIT:
		CallFocused(SCI_TARGETFROMSELECTION);
		CallFocused(SCI_LINESSPLIT);
		break;

	case IDM_EXPAND:
		wEditor.Call(SCI_TOGGLEFOLD, GetCurrentLineNumber());
		break;

	case IDM_TOGGLE_FOLDRECURSIVE: {
			int line = GetCurrentLineNumber();
			int level = wEditor.Call(SCI_GETFOLDLEVEL, line);
			ToggleFoldRecursive(line, level);
		}
		break;

	case IDM_EXPAND_ENSURECHILDRENVISIBLE: {
			int line = GetCurrentLineNumber();
			int level = wEditor.Call(SCI_GETFOLDLEVEL, line);
			EnsureAllChildrenVisible(line, level);
		}
		break;

	case IDM_LINENUMBERMARGIN:
		lineNumbers = props.GetInt("line.margin.visible");
		SetLineNumberWidth(&wEditorL);
		SetLineNumberWidth(&wEditorR);
		CheckMenus();
		break;

	case IDM_SELMARGIN:
		margin = !margin;
		wEditorL.Call(SCI_SETMARGINWIDTHN, 1, margin ? marginWidth : 0);
		wEditorR.Call(SCI_SETMARGINWIDTHN, 1, margin ? marginWidth : 0);
		CheckMenus();
		break;

	case IDM_FOLDMARGIN:
		foldMargin = !foldMargin;
		wEditorL.Call(SCI_SETMARGINWIDTHN, 2, foldMargin ? foldMarginWidth : 0);
		wEditorR.Call(SCI_SETMARGINWIDTHN, 2, foldMargin ? foldMarginWidth : 0);
		CheckMenus();
		break;

	case IDM_VIEWEOL:
	{
		int vol = !wEditor.Call(SCI_GETVIEWEOL);
		wEditorL.Call(SCI_SETVIEWEOL, vol);
		wEditorR.Call(SCI_SETVIEWEOL, vol);
		CheckMenus();
	}
		break;

	case IDM_VIEWTLBARIUP:
		iuptbVisible = !iuptbVisible;
		props.SetInteger("iuptoolbar.visible", iuptbVisible);
		CheckMenus();
		break;

	case IDM_TOGGLEOUTPUT:
		CheckMenus();
		break;

	case IDM_WRAP:
		wrap = !wrap;
		wEditor.Call(SCI_SETWRAPMODE, wrap ? wrapStyle : SC_WRAP_NONE);
		CheckMenus();
		break;

	case IDM_WRAPOUTPUT:
		wrapOutput = !wrapOutput;
		wOutput.Call(SCI_SETWRAPMODE, wrapOutput ? wrapStyle : SC_WRAP_NONE);
		CheckMenus();
		break;

	case IDM_WRAPFINDRES:
		wrapFindRes = !wrapFindRes;
		wFindRes.Call(SCI_SETWRAPMODE, wrapFindRes ? wrapStyle : SC_WRAP_NONE);
		CheckMenus();
		break;

	case IDM_READONLY:
		isReadOnly = !isReadOnly;
		wEditor.Call(SCI_SETREADONLY, isReadOnly);
		CheckMenus();
		BuffersMenu(); //!-add-[ReadOnlyTabMarker]
		break;

	case IDM_CLEAROUTPUT:
		wOutput.Send(SCI_CLEARALL);
		wOutput.Send(SCI_EMPTYUNDOBUFFER);
		break;

	case IDM_CLEARFINDRES:
		wFindRes.Send(SCI_CLEARALL);
		wFindRes.Send(SCI_EMPTYUNDOBUFFER);
		break;

	case IDM_FINDRESENSUREVISIBLE:
		MakeOutputVisible(wFindRes);
		break;

	case IDM_SWITCHPANE:
		if (wEditor.HasFocus())
			WindowSetFocus(wOutput);
		else
			WindowSetFocus(wEditor);
		break;

	case IDM_EOL_CRLF:
		wEditor.Call(SCI_SETEOLMODE, SC_EOL_CRLF);
		CheckMenus();
		break;

	case IDM_EOL_CR:
		wEditor.Call(SCI_SETEOLMODE, SC_EOL_CR);
		CheckMenus();
		break;
	case IDM_EOL_LF:
		wEditor.Call(SCI_SETEOLMODE, SC_EOL_LF);
		CheckMenus();
		break;
	case IDM_EOL_CONVERT:
		wEditor.Call(SCI_CONVERTEOLS, wEditor.Call(SCI_GETEOLMODE));
		break;

	case IDM_VIEWSPACE:
		viewWs = !wEditor.Call(SCI_GETVIEWWS);
		ViewWhitespace(viewWs);
		CheckMenus();
		Redraw();
		break;
	case IDM_VIEWHISTORYINDICATORS:
	case IDM_VIEWHISTORYMARKERS:
		if(cmdID == IDM_VIEWHISTORYINDICATORS)
			viewHisoryIndicators = ((wEditor.Call(SCI_GETCHANGEHISTORY) & SC_CHANGE_HISTORY_INDICATORS) == 0);
		else
			viewHisoryMarkers = ((wEditor.Call(SCI_GETCHANGEHISTORY) & SC_CHANGE_HISTORY_MARKERS) == 0);
		
		wEditorL.Call(SCI_SETCHANGEHISTORY, (viewHisoryMarkers? (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS) :0)| (viewHisoryIndicators ? (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_INDICATORS) : 0));
		wEditorR.Call(SCI_SETCHANGEHISTORY, (viewHisoryMarkers? (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS) :0)| (viewHisoryIndicators ? (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_INDICATORS) : 0));
		
		CheckMenus();

		Redraw();
		break;
	case IDM_VIEWGUIDES: {
			viewIndent = wEditor.Call(SCI_GETINDENTATIONGUIDES, 0, 0) == 0;
			wEditorL.Call(SCI_SETINDENTATIONGUIDES, viewIndent ? indentExamine : SC_IV_NONE);
			wEditorR.Call(SCI_SETINDENTATIONGUIDES, viewIndent ? indentExamine : SC_IV_NONE);
			CheckMenus();
			Redraw();
		}
		break;

	case IDM_COMPILE: {
			if (SaveIfUnsureForBuilt() != IDCANCEL) {
				SelectionIntoProperties();
				AddCommand(props.GetWild("command.compile.", FileNameExt().AsUTF8().c_str()), "",
				        SubsystemType("command.compile.subsystem."));
				if (jobQueue.commandCurrent > 0)
					Execute();
			}
		}
		break;

	case IDM_BUILD: {
			if (SaveIfUnsureForBuilt() != IDCANCEL) {
				SelectionIntoProperties();
				AddCommand(
				    props.GetWild("command.build.", FileNameExt().AsUTF8().c_str()),
				    props.GetNewExpand("command.build.directory.", FileNameExt().AsUTF8().c_str()),
				    SubsystemType("command.build.subsystem."));
				if (jobQueue.commandCurrent > 0) {
					jobQueue.isBuilding = true;
					Execute();
				}
			}
		}
		break;

	case IDM_GO: {
			if (SaveIfUnsureForBuilt() != IDCANCEL) {
				SelectionIntoProperties();
				long flags = 0;

				if (!jobQueue.isBuilt) {
					SString buildcmd = props.GetNewExpand("command.go.needs.", FileNameExt().AsUTF8().c_str());
					AddCommand(buildcmd, "",
					        SubsystemType("command.go.needs.subsystem."));
					if (buildcmd.length() > 0) {
						jobQueue.isBuilding = true;
						flags |= jobForceQueue;
					}
				}
				AddCommand(props.GetWild("command.go.", FileNameExt().AsUTF8().c_str()), "",
				        SubsystemType("command.go.subsystem."), "", flags);
				if (jobQueue.commandCurrent > 0)
					Execute();
			}
		}
		break;

	case IDM_STOPEXECUTE:
		StopExecute();
		break;

	case IDM_NEXTMSG:
		GoMessage(1, wOutput);
		break;

	case IDM_PREVMSG:
		GoMessage(-1, wOutput);
		break;

	case IDM_OPENLOCALPROPERTIES:
		OpenProperties(IDM_OPENLOCALPROPERTIES);
		WindowSetFocus(wEditor);
		break;

	case IDM_OPENUSERPROPERTIES:
		OpenProperties(IDM_OPENUSERPROPERTIES);
		WindowSetFocus(wEditor);
		break;

	case IDM_OPENGLOBALPROPERTIES:
		OpenProperties(IDM_OPENGLOBALPROPERTIES);
		WindowSetFocus(wEditor);
		break;

	case IDM_OPENABBREVPROPERTIES:
		OpenProperties(IDM_OPENABBREVPROPERTIES);
		WindowSetFocus(wEditor);
		break;

	case IDM_OPENLUAEXTERNALFILE:
		OpenProperties(IDM_OPENLUAEXTERNALFILE);
		WindowSetFocus(wEditor);
		break;

	case IDM_OPENDIRECTORYPROPERTIES:
		OpenProperties(IDM_OPENDIRECTORYPROPERTIES);
		WindowSetFocus(wEditor);
		break;

	case IDM_SRCWIN:
		break;

	case IDM_BOOKMARK_TOGGLE:
		BookmarkToggle();
		break;

	case IDM_BOOKMARK_NEXT:
		BookmarkNext(true);
		break;

	case IDM_BOOKMARK_PREV:
		BookmarkNext(false);
		break;

	case IDM_BOOKMARK_NEXT_SELECT:
		BookmarkNext(true, true);
		break;

	case IDM_BOOKMARK_PREV_SELECT:
		BookmarkNext(false, true);
		break;

	case IDM_BOOKMARK_CLEARALL:
		wEditor.Call(SCI_MARKERDELETEALL, markerBookmark);
		break;

	case IDM_MONOFONT:
		CurrentBuffer()->useMonoFont = !CurrentBuffer()->useMonoFont;
		ReadFontProperties();
		Redraw();
		break;

	case IDM_MACRORECORD:
		SwitchMacroHook(true);
		StartRecordMacro();
		break;

	case IDM_MACROSTOPRECORD:
		SwitchMacroHook(false);
		StopRecordMacro();
		break;

	case IDM_REBOOT:
		extender->DoReboot();
		break;
	case IDM_HELP: {
			SelectionIntoProperties();
			AddCommand(props.GetWild("command.help.", FileNameExt().AsUTF8().c_str()), "",
			        SubsystemType("command.help.subsystem."));
			if (jobQueue.commandCurrent > 0) {
				jobQueue.isBuilding = true;
				Execute();
			}
		}
		break;

	case IDM_HELP_SCITE: {
			SelectionIntoProperties();
			AddCommand(props.Get("command.scite.help"), "",
			        SubsystemType(props.Get("command.scite.help.subsystem")[0]));
			if (jobQueue.commandCurrent > 0) {
				jobQueue.isBuilding = true;
				Execute();
			}
		}
		break;

	default:
		if (cmdID >= IDM_GENERATED && cmdID < IDM_GENERATED + 2000){
			extender->OnGeneratedHotKey(cmdID);
		}
		break;
	}
}

void SciTEBase::FoldChanged(int line, int levelNow, int levelPrev, GUI::ScintillaWindow *w) {
	int parentLine = w->Call(SCI_GETFOLDPARENT, line);
	if (w->Call(SCI_GETFOLDEXPANDED, parentLine) && w->Call(SCI_GETLINEVISIBLE, parentLine)){
		if (levelNow & SC_FOLDLEVELHEADERFLAG) {
			if (!(levelPrev & SC_FOLDLEVELHEADERFLAG)) {
				// Adding a fold point.
				w->Call(SCI_SETFOLDEXPANDED, line, 1);
				Expand(line, true, false, 0, levelPrev);
			}
		}
		else if (levelPrev & SC_FOLDLEVELHEADERFLAG) {
			if (!w->Call(SCI_GETFOLDEXPANDED, line)) {
				// Removing the fold from one that has been contracted so should expand
				// otherwise lines are left invisible with no way to make them visible
				w->Call(SCI_SETFOLDEXPANDED, line, 1);
				Expand(w , line, true, false, 0, levelPrev);
			}
		}
		if (!(levelNow & SC_FOLDLEVELWHITEFLAG) &&
			((levelPrev & SC_FOLDLEVELNUMBERMASK) > (levelNow & SC_FOLDLEVELNUMBERMASK))) {
			// See if should still be hidden
			w->Call(SCI_SHOWLINES, line, line);
		}
	}
}



void SciTEBase::Expand(int &line, bool doExpand, bool force, int visLevels, int level) {
	Expand(&wEditor, line, doExpand, force, visLevels, level);
}
void SciTEBase::Expand(GUI::ScintillaWindow *w, int &line, bool doExpand, bool force, int visLevels, int level) {
	int lineMaxSubord = w->Call(SCI_GETLASTCHILD, line, level & SC_FOLDLEVELNUMBERMASK);
	line++;
	while (line <= lineMaxSubord) {
		if (force) {
			if (visLevels > 0)
				w->Call(SCI_SHOWLINES, line, line);
			else
				w->Call(SCI_HIDELINES, line, line);
		} else {
			if (doExpand)
				w->Call(SCI_SHOWLINES, line, line);
		}
		int levelLine = level;
		if (levelLine == -1)
			levelLine = w->Call(SCI_GETFOLDLEVEL, line);
		if (levelLine & SC_FOLDLEVELHEADERFLAG) {
			if (force) {
				if (visLevels > 1)
					w->Call(SCI_SETFOLDEXPANDED, line, 1);
				else
					w->Call(SCI_SETFOLDEXPANDED, line, 0);
				Expand(line, doExpand, force, visLevels - 1);
			} else {
				if (doExpand) {
					if (!w->Call(SCI_GETFOLDEXPANDED, line))
						w->Call(SCI_SETFOLDEXPANDED, line, 1);
					Expand(line, true, force, visLevels - 1);
				} else {
					Expand(line, false, force, visLevels - 1);
				}
			}
		} else {
			line++;
		}
	}
}

void SciTEBase::FoldAll() {
	wEditor.Call(SCI_COLOURISE, 0, -1);
	int maxLine = wEditor.Call(SCI_GETLINECOUNT);
	bool expanding = true;
	for (int lineSeek = 0; lineSeek < maxLine; lineSeek++) {
		if (wEditor.Call(SCI_GETFOLDLEVEL, lineSeek) & SC_FOLDLEVELHEADERFLAG) {
			expanding = !wEditor.Call(SCI_GETFOLDEXPANDED, lineSeek);
			break;
		}
	}
	for (int line = 0; line < maxLine; line++) {
		int level = wEditor.Call(SCI_GETFOLDLEVEL, line);
		if ((level & SC_FOLDLEVELHEADERFLAG) &&
		        (SC_FOLDLEVELBASE == (level & SC_FOLDLEVELNUMBERMASK))) {
			if (expanding) {
				wEditor.Call(SCI_SETFOLDEXPANDED, line, 1);
				Expand(line, true, false, 0, level);
				line--;
			} else {
				int lineMaxSubord = wEditor.Call(SCI_GETLASTCHILD, line, -1);
				wEditor.Call(SCI_SETFOLDEXPANDED, line, 0);
				if (lineMaxSubord > line)
					wEditor.Call(SCI_HIDELINES, line + 1, lineMaxSubord);
			}
		}
	}
}

void SciTEBase::CollapseOutput()	{
	wFindRes.Call(SCI_COLOURISE, 0, -1);
	int maxLine = wFindRes.Call(SCI_GETLINECOUNT);
	for (int line = 0; line < maxLine; line++) {
		int level = wFindRes.Call(SCI_GETFOLDLEVEL, line);
		if ((level & SC_FOLDLEVELHEADERFLAG) &&
			((SC_FOLDLEVELBASE + 1) == (level & SC_FOLDLEVELNUMBERMASK))) {
				int lineMaxSubord = wFindRes.Call(SCI_GETLASTCHILD, line, -1);
				wFindRes.Call(SCI_SETFOLDEXPANDED, line, 0);
				if (lineMaxSubord > line)
					wFindRes.Call(SCI_HIDELINES, line + 1, lineMaxSubord);
		}
	}
}

void SciTEBase::GotoLineEnsureVisible(int line) {
	wEditor.Call(SCI_ENSUREVISIBLEENFORCEPOLICY, line);
	wEditor.Call(SCI_GOTOLINE, line);
}

void SciTEBase::EnsureRangeVisible(int posStart, int posEnd, bool enforcePolicy) {
	int lineStart = wEditor.Call(SCI_LINEFROMPOSITION, Minimum(posStart, posEnd));
	int lineEnd = wEditor.Call(SCI_LINEFROMPOSITION, Maximum(posStart, posEnd));
	for (int line = lineStart; line <= lineEnd; line++) {
		wEditor.Call(enforcePolicy ? SCI_ENSUREVISIBLEENFORCEPOLICY : SCI_ENSUREVISIBLE, line);
	}
}

bool SciTEBase::MarginClick(int position, int modifiers, GUI::ScintillaWindow *w) {
	int lineClick = w->Call(SCI_LINEFROMPOSITION, position);
	if ((modifiers & SCMOD_SHIFT) && (modifiers & SCMOD_CTRL)) {
		FoldAll();
	} else {
		int levelClick = w->Call(SCI_GETFOLDLEVEL, lineClick);
		if (levelClick & SC_FOLDLEVELHEADERFLAG) {
			if (modifiers & SCMOD_SHIFT) {
				EnsureAllChildrenVisible(lineClick, levelClick);
			} else if (modifiers & SCMOD_CTRL) {
				ToggleFoldRecursive(lineClick, levelClick);
			} else {
				// Toggle this line
				w->Call(SCI_TOGGLEFOLD, lineClick);
			}
		}
	}
	if (!w->Call(SCI_GETLINEVISIBLE, w->Call(SCI_LINEFROMPOSITION, w->Call(SCI_GETSELECTIONSTART))) ||
		!w->Call(SCI_GETLINEVISIBLE, w->Call(SCI_LINEFROMPOSITION, w->Call(SCI_GETSELECTIONEND)))){
		int lineSel;
		for (lineSel = w->Call(SCI_LINEFROMPOSITION, w->Call(SCI_GETSELECTIONSTART));
			lineSel > 0 && !w->Call(SCI_GETLINEVISIBLE, lineSel);
			lineSel = w->Call(SCI_GETFOLDPARENT, lineSel))
		{
		}

		if (w->Call(SCI_GETLINEVISIBLE, lineSel)){
			w->Call(SCI_SETSELECTIONSTART, w->Call(SCI_POSITIONFROMLINE, lineSel));
			w->Call(SCI_SETSELECTIONEND, w->Call(SCI_POSITIONFROMLINE, lineSel));
		}
	}
	return true;
}

void SciTEBase::ToggleFoldRecursive(int line, int level) {
	int baseLevel = wEditor.Call(SCI_GETFOLDLEVEL, line, 0) & SC_FOLDLEVELNUMBERMASK;
	if (baseLevel > SC_FOLDLEVELBASE && (baseLevel & SC_FOLDLEVELHEADERFLAG) == 0) {
		line = wEditor.Call(SCI_GETFOLDPARENT, line, 0);
		level = wEditor.Call(SCI_GETFOLDLEVEL, line, 0);
	}
	
	if (wEditor.Call(SCI_GETFOLDEXPANDED, line)) {
		// Contract this line and all children
		wEditor.Call(SCI_SETFOLDEXPANDED, line, 0);
		Expand(line, false, true, 0, level);
	} else {
		// Expand this line and all children
		wEditor.Call(SCI_SETFOLDEXPANDED, line, 1);
		Expand(line, true, true, 100, level);
	}
}

void SciTEBase::EnsureAllChildrenVisible(int line, int level) {
	// Ensure all children visible
	wEditor.Call(SCI_SETFOLDEXPANDED, line, 1);
	Expand(line, true, true, 100, level);
}

void SciTEBase::RunInConcole(){
	OutputMode prevMode = curOutMode;
	curOutMode = outConsole;
	NewLineInOutput();
	curOutMode = prevMode;
}

void SciTEBase::NewLineInOutput() {
	if (jobQueue.IsExecuting())
		return;
	int line = wOutput.Call(SCI_LINEFROMPOSITION,
	        wOutput.Call(SCI_GETCURRENTPOS)) - 1;
	SString cmd = "\n";
	if(line >= 0)
		cmd = GetLine(wOutput, line);
	if (cmd.startswith("###")) {
		if (cmd.lowercase() == "###c")
			curOutMode = outConsole;
		else if (cmd.lowercase() == "###l")
			curOutMode = outLua;
		else if (cmd.lowercase() == "###i")
			curOutMode = outInterface;
		else if (cmd.lowercase() == "####")
			curOutMode = outNull;
		else if (cmd.lowercase() == "###p")
			curOutMode = outluaPrint;
		else if (cmd.lowercase() == "###?") {
			const char *c;
			switch (curOutMode)
			{
			case SciTEBase::outConsole:
				c = "Command line\n";
				break;
			case SciTEBase::outLua:
				c = "LUA concole\n";
				break;
			case SciTEBase::outluaPrint:
				c = "LUA print\n";
				break;
			case SciTEBase::outInterface:
				c = "HildiM interface\n";
				break;
			case SciTEBase::outNull:
				c = "OFF\n";
				break;
			default:
				break;
			}

			wOutput.Call(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(c));
		}
		return;
	}
	if (cmd == ">") {
		// Search output buffer for previous command
		line--;
		while (line >= 0) {
			cmd = GetLine(wOutput, line);
			if (cmd.startswith(">") && !cmd.startswith(">Exit")) {
				cmd = cmd.substr(1);
				break;
			}
			line--;
		}
	} else if (cmd.startswith(">")) {
		cmd = cmd.substr(1);
	}
	if (curOutMode == outConsole) {
		returnOutputToCommand = false;
		AddCommand(cmd, ".", jobCLI);
		Execute();
	}
	else if (curOutMode == outLua) {
		extender->DoLua(cmd.c_str());
	}
	else if (curOutMode == outluaPrint) {
		cmd = "print(" + cmd + ")";
		extender->DoLua(cmd.c_str());
	}
	else if (curOutMode == outInterface)
	{
		cmd = "IDM_" + cmd.uppercase();
		int icmd = IFaceTable::FindConstant(cmd.c_str());
		if (icmd > 0)
			::PostMessage((HWND)GetID(), WM_COMMAND, IFaceTable::GetConstantValue(icmd), 0);
		else
			wOutput.Call(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>("Invalid HildiM interface command\n"));
	}
	else
	{
		wOutput.Call(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>("Concole OFF. Type ###c for Command Line, ###l for LUA, ###p for print LUA values, ###i for interface\n"));
	}
}

void SciTEBase::Notify(SCNotification *notification) {
	bool handled = false;
	switch (notification->nmhdr.code) {
	case SCN_FOCUSIN:
		if ((notification->nmhdr.idFrom == IDM_SRCWIN) || (notification->nmhdr.idFrom == IDM_COSRCWIN)){
			if (wEditor.GetWindowIdm() != notification->nmhdr.idFrom) {
				wEditor.SwitchTo(notification->nmhdr.idFrom, NULL);
				CheckReload();
			}
		}
		break;
	case SCEN_SETFOCUS:
	case SCEN_KILLFOCUS:
		CheckMenus();
		break;
	case SCN_LISTCONTEXTMENU:	{
		POINT pt;
		::GetCursorPos(&pt);
		if(notification->listType)
			extender->OnContextMenu(pt.x, pt.y, "USERLIST");
		else
			extender->OnContextMenu(pt.x, pt.y, "AUTOCLIST");
	}
		break;

	case SCN_STYLENEEDED: {
			if (extender) {
				// Colourisation may be performed by script
				if ((notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) && (lexLanguage == SCLEX_CONTAINER)) {
					int endStyled = wEditor.Call(SCI_GETENDSTYLED);
					int lineEndStyled = wEditor.Call(SCI_LINEFROMPOSITION, endStyled);
					endStyled = wEditor.Call(SCI_POSITIONFROMLINE, lineEndStyled);
					StyleWriter styler(wEditor);
					int styleStart = 0;
					if (endStyled > 0)
						styleStart = styler.StyleAt(endStyled - 1);
					styler.SetCodePage(codePage);
					extender->OnStyle(endStyled, notification->position - endStyled,
					        styleStart, &styler);
					styler.Flush();
				}
			}
		}
		break;

//!-start-[autocompleteword.incremental]
	case SCN_AUTOCSELECTION:
		autoCompleteIncremental = false;
		extender->OnAutocSelection(notification->listCompletionMethod, notification->position);
		props.SetInteger("autocomleted.from.menu", 0);
		props.SetInteger("autocompleteword.automatic.blocked", 0);
		break;
	case SCN_AUTOCCANCELLED:
		autoCompleteIncremental = false;
		props.SetInteger("autocomleted.from.menu", 0);
		props.SetInteger("autocompleteword.automatic.blocked", 0);
		break;
	case SCN_AUTOCUPDATED:
		if (autoCompleteIncremental)
		{
			StartAutoCompleteWord(false);
		}
		break;
//!-end-[autocompleteword.incremental]

	case SCN_CHARADDED:
		{
			char cc = notification->ch;
			if (extender) {
				if (notification->ch > 255) {
					wchar_t c = static_cast<wchar_t>(notification->ch);
					WideCharToMultiByte(CP_ACP, 0, &c, 1, &cc, 1, NULL, NULL);
					handled = extender->OnChar(cc);
				} else {
					handled = extender->OnChar(static_cast<char>(notification->ch));
				}
			}
			if (!handled) {
				if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
					CharAdded(cc);
				} else if (notification->nmhdr.idFrom == IDM_RUNWIN) {
					CharAddedOutput(notification->ch);
				}
			}
		}
		break;

	case SCN_SAVEPOINTREACHED:
		if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
			if (extender)
				handled = extender->OnSavePointReached();
			if (!handled) {
				CurrentBuffer()->isDirty = false;
				if (CurrentBuffer()->pFriend)
					CurrentBuffer()->Friend()->isDirty = false;
			}
		}
		if (!bBlockUIUpdate){
			CheckMenus();
			SetWindowName();
			BuffersMenu();
		}
		break;
	case SCN_COLORIZED:
		if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
			if (extender)
				handled = extender->OnColorized(notification->wParam, notification->lParam);
		}
		break;
	case SCN_KEYCOMMAND:
		if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
			switch (notification->wParam){
			case SCI_PAGEUP:
			case SCI_PAGEUPRECTEXTEND:
			case SCI_PAGEUPEXTEND:
				handled = extender->OnNavigation(notification->lParam ? "PageUp" : "-PageUp");
				break;
			case SCI_PAGEDOWN:
			case SCI_PAGEDOWNEXTEND:
			case SCI_PAGEDOWNRECTEXTEND:
				handled = extender->OnNavigation(notification->lParam ? "PageDown":"-PageDown");
				break;
			case SCI_DOCUMENTSTART:
			case SCI_DOCUMENTSTARTEXTEND:
				handled = extender->OnNavigation(notification->lParam ? "DocStart":"-DocStart");
				break;
			case SCI_DOCUMENTEND:
			case SCI_DOCUMENTENDEXTEND:
				handled = extender->OnNavigation(notification->lParam ? "DocEnd":"-DocEnd");
				break;
			}
		}
		break;
	case SCN_SAVEPOINTLEFT:
		if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
			if (extender)
				handled = extender->OnSavePointLeft();
			if (!handled) {
				if(wEditor.GetWindowIdm() == notification->nmhdr.idFrom)
					CurrentBuffer()->isDirty = true;
				else {
					FilePath fp = wEditor.GetCoBuffPointer(); 
					if (fp.IsSet()) {
						int coCur = buffers.GetDocumentByName(fp, false, notification->nmhdr.idFrom);
						if (coCur > -1)
							buffers.buffers[coCur].isDirty = true;
						else
							extender->DoLua("print'ERROR: Can not find coEditor fo cet is modify'");
					}
				}
				jobQueue.isBuilt = false;
			}
		}
		CheckMenus();
		SetWindowName();
		BuffersMenu();
		break;

	case SCN_DOUBLECLICK:
		if (extender)
			handled = extender->OnDoubleClick(notification->modifiers);
		if (!handled && notification->nmhdr.idFrom == IDM_RUNWIN) {
			handled = GoMessage(0, wOutput);
			if (handled)
				preserveFocusOnEditor = true;

		}
		break;
//!-end-[OnDoubleClick][GoMessageImprovement][MouseClickHandled]

//!-begin-[OnClick][MouseClickHandled]
	case SCN_CLICK:
		if (extender) {
			handled = extender->OnClick(notification->modifiers);
			if (handled) {
				if (notification->nmhdr.idFrom == IDM_RUNWIN)
					wOutput.Call(SCI_SETMOUSECAPTURE, 0);
				else
					wEditor.Call(SCI_SETMOUSECAPTURE, 0);
			}
		}
		break;
//!-end-[OnClick][MouseClickHandled]

//!-begin-[OnHotSpotReleaseClick][GoMessageImprovement]
	case SCN_HOTSPOTRELEASECLICK:
		if (extender) {
			handled = extender->OnHotSpotReleaseClick(notification->modifiers);
			if (handled) {
				if (notification->nmhdr.idFrom == IDM_RUNWIN)
					wOutput.Call(SCI_SETMOUSECAPTURE, 0);
				else
					wEditor.Call(SCI_SETMOUSECAPTURE, 0);
			}
		}
		break;
//!-end-[OnHotSpotReleaseClick][GoMessageImprovement]

//!-start-[OnMouseButtonUp][GoMessageImprovement]
	case SCN_MOUSEBUTTONUP:
		if (extender)
			extender->OnMouseButtonUp(notification->modifiers);
		if (preserveFocusOnEditor) {
			preserveFocusOnEditor = false;
			WindowSetFocus(wEditor);
		}
		break;
//!-end-[OnMouseButtonUp][GoMessageImprovement]

	case SCN_UPDATEUI:
		if (extender && (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN)) {
			if(buffers.pEditor->GetWindowIdm() == notification->nmhdr.idFrom)
				handled = extender->OnUpdateUI(notification->updated & ((SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT) << 4), notification->updated & SC_UPDATE_SELECTION, notification->updated);
			else
				handled = extender->CoOnUpdateUI(notification->updated & ((SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT) << 4), notification->updated & SC_UPDATE_SELECTION, notification->updated);
			
			if (notification->updated & SC_UPDATE_SELECTION) {
				if (notification->nmhdr.idFrom == IDM_SRCWIN) {
					static int line = -1;
					int l = wEditorL.Call(SCI_VISIBLEFROMDOCLINE, wEditorL.Call(SCI_LINEFROMPOSITION, wEditorL.Call(SCI_GETCURRENTPOS)));
					if (line != l) { 
						line = l;
						layout.childMap["Source"]->setCurLine(l);
					}
				} else {
					static int line = -1;
					int l = wEditorR.Call(SCI_VISIBLEFROMDOCLINE, wEditorR.Call(SCI_LINEFROMPOSITION, wEditorR.Call(SCI_GETCURRENTPOS)));
					if (line != l) { 
						line = l;
						layout.childMap["CoSource"]->setCurLine(l);
					}
				}
			}		
		} else if (extender) {
			extender->PaneOnUpdateUI(notification->updated & ((SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT) << 4), notification->updated & SC_UPDATE_SELECTION, notification->updated);
		}
		if (!handled) {
			BraceMatch(notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN);
			if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
			}
		}

		break;

	case SCN_MODIFIED:

		if (notification->linesAdded && lineNumbers && lineNumbersExpand)
			SetLineNumberWidth();

		if (0 != (notification->modificationType & SC_MOD_CHANGEFOLD) && notification->nmhdr.idFrom != IDM_FINDRESWIN) {
			GUI::ScintillaWindow *w = &wEditor;	 //!TODO! - в будущем могут быть еще варианты

			FoldChanged(notification->line,
			        notification->foldLevelNow, notification->foldLevelPrev, w);
			sptr_t p = w->Call(SCI_LINEFROMPOSITION, w->Call(SCI_GETCURRENTPOS), 0);
			if (p == notification->line + 1 || p == notification->line){
				extender->OnCurrentLineFold(notification->line, notification->nmhdr.idFrom == IDM_SRCWIN ? 0 : 1,
					notification->foldLevelPrev, notification->foldLevelNow);
			}
		}
		if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) {
			if (!bBlockTextChangeNotify && 0 != (notification->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))){
				extender->OnTextChanged(notification->position, notification->nmhdr.idFrom == IDM_SRCWIN ? 0 : 1,
					notification->linesAdded, notification->modificationType);
			}
		}
		break;

	case SCN_MARGINCLICK: {
		GUI::ScintillaWindow *w;
		if (notification->nmhdr.idFrom == IDM_RUNWIN)
			w = &wOutput;
		else if (notification->nmhdr.idFrom == IDM_FINDRESWIN)
			w = &wFindRes;
		else
			w = &wEditor;
		
		if (extender)
			handled = extender->OnMarginClick(notification->margin, notification->modifiers, int(w->Call(SCI_LINEFROMPOSITION, notification->position)), notification->nmhdr.idFrom);

			if (!handled) {
//!-start-[SetBookmark]
				if (notification->margin == 1 && !notification->modifiers) {
					int lineClick = int(wEditor.Call(SCI_LINEFROMPOSITION, notification->position));
					BookmarkToggle(lineClick); 
				}
//!-end-[SetBookmark]
				if (notification->margin == 2) {

					MarginClick(notification->position, notification->modifiers, w);
				}
			}
		}
		break;

	case SCN_NEEDSHOWN: {
							if (notification->nmhdr.idFrom == IDM_SRCWIN || notification->nmhdr.idFrom == IDM_COSRCWIN) EnsureRangeVisible(notification->position, notification->position + notification->length, false);
		}
		break;

	case SCN_USERLISTSELECTION: {
			if (extender && notification->wParam > 2)
				extender->OnUserListSelection(notification->wParam, notification->text, notification->position+1, notification->listCompletionMethod); //!-change-[UserListItemID]
		}
		break;

	case SCN_CALLTIPCLICK: 
		extender->OnCallTipClick(notification->position);
		break;

	case SCN_MACRORECORD:
		RecordMacroCommand(notification);
		break;

	case SCN_URIDROPPED:
		OpenUriList(notification->text);
		break;

	case SCN_DWELLSTART:
		if (extender && (INVALID_POSITION != notification->position)) {
//////ToDelete
			int endWord = notification->position;
			SString message="";
			auto original = _set_se_translator([](unsigned int u, EXCEPTION_POINTERS *pExp) {
				std::string error = "!!!WARNING: SCN_DWELLSTART exeption: ";
				char result[11];
				sprintf_s(result, 11, "0x%08X", u);
				error += result;
				throw std::exception(error.c_str());
			}); 

			try {
				message = RangeExtendAndGrab((notification->nmhdr.idFrom == IDM_SRCWIN ? wEditorL : wEditorR),
					notification->position, endWord, &SciTEBase::iswordcharforsel);
			} catch (std::exception &e) {
				if (strlen(e.what())) {
					std::string warning = "print([[";
					warning += e.what();
					warning += "]])";
					extender->DoLua(warning.c_str());
				}
			}
			_set_se_translator(original);
			if (message.length()) {
				extender->OnDwellStart(notification->position,message.c_str(), ::GetAsyncKeyState(VK_CONTROL));
			}
		}
		break;

	case SCN_DWELLEND:
		if (extender) {
			extender->OnDwellStart(0,"", ::GetAsyncKeyState(VK_CONTROL)); // flags end of calltip
		}
		break;

	case SCN_ZOOM:

		if (notification->nmhdr.idFrom == IDM_RUNWIN){
			props.SetInteger("output.magnification", wOutput.Call(SCI_GETZOOM));
		}
		else if (notification->nmhdr.idFrom == IDM_FINDRESWIN) {
			props.SetInteger("findres.magnification", wFindRes.Call(SCI_GETZOOM));
		}
		else{
			int zoom = wEditor.Call(SCI_GETZOOM);
			if (wEditor.GetWindowIdm() == IDM_SRCWIN)
				props.SetInteger("magnification", zoom);
			else
				props.SetInteger("right.magnification", zoom);
			props.SetInteger("print.magnification", zoom);
			wEditor.Call(SCI_SETPRINTMAGNIFICATION, zoom);
		}
		SetLineNumberWidth();
		break;
	}
}

void SciTEBase::CheckMenus() {

	props.SetInteger("view.whitespace", viewWs);
	props.SetInteger("view.history.indicators", viewHisoryIndicators);
	props.SetInteger("view.history.markers", viewHisoryMarkers);
	props.SetInteger("view.indentation.guides", viewIndent);
	props.SetInteger("line.margin.visible", lineNumbers);
	props.SetInteger("findres.wrap", wrapFindRes);
	props.SetInteger("output.wrap", wrapOutput);
	props.SetInteger("wrap", wrap);
}

void SciTEBase::ContextMenu(GUI::ScintillaWindow &wSource, GUI::Point pt, GUI::Window wCmd, int isMargin) {
	SString mnuFake = "";
	if (wSource.GetID() == wOutput.GetID())
		extender->OnContextMenu(pt.x, pt.y, "OUTPUT");
	else if (wSource.GetID() == wFindRes.GetID() && isMargin)
		extender->OnContextMenu(pt.x, pt.y, "FINDRESMARGIN");
	else if (wSource.GetID() == wFindRes.GetID())
		extender->OnContextMenu(pt.x, pt.y, "FINDRES");
	else if(isMargin)
		extender->OnContextMenu(pt.x, pt.y, "EDITMARGIN");
	else
		extender->OnContextMenu(pt.x, pt.y, "EDITOR");
}

void SciTEBase::UIAvailable() {
	if (extender) {
		FilePath homepath = GetSciteDefaultHome();
		props.Set("SciteDefaultHome", homepath.AsUTF8().c_str());
		homepath = GetSciteUserHome();
		props.Set("SciteUserHome", homepath.AsUTF8().c_str());
		//extender->Initialise(this);
	}
}

void SciTEBase::SetBufferEncoding(int i, int e) {
	buffers.buffers[i]._encoding = e;
	buffers.buffers[i].isDirty = true;
	if (buffers.Current() == i) {
		filePath._encoding = e; wEditor.SetBuffEncoding(e);
		extender->OnSwitchFile(filePath.AsUTF8().c_str());
		layout.OnSwitchFile(buffers.buffers[i].editorSide);
	}
	BuffersMenu();

}

int  SciTEBase::SecondEditorActive() {
	for (int i = 0; i < buffers.length; i++) {
		if (buffers.buffers[i].editorSide == IDM_COSRCWIN)
			return true;
	}
	return false;
}

void SciTEBase::Open_script(const char* path) {
	extender->OnNavigation("Open");
	Open(GUI::StringFromUTF8(path));
	extender->OnNavigation("Open-");
}

void SciTEBase::SavePositions() {
	WINDOWPLACEMENT wp;
	::GetWindowPlacement((HWND)wSciTE.GetID(), &wp);
	props.SetInteger("position.maximize", wp.showCmd == SW_SHOWMAXIMIZED);
	props.SetInteger("position.height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
	props.SetInteger("position.left", wp.rcNormalPosition.left);
	props.SetInteger("position.top", wp.rcNormalPosition.top);
	props.SetInteger("position.width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
}

void SciTEBase::BlockUpdate(int cmd) {
	bBlockUIUpdate = !cmd;
	if (cmd == UPDATE_FORCE) {
		SetWindowName();
		BuffersMenu();
		CheckMenus();
		wEditor.Call(WM_SETREDRAW, 1);
		RedrawWindow((HWND)wEditor.GetID(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		wEditor.coEditor.Call(WM_SETREDRAW, 1);
		RedrawWindow((HWND)wEditor.coEditor.GetID(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

static bool IsSwitchCharacter(GUI::gui_char ch) {
#ifdef __unix__
	return ch == '-';
#else
	return (ch == '-') || (ch == '/');
#endif
}

void SciTEBase::SendOneProperty(const char *kind, const char *key, const char *val) {
	size_t keysize = strlen(kind) + 1 + strlen(key) + 1 + strlen(val) + 1;
	char *m = new char[keysize];
	strcpy(m, kind);
	strcat(m, ":");
	strcat(m, key);
	strcat(m, "=");
	strcat(m, val);
	extender->SendProperty(m);
	delete []m;
}

void SciTEBase::PropertyFromDirector(const char *arg) {
	props.Set(arg);
}

void SciTEBase::WideChrToMyltiBate(SString strIn, SString &strOut){
	GUI::gui_string gFind = GUI::StringFromUTF8(strIn.c_str()) ;

	int sz = ::WideCharToMultiByte(CP_ACP,0,gFind.c_str(),gFind.length(),NULL,0, NULL,NULL);
	LPSTR tmp = new CHAR[sz+1];
	::WideCharToMultiByte(CP_ACP,0,gFind.c_str(),gFind.length(),tmp,sz, NULL,NULL);
	tmp[sz] = '\0';
	strOut = tmp;
	delete[sz + 1] tmp;
}


/**
 * Menu/Toolbar command "Record".
 */
void SciTEBase::StartRecordMacro() {
	recording = true;
	CheckMenus();
	wEditor.Call(SCI_STARTRECORD);
}

bool SciTEBase::RecordMacroCommand(SCNotification *notification) {
	if (extender && static_iOnSendEditorCallsCount == 0) { //!-add-[OnSendEditor]
		char *sParam;
		bool handled;
		sParam = (char*)(notification->lParam);

		int fIdx = IFaceTable::FindFunctionByConstantId(notification->message);
		if (fIdx >= 0) {
			const char* fName = IFaceTable::functions[fIdx].name;
			unsigned int wp = notification->wParam + 1;
			unsigned int lp = notification->lParam + 1;

			if (IFaceTable::functions[fIdx].paramType[0] == IFaceType::iface_void)
				wp = 0;
			if (IFaceTable::functions[fIdx].paramType[1] == IFaceType::iface_void)
				lp = 0;
			if (sParam != NULL) {
				handled = extender->OnMacro(fName, wp, NULL, sParam);
			} else {
				handled = extender->OnMacro(fName, wp, lp, NULL);
			}
			return handled;
		}
	}
	return true;
}

/**
 * Menu/Toolbar command "Stop recording".
 */
void SciTEBase::StopRecordMacro() {
	wEditor.Call(SCI_STOPRECORD);
	recording = false;
	CheckMenus();
}


/*
SciTE received a macro command from director : execute it.
If command needs answer (SCI_GETTEXTLENGTH ...) : give answer to director
*/

static uptr_t ReadNum(const char *&t) {
	const char *argend = strchr(t, ';');	// find ';'
	uptr_t v = 0;
	if (*t)
		v = atoi(t);					// read value
	t = argend + 1;					// update pointer
	return v;						// return value
}

std::vector<GUI::gui_string> ListFromString(const GUI::gui_string &args) {
	// Split on \n
	std::vector<GUI::gui_string> vs;
	GUI::gui_string s;
	bool lastNewLine = false;
	for (size_t i=0; i<args.size(); i++) {
		lastNewLine = args[i] == '\n';
		if (lastNewLine) {
			vs.push_back(s);
			s = GUI::gui_string();
		} else {
			s += args[i];
		}
	}
	if ((s.size() > 0) || lastNewLine) {
		vs.push_back(s);
	}
	return vs;
}

// Implement ExtensionAPI methods
sptr_t SciTEBase::Send(Pane p, unsigned int msg, uptr_t wParam, sptr_t lParam) {
	if (p == paneEditor)
		return wEditor.Call(msg, wParam, lParam);
	else if (p == paneCoEditor)
		return wEditor.coEditor.Call(msg, wParam, lParam);
	else if (p == paneOutput)
		return wOutput.Call(msg, wParam, lParam);
	else
		return wFindRes.Call(msg, wParam, lParam);
}

char *SciTEBase::Line(Pane p, int line, int bNeedEnd) {
	GUI::ScintillaWindow *w;
	switch (p) {
	case paneEditor:
		w = &wEditor;
		break;
	case paneCoEditor:
		w = &(wEditor.coEditor);
		break;
	case paneOutput:
		w = &wOutput;
		break;
	case paneFindRes:
		w = &wFindRes;
		break;
	}
	int len = w->Call(SCI_LINELENGTH, line);
	if (!len)
		return NULL;
	char *s = new char[len + 1];
	w->Call(SCI_GETLINE, line, (sptr_t)s);
	if (!bNeedEnd)
		len = w->Call(SCI_GETLINEENDPOSITION, line) - w->Call(SCI_POSITIONFROMLINE, line);
	s[len] = 0;
	return s;
}

char *SciTEBase::Range(Pane p, int start, int end) {
	int len = end - start;
	char *s = new char[len + 1];
	if (p == paneEditor)
		GetRange(wEditor, start, end, s);
	else  if (p == paneCoEditor)
		GetRange(wEditor.coEditor, start, end, s);
	else  if (p == paneOutput)
		GetRange(wOutput, start, end, s);
	else
		GetRange(wFindRes, start, end, s);
		return s;
}

void SciTEBase::Remove(Pane p, int start, int end) {
	// Should have a scintilla call for this
	if (p == paneEditor) {
		wEditor.Call(SCI_SETSEL, start, end);
		wEditor.Call(SCI_CLEAR);
	}
	else if (p == paneCoEditor)	{
		wEditor.coEditor.Call(SCI_SETSEL, start, end);
		wEditor.coEditor.Call(SCI_CLEAR);
	}
	else  if (p == paneOutput){
		wOutput.Call(SCI_SETSEL, start, end);
		wOutput.Call(SCI_CLEAR);
	}
	else {
		wFindRes.Call(SCI_SETSEL, start, end);
		wFindRes.Call(SCI_CLEAR);
	}

}

void SciTEBase::Insert(Pane p, int pos, const char *s) {
	if (p == paneEditor)
		wEditor.CallString(SCI_INSERTTEXT, pos, s);
	else if (p == paneCoEditor)
		wEditor.coEditor.CallString(SCI_INSERTTEXT, pos, s);
	else if (p == paneOutput)
		wOutput.CallString(SCI_INSERTTEXT, pos, s);
	else
		wFindRes.CallString(SCI_INSERTTEXT, pos, s);
}

static bool makeVisible = false;
void SciTEBase::Trace(const char *s) {
	if (!makeVisible) {
		makeVisible = true;		  //«ащищаемс€ от рекурсивного зацикливани€ - может возникнуть еще одна ошибка при попытке открыти€ окна!
		MakeOutputVisible(wOutput);
		makeVisible = false;
	} 
	
	OutputAppendStringSynchronised(s);
	for (int i = 0; wEditor.Call(SCI_ENDUNDOACTION) > 0;)//
	{
		if(props.GetInt("warning.undoaction") == 0)
			OutputAppendStringSynchronised("\n!!!Warning!!! ENDUNDOACTION by Trace");
	}	
	if(!bFinalise)
		EnsureVisible();

}

char *SciTEBase::Property(const char *key) {
	SString value;
	if (key[strlen(key) - 1] == '$'){
		char *key2 = new char[strlen(key) + 1];
		strcpy(key2, key);
		key2[strlen(key) - 1] = '.';
		value = props.GetNewExpand(key2, ExtensionFileName().c_str());
		if (value.length() == 0){
			key2[strlen(key) - 1] = 0;
			value = props.GetExpanded(key2);
		}
		delete key2;
	}
	else{
		value = props.GetExpanded(key);
	}
	char *retval = new char[value.length() + 1];
	strcpy(retval, value.c_str());
	return retval;
}

void SciTEBase::SetProperty(const char *key, const char *val) {
	SString value = props.GetExpanded(key);
	if (value != val) {
		props.Set(key, val);
		needReadProperties = true;
	}
}

void SciTEBase::UnsetProperty(const char *key) {
	props.Unset(key);
	needReadProperties = true;
}

uptr_t SciTEBase::GetInstance() {
	return 0;
}

void SciTEBase::ShutDown() {
	QuitProgram();
}

void SciTEBase::DoMenuCommand(int cmdID) {
	Command(cmdID, 0);
}
int SciTEBase::ActiveEditor() {
	return buffers.pEditor->GetWindowIdm() == IDM_SRCWIN ? 0 : 1;
}

//!-start-[LocalizationFromLua]
// TODO: переделать всЄ на utf8, это вызываетс€ только из луа 
char *SciTEBase::GetTranslation(const char *s, bool retainIfNotFound) {
#if defined(GTK)
    //TODO: add get translation
    return NULL;
#else
	GUI::gui_string sValue = extender->LocalizeText(s);
	const wchar_t *lpw = sValue.c_str();
	int _convert = (lstrlenW(lpw)+1)*2;
	LPSTR lpa = (LPSTR)malloc(_convert);
	lpa[0] = '\0';
	::WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, _convert, NULL, NULL);
	SString value = lpa;
	free(lpa);
	return value.detach();
#endif
}
//!-end-[LocalizationFromLua]

