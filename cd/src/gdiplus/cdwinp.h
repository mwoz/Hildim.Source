/** \file
 * \brief Windows GDI+ Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDWINP_H
#define __CDWINP_H

#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include "cd.h"
#include "cd_private.h"
 

/* Contexto de cada imagem no servidor */
struct _cdCtxImage
{
  Bitmap  *bitmap;     /* bitmap associado                            */
  int      w;          /* largura da imagem                           */
  int      h;          /* altura da imagem                            */
  double w_mm, h_mm;   /* size in mm                                  */                  
  double xres, yres;   /* resolution in pixels/mm                     */     
  int bpp;
  unsigned char* alpha; /* the alpha values must be stored here to be used at putimage */
}; 

/* Contexto de todos os canvas GDI+ (CanvasContext). */
struct _cdCtxCanvas
{
  cdCanvas* canvas;
  
  Graphics *graphics;

  Pen   *linePen;
  SolidBrush *lineBrush;
  Brush *fillBrush;
  Color fg, bg;      /* foreground, backgound                        */
  Font  *font;
  
  struct 
  {
    int height;        
    int ascent;        
    int descent;  
  } fontinfo;
  
  Point *clip_poly; /* coordenadas do pixel no X,Y                  */
  PointF *clip_fpoly; /* coordenadas do pixel no X,Y                  */
  int clip_poly_n;       /* numero de pontos correntes                   */
  Region *clip_region;

  Region *new_region;

  int antialias, txt_antialias;

  int radial_gradient_center_x;
  int radial_gradient_center_y;
  int radial_gradient_radius;

  Point gradient[2];

  Color pathGradient[500];

  int img_format;
  unsigned char* img_alpha;

  double rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  Point img_points[3];
  int use_img_points;

  Color img_transp[2];
  int use_img_transp;

  int wtype;               /* Flag indicando qual o tipo de superficie */ 

  HWND hWnd; /* CDW_WIN handle */
  HDC hDC;   /* GDI Device Context handle */            
  int release_dc;

  HANDLE printerHandle;  // Used by the printer driver

  Metafile* metafile;    /* CDW_EMF handle */
  Bitmap* bitmap;        /* CDW_BMP handle */

  int dirty;             // Used by the double buffer driver
  CachedBitmap* bitmap_dbuffer; /* not the image_dbuffer, just a cache */
  cdCanvas* canvas_dbuffer;
  int kill_dbuffer;

  int utf8mode;
};

enum{CDW_WIN, CDW_BMP, CDW_EMF};

extern "C" {
cdCtxCanvas *cdwpCreateCanvas(cdCanvas* canvas, Graphics* graphics, int wtype);
void cdwpKillCanvas(cdCtxCanvas* ctxcanvas);

void cdwpInitTable(cdCanvas* canvas);
void cdwpUpdateCanvas(cdCtxCanvas* canvas);

WCHAR* cdwpStringToUnicodeLen(const char* s, int *len, int utf8mode);
WCHAR* cdwpStringToUnicode(const char* s, int utf8mode);
char* cdwpStringFromUnicode(const WCHAR* wstr, int utf8mode);

void cdwpShowStatus(const char* title, Status status);

void cdwpGdiPlusStartup(int debug);
void cdwpGdiPlusShutdown(void);
}

#endif /* ifndef CDWINP_H */
