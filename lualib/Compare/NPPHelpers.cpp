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

//int MarkerStart   = 0;
//int removed       = MarkerStart + 7; // 24;
//int removedSymbol = MarkerStart + 6; // 23;
//int added         = MarkerStart + 5; // 22;
//int addedSymbol   = MarkerStart + 4; // 21;
//int changed       = MarkerStart + 3; // 20;
//int changedSymbol = MarkerStart + 2; // 19;
//int moved         = MarkerStart + 1; // 18;
//int blank         = MarkerStart;     // 17;


void defineColor(int type, int color)
{
	::SendMessageA(nppData._scintillaMainHandle, SCI_MARKERDEFINE,type, (LPARAM)SC_MARK_BACKGROUND);	
	::SendMessageA(nppData._scintillaMainHandle, SCI_MARKERSETBACK,type, (LPARAM)color);
	::SendMessageA(nppData._scintillaMainHandle, SCI_MARKERSETFORE,type, (LPARAM)0);
	::SendMessageA(nppData._scintillaSecondHandle, SCI_MARKERDEFINE,type, (LPARAM)SC_MARK_BACKGROUND);	
	::SendMessageA(nppData._scintillaSecondHandle, SCI_MARKERSETBACK,type, (LPARAM)color);
	::SendMessageA(nppData._scintillaSecondHandle, SCI_MARKERSETFORE,type, (LPARAM)0);
}
void defineSymbol(int type, int symbol)
{
	::SendMessageA(nppData._scintillaMainHandle, SCI_MARKERDEFINE, type, (LPARAM)symbol);
	::SendMessageA(nppData._scintillaSecondHandle, SCI_MARKERDEFINE, type, (LPARAM)symbol);
}


void setChangedStyle(HWND window, sColorSettings Settings)
{
    ::SendMessageA(window, SCI_INDICSETSTYLE, 1, (LPARAM)INDIC_ROUNDBOX);
    ::SendMessageA(window, SCI_INDICSETFORE, 1, (LPARAM)Settings.highlight);
    ::SendMessageA(window, SCI_INDICSETALPHA, 1, (LPARAM)Settings.alpha);
}


void setTextStyle(HWND window, sColorSettings Settings)
{
	setChangedStyle(window, Settings);
}

void setTextStyles(sColorSettings Settings)
{
	setTextStyle(nppData._scintillaMainHandle, Settings);
	setTextStyle(nppData._scintillaSecondHandle, Settings);
}

void setCursor(int type)
{
	::SendMessageA(nppData._scintillaMainHandle, SCI_SETCURSOR, type, (LPARAM)type);
	::SendMessageA(nppData._scintillaSecondHandle, SCI_SETCURSOR, type, (LPARAM)type);
}

void wait()
{
	setCursor(SC_CURSORWAIT);
}

void ready()
{
	setCursor(SC_CURSORNORMAL);
}

void setBlank(HWND window, int color)
{
	SendMessage(window, SCI_MARKERDEFINE, MARKER_BLANK_LINE, (LPARAM)SC_MARK_BACKGROUND);	
	SendMessage(window, SCI_MARKERSETBACK, MARKER_BLANK_LINE, (LPARAM)color);
	SendMessage(window, SCI_MARKERSETFORE, MARKER_BLANK_LINE, (LPARAM)color);
}

void DefineXpmSymbol(int type, char ** xpm)
{
    SendMessage(nppData._scintillaMainHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
    SendMessage(nppData._scintillaSecondHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
}

void setStyles(sUserSettings Settings)
{
    int MarginMask = (1 << MARKER_CHANGED_SYMBOL) |
                     (1 << MARKER_ADDED_SYMBOL) | 
                     (1 << MARKER_REMOVED_SYMBOL) | 
                     (1 << MARKER_MOVED_SYMBOL);
        
    setBlank(nppData._scintillaMainHandle, Settings.ColorSettings.blank);
	setBlank(nppData._scintillaSecondHandle, Settings.ColorSettings.blank);

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

void markAsBlank(HWND window,int line)
{
	::SendMessageA(window, SCI_MARKERADD, line, MARKER_BLANK_LINE);
}

void markAsAdded(HWND window,int line, bool symbol)
{
	if(symbol)
		::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_ADDED_SYMBOL);
    ::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_ADDED_LINE);
}
void markAsChanged(HWND window,int line, bool symbol)
{
	if (symbol)
		::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_CHANGED_SYMBOL);
    ::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_CHANGED_LINE);
}
void markAsRemoved(HWND window,int line, bool symbol)
{
	if (symbol)
		::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_REMOVED_SYMBOL);
    ::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_REMOVED_LINE);
}

void markAsMoved(HWND window,int line, bool symbol)
{
	if (symbol)
		::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_MOVED_SYMBOL);
	::SendMessageA(window, SCI_MARKERADD, line, (LPARAM)MARKER_MOVED_LINE);
}

void markTextAsChanged(HWND window,int start,int length)
{
	if(length!=0)
		::SendMessageA(window, SCI_INDICATORFILLRANGE, start, length);
}

char **getAllLines(HWND window,int *length, int **lineNum){
	int docLines=SendMessageA(window, SCI_GETLINECOUNT, 0, (LPARAM)0);
	char **lines=new char*[docLines];
	*lineNum=new int[docLines];
	int textCount=0;
	for(int line=0;line<docLines;line++)
	{
		
		//clearLine(window,line);
		int lineLength=SendMessageA(window, SCI_LINELENGTH,line,  (LPARAM)0);
		(*lineNum)[line]=textCount;
		textCount+=lineLength;

		if(lineLength>0){
			lines[line] = new char[lineLength+1];
			::SendMessageA(window, SCI_GETLINE, line, (LPARAM)lines[line]);				
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
int deleteLine(HWND window,int line)
{
	int posAdd = ::SendMessageA(window, SCI_POSITIONFROMLINE, line, 0);
	::SendMessageA(window, SCI_SETTARGETSTART, posAdd, 0);
	int docLength = ::SendMessageA(window, SCI_GETLINECOUNT, 0, 0)-1;
	int length = 0;//::SendMessageA(window, SCI_LINELENGTH, line, 0);
	UINT EOLtype = ::SendMessageA(window,SCI_GETEOLMODE,0,0);

	//::SendMessageA(window,SCI_TARGETFROMSELECTION,0,0);
	int start = line;
	int lines = 0;
	int marker = SendMessageA(window, SCI_MARKERGET, line, 0);
	//int blankMask=pow(2.0,blank);
    int blankMask = 1 << MARKER_BLANK_LINE;

	while((marker & blankMask) != 0)
    {
		unsigned int lineLength = ::SendMessageA(window, SCI_LINELENGTH, line, 0);		
		
		//don't delete lines that actually have text in them
		if(line < docLength && lineLength > lenEOL[EOLtype])
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
		marker = SendMessageA(window, SCI_MARKERGET, line, 0);
	}
	

	//select the end of the lines, and unmark them so they aren't called for delete again
	::SendMessageA(window, SCI_SETTARGETEND, posAdd+length, 0);
	for(int i=start;i<line;i++){
		::SendMessageA(window, SCI_MARKERDELETE, i, MARKER_BLANK_LINE);
	}

	//if we're at the end of the document, we can't delete that line, because it doesn't have any EOL characters to delete
	//, so we have to delete the EOL on the previous line
	if(line>docLength){
		::SendMessageA(window, SCI_SETTARGETSTART, posAdd-(lenEOL[EOLtype]), 0);
		length+=lenEOL[EOLtype];
	}
	if(length>0){
		::SendMessageA(window, SCI_MARKERDELETE, line, MARKER_BLANK_LINE);	
		::SendMessageA(window, SCI_REPLACETARGET, 0, (LPARAM)"");
		return lines;
	}else{
		::SendMessageA(window, SCI_MARKERDELETE, line, MARKER_BLANK_LINE);	
		return 0;
	}

	//::SendMessage(window,SCI_TARGETFROMSELECTION,0,0);
	//::SendMessage(window, SCI_REPLACETARGET, 0, (LPARAM)"");	
}

blankLineList *removeEmptyLines(HWND window,bool saveList)
{
	::SendMessageA(window, SCI_SETUNDOCOLLECTION, FALSE, 0);
	blankLineList *list=NULL;	
	//int curPosBeg = ::SendMessage(window, SCI_GETSELECTIONSTART, 0, 0);
	//int curPosEnd = ::SendMessage(window, SCI_GETSELECTIONEND, 0, 0);
	//double marker=pow(2.0,blank);
	//int line=SendMessageA(window, SCI_MARKERNEXT, 0, (LPARAM)marker);	
    int marker = 1 << MARKER_BLANK_LINE;
    int line=SendMessageA(window, SCI_MARKERNEXT, 0, marker);	
	while(line!=-1){
		int lines=deleteLine(window,line);
		if(lines>0&&saveList){
			//add to list
			blankLineList *newLine=new blankLineList;
			newLine->next=list;
			newLine->line=line;
			newLine->length=lines;
			list=newLine;
		}
		//line=SendMessageA(window, SCI_MARKERNEXT, 0, marker);	
        line=SendMessageA(window, SCI_MARKERNEXT, 0, (LPARAM)marker);	
	}

	//::SendMessage(window, SCI_SETSEL, curPosBeg, curPosEnd);
	::SendMessageA(window, SCI_SETUNDOCOLLECTION, TRUE, 0);
	return list;
}

void clearUndoBuffer(HWND window){
	int modified=::SendMessageA(window, SCI_GETMODIFY, 0, (LPARAM)0);	
		::SendMessageA(window, SCI_EMPTYUNDOBUFFER, 0, (LPARAM)0);
		if(modified){
			::SendMessageA(window, SCI_BEGINUNDOACTION, 0, (LPARAM)0);
			char fake[2];
			fake[1]=0;
			fake[0]=(char)::SendMessageA(window, SCI_GETCHARAT, 0, (LPARAM)0);
			::SendMessageA(window, SCI_SETTARGETSTART, 0, 0);	
			::SendMessageA(window, SCI_SETTARGETEND, 1, 0);
			::SendMessageA(window, SCI_REPLACETARGET, 1, (LPARAM)fake);
			::SendMessageA(window, SCI_ENDUNDOACTION, 0, (LPARAM)0);
		}
}

void clearWindow(HWND window,bool clearUndo)
    {
	//int pos=SendMessageA(window, SCI_MARKERDELETEALL, changed, (LPARAM)changed);	
	
	clearUndo=(removeEmptyLines(window,false)!=NULL);

    ::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_CHANGED_LINE,   (LPARAM)MARKER_CHANGED_LINE);	
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_ADDED_LINE,     (LPARAM)MARKER_ADDED_LINE);
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_REMOVED_LINE,   (LPARAM)MARKER_REMOVED_LINE);
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_MOVED_LINE,     (LPARAM)MARKER_MOVED_LINE);
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_BLANK_LINE,     (LPARAM)MARKER_BLANK_LINE);
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_CHANGED_SYMBOL, (LPARAM)MARKER_CHANGED_SYMBOL);		
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_ADDED_SYMBOL,   (LPARAM)MARKER_ADDED_SYMBOL);
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_REMOVED_SYMBOL, (LPARAM)MARKER_REMOVED_SYMBOL);	
	::SendMessageA(window, SCI_MARKERDELETEALL, MARKER_MOVED_SYMBOL,   (LPARAM)MARKER_MOVED_SYMBOL);	

	//very aggressive approach to removing the indicators
	//clear style, than tell Notepad++ to unfold all lines, which forces it to redo the page style
	::SendMessageA(window, SCI_CLEARDOCUMENTSTYLE, 0, (LPARAM)0);	
	::SendMessageA(window, SCI_GRABFOCUS, 0, (LPARAM)0);
	//::SendMessage(nppData._nppHandle, WM_COMMAND, IDM_VIEW_TOGGLE_UNFOLDALL,0); 

	//int topLine=SendMessageA(window,SCI_GETFIRSTVISIBLELINE,0,0);
	//int linesOnScreen=SendMessageA(window,SCI_LINESONSCREEN,0,0);
	
	//int curPosBeg = ::SendMessageA(window, SCI_GETSELECTIONSTART, 0, 0);
	//int curPosEnd = ::SendMessageA(window, SCI_GETSELECTIONEND, 0, 0);
	::SendMessageA(window, SCN_UPDATEUI, 0, (LPARAM)0);
	//::SendMessageA(window, SCI_SHOWLINES, 0, (LPARAM)1);

	if(clearUndo)
    {
		clearUndoBuffer(window);
	}
	
	/*if(returnToPos){
		SendMessageA(window,SCI_GOTOLINE,topLine,0);
		SendMessageA(window,SCI_GOTOLINE,topLine+linesOnScreen-1,0);
		
		::SendMessageA(window, SCI_SETSEL, curPosBeg, curPosEnd);
	}*/
	
}

void addBlankLines(HWND window,blankLineList *list){
	::SendMessageA(window, SCI_SETUNDOCOLLECTION, FALSE, 0);
	while(list!=NULL){
		addEmptyLines(window,list->line,list->length);
		list=list->next;
	}
	::SendMessageA(window, SCI_SETUNDOCOLLECTION, TRUE, 0);
}

char *getAllText(HWND window,int *length){
	int docLength=SendMessageA(window, SCI_GETLENGTH, 0, (LPARAM)0);
	char *text = new char[docLength+1];
	SendMessageA(window, SCI_GETTEXT, docLength, (LPARAM)text);
	text[docLength]=0;
	*length=docLength;
	return text;

}
static int prev_offset;
void resetPrevOffset() {
	prev_offset = -2;
}

void addEmptyLines(HWND hSci, int offset, int length) {
	static int prev_length;

	if (offset == -1) {
		offset = 0;
		prev_offset = -1;
	} else if(offset == 0 && prev_offset == -1) {
		length += prev_length;
		prev_offset = offset;
	} else {
		prev_offset = offset;
	}
	prev_length = length;

	char *buff = new char[length];
	for (int i = 0; i < length - 1; i++)
		buff[i] = '\n';
	buff[length - 1] = 0;



	::SendMessage(hSci, SCI_ANNOTATIONSETTEXT, offset, (LPARAM)buff);
	::SendMessage(hSci, SCI_ANNOTATIONSETSTYLE, offset, 0);
	delete[] buff;
}
//void addEmptyLines(HWND hSci, int offset, int length){
//	if(length<=0){return;}
//	::SendMessageA(hSci, SCI_SETUNDOCOLLECTION, FALSE, 0);
//	int posAdd=0;
//	UINT EOLtype = ::SendMessageA(hSci,SCI_GETEOLMODE,0,0);
//
//
//	if(offset!=0){
//		int docLines=SendMessageA(hSci, SCI_GETLINECOUNT, 0, (LPARAM)0);
//		posAdd= ::SendMessageA(hSci, SCI_POSITIONFROMLINE, offset-1, 0);
//		
//		
//		posAdd+=::SendMessageA(hSci, SCI_LINELENGTH,offset-1,  (LPARAM)0)-lenEOL[EOLtype];
//		if(offset==docLines){
//			posAdd=SendMessageA(hSci, SCI_GETLENGTH, 0, (LPARAM)0);
//		}
//		if(posAdd!=0){
//			posAdd--;
//		}else{
//			posAdd=lenEOL[EOLtype]-1;
//		}
//	}
//
//	::SendMessageA(hSci, SCI_SETTARGETSTART, posAdd, 0);	
//	::SendMessageA(hSci, SCI_SETTARGETEND, posAdd+1, 0);
//
//	int blankLinesLength = lenEOL[EOLtype] * length + 1;
//	int off = 0;
//	char *buff = new char[blankLinesLength];
//	int marker = 0;
//
//	if(offset == 0)
//    {
//		marker = ::SendMessageA(hSci, SCI_MARKERGET, 0, 0);
//		::SendMessageA(hSci, SCI_MARKERDELETE, 0, (LPARAM)-1);
//		buff[blankLinesLength-1] = (char)SendMessageA(hSci, SCI_GETCHARAT, posAdd, (LPARAM)0);
//		off = 0;
//	}
//    else
//    {
//		buff[0] = (char)SendMessageA(hSci, SCI_GETCHARAT, posAdd, (LPARAM)0);
//		off = 1;
//	}
//
//	for(int j = 0; j < length; j++)
//    {
//		for(unsigned int i = 0; i < lenEOL[EOLtype]; i++)
//        {
//			buff[j * lenEOL[EOLtype] + i + off] = strEOL[EOLtype][i];
//		}
//	}
//
//	::SendMessageA(hSci, SCI_REPLACETARGET, blankLinesLength, (LPARAM)buff);
//
//	for (int i = 0; i < length; i++)
//    {
//	    markAsBlank(hSci, offset + i);
//	}
//	
//    if(offset == 0)
//    {
//		SendMessageA(hSci, SCI_MARKERADDSET, length, marker);		
//	}
//
//#if CLEANUP
//	delete[] buff;
//#endif
//	::SendMessageA(hSci, SCI_SETUNDOCOLLECTION, TRUE, 0);
//}
