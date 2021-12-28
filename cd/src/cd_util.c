/** \file
 * \brief Utilities
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "cd.h"

#include "cd_private.h"


int cdRound(double x)
{
  return _cdRound(x);
}

/* Returns a table to speed up zoom in discrete zoom rotines.
   Adds the min parameter and allocates the table using malloc.
   The table should be used when mapping from destiny coordinates to
   source coordinates (src_pos = tab[dst_pos]).
   dst_len is the full destiny size range.
   src_len is only the zoomed region size, usually max-min+1.
*/
int* cdGetZoomTable(int dst_len, int src_len, int src_min)
{
  int dst_i, src_i;
  double factor;
	int* tab = (int*)malloc(dst_len*sizeof(int));

  factor = (double)(src_len) / (double)(dst_len);

	for(dst_i = 0; dst_i < dst_len; dst_i++)
  {
    src_i = cdRound((factor*(dst_i + 0.5)) - 0.5);
		tab[dst_i] = src_i + src_min;
  }

  return tab;
}

/* funcao usada para calcular os retangulos efetivos de zoom 
   de imagens clientes. Pode ser usada para os eixos X e Y.

   canvas_size - tamanho do canvas (canvas->w, canvas->h)
   cnv_rect_pos - posicao no canvas onde a regiao sera´ desenhada (x, y)
   cnv_rect_size - tamanho da regiao no canvas com zoom (w, h)
   img_rect_pos - posicao na imagem da regiao a ser desenhada (min)
   img_rect_size - tamanho da regiao na imagem (max-min+1)

   calcula o melhor tamanho a ser usado
   retorna 0 se o retangulo resultante e´ nulo
*/
int cdCalcZoom(int canvas_size,
               int cnv_rect_pos, int cnv_rect_size, 
               int *new_cnv_rect_pos, int *new_cnv_rect_size, 
               int img_rect_pos, int img_rect_size, 
               int *new_img_rect_pos, int *new_img_rect_size, 
               int is_horizontal)
{
  int offset;
  double zoom_factor = (double)img_rect_size / (double)cnv_rect_size;

  /* valores default sem otimizacao */
  *new_cnv_rect_size = cnv_rect_size, *new_cnv_rect_pos = cnv_rect_pos;    
  *new_img_rect_size = img_rect_size, *new_img_rect_pos = img_rect_pos;

  if (cnv_rect_size > 0)
  {
    /* se posicao no canvas > tamanho do canvas, fora da janela, nao desenha nada */
    if (cnv_rect_pos >= canvas_size) 
      return 0;

    /* se posicao no canvas + tamanho da regiao no canvas < 0, fora da janela, nao desenha nada */
    if (cnv_rect_pos+cnv_rect_size < 0) 
      return 0;

    /* se posicao no canvas < 0, entao comeca fora do canvas melhor posicao no canvas e' 0
                                 E o tamanho e' reduzido do valor negativo */
    if (cnv_rect_pos < 0) 
    {
      /* valores ajustados para cair numa vizinhanca de um pixel da imagem */
      offset = (int)ceil(cnv_rect_pos*zoom_factor);   /* offset is <0 */
      offset = (int)ceil(offset/zoom_factor);
      *new_cnv_rect_pos -= offset;  
      *new_cnv_rect_size += offset; 
    }

    /* se posicao no canvas + tamanho da regiao no canvas > tamanho do canvas, 
       termina fora do canvas entao 
       o tamanho da regiao no canvas e' o tamanho do canvas reduzido da posicao */
    if (*new_cnv_rect_pos+*new_cnv_rect_size > canvas_size) 
    {
      offset = (int)((*new_cnv_rect_pos+*new_cnv_rect_size - canvas_size)*zoom_factor);
      *new_cnv_rect_size -= (int)(offset/zoom_factor);  /* offset is >0 */
    }
  }
  else
  {
    /* cnv_rect_size tamanho negativo, significa imagem top down */
    /* calculos adicionados pela Paula */

    /* se posicao no canvas + tamanho no canvas (xmin+1) >= tamanho do canvas, fora da janela, nao desenha nada */
    if (cnv_rect_pos+cnv_rect_size >= canvas_size) 
      return 0;

    /* se posicao da imagem com zoom (xmax) < 0, fora da janela, nao desenha nada */
    if (cnv_rect_pos < 0) 
      return 0;

    /* se posicao com zoom (xmax) >= tamanho do canvas, posicao da imagem com zoom e' o tamanho do canvas menos um
       tambem o tamanho e' reduzido do valor negativo */
    if (cnv_rect_pos >= canvas_size) 
    {
      *new_cnv_rect_pos = canvas_size-1; 
      *new_cnv_rect_size = cnv_rect_size + (cnv_rect_pos - *new_cnv_rect_pos);
    }

    /* se posicao + tamanho com zoom (xmin+1) < 0, 
       entao o tamanho com zoom e' a posição + 1 */
    if (cnv_rect_pos+cnv_rect_size < 0) 
      *new_cnv_rect_size = -(*new_cnv_rect_pos + 1);
  }

  /* agora ja' tenho tamanho e posicao da regiao no canvas,
     tenho que obter tamanho e posicao dentro da imagem original,
     baseado nos novos valores */

  /* tamanho da regiao na imagem e' a conversao de zoom para real do tamanho no canvas */
  *new_img_rect_size = (int)(*new_cnv_rect_size * zoom_factor + 0.5);

  if (is_horizontal)
  {
    /* em X, o offset dentro da imagem so' existe se houver diferenca entre a posicao inicial da
       imagem e a posicao otimizada (ambas da imagem com zoom) */
    if (*new_cnv_rect_pos != cnv_rect_pos)
    {
      offset = *new_cnv_rect_pos - cnv_rect_pos; /* offset is >0 */
      *new_img_rect_pos += (int)(offset*zoom_factor);
    }
  }
  else
  {
    /* em Y, o offset dentro da imagem so' existe se houver diferenca entre a posicao 
       final (posição inicial + tamanho) da imagem e a posicao otimizada (ambas da 
       imagem com zoom) */
    if ((cnv_rect_pos + cnv_rect_size) != (*new_cnv_rect_pos + *new_cnv_rect_size))
    {
      /* offset is >0, because Y axis is from top to bottom */
      offset = (cnv_rect_pos + cnv_rect_size) - (*new_cnv_rect_pos + *new_cnv_rect_size);
      *new_img_rect_pos += (int)(offset*zoom_factor);
    }
  }

  return 1;
}

int cdGetFileName(const char* strdata, char* filename)
{
  int i = 0;
  const char* start = strdata;
  char* start_fn = filename;

  if (!strdata || strdata[0] == 0) 
    return 0;
  
  if (strdata[0] == '\"')
  {   
    strdata++; /* the first " */
    while (*strdata && *strdata != '\"' && i < 10240)
    {
      *filename++ = *strdata++;
      i++;
    }
    strdata++; /* the last " */
  }
  else
  {
    while (*strdata && *strdata != ' ' && i < 10240)
    {
      *filename++ = *strdata++;
      i++;
    }
  }

  if (i == 10240)
  {
    *start_fn = 0;
    return 0;
  }

  if (*strdata == ' ')
    strdata++;

  *filename = 0;
  return (int)(strdata - start);
}

void cdNormalizeLimits(int w, int h, int *xmin, int *xmax, int *ymin, int *ymax)
{
  *xmin = *xmin < 0? 0: *xmin < w? *xmin: (w - 1);
  *ymin = *ymin < 0? 0: *ymin < h? *ymin: (h - 1);
  *xmax = *xmax < 0? 0: *xmax < w? *xmax: (w - 1);
  *ymax = *ymax < 0? 0: *ymax < h? *ymax: (h - 1);
}

int cdCheckBoxSize(int *xmin, int *xmax, int *ymin, int *ymax)
{
  if (*xmin > *xmax) _cdSwapInt(*xmin, *xmax);
  if (*ymin > *ymax) _cdSwapInt(*ymin, *ymax);

  if ((*xmax-*xmin+1) <= 0)
    return 0;

  if ((*ymax-*ymin+1) <= 0)
    return 0;

  return 1;
}

int cdfCheckBoxSize(double *xmin, double *xmax, double *ymin, double *ymax)
{
  if (*xmin > *xmax) _cdSwapDouble(*xmin, *xmax);
  if (*ymin > *ymax) _cdSwapDouble(*ymin, *ymax);

  if ((*xmax-*xmin+1) <= 0)
    return 0;

  if ((*ymax-*ymin+1) <= 0)
    return 0;

  return 1;
}

void cdMovePoint(int *x, int *y, double dx, double dy, double sin_theta, double cos_theta)
{
  double t;
  t = cos_theta*dx - sin_theta*dy;
  *x += _cdRound(t);
  t = sin_theta*dx + cos_theta*dy;
  *y += _cdRound(t);
}

void cdfMovePoint(double *x, double *y, double dx, double dy, double sin_theta, double cos_theta)
{
  *x += cos_theta*dx - sin_theta*dy;
  *y += sin_theta*dx + cos_theta*dy;
}

void cdRotatePoint(cdCanvas* canvas, int x, int y, int cx, int cy, int *rx, int *ry, double sin_theta, double cos_theta)
{
  double t;

  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
  {
    t =  (x * cos_theta) + (y * sin_theta); *rx = _cdRound(t); 
    t = -(x * sin_theta) + (y * cos_theta); *ry = _cdRound(t);
  }
  else
  {
    t = (x * cos_theta) - (y * sin_theta); *rx = _cdRound(t); 
    t = (x * sin_theta) + (y * cos_theta); *ry = _cdRound(t); 
  }

  /* translate back */
  *rx = *rx + cx;
  *ry = *ry + cy;
}

void cdfRotatePoint(cdCanvas* canvas, double x, double y, double cx, double cy, double *rx, double *ry, double sin_theta, double cos_theta)
{
  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
  {
    *rx =  (x * cos_theta) + (y * sin_theta); 
    *ry = -(x * sin_theta) + (y * cos_theta); 
  }
  else
  {
    *rx = (x * cos_theta) - (y * sin_theta); 
    *ry = (x * sin_theta) + (y * cos_theta); 
  }

  /* translate back */
  *rx = *rx + cx;
  *ry = *ry + cy;
}

void cdRotatePointY(cdCanvas* canvas, int x, int y, int cx, int cy, int *ry, double sin_theta, double cos_theta)
{
  double t;

  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
  {
    t = -(x * sin_theta) + (y * cos_theta); *ry = _cdRound(t);
  }
  else
  {
    t = (x * sin_theta) + (y * cos_theta); *ry = _cdRound(t); 
  }

  /* translate back */
  *ry = *ry + cy;
}

void cdfRotatePointY(cdCanvas* canvas, double x, double y, double cx, double cy, double *ry, double sin_theta, double cos_theta)
{
  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
    *ry = -(x * sin_theta) + (y * cos_theta);
  else
    *ry = (x * sin_theta) + (y * cos_theta);

  /* translate back */
  *ry = *ry + cy;
}


/**************************************************************************************/


/* Copied from IUP3 */

int cdStrEqualNoCase(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2 || tolower(*str1) != tolower(*str2)) return 0;

  while (str1[i] && str2[i] && tolower(str1[i])==tolower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1; 

  return 0;
}

int cdStrEqualNoCasePartial(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2 || tolower(*str1) != tolower(*str2)) return 0;

  while (str1[i] && str2[i] && tolower(str1[i])==tolower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1; 
  if (str2[i] == 0) return 1;

  return 0;
}

/* Copied from IUP3, simply ignore line breaks other than '\n' for CD */

int cdStrLineCount(const char* str)
{
  int num_lin = 1;

  if (!str)
    return num_lin;

  while(*str != 0)
  {
    while(*str!=0 && *str!='\n')
      str++;

    if (*str=='\n')   /* UNIX line end */
    {
      num_lin++;
      str++;
    }
  }

  return num_lin;
}

char* cdStrDup(const char *str)
{
  if (str)
  {
    int size = (int)strlen(str)+1;
    char *newstr = malloc(size);
    if (newstr) memcpy(newstr, str, size);
    return newstr;
  }
  return NULL;
}

char* cdStrDupN(const char *str, int len)
{
  if (str)
  {
    int size = len+1;
    char *newstr = malloc(size);
    if (newstr) 
    {
      memcpy(newstr, str, len);
      newstr[len]=0;
    }
    return newstr;
  }
  return NULL;
}

int cdStrIsAscii(const char* str)
{
  while(*str)
  {
    int c = *str;
    if (c < 0)
      return 0;
    str++;
  }
  return 1;
}


/**************************************************************************************/


void cdSetPaperSize(int size, double *w_pt, double *h_pt)
{
  static struct
  {
    int w_pt;
    int h_pt;
  } paper[] =
    {
      { 2393, 3391 },   /*   A0   */
      { 1689, 2393 },   /*   A1   */
      { 1192, 1689 },   /*   A2   */
      {  842, 1192 },   /*   A3   */
      {  595,  842 },   /*   A4   */
      {  420,  595 },   /*   A5   */
      {  612,  792 },   /* LETTER */
      {  612, 1008 }    /*  LEGAL */
    };

  if (size<CD_A0 || size>CD_LEGAL) 
    return;

  *w_pt = (double)paper[size].w_pt;
  *h_pt = (double)paper[size].h_pt;
}


/**************************************************************************************/


#ifdef WIN32
static char* winRegReadStringKey(HKEY hBaseKey, const char* key_name, const char* value_name)
{
	HKEY hKey;
	DWORD size;
  char* str;

	if (RegOpenKeyExA(hBaseKey, key_name, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return NULL;

  if (RegQueryValueExA(hKey, value_name, NULL, NULL, NULL, &size) != ERROR_SUCCESS)
  {
    RegCloseKey(hKey);
		return NULL;
  }

  str = malloc(size);
  RegQueryValueExA(hKey, value_name, NULL, NULL, (LPBYTE)str, &size);

	RegCloseKey(hKey);
	return str;
}

static int winRegGetValueCount(HKEY hKey, int *count, int *max_name_size, int *max_value_size)
{
  DWORD cValues;
  DWORD cMaxValueNameLen;
  DWORD cMaxValueLen;

  if (RegQueryInfoKeyA(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
      &cValues,             // number of values for this hKey 
      &cMaxValueNameLen,    // longest value name 
      &cMaxValueLen,        // longest value data 
      NULL, NULL) == ERROR_SUCCESS)
  {
    if (!cValues)
      return 0;

    *count = (int)cValues;
    *max_name_size = (int)cMaxValueNameLen;
    *max_value_size = (int)cMaxValueLen;
    return 1;
  }
  else
    return 0;
}

typedef char regName[MAX_PATH];

static int iCompareStr(const void *a, const void *b)
{
  const regName*aa = a;
  const regName*bb = b;
  return strcmp(*aa, *bb);
}

static char* winRegFindValue(HKEY hBaseKey, const char* key_name, const char* value_name)
{
	HKEY hKey;
  int i, count, max_name_size, max_value_size;
  DWORD cchValueName;
  regName* ValueNames;

	if (RegOpenKeyExA(hBaseKey, key_name, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return NULL;

  if (!winRegGetValueCount(hKey, &count, &max_name_size, &max_value_size))
    return NULL;

  ValueNames = malloc(sizeof(regName)*count);

  for (i=0; i<count; i++)
  {
    cchValueName = MAX_PATH-1;

    if (RegEnumValueA(hKey, i, 
        ValueNames[i], &cchValueName, 
        NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
      break;
  }
  count = i;

  /* must sort to be able to do a partial compare */
  qsort(ValueNames, count, sizeof(regName), iCompareStr);

  RegCloseKey(hKey);

  for (i = 0; i<count; i++)
  {
    if (cdStrEqualNoCasePartial(ValueNames[i], value_name))
    {
      char* lpData = winRegReadStringKey(hBaseKey, key_name, ValueNames[i]);
      free(ValueNames);
      return lpData;
    }
  }

  free(ValueNames);
	return NULL;
}

int cdGetFontFileNameSystem(const char *type_face, int style, char* filename)
{
  char win_font_name[1024];
  char *font_dir, *font_title;

  font_dir = winRegReadStringKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Fonts");
  if (!font_dir)
    return 0;

  /* Filter some pre-defined names */
  if (cdStrEqualNoCase(type_face, "Courier") || 
      cdStrEqualNoCase(type_face, "Monospace") ||
      cdStrEqualNoCase(type_face, "System"))
    type_face = "Courier New";
  else if (cdStrEqualNoCase(type_face, "Times") || 
           cdStrEqualNoCase(type_face, "Serif"))
    type_face = "Times New Roman";
  else if (cdStrEqualNoCase(type_face, "Helvetica") || 
           cdStrEqualNoCase(type_face, "Sans"))
    type_face = "Arial";

  strcpy(win_font_name, type_face);

  /* add style */
  if (style & CD_BOLD)
    strcat(win_font_name, " Bold");
  if (style & CD_ITALIC)
    strcat(win_font_name, " Italic");

  font_title = winRegFindValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", win_font_name);
  if (font_title)
  {
    /* font_title already includes file extension, but may also contains a path */
    if (strchr(font_title, ':'))
      strcpy(filename, font_title);
    else
      sprintf(filename, "%s\\%s", font_dir, font_title);  
    free(font_title);
    free(font_dir);
    return 1;
  }

  free(font_dir);
  return 0;
}
#else
#ifndef NO_FONTCONFIG
#include <fontconfig/fontconfig.h>

int cdGetFontFileNameSystem(const char *type_face, int style, char* filename)
{
  char styles[4][20];
  int style_size;
  FcObjectSet *os;
  FcFontSet *fs;
  FcPattern *pat;
  int j, s, found = 0;

  /* Filter some pre-defined names */
  if (cdStrEqualNoCase(type_face, "Courier") || 
      cdStrEqualNoCase(type_face, "Courier New") || 
      cdStrEqualNoCase(type_face, "Monospace") ||
      cdStrEqualNoCase(type_face, "System"))
    type_face = "DejaVu Sans Mono";
  else if (cdStrEqualNoCase(type_face, "Times") || 
           cdStrEqualNoCase(type_face, "Times New Roman")|| 
           cdStrEqualNoCase(type_face, "Serif"))
    type_face = "DejaVu Serif";
  else if (cdStrEqualNoCase(type_face, "Helvetica") || 
           cdStrEqualNoCase(type_face, "Arial") || 
           cdStrEqualNoCase(type_face, "Sans"))
    type_face = "DejaVu Sans";

  /* add style */
  if( style&CD_BOLD && style&CD_ITALIC )
  {
    strcpy(styles[0], "BoldItalic");
    strcpy(styles[1], "Bold Italic");
    strcpy(styles[2], "Bold Oblique");
    strcpy(styles[3], "BoldOblique");
    style_size = 4;
  }
  else if( style & CD_BOLD )
  {
    strcpy(styles[0], "Bold");
    style_size = 1;
  }
  else if( style & CD_ITALIC )
  {
    strcpy(styles[0], "Italic");
    strcpy(styles[1], "Oblique");
    style_size = 2;
  }
  else
  {
    strcpy(styles[0], "Regular");
    strcpy(styles[1], "Normal");
    strcpy(styles[2], "Medium");
    strcpy(styles[3], "Book");
    style_size = 4;
  }

  pat = FcPatternCreate();
  os = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_STYLE, NULL);
  fs = FcFontList(NULL, pat, os);
  if (pat) FcPatternDestroy(pat);
  if (os) FcObjectSetDestroy(os);

  if(!fs)
    return 0;

  /* for all installed fonts */
  for (j = 0; j < fs->nfont; j++)
  {
    FcChar8 *family;
    FcPatternGetString(fs->fonts[j], FC_FAMILY, 0, &family );

    if (cdStrEqualNoCase((char*)family, type_face))
    {
      FcChar8 *file;
      FcChar8 *style;
      FcPatternGetString(fs->fonts[j], FC_FILE, 0, &file); 
      FcPatternGetString(fs->fonts[j], FC_STYLE, 0, &style);

      /* for all styles of that family */
      for(s = 0; s < style_size; s++ )
      {
        if (cdStrEqualNoCase(styles[s], (char*)style))
        {
          strcpy(filename, (char*)file);
          FcFontSetDestroy (fs);
          return 1;
        }
      }

      strcpy(filename, (char*)file);
      found = 1;  /* ignore styles */
    }
  }

  FcFontSetDestroy (fs);
  return found;
}
#else
int cdGetFontFileNameSystem(const char *type_face, int style, char* filename)
{
  (void)type_face;
  (void)style;
  (void)filename;
  return 0;
}
#endif
#endif

int cdGetFontFileNameDefault(const char *type_face, int style, char* filename)
{
  char font[10240];
  static char * cd_ttf_font_style[4] = {
    "",
    "bd",
    "i",
    "bi"};
  const char* face;

  /* check for the pre-defined names */
  if (cdStrEqualNoCase(type_face, "Courier") ||
      cdStrEqualNoCase(type_face, "System"))
    face = "cour";
  else if (cdStrEqualNoCase(type_face, "Times"))
    face = "times";
  else if (cdStrEqualNoCase(type_face, "Helvetica"))
    face = "arial";
  else
    face = type_face;

  sprintf(font, "%s%s", face, cd_ttf_font_style[style&3]);
  if (!cdGetFontFileName(font, filename))
  {
    /* try the type_face as a file title, but with no style */
    if (face == type_face && style != CD_PLAIN)
      return cdGetFontFileName(type_face, filename);
    else
      return 0;
  }
  else 
    return 1;
}

int cdGetFontFileName(const char* type_face, char* filename)
{
  FILE *file;

  if (!type_face)
    return 0;

  /* current directory */
  sprintf(filename, "%s.ttf", type_face);
  file = fopen(filename, "r");

  if (file)
    fclose(file);
  else
  {
    /* CD environment */
    char* env = getenv("CDDIR");
    if (env)
    {
#ifdef WIN32
      sprintf(filename, "%s\\%s.ttf", env, type_face);
#else
      sprintf(filename, "%s/%s.ttf", env, type_face);
#endif
      file = fopen(filename, "r");
    }

    if (file)
      fclose(file);
    else
    {
#ifdef WIN32
      /* Windows Font folder */
      char* font_dir = winRegReadStringKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Fonts");
      sprintf(filename, "%s\\%s.ttf", font_dir, type_face);
      file = fopen(filename, "r");
      free(font_dir);

      if (file)
        fclose(file);
      else
        return 0;
#else
      return 0;
#endif
    }
  }

  return 1;
}


/**************************************************************************************/

int cdStrTmpFileName(char* filename)
{
  /* a file with the result name is created and must be removed after use */
#ifdef WIN32
  char tmpPath[10240];
  if (GetTempPathA(10240, tmpPath)==0)
    return 0;
  if (GetTempFileNameA(tmpPath, "~cd", 0, filename)==0)
    return 0;
#elif OLD2
/* OLD1: tmpnam(filename)  */
  char* tmp = tempnam(NULL, "~cd");
  if (!tmp)
    return 0;
  strcpy(filename, tmp);
  free(tmp);
#else
  char* dirname = getenv("TMPDIR");
  if (!dirname) dirname = "/tmp";
  if (strlen(dirname) >= 10240-10)
    return 0;
  strcpy(filename, dirname);
  strcat(filename, "/~cdXXXXXX");
  int fd = mkstemp(filename);
  if (fd == -1)
    return 0;
  close(fd);
#endif
  return 1;
}

int cdMakeDirectory(const char *path)
{
#ifdef WIN32
  return CreateDirectoryA(path, NULL);
#else
  if (mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
                  S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
    return 0;
  else
    return 1;
#endif
}

int cdRemoveDirectory(const char *path)
{
#ifdef WIN32
  return RemoveDirectoryA(path);
#else
  if (rmdir(path) != 0)
    return 0;
  else
    return 1;
#endif
}

int cdIsDirectory(const char* path)
{
#ifdef WIN32   
  DWORD fattrib = GetFileAttributesA(path);
  if ((fattrib != INVALID_FILE_ATTRIBUTES) && (fattrib & FILE_ATTRIBUTE_DIRECTORY))
    return 1;
  return 0;
#else
  struct stat status;
  if (stat(path, &status) != 0)
    return 0;
  if (S_ISDIR(status.st_mode))
    return 1;
  return 0;
#endif
}

#ifndef WIN32
static int dirent_is_directory(struct dirent *entry)
{
#ifdef _DIRENT_HAVE_D_TYPE
  return entry->d_type==DT_DIR;
#else
  struct stat status;
  if (stat(entry->d_name, &status) != 0)
    return 0;
  if (S_ISDIR(status.st_mode))
    return 1;
  return 0;
#endif
}
#endif

int cdDirIter(cdDirData * dirData) 
{
#ifdef WIN32
  struct _finddata_t c_file;

  if (!dirData->handle) 
  { 
    /* first entry */
    char pattern[10240];
    sprintf(pattern, "%s/*", dirData->path);

    dirData->handle = (void*)_findfirst(pattern, &c_file);
    if (dirData->handle == (void*)-1L)
    {
      dirData->closed = 1;
      return 2;
    }
    else 
    {
      strcpy(dirData->filename, c_file.name);
      dirData->isDir = (c_file.attrib & _A_SUBDIR) ? 1 : 0;
      return 1;
    }
  }
  else 
  { 
    /* next entry */
    if (_findnext((intptr_t)dirData->handle, &c_file) == -1L) 
    {
      /* no more entries => close directory */
      _findclose((intptr_t)dirData->handle);
      dirData->closed = 1;
      return 2;
    }
    else 
    {
      strcpy(dirData->filename, c_file.name);
      dirData->isDir = (c_file.attrib & _A_SUBDIR) ? 1 : 0;
      return 1;
    }
  }
#else
  struct dirent *entry = readdir((DIR*)dirData->handle);
  if (entry) 
  {
    strcpy(dirData->filename, entry->d_name);
    dirData->isDir = dirent_is_directory(entry);
    return 1;
  }
  else 
  {
    /* no more entries => close directory */
    closedir(dirData->handle);
    dirData->closed = 1;
    return 2;
  }
#endif
}

void cdDirClose(cdDirData* dirData) 
{
#ifdef WIN32
  if (!dirData->closed && dirData->handle) 
    _findclose((intptr_t)dirData->handle);
#else
  if (!dirData->closed && dirData->handle) 
    closedir(dirData->handle);
#endif
  dirData->closed = 1;
  free(dirData);
}

cdDirData* cdDirIterOpen(const char *path)
{
  cdDirData *dirData = (cdDirData *) malloc(sizeof(cdDirData));
  memset(dirData, 0, sizeof(cdDirData));
  dirData->path = path;
#ifndef WIN32
  dirData->handle = opendir(path);
  if (!dirData->handle)
  {
    free(dirData);
    return NULL;
  }
#endif
  return dirData;
}

#ifndef WIN32
static int cp(const char *from, const char *to)
{
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return -1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while (nread = read(fd_from, buf, sizeof buf), nread > 0)
  {
    char *out_ptr = buf;
    ssize_t nwritten;

    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0)
      {
        nread -= nwritten;
        out_ptr += nwritten;
      }
      else if (errno != EINTR)
      {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0)
  {
    if (close(fd_to) < 0)
    {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);

    /* Success! */
    return 0;
  }

out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);

  errno = saved_errno;
  return -1;
}
#endif

void cdCopyFile(const char* srcFile, const char* destFile)
{
#ifdef WIN32
  CopyFileA(srcFile, destFile, 0);
#else
  cp(srcFile, destFile);
#endif
}


/**************************************************************************************/


#ifdef USE_ICONV
#include <iconv.h>
#else
#ifndef WIN32
#include <glib.h>
#endif
#endif

static char* cdCheckUtf8Buffer(char* utf8_buffer, int *utf8_buffer_max, int len)
{
  if (!utf8_buffer)
  {
    utf8_buffer = malloc(len + 1);
    *utf8_buffer_max = len;
  }
  else if (*utf8_buffer_max < len)
  {
    utf8_buffer = realloc(utf8_buffer, len + 1);
    *utf8_buffer_max = len;
  }

  return utf8_buffer;
}

static char* cdStrCopyToUtf8Buffer(const char* str, int len, char* utf8_buffer, int *utf8_buffer_max)
{
  utf8_buffer = cdCheckUtf8Buffer(utf8_buffer, utf8_buffer_max, len);
  memcpy(utf8_buffer, str, len);
  utf8_buffer[len] = 0;
  return utf8_buffer;
}

char* cdStrConvertToUTF8(const char* str, int len, char* utf8_buffer, int *utf8_buffer_max, int utf8mode)
{
  if (utf8mode || cdStrIsAscii(str)) /* string is already utf8 or is ascii */
    return cdStrCopyToUtf8Buffer(str, len, utf8_buffer, utf8_buffer_max);

#ifdef WIN32
  {
    int mlen;
    wchar_t* wstr;
    int wlen = MultiByteToWideChar(CP_ACP, 0, str, len, NULL, 0);
    if (!wlen)
      return cdStrCopyToUtf8Buffer(str, len, utf8_buffer, utf8_buffer_max);

    wstr = (wchar_t*)calloc((wlen + 1), sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str, len, wstr, wlen);
    wstr[wlen] = 0;

    mlen = WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, NULL, 0, NULL, NULL);
    if (!mlen)
    {
      free(wstr);
      return cdStrCopyToUtf8Buffer(str, len, utf8_buffer, utf8_buffer_max);
    }

    utf8_buffer = cdCheckUtf8Buffer(utf8_buffer, utf8_buffer_max, mlen);

    WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, utf8_buffer, mlen, NULL, NULL);
    utf8_buffer[mlen] = 0;

    free(wstr);
  }
#else
#ifdef USE_ICONV
  {
    size_t ulen = (size_t)len;
    size_t mlen = ulen * 2;
    iconv_t cd_iconv = iconv_open("UTF-8", "ISO-8859-1");

    if (cd_iconv == (iconv_t)-1)
      return cdStrCopyToUtf8Buffer(str, len, utf8_buffer, utf8_buffer_max);

    utf8_buffer = cdCheckUtf8Buffer(utf8_buffer, utf8_buffer_max, mlen);

    iconv(cd_iconv, (char**)&str, &ulen, &utf8_buffer, &mlen);
    utf8_buffer[mlen] = 0;

    iconv_close(cd_iconv);
  }
#else
  {
    int mlen;
    char* g_buffer;

    const char *charset = NULL;
    if (g_get_charset(&charset) == TRUE)  /* current locale is already UTF-8 */
    {
      if (g_utf8_validate(str, len, NULL))
        return cdStrCopyToUtf8Buffer(str, len, utf8_buffer, utf8_buffer_max);

      charset = "ISO8859-1";  /* if string is not UTF-8, assume ISO8859-1 */
    }

    if (!charset)
      charset = "ISO8859-1";  /* if charset not found, assume ISO8859-1 */

    g_buffer = g_convert(str, len, "UTF-8", charset, NULL, NULL, NULL);   
    if (!g_buffer) 
      return cdStrCopyToUtf8Buffer(str, len, utf8_buffer, utf8_buffer_max);

    mlen = (int)strlen(g_buffer);
    utf8_buffer = cdCheckUtf8Buffer(utf8_buffer, utf8_buffer_max, mlen);
    memcpy(utf8_buffer, g_buffer, mlen);
    utf8_buffer[mlen] = 0;

    g_free(g_buffer);
  }
#endif
#endif

  return utf8_buffer;
}
