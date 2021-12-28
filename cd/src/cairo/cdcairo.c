/** \file
* \brief Cairo Base Driver
*
* See Copyright Notice in cd.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include <glib.h>
#include <pango/pangocairo.h>

#include "cdcairoctx.h"


#ifndef PANGO_VERSION_CHECK
#define PANGO_VERSION_CHECK(x,y,z) (0)
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static void sfCairoRectangle(cairo_t *cr, double xmin, double ymin, double xmax, double ymax)
{
  /* cairo_rectangle was not including the last line and last column */
  cairo_move_to(cr, xmin, ymin);
  cairo_line_to(cr, xmax, ymin);
  cairo_line_to(cr, xmax, ymax);
  cairo_line_to(cr, xmin, ymax);
  cairo_close_path(cr);
}

static void sCairoRectangleWH(cairo_t *cr, int x, int y, int w, int h)
{
  cairo_rectangle(cr, x, y, w, h);
  /* TODO: should we replace cairo_rectangle here too?
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x + w, y);
  cairo_line_to(cr, x + w, y + h);
  cairo_line_to(cr, x, y + h);
  cairo_close_path(cr); */
}

static void sUpdateFill(cdCtxCanvas *ctxcanvas, int fill)
{
  if (fill == 0 || ctxcanvas->canvas->interior_style == CD_SOLID)
  {
    if (ctxcanvas->last_source == 0)
      return;

    cairo_set_source(ctxcanvas->cr, ctxcanvas->solid);
    ctxcanvas->last_source = 0;
  }
  else
  {
    if (ctxcanvas->last_source == 1)
      return;

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
  }
}

/* This is a copy of the cdgdk.c function */
static char* cdgStrToSystem(const char* str, int *len, cdCtxCanvas *ctxcanvas)
{
  if (!str || *str == 0)
    return (char*)str;

  if (!ctxcanvas->utf8mode)  
  {
    const char *charset = NULL;
    if (g_get_charset(&charset)==TRUE)  /* current locale is already UTF-8 */
    {
      if (g_utf8_validate(str, *len, NULL))
        return (char*)str;
      else
      {
        if (ctxcanvas->utf8_buffer)
          g_free(ctxcanvas->utf8_buffer);
        ctxcanvas->utf8_buffer = g_convert(str, *len, "UTF-8", "ISO8859-1", NULL, NULL, NULL);   /* if string is not UTF-8, assume ISO8859-1 */
        if (!ctxcanvas->utf8_buffer) return (char*)str;
        *len = (int)strlen(ctxcanvas->utf8_buffer);
        return ctxcanvas->utf8_buffer;
      }
    }
    else
    {
      if (cdStrIsAscii(str) || !charset)
        return (char*)str;
      else if (charset)
      {
        if (ctxcanvas->utf8_buffer)
          g_free(ctxcanvas->utf8_buffer);
        ctxcanvas->utf8_buffer = g_convert(str, *len, "UTF-8", charset, NULL, NULL, NULL);
        if (!ctxcanvas->utf8_buffer) return (char*)str;
        *len = (int)strlen(ctxcanvas->utf8_buffer);
        return ctxcanvas->utf8_buffer;
      }
    }
  }
  return (char*)str;
}

void cdcairoKillCanvas(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->solid)
    cairo_pattern_destroy(ctxcanvas->solid);

  if (ctxcanvas->pattern)
    cairo_pattern_destroy(ctxcanvas->pattern);

#if CAIRO_VERSION >= CAIRO_VERSION_110
  if (ctxcanvas->new_rgn) cairo_region_destroy(ctxcanvas->new_rgn);
#endif

  if (ctxcanvas->fontdesc) pango_font_description_free(ctxcanvas->fontdesc);
  if (ctxcanvas->fontlayout)  g_object_unref(ctxcanvas->fontlayout);
  if (ctxcanvas->fontcontext) g_object_unref(ctxcanvas->fontcontext);

  if (ctxcanvas->utf8_buffer)
    g_free(ctxcanvas->utf8_buffer);

  if (ctxcanvas->cr)
    cairo_destroy(ctxcanvas->cr);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

/******************************************************/

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  cairo_surface_flush(cairo_get_target(ctxcanvas->cr));
  cairo_show_page(ctxcanvas->cr);
}

/******************************************************/

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->clip_mode != CD_CLIPAREA)
    return;

  cairo_reset_clip(ctxcanvas->cr);
  sfCairoRectangle(ctxcanvas->cr, xmin, ymin, xmax, ymax);
  cairo_clip(ctxcanvas->cr);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  switch (mode)
  {
  case CD_CLIPOFF:
    cairo_reset_clip(ctxcanvas->cr);
    break;
  case CD_CLIPAREA:
    cairo_reset_clip(ctxcanvas->cr);
    sfCairoRectangle(ctxcanvas->cr, ctxcanvas->canvas->clip_frect.xmin,
                                    ctxcanvas->canvas->clip_frect.ymin, 
                                    ctxcanvas->canvas->clip_frect.xmax, 
                                    ctxcanvas->canvas->clip_frect.ymax);
    cairo_clip(ctxcanvas->cr);
    break;
  case CD_CLIPPOLYGON:
    {
      int hole_index = 0;
      int i;

      cairo_reset_clip(ctxcanvas->cr);

      if (ctxcanvas->canvas->clip_poly)
      {
        cdPoint *poly = ctxcanvas->canvas->clip_poly; 
        cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);
        for (i=1; i<ctxcanvas->canvas->clip_poly_n; i++)
        {
          if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
          {
            cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
            hole_index++;
          }
          else
            cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        }
      }
      else if (ctxcanvas->canvas->clip_fpoly)
      {
        cdfPoint *poly = ctxcanvas->canvas->clip_fpoly; 
        cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);
        for (i=1; i<ctxcanvas->canvas->clip_poly_n; i++)
        {
          if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
          {
            cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
            hole_index++;
          }
          else
            cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        }
      }

      cairo_clip(ctxcanvas->cr);
      break;
    }
  case CD_CLIPREGION:
    /* if (ctxcanvas->new_rgn)
      cairo_region(ctxcanvas->cr, ctxcanvas->new_rgn); */ /* Does NOT exist. */
    break;
  }

  return mode;
}

/******************************************************/

#define CD_ALPHAPRE(_src, _alpha) (((_src)*(_alpha))/255)

static unsigned int sEncodeRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  /* CAIRO_FORMAT_ARGB32 each pixel is a 32-bit quantity, with alpha in the upper 8 bits, then red, then green, then blue. 
     The 32-bit quantities are stored native-endian. 
     Pre-multiplied alpha is used. (That is, 50% transparent red is 0x80800000, not 0x80ff0000.) */
  if (a != 255)
  {
    r = CD_ALPHAPRE(r, a);
    g = CD_ALPHAPRE(g, a);
    b = CD_ALPHAPRE(b, a);
  }

  return (((unsigned int)a) << 24) |
         (((unsigned int)r) << 16) |
         (((unsigned int)g) <<  8) |
         (((unsigned int)b) <<  0);
}

static void make_pattern(cdCtxCanvas *ctxcanvas, int n, int m, void* userdata, int (*data2rgba)(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* userdata, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a))
{
  int i, j, offset, ret, stride;
  unsigned char r, g, b, a;
  cairo_surface_t* pattern_surface;
  unsigned int* data;
  cairo_pattern_t *pattern;

  pattern_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, n, m);
  if (cairo_surface_status(pattern_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(pattern_surface);
    return;
  }

  cairo_surface_flush(pattern_surface);
  data = (unsigned int*)cairo_image_surface_get_data(pattern_surface);
  stride = cairo_image_surface_get_stride(pattern_surface);
  offset = stride/4 - n;

  for (j = 0; j < m; j++)
  {
    for (i = 0; i < n; i++)
    {
      /* internal transform, affects also pattern orientation */
      if (ctxcanvas->canvas->invert_yaxis)
        ret = data2rgba(ctxcanvas, n, i, m-1-j, userdata, &r, &g, &b, &a);
      else
        ret = data2rgba(ctxcanvas, n, i, j, userdata, &r, &g, &b, &a);

      if (ret == -1)
      {
        data++;  /* already transparent */
        continue;
      }

      *data++ = sEncodeRGBA(r, g, b, a);
    }

    if (offset)
      data += offset;
  }

  cairo_surface_mark_dirty(pattern_surface);

  pattern = cairo_pattern_create_for_surface(pattern_surface);
  if (cairo_pattern_status(pattern) == CAIRO_STATUS_SUCCESS)
  {
    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = pattern;
    cairo_pattern_reference(ctxcanvas->pattern);
    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);
  }

  cairo_surface_destroy(pattern_surface);
}

static int long2rgba(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a)
{
  long* long_data = (long*)data;
  long c = long_data[j*n+i];
  (void)ctxcanvas;
  cdDecodeColor(c, r, g, b);
  *a = cdDecodeAlpha(c);
  return 1;
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long *pattern)
{
  make_pattern(ctxcanvas, n, m, (void*)pattern, long2rgba);
  cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
  ctxcanvas->last_source = 1;
}

static int uchar2rgba(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a)
{
  unsigned char* uchar_data = (unsigned char*)data;
  if (uchar_data[j*n+i])
  {
    cdDecodeColor(ctxcanvas->canvas->foreground, r, g, b);
    *a = cdDecodeAlpha(ctxcanvas->canvas->foreground);
  }
  else
  {
    if (ctxcanvas->canvas->back_opacity == CD_TRANSPARENT)
      return -1; /* fully transparent */
    else
    {
      cdDecodeColor(ctxcanvas->canvas->background, r, g, b);
      *a = cdDecodeAlpha(ctxcanvas->canvas->background);
    }
  }

  return 1;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  make_pattern(ctxcanvas, n, m, (void*)stipple, uchar2rgba);
  cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
  ctxcanvas->last_source = 1;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  int hsize = ctxcanvas->hatchboxsize;
  int hhalf = hsize / 2;
  cairo_surface_t* hatch_surface;
  cairo_t* cr;

  hatch_surface = cairo_surface_create_similar(cairo_get_target(ctxcanvas->cr), CAIRO_CONTENT_COLOR_ALPHA, hsize, hsize);
  cr = cairo_create(hatch_surface);

  if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
  {
    cairo_set_source_rgba(cr, cdCairoGetRed(ctxcanvas->canvas->background), cdCairoGetGreen(ctxcanvas->canvas->background), cdCairoGetBlue(ctxcanvas->canvas->background), cdCairoGetAlpha(ctxcanvas->canvas->background));
    sCairoRectangleWH(cr, 0, 0, hsize, hsize);
    cairo_fill(cr);
  }

  cairo_set_source_rgba(cr, cdCairoGetRed(ctxcanvas->canvas->foreground), cdCairoGetGreen(ctxcanvas->canvas->foreground), cdCairoGetBlue(ctxcanvas->canvas->foreground), cdCairoGetAlpha(ctxcanvas->canvas->foreground));

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); 
  cairo_set_line_width(cr, 1);

  switch(style)
  {
  case CD_HORIZONTAL:
    cairo_move_to(cr, 0.0, (double)hhalf);
    cairo_line_to(cr, (double)hsize, (double)hhalf);
    break;
  case CD_VERTICAL:
    cairo_move_to(cr, (double)hhalf, 0.0);
    cairo_line_to(cr, (double)hhalf, (double)hsize);
    break;
  case CD_BDIAGONAL:
    cairo_move_to(cr, 0.0, (double)hsize);
    cairo_line_to(cr, (double)hsize, 0.0);
    break;
  case CD_FDIAGONAL:
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, (double)hsize, (double)hsize);
    break;
  case CD_CROSS:
    cairo_move_to(cr, (double)hsize, 0.0);
    cairo_line_to(cr, (double)hsize, (double)hsize);
    cairo_move_to(cr, 0.0, (double)hhalf);
    cairo_line_to(cr, (double)hsize, (double)hhalf);
    break;
  case CD_DIAGCROSS:
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, (double)hsize, (double)hsize);
    cairo_move_to(cr, (double)hsize, 0.0);
    cairo_line_to(cr, 0.0, (double)hsize);
    break;
  }

  cairo_stroke(cr);

  if (ctxcanvas->pattern)
    cairo_pattern_destroy(ctxcanvas->pattern);

  ctxcanvas->pattern = cairo_pattern_create_for_surface(hatch_surface);
  cairo_pattern_reference(ctxcanvas->pattern);
  cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

  cairo_surface_destroy(hatch_surface);
  cairo_destroy(cr);

  cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
  ctxcanvas->last_source = 1;

  return style;
}

/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdinteriorstyle (cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
  case CD_SOLID:
    cairo_set_source(ctxcanvas->cr, ctxcanvas->solid);
    ctxcanvas->last_source = 0;
    break;
  /* must recreate the current pattern */
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

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  double dashes[10];

  switch (style)
  {
  case CD_CONTINUOUS : /* empty dash */
    cairo_set_dash(ctxcanvas->cr, 0, 0, 0);
    break;
  case CD_DASHED :
    dashes[0] = 6.0;  dashes[1] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 2, 0);
    break;
  case CD_DOTTED :
    dashes[0] = 2.0;  dashes[1] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 2, 0);
    break;
  case CD_DASH_DOT :
    dashes[0] = 6.0;  dashes[1] = 2.0;
    dashes[2] = 2.0;  dashes[3] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 4, 0);
    break;
  case CD_DASH_DOT_DOT :
    dashes[0] = 6.0;  dashes[1] = 2.0;
    dashes[2] = 2.0;  dashes[3] = 2.0;
    dashes[4] = 2.0;  dashes[5] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 6, 0);
    break;
  case CD_CUSTOM :
    {
      int i;
      double* dash_style = (double*)malloc(sizeof(double)*ctxcanvas->canvas->line_dashes_count);

      for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
        dash_style[i] = (double)ctxcanvas->canvas->line_dashes[i];

      cairo_set_dash(ctxcanvas->cr, dash_style, ctxcanvas->canvas->line_dashes_count, 0);

      free(dash_style);
    }
    break;
  }

  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  if(width == 0)
    width = 1;

  cairo_set_line_width(ctxcanvas->cr, (double)width);

  return width;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  int cd2ps_join[] = {CAIRO_LINE_JOIN_MITER, CAIRO_LINE_JOIN_BEVEL, CAIRO_LINE_JOIN_ROUND};

  cairo_set_line_join(ctxcanvas->cr, cd2ps_join[join]); 

  return join;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  int cd2pdf_cap[] = {CAIRO_LINE_CAP_BUTT, CAIRO_LINE_CAP_SQUARE, CAIRO_LINE_CAP_ROUND};

  cairo_set_line_cap(ctxcanvas->cr, cd2pdf_cap[cap]); 

  return cap;
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  int is_italic = 0, is_bold = 0;   /* default is CD_PLAIN */
  int is_strikeout = 0, is_underline = 0;
  char font[256];
  PangoAttrList *attrs;

  if (cdStrEqualNoCase(type_face, "Courier") || cdStrEqualNoCase(type_face, "Courier New"))
    type_face = "Monospace";
  else if (cdStrEqualNoCase(type_face, "Times") || cdStrEqualNoCase(type_face, "Times New Roman"))
    type_face = "Serif";
  else if (cdStrEqualNoCase(type_face, "Helvetica") || cdStrEqualNoCase(type_face, "Arial"))
    type_face = "Sans";

  if (style & CD_BOLD)
    is_bold = 1;

  if (style & CD_ITALIC)
    is_italic = 1;

  if (style & CD_UNDERLINE)
    is_underline = 1;

  if (style & CD_STRIKEOUT)
    is_strikeout = 1;

  size = cdGetFontSizePoints(ctxcanvas->canvas, size);

  if (ctxcanvas->scale_points)
    size = (int)(size / ctxcanvas->scale);

  sprintf(font, "%s, %s%s%d", type_face, is_bold?"Bold ":"", is_italic?"Italic ":"", size);

  if (ctxcanvas->fontdesc) 
    pango_font_description_free(ctxcanvas->fontdesc);

  ctxcanvas->fontdesc = pango_font_description_from_string(font);

  if (!ctxcanvas->fontdesc)
    return 0;

  if (ctxcanvas->fontlayout)  
    g_object_unref(ctxcanvas->fontlayout);

  ctxcanvas->fontlayout = pango_layout_new(ctxcanvas->fontcontext);
  pango_layout_set_font_description(ctxcanvas->fontlayout, ctxcanvas->fontdesc);

  attrs = pango_attr_list_new();
  pango_attr_list_insert(attrs, pango_attribute_copy(pango_attr_strikethrough_new(is_strikeout ? TRUE : FALSE)));
  pango_attr_list_insert(attrs, pango_attribute_copy(pango_attr_underline_new(is_underline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE)));
  pango_layout_set_attributes(ctxcanvas->fontlayout, attrs);

  pango_attr_list_unref(attrs);

  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  return 1;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  PangoFontMetrics* metrics;
  int charwidth, charheight, charascent, chardescent;

  if (!ctxcanvas->fontdesc)
    return;

  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);
  metrics = pango_context_get_metrics(ctxcanvas->fontcontext, ctxcanvas->fontdesc, pango_context_get_language(ctxcanvas->fontcontext));
  charascent  = pango_font_metrics_get_ascent(metrics);
  chardescent = pango_font_metrics_get_descent(metrics);
  charheight  = charascent + chardescent;
  charwidth   = pango_font_metrics_get_approximate_char_width(metrics);

  if (max_width) *max_width = (((charwidth)   + PANGO_SCALE/2) / PANGO_SCALE);
  if (height)    *height    = (((charheight)  + PANGO_SCALE/2) / PANGO_SCALE);
  if (ascent)    *ascent    = (((charascent)  + PANGO_SCALE/2) / PANGO_SCALE);
  if (descent)   *descent   = (((chardescent) + PANGO_SCALE/2) / PANGO_SCALE);

  pango_font_metrics_unref(metrics); 
}

static long cdforeground(cdCtxCanvas *ctxcanvas, long color)
{
  if (ctxcanvas->solid)
    cairo_pattern_destroy(ctxcanvas->solid);

  cairo_set_source_rgba(ctxcanvas->cr, cdCairoGetRed(color),
                                       cdCairoGetGreen(color),
                                       cdCairoGetBlue(color),
                                       cdCairoGetAlpha(color));
  ctxcanvas->solid = cairo_get_source(ctxcanvas->cr);
  cairo_pattern_reference(ctxcanvas->solid);
  ctxcanvas->last_source = 0;
  return color;
}


/******************************************************/

static void sSetTransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    cairo_matrix_t mtx;

    /* configure a bottom-up coordinate system */
    mtx.xx = 1; mtx.yx = 0;
    mtx.xy = 0; mtx.yy = -1;
    mtx.x0 = 0; mtx.y0 = (ctxcanvas->canvas->h-1);
    cairo_transform(ctxcanvas->cr, &mtx);

    mtx.xx = matrix[0]; mtx.yx = matrix[1];
    mtx.xy = matrix[2]; mtx.yy = matrix[3];
    mtx.x0 = matrix[4]; mtx.y0 = matrix[5];
    cairo_transform(ctxcanvas->cr, &mtx);
  }
  else if (ctxcanvas->rotate_angle)
  {
    /* rotation = translate to point + rotation + translate back */
    /* the rotation must be corrected because of the Y axis orientation */
    cairo_translate(ctxcanvas->cr, ctxcanvas->rotate_center_x, _cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y));
    cairo_rotate(ctxcanvas->cr, -ctxcanvas->rotate_angle * CD_DEG2RAD);
    cairo_translate(ctxcanvas->cr, -ctxcanvas->rotate_center_x, -_cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y));
  }
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  cairo_save (ctxcanvas->cr);

  cairo_identity_matrix(ctxcanvas->cr);
  cairo_reset_clip(ctxcanvas->cr);

  sCairoRectangleWH(ctxcanvas->cr, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  cairo_clip(ctxcanvas->cr);
  cairo_set_source_rgba(ctxcanvas->cr, cdCairoGetRed(ctxcanvas->canvas->background), cdCairoGetGreen(ctxcanvas->canvas->background), cdCairoGetBlue(ctxcanvas->canvas->background), 1.0); /* clear is opaque */
  cairo_set_operator (ctxcanvas->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctxcanvas->cr);  /* paints the current source everywhere within the current clip region. */

  cairo_restore (ctxcanvas->cr);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{ 
  sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, x1, y1);
  cairo_line_to(ctxcanvas->cr, x2, y2);
  cairo_stroke(ctxcanvas->cr);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  /* try to draw single row of pixels at full intensity */
  if (ctxcanvas->canvas->line_width == 1 && !ctxcanvas->canvas->use_matrix)
  {
    if (x1==x2)
    {
      if (y1>y2)
        cdfline(ctxcanvas, (double)x1+0.5, (double)y1+1, (double)x2+0.5, (double)y2);
      else
        cdfline(ctxcanvas, (double)x1+0.5, (double)y1, (double)x2+0.5, (double)y2+1);
      return;
    }
    else if (y1==y2)
    {
      if (x1>x2)
        cdfline(ctxcanvas, (double)x1+1, (double)y1+0.5, (double)x2, (double)y2+0.5);
      else
        cdfline(ctxcanvas, (double)x1, (double)y1+0.5, (double)x2+1, (double)y2+0.5);
      return;
    }
  }

  cdfline(ctxcanvas, (double)x1, (double)y1, (double)x2, (double)y2);
}

static void sFixAngles(cdCanvas* canvas, double *a1, double *a2, int swap)
{
  /* Cairo angles are clock-wise by default, in radians */

  /* if NOT inverted means a transformation is set, 
     so the angle will follow the transformation that includes the axis inversion,
     then it is already counter-clockwise */

  if (canvas->invert_yaxis)
  {
    /* change orientation */
    *a1 *= -1;
    *a2 *= -1;

    /* swap, so the start angle is the smaller */
    if (swap)
    {
      double t = *a1;
      *a1 = *a2;
      *a2 = t;
    }
  }

  /* convert to radians */
  *a1 *= CD_DEG2RAD;
  *a2 *= CD_DEG2RAD;
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 0);

  sFixAngles(ctxcanvas->canvas, &a1, &a2, 1);

  if (w == h)
  {
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
    cairo_stroke(ctxcanvas->cr);
  }
  else  /* Ellipse: change the scale to create from the circle */
  {
    cairo_save(ctxcanvas->cr);  /* save to use the local transform */

    cairo_translate(ctxcanvas->cr, xc, yc);
    cairo_scale(ctxcanvas->cr, w/h, 1.0);
    cairo_translate(ctxcanvas->cr, -xc, -yc);

    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
    cairo_stroke(ctxcanvas->cr);

    cairo_restore(ctxcanvas->cr);  /* restore from local */
  }
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  sFixAngles(ctxcanvas->canvas, &a1, &a2, 1);

  if (w == h)
  {
    cairo_move_to(ctxcanvas->cr, xc, yc);
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
    cairo_fill(ctxcanvas->cr);
  }
  else  /* Ellipse: change the scale to create from the circle */
  {
    cairo_save(ctxcanvas->cr);  /* save to use the local transform */

    cairo_translate(ctxcanvas->cr, xc, yc);
    cairo_scale(ctxcanvas->cr, w/h, 1.0);
    cairo_translate(ctxcanvas->cr, -xc, -yc);

    cairo_move_to(ctxcanvas->cr, xc, yc);
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);

    cairo_fill(ctxcanvas->cr);

    cairo_restore(ctxcanvas->cr);  /* restore from local */
  }
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  sFixAngles(ctxcanvas->canvas, &a1, &a2, 1);

  if (w == h)
  {
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
    cairo_fill(ctxcanvas->cr);
  }
  else  /* Ellipse: change the scale to create from the circle */
  {
    cairo_save(ctxcanvas->cr);  /* save to use the local transform */

    /* local transform */
    cairo_translate(ctxcanvas->cr, xc, yc);
    cairo_scale(ctxcanvas->cr, w/h, 1.0);
    cairo_translate(ctxcanvas->cr, -xc, -yc);

    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
    cairo_fill(ctxcanvas->cr);

    cairo_restore(ctxcanvas->cr);  /* restore from local */
  }
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfchord(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 0);
  sfCairoRectangle(ctxcanvas->cr, xmin, ymin, xmax, ymax);
  cairo_stroke(ctxcanvas->cr);
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

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 1);
  sfCairoRectangle(ctxcanvas->cr, xmin, ymin, xmax, ymax);
  cairo_fill(ctxcanvas->cr);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->new_region)
  {
#if CAIRO_VERSION >= CAIRO_VERSION_110
    cairo_rectangle_int_t rect;

    rect.x = xmin;
    rect.width = xmax - xmin + 1;
    rect.y = ymin;
    rect.height = ymax - ymin + 1;

    switch (ctxcanvas->canvas->combine_mode)
    {
    case CD_UNION:
      cairo_region_union_rectangle(ctxcanvas->new_rgn, &rect);
      break;
    case CD_INTERSECT:
      cairo_region_intersect_rectangle(ctxcanvas->new_rgn, &rect);
      break;
    case CD_DIFFERENCE:
      cairo_region_subtract_rectangle(ctxcanvas->new_rgn, &rect);
      break;
    case CD_NOTINTERSECT:
      cairo_region_xor_rectangle(ctxcanvas->new_rgn, &rect);
      break;
    }
#endif
  }
  else
  {
    /* try to include the last integer line */
    if (!ctxcanvas->canvas->use_matrix)
      cdfbox(ctxcanvas, (double)xmin, (double)xmax + 0.75, (double)ymin, (double)ymax + 0.75);
    else
      cdfbox(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
  }
}

static void sGetTransformTextHeight(cdCanvas* canvas, double x, double y, int w, int h, double *hbox)
{
  double xmin, xmax, ymin, ymax;
  int baseline, height, ascent;

  /* distance from bottom to baseline */
  cdgetfontdim(canvas->ctxcanvas, NULL, &height, &ascent, NULL);
  baseline = height - ascent; 

  /* move to bottom-left */
  cdfTextTranslatePoint(canvas, x, y, w, h, baseline, &xmin, &ymin);

  xmax = xmin + w-1;
  ymax = ymin + h-1;

  if (canvas->text_orientation)
  {
    double angle = canvas->text_orientation*CD_DEG2RAD;
    double cos_theta = cos(angle);
    double sin_theta = sin(angle);
    double rectY[4];

    cdfRotatePointY(canvas, xmin, ymin, x, y, &rectY[0], sin_theta, cos_theta);
    cdfRotatePointY(canvas, xmax, ymin, x, y, &rectY[1], sin_theta, cos_theta);
    cdfRotatePointY(canvas, xmax, ymax, x, y, &rectY[2], sin_theta, cos_theta);
    cdfRotatePointY(canvas, xmin, ymax, x, y, &rectY[3], sin_theta, cos_theta);

    ymin = ymax = rectY[0];
    if (rectY[1] < ymin) ymin = rectY[1];
    if (rectY[2] < ymin) ymin = rectY[2];
    if (rectY[3] < ymin) ymin = rectY[3];
    if (rectY[1] > ymax) ymax = rectY[1];
    if (rectY[2] > ymax) ymax = rectY[2];
    if (rectY[3] > ymax) ymax = rectY[3];
  }

  *hbox = ymax-ymin+1;
}

static void sSetTextTransform(cdCtxCanvas* ctxcanvas, double *x, double *y, int w, int h)
{
  double hbox;
  cairo_matrix_t mtx;

  sGetTransformTextHeight(ctxcanvas->canvas, *x, *y, w, h, &hbox);

  /* move to (x,y) and remove a vertical offset since text reference point is top-left */
  mtx.xx = 1; mtx.yx = 0;
  mtx.xy = 0; mtx.yy = 1;
  mtx.x0 = *x; mtx.y0 = *y - (hbox-1);
  cairo_transform(ctxcanvas->cr, &mtx);

  /* invert the text vertical orientation, relative to itself */
  mtx.xx = 1; mtx.yx = 0;
  mtx.xy = 0; mtx.yy = -1;
  mtx.x0 = 0; mtx.y0 = hbox-1;
  cairo_transform(ctxcanvas->cr, &mtx);

  *x = 0;
  *y = 0;
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  PangoFontMetrics* metrics;
  int w, h, desc, dir = -1, reset_transform = 0;

  s = cdgStrToSystem(s, &len, ctxcanvas);
  pango_layout_set_text(ctxcanvas->fontlayout, s, len);
  
	pango_layout_get_pixel_size(ctxcanvas->fontlayout, &w, &h);
  metrics = pango_context_get_metrics(ctxcanvas->fontcontext, ctxcanvas->fontdesc, pango_context_get_language(ctxcanvas->fontcontext));
  desc = (((pango_font_metrics_get_descent(metrics)) + PANGO_SCALE/2) / PANGO_SCALE);

  if (ctxcanvas->canvas->text_orientation || 
      ctxcanvas->canvas->use_matrix ||
      ctxcanvas->rotate_angle)
    reset_transform = 1;

  if (reset_transform)
  {
    cairo_save (ctxcanvas->cr);
    cairo_identity_matrix(ctxcanvas->cr);

    if (ctxcanvas->scale_points)
      cairo_scale(ctxcanvas->cr, ctxcanvas->scale, ctxcanvas->scale);
  }

  if (ctxcanvas->canvas->text_orientation)
  {
    double angle = ctxcanvas->canvas->text_orientation*CD_DEG2RAD;
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_rotate(ctxcanvas->cr, -angle);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

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
      x = x - w/2;
      break;
    case CD_BASE_LEFT:
    case CD_NORTH_WEST:
    case CD_WEST:
    case CD_SOUTH_WEST:
      x = x;
      break;
  }

  if (ctxcanvas->canvas->invert_yaxis)
    dir = 1;

  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_BASE_LEFT:
    case CD_BASE_CENTER:
    case CD_BASE_RIGHT:
      y = y - (dir*h - desc);
      break;
    case CD_SOUTH_EAST:
    case CD_SOUTH_WEST:
    case CD_SOUTH:
      y = y - (dir*h);
      break;
    case CD_NORTH_EAST:
    case CD_NORTH:
    case CD_NORTH_WEST:
      y = y;
      break;
    case CD_CENTER:
    case CD_EAST:
    case CD_WEST:
      y = y - (dir*(h/2));
      break;
  }

  if (ctxcanvas->canvas->use_matrix)
  {
    double* matrix = ctxcanvas->canvas->matrix;
    sSetTransform(ctxcanvas, matrix);
    sSetTextTransform(ctxcanvas, &x, &y, w, h);
  }
  else 
    sSetTransform(ctxcanvas, NULL);

  /* Inform Pango to re-layout the text with the new transformation */
  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, x, y);
  pango_cairo_show_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  if (reset_transform)
    cairo_restore(ctxcanvas->cr);

  pango_font_metrics_unref(metrics); 
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, s, len);
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  if (!ctxcanvas->fontlayout)
    return;

  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  s = cdgStrToSystem(s, &len, ctxcanvas);
  pango_layout_set_text(ctxcanvas->fontlayout, s, len);
  pango_layout_get_pixel_size(ctxcanvas->fontlayout, width, height);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
    return;

  if (mode == CD_PATH)
  {
    int p;

    /* if there is any current path, remove it */
    cairo_new_path(ctxcanvas->cr);

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        cairo_new_path(ctxcanvas->cr);
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;

          if (i+3 > n) return;

          if (!cdGetArcPathF(poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          sFixAngles(ctxcanvas->canvas, &a1, &a2, 0);  /* do not swap because we handle negative arcs here */

          if (w == h)
          {
            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
          }
          else  /* Ellipse: change the scale to create from the circle */
          {
            cairo_save(ctxcanvas->cr);  /* save to use the local transform */

            cairo_translate(ctxcanvas->cr, xc, yc);
            cairo_scale(ctxcanvas->cr, w/h, 1.0);
            cairo_translate(ctxcanvas->cr, -xc, -yc);

            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);

            cairo_restore(ctxcanvas->cr);  /* restore from local */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        cairo_close_path(ctxcanvas->cr);
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill(ctxcanvas->cr);
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill_preserve(ctxcanvas->cr);
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_CLIP:
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_clip(ctxcanvas->cr);
        ctxcanvas->canvas->clip_mode = CD_CLIPPATH;
        break;
      }
    }
    return;
  }

  if (mode == CD_FILL)
  {
    sUpdateFill(ctxcanvas, 1);

    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_EVEN_ODD);
    else
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_WINDING);
  }
  else
    sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
      cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    cairo_close_path(ctxcanvas->cr);
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_OPEN_LINES :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_BEZIER :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_FILL :
    cairo_fill(ctxcanvas->cr);
    break;
  }
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
    return;

  if (mode == CD_PATH)
  {
    int p;

    /* if there is any current path, remove it */
    cairo_new_path(ctxcanvas->cr);

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        cairo_new_path(ctxcanvas->cr);
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;

          if (i+3 > n) return;

          if (!cdfGetArcPath(poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          sFixAngles(ctxcanvas->canvas, &a1, &a2, 0);  /* do not swap because we handle negative arcs here */

          if (w == h)
          {
            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
          }
          else  /* Ellipse: change the scale to create from the circle */
          {
            cairo_save(ctxcanvas->cr);  /* save to use the local transform */

            cairo_translate(ctxcanvas->cr, xc, yc);
            cairo_scale(ctxcanvas->cr, w/h, 1.0);
            cairo_translate(ctxcanvas->cr, -xc, -yc);

            /* cairo_arc already includes an initial line segment */

            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);

            cairo_restore(ctxcanvas->cr);  /* restore from local */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        cairo_close_path(ctxcanvas->cr);
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill(ctxcanvas->cr);
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill_preserve(ctxcanvas->cr);
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_CLIP:
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_clip(ctxcanvas->cr);
        ctxcanvas->canvas->clip_mode = CD_CLIPPATH;
        break;
      }
    }
    return;
  }

  if (mode == CD_FILL)
  {
    sUpdateFill(ctxcanvas, 1);

    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_EVEN_ODD);
    else
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_WINDING);
  }
  else
    sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
      cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    cairo_close_path(ctxcanvas->cr);
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_OPEN_LINES :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_BEZIER :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_FILL :
    cairo_fill(ctxcanvas->cr);
    break;
  }
}

/******************************************************/

static void cdgetimagergb(cdCtxCanvas *ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  int i, j, pos, offset, stride;
  unsigned int* data;
  cairo_surface_t* image_surface;
  cairo_t* cr;

  /* reset to the identity in image get operations */
  cairo_save(ctxcanvas->cr);
  cairo_identity_matrix(ctxcanvas->cr);

  /* if 0, invert because the transform was reset */
  if (!ctxcanvas->canvas->invert_yaxis) 
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= h-1;

  /* CAIRO_FORMAT_RGB24	each pixel is a 32-bit quantity, with the upper 8 bits unused. 
     Red, Green, and Blue are stored in the remaining 24 bits in that order. */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  cr = cairo_create(image_surface);

  /* creates a pattern from the canvas and sets it as source in the image. */
  cairo_set_source_surface(cr, cairo_get_target(ctxcanvas->cr), -x, -y);

  cairo_pattern_set_extend (cairo_get_source(cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);  /* paints the current source everywhere within the current clip region. */

  cairo_surface_flush(image_surface);
  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - w;

  for (i=0; i<h; i++)
  {
    for (j=0; j<w; j++)
    {
      pos = i*w+j;
      r[pos] = cdRed(*data);
      g[pos] = cdGreen(*data);
      b[pos] = cdBlue(*data);
      data++;
    }

    if (offset)
      data += offset;
  }

  cairo_surface_destroy(image_surface);
  cairo_destroy(cr);

  cairo_restore(ctxcanvas->cr);
}

static void sFixImageY(cdCanvas* canvas, int *topdown, double *y, double h)
{
  if (canvas->invert_yaxis)
    *topdown = 0;
  else
    *topdown = 1;

  if (!(*topdown))
    *y -= (h - 1);  /* move Y to top-left corner, since it was at the bottom of the image */
}

static void cdfputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, 
                               double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, pos, offset, topdown, stride;
  unsigned int* data;
  cairo_surface_t* image_surface;
  cairo_filter_t filter;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* CAIRO_FORMAT_RGB24	each pixel is a 32-bit quantity, with the upper 8 bits unused. 
     Red, Green, and Blue are stored in the remaining 24 bits in that order. */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, rw, rh);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  cairo_surface_flush(image_surface);
  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - rw;

  sFixImageY(ctxcanvas->canvas, &topdown, &y, h);

  for (i=ymin; i<=ymax; i++)
  {
    for (j=xmin; j<=xmax; j++)
    {
      if (topdown)
        pos = i*iw+j;
      else
        pos = (ymax+ymin - i)*iw+j;
      *data++ = sEncodeRGBA(r[pos], g[pos], b[pos], 255);
    }

    if (offset)
      data += offset;
  }

  cairo_surface_mark_dirty(image_surface);

  cairo_save (ctxcanvas->cr);

  sfCairoRectangle(ctxcanvas->cr, x, y, x+w, y+h);
  cairo_clip(ctxcanvas->cr);

  if (w != rw || h != rh)
  {
    /* Scale *before* setting the source surface (1) */
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_scale (ctxcanvas->cr, (double)w / rw, (double)h / rh);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  filter = cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr));
  cairo_set_source_surface(ctxcanvas->cr, image_surface, x, y);
  cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), filter);

  cairo_paint(ctxcanvas->cr);

  cairo_surface_destroy(image_surface);
  cairo_restore (ctxcanvas->cr);
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, 
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectrgb(ctxcanvas, iw, ih, r, g, b, (double)x, (double)y, (double)w, (double)h, xmin, xmax, ymin, ymax);
}

static void cdfputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, 
                                double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, pos, offset, topdown, stride;
  unsigned int* data;
  cairo_surface_t* image_surface;
  cairo_filter_t filter;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rw, rh);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  cairo_surface_flush(image_surface);
  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - rw;

  sFixImageY(ctxcanvas->canvas, &topdown, &y, h);

  for (i=ymin; i<=ymax; i++)
  {
    for (j=xmin; j<=xmax; j++)
    {
      if (topdown)
        pos = i*iw+j;
      else
        pos = (ymax+ymin - i)*iw+j;
      *data++ = sEncodeRGBA(r[pos], g[pos], b[pos], a[pos]);
    }

    if (offset)
      data += offset;
  }

  cairo_surface_mark_dirty(image_surface);

  cairo_save (ctxcanvas->cr);

  sfCairoRectangle(ctxcanvas->cr, x, y, x+w, y+h);
  cairo_clip(ctxcanvas->cr);

  if (w != rw || h != rh)
  {
    /* Scale *before* setting the source surface (1) */
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_scale (ctxcanvas->cr, (double)w / rw, (double)h / rh);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  filter = cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr));
  cairo_set_source_surface(ctxcanvas->cr, image_surface, x, y);
  cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), filter);

  cairo_paint(ctxcanvas->cr);

  cairo_surface_destroy(image_surface);
  cairo_restore (ctxcanvas->cr);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, 
                               int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectrgba(ctxcanvas, iw, ih, r, g, b, a, (double)x, (double)y, (double)w, (double)h, xmin, xmax, ymin, ymax);
}

static int sCalcPalSize(int size, const unsigned char *index)
{
  int i, pal_size = 0;

  for (i = 0; i < size; i++)
  {
    if (index[i] > pal_size)
      pal_size = index[i];
  }

  pal_size++;
  return pal_size;
}

static void cdfputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long *colors, 
                               double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, pos, offset, pal_size, topdown, stride;
  unsigned int* data, cairo_colors[256];
  long c;
  cairo_surface_t* image_surface;
  cairo_filter_t filter;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* CAIRO_FORMAT_RGB24	each pixel is a 32-bit quantity, with the upper 8 bits unused. 
     Red, Green, and Blue are stored in the remaining 24 bits in that order. */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, rw, rh);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  cairo_surface_flush(image_surface);
  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - rw;

  pal_size = sCalcPalSize(iw*ih, index);
  for (i=0; i<pal_size; i++)
  {
    c = colors[i];
    cairo_colors[i] = sEncodeRGBA(cdRed(c), cdGreen(c), cdBlue(c), 255);
  }

  sFixImageY(ctxcanvas->canvas, &topdown, &y, h);

  for (i=ymin; i<=ymax; i++)
  {
    for (j=xmin; j<=xmax; j++)
    {
      if (topdown)
        pos = i*iw+j;
      else
        pos = (ymax+ymin - i)*iw+j;
      *data++ = cairo_colors[index[pos]];
    }

    if (offset)
      data += offset;
  }

  cairo_surface_mark_dirty(image_surface);

  cairo_save (ctxcanvas->cr);

  sfCairoRectangle(ctxcanvas->cr, x, y, x+w, y+h);
  cairo_clip(ctxcanvas->cr);

  if (w != rw || h != rh)
  {
    /* Scale *before* setting the source surface (1) */
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_scale (ctxcanvas->cr, (double)w / rw, (double)h / rh);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  filter = cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr));
  cairo_set_source_surface(ctxcanvas->cr, image_surface, x, y);
  cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), filter);

  cairo_paint(ctxcanvas->cr);

  cairo_surface_destroy(image_surface);
  cairo_restore (ctxcanvas->cr);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long *colors, 
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectmap(ctxcanvas, iw, ih, index, colors, (double)x, (double)y, (double)w, (double)h, xmin, xmax, ymin, ymax);
}

static void cdfpixel(cdCtxCanvas *ctxcanvas, double x, double y, long color)
{
  cairo_pattern_t* old_source = cairo_get_source(ctxcanvas->cr);
  cairo_set_source_rgba(ctxcanvas->cr, cdCairoGetRed(color), cdCairoGetGreen(color), cdCairoGetBlue(color), cdCairoGetAlpha(color));

  cairo_move_to(ctxcanvas->cr, x, y);
  cairo_arc(ctxcanvas->cr, x, y, 0.5, 0.0, 2 * M_PI);

  cairo_fill(ctxcanvas->cr);
  cairo_set_source(ctxcanvas->cr, old_source);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long color)
{
  cdfpixel(ctxcanvas, (double)x, (double)y, color);
}

static void cdnewregion(cdCtxCanvas *ctxcanvas)
{
#if CAIRO_VERSION >= CAIRO_VERSION_110
  if (ctxcanvas->new_rgn)
    cairo_region_destroy(ctxcanvas->new_rgn);

  ctxcanvas->new_rgn = cairo_region_create();
#endif
}

static int cdispointinregion(cdCtxCanvas *ctxcanvas, int x, int y)
{
#if CAIRO_VERSION >= CAIRO_VERSION_110
  if (!ctxcanvas->new_rgn)
    return 0;

  if (cairo_region_contains_point(ctxcanvas->new_rgn, x, y))
    return 1;

  return 0;
#endif
}

static void cdoffsetregion(cdCtxCanvas *ctxcanvas, int x, int y)
{
#if CAIRO_VERSION >= CAIRO_VERSION_110
  if (!ctxcanvas->new_rgn)
    return;

  cairo_region_translate(ctxcanvas->new_rgn, x, y);
#endif
}

static void cdgetregionbox(cdCtxCanvas *ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
#if CAIRO_VERSION >= CAIRO_VERSION_110
  cairo_rectangle_int_t rect;

  if (!ctxcanvas->new_rgn)
    return;

  cairo_region_get_extents(ctxcanvas->new_rgn, &rect);

  *xmin = rect.x;
  *xmax = rect.x + rect.width - 1;
  *ymin = rect.y;
  *ymax = rect.y + rect.height - 1;
#endif
}

static cdCtxImage *cdcreateimage (cdCtxCanvas *ctxcanvas, int w, int h)
{
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));
  cairo_surface_t* img_surface;

  ctximage->w = w;
  ctximage->h = h;
  ctximage->bpp = ctxcanvas->canvas->bpp;
  ctximage->xres = ctxcanvas->canvas->xres;
  ctximage->yres = ctxcanvas->canvas->yres;
  ctximage->w_mm = ctximage->w / ctximage->xres;
  ctximage->h_mm = ctximage->h / ctximage->yres;

  img_surface = cairo_surface_create_similar(cairo_get_target(ctxcanvas->cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
  ctximage->cr = cairo_create(img_surface);

  if (!ctximage->cr)
  {
    free(ctximage);
    return (void *)0;
  }

  sCairoRectangleWH(ctximage->cr, 0, 0, ctximage->w, ctximage->h);
  cairo_set_source_rgba(ctximage->cr, 1.0, 0.0, 0.0, 1.0); /* white opaque */
  cairo_fill(ctximage->cr);

  cairo_surface_destroy(img_surface);

  return (void*)ctximage;
}

static void cdkillimage (cdCtxImage *ctximage)
{
  cairo_destroy(ctximage->cr);
  free(ctximage);
}

static void cdgetimage (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  /* reset to the identity in image get operations */
  cairo_save(ctximage->cr);
  cairo_identity_matrix(ctximage->cr);

  cairo_reset_clip(ctximage->cr);

  /* if 0, invert because the transform was reset */
  if (!ctxcanvas->canvas->invert_yaxis)  
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h-1;

  /* creates a pattern from the canvas and sets it as source in the image. */
  cairo_set_source_surface(ctximage->cr, cairo_get_target(ctxcanvas->cr), -x, -y);

  cairo_pattern_set_extend (cairo_get_source(ctximage->cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator (ctximage->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctximage->cr);  /* paints the current source everywhere within the current clip region. */

  /* must restore matrix, clipping and source */
  cairo_restore (ctximage->cr);
}

static void cdputimagerect (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  cairo_save (ctxcanvas->cr);

  /* y is the bottom-left of the image region in CD */
  y -= (ymax-ymin+1)-1;

  sCairoRectangleWH(ctxcanvas->cr, x, y, xmax - xmin + 1, ymax - ymin + 1);
  cairo_clip(ctxcanvas->cr);

  /* creates a pattern from the image and sets it as source in the canvas. */
  cairo_set_source_surface(ctxcanvas->cr, cairo_get_target(ctximage->cr), x, y);

  cairo_pattern_set_extend (cairo_get_source(ctxcanvas->cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator (ctxcanvas->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctxcanvas->cr);  /* paints the current source everywhere within the current clip region. */

  /* must restore clipping and source */
  cairo_restore (ctxcanvas->cr);
}

static void cdscrollarea (cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  /* reset to the identity in image get operations */
  cairo_save(ctxcanvas->cr);
  cairo_identity_matrix(ctxcanvas->cr);

  /* if 0, invert because the transform was reset */
  if (!ctxcanvas->canvas->invert_yaxis)  
  {
    dy = -dy;
    ymin = _cdInvertYAxis(ctxcanvas->canvas, ymin);
    ymax = _cdInvertYAxis(ctxcanvas->canvas, ymax);
    _cdSwapInt(ymin, ymax);
  }

  sCairoRectangleWH(ctxcanvas->cr, xmin + dx, ymin + dy, xmax - xmin + 1, ymax - ymin + 1);
  cairo_clip(ctxcanvas->cr);

  /* creates a pattern from the canvas and sets it as source in the canvas. */
  cairo_set_source_surface(ctxcanvas->cr, cairo_get_target(ctxcanvas->cr), xmin, ymin);

  cairo_pattern_set_extend (cairo_get_source(ctxcanvas->cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator (ctxcanvas->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctxcanvas->cr);  /* paints the current source everywhere within the current clip region. */

  /* must restore matrix, clipping and source */
  cairo_restore (ctxcanvas->cr);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  /* reset to identity */
  cairo_identity_matrix(ctxcanvas->cr);
  
  if (ctxcanvas->scale_points)
    cairo_scale(ctxcanvas->cr, ctxcanvas->scale, ctxcanvas->scale);

  if (matrix)
    ctxcanvas->canvas->invert_yaxis = 0;  /* let the transformation do the axis inversion */
  else
    ctxcanvas->canvas->invert_yaxis = 1;

  sSetTransform(ctxcanvas, matrix);
}

/******************************************************************/

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

static void set_polyhole_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  int hole;

  if (data == NULL)
  {
    ctxcanvas->holes = 0;
    return;
  }

  sscanf(data, "%d", &hole);
  ctxcanvas->poly_holes[ctxcanvas->holes] = hole;
  ctxcanvas->holes++;
}

static char* get_polyhole_attrib(cdCtxCanvas *ctxcanvas)
{
  static char holes[10];
  sprintf(holes, "%d", ctxcanvas->holes);
  return holes;
}

static cdAttribute polyhole_attrib =
{
  "POLYHOLE",
  set_polyhole_attrib,
  get_polyhole_attrib
}; 

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

  if (!ctxcanvas->rotate_angle)
    return NULL;

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

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
    cairo_set_antialias(ctxcanvas->cr, CAIRO_ANTIALIAS_NONE);
  else
    cairo_set_antialias(ctxcanvas->cr, CAIRO_ANTIALIAS_DEFAULT);
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  if (cairo_get_antialias(ctxcanvas->cr) != CAIRO_ANTIALIAS_NONE)
    return "1";
  else
    return "0";
}

static cdAttribute aa_attrib =
{
  "ANTIALIAS",
  set_aa_attrib,
  get_aa_attrib
}; 

static void set_txtaa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  cairo_font_options_t* options = cairo_font_options_copy(pango_cairo_context_get_font_options(ctxcanvas->fontcontext));

  if (!data || data[0] == '0')
    cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_NONE);
  else
    cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_DEFAULT);

  pango_cairo_context_set_font_options(ctxcanvas->fontcontext, options);
  cairo_font_options_destroy(options);
}

static char* get_txtaa_attrib(cdCtxCanvas* ctxcanvas)
{
  const cairo_font_options_t* options = pango_cairo_context_get_font_options(ctxcanvas->fontcontext);
  if (cairo_font_options_get_antialias(options) != CAIRO_ANTIALIAS_NONE)
    return "1";
  else
    return "0";
}

static cdAttribute txtaa_attrib =
{
  "TEXTANTIALIAS",
  set_txtaa_attrib,
  get_txtaa_attrib
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

static void set_pattern_image_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
  {
    cdCtxImage *ctximage = (cdCtxImage *)data;

    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = cairo_pattern_create_for_surface(cairo_get_target(ctximage->cr));
    cairo_pattern_reference(ctxcanvas->pattern);
    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
    ctxcanvas->canvas->interior_style = CD_CUSTOMPATTERN;
  }
}

static cdAttribute pattern_image_attrib =
{
  "PATTERNIMAGE",
  set_pattern_image_attrib,
  NULL
}; 

static void set_lineargradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int x1, y1, x2, y2;

    sscanf(data, "%d %d %d %d", &x1, &y1, &x2, &y2);

    if (ctxcanvas->canvas->invert_yaxis)
    {
      y1 = _cdInvertYAxis(ctxcanvas->canvas, y1);
      y2 = _cdInvertYAxis(ctxcanvas->canvas, y2);
    }

    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = cairo_pattern_create_linear((double)x1, (double)y1, (double)x2, (double)y2);
    cairo_pattern_reference(ctxcanvas->pattern);

    cairo_pattern_add_color_stop_rgba(ctxcanvas->pattern, 0.0,
                                      cdCairoGetRed(ctxcanvas->canvas->foreground),
                                      cdCairoGetGreen(ctxcanvas->canvas->foreground),
                                      cdCairoGetBlue(ctxcanvas->canvas->foreground),
                                      cdCairoGetAlpha(ctxcanvas->canvas->foreground));

    cairo_pattern_add_color_stop_rgba(ctxcanvas->pattern, 1.0,
                                      cdCairoGetRed(ctxcanvas->canvas->background),
                                      cdCairoGetGreen(ctxcanvas->canvas->background),
                                      cdCairoGetBlue(ctxcanvas->canvas->background),
                                      cdCairoGetAlpha(ctxcanvas->canvas->background));

    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
    ctxcanvas->canvas->interior_style = CD_CUSTOMPATTERN;
  }
}

static char* get_lineargradient_attrib(cdCtxCanvas* ctxcanvas)
{
  double x1, y1, x2, y2;

#if (CAIRO_VERSION_MAJOR>1 || (CAIRO_VERSION_MAJOR==1 && CAIRO_VERSION_MINOR>=4))
  if (cairo_pattern_get_linear_points(ctxcanvas->pattern, &x1, &y1, &x2, &y2) == CAIRO_STATUS_SUCCESS)
  {
    static char data[100];
    sprintf(data, "%d %d %d %d", (int)x1, (int)y1, (int)x2, (int)y2);
    return data;
  }
  else
#endif
    return NULL;
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

static void set_radialgradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int cx, cy, r;

    sscanf(data, "%d %d %d", &cx, &cy, &r);

    if (ctxcanvas->canvas->invert_yaxis)
      cy = _cdInvertYAxis(ctxcanvas->canvas, cy);

    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = cairo_pattern_create_radial((double)cx, (double)cy, 0, (double)cx, (double)cy, r);
    cairo_pattern_reference(ctxcanvas->pattern);

    cairo_pattern_add_color_stop_rgba(ctxcanvas->pattern, 0.0,
                                      cdCairoGetRed(ctxcanvas->canvas->foreground),
                                      cdCairoGetGreen(ctxcanvas->canvas->foreground),
                                      cdCairoGetBlue(ctxcanvas->canvas->foreground),
                                      cdCairoGetAlpha(ctxcanvas->canvas->foreground));

    cairo_pattern_add_color_stop_rgba(ctxcanvas->pattern, 1.0,
                                      cdCairoGetRed(ctxcanvas->canvas->background),
                                      cdCairoGetGreen(ctxcanvas->canvas->background),
                                      cdCairoGetBlue(ctxcanvas->canvas->background),
                                      cdCairoGetAlpha(ctxcanvas->canvas->background));

    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
    ctxcanvas->canvas->interior_style = CD_CUSTOMPATTERN;
  }
}

static char* get_radialgradient_attrib(cdCtxCanvas* ctxcanvas)
{
  double cx1, cy1, r1, cx, cy, r;

#if (CAIRO_VERSION_MAJOR>1 || (CAIRO_VERSION_MAJOR==1 && CAIRO_VERSION_MINOR>=4))
  if (cairo_pattern_get_radial_circles(ctxcanvas->pattern, &cx1, &cy1, &r1, &cx, &cy, &r) == CAIRO_STATUS_SUCCESS)
  {
    static char data[100];
    sprintf(data, "%d %d %d", (int)cx, (int)cy, (int)r);
    return data;
  }
  else
#endif
    return NULL;
}

static cdAttribute radialgradient_attrib =
{
  "RADIALGRADIENT",
  set_radialgradient_attrib,
  get_radialgradient_attrib
}; 

static char* get_version_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return (char*)cairo_version_string();
}

static cdAttribute version_attrib =
{
  "CAIROVERSION",
  NULL,
  get_version_attrib
};

static void set_interp_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && cdStrEqualNoCase(data, "BEST"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_BEST);
  else if (data && cdStrEqualNoCase(data, "NEAREST"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_NEAREST);
  else if (data && cdStrEqualNoCase(data, "FAST"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_FAST);
  else if (data && cdStrEqualNoCase(data, "BILINEAR"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_BILINEAR);
  else
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_GOOD);
}

static char* get_interp_attrib(cdCtxCanvas* ctxcanvas)
{
  cairo_filter_t filter = cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr));
  if (filter == CAIRO_FILTER_BEST)
    return "BEST";
  else if (filter == CAIRO_FILTER_NEAREST)
    return "NEAREST";
  else if (filter == CAIRO_FILTER_FAST)
    return "FAST";
  else if (filter == CAIRO_FILTER_BILINEAR)
    return "BILINEAR";
  else
    return "GOOD";
}

static cdAttribute interp_attrib =
{
  "IMGINTERP",
  set_interp_attrib,
  get_interp_attrib
};

static char* get_status_attrib(cdCtxCanvas *ctxcanvas)
{
  return (char*)cairo_status_to_string(cairo_status(ctxcanvas->cr));
}

static cdAttribute status_attrib =
{
  "STATUS",
  NULL,
  get_status_attrib
};

static char* get_cairodc_attrib(cdCtxCanvas *ctxcanvas)
{
  return (char*)ctxcanvas->cr;
}

static cdAttribute cairodc_attrib =
{
  "CAIRODC",
  NULL,
  get_cairodc_attrib
}; 

static cdAttribute gc_attrib =
{
  "GC",
  NULL,
  get_cairodc_attrib
}; 

#if !PANGO_VERSION_CHECK(1,22,0)
static PangoContext * cd_pango_cairo_create_context (cairo_t *cr)
{
  PangoFontMap *fontmap = pango_cairo_font_map_get_default ();
  PangoContext *context = pango_context_new();
  pango_context_set_font_map (context, fontmap);
  pango_cairo_update_context (cr, context);
  return context;
}
#endif

cdCtxCanvas *cdcairoCreateCanvas(cdCanvas* canvas, cairo_t* cr)
{
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->cr = cr;
  ctxcanvas->canvas = canvas;
  ctxcanvas->last_source = -1;
  ctxcanvas->hatchboxsize = 8;

  canvas->ctxcanvas = ctxcanvas;
  canvas->invert_yaxis = 1;

#if PANGO_VERSION_CHECK(1,22,0)
  ctxcanvas->fontcontext = pango_cairo_create_context(ctxcanvas->cr);
#else
  ctxcanvas->fontcontext = cd_pango_cairo_create_context(ctxcanvas->cr);
#endif
#if PANGO_VERSION_CHECK(1,16,0)
  pango_context_set_language(ctxcanvas->fontcontext, pango_language_get_default());
#endif

  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &version_attrib);
  cdRegisterAttribute(canvas, &polyhole_attrib);
  cdRegisterAttribute(canvas, &aa_attrib);
  cdRegisterAttribute(canvas, &txtaa_attrib);
  cdRegisterAttribute(canvas, &lineargradient_attrib);
  cdRegisterAttribute(canvas, &old_lineargradient_attrib);
  cdRegisterAttribute(canvas, &radialgradient_attrib);
  cdRegisterAttribute(canvas, &interp_attrib);
  cdRegisterAttribute(canvas, &cairodc_attrib);
  cdRegisterAttribute(canvas, &gc_attrib);
  cdRegisterAttribute(canvas, &hatchboxsize_attrib);
  cdRegisterAttribute(canvas, &pattern_image_attrib);
  cdRegisterAttribute(canvas, &utf8mode_attrib);
  cdRegisterAttribute(canvas, &status_attrib);

  cairo_save(ctxcanvas->cr);
  cairo_set_operator(ctxcanvas->cr, CAIRO_OPERATOR_OVER);

  return ctxcanvas;
}

void cdcairoInitTable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;

  canvas->cxPixel  = cdpixel;

  canvas->cxLine   = cdline;
  canvas->cxPoly   = cdpoly;
  canvas->cxRect   = cdrect;
  canvas->cxBox    = cdbox;
  canvas->cxArc    = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord  = cdchord;
  canvas->cxText   = cdtext;

  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;
  canvas->cxFText = cdftext;

  canvas->cxClip = cdclip;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxTransform = cdtransform;
  canvas->cxForeground = cdforeground;

  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;

  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxScrollArea = cdscrollarea;

  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxFPutImageRectRGB = cdfputimagerectrgb;
  canvas->cxFPutImageRectRGBA = cdfputimagerectrgba;
  canvas->cxFPutImageRectMap = cdfputimagerectmap;
  canvas->cxFPixel = cdfpixel;
}

#ifdef USE_GTK3
int cdBaseDriver(void)
{
  return CD_BASE_GDK;
}
#endif
