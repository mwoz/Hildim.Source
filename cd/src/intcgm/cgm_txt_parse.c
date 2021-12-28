#include <stdlib.h>
#include <string.h>

#include "cgm_types.h"
#include "cgm_txt_get.h"

/*******************************
*     Delimiter Elements       *
*******************************/

static int cgm_txt_begmtf(tCGM* cgm)
{
  char *s;
  if(cgm_txt_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  cgm->dof.BeginMetafile(s, cgm->userdata);
  free(s);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_endmtf(tCGM* cgm)
{
  cgm->dof.EndMetafile(cgm->userdata);
  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_begpic(tCGM* cgm)
{
  char *s;
  if(cgm_txt_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  cgm->dof.BeginPicture(s, cgm->userdata);
  free(s);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_begpib(tCGM* cgm)
{
  cgm->dof.BeginPictureBody(cgm->userdata);
  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_endpic(tCGM* cgm) /* end picture */
{
  cgm->dof.EndPicture(cgm->userdata);
  return cgm_txt_get_ter(cgm);
}

/*******************************
* Metafile Descriptor Elements *
*******************************/

static int cgm_txt_mtfver(tCGM* cgm)   /* metafile version */
{
  long version;

  if(cgm_txt_get_i(cgm, &version)) 
    return CGM_ERR_READ;
  
  if (version > 3)      /* unsupported */
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mtfdsc(tCGM* cgm)   /* metafile description */
{
  char *s;

  if(cgm_txt_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  /* ignored */
  free(s);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_vdctyp(tCGM* cgm)  /* vdc type */
{
  const char *options[] = { "INTEGER", "REAL", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->vdc_type), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_intpre(tCGM* cgm)   /* integer precision */
{
  if(cgm_txt_get_i(cgm, &(cgm->int_prec.t_prec.minint))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_i(cgm, &(cgm->int_prec.t_prec.maxint))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_realpr(tCGM* cgm)   /* real precision */
{
  if(cgm_txt_get_r(cgm, &(cgm->real_prec.t_prec.minreal))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_r(cgm, &(cgm->real_prec.t_prec.maxreal))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_i(cgm, &(cgm->real_prec.t_prec.digits))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_indpre(tCGM* cgm)   /* index precision */
{
  if(cgm_txt_get_i(cgm, &(cgm->ix_prec.t_prec.minint))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_i(cgm, &(cgm->ix_prec.t_prec.maxint))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_colpre(tCGM* cgm)   /* color precision */
{
  if(cgm_txt_get_i(cgm, &(cgm->cd_prec))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_colipr(tCGM* cgm)   /* color index precision */
{
  if(cgm_txt_get_i(cgm, &(cgm->cix_prec))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_maxcoi(tCGM* cgm)   /* maximum color index */
{
  if(cgm_txt_get_i(cgm, &(cgm->max_cix))) 
    return CGM_ERR_READ;

  cgm->color_table =(tRGB *) realloc(cgm->color_table, sizeof(tRGB)*(cgm->max_cix+1));

  cgm->color_table[0].red   = 255;
  cgm->color_table[0].green = 255;
  cgm->color_table[0].blue  = 255;
  cgm->color_table[1].red   = 0;
  cgm->color_table[1].green = 0;
  cgm->color_table[1].blue  = 0;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_covaex(tCGM* cgm)   /* color value extent */
{
  if(cgm_txt_get_cd(cgm, &(cgm->color_ext.black.red), &(cgm->color_ext.black.green), &(cgm->color_ext.black.blue))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_cd(cgm, &(cgm->color_ext.white.red), &(cgm->color_ext.white.green), &(cgm->color_ext.white.blue))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mtfell(tCGM* cgm)   /* metafile element list */
{
  char *elist;
  if(cgm_txt_get_s(cgm, &elist)) 
    return CGM_ERR_READ;
  /* ignored */
  free(elist);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_bmtfdf(tCGM* cgm)   /* begin metafile defaults */
{
  /* ignored */
  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_emtfdf(tCGM* cgm)   /* end metafile defaults */
{
  /* ignored */
  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_fntlst(tCGM* cgm)   /* font list */
{
  char *font=NULL;

  if(cgm->text_att.font_list==NULL) 
    cgm->text_att.font_list = cgm_list_new();

  while(cgm_txt_get_ter_noerr(cgm)) 
  {
    if(cgm_txt_get_s(cgm, &font)) 
      return CGM_ERR_READ;

    cgm_list_append(cgm->text_att.font_list, font);
    /* do not free "font", it is stored in the list */
  }

  return CGM_OK;
}

static int cgm_txt_chslst(tCGM* cgm)  /* character set list */
{
  const char *options[] = { "STD94", "STD96", "STD94MULTIBYTE", 
                            "STD96MULTIBYTE", "COMPLETECODE", NULL };
  char *tail;
  short code;

  do
  {
    if(cgm_txt_get_e(cgm, &(code), options)) 
      return CGM_ERR_READ;

    if(cgm_txt_get_s(cgm, &tail)) 
      return CGM_ERR_READ;
    free(tail);

  } while(cgm_txt_get_ter_noerr(cgm));
  /* ignored */

  return CGM_OK;
}

static int cgm_txt_chcdac(tCGM* cgm)   /* character coding announcer */
{
  const char *options[] = { "BASIC7BIT", "BASIC8BIT", "EXTD7BIT", "EXTD8BIT", NULL };
  short code;

  if(cgm_txt_get_e(cgm, &(code), options)) 
    return CGM_ERR_READ;
  /* ignored */

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_maxvdcext(tCGM* cgm)   /* maximum vdc extent */
{
  if(cgm_txt_get_p(cgm, &(cgm->vdc_ext.maxFirst.x), &(cgm->vdc_ext.maxFirst.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(cgm->vdc_ext.maxSecond.x), &(cgm->vdc_ext.maxSecond.y))) 
    return CGM_ERR_READ;

  cgm->vdc_ext.has_max = 1;

  return CGM_OK;
}

/******************************
* Picture Descriptor Elements *
******************************/

static int cgm_txt_sclmde(tCGM* cgm)   /* scaling mode */
{
  const char *options[] = { "ABSTRACT", "METRIC", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->scale_mode.mode), options)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_r(cgm, &(cgm->scale_mode.factor))) 
    return CGM_ERR_READ;

  if(cgm->scale_mode.mode==CGM_ABSTRACT) 
    cgm->scale_mode.factor = 1.;

  cgm->dof.ScaleMode(cgm->scale_mode.mode, &(cgm->scale_mode.factor), cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_clslmd(tCGM* cgm)   /* color selection mode */
{
  const char *options[] = { "INDEXED", "DIRECT", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->color_mode), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_lnwdmd(tCGM* cgm)   /* line width specification mode */
{
  const char *options[] = { "ABS", "SCALED", "FRACTIONAL", "MM", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->linewidth_mode), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mkszmd(tCGM* cgm)   /* marker size specification mode */
{
  const char *options[] = { "ABS", "SCALED", "FRACTIONAL", "MM", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->markersize_mode), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edwdmd(tCGM* cgm)   /* edge width specification mode */
{
  const char *options[] = { "ABS", "SCALED", "FRACTIONAL", "MM", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->edgewidth_mode), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_vdcext(tCGM* cgm)   /* vdc extent */
{
  if(cgm_txt_get_p(cgm, &(cgm->vdc_ext.first.x), &(cgm->vdc_ext.first.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_p(cgm, &(cgm->vdc_ext.second.x), &(cgm->vdc_ext.second.y))) 
    return CGM_ERR_READ;

  if (cgm->vdc_ext.has_max)
  {
    /* Verify the bounds values, defined by Maximum VDC Extent element */
    if(cgm->vdc_ext.first.x < cgm->vdc_ext.maxFirst.x)
      cgm->vdc_ext.first.x = cgm->vdc_ext.maxFirst.x;

    if(cgm->vdc_ext.first.y < cgm->vdc_ext.maxFirst.y)
      cgm->vdc_ext.first.y = cgm->vdc_ext.maxFirst.y;

    if(cgm->vdc_ext.second.x > cgm->vdc_ext.maxSecond.x)
      cgm->vdc_ext.second.x = cgm->vdc_ext.maxSecond.x;

    if(cgm->vdc_ext.second.y > cgm->vdc_ext.maxSecond.y)
      cgm->vdc_ext.second.y = cgm->vdc_ext.maxSecond.y;
  }

  cgm->dof.DeviceExtent(&(cgm->vdc_ext.first), &(cgm->vdc_ext.second), cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_bckcol(tCGM* cgm)   /* background color */
{
  if(cgm_txt_get_cd(cgm, &(cgm->back_color.red), &(cgm->back_color.green), &(cgm->back_color.blue))) 
    return CGM_ERR_READ;

  cgm->dof.BackgroundColor(cgm_getrgb(cgm, cgm->back_color), cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_intstymode(tCGM* cgm)  /* interior style specification mode */
{
  const char *options[] = { "ABS", "SCALED", "FRACTIONAL", "MM", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->interiorstyle_mode), options)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

/*******************
* Control Elements *
*******************/

static int cgm_txt_vdcipr(tCGM* cgm)   /* vdc integer precision */
{
  if(cgm_txt_get_i(cgm, &(cgm->vdc_int.t_prec.minint))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_i(cgm, &(cgm->vdc_int.t_prec.maxint))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_vdcrpr(tCGM* cgm)   /* vdc real precision */
{
  if(cgm_txt_get_r(cgm, &(cgm->vdc_real.t_prec.minreal))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_r(cgm, &(cgm->vdc_real.t_prec.maxreal))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_i(cgm, &(cgm->vdc_real.t_prec.digits))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_auxcol(tCGM* cgm)   /* auxiliary color */
{
  if(cgm_txt_get_co(cgm, &cgm->aux_color)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_transp(tCGM* cgm)   /* transparency */
{
  const char *options[] = { "OFF", "ON", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->transparency), options)) 
    return CGM_ERR_READ;

  cgm->dof.Transparency(cgm->transparency, cgm_getcolor(cgm, cgm->aux_color), cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_clprec(tCGM* cgm)   /* clip rectangle */
{
  if(cgm_txt_get_p(cgm, &(cgm->clip_rect.first.x),  &(cgm->clip_rect.first.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_p(cgm, &(cgm->clip_rect.second.x),  &(cgm->clip_rect.second.y))) 
    return CGM_ERR_READ;

  cgm->dof.ClipRectangle(cgm->clip_rect.first, cgm->clip_rect.second, cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_clpind(tCGM* cgm)   /* clip indicator */
{
  const char *options[] = { "OFF", "ON", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->clip_ind), options)) 
    return CGM_ERR_READ;

  cgm->dof.ClipIndicator(cgm->clip_ind, cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_pregionind(tCGM* cgm)
{
  if(cgm_txt_get_i(cgm, &(cgm->region_idx))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_i(cgm, &(cgm->region_ind))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_txt_miterlimit(tCGM* cgm)
{
  if(cgm_txt_get_r(cgm, &(cgm->mitrelimit))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_txt_transpcellcolor(tCGM* cgm)
{
  const char *options[] = { "OFF", "ON", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->cell_transp), options)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_co(cgm, &cgm->cell_color)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

/*******************************
* Graphical Primitive Elements *
*******************************/

static cgmPoint *get_points(tCGM* cgm, int *np)
{
  *np=0;

  do
  {
    if(cgm_txt_get_p(cgm, &(cgm->point_list[*np].x), &(cgm->point_list[*np].y))) 
      return NULL;

    ++(*np);

    if(*np==cgm->point_list_n)
    {
      cgm->point_list_n *= 2;
      cgm->point_list =(cgmPoint *) realloc(cgm->point_list, cgm->point_list_n*sizeof(cgmPoint));
    }
  } while(cgm_txt_get_ter_noerr(cgm));

  return cgm->point_list;
}

static int cgm_txt_polyln(tCGM* cgm)   /* polyline */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  cgm_setline_attrib(cgm);
  cgm->dof.Polygon(np, pt, CGM_LINES, cgm->userdata);

  return CGM_OK;
}

static void calc_incremental_polyline(int n_points, cgmPoint *pt)
{
  int i;
  for(i=1; i<n_points; i++)
  {
    pt[i].x += pt[i-1].x;    
    pt[i].y += pt[i-1].y;
  }
}

static int cgm_txt_incply(tCGM* cgm)   /* incremental polyline */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  calc_incremental_polyline(np, pt);

  cgm_setline_attrib(cgm);
  cgm->dof.Polygon(np, pt, CGM_LINES, cgm->userdata);

  return CGM_OK;
}

static int cgm_txt_djtply(tCGM* cgm)   /* disjoint polyline */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  cgm_setline_attrib(cgm);
  cgm->dof.PolyLine(np, pt, cgm->userdata);

  return CGM_OK;
}

static int cgm_txt_indjpl(tCGM* cgm)   /* incremental disjoint polyline */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  calc_incremental_polyline(np, pt);

  cgm_setline_attrib(cgm);
  cgm->dof.PolyLine(np, pt, cgm->userdata);

  return CGM_OK;
}

static int cgm_txt_polymk(tCGM* cgm)   /* polymarker */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  cgm_setmarker_attrib(cgm);
  cgm->dof.PolyMarker(np, pt, cgm->userdata);

  return CGM_OK;
}

static int cgm_txt_incplm(tCGM* cgm)   /* incremental polymarker */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  calc_incremental_polyline(np, pt);

  cgm_setmarker_attrib(cgm);
  cgm->dof.PolyMarker(np, pt, cgm->userdata);

  return CGM_OK;
}

static int cgm_txt_text(tCGM* cgm)   /* text */
{
  cgmPoint pos;
  const char *options[] = { "FINAL", "NOTFINAL", NULL };
  short flag;
  char *s;

  if(cgm_txt_get_p(cgm, &(pos.x), &(pos.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_e(cgm, &flag, options))  /* ignored */
    return CGM_ERR_READ;

  if(cgm_txt_get_s(cgm, &s)) 
    return CGM_ERR_READ;

  cgm_settext_attrib(cgm);
  cgm->dof.Text(s, pos, cgm->userdata);

  free(s);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_rsttxt(tCGM* cgm)   /* restricted text */
{
  double width, height;
  cgmPoint pos;
  const char *options[] = { "FINAL", "NOTFINAL", NULL };
  short flag;
  char *s;

  if(cgm_txt_get_vdc(cgm, &width)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &height)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_p(cgm, &(pos.x), &(pos.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_e(cgm, &flag, options))  /* ignored */
    return CGM_ERR_READ;

  if(cgm_txt_get_s(cgm, &s)) 
    return CGM_ERR_READ;

  /* WARNING: restricted text not supported, treated as normal text */

  cgm_settext_attrib(cgm);
  cgm->dof.Text(s, pos, cgm->userdata);

  free(s);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_apdtxt(tCGM* cgm)   /* append text */
{
  const char *options[] = { "FINAL", "NOTFINAL", NULL };
  short flag;
  char *s;

  if(cgm_txt_get_e(cgm, &flag, options)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  /* ignored */
  free(s);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_polygn(tCGM* cgm)   /* polygon */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.Polygon(np, pt, CGM_FILL, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.Polygon(np, pt, CGM_CLOSEDLINES, cgm->userdata);
  }

  return CGM_OK;
}

static int cgm_txt_incplg(tCGM* cgm)   /* incremental polygon */
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt == NULL) 
    return CGM_ERR_READ;

  calc_incremental_polyline(np, pt);

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.Polygon(np, pt, CGM_FILL, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.Polygon(np, pt, CGM_CLOSEDLINES, cgm->userdata);
  }

  return CGM_OK;
}

static int get_point_set(tCGM* cgm, cgmPoint **pt, short **flags, int *np)
{
  int block=500;
  const char *options[] = { "INVIS", "VIS", "CLOSEINVIS", "CLOSEVIS", NULL };

  *np=0;
  *pt =(cgmPoint *) malloc(block*sizeof(cgmPoint));
  *flags =(short *) malloc(block*sizeof(short));

  do
  {
    if(cgm_txt_get_p(cgm, &((*pt)[*np].x), &((*pt)[*np].y))) 
    {
      free(*pt);
      free(*flags);
      *pt = NULL;
      *flags = NULL;
      return CGM_ERR_READ;
    }

    if(cgm_txt_get_e(cgm, &((*flags)[*np]), options)) 
    {
      free(*pt);
      free(*flags);
      *pt = NULL;
      *flags = NULL;
      return CGM_ERR_READ;
    }

    ++(*np);
    if(*np==block)
    {
      block *= 2;
      *pt =(cgmPoint *) realloc(*pt, block*sizeof(cgmPoint));
      *flags =(short *) realloc(*flags, block*sizeof(short));
    }
  } while(cgm_txt_get_ter_noerr(cgm));

  return CGM_OK;
}

static int cgm_txt_plgset(tCGM* cgm)   /* polygon set */
{
  cgmPoint *pt;
  short *flags;
  int np;

  if(get_point_set(cgm, &pt, &flags, &np)) 
    return CGM_ERR_READ;

  cgm_polygonset(cgm, np, pt, flags);

  free(pt);
  free(flags);

  return CGM_OK;
}

static int cgm_txt_inpgst(tCGM* cgm)   /* incremental polygon set */
{
  cgmPoint *pt;
  short *flags;
  int np;

  if(get_point_set(cgm, &pt, &flags, &np)) 
    return CGM_ERR_READ;

  calc_incremental_polyline(np, pt);
  cgm_polygonset(cgm, np, pt, flags);

  free(pt);
  free(flags);

  return CGM_OK;
}

static int cgm_txt_cellar(tCGM* cgm)   /* cell array */
{
  cgmPoint corner1;
  cgmPoint corner2;
  cgmPoint corner3;
  long nx, ny;
  long local_color_prec;
  unsigned char *rgb;
  tColor cell;
  int i,k,offset;

  if(cgm_txt_get_p(cgm, &(corner1.x), &(corner1.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(corner2.x), &(corner2.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(corner3.x), &(corner3.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_i(cgm, &nx)) 
    return CGM_ERR_READ;
  if(cgm_txt_get_i(cgm, &ny)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_i(cgm, &(local_color_prec)))  /* unused */
    return CGM_ERR_READ;

  rgb = malloc(nx*ny*3);

  cgm_txt_skip_parentheses(cgm);

  for(k=0; k<ny; k++)
  {
    for(i=0; i<nx; i++)
    {
      if(cgm_txt_get_co(cgm, &cell)) 
      {
        free(rgb);
        return CGM_ERR_READ;
      }

      offset = 3*(k*nx+i);
      cgm_getcolor_ar(cgm, cell, rgb + offset+0, rgb + offset+1, rgb + offset+2);
    }

    if (cgm_inccounter(cgm))
    {
      free(rgb);
      return CGM_ABORT_COUNTER;
    }
  }

  cgm_txt_skip_parentheses(cgm);

  cgm->dof.CellArray(corner1, corner2, corner3, nx, ny, rgb, cgm->userdata);

  free(rgb);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_gdp(tCGM* cgm)   /* generalized drawing picture */
{
  long identifier;
  cgmPoint *pt = NULL;
  int block = 500;
  int np = 0;
  char *data_rec;
  char chr[1024];

  if(cgm_txt_get_i(cgm, &identifier)) 
    return CGM_ERR_READ;

  pt =(cgmPoint *) malloc(block*sizeof(cgmPoint));

  while(strstr(cgm_txt_get_sep(cgm, chr),",")==NULL)
  {
    if(cgm_txt_get_p(cgm, &(pt[np].x), &(pt[np].y)))
    {
      free(pt);
      return CGM_ERR_READ;
    }
    ++np;

    if(np==block)
    {
      block *= 2;
      pt =(cgmPoint *) realloc(pt, block*sizeof(cgmPoint));
    }
  }

  if(cgm_txt_get_s(cgm, &data_rec)) 
  {
    free(pt);
    return CGM_ERR_READ;
  }

  cgm_generalizeddrawingprimitive(cgm, identifier, np, pt, data_rec);

  free(data_rec);
  free(pt);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_rect(tCGM* cgm)   /* rectangle */
{
  cgmPoint point1;
  cgmPoint point2;

  if(cgm_txt_get_p(cgm, &(point1.x), &(point1.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(point2.x), &(point2.y))) 
    return CGM_ERR_READ;

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.Rectangle(point1, point2, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.Rectangle(point1, point2, cgm->userdata);
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_circle(tCGM* cgm)   /* circle */
{
  cgmPoint center;
  double radius;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.Circle(center, radius, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.Circle(center, radius, cgm->userdata);
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_circ3p(tCGM* cgm)   /* circular arc 3 point */
{
  cgmPoint start, intermediate, end, center;
  double radius, angle1, angle2;

  if(cgm_txt_get_p(cgm, &(start.x), &(start.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(intermediate.x), &(intermediate.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(end.x), &(end.y))) 
    return CGM_ERR_READ;

  cgm_calc_arc_3p(start, intermediate, end, &center, &radius, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.CircularArc(center, radius, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_cir3pc(tCGM* cgm)  /* circular arc 3 point close */
{
  cgmPoint start, intermediate, end, center;
  double radius, angle1, angle2;
  const char *options[] = { "PIE", "CHORD", NULL };
  short close_type;

  if(cgm_txt_get_p(cgm, &(start.x), &(start.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(intermediate.x), &(intermediate.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(end.x), &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_e(cgm, &close_type, options)) 
    return CGM_ERR_READ;

  cgm_calc_arc_3p(start, intermediate, end, &center, &radius, &angle1, &angle2);

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.CircularArc(center, radius, angle1, angle2, close_type, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.CircularArc(center, radius, angle1, angle2, close_type, cgm->userdata);
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_circnt(tCGM* cgm)   /* circular arc center */
{
  cgmPoint center, start, end;
  double radius, angle1, angle2;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  cgm_calc_arc(start, end, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.CircularArc(center, radius, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_ccntcl(tCGM* cgm)   /* circular arc center close */
{
  cgmPoint center, start, end;
  double radius, angle1, angle2;
  const char *options[] = { "PIE", "CHORD", NULL };
  short close_type;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_e(cgm, &close_type, options)) 
    return CGM_ERR_READ;

  cgm_calc_arc(start, end, &angle1, &angle2);

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.CircularArc(center, radius, angle1, angle2, close_type, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.CircularArc(center, radius, angle1, angle2, close_type, cgm->userdata);
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_ellips(tCGM* cgm)   /* ellipse */
{
  cgmPoint center;
  cgmPoint first;
  cgmPoint second;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_p(cgm, &(first.x), &(first.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(second.x), &(second.y))) 
    return CGM_ERR_READ;

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.Ellipse(center, first, second, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.Ellipse(center, first, second, cgm->userdata);
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_ellarc(tCGM* cgm)   /* elliptical arc */
{
  cgmPoint center;
  cgmPoint first;
  cgmPoint second;
  cgmPoint start, end;
  double angle1, angle2;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_p(cgm, &(first.x), &(first.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(second.x), &(second.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  cgm_calc_ellipse(center, first, second, start, end, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.EllipticalArc(center, first, second, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_ellacl(tCGM* cgm)   /* elliptical arc close */
{
  cgmPoint center;
  cgmPoint first;
  cgmPoint second;
  cgmPoint start, end;
  const char *options[] = { "PIE", "CHORD", NULL };
  short close_type;
  double angle1, angle2;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_p(cgm, &(first.x), &(first.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_p(cgm, &(second.x), &(second.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_e(cgm, &close_type, options)) 
    return CGM_ERR_READ;

  cgm_calc_ellipse(center, first, second, start, end, &angle1, &angle2);

  if (cgm->fill_att.int_style != CGM_EMPTY)
  {
    cgm_setfill_attrib(cgm);
    cgm->dof.EllipticalArc(center, first, second, angle1, angle2, close_type, cgm->userdata);
  }

  if (cgm->edge_att.visibility == CGM_ON)
  {
    cgm_setedge_attrib(cgm);
    cgm->dof.EllipticalArc(center, first, second, angle1, angle2, close_type, cgm->userdata);
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_circntrev(tCGM* cgm)   /* circular arc center reversed */
{
  cgmPoint center, start, end;
  double radius, angle1, angle2;

  if(cgm_txt_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  cgm_calc_arc_rev(start, end, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.CircularArc(center, radius, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_polybz(tCGM* cgm)
{
  cgmPoint *pt;
  int np;
  long indicator;

  if(cgm_txt_get_i(cgm, &(indicator))) 
    return CGM_ERR_READ;

  pt = get_points(cgm, &np);
  if(pt==NULL) 
    return CGM_ERR_READ;

  cgm_setline_attrib(cgm);

  if(indicator == 1)  /* discontinuous: sequence of curves with four points (one or more) */
  {
    int i, j = 0, numCurves = np / 4;
    cgmPoint ptTmp[4];

    for(i = 0; i < numCurves; i++)
    {
      ptTmp[0] = pt[j++];
      ptTmp[1] = pt[j++];
      ptTmp[2] = pt[j++];
      ptTmp[3] = pt[j++];
      cgm->dof.Polygon(4, ptTmp, CGM_BEZIER, cgm->userdata);
    }
  }
  else  /* continuous: sequence of curves with three points (one or more) */
  {
    /* Two or more curves: the first curve, and only the first, is defined by 4 points */
    cgm->dof.Polygon(np, pt, CGM_BEZIER, cgm->userdata);
  }

  return CGM_OK;
}

/*********************
* Attribute Elements *
*********************/

static int cgm_txt_lnbdin(tCGM* cgm)   /* line bundle index */
{
  if(cgm_txt_get_i(cgm, &(cgm->line_att.index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_lntype(tCGM* cgm)   /* line type */
{
  if(cgm_txt_get_i(cgm, &(cgm->line_att.type))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_lncap(tCGM* cgm)   /* line cap */
{
  if(cgm_txt_get_i(cgm, &(cgm->line_att.linecap))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_i(cgm, &(cgm->line_att.dashcap))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_lnjoin(tCGM* cgm)   /* line join */
{
  if(cgm_txt_get_i(cgm, &(cgm->line_att.linejoin))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_lnwidt(tCGM* cgm)  /* line width */
{
  if(cgm->linewidth_mode==CGM_ABSOLUTE)
  {
    if(cgm_txt_get_vdc(cgm, &(cgm->line_att.width))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_txt_get_r(cgm, &(cgm->line_att.width))) 
      return CGM_ERR_READ;
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_lncolr(tCGM* cgm)   /* line color */
{
  if(cgm_txt_get_co(cgm, &(cgm->line_att.color))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mkbdin(tCGM* cgm)   /* marker bundle index */
{
  if(cgm_txt_get_i(cgm, &(cgm->marker_att.index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mktype(tCGM* cgm)   /* marker type */
{
  if(cgm_txt_get_i(cgm, &(cgm->marker_att.type))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mksize(tCGM* cgm)   /* marker size */
{
  if(cgm->markersize_mode == CGM_ABSOLUTE)
  {
    if(cgm_txt_get_vdc(cgm, &(cgm->marker_att.size))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_txt_get_r(cgm, &(cgm->marker_att.size))) 
      return CGM_ERR_READ;
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_mkcolr(tCGM* cgm)   /* marker color */
{
  if(cgm_txt_get_co(cgm, &(cgm->marker_att.color))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_txbdin(tCGM* cgm)   /* text bundle index */
{
  if(cgm_txt_get_i(cgm, &(cgm->text_att.index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_txftin(tCGM* cgm)   /* text font index */
{
  if(cgm_txt_get_i(cgm, &(cgm->text_att.font_index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_txtprc(tCGM* cgm)   /* text precision */
{
  const char *options[] = { "S", "CHAR", "STROKE", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->text_att.prec), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_chrexp(tCGM* cgm)   /* char expansion factor */
{
  if(cgm_txt_get_r(cgm, &(cgm->text_att.exp_fact))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_chrspc(tCGM* cgm)   /* char spacing */
{
  if(cgm_txt_get_r(cgm, &(cgm->text_att.char_spacing))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_txtclr(tCGM* cgm)   /* text color */
{
  if(cgm_txt_get_co(cgm, &(cgm->text_att.color))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_chrhgt(tCGM* cgm)   /* char height */
{
  if(cgm_txt_get_vdc(cgm, &(cgm->text_att.height))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_chrori(tCGM* cgm)   /* char orientation */
{
  if(cgm_txt_get_vdc(cgm, &(cgm->text_att.char_up.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(cgm->text_att.char_up.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(cgm->text_att.char_base.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(cgm->text_att.char_base.y))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_txtpat(tCGM* cgm)   /* text path */
{
  const char *options[] = { "RIGHT", "LEFT", "UP", "DOWN", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->text_att.path), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_txtali(tCGM* cgm)   /* text alignment */
{
  const char *hor[] = { "NORMHORIZ", "LEFT", "CTR", "RIGHT", "CONTHORIZ", NULL };
  const char *ver[] = { "NORMVERT", "TOP", "CAP", "HALF", "BASE", "BOTTOM", "CONTVERT", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->text_att.alignment.hor), hor)) 
    return CGM_ERR_READ;
  if(cgm_txt_get_e(cgm, &(cgm->text_att.alignment.ver), ver)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_r(cgm, &(cgm->text_att.alignment.cont_hor))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_r(cgm, &(cgm->text_att.alignment.cont_ver))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_chseti(tCGM* cgm)   /* character set index */
{
  long set;

  if(cgm_txt_get_i(cgm, &set)) 
    return CGM_ERR_READ;
  /* ignored */
  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_achsti(tCGM* cgm)   /* alternate character set index */
{
  long set;

  if(cgm_txt_get_i(cgm, &set)) 
    return CGM_ERR_READ;
  /* ignored */

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_fillin(tCGM* cgm)   /* fill bundle index */
{
  if(cgm_txt_get_i(cgm, &(cgm->fill_att.index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_intsty(tCGM* cgm)   /* interior style */
{
  const char *options[] = { "HOLLOW", "SOLID", "PAT", "HATCH", "EMPTY", "GEOPAT", "INTERP", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->fill_att.int_style), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_fillco(tCGM* cgm)   /* fill color */
{
  if(cgm_txt_get_co(cgm, &(cgm->fill_att.color))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_hatind(tCGM* cgm)   /* hatch index */
{
  if(cgm_txt_get_i(cgm, &(cgm->fill_att.hatch_index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_patind(tCGM* cgm)   /* pattern index */
{
  if(cgm_txt_get_i(cgm, &(cgm->fill_att.pat_index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edgind(tCGM* cgm)   /* edge bundle index */
{
  if(cgm_txt_get_i(cgm, &(cgm->edge_att.index))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edgtyp(tCGM* cgm)   /* edge type */
{
  if(cgm_txt_get_i(cgm, &(cgm->edge_att.type))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edgcap(tCGM* cgm)   /* edge type */
{
  if(cgm_txt_get_i(cgm, &(cgm->edge_att.linecap))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_i(cgm, &(cgm->edge_att.dashcap))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edgjoin(tCGM* cgm)   /* edge type */
{
  if(cgm_txt_get_i(cgm, &(cgm->edge_att.linejoin))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_restrtype(tCGM* cgm)   /* restricted text type */
{
  if(cgm_txt_get_i(cgm, &(cgm->text_att.restr_type))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_txt_edgwid(tCGM* cgm)   /* edge width */
{
  if(cgm->edgewidth_mode==CGM_ABSOLUTE)
  {
    if(cgm_txt_get_vdc(cgm, &(cgm->edge_att.width))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_txt_get_r(cgm, &(cgm->edge_att.width))) 
      return CGM_ERR_READ;
  }

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edgcol(tCGM* cgm)   /* edge color */
{
  if(cgm_txt_get_co(cgm, &(cgm->edge_att.color))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_edgvis(tCGM* cgm)   /* edge visibility */
{
  const char *options[] = { "OFF", "ON", NULL };

  if(cgm_txt_get_e(cgm, &(cgm->edge_att.visibility), options)) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_fillrf(tCGM* cgm)   /* fill reference point */
{
  if(cgm_txt_get_p(cgm, &(cgm->fill_att.ref_pt.x),  &(cgm->fill_att.ref_pt.y))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_pattab(tCGM* cgm)   /* pattern table */
{
  long local_color_prec;
  int i;
  tPatTable *pat, *p;

  pat =(tPatTable *) malloc(sizeof(tPatTable));

  if(cgm->fill_att.pat_list==NULL) 
    cgm->fill_att.pat_list = cgm_list_new();

  if(cgm_txt_get_i(cgm, &(pat->index))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }

  if(cgm_txt_get_i(cgm, &(pat->nx))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }
  if(cgm_txt_get_i(cgm, &(pat->ny))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }

  if(cgm_txt_get_i(cgm, &(local_color_prec))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }

  pat->pattern =(tColor *) malloc(pat->nx*pat->ny*sizeof(tColor));

  cgm_txt_skip_parentheses(cgm);

  for(i=0; i<(pat->nx*pat->ny); i++)
  {
    if(cgm_txt_get_co(cgm, &(pat->pattern[i]))) 
    {
      free(pat->pattern);
      free(pat);
      return CGM_ERR_READ;
    }
  }

  cgm_txt_skip_parentheses(cgm);

  /* remove if exist a patttern with the same index */
  for(i=1; (p=(tPatTable *)cgm_list_get(cgm->fill_att.pat_list,i))!=NULL; i++)
  {
    if(p->index==pat->index)
    {
      free(p->pattern);
      cgm_list_del(cgm->fill_att.pat_list, i);
      break;
    }
  }

  cgm_list_append(cgm->fill_att.pat_list, pat);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_patsiz(tCGM* cgm)   /* pattern size */
{
  if(cgm_txt_get_vdc(cgm, &(cgm->fill_att.pat_size.height.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(cgm->fill_att.pat_size.height.y))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(cgm->fill_att.pat_size.width.x))) 
    return CGM_ERR_READ;
  if(cgm_txt_get_vdc(cgm, &(cgm->fill_att.pat_size.width.y))) 
    return CGM_ERR_READ;

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_coltab(tCGM* cgm)   /* color table */
{
  long starting_index;

  if(cgm_txt_get_i(cgm, &(starting_index))) 
    return CGM_ERR_READ;

  while(cgm_txt_get_ter_noerr(cgm))
  {
    if (starting_index <= cgm->max_cix)
    {
      if(cgm_txt_get_cd(cgm, &(cgm->color_table[starting_index].red), 
                              &(cgm->color_table[starting_index].green),
                              &(cgm->color_table[starting_index].blue))) 
        return CGM_ERR_READ;
      starting_index++;
    }
    else
    {
      tRGB c;
      if(cgm_txt_get_cd(cgm, &c.red, 
                             &c.green,
                             &c.blue)) 
        return CGM_ERR_READ;
    }
  }

  return CGM_OK;
}

static int cgm_txt_asf(tCGM* cgm)   /* asfs */
{
  /* saved, but ignored */
  const char *asf_value[] = { "BUNDLED", "INDIV", NULL };
  const char *asf_type[] = { "LINETYPE"     , "LINEWIDTH" , "LINECOLR"  ,
    "MARKERTYPE"   , "MARKERSIZE", "MARKERCOLR",
    "TEXTFONTINDEX", "TEXTPREC"  , "CHAREXP"   ,
    "CHARSPACE"    , "TEXTCOLR"  , "INTSTYLE"  ,
    "FILLCOLR"     , "HATCHINDEX", "PATINDEX"  ,
    "EDGETYPE"     , "EDGEWIDTH" , "EDGECOLR"  ,
    "ALL"          , "ALLLINE"   , "ALLMARKER" ,
    "ALLTEXT"      , "ALLFILL"   , "ALLEDGE", NULL };
  tASF *pair;

  if(cgm->asf_list==NULL) 
    cgm->asf_list = cgm_list_new();

  while(cgm_txt_get_ter_noerr(cgm))
  {
    pair = (tASF*)malloc(sizeof(tASF));

    if(cgm_txt_get_e(cgm, &(pair->type), asf_type)) 
    {
      free(pair);
      return CGM_ERR_READ;
    }
    if (cgm_txt_get_e(cgm, &(pair->value), asf_value))
    {
      free(pair);
      return CGM_ERR_READ;
    }

    cgm_list_append(cgm->asf_list, pair);
  }

  return CGM_OK;
}

/*****************
* Escape Element *
*****************/

static int cgm_txt_escape(tCGM* cgm)   /* escape */
{
  long identifier;
  char *data_rec;

  if(cgm_txt_get_i(cgm, &(identifier))) 
    return CGM_ERR_READ;

  if(cgm_txt_get_s(cgm, &data_rec)) 
    return CGM_ERR_READ;
  /* ignored */

  free(data_rec);

  return cgm_txt_get_ter(cgm);
}

/********************
* External elements *
********************/

static int cgm_txt_messag(tCGM* cgm)   /* message */
{
  const char *options[] = { "NOACTION", "ACTION", NULL };
  char *text;
  short flag;

  if(cgm_txt_get_e(cgm, &flag, options)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_s(cgm, &text)) 
    return CGM_ERR_READ;
  /* ignored */

  free(text);

  return cgm_txt_get_ter(cgm);
}

static int cgm_txt_appdta(tCGM* cgm)   /* application data */
{
  long identifier;
  char *data_rec;

  if(cgm_txt_get_i(cgm, &identifier)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_s(cgm, &data_rec)) 
    return CGM_ERR_READ;
  /* ignored */
  free(data_rec);

  return cgm_txt_get_ter(cgm);
}

/* delimiter elements */

static tCommand _cgm_txt_NULL             = { "", NULL };
static tCommand _cgm_txt_BEGMF            = { "BEGMF"     , cgm_txt_begmtf };
static tCommand _cgm_txt_ENDMF            = { "ENDMF"     , cgm_txt_endmtf };
static tCommand _cgm_txt_BEG_PIC          = { "BEGPIC"    , cgm_txt_begpic };
static tCommand _cgm_txt_BEG_PIC_BODY     = { "BEGPICBODY", cgm_txt_begpib };
static tCommand _cgm_txt_END_PIC          = { "ENDPIC"    , cgm_txt_endpic };

/* metafile descriptor elements */

static tCommand _cgm_txt_MF_VERSION         = { "MFVERSION"    , cgm_txt_mtfver };
static tCommand _cgm_txt_MF_DESC            = { "MFDESC"       , cgm_txt_mtfdsc };
static tCommand _cgm_txt_VDC_TYPE           = { "VDCTYPE"      , cgm_txt_vdctyp };
static tCommand _cgm_txt_INTEGER_PREC       = { "INTEGERPREC"  , cgm_txt_intpre };
static tCommand _cgm_txt_REAL_PREC          = { "REALPREC"     , cgm_txt_realpr };
static tCommand _cgm_txt_INDEX_PREC         = { "INDEXPREC"    , cgm_txt_indpre };
static tCommand _cgm_txt_COLR_PREC          = { "COLRPREC"     , cgm_txt_colpre };
static tCommand _cgm_txt_COLR_INDEX_PREC    = { "COLRINDEXPREC", cgm_txt_colipr };
static tCommand _cgm_txt_MAX_COLR_INDEX     = { "MAXCOLRINDEX" , cgm_txt_maxcoi };
static tCommand _cgm_txt_COLR_VALUE_EXT     = { "COLRVALUEEXT" , cgm_txt_covaex };
static tCommand _cgm_txt_MF_ELEM_LIST       = { "MFELEMLIST"   , cgm_txt_mtfell };
static tCommand _cgm_txt_BEG_MF_DEFAULTS    = { "BEGMFDEFAULTS", cgm_txt_bmtfdf };
static tCommand _cgm_txt_END_MF_DEFAULTS    = { "ENDMFDEFAULTS", cgm_txt_emtfdf };
static tCommand _cgm_txt_FONT_LIST          = { "FONTLIST"     , cgm_txt_fntlst };
static tCommand _cgm_txt_CHAR_SET_LIST      = { "CHARSETLIST"  , cgm_txt_chslst };
static tCommand _cgm_txt_CHAR_CODING        = { "CHARCODING"   , cgm_txt_chcdac };
static tCommand _cgm_bin_MAXIMUM_VDC_EXTENT = { "MAXVDCEXT"    , cgm_txt_maxvdcext };

/* picture descriptor elements */

static tCommand _cgm_txt_SCALE_MODE          = { "SCALEMODE"      , cgm_txt_sclmde };
static tCommand _cgm_txt_COLR_MODE           = { "COLRMODE"       , cgm_txt_clslmd };
static tCommand _cgm_txt_LINE_WIDTH_MODE     = { "LINEWIDTHMODE"  , cgm_txt_lnwdmd };
static tCommand _cgm_txt_MARKER_SIZE_MODE    = { "MARKERSIZEMODE" , cgm_txt_mkszmd };
static tCommand _cgm_txt_EDGE_WIDTH_MODE     = { "EDGEWIDTHMODE"  , cgm_txt_edwdmd };
static tCommand _cgm_txt_VDC_EXTENT          = { "VDCEXT"         , cgm_txt_vdcext };
static tCommand _cgm_txt_BACK_COLR           = { "BACKCOLR"       , cgm_txt_bckcol };
static tCommand _cgm_txt_INTERIOR_STYLE_MODE = { "INTSTYLEMODE"   , cgm_txt_intstymode };

/* control elements */

static tCommand _cgm_txt_VDC_INTEGER_PREC  = { "VDCINTEGERPREC" , cgm_txt_vdcipr };
static tCommand _cgm_txt_VDC_REAL_PREC     = { "VDCREALPREC"    , cgm_txt_vdcrpr };
static tCommand _cgm_txt_AUX_COLR          = { "AUXCOLR"        , cgm_txt_auxcol };
static tCommand _cgm_txt_TRANSPARENCY      = { "TRANSPARENCY"   , cgm_txt_transp };
static tCommand _cgm_txt_CLIP_RECT         = { "CLIPRECT"       , cgm_txt_clprec };
static tCommand _cgm_txt_CLIP              = { "CLIP"           , cgm_txt_clpind };
static tCommand _cgm_txt_PROTECTION_REGION = { "PROTREGION"     , cgm_txt_pregionind };
static tCommand _cgm_txt_MITER_LIMIT       = { "MITRELIMIT"     , cgm_txt_miterlimit };
static tCommand _cgm_txt_TRANSP_CELL_COLOR = { "TRANSPCELLCOLR" , cgm_txt_transpcellcolor };

/* primitive elements */

static tCommand _cgm_txt_LINE             = { "LINE"           , cgm_txt_polyln };
static tCommand _cgm_txt_INCR_LINE        = { "INCRLINE"       , cgm_txt_incply };
static tCommand _cgm_txt_DISJT_LINE       = { "DISJTLINE"      , cgm_txt_djtply };
static tCommand _cgm_txt_INCR_DISJT_LINE  = { "INCRDISJT_LINE" , cgm_txt_indjpl };
static tCommand _cgm_txt_MARKER           = { "MARKER"         , cgm_txt_polymk };
static tCommand _cgm_txt_INCR_MARKER      = { "INCRMARKER"     , cgm_txt_incplm };
static tCommand _cgm_txt_TEXT             = { "TEXT"           , cgm_txt_text };
static tCommand _cgm_txt_RESTR_TEXT       = { "RESTRTEXT"      , cgm_txt_rsttxt };
static tCommand _cgm_txt_APND_TEXT        = { "APNDTEXT"       , cgm_txt_apdtxt };
static tCommand _cgm_txt_POLYGON          = { "POLYGON"        , cgm_txt_polygn };
static tCommand _cgm_txt_INCR_POLYGON     = { "INCRPOLYGON"    , cgm_txt_incplg };
static tCommand _cgm_txt_POLYGON_SET      = { "POLYGONSET"     , cgm_txt_plgset };
static tCommand _cgm_txt_INCR_POLYGON_SET = { "INCRPOLYGONSET" , cgm_txt_inpgst };
static tCommand _cgm_txt_CELL_ARRAY       = { "CELLARRAY"      , cgm_txt_cellar };
static tCommand _cgm_txt_GDP              = { "GDP"            , cgm_txt_gdp };
static tCommand _cgm_txt_RECT             = { "RECT"           , cgm_txt_rect };
static tCommand _cgm_txt_CIRCLE           = { "CIRCLE"         , cgm_txt_circle };
static tCommand _cgm_txt_ARC_3_PT         = { "ARC3PT"         , cgm_txt_circ3p };
static tCommand _cgm_txt_ARC_3_PT_CLOSE   = { "ARC3PTCLOSE"    , cgm_txt_cir3pc };
static tCommand _cgm_txt_ARC_CTR          = { "ARCCTR"         , cgm_txt_circnt };
static tCommand _cgm_txt_ARC_CTR_CLOSE    = { "ARCCTR_CLOSE"   , cgm_txt_ccntcl };
static tCommand _cgm_txt_ELLIPSE          = { "ELLIPSE"        , cgm_txt_ellips };
static tCommand _cgm_txt_ELLIP_ARC        = { "ELLIPARC"       , cgm_txt_ellarc };
static tCommand _cgm_txt_ELLIP_ARC_CLOSE  = { "ELLIPARCCLOSE"  , cgm_txt_ellacl };
static tCommand _cgm_txt_ARC_CTR_REVERSE  = { "ARCCTRREV"      , cgm_txt_circntrev };
static tCommand _cgm_txt_BEZIER           = { "POLYBEZIER"     , cgm_txt_polybz }; 

/* attribute elements */

static tCommand _cgm_txt_LINE_INDEX       = { "LINEINDEX"      , cgm_txt_lnbdin };
static tCommand _cgm_txt_LINE_TYPE        = { "LINETYPE"       , cgm_txt_lntype };
static tCommand _cgm_txt_LINE_WIDTH       = { "LINEWIDTH"      , cgm_txt_lnwidt };
static tCommand _cgm_txt_LINE_COLR        = { "LINECOLR"       , cgm_txt_lncolr };
static tCommand _cgm_txt_MARKER_INDEX     = { "MARKERINDEX"    , cgm_txt_mkbdin };
static tCommand _cgm_txt_MARKER_TYPE      = { "MARKERTYPE"     , cgm_txt_mktype };
static tCommand _cgm_txt_MARKER_WIDTH     = { "MARKERSIZE"     , cgm_txt_mksize };
static tCommand _cgm_txt_MARKER_COLR      = { "MARKERCOLR"     , cgm_txt_mkcolr };
static tCommand _cgm_txt_TEXT_INDEX       = { "TEXTINDEX"      , cgm_txt_txbdin };
static tCommand _cgm_txt_TEXT_FONT_INDEX  = { "TEXTFONTINDEX"  , cgm_txt_txftin };
static tCommand _cgm_txt_TEXT_PREC        = { "TEXTPREC"       , cgm_txt_txtprc };
static tCommand _cgm_txt_CHAR_EXPAN       = { "CHAREXPAN"      , cgm_txt_chrexp };
static tCommand _cgm_txt_CHAR_SPACE       = { "CHARSPACE"      , cgm_txt_chrspc };
static tCommand _cgm_txt_TEXT_COLR        = { "TEXTCOLR"       , cgm_txt_txtclr };
static tCommand _cgm_txt_CHAR_HEIGHT      = { "CHARHEIGHT"     , cgm_txt_chrhgt };
static tCommand _cgm_txt_CHAR_ORI         = { "CHARORI"        , cgm_txt_chrori };
static tCommand _cgm_txt_TEXT_PATH        = { "TEXTPATH"       , cgm_txt_txtpat };
static tCommand _cgm_txt_TEXT_ALIGN       = { "TEXTALIGN"      , cgm_txt_txtali };
static tCommand _cgm_txt_CHAR_SET_INDEX   = { "CHARSETINDEX"   , cgm_txt_chseti };
static tCommand _cgm_txt_ALT_CHAR_SET     = { "ALTCHARSETINDEX", cgm_txt_achsti };
static tCommand _cgm_txt_FILL_INDEX       = { "FILLINDEX"      , cgm_txt_fillin };
static tCommand _cgm_txt_INT_STYLE        = { "INTSTYLE"       , cgm_txt_intsty };
static tCommand _cgm_txt_FILL_COLR        = { "FILLCOLR"       , cgm_txt_fillco };
static tCommand _cgm_txt_HATCH_INDEX      = { "HATCHINDEX"     , cgm_txt_hatind };
static tCommand _cgm_txt_PAT_INDEX        = { "PATINDEX"       , cgm_txt_patind };
static tCommand _cgm_txt_EDGE_INDEX       = { "EDGEINDEX"      , cgm_txt_edgind };
static tCommand _cgm_txt_EDGE_TYPE        = { "EDGETYPE"       , cgm_txt_edgtyp };
static tCommand _cgm_txt_EDGE_WIDTH       = { "EDGEWIDTH"      , cgm_txt_edgwid };
static tCommand _cgm_txt_EDGE_COLR        = { "EDGECOLR"       , cgm_txt_edgcol };
static tCommand _cgm_txt_EDGE_VIS         = { "EDGEVIS"        , cgm_txt_edgvis };
static tCommand _cgm_txt_FILL_REF_PT      = { "FILLREFPT"      , cgm_txt_fillrf };
static tCommand _cgm_txt_PAT_TABLE        = { "PATTABLE"       , cgm_txt_pattab };
static tCommand _cgm_txt_PAT_SIZE         = { "PATSIZE"        , cgm_txt_patsiz };
static tCommand _cgm_txt_COLR_TABLE       = { "COLRTABLE"      , cgm_txt_coltab };
static tCommand _cgm_txt_ASF              = { "ASF"            , cgm_txt_asf };
static tCommand _cgm_txt_LINE_CAP         = { "LINECAP"        , cgm_txt_lncap  };
static tCommand _cgm_txt_LINE_JOIN        = { "LINEJOIN"       , cgm_txt_lnjoin };
static tCommand _cgm_txt_EDGE_CAP         = { "EDGECAP"        , cgm_txt_edgcap };
static tCommand _cgm_txt_EDGE_JOIN        = { "EDGEJOIN"       , cgm_txt_edgjoin };
static tCommand _cgm_txt_RESTR_TEXT_TYPE  = { "RESTRTEXTTYPE"  , cgm_txt_restrtype };

/* escape elements */

static tCommand _cgm_txt_ESCAPE           = { "ESCAPE"         , cgm_txt_escape };
static tCommand _cgm_txt_DOMAIN_RING      = { "DOMAINRING"     , NULL };

/* external elements */

static tCommand _cgm_txt_MESSAGE          = { "MESSAGE"        , cgm_txt_messag };
static tCommand _cgm_txt_APPL_DATA        = { "APPLDATA"       , cgm_txt_appdta };

static tCommand *_cgm_txt_delimiter[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_BEGMF,
      &_cgm_txt_ENDMF,
      &_cgm_txt_BEG_PIC,
      &_cgm_txt_BEG_PIC_BODY,
      &_cgm_txt_END_PIC,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static tCommand *_cgm_txt_metafile[] = {
      &_cgm_txt_END_MF_DEFAULTS,
      &_cgm_txt_MF_VERSION,
      &_cgm_txt_MF_DESC,
      &_cgm_txt_VDC_TYPE,
      &_cgm_txt_INTEGER_PREC,
      &_cgm_txt_REAL_PREC,
      &_cgm_txt_INDEX_PREC,
      &_cgm_txt_COLR_PREC,
      &_cgm_txt_COLR_INDEX_PREC,
      &_cgm_txt_MAX_COLR_INDEX,
      &_cgm_txt_COLR_VALUE_EXT,
      &_cgm_txt_MF_ELEM_LIST,
      &_cgm_txt_BEG_MF_DEFAULTS,
      &_cgm_txt_FONT_LIST,
      &_cgm_txt_CHAR_SET_LIST,
      &_cgm_txt_CHAR_CODING,
      NULL,
      &_cgm_bin_MAXIMUM_VDC_EXTENT,
      NULL, NULL, NULL, NULL, NULL, NULL };

static tCommand *_cgm_txt_picture[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_SCALE_MODE,
      &_cgm_txt_COLR_MODE,
      &_cgm_txt_LINE_WIDTH_MODE,
      &_cgm_txt_MARKER_SIZE_MODE,
      &_cgm_txt_EDGE_WIDTH_MODE,
      &_cgm_txt_VDC_EXTENT,
      &_cgm_txt_BACK_COLR,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      &_cgm_txt_INTERIOR_STYLE_MODE,
      NULL, NULL, NULL };

static tCommand *_cgm_txt_control[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_VDC_INTEGER_PREC,
      &_cgm_txt_VDC_REAL_PREC,
      &_cgm_txt_AUX_COLR,
      &_cgm_txt_TRANSPARENCY,
      &_cgm_txt_CLIP_RECT,
      &_cgm_txt_CLIP,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      &_cgm_txt_PROTECTION_REGION,
      NULL,
      &_cgm_txt_MITER_LIMIT,
      &_cgm_txt_TRANSP_CELL_COLOR };

static tCommand *_cgm_txt_primitive[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_LINE,
      &_cgm_txt_INCR_LINE,
      &_cgm_txt_DISJT_LINE,
      &_cgm_txt_INCR_DISJT_LINE,
      &_cgm_txt_MARKER,
      &_cgm_txt_INCR_MARKER,
      &_cgm_txt_TEXT,
      &_cgm_txt_RESTR_TEXT,
      &_cgm_txt_APND_TEXT,
      &_cgm_txt_POLYGON,
      &_cgm_txt_INCR_POLYGON,
      &_cgm_txt_POLYGON_SET,
      &_cgm_txt_INCR_POLYGON_SET,
      &_cgm_txt_CELL_ARRAY,
      &_cgm_txt_GDP,
      &_cgm_txt_RECT,
      &_cgm_txt_CIRCLE,
      &_cgm_txt_ARC_3_PT,
      &_cgm_txt_ARC_3_PT_CLOSE,
      &_cgm_txt_ARC_CTR,
      &_cgm_txt_ARC_CTR_CLOSE,
      &_cgm_txt_ELLIPSE,
      &_cgm_txt_ELLIP_ARC,
      &_cgm_txt_ELLIP_ARC_CLOSE,
      &_cgm_txt_ARC_CTR_REVERSE,
      NULL, NULL, NULL, NULL, NULL,
      &_cgm_txt_BEZIER,
      NULL, NULL, NULL };

static tCommand *_cgm_txt_attributes[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_LINE_INDEX,
      &_cgm_txt_LINE_TYPE,
      &_cgm_txt_LINE_WIDTH,
      &_cgm_txt_LINE_COLR,
      &_cgm_txt_MARKER_INDEX,
      &_cgm_txt_MARKER_TYPE,
      &_cgm_txt_MARKER_WIDTH,
      &_cgm_txt_MARKER_COLR,
      &_cgm_txt_TEXT_INDEX,
      &_cgm_txt_TEXT_FONT_INDEX,
      &_cgm_txt_TEXT_PREC,
      &_cgm_txt_CHAR_EXPAN,
      &_cgm_txt_CHAR_SPACE,
      &_cgm_txt_TEXT_COLR,
      &_cgm_txt_CHAR_HEIGHT,
      &_cgm_txt_CHAR_ORI,
      &_cgm_txt_TEXT_PATH,
      &_cgm_txt_TEXT_ALIGN,
      &_cgm_txt_CHAR_SET_INDEX,
      &_cgm_txt_ALT_CHAR_SET,
      &_cgm_txt_FILL_INDEX,
      &_cgm_txt_INT_STYLE,
      &_cgm_txt_FILL_COLR,
      &_cgm_txt_HATCH_INDEX,
      &_cgm_txt_PAT_INDEX,
      &_cgm_txt_EDGE_INDEX,
      &_cgm_txt_EDGE_TYPE,
      &_cgm_txt_EDGE_WIDTH,
      &_cgm_txt_EDGE_COLR,
      &_cgm_txt_EDGE_VIS,
      &_cgm_txt_FILL_REF_PT,
      &_cgm_txt_PAT_TABLE,
      &_cgm_txt_PAT_SIZE,
      &_cgm_txt_COLR_TABLE,
      &_cgm_txt_ASF,
      NULL,
      &_cgm_txt_LINE_CAP, 
      &_cgm_txt_LINE_JOIN,
      NULL, NULL, NULL,
      &_cgm_txt_RESTR_TEXT_TYPE,
      NULL,
      &_cgm_txt_EDGE_CAP, 
      &_cgm_txt_EDGE_JOIN,
      NULL, NULL, NULL, NULL, NULL, NULL };

static tCommand *_cgm_txt_escape[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_ESCAPE,
      &_cgm_txt_DOMAIN_RING,
      NULL};

static tCommand *_cgm_txt_external[] = {
      &_cgm_txt_NULL,
      &_cgm_txt_MESSAGE,
      &_cgm_txt_APPL_DATA,
      NULL };

static tCommand *_cgm_txt_segment[] = {
      &_cgm_txt_NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static tCommand *_cgm_txt_NULL_NULL[] = {
      &_cgm_txt_NULL,
      NULL };
		
static tCommand **_cgm_txt_commands[] = {
       _cgm_txt_NULL_NULL,
       _cgm_txt_delimiter,
       _cgm_txt_metafile,
       _cgm_txt_picture,
       _cgm_txt_control,
       _cgm_txt_primitive,
       _cgm_txt_attributes,
       _cgm_txt_escape,
       _cgm_txt_external,
       _cgm_txt_segment,
      NULL };

int cgm_txt_rch(tCGM* cgm)
{
  char chr[1024] = "";
  char *pt;
  int i, j;

  cgm_txt_skip_sep(cgm);

  cgm_txt_skip_com(cgm);

  if (fscanf(cgm->fp, "%[^ \r\n\t\v\f,/;%\"()]", chr)==0)
    return CGM_ERR_READ;

  pt = strtok(chr,"_$");
  while(pt)
  {
    pt = strtok(NULL, "_$");
    if (pt)
      strcat(chr, pt);
  }

  if (chr[0]==0)
    return CGM_ERR_READ;

  cgm_strupper(chr);

  for(i=0; _cgm_txt_commands[i]!=NULL; i++)
  {
    for(j=0; _cgm_txt_commands[i][j]!=NULL; j++)
    {
      if(strcmp(chr, _cgm_txt_commands[i][j]->name)==0)
      {
        int ret = _cgm_txt_commands[i][j]->func(cgm);
        if (ret != CGM_OK)
          return ret;

        if (cgm_inccounter(cgm))
          return CGM_ABORT_COUNTER;

        if (feof(cgm->fp))
          return CGM_OK;
        else
          return CGM_CONT;
      }
    }
  }

  if (feof(cgm->fp))
    return CGM_OK;
  else
    return CGM_CONT;
}
