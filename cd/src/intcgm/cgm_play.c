#include <stdlib.h>   
#include <string.h>   
#include <math.h>     
#include <ctype.h>

#include "cgm_types.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void solve2(double m[][2], double *b, double *x)
{
  double det;

  det = m[0][0]*m[1][1] - m[0][1]*m[1][0];

  if (det==0) 
    return;

  x[0] = ( b[0]*m[1][1] - b[1]*m[1][0] ) / det;
  x[1] = ( m[0][0]*b[1] - m[0][1]*b[0] ) / det;
}

static void getcenter(double xs, double ys, double xi, double yi, double xe, double ye,
                      double *xc, double *yc)
{
  double c[2] = {0 , 0};

  double x2, y2, x3, y3, m[2][2], b[2];

  x2 = xi - xs;
  y2 = yi - ys;
  x3 = xe - xs;
  y3 = ye - ys;

  m[0][0] = 2*x2;
  m[1][0] = 2*y2;
  m[0][1] = 2*x3;
  m[1][1] = 2*y3;
  b[0]    = x2*x2 + y2*y2;
  b[1]    = x3*x3 + y3*y3;

  solve2(m, b, c);

  *xc = c[0] + xs;
  *yc = c[1] + ys;
}

void cgm_calc_ellipse(cgmPoint center, cgmPoint first, cgmPoint second, cgmPoint start, cgmPoint end, double *angle1, double *angle2)
{
  /* The arc is drawn from start to end,
     oriented from first to second through the smaller angle. */
  double delta;
  double w = fabs(second.x-first.x);
  double h = fabs(second.y-first.y);
  double angleF = atan2(first.y-center.y, first.x-center.x);
  double angleS = atan2(second.y-center.y, second.x-center.x);

  *angle1 = atan2(start.y*w, start.x*h);
  *angle2 = atan2(end.y*w, end.x*h);

  /* turn all values positive */
  if (angleF < 0) angleF += 2*M_PI;
  if (angleS < 0) angleS += 2*M_PI;

  /* use the smaller angle */
  delta = angleS - angleF;
  if (fabs(delta) > M_PI)
  {
    if (delta>0)
      delta = -(2*M_PI - fabs(delta));
    else
      delta = (2*M_PI - fabs(delta));
  }

  /* sort the angles so they will be in the positive angular direction */
  if (delta < 0)
  {
    double tmp = *angle1;
    *angle1 = *angle2;
    *angle2 = tmp;
  }
}

void cgm_calc_arc(cgmPoint start, cgmPoint end, 
                  double *angle1, double *angle2)
{
  /* The arc is drawn from start to end, 
     in the positive angular direction */
  *angle1 = atan2(start.y, start.x);
  *angle2 = atan2(end.y, end.x);
}

void cgm_calc_arc_rev(cgmPoint start, cgmPoint end, 
                      double *angle1, double *angle2)
{
  /* The arc is drawn from start to end, 
     in the negative angular direction */
  double temp;
  cgm_calc_arc(start, end, angle1, angle2);

  /* if angle2 is greater than angle1 it will be progressively decreased by 2*M_PI until it is less than angle1 */
  while(*angle2 > *angle1)
    *angle2 -= 2*M_PI;

  /* angle1 must be less than angle2 */ 
  temp = *angle1;
  *angle1 = *angle2;
  *angle2 = temp;
}

void cgm_calc_arc_3p(cgmPoint start, cgmPoint intermediate, cgmPoint end, 
                    cgmPoint *center, double *radius, double *angle1, double *angle2)
{
  /* The arc is drawn from the start to the end points, 
     passing by the intermediate point. */
  double angleM;

  getcenter(start.x, start.y, intermediate.x, intermediate.y, end.x, end.y, &(center->x), &(center->y));

  *radius = sqrt((start.x-center->x)*(start.x-center->x) + 
                 (start.y-center->y)*(start.y-center->y));

  *angle1 = atan2(start.y-center->y, start.x-center->x);
   angleM = atan2(intermediate.y-center->y, intermediate.x-center->x);
  *angle2 = atan2(end.y-center->y, end.x-center->x);

  /* turn all values positive */
  if (*angle1 < 0) *angle1 += 2*M_PI;
  if ( angleM < 0)  angleM += 2*M_PI;
  if (*angle2 < 0) *angle2 += 2*M_PI;

  /* sort the angles so they will be in the positive angular direction */
  if (*angle1 > angleM)
  {
    double tmp = *angle1;
    *angle1 = *angle2;
    *angle2 = tmp;
  }
}

static unsigned char get_value(unsigned long value, unsigned long min, unsigned long max)
{
  unsigned long v = ((value-min)*255)/(max-min);
  if (v > 255) v = 255;
  return (unsigned char)v;
}

void cgm_getcolor_ar(tCGM* cgm, tColor color, unsigned char *r, unsigned char *g, unsigned char *b)
{
  tRGB rgb;
  if (cgm->color_mode==CGM_INDEXED)
  {
    if (color.index > (unsigned long)cgm->max_cix)
      rgb = cgm->color_table[0];
    else
      rgb = cgm->color_table[color.index];
  }
  else
    rgb = color.rgb;

  *r = get_value(rgb.red,   cgm->color_ext.black.red,   cgm->color_ext.white.red);
  *g = get_value(rgb.green, cgm->color_ext.black.green, cgm->color_ext.white.green);
  *b = get_value(rgb.blue,  cgm->color_ext.black.blue,  cgm->color_ext.white.blue);
}

cgmRGB cgm_getrgb(tCGM* cgm, tRGB rgb)
{
  cgmRGB c;
  c.red   = get_value(rgb.red,   cgm->color_ext.black.red,   cgm->color_ext.white.red);
  c.green = get_value(rgb.green, cgm->color_ext.black.green, cgm->color_ext.white.green);
  c.blue  = get_value(rgb.blue,  cgm->color_ext.black.blue,  cgm->color_ext.white.blue);
  return c;
}

cgmRGB cgm_getcolor(tCGM* cgm, tColor color)
{
  if (cgm->color_mode==CGM_INDEXED)
  {
    if (color.index > (unsigned long)cgm->max_cix)
      return cgm_getrgb(cgm, cgm->color_table[0]);
    else
      return cgm_getrgb(cgm, cgm->color_table[color.index]);
  }
  else
    return cgm_getrgb(cgm, color.rgb);
}

void cgm_setmarker_attrib(tCGM* cgm)
{
  const char* options[] = {"DOT", "PLUS", "ASTERISK", "CIRCLE", "CROSS" };  /* starts at 1 */
  const char* mode[] = {"ABSOLUTE", "SCALED", "FRACTIONAL", "MM"};
  int op = cgm->marker_att.type-1;
  int opMode = cgm->markersize_mode;

  if (op<0 || op>4) op=0;
  if (opMode<0 || opMode>3) opMode=0;

  cgm->dof.MarkerAttrib(options[op], cgm->marker_att.size, mode[opMode], cgm_getcolor(cgm, cgm->marker_att.color), cgm->userdata);
}

void cgm_setline_attrib(tCGM* cgm)
{
  const char* options[] = {"SOLID", "DASH", "DOT", "DASH_DOT", "DASH_DOT_DOT" };  /* starts at 1 */
  const char* cap[] = {"UNSPECIFIED", "BUTT", "ROUND", "PROJECTING_SQUARE", "TRIANGLE" };  /* starts at 1 */
  const char* join[] = {"UNSPECIFIED", "MITRE", "ROUND", "BEVEL" };  /* starts at 1 */
  const char* mode[] = {"ABSOLUTE", "SCALED", "FRACTIONAL", "MM"};
  int op = cgm->line_att.type-1;
  int opCap = cgm->line_att.linecap-1;
  int opJoin = cgm->line_att.linejoin-1;
  int opMode = cgm->linewidth_mode;

  if (op<0 || op>4) op=0;
  if (opCap<0 || opCap>4) opCap=0;
  if (opJoin<0 || opJoin>3) opJoin=0;
  if (opMode<0 || opMode>3) opMode=0;

  cgm->dof.LineAttrib(options[op], cap[opCap], join[opJoin], cgm->line_att.width, mode[opMode], cgm_getcolor(cgm, cgm->line_att.color), cgm->userdata);
}

void cgm_setedge_attrib(tCGM* cgm)
{
  const char* options[] = {"SOLID", "DASH", "DOT", "DASH_DOT", "DASH_DOT_DOT" };  /* starts at 1 */
  const char* cap[] = {"UNSPECIFIED", "BUTT", "ROUND", "PROJECTING_SQUARE", "TRIANGLE" };  /* starts at 1 */
  const char* join[] = {"UNSPECIFIED", "MITRE", "ROUND", "BEVEL" };  /* starts at 1 */
  const char* mode[] = {"ABSOLUTE", "SCALED", "FRACTIONAL", "MM"};
  int op = cgm->edge_att.type-1;
  int opCap = cgm->edge_att.linecap-1;
  int opJoin = cgm->edge_att.linejoin-1;
  int opMode = cgm->edgewidth_mode;

  if (op<0 || op>4) op=0;
  if (opCap<0 || opCap>4) opCap=0;
  if (opJoin<0 || opJoin>3) opJoin=0;
  if (opMode<0 || opMode>3) opMode=0;

  cgm->dof.LineAttrib(options[op], cap[opCap], join[opJoin], cgm->edge_att.width, mode[opMode], cgm_getcolor(cgm, cgm->edge_att.color), cgm->userdata);
  cgm->dof.FillAttrib("HOLLOW", cgm_getcolor(cgm, cgm->edge_att.color), NULL, NULL, cgm->userdata);
}

cgmPattern* get_pattern(tCGM* cgm)
{
  int i, count;
  tPatTable *pat = NULL;
  cgmPattern *p;
  
  for (i=1; (pat=(tPatTable *)cgm_list_get(cgm->fill_att.pat_list,i ))!=NULL; i++)
  {
    if (pat->index==cgm->fill_att.pat_index) 
      break;
  }

  if (!pat)
    return NULL;
  
  count = pat->nx*pat->ny;
  p = (cgmPattern*) malloc(sizeof(cgmPattern));
  p->pattern = (cgmRGB*) malloc(count*sizeof(cgmRGB));
  p->w = pat->nx;
  p->h = pat->ny;
  
  for (i=0; i<count; i++)
    p->pattern[i] = cgm_getcolor(cgm, pat->pattern[i]);
  
  return p;
}

void cgm_setfill_attrib(tCGM* cgm)
{
  const char* options[] = {"HOLLOW", "SOLID", "PATTERN", "HATCH"};  /* starts at 0, EMPTY not supported here, GEOPAT and INTERP are not supported at all */
  const char* hatch_options[] = {"HORIZONTAL", "VERTICAL", "POSITIVE_SLOPE", "NEGATIVE_SLOPE", "HV_CROSS", "SLOPE_CROSS"};  /* starts at 1 */
  const char* hatch = NULL;
  cgmPattern* pat = NULL;
  int op;

  if (cgm->fill_att.int_style == CGM_HATCH)
  {
    op = cgm->fill_att.hatch_index-1;
    if (op<0 || op>5) op=0;
    hatch = hatch_options[op];
  }

  op = cgm->fill_att.int_style;
  if (op<0 || op>3) op=0;  /* EMPTY not supported here, GEOPAT and INTERP are not supported at all */

  if (cgm->fill_att.int_style == CGM_PATTERN)
  {
    pat = get_pattern(cgm);
    if (!pat)
      op = CGM_SOLID;
  }
  
  cgm->dof.FillAttrib(options[op], cgm_getcolor(cgm, cgm->fill_att.color), hatch, pat, cgm->userdata);

  if (pat)
  {
    free(pat->pattern);
    free(pat);
  }
}

void cgm_settext_attrib(tCGM* cgm)
{
  const char *hor_options[] = { NULL, "LEFT", "CENTER", "RIGHT", NULL};  /* continuous alignment not supported */
  const char *ver_options[] = { NULL, "TOP", "CAP", "CENTER", "BASELINE", "BOTTOM", NULL};  /* continuous alignment not supported */
  const char* font = (const char *)cgm_list_get(cgm->text_att.font_list, cgm->text_att.font_index);
  int hor_op = cgm->text_att.alignment.hor<0||cgm->text_att.alignment.hor>4? 0: cgm->text_att.alignment.hor;
  int ver_op = cgm->text_att.alignment.ver<0||cgm->text_att.alignment.ver>4? 0: cgm->text_att.alignment.ver;
  const char* hor = hor_options[hor_op];
  const char* ver = ver_options[ver_op];
  double height = cgm->text_att.height;

  /* continuous is handle as normal */

  if (!hor)  /* normal alignment depends on "path" */
  {
    if (cgm->text_att.path == CGM_PATH_RIGHT)
      hor = "LEFT";   /* inverted */
    else if (cgm->text_att.path == CGM_PATH_LEFT)
      hor = "RIGHT";  /* inverted */
    else
      hor = "CENTER";
  }

  if (!ver)  /* normal alignment depends on "path" */
  {
    if (cgm->text_att.path == CGM_PATH_DOWN)
      ver = "TOP";
    else
      ver = "BASELINE";
  }

  /* default text height =
     1/100 of the length of the longest side of the VDC extent */
  if (height == 0)
  {
    double w = fabs(cgm->vdc_ext.second.x-cgm->vdc_ext.first.x);
    double h = fabs(cgm->vdc_ext.second.y-cgm->vdc_ext.first.y);
    if (w > h)
      height = w/100;
    else
      height = h/100;
  }

  if (!font)
    font = "TIMES_ROMAN";

  cgm->dof.TextAttrib(hor, ver, font, height*cgm->text_att.exp_fact, cgm_getcolor(cgm, cgm->text_att.color), cgm->text_att.char_base, cgm->userdata);
}

void cgm_polygonset(tCGM* cgm, int np, cgmPoint* pt, short* flags)
{
  int i;
  int num_closure;
  cgmPoint *n_pt = NULL;

  num_closure = 0;
  for (i=0; i<np; i++)
  {
    if (flags[i]==CGM_CLOSE_VISIBLE || 
        flags[i]==CGM_CLOSE_INVISIBLE)
      num_closure++;
  }

  /* WARNING: holes in polygons are not processed. */

  if (num_closure!=0)
  {
    int set_closure, j;
    cgmPoint closure = {0, 0};

    n_pt =(cgmPoint *) malloc((np+num_closure)*sizeof(cgmPoint));

    set_closure = 1;
    j = 0;
    for (i=0; i<np; i++)
    {
      if (set_closure)
      {
        closure = pt[i];
        set_closure = 0;
      }

      n_pt[j] = pt[i]; j++;

      if (flags[i]==CGM_CLOSE_VISIBLE || flags[i]==CGM_CLOSE_INVISIBLE)
      {
        n_pt[j] = closure; j++;
        set_closure = 1;
      }
    }

    np += num_closure;
    pt = n_pt;
  }

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

  if (n_pt)
    free(n_pt);
}

char* cgm_strdup(const char *str)
{
  if (str)
  {
    int size = strlen(str)+1;
    char *newstr = malloc(size);
    if (newstr) memcpy(newstr, str, size);
    return newstr;
  }
  return NULL;
}

void cgm_generalizeddrawingprimitive(tCGM* cgm, int id, int np, cgmPoint* pt, const char* data_rec)
{
  (void)np;
  if (id==-4)
  {
    cgm->gdp_pt[0] = pt[0];
    cgm->gdp_pt[1] = pt[1];
    cgm->gdp_pt[2] = pt[2];
    cgm->gdp_pt[3] = pt[3];
    if (cgm->gdp_data_rec) free(cgm->gdp_data_rec);
    cgm->gdp_data_rec = cgm_strdup(data_rec);
  }
  else if (id==-5)
    cgm_sism5(cgm, data_rec);
}

void cgm_strupper(char *s)
{
  while(*s)
  {
    *s =(char)toupper(*s);
    s++;
  }
}

static int dummy_func(tCGM* cgm)
{
  (void)cgm;
  return CGM_OK;
}

static void set_funcs(cgmPlayFuncs* dof, cgmPlayFuncs* funcs)
{ 
  dof->BeginMetafile = (void (*)(const char*, void*))dummy_func;
  dof->EndMetafile = (void (*)(void*))dummy_func;
  dof->BeginPicture = (void (*)(const char*, void*))dummy_func;
  dof->BeginPictureBody = (void (*)(void*))dummy_func;
  dof->EndPicture = (void (*)(void*))dummy_func;
  dof->DeviceExtent = (void (*)(cgmPoint*, cgmPoint*, void*))dummy_func;
  dof->ScaleMode = (void (*)(int, double*, void*))dummy_func;
  dof->BackgroundColor = (void (*)(cgmRGB, void*))dummy_func;
  dof->Transparency = (void (*)(int, cgmRGB, void*))dummy_func;
  dof->ClipRectangle = (void (*)(cgmPoint, cgmPoint, void*))dummy_func;
  dof->ClipIndicator = (void (*)(int, void*))dummy_func;
  dof->PolyLine = (void (*)(int, cgmPoint*, void*))dummy_func;
  dof->PolyMarker = (void (*)(int, cgmPoint*, void*))dummy_func;
  dof->Text = (void (*)(const char*, cgmPoint, void*))dummy_func;
  dof->CellArray = (void (*)(cgmPoint, cgmPoint, cgmPoint, int, int, unsigned char*, void*))dummy_func;
  dof->Polygon = (void (*)(int, cgmPoint*, int, void*))dummy_func;
  dof->Rectangle = (void (*)(cgmPoint, cgmPoint, void*))dummy_func;
  dof->Circle = (void (*)(cgmPoint, double, void*))dummy_func;
  dof->CircularArc = (void (*)(cgmPoint, double, double, double, int, void*))dummy_func;
  dof->Ellipse = (void (*)(cgmPoint, cgmPoint, cgmPoint, void*))dummy_func;
  dof->EllipticalArc = (void (*)(cgmPoint, cgmPoint, cgmPoint, double, double, int, void*))dummy_func;
  dof->LineAttrib = (void (*)(const char*, const char*, const char*, double, const char*, cgmRGB, void*))dummy_func;
  dof->MarkerAttrib = (void (*)(const char*, double, const char*, cgmRGB, void*))dummy_func;
  dof->FillAttrib = (void (*)(const char*, cgmRGB, const char*, cgmPattern*, void*))dummy_func;
  dof->TextAttrib = (void (*)(const char*, const char*, const char*, double, cgmRGB, cgmPoint, void*))dummy_func;
  dof->Counter = (int(*)(double, void*))dummy_func;

  if (funcs->BeginMetafile)    dof->BeginMetafile = funcs->BeginMetafile; 
  if (funcs->EndMetafile)      dof->EndMetafile = funcs->EndMetafile; 
  if (funcs->BeginPicture)     dof->BeginPicture = funcs->BeginPicture; 
  if (funcs->BeginPictureBody) dof->BeginPictureBody = funcs->BeginPictureBody; 
  if (funcs->EndPicture)       dof->EndPicture = funcs->EndPicture; 
  if (funcs->DeviceExtent)     dof->DeviceExtent = funcs->DeviceExtent; 
  if (funcs->ScaleMode)        dof->ScaleMode = funcs->ScaleMode; 
  if (funcs->BackgroundColor)  dof->BackgroundColor = funcs->BackgroundColor; 
  if (funcs->Transparency)     dof->Transparency = funcs->Transparency; 
  if (funcs->ClipRectangle)    dof->ClipRectangle = funcs->ClipRectangle; 
  if (funcs->ClipIndicator)    dof->ClipIndicator = funcs->ClipIndicator; 
  if (funcs->PolyLine)         dof->PolyLine = funcs->PolyLine; 
  if (funcs->PolyMarker)       dof->PolyMarker = funcs->PolyMarker; 
  if (funcs->Polygon)          dof->Polygon = funcs->Polygon; 
  if (funcs->Text)             dof->Text = funcs->Text;
  if (funcs->CellArray)        dof->CellArray = funcs->CellArray; 
  if (funcs->Rectangle)        dof->Rectangle = funcs->Rectangle;
  if (funcs->Circle)           dof->Circle = funcs->Circle; 
  if (funcs->CircularArc)      dof->CircularArc = funcs->CircularArc; 
  if (funcs->Ellipse)          dof->Ellipse = funcs->Ellipse; 
  if (funcs->EllipticalArc)    dof->EllipticalArc = funcs->EllipticalArc; 
  if (funcs->LineAttrib)       dof->LineAttrib = funcs->LineAttrib; 
  if (funcs->MarkerAttrib)     dof->MarkerAttrib = funcs->MarkerAttrib; 
  if (funcs->FillAttrib)       dof->FillAttrib = funcs->FillAttrib; 
  if (funcs->TextAttrib)       dof->TextAttrib = funcs->TextAttrib; 
  if (funcs->Counter)          dof->Counter = funcs->Counter;
}

int cgm_inccounter (tCGM* cgm)
{
  return cgm->dof.Counter((ftell(cgm->fp )*100.)/cgm->file_size, cgm->userdata);
}

static FILE* open_cgm(const char *filename, int *mode, int *file_size)
{
  unsigned char ch[2];
  int c, id;
  unsigned short b;

  FILE *fp = fopen(filename, "rb");
  if(!fp)
    return NULL;

  if (fread(ch, 1, 2, fp) != 2)
  {
    fclose(fp);
    return NULL;
  }

  b =(ch[0] << 8) + ch[1];
  id =(b & 0x0FE0) >> 5;
  c =(b & 0xF000) >> 12;

  fseek(fp, 0, SEEK_END);
  *file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if(c==0 && id==1)
    *mode = 1;   /* binary */
  else
  {
    *mode = 2;   /* text */

    fclose(fp);
    fp = fopen(filename, "r");
  }

  return fp;
}

int cgmPlay(const char* filename, void* userdata, cgmPlayFuncs* funcs)
{
  tCGM* cgm;
  FILE* fp;
  int mode, file_size, ret;

  fp = open_cgm(filename, &mode, &file_size);
  if (!fp)
    return CGM_ERR_OPEN;

  cgm = calloc(1, sizeof(tCGM));

  set_funcs(&(cgm->dof), funcs);
  cgm->userdata = userdata;

  cgm->fp = fp;
  cgm->file_size = file_size;

  cgm->dof.Counter(0, cgm->userdata);

  if(mode == 1) /* binary */
  {
    cgm->int_prec.b_prec  = 1;
    cgm->real_prec.b_prec = 2;
    cgm->ix_prec.b_prec   = 1;
    cgm->cd_prec   = 0;
    cgm->cix_prec  = 0;
    cgm->vdc_int.b_prec  = 1;
    cgm->vdc_real.b_prec = 2;
  }
  else
  {
    cgm->int_prec.t_prec.minint  = -32767;
    cgm->int_prec.t_prec.maxint  = 32767;
    cgm->real_prec.t_prec.minreal = -32767;
    cgm->real_prec.t_prec.maxreal = 32767;
    cgm->real_prec.t_prec.digits = 4;
    cgm->ix_prec.t_prec.minint   = 0;
    cgm->ix_prec.t_prec.maxint   = 127;
    cgm->cd_prec   = 127;
    cgm->vdc_int.t_prec.minint  = -32767;
    cgm->vdc_int.t_prec.maxint  = 32767;
    cgm->vdc_real.t_prec.minreal = 0;
    cgm->vdc_real.t_prec.maxreal = 1;
    cgm->vdc_real.t_prec.digits = 4;
  }

  cgm->vdc_type = CGM_INTEGER;
  cgm->scale_mode.mode = CGM_ABSTRACT;
  cgm->scale_mode.factor = 1.;
  cgm->color_mode = CGM_INDEXED;
  cgm->linewidth_mode = CGM_SCALED;
  cgm->markersize_mode = CGM_SCALED;
  cgm->edgewidth_mode = CGM_SCALED;
  cgm->interiorstyle_mode = CGM_SCALED;
  cgm->vdc_ext.first.x = 0;
  cgm->vdc_ext.first.y = 0;
  cgm->vdc_ext.second.x = 32767;
  cgm->vdc_ext.second.y = 32767;
  cgm->vdc_ext.maxFirst.x = cgm->vdc_ext.first.x;
  cgm->vdc_ext.maxFirst.y = cgm->vdc_ext.first.y;
  cgm->vdc_ext.maxSecond.x = cgm->vdc_ext.second.x;
  cgm->vdc_ext.maxSecond.y = cgm->vdc_ext.second.y;
  cgm->back_color.red = 0;
  cgm->back_color.green = 0;
  cgm->back_color.blue = 0;
  cgm->color_ext.black.red = 0;
  cgm->color_ext.black.green = 0;
  cgm->color_ext.black.blue = 0;
  cgm->color_ext.white.red = 255;
  cgm->color_ext.white.green = 255;
  cgm->color_ext.white.blue = 255;
  cgm->transparency = CGM_ON;
  cgm->clip_rect.first.x = 0;
  cgm->clip_rect.first.y = 0;
  cgm->clip_rect.second.x = 32767;
  cgm->clip_rect.second.y = 32767;
  cgm->clip_ind = CGM_ON;
  cgm->cell_transp = CGM_OFF;
  cgm->region_idx = 1;
  cgm->region_ind = 1;
  cgm->mitrelimit = 32767;

  cgm->point_list_n = 500;
  cgm->point_list =(cgmPoint *) malloc(sizeof(cgmPoint)*cgm->point_list_n);

  cgm->buff.len = 0;
  cgm->buff.size = 1024;
  cgm->buff.data =(char *) malloc(sizeof(char) * cgm->buff.size);
  cgm->buff.bc = 0;
  cgm->buff.pc = 0;

  cgm->line_att.index = 1;
  cgm->line_att.type = 1;
  cgm->line_att.linecap = 1;
  cgm->line_att.dashcap = 1;
  cgm->line_att.linejoin = 1;
  cgm->line_att.width = 1;
  cgm->line_att.color.index = 1;

  cgm->edge_att.index = 1;
  cgm->edge_att.type  = 1;
  cgm->edge_att.linecap = 1;
  cgm->edge_att.dashcap = 1;
  cgm->edge_att.linejoin = 1;
  cgm->edge_att.width = 1;
  cgm->edge_att.color.index = 1;
  cgm->edge_att.visibility = CGM_OFF;

  cgm->marker_att.index = 1;
  cgm->marker_att.type = 1;
  cgm->marker_att.size = 1;
  cgm->marker_att.color.index = 1;

  cgm->text_att.index = 1;
  cgm->text_att.font_index = 1;
  cgm->text_att.font_list = NULL;
  cgm->text_att.prec = CGM_STRING;
  cgm->text_att.exp_fact = 1;
  cgm->text_att.char_spacing = 0;
  cgm->text_att.color.index = 1;
  cgm->text_att.height = 0;
  cgm->text_att.char_up.x = 0;
  cgm->text_att.char_up.y = 1;
  cgm->text_att.char_base.x = 1;
  cgm->text_att.char_base.y = 0;
  cgm->text_att.path = 0;  /* RIGHT */
  cgm->text_att.restr_type = 1;

  cgm->fill_att.index = 1;
  cgm->fill_att.int_style = 0;
  cgm->fill_att.color.index = 1;
  cgm->fill_att.hatch_index = 1;
  cgm->fill_att.pat_index = 1;
  cgm->fill_att.ref_pt.x = 0;
  cgm->fill_att.ref_pt.y = 0;
  cgm->fill_att.pat_list = NULL;
  cgm->fill_att.pat_size.height.x = 0;
  cgm->fill_att.pat_size.height.y = 0;
  cgm->fill_att.pat_size.height.x = 0;
  cgm->fill_att.pat_size.width.y = 0;

  cgm->max_cix = 63;
  cgm->color_table =(tRGB *) malloc(sizeof(tRGB)*(cgm->max_cix+1));
  cgm->color_table[0].red   = 255;
  cgm->color_table[0].green = 255;
  cgm->color_table[0].blue  = 255;
  cgm->color_table[1].red   = 0;
  cgm->color_table[1].green = 0;
  cgm->color_table[1].blue  = 0;

  if(mode == 1)
  {
    do
    {
      ret = cgm_bin_rch(cgm);  /* binary */
    } while(ret==CGM_CONT);  
  }
  else
  {
    do
    {
      ret = cgm_txt_rch(cgm);  /* text */
    } while(ret==CGM_CONT);  
  }

  if(cgm->point_list)
    free(cgm->point_list);

  if(cgm->buff.data)
    free(cgm->buff.data);

  if(cgm->color_table)
    free(cgm->color_table);

  if(cgm->fill_att.pat_list)
  {
    int i;
    tPatTable* p;

    for(i=1; (p=(tPatTable *)cgm_list_get(cgm->fill_att.pat_list,i))!=NULL; i++)
      free(p->pattern);

    cgm_list_delete(cgm->fill_att.pat_list);
  }

  if(cgm->text_att.font_list)
    cgm_list_delete(cgm->text_att.font_list);

  if(cgm->asf_list)
    cgm_list_delete(cgm->asf_list);

  if (cgm->gdp_data_rec) 
    free(cgm->gdp_data_rec);

  cgm->dof.Counter(100., cgm->userdata);

  fclose(cgm->fp);
  free(cgm);

  return ret;
}
