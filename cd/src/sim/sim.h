/** \file
 * \brief Simulation Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __SIM_H
#define __SIM_H


struct _cdSimulation
{
  cdTT_Text* tt_text; /* TrueType Font Simulation using FreeType library */

  int antialias, txt_antialias;

  cdCanvas *canvas;

  const char* font_map[100];
  int font_map_n;

  /* horizontal line draw functions */
  void (*SolidLine)(cdCanvas* canvas, int xmin, int y, int xmax, long color);
  void (*PatternLine)(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const long *pattern);
  void (*StippleLine)(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const unsigned char *stipple);
  void (*HatchLine)(cdCanvas* canvas, int xmin, int xmax, int y, unsigned char hatch);
};

#define simRotateHatchN(_x,_n) ((_x) = ((_x) << (_n)) | ((_x) >> (8-(_n))))

void simFillDrawAAPixel(cdCanvas *canvas, int x, int y, unsigned short alpha_weight);
void simFillHorizLine(cdSimulation* simulation, int xmin, int y, int xmax);
void simFillHorizBox(cdSimulation* simulation, int xmin, int xmax, int ymin, int ymax);
void simGetPenPos(cdCanvas* canvas, int x, int y, const char* s, int len, FT_Matrix *matrix, FT_Vector *pen);
int simIsPointInPolyWind(cdPoint* poly, int n, int x, int y);


typedef struct _simLineSegment simLineSegment;

simLineSegment* simLineSegmentArrayCreate(int n);
int simLineSegmentArrayFindHorizontalIntervals(simLineSegment *segments, int n_seg, int* xx, int *hh, int y, int height);
void simLineSegmentArrayMakeAll(simLineSegment *segments, int *n_seg, cdPoint* poly, int n, int *max_hh, int *y_max, int *y_min);

#endif

