/** \file
 * \brief Gdk Native Window Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cd.h"
#include "cd_private.h"
#include "cdhaiku.h"

#include <Looper.h>
#include <View.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNIMPLEMENTED printf("%s (%s %d) UNIMPLEMENTED\n",__func__,__FILE__,__LINE__);

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdhaikuKillCanvas(ctxcanvas);
}

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  BView* view = ctxcanvas->view;

  BLooper* looper = view->Looper();
  const char* ln = "";
  if (looper != NULL) {
	  ln = looper->Name();
  }
  printf("CD Activate view %p (%s), looper is %s\n", view, view->Name(), ln);

  BRect rect = view->Bounds();

  ctxcanvas->canvas->w = (int)(rect.Width());
  ctxcanvas->canvas->h = (int)(rect.Height());

  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;

  if (ctxcanvas->canvas->use_matrix)
    ctxcanvas->canvas->cxTransform(ctxcanvas, ctxcanvas->canvas->matrix);
  return CD_OK;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  BView* view = (BView*)data;
  if(!view) {
	  return;
  }

  cdhaikuCreateCanvas(canvas, view);
}

static void cdinittable(cdCanvas* canvas)
{
  cdhaikuInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
}



static cdContext cdNativeWindowContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_FPRIMTIVES | CD_CAP_PATH | CD_CAP_BEZIER ),
  CD_CTX_WINDOW,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};


extern "C" {
cdContext* cdContextNativeWindow(void)
{
  return &cdNativeWindowContext;
}
}
