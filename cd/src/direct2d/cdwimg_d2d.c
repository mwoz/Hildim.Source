/** \file
 * \brief Direct2D Image Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwin_d2d.h"
#include "cdimage.h"
#include <stdlib.h>
#include <stdio.h>


static void cdflush(cdCtxCanvas *ctxcanvas)
{
  dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  dummy_ID2D1RenderTarget_EndDraw(ctxcanvas->d2d_canvas->target, NULL, NULL);

  d2dCanvasDestroy(ctxcanvas->d2d_canvas);
  cdwd2dKillCanvas(ctxcanvas);  /* this will NOT release the target */
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  cdCtxImage* ctximage;
  d2dCanvas* d2d_canvas;

  if (data == NULL)
    return;

  ctximage = ((cdImage*)data)->ctximage;

  if (ctximage == NULL)
    return;

  canvas->w = ctximage->w;
  canvas->h = ctximage->h;
  canvas->w_mm = ctximage->w_mm;
  canvas->h_mm = ctximage->h_mm;
  canvas->bpp = ctximage->bpp;
  canvas->xres = ctximage->xres;
  canvas->yres = ctximage->yres;

  ctxcanvas = cdwd2dCreateCanvas(canvas, NULL, NULL);

  d2d_canvas = d2dCreateCanvasWithTarget((dummy_ID2D1RenderTarget*)ctximage->bitmap_target);

  ctxcanvas->d2d_canvas = d2d_canvas;

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwd2dInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxFlush = cdflush;
}

static cdContext cdImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | CD_CAP_YAXIS),
  CD_CTX_IMAGE | CD_CTX_PLUS,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextDirect2DImage(void)
{
  return &cdImageContext;
}
