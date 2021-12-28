#include <stdio.h>    

#include "cgm_play.h"
#include "cgm_list.h"

#ifndef _CGM_TYPES_H_
#define _CGM_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int(*CGM_FUNC)(tCGM* cgm);

typedef struct { 
  double xmin;
  double xmax;
  double ymin;
  double ymax;
} tLimit;

typedef struct {
  unsigned long red;
  unsigned long green;
  unsigned long blue;
} tRGB;

typedef union {
  unsigned long index;
  tRGB rgb;
} tColor;

typedef struct {
  char *data;
  int size;  /* allocated size */
  int len;   /* used size */
  int bc;    /* byte count */
  int pc;    /* pixel count */
} tData;

typedef struct {
  const char *name;
	CGM_FUNC func;
} tCommand;

typedef struct {
  long index;
  long type;
  long linecap;
  long dashcap;  /* unused */
  long linejoin;
  double width;
  tColor color;
} tLineAtt;

typedef struct {
  long index;
  long type;
  long linecap;
  long dashcap;  /* unused */
  long linejoin;
  double width;
  tColor color;
  short visibility;
} tEdgeAtt;

typedef struct {
  long index;
  long type;
  double size;
  tColor color;
} tMarkerAtt;

typedef struct {
  long index;
  long font_index;
  tList *font_list;
  short prec;          /* unused */
  double exp_fact;
  double char_spacing; /* unused */
  tColor color;
  double height;
  cgmPoint char_up;    /* unused */
  cgmPoint char_base; 
  short path;          
  long restr_type;  /* unused */
  struct { 
    short hor;
    short ver;
    double cont_hor;   /* unused */
    double cont_ver;   /* unused */
  } alignment;
} tTextAtt;

typedef struct {
  long index;
  short int_style;
  tColor color;
  long hatch_index;
  long pat_index;
  tList *pat_list;
  cgmPoint ref_pt;    /* unused */
  struct {
    cgmPoint height;  /* unused */
    cgmPoint width;   /* unused */
  } pat_size;         /* unused */
} tFillAtt;

typedef struct {
  long index;
  long nx, ny;
  tColor *pattern;
} tPatTable;

typedef struct {
  short type;
  short value;
} tASF;

struct _tCGM {
  FILE *fp;
  int file_size;

  tData buff;

  union  {
    long b_prec;  /* 0=8, 1=16, 2=24, 3=32 */
    struct { long minint; long maxint; } t_prec ;
  } int_prec;
  union {
    long b_prec;  /* 0=float*32, 1=float*64(double), 2=fixed*32, 3=fixed*64 */
    struct { double minreal; double maxreal; long digits; } t_prec;
  } real_prec;
  union {
    long b_prec;  /* 0=8, 1=16, 2=24, 3=32 */
    struct { long minint; long maxint; } t_prec;
  } ix_prec;
  long cd_prec;   /* used only for binary */
  long cix_prec;  /* used only for binary */ 

  struct {
    tRGB black;
    tRGB white;
  } color_ext;
  long max_cix;
  tRGB* color_table;

  struct {
    short mode;    /* abstract (pixels, factor is always 1), 
                      metric (coordinate * factor = coordinate_mm) */
    double factor;
  } scale_mode;

  short color_mode;       /* indexed, direct */
  short linewidth_mode;   /* absolute, scaled, fractional, mm */
  short markersize_mode;  /* absolute, scaled, fractional, mm */
  short edgewidth_mode;   /* absolute, scaled, fractional, mm */
  short interiorstyle_mode;  /* absolute, scaled, fractional, mm (unused) */

  short vdc_type;  /* integer, real */
  union {
    long b_prec;   /* 0=8, 1=16, 2=24, 3=32 */
    struct { long minint; long maxint; } t_prec;
  } vdc_int;
  union {
    long b_prec;   /* 0=float*32, 1=float*64(double), 2=fixed*32, 3=fixed*64 */
    struct { double minreal; double maxreal; long digits; } t_prec;
  } vdc_real;

  struct {
    cgmPoint first;    /* lower-left corner  */
    cgmPoint second;   /* upper-right corner */
    cgmPoint maxFirst;    /* maximum lower-left  corner */
    cgmPoint maxSecond;   /* maximum upper-right corner */
    short has_max;
  } vdc_ext;

  tRGB back_color;
  tColor aux_color;
  short transparency;

  short cell_transp;  /* (affects cellarray and pattern) unused */
  tColor cell_color;  /* unused */

  struct {
    cgmPoint first;
    cgmPoint second;
  } clip_rect;
  short clip_ind;
  long region_idx;  /* unused */
  long region_ind;  /* unused */

  long gdp_sample_type, 
       gdp_n_samples;
  cgmPoint gdp_pt[4];
  char* gdp_data_rec;

  double mitrelimit;  /* unused */

  tLineAtt line_att;
  tMarkerAtt marker_att;
  tTextAtt text_att;
  tFillAtt fill_att;
  tEdgeAtt edge_att;

  tList* asf_list;  /* unused */

  cgmPoint* point_list;
  int point_list_n;

  cgmPlayFuncs dof;
  void* userdata;
};

#define CGM_CONT 3

enum { CGM_OFF, CGM_ON };
enum { CGM_INTEGER, CGM_REAL };
enum { CGM_STRING, CGM_CHAR, CGM_STROKE };
enum { CGM_ABSOLUTE, CGM_SCALED, CGM_FRACTIONAL, CGM_MM };
enum { CGM_ABSTRACT, CGM_METRIC };
enum { CGM_INDEXED, CGM_DIRECT };
enum { CGM_HOLLOW, CGM_SOLID, CGM_PATTERN, CGM_HATCH, CGM_EMPTY, CGM_GEOPAT, CGM_INTERP };
enum { CGM_INVISIBLE, CGM_VISIBLE, CGM_CLOSE_INVISIBLE, CGM_CLOSE_VISIBLE };
enum { CGM_PATH_RIGHT, CGM_PATH_LEFT, CGM_PATH_UP, CGM_PATH_DOWN };

int cgm_bin_rch(tCGM* cgm);
int cgm_txt_rch(tCGM* cgm);

void cgm_strupper(char *s);

void cgm_calc_arc_3p(cgmPoint start, cgmPoint intermediate, cgmPoint end, 
                    cgmPoint *center, double *radius, double *angle1, double *angle2);
void cgm_calc_arc(cgmPoint start, cgmPoint end, 
                  double *angle1, double *angle2);
void cgm_calc_arc_rev(cgmPoint start, cgmPoint end, 
                      double *angle1, double *angle2);
void cgm_calc_ellipse(cgmPoint center, cgmPoint first, cgmPoint second, cgmPoint start, cgmPoint end, 
                      double *angle1, double *angle2);

cgmRGB cgm_getcolor(tCGM* cgm, tColor color);
cgmRGB cgm_getrgb(tCGM* cgm, tRGB rgb);
void   cgm_getcolor_ar(tCGM* cgm, tColor color, 
                       unsigned char *r, unsigned char *g, unsigned char *b);

void cgm_setmarker_attrib(tCGM* cgm);
void cgm_setline_attrib(tCGM* cgm);
void cgm_setedge_attrib(tCGM* cgm);
void cgm_setfill_attrib(tCGM* cgm);
void cgm_settext_attrib(tCGM* cgm);

int cgm_inccounter(tCGM* cgm);
void cgm_polygonset(tCGM* cgm, int np, cgmPoint* pt, short* flags);
void cgm_generalizeddrawingprimitive(tCGM* cgm, int id, int np, cgmPoint* pt, const char* data_rec);

void cgm_sism5(tCGM* cgm, const char *data_rec);


#ifdef __cplusplus
}
#endif

#endif
