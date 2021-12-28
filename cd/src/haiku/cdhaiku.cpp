/*
 Canvas Draw - CD_Haiku Driver
*/

#include "cd.h"
#include "cdhaiku.h"
#include "cd_private.h"
#include <stdlib.h> 
#include <stdio.h> 
#include <memory.h> 

#include <Bitmap.h>
#include <View.h>

#define UNIMPLEMENTED printf("%s (%s %d) UNIMPLEMENTED\n",__func__,__FILE__,__LINE__);


static rgb_color cdColorToHaiku(unsigned long rgb)
{
  rgb_color clrRGB;

  clrRGB.red   = cdRed(rgb);
  clrRGB.green = cdGreen(rgb);
  clrRGB.blue  = cdBlue(rgb);

  return clrRGB;
}


static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->SetHighColor(cdColorToHaiku(color));
  ctxcanvas->view->StrokeLine(BPoint(x, y), BPoint(x, y));
  ctxcanvas->view->UnlockLooper();
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
  ctxcanvas->view->UnlockLooper();
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  BPoint points[n];
  for(int i = 0; i < n; i++)
  {
	points[i] = BPoint(poly[i].x, poly[i].y);
  }
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->FillPolygon(points, n);
  ctxcanvas->view->UnlockLooper();
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->StrokeRect(BRect(xmin, ymin, xmax, ymax));
  ctxcanvas->view->UnlockLooper();
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->FillRect(BRect(xmin, ymin, xmax, ymax));
  ctxcanvas->view->UnlockLooper();
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->StrokeArc(BPoint(xc, yc), w, h, a1, a1 + a2);
  ctxcanvas->view->UnlockLooper();
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  UNIMPLEMENTED
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->DrawString(s, len, BPoint(x,y));
  ctxcanvas->view->UnlockLooper();
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *typeface, int style, int size)
{
  int face = 0;

  // Recognize Windows fonts and map them to something we have.
  if (cdStrEqualNoCase(typeface, "Courier") || cdStrEqualNoCase(typeface, "Courier New"))
    typeface = "Monospace";
  else if (cdStrEqualNoCase(typeface, "Times") || cdStrEqualNoCase(typeface, "Times New Roman"))
    typeface = "Serif";
  else if (cdStrEqualNoCase(typeface, "Helvetica") || cdStrEqualNoCase(typeface, "Arial"))
    typeface = "Sans";
  
  if (style & CD_BOLD)
    face |= B_BOLD_FACE;

  if (style & CD_ITALIC)
    face |= B_ITALIC_FACE;

  if (style & CD_UNDERLINE)
    face |= B_UNDERSCORE_FACE;

  if (style & CD_STRIKEOUT)
    face |= B_STRIKEOUT_FACE;

  size = cdGetFontSizePoints(ctxcanvas->canvas, size);
  
  BFont font;
  font.SetFace(style);
  // TODO SetFamilyAndStyle() (see what we did in IUP, may be reuseable ?)
  font.SetSize(size);

  if (ctxcanvas->view->LockLooper()) {
    ctxcanvas->view->SetFont(&font);
    ctxcanvas->view->UnlockLooper();
  }

  return 1;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  font_height metrics;
  ctxcanvas->view->GetFontHeight(&metrics);

  if (max_width) { *max_width = 8; UNIMPLEMENTED }
  if (height)    *height    = (int)(metrics.ascent + metrics.descent + metrics.leading);
  if (ascent)    *ascent    = (int)metrics.ascent;
  if (descent)   *descent   = (int)metrics.descent;
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  // TODO maybe needs to handle newlines ?
  if (width) *width = (int)ctxcanvas->view->StringWidth(s, len);
  cdgetfontdim(ctxcanvas, NULL, height, NULL, NULL);
}



void cdhaikuKillCanvas(cdCtxCanvas* ctxcanvas)
{
  // FIXME how are the canvas_dbuffer and image_dbuffer freed ? gdk version does not do it ?
  if (ctxcanvas->view->LockLooper()) {
    if (ctxcanvas->view->RemoveSelf())
      delete ctxcanvas->view;
    ctxcanvas->view->UnlockLooper();
	ctxcanvas->view = NULL; // be extra safe to make sure we won't reuse it...
  }
  free(ctxcanvas);
}


static long int cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  if (ctxcanvas->view->LockLooper()) {
	rgb_color c = cdColorToHaiku(color);
    ctxcanvas->view->SetLowColor(c);
    ctxcanvas->view->SetViewColor(c);
    ctxcanvas->view->UnlockLooper();
  }
  return color;
}

static long int cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{
  if (ctxcanvas->view->LockLooper()) {
    ctxcanvas->view->SetHighColor(cdColorToHaiku(color));
    ctxcanvas->view->UnlockLooper();
  }
  return color;
}

static cdCtxImage *cdcreateimage (cdCtxCanvas *ctxcanvas, int w, int h)
{
  // TODO we need to get the colorspace from cdCtxCanvas
  BBitmap* bitmap = new BBitmap(BRect(0, 0, w, h), B_RGB32, true);
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));

  /*
  ctximage->w = w;
  ctximage->h = h;
  ctximage->depth = ctxcanvas->depth;
  ctximage->scr   = ctxcanvas->scr;
  ctximage->vis   = ctxcanvas->vis;
*/
  ctximage->bitmap = bitmap;

  if (!ctximage->bitmap)
  {
    free(ctximage);
    return NULL;
  }

  return ctximage;
}

static void cdputimagerect (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  /* y is the bottom-left of the image region in CD */
  y -= (ymax-ymin+1)-1;

  int w = xmax - xmin;
  int h = ymax - ymin;
  if (ctxcanvas->view->LockLooper()) {
    ctxcanvas->view->DrawBitmapAsync(ctximage->bitmap, BRect(xmin, ymin, xmax, ymax),
	  BRect(x, y, x + w, y + h));
    ctxcanvas->view->UnlockLooper();
  }
}

static void cdkillimage (cdCtxImage *ctximage)
{
  delete ctximage->bitmap;
  free(ctximage);
}


void cdhaikuInitTable(cdCanvas* canvas)
{
//  canvas->cxFlush = cdflush;
//  canvas->cxClear = cdclear;

  canvas->cxPixel  = cdpixel;
  canvas->cxLine   = cdline;
  canvas->cxPoly   = cdpoly;
  canvas->cxRect   = cdrect;
  canvas->cxBox    = cdbox;
  canvas->cxArc    = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord  = cdSimChord;
  canvas->cxText   = cdtext;

  canvas->cxFont   = cdfont;

  // TODO optional
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  // cxFLine, cxFPoly, cxFRect,
  // cxFBox, cxFArc, cxFSector, cxFChord, cxFText, cxClip, cxClipArea,
  // cxFClipArea, cxBackOpacity, cxWriteMode, cxLineStyle, cxLineWidth,
  // cxLineJoin, cxLineCap, cxInteriorStyle, cxHatch, cxStipple, cxPattern,
  // cxNativeFont, cxTextAlignment, cxTextOrientation, cxPalette, cxTransform, 
  // cxIsPointInRegion, cxOffsetRegion, cxGetRegionBox, cxActivate, cxDeactivate
  // FIXME can we LockLooper and UnlockLooper in cxActivate and cxDeactivate ?
  canvas->cxBackground = cdbackground;
  canvas->cxForeground = cdforeground;

  // cxScrollArea, cxGetImage, cxNewRegion,
  canvas->cxCreateImage = cdcreateimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  // cxGetImageRGB, cxPutImageRectRGBA, cxPutImageRectMap, cxPutImageRectRGB
}


cdCtxCanvas* cdhaikuCreateCanvas(cdCanvas* canvas, BView* destination)
{
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->view = destination;

  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  if (destination->LockLooper())
  {
    BRect rect = destination->Bounds();
    destination->UnlockLooper();
    ctxcanvas->canvas->w = (int)(rect.Width());
    ctxcanvas->canvas->h = (int)(rect.Height());
  }

  // TODO
  // canvas->bpp, xres, yres, w_mm, h_mm, colormap
  // ctxcanvas->depth
  canvas->invert_yaxis = 1;

  /*
  cdRegisterAttribute(canvas, &gc_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &pangoversion_attrib);
  cdRegisterAttribute(canvas, &imgdither_attrib);
  cdRegisterAttribute(canvas, &interp_attrib);
  */
  
  return ctxcanvas;
}

extern "C" int cdBaseDriver(void)
{
  return CD_BASE_HAIKU;
}

