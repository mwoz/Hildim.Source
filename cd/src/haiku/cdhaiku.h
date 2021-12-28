#ifndef __CD_HAIKU_H
#define __CD_HAIKU_H

#include "cd_private.h"

class BBitmap;
class BView;

#ifdef __cplusplus
extern "C" {
#endif

struct _cdCtxCanvas {
  cdCanvas* canvas;
  BView* view;

  cdImage* image_dbuffer;       /* Used by double buffer driver */
  cdCanvas* canvas_dbuffer;
};

struct _cdCtxImage {
  BBitmap * bitmap;
};

cdContext* cdContextHaiku(void);

#define CD_HAIKU cdContextHaiku()

void cdhaikuInitTable(cdCanvas* canvas);
cdCtxCanvas* cdhaikuCreateCanvas(cdCanvas* canvas, BView* destination);
void cdhaikuKillCanvas(cdCtxCanvas *ctxcanvas);

#ifdef __cplusplus
}
#endif

#endif
