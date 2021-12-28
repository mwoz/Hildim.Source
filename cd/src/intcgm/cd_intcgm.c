
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <cd.h>
#include <cdcgm.h>

#include "cgm_play.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef int(*_cdcgmsizecb)(cdCanvas* canvas, int w, int h, double w_mm, double h_mm);
typedef int(*_cdcgmbegmtfcb)(cdCanvas* canvas, int *xmin, int *ymin, int *xmax, int *ymax);
typedef int(*_cdcgmcountercb)(cdCanvas* canvas, double size);
typedef int(*_cdcgmsclmdecb)(cdCanvas* canvas, short scl_mde, short *drw_mode, double *factor);
typedef int(*_cdcgmvdcextcb)(cdCanvas* canvas, short type, double *xmn, double *ymn, double *xmx, double *ymx);
typedef int(*_cdcgmbegpictcb)(cdCanvas* canvas, const char *name);
typedef int(*_cdcgmbegpictbcb)(cdCanvas* canvas, double scale_x, double scale_y, 
                               double vdc_x2pix, double vdc_y2pix,
                               double vdc_x2mm, double vdc_y2mm, int drw_mode, 
                               double xmin, double ymin, double xmax, double ymax);

static _cdcgmsizecb cdcgmsizecb = NULL;
static _cdcgmbegmtfcb cdcgmbegmtfcb = NULL;
static _cdcgmcountercb cdcgmcountercb = NULL;
static _cdcgmsclmdecb cdcgmsclmdecb = NULL;
static _cdcgmvdcextcb cdcgmvdcextcb = NULL;
static _cdcgmbegpictcb cdcgmbegpictcb = NULL;
static _cdcgmbegpictbcb cdcgmbegpictbcb = NULL;

int cdRegisterCallbackCGM(int cb, cdCallback func)
{
  switch (cb)
  {
  case CD_SIZECB:
    cdcgmsizecb = (_cdcgmsizecb)func;
    return CD_OK;
  case CD_CGMBEGMTFCB:
    cdcgmbegmtfcb = (_cdcgmbegmtfcb)func;
    return CD_OK;
  case CD_CGMCOUNTERCB:
    cdcgmcountercb = (_cdcgmcountercb)func;
    return CD_OK;
  case CD_CGMSCLMDECB:
    cdcgmsclmdecb = (_cdcgmsclmdecb)func;
    return CD_OK;
  case CD_CGMVDCEXTCB:
    cdcgmvdcextcb = (_cdcgmvdcextcb)func;
    return CD_OK;
  case CD_CGMBEGPICTCB:
    cdcgmbegpictcb = (_cdcgmbegpictcb)func;
    return CD_OK;
  case CD_CGMBEGPICTBCB:
    cdcgmbegpictbcb = (_cdcgmbegpictbcb)func;
    return CD_OK;
  }
  
  return CD_ERROR;
}

typedef struct {
  cdCanvas* canvas;
  int abort, first_pic, drawing_metric;

  cgmPoint vdc_first, vdc_second;
  int metric;
  double scale_factor;

  int xmin, xmax, ymin, ymax;
  double factorX, factorY;
  int scale;
} cdCGM;

#define sMin1(_v) (_v <= 1? 1: _v)

#define sScaleX(_x) (cd_cgm->scale? sScaleMM(cd_cgm,((_x) - cd_cgm->vdc_first.x) * cd_cgm->factorX + cd_cgm->xmin): (_x))
#define sScaleY(_y) (cd_cgm->scale? sScaleMM(cd_cgm,((_y) - cd_cgm->vdc_first.y) * cd_cgm->factorY + cd_cgm->ymin): (_y))
#define sScaleW(_w) sMin1(cd_cgm->scale? fabs(sScaleMM(cd_cgm,(_w) * cd_cgm->factorX)): (_w))
#define sScaleH(_h) sMin1(cd_cgm->scale? fabs(sScaleMM(cd_cgm,(_h) * cd_cgm->factorY)): (_h))

static int sRound(double x)
{
  return ((int)(x < 0? (x-0.5): (x+0.5)));
}

static double sScaleMM(cdCGM* cd_cgm, double v)
{
  if (cd_cgm->drawing_metric == 0)  /* ABSTRACT */
    return v;
  else
    return v*cd_cgm->scale_factor;
}

static void cdcgm_BeginMetafile(const char* name, cdCGM* cd_cgm)
{
  if (cdcgmbegmtfcb)
  {
    int ret = cdcgmbegmtfcb(cd_cgm->canvas, &(cd_cgm->xmin), &(cd_cgm->ymin), 
                                            &(cd_cgm->xmax), &(cd_cgm->ymax));
    if (ret == CD_ABORT)
      cd_cgm->abort = 1;
  }

  (void)name;
}

static void cdcgm_BeginPicture(const char* name, cdCGM* cd_cgm)
{
  int width, height;

  if (cd_cgm->first_pic) 
    cd_cgm->first_pic = 0;
  else
    cdCanvasFlush(cd_cgm->canvas); /* do it only if it has more than one picture */

  /* default clipping is ON */
  cdCanvasGetSize(cd_cgm->canvas, &width, &height, NULL, NULL);
  cdCanvasClipArea(cd_cgm->canvas, 0, width-1, 0, height-1);
  cdCanvasClip(cd_cgm->canvas, CD_CLIPAREA);

  if (cdcgmbegpictcb)
  {
    int ret = cdcgmbegpictcb(cd_cgm->canvas, name);
    if (ret == CD_ABORT)
      cd_cgm->abort = 1;
  }
}

static void cdcgm_BeginPictureBody(cdCGM* cd_cgm)
{
  if (cdcgmbegpictbcb)
  {
    /* TODO: the documentation does not describe these parameters, 
             so probably they were implemented for a specific application. 
             That application must be updated... */
    int ret = cdcgmbegpictbcb(cd_cgm->canvas, 1., 1., 
                              cd_cgm->factorX, cd_cgm->factorY,
                              cd_cgm->factorX*cd_cgm->scale_factor, cd_cgm->factorY*cd_cgm->scale_factor,
                              cd_cgm->drawing_metric,
                              cd_cgm->vdc_first.x, cd_cgm->vdc_first.y, 
                              cd_cgm->vdc_second.x, cd_cgm->vdc_second.y);
    if (ret == CD_ABORT)
    {
      cd_cgm->abort = 1;
      return;
    }
  }

  if (cdcgmsizecb)
  {
    int ret;
    double factor=1, w, h;

    w = fabs(cd_cgm->vdc_second.x - cd_cgm->vdc_first.x);
    h = fabs(cd_cgm->vdc_second.y - cd_cgm->vdc_first.y);

    if (cd_cgm->metric)
      factor = cd_cgm->scale_factor;

    ret = cdcgmsizecb(cd_cgm->canvas, (int)w, (int)h, w*factor, h*factor);
    if (ret == CD_ABORT)
      cd_cgm->abort = 1;
  }
}

static void cdcgm_DeviceExtent(cgmPoint* first, cgmPoint* second, cdCGM* cd_cgm)
{
  if (cdcgmvdcextcb)
  {
    int ret = cdcgmvdcextcb(cd_cgm->canvas, 1,  /* report as REAL always */
                            &(first->x), &(first->y),
                            &(second->x), &(second->y));
    if (ret == CD_ABORT)
    {
      cd_cgm->abort = 1;
      return;
    }
  }

  cd_cgm->vdc_second = *second;
  cd_cgm->vdc_first = *first;

  if (fabs(cd_cgm->vdc_second.x-cd_cgm->vdc_first.x+1)>1 && 
      fabs(cd_cgm->vdc_second.y-cd_cgm->vdc_first.y+1)>1 && 
      (cd_cgm->xmax-cd_cgm->xmin+1)>1 && 
      (cd_cgm->ymax-cd_cgm->ymin+1)>1)
  {
    cd_cgm->scale = 1;
    cd_cgm->factorX = ((double)(cd_cgm->xmax-cd_cgm->xmin+1)) / ((double)(cd_cgm->vdc_second.x-cd_cgm->vdc_first.x+1));
    cd_cgm->factorY = ((double)(cd_cgm->ymax-cd_cgm->ymin+1)) / ((double)(cd_cgm->vdc_second.y-cd_cgm->vdc_first.y+1));
  }
  else
    cd_cgm->scale = 0;
}

static void cdcgm_ScaleMode(int metric, double* factor, cdCGM* cd_cgm)
{
  if (cdcgmsclmdecb) 
  {
    short draw_metric = 0;
    int ret = cdcgmsclmdecb(cd_cgm->canvas, (short)metric, &draw_metric, factor);
    if (ret == CD_ABORT)
    {
      cd_cgm->abort = 1;
      return;
    }

    cd_cgm->metric = metric;
    cd_cgm->scale_factor = *factor;
    cd_cgm->drawing_metric = draw_metric;
  }
  else
  {
    cd_cgm->metric = metric;
    cd_cgm->scale_factor = *factor;
  }
}

static void cdcgm_BackgroundColor(cgmRGB color, cdCGM* cd_cgm)
{
  cdCanvasSetBackground(cd_cgm->canvas, cdEncodeColor(color.red, color.green, color.blue));
}

static void cdcgm_Transparency(int transp, cgmRGB color, cdCGM* cd_cgm)
{
  int style = transp? CD_TRANSPARENT: CD_OPAQUE;
  cdCanvasSetBackground(cd_cgm->canvas, cdEncodeColor(color.red, color.green, color.blue));
  cdCanvasBackOpacity (cd_cgm->canvas, style);
}

static void cdcgm_ClipRectangle(cgmPoint first, cgmPoint second, cdCGM* cd_cgm)
{
  cdfCanvasClipArea(cd_cgm->canvas, sScaleX(first.x), sScaleX(second.x),
                                    sScaleY(first.y), sScaleY(second.y));
}

static void cdcgm_ClipIndicator(int clip, cdCGM* cd_cgm)
{
  int style = clip? CD_CLIPAREA: CD_CLIPOFF;
  cdCanvasClip(cd_cgm->canvas, style);
}

static void cdcgm_PolyLine(int n, cgmPoint* pt, cdCGM* cd_cgm)
{
  int i;
  for (i=1; i<n; i+=2)
    cdfCanvasLine(cd_cgm->canvas, sScaleX(pt[i-1].x), sScaleY(pt[i-1].y), 
                                  sScaleX(pt[i].x),   sScaleY(pt[i].y));
}

static void cdcgm_PolyMarker(int n, cgmPoint* pt, cdCGM* cd_cgm)
{
  int i;

  if (cdCanvasMarkType(cd_cgm->canvas, CD_QUERY)==CD_BOX)
  {
    long color = cdCanvasForeground(cd_cgm->canvas, CD_QUERY);
    for (i=0; i<n; i++)
      cdCanvasPixel(cd_cgm->canvas, sRound(sScaleX(pt[i].x)), sRound(sScaleY(pt[i].y)), color);
  }
  else
  {
    for (i=0; i<n; i++)
      cdCanvasMark(cd_cgm->canvas, sRound(sScaleX(pt[i].x)), sRound(sScaleY(pt[i].y)));
  }
}

static void cdcgm_Rectangle(cgmPoint first, cgmPoint second, cdCGM* cd_cgm)
{
  cdfCanvasBox(cd_cgm->canvas, sScaleX(first.x), sScaleX(second.x),
                               sScaleY(first.y), sScaleY(second.y));
}

static void cdcgm_Polygon(int n, cgmPoint* pt, int fill, cdCGM* cd_cgm)
{
  int i, style;

  style = CD_FILL;
  if (fill==CGM_FILL)
    style = CD_FILL;
  else if (fill==CGM_CLOSEDLINES)
    style = CD_CLOSED_LINES;
  else if (fill==CGM_LINES)
    style = CD_OPEN_LINES;
  else if (fill==CGM_BEZIER)
    style = CD_BEZIER;

  cdCanvasBegin(cd_cgm->canvas, style);

  for (i=0; i<n; i++)
   cdfCanvasVertex(cd_cgm->canvas, sScaleX(pt[i].x), sScaleY(pt[i].y));

  cdCanvasEnd(cd_cgm->canvas);
}

static void cdcgm_Circle(cgmPoint center, double radius, cdCGM* cd_cgm)
{
  cdfCanvasSector(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), sScaleW(2*radius), sScaleH(2*radius), 0, 360);
}

static void cdcgm_Ellipse(cgmPoint center, cgmPoint first, cgmPoint second, cdCGM* cd_cgm)
{
  double w = 2*sScaleW(second.x-first.x);
  double h = 2*sScaleH(second.y-first.y);
  cdfCanvasSector(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), w, h, 0, 360);
}

static void fix_angles(cdCGM* cd_cgm, double *angle1, double *angle2)
{
  int small = 0;

  *angle1 *= CD_RAD2DEG;
  *angle2 *= CD_RAD2DEG;

  if (*angle1 < *angle2)
    small = 1;

  if (cd_cgm->factorY < 0)
  {
    /* change orientation */
    *angle1 *= -1;
    *angle2 *= -1;
  }

  if (cd_cgm->factorX < 0)
  {
    if (*angle1 < 180)
      *angle1 = 180 - *angle1;
    else
      *angle1 = 360 - *angle1;
    if (*angle2 < 180)
      *angle2 = 180 - *angle2;
    else
      *angle2 = 360 - *angle2;
  }

  /* swap, so the angle orientation is preserved */
  if ((small && *angle1 > *angle2) ||
      (!small && *angle1 < *angle2))
  {
    double t = *angle1;
    *angle1 = *angle2;
    *angle2 = t;
  }
}

static void cdcgm_CircularArc(cgmPoint center, double radius, double angle1, double angle2, int arc, cdCGM* cd_cgm)
{
  fix_angles(cd_cgm, &angle1, &angle2);

  if (arc==CGM_OPENARC)
    cdfCanvasArc(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), sScaleW(2*radius), sScaleH(2*radius), angle1, angle2);
  else
  {
    if (arc==CGM_PIE)
      cdfCanvasSector(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), sScaleW(2*radius), sScaleH(2*radius), angle1, angle2);
    else  /* CGM_CHORD */
      cdfCanvasChord(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), sScaleW(2*radius), sScaleH(2*radius), angle1, angle2);
  }
}

static void cdcgm_EllipticalArc(cgmPoint center, cgmPoint first, cgmPoint second, double angle1, double angle2, int arc, cdCGM* cd_cgm)
{
  /* oriented ellipsis are not supported in CD */
  double w = 2*sScaleW(second.x-first.x);
  double h = 2*sScaleH(second.y-first.y);

  fix_angles(cd_cgm, &angle1, &angle2);

  if (arc==CGM_OPENARC)
    cdfCanvasArc(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), w, h, angle1, angle2);
  else
  {
    if (arc==CGM_PIE)
      cdfCanvasSector(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), w, h, angle1, angle2);
    else  /* CGM_CHORD */
      cdfCanvasChord(cd_cgm->canvas, sScaleX(center.x), sScaleY(center.y), w, h, angle1, angle2);
  }
}

static void cdcgm_CellArray(cgmPoint corner1, cgmPoint corner2, cgmPoint corner3, int w, int h, unsigned char* rgb, cdCGM* cd_cgm)
{
  int cx1, cy1, cx2, cy2, cx3, cy3, 
      tmp, i, j, count, off, rgb_off;
  unsigned char *r, *g, *b;

  count = w*h;
  r = (unsigned char *) malloc (3*count*sizeof(unsigned char));
  if (!r) return;
  g = r + count;
  b = g + count;

  cx1 = sRound(sScaleX(corner1.x));
  cy1 = sRound(sScaleY(corner1.y));
  cx2 = sRound(sScaleX(corner2.x));
  cy2 = sRound(sScaleY(corner2.y));
  cx3 = sRound(sScaleX(corner3.x));
  cy3 = sRound(sScaleY(corner3.y));

  if ( cx1<cx3 && cy1==cy3 && cx2==cx3 && cy2>cy3 )
  {
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = 3*(w*i + j);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1==cx3 && cy1<cy3 && cx2>cx3 && cy2==cy3 )
  {
    tmp = w;   w = h;    h = tmp;
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = 3*(h*j + i);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1<cx3 && cy1==cy3 && cx2==cx3 && cy2<cy3 )
  {
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = 3*(w*(h-1 - i) + j);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1==cx3 && cy1>cy3 && cx2>cx3 && cy2==cy3 )
  {
    tmp = w;   w = h;    h = tmp;
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = h*j + (h-1 - i);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1>cx3 && cy1==cy3 && cx2==cx3 && cy2>cy3 )
  {
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = w*i + (w-1 - j);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1==cx3 && cy1>cy3 && cx2<cx3 && cy2==cy3 )
  {
    tmp = w;   w = h;    h = tmp;
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = h*(w-1 - j) + (h-1 - i);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1>cx3 && cy1==cy3 && cx2==cx3 && cy2<cy3 )
  {
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = w*(h-1 - i) + (w-1 - j);
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }
  else if ( cx1==cx3 && cy1<cy3 && cx2<cx3 && cy2==cy3 )
  {
    tmp = w;   w = h;    h = tmp;
    for ( i=0; i<h; i++ )
      for ( j=0; j<w; j++ )
      {
        off = w*i + j;
        rgb_off = h*(w-1 - j) + i;
        r[off] = rgb[rgb_off+0];
        g[off] = rgb[rgb_off+1];
        b[off] = rgb[rgb_off+2];
      }
  }

  if (cx1>cx2)
  {
    tmp = cx1;
    cx1 = cx2;
    cx2 = tmp;
  }

  if (cy1>cy2)
  {
    tmp = cy1;
    cy1 = cy2;
    cy2 = tmp;
  }

  cdCanvasPutImageRectRGB(cd_cgm->canvas, w, h, r, g, b, cx1, cy1, cx2-cx1+1, cy2-cy1+1,0,0,0,0);

  free(r);
}

static void cdcgm_Text(const char* text, cgmPoint pos, cdCGM* cd_cgm)
{
  cdfCanvasText(cd_cgm->canvas, sScaleX(pos.x), sScaleY(pos.y), text);
}

static void cdcgm_TextAttrib(const char* horiz_align, const char* vert_align, const char* font, double height, cgmRGB color, cgmPoint base_dir, cdCGM* cd_cgm)
{
  char* str, type_face[256];
  double angle;

  int style = CD_BASE_LEFT;
  if (strcmp(horiz_align, "LEFT")==0)
  {
    if (strcmp(vert_align, "TOP")==0 ||
        strcmp(vert_align, "CAP")==0)     /* no CAP support in CD */
      style = CD_NORTH_WEST;
    else if (strcmp(vert_align, "CENTER")==0)
      style = CD_WEST;
    else if (strcmp(vert_align, "BASELINE")==0)
      style = CD_BASE_LEFT;
    else if (strcmp(vert_align, "BOTTOM")==0)
      style = CD_SOUTH_WEST;
  }
  else if (strcmp(horiz_align, "CENTER")==0)
  {
    if (strcmp(vert_align, "TOP")==0 ||
        strcmp(vert_align, "CAP")==0)     /* no CAP support in CD */
      style = CD_NORTH;
    else if (strcmp(vert_align, "CENTER")==0)
      style = CD_CENTER;
    else if (strcmp(vert_align, "BASELINE")==0)
      style = CD_BASE_CENTER;
    else if (strcmp(vert_align, "BOTTOM")==0)
      style = CD_SOUTH;
  }
  else if (strcmp(horiz_align, "RIGHT")==0)
  {
    if (strcmp(vert_align, "TOP")==0 ||
        strcmp(vert_align, "CAP")==0)     /* no CAP support in CD */
      style = CD_NORTH_EAST;
    else if (strcmp(vert_align, "CENTER")==0)
      style = CD_EAST;
    else if (strcmp(vert_align, "BASELINE")==0)
      style = CD_BASE_RIGHT;
    else if (strcmp(vert_align, "BOTTOM")==0)
      style = CD_SOUTH_EAST;
  }

  cdCanvasTextAlignment(cd_cgm->canvas, style);

  angle = atan2(sScaleH(base_dir.y), sScaleW(base_dir.x))*CD_RAD2DEG;
  cdCanvasTextOrientation(cd_cgm->canvas, angle);

  style = CD_PLAIN;
  str = strstr(font, "BOLD");
  if (!str) str = strstr(font, "Bold");
  if (str)
    style |= CD_BOLD;
  str = strstr(font, "ITALIC");
  if (!str) str = strstr(font, "Italic");
  if (!str) str = strstr(font, "OBLIQUE");
  if (!str) str = strstr(font, "Oblique");
  if (str)
    style |= CD_ITALIC;

  str = strstr(font, "_");
  if (!str) str = strstr(font, "-");
  if (str)
    strncpy(type_face, font, str-font);
  else
    strcpy(type_face, font);

  height = sScaleH(height);
  if (height < 5) height = 5;

  cdCanvasFont(cd_cgm->canvas, type_face, style, sRound(-height));  /* always in vdc (pixels) */

  cdCanvasForeground(cd_cgm->canvas, cdEncodeColor(color.red, color.green, color.blue));
}

static double get_size_mode(cdCanvas* canvas, double size, const char* mode, double current_size)
{
  if (strcmp(mode, "SCALED")==0)
  {
    /* scale factor to be applied by the interpreter to a device-dependent "nominal" measure */
    return size*current_size;
  }
  else if (strcmp(mode, "FRACTIONAL")==0)
  {
    /* fraction of the horizontal dimension of the default device viewport */
    int width;
    cdCanvasGetSize(canvas, &width, NULL, NULL, NULL);
    return size*width;
  }
  else if (strcmp(mode, "MM")==0)
  {
    /* millimetres */
    cdfCanvasMM2Pixel(canvas, size, 0, &size, NULL);
    return size;
  }
  else /* "ABSOLUTE" */
  {
    /* pixels */
    return size;  
  }
}

static void cdcgm_LineAttrib(const char *type, const char *cap, const char *join, double width, const char *mode, cgmRGB color, cdCGM* cd_cgm)
{
  int style = CD_CONTINUOUS;
  int linecap = CD_CAPFLAT;
  int linejoin = CD_MITER;

  if (strcmp(type, "SOLID")==0)
    style = CD_CONTINUOUS;
  else if (strcmp(type, "DASH")==0)
    style = CD_DASHED;
  else if (strcmp(type, "DOT")==0)
    style = CD_DOTTED;
  else if (strcmp(type, "DASH_DOT")==0)
    style = CD_DASH_DOT;
  else if (strcmp(type, "DASH_DOT_DOT")==0)
    style = CD_DASH_DOT_DOT;

  if (strcmp(cap, "UNSPECIFIED")==0  || strcmp(cap, "BUTT")==0 || strcmp(cap, "TRIANGLE")==0)
    linecap = CD_CAPFLAT;
  else if (strcmp(cap, "ROUND")==0)
    linecap = CD_CAPROUND;
  else if (strcmp(cap, "PROJECTING_SQUARE")==0)
    linecap = CD_CAPSQUARE;

  if (strcmp(join, "UNSPECIFIED")==0  || strcmp(join, "MITRE")==0)
    linejoin = CD_MITER;
  else if (strcmp(join, "ROUND")==0)
    linejoin = CD_ROUND;
  else if (strcmp(join, "BEVEL")==0)
    linejoin = CD_BEVEL;

  cdCanvasLineCap(cd_cgm->canvas, linecap);

  cdCanvasLineJoin(cd_cgm->canvas, linejoin);

  cdCanvasLineStyle(cd_cgm->canvas, style);

  width = get_size_mode(cd_cgm->canvas, width, mode, cdCanvasLineWidth(cd_cgm->canvas, CD_QUERY));

  cdCanvasLineWidth(cd_cgm->canvas, sMin1(sRound(width)));

  cdCanvasForeground(cd_cgm->canvas, cdEncodeColor(color.red, color.green, color.blue));
}

static void cdcgm_MarkerAttrib(const char *type, double size, const char *mode, cgmRGB color, cdCGM* cd_cgm)
{
  int style = CD_PLUS;
  if (strcmp(type, "DOT")==0)
    style = CD_BOX;  /* actually a pixel */
  else if (strcmp(type, "PLUS")==0)
    style = CD_PLUS;
  else if (strcmp(type, "ASTERISK")==0)
    style = CD_STAR;
  else if (strcmp(type, "CIRCLE")==0)
    style = CD_HOLLOW_CIRCLE;
  else if (strcmp(type, "CROSS")==0)
    style = CD_X;

  cdCanvasMarkType(cd_cgm->canvas, style);

  size = get_size_mode(cd_cgm->canvas, size, mode, cdCanvasMarkSize(cd_cgm->canvas, CD_QUERY));

  cdCanvasMarkSize(cd_cgm->canvas, sMin1(sRound(size)));

  cdCanvasForeground(cd_cgm->canvas, cdEncodeColor(color.red, color.green, color.blue));
}

static void cdcgm_FillAttrib(const char* type, cgmRGB color, const char* hatch, cgmPattern* pat, cdCGM* cd_cgm)
{
  int style = CD_SOLID;
  if (strcmp(type, "HOLLOW")==0)
    style = CD_HOLLOW;
  else if (strcmp(type, "SOLID")==0)
    style = CD_SOLID;
  else if (strcmp(type, "HATCH")==0)
    style = CD_HATCH;
  else if (strcmp(type, "PATTERN")==0)
    style = CD_PATTERN;

  cdCanvasInteriorStyle(cd_cgm->canvas, style);

  cdCanvasForeground(cd_cgm->canvas, cdEncodeColor(color.red, color.green, color.blue));

  if (style == CD_HATCH)
  {
    style = CD_HORIZONTAL;
    if (strcmp(hatch, "HORIZONTAL")==0)
      style = CD_HORIZONTAL;
    else if (strcmp(hatch, "VERTICAL")==0)
      style = CD_VERTICAL;
    else if (strcmp(hatch, "POSITIVE_SLOPE")==0)
      style = CD_BDIAGONAL;
    else if (strcmp(hatch, "NEGATIVE_SLOPE")==0)
      style = CD_FDIAGONAL;
    else if (strcmp(hatch, "HV_CROSS")==0)
      style = CD_CROSS;
    else if (strcmp(hatch, "SLOPE_CROSS")==0)
      style = CD_DIAGCROSS;

    cdCanvasHatch(cd_cgm->canvas, style);
  }
  else if (style == CD_PATTERN)
  {
    int i, count = pat->w*pat->h;
    long int *pattern = (long int *) malloc(count*sizeof(long int));
    
    for (i=0; i<count; i++)
      pattern[i] = cdEncodeColor(pat->pattern[i].red, pat->pattern[i].green, pat->pattern[i].blue);

    cdCanvasPattern(cd_cgm->canvas, pat->w, pat->h, pattern);

    free(pattern);
  }
}

static int cdcgm_Counter(double percent, cdCGM* cd_cgm)
{
  if (cdcgmcountercb)
  {
    int ret = cdcgmcountercb(cd_cgm->canvas, percent);
    if (ret == CD_ABORT)
      return CGM_ABORT_COUNTER;
  }

  if (cd_cgm->abort)
    return CGM_ABORT_COUNTER;

  return CGM_OK; 
}

int cdplayCGM(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  cgmPlayFuncs funcs;
  cdCGM cd_cgm;
  int ret;

  cd_cgm.canvas = canvas;
  cd_cgm.xmin = xmin;
  cd_cgm.xmax = xmax;
  cd_cgm.ymin = ymin;
  cd_cgm.ymax = ymax;
  cd_cgm.abort = 0;
  cd_cgm.first_pic = 1;
  cd_cgm.drawing_metric = 0;

  funcs.BeginMetafile = cdcgm_BeginMetafile; 
  funcs.EndMetafile = NULL;
  funcs.BeginPicture = cdcgm_BeginPicture; 
  funcs.EndPicture = NULL;
  funcs.BeginPictureBody = cdcgm_BeginPictureBody;
  funcs.DeviceExtent = cdcgm_DeviceExtent; 
  funcs.ScaleMode = cdcgm_ScaleMode; 
  funcs.BackgroundColor = cdcgm_BackgroundColor; 
  funcs.Transparency = cdcgm_Transparency; 
  funcs.ClipRectangle = cdcgm_ClipRectangle; 
  funcs.ClipIndicator = cdcgm_ClipIndicator; 
  funcs.PolyLine = cdcgm_PolyLine; 
  funcs.PolyMarker = cdcgm_PolyMarker; 
  funcs.Polygon = cdcgm_Polygon; 
  funcs.Text = cdcgm_Text;
  funcs.CellArray = cdcgm_CellArray; 
  funcs.Rectangle = cdcgm_Rectangle;
  funcs.Circle = cdcgm_Circle; 
  funcs.CircularArc = cdcgm_CircularArc; 
  funcs.Ellipse = cdcgm_Ellipse; 
  funcs.EllipticalArc = cdcgm_EllipticalArc; 
  funcs.LineAttrib = cdcgm_LineAttrib; 
  funcs.MarkerAttrib = cdcgm_MarkerAttrib; 
  funcs.FillAttrib = cdcgm_FillAttrib; 
  funcs.TextAttrib = cdcgm_TextAttrib; 
  funcs.Counter = cdcgm_Counter;

  ret = cgmPlay((char*)data, (void*)&cd_cgm, &funcs);
  if (ret == CGM_OK)
    return CD_OK;
  else if (ret == CGM_ABORT_COUNTER)
    return CD_ABORT;
  else
    return CD_ERROR;
}
