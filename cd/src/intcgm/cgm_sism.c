#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#include "cgm_types.h"

/* WARNING: don't know exactly why all this is implemented with this logic.
            don't know what "sism" means.
*/

typedef struct { 
  cgmPoint trace_start_pos, 
           base_direction, 
           amp_direction, 
           trace_direction;
  double bline_sc_f, 
         amp_sc_f, 
         trc_st_f, 
         pos_bckfil_bnd, 
         neg_bckfil_bnd;
  int trace,
      n_samp;
} tSism;

/* adjust the samples to the amplitude direction vector */
static void dirvet(double *sample, int n_samp, double dx)
{
  int i;
  for ( i=0; i<n_samp; i++ )
    sample[i] *= dx/fabs(dx);
}

/* adust the x offset from the baseline to maximum amplitude */
static void ampscf (double *sample, int n_samp, int samp_type, double dx, double amp_sc_f)
{
  double max=1;
  int i;

  if ( samp_type == 0 || samp_type == 4 ) max = (double) SHRT_MAX;
  else if ( samp_type == 1 ) max = (double) INT_MAX;
  else if ( samp_type == 2 ) max = (double) FLT_MAX;
  else if ( samp_type == 3 || samp_type == 5 ) max = (double) SCHAR_MAX;

  for ( i=0; i<n_samp; i++ )
    sample[i] = (sample[i]/max) * amp_sc_f * dx;
}

static double interpl (double *sample, int i, double y1, double y2, double x )
{
  return ( y1*(x-sample[i+1]) + y2*(sample[i]-x) ) / ( sample[i]-sample[i+1] );
}

static void vasamp (tCGM* cgm, double *sample, int mode, double max, double min, long *coind, tSism* sism)
{
  double factx, facty;
  double samp1, samp2, mn, mx;
  int i,n;
  cgmPoint pt[5];

  facty = sism->base_direction.y * sism->bline_sc_f;
  factx = sism->trace_direction.x * sism->trc_st_f;

  max *= sism->amp_sc_f;
  min *= sism->amp_sc_f;
  mn = min;
  mx = max;

  if ( mode==64 )
  {
    double tmp = min;
    min = max;
    max = tmp;
    mn = min * -1.;
    mx = max * -1.;
  }

  for ( i=0; i<(sism->n_samp-1); i++ )
  {
    double y1 = sism->trace_start_pos.y + facty*(i);
    double y2 = sism->trace_start_pos.y + facty*(i+1);
    double dx = sism->trace_start_pos.x + factx*(sism->trace-1);

    samp1 = sample[i];
    samp2 = sample[i+1];

    if ( mode==64 )
    {
      samp1 *= -1.;
      samp2 *= -1.;
    }

    n=0;

    if (samp1<mn && samp2>mn && samp2<mx)
    {
      pt[n].x   = min + dx;
      pt[n].y = interpl(sample, i,y1,y2,min);
      n++;
      pt[n].x   = sample[i+1] + dx;
      pt[n].y = y2;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = y2;
      n++;
    }
    else if (samp1<mn && samp2>mx)
    {
      pt[n].x   = min + dx;
      pt[n].y = interpl(sample, i,y1,y2,min);
      n++;
      pt[n].x   = max + dx;
      pt[n].y = interpl(sample, i,y1,y2,max);
      n++;
      pt[n].x   = max + dx;
      pt[n].y = y2;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = y2;
      n++;
    }
    else if ( (samp1>mn && samp1<mx) &&
              (samp2>mn && samp2<mx) )
    {
      pt[n].x   = min + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = sample[i] + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = sample[i+1] + dx;
      pt[n].y = y2;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = y2;
      n++;
    }
    else if ( samp1>mn && samp1<mx && samp2>mx )
    {
      pt[n].x   = min + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = sample[i] + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = max + dx;
      pt[n].y = interpl(sample, i,y1,y2,max);
      n++;
      pt[n].x   = max + dx;
      pt[n].y = y2;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = y2;
      n++;
    }
    else if ( samp1>mx && samp2>mx )
    {
      pt[n].x   = min + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = max + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = max + dx;
      pt[n].y = y2;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = y2;
      n++;
    }
    else if ( samp1>mx && samp2<mx && samp2>mn )
    {
      pt[n].x   = min + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = max + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = max + dx;
      pt[n].y = interpl(sample, i,y1,y2,max);
      n++;
      pt[n].x   = sample[i+1] + dx;
      pt[n].y = y2;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = y2;
      n++;
    }
    else if ( samp1>mn && samp1<mx && samp2<mn )
    {
      pt[n].x   = min + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = sample[i] + dx;
      pt[n].y = y1;
      n++;
      pt[n].x   = min + dx;
      pt[n].y = interpl(sample, i,y1,y2,min);
      n++;
    }

    if (n>0)
    {
      if (coind)
      {
        int index = (coind[i] <= cgm->max_cix)? coind[i]: 1;
        cgmRGB color = cgm_getrgb(cgm, cgm->color_table[index]);
        cgm->dof.FillAttrib("SOLID", color, 0, NULL, cgm->userdata);
      }

      cgm->dof.Polygon(n, pt, CGM_FILL, cgm->userdata);
    }
  }
}

static void bgclfl (tCGM* cgm, long *coind, tSism* sism)
{
  double factx, facty;
  double posdx, negdx;
  cgmRGB color;
  int i;
  cgmPoint pt[4];

  facty = sism->base_direction.y * sism->bline_sc_f;
  factx = sism->trace_direction.x * sism->trc_st_f;

  posdx = sism->pos_bckfil_bnd * sism->amp_sc_f * sism->amp_direction.x;
  negdx = sism->neg_bckfil_bnd * sism->amp_sc_f * sism->amp_direction.x;

  for ( i=0; i<sism->n_samp; i++ )
  {
    int index = (coind[i] <= cgm->max_cix)? coind[i]: 1;
    color = cgm_getrgb(cgm, cgm->color_table[index]);
    cgm->dof.FillAttrib("SOLID", color, 0, NULL, cgm->userdata);

    pt[0].x = posdx + sism->trace_start_pos.x + factx*(sism->trace-1);
    pt[0].y = sism->trace_start_pos.y + facty*(i);
    pt[1].x = posdx + sism->trace_start_pos.x + factx*(sism->trace-1);
    pt[1].y = sism->trace_start_pos.y + facty*(i+1);
    pt[2].x = negdx + sism->trace_start_pos.x + factx*(sism->trace-1);
    pt[2].y = sism->trace_start_pos.y + facty*(i+1);
    pt[3].x = negdx + sism->trace_start_pos.x + factx*(sism->trace-1);
    pt[3].y = sism->trace_start_pos.y + facty*(i);

    cgm->dof.Polygon(4, pt, CGM_FILL, cgm->userdata);
  }
}

static void wiggle (tCGM* cgm, double *sample, double max, double min, tSism* sism)
{
  int i;
  double facty = sism->base_direction.y * sism->bline_sc_f;
  double factx = sism->trace_direction.x * sism->trc_st_f;
  double dx = sism->trace_start_pos.x + factx*(sism->trace-1);
  cgmPoint *pt;
  int np;

  max *= sism->amp_sc_f;
  min *= sism->amp_sc_f;

  pt = (cgmPoint *)malloc((2*sism->n_samp)*sizeof(cgmPoint));

  np = 0;
  for ( i=0; i<sism->n_samp; i++ )
  {
    double y1 = sism->trace_start_pos.y + facty*(i);
    double y2 = sism->trace_start_pos.y + facty*(i+1);

    if ( sample[i]>min && sample[i]<max && 
         sample[i+1]>min && sample[i+1]<max )
    {
      pt[np].x = sample[i] + dx;
      pt[np].y = y1;
      np++;
    }
    else if ( sample[i]<min && sample[i+1]>min && sample[i+1]<max )
    {
      pt[np].x = min + dx;
      pt[np].y = interpl(sample,i,y1,y2,min);
      np++;
    }
    else if ( sample[i]<min && sample[i+1]>max )
    {
      pt[np].x = min + dx;
      pt[np].y = interpl(sample, i,y1,y2,min);
      np++;
      pt[np].x = max + dx;
      pt[np].y = interpl(sample, i,y1,y2,max);
      np++;
    }
    else if ( sample[i]>min && sample[i]<max && sample[i+1]>max )
    {
      pt[np].x = sample[i] + dx;
      pt[np].y = y1;
      np++;
      pt[np].x = max + dx;
      pt[np].y = interpl(sample, i,y1,y2,max);
      np++;
    }
    else if ( sample[i]>max && sample[i+1]<max && sample[i+1]>min )
    {
      pt[np].x = max + dx;
      pt[np].y = interpl(sample, i,y1,y2,max);
      np++;
    }
    else if ( sample[i]>max && sample[i+1]<min )
    {
      pt[np].x = max + dx;
      pt[np].y = interpl(sample, i,y1,y2,max);
      np++;
      pt[np].x = min + dx;
      pt[np].y = interpl(sample, i,y1,y2,min);
      np++;
    }
    else if ( sample[i]>min && sample[i]<max && sample[i+1]<min )
    {
      pt[np].x = sample[i] + dx;
      pt[np].y = y1;
      np++;
      pt[np].x = min + dx;
      pt[np].y = interpl(sample, i,y1,y2,min);
      np++;
    }
  }

  cgm->dof.Polygon(np, pt, CGM_LINES, cgm->userdata);
  free(pt);
}

void cgm_sism5(tCGM* cgm, const char *data_rec)
{
  double VA_bline_offset, pos_clp_lmt, neg_clp_lmt, pos_clp, neg_clp;
  int trace_mode, trace_dsp_md, samp_type, wig_trc_mod, nul_clr_i, n_trace;
  int i, mode = 0;
  double *sample;
  char* s;
  long *coind = NULL;
  tSism sism;

  sscanf(cgm->gdp_data_rec, "%lg %lg %lg %lg %lg %lg %d %d %d %lg %lg %*d %d %d %d",
         &(sism.bline_sc_f), &(sism.amp_sc_f), &(sism.trc_st_f), &VA_bline_offset, &pos_clp_lmt,
         &(neg_clp_lmt), &trace_dsp_md, &samp_type, &(sism.n_samp), &(sism.pos_bckfil_bnd), 
         &(sism.neg_bckfil_bnd), &wig_trc_mod, &nul_clr_i, &n_trace);

  sism.trace_start_pos = cgm->gdp_pt[0];
  sism.base_direction = cgm->gdp_pt[1];
  sism.amp_direction = cgm->gdp_pt[2];
  sism.trace_direction = cgm->gdp_pt[3];
  sism.trace = 0;

  sample = (double *) malloc(sism.n_samp*sizeof(double));

  if ( trace_dsp_md > 7 )
    coind = (long *) malloc(sism.n_samp*sizeof(long));

  s = (char*)data_rec;
  for ( i=0; i<sism.n_samp; i++ )
  {
    if ( trace_dsp_md < 8 )
      sample[i] = strtod(s, &s);
    else
    {
      sample[i] = (double)strtol(s, &s, 10 );
      coind[i] = strtol(s, &s, 10 );
    }
  }

  sism.trace += 1;

  dirvet(sample, sism.n_samp, sism.amp_direction.x);

  ampscf(sample, sism.n_samp, samp_type, sism.amp_direction.x, sism.amp_sc_f);

  trace_mode = trace_dsp_md;

  do
  {
    if ( trace_mode >= 64 )
      mode = 64;
    else if ( trace_mode >= 32 )
      mode = 32;
    else if ( trace_mode >= 16 )
      mode = 16;
    else if ( trace_mode >= 8 )
      mode = 8;
    else if ( trace_mode >= 4 )
      mode = 4;
    else if ( trace_mode >= 2 )
      mode = 2;
    else if ( trace_mode == 1 )
      mode = 1;

    switch ( mode )
    {
    case 64:
      neg_clp = neg_clp_lmt;
      pos_clp = ( pos_clp_lmt < 0 ) ?  pos_clp_lmt : 0;
      vasamp(cgm, sample, mode, pos_clp, neg_clp, coind, &sism);
      trace_mode -= 64;
      break;
    case 32:
      neg_clp = ( neg_clp_lmt > 0 ) ? neg_clp_lmt : 0;
      pos_clp = pos_clp_lmt;
      vasamp(cgm, sample, mode, pos_clp, neg_clp, coind, &sism);
      trace_mode -= 32;
      break;
    case 16:
      bgclfl(cgm, coind, &sism);
      trace_mode -= 16;
      break;
    case  8:
      bgclfl(cgm, coind, &sism);
      trace_mode -= 8;
      break;
    case  4:
      neg_clp = ( neg_clp_lmt > 0 ) ? neg_clp_lmt : 0;
      pos_clp = pos_clp_lmt;
      vasamp(cgm, sample, mode, pos_clp, neg_clp, NULL, &sism);
      trace_mode -= 4;
      break;
    case  2:
      neg_clp = ( neg_clp_lmt > 0 ) ? neg_clp_lmt : 0;
      pos_clp = pos_clp_lmt;
      vasamp(cgm, sample, mode, pos_clp, neg_clp, NULL, &sism);
      trace_mode -= 2;
      break;
    case  1:
      neg_clp = neg_clp_lmt;
      pos_clp = pos_clp_lmt;
      wiggle (cgm, sample, pos_clp, neg_clp, &sism);
      trace_mode -= 1;
      break;
    }
  } while ( trace_mode != 0 );

  free(sample);

  if ( trace_dsp_md > 7 )
    free(coind);
}
