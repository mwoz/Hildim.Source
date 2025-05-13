/*
This file is part of Compare plugin for Notepad++
Copyright (C)2011 Jean-Sébastien Leroy (jean.sebastien.leroy@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NPPHELPERS_H
#define NPPHELPERS_H

#include "Scintilla.h"
//#include "ScintillaMessages.h"
#include "ScintillaTypes.h"
#include "ScintillaStructures.h"
#include "ILoader.h"
#include "ScintillaCall.h"

enum SciCaller {
	sciLeft = 1, sciRight, sciOutput, sciFindres
};

void markTextAsChanged(pSciCaller pc,int start,int length);
void markAsMoved(pSciCaller pc,int line, bool symbol);
void markAsRemoved(pSciCaller pc,int line, bool symbol);
void markAsChanged(pSciCaller pc,int line, bool symbol);
void markAsAdded(pSciCaller pc,int line, bool symbol);
void markAsBlank(pSciCaller pc,int line);
void setStyles(sUserSettings Settings);
void setBlank(pSciCaller pc,int color);
void ready();
void wait();
void setCursor(Scintilla::CursorShape type);
void setTextStyles(sColorSettings Settings);
void setTextStyle(pSciCaller pc, sColorSettings Settings);
void setChangedStyle(pSciCaller pc, sColorSettings Settings);
//void setChangedStyle(HWND window, int color);
void defineSymbol(int type,int symbol);
void defineColor(int type,int color);
//void clearWindow(pSciCaller pc,bool clearUndo);
//void clearUndoBuffer(HWND window);
blankLineList *removeEmptyLines(pSciCaller pc,bool saveList);
int deleteLine(pSciCaller pc, Scintilla::Line);
char **getAllLines(pSciCaller pc,int *length, int **lineNum);
void addBlankLines(pSciCaller pc,blankLineList *list);
void addEmptyLines(pSciCaller pc, int offset, int length, const char *lines);
void resetPrevOffset();
__declspec(dllimport) void* GetCaller(SciCaller c);
#endif
