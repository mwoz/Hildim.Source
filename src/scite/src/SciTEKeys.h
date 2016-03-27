// SciTE - Scintilla based Text Editor
/** @file SciTEKeys.h
 ** SciTE keyboard shortcut facilities.
 **/
// Copyright 1998-2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCITEKEYS_H
#define SCITEKEYS_H

class SciTEKeys {
public:
	static long ParseKeyCode(const char *mnemonic);
	static bool MatchKeyCode(long parsedKeyCode, int key, int modifiers);
	static long ParseKeyCodeWin(const char *mnemonic);
	static int GetVK(SString sKey);
	static void FillAccel(void *pAcc, const char *mnemonic, int cmd);
};

#endif
