// SciTE - Scintilla based Text Editor
/** @file SciTEProps.cxx
 ** Properties management.
 **/
// Copyright 1998-2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <valarray>
#include <cmath>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <map>

#include "ILexer.h"

#include "Scintilla.h"
#include "SciLexer.h"
#include "Lexilla.h"
#include "../Access/LexillaAccess.h"

#include "GUI.h"

#if defined(GTK)

#include <unistd.h>
#include <gtk/gtk.h>

const GUI::gui_char menuAccessIndicator[] = GUI_TEXT("_");

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

const GUI::gui_char menuAccessIndicator[] = GUI_TEXT("&");
//#include "platform.h" //!-add-[StyleDefault]

#endif

#include "SString.h"
#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "SciTE.h"
#include "IFaceTable.h"
#include "Mutex.h"
#include "JobQueue.h"
#include "SciTEBase.h"

using namespace Scintilla;

const GUI::gui_char propLocalFileName[] = GUI_TEXT("SciTE.properties");
const GUI::gui_char propDirectoryFileName[] = GUI_TEXT("SciTEDirectory.properties");

/**
Read global and user properties files.
*/
void SciTEBase::ReadGlobalPropFile(GUI::gui_string adv) {
#ifdef __unix__
	extern char **environ;
	char **e=environ;
#else
	char **e=_environ;
#endif
	for (; e && *e; e++) {
		char key[1024];
		char *k=*e;
		char *v=strchr(k,'=');
		if (v && (static_cast<size_t>(v-k) < sizeof(key))) {
			memcpy(key, k, v-k);
			key[v-k] = '\0';
			propsEmbed.Set(key, v+1);
		}
	}

	for (int stackPos = 0; stackPos < importMax; stackPos++) {
		importFiles[stackPos] = GUI_TEXT("");
	}

	propsBase.Clear();

//!-start-[scite.userhome]
	FilePath homepath = GetSciteDefaultHome();
	props.Set("SciteDefaultHome", homepath.AsUTF8().c_str());
	homepath = GetSciteUserHome();
	props.Set("SciteUserHome", homepath.AsUTF8().c_str());
//!-end-[scite.userhome]

	FilePath propfileBase = GetDefaultPropertiesFileName();	
	//�������� ���������������� ����� - � ��������� ������� ����������
	propsBase.Read(GetSciteDefaultHome().AsUTF8().c_str(), propfileBase, propfileBase.Directory(), importFiles, importMax);
	if (adv != L"") {
		FilePath fAdv = adv; 
		propsBase.Read(fAdv, fAdv.Directory(), importFiles, importMax);
	}

	propsUser.Clear();
	FilePath propfileUser = GetUserPropertiesFileName();
	propsUser.Read(propfileUser, propfileUser.Directory(), importFiles, importMax);

}

int IntFromHexDigit(int ch) {
	if ((ch >= '0') && (ch <= '9')) {
		return ch - '0';
	} else if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	} else if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	} else {
		return 0;
	}
}

int IntFromHexByte(const char *hexByte) {
	return IntFromHexDigit(hexByte[0]) * 16 + IntFromHexDigit(hexByte[1]);
}

static Scintilla::Colour ColourFromString(const SString &s) {
	if (s.length()) {
		int r = IntFromHexByte(s.c_str() + 1);
		int g = IntFromHexByte(s.c_str() + 3);
		int b = IntFromHexByte(s.c_str() + 5);
		return ColourRGB(r, g, b);
	} else {
		return 0;
	}
}

Scintilla::Colour SciTEBase::ColourOfProperty(const char *key, Scintilla::Colour colourDefault, bool invClr) {
	SString colour = props.GetExpanded(key);
	if (colour.length()) {
		colourDefault = ColourFromString(colour);
	}
	if (invClr)
		colourDefault = convMain.Convert(colourDefault);
	return colourDefault;
}

float clr_brightness(long clr) {
	return static_cast<float>((((clr & 0x0000FF) * 299.) + (((clr & 0x00FF00) >> 8) * 587.) + (((clr & 0xFF0000) >> 16) * 114.)) / 1000.);
}

void rgb2lab(float R, float G, float B, float & l_s, float &a_s, float &b_s) {
	double var_R = R / 255.0;
	double var_G = G / 255.0;
	double var_B = B / 255.0;
	if (var_R > 0.04045) var_R = pow(((var_R + 0.055) / 1.055), 2.4);
	else                   var_R = var_R / 12.92;
	if (var_G > 0.04045) var_G = pow(((var_G + 0.055) / 1.055), 2.4);
	else                   var_G = var_G / 12.92;
	if (var_B > 0.04045) var_B = pow(((var_B + 0.055) / 1.055), 2.4);
	else                   var_B = var_B / 12.92;
	var_R = var_R * 100.;
	var_G = var_G * 100.;
	var_B = var_B * 100.;
	//Observer. = 2�, Illuminant = D65
	double X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
	double Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
	double Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
	double var_X = X / 95.047;         //ref_X =  95.047   Observer= 2�, Illuminant= D65
	double var_Y = Y / 100.000;          //ref_Y = 100.000
	double var_Z = Z / 108.883;          //ref_Z = 108.883
	if (var_X > 0.008856) var_X = pow(var_X, (1. / 3.));
	else                    var_X = (7.787 * var_X) + (16. / 116.);
	if (var_Y > 0.008856) var_Y = pow(var_Y, (1. / 3.));
	else                    var_Y = (7.787 * var_Y) + (16. / 116.);
	if (var_Z > 0.008856) var_Z = pow(var_Z, (1. / 3.));
	else                    var_Z = (7.787 * var_Z) + (16. / 116.);
	double l_sd = (116. * var_Y) - 16.;
	double a_sd = 500. * (var_X - var_Y);
	double b_sd = 200. * (var_Y - var_Z);

	l_s = static_cast<float>(l_sd < 0. ? 0. : l_sd > 100. ? 100. : l_sd);
	a_s = static_cast<float>(a_sd < -128. ? -128. : a_sd > 128. ? 128 : a_sd);
	b_s = static_cast<float>(b_sd < -128. ? -128. : b_sd > 128. ? 128 : b_sd);
}

void lab2rgb(float l_s, float a_s, float b_s, float& R, float& G, float& B) {
	double var_Y = (l_s + 16.) / 116.;
	double var_X = a_s / 500. + var_Y;
	double var_Z = var_Y - b_s / 200.;
	if (pow(var_Y, 3) > 0.008856) var_Y = pow(var_Y, 3);
	else                      var_Y = (var_Y - 16. / 116.) / 7.787;
	if (pow(var_X, 3) > 0.008856) var_X = pow(var_X, 3);
	else                      var_X = (var_X - 16. / 116.) / 7.787;
	if (pow(var_Z, 3) > 0.008856) var_Z = pow(var_Z, 3);
	else                      var_Z = (var_Z - 16. / 116.) / 7.787;
	double X = 95.047 * var_X;    //ref_X =  95.047     Observer= 2�, Illuminant= D65
	double Y = 100.000 * var_Y;   //ref_Y = 100.000
	double Z = 108.883 * var_Z;    //ref_Z = 108.883
	var_X = X / 100.;       //X from 0 to  95.047      (Observer = 2�, Illuminant = D65)
	var_Y = Y / 100.;       //Y from 0 to 100.000
	var_Z = Z / 100.;      //Z from 0 to 108.883
	double var_R = var_X * 3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
	double var_G = var_X * -0.9689 + var_Y * 1.8758 + var_Z * 0.0415;
	double var_B = var_X * 0.0557 + var_Y * -0.2040 + var_Z * 1.0570;
	if (var_R > 0.0031308) var_R = 1.055 * pow(var_R, (1 / 2.4)) - 0.055;
	else                     var_R = 12.92 * var_R;
	if (var_G > 0.0031308) var_G = 1.055 * pow(var_G, (1 / 2.4)) - 0.055;
	else                     var_G = 12.92 * var_G;
	if (var_B > 0.0031308) var_B = 1.055 * pow(var_B, (1 / 2.4)) - 0.055;
	else                     var_B = 12.92 * var_B;
	double Rd = var_R * 255.;
	double Gd = var_G * 255.;
	double Bd = var_B * 255.;
	R = static_cast<float>(Rd > 255 ? 255 : Rd < 0 ? 0 : Rd);
	G = static_cast<float>(Gd > 255 ? 255 : Gd < 0 ? 0 : Gd);
	B = static_cast<float>(Bd > 255 ? 255 : Bd < 0 ? 0 : Bd);
}

void ColorConvertorLAB::Init(const char *points, ExtensionAPI *h) {
	if (inicialized && prevPoints == points) {
		return;
	}
	prevPoints = "";
	inicialized = false;
	std::string v = points; 
	//std::regex re = std::regex("\\(\\d+", std::regex::ECMAScript);
	std::regex re = std::regex(" *\\( *(\\d+\\.?\\d*) *, *(\\d+\\.?\\d*) *\\) *", std::regex::ECMAScript);
	std::smatch mtch;
	//bool b = std::regex_search(v, mtch, re);
	nPoints = -1;
	//int tt = mtch[0].length();
	
	for (int i = 0; std::regex_search(v, mtch, re) && mtch.length(); i++, v.erase(0, mtch[0].length())) {
		if (i > CONVERTORLAB_MAXPOINTS) {
			h->Trace("Too many points\n");
			return;
		}
		nPoints = i;
		m_x[i] = static_cast<float>(atof(mtch[1].str().c_str()));
		m_y[i] = static_cast<float>(atof(mtch[2].str().c_str()));
		if (i && m_x[i] <= m_x[i - 1]) {
			h->Trace("Incorrect points order\n");
			return;
		}
		if (m_x[i] > 100. || m_y[i] > 100.) {
			h->Trace("Incorrect points sizes\n");
			return;
		}
	}
	if (nPoints < 0) {
		if (v.length() > 0)
			h->Trace("Points not found\n");
		return;
	}
	if (v.length()) {
		h->Trace("Incorrect points format\n");
		return;
	}

	for (int i = 0; i < nPoints - 1; ++i) {
		m_k[i] = (m_y[i + 1] - m_y[i]) / (m_x[i + 1] - m_x[i]);
		m_b[i] = m_y[i] - m_k[i] * m_x[i];
	}
	inicialized = true;
	prevPoints = points;
}
Scintilla::Colour ColorConvertorLAB::Convert(Scintilla::Colour colorIn) {
	if(!inicialized)
		return colorIn;

	float R = static_cast<float>(colorIn & 0x0000FF);
	float G = static_cast<float>((colorIn & 0x00FF00) >> 8);
	float B = static_cast<float>((colorIn & 0xFF0000) >> 16);

	float L, a_s, b_s;

	rgb2lab(R, G, B, L, a_s, b_s);
	
	float L2;
	if (L <= m_x[0]){
		L2 = m_y[0];
	}
	else if (L >= m_x[nPoints]){
		L2 = m_y[nPoints];
	}
	else{
		L2 = m_y[nPoints];
		for (int i = 0; i <= nPoints - 1; ++i){
			if (L >= m_x[i] && L < m_x[i + 1]) {
				L2 = L * m_k[i] + m_b[i];
				break;
			}
		}
    }
	lab2rgb(L2, a_s, b_s, R, G, B);

	return ColourRGB(static_cast<int>(R), static_cast<int>(G), static_cast<int>(B));
}

/**
 * Put the next property item from the given property string
 * into the buffer pointed by @a pPropItem.
 * @return NULL if the end of the list is met, else, it points to the next item.
 */
const char *SciTEBase::GetNextPropItem(
	const char *pStart,	/**< the property string to parse for the first call,
						 * pointer returned by the previous call for the following. */
	char *pPropItem,	///< pointer on a buffer receiving the requested prop item
	int maxLen)			///< size of the above buffer
{
	sptr_t size = maxLen - 1;

	*pPropItem = '\0';
	if (pStart == NULL) {
		return NULL;
	}
	const char *pNext = strchr(pStart, ',');
	if (pNext) {	// Separator is found
		if (size > pNext - pStart) {
			// Found string fits in buffer
			size = pNext - pStart;
		}
		pNext++;
	}
	strncpy(pPropItem, pStart, size);
	pPropItem[size] = '\0';
	return pNext;
}

StyleDefinition::StyleDefinition(const char *definition, ColorConvertor * pc, bool useConv) :
	//!		size(0), fore("#000000"), back("#FFFFFF"),
	size(0), fore(""), back(""), //!-change-[StyleDefault]
	bold(false), italics(false), eolfilled(false), underlined(false),
	caseForce(SC_CASE_MIXED),
	visible(true), changeable(true), pConvertor(pc), invertColors(useConv),
//!		specified(sdNone) {
		hotspot(false), specified(sdNone) { //!-change-[StyleDefHotspot]
	ParseStyleDefinition(definition);
}

bool StyleDefinition::ParseStyleDefinition(const char *definition) {
	if (definition == NULL || *definition == '\0') {
		return false;
	}
	char *val = StringDup(definition);
	char *opt = val;
	while (opt) {
		// Find attribute separator
		char *cpComma = strchr(opt, ',');
		if (cpComma) {
			// If found, we terminate the current attribute (opt) string
			*cpComma = '\0';
		}
		// Find attribute name/value separator
		char *colon = strchr(opt, ':');
		if (colon) {
			// If found, we terminate the current attribute name and point on the value
			*colon++ = '\0';
		}
		if (0 == strcmp(opt, "italics")) {
			specified = static_cast<flags>(specified | sdItalics);
			italics = true;
		}
		if (0 == strcmp(opt, "notitalics")) {
			specified = static_cast<flags>(specified | sdItalics);
			italics = false;
		}
		if (0 == strcmp(opt, "bold")) {
			specified = static_cast<flags>(specified | sdBold);
			bold = true;
		}
		if (0 == strcmp(opt, "notbold")) {
			specified = static_cast<flags>(specified | sdBold);
			bold = false;
		}
		if (0 == strcmp(opt, "font")) {
			specified = static_cast<flags>(specified | sdFont);
			font = colon;
			font.substitute('|', ',');
		}
		if (0 == strcmp(opt, "fore")) {
			specified = static_cast<flags>(specified | sdFore);
			fore = colon;
		}
		if (0 == strcmp(opt, "back")) {
			specified = static_cast<flags>(specified | sdBack);
			back = colon;
		}
		if ((0 == strcmp(opt, "size")) && colon) {
			specified = static_cast<flags>(specified | sdSize);
			size = atoi(colon);
		}
		if (0 == strcmp(opt, "eolfilled")) {
			specified = static_cast<flags>(specified | sdEOLFilled);
			eolfilled = true;
		}
		if (0 == strcmp(opt, "noteolfilled")) {
			specified = static_cast<flags>(specified | sdEOLFilled);
			eolfilled = false;
		}
		if (0 == strcmp(opt, "underlined")) {
			specified = static_cast<flags>(specified | sdUnderlined);
			underlined = true;
		}
		if (0 == strcmp(opt, "notunderlined")) {
			specified = static_cast<flags>(specified | sdUnderlined);
			underlined = false;
		}
		if (0 == strcmp(opt, "case")) {
			specified = static_cast<flags>(specified | sdCaseForce);
			caseForce = SC_CASE_MIXED;
			if (colon) {
				if (*colon == 'u')
					caseForce = SC_CASE_UPPER;
				else if (*colon == 'l')
					caseForce = SC_CASE_LOWER;
			}
		}
		if (0 == strcmp(opt, "visible")) {
			specified = static_cast<flags>(specified | sdVisible);
			visible = true;
		}
		if (0 == strcmp(opt, "notvisible")) {
			specified = static_cast<flags>(specified | sdVisible);
			visible = false;
		}
		if (0 == strcmp(opt, "changeable")) {
			specified = static_cast<flags>(specified | sdChangeable);
			changeable = true;
		}
		if (0 == strcmp(opt, "notchangeable")) {
			specified = static_cast<flags>(specified | sdChangeable);
			changeable = false;
		}
//!-start-[StyleDefHotspot]
		if (0 == strcmp(opt, "hotspot")) {
			specified = static_cast<flags>(specified | sdHotspot);
			hotspot = true;
		}
		if (0 == strcmp(opt, "nothotspot")) {
			specified = static_cast<flags>(specified | sdHotspot);
			hotspot = false;
		}
//!-end-[StyleDefHotspot]
		if (cpComma)
			opt = cpComma + 1;
		else
			opt = 0;
	}
	delete []val;
	return true;
}

Scintilla::Colour invertColor2(Scintilla::Colour clr) {
	if (!clr)
		return 0xFFFFFF;
	if (clr == 0xFFFFFF)
		return 0;

	float R = static_cast<float>(clr & 0x0000FF);
	float G = static_cast<float>((clr & 0x00FF00) >> 8);
	float B = static_cast<float>((clr & 0xFF0000) >> 16);

	float l_s, a_s, b_s;

	rgb2lab(R, G, B, l_s, a_s, b_s);

	//if (isBg) {
	//	// L = ((L - 100) / 100.) ^ 2 * 100
	//	l_s = 100. - (pow((l_s / 100.), 5) * 100);
	//} else {
	//	//l_s = 100 - (pow(l_s / 100., 2) * 100);
	//	l_s = 70. - ((l_s - 43.) / 43.) *((l_s - 43.) / 43.) *((l_s - 43.) / 43.) * 30.;
	//}
	l_s = static_cast<float>(l_s / 100.);
	//l_s = (-lN*lN*lN + lN*lN - lN + 1) * 100;
	//l_s = (-2.5*lN*lN*lN + 2.5*lN*lN - lN + 1) * 100;
	//l_s = (-2.8*lN*lN*lN + 2.6*lN*lN - 0.8*lN + 1) * 100;
	//l_s = (4*(1-lN)*lN*lN*lN - lN + 1) * 100;
	l_s = static_cast<float>(((2-2.3*l_s)*l_s*l_s*l_s - 0.7*l_s + 1) * 100);




	//l_s = 50 - ((l_s - 50) / 50.) *((l_s - 50) / 50.) *((l_s - 50) / 50.) * 50;
	//l_s = 70. - ((l_s - 43.) / 43.) *((l_s - 43.) / 43.) *((l_s - 43.) / 43.) * 30.;
	//l_s = 80. - ((l_s - 39.) / 39.) *((l_s - 39.) / 39.) *((l_s - 39.) / 39.) * 20.;
	//l_s = 30. - ((l_s - 57.) / 57.) *((l_s - 57.) / 57.) *((l_s - 57.) / 57.) * 70.;

	//l_s = 100 - l_s;

	//l_s = 50 * (1 + cos(3.14159265358979323846 * l_s* 0.01));
	////////////////
	lab2rgb(l_s, a_s, b_s, R, G, B);

	return ColourRGB(static_cast<unsigned int>(R), static_cast<unsigned int>(G), static_cast<unsigned int>(B));
}

//Colour ColourRGBOrInvert(unsigned int red, unsigned int green, unsigned int blue, bool invert, bool isBG) {
//	if(!invert)
//		return ColourRGB(red, green, blue);
//}

long StyleDefinition::ForeAsLong(bool useInv) const {
	long l = ColourFromString(fore);
	if (invertColors && useInv)
		l = pConvertor->Convert(l);
	return l;
}

long StyleDefinition::BackAsLong(bool useInv) const {
	long l = ColourFromString(back);
	if (invertColors && useInv)
		l = pConvertor->Convert(l);
	return l;
}


void SciTEBase::SetOneStyle(GUI::ScintillaWindow &win, int style, const StyleDefinition &sd) {
	bool bSetClr = true;
	if (style == STYLE_LINENUMBER) {
		Scintilla::Colour fore, back;

		int needRecalc = 0;
		if (!(sd.specified & StyleDefinition::sdFore)) {
			fore = layout.GetColorRef("FGCOLOR");
			needRecalc = 1;
		} else {
			fore = sd.ForeAsLong(false);
		}
		if (!(sd.specified & StyleDefinition::sdBack)) {
			back = layout.GetColorRef("SCR_BACKCOLOR");
			needRecalc += 1;
		} else {
			back = sd.BackAsLong(false);
		}
		if (needRecalc >= 1) {
			//float l_s, a_s, b_s;
			//rgb2lab((back & 0x0000FF), ((back & 0x00FF00) >> 8), ((back & 0xFF0000) >> 16), l_s, a_s, b_s);


			float bBack = clr_brightness(back);
			float bFore = clr_brightness(fore);
			if ((bFore > 128 && bBack > 128) || (bFore < 128 && bBack < 128))
				fore = convMain.Convert(fore);

		} else if (invertColors && needRecalc == 0) {
			fore = convMain.Convert(fore);
			back = convMain.Convert(back);
		}

		//LineNumberColors(fore, back, sd);
		win.StyleSetFore(style, fore);
		win.StyleSetBack(style, back);
		int iMrg = win.Margins();
		for (int i = 1; i < iMrg; i++) {
			win.SetMarginBackN(i, back);
		}
		bSetClr = false;
	} else if (style == STYLE_CALLTIP) {
		Scintilla::Colour fore, back;

		int needRecalc = 0;
		if (!(sd.specified & StyleDefinition::sdFore)) {
			fore = layout.GetColorRef("TIPFGCOLOR");
			needRecalc = 1;
		} else {
			fore = sd.ForeAsLong();
		}
		if (!(sd.specified & StyleDefinition::sdBack)) {
			back = layout.GetColorRef("TIPBGCOLOR");
			needRecalc +=1;
		} else {
			back = sd.BackAsLong();
		}
		if (needRecalc == 1) {
			//float l_s, a_s, b_s;
			//rgb2lab((back & 0x0000FF), ((back & 0x00FF00) >> 8), ((back & 0xFF0000) >> 16), l_s, a_s, b_s);


			float bBack = clr_brightness(back);
			float bFore = clr_brightness(fore);
			if ((bFore > 128 && bBack > 128) || (bFore < 128 && bBack < 128))
				fore = convMain.Convert(fore);

		} else if (invertColors && needRecalc == 0) {
			fore = convMain.Convert(fore);
			back = convMain.Convert(back);
		}

		//LineNumberColors(fore, back, sd);
		win.StyleSetFore(style, fore);
		win.StyleSetBack(style, back);

		bSetClr = false;
	}
	
	if (sd.specified & StyleDefinition::sdItalics)
		win.StyleSetItalic(style, sd.italics ? 1 : 0);
	if (sd.specified & StyleDefinition::sdBold)
		win.StyleSetBold(style, sd.bold ? 1 : 0);
	if (sd.specified & StyleDefinition::sdFont)
		win.StyleSetFont(style,sd.font.c_str());
	if (bSetClr && (sd.specified & StyleDefinition::sdFore))
		win.StyleSetFore(style, sd.ForeAsLong());
	if (bSetClr && (sd.specified & StyleDefinition::sdBack))
		win.StyleSetBack(style, sd.BackAsLong());
	if (sd.specified & StyleDefinition::sdSize)
		win.StyleSetSize(style, sd.size);
	if (sd.specified & StyleDefinition::sdEOLFilled)
		win.StyleSetEOLFilled(style, sd.eolfilled ? 1 : 0);
	if (sd.specified & StyleDefinition::sdUnderlined)
		win.StyleSetUnderline(style, sd.underlined ? 1 : 0);
	if (sd.specified & StyleDefinition::sdCaseForce)
		win.StyleSetCase(style, static_cast<CaseVisible>(sd.caseForce));
	if (sd.specified & StyleDefinition::sdVisible) {
		props.SetInteger("hidden.styles.found", 1);
		if (hideHiddenStyles) {
			win.StyleSetVisible(style, sd.visible ? 1 : 0);
		}
	}
	if (sd.specified & StyleDefinition::sdChangeable)
		win.StyleSetChangeable(style, sd.changeable ? 1 : 0);
	if (sd.specified & StyleDefinition::sdHotspot)
		win.StyleSetHotSpot(style, sd.hotspot ? 1 : 0);
	win.StyleSetCharacterSet(style, static_cast<CharacterSet>(characterSet));
}

void SciTEBase::SetStyleFor(GUI::ScintillaWindow &win, const char *lang) {
	if(win.GetID()==wEditor.GetID())
		props.SetInteger("hidden.styles.found", 0);
	for (int style = 0; style <= STYLE_MAX; style++) {
		if (style != STYLE_DEFAULT) {
			char key[200];
			sprintf(key, "style.%s.%0d", lang, style);
			SString sval = props.GetExpanded(key);
			if (sval.length()) {
				StyleDefinition sd(sval.c_str(), &convMain, invertColors);
				SetOneStyle(win, style, sd);
			}
		}
	}
}

void LowerCaseString(char *s) {
	while (*s) {
		if ((*s >= 'A') && (*s <= 'Z')) {
			*s = static_cast<char>(*s - 'A' + 'a');
		}
		s++;
	}
}

SString SciTEBase::ExtensionFileName() {
	if (CurrentBuffer()->overrideExtension.length()) {
		return CurrentBuffer()->overrideExtension;
	} else {
		FilePath name = FileNameExt();
		if (name.IsSet()) {
			// Force extension to lower case
			char fileNameWithLowerCaseExtension[MAX_PATH];
			strcpy(fileNameWithLowerCaseExtension, name.AsUTF8().c_str());
#if !defined(GTK)
			char *extension = strrchr(fileNameWithLowerCaseExtension, '.');
			if (extension) {
				LowerCaseString(extension);
			}
#endif
			return SString(fileNameWithLowerCaseExtension);
		} else {
			return props.Get("default.file.ext");
		}
	}
}

void SciTEBase::ForwardPropertyToEditor(const char *key) {
	SString value;
	if (*key == '$')
		value = props.GetNewExpand(key + 1, ExtensionFileName().c_str());
	else
		value = props.GetExpanded(key);
	wEditor.SetProperty(key, value.c_str());
	wOutput.SetProperty(key, value.c_str());
	wFindRes.SetProperty(key, value.c_str());
}

void SciTEBase::SetFoldingMarkers(bool main) {
	Scintilla::Colour fore = layout.GetColorRef("SCR_FORECOLOR");  //props.GetInt("invert.lexer.colors") ? ColourRGB(0x80, 0x80, 0x80) : ColourRGB(0xff, 0xff, 0xff);
	Scintilla::Colour back = layout.GetColorRef("FGCOLOR"); //props.GetInt("invert.lexer.colors") ? ColourRGB(0xff, 0xff, 0xff) : ColourRGB(0x80, 0x80, 0x80);
	
	float brBack = clr_brightness(back);
	float brFore = clr_brightness(fore);
	if ((brFore > 128 && brBack > 128) || (brFore < 128 && brBack < 128))
		back = convMain.Convert(layout.GetColorRef("SCR_BACKCOLOR"));
	int sym = main ? props.GetInt("fold.symbols") : props.GetInt("fold.symbols.findes", 1);
	switch (sym) {
	case 0:
		// Arrow pointing right for contracted folders, arrow pointing down for expanded
		DefineMarker(main, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROWDOWN, back, back);
		DefineMarker(main, SC_MARKNUM_FOLDER, SC_MARK_ARROW, back, back);
		DefineMarker(main, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY, back, back);
		DefineMarker(main, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY, back, back);
		DefineMarker(main, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY, fore, back);
		break;
	case 1:
		// Plus for contracted folders, minus for expanded
		DefineMarker(main, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDER, SC_MARK_PLUS, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY, fore, back);
		break;
	case 2:
		// Like a flattened tree control using circular headers and curved joins
		DefineMarker(main, SC_MARKNUM_FOLDEROPEN, SC_MARK_CIRCLEMINUS, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEREND, SC_MARK_CIRCLEPLUSCONNECTED, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE, fore, back);
		break;
	case 3:
		// Like a flattened tree control using square headers		
		DefineMarker(main, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED, fore, back);
		DefineMarker(main, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER, fore, back);
		break;
	}
	if(main)
		DefineMarker(main, markerHideLines, SC_MARK_ARROWDOWN, back, fore);
}

void SciTEBase::DefineMarker(bool main, int marker, int markerType, Scintilla::Colour fore, Scintilla::Colour back) {
	if (main) {
		wEditor.MarkerDefine(marker,static_cast<MarkerSymbol>(markerType));
		wEditor.MarkerSetFore(marker, fore);
		wEditor.MarkerSetBack(marker, back);
	} else {
		wOutput.MarkerDefine(marker, static_cast<MarkerSymbol>(markerType));
		wOutput.MarkerSetFore(marker, fore);
		wOutput.MarkerSetBack(marker, back);
		wFindRes.MarkerDefine(marker, static_cast<MarkerSymbol>(markerType));
		wFindRes.MarkerSetFore(marker, fore);
		wFindRes.MarkerSetBack(marker, back);
	}
}

static int FileLength(const char *path) {
	int len = 0;
	FILE *fp = fopen(path, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fclose(fp);
	}
	return len;
}

SString SciTEBase::FindLanguageProperty(const char *pattern, const char *defaultValue) {
	SString key = pattern;
	key.substitute("*", language.c_str());
	SString ret = props.GetExpanded(key.c_str());
	if (ret == "")
		ret = props.GetExpanded(pattern);
	if (ret == "")
		ret = defaultValue;
	return ret;
}

//!-start-[BetterCalltips]
int SciTEBase::FindIntLanguageProperty(const char *pattern, int defaultValue /*=0*/) {
	SString key = pattern;
	key.substitute("*", language.c_str());
	SString val = props.GetExpanded(key.c_str());
	if (val == "") {
		val = props.GetExpanded(pattern);
	}
	if (val == "") {
		return defaultValue;
	}
	return val.value();
}
//!-end-[BetterCalltips]

/**
 * A list of all the properties that should be forwarded to Scintilla lexers.
 */
static const char *propertiesToForward[] = {
	"fold.lpeg.by.indentation",
	"lexer.lpeg.color.theme",
	"lexer.lpeg.home",
	"lexer.lpeg.script",
//++Autogenerated -- run src/LexGen.py to regenerate
//**\(\t"\*",\n\)
	"asp.default.language",
	"fold",
	"fold.at.else",
	"fold.comment",
	"fold.comment.nimrod",
	"fold.comment.yaml",
	"fold.compact",
	"fold.cpp.comment.explicit",
	"fold.directive",
	"fold.html",
	"fold.html.preprocessor",
	"fold.hypertext.comment",
	"fold.hypertext.heredoc",
	"fold.perl.package",
	"fold.perl.pod",
	"fold.preprocessor",
	"fold.quotes.nimrod",
	"fold.quotes.python",
	"fold.sql.exists",
	"fold.sql.only.begin",
	"fold.symbols",
	"fold.verilog.flags",
	"html.tags.case.sensitive",
	"lexer.batch.enabledelayedexpansion",
	"lexer.caml.magic",
	"lexer.cpp.allow.dollars",
	"lexer.cpp.track.preprocessor",
	"lexer.cpp.update.preprocessor",
	"lexer.d.fold.at.else",
	"lexer.errorlist.value.separate",
	"lexer.flagship.styling.within.preprocessor",
	"lexer.forth.no.interpretation",
	"lexer.html.django",
	"lexer.html.mako",
	"lexer.metapost.comment.process",
	"lexer.metapost.interface.default",
	"lexer.pascal.smart.highlighting",
	"lexer.props.allow.initial.spaces",
	"lexer.python.literals.binary",
	"lexer.python.strings.b",
	"lexer.python.strings.over.newline",
	"lexer.python.strings.u",
	"lexer.sql.backticks.identifier",
	"lexer.sql.numbersign.comment",
	"lexer.tex.auto.if",
	"lexer.tex.comment.process",
	"lexer.tex.interface.default",
	"lexer.tex.use.keywords",
	"lexer.xml.allow.scripts",
	"nsis.ignorecase",
	"nsis.uservars",
	"ps.level",
	"ps.tokenize",
	"sql.backslash.escapes",
	"styling.within.preprocessor",
	"tab.timmy.whinge.level",
	"fold.onerror",
	"fold.cdata",
	"precompiller.debugmode",
	"precompiller.debugsuffix",
	"lexer.mssql.fold.brackets",
	"lexer.mssql.fold.querry",
	"lexer.pgsql.transparent.tags",
	"$lexer.formenjine.syslog",

//--Autogenerated -- end of automatically generated section

	0,
};

/* XPM */
static const char *bookmarkBluegem[] = {
/* width height num_colors chars_per_pixel */
"    15    15      64            1",
/* colors */
"  c none",
". c #0c0630",
"# c #8c8a8c",
"a c #244a84",
"b c #545254",
"c c #cccecc",
"d c #949594",
"e c #346ab4",
"f c #242644",
"g c #3c3e3c",
"h c #6ca6fc",
"i c #143789",
"j c #204990",
"k c #5c8dec",
"l c #707070",
"m c #3c82dc",
"n c #345db4",
"o c #619df7",
"p c #acacac",
"q c #346ad4",
"r c #1c3264",
"s c #174091",
"t c #5482df",
"u c #4470c4",
"v c #2450a0",
"w c #14162c",
"x c #5c94f6",
"y c #b7b8b7",
"z c #646464",
"A c #3c68b8",
"B c #7cb8fc",
"C c #7c7a7c",
"D c #3462b9",
"E c #7c7eac",
"F c #44464c",
"G c #a4a4a4",
"H c #24224c",
"I c #282668",
"J c #5c5a8c",
"K c #7c8ebc",
"L c #dcd7e4",
"M c #141244",
"N c #1c2e5c",
"O c #24327c",
"P c #4472cc",
"Q c #6ca2fc",
"R c #74b2fc",
"S c #24367c",
"T c #b4b2c4",
"U c #403e58",
"V c #4c7fd6",
"W c #24428c",
"X c #747284",
"Y c #142e7c",
"Z c #64a2fc",
"0 c #3c72dc",
"1 c #bcbebc",
"2 c #6c6a6c",
"3 c #848284",
"4 c #2c5098",
"5 c #1c1a1c",
"6 c #243250",
"7 c #7cbefc",
"8 c #d4d2d4",
/* pixels */
"    yCbgbCy    ",
"   #zGGyGGz#   ",
"  #zXTLLLTXz#  ",
" p5UJEKKKEJU5p ",
" lfISa444aSIfl ",
" wIYij444jsYIw ",
" .OsvnAAAnvsO. ",
" MWvDuVVVPDvWM ",
" HsDPVkxxtPDsH ",
" UiAtxohZxtuiU ",
" pNnkQRBRhkDNp ",
" 1FrqoR7Bo0rF1 ",
" 8GC6aemea6CG8 ",
"  cG3l2z2l3Gc  ",
"    1GdddG1    "
};

SString SciTEBase::GetFileNameProperty(const char *name) {
	SString namePlusDot = name;
	namePlusDot.append(".");
	SString valueForFileName = props.GetNewExpand(namePlusDot.c_str(),
	        ExtensionFileName().c_str());
	if (valueForFileName.length() != 0) {
		return valueForFileName;
	} else {
		return props.Get(name);
	}
}
void SciTEBase::SetColourElement(GUI::ScintillaWindow *pWin, int elem,const char *colourProp,const char *alphaProp) {
	SString clr = props.Get(colourProp);
	if (clr.length()) {
		Scintilla::Colour c = ColourFromString(clr);
		int a = props.GetInt(alphaProp, 255) & 0xFF;
		if (invertColors) {
			c = convMain.Convert(c);
			a *= 2;
			if (a > 255)
				a = 255;
		}
		c |= a << 24;
		if (!pWin)
			CallChildren(Message::SetElementColour, elem, c);
		else
			pWin->SetElementColour(static_cast<Element>(elem), c);
	} else {
		if (!pWin)
			CallChildren(Message::ResetElementColour, elem);
		else
			pWin->ResetElementColour(static_cast<Element>(elem));
	}
}

void SciTEBase::ReadProperties() {

	SString fileNameForExtension = ExtensionFileName();
	language = props.GetNewExpand("lexer.", fileNameForExtension.c_str());
	std::string languageCurrent = wEditor.LexerLanguage();

	ScintillaWindowEditor* pEd = wEditor.GetWindowIdm() == IDM_SRCWIN ? &wEditorL : &wEditorR;


	if (pEd->languageCurrent == language.c_str() && languageCurrent == pEd->languageCurrent){
		//int ttt = 55;
		return;
	}
	
	if (extender)
		extender->Clear();
	//const std::string lexillaPath = props.GetExpandedString("lexilla.path");
	Lexilla::Load(".");


	if (language.length()) {
		if (strcmp(language.c_str(), languageCurrent.c_str())) {
			if (language.startswith("script_")) {
				wEditor.SetILexer(nullptr);
			}
			else {

				Scintilla::ILexer5* plexer = Lexilla::MakeLexer(language.c_str());
				wEditor.SetILexer(plexer);
			}
		}
	}
	else if (languageCurrent.length()) {
		wEditor.SetILexer(nullptr);
	}

	props.Set("Language", language.c_str());
	invertColors = props.GetInt("invert.lexer.colors");
	hideHiddenStyles = props.GetInt("hide.hidden.styles");

	lexLanguage = wEditor.Lexer();

	SString kw0 = props.GetNewExpand("keywords.", fileNameForExtension.c_str());
	wEditor.SetKeyWords(0, kw0.c_str());

	int maxN = KEYWORDSET_MAX;
	SString maxS = props.GetNewExpand("max.keywords.", fileNameForExtension.c_str());
	if (maxS.length())
		maxN = maxS.value() - 1;
	for (int wl = 1; wl <= maxN; wl++) {
		SString kwk(wl + 1);
		kwk += '.';
		kwk.insert(0, "keywords");
		SString kw = props.GetNewExpand(kwk.c_str(), fileNameForExtension.c_str());
		wEditor.SetKeyWords(wl, kw.c_str());
	}

	for (size_t i = 0; propertiesToForward[i]; i++) {
		if (*propertiesToForward[i] == '$')
			wEditor.SetProperty(propertiesToForward[i] + 1, props.GetNewExpand(propertiesToForward[i] + 1, ExtensionFileName().c_str()).c_str());
		else
			wEditor.SetProperty(propertiesToForward[i], props.GetExpanded(propertiesToForward[i]).c_str());
	}

	if (firstPropertiesRead) {
		SString points = props.Get("lexer.lightness.points");
		if (points == "")
			points = "(0,100)(10,93.177)(40, 78.912)(50,75.625)(60,71.392)(70,64.377)(80,52.192)(90, 31.897)(99, 3.822)(100,0)";
		convMain.Init(points.c_str(), this);
	}

	FilePath fileAbbrev = GUI::StringFromUTF8(props.GetNewExpand("abbreviations.", fileNameForExtension.c_str()).c_str());
	props.Set("AbbrevPath", fileAbbrev.AsUTF8().c_str());

	wEditor.SetOvertype(props.GetInt("change.overwrite.enable", 0));

	codePage = props.GetInt("code.page");
	if (CurrentBuffer()->unicodeMode != uni8Bit) {
		// Override properties file to ensure Unicode displayed.
		codePage = SC_CP_UTF8;
	}
	props.SetInteger("editor.unicode.mode", CurrentBuffer()->unicodeMode + IDM_ENCODING_DEFAULT); //!-add-[EditorUnicodeMode]
	wEditor.SetCodePage(codePage);

	SString tmp_str;
	tmp_str = props.GetNewExpand("caret.fore.", fileNameForExtension.c_str());

	if (tmp_str.length())
		wEditor.SetCaretFore(ColourFromString(tmp_str));
	else
		wEditor.SetCaretFore(ColourOfProperty("caret.fore", ColourRGB(0, 0, 0), invertColors));
	
	wEditor.SetCaretStyle(static_cast<CaretStyle>(CARETSTYLE_LINE | (CARETSTYLE_OVERSTRIKE_BLOCK * props.GetInt("caret.overstrike.block"))));

	wEditor.SetMultipleSelection(props.GetInt("selection.multiple", 1));
	wEditor.SetAdditionalSelectionTyping(props.GetInt("selection.additional.typing", 1));
	wEditor.SetAdditionalCaretsBlink(props.GetInt("caret.additional.blinks", 1));
	wEditor.SetVirtualSpaceOptions(static_cast<VirtualSpace>(props.GetInt("virtual.space")));

	wEditor.SetMouseDwellTime(props.GetInt("dwell.period", SC_TIME_FOREVER));


	wEditor.SetCaretWidth(props.GetInt("caret.width", 1));

	SetColourElement(&wEditor, SC_ELEMENT_CARET_LINE_BACK, "caret.line.back", "caret.line.back.alpha");

	SString controlCharSymbol = props.Get("control.char.symbol");
	if (controlCharSymbol.length()) {
		wEditor.SetControlCharSymbol(controlCharSymbol[0]);
	}
	else {
		wEditor.SetControlCharSymbol(0);
	}

	SString caretPeriod = props.Get("caret.period");
	if (caretPeriod.length()) {
		wEditor.SetCaretPeriod(caretPeriod.value());
	}

	int caretSlop = props.GetInt("caret.policy.xslop", 1) ? CARET_SLOP : 0;
	int caretZone = props.GetInt("caret.policy.width", 50);
	int caretStrict = props.GetInt("caret.policy.xstrict") ? CARET_STRICT : 0;
	int caretEven = props.GetInt("caret.policy.xeven", 1) ? CARET_EVEN : 0;
	int caretJumps = props.GetInt("caret.policy.xjumps") ? CARET_JUMPS : 0;
	wEditor.SetXCaretPolicy(static_cast<CaretPolicy>(caretStrict | caretSlop | caretEven | caretJumps), caretZone);

	caretSlop = props.GetInt("caret.policy.yslop", 1) ? CARET_SLOP : 0;
	caretZone = props.GetInt("caret.policy.lines");
	caretStrict = props.GetInt("caret.policy.ystrict") ? CARET_STRICT : 0;
	caretEven = props.GetInt("caret.policy.yeven", 1) ? CARET_EVEN : 0;
	caretJumps = props.GetInt("caret.policy.yjumps") ? CARET_JUMPS : 0;
	wEditor.SetYCaretPolicy(static_cast<CaretPolicy>(caretStrict | caretSlop | caretEven | caretJumps), caretZone);

	int visibleStrict = props.GetInt("visible.policy.strict") ? VISIBLE_STRICT : 0;
	int visibleSlop = props.GetInt("visible.policy.slop", 1) ? VISIBLE_SLOP : 0;
	int visibleLines = props.GetInt("visible.policy.lines");
	wEditor.SetVisiblePolicy(static_cast<VisiblePolicy>( visibleStrict | visibleSlop), visibleLines);

	wEditor.SetEdgeColumn(props.GetInt("edge.column", 0));
	wEditor.SetEdgeMode(static_cast<EdgeVisualStyle>(props.GetInt("edge.mode", EDGE_NONE)));
	wEditor.SetEdgeColour(ColourOfProperty("edge.colour", ColourRGB(0xff, 0xda, 0xda), invertColors));


	SetColourElement(&wEditor, SC_ELEMENT_SELECTION_TEXT, "selection.fore", "selection.alpha");

	SetColourElement(&wEditor, SC_ELEMENT_SELECTION_BACK, "selection.back", "selection.alpha");

	SetColourElement(&wEditor, SC_ELEMENT_SELECTION_ADDITIONAL_TEXT, "selection.additional.fore", "selection.additional.alpha");

	SetColourElement(&wEditor, SC_ELEMENT_SELECTION_ADDITIONAL_BACK, "selection.additional.back", "selection.additional.alpha");


	SString whitespaceFore = props.Get("whitespace.fore");
	if (whitespaceFore.length()) {
		wEditor.SetWhitespaceFore(1, ColourFromString(whitespaceFore));
	} else {
		wEditor.SetWhitespaceFore(0, 0);
	}
	SString whitespaceBack = props.Get("whitespace.back");
	if (whitespaceBack.length()) {
		wEditor.SetWhitespaceBack(1, ColourFromString(whitespaceBack));
	} else {
		wEditor.SetWhitespaceBack(0, 0);
	}

	char bracesStyleKey[200];
	sprintf(bracesStyleKey, "braces.%s.style", language.c_str());
	bracesStyle = props.GetInt(bracesStyleKey, 0);

	char key[200];
	SString sval;


	sprintf(key, "autocomplete.%s.fillups", language.c_str());
	autoCompleteFillUpCharacters = props.GetExpanded(key);
	if (autoCompleteFillUpCharacters == "")
	autoCompleteFillUpCharacters =
	props.GetExpanded("autocomplete.*.fillups");
	wEditor.AutoCSetFillUps(autoCompleteFillUpCharacters.c_str());

	sprintf(key, "autocomplete.%s.ignorecase", "*");
	sval = props.GetNewExpand(key);
	autoCompleteIgnoreCase = sval == "1";
	sprintf(key, "autocomplete.%s.ignorecase", language.c_str());
	sval = props.GetNewExpand(key);
	if (sval != "")
	autoCompleteIgnoreCase = sval == "1";
	wEditor.AutoCSetIgnoreCase(autoCompleteIgnoreCase ? 1 : 0);

	int autoCChooseSingle = props.GetInt("autocomplete.choose.single");
	wEditor.AutoCSetChooseSingle(autoCChooseSingle),

	wEditor.AutoCSetCancelAtStart(0);
	wEditor.AutoCSetDropRestOfWord(0);

	if (firstPropertiesRead) {
		ReadPropertiesInitial();
		props.SetInteger("system.code.page", ::GetACP());
	}

	characterSet = props.GetInt("character.set", SC_CHARSET_DEFAULT);
	ReadFontProperties();

	COLORREF cFold = layout.GetColorRef("SCR_BACKCOLOR");

	wEditor.SetFoldMarginHiColour(1, cFold);
	wOutput.SetFoldMarginHiColour(1, cFold);
	wFindRes.SetFoldMarginHiColour(1, cFold);

	unsigned char r = static_cast<unsigned char>((cFold & 0xFF0000) >> 16);
	unsigned char g = static_cast<unsigned char>((cFold & 0x00FF00) >> 8);
	unsigned char b = static_cast<unsigned char>(cFold & 0x0000FF);

	unsigned char r1 = static_cast<unsigned char>((clrDefaultBack & 0xFF0000) >> 16);
	unsigned char g1 = (clrDefaultBack & 0x00FF00) >> 8;
	unsigned char b1 = clrDefaultBack & 0x0000FF;


	if (clr_brightness(cFold) < clr_brightness(clrDefaultBack)) {
		r = r >= 0xDF ? 0xFF : r + 0x20;
		g = g >= 0xDF ? 0xFF : g + 0x20;
		b = b >= 0xDF ? 0xFF : b + 0x20;
		r = min(r, r1);
		g = min(g, g1);
		b = min(b, b1);
	}
	else {
		r = r <= 0x20 ? 0 : r - 0x20;
		g = g <= 0x20 ? 0 : g - 0x20;
		b = b <= 0x20 ? 0 : b - 0x20;
		r = max(r, r1);
		g = max(g, g1);
		b = max(b, b1);
	}

	cFold = ((r << 16) & 0xFF0000) | ((g << 8) & 0x00FF00) | (b & 0x0000FF);

	wEditor.SetFoldMarginColour(1, cFold);
	wOutput.SetFoldMarginColour(1, cFold);
	wFindRes.SetFoldMarginColour(1, cFold);

	wEditor.SetViewWS(static_cast<WhiteSpace>(props.GetInt("view.whitespace")));
	wEditor.SetIndentationGuides(static_cast<IndentView>(props.GetInt("view.indentation.guides")));

	wEditor.SetPrintMagnification(props.GetInt("print.magnification"));
	wEditor.SetPrintColourMode(static_cast<PrintOption>(props.GetInt("print.colour.mode")));


	int blankMarginLeft = props.GetInt("blank.margin.left", 1);
	int blankMarginRight = props.GetInt("blank.margin.right", 1);
	wEditor.SetMarginLeft(blankMarginLeft);
	wEditor.SetMarginRight(blankMarginRight);

	wEditor.SetMarginWidthN(1, margin ? marginWidth : 0);

	SString lineMarginProp = props.Get("line.margin.width");
	lineNumbersWidth = lineMarginProp.value();
	if (lineNumbersWidth == 0)
	lineNumbersWidth = lineNumbersWidthDefault;
	lineNumbersExpand = lineMarginProp.contains('+');

	SetLineNumberWidth();

	bufferedDraw = props.GetInt("buffered.draw", 1);
	wEditor.SetBufferedDraw(bufferedDraw);

	wEditor.SetLayoutCache(static_cast<LineCache>(props.GetInt("cache.layout", SC_CACHE_CARET)));

	bracesCheck = props.GetInt("braces.check");
	bracesSloppy = props.GetInt("braces.sloppy");

	wEditor.SetCharsDefault();
	wordCharacters = props.GetNewExpand("word.characters.", fileNameForExtension.c_str());
	if (wordCharacters.length()) {
		wEditor.SetWordChars(wordCharacters.c_str());
	} else {
		wordCharacters = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	}

	whitespaceCharacters = props.GetNewExpand("whitespace.characters.", fileNameForExtension.c_str());
	if (whitespaceCharacters.length()) {
		wEditor.SetWhitespaceChars(whitespaceCharacters.c_str());
	}

	SString viewIndentExamine = GetFileNameProperty("view.indentation.examine");
	indentExamine = viewIndentExamine.length() ? viewIndentExamine.value() : SC_IV_REAL;
	wEditor.SetIndentationGuides(static_cast<IndentView>(props.GetInt("view.indentation.guides") ?
		indentExamine : SC_IV_NONE));

	wEditor.SetTabIndents(props.GetInt("tab.indents", 1));
	wEditor.SetBackSpaceUnIndents(props.GetInt("backspace.unindents", 1));

	wEditor.CallTipUseStyle(32);

	SString lookback = props.GetNewExpand("statement.lookback.", fileNameForExtension.c_str());
	statementLookback = lookback.value();
	statementIndent = GetStyleAndWords("statement.indent.");
	statementEnd = GetStyleAndWords("statement.end.");
	blockStart = GetStyleAndWords("block.start.");
	blockEnd = GetStyleAndWords("block.end.");

	SString list;
	list = props.GetNewExpand("preprocessor.symbol.", fileNameForExtension.c_str());
	preprocessorSymbol = list[0];
	list = props.GetNewExpand("preprocessor.start.", fileNameForExtension.c_str());
	preprocCondStart.Clear();
	preprocCondStart.Set(list.c_str());
	list = props.GetNewExpand("preprocessor.middle.", fileNameForExtension.c_str());
	preprocCondMiddle.Clear();
	preprocCondMiddle.Set(list.c_str());
	list = props.GetNewExpand("preprocessor.end.", fileNameForExtension.c_str());
	preprocCondEnd.Clear();
	preprocCondEnd.Set(list.c_str());

	wEditor.SetWrapVisualFlags(static_cast<WrapVisualFlag>(props.GetInt("wrap.visual.flags")));
	wEditor.SetWrapVisualFlagsLocation(static_cast<WrapVisualLocation>(props.GetInt("wrap.visual.flags.location")));
	wEditor.SetWrapStartIndent(props.GetInt("wrap.visual.startindent"));
	wEditor.SetWrapIndentMode(static_cast<WrapIndentMode>(props.GetInt("wrap.indent.mode")));

	wEditor.SetFoldFlags(static_cast<FoldFlag>(props.GetInt("fold.flags")));

	// Create a margin column for the folding symbols
	wEditor.SetMarginTypeN(2, MarginType::Symbol);

	wEditor.SetMarginWidthN(2, foldMargin ? foldMarginWidth : 0);

	wEditor.SetMarginMaskN(2, SC_MASK_FOLDERS | 1);
	wEditor.SetMarginSensitiveN(2, 1);
	if (props.GetInt("margin.bookmark.by.single.click", 1) == 1) { 
		wEditor.SetMarginSensitiveN(1, 1);	
		wEditor.SetMarginSensitiveN(0, 1);	
	}

	SetFoldingMarkers(true);

	wEditor.MarkerSetFore(markerBookmark, ColourOfProperty("bookmark.fore", ColourRGB(0, 0, 0x7f)));
	wEditor.MarkerSetBack(markerBookmark,ColourOfProperty("bookmark.back", ColourRGB(0x80, 0xff, 0xff)));
	wEditor.MarkerSetAlpha(markerBookmark, static_cast<Alpha>(props.GetInt("bookmark.alpha", SC_ALPHA_NOALPHA)));
	SString bookMarkXPM = props.Get("bookmark.pixmap");
	if (bookMarkXPM.length()) {
		wEditor.MarkerDefinePixmap(markerBookmark, bookMarkXPM.c_str());
	} else if (props.Get("bookmark.fore").length()) {
		wEditor.MarkerDefine(markerBookmark, MarkerSymbol::Circle);
	} else {
		// No bookmark.fore setting so display default pixmap.
		wEditor.MarkerDefinePixmap(markerBookmark, reinterpret_cast<char *>(bookmarkBluegem));
	}
	wEditor.MarkerSetBack(SC_MARKNUM_HISTORY_MODIFIED, ColourRGB(0xff, 0x80, 0x00));

	wEditor.MarkerDefine(markerScipLineFormat, MarkerSymbol::Empty);

	wEditor.MarkerDefine(markerError, MarkerSymbol::ShortArrow);
	wEditor.MarkerSetFore(markerError, ColourOfProperty("error.marker.fore", ColourRGB(0x7f, 0, 0)));
	wEditor.MarkerSetBack(markerError, ColourOfProperty("error.marker.back", ColourRGB(0xff, 0xff, 0)));

	wEditor.MarkerDefine(markerBreakPoint, MarkerSymbol::Arrow);
	wEditor.MarkerSetFore(markerBreakPoint, ColourOfProperty("breakpoint.marker.fore", ColourRGB(0x7f, 0, 0x7f)));
	wEditor.MarkerSetBack(markerBreakPoint, ColourOfProperty("breakpoint.marker.back", ColourRGB(0xff, 0, 0)));

	wEditor.MarkerDefine(markerVertAlign, MarkerSymbol::VLine);
	wEditor.MarkerSetFore(markerVertAlign, ColourOfProperty("vertalign.marker.fore", ColourRGB(0x0, 0, 0x7f)));

	wEditor.AutoCSetMulti(MultiAutoComplete::Each);

	wEditor.SetEndAtLastLine( props.GetInt("end.at.last.line", 1));
	wEditor.SetCaretSticky(static_cast<CaretSticky>(props.GetInt("caret.sticky", 0)));


	SetColourElement(&wEditor, SC_ELEMENT_LIST, "list.colour", "#");
	SetColourElement(&wEditor, SC_ELEMENT_LIST_BACK, "list.back", "#");
	SetColourElement(&wEditor, SC_ELEMENT_LIST_SELECTED, "list.selection", "#");
	SetColourElement(&wEditor, SC_ELEMENT_LIST_SELECTED_BACK, "list.selection.back", "#");

	SetColourElement(&wEditor, SC_ELEMENT_HIDDEN_LINE, "colour.comment", "#");

	//wEditor.EOLAnnotationSetVisible(static_cast<EOLAnnotationVisible>(props.GetInt("eol.annotation.visible", static_cast<int>(EOLAnnotationVisible::Angles))));
	wEditor.EOLAnnotationSetVisible(EOLAnnotationVisible::Angles);
	//wEditorL.EOLAnnotationSetVisible(EOLAnnotationVisible::Angles);
	
	
	if (extender) {
		FilePath defaultDir = GetDefaultDirectory();
		FilePath scriptPath;

		// Check for an extension script
		GUI::gui_string extensionFile = GUI::StringFromUTF8(
			props.GetNewExpand("extension.", fileNameForExtension.c_str()).c_str());
		if (extensionFile.length()) {
			// find file in local directory
			FilePath docDir = filePath.Directory();
			if (Exists(docDir.AsInternal(), extensionFile.c_str(), &scriptPath)) {
				// Found file in document directory
				extender->Load(scriptPath.AsUTF8().c_str());
			} else if (Exists(defaultDir.AsInternal(), extensionFile.c_str(), &scriptPath)) {
				// Found file in global directory
				extender->Load(scriptPath.AsUTF8().c_str());
			} else if (Exists(GUI_TEXT(""), extensionFile.c_str(), &scriptPath)) {
				// Found as completely specified file name
				extender->Load(scriptPath.AsUTF8().c_str());
			}
		}
	}
	if(firstPropertiesRead)
		ReadPropertiesEx();

	firstPropertiesRead = false;
	needReadProperties = false;

	pEd->languageCurrent = language.c_str();
}

void SciTEBase::ReadPropertiesEx() {
	if (extender)
		extender->Clear();
	CallAll(Message::SetTechnology, props.GetInt("technology", SC_TECHNOLOGY_DIRECT_WRITE_1));

	std::string languageCurrent = wOutput.LexerLanguage();
	if (strcmp("errorlist", languageCurrent.c_str())) {
		Scintilla::ILexer5 *plexer = Lexilla::MakeLexer("errorlist");
		wOutput.SetILexer(plexer);

		plexer = Lexilla::MakeLexer("searchResult");
		wFindRes.SetILexer(plexer);
	}

	FilePath homepath = GetSciteDefaultHome();
	props.Set("SciteDefaultHome", homepath.AsUTF8().c_str());
	homepath = GetSciteUserHome();
	props.Set("SciteUserHome", homepath.AsUTF8().c_str());

	for (size_t i=0; propertiesToForward[i]; i++) {
		ForwardPropertyToEditor(propertiesToForward[i]);
	}

	wEditorL.SetAutomaticFold(static_cast<AutomaticFold>(SC_AUTOMATICFOLD_CHANGE | SC_AUTOMATICFOLD_SHOW));
	wEditorR.SetAutomaticFold(static_cast<AutomaticFold>(SC_AUTOMATICFOLD_CHANGE | SC_AUTOMATICFOLD_SHOW));

	FilePath fileAbbrev = GUI::StringFromUTF8(props.GetNewExpand("abbreviations.", ExtensionFileName().c_str()).c_str());
	props.Set("AbbrevPath", fileAbbrev.AsUTF8().c_str());


	int outputCodePage = props.GetInt("output.code.page", codePage);
	wOutput.SetCodePage(outputCodePage);
	wFindRes.SetCodePage(outputCodePage);

	wrapStyle = props.GetInt("wrap.style", SC_WRAP_WORD);

	CallChildren(Message::SetCaretFore,
	   ColourOfProperty("caret.fore", ColourRGB(0, 0, 0), invertColors));
	CallChildren(Message::SetCaretStyle, CARETSTYLE_LINE | (CARETSTYLE_OVERSTRIKE_BLOCK * props.GetInt("caret.overstrike.block")));

	CallChildren(Message::SetMultipleSelection, props.GetInt("selection.multiple", 1));
	CallChildren(Message::SetAdditionalSelectionTyping, props.GetInt("selection.additional.typing", 1));
	CallChildren(Message::SetAdditionalCaretsBlink, props.GetInt("caret.additional.blinks", 1));
	CallChildren(Message::SetVirtualSpaceOptions, props.GetInt("virtual.space"));

	wEditor.SetMouseDwellTime(props.GetInt("dwell.period", SC_TIME_FOREVER));

	CallChildren(Message::SetCaretWidth, props.GetInt("caret.width", 1));
	
	SetColourElement(&wFindRes, SC_ELEMENT_CARET_LINE_BACK, "findres.caret.line.back", "findres.caret.line.back.alpha");

//!-start-[output.caret]  - �� ����� ������ � ������
	wOutput.SetCaretFore(ColourOfProperty("output.caret.fore", ColourRGB(0x00, 0x00, 0x00)));

	SetColourElement(&wOutput, SC_ELEMENT_CARET_LINE_BACK, "output.caret.line.back", "output.caret.line.back.alpha");

	SString caretPeriod = props.Get("caret.period");
	if (caretPeriod.length()) {
		CallChildren(Message::SetCaretPeriod, caretPeriod.value());
	}

	wEditorL.SetCaretLineLayer(Layer::UnderText);
	wEditorR.SetCaretLineLayer(Layer::UnderText);
	wOutput.SetCaretLineLayer(Layer::UnderText);
	wFindRes.SetCaretLineLayer(Layer::UnderText);

	wEditorL.SetSelectionLayer(Layer::UnderText);
	wEditorR.SetSelectionLayer(Layer::UnderText);
	wOutput.SetSelectionLayer(Layer::UnderText);
	wFindRes.SetSelectionLayer(Layer::UnderText);

	SetColourElement(NULL, SC_ELEMENT_SELECTION_TEXT, "selection.fore", "selection.alpha");

	SetColourElement(NULL, SC_ELEMENT_SELECTION_BACK, "selection.back", "selection.alpha");
	
	SetColourElement(NULL, SC_ELEMENT_SELECTION_ADDITIONAL_TEXT, "selection.additional.fore", "selection.additional.alpha");

	SetColourElement(NULL, SC_ELEMENT_SELECTION_ADDITIONAL_BACK, "selection.additional.back", "selection.additional.alpha");

	SString whitespaceFore = props.Get("whitespace.fore");
	if (whitespaceFore.length()) {
		CallChildren(Message::SetWhitespaceFore, 1, ColourFromString(whitespaceFore));
	} else {
		CallChildren(Message::SetWhitespaceFore, 0, 0);
	}
	SString whitespaceBack = props.Get("whitespace.back");
	if (whitespaceBack.length()) {
		CallChildren(Message::SetWhitespaceBack, 1, ColourFromString(whitespaceBack));
	} else {
		CallChildren(Message::SetWhitespaceBack, 0, 0);
	}

	char bracesStyleKey[200];
	sprintf(bracesStyleKey, "braces.%s.style", language.c_str());
	bracesStyle = props.GetInt(bracesStyleKey, 0);

	char key[200];
	SString sval;

	sval = FindLanguageProperty("calltip.*.ignorecase");
	callTipIgnoreCase = sval == "1";

	calltipWordCharacters = FindLanguageProperty("calltip.*.word.characters",
		"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

	calltipParametersStart = FindLanguageProperty("calltip.*.parameters.start", "(");
	calltipParametersEnd = FindLanguageProperty("calltip.*.parameters.end", ")");
	calltipParametersSeparators = FindLanguageProperty("calltip.*.parameters.separators", ",;");

	calltipEndDefinition = FindLanguageProperty("calltip.*.end.definition");

	sprintf(key, "autocomplete.%s.start.characters", language.c_str());
	autoCompleteStartCharacters = props.GetExpanded(key);
	if (autoCompleteStartCharacters == "")
		autoCompleteStartCharacters = props.GetExpanded("autocomplete.*.start.characters");

	jobQueue.clearBeforeExecute = props.GetInt("clear.before.execute");
	jobQueue.timeCommands = props.GetInt("time.commands");

	int blankMarginLeft = props.GetInt("blank.margin.left", 1);
	int blankMarginRight = props.GetInt("blank.margin.right", 1);

	CallChildren(Message::SetMarginLeft, 0, blankMarginLeft);
	CallChildren(Message::SetMarginRight, 0, blankMarginRight);

	CallChildren(Message::SetLayoutCache, props.GetInt("output.cache.layout", SC_CACHE_CARET));

	props.Set("CurrentWordCharacters", wordCharacters.c_str() );

	scrollOutput = props.GetInt("output.scroll", 1);

	CallChildren(Message::SetFoldFlags, props.GetInt("fold.flags"));

	ModificationFlags flag = ModificationFlags::InsertText | ModificationFlags::DeleteText | ModificationFlags::BeforeInsert | ModificationFlags::BeforeDelete
		| ModificationFlags::LastStepInUndoRedo | ModificationFlags::ChangeMarker | ModificationFlags::ChangeFold;
	wEditorL.SetModEventMask(flag);
	wEditorR.SetModEventMask(flag);
	wOutput.SetModEventMask(ModificationFlags::InsertText | ModificationFlags::DeleteText | ModificationFlags::LastStepInUndoRedo | ModificationFlags::BeforeInsert );
	wFindRes.SetModEventMask(ModificationFlags::InsertText | ModificationFlags::DeleteText | ModificationFlags::LastStepInUndoRedo);
	// Create a margin column for the folding symbols
	CallChildren(Message::SetMarginTypeN, 2, SC_MARGIN_SYMBOL);

	CallChildren(Message::SetMarginWidthN, 2, foldMargin ? foldMarginWidth : 0);

	CallChildren(Message::SetMarginMaskN, 2, SC_MASK_FOLDERS | 1);
	CallChildren(Message::SetMarginSensitiveN, 2, 1);
	if (props.GetInt("margin.bookmark.by.single.click",1)==1) //!-add-[SetBookmark]
		wEditor.SetMarginSensitiveN(1, 1);	//!-add-[SetBookmark]

	SetFoldingMarkers(false);


	CallChildren(Message::SetScrollWidthTracking, 1);

	wOutput.SetUndoCollection(0);
	wFindRes.SetUndoCollection(0);

	wFindRes.MarkerDefine(0, MarkerSymbol::Bar);
	wFindRes.MarkerSetFore(0, ColourRGB(0xff, 0x80, 0x00));
	wFindRes.MarkerSetBack(0, ColourRGB(0xff, 0x80, 0x00));



}

void SciTEBase::ResetAllStyles(ScintillaWindowEditor& win, const char* languageName) {
	win.StyleClearAll();
	win.ReleaseAllExtendedStyles();

	SetStyleFor(win, "*");
	SetStyleFor(win, languageName);

	int styleOffset = win.AllocateExtendedStyles(32);
	win.EOLAnnotationSetStyleOffset(styleOffset);

	SetOneStyle(win, styleOffset + 1, StyleDefinition(props.GetNewExpand("font.comment").c_str(), &convMain, invertColors));
	win.StyleSetBack(styleOffset + 1, layout.GetColorRef("SCR_FORECOLOR"));
	win.StyleSetFore(styleOffset + 1, layout.GetColorRef("FGCOLOR"));


}
void SciTEBase::ReadFontProperties() {
	char key[200];
	SString sval;
	const char *languageName = language.c_str();

	if (lexLanguage == lexLPeg) {
		// Retrieve style info.
		char propStr[256];
		for (int i = 0; i < STYLE_MAX; i++) {
			sprintf(key, "style.lpeg.%0d", i);
			wEditor.PrivateLexerCall(i - STYLE_MAX, propStr);
			props.Set(key, static_cast<const char*>(propStr));
		}
		languageName = "lpeg";
	}

	// Set styles
	// For each window set the global default style, then the language default style, then the other global styles, then the other language styles
	CallChildren(Message::StyleResetDefault);

	sprintf(key, "style.%s.%0d", "*", STYLE_DEFAULT);
	sval = props.GetNewExpand(key);


	StyleDefinition style(sval.c_str(), &convMain, invertColors);
	char sColor[8];
	ColourDesired color;
	if (!(style.specified & StyleDefinition::sdBack)) {
		color.Set( ::GetSysColor(COLOR_WINDOW) );
		memset(sColor,0,sizeof(sColor));
		sprintf(sColor, "#%2X%2X%2X",color.GetRed(), color.GetGreen(), color.GetBlue() );
		style.back = sColor;
		style.back.substitute(' ', '0');
		style.specified = static_cast<StyleDefinition::flags>(style.specified | style.sdBack);
	}
	if (!(style.specified & StyleDefinition::sdFore)) {
		color.Set( ::GetSysColor(COLOR_WINDOWTEXT) );
		memset(sColor,0,sizeof(sColor));
		sprintf(sColor, "#%2X%2X%2X",color.GetRed(), color.GetGreen(), color.GetBlue() );
		style.fore = sColor;
		style.fore.substitute(' ', '0');
		style.specified = static_cast<StyleDefinition::flags>(style.specified | style.sdFore);
	}
	SetOneStyle(wEditor, STYLE_DEFAULT, style);
	SetOneStyle(wOutput, STYLE_DEFAULT, style);
	SetOneStyle(wFindRes, STYLE_DEFAULT, style);

	clrDefaultBack = style.BackAsLong();


	sprintf(key, "style.%s.%0d", languageName, STYLE_DEFAULT);
	sval = props.GetNewExpand(key);
	{
		StyleDefinition style(sval.c_str(), &convMain, invertColors);
		style.invertColors = invertColors;
		SetOneStyle(wEditor, STYLE_DEFAULT, style);
	}

	ResetAllStyles(wEditor, languageName);
	
	wOutput.StyleClearAll();


	sprintf(key, "style.%s.%0d", "errorlist", STYLE_DEFAULT);
	sval = props.GetNewExpand(key);
	{
		StyleDefinition style(sval.c_str(), &convMain, invertColors);
		SetOneStyle(wOutput, STYLE_DEFAULT, style);
	}
	wOutput.StyleClearAll();
	SetStyleFor(wOutput, "*");
	SetStyleFor(wOutput, "errorlist");
	
	wFindRes.StyleClearAll();
	sprintf(key, "style.%s.%0d", "searchResult", STYLE_DEFAULT);
	sval = props.GetNewExpand(key);
	{
		StyleDefinition style(sval.c_str(), &convMain, invertColors);
		SetOneStyle(wFindRes, STYLE_DEFAULT, style);
	}
	wFindRes.StyleClearAll();
	SetStyleFor(wFindRes, "*");
	SetStyleFor(wFindRes, "searchResult");
	wFindRes.SetIndent(2);
	wFindRes.SetTabWidth(2);

	if (CurrentBuffer()->useMonoFont) {
		sval = props.GetExpanded("font.monospace");
		StyleDefinition sd(sval.c_str(), &convMain, invertColors);
		for (int style = 0; style <= STYLE_MAX; style++) {
			if (style != STYLE_LINENUMBER) {
				if (sd.specified & StyleDefinition::sdFont) {
					wEditor.StyleSetFont(style, sd.font.c_str());
				}
				if (sd.specified & StyleDefinition::sdSize) {
					wEditor.StyleSetSize(style, sd.size);
				}
			}
		}
	}
}

// Properties that are interactively modifiable are only read from the properties file once.
void SciTEBase::SetPropertiesInitial() {
	openFilesHere = props.GetInt("check.if.already.open");
	wrap = props.GetInt("wrap");
	wrapOutput = props.GetInt("output.wrap");
	wrapFindRes = props.GetInt("findres.wrap");
	indentationWSVisible = props.GetInt("view.indentation.whitespace", 1);

	lineNumbers = props.GetInt("line.margin.visible");	
	viewIndent = props.GetInt("view.indentation.guides");
	viewHisoryIndicators = props.GetInt("view.history.indicators");
	viewHisoryMarkers = props.GetInt("view.history.markers", 1);
	viewWs = props.GetInt("view.whitespace");

	marginWidth = 0;
	SString margwidth = props.Get("margin.width");
	if (margwidth.length())
		marginWidth = margwidth.value();
	margin = marginWidth;
	if (marginWidth == 0)
		marginWidth = marginWidthDefault;
	foldMarginWidth = props.GetInt("fold.margin.width", foldMarginWidthDefault);
	foldMargin = foldMarginWidth;
	if (foldMarginWidth == 0)
		foldMarginWidth = foldMarginWidthDefault;

	matchCase = props.GetInt("find.replace.matchcase");
	regExp = props.GetInt("find.replace.regexp");
	unSlash = props.GetInt("find.replace.escapes");
	wrapFind = props.GetInt("find.replace.wrap", 1);
	focusOnReplace = props.GetInt("find.replacewith.focus", 1);
	subDirSearch = props.GetInt("find.in.subfolders", 1);
}

void SciTEBase::ReadPropertiesInitial() {
	SetPropertiesInitial();
	int sizeVertical = props.GetInt("output.vertical.size", 0);
	int hideOutput = props.GetInt("output.initial.hide", 0);

	ViewWhitespace(props.GetInt("view.whitespace"));
	wEditor.Call(Message::SetIndentationGuides, props.GetInt("view.indentation.guides") ?
		indentExamine : SC_IV_NONE);

	wEditor.Call(Message::SetViewEOL, props.GetInt("view.eol"));
	wEditorL.Call(Message::SetZoom, props.GetInt("magnification"));
	wEditorR.Call(Message::SetZoom, props.GetInt("right.magnification"));
	wOutput.Call(Message::SetZoom, props.GetInt("output.magnification"));
	wFindRes.Call(Message::SetZoom, props.GetInt("findres.magnification"));
	wEditor.Call(Message::SetWrapMode, wrap ? wrapStyle : SC_WRAP_NONE);
	wOutput.Call(Message::SetWrapMode, wrapOutput ? wrapStyle : SC_WRAP_NONE);
	wFindRes.Call(Message::SetWrapMode, wrapFindRes ? wrapStyle : SC_WRAP_NONE);

	FilePath homepath = GetSciteDefaultHome();
	props.Set("SciteDefaultHome", homepath.AsUTF8().c_str());
	homepath = GetSciteUserHome();
	props.Set("SciteUserHome", homepath.AsUTF8().c_str());
}

FilePath SciTEBase::GetDefaultPropertiesFileName() {
	return FilePath(GetSciteDefaultHome(), propGlobalFileName);
}

FilePath SciTEBase::GetAbbrevPropertiesFileName() {
	return FilePath(GetSciteUserHome(), propAbbrevFileName);
}

FilePath SciTEBase::GetUserPropertiesFileName() {
	return FilePath(GetSciteUserHome(), propUserFileName);
}

FilePath SciTEBase::GetLocalPropertiesFileName() {
	return FilePath(filePath.Directory(), propLocalFileName);
}

FilePath SciTEBase::GetDirectoryPropertiesFileName() {
	FilePath propfile;

	if (filePath.IsSet()) {
		propfile.Set(filePath.Directory(), propDirectoryFileName);

		// if this file does not exist try to find the prop file in a parent directory
		while (!propfile.Directory().IsRoot() && !propfile.Exists()) {
			propfile.Set(propfile.Directory().Directory(), propDirectoryFileName);
		}

		// not found -> set it to the initial directory
		if (!propfile.Exists()) {
			propfile.Set(filePath.Directory(), propDirectoryFileName);
		}
	}
	return propfile;
}

void SciTEBase::OpenProperties(int propsFile) {
	FilePath propfile;
	switch (propsFile) {
	case IDM_OPENLOCALPROPERTIES:
		propfile = GetLocalPropertiesFileName();
		Open(propfile, ofQuiet);
		break;
	case IDM_OPENUSERPROPERTIES:
		propfile = GetUserPropertiesFileName();
		Open(propfile, ofQuiet);
		break;
	case IDM_OPENABBREVPROPERTIES:
		propfile = pathAbbreviations;
		Open(propfile, ofQuiet);
		break;
	case IDM_OPENGLOBALPROPERTIES:
		propfile = GetDefaultPropertiesFileName();
		Open(propfile, ofQuiet);
		break;
	case IDM_OPENLUAEXTERNALFILE: {
			GUI::gui_string extlua = GUI::StringFromUTF8(props.GetExpanded("ext.lua.startup.script").c_str());
			if (extlua.length()) {
				Open(extlua.c_str(), ofQuiet);
			}
			break;
		}
	case IDM_OPENDIRECTORYPROPERTIES: {
			propfile = GetDirectoryPropertiesFileName();
			bool alreadyExists = propfile.Exists();
			Open(propfile, ofQuiet);
			if (!alreadyExists)
				SaveAsDialog();
		}
		break;
	}
}

// return the int value of the command name passed in.
int SciTEBase::GetMenuCommandAsInt(SString commandName) {
	int i = IFaceTable::FindConstant(commandName.c_str());
	if (i != -1) {
		return IFaceTable::constants[i].value;
	}
	// Otherwise we might have entered a number as command to access a "SCI_" command
	return commandName.value();
}

unsigned long SciTEBase::InvertColor(unsigned long clr)
{
	return convMain.Convert(clr);
}