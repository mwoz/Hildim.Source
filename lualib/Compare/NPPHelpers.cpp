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
	::SendMessageA(nppData.scintillaMainHandle, SCI_MARKERDEFINE, type, (LPARAM)symbol);
	::SendMessageA(nppData.scintillaSecondHandle, SCI_MARKERDEFINE, type, (LPARAM)symbol);
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

void DefineXpmSymbol(int type, char ** xpm)
{
	//nppData.pMain->MarkerDefinePixmap(type, xpm)
    SendMessage(nppData.scintillaMainHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
    SendMessage(nppData.scintillaSecondHandle, SCI_MARKERDEFINEPIXMAP, type, (LPARAM)xpm);
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

	nppData.pMain->RGBAImageSetWidth(14);
	nppData.pMain->RGBAImageSetHeight(14);
	nppData.pSecond->RGBAImageSetWidth(14);
	nppData.pSecond->RGBAImageSetHeight(14);

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

static int prev_offset;
void resetPrevOffset() {
	prev_offset = -2;
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

	if (Settings.ignoreComments && Settings.commentStyles.size()) {
		for (intptr_t p = startPos; p < endPos; p++) {
			if (Settings.commentStyles.count(pS->StyleAt(p))) {
				text[p - startPos] = ' ';
			}
		}

	}


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
	blank.push_back('\0');

	if (textLinePos > 0 && text != nullptr)
	{
		if (length < textLinePos)
			return;
		blank.insert(blank.begin() + textLinePos - 1, text, text + strlen(text));

		//blank.resize(blank.size() - 1);
	}

	

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

void hideOutsideRange(pSciCaller pc, intptr_t startLine, intptr_t endLine)
{
	const intptr_t linesCount = pc->LineCount();

	if (startLine >= 0 && (endLine > startLine && endLine < linesCount))
	{
		auto foldedLines = getFoldedLines(pc);
		pc->ShowLines(startLine, endLine);
		setFoldedLines(pc, foldedLines);
	}

	// First line (0) cannot be hidden so start from line 1
	if (startLine > 1)
		pc->HideLines(1, startLine - 1);

	if (endLine > 0 && endLine + 1 < linesCount)
		pc->HideLines(endLine + 1, linesCount - 1);
}

void hideUnmarked(pSciCaller pc, int markMask)
{
	const intptr_t linesCount = pc->LineCount();

	// First line (0) cannot be hidden so start from line 1
	for (intptr_t nextMarkedLine, nextUnmarkedLine = 1; nextUnmarkedLine < linesCount;
		nextUnmarkedLine = nextMarkedLine)
	{
		for (; (nextUnmarkedLine < linesCount) && (pc->MarkerGet(nextUnmarkedLine) & markMask); ++nextUnmarkedLine);

		if (nextUnmarkedLine == linesCount)
			break;

		nextMarkedLine = pc->MarkerNext(nextUnmarkedLine, markMask);

		if (nextMarkedLine < 0)
			nextMarkedLine = linesCount;

		pc->HideLines(nextUnmarkedLine, nextMarkedLine - 1);
	}
}