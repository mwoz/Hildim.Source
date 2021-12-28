/** \file
 * \brief Windows GDI+ Base Driver
 *
 * See Copyright Notice in cd.h
 */


#include "cdwin_d2d.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <malloc.h>

TCHAR* cdwStrToSystem(const char* str, int utf8mode);
char* cdwStrFromSystem(const TCHAR* wstr, int utf8mode);
TCHAR* cdwStrToSystemLen(const char* str, int *len, int utf8mode);

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix);

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  /* ignore ROTATE if transform is set,
  because there is native support for transformations */
  if (ctxcanvas->canvas->use_matrix)
    return;

  if (data)
  {
    sscanf(data, "%lg %d %d", &ctxcanvas->rotate_angle,
           &ctxcanvas->rotate_center_x,
           &ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;
  }

  cdtransform(ctxcanvas, NULL);
}

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  sprintf(data, "%g %d %d", ctxcanvas->rotate_angle,
          ctxcanvas->rotate_center_x,
          ctxcanvas->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
};

static void set_utf8mode_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
    ctxcanvas->utf8mode = 0;
  else
    ctxcanvas->utf8mode = 1;
}

static char* get_utf8mode_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->utf8mode)
    return "1";
  else
    return "0";
}

static cdAttribute utf8mode_attrib =
{
  "UTF8MODE",
  set_utf8mode_attrib,
  get_utf8mode_attrib
};

static void set_hatchboxsize_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  int hatchboxsize;

  if (data == NULL)
  {
    ctxcanvas->hatchboxsize = 8;
    return;
  }

  sscanf(data, "%d", &hatchboxsize);
  ctxcanvas->hatchboxsize = hatchboxsize;
}

static char* get_hatchboxsize_attrib(cdCtxCanvas *ctxcanvas)
{
  static char size[10];
  sprintf(size, "%d", ctxcanvas->hatchboxsize);
  return size;
}

static cdAttribute hatchboxsize_attrib =
{
  "HATCHBOXSIZE",
  set_hatchboxsize_attrib,
  get_hatchboxsize_attrib
};

static void set_lineargradient_brush(cdCtxCanvas* ctxcanvas)
{
  int y1, y2;

  y1 = ctxcanvas->linear_gradient_y1;
  y2 = ctxcanvas->linear_gradient_y2;

  if (ctxcanvas->canvas->invert_yaxis)
  {
    y1 = _cdInvertYAxis(ctxcanvas->canvas, y1);
    y2 = _cdInvertYAxis(ctxcanvas->canvas, y2);
  }

  if (ctxcanvas->fillBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
  ctxcanvas->fillBrush = d2dCreateLinearGradientBrush(ctxcanvas->d2d_canvas->target, (float)ctxcanvas->linear_gradient_x1, (float)y1, (float)ctxcanvas->linear_gradient_x2, (float)y2, ctxcanvas->canvas->foreground, ctxcanvas->canvas->background);
}

static void set_lineargradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int x1, y1, x2, y2;
    sscanf(data, "%d %d %d %d", &x1, &y1, &x2, &y2);

    ctxcanvas->linear_gradient_x1 = x1;
    ctxcanvas->linear_gradient_y1 = y1;
    ctxcanvas->linear_gradient_x2 = x2;
    ctxcanvas->linear_gradient_y2 = y2;

    set_lineargradient_brush(ctxcanvas);

    ctxcanvas->canvas->interior_style = CD_CUSTOMPATTERN;
    ctxcanvas->fillBrushType = FILL_BRUSH_LINEAR;
  }
}

static char* get_lineargradient_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  sprintf(data, "%d %d %d %d", ctxcanvas->linear_gradient_x1,
          ctxcanvas->linear_gradient_y1,
          ctxcanvas->linear_gradient_x2,
          ctxcanvas->linear_gradient_y2);

  return data;
}

static cdAttribute lineargradient_attrib =
{
  "LINEARGRADIENT",
  set_lineargradient_attrib,
  get_lineargradient_attrib
};

static cdAttribute old_lineargradient_attrib =
{
  "LINEGRADIENT",
  set_lineargradient_attrib,
  get_lineargradient_attrib
};

static void set_radialgradient_brush(cdCtxCanvas* ctxcanvas)
{
  int cy = ctxcanvas->radial_gradient_center_y;
  if (ctxcanvas->canvas->invert_yaxis)
    cy = _cdInvertYAxis(ctxcanvas->canvas, cy);

  if (ctxcanvas->fillBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
  ctxcanvas->fillBrush = d2dCreateRadialGradientBrush(ctxcanvas->d2d_canvas->target, (float)ctxcanvas->radial_gradient_center_x, (float)cy,
                                                      0, 0, (float)ctxcanvas->radial_gradient_radius, (float)ctxcanvas->radial_gradient_radius,
                                                      ctxcanvas->canvas->foreground, ctxcanvas->canvas->background);
}

static void set_radialgradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int cx, cy, r;

    sscanf(data, "%d %d %d", &cx, &cy, &r);

    ctxcanvas->radial_gradient_center_x = cx;
    ctxcanvas->radial_gradient_center_y = cy;
    ctxcanvas->radial_gradient_radius = r;

    set_radialgradient_brush(ctxcanvas);

    ctxcanvas->canvas->interior_style = CD_CUSTOMPATTERN;
    ctxcanvas->fillBrushType = FILL_BRUSH_RADIAL;
  }
}

static char* get_radialgradient_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  sprintf(data, "%d %d %d", ctxcanvas->radial_gradient_center_x,
          ctxcanvas->radial_gradient_center_y,
          ctxcanvas->radial_gradient_radius);

  return data;
}

static cdAttribute radialgradient_attrib =
{
  "RADIALGRADIENT",
  set_radialgradient_attrib,
  get_radialgradient_attrib
};

static void set_pattern_image_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
  {
    dummy_ID2D1Bitmap *bitmap;
    dummy_ID2D1BitmapBrush* brush;
    dummy_D2D1_BITMAP_BRUSH_PROPERTIES props;
    HRESULT hr;
    cdCtxImage *ctximage = (cdCtxImage *)data;

    hr = dummy_ID2D1BitmapRenderTarget_GetBitmap(ctximage->bitmap_target, &bitmap);
    if (FAILED(hr))
      return;

    props.extendModeX = dummy_D2D1_EXTEND_MODE_WRAP;
    props.extendModeY = dummy_D2D1_EXTEND_MODE_WRAP;
    props.interpolationMode = dummy_D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

    hr = dummy_ID2D1RenderTarget_CreateBitmapBrush(ctxcanvas->d2d_canvas->target, bitmap, &props, NULL, &brush);
    dummy_ID2D1Bitmap_Release(bitmap);

    if (FAILED(hr))
      return;

    if (ctxcanvas->fillBrush)
      dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);

    ctxcanvas->fillBrush = (dummy_ID2D1Brush*)brush;
    ctxcanvas->canvas->interior_style = CD_CUSTOMPATTERN;
  }
}

static cdAttribute pattern_image_attrib =
{
  "PATTERNIMAGE",
  set_pattern_image_attrib,
  NULL
};

static void set_interp_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && cdStrEqualNoCase(data, "NEAREST"))
    ctxcanvas->interpolation_mode = dummy_D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
  else
    ctxcanvas->interpolation_mode = dummy_D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
}

static char* get_interp_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->interpolation_mode == dummy_D2D1_BITMAP_INTERPOLATION_MODE_LINEAR)
    return "BILINEAR";
  else
    return "NEAREST";
}

static cdAttribute interp_attrib =
{
  "IMGINTERP",
  set_interp_attrib,
  get_interp_attrib
};

static char* get_direct2d_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return "1";
}

static cdAttribute direct2d_attrib =
{
  "Direct2D",
  NULL,
  get_direct2d_attrib
};

cdCtxCanvas *cdwd2dCreateCanvas(cdCanvas* canvas, HWND hWnd, HDC hDc)
{
  cdCtxCanvas* ctxcanvas;

  ctxcanvas = (cdCtxCanvas*)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  /* update canvas context */
  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;
  ctxcanvas->hWnd = hWnd;
  ctxcanvas->hDC = hDc;
  ctxcanvas->hatchboxsize = 8;

  cdRegisterAttribute(canvas, &utf8mode_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &hatchboxsize_attrib);
  cdRegisterAttribute(canvas, &lineargradient_attrib);
  cdRegisterAttribute(canvas, &old_lineargradient_attrib);
  cdRegisterAttribute(canvas, &radialgradient_attrib);
  cdRegisterAttribute(canvas, &direct2d_attrib);
  cdRegisterAttribute(canvas, &interp_attrib);
  cdRegisterAttribute(canvas, &pattern_image_attrib);

  return ctxcanvas;
}

void cdwd2dKillCanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->drawBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->drawBrush);

  if (ctxcanvas->fillBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);

  if (ctxcanvas->stroke_style)
    dummy_ID2D1StrokeStyle_Release(ctxcanvas->stroke_style);

  if (ctxcanvas->new_rgn)
    dummy_ID2D1PathGeometry_Release(ctxcanvas->new_rgn);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static long int cdforeground(cdCtxCanvas* ctxcanvas, long int color);
static int cdinteriorstyle(cdCtxCanvas* ctxcanvas, int style);

void cdwd2dUpdateCanvas(cdCtxCanvas* ctxcanvas)
{
  /* must update attributes that depends on target */

  cdforeground(ctxcanvas, ctxcanvas->canvas->foreground);

  if (ctxcanvas->canvas->interior_style != CD_SOLID)
  {
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);

    if (ctxcanvas->fillBrushType == FILL_BRUSH_RADIAL)
      set_radialgradient_brush(ctxcanvas);
    else if (ctxcanvas->fillBrushType == FILL_BRUSH_LINEAR)
      set_lineargradient_brush(ctxcanvas);
  }

  cdtransform(ctxcanvas, ctxcanvas->canvas->use_matrix ? ctxcanvas->canvas->matrix : NULL);

  /* no need to recreate the font, path geometry or stroke style, they do NOT depend on target */
}

static void sSetTransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    dummy_D2D1_MATRIX_3X2_F mtx;

    /* configure a bottom-up coordinate system */
    mtx._11 = 1.0f; mtx._12 = 0.0f;
    mtx._21 = 0.0f; mtx._22 = -1.0f;
    mtx._31 = 0.0f; mtx._32 = (float)(ctxcanvas->canvas->h - 1);
    d2dApplyTransform(ctxcanvas->d2d_canvas->target, &mtx);

    mtx._11 = (float)matrix[0]; mtx._12 = (float)matrix[1];
    mtx._21 = (float)matrix[2]; mtx._22 = (float)matrix[3];
    mtx._31 = (float)matrix[4]; mtx._32 = (float)matrix[5];
    d2dApplyTransform(ctxcanvas->d2d_canvas->target, &mtx);
  }
  else if (ctxcanvas->rotate_angle)
  {
    /* rotation = translate to point + rotation + translate back */
    /* the rotation must be corrected because of the Y axis orientation */
    d2dRotateWorld(ctxcanvas->d2d_canvas->target, (float)ctxcanvas->rotate_center_x, (float)_cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y), (float)-ctxcanvas->rotate_angle);
  }
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  /* reset to identity */
  d2dResetTransform(ctxcanvas->d2d_canvas->target);

  if (matrix)
    ctxcanvas->canvas->invert_yaxis = 0;  /* let the transformation do the axis inversion */
  else
    ctxcanvas->canvas->invert_yaxis = 1;

  sSetTransform(ctxcanvas, matrix);
}

static void cdnewregion(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->new_rgn)
    dummy_ID2D1Geometry_Release(ctxcanvas->new_rgn);
  ctxcanvas->new_rgn = d2dCreateBoxGeometry(0, 0, 0, 0);  /* create an empty region */
}

static int cdispointinregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
  dummy_D2D1_POINT_2F pt;
  int contains = 0;

  if (!ctxcanvas->new_rgn)
    return 0;

  pt.x = type2float(x);
  pt.y = type2float(y);

  dummy_ID2D1PathGeometry_FillContainsPoint(ctxcanvas->new_rgn, pt, NULL, &contains);

  return contains;
}

static cdCtxImage *cdcreateimage(cdCtxCanvas *ctxcanvas, int w, int h)
{
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));
  dummy_ID2D1BitmapRenderTarget* bitmap_target = NULL;
  dummy_D2D1_SIZE_U desiredPixelSize;
  HRESULT hr;

  desiredPixelSize.width = w;
  desiredPixelSize.height = h;

  hr = dummy_ID2D1RenderTarget_CreateCompatibleRenderTarget(ctxcanvas->d2d_canvas->target, NULL, &desiredPixelSize, NULL, dummy_D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE, &bitmap_target);
  if (FAILED(hr))
  {
    free(ctximage);
    return NULL;
  }

  ctximage->bitmap_target = bitmap_target;

  ctximage->w = w;
  ctximage->h = h;
  ctximage->bpp = ctxcanvas->canvas->bpp;
  ctximage->xres = ctxcanvas->canvas->xres;
  ctximage->yres = ctxcanvas->canvas->yres;
  ctximage->w_mm = ctximage->w / ctximage->xres;
  ctximage->h_mm = ctximage->h / ctximage->yres;

  return ctximage;
}

static void cdkillimage(cdCtxImage *ctximage)
{
  dummy_ID2D1BitmapRenderTarget_Release(ctximage->bitmap_target);
  free(ctximage);
}

static void cdgetimage(cdCtxCanvas* ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  dummy_D2D1_POINT_2U destPoint;
  dummy_D2D1_RECT_U srcRect;
  dummy_D2D1_MATRIX_3X2_F old_matrix;
  dummy_ID2D1Bitmap* bitmap;
  HRESULT hr;

  d2dGetTransform(ctxcanvas->d2d_canvas->target, &old_matrix);
  d2dResetTransform(ctxcanvas->d2d_canvas->target);

  destPoint.x = 0;
  destPoint.y = 0;

  /* if 0, invert because the transform was reset */
  if (!ctxcanvas->canvas->invert_yaxis)
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h - 1;

  hr = dummy_ID2D1BitmapRenderTarget_GetBitmap(ctximage->bitmap_target, &bitmap);
  if (FAILED(hr))
    return;

  srcRect.left = x;
  srcRect.right = x + ctximage->w;
  srcRect.top = y;
  srcRect.bottom = y + ctximage->h;

  dummy_ID2D1Bitmap_CopyFromRenderTarget(bitmap, &destPoint, ctxcanvas->d2d_canvas->target, &srcRect);

  d2dApplyTransform(ctxcanvas->d2d_canvas->target, &old_matrix);

  dummy_ID2D1Bitmap_Release(bitmap);
}

static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char *red, unsigned char *green, unsigned char *blue, int x, int y, int w, int h)
{
  dummy_ID2D1RenderTarget *target;
  IWICBitmap *wic_bitmap;
  WICRect wic_rect;
  IWICBitmapLock *bitmap_lock = NULL;
  dummy_ID2D1Bitmap* bitmap;
  dummy_D2D1_RENDER_TARGET_PROPERTIES props;
  dummy_D2D1_POINT_2U destPoint;
  dummy_D2D1_RECT_U srcRect;
  dummy_D2D1_RECT_F dstRect;
  dummy_D2D1_MATRIX_3X2_F old_matrix;
  UINT dstStride = 0;
  UINT cbBufferSize = 0;
  int i, j;
  BYTE *Scan0 = NULL;
  HRESULT hr;
  dummy_ID2D1BitmapRenderTarget* bitmap_target = NULL;
  dummy_D2D1_SIZE_U desiredPixelSize;

  d2dGetTransform(ctxcanvas->d2d_canvas->target, &old_matrix);
  d2dResetTransform(ctxcanvas->d2d_canvas->target);

  destPoint.x = 0;
  destPoint.y = 0;

  /* if 0, invert because the transform was reset */
  if (!ctxcanvas->canvas->invert_yaxis)
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= h - 1;


  /* Copy from target to a Bitmap - OK */

  desiredPixelSize.width = w;
  desiredPixelSize.height = h;

  hr = dummy_ID2D1RenderTarget_CreateCompatibleRenderTarget(ctxcanvas->d2d_canvas->target, NULL, &desiredPixelSize, NULL, dummy_D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE, &bitmap_target);
  if (FAILED(hr))
    return;

  hr = dummy_ID2D1BitmapRenderTarget_GetBitmap(bitmap_target, &bitmap);
  if (FAILED(hr))
  {
    dummy_ID2D1RenderTarget_Release(bitmap_target);
    return;
  }

  srcRect.left = x;
  srcRect.right = x + w;
  srcRect.top = y;
  srcRect.bottom = y + h;

  hr = dummy_ID2D1Bitmap_CopyFromRenderTarget(bitmap, &destPoint, ctxcanvas->d2d_canvas->target, &srcRect);
  if (FAILED(hr))
  {
    dummy_ID2D1RenderTarget_Release(bitmap_target);
    dummy_ID2D1Bitmap_Release(bitmap);
    return;
  }

  /* Copy from Bitmap to WIC Bitmap - FAIL */

  wic_bitmap = d2dCreateImage(w, h);
  if (!wic_bitmap)
    return;

  props.type = dummy_D2D1_RENDER_TARGET_TYPE_DEFAULT;
  props.pixelFormat.format = dummy_DXGI_FORMAT_B8G8R8A8_UNORM;
  props.pixelFormat.alphaMode = dummy_D2D1_ALPHA_MODE_PREMULTIPLIED;
  props.dpiX = 0.0f;
  props.dpiY = 0.0f;
  props.usage = 0;
  props.minLevel = dummy_D2D1_FEATURE_LEVEL_DEFAULT;

  hr = dummy_ID2D1Factory_CreateWicBitmapRenderTarget(d2d_cd_factory, wic_bitmap, &props, &target);
  if (FAILED(hr))
  {
    d2dDestroyImage(wic_bitmap);
    return;
  }

  dstRect.left = 0;
  dstRect.top = 0;
  dstRect.right = (FLOAT)w;
  dstRect.bottom = (FLOAT)h;

  dummy_ID2D1RenderTarget_BeginDraw(target);

  d2dBitBltBitmap(target, bitmap, &dstRect, &dstRect, ctxcanvas->interpolation_mode); /* FAIL HERE - bitmap and target are NOT compatible */

  hr = dummy_ID2D1RenderTarget_EndDraw(target, NULL, NULL);

  /* Copy from WicBitmap to RGB data - OK */

  wic_rect.X = 0;
  wic_rect.Y = 0;
  wic_rect.Width = w;
  wic_rect.Height = h;

  hr = IWICBitmap_Lock(wic_bitmap, &wic_rect, WICBitmapLockWrite, &bitmap_lock);
  if (FAILED(hr)) 
  {
    dummy_ID2D1RenderTarget_Release(target);
    dummy_ID2D1Bitmap_Release(bitmap);
    d2dDestroyImage(wic_bitmap);
    return;
  }

  IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
  IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

  for (j = 0; j < h; j++)
  {
    UINT line_offset = (h - 1 - j) * w;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < w; i++)
    {
      int offset_data = i * 4;
      red[line_offset + i] = line_data[offset_data + 0] * 255;
      green[line_offset + i] = line_data[offset_data + 1] * 255;
      blue[line_offset + i] = line_data[offset_data + 2] * 255;
    }
  }

  IWICBitmapLock_Release(bitmap_lock);

  dummy_ID2D1Bitmap_Release(bitmap);
  dummy_ID2D1RenderTarget_Release(target);
  d2dDestroyImage(wic_bitmap);

  d2dApplyTransform(ctxcanvas->d2d_canvas->target, &old_matrix);
}

static void cdputimagerect(cdCtxCanvas* ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  dummy_ID2D1Bitmap* bitmap;
  dummy_D2D1_RECT_F destRect;
  dummy_D2D1_RECT_F srcRect;
  HRESULT hr;

  /* y is already inverted, but must be relative to the top-left corner */
  /* must add a pixel to bottom-right */

  destRect.left = type2float(x);
  destRect.top = type2float(y - (ctximage->h - 1));
  destRect.right = type2float(x + ctximage->w);
  destRect.bottom = type2float(y + 1);

  srcRect.left = type2float(xmin);
  srcRect.top = type2float(ctximage->h - 1 - ymax);
  srcRect.right = type2float(xmax + 1);
  srcRect.bottom = type2float(ctximage->h - 1 - ymin + 1);

  hr = dummy_ID2D1BitmapRenderTarget_GetBitmap(ctximage->bitmap_target, &bitmap);
  if (FAILED(hr))
    return;

  d2dBitBltBitmap(ctxcanvas->d2d_canvas->target, bitmap, &destRect, &srcRect, ctxcanvas->interpolation_mode);

  dummy_ID2D1Bitmap_Release(bitmap);
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  IWICBitmap* wic_bitmap = d2dCreateImageFromHatch(style, ctxcanvas->hatchboxsize, ctxcanvas->canvas->back_opacity, ctxcanvas->canvas->foreground, ctxcanvas->canvas->background);
  if (!wic_bitmap)
    return style;

  if (ctxcanvas->fillBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
  ctxcanvas->fillBrush = d2dCreateImageBrush(ctxcanvas->d2d_canvas->target, wic_bitmap);
  ctxcanvas->fillBrushType = FILL_BRUSH_NORMAL;

  d2dDestroyImage(wic_bitmap);

  return style;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  IWICBitmap *wic_bitmap = d2dCreateImageFromStipple(n, m, stipple, ctxcanvas->canvas->back_opacity, ctxcanvas->canvas->foreground, ctxcanvas->canvas->background);
  if (!wic_bitmap)
    return;

  if (ctxcanvas->fillBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
  ctxcanvas->fillBrush = d2dCreateImageBrush(ctxcanvas->d2d_canvas->target, wic_bitmap);
  ctxcanvas->fillBrushType = FILL_BRUSH_NORMAL;

  d2dDestroyImage(wic_bitmap);
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long *pattern)
{
  IWICBitmap *wic_bitmap = d2dCreateImageFromPattern(n, m, pattern);
  if (!wic_bitmap)
    return;

  if (ctxcanvas->fillBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
  ctxcanvas->fillBrush = d2dCreateImageBrush(ctxcanvas->d2d_canvas->target, wic_bitmap);
  ctxcanvas->fillBrushType = FILL_BRUSH_NORMAL;

  d2dDestroyImage(wic_bitmap);
}

static int cdinteriorstyle(cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
    case CD_SOLID:
      if (ctxcanvas->fillBrush)
        dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
      ctxcanvas->fillBrush = d2dCreateSolidBrush(ctxcanvas->d2d_canvas->target, ctxcanvas->canvas->foreground);
      ctxcanvas->fillBrushType = FILL_BRUSH_NORMAL;
      break;
    case CD_HATCH:
      cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
      break;
    case CD_STIPPLE:
      cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
      break;
    case CD_PATTERN:
      cdpattern(ctxcanvas, ctxcanvas->canvas->pattern_w, ctxcanvas->canvas->pattern_h, ctxcanvas->canvas->pattern);
      break;
  }

  return style;
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  dummy_D2D1_COLOR_F c;
  d2dInitColor(&c, ctxcanvas->canvas->background);
  c.a = 1.0f; /* clear is opaque */
  dummy_ID2D1RenderTarget_Clear(ctxcanvas->d2d_canvas->target, &c);
}

static long int cdforeground(cdCtxCanvas* ctxcanvas, long int color)
{
  if (ctxcanvas->drawBrush)
    dummy_ID2D1Brush_Release(ctxcanvas->drawBrush);
  ctxcanvas->drawBrush = d2dCreateSolidBrush(ctxcanvas->d2d_canvas->target, color);

  if (ctxcanvas->canvas->interior_style == CD_SOLID)
  {
    if (ctxcanvas->fillBrush)
      dummy_ID2D1Brush_Release(ctxcanvas->fillBrush);
    ctxcanvas->fillBrush = d2dCreateSolidBrush(ctxcanvas->d2d_canvas->target, color);
    ctxcanvas->fillBrushType = FILL_BRUSH_NORMAL;
  }
  return color;
}

static int cdlinestyle(cdCtxCanvas* ctxcanvas, int style)
{
  if (ctxcanvas->stroke_style)
    dummy_ID2D1StrokeStyle_Release(ctxcanvas->stroke_style);

  if (style == CD_CUSTOM)
    ctxcanvas->stroke_style = d2dSetCustomLineStyle(ctxcanvas->canvas->line_dashes, ctxcanvas->canvas->line_dashes_count, type2float(ctxcanvas->canvas->line_width), ctxcanvas->canvas->line_cap, ctxcanvas->canvas->line_join);
  else
    ctxcanvas->stroke_style = d2dSetLineStyle(style, ctxcanvas->canvas->line_cap, ctxcanvas->canvas->line_join);

  return style;
}

static int cdlinecap(cdCtxCanvas* ctxcanvas, int cap)
{
  if (ctxcanvas->stroke_style)
    dummy_ID2D1StrokeStyle_Release(ctxcanvas->stroke_style);

  if (ctxcanvas->canvas->line_style == CD_CUSTOM)
    ctxcanvas->stroke_style = d2dSetCustomLineStyle(ctxcanvas->canvas->line_dashes, ctxcanvas->canvas->line_dashes_count, type2float(ctxcanvas->canvas->line_width), cap, ctxcanvas->canvas->line_join);
  else
    ctxcanvas->stroke_style = d2dSetLineStyle(ctxcanvas->canvas->line_style, cap, ctxcanvas->canvas->line_join);

  return cap;
}

static int cdlinejoin(cdCtxCanvas* ctxcanvas, int join)
{
  if (ctxcanvas->stroke_style)
    dummy_ID2D1StrokeStyle_Release(ctxcanvas->stroke_style);

  if (ctxcanvas->canvas->line_style == CD_CUSTOM)
    ctxcanvas->stroke_style = d2dSetCustomLineStyle(ctxcanvas->canvas->line_dashes, ctxcanvas->canvas->line_dashes_count, type2float(ctxcanvas->canvas->line_width), ctxcanvas->canvas->line_cap, join);
  else
    ctxcanvas->stroke_style = d2dSetLineStyle(ctxcanvas->canvas->line_style, ctxcanvas->canvas->line_cap, join);

  return join;
}

static void cdfpixel(cdCtxCanvas* ctxcanvas, double x, double y, long int color)
{
  dummy_ID2D1Brush *brush = d2dCreateSolidBrush(ctxcanvas->d2d_canvas->target, color);

  d2dFillArc(ctxcanvas->d2d_canvas->target, brush, (float)x, (float)y, 1.0f, 1.0f, 0.0f, 360.0f, 0);

  dummy_ID2D1Brush_Release(brush);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long color)
{
  cdfpixel(ctxcanvas, (double)x, (double)y, color);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  d2dDrawLine(ctxcanvas->d2d_canvas->target, ctxcanvas->drawBrush, (float)x1, (float)y1, (float)x2, (float)y2, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  if (ctxcanvas->canvas->line_width == 1)
  {
    /* compensate Direct2D horizontal and vertical lines when line_width == 1 */
    if (x1 == x2)
      cdfline(ctxcanvas, (double)x1, (double)y1 - 0.5, (double)x2, (double)y2 + 0.5);
    else if (y1 == y2)
      cdfline(ctxcanvas, (double)x1 - 0.5, (double)y1, (double)x2 + 0.5, (double)y2);
    else
      cdfline(ctxcanvas, (double)x1, type2float(y1), type2float(x2), type2float(y2));
  }
  else
    cdfline(ctxcanvas, (double)x1, (double)y1, (double)x2, (double)y2);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  d2dDrawRect(ctxcanvas->d2d_canvas->target, ctxcanvas->drawBrush, (float)xmin, (float)ymin, (float)xmax, (float)ymax, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->line_width == 1 && !ctxcanvas->canvas->use_matrix)
  {
    cdline(ctxcanvas, xmin, ymin, xmin, ymax);
    cdline(ctxcanvas, xmin, ymax, xmax, ymax);
    cdline(ctxcanvas, xmax, ymax, xmax, ymin);
    cdline(ctxcanvas, xmax, ymin, xmin, ymin);
  }
  else
    cdfrect(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static dummy_D2D1_COMBINE_MODE getCombineMode(int combineMode)
{
  switch (combineMode)
  {
    case CD_UNION:
      return dummy_D2D1_COMBINE_MODE_UNION;
      break;
    case CD_INTERSECT:
      return dummy_D2D1_COMBINE_MODE_INTERSECT;
      break;
    case CD_DIFFERENCE:
      return dummy_D2D1_COMBINE_MODE_EXCLUDE;
      break;
    case CD_NOTINTERSECT:
      return dummy_D2D1_COMBINE_MODE_XOR;
      break;
  }
  return dummy_D2D1_COMBINE_MODE_UNION;
}

static void regionCombineGeometry(cdCtxCanvas *ctxcanvas, dummy_ID2D1PathGeometry* g)
{
  dummy_ID2D1GeometrySink *sink;
  dummy_ID2D1PathGeometry* combined;

  HRESULT hr = dummy_ID2D1Factory_CreatePathGeometry(d2d_cd_factory, &combined);
  if (FAILED(hr))
    return;

  dummy_ID2D1PathGeometry_Open(combined, &sink);

  dummy_ID2D1PathGeometry_CombineWithGeometry(ctxcanvas->new_rgn, (dummy_ID2D1Geometry*)g, getCombineMode(ctxcanvas->canvas->combine_mode), NULL, 0.25f, sink);

  dummy_ID2D1GeometrySink_Close(sink);
  dummy_ID2D1GeometrySink_Release(sink);

  if (ctxcanvas->new_rgn)
    dummy_ID2D1PathGeometry_Release(ctxcanvas->new_rgn);
  ctxcanvas->new_rgn = combined;
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->new_region)
  {
    dummy_ID2D1PathGeometry* g = d2dCreateBoxGeometry(xmin, xmax, ymin, ymax);
    if (!g)
      return;

    regionCombineGeometry(ctxcanvas, g);

    dummy_ID2D1PathGeometry_Release(g);
  }
  else
    d2dFillRect(ctxcanvas->d2d_canvas->target, ctxcanvas->fillBrush, (float)xmin, (float)ymin, (float)xmax, (float)ymax);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  /* try to include the last integer line */
  if (!ctxcanvas->canvas->use_matrix)
    cdfbox(ctxcanvas, (double)xmin - 0.5, (double)xmax + 0.5, (double)ymin - 0.5, (double)ymax + 0.5);
  else
    cdfbox(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  d2dDrawArc(ctxcanvas->d2d_canvas->target, ctxcanvas->drawBrush, (float)xc, (float)yc, (float)w, (float)h, a1, a2, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  if (ctxcanvas->canvas->new_region)
  {
    dummy_ID2D1PathGeometry* g;
    float baseAngle = (float)(360.0 - a2);
    float sweepAngle = (float)(a2 - a1);

    g = d2dCreateArcGeometry((float)xc, (float)yc, (float)w / 2.f, (float)h / 2.f, baseAngle, sweepAngle, 2);
    if (!g)
      return;

    regionCombineGeometry(ctxcanvas, g);

    dummy_ID2D1PathGeometry_Release(g);
  }
  else
    d2dFillArc(ctxcanvas->d2d_canvas->target, ctxcanvas->fillBrush, (float)xc, (float)yc, (float)w, (float)h, a1, a2, 1);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  if (ctxcanvas->canvas->new_region)
  {
    dummy_ID2D1PathGeometry* g;
    float baseAngle = (float)(360.0 - a2);
    float sweepAngle = (float)(a2 - a1);

    g = d2dCreateArcGeometry((float)xc, (float)yc, (float)w / 2, (float)h / 2, baseAngle, sweepAngle, 2);
    if (!g)
      return;

    regionCombineGeometry(ctxcanvas, g);

    dummy_ID2D1PathGeometry_Release(g);
  }
  else
    d2dFillArc(ctxcanvas->d2d_canvas->target, ctxcanvas->fillBrush, (float)xc, (float)yc, (float)w, (float)h, a1, a2, 2);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfchord(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  if (mode == CD_PATH)
  {
    int clip_set = d2dPolyPath(ctxcanvas->d2d_canvas, ctxcanvas->drawBrush, ctxcanvas->fillBrush, (int*)poly, n, ctxcanvas->canvas->path, ctxcanvas->canvas->path_n, ctxcanvas->canvas->invert_yaxis, ctxcanvas->canvas->fill_mode, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);
    if (clip_set)
      ctxcanvas->canvas->clip_mode = CD_CLIPPATH;
  }
  else
  {
    dummy_ID2D1PathGeometry* g = d2dCreatePolygonGeometry((int*)poly, n, mode, ctxcanvas->canvas->fill_mode);
    if (!g)
      return;

    if (mode == CD_CLIP)
    {
      if (ctxcanvas->clip_poly)
        dummy_ID2D1PathGeometry_Release(ctxcanvas->clip_poly);
      ctxcanvas->clip_poly = g;
      g = NULL;
    }
    else if (mode == CD_FILL && ctxcanvas->canvas->new_region)
      regionCombineGeometry(ctxcanvas, g);
    else if (mode == CD_FILL)
      d2dFillGeometry(ctxcanvas->d2d_canvas->target, ctxcanvas->fillBrush, g);
    else
      d2dDrawGeometry(ctxcanvas->d2d_canvas->target, ctxcanvas->drawBrush, g, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);

    if (g)
      dummy_ID2D1Geometry_Release(g);
  }
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  if (mode == CD_PATH)
  {
    int clip_set = d2dPolyPathF(ctxcanvas->d2d_canvas, ctxcanvas->drawBrush, ctxcanvas->fillBrush, (double*)poly, n, ctxcanvas->canvas->path, ctxcanvas->canvas->path_n, ctxcanvas->canvas->invert_yaxis, ctxcanvas->canvas->fill_mode, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);
    if (clip_set)
      ctxcanvas->canvas->clip_mode = CD_CLIPPATH;
  }
  else
  {
    dummy_ID2D1PathGeometry* g = d2dCreatePolygonGeometryF((double*)poly, n, mode, ctxcanvas->canvas->fill_mode);
    if (!g)
      return;

    if (mode == CD_CLIP)
    {
      if (ctxcanvas->clip_poly)
        dummy_ID2D1PathGeometry_Release(ctxcanvas->clip_poly);
      ctxcanvas->clip_poly = g;
      g = NULL;
    }
    else if (mode == CD_FILL && ctxcanvas->canvas->new_region)
      regionCombineGeometry(ctxcanvas, g);
    else if (mode == CD_FILL)
      d2dFillGeometry(ctxcanvas->d2d_canvas->target, ctxcanvas->fillBrush, g);
    else
      d2dDrawGeometry(ctxcanvas->d2d_canvas->target, ctxcanvas->drawBrush, g, type2float(ctxcanvas->canvas->line_width), ctxcanvas->stroke_style);

    if (g)
      dummy_ID2D1Geometry_Release(g);
  }
}

static int cdfont(cdCtxCanvas* ctxcanvas, const char *type_face, int style, int size)
{
  LOGFONTW lf;
  d2dFont* new_font;
  LONG bold = 0, italic = 0, underline = 0, strikeout = 0, size_pixel;

  if (style&CD_BOLD)
    bold = FW_BOLD;
  else
    bold = FW_NORMAL;

  if (style&CD_ITALIC)
    italic = 1;

  if (style&CD_UNDERLINE)
    underline = 1;

  if (style&CD_STRIKEOUT)
    strikeout = 1;

  if (cdStrEqualNoCase(type_face, "Courier") || cdStrEqualNoCase(type_face, "Monospace"))
    type_face = "Courier New";
  else if (cdStrEqualNoCase(type_face, "Times") || cdStrEqualNoCase(type_face, "Serif"))
    type_face = "Times New Roman";
  else if (cdStrEqualNoCase(type_face, "Helvetica") || cdStrEqualNoCase(type_face, "Sans"))
    type_face = "Arial";

  size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, size);

  wcsncpy(lf.lfFaceName, cdwStrToSystem(type_face, ctxcanvas->utf8mode), LF_FACESIZE);
  lf.lfWeight = bold;
  lf.lfHeight = -size_pixel;
  lf.lfItalic = (BYTE)italic;
  lf.lfUnderline = (BYTE)underline;
  lf.lfStrikeOut = (BYTE)strikeout;
  lf.lfEscapement = ctxcanvas->font_angle;
  lf.lfOrientation = ctxcanvas->font_angle;
  lf.lfWidth = 0;
  lf.lfOutPrecision = OUT_TT_PRECIS;
  lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  lf.lfQuality = DEFAULT_QUALITY;
  lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;

  new_font = d2dCreateFont(&lf);
  if (!new_font)
    return 0;

  if (ctxcanvas->font)
    d2dDestroyFont(ctxcanvas->font);

  ctxcanvas->font = new_font;

  return 1;
}

static int cdnativefont(cdCtxCanvas* ctxcanvas, const char* nativefont)
{
  LOGFONT lf;
  int size = 12, bold = FW_NORMAL, italic = 0,
    style = CD_PLAIN, underline = 0, strikeout = 0,
    size_pixel;
  char type_face[1024];

  if (nativefont[0] == '-' && nativefont[1] == 'd')
  {
    COLORREF rgbColors;
    CHOOSEFONT cf;

    ZeroMemory(&cf, sizeof(CHOOSEFONT));

    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = GetForegroundWindow();
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
    rgbColors = cf.rgbColors = ctxcanvas->canvas->foreground;

    size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, ctxcanvas->canvas->font_size);

    if (style&ctxcanvas->canvas->font_style)
      bold = FW_BOLD;
    else
      bold = FW_NORMAL;

    if (style&ctxcanvas->canvas->font_style)
      italic = 1;

    if (style&ctxcanvas->canvas->font_style)
      underline = 1;

    if (style&ctxcanvas->canvas->font_style)
      strikeout = 1;

    lstrcpy(lf.lfFaceName, cdwStrToSystem(ctxcanvas->canvas->font_type_face, ctxcanvas->utf8mode));
    lf.lfWeight = bold;
    lf.lfHeight = -size_pixel;
    lf.lfItalic = (BYTE)italic;
    lf.lfUnderline = (BYTE)underline;
    lf.lfStrikeOut = (BYTE)strikeout;
    lf.lfEscapement = ctxcanvas->font_angle;
    lf.lfOrientation = ctxcanvas->font_angle;
    lf.lfWidth = 0;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;

    if (ChooseFont(&cf))
    {
      if (rgbColors != cf.rgbColors)
        cdCanvasSetForeground(ctxcanvas->canvas, cdEncodeColor(GetRValue(cf.rgbColors), GetGValue(cf.rgbColors), GetBValue(cf.rgbColors)));
    }
    else
      return 0;

    bold = lf.lfWeight;
    italic = lf.lfItalic;
    size = lf.lfHeight;
    strcpy(type_face, cdwStrFromSystem(lf.lfFaceName, ctxcanvas->utf8mode));
    underline = lf.lfUnderline;
    strikeout = lf.lfStrikeOut;

    if (bold != FW_NORMAL) style |= CD_BOLD;
    if (italic) style |= CD_ITALIC;
    if (underline) style |= CD_UNDERLINE;
    if (strikeout) style |= CD_STRIKEOUT;
  }
  else
  {
    if (!cdParseIupWinFont(nativefont, type_face, &style, &size))
    {
      if (!cdParsePangoFont(nativefont, type_face, &style, &size))
        return 0;
    }

    if (style&CD_BOLD)
      bold = FW_BOLD;
    if (style&CD_ITALIC)
      italic = 1;
    if (style&CD_UNDERLINE)
      underline = 1;
    if (style&CD_STRIKEOUT)
      strikeout = 1;

    size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, size);

    lstrcpy(lf.lfFaceName, cdwStrToSystem(type_face, ctxcanvas->utf8mode));
    lf.lfWeight = bold;
    lf.lfHeight = -size_pixel;
    lf.lfItalic = (BYTE)italic;
    lf.lfUnderline = (BYTE)underline;
    lf.lfStrikeOut = (BYTE)strikeout;
    lf.lfEscapement = ctxcanvas->font_angle;
    lf.lfOrientation = ctxcanvas->font_angle;
    lf.lfWidth = 0;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
  }

  if (ctxcanvas->font)
    d2dDestroyFont(ctxcanvas->font);

  ctxcanvas->font = d2dCreateFont(&lf);

  return 1;
}

static double cdtextorientation(cdCtxCanvas* ctxcanvas, double angle)
{
  int font_angle = (int)(angle * 10);
  if (ctxcanvas->font_angle == font_angle) /* first time angle=0, do not create font twice */
    return angle;

  ctxcanvas->font_angle = font_angle;

  /* recreate the font, because angle changed */
  cdfont(ctxcanvas, ctxcanvas->canvas->font_type_face, ctxcanvas->canvas->font_style, ctxcanvas->canvas->font_size);

  return angle;
}

static void cdgetfontdim(cdCtxCanvas* ctxcanvas, int *max_width, int *line_height, int *ascent, int *descent)
{
  d2dFontMetrics metrics;
  d2dFontGetMetrics(ctxcanvas->font, &metrics);

  if (max_width)
    d2dFontMeasureString(ctxcanvas->font, L"W", 1, max_width, NULL);

  if (line_height)
    *line_height = (int)metrics.fLeading;

  if (ascent)
    *ascent = (int)metrics.fAscent;

  if (descent)
    *descent = (int)metrics.fDescent;

  return;
}

static void cdgettextsize(cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
  d2dFontMeasureString(ctxcanvas->font, cdwStrToSystem(s, ctxcanvas->utf8mode), len, width, height);
}

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    d2dSetClipRect(ctxcanvas->d2d_canvas, xmin, ymin, xmax, ymax);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  if (mode == CD_CLIPAREA)
    d2dSetClipRect(ctxcanvas->d2d_canvas, ctxcanvas->canvas->clip_frect.xmin, ctxcanvas->canvas->clip_frect.ymin, ctxcanvas->canvas->clip_frect.xmax, ctxcanvas->canvas->clip_frect.ymax);
  else if (mode == CD_CLIPPOLYGON)
    d2dSetClipGeometry(ctxcanvas->d2d_canvas, ctxcanvas->clip_poly);
  else if (mode == CD_CLIPOFF)
    d2dResetClip(ctxcanvas->d2d_canvas);
  else if (mode == CD_CLIPREGION)
    d2dSetClipGeometry(ctxcanvas->d2d_canvas, ctxcanvas->new_rgn);
  return mode;
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  d2dFontMetrics fm;
  int w, h, desc;
  double ox = x, oy = y;

  /* cxFText method is called for single lines only */

  WCHAR* wstr = cdwStrToSystemLen(s, &len, ctxcanvas->utf8mode);

  d2dFontGetMetrics(ctxcanvas->font, &fm);
  desc = (int)fm.fDescent;
  d2dFontMeasureString(ctxcanvas->font, wstr, len, &w, &h);

  /* move to top-left corner of the text */
  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_BASE_RIGHT:
    case CD_NORTH_EAST:
    case CD_EAST:
    case CD_SOUTH_EAST:
      x = x - w;
      break;
    case CD_BASE_CENTER:
    case CD_CENTER:
    case CD_NORTH:
    case CD_SOUTH:
      x = x - w / 2;
      break;
    case CD_BASE_LEFT:
    case CD_NORTH_WEST:
    case CD_WEST:
    case CD_SOUTH_WEST:
      x = x;
      break;
  }

  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_BASE_LEFT:
    case CD_BASE_CENTER:
    case CD_BASE_RIGHT:
      y = y - h + desc;
      break;
    case CD_SOUTH_EAST:
    case CD_SOUTH_WEST:
    case CD_SOUTH:
      y = y - h;
      break;
    case CD_NORTH_EAST:
    case CD_NORTH:
    case CD_NORTH_WEST:
      y = y;
      break;
    case CD_CENTER:
    case CD_EAST:
    case CD_WEST:
      y = y - h / 2;
      break;
  }

  if (ctxcanvas->canvas->text_orientation)
  {
    double cos_angle = cos(ctxcanvas->canvas->text_orientation*CD_DEG2RAD);
    double sin_angle = sin(ctxcanvas->canvas->text_orientation*CD_DEG2RAD);
    cdfRotatePoint(ctxcanvas->canvas, x, y, ox, oy, &x, &y, sin_angle, cos_angle);
  }

  if (ctxcanvas->canvas->text_orientation)
    d2dRotateWorld(ctxcanvas->d2d_canvas->target, (float)x, (float)y, (float)-ctxcanvas->canvas->text_orientation);  /* counterclockwise */

  d2dDrawText(ctxcanvas->d2d_canvas->target, ctxcanvas->drawBrush, (float)x, (float)y, (float)w, (float)h, wstr, len, ctxcanvas->font);

  /* restore settings */
  if (ctxcanvas->canvas->text_orientation)
    cdtransform(ctxcanvas, ctxcanvas->canvas->use_matrix ? ctxcanvas->canvas->matrix : NULL);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, s, len);
}

static void cdfputimagerectrgba(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red,
                                const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  dummy_D2D1_RECT_F destRect;
  dummy_D2D1_RECT_F srcRect;

  IWICBitmap* wic_bitmap = d2dCreateImageFromBufferRGB(width, height, red, green, blue, alpha);
  if (!wic_bitmap)
    return;

  /* y is already inverted, but must be relative to the top-left corner */
  /* must add a pixel to bottom-right */

  destRect.left = type2float(x);
  destRect.top = type2float(y - (h - 1));
  destRect.right = type2float(x + w);
  destRect.bottom = type2float(y + 1);

  srcRect.left = type2float(xmin);
  srcRect.top = type2float(height - 1 - ymax);
  srcRect.right = type2float(xmax + 1);
  srcRect.bottom = type2float(height - 1 - ymin + 1);

  d2dBitBltImage(ctxcanvas->d2d_canvas->target, wic_bitmap, &destRect, &srcRect, ctxcanvas->interpolation_mode);

  d2dDestroyImage(wic_bitmap);
}

static void cdfputimagerectrgb(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red,
                               const unsigned char *green, const unsigned char *blue, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  dummy_D2D1_RECT_F destRect;
  dummy_D2D1_RECT_F srcRect;

  IWICBitmap* wic_bitmap = d2dCreateImageFromBufferRGB(width, height, red, green, blue, NULL);
  if (!wic_bitmap)
    return;

  /* y is already inverted, but must be relative to the top-left corner */
  /* must add a pixel to bottom-right */

  destRect.left = type2float(x);
  destRect.top = type2float(y - (h - 1));
  destRect.right = type2float(x + w);
  destRect.bottom = type2float(y + 1);

  srcRect.left = type2float(xmin);
  srcRect.top = type2float(height - 1 - ymax);
  srcRect.right = type2float(xmax + 1);
  srcRect.bottom = type2float(height - 1 - ymin + 1);

  d2dBitBltImage(ctxcanvas->d2d_canvas->target, wic_bitmap, &destRect, &srcRect, ctxcanvas->interpolation_mode);

  d2dDestroyImage(wic_bitmap);
}

static void cdfputimagerectmap(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *index,
                               const long int *colors, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  dummy_D2D1_RECT_F destRect;
  dummy_D2D1_RECT_F srcRect;

  IWICBitmap* wic_bitmap = d2dCreateImageFromBufferMap(width, height, index, colors);
  if (!wic_bitmap)
    return;

  /* y is already inverted, but must be relative to the top-left corner */
  /* must add a pixel to bottom-right */

  destRect.left = type2float(x);
  destRect.top = type2float(y - (h - 1));
  destRect.right = type2float(x + w);
  destRect.bottom = type2float(y + 1);

  srcRect.left = type2float(xmin);
  srcRect.top = type2float(height - 1 - ymax);
  srcRect.right = type2float(xmax + 1);
  srcRect.bottom = type2float(height - 1 - ymin + 1);

  d2dBitBltImage(ctxcanvas->d2d_canvas->target, wic_bitmap, &destRect, &srcRect, ctxcanvas->interpolation_mode);

  d2dDestroyImage(wic_bitmap);
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b,
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectrgb(ctxcanvas, iw, ih, r, g, b, (double)x, (double)y, (double)w, (double)h, xmin, xmax, ymin, ymax);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a,
                               int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectrgba(ctxcanvas, iw, ih, r, g, b, a, (double)x, (double)y, (double)w, (double)h, xmin, xmax, ymin, ymax);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long *colors,
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectmap(ctxcanvas, iw, ih, index, colors, (double)x, (double)y, (double)w, (double)h, xmin, xmax, ymin, ymax);
}

void cdwd2dInitTable(cdCanvas* canvas)
{
  canvas->cxClear = cdclear;

  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;

  canvas->cxFPixel = cdfpixel;
  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;
  canvas->cxFText = cdftext;

  canvas->cxFont = cdfont;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;

  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage;
/*  canvas->cxGetImageRGB = cdgetimagergb; NOT WORKING */
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxFPutImageRectRGBA = cdfputimagerectrgba;
  canvas->cxFPutImageRectRGB = cdfputimagerectrgb;
  canvas->cxFPutImageRectMap = cdfputimagerectmap;

  canvas->cxClip = cdclip;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxForeground = cdforeground;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxTransform = cdtransform;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
}
