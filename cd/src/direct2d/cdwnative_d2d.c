/** \file
 * \brief Direct2D Native Window Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwin_d2d.h"
#include "cdnative.h"
#include <stdlib.h>
#include <stdio.h>


static int cdactivate(cdCtxCanvas* ctxcanvas)
{
  d2dCanvas *d2d_canvas;

  if (!ctxcanvas->hWnd && !ctxcanvas->hDC)
    return CD_ERROR;

  if (ctxcanvas->d2d_canvas)
    dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);

  d2dCanvasDestroy(ctxcanvas->d2d_canvas);

  if (ctxcanvas->hWnd)
  {
    RECT rect;
    GetClientRect(ctxcanvas->hWnd, &rect);
    ctxcanvas->canvas->w = rect.right - rect.left;
    ctxcanvas->canvas->h = rect.bottom - rect.top;

    ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
    ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;

    ctxcanvas->canvas->bpp = cdGetScreenColorPlanes();

    d2d_canvas = d2dCreateCanvasWithWindow(ctxcanvas->hWnd, CANVAS_NOGDICOMPAT);
  }
  else if (ctxcanvas->hDC)
  {
    RECT rcPaint;
    rcPaint.left = 0;
    rcPaint.top = 0;
    rcPaint.right = ctxcanvas->canvas->w;
    rcPaint.bottom = ctxcanvas->canvas->h;

    d2d_canvas = d2dCreateCanvasWithHDC(ctxcanvas->hDC, &rcPaint, 0);
  }
  else
    return CD_ERROR;

  ctxcanvas->d2d_canvas = d2d_canvas;

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);

  cdwd2dUpdateCanvas(ctxcanvas);

  return CD_OK;
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  HRESULT hr = dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);
  if (hr == D2DERR_RECREATE_TARGET)
    cdactivate(ctxcanvas);

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->d2d_canvas)
    dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);

  if (ctxcanvas->hDC && ctxcanvas->release_dc)
    ReleaseDC(NULL, ctxcanvas->hDC);  /* to match GetDC(NULL) */

  d2dCanvasDestroy(ctxcanvas->d2d_canvas);
  cdwd2dKillCanvas(ctxcanvas);  /* this will NOT release the target */
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  HWND hWnd = NULL;
  HDC hDC = NULL, ScreenDC;
  int release_dc = 0;

  if (!data)
  {
    hDC = GetDC(NULL);
    release_dc = 1;
    canvas->w = GetDeviceCaps(hDC, HORZRES);
    canvas->h = GetDeviceCaps(hDC, VERTRES);
  }
  else if (IsWindow((HWND)data)) 
  {
    RECT rect;
    hWnd = (HWND)data;

    GetClientRect(hWnd, &rect);
    canvas->w = rect.right - rect.left;
    canvas->h = rect.bottom - rect.top;
  }
  else  /* can be a HDC or a string */
  {
    DWORD objtype = GetObjectType((HGDIOBJ)data);
    if (objtype == OBJ_DC || objtype == OBJ_MEMDC || 
        objtype == OBJ_ENHMETADC || objtype == OBJ_METADC)   
    {
      hDC = (HDC)data;
      canvas->w = GetDeviceCaps(hDC, HORZRES);
      canvas->h = GetDeviceCaps(hDC, VERTRES);
    }
    else
      return;
  }

  ScreenDC = GetDC(NULL);
  canvas->bpp = GetDeviceCaps(ScreenDC, BITSPIXEL);
  canvas->xres = ((double)GetDeviceCaps(ScreenDC, LOGPIXELSX)) / 25.4;
  canvas->yres = ((double)GetDeviceCaps(ScreenDC, LOGPIXELSY)) / 25.4;
  ReleaseDC(NULL, ScreenDC);

  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;

  ctxcanvas = cdwd2dCreateCanvas(canvas, hWnd, hDC);
  
  ctxcanvas->release_dc = release_dc;
  ctxcanvas->canvas->invert_yaxis = 1;

  cdactivate(ctxcanvas);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwd2dInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
/*  canvas->cxActivate = cdactivate; -- NOT necessary */
  canvas->cxFlush = cdflush;
}

static cdContext cdNativeContext =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | CD_CAP_YAXIS ),
  CD_CTX_WINDOW|CD_CTX_PLUS,
  cdcreatecanvas,
  cdinittable,
  NULL,              
  NULL,
};

cdContext* cdContextDirect2DNativeWindow(void)
{
  return &cdNativeContext;
}
