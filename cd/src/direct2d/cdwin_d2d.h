/** \file
 * \brief Windows direct2d Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDWIN_D2D_H
#define __CDWIN_D2D_H

#define COBJMACROS

#include "cd.h"
#include "cd_private.h"
#include "cd_d2d.h"

enum { FILL_BRUSH_NORMAL, FILL_BRUSH_LINEAR, FILL_BRUSH_RADIAL, FILL_BRUSH_PATTERNIMAGE };

struct _cdCtxImage
{
  dummy_ID2D1BitmapRenderTarget *bitmap_target;
  int w, h;
  double w_mm, h_mm;   
  double xres, yres;   /* resolution in pixels/mm */
  int bpp;
};

struct _cdCtxCanvas
{
  cdCanvas* canvas;

  d2dCanvas *d2d_canvas;
  d2dFont *font;

  dummy_ID2D1Brush *drawBrush;
  dummy_ID2D1Brush *fillBrush;
  dummy_ID2D1StrokeStyle *stroke_style;

  int radial_gradient_center_x;
  int radial_gradient_center_y;
  int radial_gradient_radius;

  int linear_gradient_x1;
  int linear_gradient_y1;
  int linear_gradient_x2;
  int linear_gradient_y2;

  int fillBrushType;

  int interpolation_mode;

  HWND hWnd; /* CDW_WIN handle */
  HDC hDC;   /* GDI Device Context handle */            
  int release_dc;

  int font_angle;
  int utf8mode;

  int hatchboxsize;

  double rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  dummy_ID2D1PathGeometry* clip_poly;
  dummy_ID2D1PathGeometry *new_rgn;

  /* Used by the Double buffer driver */
  cdImage* image_dbuffer; 
  cdCanvas* canvas_dbuffer;
  int kill_dbuffer;

  /* Used by the IMAGERGB driver */
  IWICBitmap* wic_bitmap;
};

cdCtxCanvas *cdwd2dCreateCanvas(cdCanvas* canvas, HWND hWnd, HDC hDc);
void cdwd2dKillCanvas(cdCtxCanvas* ctxcanvas);
void cdwd2dUpdateCanvas(cdCtxCanvas* ctxcanvas);

void cdwd2dInitTable(cdCanvas* canvas);

#endif 
