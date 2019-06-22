// Scintilla source code edit control
/** @file Catalogue.cxx
 ** Lexer infrastructure.
 ** Contains a list of LexerModules which can be searched to find a module appropriate for a
 ** particular language.
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstdlib>
#include <cassert>
#include <cstring>

#include <stdexcept>
#include <vector>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "LexerModule.h"
#include "Catalogue.h"

using namespace Scintilla;

static std::vector<LexerModule *> lexerCatalogue;
static int nextLanguage = SCLEX_AUTOMATIC+1;

const LexerModule *Catalogue::Find(int language) {
	Scintilla_LinkLexers();
	for (const LexerModule *lm : lexerCatalogue) {
		if (lm->GetLanguage() == language) {
			return lm;
		}
	}
	return nullptr;
}

const LexerModule *Catalogue::Find(const char *languageName) {
	Scintilla_LinkLexers();
	if (languageName) {
		for (const LexerModule *lm : lexerCatalogue) {
			if (lm->languageName && (0 == strcmp(lm->languageName, languageName))) {
				return lm;
			}
		}
	}
	return nullptr;
}

void Catalogue::AddLexerModule(LexerModule *plm) {
	if (plm->GetLanguage() == SCLEX_AUTOMATIC) {
		plm->language = nextLanguage;
		nextLanguage++;
	}
	lexerCatalogue.push_back(plm);
}

// To add or remove a lexer, add or remove its file and run LexGen.py.

// Force a reference to all of the Scintilla lexers so that the linker will
// not remove the code of the lexers.
int Scintilla_LinkLexers() {

	static int initialised = 0;
	if (initialised)
		return 0;
	initialised = 1;

// Shorten the code that declares a lexer and ensures it is linked in by calling a method.
#define LINK_LEXER(lexer) extern LexerModule lexer; Catalogue::AddLexerModule(&lexer);

//++Autogenerated -- run scripts/LexGen.py to regenerate
//**\(\tLINK_LEXER(\*);\n\)
	LINK_LEXER(lmA68k);
	LINK_LEXER(lmAbaqus);
	LINK_LEXER(lmAda);
	LINK_LEXER(lmAPDL);
	LINK_LEXER(lmAs);
	LINK_LEXER(lmAsm);
	LINK_LEXER(lmAsn1);
	LINK_LEXER(lmASY);
	LINK_LEXER(lmAU3);
	LINK_LEXER(lmAVE);
	LINK_LEXER(lmAVS);
	LINK_LEXER(lmBaan);
	LINK_LEXER(lmBash);
	LINK_LEXER(lmBatch);
	LINK_LEXER(lmBibTeX);
	LINK_LEXER(lmBlitzBasic);
	LINK_LEXER(lmBullant);
	LINK_LEXER(lmCaml);
	LINK_LEXER(lmCIL);
	LINK_LEXER(lmClw);
	LINK_LEXER(lmClwNoCase);
	LINK_LEXER(lmCmake);
	LINK_LEXER(lmCOBOL);
	LINK_LEXER(lmCoffeeScript);
	LINK_LEXER(lmConf);
	LINK_LEXER(lmCPP);
	LINK_LEXER(lmCPPNoCase);
	LINK_LEXER(lmCsound);
	LINK_LEXER(lmCss);
	LINK_LEXER(lmD);
	LINK_LEXER(lmDiff);
	LINK_LEXER(lmDMAP);
	LINK_LEXER(lmDMIS);
	LINK_LEXER(lmECL);
	LINK_LEXER(lmEDIFACT);
	LINK_LEXER(lmEiffel);
	LINK_LEXER(lmEiffelkw);
	LINK_LEXER(lmErlang);
	LINK_LEXER(lmErrorList);
	LINK_LEXER(lmESCRIPT);
	LINK_LEXER(lmF77);
	LINK_LEXER(lmFlagShip);
	LINK_LEXER(lmFormEngine);
	LINK_LEXER(lmForth);
	LINK_LEXER(lmFortran);
	LINK_LEXER(lmFreeBasic);
	LINK_LEXER(lmGAP);
	LINK_LEXER(lmGui4Cli);
	LINK_LEXER(lmHaskell);
	LINK_LEXER(lmHTML);
	LINK_LEXER(lmIHex);
	LINK_LEXER(lmIndent);
	LINK_LEXER(lmInno);
	LINK_LEXER(lmJSON);
	LINK_LEXER(lmKix);
	LINK_LEXER(lmKVIrc);
	LINK_LEXER(lmLatex);
	LINK_LEXER(lmLISP);
	LINK_LEXER(lmLiterateHaskell);
	LINK_LEXER(lmLot);
	LINK_LEXER(lmLout);
	LINK_LEXER(lmLua);
	LINK_LEXER(lmMagikSF);
	LINK_LEXER(lmMake);
	LINK_LEXER(lmMarkdown);
	LINK_LEXER(lmMatlab);
	LINK_LEXER(lmMaxima);
	LINK_LEXER(lmMETAPOST);
	LINK_LEXER(lmMMIXAL);
	LINK_LEXER(lmModula);
	LINK_LEXER(lmMSSQL);
	LINK_LEXER(lmMySQL);
	LINK_LEXER(lmNim);
	LINK_LEXER(lmNimrod);
	LINK_LEXER(lmNncrontab);
	LINK_LEXER(lmNsis);
	LINK_LEXER(lmNull);
	LINK_LEXER(lmOctave);
	LINK_LEXER(lmOpal);
	LINK_LEXER(lmOScript);
	LINK_LEXER(lmPascal);
	LINK_LEXER(lmPB);
	LINK_LEXER(lmPerl);
	LINK_LEXER(lmPHPSCRIPT);
	LINK_LEXER(lmPLM);
	LINK_LEXER(lmPO);
	LINK_LEXER(lmPOV);
	LINK_LEXER(lmPowerPro);
	LINK_LEXER(lmPowerShell);
	LINK_LEXER(lmProgress);
	LINK_LEXER(lmProps);
	LINK_LEXER(lmPS);
	LINK_LEXER(lmPureBasic);
	LINK_LEXER(lmPython);
	LINK_LEXER(lmR);
	LINK_LEXER(lmREBOL);
	LINK_LEXER(lmRegistry);
	LINK_LEXER(lmRuby);
	LINK_LEXER(lmRust);
	LINK_LEXER(lmSAS);
	LINK_LEXER(lmScriptol);
	LINK_LEXER(lmSearchResult);
	LINK_LEXER(lmSmalltalk);
	LINK_LEXER(lmSML);
	LINK_LEXER(lmSorc);
	LINK_LEXER(lmSpecman);
	LINK_LEXER(lmSpice);
	LINK_LEXER(lmSQL);
	LINK_LEXER(lmSrec);
	LINK_LEXER(lmStata);
	LINK_LEXER(lmSTTXT);
	LINK_LEXER(lmTACL);
	LINK_LEXER(lmTADS3);
	LINK_LEXER(lmTAL);
	LINK_LEXER(lmTCL);
	LINK_LEXER(lmTCMD);
	LINK_LEXER(lmTEHex);
	LINK_LEXER(lmTeX);
	LINK_LEXER(lmTxt2tags);
	LINK_LEXER(lmVB);
	LINK_LEXER(lmVBScript);
	LINK_LEXER(lmVerilog);
	LINK_LEXER(lmVHDL);
	LINK_LEXER(lmVisualProlog);
	LINK_LEXER(lmX12);
	LINK_LEXER(lmXML);
	LINK_LEXER(lmYAML);

//--Autogenerated -- end of automatically generated section

	return 1;
}
