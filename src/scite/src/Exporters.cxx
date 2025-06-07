// SciTE - Scintilla based Text Editor
/** @file Exporters.cxx
 ** Export the current document to various markup languages.
 **/
// Copyright 1998-2006 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <map>
#include <sstream>

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

// For chdir
#ifdef _MSC_VER
#include <direct.h>
#endif

#endif

#include "Scintilla.h"

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


//---------- Save to RTF ----------

#define RTF_HEADEROPEN "{\\rtf1\\ansi\\deff0\\deftab720"
#define RTF_FONTDEFOPEN "{\\fonttbl"
#define RTF_FONTDEF "{\\f%d\\fnil\\fcharset%u %s;}"
#define RTF_FONTDEFCLOSE "}"
#define RTF_COLORDEFOPEN "{\\colortbl"
#define RTF_COLORDEF "\\red%d\\green%d\\blue%d;"
#define RTF_COLORDEFCLOSE "}"
#define RTF_HEADERCLOSE "\n"
#define RTF_BODYOPEN ""
#define RTF_BODYCLOSE "}"

#define RTF_SETFONTFACE "\\f"
#define RTF_SETFONTSIZE "\\fs"
#define RTF_SETCOLOR "\\cf"
#define RTF_SETBACKGROUND "\\highlight"
#define RTF_BOLD_ON "\\b"
#define RTF_BOLD_OFF "\\b0"
#define RTF_ITALIC_ON "\\i"
#define RTF_ITALIC_OFF "\\i0"
#define RTF_UNDERLINE_ON "\\ul"
#define RTF_UNDERLINE_OFF "\\ulnone"
#define RTF_STRIKE_ON "\\i"
#define RTF_STRIKE_OFF "\\strike0"

#define RTF_EOLN "\\par\n"
#define RTF_TAB "\\tab "

#define MAX_STYLEDEF 128
#define MAX_FONTDEF 64
#define MAX_COLORDEF 8
#define RTF_FONTFACE "Courier New"
#define RTF_COLOR "#000000"

constexpr bool IsASCII(int ch) noexcept {
	return (ch >= 0) && (ch < 0x80);
}

unsigned int UTF32Character(const char *utf8) noexcept {
	unsigned char ch = utf8[0];
	unsigned int u32Char;
	if (ch < 0x80) {
		u32Char = ch;
	} else if (ch < 0x80 + 0x40 + 0x20) {
		u32Char = (ch & 0x1F) << 6;
		ch = utf8[1];
		u32Char += ch & 0x7F;
	} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
		u32Char = (ch & 0xF) << 12;
		ch = utf8[1];
		u32Char += (ch & 0x7F) << 6;
		ch = utf8[2];
		u32Char += ch & 0x7F;
	} else {
		u32Char = (ch & 0x7) << 18;
		ch = utf8[1];
		u32Char += (ch & 0x3F) << 12;
		ch = utf8[2];
		u32Char += (ch & 0x3F) << 6;
		ch = utf8[3];
		u32Char += ch & 0x3F;
	}
	return u32Char;
}

// extract the next RTF control word from *style
void GetRTFNextControl(char **style, char *control) {
	size_t len;
	char *pos = *style;
	*control = '\0';
	if ('\0' == *pos) return;
	pos++; // implicit skip over leading '\'
	while ('\0' != *pos && '\\' != *pos) { pos++; }
	len = pos - *style;
	memcpy(control, *style, len);
	*(control + len) = '\0';
	*style = pos;
}

// extracts control words that are different between two styles
void GetRTFStyleChange(char *delta, char *last, char *current) { // \f0\fs20\cf0\highlight0\b0\i0
	char lastControl[MAX_STYLEDEF], currentControl[MAX_STYLEDEF];
	char *lastPos = last;
	char *currentPos = current;
	*delta = '\0';
	// font face, size, color, background, bold, italic
	for (int i = 0; i < 6; i++) {
		GetRTFNextControl(&lastPos, lastControl);
		GetRTFNextControl(&currentPos, currentControl);
		if (strcmp(lastControl, currentControl)) {	// changed
			strcat(delta, currentControl);
		}
	}
	if ('\0' != *delta) { strcat(delta, " "); }
	strcpy(last, current);
}

static void GetRTFNextControl(const char **style, char *control) noexcept {
	const char *pos = *style;
	*control = '\0';
	if ('\0' == *pos) return;
	pos++; // implicit skip over leading '\'
	while ('\0' != *pos && '\\' != *pos) { pos++; }
	ptrdiff_t len = pos - *style;
	memcpy(control, *style, len);
	*(control + len) = '\0';
	*style = pos;
}

static std::string GetRTFStyleChange(const char *last, const char *current) { // \f0\fs20\cf0\highlight0\b0\i0
	char lastControl[MAX_STYLEDEF] = "";
	char currentControl[MAX_STYLEDEF] = "";
	const char *lastPos = last;
	const char *currentPos = current;
	std::string delta;
	// font face, size, color, background, bold, italic
	for (int i = 0; i < 6; i++) {
		GetRTFNextControl(&lastPos, lastControl);
		GetRTFNextControl(&currentPos, currentControl);
		if (strcmp(lastControl, currentControl)) {	// changed
			delta += currentControl;
		}
	}
	if (!delta.empty()) { delta += " "; }
	return delta;
}

static size_t FindCaseInsensitive(const std::vector<std::string> &values, const std::string &s) {
	for (size_t i = 0; i < values.size(); i++)
		if (EqualCaseInsensitive(s.c_str(), values[i].c_str()))
			return i;
	return values.size();
}

void SciTEBase::SaveToStreamRTF(std::ostream &os, Sci_Position start, Sci_Position end) {
	const Sci_Position lengthDoc = LengthDocument();
	if (end < 0)
		end = lengthDoc; 
	//RemoveFindMarks();
	wEditor.Colourise(0, -1);

	// Read the default settings
	char key[200];
	sprintf(key, "style.*.%0d", STYLE_DEFAULT);
	char *valdef = StringDup(props.GetExpanded(key).c_str());
	sprintf(key, "style.%s.%0d", language.c_str(), STYLE_DEFAULT);
	char *val = StringDup(props.GetExpanded(key).c_str());

	StyleDefinition defaultStyle(valdef);
	defaultStyle.ParseStyleDefinition(val);

	int tabSize = props.GetInt("export.rtf.tabsize", props.GetInt("tabsize"));
	const int wysiwyg = props.GetInt("export.rtf.wysiwyg", 1);
	std::string fontFace = props.GetExpanded("export.rtf.font.face").c_str();
	if (fontFace.length()) {
		defaultStyle.font = fontFace.c_str();
	} else if (defaultStyle.font.length() == 0) {
		defaultStyle.font = RTF_FONTFACE;
	}
	const int fontSize = props.GetInt("export.rtf.font.size", 0);
	if (fontSize > 0) {
		defaultStyle.size = fontSize << 1;
	} else if (defaultStyle.size == 0) {
		defaultStyle.size = 10 << 1;
	} else {
		defaultStyle.size <<= 1;
	}
	const bool isUTF8 = (codePage == SC_CP_UTF8);
	const unsigned int characterset = props.GetInt("character.set", 1251);
	const int tabs = props.GetInt("export.rtf.tabs", 0);
	if (tabSize == 0)
		tabSize = 4;

	std::vector<std::string> styles;
	std::vector<std::string> fonts;
	std::vector<std::string> colors;
	os << RTF_HEADEROPEN << RTF_FONTDEFOPEN;
	fonts.push_back(defaultStyle.font.c_str());
	os << "{\\f" << 0 << "\\fnil\\fcharset" << characterset << " " << defaultStyle.font.c_str() << ";}";
	if (defaultStyle.fore.size() == 0)
		defaultStyle.fore = "#000000";
	colors.push_back(defaultStyle.fore.c_str());
	if (defaultStyle.back.size() == 0)
		defaultStyle.back = "#FFFFFF";
	colors.push_back(defaultStyle.back.c_str());

	for (int istyle = 0; istyle <= STYLE_MAX; istyle++) {
		std::ostringstream osStyle;

		sprintf(key, "style.%s.%0d", language.c_str(), istyle);
		SString sval = props.GetExpanded(key);

		const StyleDefinition sd(sval.c_str());

		if (sd.specified != StyleDefinition::sdNone) {
			size_t iFont = 0;
			if (wysiwyg && sd.font.length()) {
				iFont = FindCaseInsensitive(fonts, sd.font.c_str());
				if (iFont >= fonts.size()) {
					fonts.push_back(sd.font.c_str());
					os << "{\\f" << iFont << "\\fnil\\fcharset" << characterset << " " << sd.font.c_str() << ";}";
				}
			}
			osStyle << RTF_SETFONTFACE << iFont;

			osStyle << RTF_SETFONTSIZE << (wysiwyg && sd.size ? sd.size << 1 : defaultStyle.size);

			size_t iFore = 0;
			if (sd.specified & StyleDefinition::sdFore) {
				iFore = FindCaseInsensitive(colors, sd.fore.c_str());
				if (iFore >= colors.size())
					colors.push_back(sd.fore.c_str());
			}
			osStyle << RTF_SETCOLOR << iFore;

			// PL: highlights doesn't seems to follow a distinct table, at least with WordPad and Word 97
			// Perhaps it is different for Word 6?
			size_t iBack = 1;
			if (sd.specified & StyleDefinition::sdBack) {
				iBack = FindCaseInsensitive(colors, sd.back.c_str());
				if (iBack >= colors.size())
					colors.push_back(sd.back.c_str());
			}
			osStyle << RTF_SETBACKGROUND << iBack;

			if (sd.specified & StyleDefinition::sdBold) {
				osStyle << (sd.bold ? RTF_BOLD_ON : RTF_BOLD_OFF);
			} else {
				osStyle << (defaultStyle.bold ? RTF_BOLD_ON : RTF_BOLD_OFF);
			}
			if (sd.specified & StyleDefinition::sdItalics) {
				osStyle << (sd.italics ? RTF_ITALIC_ON : RTF_ITALIC_OFF);
			} else {
				osStyle << (defaultStyle.italics ? RTF_ITALIC_ON : RTF_ITALIC_OFF);
			}
		} else {
			osStyle << RTF_SETFONTFACE "0" RTF_SETFONTSIZE << defaultStyle.size <<
				RTF_SETCOLOR "0" RTF_SETBACKGROUND "1"
				RTF_BOLD_OFF RTF_ITALIC_OFF;
		}
		styles.push_back(osStyle.str());
	}
	os << RTF_FONTDEFCLOSE RTF_COLORDEFOPEN;
	for (const std::string &color : colors) {
		os << "\\red" << IntFromHexByte(color.c_str() + 1) << "\\green" << IntFromHexByte(color.c_str() + 3) <<
			"\\blue" << IntFromHexByte(color.c_str() + 5) << ";";
	}
	os << RTF_COLORDEFCLOSE RTF_HEADERCLOSE RTF_BODYOPEN RTF_SETFONTFACE "0"
		RTF_SETFONTSIZE << defaultStyle.size << RTF_SETCOLOR "0 ";
	std::ostringstream osStyleDefault;
	osStyleDefault << RTF_SETFONTFACE "0" RTF_SETFONTSIZE << defaultStyle.size <<
		RTF_SETCOLOR "0" RTF_SETBACKGROUND "1"
		RTF_BOLD_OFF RTF_ITALIC_OFF; 
	std::string lastStyle = osStyleDefault.str();
	bool prevCR = false;
	int styleCurrent = -1;
	TextReader acc(wEditor); 
	int column = 0;
	for (Sci_Position iPos = start; iPos < end; iPos++) {
		const char ch = acc[iPos];
		const UCHAR style = acc.StyleAt(iPos); 

		if (style != styleCurrent) {
			const std::string deltaStyle = GetRTFStyleChange(lastStyle.c_str(), styles[style].c_str());
			lastStyle = styles[style];
			if (!deltaStyle.empty())
				os << deltaStyle;
			styleCurrent = style;
		}
		if (ch == '{')
			os << "\\{";
		else if (ch == '}')
			os << "\\}";
		else if (ch == '\\')
			os << "\\\\";
		else if (ch == '\t') {
			if (tabs) {
				os << RTF_TAB;
			} else {
				const int ts = tabSize - (column % tabSize);
				for (int itab = 0; itab < ts; itab++) {
					os << ' ';
				}
				column += ts - 1;
			}
		} else if (ch == '\n') {
			if (!prevCR) {
				os << RTF_EOLN;
				column = -1;
			}
		} else if (ch == '\r') {
			os << RTF_EOLN;
			column = -1;
		} else if (isUTF8 && !IsASCII(ch)) {
			const Sci_Position nextPosition = wEditor.PositionAfter(iPos);
			wEditor.SetTargetStart(iPos);
			wEditor.SetTargetEnd(nextPosition);
			char u8Char[5] = "";
			wEditor.TargetAsUTF8(u8Char);
			const unsigned int u32 = UTF32Character(u8Char);
			if (u32 < 0x10000) {
				os << "\\u" << static_cast<short>(u32) << "?";
			} else {
				os << "\\u" << static_cast<short>(((u32 - 0x10000) >> 10) + 0xD800) << "?";
				os << "\\u" << static_cast<short>((u32 & 0x3ff) + 0xDC00) << "?";
			}
			iPos = nextPosition - 1;
		} else {
			os << ch;
		}
		column++;
		prevCR = ch == '\r';
	}
	os << RTF_BODYCLOSE;
}

void SciTEBase::SaveToRTF(const FilePath &saveName, int start, int end) {
	FILE *fp = saveName.Open(GUI_TEXT("wt"));
	bool failedWrite = fp == nullptr;
	if (fp) {
		try {
			std::ostringstream oss;
			SaveToStreamRTF(oss, start, end);
			const std::string rtf = oss.str();
			if (fwrite(rtf.c_str(), 1, rtf.length(), fp) != rtf.length()) {
				failedWrite = true;
			}
			if (fclose(fp) != 0) {
				failedWrite = true;
			}
		} catch (std::exception &) {
			failedWrite = true;
		}
	}
	if (failedWrite) {
		extender->HildiAlarm("Could not save file\n'%1'",
			MB_OK | MB_ICONWARNING, filePath.AsInternal());
	}
}

//---------- Save to HTML ----------
void SciTEBase::SaveToStreamHTMLText(std::ostream& os, int start, int end) {
	wEditor.Colourise(0, -1);
	int tabSize = props.GetInt("tabsize");
	if (tabSize == 0)
		tabSize = 4;
	const Sci_Position lengthDoc = end == -1 ? LengthDocument() : end;
	TextReader acc(wEditor);
	constexpr int StyleLastPredefined = STYLE_LASTPREDEFINED;

	std::string bgColour;
	std::string spanOpen[STYLE_MAX + 1];
	std::string spanClose[STYLE_MAX + 1];
	std::string style;
	std::string pStyle = "";
	std::string defBack = "";
	SString sval;

	char key[200];
	sprintf(key, "style.%s.%0d", language.c_str(), 0);
	sval = props.GetExpanded(key);
	const StyleDefinition sd(sval.c_str());
	if (sd.font.length()) 
		pStyle = sd.font.c_str();
	if (sd.back.length()) 
		defBack = sd.back.c_str();
	

	for (int istyle = 0; istyle <= STYLE_MAX; istyle++) {
		spanOpen[istyle] = "";
		spanClose[istyle] = "";
		style = "";
		if ((istyle > STYLE_DEFAULT) && (istyle <= StyleLastPredefined))
			continue;

		sprintf(key, "style.%s.%0d", language.c_str(), istyle);
		sval = props.GetExpanded(key);

		const StyleDefinition sd(sval.c_str());

		if (sd.specified != StyleDefinition::sdNone) {

			if (sd.italics) {
				spanOpen[istyle] += "<em>";
				spanClose[istyle] = "</em>" + spanClose[istyle];
			}
			if (sd.bold) {
				spanOpen[istyle] += "<strong>";
				spanClose[istyle] = "</strong>" + spanClose[istyle];
			}
			if (sd.font.length() && (pStyle == "" || pStyle != sd.font.c_str())) {
				style += "font-family: ";
				style += sd.font.c_str() ;
				style += ";";
			}
			if (sd.fore.length()) {
				style += "color: ";
				style += sd.fore.c_str();
				style += ";";
			}
			if (sd.back.length() && defBack != sd.back.c_str()) {
				style += "background: ";
				style += sd.back.c_str();
				style += ";";
			}
			if(style != ""){
				spanOpen[istyle] += "<span style=\"";
				spanOpen[istyle] += style;
				spanOpen[istyle] += "\">";
				spanClose[istyle] = "</span>" + spanClose[istyle];
			}

		}
	}
	if (pStyle == "")
		pStyle = "<p>";
	else
		pStyle = "<p style=\"font-family: " + pStyle + "\">";

	int styleCurrent = -1;
	std::string close = "";
	os << pStyle;
	int column = 0;
	for (int i = start; i < lengthDoc; i++) {
		const char ch = acc[i];
		const UCHAR style = acc.StyleAt(i);
		column++;
		if (style != styleCurrent) {
			styleCurrent = style;

			if (close != "")
				os << close;
			

			if (style) {
				close = spanClose[style];
				os << spanOpen[style];
			}
		}
		switch (ch) {
		case '\r':
		case '\n':
			if (close != "") {
				os << close;
			}
			os << "</p>\r\n";
			os << pStyle;
			os << spanOpen[style];
			column = 0;
			if (ch == '\r' && i < lengthDoc && acc[i + 1] == '\n') {
				i++;	// CR+LF line ending, skip the "extra" EOL char
			}
			break;
		case ' ':		
			if (i < lengthDoc && acc[i + 1] == ' ') {
				do {
					os << "&nbsp;";
					i++;
					column++;
				} while (i < lengthDoc && acc[i + 1] == ' ');
			}
			else
				os << "&nbsp;";
			break;
		case '\t':
		{
			const int ts = tabSize - (column % tabSize);
			for (int itab = 0; itab < ts; itab++) {
				os << "&nbsp;";
			}
			column += ts;
		}
			break;
		case '<':
			os << "&lt;";
			break;
		case '>':
			os << "&gt;";
			break;
		case '&':
			os << "&amp;";
			break;
		default:
			os << ch;
		}
	}
	os << "</p>";
}
void SciTEBase::SaveToStreamHTML(std::ostream &os, int start, int end) {

	wEditor.Colourise(0, -1);
	int tabSize = props.GetInt("tabsize");
	if (tabSize == 0)
		tabSize = 4;
	const int wysiwyg = props.GetInt("export.html.wysiwyg", 1);
	const bool tabs = (props.GetInt("export.html.tabs", 0) != 0) && (end == -1);
	const int folding = props.GetInt("export.html.folding", 0);
	const int onlyStylesUsed = end == -1 ? props.GetInt("export.html.styleused", 0) : 1;
	const int titleFullPath = props.GetInt("export.html.title.fullpath", 0);

	const Sci_Position lengthDoc = end == -1 ? LengthDocument() : end;
	TextReader acc(wEditor);

	constexpr int StyleLastPredefined = STYLE_LASTPREDEFINED;

	bool styleIsUsed[STYLE_MAX + 1] = {};
	if (onlyStylesUsed) {
		// check the used styles
		for (int i = 0; i < lengthDoc; i++) {
			styleIsUsed[(UCHAR)acc.StyleAt(i)] = true;
		}
	} else {
		for (int i = 0; i <= STYLE_MAX; i++) {
			styleIsUsed[i] = true;
		}
	}
	styleIsUsed[STYLE_DEFAULT] = true;

	if (end == -1) {
		os << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
		os << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
	} else {
		os << "Version:0.9\n"
			"StartHTML:0000000000\n"
			"EndHTML:0000000000\n"
			"StartFragment:0000000000\n"
			"EndFragment:0000000000\n"
			"<html>\n";
	}
	os << "<head>\n";
	if (titleFullPath)
		os << "<title>" << filePath.AsUTF8().c_str() << "</title>\n";
	else
		os << "<title>" << filePath.AsUTF8().c_str() << "</title>\n";

	// Probably not used by robots, but making a little advertisement for those looking
	// at the source code doesn't hurt...
	os << "<meta name=\"Generator\" content=\"SciTE - www.Scintilla.org\" />\n";
	bool toUtf = (codePage == CP_ACP) && (end != -1);
	if (codePage == CP_UTF8 && end != -1)
		os << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n";

	if (folding) {
		os << "<script language=\"JavaScript\" type=\"text/javascript\">\n"
			"<!--\n"
			"function symbol(id, sym) {\n"
			" if (id.textContent==undefined) {\n"
			" id.innerText=sym; } else {\n"
			" id.textContent=sym; }\n"
			"}\n"
			"function toggle(id) {\n"
			"var thislayer=document.getElementById('ln'+id);\n"
			"id-=1;\n"
			"var togline=document.getElementById('hd'+id);\n"
			"var togsym=document.getElementById('bt'+id);\n"
			"if (thislayer.style.display == 'none') {\n"
			" thislayer.style.display='';\n"
			" togline.style.textDecoration='none';\n"
			" symbol(togsym,'- ');\n"
			"} else {\n"
			" thislayer.style.display='none';\n"
			" togline.style.textDecoration='underline';\n"
			" symbol(togsym,'+ ');\n"
			"}\n"
			"}\n"
			"//-->\n"
			"</script>\n";
	}

	os << "<style type=\"text/css\">\n";

	std::string bgColour;

	char key[200];
	sprintf(key, "style.*.%0d", STYLE_DEFAULT);
	char *valdef = StringDup(props.GetExpanded(key).c_str());
	StyleDefinition sddef(valdef);

	if (sddef.back.length()) {
		bgColour = sddef.back.c_str();
	}

	std::string sval = props.GetExpanded("font.monospace").c_str();

	for (int istyle = 0; istyle <= STYLE_MAX; istyle++) {
		if ((istyle > STYLE_DEFAULT) && (istyle <= StyleLastPredefined))
			continue;
		if (styleIsUsed[istyle]) {
			sprintf(key, "style.%s.%0d", language.c_str(), istyle);
			SString sval = props.GetExpanded(key);

			const StyleDefinition sd(sval.c_str());

			if (sd.specified != StyleDefinition::sdNone) {
				if (istyle == STYLE_DEFAULT) {
					if (wysiwyg) {
						os << "span {\n";
					} else {
						os << "pre {\n";
					}
				} else {
					os << ".S" << istyle << " {\n";
				}
				if (sd.italics) {
					os << "\tfont-style: italic;\n";
				}
				if (sd.bold) {
					os << "\tfont-weight: bold;\n";
				}
				if (wysiwyg && sd.font.length()) {
					os << "\tfont-family: '" << sd.font.c_str() << "';\n";
				}
				if (sd.fore.length()) {
					os << "\tcolor: " << sd.fore.c_str() << ";\n";
				} else if (istyle == STYLE_DEFAULT) {
					os << "\tcolor: #000000;\n";
				}
				if ((sd.specified & StyleDefinition::sdBack) && sd.back.length()) {
					if (istyle != STYLE_DEFAULT && strcmp(bgColour.c_str(), sd.back.c_str())) {
						os << "\tbackground: " << sd.back.c_str() << ";\n";
						os << "\ttext-decoration: inherit;\n";
					}
				}
				if (wysiwyg && sd.size) {
					os << "\tfont-size: " << sd.size << "pt;\n";
				}
				os << "}\n";
			} else {
				styleIsUsed[istyle] = false;	// No definition, it uses default style (32)
			}
		}
	}
	os << "</style>\n";
	os << "</head>\n";
	if (bgColour.length() > 0)
		os << "<body bgcolor=\"" << bgColour.c_str() << "\">\n";
	else
		os << "<body>\n";
	if (end != -1)
		os << "<!--StartFragment-->\n";

	Sci_Position line = acc.GetLine(start);
	Sci_Position level = (acc.LevelAt(line) & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE;
	UCHAR styleCurrent = acc.StyleAt(start);
	bool inStyleSpan = false;
	bool inFoldSpan = false;
	// Global span for default attributes
	if (wysiwyg) {
		os << "<span>";
	} else {
		os << "<pre>";
	}

	if (folding) {
		const int lvl = acc.LevelAt(line);
		level = (lvl & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE;

		if (lvl && SC_FOLDLEVELHEADERFLAG) {
			const std::string sLine = std::to_string(line);
			const std::string sLineNext = std::to_string(line + 1);
			os << "<span id=\"hd" << sLine.c_str() << "\" onclick=\"toggle('" << sLineNext.c_str() << "')\">";
			os << "<span id=\"bt" << sLine.c_str() << "\">- </span>";
			inFoldSpan = true;
		} else {
			os << "&nbsp; ";
		}
	}

	if (styleIsUsed[styleCurrent]) {
		os << "<span class=\"S" << (int)styleCurrent << "\">";
		inStyleSpan = true;
	}
	// Else, this style has no definition (beside default one):
	// no span for it, except the global one

	int column = 0;
	for (int i = start; i < lengthDoc; i++) {
		const char ch = acc[i];
		const UCHAR style = acc.StyleAt(i);

		if (style != styleCurrent) {
			if (inStyleSpan) {
				os << "</span>";
				inStyleSpan = false;
			}
			if (ch != '\r' && ch != '\n') {	// No need of a span for the EOL
				if (styleIsUsed[style]) {
					os << "<span class=\"S" << (int)style << "\">";
					inStyleSpan = true;
				}
				styleCurrent = style;
			}
		}
		if (ch == ' ') {
			if (wysiwyg) {
				char prevCh = '\0';
				if (column == 0) {	// At start of line, must put a &nbsp; because regular space will be collapsed
					prevCh = ' ';
				}
				while (i < lengthDoc && acc[i] == ' ') {
					if (prevCh != ' ') {
						os << ' ';
					} else {
						os << "&nbsp;";
					}
					prevCh = acc[i];
					i++;
					column++;
				}
				i--; // the last incrementation will be done by the for loop
			} else {
				os << ' ';
				column++;
			}
		} else if (ch == '\t') {
			const int ts = tabSize - (column % tabSize);
			if (wysiwyg) {
				for (int itab = 0; itab < ts; itab++) {
					if (itab % 2) {
						os << ' ';
					} else {
						os << "&nbsp;";
					}
				}
				column += ts;
			} else {
				if (tabs) {
					os << ch;
					column++;
				} else {
					for (int itab = 0; itab < ts; itab++) {
						os << ' ';
					}
					column += ts;
				}
			}
		} else if (ch == '\r' || ch == '\n') {
			if (inStyleSpan) {
				os << "</span>";
				inStyleSpan = false;
			}
			if (inFoldSpan) {
				os << "</span>";
				inFoldSpan = false;
			}
			if (ch == '\r' && acc[i + 1] == '\n') {
				i++;	// CR+LF line ending, skip the "extra" EOL char
			}
			column = 0;
			if (wysiwyg) {
				os << "<br />";
			}

			styleCurrent = acc.StyleAt(i + 1);
			if (folding) {
				line = acc.GetLine(i + 1);

				const int lvl = acc.LevelAt(line);
				const int newLevel = (lvl & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE;

				if (newLevel < level)
					os << "</span>";
				os << '\n'; // here to get clean code
				if (newLevel > level) {
					const std::string sLine = std::to_string(line);
					os << "<span id=\"ln" << sLine.c_str() << "\">";
				}

				if (lvl & SC_FOLDLEVELHEADERFLAG) {
					const std::string sLine = std::to_string(line);
					const std::string sLineNext = std::to_string(line + 1);
					os << "<span id=\"hd" << sLine.c_str() << "\" onclick=\"toggle('" << sLineNext.c_str() << "')\">";
					os << "<span id=\"bt" << sLine.c_str() << "\">- </span>";
					inFoldSpan = true;
				} else
					os << "&nbsp; ";
				level = newLevel;
			} else {
				os << '\n';
			}

			if (styleIsUsed[styleCurrent] && acc[i + 1] != '\r' && acc[i + 1] != '\n') {
				// We know it's the correct next style,
				// but no (empty) span for an empty line
				os << "<span class=\"S" << (int)styleCurrent << "\">";
				inStyleSpan = true;
			}
		} else {
			switch (ch) {
			case '<':
				os << "&lt;";
				break;
			case '>':
				os << "&gt;";
				break;
			case '&':
				os << "&amp;";
				break;
			default:
				if (ch > 0 || !toUtf) {
					os << ch;
				} else {
					std::string s;
					s += ch;
					os << GUI::ConvertToUTF8(s, CP_ACP);
				}
			}
			column++;
		}
	}

	if (inStyleSpan) {
		os << "</span>";
	}

	if (folding) {
		while (level > 0) {
			os << "</span>";
			level--;
		}
	}

	if (!wysiwyg) {
		os << "</pre>";
	} else {
		os << "</span>";
	}
	if (end != -1)
		os << "<!--EndFragment-->";
	os << "\n</body>\n</html>\n";

}

void SciTEBase::SaveToHTML(FilePath saveName) {
	FILE *fp = saveName.Open(GUI_TEXT("wt"));
	bool failedWrite = fp == nullptr;
	if (fp) {
		try {
			std::ostringstream oss;
			SaveToStreamHTML(oss);
			const std::string html = oss.str();
			if (fwrite(html.c_str(), 1, html.length(), fp) != html.length()) {
				failedWrite = true;
			}
			if (fclose(fp) != 0) {
				failedWrite = true;
			}
		} catch (std::exception &) {
			failedWrite = true;
		}
	}
	if (failedWrite) {
		extender->HildiAlarm("Could not save file\n'%1'",
			MB_OK | MB_ICONWARNING, filePath.AsInternal());
	}

}


//---------- Save to PDF ----------

/*
	PDF Exporter. Status: Beta
	Contributed by Ahmad M. Zawawi <zeus_go64@hotmail.com>
	Modifications by Darren Schroeder Feb 22, 2003; Philippe Lhoste 2003-10
	Overhauled by Kein-Hong Man 2003-11

	This exporter is meant to be small and simple; users are expected to
	use other methods for heavy-duty formatting. PDF elements marked with
	"PDF1.4Ref" states where in the PDF 1.4 Reference Spec (the PDF file of
	which is freely available from Adobe) the particular element can be found.

	Possible TODOs that will probably not be implemented: full styling,
	optimization, font substitution, compression, character set encoding.
*/
#define PDF_TAB_DEFAULT		8
#define PDF_FONT_DEFAULT	1	// Helvetica
#define PDF_FONTSIZE_DEFAULT	10
#define PDF_SPACING_DEFAULT	1.2
#define PDF_HEIGHT_DEFAULT	792	// Letter
#define PDF_WIDTH_DEFAULT	612
#define PDF_MARGIN_DEFAULT	72	// 1.0"
#define PDF_ENCODING		"WinAnsiEncoding"

struct PDFStyle {
	char fore[24];
	int font;
};

static const char *PDFfontNames[] = {
            "Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique",
            "Helvetica", "Helvetica-Bold", "Helvetica-Oblique", "Helvetica-BoldOblique",
            "Times-Roman", "Times-Bold", "Times-Italic", "Times-BoldItalic"
        };

// ascender, descender aligns font origin point with page
static short PDFfontAscenders[] =  { 629, 718, 699 };
static short PDFfontDescenders[] = { 157, 207, 217 };
static short PDFfontWidths[] =     { 600,   0,   0 };

inline void getPDFRGB(char* pdfcolour, const char* stylecolour) {
	// grab colour components (max string length produced = 18)
	for (int i = 1; i < 6; i += 2) {
		char val[20];
		// 3 decimal places for enough dynamic range
		int c = (IntFromHexByte(stylecolour + i) * 1000 + 127) / 255;
		if (c == 0 || c == 1000) {	// optimise
			sprintf(val, "%d ", c / 1000);
		} else {
			sprintf(val, "0.%03d ", c);
		}
		strcat(pdfcolour, val);
	}
}

void SciTEBase::SaveToPDF(FilePath saveName) {
	// This class conveniently handles the tracking of PDF objects
	// so that the cross-reference table can be built (PDF1.4Ref(p39))
	// All writes to fp passes through a PDFObjectTracker object.
	class PDFObjectTracker {
	private:
		FILE *fp;
		int *offsetList, tableSize;
	public:
		int index;
		PDFObjectTracker(FILE *fp_) {
			fp = fp_;
			tableSize = 100;
			offsetList = new int[tableSize];
			index = 1;
		}
		~PDFObjectTracker() {
			delete []offsetList;
		}
		void write(const char *objectData) {
			size_t length = strlen(objectData);
			// note binary write used, open with "wb"
			fwrite(objectData, sizeof(char), length, fp);
		}
		void write(int objectData) {
			char val[20];
			sprintf(val, "%d", objectData);
			write(val);
		}
		// returns object number assigned to the supplied data
		int add(const char *objectData) {
			// resize xref offset table if too small
			if (index > tableSize) {
				int newSize = tableSize * 2;
				int *newList = new int[newSize];
				for (int i = 0; i < tableSize; i++) {
					newList[i] = offsetList[i];
				}
				delete []offsetList;
				offsetList = newList;
				tableSize = newSize;
			}
			// save offset, then format and write object
			offsetList[index - 1] = ftell(fp);
			write(index);
			write(" 0 obj\n");
			write(objectData);
			write("endobj\n");
			return index++;
		}
		// builds xref table, returns file offset of xref table
		int xref() {
			char val[32];
			// xref start index and number of entries
			int xrefStart = ftell(fp);
			write("xref\n0 ");
			write(index);
			// a xref entry *must* be 20 bytes long (PDF1.4Ref(p64))
			// so extra space added; also the first entry is special
			write("\n0000000000 65535 f \n");
			for (int i = 0; i < index - 1; i++) {
				sprintf(val, "%010d 00000 n \n", offsetList[i]);
				write(val);
			}
			return xrefStart;
		}
	};

	// Object to manage line and page rendering. Apart from startPDF, endPDF
	// everything goes in via add() and nextLine() so that line formatting
	// and pagination can be done properly.
	class PDFRender {
	private:
		bool pageStarted;
		bool firstLine;
		int pageCount;
		int pageContentStart;
		double xPos, yPos;	// position tracking for line wrapping
		SString pageData;	// holds PDF stream contents
		SString segment;	// character data
		char *segStyle;		// style of segment
		bool justWhiteSpace;
		int styleCurrent, stylePrev;
		double leading;
		char *buffer;
	public:
		PDFObjectTracker *oT;
		PDFStyle *style;
		int fontSize;		// properties supplied by user
		int fontSet;
		int pageWidth, pageHeight;
		GUI::Rectangle pageMargin;
		//
		PDFRender() {
			pageStarted = false;
			pageCount = 0;
			style = NULL;
			buffer = new char[250];
			segStyle = new char[100];
		}
		~PDFRender() {
			delete []style;
			delete []buffer;
			delete []segStyle;
		}
		//
		double fontToPoints(int thousandths) {
			return (double)fontSize * thousandths / 1000.0;
		}
		void setStyle(char *buff, int style_) {
			int styleNext = style_;
			if (style_ == -1) { styleNext = styleCurrent; }
			*buff = '\0';
			if (styleNext != styleCurrent || style_ == -1) {
				if (style[styleCurrent].font != style[styleNext].font
				        || style_ == -1) {
					sprintf(buff, "/F%d %d Tf ",
					        style[styleNext].font + 1, fontSize);
				}
				if (strcmp(style[styleCurrent].fore, style[styleNext].fore) != 0
				        || style_ == -1) {
					strcat(buff, style[styleNext].fore);
					strcat(buff, "rg ");
				}
			}
		}
		//
		void startPDF() {
			if (fontSize <= 0) {
				fontSize = PDF_FONTSIZE_DEFAULT;
			}
			// leading is the term for distance between lines
			leading = fontSize * PDF_SPACING_DEFAULT;
			// sanity check for page size and margins
			int pageWidthMin = (int)leading + pageMargin.left + pageMargin.right;
			if (pageWidth < pageWidthMin) {
				pageWidth = pageWidthMin;
			}
			int pageHeightMin = (int)leading + pageMargin.top + pageMargin.bottom;
			if (pageHeight < pageHeightMin) {
				pageHeight = pageHeightMin;
			}
			// start to write PDF file here (PDF1.4Ref(p63))
			// ASCII>127 characters to indicate binary-possible stream
			oT->write("%PDF-1.3\n%«Ïè¢\n");
			styleCurrent = STYLE_DEFAULT;

			// build objects for font resources; note that font objects are
			// *expected* to start from index 1 since they are the first objects
			// to be inserted (PDF1.4Ref(p317))
			for (int i = 0; i < 4; i++) {
				sprintf(buffer, "<</Type/Font/Subtype/Type1"
				        "/Name/F%d/BaseFont/%s/Encoding/"
				        PDF_ENCODING
				        ">>\n", i + 1,
				        PDFfontNames[fontSet * 4 + i]);
				oT->add(buffer);
			}
			pageContentStart = oT->index;
		}
		void endPDF() {
			if (pageStarted) {	// flush buffers
				endPage();
			}
			// refer to all used or unused fonts for simplicity
			int resourceRef = oT->add(
			            "<</ProcSet[/PDF/Text]\n"
			            "/Font<</F1 1 0 R/F2 2 0 R/F3 3 0 R"
			            "/F4 4 0 R>> >>\n");
			// create all the page objects (PDF1.4Ref(p88))
			// forward reference pages object; calculate its object number
			int pageObjectStart = oT->index;
			int pagesRef = pageObjectStart + pageCount;
			for (int i = 0; i < pageCount; i++) {
				sprintf(buffer, "<</Type/Page/Parent %d 0 R\n"
				        "/MediaBox[ 0 0 %d %d"
				        "]\n/Contents %d 0 R\n"
				        "/Resources %d 0 R\n>>\n",
				        pagesRef, pageWidth, pageHeight,
				        pageContentStart + i, resourceRef);
				oT->add(buffer);
			}
			// create page tree object (PDF1.4Ref(p86))
			pageData = "<</Type/Pages/Kids[\n";
			for (int j = 0; j < pageCount; j++) {
				sprintf(buffer, "%d 0 R\n", pageObjectStart + j);
				pageData += buffer;
			}
			sprintf(buffer, "]/Count %d\n>>\n", pageCount);
			pageData += buffer;
			oT->add(pageData.c_str());
			// create catalog object (PDF1.4Ref(p83))
			sprintf(buffer, "<</Type/Catalog/Pages %d 0 R >>\n", pagesRef);
			int catalogRef = oT->add(buffer);
			// append the cross reference table (PDF1.4Ref(p64))
			int xref = oT->xref();
			// end the file with the trailer (PDF1.4Ref(p67))
			sprintf(buffer, "trailer\n<< /Size %d /Root %d 0 R\n>>"
			        "\nstartxref\n%d\n%%%%EOF\n",
			        oT->index, catalogRef, xref);
			oT->write(buffer);
		}
		void add(char ch, int style_) {
			if (!pageStarted) {
				startPage();
			}
			// get glyph width (TODO future non-monospace handling)
			double glyphWidth = fontToPoints(PDFfontWidths[fontSet]);
			xPos += glyphWidth;
			// if cannot fit into a line, flush, wrap to next line
			if (xPos > pageWidth - pageMargin.right) {
				nextLine();
				xPos += glyphWidth;
			}
			// if different style, then change to style
			if (style_ != styleCurrent) {
				flushSegment();
				// output code (if needed) for new style
				setStyle(segStyle, style_);
				stylePrev = styleCurrent;
				styleCurrent = style_;
			}
			// escape these characters
			if (ch == ')' || ch == '(' || ch == '\\') {
				segment += '\\';
			}
			if (ch != ' ') { justWhiteSpace = false; }
			segment += ch;	// add to segment data
		}
		void flushSegment() {
			if (segment.length() > 0) {
				if (justWhiteSpace) {	// optimise
					styleCurrent = stylePrev;
				} else {
					pageData += segStyle;
				}
				pageData += "(";
				pageData += segment;
				pageData += ")Tj\n";
			}
			segment.clear();
			*segStyle = '\0';
			justWhiteSpace = true;
		}
		void startPage() {
			pageStarted = true;
			firstLine = true;
			pageCount++;
			double fontAscender = fontToPoints(PDFfontAscenders[fontSet]);
			yPos = pageHeight - pageMargin.top - fontAscender;
			// start a new page
			sprintf(buffer, "BT 1 0 0 1 %d %d Tm\n",
			        pageMargin.left, (int)yPos);
			// force setting of initial font, colour
			setStyle(segStyle, -1);
			strcat(buffer, segStyle);
			pageData = buffer;
			xPos = pageMargin.left;
			segment.clear();
			flushSegment();
		}
		void endPage() {
			pageStarted = false;
			flushSegment();
			// build actual text object; +3 is for "ET\n"
			// PDF1.4Ref(p38) EOL marker preceding endstream not counted
			char *textObj = new char[pageData.length() + 100];
			// concatenate stream within the text object
			sprintf(textObj, "<</Length %d>>\nstream\n%s"
			        "ET\nendstream\n",
			        static_cast<int>(pageData.length() - 1 + 3),
			        pageData.c_str());
			oT->add(textObj);
			delete []textObj;
		}
		void nextLine() {
			if (!pageStarted) {
				startPage();
			}
			xPos = pageMargin.left;
			flushSegment();
			// PDF follows cartesian coords, subtract -> down
			yPos -= leading;
			double fontDescender = fontToPoints(PDFfontDescenders[fontSet]);
			if (yPos < pageMargin.bottom + fontDescender) {
				endPage();
				startPage();
				return;
			}
			if (firstLine) {
				// avoid breakage due to locale setting
				int f = (int)(leading * 10 + 0.5);
				sprintf(buffer, "0 -%d.%d TD\n", f / 10, f % 10);
				firstLine = false;
			} else {
				sprintf(buffer, "T*\n");
			}
			pageData += buffer;
		}
	};
	PDFRender pr;

	wEditor.Colourise(0, -1);
	// read exporter flags
	int tabSize = props.GetInt("tabsize", PDF_TAB_DEFAULT);
	if (tabSize < 0) {
		tabSize = PDF_TAB_DEFAULT;
	}
	// read magnification value to add to default screen font size
	pr.fontSize = props.GetInt("export.pdf.magnification");
	// set font family according to face name
	SString propItem = props.GetExpanded("export.pdf.font");
	pr.fontSet = PDF_FONT_DEFAULT;
	if (propItem.length()) {
		if (propItem == "Courier")
			pr.fontSet = 0;
		else if (propItem == "Helvetica")
			pr.fontSet = 1;
		else if (propItem == "Times")
			pr.fontSet = 2;
	}
	// page size: width, height
	propItem = props.GetExpanded("export.pdf.pagesize");
	char *buffer = new char[200];
	char *ps = StringDup(propItem.c_str());
	const char *next = GetNextPropItem(ps, buffer, 32);
	if (0 >= (pr.pageWidth = atol(buffer))) {
		pr.pageWidth = PDF_WIDTH_DEFAULT;
	}
	GetNextPropItem(next, buffer, 32);
	if (0 >= (pr.pageHeight = atol(buffer))) {
		pr.pageHeight = PDF_HEIGHT_DEFAULT;
	}
	delete []ps;
	// page margins: left, right, top, bottom
	propItem = props.GetExpanded("export.pdf.margins");
	ps = StringDup(propItem.c_str());
	next = GetNextPropItem(ps, buffer, 32);
	if (0 >= (pr.pageMargin.left = atol(buffer))) {
		pr.pageMargin.left = PDF_MARGIN_DEFAULT;
	}
	next = GetNextPropItem(next, buffer, 32);
	if (0 >= (pr.pageMargin.right = atol(buffer))) {
		pr.pageMargin.right = PDF_MARGIN_DEFAULT;
	}
	next = GetNextPropItem(next, buffer, 32);
	if (0 >= (pr.pageMargin.top = atol(buffer))) {
		pr.pageMargin.top = PDF_MARGIN_DEFAULT;
	}
	GetNextPropItem(next, buffer, 32);
	if (0 >= (pr.pageMargin.bottom = atol(buffer))) {
		pr.pageMargin.bottom = PDF_MARGIN_DEFAULT;
	}
	delete []ps;

	// collect all styles available for that 'language'
	// or the default style if no language is available...
	pr.style = new PDFStyle[STYLE_MAX + 1];
	for (int i = 0; i <= STYLE_MAX; i++) {	// get keys
		pr.style[i].font = 0;
		pr.style[i].fore[0] = '\0';

		sprintf(buffer, "style.*.%0d", i);
		char *valdef = StringDup(props.GetExpanded(buffer).c_str());
		sprintf(buffer, "style.%s.%0d", language.c_str(), i);
		char *val = StringDup(props.GetExpanded(buffer).c_str());

		StyleDefinition sd(valdef);
		sd.ParseStyleDefinition(val);

		if (sd.specified != StyleDefinition::sdNone) {
			if (sd.italics) { pr.style[i].font |= 2; }
			if (sd.bold) { pr.style[i].font |= 1; }
			if (sd.fore.length()) {
				getPDFRGB(pr.style[i].fore, sd.fore.c_str());
			} else if (i == STYLE_DEFAULT) {
				strcpy(pr.style[i].fore, "0 0 0 ");
			}
			// grab font size from default style
			if (i == STYLE_DEFAULT) {
				if (sd.size > 0)
					pr.fontSize += sd.size;
				else
					pr.fontSize = PDF_FONTSIZE_DEFAULT;
			}
		}
		delete []val;
		delete []valdef;
	}
	// patch in default foregrounds
	for (int j = 0; j <= STYLE_MAX; j++) {
		if (pr.style[j].fore[0] == '\0') {
			strcpy(pr.style[j].fore, pr.style[STYLE_DEFAULT].fore);
		}
	}
	delete []buffer;

	FILE *fp = saveName.Open(GUI_TEXT("wb"));
	if (!fp) {
		// couldn't open the file for saving, issue an error message
		extender->HildiAlarm("Could not save file\n'%1'",
			MB_OK | MB_ICONWARNING, filePath.AsInternal());
		return;
	}
	// initialise PDF rendering
	PDFObjectTracker ot(fp);
	pr.oT = &ot;
	pr.startPDF();

	// do here all the writing
	Sci_Position lengthDoc = LengthDocument();
	TextReader acc(wEditor);

	if (!lengthDoc) {	// enable zero length docs
		pr.nextLine();
	} else {
		int lineIndex = 0;
		for (int i = 0; i < lengthDoc; i++) {
			char ch = acc[i];
			int style = acc.StyleAt(i);

			if (ch == '\t') {
				// expand tabs
				int ts = tabSize - (lineIndex % tabSize);
				lineIndex += ts;
				for (; ts; ts--) {	// add ts count of spaces
					pr.add(' ', style);	// add spaces
				}
			} else if (ch == '\r' || ch == '\n') {
				if (ch == '\r' && acc[i + 1] == '\n') {
					i++;
				}
				// close and begin a newline...
				pr.nextLine();
				lineIndex = 0;
			} else {
				// write the character normally...
				pr.add(ch, style);
				lineIndex++;
			}
		}
	}
	// write required stuff and close the PDF file
	pr.endPDF();
	fclose(fp);
}


//---------- Save to TeX ----------

static char* getTexRGB(char* texcolor, const char* stylecolor) {
	//texcolor[rgb]{0,0.5,0}{....}
	double rf = IntFromHexByte(stylecolor + 1) / 256.0;
	double gf = IntFromHexByte(stylecolor + 3) / 256.0;
	double bf = IntFromHexByte(stylecolor + 5) / 256.0;
	// avoid breakage due to locale setting
	int r = (int)(rf * 10 + 0.5);
	int g = (int)(gf * 10 + 0.5);
	int b = (int)(bf * 10 + 0.5);
	sprintf(texcolor, "%d.%d, %d.%d, %d.%d", r / 10, r % 10, g / 10, g % 10, b / 10, b % 10);
	return texcolor;
}

#define CHARZ ('z' - 'b')
static char* texStyle(int style) {
	static char buf[10];
	int i = 0;
	do {
		buf[i++] = static_cast<char>('a' + (style % CHARZ));
		style /= CHARZ;
	} while ( style > 0 );
	buf[i] = 0;
	return buf;
}

static void defineTexStyle(StyleDefinition &style, FILE* fp, int istyle) {
	int closing_brackets = 2;
	char rgb[200];
	fprintf(fp, "\\newcommand{\\scite%s}[1]{\\noindent{\\ttfamily{", texStyle(istyle));
	if (style.italics) {
		fputs("\\textit{", fp);
		closing_brackets++;
	}
	if (style.bold) {
		fputs("\\textbf{", fp);
		closing_brackets++;
	}
	if (style.fore.length()) {
		fprintf(fp, "\\textcolor[rgb]{%s}{", getTexRGB(rgb, style.fore.c_str()) );
		closing_brackets++;
	}
	if (style.back.length()) {
		fprintf(fp, "\\colorbox[rgb]{%s}{", getTexRGB( rgb, style.back.c_str()) );
		closing_brackets++;
	}
	fputs("#1", fp);
	for (int i = 0; i <= closing_brackets; i++) {
		fputc( '}', fp );
	}
	fputc('\n', fp);
}

void SciTEBase::SaveToTEX(FilePath saveName) {
	wEditor.Colourise(0, -1);
	int tabSize = props.GetInt("tabsize");
	if (tabSize == 0)
		tabSize = 4;

	char key[200];
	Sci_Position lengthDoc = LengthDocument();
	TextReader acc(wEditor);
	bool styleIsUsed[STYLE_MAX + 1];

	int titleFullPath = props.GetInt("export.tex.title.fullpath", 0);

	int i;
	for (i = 0; i <= STYLE_MAX; i++) {
		styleIsUsed[i] = false;
	}
	for (i = 0; i < lengthDoc; i++) {	// check the used styles
		styleIsUsed[acc.StyleAt(i) & 0X7f] = true;
	}
	styleIsUsed[STYLE_DEFAULT] = true;

	FILE *fp = saveName.Open(GUI_TEXT("wt"));
	if (fp) {
		fputs("\\documentclass[a4paper]{article}\n"
		      "\\usepackage[a4paper,margin=2cm]{geometry}\n"
		      "\\usepackage[T1]{fontenc}\n"
		      "\\usepackage{color}\n"
		      "\\usepackage{alltt}\n"
 		      "\\usepackage{times}\n"
 		      "\\setlength{\\fboxsep}{0pt}\n", fp);

		for (i = 0; i < STYLE_MAX; i++) {      // get keys
			if (styleIsUsed[i]) {
				sprintf(key, "style.*.%0d", i);
				char *valdef = StringDup(props.GetExpanded(key).c_str());
				sprintf(key, "style.%s.%0d", language.c_str(), i);
				char *val = StringDup(props.GetExpanded(key).c_str());

				StyleDefinition sd(valdef); //check default properties
				sd.ParseStyleDefinition(val); //check language properties

				defineTexStyle(sd, fp, i); // writeout style macroses
				delete []val;
				delete []valdef;
			}
		}

		fputs("\\begin{document}\n\n", fp);
		fprintf(fp, "Source File: %s\n\n\\noindent\n\\small{\n",
		        static_cast<const char *>(titleFullPath ? filePath.AsUTF8().c_str() : filePath.Name().AsUTF8().c_str()));

		int styleCurrent = acc.StyleAt(0);

		fprintf(fp, "\\scite%s{", texStyle(styleCurrent));

		int lineIdx = 0;

		for (i = 0; i < lengthDoc; i++) { //here process each character of the document
			char ch = acc[i];
			int style = acc.StyleAt(i);

			if (style != styleCurrent) { //new style?
				fprintf(fp, "}\\scite%s{", texStyle(style) );
				styleCurrent = style;
			}

			switch ( ch ) { //write out current character.
			case '\t': {
					int ts = tabSize - (lineIdx % tabSize);
					lineIdx += ts - 1;
					fprintf(fp, "\\hspace*{%dem}", ts);
					break;
				}
			case '\\':
				fputs("{\\textbackslash}", fp);
				break;
			case '>':
			case '<':
			case '@':
				fprintf(fp, "$%c$", ch);
				break;
			case '{':
			case '}':
			case '^':
			case '_':
			case '&':
			case '$':
			case '#':
			case '%':
			case '~':
				fprintf(fp, "\\%c", ch);
				break;
			case '\r':
			case '\n':
				lineIdx = -1;	// Because incremented below
				if (ch == '\r' && acc[i + 1] == '\n')
					i++;	// Skip the LF
				styleCurrent = acc.StyleAt(i + 1);
				fprintf(fp, "} \\\\\n\\scite%s{", texStyle(styleCurrent) );
				break;
			case ' ':
				if (acc[i + 1] == ' ') {
					fputs("{\\hspace*{1em}}", fp);
				} else {
					fputc(' ', fp);
				}
				break;
			default:
				fputc(ch, fp);
			}
			lineIdx++;
		}
		fputs("}\n} %end small\n\n\\end{document}\n", fp); //close last empty style macros and document too
		fclose(fp);
	} else {
		extender->HildiAlarm("Could not save file\n'%1'",
			MB_OK | MB_ICONWARNING, filePath.AsInternal());
	}
}


//---------- Save to XML ----------

void SciTEBase::SaveToXML(FilePath saveName) {

	// Author: Hans Hagen / PRAGMA ADE / www.pragma-ade.com
	// Version: 1.0 / august 18, 2003
	// Remark: for a suitable style, see ConTeXt (future) distributions

	// The idea is that one can use whole files, or ranges of lines in manuals
	// and alike. Since ConTeXt can handle XML files, it's quite convenient to
	// use this format instead of raw TeX, although the output would not look
	// much different in structure.

	// We don't put style definitions in here since the main document will in
	// most cases determine the look and feel. This way we have full control over
	// the layout. The type attribute will hold the current lexer value.

	// <document>            : the whole thing
	// <data>                : reserved for metadata
	// <text>                : the main bodyof text
	// <line n-'number'>     : a line of text

	// <t n='number'>...<t/> : tag
	// <s n='number'/>       : space
	// <g/>                  : >
	// <l/>                  : <
	// <a/>                  : &
	// <h/>                  : #

	// We don't use entities, but empty elements for special characters
	// but will eventually use utf-8 (once i know how to get them out).

	wEditor.Colourise(0, -1);

	int tabSize = props.GetInt("tabsize");
	if (tabSize == 0) {
		tabSize = 4;
	}

	Sci_Position lengthDoc = LengthDocument();

	TextReader acc(wEditor);

	FILE *fp = saveName.Open(GUI_TEXT("wt"));

	if (fp) {

		bool collapseSpaces = (props.GetInt("export.xml.collapse.spaces", 1) == 1);
		bool collapseLines  = (props.GetInt("export.xml.collapse.lines", 1) == 1);

		fprintf(fp, "<?xml version='1.0' encoding='%s'?>\n", (codePage == SC_CP_UTF8) ? "utf-8" : "ascii");

		fputs("<document xmlns='http://www.scintila.org/scite.rng'", fp);
		fprintf(fp, " filename='%s'",
		        static_cast<const char *>(filePath.Name().AsUTF8().c_str()));
		fprintf(fp, " type='%s'", "unknown");
		fprintf(fp, " version='%s'", "1.0");
		fputs(">\n", fp);

		fputs("<data comment='This element is reserved for future usage.'/>\n", fp);

		fputs("<text>\n", fp);

		int styleCurrent = -1; // acc.StyleAt(0);
		int lineNumber = 1;
		int lineIndex = 0;
		bool styleDone = false;
		bool lineDone = false;
		bool charDone = false;
		int styleNew = -1;
		int spaceLen = 0;
		int emptyLines = 0;

		for (int i = 0; i < lengthDoc; i++) {
			char ch = acc[i];
			int style = acc.StyleAt(i);
			if (style != styleCurrent) {
				styleCurrent = style;
				styleNew = style;
			}
			if (ch == ' ') {
				spaceLen++;
			} else if (ch == '\t') {
				int ts = tabSize - (lineIndex % tabSize);
				lineIndex += ts - 1;
				spaceLen += ts;
			} else if (ch == '\f') {
				// ignore this animal
			} else if (ch == '\r' || ch == '\n') {
				if (ch == '\r' && acc[i + 1] == '\n') {
					i++;
				}
				if (styleDone) {
					fputs("</t>", fp);
					styleDone = false;
				}
				lineIndex = -1;
				if (lineDone) {
					fputs("</line>\n", fp);
					lineDone = false;
				} else if (collapseLines) {
					emptyLines++;
				} else {
					fprintf(fp, "<line n='%d'/>\n", lineNumber);
				}
				charDone = false;
				lineNumber++;
				styleCurrent = -1; // acc.StyleAt(i + 1);
			} else {
				if (collapseLines && (emptyLines > 0)) {
					fputs("<line/>\n", fp);
				}
				emptyLines = 0;
				if (! lineDone) {
					fprintf(fp, "<line n='%d'>", lineNumber);
					lineDone = true;
				}
				if (styleNew >= 0) {
					if (styleDone) { fputs("</t>", fp); }
				}
				if (! collapseSpaces) {
					while (spaceLen > 0) {
						fputs("<s/>", fp);
						spaceLen--;
					}
				} else if (spaceLen == 1) {
					fputs("<s/>", fp);
					spaceLen = 0;
				} else if (spaceLen > 1) {
					fprintf(fp, "<s n='%d'/>", spaceLen);
					spaceLen = 0;
				}
				if (styleNew >= 0) {
					fprintf(fp, "<t n='%d'>", style);
					styleNew = -1;
					styleDone = true;
				}
				switch (ch) {
				case '>' :
					fputs("<g/>", fp);
					break;
				case '<' :
					fputs("<l/>", fp);
					break;
				case '&' :
					fputs("<a/>", fp);
					break;
				case '#' :
					fputs("<h/>", fp);
					break;
				default  :
					fputc(ch, fp);
				}
				charDone = true;
			}
			lineIndex++;
		}
		if (styleDone) {
			fputs("</t>", fp);
		}
		if (lineDone) {
			fputs("</line>\n", fp);
		}
		if (charDone) {
			// no last empty line: fprintf(fp, "<line n='%d'/>", lineNumber);
		}

		fputs("</text>\n", fp);
		fputs("</document>\n", fp);

		fclose(fp);
	} else {
		extender->HildiAlarm("Could not save file\n'%1'",
			MB_OK | MB_ICONWARNING, filePath.AsInternal());
	}
}
