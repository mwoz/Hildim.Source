// SciTE - Scintilla based Text  Editor
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
#include <cassert>
}

#define ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

inline Sci_Position Minimum(Sci_Position a, Sci_Position b) {
	return (a < b) ? a : b;
}

inline Sci_Position Maximum(Sci_Position a, Sci_Position b) {
	return (a > b) ? a : b;
}
#ifdef _WIN64
inline int Maximum(int a, int b) {
	return (a > b) ? a : b;
}
#endif

inline long LongFromTwoShorts(short a,short b) {
	return (a) | ((b) << 16);
}

typedef long Colour;
inline Scintilla::Colour ColourRGB(unsigned int red, unsigned int green, unsigned int blue) {
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
	Scintilla::Sci_CharacterRange selection;
	Sci_Position scrollPosition;
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

enum class EditorSide : int {
	SrcWin = 350,
	CoSrcWin = 356
};

class BufferList;

class Buffer : public RecentFile {
public:
	EditorSide editorSide;
	bool isDirty;
	bool boundWithFS;
	bool ROMarker; 
	bool useMonoFont;
	Scintilla::IDocumentEditable* doc;
	UniMode unicodeMode;
	time_t fileModTime;
	time_t fileModLastAsk;
	enum { fmNone, fmMarked, fmModified} findMarks;
	SString overrideExtension;	///< User has chosen to use a particular language
	std::vector<Sci_Position> foldState;
	bool pFriend;
	BufferList* pBase = nullptr;
	int FriendIndex();
	Buffer* Friend();
	Buffer() :
			RecentFile(), doc(0), isDirty(false), boundWithFS(true), ROMarker(false), useMonoFont(false),  //!-change-[ReadOnlyTabMarker]
			unicodeMode(uni8Bit), fileModTime(0), fileModLastAsk(0), findMarks(fmNone), editorSide(EditorSide::SrcWin), foldState(), pFriend(false){}

	void Init(BufferList* pB) {
		RecentFile::Init();
		isDirty = false;
		boundWithFS = true;
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
		doc = 0;
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
		bool rez = (isDirty);// || (!IsUntitled() && (fileModTime == 0)));
		if (rez || !pFriend)
			return rez;
		Buffer* b = Friend();
		return b && (b->isDirty);// || (!b->IsUntitled() && (b->fileModTime == 0)));
	}
//!-end-[OpenNonExistent]
};