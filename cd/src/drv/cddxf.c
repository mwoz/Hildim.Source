/** \file
 * \brief DXF driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "cd.h"
#include "cd_private.h"
#include "cddxf.h"


#ifndef ignore
#define ignore(x) (void)x
#endif

#ifndef max
#define max(x, y) ((x > y)? x : y)
#endif

#ifndef min
#define min(x, y) ((x < y)? x : y)
#endif

struct _cdCtxCanvas
{
  cdCanvas* canvas;

  FILE *file;        /* pointer to file                        */
  int layer;         /* layer                                  */

  int font_index;           
  double text_height;         /* (in points)          */
  int text_oblique_angle;     /* (for italics)        */
  int text_horiz_align,       /* horizontal alignment */
      text_vert_align;        /* vertical alignment   */

  int fgcolor_index;

  int line_type_index;
  double line_width;

  /* AutoCAD 2000 only */
  int acad2000;      /* Use new DXF version  */
  int handle;        /* next handle, starts at 0x30 */
};


static void write_code(cdCtxCanvas *ctxcanvas, int code, const char* value)
{
  fprintf (ctxcanvas->file, "%d\n", code);
  fprintf (ctxcanvas->file, "%s\n", value);
}

static void write_code_int(cdCtxCanvas *ctxcanvas, int code, int value)
{
  fprintf (ctxcanvas->file, "%d\n", code);
  fprintf (ctxcanvas->file, "%d\n", value);
}

static void write_code_hex(cdCtxCanvas *ctxcanvas, int code, int value)
{
  fprintf (ctxcanvas->file, "%d\n", code);
  fprintf (ctxcanvas->file, "%0X\n", value);
}

static void write_code_real(cdCtxCanvas *ctxcanvas, int code, double value)
{
  fprintf (ctxcanvas->file, "%d\n", code);
  fprintf (ctxcanvas->file, "%f\n", value);
}

static void write_header_variable(cdCtxCanvas *ctxcanvas, const char* variable)
{
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$%s\n", variable);
}

static void begin_section(cdCtxCanvas *ctxcanvas, const char* section_name)
{
  write_code(ctxcanvas, 0, "SECTION");
  write_code(ctxcanvas, 2, section_name);
}

static void end_section(cdCtxCanvas *ctxcanvas)
{
  write_code(ctxcanvas, 0, "ENDSEC");
}

static void write_unique_handle(cdCtxCanvas *ctxcanvas)
{
  write_code_hex(ctxcanvas, 5, ctxcanvas->handle);
  ctxcanvas->handle++;
}

static void write_header(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  /* The AutoCAD drawing database version number */
  write_header_variable(ctxcanvas, "ACADVER");
  if(ctxcanvas->acad2000)
    write_code(ctxcanvas, 1, "AC1015");  /* AutoCAD 2000 */
  else
    write_code(ctxcanvas, 1, "AC1006");  /* AutoCAD R10 */

  if (ctxcanvas->acad2000)
  {
    /* Next available handle */
    write_header_variable(ctxcanvas, "HANDSEED");
    write_code(ctxcanvas, 5, "FFFF");

    /* drawing units for AutoCAD DesignCenter blocks */
    write_header_variable(ctxcanvas, "INSUNITS");
    write_code(ctxcanvas, 70, "4");  /* 4 = Millimeters */
     
    /* Extension line extension */
    write_header_variable(ctxcanvas, "DIMEXE");
    write_code(ctxcanvas, 40, "1.25");
  }

  /* Nonzero if limits checking is on */
  write_header_variable(ctxcanvas, "LIMCHECK");  
  write_code(ctxcanvas, 70, "1");

  /* XY drawing limits lower-left corner (in WCS) */
  write_header_variable(ctxcanvas, "LIMMIN");
  write_code_real(ctxcanvas, 10, xmin);
  write_code_real(ctxcanvas, 20, ymin);
    
  /* XY drawing limits upper-right corner (in WCS) */
  write_header_variable(ctxcanvas, "LIMMAX");
  write_code_real(ctxcanvas, 10, xmax);
  write_code_real(ctxcanvas, 20, ymax);

  /* X, Y, and Z drawing extents lower-left corner (in WCS) */
  write_header_variable(ctxcanvas, "EXTMIN");
  write_code_real(ctxcanvas, 10, xmin);
  write_code_real(ctxcanvas, 20, ymin);

  /* X, Y, and Z drawing extents upper-right corner (in WCS) */
  write_header_variable(ctxcanvas, "EXTMAX");
  write_code_real(ctxcanvas, 10, xmax);
  write_code_real(ctxcanvas, 20, ymax);

  /* Current layer name */
  write_header_variable(ctxcanvas, "CLAYER");
  write_code(ctxcanvas, 8, "0");
  /* Units format for coordinates and distances */
  write_header_variable(ctxcanvas, "LUNITS");
  write_code(ctxcanvas, 70, "2");  /* 2 = Decimal */
  /* Units precision for coordinates and distances */
  write_header_variable(ctxcanvas, "LUPREC");
  write_code_int(ctxcanvas, 70, (int)ceil(log10(ctxcanvas->canvas->xres)));  /* precision (resolution dependant) */
  /* Units format for angles */
  write_header_variable(ctxcanvas, "AUNITS");
  write_code(ctxcanvas, 70, "0");  /* 0 = Decimal degrees */
  /* Units precision for angles */
  write_header_variable(ctxcanvas, "AUPREC");
  write_code(ctxcanvas, 70, "2");
  /* Current text style name */
  write_header_variable(ctxcanvas, "TEXTSTYLE");
  write_code(ctxcanvas, 7, "STANDARD");
}

static void begin_table(cdCtxCanvas *ctxcanvas, const char* name, int count, const char* handle)
{
  write_code(ctxcanvas, 0, "TABLE");
  write_code(ctxcanvas, 2, name);

  if (ctxcanvas->acad2000)
  {
    write_code(ctxcanvas, 5, handle);
    write_code(ctxcanvas, 100, "AcDbSymbolTable");
  }

  /* Maximum number of entries in table */
  write_code_int(ctxcanvas, 70, count);
}

static void end_table(cdCtxCanvas *ctxcanvas)
{
  write_code(ctxcanvas, 0, "ENDTAB");
}

static void write_line_type_name (cdCtxCanvas* ctxcanvas, int code, int t)   /* write name of a line */
{

  static char *line[] =
  {"CONTINUOUS",
   "DASHED",
   "HIDDEN",
   "CENTER",
   "PHANTOM",
   "DOT",
   "DASHDOT",
   "BORDER",
   "DIVIDE"};

/*
   AutoCAD line styles ( see acad.lin ):

   0 CONTINUOUS  ____________________________________________
   1 DASHED      __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
   2 HIDDEN      _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
   3 CENTER      ____ _ ____ _ ____ _ ____ _ ____ _ ____ _ __
   4 PHANTOM     _____ _ _ _____ _ _ _____ _ _ _____ _ _ ____
   5 DOT         ............................................
   6 DASHDOT     __ . __ . __ . __ . __ . __ . __ . __ . __ .
   7 BORDER      __ __ . __ __ . __ __ . __ __ . __ __ . __ _
   8 DIVIDE      __ . . __ . . __ . . __ . . __ . . __ . . __

*/

  write_code(ctxcanvas, code, line[t]);
}

static void write_font_type (cdCtxCanvas *ctxcanvas, int code, int t)   /* write name of a font */
{
  static char *font[7] =
  {
    "STANDARD",
    "ROMAN",
    "ROMAN_BOLD",
    "ROMANTIC",
    "ROMANTIC_BOLD",
    "SANSSERIF",
    "SANSSERIF_BOLD",
  };
/*
             CD Fonts / Style                 AutoCAD Fonts
      -------------------------------------------------------------------
       CD_SYSTEM                           0 STANDARD
       CD_COURIER     / CD_PLAIN           1 ROMAN
       CD_COURIER     / CD_BOLD            2 ROMAN_BOLD
       CD_COURIER     / CD_ITALIC          1 ROMAN          (oblique angle = 15)
       CD_COURIER     / CD_BOLD_ITALIC     2 ROMAN_BOLD     (oblique angle = 15)
       CD_TIMES_ROMAN / CD_PLAIN           3 ROMANTIC
       CD_TIMES_ROMAN / CD_BOLD            4 ROMANTIC_BOLD
       CD_TIMES_ROMAN / CD_ITALIC          3 ROMANTIC       (oblique angle = 15)
       CD_TIMES_ROMAN / CD_BOLD_ITALIC     4 ROMANTIC_BOLD  (oblique angle = 15)
       CD_HELVETICA   / CD_PLAIN           5 SANSSERIF
       CD_HELVETICA   / CD_BOLD            6 SANSSERIF_BOLD
       CD_HELVETICA   / CD_ITALIC          5 SANSSERIF      (oblique angle = 15)
       CD_HELVETICA   / CD_BOLD_ITALIC     6 SANSSERIF_BOLD (oblique angle = 15)
*/

  write_code(ctxcanvas, code, font[t]);
}

static void write_line_type(cdCtxCanvas *ctxcanvas, int j, int* tab, const char* desc)
{
  int i;

  /* Object type */
  write_code(ctxcanvas, 0, "LTYPE");
  if (ctxcanvas->acad2000)
  {
    write_unique_handle(ctxcanvas);
    write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
    write_code(ctxcanvas, 100, "AcDbLinetypeTableRecord");
  }

  /* Linetype name */
  if (j == -1)
    write_code(ctxcanvas, 2, desc);
  else
    write_line_type_name (ctxcanvas, 2, j);
  /* Standard flag values (bit-coded values) */
  write_code(ctxcanvas, 70, "0");  /* need not be set by programs that write DXF files */

  /* other group codes */

  /* Descriptive text for linetype */
  if (j == -1)
    write_code(ctxcanvas, 3, "");
  else
    write_code(ctxcanvas, 3, desc);
  /* Alignment code; value is always 65, the ASCII code for A */
  write_code(ctxcanvas, 72, "65");
  /* The number of linetype elements */
  write_code_int(ctxcanvas, 73, tab[0]);
  /* Total pattern length */
  write_code_int(ctxcanvas, 40, tab[1]);

  for (i = 2; i < 2 + tab[0]; i++)
  {
    /* Dash, dot or space length (one entry per element) */
    write_code_int(ctxcanvas, 49, tab[i]);  /* parameters */

    if (ctxcanvas->acad2000)
      write_code(ctxcanvas, 74, "0");
  }
}

static void write_line_types (cdCtxCanvas *ctxcanvas)
{
  int j;
  static char *line[9] = {
    "Solid line",
    "Dashed line",
    "Hidden line",
    "Center line",
    "Phantom line",
    "Dot line",
    "Dashdot line",
    "Border line",
    "Divide Line"};

  static int tab[9][8] =  {
    { 0,  0, 0 ,  0,  0,  0, 0,  0 },
    { 2, 15, 10, -5,  0,  0, 0,  0 },
    { 2, 10, 5 , -5,  0,  0, 0,  0 },
    { 4, 35, 20, -5,  5, -5, 0,  0 },
    { 6, 50, 25, -5,  5, -5, 5, -5 },
    { 2,  5, 0 , -5,  0,  0, 0,  0 },
    { 4, 20, 10, -5,  0, -5, 0,  0 },
    { 6, 35, 10, -5, 10, -5, 0, -5 },
    { 6, 25, 10, -5,  0, -5, 0, -5 }
  };

  begin_table(ctxcanvas, "LTYPE", 11, "5");   /* Line Types table handle=5 */

  write_line_type(ctxcanvas, -1, tab[0], "BYBLOCK");
  write_line_type(ctxcanvas, -1, tab[0], "BYLAYER");

  for (j = 0; j < 9; j++)
    write_line_type(ctxcanvas, j, tab[j], line[j]);

  end_table(ctxcanvas);
}

static void write_fonts (cdCtxCanvas *ctxcanvas)
{
  int i;
  static char *font[6] =
  {
    "romanc.shx",
    "romant.shx",
    "rom_____.pfb",
    "romb____.pfb",
    "sas_____.pfb",
    "sasb____.pfb"
  };

  begin_table(ctxcanvas, "STYLE", 6, "3");   /* Style table handle=3 */

  for (i = 0; i < 6; i++)
  {
    /* Object type */
    write_code(ctxcanvas, 0, "STYLE");
    if (ctxcanvas->acad2000)
    {
      write_unique_handle(ctxcanvas);
      write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
      write_code(ctxcanvas, 100, "AcDbTextStyleTableRecord");
    }

    /* Style name */
    write_font_type (ctxcanvas, 2, i+1);  /* skip Standard */
    /* Standard flag values (bit-coded values) */
    write_code(ctxcanvas, 70, "0");  /* need not be set by programs that write DXF files */

    /* other group codes */

    /* Primary font file name */
    write_code(ctxcanvas, 3, font[i]);    /* font style file */
    /* Text generation flags */
    write_code(ctxcanvas, 71, "0");  /* normal, no mirror */
    /* Fixed text height */
    write_code(ctxcanvas, 40, "0");  /* not fixed */
    /* Width factor */
    write_code(ctxcanvas, 41, "1");
    /* Last height used */
    write_code(ctxcanvas, 42, "0");
    /* Bigfont file name; blank if none */
    write_code(ctxcanvas, 4, "");
    /* Oblique angle */
    write_code(ctxcanvas, 50, "0");
  }

  end_table(ctxcanvas);
}

static void write_vport(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "VPORT", 1, "8");    /* View port table handle=8 */

  write_code(ctxcanvas, 0, "VPORT");
  write_unique_handle(ctxcanvas);
  write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas, 100, "AcDbViewportTableRecord");

  write_code(ctxcanvas,   2, "*Active");
  write_code(ctxcanvas,  70, "0");
  write_code(ctxcanvas,  10, "0.0");
  write_code(ctxcanvas,  20, "0.0");
  write_code(ctxcanvas,  11, "1.0");
  write_code(ctxcanvas,  21, "1.0");
  write_code(ctxcanvas,  12, "286.3055555555554861");
  write_code(ctxcanvas,  22, "148.5");
  write_code(ctxcanvas,  13, "0.0");
  write_code(ctxcanvas,  23, "0.0");
  write_code(ctxcanvas,  14, "10.0");
  write_code(ctxcanvas,  24, "10.0");
  write_code(ctxcanvas,  15, "10.0");
  write_code(ctxcanvas,  25, "10.0");
  write_code(ctxcanvas,  16, "0.0");
  write_code(ctxcanvas,  26, "0.0");
  write_code(ctxcanvas,  36, "1.0");
  write_code(ctxcanvas,  17, "0.0");
  write_code(ctxcanvas,  27, "0.0");
  write_code(ctxcanvas,  37, "0.0");
  write_code(ctxcanvas,  40, "297.0");
  write_code(ctxcanvas,  41, "1.92798353909465");
  write_code(ctxcanvas,  42, "50.0");
  write_code(ctxcanvas,  43, "0.0");
  write_code(ctxcanvas,  44, "0.0");
  write_code(ctxcanvas,  50, "0.0");
  write_code(ctxcanvas,  51, "0.0");
  write_code(ctxcanvas,  71, "0");
  write_code(ctxcanvas,  72, "100");
  write_code(ctxcanvas,  73, "1");
  write_code(ctxcanvas,  74, "3");
  write_code(ctxcanvas,  75, "1");
  write_code(ctxcanvas,  76, "1");
  write_code(ctxcanvas,  77, "0");
  write_code(ctxcanvas,  78, "0");
  write_code(ctxcanvas, 281, "0");
  write_code(ctxcanvas,  65, "1");
  write_code(ctxcanvas, 110, "0.0");
  write_code(ctxcanvas, 120, "0.0");
  write_code(ctxcanvas, 130, "0.0");
  write_code(ctxcanvas, 111, "1.0");
  write_code(ctxcanvas, 121, "0.0");
  write_code(ctxcanvas, 131, "0.0");
  write_code(ctxcanvas, 112, "0.0");
  write_code(ctxcanvas, 122, "1.0");
  write_code(ctxcanvas, 132, "0.0");
  write_code(ctxcanvas,  79, "0");
  write_code(ctxcanvas, 146, "0.0");

  end_table(ctxcanvas);
}

static void write_layers(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "LAYER", 1, "2");    /* Layers table handle=2 */

  write_code(ctxcanvas, 0, "LAYER");
  write_code(ctxcanvas, 5, "10");   /* layer entry handle=10 */
  write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas, 100, "AcDbLayerTableRecord");

  write_code(ctxcanvas, 2, "0");
  write_code(ctxcanvas, 70, "0");
  write_code(ctxcanvas, 62, "7");
  write_code(ctxcanvas, 6, "CONTINUOUS");
  write_code(ctxcanvas, 390, "F");

  end_table(ctxcanvas);
}

static void write_view(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "VIEW", 0, "6");    /* View table handle=6 */
    /* empty */
  end_table(ctxcanvas);
}

static void write_ucs(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "UCS", 0, "7");    /* UCS table handle=7 */
    /* empty */
  end_table(ctxcanvas);
}

static void write_appid(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "APPID", 1, "9");    /* APPID table handle=9 */

  write_code(ctxcanvas, 0, "APPID");
  write_code(ctxcanvas, 5, "12");   /* appid entry handle=12 */
  write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas, 100, "AcDbRegAppTableRecord");

  write_code(ctxcanvas, 2, "ACAD");
  write_code(ctxcanvas, 70, "0");

  end_table(ctxcanvas);
}

static void write_dimstyle(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "DIMSTYLE", 1, "A");    /* DIMSTYLE table handle=A */
  write_code(ctxcanvas, 100, "AcDbDimStyleTable");
  write_code(ctxcanvas, 71, "0");

  write_code(ctxcanvas, 0, "DIMSTYLE");
  write_code(ctxcanvas, 105, "27");   /* dimstyle entry handle=27, notice the different code for DIMSTYLE entry handle */
  write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas, 100, "AcDbDimStyleTableRecord");

  write_code(ctxcanvas, 2, "Standard");
  write_code(ctxcanvas, 41, "1");
  write_code(ctxcanvas, 42, "1");
  write_code(ctxcanvas, 43, "3.75");
  write_code(ctxcanvas, 44, "1");
  write_code(ctxcanvas, 70, "0");
  write_code(ctxcanvas, 73, "0");
  write_code(ctxcanvas, 74, "0");
  write_code(ctxcanvas, 77, "1");
  write_code(ctxcanvas, 78, "8");
  write_code(ctxcanvas,140, "1");
  write_code(ctxcanvas,141, "2.5");
  write_code(ctxcanvas,143, "0.03937007874016");
  write_code(ctxcanvas,147, "1");
  write_code(ctxcanvas,171, "3");
  write_code(ctxcanvas,172, "1");
  write_code(ctxcanvas,271, "2");
  write_code(ctxcanvas,272, "2");
  write_code(ctxcanvas,274, "3");
  write_code(ctxcanvas,278, "44");
  write_code(ctxcanvas,283, "0");
  write_code(ctxcanvas,284, "8");
  write_code(ctxcanvas,340, "11");

  end_table(ctxcanvas);
}

static void write_blockrecord(cdCtxCanvas *ctxcanvas)
{
  /* Used here, only for AutoCAD 2000 */

  begin_table(ctxcanvas, "BLOCK_RECORD", 3, "1");    /* BLOCK_RECORD table handle=1 */

  write_code(ctxcanvas, 0, "BLOCK_RECORD");
  write_code(ctxcanvas, 5, "1F");   /* blockrecord entry handle=1F */
  write_code(ctxcanvas, 100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas, 100, "AcDbBlockTableRecord");
  write_code(ctxcanvas,  2, "*Model_Space");
  write_code(ctxcanvas,340, "22");

  write_code(ctxcanvas, 0, "BLOCK_RECORD");
  write_code(ctxcanvas,5, "1B");   /* blockrecord entry handle=1B */
  write_code(ctxcanvas,100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas,100, "AcDbBlockTableRecord");
  write_code(ctxcanvas,  2, "*Paper_Space");
  write_code(ctxcanvas,340, "1E");

  write_code(ctxcanvas, 0, "BLOCK_RECORD");
  write_code(ctxcanvas,5, "23");  /* blockrecord entry handle=23 */
  write_code(ctxcanvas,100, "AcDbSymbolTableRecord");
  write_code(ctxcanvas,100, "AcDbBlockTableRecord");
  write_code(ctxcanvas,  2, "*Paper_Space0");
  write_code(ctxcanvas,340, "26");

  end_table(ctxcanvas);
}

static void write_block(cdCtxCanvas *ctxcanvas, const char* name, const char* handle_begin, const char* handle_end, int paper)
{
  /* Used here, only for AutoCAD 2000 */

  write_code(ctxcanvas, 0, "BLOCK");
  write_code(ctxcanvas, 5, handle_begin);
  write_code(ctxcanvas, 100, "AcDbEntity");
  if (paper)
    write_code(ctxcanvas, 67, "1");
  write_code(ctxcanvas, 8, "0");
  write_code(ctxcanvas, 100, "AcDbBlockBegin");
  write_code(ctxcanvas,2, name);
  write_code(ctxcanvas,70, "0");
  write_code(ctxcanvas,10, "0");
  write_code(ctxcanvas,20, "0");
  write_code(ctxcanvas,30, "0");
  write_code(ctxcanvas,3, name);
  write_code(ctxcanvas,1, "");

  write_code(ctxcanvas, 0, "ENDBLK");
  write_code(ctxcanvas, 5, handle_end);
  write_code(ctxcanvas, 100, "AcDbEntity");
  if (paper)
    write_code(ctxcanvas, 67, "1");
  write_code(ctxcanvas, 8, "0");
  write_code(ctxcanvas, 100, "AcDbBlockEnd");
}

static void write_objects(cdCtxCanvas *ctxcanvas)
{
  write_code(ctxcanvas, 0, "DICTIONARY");
  write_code_hex(ctxcanvas,5, 0xC); 
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,280, 0);
  write_code_int(ctxcanvas,281, 1);
  write_code(ctxcanvas, 3, "ACAD_GROUP");
  write_code_hex(ctxcanvas,350, 0xD);
  write_code(ctxcanvas, 3, "ACAD_LAYOUT");
  write_code_hex(ctxcanvas,350, 0x1A);
  write_code(ctxcanvas, 3, "ACAD_MLINESTYLE");
  write_code_hex(ctxcanvas,350, 0x17);
  write_code(ctxcanvas, 3, "ACAD_PLOTSETTINGS");
  write_code_hex(ctxcanvas,350, 0x19);
  write_code(ctxcanvas, 3, "ACAD_PLOTSTYLENAME");
  write_code_hex(ctxcanvas,350, 0xE);
  write_code(ctxcanvas, 3, "AcDbVariableDictionary");
  write_code_hex(ctxcanvas,350, ctxcanvas->handle);

  write_code(ctxcanvas, 0, "DICTIONARY");
  write_code_hex(ctxcanvas,5, 0xD);
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,280, 0);
  write_code_int(ctxcanvas,281, 1);

  write_code(ctxcanvas, 0, "ACDBDICTIONARYWDFLT");
  write_code_hex(ctxcanvas,5, 0xE);
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,281, 1);
  write_code(ctxcanvas, 3, "Normal");
  write_code_hex(ctxcanvas,350, 0xF);
  write_code(ctxcanvas,100, "AcDbDictionaryWithDefault");
  write_code_hex(ctxcanvas,340, 0xF);

  write_code(ctxcanvas, 0, "ACDBPLACEHOLDER");
  write_code_hex(ctxcanvas,5, 0xF);

  write_code(ctxcanvas, 0, "DICTIONARY");
  write_code_hex(ctxcanvas,5, 0x17);
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,280, 0);
  write_code_int(ctxcanvas,281, 1);
  write_code(ctxcanvas, 3, "Standard");
  write_code_hex(ctxcanvas,350, 0x18);

  write_code(ctxcanvas, 0, "MLINESTYLE");
  write_code_hex(ctxcanvas,5, 0x18);
  write_code(ctxcanvas,100, "AcDbMlineStyle");
  write_code(ctxcanvas, 2, "STANDARD");
  write_code_int(ctxcanvas, 70, 0);
  write_code(ctxcanvas, 3, "");
  write_code_int(ctxcanvas, 62, 256);
  write_code_real(ctxcanvas, 51, 90.0);
  write_code_real(ctxcanvas, 52, 90.0);
  write_code_int(ctxcanvas, 71, 2);
  write_code_real(ctxcanvas, 49, 0.5);
  write_code_int(ctxcanvas, 62, 256);
  write_code(ctxcanvas, 6, "BYLAYER");
  write_code_real(ctxcanvas, 49, -0.5);
  write_code_int(ctxcanvas, 62, 256);
  write_code(ctxcanvas, 6, "BYLAYER");

  write_code(ctxcanvas, 0, "DICTIONARY");
  write_code_hex(ctxcanvas,5, 0x19);
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,280, 0);
  write_code_int(ctxcanvas,281, 1);

  write_code(ctxcanvas, 0, "DICTIONARY");
  write_code_hex(ctxcanvas,5, 0x1A);
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,281, 1);
  write_code(ctxcanvas, 3, "Layout1");
  write_code_hex(ctxcanvas,350, 0x1E);
  write_code(ctxcanvas, 3, "Layout2");
  write_code_hex(ctxcanvas,350, 0x26);
  write_code(ctxcanvas, 3, "Model");
  write_code_hex(ctxcanvas,350, 0x22);

  write_code(ctxcanvas, 0, "LAYOUT");
  write_code_hex(ctxcanvas,5, 0x1E);
  write_code(ctxcanvas,100, "AcDbPlotSettings");
  write_code(ctxcanvas, 1, "");
  write_code(ctxcanvas, 2, "");
  write_code(ctxcanvas, 4, "");
  write_code(ctxcanvas, 6, "");
  write_code_real(ctxcanvas, 40, 0.0);
  write_code_real(ctxcanvas, 41, 0.0);
  write_code_real(ctxcanvas, 42, 0.0);
  write_code_real(ctxcanvas, 43, 0.0);
  write_code_real(ctxcanvas, 44, 0.0);
  write_code_real(ctxcanvas, 45, 0.0);
  write_code_real(ctxcanvas, 46, 0.0);
  write_code_real(ctxcanvas, 47, 0.0);
  write_code_real(ctxcanvas, 48, 0.0);
  write_code_real(ctxcanvas, 49, 0.0);
  write_code_real(ctxcanvas,140, 0.0);
  write_code_real(ctxcanvas,141, 0.0);
  write_code_real(ctxcanvas,142, 1.0);
  write_code_real(ctxcanvas,143, 1.0);
  write_code_int(ctxcanvas, 70, 688);
  write_code_int(ctxcanvas, 72, 0);
  write_code_int(ctxcanvas, 73, 0);
  write_code_int(ctxcanvas, 74, 5);
  write_code(ctxcanvas, 7, "");
  write_code_int(ctxcanvas, 75, 16);
  write_code_real(ctxcanvas,147, 1.0);
  write_code_real(ctxcanvas,148, 0.0);
  write_code_real(ctxcanvas,149, 0.0);
  write_code(ctxcanvas,100, "AcDbLayout");
  write_code(ctxcanvas, 1, "Layout1");
  write_code_int(ctxcanvas, 70, 1);
  write_code_int(ctxcanvas, 71, 1);
  write_code_real(ctxcanvas, 10, 0.0);
  write_code_real(ctxcanvas, 20, 0.0);
  write_code_real(ctxcanvas, 11, 420.0);
  write_code_real(ctxcanvas, 21, 297.0);
  write_code_real(ctxcanvas, 12, 0.0);
  write_code_real(ctxcanvas, 22, 0.0);
  write_code_real(ctxcanvas, 32, 0.0);
  write_code_real(ctxcanvas, 14, 1.000000000000000E+20);
  write_code_real(ctxcanvas, 24, 1.000000000000000E+20);
  write_code_real(ctxcanvas, 34, 1.000000000000000E+20);
  write_code_real(ctxcanvas, 15, -1.000000000000000E+20);
  write_code_real(ctxcanvas, 25, -1.000000000000000E+20);
  write_code_real(ctxcanvas, 35, -1.000000000000000E+20);
  write_code_real(ctxcanvas,146, 0.0);
  write_code_real(ctxcanvas, 13, 0.0);
  write_code_real(ctxcanvas, 23, 0.0);
  write_code_real(ctxcanvas, 33, 0.0);
  write_code_real(ctxcanvas, 16, 1.0);
  write_code_real(ctxcanvas, 26, 0.0);
  write_code_real(ctxcanvas, 36, 0.0);
  write_code_real(ctxcanvas, 17, 0.0);
  write_code_real(ctxcanvas, 27, 1.0);
  write_code_real(ctxcanvas, 37, 0.0);
  write_code_int(ctxcanvas, 76, 0);
  write_code_hex(ctxcanvas,330, 0x1B);

  write_code(ctxcanvas, 0, "LAYOUT");
  write_code_hex(ctxcanvas,5, 0x22);
  write_code(ctxcanvas,100, "AcDbPlotSettings");
  write_code(ctxcanvas, 1, "");
  write_code(ctxcanvas, 2, "");
  write_code(ctxcanvas, 4, "");
  write_code(ctxcanvas, 6, "");
  write_code_real(ctxcanvas, 40, 0.0);
  write_code_real(ctxcanvas, 41, 0.0);
  write_code_real(ctxcanvas, 42, 0.0);
  write_code_real(ctxcanvas, 43, 0.0);
  write_code_real(ctxcanvas, 44, 0.0);
  write_code_real(ctxcanvas, 45, 0.0);
  write_code_real(ctxcanvas, 46, 0.0);
  write_code_real(ctxcanvas, 47, 0.0);
  write_code_real(ctxcanvas, 48, 0.0);
  write_code_real(ctxcanvas, 49, 0.0);
  write_code_real(ctxcanvas,140, 0.0);
  write_code_real(ctxcanvas,141, 0.0);
  write_code_real(ctxcanvas,142, 1.0);
  write_code_real(ctxcanvas,143, 1.0);
  write_code_int(ctxcanvas, 70, 1712);
  write_code_int(ctxcanvas, 72, 0);
  write_code_int(ctxcanvas, 73, 0);
  write_code_int(ctxcanvas, 74, 0);
  write_code(ctxcanvas, 7, "");
  write_code_int(ctxcanvas, 75, 0);
  write_code_real(ctxcanvas,147, 1.0);
  write_code_real(ctxcanvas,148, 0.0);
  write_code_real(ctxcanvas,149, 0.0);
  write_code(ctxcanvas,100, "AcDbLayout");
  write_code(ctxcanvas, 1, "Model");
  write_code_int(ctxcanvas, 70, 1);
  write_code_int(ctxcanvas, 71, 0);
  write_code_real(ctxcanvas, 10, 0.0);
  write_code_real(ctxcanvas, 20, 0.0);
  write_code_real(ctxcanvas, 11, 12.0);
  write_code_real(ctxcanvas, 21, 9.0);
  write_code_real(ctxcanvas, 12, 0.0);
  write_code_real(ctxcanvas, 22, 0.0);
  write_code_real(ctxcanvas, 32, 0.0);
  write_code_real(ctxcanvas, 14, 0.0);
  write_code_real(ctxcanvas, 24, 0.0);
  write_code_real(ctxcanvas, 34, 0.0);
  write_code_real(ctxcanvas, 15, 0.0);
  write_code_real(ctxcanvas, 25, 0.0);
  write_code_real(ctxcanvas, 35, 0.0);
  write_code_real(ctxcanvas,146, 0.0);
  write_code_real(ctxcanvas, 13, 0.0);
  write_code_real(ctxcanvas, 23, 0.0);
  write_code_real(ctxcanvas, 33, 0.0);
  write_code_real(ctxcanvas, 16, 1.0);
  write_code_real(ctxcanvas, 26, 0.0);
  write_code_real(ctxcanvas, 36, 0.0);
  write_code_real(ctxcanvas, 17, 0.0);
  write_code_real(ctxcanvas, 27, 1.0);
  write_code_real(ctxcanvas, 37, 0.0);
  write_code_int(ctxcanvas, 76, 0);
  write_code_hex(ctxcanvas,330, 0x1F);

  write_code(ctxcanvas, 0, "LAYOUT");
  write_code_hex(ctxcanvas,5, 0x26);
  write_code(ctxcanvas,100, "AcDbPlotSettings");
  write_code(ctxcanvas, 1, "");
  write_code(ctxcanvas, 2, "");
  write_code(ctxcanvas, 4, "");
  write_code(ctxcanvas, 6, "");
  write_code_real(ctxcanvas, 40, 0.0);
  write_code_real(ctxcanvas, 41, 0.0);
  write_code_real(ctxcanvas, 42, 0.0);
  write_code_real(ctxcanvas, 43, 0.0);
  write_code_real(ctxcanvas, 44, 0.0);
  write_code_real(ctxcanvas, 45, 0.0);
  write_code_real(ctxcanvas, 46, 0.0);
  write_code_real(ctxcanvas, 47, 0.0);
  write_code_real(ctxcanvas, 48, 0.0);
  write_code_real(ctxcanvas, 49, 0.0);
  write_code_real(ctxcanvas,140, 0.0);
  write_code_real(ctxcanvas,141, 0.0);
  write_code_real(ctxcanvas,142, 1.0);
  write_code_real(ctxcanvas,143, 1.0);
  write_code_int(ctxcanvas, 70, 688);
  write_code_int(ctxcanvas, 72, 0);
  write_code_int(ctxcanvas, 73, 0);
  write_code_int(ctxcanvas, 74, 5);
  write_code(ctxcanvas, 7, "");
  write_code_int(ctxcanvas, 75, 16);
  write_code_real(ctxcanvas,147, 1.0);
  write_code_real(ctxcanvas,148, 0.0);
  write_code_real(ctxcanvas,149, 0.0);
  write_code(ctxcanvas,100, "AcDbLayout");
  write_code(ctxcanvas, 1, "Layout2");
  write_code_int(ctxcanvas, 70, 1);
  write_code_int(ctxcanvas, 71, 2);
  write_code_real(ctxcanvas, 10, 0.0);
  write_code_real(ctxcanvas, 20, 0.0);
  write_code_real(ctxcanvas, 11, 12.0);
  write_code_real(ctxcanvas, 21, 9.0);
  write_code_real(ctxcanvas, 12, 0.0);
  write_code_real(ctxcanvas, 22, 0.0);
  write_code_real(ctxcanvas, 32, 0.0);
  write_code_real(ctxcanvas, 14, 0.0);
  write_code_real(ctxcanvas, 24, 0.0);
  write_code_real(ctxcanvas, 34, 0.0);
  write_code_real(ctxcanvas, 15, 0.0);
  write_code_real(ctxcanvas, 25, 0.0);
  write_code_real(ctxcanvas, 35, 0.0);
  write_code_real(ctxcanvas,146, 0.0);
  write_code_real(ctxcanvas, 13, 0.0);
  write_code_real(ctxcanvas, 23, 0.0);
  write_code_real(ctxcanvas, 33, 0.0);
  write_code_real(ctxcanvas, 16, 1.0);
  write_code_real(ctxcanvas, 26, 0.0);
  write_code_real(ctxcanvas, 36, 0.0);
  write_code_real(ctxcanvas, 17, 0.0);
  write_code_real(ctxcanvas, 27, 1.0);
  write_code_real(ctxcanvas, 37, 0.0);
  write_code_int(ctxcanvas, 76, 0);
  write_code_hex(ctxcanvas,330, 0x23);

  write_code(ctxcanvas, 0, "DICTIONARY");
  write_unique_handle(ctxcanvas); 
  write_code(ctxcanvas,100, "AcDbDictionary");
  write_code_int(ctxcanvas,281, 1);
  write_code(ctxcanvas, 3, "DIMASSOC");
  write_code_hex(ctxcanvas,350, ctxcanvas->handle+1); 
  write_code(ctxcanvas, 3, "HIDETEXT");
  write_code_hex(ctxcanvas,350, ctxcanvas->handle); 

  write_code(ctxcanvas, 0, "DICTIONARYVAR");
  write_unique_handle(ctxcanvas); 
  write_code(ctxcanvas,100, "DictionaryVariables");
  write_code_int(ctxcanvas,280, 0);
  write_code_int(ctxcanvas, 1, 2);

  write_code(ctxcanvas, 0, "DICTIONARYVAR");
  write_unique_handle(ctxcanvas); 
  write_code(ctxcanvas,100, "DictionaryVariables");
  write_code_int(ctxcanvas,280, 0);
  write_code_int(ctxcanvas, 1, 1);
}

static void begin_entity(cdCtxCanvas *ctxcanvas, const char* name, const char* db_name)
{
  write_code(ctxcanvas, 0, name);

  if (ctxcanvas->acad2000)
  {
    write_unique_handle(ctxcanvas);
    write_code(ctxcanvas, 100, "AcDbEntity");
    write_code(ctxcanvas, 100, db_name);
  }

  write_code_int(ctxcanvas, 8, ctxcanvas->layer);
}

static void begin_polyline (cdCtxCanvas *ctxcanvas, int nv)
{
  if (ctxcanvas->acad2000)
    begin_entity(ctxcanvas, "LWPOLYLINE", "AcDbPolyline");
  else
    begin_entity(ctxcanvas, "POLYLINE", NULL);
                               
  write_code_int(ctxcanvas, 62, ctxcanvas->fgcolor_index);

  write_line_type_name(ctxcanvas, 6, ctxcanvas->line_type_index);

  write_code(ctxcanvas, 70, "0" );  /* both (flag=0) but different meaning for ac2000 */

  if (ctxcanvas->acad2000)
  {
    write_code(ctxcanvas, 370, "0");  /* line weight */
    write_code_real(ctxcanvas, 43, ctxcanvas->line_width);  /* constant width */
    write_code_int(ctxcanvas, 90, nv);  /* number of vertices */
  }
  else
  {
    write_code(ctxcanvas, 66, "1");

    write_code_real(ctxcanvas, 40, ctxcanvas->line_width);   /* start width */
    write_code_real(ctxcanvas, 41, ctxcanvas->line_width);   /* end width */

    /* DXF: always 0 */
    write_code(ctxcanvas, 10, "0");
    write_code(ctxcanvas, 20, "0");
    write_code(ctxcanvas, 30, "0");
  }
}

static void end_polyline (cdCtxCanvas *ctxcanvas)
{
  if (!ctxcanvas->acad2000)
    write_code(ctxcanvas, 0, "SEQEND");
}

static void write_vertex(cdCtxCanvas *ctxcanvas, double x, double y)
{
  if (!ctxcanvas->acad2000)
    begin_entity(ctxcanvas, "VERTEX", NULL);   /* there is no specific end */

  write_code_real(ctxcanvas, 10, x);
  write_code_real(ctxcanvas, 20, y);
}

static void write_polyline_real(cdCtxCanvas *ctxcanvas, cdfPoint *poly, int nv)
{
  int i;

  begin_polyline(ctxcanvas, nv);

  for (i=0; i<nv; i++)
    write_vertex(ctxcanvas, poly[i].x, poly[i].y);

  end_polyline(ctxcanvas);
}

static void write_polyline_int(cdCtxCanvas *ctxcanvas, cdPoint *poly, int nv)
{
  int i;

  begin_polyline(ctxcanvas, nv);

  for (i=0; i<nv; i++)
    write_vertex(ctxcanvas, (double)poly[i].x, (double)poly[i].y);

  end_polyline(ctxcanvas);
}

static void begin_hatch (cdCtxCanvas *ctxcanvas, int nv, int has_bulge)
{
  begin_entity(ctxcanvas, "HATCH", "AcDbHatch");

  write_code_int(ctxcanvas, 62, ctxcanvas->fgcolor_index);

  /* Elevation point (in OCS) */
  write_code(ctxcanvas, 10, "0");
  write_code(ctxcanvas, 20, "0");
  write_code(ctxcanvas, 30, "0");

  /* Extrusion direction */
  write_code(ctxcanvas, 210, "0");
  write_code(ctxcanvas, 220, "0");
  write_code(ctxcanvas, 230, "1");

  if (ctxcanvas->canvas->interior_style == CD_HATCH)
  {
    write_code(ctxcanvas, 2, "HATCHED");
    write_code(ctxcanvas, 70, "0");       /* pattern fill */
  }
  else
  {
    write_code(ctxcanvas, 2, "SOLID");
    write_code(ctxcanvas, 70, "1");       /* solid fill */
  }

  write_code(ctxcanvas, 71, "0");   /* non-associative */
  write_code(ctxcanvas, 91, "1");   /* number of boundary paths */

  /* Begin Boundary Path Data */ 
  write_code(ctxcanvas, 92, "2");   /* Boundary path type flag (Polyline) */

  /* Begin Polyline boundary data */
  write_code_int(ctxcanvas, 72, has_bulge);   /* Has bulge flag */
  write_code(ctxcanvas, 73, "1");   /* Is closed flag */
  write_code_int(ctxcanvas, 93, nv);  /* Number of polyline vertices */
}

static void write_hatch_line(cdCtxCanvas *ctxcanvas, double angle, double x, double y)
{
  write_code_real(ctxcanvas, 53, angle); /* line angle */
  write_code_real(ctxcanvas, 43, 0.0);   /* line base point */
  write_code_real(ctxcanvas, 44, 0.0);
  write_code_real(ctxcanvas, 45, x);    /* line offset */
  write_code_real(ctxcanvas, 46, y);
  write_code_int(ctxcanvas, 79, 0);      /* Number of dash length items */
}

static void end_hatch (cdCtxCanvas *ctxcanvas)
{
  /* End Polyline boundary data */

  write_code(ctxcanvas, 97, "0");  /* Number of source boundary objects */
  /* End Boundary Path Data */ 

  write_code(ctxcanvas, 75, "0");   /* Hatch style (Normal) */
  write_code(ctxcanvas, 76, "1");   /* Hatch pattern type (Predefined) */

  if (ctxcanvas->canvas->interior_style == CD_HATCH)
  {
    write_code_real(ctxcanvas, 52, 0);   /* pattern angle */
    write_code_real(ctxcanvas, 41, 0);   /* pattern scale or spacing */
    write_code_int(ctxcanvas, 77, 0);    /* pattern double flag */

    /* Pattern Data */
    switch(ctxcanvas->canvas->hatch_style)
    {
    default: /* CD_HORIZONTAL */
      write_code_int(ctxcanvas, 78, 1);      /* Number of pattern definition lines */
      write_hatch_line(ctxcanvas, 0, 0, 10);
      break;
    case CD_VERTICAL:
      write_code_int(ctxcanvas, 78, 1);
      write_hatch_line(ctxcanvas, 90, 10, 0);
      break;
    case CD_FDIAGONAL:
      write_code_int(ctxcanvas, 78, 1);
      write_hatch_line(ctxcanvas, -45, 10, 0);
      break;
    case CD_BDIAGONAL:
      write_code_int(ctxcanvas, 78, 1);
      write_hatch_line(ctxcanvas, 45, 10, 0);  
      break;
    case CD_CROSS:
      write_code_int(ctxcanvas, 78, 2);
      write_hatch_line(ctxcanvas, 0, 0, 10);
      write_hatch_line(ctxcanvas, 90, 10, 0);
      break;
    case CD_DIAGCROSS:
      write_code_int(ctxcanvas, 78, 2);
      write_hatch_line(ctxcanvas, 45, 10, 0);
      write_hatch_line(ctxcanvas, -45, 10, 0);
      break;
    }
  }

  /* Number of seed points */
  write_code(ctxcanvas, 98, "0");
}

static void write_hatch_real(cdCtxCanvas *ctxcanvas, cdfPoint *poly, int nv)
{
  /* Used here, only for AutoCAD 2000 */
  int i;

  begin_hatch(ctxcanvas, nv, 0);

  for ( i=0; i<nv; i++ )
    write_vertex(ctxcanvas, poly[i].x, poly[i].y);

  end_hatch(ctxcanvas);
}

static void write_hatch_int(cdCtxCanvas *ctxcanvas, cdPoint *poly, int nv)
{
  /* Used here, only for AutoCAD 2000 */
  int i;

  begin_hatch(ctxcanvas, nv, 0);

  for ( i=0; i<nv; i++ )
    write_vertex(ctxcanvas, (double)poly[i].x, (double)poly[i].y);

  end_hatch(ctxcanvas);
}

/*********************************************************************************************/


static void cddeactivate (cdCtxCanvas *ctxcanvas)
{
  fflush (ctxcanvas->file);
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  end_section(ctxcanvas);  /* End ENTITIES section */

  if (ctxcanvas->acad2000)
  {
    begin_section(ctxcanvas, "OBJECTS");
      write_objects(ctxcanvas);
    end_section(ctxcanvas);
  }

  write_code(ctxcanvas, 0, "EOF");  /* End the ASCII format */

  fflush (ctxcanvas->file);
  fclose (ctxcanvas->file);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free (ctxcanvas);
}

static void cdflush (cdCtxCanvas *ctxcanvas)
{
  fflush (ctxcanvas->file);               /* flush file */
  ctxcanvas->layer++;
}

/*==========================================================================*/
/* Primitives                                                               */
/*==========================================================================*/

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  if (mode == CD_BEZIER)
  {
    cdfSimPolyBezier(ctxcanvas->canvas, poly, n);
    return;
  }
  if (mode == CD_PATH)
  {
    cdfSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  if (mode == CD_FILL && ctxcanvas->acad2000 && ctxcanvas->canvas->interior_style != CD_HOLLOW)
    write_hatch_real(ctxcanvas, poly, n);
  else
    write_polyline_real(ctxcanvas, poly, n);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  if (mode == CD_BEZIER)
  {
    cdSimPolyBezier(ctxcanvas->canvas, poly, n);
    return;
  }
  if (mode == CD_PATH)
  {
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  if (mode == CD_FILL && ctxcanvas->acad2000 && ctxcanvas->canvas->interior_style != CD_HOLLOW)
    write_hatch_int (ctxcanvas, poly, n);
  else
    write_polyline_int(ctxcanvas, poly, n);
}

static void cdftext (cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  begin_entity(ctxcanvas, "TEXT", "AcDbText");

  /* attributes */
  write_code_int(ctxcanvas, 62, ctxcanvas->fgcolor_index);

  /* coordinates */
  write_code_real(ctxcanvas, 10, x);
  write_code_real(ctxcanvas, 20, y);
  write_code(ctxcanvas, 30, "0");  /* z */

  write_code_real(ctxcanvas, 40, ctxcanvas->text_height );    /* text height */

  s = cdStrDupN(s, len);
  write_code(ctxcanvas, 1, s);          /* text */
  free((char*)s);

  write_code_real(ctxcanvas, 50, ctxcanvas->canvas->text_orientation );    /* text orientation angle    */
  write_code(ctxcanvas, 41, "1");                   /* Relative X scale factor */
  write_code_int(ctxcanvas, 51, ctxcanvas->text_oblique_angle );   /* text oblique angle        */
  write_font_type( ctxcanvas, 7, ctxcanvas->font_index );                      /* current font  */
  write_code(ctxcanvas, 71, "0");                   /* Text generation flags */
  write_code_int(ctxcanvas, 72, ctxcanvas->text_horiz_align );   /* horizontal alignment */

  /* alignment point  */
  write_code_real(ctxcanvas, 11, x);
  write_code_real(ctxcanvas, 21, y);
  write_code(ctxcanvas, 31, "0");  /* z */
  
  if (ctxcanvas->acad2000)
    write_code(ctxcanvas, 100, "AcDbText");

  write_code_int(ctxcanvas, 73, ctxcanvas->text_vert_align );   /* vertical alignment   */
}

static void cdtext (cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdftext (ctxcanvas, (double)x, (double)y, s, len);
}


/*--------------------------------------------------------------------------*/
/* gives radius of the circle most resembling elliptic arc at angle t       */
/*--------------------------------------------------------------------------*/
static double calc_radius (double a, double b, double dt)
{
  double sin_dt = sin(dt);
  double cos_dt = cos(dt);
  return (pow ((a*a*sin_dt*sin_dt + b*b*cos_dt*cos_dt), 1.5))/(a*b);
}

/*--------------------------------------------------------------------------*/
/* calculates bulge for a given circular arc segment (between points p1 and */
/* p2, with radius r). Bulge is the tangent of 1/4 the angle theta of the   */
/* arc segment(a bulge of 1 is a semicircle, which has an angle of 180 deg) */
/*--------------------------------------------------------------------------*/
static double calc_bulge (double a, double b, double t1, double sin_t1, double cos_t1, double t2, double sin_t2, double cos_t2)
{
  cdfPoint p1, p2;        /* initial and ending arc points                 */
  double r;               /* radius most resembling arc at angle (t1+t2)/2 */
  double theta;           /* angle of circular arc segment                 */
  double sin_theta;       /* sine of theta                                 */
  double dist_x;          /* distance between two points along the x axis  */
  double dist_y;          /* distance between two points along the y axis  */
  double halfdist;        /* half distance between two points              */

  p1.x = a*cos_t1;
  p1.y = b*sin_t1;
  p2.x = a*cos_t2;
  p2.y = b*sin_t2;
  r    = calc_radius (a, b, (t1+t2)/2);

  dist_x      = p2.x - p1.x;
  dist_y      = p2.y - p1.y;
  halfdist    = (sqrt (dist_x*dist_x + dist_y*dist_y))/2;
  sin_theta   = halfdist/r;
  if (sin_theta > 1)  sin_theta = 1;
  theta       = 2*asin(sin_theta);

  return tan(theta/4);
}

static void write_bulge(cdCtxCanvas *ctxcanvas, double bulge)
{
  /* The bulge is the tangent of one fourth the
    included angle for an arc segment, made negative if the arc goes
    clockwise from the start point to the endpoint. A bulge of 0 indicates a
    straight segment, and a bulge of 1 is a semicircle. */
  write_code_real(ctxcanvas, 42, bulge);
}

static void write_poly_arc (cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2, int nseg)
{
  double bulge;        /* bulge is the tangent of 1/4 the angle for a given */
                       /* circle arc segment (a bulge of 1 is a semicircle) */
  double t;            /* current arc angle being calculated    */
  double t1;           /* a1 in radians                         */
  double t2;           /* a2 in radians                         */
  double a;            /* half horizontal axis                  */
  double b;            /* half vertical axis                    */
  double seg_angle;    /* angle of every arc segment            */
  double x, y, sin_t, cos_t, t_seg, sin_tseg, cos_tseg;
  int i;

  a         = w/2.0;
  b         = h/2.0;
  t1        = a1*CD_DEG2RAD;                /* a1 in radians */
  t2        = a2*CD_DEG2RAD;                /* a2 in radians */
  seg_angle = (t2-t1)/nseg;

  t = t1;
  sin_t = sin(t);
  cos_t = cos(t);

  for (i=0; i<nseg; i++)
  {                 
    x = xc + a*cos_t;
    y = yc + b*sin_t;
    write_vertex (ctxcanvas, x, y);

    t_seg = t+seg_angle;
    sin_tseg = sin(t_seg);
    cos_tseg = cos(t_seg);
    bulge = calc_bulge (a, b, t, sin_t, cos_t, t+seg_angle, sin_tseg, cos_tseg);
    write_bulge(ctxcanvas, bulge);

    t = t_seg;
    sin_t = sin_tseg;
    cos_t = cos_tseg;
  }

  x = xc + a*cos(t2);
  y = yc + b*sin(t2);
  write_vertex (ctxcanvas, x, y);
  /* bulge of last vertex is useless */
  write_bulge(ctxcanvas, 0);
}

static void cdfarc (cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double diff  = fabs(a2 - a1);   /* angle between a1 and a2 */
  int nseg = cdRound(diff)/(360/32); /* number of arc segments = diff/angle of 1 segment  (32 segments in closed ellipse) */
  nseg = max(nseg, 1);

  begin_polyline(ctxcanvas, nseg+1);  /* add room for last vertex */

  write_poly_arc(ctxcanvas, xc, yc, w, h, a1, a2, nseg);

  end_polyline(ctxcanvas);
}

static void cdarc (cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfsector (cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double diff  = fabs(a2 - a1);   /* angle between a1 and a2 */
  int nseg = cdRound(diff)/(360/32); /* number of arc segments = diff/angle of 1 segment  (32 segments in closed ellipse) */
  nseg = max(nseg, 1);

  if (ctxcanvas->canvas->interior_style != CD_HOLLOW && ctxcanvas->acad2000)
  {
    if ((a2-a1) != 360)
      begin_hatch(ctxcanvas, nseg+1+2, 1);   /* add room for last vertex and center, has bulge */
    else
      begin_hatch(ctxcanvas, nseg+1, 1);
  }
  else
  {
    if ((a2-a1) != 360)
      begin_polyline(ctxcanvas, nseg+1+2);
    else
      begin_polyline(ctxcanvas, nseg+1);
  }

  if ((a2-a1) != 360)
  {
    write_vertex (ctxcanvas, xc, yc);    /* center */
    write_bulge(ctxcanvas, 0);
  }

  write_poly_arc(ctxcanvas, xc, yc, w, h, a1, a2, nseg);

  if ((a2-a1) != 360)
  {
    write_vertex (ctxcanvas, xc, yc);    /* center */
    write_bulge(ctxcanvas, 0);
  }

  if (ctxcanvas->canvas->interior_style != CD_HOLLOW && ctxcanvas->acad2000)
    end_hatch(ctxcanvas);
  else
    end_polyline(ctxcanvas);
}

static void cdsector (cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

/*==========================================================================*/
/* Attributes                                                               */
/*==========================================================================*/

static int cdlinestyle (cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    ctxcanvas->line_type_index = 0;
    break;
  case CD_DASHED:
    ctxcanvas->line_type_index = 1;
    break;
  case CD_DOTTED:
    ctxcanvas->line_type_index = 5;
    break;
  case CD_DASH_DOT:
    ctxcanvas->line_type_index = 6;
    break;
  case CD_DASH_DOT_DOT:
    ctxcanvas->line_type_index = 8;
    break;
  }

  return style;
}

static int cdlinewidth (cdCtxCanvas *ctxcanvas, int width)
{
  ctxcanvas->line_width = ( width <= 1 ? 0.0 : width );
  return width;
}

static int cdfont (cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  if (cdStrEqualNoCase(type_face, "System"))
  {
    ctxcanvas->font_index = 0;

    ctxcanvas->text_height = 0.75;
  }
  else if (cdStrEqualNoCase(type_face, "Courier"))
  {
    switch (style&3)
    {
      case CD_PLAIN: ctxcanvas->font_index = 1; break;
      case CD_BOLD: ctxcanvas->font_index = 2; break;
      case CD_ITALIC: ctxcanvas->font_index = 1; break;
      case CD_BOLD_ITALIC: ctxcanvas->font_index = 2; break;
    }

    ctxcanvas->text_height = 0.75;
  }
  else if (cdStrEqualNoCase(type_face, "Times"))
  {
    switch (style&3)
    {
      case CD_PLAIN: ctxcanvas->font_index = 3; break;
      case CD_BOLD: ctxcanvas->font_index = 4; break;
      case CD_ITALIC: ctxcanvas->font_index = 3; break;
      case CD_BOLD_ITALIC: ctxcanvas->font_index = 4; break;
    }

    ctxcanvas->text_height = 1.125;
  }
  else if (cdStrEqualNoCase(type_face, "Helvetica"))
  {
    switch (style&3)
    {
      case CD_PLAIN: ctxcanvas->font_index = 5; break;
      case CD_BOLD: ctxcanvas->font_index = 6; break;
      case CD_ITALIC: ctxcanvas->font_index = 5; break;
      case CD_BOLD_ITALIC: ctxcanvas->font_index = 6; break;
    }

    ctxcanvas->text_height = 1.;
  }
  else
    return 0;

  if (style & CD_ITALIC)
    ctxcanvas->text_oblique_angle = 15;
  else
    ctxcanvas->text_oblique_angle = 0;

  /* DXF's text height corresponds to CD ascent */
  ctxcanvas->text_height = ctxcanvas->text_height * cdGetFontSizePoints(ctxcanvas->canvas, size);

  return 1;
}

static void cdgetfontdim (cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  double tangent_ta;
  double pixel_th;

  tangent_ta = tan(ctxcanvas->text_oblique_angle*CD_DEG2RAD);
  pixel_th = (ctxcanvas->text_height*ctxcanvas->canvas->xres)/CD_MM2PT;  /* points to pixels */
  switch (ctxcanvas->font_index)
  {
    case 0:                                  /* STANDARD font (CD_SYSTEM) */
      if (height)    *height    =  cdRound(pixel_th*4/3);
      if (ascent)    *ascent    = _cdRound(pixel_th);
      if (descent)   *descent   =  cdRound(pixel_th/3);
      if (max_width) *max_width = _cdRound(pixel_th);
      break;

    case 1:                                  /* ROMAN fonts (CD_COURIER)  */
    case 2:
      if (height)    *height    =  cdRound(pixel_th*4/3);
      if (ascent)    *ascent    = _cdRound(pixel_th);
      if (descent)   *descent   =  cdRound(pixel_th/3);
      if (max_width) *max_width =  cdRound((pixel_th*21/20) + tangent_ta*(*ascent));
      break;

    case 3:                            /* ROMANTIC fonts (CD_TIMES_ROMAN) */
      if (height)    *height    = cdRound(pixel_th*8/9);
      if (ascent)    *ascent    = cdRound(pixel_th*2/3);
      if (descent)   *descent   = cdRound(pixel_th*2/9);
      if (max_width) *max_width = cdRound((pixel_th*14/15) + tangent_ta*(*ascent));
      break;

    case 4:
      if (height)    *height    = cdRound(pixel_th*8/9);
      if (ascent)    *ascent    = cdRound(pixel_th*2/3);
      if (descent)   *descent   = cdRound(pixel_th*2/9);
      if (max_width) *max_width = cdRound((pixel_th*29/30) + tangent_ta*(*ascent));
      break;

    case 5:                            /* SANSSERIF fonts (CD_HELVETICA)  */
    case 6:
      if (height)    *height    = _cdRound(pixel_th);
      if (ascent)    *ascent    =  cdRound(pixel_th*3/4);
      if (descent)   *descent   =  cdRound(pixel_th/4);
      if (max_width) *max_width =  cdRound((pixel_th*15/16) + tangent_ta*(*ascent));
      break;
  }
}

static void cdgettextsize (cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  int i;
  double tangent_ta;
  double pixel_th;
  (void)s;

  i = len;
  tangent_ta = tan(ctxcanvas->text_oblique_angle*CD_DEG2RAD);
  pixel_th = (ctxcanvas->text_height*ctxcanvas->canvas->xres)/CD_MM2PT;  /* points to pixels */

  switch (ctxcanvas->font_index)  /* width return value based on maximum character width */
  {
    case 0:                                  /* STANDARD font (CD_SYSTEM) */
      if (height) *height = cdRound(pixel_th*4/3);
      if (width)  *width  = cdRound(pixel_th*i + (pixel_th/3)*(i-1));
      break;

    case 1:                                  /* ROMAN fonts (CD_COURIER)  */
    case 2:
      if (height) *height = cdRound(pixel_th*4/3);
      if (width)  *width  = cdRound((pixel_th*21/20)*i + (pixel_th/10)*(i-1) + tangent_ta*pixel_th);
      break;

    case 3:                            /* ROMANTIC fonts (CD_TIMES_ROMAN) */
      if (height) *height = cdRound(pixel_th*2/3 + pixel_th*2/9);
      if (width)  *width  = cdRound((pixel_th*14/15)*i + (pixel_th/45)*(i-1) + tangent_ta*pixel_th*2/3);
      break;

    case 4:
      if (height) *height = cdRound(pixel_th*2/3 + pixel_th*2/9);
      if (width)  *width  = cdRound((pixel_th*29/30)*i + (pixel_th*2/45)*(i-1) + tangent_ta*pixel_th*2/3);
      break;

    case 5:                            /* SANSSERIF fonts (CD_HELVETICA)  */
    case 6:
      if (height) *height = _cdRound(pixel_th);
      if (width)  *width  =  cdRound((pixel_th*15/16)*i + (pixel_th/45)*(i-1) + tangent_ta*pixel_th*3/4);
      break;
  }
}

static int cdtextalignment (cdCtxCanvas *ctxcanvas, int alignment)
{
  switch (alignment)          /* convert alignment to DXF format */
  {
    case CD_BASE_LEFT:
      ctxcanvas->text_vert_align = 0;
      ctxcanvas->text_horiz_align = 0;
      break;

    case CD_BASE_CENTER:
      ctxcanvas->text_vert_align = 0;
      ctxcanvas->text_horiz_align = 1;
      break;

    case CD_BASE_RIGHT:
      ctxcanvas->text_vert_align = 0;
      ctxcanvas->text_horiz_align = 2;
      break;

    case CD_SOUTH_WEST:
      ctxcanvas->text_vert_align = 1;
      ctxcanvas->text_horiz_align = 0;
      break;

    case CD_SOUTH:
      ctxcanvas->text_vert_align = 1;
      ctxcanvas->text_horiz_align = 1;
      break;

    case CD_SOUTH_EAST:
      ctxcanvas->text_vert_align = 1;
      ctxcanvas->text_horiz_align = 2;
      break;

    case CD_WEST:
      ctxcanvas->text_vert_align = 2;
      ctxcanvas->text_horiz_align = 0;
      break;

    case CD_CENTER:
      ctxcanvas->text_vert_align = 2;
      ctxcanvas->text_horiz_align = 1;
      break;

    case CD_EAST:
      ctxcanvas->text_vert_align = 2;
      ctxcanvas->text_horiz_align = 2;
      break;

    case CD_NORTH_WEST:
      ctxcanvas->text_vert_align = 3;
      ctxcanvas->text_horiz_align = 0;
      break;

    case CD_NORTH:
      ctxcanvas->text_vert_align = 3;
      ctxcanvas->text_horiz_align = 1;
      break;

    case CD_NORTH_EAST:
      ctxcanvas->text_vert_align = 3;
      ctxcanvas->text_horiz_align = 2;
      break;
  }

  return alignment;
}

/*==========================================================================*/
/* Color                                                                    */
/*==========================================================================*/

static void RGB_to_HSB (unsigned char r, unsigned char g, unsigned char b,
                        double *hue, double *sat, double *bright)
{
  double maximum;
  double minimum;
  double delta;
  double red   = r/255.;         /* red, green and blue range from 0 to 1 */
  double green = g/255.;
  double blue  = b/255.;

  maximum = max(max(red, green), blue);   /* stores higher index */
  minimum = min(min(red, green), blue);   /* stores lower index  */
  delta   = maximum - minimum;

  *bright = maximum*100;
  *sat  = 0;
  *hue = 0;      /* if color is greyscale (hue is meaningless) */

  if (maximum != 0)     /* sat from 0 to 100 */
    *sat = (delta*100)/maximum;
  
  if (*sat != 0)        /* hue from 0 to 359 */
  {
    if (red   == maximum) *hue = (green - blue)/delta;
    if (green == maximum) *hue = 2 + (blue - red)/delta;
    if (blue  == maximum) *hue = 4 + (red - green)/delta;
    *hue *= 60;
    if (*hue < 0) *hue += 360;
  }
}

static int HSB_to_AutoCAD_Palette (double hue, double sat, double bright)
{
  int index;
  int h, s, b;

  if (bright < 17)     /* 5 levels of brightness in AutoCAD palette, 6 with */
  {                    /* black. If bright < 17, index is black (7).        */
    index = 7;         /* 17 is 100/6 (rounded up)                          */
  }
  else if (sat < 10)              /* low saturation makes color tend to     */
  {                               /* grey/white. 6 levels of grey/white in  */
    b = (int)floor(bright/14.3)-1;/* palette WITHOUT black. 14.3 is 100/7   */
    index = 250 + b;              /* index is grey to white(255 in palette) */
  }
  else
  {
    h = cdRound(hue/15.) + 1;
    if (h > 24) h -= 24;          /* 15 is 360/24                           */
    h *= 10;                      /* h ranges from 10 to 240 in palette     */
    s = (sat < 55) ? 1 : 0;       /* s is 'high'(0) or 'low'(1) in palette  */
    b = (int)floor(bright/16.7)-1;/* b is 0, 2, 4, 6 or 8 in palette        */
    b = 2*(4 - b);                /* (from brightest to dimmest)            */
    index = h + s + b;            /* index is simple sum of h, s and b      */
  }
  return index;
}

static int get_palette_index (long int color)      /* gives closest palette */
{                                                  /* index to RGB color    */
  unsigned char red, green, blue;
  double hue, sat, bright;

  cdDecodeColor (color, &red, &green, &blue);         /* AutoCAD palette is */
  RGB_to_HSB (red, green, blue, &hue, &sat, &bright); /* based on HSB model */

  return HSB_to_AutoCAD_Palette (hue, sat, bright);
}

static long int cdforeground (cdCtxCanvas *ctxcanvas, long int color)
{
  ctxcanvas->fgcolor_index = get_palette_index (color);
  return color;
}


/*==========================================================================*/
/* Server Images                                                            */
/*==========================================================================*/

static void cdpixel (cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  int color_index = get_palette_index(color);

  begin_entity(ctxcanvas, "POINT", "AcDbPoint");

  /* attributes */
  write_code_int(ctxcanvas, 62, color_index);

  /* coordinates */
  write_code_int(ctxcanvas, 10, x);
  write_code_int(ctxcanvas, 20, y);
  write_code(ctxcanvas, 30, "0");  /* z */
}

static void cdfpixel(cdCtxCanvas *ctxcanvas, double x, double y, long int color)
{
  int color_index = get_palette_index(color);

  begin_entity(ctxcanvas, "POINT", "AcDbPoint");

  /* attributes */
  write_code_int(ctxcanvas, 62, color_index);

  /* coordinates */
  write_code_real(ctxcanvas, 10, x);
  write_code_real(ctxcanvas, 20, y);
  write_code(ctxcanvas, 30, "0");  /* z */
}

/******************************************************/

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  cdCtxCanvas *ctxcanvas;
  double res = 3.78;
  double w_mm = (INT_MAX-1)/res, 
         h_mm = (INT_MAX-1)/res;
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  ctxcanvas = (cdCtxCanvas *) malloc (sizeof (cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->file = fopen (filename, "w");
  if (ctxcanvas->file == NULL)
  {
    free(ctxcanvas);
    return;
  }

  /* store the base canvas */
  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  /* get size */
  sscanf(strdata, "%lgx%lg %lg", &w_mm, &h_mm, &res);
  canvas->w = (int)(w_mm * res);
  canvas->h = (int)(h_mm * res);
  canvas->w_mm = w_mm;
  canvas->h_mm = h_mm;
  canvas->xres = res;
  canvas->yres = res;

  /* Get Additional parameters */
  if(strstr(strdata, "-ac2000") != NULL)
    ctxcanvas->acad2000 = 1;

  strdata = strstr(strdata, "-limits");
  if (strdata)
  {
    strdata += strlen("-limits ");
    sscanf(strdata, "%lg %lg %lg %lg", &xmin, &ymin, &xmax, &ymax);
  }

  if (xmin==xmax) xmax = canvas->w;
  if (ymin==ymax) ymax = canvas->h;

  canvas->bpp = 8;

  /* internal defaults */
  ctxcanvas->text_height  = 9;
  ctxcanvas->fgcolor_index = 7;    /* default AutoCAD index        */
  ctxcanvas->handle = 0x30;        /* unique handle start value */

  /* comment */
  write_code(ctxcanvas, 999, CD_NAME", version "CD_VERSION);

  begin_section(ctxcanvas, "HEADER");
    write_header(ctxcanvas, xmin, xmax, ymin, ymax);
  end_section(ctxcanvas);  /* End HEADER section */

  begin_section(ctxcanvas, "TABLES");
    write_line_types (ctxcanvas);   /* must be before layers */
    write_fonts (ctxcanvas);
    if (ctxcanvas->acad2000)
    {
      write_vport(ctxcanvas);
      write_layers(ctxcanvas);
      write_view(ctxcanvas);
      write_ucs(ctxcanvas);
      write_appid(ctxcanvas);
      write_dimstyle(ctxcanvas);
      write_blockrecord(ctxcanvas);
    }
  end_section(ctxcanvas);  /* End TABLES section */

  begin_section(ctxcanvas, "BLOCKS");
    if (ctxcanvas->acad2000)
    {
      write_block(ctxcanvas, "*Model_Space",  "20", "21", 0);  /* handle=20  handle=21 */
      write_block(ctxcanvas, "*Paper_Space",  "1C", "1D", 1);  /* handle=1C  handle=1D */
      write_block(ctxcanvas, "*Paper_Space0", "24", "25", 0);  /* handle=24  handle=25 */
    }
  end_section(ctxcanvas);  /* End BLOCKS section */

  begin_section(ctxcanvas, "ENTITIES");
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxPixel = cdpixel;
  canvas->cxPoly = cdpoly;
  canvas->cxFPoly = cdfpoly;

  canvas->cxLine = cdSimLine;
  canvas->cxRect = cdSimRect;
  canvas->cxBox = cdSimBox;
  canvas->cxChord = cdSimChord;
  canvas->cxFLine = cdfSimLine;
  canvas->cxFRect = cdfSimRect;
  canvas->cxFBox = cdfSimBox;
  canvas->cxFChord = cdfSimChord;
  canvas->cxFPixel = cdfpixel;

  canvas->cxArc = cdarc;
  canvas->cxFArc = cdfarc;
  canvas->cxSector = cdsector;
  canvas->cxFSector = cdfsector;
  /* Results where very similar in the AutoDesk TrueView application.
     But since the bulge parameter maybe useful, we leave the local implementation.
  canvas->cxArc = cdSimArc;
  canvas->cxFArc = cdfSimArc;
  canvas->cxSector = cdSimSector;
  canvas->cxFSector = cdfSimSsector;
  */
  
  canvas->cxText = cdtext;
  canvas->cxFText = cdftext;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;

  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxFont = cdfont;
  canvas->cxTextAlignment = cdtextalignment;
  canvas->cxForeground = cdforeground;

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxDeactivate = cddeactivate;
}

/******************************************************/

static cdContext cdDXFContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE |
                 CD_CAP_CLIPAREA | CD_CAP_CLIPPOLY | CD_CAP_PATH | CD_CAP_BEZIER |
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_IMAGERGB | CD_CAP_IMAGEMAP | CD_CAP_IMAGESRV |
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE |
                 CD_CAP_HATCH | CD_CAP_STIPPLE | CD_CAP_PATTERN |
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB),
  CD_CTX_FILE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextDXF(void)
{
  return &cdDXFContext;
}
