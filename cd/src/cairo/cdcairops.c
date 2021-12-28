/** \file
 * \brief Cairo PS Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdps.h"
#include "cdcairo.h"
#include "cdcairoctx.h"

#include <cairo-ps.h>


static void set_comment_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    cairo_ps_surface_dsc_comment(cairo_get_target(ctxcanvas->cr), data);
}

static cdAttribute comment_attrib =
{
  "DSCCOMMENT",
  set_comment_attrib,
  NULL
}; 

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  if (!ctxcanvas->eps)
    cairo_show_page(ctxcanvas->cr);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas *ctxcanvas;
  char* strdata = (char*)data;
  char filename[10240] = "";
  cairo_surface_t *surface;
  int res = 300;  /* dpi */
  double w_pt;         /* Largura do papel (points) */
  double h_pt;         /* Altura do papel (points) */
  double scale;          /* Fator de conversao de coordenadas (pixel2points) */
  int eps = 0;               /* Postscrip encapsulado? */
  int level = 0;
  int landscape = 0;         /* page orientation */

  /* Starting parameters */
  if (strdata == NULL) 
    return;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  cdSetPaperSize(CD_A4, &w_pt, &h_pt);

  while (*strdata != '\0')
  {
    while (*strdata != '\0' && *strdata != '-') 
      strdata++;

    if (*strdata != '\0')
    {
      double num;
      strdata++;
      switch (*strdata++)
      {
      case 'p':
        {
          int paper;
          sscanf(strdata, "%d", &paper);
          cdSetPaperSize(paper, &w_pt, &h_pt);
          break;
        }
      case 'w':
        sscanf(strdata, "%lg", &num);
        w_pt = CD_MM2PT*num;
        break;
      case 'h':
        sscanf(strdata, "%lg", &num);
        h_pt = CD_MM2PT*num;
        break;
      case 'e':
        eps = 1;
        break;
      case 'o':
        landscape = 1;
        break;
      case '2':
        level = 2;
        break;
      case '3':
        level = 3;
        break;
      case 's':
        sscanf(strdata, "%d", &res);
        break;
      }
    }

    while (*strdata != '\0' && *strdata != ' ') 
      strdata++;
  }

  if (landscape)
    _cdSwapDouble(w_pt, h_pt);

  scale = 72.0/res;  /* inches to points */

  canvas->w = (int)(w_pt/scale + 0.5);   /* points to pixels */
  canvas->h = (int)(h_pt/scale + 0.5);
  canvas->w_mm = w_pt/CD_MM2PT;   /* points to mm */
  canvas->h_mm = h_pt/CD_MM2PT;
  canvas->bpp = 24;
  canvas->xres = canvas->w / canvas->w_mm;
  canvas->yres = canvas->h / canvas->h_mm;

  surface = cairo_ps_surface_create(filename, w_pt, h_pt);  /* default coordinate system in points */

#if (CAIRO_VERSION_MAJOR>1 || (CAIRO_VERSION_MAJOR==1 && CAIRO_VERSION_MINOR>=6))
  if (level == 2)
    cairo_ps_surface_restrict_to_level(surface, CAIRO_PS_LEVEL_2);
  else if (level == 3)
    cairo_ps_surface_restrict_to_level(surface, CAIRO_PS_LEVEL_3);

  if (eps)
    cairo_ps_surface_set_eps(surface, 1);
#endif

  cairo_ps_surface_dsc_comment(surface, "%%Title: CanvasDraw");
  cairo_ps_surface_dsc_begin_setup (surface);
  cairo_ps_surface_dsc_begin_page_setup (surface);

  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(surface));
  ctxcanvas->eps = eps;

  ctxcanvas->scale = scale;
  ctxcanvas->scale_points = 1;
  cairo_identity_matrix(ctxcanvas->cr);
  cairo_scale(ctxcanvas->cr, ctxcanvas->scale, ctxcanvas->scale);

  cairo_surface_destroy(surface);

  cdRegisterAttribute(canvas, &comment_attrib);
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);
  canvas->cxKillCanvas = cdcairoKillCanvas;
  canvas->cxFlush = cdflush;
}

static cdContext cdCairoPSContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  CD_CTX_FILE,  /* not a plus driver */
  cdcreatecanvas,  
  cdinittable,
  NULL,                 
  NULL
};

cdContext* cdContextCairoPS(void)
{
  return &cdCairoPSContext;
}

