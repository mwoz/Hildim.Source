/** \file
* \brief CD PPTX driver
*
* See Copyright Notice in cd.h
*/

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <limits.h> 
#include <math.h>

#include "cd.h"
#include "cd_private.h"
#include "cdpptx.h"

#include "pptx.h"

struct _cdCtxCanvas
{
  /* public */
  cdCanvas* canvas;
  char filename[10240];

  pptxPresentation *presentation;

  int nDashes;
  int *dashes;

  char* utf8_buffer;
  int utf8mode, utf8_buffer_len;

  int isMasterSlide;

  char* masterSlideFile;
};

static const char* getHatchStyles(int style)
{
  switch (style)
  {
  default: /* CD_HORIZONTAL */
    return "ltHorz";
  case CD_VERTICAL:
    return "ltVert";
  case CD_FDIAGONAL:
    return "ltDnDiag";
  case CD_BDIAGONAL:
    return "ltUpDiag";
  case CD_CROSS:
    return "smGrid";
  case CD_DIAGCROSS:
    return "diagCross";
  }
}

static const char* getLineStyle(cdCtxCanvas *ctxcanvas, int lineStyle)
{
  int i;

  switch (lineStyle)
  {
  default: /* CD_CONTINUOUS */
    return "solid";
  case CD_DASHED:
    return "sysDash";
  case CD_DOTTED:
    return "sysDot";
  case CD_DASH_DOT:
    return "sysDashDot";
  case CD_DASH_DOT_DOT:
    return "sysDashDotDot";
  case CD_CUSTOM:
    ctxcanvas->nDashes = ctxcanvas->canvas->line_dashes_count;
    if (ctxcanvas->dashes)
      free(ctxcanvas->dashes);
    ctxcanvas->dashes = (int *)malloc(sizeof(int)*ctxcanvas->nDashes);
    for (i = 0; i < ctxcanvas->nDashes; i += 2)
    {
      ctxcanvas->dashes[i] = (ctxcanvas->canvas->line_dashes[i] / ctxcanvas->canvas->line_width) * 100;
      ctxcanvas->dashes[i + 1] = (ctxcanvas->canvas->line_dashes[i + 1] / ctxcanvas->canvas->line_width) * 100;
    }
    return "custom";
  }
}

static void setInteriorStyle(cdCtxCanvas *ctxcanvas, int interiorStyle, int hatchStyle, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha,
                             unsigned char bRed, unsigned char bGreen, unsigned char bBlue, unsigned char bAlpha, int backopacity)
{
  switch (interiorStyle)
  {
  case CD_SOLID:
    pptxSolidFill(ctxcanvas->presentation, red, green, blue, alpha);
    break;
  case CD_HATCH:
    if (backopacity == CD_TRANSPARENT)
      bAlpha = 0;
    pptxHatchLine(ctxcanvas->presentation, getHatchStyles(hatchStyle), red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha);
    break;
  case CD_PATTERN:
  {
    int width, height;
    long *pattern = cdCanvasGetPattern(ctxcanvas->canvas, &width, &height);
    int plane_size = width*height;
    unsigned char* rgba = (unsigned char*)malloc(plane_size * 4);
    int lin, col;

    for (lin = 0; lin < width; lin++)
    {
      for (col = 0; col < height; col++)
      {
        int ind = ((height - 1 - lin)*width + col) * 4;
        int i = width*lin + col;
        rgba[ind + 0] = cdRed(pattern[i]);
        rgba[ind + 1] = cdGreen(pattern[i]);
        rgba[ind + 2] = cdBlue(pattern[i]);
        rgba[ind + 3] = cdAlpha(pattern[i]);
      }
    }

    pptxPattern(ctxcanvas->presentation, rgba, width, height);

    free(rgba);
    break;
  }
  case CD_STIPPLE:
  {
    int width, height;
    unsigned char *stipple = cdCanvasGetStipple(ctxcanvas->canvas, &width, &height);
    long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
    long background = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
    int lin, col;
    int plane_size = width*height;
    unsigned char* rgba = (unsigned char*)malloc(plane_size * 4);

    for (lin = 0; lin < width; lin++)
    {
      for (col = 0; col < height; col++)
      {
        int ind = ((height - 1 - lin)*width + col) * 4;
        if (stipple[width*lin + col] == 0)
        {
          rgba[ind + 0] = cdRed(background);
          rgba[ind + 1] = cdGreen(background);
          rgba[ind + 2] = cdBlue(background);
          if (backopacity == CD_TRANSPARENT)
            rgba[ind + 3] = 0;
          else
            rgba[ind + 3] = cdAlpha(background);
        }
        else
        {
          rgba[ind + 0] = cdRed(foreground);
          rgba[ind + 1] = cdGreen(foreground);
          rgba[ind + 2] = cdBlue(foreground);
          rgba[ind + 3] = cdAlpha(foreground);
        }
      }
    }

    pptxStipple(ctxcanvas->presentation, rgba, width, height);

    free(rgba);
    break;
  }
  default: /* CD_HOLLOW */
    pptxNoFill(ctxcanvas->presentation);
    break;
  }
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  pptxKillPresentation(ctxcanvas->presentation, ctxcanvas->filename);

  if (ctxcanvas->masterSlideFile)
    free(ctxcanvas->masterSlideFile);

  if (ctxcanvas->dashes)
    free(ctxcanvas->dashes);

  if (ctxcanvas->utf8_buffer)
    free(ctxcanvas->utf8_buffer);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->isMasterSlide)
    return;

  pptxCloseSlide(ctxcanvas->presentation);

  pptxOpenSlide(ctxcanvas->presentation);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  /* dummy - must be defined */
  (void)ctxcanvas;
  return mode;
}

static void cdline(cdCtxCanvas* ctxcanvas, int x1, int y1, int x2, int y2)
{
  int xmin = x1;
  int xmax = x1;
  int ymin = y1;
  int ymax = y1;

  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);

  unsigned char alpha = cdAlpha(foreground);

  const char* lineStyle = getLineStyle(ctxcanvas, cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY));

  int line_width = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);

  if (x2<xmin) xmin = x2;
  if (x2>xmax) xmax = x2;
  if (y2<ymin) ymin = y2;
  if (y2>ymax) ymax = y2;

  pptxBeginPath(ctxcanvas->presentation, xmin, ymin, (xmax - xmin) + 1, (ymax - ymin) + 1);

  pptxMoveTo(ctxcanvas->presentation, x1 - xmin, y1 - ymin);

  pptxLineTo(ctxcanvas->presentation, x2 - xmin, y2 - ymin);

  pptxClosePath(ctxcanvas->presentation);

  pptxNoFill(ctxcanvas->presentation);

  pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);

  unsigned char alpha = cdAlpha(foreground);

  const char* lineStyle = getLineStyle(ctxcanvas, cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY));

  int line_width = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);

  pptxBeginPath(ctxcanvas->presentation, xmin, ymin, (xmax - xmin) + 1, (ymax - ymin) + 1);

  pptxMoveTo(ctxcanvas->presentation, 0, 0);

  pptxLineTo(ctxcanvas->presentation, xmax - xmin, 0);
  pptxLineTo(ctxcanvas->presentation, xmax - xmin, ymax - ymin);
  pptxLineTo(ctxcanvas->presentation, 0, ymax - ymin);
  pptxLineTo(ctxcanvas->presentation, 0, 0);

  pptxClosePath(ctxcanvas->presentation);

  pptxNoFill(ctxcanvas->presentation);

  pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  long background = cdCanvasBackground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);
  unsigned char alpha = cdAlpha(foreground);

  unsigned char bRed = cdRed(background);
  unsigned char bGreen = cdGreen(background);
  unsigned char bBlue = cdBlue(background);
  unsigned char bAlpha = cdAlpha(background);

  int backOpacity = cdCanvasBackOpacity(ctxcanvas->canvas, CD_QUERY);
  int interiorStyle = cdCanvasInteriorStyle(ctxcanvas->canvas, CD_QUERY);
  int hatchStyle = cdCanvasHatch(ctxcanvas->canvas, CD_QUERY);

  pptxBeginPath(ctxcanvas->presentation, xmin, ymin, (xmax - xmin) + 1, (ymax - ymin) + 1);

  pptxMoveTo(ctxcanvas->presentation, 0, 0);

  pptxLineTo(ctxcanvas->presentation, xmax - xmin, 0);
  pptxLineTo(ctxcanvas->presentation, xmax - xmin, ymax - ymin);
  pptxLineTo(ctxcanvas->presentation, 0, ymax - ymin);
  pptxLineTo(ctxcanvas->presentation, 0, 0);

  pptxClosePath(ctxcanvas->presentation);

  setInteriorStyle(ctxcanvas, interiorStyle, hatchStyle, red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha, backOpacity);

  pptxEndFill(ctxcanvas->presentation);
}

static void sCalcAngles(int xc, int yc, int arcStartX, int arcStartY, int arcEndX, int arcEndY, double *angle1, double *angle2)
{
  *angle1 = atan2(arcStartY - yc, arcStartX - xc)*CD_RAD2DEG;
  *angle2 = atan2(arcEndY - yc, arcEndX - xc)*CD_RAD2DEG;

  if (*angle1 < 0.)
    *angle1 += 360.;

  if (*angle2 < 0.)
    *angle2 += 360.;

  if (*angle2 < *angle1)
  {
    double tmp = *angle1;
    *angle1 = *angle2;
    *angle2 = tmp;
  }
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int arcStartX, arcStartY, arcEndX, arcEndY;
  double angle1, angle2;

  int pxmin = xc - (w / 2);
  int pymin = yc - (h / 2);

  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);

  unsigned char alpha = cdAlpha(foreground);

  const char* lineStyle = getLineStyle(ctxcanvas, cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY));

  int line_width = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);

  if (ctxcanvas->canvas->invert_yaxis)
    cdGetArcStartEnd(xc, yc, w, h, -a1, -a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);
  else
    cdGetArcStartEnd(xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);

  sCalcAngles(xc, yc, arcStartX, arcStartY, arcEndX, arcEndY, &angle1, &angle2);

  pptxBeginSector(ctxcanvas->presentation, "arc", pxmin, pymin, abs(w), abs(h), (int)angle1, (int)angle2);

  pptxNoFill(ctxcanvas->presentation);

  pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int arcStartX, arcStartY, arcEndX, arcEndY;
  double angle1, angle2;

  int pxmin = xc - (w / 2);
  int pymin = yc - (h / 2);

  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  long background = cdCanvasBackground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);
  unsigned char alpha = cdAlpha(foreground);

  unsigned char bRed = cdRed(background);
  unsigned char bGreen = cdGreen(background);
  unsigned char bBlue = cdBlue(background);
  unsigned char bAlpha = cdAlpha(background);

  int backOpacity = cdCanvasBackOpacity(ctxcanvas->canvas, CD_QUERY);
  int interiorStyle = cdCanvasInteriorStyle(ctxcanvas->canvas, CD_QUERY);
  int hatchStyle = cdCanvasHatch(ctxcanvas->canvas, CD_QUERY);

  if (ctxcanvas->canvas->invert_yaxis)
    cdGetArcStartEnd(xc, yc, w, h, -a1, -a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);
  else
    cdGetArcStartEnd(xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);

  sCalcAngles(xc, yc, arcStartX, arcStartY, arcEndX, arcEndY, &angle1, &angle2);

  pptxBeginSector(ctxcanvas->presentation, "pie", pxmin, pymin, abs(w), abs(h), (int)angle1, (int)angle2);

  setInteriorStyle(ctxcanvas, interiorStyle, hatchStyle, red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha, backOpacity);

  pptxEndFill(ctxcanvas->presentation);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int arcStartX, arcStartY, arcEndX, arcEndY;
  double angle1, angle2;

  int pxmin = xc - (w / 2);
  int pymin = yc - (h / 2);

  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  long background = cdCanvasBackground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);
  unsigned char alpha = cdAlpha(foreground);

  unsigned char bRed = cdRed(background);
  unsigned char bGreen = cdGreen(background);
  unsigned char bBlue = cdBlue(background);
  unsigned char bAlpha = cdAlpha(background);

  int backOpacity = cdCanvasBackOpacity(ctxcanvas->canvas, CD_QUERY);
  int interiorStyle = cdCanvasInteriorStyle(ctxcanvas->canvas, CD_QUERY);
  int hatchStyle = cdCanvasHatch(ctxcanvas->canvas, CD_QUERY);

  if (ctxcanvas->canvas->invert_yaxis)
    cdGetArcStartEnd(xc, yc, w, h, -a1, -a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);
  else
    cdGetArcStartEnd(xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);

  sCalcAngles(xc, yc, arcStartX, arcStartY, arcEndX, arcEndY, &angle1, &angle2);

  pptxBeginSector(ctxcanvas->presentation, "chord", pxmin, pymin, abs(w), abs(h), (int)angle1, (int)angle2);

  setInteriorStyle(ctxcanvas, interiorStyle, hatchStyle, red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha, backOpacity);

  pptxEndFill(ctxcanvas->presentation);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *text, int len)
{
  char typeface[1024];
  int bold = 0;
  int italic = 0;
  int strikeout = 0;
  int underline = 0;
  int style, size;
  int xmin, xmax, ymin, ymax;
  int px, py;
  int width, height;
  double rotAngle;
  char *str;
  int line_height, ascent, baseline;

  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);

  unsigned char alpha = cdAlpha(foreground);

  cdCanvasGetFont(ctxcanvas->canvas, typeface, &style, &size);

  if (style&CD_BOLD)
    bold = 1;

  if (style&CD_ITALIC)
    italic = 1;

  if (style&CD_STRIKEOUT)
    strikeout = 1;

  if (style&CD_UNDERLINE)
    underline = 1;

  rotAngle = ctxcanvas->canvas->text_orientation;

  ctxcanvas->canvas->text_orientation = 0.;

  str = cdStrDupN(text, len);

  cdCanvasGetTextBox(ctxcanvas->canvas, x, y, str, &xmin, &xmax, &ymin, &ymax);

  px = xmin;
  py = ymin;

  ctxcanvas->canvas->text_orientation = rotAngle;
  rotAngle *= -1;

  width = (xmax - xmin) + 1;
  height = (ymax - ymin) + 1;

  cdCanvasGetFontDim(ctxcanvas->canvas, NULL, &line_height, &ascent, NULL);
  baseline = line_height - ascent;

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_BASE_LEFT:
  case CD_BASE_CENTER:
  case CD_BASE_RIGHT:
    py -= baseline;
    py -= baseline;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH:
  case CD_SOUTH_WEST:
    py -= height / 2;
    py -= height / 2;
    break;
  case CD_NORTH_EAST:
  case CD_NORTH:
  case CD_NORTH_WEST:
    py += height / 2;
    py += height / 2;
    break;
  }

  /* PPTX multibyte strings are always UTF-8 */
  ctxcanvas->utf8_buffer = cdStrConvertToUTF8(str, len, ctxcanvas->utf8_buffer, &(ctxcanvas->utf8_buffer_len), ctxcanvas->utf8mode);

  pptxText(ctxcanvas->presentation, px, py, width, height, rotAngle, bold, italic, underline, strikeout, size, red, green, blue, alpha,
           typeface, ctxcanvas->utf8_buffer);

  free(str);
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  /* dummy - must be defined */
  (void)ctxcanvas;
  (void)type_face;
  (void)style;
  (void)size;
  return 1;
}

static void getPolyBBox(cdCtxCanvas *ctxcanvas, cdPoint* poly, int n, int mode, int *xmin, int *xmax, int *ymin, int *ymax)
{
  int x, y, i;

#define _BBOX()               \
  if (x > *xmax) *xmax = x;   \
  if (y > *ymax) *ymax = y;   \
  if (x < *xmin) *xmin = x;   \
  if (y < *ymin) *ymin = y;

  *xmin = poly[0].x;
  *xmax = poly[0].x;
  *ymin = poly[0].y;
  *ymax = poly[0].y;

  if (mode == CD_PATH)
  {
    int p;
    i = 0;
    for (p = 0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch (ctxcanvas->canvas->path[p])
      {
      case CD_PATH_MOVETO:
        if (i + 1 > n) return;
        x = poly[i].x;
        y = poly[i].y;
        _BBOX();
        i++;
        break;
      case CD_PATH_LINETO:
        if (i + 1 > n) return;
        x = poly[i].x;
        y = poly[i].y;
        _BBOX();
        i++;
        break;
      case CD_PATH_ARC:
      {
        int xc, yc, w, h;
        double a1, a2;
        int xmn, xmx, ymn, ymx;

        if (i + 3 > n) return;

        if (!cdGetArcPath(poly + i, &xc, &yc, &w, &h, &a1, &a2))
          return;

        cdGetArcBox(xc, yc, w, h, a1, a2, &xmn, &xmx, &ymn, &ymx);

        x = xmn;
        y = ymn;
        _BBOX();
        x = xmx;
        y = ymn;
        _BBOX();
        x = xmx;
        y = ymx;
        _BBOX();
        x = xmn;
        y = ymx;
        _BBOX();

        i += 3;
        break;
      }
      case CD_PATH_CURVETO:
        if (i + 3 > n) return;

        x = poly[i].x;
        y = poly[i].y;
        _BBOX();
        x = poly[i + 1].x;
        y = poly[i + 1].y;
        _BBOX();
        x = poly[i + 2].x;
        y = poly[i + 2].y;
        _BBOX();

        i += 3;
        break;
      default:
        break;
      }
    }
  }
  else if (mode == CD_BEZIER)
  {
    x = poly[0].x;
    y = poly[0].y;
    _BBOX();
    for (i = 1; i < n; i += 3)
    {
      x = poly[i].x;
      y = poly[i].y;
      _BBOX();
      x = poly[i + 1].x;
      y = poly[i + 1].y;
      _BBOX();
      x = poly[i + 2].x;
      y = poly[i + 2].y;
      _BBOX();
    }
  }
  else
  {
    for (i = 0; i < n; i++)
    {
      x = poly[i].x;
      y = poly[i].y;
      _BBOX();
    }
  }
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int xmin, xmax, ymin, ymax, i;

  long foreground = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  long background = cdCanvasBackground(ctxcanvas->canvas, CD_QUERY);

  unsigned char red = cdRed(foreground);
  unsigned char green = cdGreen(foreground);
  unsigned char blue = cdBlue(foreground);
  unsigned char alpha = cdAlpha(foreground);

  unsigned char bRed = cdRed(background);
  unsigned char bGreen = cdGreen(background);
  unsigned char bBlue = cdBlue(background);
  unsigned char bAlpha = cdAlpha(background);

  const char* lineStyle = getLineStyle(ctxcanvas, cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY));
  int line_width = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);
  int backOpacity = cdCanvasBackOpacity(ctxcanvas->canvas, CD_QUERY);
  int interiorStyle = cdCanvasInteriorStyle(ctxcanvas->canvas, CD_QUERY);
  int hatchStyle = cdCanvasHatch(ctxcanvas->canvas, CD_QUERY);

  if (mode == CD_CLIP)
    return;

  getPolyBBox(ctxcanvas, poly, n, mode, &xmin, &xmax, &ymin, &ymax);

  if (mode == CD_PATH)
  {
    int p, end_path;

    pptxBeginPath(ctxcanvas->presentation, xmin, ymin, (xmax - xmin) + 1, (ymax - ymin) + 1);
    end_path = 0;

    i = 0;
    for (p = 0; p < ctxcanvas->canvas->path_n; p++)
    {
      switch (ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        if (!end_path)
        {
          pptxClosePath(ctxcanvas->presentation);
          pptxNoFill(ctxcanvas->presentation);
          pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, 0, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
          end_path = 1;
        }

        pptxBeginPath(ctxcanvas->presentation, xmin, ymin, (xmax - xmin) + 1, (ymax - ymin) + 1);
        end_path = 0;
        break;
      case CD_PATH_MOVETO:
        if (i + 1 > n) return;
        pptxMoveTo(ctxcanvas->presentation, poly[i].x - xmin, poly[i].y - ymin);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i + 1 > n) return;
        pptxLineTo(ctxcanvas->presentation, poly[i].x - xmin, poly[i].y - ymin);
        i++;
        break;
      case CD_PATH_ARC:
      {
        int xc, yc, w, h;
        double a1, a2, angle1, angle2;
        int arcStartX, arcStartY, arcEndX, arcEndY;

        if (i + 3 > n) return;

        if (!cdGetArcPath(poly + i, &xc, &yc, &w, &h, &a1, &a2))
          return;

        i += 3;

        if (ctxcanvas->canvas->invert_yaxis)
          cdGetArcStartEnd(xc, yc, w, h, -a1, -a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);
        else
          cdGetArcStartEnd(xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY);

        angle1 = atan2(arcStartY - yc, arcStartX - xc)*CD_RAD2DEG;
        angle2 = atan2(arcEndY - yc, arcEndX - xc)*CD_RAD2DEG;
        angle2 -= angle1;

        pptxLineTo(ctxcanvas->presentation, (int)arcStartX - xmin, (int)arcStartY - ymin);

        pptxArcTo(ctxcanvas->presentation, h / 2, w / 2, angle1, angle2);
        break;
      }
      case CD_PATH_CURVETO:
        if (i + 3 > n) return;
        pptxBezierLineTo(ctxcanvas->presentation, poly[i].x - xmin, poly[i].y - ymin,
                         poly[i + 1].x - xmin, poly[i + 1].y - ymin,
                         poly[i + 2].x - xmin, poly[i + 2].y - ymin);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        pptxLineTo(ctxcanvas->presentation, poly[0].x - xmin, poly[0].y - ymin);
        pptxClosePath(ctxcanvas->presentation);
        pptxNoFill(ctxcanvas->presentation);
        pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
        end_path = 1;
        break;
      case CD_PATH_FILL:
        pptxClosePath(ctxcanvas->presentation);
        setInteriorStyle(ctxcanvas, interiorStyle, hatchStyle, red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha, backOpacity);
        pptxEndFill(ctxcanvas->presentation);
        end_path = 1;
        break;
      case CD_PATH_STROKE:
        pptxClosePath(ctxcanvas->presentation);
        pptxNoFill(ctxcanvas->presentation);
        pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
        end_path = 1;
        break;
      case CD_PATH_FILLSTROKE:
        pptxClosePath(ctxcanvas->presentation);
        setInteriorStyle(ctxcanvas, interiorStyle, hatchStyle, red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha, backOpacity);
        pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
        end_path = 1;
        break;
      case CD_PATH_CLIP:
        break;
      }
    }
    return;
  }

  pptxBeginPath(ctxcanvas->presentation, xmin, ymin, (xmax - xmin) + 1, (ymax - ymin) + 1);

  pptxMoveTo(ctxcanvas->presentation, poly[0].x - xmin, poly[0].y - ymin);

  if (mode == CD_BEZIER)
  {
    for (i = 1; i < n; i += 3)
      pptxBezierLineTo(ctxcanvas->presentation, poly[i].x - xmin, poly[i].y - ymin,
      poly[i + 1].x - xmin, poly[i + 1].y - ymin, poly[i + 2].x - xmin,
      poly[i + 2].y - ymin);
  }
  else
  {
    for (i = 1; i < n; i++)
      pptxLineTo(ctxcanvas->presentation, poly[i].x - xmin, poly[i].y - ymin);

    if (mode == CD_CLOSED_LINES)
      pptxLineTo(ctxcanvas->presentation, poly[0].x - xmin, poly[0].y - ymin);
  }

  pptxClosePath(ctxcanvas->presentation);

  if (mode == CD_FILL)
  {
    setInteriorStyle(ctxcanvas, interiorStyle, hatchStyle, red, green, blue, alpha, bRed, bGreen, bBlue, bAlpha, backOpacity);
    pptxEndFill(ctxcanvas->presentation);
  }
  else
  {
    pptxNoFill(ctxcanvas->presentation);
    pptxEndLine(ctxcanvas->presentation, line_width, red, green, blue, alpha, lineStyle, ctxcanvas->nDashes, ctxcanvas->dashes);
  }
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh;
  unsigned char* rgb;

  if (xmin<0 || ymin<0 || xmax - xmin + 1>iw || ymax - ymin + 1>ih) return;

  rw = xmax - xmin + 1;
  rh = ymax - ymin + 1;

  rgb = (unsigned char*)malloc(3 * rw*rh);
  if (!rgb)
    return;

  d = 0;
  for (i = ymax; i >= ymin; i--)
  {
    for (j = xmin; j <= xmax; j++)
    {
      int off = i*iw + j;
      long c = colors[index[off]];
      rgb[d] = cdRed(c); d++;
      rgb[d] = cdGreen(c); d++;
      rgb[d] = cdBlue(c); d++;
    }
  }

  pptxImageRGB(ctxcanvas->presentation, rw, rh, rgb, x, y, w, h);

  free(rgb);
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh;
  unsigned char* rgb;

  if (xmin<0 || ymin<0 || xmax - xmin + 1>iw || ymax - ymin + 1>ih) return;

  rw = xmax - xmin + 1;
  rh = ymax - ymin + 1;

  rgb = (unsigned char*)malloc(3 * rw*rh);
  if (!rgb)
    return;

  d = 0;
  for (i = ymax; i >= ymin; i--)
  {
    for (j = xmin; j <= xmax; j++)
    {
      int off = i*iw + j;
      rgb[d] = r[off]; d++;
      rgb[d] = g[off]; d++;
      rgb[d] = b[off]; d++;
    }
  }

  pptxImageRGB(ctxcanvas->presentation, rw, rh, rgb, x, y, w, h);

  free(rgb);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh;
  unsigned char* rgba;

  if (xmin<0 || ymin<0 || xmax - xmin + 1>iw || ymax - ymin + 1>ih) return;

  rw = xmax - xmin + 1;
  rh = ymax - ymin + 1;

  rgba = (unsigned char*)malloc(4 * rw*rh);
  if (!rgba)
    return;

  d = 0;
  for (i = ymax; i >= ymin; i--)
  {
    for (j = xmin; j <= xmax; j++)
    {
      int off = i*iw + j;
      rgba[d] = r[off]; d++;
      rgba[d] = g[off]; d++;
      rgba[d] = b[off]; d++;
      rgba[d] = a[off]; d++;
    }
  }

  pptxImageRGBA(ctxcanvas->presentation, rw, rh, rgba, x, y, w, h);

  free(rgba);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  pptxPixel(ctxcanvas->presentation, x, y, 1, cdRed(color), cdGreen(color), cdBlue(color), cdAlpha(color));
}


/*******************/
/* Canvas Creation */
/*******************/

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

static void set_master_slide_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && data[0] == '1' && ctxcanvas->isMasterSlide == 0)
  {
    ctxcanvas->isMasterSlide = 1;
    pptsBeginMasterFile(ctxcanvas->presentation);
  }
  else if (ctxcanvas->isMasterSlide == 1)
  {
    ctxcanvas->isMasterSlide = 0;
    pptsEndMasterFile(ctxcanvas->presentation);
  }
}

static char* get_master_slide_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->isMasterSlide)
    return "1";
  else
    return "0";
}

static cdAttribute master_slide_attrib =
{
  "MASTERSLIDE",
  set_master_slide_attrib,
  get_master_slide_attrib,
};

static void set_master_slide_file_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    if (ctxcanvas->masterSlideFile)
      free(ctxcanvas->masterSlideFile);
    ctxcanvas->masterSlideFile = cdStrDup(data);

    pptxSetImportedMasterSlideFile(ctxcanvas->presentation, data);
  }
};

static char* get_master_slide_file_attrib(cdCtxCanvas* ctxcanvas)
{
  return ctxcanvas->masterSlideFile;
}

static cdAttribute master_slide_file_attrib =
{
  "MASTERSLIDEFILE",
  set_master_slide_file_attrib,
  get_master_slide_file_attrib,
};

static void cdcreatecanvas(cdCanvas *canvas, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  double res = 11.81; /* 300 DPI in pixels/mm */
  double w_mm = 297, h_mm = 210;  /* A4 size in mm */
  cdCtxCanvas* ctxcanvas;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  sscanf(strdata, "%lgx%lg %lg", &w_mm, &h_mm, &res);

  canvas->w = (int)(w_mm * res);
  canvas->h = (int)(h_mm * res);
  canvas->w_mm = w_mm;
  canvas->h_mm = h_mm;
  canvas->xres = res;
  canvas->yres = res;

  /* top-down orientation */
  canvas->invert_yaxis = 1;

  canvas->bpp = 24;

  ctxcanvas->presentation = pptxCreatePresentation(canvas->w_mm, canvas->h_mm, canvas->w, canvas->h);
  if (!ctxcanvas->presentation)
  {
    free(ctxcanvas);
    return;
  }

  strcpy(ctxcanvas->filename, filename);

  /* store the base canvas */
  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  cdRegisterAttribute(canvas, &utf8mode_attrib);
  cdRegisterAttribute(canvas, &master_slide_attrib);
  cdRegisterAttribute(canvas, &master_slide_file_attrib);
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClip = cdclip;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;
  canvas->cxFont = cdfont;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdPPTXContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | CD_CAP_PLAY | CD_CAP_PALETTE | CD_CAP_YAXIS |
  CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_FONTDIM | CD_CAP_TEXTSIZE),
  CD_CTX_FILE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextPPTX(void)
{
  return &cdPPTXContext;
}
