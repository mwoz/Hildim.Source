/** \file
 * \brief Direct2D Image Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwin_d2d.h"
#include "cdimage.h"
#include <stdlib.h>
#include <stdio.h>


static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char *red, unsigned char *green, unsigned char *blue, int x, int y, int w, int h)
{
  WICRect wic_rect;
  IWICBitmapLock *bitmap_lock = NULL;
  UINT dstStride = 0;
  UINT cbBufferSize = 0;
  int i, j;
  BYTE *Scan0 = NULL;
  HRESULT hr;

  /* if 0, invert because the transform was reset */
  if (!ctxcanvas->canvas->invert_yaxis)
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= h - 1;

  wic_rect.X = 0;
  wic_rect.Y = 0;
  wic_rect.Width = ctxcanvas->canvas->w;
  wic_rect.Height = ctxcanvas->canvas->h;

  hr = IWICBitmap_Lock(ctxcanvas->wic_bitmap, &wic_rect, WICBitmapLockWrite, &bitmap_lock);
  if (FAILED(hr))
    return;

  IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
  IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

  for (j = y; j < y+h; j++)
  {
    UINT line_offset = (ctxcanvas->canvas->h - 1 - j) * ctxcanvas->canvas->w;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = x; i < x+w; i++)
    {
      int offset_data = i * 4;
      red[line_offset + i] = line_data[offset_data + 0] * 255;
      green[line_offset + i] = line_data[offset_data + 1] * 255;
      blue[line_offset + i] = line_data[offset_data + 2] * 255;
    }
  }

  IWICBitmapLock_Release(bitmap_lock);
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);

  if (ctxcanvas->wic_bitmap)
    d2dDestroyImage(ctxcanvas->wic_bitmap);

  d2dCanvasDestroy(ctxcanvas->d2d_canvas);
  cdwd2dKillCanvas(ctxcanvas);  /* this will NOT release the target */
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  d2dCanvas* d2d_canvas;
  int w = 0, h = 0;
  double res = 3.78;
  char* str_data = (char*)data;
  char* res_ptr = NULL;
  IWICBitmap* wic_bitmap;

  if (data == NULL)
    return;

  res_ptr = strstr(str_data, "-r");
  if (res_ptr)
    sscanf(res_ptr + 2, "%lg", &res);

  /* size */
  sscanf(str_data, "%dx%d", &w, &h);

  if (w == 0) w = 1;
  if (h == 0) h = 1;

  canvas->w = w;
  canvas->h = h;
  canvas->yres = res;
  canvas->xres = res;
  canvas->w_mm = ((double)w) / res;
  canvas->h_mm = ((double)h) / res;
  canvas->bpp = 32;

  wic_bitmap = d2dCreateImage(w, h);
  if (!wic_bitmap)
    return;

  d2d_canvas = d2dCreateCanvasWithImage(wic_bitmap);
  if (!d2d_canvas)
  {
    d2dDestroyImage(wic_bitmap);
    return;
  }

  ctxcanvas = cdwd2dCreateCanvas(canvas, NULL, NULL);

  ctxcanvas->d2d_canvas = d2d_canvas;
  ctxcanvas->wic_bitmap = wic_bitmap;

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwd2dInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxFlush = cdflush;
  canvas->cxGetImageRGB = cdgetimagergb;
}

static cdContext cdImageRGBContext =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | CD_CAP_YAXIS),
  CD_CTX_IMAGE | CD_CTX_PLUS,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextDirect2DImageRGB(void)
{
  return &cdImageRGBContext;
}
