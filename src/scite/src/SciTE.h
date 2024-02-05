// SciTE - Scintilla based Text Editor
/** @file SciTE.h
 ** Define command IDs used within SciTE.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCITE_H
#define SCITE_H


// Menu IDs.
// These are located 100 apart. No one will want more than 100 in each menu ;)
#define IDM_MRUFILE			1000
//! #define IDM_TOOLS			1100
//!-start-[ToolsMax]
#define IDM_TOOLS			9000
#define IDM_TOOLSMAX		9300
//!-end-[ToolsMax]
#define IDM_BUFFER			1200
#define IDM_IMPORT			1300
#define IDM_LANGUAGE			1400
#define IDM_GENERATED       28000
#define IDM_REBOOT         667
#define IDM_CHANGETAB         668
#define IDM_CLONETAB         669

// File
#define IDM_NEW			101
#define IDM_OPEN			102
#define IDM_OPENSELECTED		103
#define IDM_REVERT			104
#define IDM_CLOSE			105
#define IDM_SAVE			106
#define IDM_SAVEAS			110
#define IDM_SAVEASHTML		111
#define IDM_SAVEASRTF		112
#define IDM_SAVEASPDF		113
#define IDM_FILER			114
#define IDM_SAVEASTEX		115
#define IDM_SAVEACOPY		116
#define IDM_SAVEASXML		117
#define IDM_COPYPATH			118
#define IDM_MRU_SEP			120
#define IDM_PRINTSETUP		130
#define IDM_PRINT			131
#define IDM_LOADSESSION		132
#define IDM_SAVESESSION		133
#define IDM_QUIT			140
#define IDM_ENCODING_DEFAULT	150
#define IDM_ENCODING_UCS2BE	151
#define IDM_ENCODING_UCS2LE	152
#define IDM_ENCODING_UTF8	153
#define IDM_ENCODING_UCOOKIE	154

#define MRU_START	17
#define IMPORT_START 21
#define TOOLS_START 3

// Edit
#define IDM_UNDO			201
#define IDM_REDO			202
#define IDM_CUT				203
#define IDM_COPY			204
#define IDM_PASTE			205
#define IDM_CLEAR			206
#define IDM_SELECTALL		207
#define IDM_PASTEANDDOWN	208
#define IDM_FIND			210
#define IDM_FINDNEXT			211
#define IDM_FINDNEXTBACK		212
#define IDM_FINDNEXTSEL		213
#define IDM_FINDNEXTBACKSEL	214
#define IDM_FINDINFILES		215
#define IDM_REPLACE			216
#define IDM_GOTO			220
#define IDM_BOOKMARK_NEXT	221
#define IDM_BOOKMARK_TOGGLE	222
#define IDM_BOOKMARK_PREV	223
#define IDM_BOOKMARK_CLEARALL	224
#define IDM_BOOKMARK_NEXT_SELECT	225
#define IDM_BOOKMARK_PREV_SELECT	226
#define IDM_MATCHBRACE		230
#define IDM_SELECTTOBRACE	231
#define IDM_SHOWCALLTIP		232
#define IDM_COMPLETE		233
#define IDM_COMPLETEWORD	234
#define IDM_EXPAND			235
#define IDM_TOGGLE_FOLDALL	236
#define IDM_TOGGLE_FOLDRECURSIVE 237
#define IDM_EXPAND_ENSURECHILDRENVISIBLE 238
#define IDM_UPRCASE			240
#define IDM_LWRCASE			241
#define IDM_ABBREV			242
#define IDM_BLOCK_COMMENT	243
#define IDM_STREAM_COMMENT	244
#define IDM_COPYASRTF		245
#define IDM_BOX_COMMENT		246
#define IDM_INS_ABBREV		247
#define IDM_JOIN		248
#define IDM_SPLIT		249
#define IDM_DUPLICATE	250
#define IDM_ENTERSELECTION  256
#define IDM_COPYASHTML      257

#define IDC_INCFINDTEXT     253
#define IDC_INCFINDBTNOK	254
#define IDC_EDIT1           1000
#define IDC_STATIC          -1


#define IDM_PREVMATCHPPC	260
#define IDM_SELECTTOPREVMATCHPPC	261
#define IDM_NEXTMATCHPPC	262
#define IDM_SELECTTONEXTMATCHPPC	263

// Tools
#define IDM_COMPILE			301
#define IDM_BUILD			302
#define IDM_GO				303
#define IDM_STOPEXECUTE		304
#define IDM_FINISHEDEXECUTE	305
#define IDM_NEXTMSG			306
#define IDM_PREVMSG			307

#define IDM_MACRO_SEP		310
#define IDM_MACRORECORD		311
#define IDM_MACROSTOPRECORD	312
#define IDM_MACROPLAY		313
#define IDM_MACROLIST		314

#define IDM_ACTIVATE			320

#define IDM_SRCWIN			350
#define IDM_COSRCWIN		356
#define IDM_RUNWIN			351
#define IDM_FINDRESWIN      355

// Options
#define IDM_SPLITVERTICAL		401
#define IDM_VIEWSPACE		402
#define IDM_VIEWEOL			403
#define IDM_VIEWGUIDES		404
#define IDM_SELMARGIN		405
#define IDM_FOLDMARGIN		406
#define IDM_LINENUMBERMARGIN	407
#define IDM_VIEWTOOLBAR		408
#define IDM_TOGGLEOUTPUT		409
#define IDM_VIEWTABBAR		410
#define IDM_VIEWSTATUSBAR	411
#define IDM_TOGGLEPARAMETERS	412
#define IDM_OPENFILESHERE		413
#define IDM_WRAP			414
#define IDM_WRAPOUTPUT		415
#define IDM_READONLY			416
#define IDM_VIEWTLBARIUP		417
#define IDM_WRAPFINDRES                 418
#define IDM_CLEARFINDRES                419

#define IDM_CLEAROUTPUT		420
#define IDM_SWITCHPANE			421
#define IDM_FINDRESENSUREVISIBLE        422

#define IDM_EOL_CRLF			430
#define IDM_EOL_CR			431
#define IDM_EOL_LF			432
#define IDM_EOL_CONVERT		433

#define IDM_MONOFONT		450

#define IDM_OPENLOCALPROPERTIES	460
#define IDM_OPENUSERPROPERTIES	461
#define IDM_OPENGLOBALPROPERTIES	462
#define IDM_OPENABBREVPROPERTIES	463
#define IDM_OPENLUAEXTERNALFILE	464
#define IDM_OPENDIRECTORYPROPERTIES	465

//#define IDM_SELECTIONMARGIN	490
//#define IDM_BUFFEREDDRAW	491
//#define IDM_USEPALETTE		492

// Buffers
#define IDM_PREVFILE			501
#define IDM_NEXTFILE			502
#define IDM_CLOSEALL			503
#define IDM_SAVEALL			504
#define IDM_BUFFERSEP		505
#define IDM_PREVFILESTACK			506
#define IDM_NEXTFILESTACK			507
#define IDM_MOVETABRIGHT			508
#define IDM_MOVETABLEFT			509
#define IDM_VIEWHISTORYINDICATORS       510
#define IDM_VIEWHISTORYMARKERS          511
#define IDM_COPYASHTMLTEXT         512

#define IDM_WHOLEWORD			800
#define IDM_MATCHCASE			801
#define IDM_REGEXP					802
#define IDM_WRAPAROUND		803
#define IDM_UNSLASH				804
#define IDM_DIRECTIONUP			805
#define IDM_DIRECTIONDOWN	806
#define IDM_CLOSEALLBUTCURRENT	   9132
#define IDM_CLOSEALLTEMPORALLY	   9134

// Help
#define IDM_HELP			901
#define IDM_ABOUT			902
#define IDM_HELP_SCITE		903

// Windows specific windowing options
#define IDM_ONTOP			960
#define IDM_FULLSCREEN		961
#define IDC_TABCLOSE		962
#define IDC_SHIFTTAB		963
#define IDC_TABDBLCLK		964 //!-add-[close_on_dbl_clk]

// Dialog control IDs
#define IDGOLINE			220
#define IDABOUTSCINTILLA	221
#define IDFINDWHAT			222
#define IDFILES				223
#define IDDIRECTORY			224
#define IDCURRLINE			225
#define IDLASTLINE			226
#define IDEXTEND			227
#define IDTABSIZE			228
#define IDINDENTSIZE		229
#define IDUSETABS			230

#define IDREPLACEWITH		231
#define IDWHOLEWORD			232
#define IDMATCHCASE			233
#define IDDIRECTIONUP		234
#define IDDIRECTIONDOWN		235
#define IDREPLACE			236
#define IDREPLACEALL		237
#define IDREPLACEINSEL		238
#define IDREGEXP			239
#define IDWRAP			    240
#define IDSUBDIRSEARCH      1002

#define IDUNSLASH			241
#define IDCMD			242

// id for the browse button in the grep dialog
#define IDBROWSE 243

#define IDABBREV			244

#define IDREPLACEINBUF		244
#define IDMARKALL 			245
#define IDFINDALL 			246

#define IDGOLINECHAR		246
#define IDCURRLINECHAR		247
#define IDREPLDONE			248

#define IDDOTDOT			249
#define IDFINDINSTYLE		250
#define IDFINDSTYLE			251
#define IDCONVERT			252

#define IDPARAMSTART		300

// Dialog IDs
#define IDD_FIND			400
#define IDD_REPLACE			401
#define IDD_BUFFERS			402
#define IDD_FIND_ADV		403
#define IDD_REPLACE_ADV		404

#endif
#define UPDATE_BLOCK 0
#define UPDATE_FORCE 2
#define UPDATE_UNBLOCK 1

#define SCITE_TRAY 0x8000
#define SCITE_DROP 0x8001
#define SCITE_NOTIYCMD 0x8002
#define SCITE_NOTIFYCMDEXIT 0x8003
#define SCITE_NEEDNCPAINT 0x8004
#define SCITE_NOTIFYTREAD 0x8008

