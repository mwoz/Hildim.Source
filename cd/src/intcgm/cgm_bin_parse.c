#include <string.h>   
#include <stdlib.h>   

#include "cgm_types.h"
#include "cgm_bin_get.h"

static int cgm_bin_exec_command(tCGM* cgm, int, int);

/*******************************
*     Delimiter Elements       *
*******************************/

static int cgm_bin_noop(tCGM* cgm)
{
  (void)cgm;
  return CGM_OK;
}

static int cgm_bin_begmtf(tCGM* cgm)
{
  if (cgm->buff.len)
  {
    char *s;
    if(cgm_bin_get_s(cgm, &s)) 
      return CGM_ERR_READ;

    cgm->dof.BeginMetafile(s, cgm->userdata);

    free(s);
  }
  else
    cgm->dof.BeginMetafile(NULL, cgm->userdata);

  return CGM_OK;
} 

static int cgm_bin_endmtf(tCGM* cgm)
{
  cgm->dof.EndMetafile(cgm->userdata);
  return CGM_OK;
}

static int cgm_bin_begpic(tCGM* cgm)
{
  char *s;
  if(cgm_bin_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  cgm->dof.BeginPicture(s, cgm->userdata);
  free(s);
  return CGM_OK;
}

static int cgm_bin_begpib(tCGM* cgm)
{
  cgm->dof.BeginPictureBody(cgm->userdata);
  return CGM_OK;
}

static int cgm_bin_endpic(tCGM* cgm)
{
  cgm->dof.EndPicture(cgm->userdata);
  return CGM_OK;
}

/*******************************
* Metafile Descriptor Elements *
*******************************/

static int cgm_bin_mtfver(tCGM* cgm)
{
  long version;
  if(cgm_bin_get_i(cgm, &version)) 
    return CGM_ERR_READ;
  
  if (version > 3)      /* unsupported */
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_mtfdsc(tCGM* cgm)
{
  char *s;
  if(cgm_bin_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  /* ignored */
  free(s);
  return CGM_OK;
}

static int cgm_bin_vdctyp(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->vdc_type))) 
    return CGM_ERR_READ;
  return CGM_OK;
}

static int cgm_bin_intpre(tCGM* cgm)
{
  long prec;

  if(cgm_bin_get_i(cgm, &prec)) 
    return CGM_ERR_READ;

  if(prec==8) cgm->int_prec.b_prec = 0;
  else if(prec==16)  cgm->int_prec.b_prec = 1; 
  else if(prec==24)  cgm->int_prec.b_prec = 2; 
  else if(prec==32)  cgm->int_prec.b_prec = 3; 

  return CGM_OK;
}

static int cgm_bin_realpr(tCGM* cgm)
{
  short mode = 0, i1;
  long i2, i3;

  if(cgm_bin_get_e(cgm, &i1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_i(cgm, &i2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_i(cgm, &i3)) 
    return CGM_ERR_READ;

  if(i1 == 0 && i2 == 9 && i3 == 23) mode = 0;
  else if(i1 == 0 && i2 == 12 && i3 == 52) mode = 1;
  else if(i1 == 1 && i2 == 16 && i3 == 16) mode = 2;
  else if(i1 == 1 && i2 == 32 && i3 == 32) mode = 3;

  cgm->real_prec.b_prec = mode;

  return CGM_OK;
}

static int cgm_bin_indpre(tCGM* cgm)
{
  long prec;

  if(cgm_bin_get_i(cgm, &prec)) 
    return CGM_ERR_READ;

  if(prec==8) cgm->ix_prec.b_prec = 0;
  else if(prec==16) cgm->ix_prec.b_prec = 1;
  else if(prec==24) cgm->ix_prec.b_prec = 2;
  else if(prec==32) cgm->ix_prec.b_prec = 3;

  return CGM_OK;
}

static int cgm_bin_colpre(tCGM* cgm)
{
  long prec;

  if(cgm_bin_get_i(cgm, &prec)) 
    return CGM_ERR_READ;

  if(prec==8) cgm->cd_prec = 0;
  else if(prec==16) cgm->cd_prec = 1;
  else if(prec==24) cgm->cd_prec = 2;
  else if(prec==32) cgm->cd_prec = 3;

  return CGM_OK;
}

static int cgm_bin_colipr(tCGM* cgm)
{
  long prec;

  if(cgm_bin_get_i(cgm, &prec)) 
    return CGM_ERR_READ;

  if(prec==8) cgm->cix_prec = 0;
  else if(prec==16) cgm->cix_prec = 1;
  else if(prec==24) cgm->cix_prec = 2;
  else if(prec==32) cgm->cix_prec = 3;

  return CGM_OK;
}

static int cgm_bin_maxcoi(tCGM* cgm)
{
  if(cgm_bin_get_ci(cgm,(unsigned long *)&(cgm->max_cix))) 
    return CGM_ERR_READ;

  cgm->color_table =(tRGB *) realloc(cgm->color_table, sizeof(tRGB)*(cgm->max_cix+1));

  cgm->color_table[0].red   = 255;
  cgm->color_table[0].green = 255;
  cgm->color_table[0].blue  = 255;
  cgm->color_table[1].red   = 0;
  cgm->color_table[1].green = 0;
  cgm->color_table[1].blue  = 0;

  return CGM_OK;
}

static int cgm_bin_covaex(tCGM* cgm)
{
  if(cgm_bin_get_cd(cgm, &(cgm->color_ext.black.red), &(cgm->color_ext.black.green), &(cgm->color_ext.black.blue))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_cd(cgm, &(cgm->color_ext.white.red), &(cgm->color_ext.white.green), &(cgm->color_ext.white.blue))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_mtfell(tCGM* cgm)
{
  long group, element;
  long n, i;

  if(cgm_bin_get_i(cgm, &n)) 
    return CGM_ERR_READ;

  for(i=0; i<n; i++)
  {
    if(cgm_bin_get_ix(cgm, &group))
      return CGM_ERR_READ;
    if(cgm_bin_get_ix(cgm, &element))
      return CGM_ERR_READ;
  }
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_bmtfdf(tCGM* cgm)
{
  /* default state is the state to which the interpreter is returned 
     at the start of each picture. */
  int c, id, len, cont, totbc; 
  unsigned short b;
  int count=0, ret;
  int old_cgmlen;
  char *buff;

  buff =(char *) malloc(sizeof(char)*cgm->buff.len);

  memcpy(buff, cgm->buff.data, cgm->buff.len);

  old_cgmlen = cgm->buff.len;

  totbc = 0;

  while(count<old_cgmlen)
  {
    cgm->buff.bc = 0;

    b =((unsigned char)buff[count] << 8) |(unsigned char)buff[count+1];
    count += 2;

    len = b & 0x001F;
    id =(b & 0x0FE0) >> 5;
    c =(b & 0xF000) >> 12;

    cont = 0;

    if(len > 30)
    {
      b =((unsigned char)buff[count] << 8) |(unsigned char)buff[count+1];
      count += 2;

      len = b & 0x7FFF;
      cont =(b & 0x8000);
    }

    cgm->buff.len = len;

    if(cgm->buff.len)
    {
      if (cgm->buff.len>cgm->buff.size)
      {
        cgm->buff.data = (char *)realloc(cgm->buff.data, cgm->buff.len);
        cgm->buff.size = cgm->buff.len;
      }

      memcpy(cgm->buff.data, &buff[count], cgm->buff.len);
      count += cgm->buff.len;

      if(len & 1)
        count++;

      while(cont)
      {
        unsigned short b;
        int old_len = cgm->buff.len;

        b =((unsigned char)buff[count] << 8) |(unsigned char)buff[count+1];
        count += 2;

        cont =(b & 0x8000);

        len = b & 0x7fff;

        cgm->buff.len += len;

        if (cgm->buff.len>cgm->buff.size)
        {
          cgm->buff.data = (char *)realloc(cgm->buff.data, cgm->buff.len);
          cgm->buff.size = cgm->buff.len;
        }

        memcpy(&cgm->buff.data[old_len], &buff[count], len);
        count += len;

        if(len & 1)
          count++;
      }
    }

    ret = cgm_bin_exec_command(cgm, c, id);
    if(ret != CGM_OK) 
    {
      free(buff);
      return ret;
    }

    totbc += count;
  }

  free(buff);
  return CGM_OK;
}

static int cgm_bin_fntlst(tCGM* cgm)
{
  char *fl = NULL;

  if(cgm->text_att.font_list==NULL) 
    cgm->text_att.font_list = cgm_list_new();

  while(cgm->buff.bc < cgm->buff.len)
  {
    if(cgm_bin_get_s(cgm, &fl)) 
      return CGM_ERR_READ;

    cgm_list_append(cgm->text_att.font_list, fl);
    /* do not free "fl", it is stored in the list */
  }

  return CGM_OK;
}

static int cgm_bin_chslst(tCGM* cgm)
{
  short mode;
  char *s;

  while(cgm->buff.bc < cgm->buff.len)
  {
    if(cgm_bin_get_e(cgm, &mode)) 
      return CGM_ERR_READ;
    if(cgm_bin_get_s(cgm, &s)) 
      return CGM_ERR_READ;
    free(s); 
  }
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_chcdac(tCGM* cgm)
{
  short mode;

  if(cgm_bin_get_e(cgm, &mode)) 
    return CGM_ERR_READ;
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_maxvdcext(tCGM* cgm)
{
  if(cgm_bin_get_p(cgm, &(cgm->vdc_ext.maxFirst.x), &(cgm->vdc_ext.maxFirst.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(cgm->vdc_ext.maxSecond.x), &(cgm->vdc_ext.maxSecond.y))) 
    return CGM_ERR_READ;

  cgm->vdc_ext.has_max = 1;

  return CGM_OK;
}

/******************************
* Picture Descriptor Elements *
******************************/

static int cgm_bin_sclmde(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->scale_mode.mode))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_rf(cgm, &(cgm->scale_mode.factor))) 
    return CGM_ERR_READ;

  if(cgm->scale_mode.mode==CGM_ABSTRACT) 
    cgm->scale_mode.factor = 1.;

  cgm->dof.ScaleMode(cgm->scale_mode.mode, &(cgm->scale_mode.factor), cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_clslmd(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->color_mode))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_lnwdmd(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->linewidth_mode))) 
    return CGM_ERR_READ;
  return CGM_OK;
}

static int cgm_bin_mkszmd(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->markersize_mode))) 
    return CGM_ERR_READ;
  return CGM_OK;
}

static int cgm_bin_edwdmd(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->edgewidth_mode))) 
    return CGM_ERR_READ;
  return CGM_OK;
}

static int cgm_bin_vdcext(tCGM* cgm)
{
  if(cgm_bin_get_p(cgm, &(cgm->vdc_ext.first.x), &(cgm->vdc_ext.first.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(cgm->vdc_ext.second.x), &(cgm->vdc_ext.second.y))) 
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

  return CGM_OK;
}

static int cgm_bin_bckcol(tCGM* cgm)
{
  if(cgm_bin_get_cd(cgm, &(cgm->back_color.red), &(cgm->back_color.green), &(cgm->back_color.blue))) 
    return CGM_ERR_READ;

  cgm->dof.BackgroundColor(cgm_getrgb(cgm, cgm->back_color), cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_intstymode(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->interiorstyle_mode))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

/*******************
* Control Elements *
*******************/

static int cgm_bin_vdcipr(tCGM* cgm)
{
  long prec;

  if(cgm_bin_get_i(cgm, &prec)) 
    return CGM_ERR_READ;

  if(prec==8) cgm->vdc_int.b_prec = 0;
  else if(prec==16)  cgm->vdc_int.b_prec = 1; 
  else if(prec==24)  cgm->vdc_int.b_prec = 2; 
  else if(prec==32)  cgm->vdc_int.b_prec = 3; 

  return CGM_OK;
}

static int cgm_bin_vdcrpr(tCGM* cgm)
{
  short i1;
  long mode = 0, i2, i3;

  if(cgm_bin_get_e(cgm, &i1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_i(cgm, &i2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_i(cgm, &i3)) return 1 ;

  if(i1 == 0 && i2 == 9 && i3 == 23) mode = 0;
  else if(i1 == 0 && i2 == 12 && i3 == 52) mode = 1;
  else if(i1 == 1 && i2 == 16 && i3 == 16) mode = 2;
  else if(i1 == 1 && i2 == 32 && i3 == 32) mode = 3;

  cgm->vdc_real.b_prec = mode;

  return CGM_OK;
}

static int cgm_bin_auxcol(tCGM* cgm)
{
  if(cgm_bin_get_co(cgm, &cgm->aux_color)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_transp(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->transparency))) 
    return CGM_ERR_READ;

  cgm->dof.Transparency(cgm->transparency, cgm_getcolor(cgm, cgm->aux_color), cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_clprec(tCGM* cgm)
{
  if(cgm_bin_get_p(cgm, &(cgm->clip_rect.first.x),  &(cgm->clip_rect.first.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(cgm->clip_rect.second.x),  &(cgm->clip_rect.second.y))) 
    return CGM_ERR_READ;

  cgm->dof.ClipRectangle(cgm->clip_rect.first, cgm->clip_rect.second, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_clpind(tCGM* cgm)
{
  if(cgm_bin_get_e  (cgm, &(cgm->clip_ind))) 
    return CGM_ERR_READ;

  cgm->dof.ClipIndicator(cgm->clip_ind, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_pregionind(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->region_idx))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_ix(cgm, &(cgm->region_ind))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_miterlimit(tCGM* cgm)
{
  if(cgm_bin_get_r(cgm, &(cgm->mitrelimit))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_transpcellcolor(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->cell_transp))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_co(cgm, &cgm->cell_color)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

/*******************************
* Graphical Primitive Elements *
*******************************/

static cgmPoint *get_points(tCGM* cgm, int *np)
{
  *np=0;

  while(cgm->buff.bc < cgm->buff.len)
  {
    if(cgm_bin_get_p(cgm, &(cgm->point_list[*np].x), &(cgm->point_list[*np].y))) 
      return NULL;

    ++(*np);

    if(*np==cgm->point_list_n)
    {
      cgm->point_list_n *= 2;
      cgm->point_list =(cgmPoint *) realloc(cgm->point_list, cgm->point_list_n*sizeof(cgmPoint));
    }
  }

  return cgm->point_list;
}

static int cgm_bin_polyln(tCGM* cgm)
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt==NULL) 
    return CGM_ERR_READ;

  cgm_setline_attrib(cgm);
  cgm->dof.Polygon(np, pt, CGM_LINES, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_djtply(tCGM* cgm)
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt==NULL) 
    return CGM_ERR_READ;

  cgm_setline_attrib(cgm);
  cgm->dof.PolyLine(np, pt, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_polymk(tCGM* cgm)
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt==NULL)                                                                   
    return CGM_ERR_READ;

  cgm_setmarker_attrib(cgm);
  cgm->dof.PolyMarker(np, pt, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_text(tCGM* cgm)
{
  cgmPoint pos;
  char *s;
  short t;

  if(cgm_bin_get_p(cgm, &pos.x, &pos.y)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_e(cgm, &t))   /* ignored */
    return CGM_ERR_READ;
  if(cgm_bin_get_s(cgm, &s)) 
    return CGM_ERR_READ;

  cgm_settext_attrib(cgm);
  cgm->dof.Text(s, pos, cgm->userdata);

  free(s);

  return CGM_OK;
}

static int cgm_bin_rsttxt(tCGM* cgm)
{
  double height, width;
  cgmPoint pos;
  char *s;
  short t;

  if(cgm_bin_get_vdc(cgm, &width)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &height)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &pos.x, &pos.y)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_e(cgm, &t))   /* ignored */
    return CGM_ERR_READ;
  if(cgm_bin_get_s(cgm, &s)) 
    return CGM_ERR_READ;

  /* WARNING: restricted text not supported, treated as normal text */

  cgm_settext_attrib(cgm);
  cgm->dof.Text(s, pos, cgm->userdata);

  free(s);

  return CGM_OK;
}

static int cgm_bin_apdtxt(tCGM* cgm)
{
  char *s;
  short t;

  if(cgm_bin_get_e(cgm, &t)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_s(cgm, &s)) 
    return CGM_ERR_READ;
  free(s);
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_polygn(tCGM* cgm)
{
  cgmPoint *pt;
  int np;

  pt = get_points(cgm, &np);
  if(pt==NULL) 
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

static int get_point_set(tCGM* cgm, cgmPoint **pt, short **flags, int *np)
{
  int block=500;

  *np=0;
  *pt =(cgmPoint *) malloc(block*sizeof(cgmPoint));
  *flags =(short *) malloc(block*sizeof(short));

  while(cgm->buff.bc < cgm->buff.len)
  {
    if(cgm_bin_get_p(cgm, &((*pt)[*np].x), &((*pt)[*np].y))) 
    {
      free(*pt);
      free(*flags);
      *pt = NULL;
      *flags = NULL;
      return CGM_ERR_READ;
    }
    if(cgm_bin_get_e(cgm, &((*flags)[*np]))) 
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
  }

  return CGM_OK;
}

static int cgm_bin_plgset(tCGM* cgm)
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

static int cgm_bin_cellar(tCGM* cgm)
{
  register int i, j, k, offset;
  long prec;
  long nx, ny;
  short mode;
  int b;
  unsigned char dummy;
  cgmPoint corner1, corner2, corner3;
  tColor cell;
  unsigned char* rgb;

  if(cgm_bin_get_p(cgm, &(corner1.x), &(corner1.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(corner2.x), &(corner2.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(corner3.x), &(corner3.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_i(cgm, &nx)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_i(cgm, &ny)) 
    return CGM_ERR_READ;

  if(cgm_bin_get_i(cgm, &prec)) 
    return CGM_ERR_READ;

  /* cell representation flag (only in binary mode) */
  if(cgm_bin_get_e(cgm, &mode)) 
    return CGM_ERR_READ;

  rgb = malloc(nx*ny*3);

  if (mode)
  {
    /* Packed mode */
    for(k=0; k<ny; k++)
    {
      b=cgm->buff.bc;
      cgm->buff.pc=0;

      for(i=0; i<nx; i++)
      {
        if(cgm_bin_get_pixel(cgm, &cell, prec)) 
        {
          free(rgb);
          return CGM_ERR_READ;
        }

        offset = 3*(k*nx+i);
        cgm_getcolor_ar(cgm, cell, rgb + offset+0, rgb + offset+1, rgb + offset+2);
      }

      /* row starts on a word boundary */
      if(k<(ny-1) &&(cgm->buff.bc-b)%2) 
        cgm_bin_get_c(cgm, &dummy);

      if (cgm_inccounter(cgm))
      {
        free(rgb);
        return CGM_ABORT_COUNTER;
      }
    }
  }
  else
  {
    /* run length encoding */
    for(k=0; k<ny; k++)
    {
      b=cgm->buff.bc;
      cgm->buff.pc=0;

      for(i=0; i<nx; ) /* do not increment here */
      {
        long run_count;
        if(cgm_bin_get_i(cgm, &run_count)) 
        {
          free(rgb);
          return CGM_ERR_READ;
        }
        if(cgm_bin_get_pixel(cgm, &cell, prec)) 
        {
          free(rgb);
          return CGM_ERR_READ;
        }

        for(j=0; j<run_count && i<nx; j++)
        {  
          offset = 3*(k*nx+i);
          cgm_getcolor_ar(cgm, cell, rgb + offset+0, rgb + offset+1, rgb + offset+2);
          i++; /* only increment here */
        }
      }

      /* row starts on a word boundary */
      if(k<(ny-1) &&(cgm->buff.bc-b)%2) 
        cgm_bin_get_c(cgm, &dummy);

      if (cgm_inccounter(cgm))
      {
        free(rgb);
        return CGM_ABORT_COUNTER;
      }
    }
  }

  cgm->dof.CellArray(corner1, corner2, corner3, nx, ny, rgb, cgm->userdata);

  free(rgb);
  return CGM_OK;
}

static int build_string(char *sin, char **sout, int *slen, int *block)
{
  *slen = strlen(sin) + strlen(*sout) + 1 + 1; /* + white space + \0 */
  if(*slen > *block)
  {
    char **o = sout;
    *block *= 2;
    *sout =(char *) realloc(*sout,sizeof(char)*(*block));
    if(*sout==NULL) 
    {
      free(*o);
      return CGM_ERR_READ;
    }
  }

  strcat(*sout,sin);
  strcat(*sout," ");

  return CGM_OK;
}

static int generalized_drawing_primitive_4(tCGM* cgm)
{
  long j, i;
  cgmPoint pt[4];
  unsigned char c;
  double r;
  char *s, tmp[80];
  int block = 500, slen = 0;

  for(j=0; j<4; j++)
  {
    if(cgm_bin_get_p(cgm, &(pt[j].x), &(pt[j].y))) 
      return CGM_ERR_READ;
  }

  if(cgm_bin_get_c(cgm, &c)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &c)) 
    return CGM_ERR_READ;

  s =(char *) malloc(block*sizeof(char));
  strcpy(s, "");

  for(j=0; j<6; j++)
  {
    if(cgm_bin_get_r(cgm, &r)) 
    {
      free(s);
      return CGM_ERR_READ;
    }
    sprintf(tmp,"%g",r);
    if(build_string(tmp, &s, &slen, &block)) 
      return CGM_ERR_READ;
  }

  if(cgm_bin_get_i(cgm, &i)) 
  {
    free(s);
    return CGM_ERR_READ;
  }
  sprintf(tmp,"%ld",i);
  if(build_string(tmp, &s, &slen, &block)) 
    return CGM_ERR_READ;

  if(cgm_bin_get_i(cgm, &cgm->gdp_sample_type)) 
  {
    free(s);
    return CGM_ERR_READ;
  }
  sprintf(tmp,"%ld",cgm->gdp_sample_type);
  if(build_string(tmp, &s, &slen, &block)) 
    return CGM_ERR_READ;

  if(cgm_bin_get_i(cgm, &cgm->gdp_n_samples)) 
  {
    free(s);
    return CGM_ERR_READ;
  }
  sprintf(tmp,"%ld",cgm->gdp_n_samples);
  if(build_string(tmp, &s, &slen, &block)) 
    return CGM_ERR_READ;

  for(j=0; j<2; j++)
  {
    if(cgm_bin_get_r(cgm, &r)) 
    {
      free(s);
      return CGM_ERR_READ;
    }
    sprintf(tmp,"%g",r);
    if(build_string(tmp, &s, &slen, &block)) 
      return CGM_ERR_READ;
  }

  for(j=0; j<4; j++)
  {
    if(cgm_bin_get_i(cgm, &i)) 
    {
      free(s);
      return CGM_ERR_READ;
    }
    sprintf(tmp,"%ld",i);
    if(build_string(tmp, &s, &slen, &block)) 
      return CGM_ERR_READ;
  }

  cgm_generalizeddrawingprimitive(cgm, -4, 4, pt, s);

  free(s);
  return CGM_OK;
}

static int generalized_drawing_primitive_5(tCGM* cgm)
{
  cgmGetData getdata = NULL;
  void  *data;
  char  format[10];
  int   i;
  char  *s, tmp[80];
  int   block = 500, slen = 0;

  getdata = cgm_bin_get_samplefunc(cgm->gdp_sample_type);
  if (!getdata)
    return CGM_ERR_READ;

  switch(cgm->gdp_sample_type)
  {
  case 0:
    data =(short *) malloc(sizeof(short));
    strcpy(format, "%d\0");
    break;
  case 1:
    data =(long *) malloc(sizeof(long));
    strcpy(format, "%d\0");
    break;
  case 2:
    data =(float *) malloc(sizeof(float));
    strcpy(format, "%g\0");
    break;
  case 3:
    data =(signed char *) malloc(sizeof(signed char));
    strcpy(format, "%d\0");
    break;
  case 4:
    data =(short *) malloc(sizeof(short));
    strcpy(format, "%d\0");
    break;
  case 5:
    data =(signed char *) malloc(sizeof(signed char));
    strcpy(format, "%d\0");
    break;
  default:
    return CGM_ERR_READ;
  }

  s =(char *) malloc(sizeof(char)*block);
  strcpy(s, "");

  for(i=0; i<cgm->gdp_n_samples; i++)
  {
    if (getdata(cgm, data))
    {
      free(data);
      free(s);
      return CGM_ERR_READ;
    }

    if(cgm->gdp_sample_type==0)
      sprintf(tmp,format,*(short *)data);
    else if(cgm->gdp_sample_type==1)
      sprintf(tmp,format,*(long *)data);
    else if(cgm->gdp_sample_type==2)
      sprintf(tmp,format,*(float *)data);
    else if(cgm->gdp_sample_type==3)
      sprintf(tmp,format,*(signed char *)data);
    else if(cgm->gdp_sample_type==4)
      sprintf(tmp,format,*(short *)data);
    else if(cgm->gdp_sample_type==5)
      sprintf(tmp,format,*(signed char *)data);

    if(build_string(tmp, &s, &slen, &block)) 
    {
      free(data);
      free(s);
      return CGM_ERR_READ;
    }

    if(cgm->gdp_sample_type==4 || cgm->gdp_sample_type==5)
    {
      unsigned long ci;
      char endstr='\0';

      if(cgm_bin_get_ci (cgm, &ci)) 
      {
        free(data);
        free(s);
        return CGM_ERR_READ;
      }
      sprintf(tmp,"%ld%c",ci,endstr);
      if(build_string(tmp, &s, &slen, &block)) 
      {
        free(data);
        free(s);
        return CGM_ERR_READ;
      }
    }
  }

  free(data);
  data = NULL;

  if(cgm->buff.bc < cgm->buff.len)
  {
    int i;
    unsigned char c;

    for(i=0; i<cgm->buff.len-cgm->buff.bc; i++)
    {
      if(cgm_bin_get_c(cgm, &c)) 
      {
        free(s);
        return CGM_ERR_READ;
      }
      if(cgm_bin_get_c(cgm, &c)) 
      {
        free(s);
        return CGM_ERR_READ;
      }
    }
  }

  cgm_generalizeddrawingprimitive(cgm, -5, 0, NULL, s);

  free(s);
  return CGM_OK;
}

static int cgm_bin_gdp(tCGM* cgm)
{
  long identifier, n, i;
  double x, y;

  cgm_bin_get_i(cgm, &identifier);

  if(identifier==-4) 
  {
    if(generalized_drawing_primitive_4(cgm)) 
      return CGM_ERR_READ;
  }
  else if(identifier==-5) 
  {
    if(generalized_drawing_primitive_5(cgm)) 
      return CGM_ERR_READ;
  }
  else
  {
    char *s;
    if(cgm_bin_get_i(cgm, &n)) 
      return CGM_ERR_READ;
    for(i=0; i<n; i++)
    {
      if(cgm_bin_get_p(cgm, &x, &y)) 
        return CGM_ERR_READ;
    }
    if(cgm_bin_get_s(cgm, &s)) 
      return CGM_ERR_READ;
    free(s); 
  }

  return CGM_OK;
}

static int cgm_bin_rect(tCGM* cgm)
{
  cgmPoint point1;
  cgmPoint point2;

  if(cgm_bin_get_p(cgm, &(point1.x), &(point1.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(point2.x), &(point2.y))) 
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

  return CGM_OK;
}

static int cgm_bin_circle(tCGM* cgm)
{
  cgmPoint center;
  double radius;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &radius)) 
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

  return CGM_OK;
}

static int cgm_bin_circ3p(tCGM* cgm)
{
  cgmPoint start, intermediate, end, center;
  double radius, angle1, angle2;

  if(cgm_bin_get_p(cgm, &(start.x), &(start.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(intermediate.x), &(intermediate.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(end.x), &(end.y))) 
    return CGM_ERR_READ;

  cgm_calc_arc_3p(start, intermediate, end, &center, &radius, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.CircularArc(center, radius, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_cir3pc(tCGM* cgm)
{
  cgmPoint start, intermediate, end, center;
  double radius;
  short close_type;
  double angle1, angle2;

  if(cgm_bin_get_p(cgm, &(start.x), &(start.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(intermediate.x), &(intermediate.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(end.x), &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_e(cgm, &close_type)) 
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

  return CGM_OK;
}

static int cgm_bin_circnt(tCGM* cgm)
{
  cgmPoint center, start, end;
  double radius, angle1, angle2;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  cgm_calc_arc(start, end, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.CircularArc(center, radius, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_ccntcl(tCGM* cgm)
{
  cgmPoint center, start, end;
  double radius, angle1, angle2;
  short close_type;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  if(cgm_bin_get_e(cgm, &close_type)) 
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

  return CGM_OK;
}

static int cgm_bin_ellips(tCGM* cgm)
{
  cgmPoint center;
  cgmPoint first;
  cgmPoint second;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_p(cgm, &(first.x), &(first.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(second.x), &(second.y))) 
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

  return CGM_OK;
}

static int cgm_bin_ellarc(tCGM* cgm)
{
  cgmPoint center;
  cgmPoint first;
  cgmPoint second;
  cgmPoint start, end;
  double angle1, angle2;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_p(cgm, &(first.x), &(first.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(second.x), &(second.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  cgm_calc_ellipse(center, first, second, start, end, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.EllipticalArc(center, first, second, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_ellacl(tCGM* cgm)
{
  cgmPoint center;
  cgmPoint first;
  cgmPoint second;
  cgmPoint start, end;
  short close_type;
  double angle1, angle2;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_p(cgm, &(first.x), &(first.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_p(cgm, &(second.x), &(second.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_e(cgm, &close_type)) 
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

  return CGM_OK;
}

static int cgm_bin_circntrev(tCGM* cgm)
{
  cgmPoint center, start, end;
  double radius, angle1, angle2;

  if(cgm_bin_get_p(cgm, &(center.x), &(center.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(start.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(start.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &(end.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(end.y))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_vdc(cgm, &radius)) 
    return CGM_ERR_READ;

  cgm_calc_arc_rev(start, end, &angle1, &angle2);

  cgm_setline_attrib(cgm);
  cgm->dof.CircularArc(center, radius, angle1, angle2, CGM_OPENARC, cgm->userdata);

  return CGM_OK;
}

static int cgm_bin_polybz(tCGM* cgm)
{
  cgmPoint *pt;
  int np;
  long indicator;

  if(cgm_bin_get_ix(cgm, &(indicator))) 
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

static int cgm_bin_lnbdin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->line_att.index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_lntype(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->line_att.type))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_lnwidt(tCGM* cgm)
{
  if(cgm->linewidth_mode==CGM_ABSOLUTE)
  {
    if(cgm_bin_get_vdc(cgm, &(cgm->line_att.width))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_bin_get_r(cgm, &(cgm->line_att.width))) 
      return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int cgm_bin_lncolr(tCGM* cgm)
{
  if(cgm_bin_get_co(cgm, &(cgm->line_att.color))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_mkbdin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->marker_att.index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_mktype(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->marker_att.type))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_mksize(tCGM* cgm)
{
  if(cgm->markersize_mode == CGM_ABSOLUTE)
  {
    if(cgm_bin_get_vdc(cgm, &(cgm->marker_att.size))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_bin_get_r(cgm, &(cgm->marker_att.size))) 
      return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int cgm_bin_mkcolr(tCGM* cgm)
{
  if(cgm_bin_get_co(cgm, &(cgm->marker_att.color))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_txbdin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->text_att.index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_txftin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->text_att.font_index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_txtprc(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->text_att.prec))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_chrexp(tCGM* cgm)
{
  if(cgm_bin_get_r(cgm, &(cgm->text_att.exp_fact))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_chrspc(tCGM* cgm)
{
  if(cgm_bin_get_r(cgm, &(cgm->text_att.char_spacing))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_txtclr(tCGM* cgm)
{
  if(cgm_bin_get_co(cgm, &(cgm->text_att.color))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_chrhgt(tCGM* cgm)
{
  if(cgm_bin_get_vdc(cgm, &(cgm->text_att.height))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_chrori(tCGM* cgm)
{
  if(cgm_bin_get_vdc(cgm, &(cgm->text_att.char_up.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(cgm->text_att.char_up.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(cgm->text_att.char_base.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(cgm->text_att.char_base.y))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_txtpat(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->text_att.path))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_txtali(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->text_att.alignment.hor))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_e(cgm, &(cgm->text_att.alignment.ver))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_r(cgm, &(cgm->text_att.alignment.cont_hor))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_r(cgm, &(cgm->text_att.alignment.cont_ver))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_chseti(tCGM* cgm)
{
  long index;

  if(cgm_bin_get_ix(cgm, &index)) 
    return CGM_ERR_READ;
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_achsti(tCGM* cgm)
{
  long index;

  if(cgm_bin_get_ix(cgm, &index)) 
    return CGM_ERR_READ;
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_fillin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->fill_att.index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_intsty(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->fill_att.int_style))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_fillco(tCGM* cgm)
{
  if(cgm_bin_get_co(cgm, &(cgm->fill_att.color))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_hatind(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->fill_att.hatch_index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_patind(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->fill_att.pat_index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_edgind(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->edge_att.index))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_edgtyp(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->edge_att.type))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_edgwid(tCGM* cgm)
{
  if(cgm->edgewidth_mode==CGM_ABSOLUTE)
  {
    if(cgm_bin_get_vdc(cgm, &(cgm->edge_att.width))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_bin_get_r(cgm, &(cgm->edge_att.width))) 
      return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int cgm_bin_edgcol(tCGM* cgm)
{
  if(cgm_bin_get_co(cgm, &(cgm->edge_att.color))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_edgvis(tCGM* cgm)
{
  if(cgm_bin_get_e(cgm, &(cgm->edge_att.visibility))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_fillrf(tCGM* cgm)
{
  if(cgm_bin_get_p(cgm, &(cgm->fill_att.ref_pt.x),  &(cgm->fill_att.ref_pt.y))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_pattab(tCGM* cgm)
{
  long localp;
  int i;
  tPatTable *pat, *p;

  pat =(tPatTable *) malloc(sizeof(tPatTable));

  if(cgm->fill_att.pat_list==NULL) 
    cgm->fill_att.pat_list = cgm_list_new();

  if(cgm_bin_get_ix(cgm, &(pat->index))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }

  if(cgm_bin_get_i(cgm, &(pat->nx))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }
  if(cgm_bin_get_i(cgm, &(pat->ny))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }

  if(cgm_bin_get_i(cgm, &(localp))) 
  {
    free(pat);
    return CGM_ERR_READ;
  }

  pat->pattern =(tColor *) malloc(pat->nx*pat->ny*sizeof(tColor));

  cgm->buff.pc=0;
  for(i=0; i<(pat->nx*pat->ny); i++)
  {
    if(cgm_bin_get_pixel(cgm, &(pat->pattern[i]), localp)) 
    {
      free(pat->pattern);
      free(pat);
      return CGM_ERR_READ;
    }
  }

  /* remove if exist a pattern with the same index */
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

  return CGM_OK;
}

static int cgm_bin_patsiz(tCGM* cgm)
{
  if(cgm_bin_get_vdc(cgm, &(cgm->fill_att.pat_size.height.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(cgm->fill_att.pat_size.height.y))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(cgm->fill_att.pat_size.width.x))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, &(cgm->fill_att.pat_size.width.y))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_coltab(tCGM* cgm)
{
  unsigned long starting_index, i;
  int p[] = {8, 16, 24, 32};
  int n =(cgm->buff.len-cgm->cix_prec)/(3*(p[cgm->cd_prec]/8));

  if(cgm_bin_get_ci(cgm, &(starting_index))) 
    return CGM_ERR_READ;

  for(i=starting_index; i<starting_index+n; i++)
  {
    if (i <= (unsigned long)cgm->max_cix)
    {
      if(cgm_bin_get_cd(cgm, &(cgm->color_table[i].red), &(cgm->color_table[i].green), &(cgm->color_table[i].blue))) 
        return CGM_ERR_READ;
    }
    else
    {
      tRGB c;
      if(cgm_bin_get_cd(cgm, &c.red, 
                             &c.green,
                             &c.blue)) 
        return CGM_ERR_READ;
    }
  }

  if(cgm->buff.bc==(cgm->buff.len-1)) 
    cgm->buff.bc++;

  return CGM_OK;
}

static int cgm_bin_asf(tCGM* cgm)
{
  /* saved, but ignored */
  tASF *pair;

  if(cgm->asf_list==NULL) 
    cgm->asf_list = cgm_list_new();

  while(cgm->buff.bc < cgm->buff.len)
  {
    pair = (tASF*)malloc(sizeof(tASF));

    if(cgm_bin_get_e(cgm, &(pair->type))) 
    {
      free(pair);
      return CGM_ERR_READ;
    }
    if(cgm_bin_get_e(cgm, &(pair->value))) 
    {
      free(pair);
      return CGM_ERR_READ;
    }

    cgm_list_append(cgm->asf_list, pair);
  }

  return CGM_OK;
}

static int cgm_bin_lncap(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->line_att.linecap))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_ix(cgm, &(cgm->line_att.dashcap))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_lnjoin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->line_att.linejoin))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_restrtype(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->text_att.restr_type))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_edgcap(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->edge_att.linecap))) 
    return CGM_ERR_READ;

  if(cgm_bin_get_ix(cgm, &(cgm->edge_att.dashcap))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_edgjoin(tCGM* cgm)
{
  if(cgm_bin_get_ix(cgm, &(cgm->edge_att.linejoin))) 
    return CGM_ERR_READ;

  return CGM_OK;
}

/*****************
* Escape Element *
*****************/

static int cgm_bin_escape(tCGM* cgm)
{
  /* ignored */
  long identifier;
  char *data_rec;

  if(cgm_bin_get_i(cgm, &(identifier))) 
    return CGM_ERR_READ;
  if(cgm_bin_get_s(cgm, &data_rec)) 
    return CGM_ERR_READ;

  free(data_rec);

  return CGM_OK;
}

/********************
* External elements *
********************/

static int cgm_bin_messag(tCGM* cgm)
{
  char *text;
  short flag;

  if(cgm_bin_get_e(cgm, &flag)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_s(cgm, &text)) 
    return CGM_ERR_READ;
  free(text);
  /* ignored */

  return CGM_OK;
}

static int cgm_bin_appdta(tCGM* cgm)
{
  long identifier;
  char *data_rec;

  if(cgm_bin_get_i(cgm, &identifier)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_s(cgm, &data_rec)) 
    return CGM_ERR_READ;
  free(data_rec);
  /* ignored */

  return CGM_OK;
}

/*******************
* Segment elements *
*******************/


/* delimiter elements */

static CGM_FUNC _cgm_bin_NULL            = NULL;

static CGM_FUNC _cgm_bin_NOOP            = &cgm_bin_noop;
static CGM_FUNC _cgm_bin_BEGMF           = &cgm_bin_begmtf;
static CGM_FUNC _cgm_bin_ENDMF           = &cgm_bin_endmtf;
static CGM_FUNC _cgm_bin_BEG_PIC         = &cgm_bin_begpic;
static CGM_FUNC _cgm_bin_BEG_PIC_BODY    = &cgm_bin_begpib;
static CGM_FUNC _cgm_bin_END_PIC         = &cgm_bin_endpic;

/* metafile descriptor elements */

static CGM_FUNC _cgm_bin_MF_VERSION         = &cgm_bin_mtfver;
static CGM_FUNC _cgm_bin_MF_DESC            = &cgm_bin_mtfdsc;
static CGM_FUNC _cgm_bin_VDC_TYPE           = &cgm_bin_vdctyp;
static CGM_FUNC _cgm_bin_INTEGER_PREC       = &cgm_bin_intpre;
static CGM_FUNC _cgm_bin_REAL_PREC          = &cgm_bin_realpr;
static CGM_FUNC _cgm_bin_INDEX_PREC         = &cgm_bin_indpre;
static CGM_FUNC _cgm_bin_COLR_PREC          = &cgm_bin_colpre;
static CGM_FUNC _cgm_bin_COLR_INDEX_PREC    = &cgm_bin_colipr;
static CGM_FUNC _cgm_bin_MAX_COLR_INDEX     = &cgm_bin_maxcoi;
static CGM_FUNC _cgm_bin_COLR_VALUE_EXT     = &cgm_bin_covaex;
static CGM_FUNC _cgm_bin_MF_ELEM_LIST       = &cgm_bin_mtfell;
static CGM_FUNC _cgm_bin_MF_DEFAULTS_RPL    = &cgm_bin_bmtfdf;
static CGM_FUNC _cgm_bin_FONT_LIST          = &cgm_bin_fntlst;
static CGM_FUNC _cgm_bin_CHAR_SET_LIST      = &cgm_bin_chslst;
static CGM_FUNC _cgm_bin_CHAR_CODING        = &cgm_bin_chcdac;
static CGM_FUNC _cgm_bin_MAXIMUM_VDC_EXTENT = &cgm_bin_maxvdcext;

/* picture descriptor elements */

static CGM_FUNC _cgm_bin_SCALE_MODE          = &cgm_bin_sclmde;
static CGM_FUNC _cgm_bin_COLR_MODE           = &cgm_bin_clslmd;
static CGM_FUNC _cgm_bin_LINE_WIDTH_MODE     = &cgm_bin_lnwdmd;
static CGM_FUNC _cgm_bin_MARKER_SIZE_MODE    = &cgm_bin_mkszmd;
static CGM_FUNC _cgm_bin_EDGE_WIDTH_MODE     = &cgm_bin_edwdmd;
static CGM_FUNC _cgm_bin_VDC_EXTENT          = &cgm_bin_vdcext;
static CGM_FUNC _cgm_bin_BACK_COLR           = &cgm_bin_bckcol;
static CGM_FUNC _cgm_bin_INTERIOR_STYLE_MODE = &cgm_bin_intstymode;

/* control elements */

static CGM_FUNC _cgm_bin_VDC_INTEGER_PREC  = &cgm_bin_vdcipr;
static CGM_FUNC _cgm_bin_VDC_REAL_PREC     = &cgm_bin_vdcrpr;
static CGM_FUNC _cgm_bin_AUX_COLR          = &cgm_bin_auxcol;
static CGM_FUNC _cgm_bin_TRANSPARENCY      = &cgm_bin_transp;
static CGM_FUNC _cgm_bin_CLIP_RECT         = &cgm_bin_clprec;
static CGM_FUNC _cgm_bin_CLIP              = &cgm_bin_clpind;
static CGM_FUNC _cgm_bin_PROTECTION_REGION = &cgm_bin_pregionind;
static CGM_FUNC _cgm_bin_MITER_LIMIT       = &cgm_bin_miterlimit;
static CGM_FUNC _cgm_bin_TRANSP_CELL_COLOR = &cgm_bin_transpcellcolor;

/* primitive elements */

static CGM_FUNC _cgm_bin_LINE             = &cgm_bin_polyln;
static CGM_FUNC _cgm_bin_DISJT_LINE       = &cgm_bin_djtply;
static CGM_FUNC _cgm_bin_MARKER           = &cgm_bin_polymk;
static CGM_FUNC _cgm_bin_TEXT             = &cgm_bin_text;
static CGM_FUNC _cgm_bin_RESTR_TEXT       = &cgm_bin_rsttxt;
static CGM_FUNC _cgm_bin_APND_TEXT        = &cgm_bin_apdtxt;
static CGM_FUNC _cgm_bin_POLYGON          = &cgm_bin_polygn;
static CGM_FUNC _cgm_bin_POLYGON_SET      = &cgm_bin_plgset;
static CGM_FUNC _cgm_bin_CELL_ARRAY       = &cgm_bin_cellar;
static CGM_FUNC _cgm_bin_GDP              = &cgm_bin_gdp;
static CGM_FUNC _cgm_bin_RECT             = &cgm_bin_rect;
static CGM_FUNC _cgm_bin_CIRCLE           = &cgm_bin_circle;
static CGM_FUNC _cgm_bin_ARC_3_PT         = &cgm_bin_circ3p;
static CGM_FUNC _cgm_bin_ARC_3_PT_CLOSE   = &cgm_bin_cir3pc;
static CGM_FUNC _cgm_bin_ARC_CTR          = &cgm_bin_circnt;
static CGM_FUNC _cgm_bin_ARC_CTR_CLOSE    = &cgm_bin_ccntcl;
static CGM_FUNC _cgm_bin_ELLIPSE          = &cgm_bin_ellips;
static CGM_FUNC _cgm_bin_ELLIP_ARC        = &cgm_bin_ellarc;
static CGM_FUNC _cgm_bin_ELLIP_ARC_CLOSE  = &cgm_bin_ellacl;
static CGM_FUNC _cgm_bin_ARC_CTR_REVERSE  = &cgm_bin_circntrev;
static CGM_FUNC _cgm_bin_BEZIER           = &cgm_bin_polybz;

/* attribute elements */

static CGM_FUNC _cgm_bin_LINE_INDEX       = &cgm_bin_lnbdin;
static CGM_FUNC _cgm_bin_LINE_TYPE        = &cgm_bin_lntype;
static CGM_FUNC _cgm_bin_LINE_WIDTH       = &cgm_bin_lnwidt;
static CGM_FUNC _cgm_bin_LINE_COLR        = &cgm_bin_lncolr;
static CGM_FUNC _cgm_bin_MARKER_INDEX     = &cgm_bin_mkbdin;
static CGM_FUNC _cgm_bin_MARKER_TYPE      = &cgm_bin_mktype;
static CGM_FUNC _cgm_bin_MARKER_WIDTH     = &cgm_bin_mksize;
static CGM_FUNC _cgm_bin_MARKER_COLR      = &cgm_bin_mkcolr;
static CGM_FUNC _cgm_bin_TEXT_INDEX       = &cgm_bin_txbdin;
static CGM_FUNC _cgm_bin_TEXT_FONT_INDEX  = &cgm_bin_txftin;
static CGM_FUNC _cgm_bin_TEXT_PREC        = &cgm_bin_txtprc;
static CGM_FUNC _cgm_bin_CHAR_EXPAN       = &cgm_bin_chrexp;
static CGM_FUNC _cgm_bin_CHAR_SPACE       = &cgm_bin_chrspc;
static CGM_FUNC _cgm_bin_TEXT_COLR        = &cgm_bin_txtclr;
static CGM_FUNC _cgm_bin_CHAR_HEIGHT      = &cgm_bin_chrhgt;
static CGM_FUNC _cgm_bin_CHAR_ORI         = &cgm_bin_chrori;
static CGM_FUNC _cgm_bin_TEXT_PATH        = &cgm_bin_txtpat;
static CGM_FUNC _cgm_bin_TEXT_ALIGN       = &cgm_bin_txtali;
static CGM_FUNC _cgm_bin_CHAR_SET_INDEX   = &cgm_bin_chseti;
static CGM_FUNC _cgm_bin_ALT_CHAR_SET     = &cgm_bin_achsti;
static CGM_FUNC _cgm_bin_FILL_INDEX       = &cgm_bin_fillin;
static CGM_FUNC _cgm_bin_INT_STYLE        = &cgm_bin_intsty;
static CGM_FUNC _cgm_bin_FILL_COLR        = &cgm_bin_fillco;
static CGM_FUNC _cgm_bin_HATCH_INDEX      = &cgm_bin_hatind;
static CGM_FUNC _cgm_bin_PAT_INDEX        = &cgm_bin_patind;
static CGM_FUNC _cgm_bin_EDGE_INDEX       = &cgm_bin_edgind;
static CGM_FUNC _cgm_bin_EDGE_TYPE        = &cgm_bin_edgtyp;
static CGM_FUNC _cgm_bin_EDGE_WIDTH       = &cgm_bin_edgwid;
static CGM_FUNC _cgm_bin_EDGE_COLR        = &cgm_bin_edgcol;
static CGM_FUNC _cgm_bin_EDGE_VIS         = &cgm_bin_edgvis;
static CGM_FUNC _cgm_bin_FILL_REF_PT      = &cgm_bin_fillrf;
static CGM_FUNC _cgm_bin_PAT_TABLE        = &cgm_bin_pattab;
static CGM_FUNC _cgm_bin_PAT_SIZE         = &cgm_bin_patsiz;
static CGM_FUNC _cgm_bin_COLR_TABLE       = &cgm_bin_coltab;
static CGM_FUNC _cgm_bin_ASF              = &cgm_bin_asf;
static CGM_FUNC _cgm_bin_LINE_CAP         = &cgm_bin_lncap; 
static CGM_FUNC _cgm_bin_LINE_JOIN        = &cgm_bin_lnjoin;
static CGM_FUNC _cgm_bin_EDGE_CAP         = &cgm_bin_edgcap; 
static CGM_FUNC _cgm_bin_EDGE_JOIN        = &cgm_bin_edgjoin;
static CGM_FUNC _cgm_bin_RESTR_TEXT_TYPE  = &cgm_bin_restrtype;

/* escape elements */

static CGM_FUNC _cgm_bin_ESCAPE           = &cgm_bin_escape;

/* external elements */

static CGM_FUNC _cgm_bin_MESSAGE          = &cgm_bin_messag;
static CGM_FUNC _cgm_bin_APPL_DATA        = &cgm_bin_appdta;

/* segment elements */


static CGM_FUNC *_cgm_bin_delimiter[] = {
      &_cgm_bin_NOOP,
      &_cgm_bin_BEGMF,
      &_cgm_bin_ENDMF,
      &_cgm_bin_BEG_PIC,
      &_cgm_bin_BEG_PIC_BODY,
      &_cgm_bin_END_PIC,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static CGM_FUNC *_cgm_bin_metafile[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_MF_VERSION,
      &_cgm_bin_MF_DESC,
      &_cgm_bin_VDC_TYPE,
      &_cgm_bin_INTEGER_PREC,
      &_cgm_bin_REAL_PREC,
      &_cgm_bin_INDEX_PREC,
      &_cgm_bin_COLR_PREC,
      &_cgm_bin_COLR_INDEX_PREC,
      &_cgm_bin_MAX_COLR_INDEX,
      &_cgm_bin_COLR_VALUE_EXT,
      &_cgm_bin_MF_ELEM_LIST,
      &_cgm_bin_MF_DEFAULTS_RPL,
      &_cgm_bin_FONT_LIST,
      &_cgm_bin_CHAR_SET_LIST,
      &_cgm_bin_CHAR_CODING,
      NULL,
      &_cgm_bin_MAXIMUM_VDC_EXTENT,
      NULL, NULL, NULL, NULL, NULL, NULL };

static CGM_FUNC *_cgm_bin_picture[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_SCALE_MODE,
      &_cgm_bin_COLR_MODE,
      &_cgm_bin_LINE_WIDTH_MODE,
      &_cgm_bin_MARKER_SIZE_MODE,
      &_cgm_bin_EDGE_WIDTH_MODE,
      &_cgm_bin_VDC_EXTENT,
      &_cgm_bin_BACK_COLR,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      &_cgm_bin_INTERIOR_STYLE_MODE,
      NULL, NULL, NULL };

static CGM_FUNC *_cgm_bin_control[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_VDC_INTEGER_PREC,
      &_cgm_bin_VDC_REAL_PREC,
      &_cgm_bin_AUX_COLR,
      &_cgm_bin_TRANSPARENCY,
      &_cgm_bin_CLIP_RECT,
      &_cgm_bin_CLIP,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      &_cgm_bin_PROTECTION_REGION,
      NULL,
      &_cgm_bin_MITER_LIMIT,
      &_cgm_bin_TRANSP_CELL_COLOR };

static CGM_FUNC *_cgm_bin_primitive[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_LINE,
      &_cgm_bin_DISJT_LINE,
      &_cgm_bin_MARKER,
      &_cgm_bin_TEXT,
      &_cgm_bin_RESTR_TEXT,
      &_cgm_bin_APND_TEXT,
      &_cgm_bin_POLYGON,
      &_cgm_bin_POLYGON_SET,
      &_cgm_bin_CELL_ARRAY,
      &_cgm_bin_GDP,
      &_cgm_bin_RECT,
      &_cgm_bin_CIRCLE,
      &_cgm_bin_ARC_3_PT,
      &_cgm_bin_ARC_3_PT_CLOSE,
      &_cgm_bin_ARC_CTR,
      &_cgm_bin_ARC_CTR_CLOSE,
      &_cgm_bin_ELLIPSE,
      &_cgm_bin_ELLIP_ARC,
      &_cgm_bin_ELLIP_ARC_CLOSE,
      &_cgm_bin_ARC_CTR_REVERSE,
      NULL, NULL, NULL, NULL, NULL,
      &_cgm_bin_BEZIER,
      NULL, NULL, NULL };

static CGM_FUNC *_cgm_bin_attributes[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_LINE_INDEX,
      &_cgm_bin_LINE_TYPE,
      &_cgm_bin_LINE_WIDTH,
      &_cgm_bin_LINE_COLR,
      &_cgm_bin_MARKER_INDEX,
      &_cgm_bin_MARKER_TYPE,
      &_cgm_bin_MARKER_WIDTH,
      &_cgm_bin_MARKER_COLR,
      &_cgm_bin_TEXT_INDEX,
      &_cgm_bin_TEXT_FONT_INDEX,
      &_cgm_bin_TEXT_PREC,
      &_cgm_bin_CHAR_EXPAN,
      &_cgm_bin_CHAR_SPACE,
      &_cgm_bin_TEXT_COLR,
      &_cgm_bin_CHAR_HEIGHT,
      &_cgm_bin_CHAR_ORI,
      &_cgm_bin_TEXT_PATH,
      &_cgm_bin_TEXT_ALIGN,
      &_cgm_bin_CHAR_SET_INDEX,
      &_cgm_bin_ALT_CHAR_SET,
      &_cgm_bin_FILL_INDEX,
      &_cgm_bin_INT_STYLE,
      &_cgm_bin_FILL_COLR,
      &_cgm_bin_HATCH_INDEX,
      &_cgm_bin_PAT_INDEX,
      &_cgm_bin_EDGE_INDEX,
      &_cgm_bin_EDGE_TYPE,
      &_cgm_bin_EDGE_WIDTH,
      &_cgm_bin_EDGE_COLR,
      &_cgm_bin_EDGE_VIS,
      &_cgm_bin_FILL_REF_PT,
      &_cgm_bin_PAT_TABLE,
      &_cgm_bin_PAT_SIZE,
      &_cgm_bin_COLR_TABLE,
      &_cgm_bin_ASF,
      NULL,
      &_cgm_bin_LINE_CAP, 
      &_cgm_bin_LINE_JOIN,
      NULL, NULL, NULL,
      &_cgm_bin_RESTR_TEXT_TYPE,
      NULL, 
      &_cgm_bin_EDGE_CAP, 
      &_cgm_bin_EDGE_JOIN,
      NULL, NULL, NULL, NULL, NULL, NULL };

static CGM_FUNC *_cgm_bin_escape[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_ESCAPE,
      NULL };

static CGM_FUNC *_cgm_bin_external[] = {
      &_cgm_bin_NULL,
      &_cgm_bin_MESSAGE,
      &_cgm_bin_APPL_DATA,
      NULL };

static CGM_FUNC *_cgm_bin_segment[] = {
      &_cgm_bin_NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static CGM_FUNC **_cgm_bin_commands[] = {
       _cgm_bin_delimiter,
       _cgm_bin_metafile,
       _cgm_bin_picture,
       _cgm_bin_control,
       _cgm_bin_primitive,
       _cgm_bin_attributes,
       _cgm_bin_escape,
       _cgm_bin_external,
       _cgm_bin_segment,
       NULL };

static int cgm_bin_exec_command(tCGM* cgm, int classe, int id)
{
  if (_cgm_bin_commands[classe][id]==NULL)
  {
    cgm->buff.bc = cgm->buff.len;
    return CGM_OK;
  }

  return (*_cgm_bin_commands[classe][id])(cgm);
}

int cgm_bin_rch(tCGM* cgm)
{
  int c, id, len, cont, i; 
  unsigned char ch[2], dummy;
  unsigned short b;
  int ret;

  if(cgm->buff.bc!=cgm->buff.len)
    return CGM_ERR_READ;

  cgm->buff.bc = 0;

  ret = fread(ch, 1, 2, cgm->fp);
  if(ret<2) 
  {
    if (feof(cgm->fp))
      return CGM_OK;
    else
      return CGM_ERR_READ;
  }

  b =(ch[0] << 8) + ch[1];
  len = b & 0x001F;
  id =(b & 0x0FE0) >> 5;
  c =(b & 0xF000) >> 12;

  cont = 0;

  if(len > 30)
  {
    ret = fread(ch, 1, 2, cgm->fp);
    if(ret<2) 
      return CGM_ERR_READ;

    b =(ch[0] << 8) + ch[1];

    len = b & 0x7FFF;
    cont =(b & 0x8000);
  }

  cgm->buff.len = len;

  if(cgm->buff.len)
  {
    if (cgm->buff.len>cgm->buff.size)
    {
      cgm->buff.data = (char *)realloc(cgm->buff.data, cgm->buff.len);
      cgm->buff.size = cgm->buff.len;
    }

    ret = fread(cgm->buff.data, 1, cgm->buff.len, cgm->fp);
    if(ret<cgm->buff.len) 
      return CGM_ERR_READ;

    if (len & 1)
    {
      ret = fread(&dummy, 1, 1, cgm->fp);
      if(ret<1) 
        return CGM_ERR_READ;
    }

    while(cont)
    {
      unsigned char ch[2];
      unsigned short b;
      int old_len = cgm->buff.len;

      ret = fread(ch, 1, 2, cgm->fp);
      if(ret<2) 
        return CGM_ERR_READ;

      b =(ch[0] << 8) + ch[1];

      cont =(b & 0x8000);

      len = b & 0x7fff;

      cgm->buff.len += len;

      if (cgm->buff.len>cgm->buff.size)
      {
        cgm->buff.data = (char *)realloc(cgm->buff.data, cgm->buff.len);
        cgm->buff.size = cgm->buff.len;
      }

      ret = fread(&cgm->buff.data[old_len], 1, len, cgm->fp);
      if(ret<len) 
        return CGM_ERR_READ;

      if(len & 1)
      {
        ret = fread(&dummy, 1, 1, cgm->fp);
        if(ret<1) 
          return CGM_ERR_READ;
      }
    }
  }

  if (cgm_inccounter(cgm))
    return CGM_ABORT_COUNTER;

  ret = cgm_bin_exec_command(cgm, c, id);
  if (ret != CGM_OK) 
    return ret;

  for(i=0; i<cgm->buff.len-cgm->buff.bc; i++)
  {
    unsigned char dummy;
    if(cgm_bin_get_c(cgm, &dummy)) 
      return CGM_ERR_READ;
  }

  if (feof(cgm->fp))
    return CGM_OK;
  else
    return CGM_CONT;
}
