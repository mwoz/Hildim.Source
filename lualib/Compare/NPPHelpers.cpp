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
#include "ScintillaMessages.h"

#include "icon_added.h"
#include "icon_removed.h"
#include "icon_changed.h"
#include "icon_moved.h"
#include "icon_arrows.h"

extern NppData nppData;
extern UINT	EOLtype;

int nppBookmarkMarker = -1;
int indicatorHighlight = -1;

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


void setChangedStyle(pSciCaller pc, ColorSettings s)
{
//	pc->IndicSetStyle(1, Scintilla::IndicatorStyle::RoundBox);
//	pc->IndicSetFore(1, Settings.highlight);
//	pc->IndicSetAlpha(1, static_cast<Scintilla::Alpha>(Settings.alpha));
}


void setTextStyle(pSciCaller pc, ColorSettings s)
{
	setChangedStyle(pc, s);
}

void setTextStyles(ColorSettings s)
{
	setTextStyle(nppData.pMain, s);
	setTextStyle(nppData.pSecond, s);
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
	pc->MarkerDefine(MARKER_BLANK, Scintilla::MarkerSymbol::Background);
	pc->MarkerSetBack(MARKER_BLANK, color);
	pc->MarkerSetFore(MARKER_BLANK, color);
}

void DefineXpmSymbol(int type, char ** xpm)
{
	//nppData.pMain->MarkerDefinePixmap(type, xpm)
    SendMessage(nppData._scintillaMainHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
    SendMessage(nppData._scintillaSecondHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
}

void defineRgbaSymbol(int type, const unsigned char* rgba)
{
	nppData.pMain->MarkerDefineRGBAImage(type, reinterpret_cast<const char*>(rgba));
	nppData.pSecond->MarkerDefineRGBAImage(type, reinterpret_cast<const char*>(rgba));
}

void setStyles(UserSettings s)
{
	defineColor(MARKER_ADDED_LINE, s.colors.added);
	defineColor(MARKER_REMOVED_LINE, s.colors.removed);
	defineColor(MARKER_MOVED_LINE, s.colors.moved);
	defineColor(MARKER_CHANGED_LINE, s.colors.changed);
	defineColor(MARKER_BLANK, s.colors.blank);

	nppData.pMain->RGBAImageSetWidth(14);
	nppData.pMain->RGBAImageSetHeight(14);
	nppData.pSecond->RGBAImageSetWidth(14);
	nppData.pSecond->RGBAImageSetHeight(14);

	defineRgbaSymbol(MARKER_CHANGED_SYMBOL, icon_changed);
	defineRgbaSymbol(MARKER_CHANGED_LOCAL_SYMBOL, icon_changed_local);
	defineRgbaSymbol(MARKER_ADDED_SYMBOL, icon_added);
	defineRgbaSymbol(MARKER_ADDED_LOCAL_SYMBOL, icon_added_local);
	defineRgbaSymbol(MARKER_REMOVED_SYMBOL, icon_removed);
	defineRgbaSymbol(MARKER_REMOVED_LOCAL_SYMBOL, icon_removed_local);
	defineRgbaSymbol(MARKER_MOVED_LINE_SYMBOL, icon_moved_line);
	defineRgbaSymbol(MARKER_MOVED_BLOCK_BEGIN_SYMBOL, icon_moved_block_start);
	defineRgbaSymbol(MARKER_MOVED_BLOCK_MID_SYMBOL, icon_moved_block_middle);
	defineRgbaSymbol(MARKER_MOVED_BLOCK_END_SYMBOL, icon_moved_block_end);

}

void markAsBlank(pSciCaller pc,int line)
{
	pc->MarkerAdd(line, MARKER_BLANK);
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
		pc->MarkerAdd(line, MARKER_MOVED_LINE_SYMBOL);
	pc->MarkerAdd(line, MARKER_MOVED_LINE);
}

void markTextAsChanged(pSciCaller pc, intptr_t start, intptr_t length, int color)
{
	if (length > 0)
	{
		//const int curIndic = pc->IndicatorCurrent();
		//pc->SetIndicatorCurrent(indicatorHighlight);
		//pc->SetIndicatorValue(color | SC_INDICVALUEBIT);
		pc->IndicatorFillRange(start, length);
		//pc->SetIndicatorCurrent(curIndic);
	}
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
    int blankMask = 1 << MARKER_BLANK;

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
		pc->MarkerDelete(i, MARKER_BLANK);
	}

	//if we're at the end of the document, we can't delete that line, because it doesn't have any EOL characters to delete
	//, so we have to delete the EOL on the previous line
	if(line>docLength){
		pc->SetTargetStart(posAdd - eolLen);
		length+= eolLen;
	}
	if(length>0){
		pc->MarkerDelete(line, MARKER_BLANK);
		pc->ReplaceTarget(std::string_view(""));
		return lines;
	}else{
		pc->MarkerDelete(line, MARKER_BLANK);
		return 0;
	}

}

//blankLineList *removeEmptyLines(pSciCaller pc,bool saveList)
//{
//	pc->SetUndoCollection(false);
//	blankLineList *list=NULL;	
//
//    int marker = 1 << MARKER_BLANK;
//    Scintilla::Line line = pc->MarkerNext(0, marker);
//	while(line!=-1){
//		int lines=deleteLine(pc,line);
//		if(lines>0&&saveList){
//			//add to list
//			blankLineList *newLine=new blankLineList;
//			newLine->next=list;
//			newLine->line=line;
//			newLine->length=lines;
//			list=newLine;
//		}
//		//line=SendMessageA(window, SCI_MARKERNEXT, 0, marker);	
//        line = pc->MarkerNext(0, marker);
//	}
//
//	//::SendMessage(window, SCI_SETSEL, curPosBeg, curPosEnd);
//	pc->SetUndoCollection(true);
//	return list;
//}

//void addBlankLines(pSciCaller pc,blankLineList *list){
//	pc->SetUndoCollection(false);
//	while(list!=NULL){
//		addEmptyLines(pc,list->line,list->length, NULL);
//		list=list->next;
//	}
//	pc->SetUndoCollection(true);
//}

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

bool getNextLineAfterFold(pSciCaller pS, intptr_t* line)
{
	const intptr_t foldParent = pS->FoldParent(*line);  

	if ((foldParent < 0) || (pS->FoldExpanded(foldParent) != 0))
		return false;

	*line = pS->Call(Scintilla::Message::GetLastChild, foldParent, -1) + 1;

	return true;
}

std::vector<char> getText(pSciCaller pS, intptr_t startPos, intptr_t endPos)
{
	const intptr_t len = endPos - startPos;

	if (len <= 0)
		return std::vector<char>(1, 0);

	std::vector<char> text(len + 1, 0);

	//Sci_TextRangeFull tr;
	Scintilla::TextRangeFull tr;
	tr.chrg.cpMin = startPos;
	tr.chrg.cpMax = endPos;
	tr.lpstrText = text.data();

	pS->GetTextRangeFull(&tr);

	return text;
}


void toLowerCase(std::vector<char>& text, int codepage)
{
	const int len = static_cast<int>(text.size());

	if (len == 0)
		return;

	const int wLen = ::MultiByteToWideChar(codepage, 0, text.data(), len, NULL, 0);

	std::vector<wchar_t> wText(wLen);

	::MultiByteToWideChar(codepage, 0, text.data(), len, wText.data(), wLen);

	wText.push_back(L'\0');
	::CharLowerW((LPWSTR)wText.data());
	wText.pop_back();

	::WideCharToMultiByte(codepage, 0, wText.data(), wLen, text.data(), len, NULL, NULL);
}

void clearWindow(pSciCaller pc)
{
	auto foldedLines = getFoldedLines(pc);
	pc->FoldAll(Scintilla::FoldAction::Expand);
	setFoldedLines(pc, foldedLines);

	pc->AnnotationClearAll();

	pc->MarkerDeleteAll(MARKER_CHANGED_LINE);
	pc->MarkerDeleteAll(MARKER_ADDED_LINE);
	pc->MarkerDeleteAll(MARKER_REMOVED_LINE);
	pc->MarkerDeleteAll(MARKER_MOVED_LINE);
	pc->MarkerDeleteAll(MARKER_CHANGED_SYMBOL);
	pc->MarkerDeleteAll(MARKER_CHANGED_LOCAL_SYMBOL);
	pc->MarkerDeleteAll(MARKER_ADDED_SYMBOL);
	pc->MarkerDeleteAll(MARKER_ADDED_LOCAL_SYMBOL);
	pc->MarkerDeleteAll(MARKER_REMOVED_SYMBOL);
	pc->MarkerDeleteAll(MARKER_REMOVED_LOCAL_SYMBOL);
	pc->MarkerDeleteAll(MARKER_MOVED_LINE_SYMBOL);
	pc->MarkerDeleteAll(MARKER_MOVED_BLOCK_BEGIN_SYMBOL);
	pc->MarkerDeleteAll(MARKER_MOVED_BLOCK_MID_SYMBOL);
	pc->MarkerDeleteAll(MARKER_MOVED_BLOCK_END_SYMBOL);
	pc->MarkerDeleteAll(MARKER_ARROW_SYMBOL);


	clearChangedIndicator(pc, 0, pc->Length());

	pc->ColouriseAll();
}

std::vector<intptr_t> getFoldedLines(pSciCaller pc)
{
	const intptr_t linesCount = pc->LineCount();

	std::vector<intptr_t> foldedLines;

	for (intptr_t line = 0; line < linesCount; ++line)
	{
		line = pc->ContractedFoldNext(line);

		if (line < 0)
			break;

		foldedLines.emplace_back(line);
	}

	return foldedLines;
}


void setFoldedLines(pSciCaller pc, const std::vector<intptr_t>& foldedLines)
{
	for (auto line : foldedLines)
		pc->FoldLine(line, Scintilla::FoldAction::Contract);
}

void clearChangedIndicator(pSciCaller pc, intptr_t start, intptr_t length)
{
	if (length > 0)
	{
		const int curIndic = pc->IndicatorCurrent();
		pc->SetIndicatorCurrent(indicatorHighlight);
		pc->IndicatorClearRange(start, length);
		pc->SetIndicatorCurrent(curIndic);
	}
}

inline intptr_t getPreviousUnhiddenLine(pSciCaller pc, intptr_t line)
{
	intptr_t visibleLine = pc->VisibleFromDocLine(line) - 1;

	if (visibleLine < 0)
		visibleLine = 0;

	return pc->DocLineFromVisible(visibleLine);
}

void addBlankSection(pSciCaller pc, intptr_t line, intptr_t length, intptr_t textLinePos, const char* text)
{
	if (length <= 0)
		return;

	std::vector<char> blank(length - 1, '\n');
	//std::vector<char> blank(length, '\n');

	if (textLinePos > 0 && text != nullptr)
	{
		if (length < textLinePos)
			return;

		blank.insert(blank.begin() + textLinePos - 1, text, text + strlen(text));
	}

	blank.push_back('\0');

	pc->AnnotationSetText(getPreviousUnhiddenLine(pc, line), blank.data());
}

void addBlankSectionAfter(pSciCaller pc, intptr_t line, intptr_t length)
{
	if (length <= 0)
		return;

	std::vector<char> blank(length - 1, '\n');
	blank.push_back('\0');

	pc->AnnotationSetText(pc->DocLineFromVisible(pc->VisibleFromDocLine(line)), blank.data());
}

bool isLineFolded(pSciCaller pc, intptr_t line)
{
	const intptr_t foldParent = pc->FoldParent(line); 

	return (foldParent >= 0 && !pc->FoldExpanded(SCI_GETFOLDEXPANDED));
}