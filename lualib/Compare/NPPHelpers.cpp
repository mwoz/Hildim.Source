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

#include "Compare.h"
#include "NPPHelpers.h"
#include "Scintilla.h"
#include "icon_add_16.h"
#include "icon_sub_16.h"
#include "icon_warning_16.h"
#include "icon_moved_16.h"

extern NppData nppData;
extern UINT	EOLtype;

void defineColor(int type, int color)
{
	nppData.pMain->MarkerDefine(type, Scintilla::MarkerSymbol::Background);
	nppData.pMain->MarkerSetBack(type, color);
	nppData.pMain->MarkerSetFore(type, 0);

	nppData.pSecond->MarkerDefine(type, Scintilla::MarkerSymbol::Background);
	nppData.pSecond->MarkerSetBack(type, color);
	nppData.pSecond->MarkerSetFore(type, 0);

}
void defineSymbol(int type, int symbol)
{
	::SendMessageA(nppData._scintillaMainHandle, SCI_MARKERDEFINE, type, (LPARAM)symbol);
	::SendMessageA(nppData._scintillaSecondHandle, SCI_MARKERDEFINE, type, (LPARAM)symbol);
}


void setChangedStyle(pSciCaller pc, sColorSettings Settings)
{
	pc->IndicSetStyle(1, Scintilla::IndicatorStyle::RoundBox);
	pc->IndicSetFore(1, Settings.highlight);
	pc->IndicSetAlpha(1, static_cast<Scintilla::Alpha>(Settings.alpha));
}


void setTextStyle(pSciCaller pc, sColorSettings Settings)
{
	setChangedStyle(pc, Settings);
}

void setTextStyles(sColorSettings Settings)
{
	setTextStyle(nppData.pMain, Settings);
	setTextStyle(nppData.pSecond, Settings);
}

void setCursor(Scintilla::CursorShape type)
{
	nppData.pMain->SetCursor(type);
	nppData.pSecond->SetCursor(type);
}

void wait()
{
	setCursor(Scintilla::CursorShape::Wait);
}

void ready()
{
	setCursor(Scintilla::CursorShape::Normal);
}

void setBlank(pSciCaller pc, int color)
{
	pc->MarkerDefine(MARKER_BLANK_LINE, Scintilla::MarkerSymbol::Background);
	pc->MarkerSetBack(MARKER_BLANK_LINE, color);
	pc->MarkerSetFore(MARKER_BLANK_LINE, color);
}

void DefineXpmSymbol(int type, char ** xpm)
{
	//nppData.pMain->MarkerDefinePixmap(type, xpm)
    SendMessage(nppData._scintillaMainHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
    SendMessage(nppData._scintillaSecondHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
}

void setStyles(sUserSettings Settings)
{
    int MarginMask = (1 << MARKER_CHANGED_SYMBOL) |
                     (1 << MARKER_ADDED_SYMBOL) | 
                     (1 << MARKER_REMOVED_SYMBOL) | 
                     (1 << MARKER_MOVED_SYMBOL);
        
    setBlank(nppData.pMain, Settings.ColorSettings.blank);
	setBlank(nppData.pSecond, Settings.ColorSettings.blank);

    defineColor(MARKER_ADDED_LINE,   Settings.ColorSettings.added);
    defineColor(MARKER_CHANGED_LINE, Settings.ColorSettings.changed);
    defineColor(MARKER_MOVED_LINE,   Settings.ColorSettings.moved);
    defineColor(MARKER_REMOVED_LINE, Settings.ColorSettings.deleted);

    DefineXpmSymbol(MARKER_ADDED_SYMBOL,   &icon_add_16_xpm[0]);
    DefineXpmSymbol(MARKER_REMOVED_SYMBOL, &icon_sub_16_xpm[0]);
    DefineXpmSymbol(MARKER_CHANGED_SYMBOL, &icon_warning_16_xpm[0]);
    DefineXpmSymbol(MARKER_MOVED_SYMBOL,   &icon_moved_16_xpm[0]);   

    setTextStyles(Settings.ColorSettings);
}

void markAsBlank(pSciCaller pc,int line)
{
	pc->MarkerAdd(line, MARKER_BLANK_LINE);
}

void markAsAdded(pSciCaller pc,int line, bool symbol)
{
	if(symbol)
		pc->MarkerAdd(line, MARKER_ADDED_SYMBOL);
	pc->MarkerAdd(line, MARKER_ADDED_LINE);
}
void markAsChanged(pSciCaller pc,int line, bool symbol)
{
	if (symbol)
		pc->MarkerAdd(line, MARKER_CHANGED_SYMBOL);
	pc->MarkerAdd(line, MARKER_CHANGED_LINE);
}
void markAsRemoved(pSciCaller pc,int line, bool symbol)
{
	if (symbol)
		pc->MarkerAdd(line, MARKER_REMOVED_SYMBOL);
	pc->MarkerAdd(line, MARKER_REMOVED_LINE);
}

void markAsMoved(pSciCaller pc,int line, bool symbol)
{
	if (symbol)
		pc->MarkerAdd(line, MARKER_MOVED_SYMBOL);
	pc->MarkerAdd(line, MARKER_MOVED_LINE);
}

void markTextAsChanged(pSciCaller pc,int start,int length)
{
	if (length != 0)
		pc->IndicatorFillRange(start, length);
}

char **getAllLines(pSciCaller pc,int *length, int **lineNum){
	Scintilla::Line docLines=pc->LineCount();
	char **lines=new char*[docLines];
	*lineNum=new int[docLines];
	int textCount=0;
	for(Scintilla::Line line=0;line<docLines;line++)
	{
		
		//clearLine(window,line);
		Scintilla::Position lineLength = pc->LineLength(line);
		(*lineNum)[line]=textCount;
		textCount+=lineLength;

		if(lineLength>0){
			lines[line] = new char[lineLength+1];
			pc->GetLine(line, lines[line]);		
			int i=0;
			for(i=0;i<lineLength&& lines[line][i]!='\n' && lines[line][i]!='\r';i++);

			
			lines[line][i]=0;
			
					
		}else{
			lines[line]="";
		}

	}
	*length=docLines;
	return lines;
}
int deleteLine(pSciCaller pc,Scintilla::Line line)
{
	Scintilla::Position posAdd = pc->PositionFromLine(line);
	pc->SetTargetStart(posAdd);

	Scintilla::Line docLength = pc->LineCount() - 1;

	Scintilla::Position length = 0;//::SendMessageA(window, SCI_LINELENGTH, line, 0);
	Scintilla::EndOfLine EOLtype = pc->EOLMode();
	int eolLen = (EOLtype == Scintilla::EndOfLine::CrLf ? 2 : 1);

	int start = line;
	int lines = 0;
	int marker = pc->MarkerGet(line);

	//int blankMask=pow(2.0,blank);
    int blankMask = 1 << MARKER_BLANK_LINE;

	while((marker & blankMask) != 0)
    {
		Scintilla::Position lineLength = pc->LineLength(line);	
		
		//don't delete lines that actually have text in them
		if(line < docLength && lineLength > eolLen)
        {
			//lineLength-=lenEOL[EOLtype];
			break;
		}
        else if(line == docLength && lineLength > 0)
        {			
			break;
		}
		lines++;
		length += lineLength;
		line++;
		marker = pc->MarkerGet(line);
	}
	

	//select the end of the lines, and unmark them so they aren't called for delete again
	pc->SetTargetEnd(posAdd + length);
	for(Scintilla::Line i=start;i<line;i++){
		pc->MarkerDelete(i, MARKER_BLANK_LINE);
	}

	//if we're at the end of the document, we can't delete that line, because it doesn't have any EOL characters to delete
	//, so we have to delete the EOL on the previous line
	if(line>docLength){
		pc->SetTargetStart(posAdd - eolLen);
		length+= eolLen;
	}
	if(length>0){
		pc->MarkerDelete(line, MARKER_BLANK_LINE);
		pc->ReplaceTarget(std::string_view(""));
		return lines;
	}else{
		pc->MarkerDelete(line, MARKER_BLANK_LINE);
		return 0;
	}

}

blankLineList *removeEmptyLines(pSciCaller pc,bool saveList)
{
	pc->SetUndoCollection(false);
	blankLineList *list=NULL;	

    int marker = 1 << MARKER_BLANK_LINE;
    Scintilla::Line line = pc->MarkerNext(0, marker);
	while(line!=-1){
		int lines=deleteLine(pc,line);
		if(lines>0&&saveList){
			//add to list
			blankLineList *newLine=new blankLineList;
			newLine->next=list;
			newLine->line=line;
			newLine->length=lines;
			list=newLine;
		}
		//line=SendMessageA(window, SCI_MARKERNEXT, 0, marker);	
        line = pc->MarkerNext(0, marker);
	}

	//::SendMessage(window, SCI_SETSEL, curPosBeg, curPosEnd);
	pc->SetUndoCollection(true);
	return list;
}

void addBlankLines(pSciCaller pc,blankLineList *list){
	pc->SetUndoCollection(false);
	while(list!=NULL){
		addEmptyLines(pc,list->line,list->length, NULL);
		list=list->next;
	}
	pc->SetUndoCollection(true);
}

static int prev_offset;
void resetPrevOffset() {
	prev_offset = -2;
}

void addEmptyLines(pSciCaller pc, int offset, int length, const char *lines) {
	static int prev_length;
	bool l0l1 = false;
	if (offset == -1) {
		offset = 0;
		prev_offset = -1;
	} else if(offset == 0 && prev_offset == -1) {
		length += prev_length;
		prev_offset = offset;
		l0l1 = true;
	} else {
		prev_offset = offset;
	}
	prev_length = length;

	
	if(l0l1) {
		std::string b = "";
		b += "\n   --------------------\\/under 1-st line\\/";
		b += lines;

		pc->AnnotationSetText(offset, b.c_str());
	}else
		pc->AnnotationSetText(offset, lines);
	pc->AnnotationSetStyle(offset, 0);
}
