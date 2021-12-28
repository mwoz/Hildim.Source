#include <math.h>
#include "cd_d2d.h"
#include "cd.h"


#define checkSwapCoord(_c1, _c2) { if (_c1 > _c2) { float t = _c2; _c2 = _c1; _c1 = t; } }   /* make sure _c1 is smaller than _c2 */

void d2dInitColor(dummy_D2D1_COLOR_F* c, long color)
{
  unsigned char red, green, blue, alpha;
  cdDecodeColorAlpha(color, &red, &green, &blue, &alpha);
  c->r = red / 255.0f;
  c->g = green / 255.0f;
  c->b = blue / 255.0f;
  c->a = alpha / 255.0f;
}

dummy_ID2D1PathGeometry* d2dCreateBoxGeometry(double xmin, double xmax, double ymin, double ymax)
{
  HRESULT hr;
  dummy_ID2D1GeometrySink *sink;
  dummy_D2D1_POINT_2F pt;
  dummy_ID2D1PathGeometry* g;

  hr = dummy_ID2D1Factory_CreatePathGeometry(d2d_cd_factory, &g);
  if (FAILED(hr)) {
    return NULL;
  }

  dummy_ID2D1PathGeometry_Open(g, &sink);

  pt.x = type2float(xmin);
  pt.y = type2float(ymin);
  dummy_ID2D1GeometrySink_BeginFigure(sink, pt, dummy_D2D1_FIGURE_BEGIN_FILLED);

  pt.x = type2float(xmax);
  pt.y = type2float(ymin);
  dummy_ID2D1GeometrySink_AddLine(sink, pt);

  pt.x = type2float(xmax);
  pt.y = type2float(ymax);
  dummy_ID2D1GeometrySink_AddLine(sink, pt);

  pt.x = type2float(xmin);
  pt.y = type2float(ymax);
  dummy_ID2D1GeometrySink_AddLine(sink, pt);

  dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);

  dummy_ID2D1GeometrySink_Close(sink);
  dummy_ID2D1GeometrySink_Release(sink);

  return g;
}

dummy_ID2D1PathGeometry* d2dCreateArcGeometry(float cx, float cy, float rx, float ry, float base_angle, float sweep_angle, int pie)
{
  dummy_ID2D1PathGeometry* g = NULL;
  dummy_ID2D1GeometrySink* s;
  HRESULT hr;
  float base_rads = base_angle * (PI / 180.0f);
  dummy_D2D1_POINT_2F pt;
  dummy_D2D1_ARC_SEGMENT arc_seg;

  hr = dummy_ID2D1Factory_CreatePathGeometry(d2d_cd_factory, &g);
  if (FAILED(hr)) {
    return NULL;
  }

  dummy_ID2D1PathGeometry_Open(g, &s);

  pt.x = cx + rx * cosf(base_rads);
  pt.y = cy + ry * sinf(base_rads);
  dummy_ID2D1GeometrySink_BeginFigure(s, pt, dummy_D2D1_FIGURE_BEGIN_FILLED);

  if (sweep_angle == 360.0f)
  {
    d2dInitArcSegment(&arc_seg, cx, cy, rx, ry, base_angle, 180.0f);
    dummy_ID2D1GeometrySink_AddArc(s, &arc_seg);
    d2dInitArcSegment(&arc_seg, cx, cy, rx, ry, base_angle + 180.0f, 180.0f);
    dummy_ID2D1GeometrySink_AddArc(s, &arc_seg);
  }
  else
  {
    d2dInitArcSegment(&arc_seg, cx, cy, rx, ry, base_angle, sweep_angle);
    dummy_ID2D1GeometrySink_AddArc(s, &arc_seg);
  }

  if (pie == 2)
    dummy_ID2D1GeometrySink_EndFigure(s, dummy_D2D1_FIGURE_END_CLOSED);
  else if (pie == 1)
  {
    pt.x = cx;
    pt.y = cy;
    dummy_ID2D1GeometrySink_AddLine(s, pt);
    dummy_ID2D1GeometrySink_EndFigure(s, dummy_D2D1_FIGURE_END_CLOSED);
  }
  else
    dummy_ID2D1GeometrySink_EndFigure(s, dummy_D2D1_FIGURE_END_OPEN);

  dummy_ID2D1GeometrySink_Close(s);
  dummy_ID2D1GeometrySink_Release(s);

  return g;
}

static dummy_ID2D1StrokeStyle* createStrokeStyleDashed(const float* dashes, UINT dashesCount, UINT lineCap, UINT lineJoin)
{
  HRESULT hr;
  dummy_D2D1_STROKE_STYLE_PROPERTIES p;
  dummy_ID2D1StrokeStyle *s;

  p.startCap = (dummy_D2D1_CAP_STYLE)0;
  p.endCap = (dummy_D2D1_CAP_STYLE)lineCap;
  p.dashCap = (dummy_D2D1_CAP_STYLE)lineCap;
  p.lineJoin = (dummy_D2D1_LINE_JOIN)lineJoin;
  p.miterLimit = 1.0f;
  p.dashStyle = dummy_D2D1_DASH_STYLE_CUSTOM;
  p.dashOffset = 0.0f;

  hr = dummy_ID2D1Factory_CreateStrokeStyle(d2d_cd_factory, &p, dashes, dashesCount, &s);
  if (FAILED(hr)) {
    return NULL;
  }

  return s;
}

static dummy_ID2D1StrokeStyle* createStrokeStyle(UINT lineCap, UINT lineJoin)
{
  HRESULT hr;
  dummy_D2D1_STROKE_STYLE_PROPERTIES p;
  dummy_ID2D1StrokeStyle *s;

  p.startCap = (dummy_D2D1_CAP_STYLE)0;
  p.endCap = (dummy_D2D1_CAP_STYLE)lineCap;
  p.dashCap = (dummy_D2D1_CAP_STYLE)lineCap;
  p.lineJoin = (dummy_D2D1_LINE_JOIN)lineJoin;
  p.miterLimit = 1.0f;
  p.dashStyle = dummy_D2D1_DASH_STYLE_SOLID;
  p.dashOffset = 0.0f;

  hr = dummy_ID2D1Factory_CreateStrokeStyle(d2d_cd_factory, &p, NULL, 0, &s);
  if (FAILED(hr)) {
    return NULL;
  }

  return s;
}

dummy_ID2D1StrokeStyle *d2dSetLineStyle(int line_style, int line_cap, int line_join)
{
  if (line_style == CD_DASHED)
  {
    float dashes[2] = { 9.0f, 3.0f };
    return createStrokeStyleDashed(dashes, 2, line_cap, line_join);  /* CD and D2D line cap and line join use the same definitions */
  }
  else if (line_style == CD_DOTTED)
  {
    float dashes[2] = { 1.0f, 2.0f };
    return createStrokeStyleDashed(dashes, 2, line_cap, line_join);
  }
  else if (line_style == CD_DASH_DOT)
  {
    float dashes[4] = { 7.0f, 3.0f, 1.0f, 3.0f };
    return createStrokeStyleDashed(dashes, 4, line_cap, line_join);
  }
  else if (line_style == CD_DASH_DOT_DOT)
  {
    float dashes[6] = { 7.0f, 3.0f, 1.0f, 3.0f, 1.0f, 3.0f };
    return createStrokeStyleDashed(dashes, 6, line_cap, line_join);
  }
  else if (line_cap != CD_CAPFLAT || line_join != CD_MITER)
    return createStrokeStyle(line_cap, line_join);

  return NULL;
}

dummy_ID2D1StrokeStyle *d2dSetCustomLineStyle(int *dashes, int dashes_count, float line_width, int line_cap, int line_join)
{
  int i;
  float *fdashes = (float *)malloc(dashes_count*sizeof(float));
  for (i = 0; i < dashes_count; i++)
    fdashes[i] = (float)dashes[i] / line_width;
  return createStrokeStyleDashed(fdashes, dashes_count, line_cap, line_join);
}

dummy_ID2D1Brush* d2dCreateSolidBrush(dummy_ID2D1RenderTarget *target, long color)
{
  dummy_ID2D1SolidColorBrush* brush;
  dummy_D2D1_COLOR_F clr;
  HRESULT hr;

  d2dInitColor(&clr, color);

  hr = dummy_ID2D1RenderTarget_CreateSolidColorBrush(target, &clr, NULL, &brush);
  if (FAILED(hr)) {
    return NULL;
  }
  return (dummy_ID2D1Brush*)brush;
}

dummy_ID2D1Brush* d2dCreateImageBrush(dummy_ID2D1RenderTarget *target, IWICBitmap* wic_bitmap)
{
  dummy_ID2D1Bitmap *bitmap;
  dummy_ID2D1BitmapBrush* brush;
  dummy_D2D1_BITMAP_BRUSH_PROPERTIES props;
  HRESULT hr;

  hr = dummy_ID2D1RenderTarget_CreateBitmapFromWicBitmap(target, (IWICBitmapSource*)wic_bitmap, NULL, &bitmap);
  if (FAILED(hr))
    return NULL;

  props.extendModeX = dummy_D2D1_EXTEND_MODE_WRAP;
  props.extendModeY = dummy_D2D1_EXTEND_MODE_WRAP;
  props.interpolationMode = dummy_D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

  hr = dummy_ID2D1RenderTarget_CreateBitmapBrush(target, bitmap, &props, NULL, &brush);
  dummy_ID2D1Bitmap_Release(bitmap);

  if (FAILED(hr))
    return NULL;
  
  return (dummy_ID2D1Brush*)brush;
}

dummy_ID2D1Brush* d2dCreateLinearGradientBrush(dummy_ID2D1RenderTarget *target, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, long foreground, long background)
{
  dummy_D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props;
  dummy_D2D1_GRADIENT_STOP stop[2];
  dummy_ID2D1GradientStopCollection *stops;
  dummy_ID2D1LinearGradientBrush *brush;
  dummy_D2D1_COLOR_F color;
  HRESULT hr;

  props.startPoint.x = x1;
  props.startPoint.y = y1;
  props.endPoint.x = x2;
  props.endPoint.y = y2;

  d2dInitColor(&color, foreground);
  stop[0].color = color;
  stop[0].position = 0.0f;

  d2dInitColor(&color, background);
  stop[1].color = color;
  stop[1].position = 1.0f;

  hr = dummy_ID2D1RenderTarget_CreateGradientStopCollection(target, stop, 2, dummy_D2D1_GAMMA_2_2, dummy_D2D1_EXTEND_MODE_WRAP, &stops);
  if (FAILED(hr)) {
    return NULL;
  }

  hr = dummy_ID2D1RenderTarget_CreateLinearGradientBrush(target, &props, NULL, stops, &brush);
  if (FAILED(hr)) {
    dummy_ID2D1GradientStopCollection_Release(stops);
    return NULL;
  }

  dummy_ID2D1GradientStopCollection_Release(stops);
  return (dummy_ID2D1Brush*)brush;
}

dummy_ID2D1Brush* d2dCreateRadialGradientBrush(dummy_ID2D1RenderTarget *target, FLOAT cx, FLOAT cy, FLOAT ox, FLOAT oy, FLOAT rx, FLOAT ry, long foreground, long background)
{
  dummy_D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES props;
  dummy_D2D1_GRADIENT_STOP stop[2];
  dummy_ID2D1GradientStopCollection *stops;
  dummy_ID2D1RadialGradientBrush *brush;
  dummy_D2D1_COLOR_F color;
  HRESULT hr;

  props.center.x = cx;
  props.center.y = cy;
  props.gradientOriginOffset.x = ox;
  props.gradientOriginOffset.y = oy;
  props.radiusX = rx;
  props.radiusY = ry;

  d2dInitColor(&color, foreground);
  stop[0].color = color;
  stop[0].position = 0.0f;

  d2dInitColor(&color, background);
  stop[1].color = color;
  stop[1].position = 1.0f;

  hr = dummy_ID2D1RenderTarget_CreateGradientStopCollection(target, stop, 2, dummy_D2D1_GAMMA_2_2, dummy_D2D1_EXTEND_MODE_WRAP, &stops);
  if (FAILED(hr)) {
    return NULL;
  }

  hr = dummy_ID2D1RenderTarget_CreateRadialGradientBrush(target, &props, NULL, stops, &brush);
  if (FAILED(hr)) {
    dummy_ID2D1GradientStopCollection_Release(stops);
    return NULL;
  }

  dummy_ID2D1GradientStopCollection_Release(stops);
  return (dummy_ID2D1Brush*)brush;
}

static void d2dFillEllipse(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float cx, float cy, float rx, float ry)
{
  dummy_D2D1_ELLIPSE e;

  e.point.x = cx;
  e.point.y = cy;
  e.radiusX = rx;
  e.radiusY = ry;

  dummy_ID2D1RenderTarget_FillEllipse(target, &e, brush);
}

static void d2dFillEllipsePie(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float cx, float cy, float rx, float ry,
                              float fBaseAngle, float fSweepAngle)
{
  dummy_ID2D1PathGeometry* g = d2dCreateArcGeometry(cx, cy, rx, ry, fBaseAngle, fSweepAngle, 1);
  if (g == NULL)
    return;
  dummy_ID2D1RenderTarget_FillGeometry(target, (dummy_ID2D1Geometry*)g, brush, NULL);
  dummy_ID2D1PathGeometry_Release(g);
}

static void d2dFillEllipseChord(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float cx, float cy, float rx, float ry,
                                float fBaseAngle, float fSweepAngle)
{
  dummy_ID2D1PathGeometry* g = d2dCreateArcGeometry(cx, cy, rx, ry, fBaseAngle, fSweepAngle, 2);
  if (g == NULL)
    return;
  dummy_ID2D1RenderTarget_FillGeometry(target, (dummy_ID2D1Geometry*)g, brush, NULL);
  dummy_ID2D1PathGeometry_Release(g);
}

static void d2dDrawEllipse(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float cx, float cy, float rx, float ry,
                           float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_D2D1_ELLIPSE e;

  e.point.x = cx;
  e.point.y = cy;
  e.radiusX = rx;
  e.radiusY = ry;

  dummy_ID2D1RenderTarget_DrawEllipse(target, &e, brush, fStrokeWidth, hStrokeStyle);
}

static void d2dDrawEllipseArc(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float cx, float cy, float rx, float ry,
                              float fBaseAngle, float fSweepAngle, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_ID2D1PathGeometry* g = d2dCreateArcGeometry(cx, cy, rx, ry, fBaseAngle, fSweepAngle, 0);
  if (g == NULL)
    return;
  dummy_ID2D1RenderTarget_DrawGeometry(target, (dummy_ID2D1Geometry*)g, brush, fStrokeWidth, hStrokeStyle);
  dummy_ID2D1PathGeometry_Release(g);
}

void d2dFillRect(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x0, float y0, float x1, float y1)
{
  dummy_D2D1_RECT_F r;

  r.left = x0;
  r.top = y0;
  r.right = x1;
  r.bottom = y1;

  dummy_ID2D1RenderTarget_FillRectangle(target, &r, brush);
}

void d2dDrawRect(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x0, float y0, float x1, float y1, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_D2D1_RECT_F r;

  r.left = x0;
  r.top = y0;
  r.right = x1;
  r.bottom = y1;

  dummy_ID2D1RenderTarget_DrawRectangle(target, &r, brush, fStrokeWidth, hStrokeStyle);
}

void d2dDrawLine(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x0, float y0, float x1, float y1, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_D2D1_POINT_2F pt0;
  dummy_D2D1_POINT_2F pt1;

  pt0.x = x0;
  pt0.y = y0;
  pt1.x = x1;
  pt1.y = y1;

  dummy_ID2D1RenderTarget_DrawLine(target, pt0, pt1, brush, fStrokeWidth, hStrokeStyle);
}

void d2dDrawArc(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float xc, float yc, float w, float h, double a1, double a2, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  float baseAngle = (float)(360.0 - a2);
  float sweepAngle = (float)(a2 - a1);

  if (sweepAngle == 360.0f)
    d2dDrawEllipse(target, brush, xc, yc, w / 2.0f, h / 2.0f, fStrokeWidth, hStrokeStyle);
  else
    d2dDrawEllipseArc(target, brush, xc, yc, w / 2.0f, h / 2.0f, baseAngle, sweepAngle, fStrokeWidth, hStrokeStyle);
}

void d2dFillArc(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float xc, float yc, float w, float h, double a1, double a2, int pie)
{
  float baseAngle = (float)(360.0 - a2);
  float sweepAngle = (float)(a2 - a1);

  if (sweepAngle == 360.0f)
    d2dFillEllipse(target, brush, xc, yc, w / 2.0f, h / 2.0f);
  else if (pie)
    d2dFillEllipsePie(target, brush, xc, yc, w / 2.0f, h / 2.0f, baseAngle, sweepAngle);
  else
    d2dFillEllipseChord(target, brush, xc, yc, w / 2.0f, h / 2.0f, baseAngle, sweepAngle);
}

static dummy_ID2D1PathGeometry* createPolyGeometry(dummy_ID2D1GeometrySink* *sink, int fill_mode)
{
  dummy_ID2D1PathGeometry* g;
  HRESULT hr = dummy_ID2D1Factory_CreatePathGeometry(d2d_cd_factory, &g);
  if (FAILED(hr))
    return NULL;

  dummy_ID2D1PathGeometry_Open(g, sink);
  dummy_ID2D1GeometrySink_SetFillMode(*sink, fill_mode == CD_EVENODD ? dummy_D2D1_FILL_MODE_ALTERNATE : dummy_D2D1_FILL_MODE_WINDING);

  return g;
}

dummy_ID2D1PathGeometry* d2dCreatePolygonGeometry(int* points, int count, int mode, int fill_mode)
{
  dummy_ID2D1PathGeometry* g;
  dummy_ID2D1GeometrySink* sink;
  dummy_D2D1_POINT_2F pt;
  int i;

  g = createPolyGeometry(&sink, fill_mode);
  if (!g)
    return NULL;

  pt.x = type2float(points[0]);
  pt.y = type2float(points[1]);
  dummy_ID2D1GeometrySink_BeginFigure(sink, pt, dummy_D2D1_FIGURE_BEGIN_FILLED);

  if (mode == CD_BEZIER)
  {
    for (i = 2; i < count * 2; i = i + 6)
    {
      dummy_D2D1_BEZIER_SEGMENT segment;
      segment.point1.x = type2float(points[i]);
      segment.point1.y = type2float(points[i + 1]);
      segment.point2.x = type2float(points[i + 2]);
      segment.point2.y = type2float(points[i + 3]);
      segment.point3.x = type2float(points[i + 4]);
      segment.point3.y = type2float(points[i + 5]);
      dummy_ID2D1GeometrySink_AddBezier(sink, &segment);
    }
  }
  else
  {
    for (i = 2; i < count * 2; i = i + 2)
    {
      pt.x = type2float(points[i]);
      pt.y = type2float(points[i + 1]);
      dummy_ID2D1GeometrySink_AddLine(sink, pt);
    }
  }


  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    pt.x = type2float(points[0]);
    pt.y = type2float(points[1]);
    dummy_ID2D1GeometrySink_AddLine(sink, pt);
  }

  dummy_ID2D1GeometrySink_EndFigure(sink, (0 ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN));

  dummy_ID2D1GeometrySink_Close(sink);
  dummy_ID2D1GeometrySink_Release(sink);

  return g;
}

dummy_ID2D1PathGeometry* d2dCreatePolygonGeometryF(double* points, int count, int mode, int fill_mode)
{
  dummy_ID2D1PathGeometry* g;
  dummy_ID2D1GeometrySink* sink;
  dummy_D2D1_POINT_2F pt;
  int i;

  g = createPolyGeometry(&sink, fill_mode);
  if (!g)
    return NULL;

  pt.x = type2float(points[0]);
  pt.y = type2float(points[1]);
  dummy_ID2D1GeometrySink_BeginFigure(sink, pt, dummy_D2D1_FIGURE_BEGIN_FILLED);

  if (mode == CD_BEZIER)
  {
    for (i = 2; i < count * 2; i = i + 6)
    {
      dummy_D2D1_BEZIER_SEGMENT segment;
      segment.point1.x = type2float(points[i]);
      segment.point1.y = type2float(points[i + 1]);
      segment.point2.x = type2float(points[i + 2]);
      segment.point2.y = type2float(points[i + 3]);
      segment.point3.x = type2float(points[i + 4]);
      segment.point3.y = type2float(points[i + 5]);
      dummy_ID2D1GeometrySink_AddBezier(sink, &segment);
    }
  }
  else
  {
    for (i = 2; i < count * 2; i = i + 2)
    {
      pt.x = type2float(points[i]);
      pt.y = type2float(points[i + 1]);
      dummy_ID2D1GeometrySink_AddLine(sink, pt);
    }
  }


  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    pt.x = type2float(points[0]);
    pt.y = type2float(points[1]);
    dummy_ID2D1GeometrySink_AddLine(sink, pt);
  }

  dummy_ID2D1GeometrySink_EndFigure(sink, (0 ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN));

  dummy_ID2D1GeometrySink_Close(sink);
  dummy_ID2D1GeometrySink_Release(sink);

  return g;
}

static dummy_D2D1_FIGURE_BEGIN getFigureBegin(int *path, int path_n, int current_p)
{
  int p;

  for (p = current_p + 1; p < path_n; p++)
  {
    if (path[p] == CD_PATH_NEW || path[p] == CD_PATH_STROKE)
      break;
    if (path[p] == CD_PATH_CLOSE || path[p] == CD_PATH_FILL || path[p] == CD_PATH_FILLSTROKE || path[p] == CD_PATH_CLIP)
      return dummy_D2D1_FIGURE_BEGIN_FILLED;
  }
  return dummy_D2D1_FIGURE_BEGIN_HOLLOW;
}

void d2dDrawGeometry(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, dummy_ID2D1PathGeometry* g, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_ID2D1RenderTarget_DrawGeometry(target, (dummy_ID2D1Geometry*)g, brush, fStrokeWidth, hStrokeStyle);
}

void d2dFillGeometry(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, dummy_ID2D1PathGeometry* g)
{
  dummy_ID2D1RenderTarget_FillGeometry(target, (dummy_ID2D1Geometry*)g, brush, NULL);
}

int d2dPolyPath(d2dCanvas *canvas, dummy_ID2D1Brush *drawBrush, dummy_ID2D1Brush *fillBrush, int* points, int points_n, int* path, int path_n, int invert_yaxis, int fill_mode, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_ID2D1PathGeometry* g;
  dummy_ID2D1GeometrySink* sink;
  dummy_D2D1_POINT_2F pt;
  dummy_D2D1_FIGURE_BEGIN figureBegin = dummy_D2D1_FIGURE_BEGIN_HOLLOW;
  dummy_D2D1_ARC_SEGMENT arc_seg;
  dummy_D2D1_BEZIER_SEGMENT segment;
  float cx, cy, w, h, a1, a2;
  float baseAngle, sweepAngle, base_rads;
  int i, begin_picture = 0, p, n = 2 * points_n, ret = 0;

  g = createPolyGeometry(&sink, fill_mode);
  if (!g)
    return ret;

  i = 0;
  for (p = 0; p<path_n; p++)
  {
    switch (path[p])
    {
    case CD_PATH_NEW:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, figureBegin == dummy_D2D1_FIGURE_BEGIN_FILLED ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      if (g)
        dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_MOVETO:
      if (i + 1*2 > n) return ret;
      pt.x = type2float(points[i++]);
      pt.y = type2float(points[i++]);

      /* if BeginFigure called, then call EndFigure */
      if (begin_picture)
        dummy_ID2D1GeometrySink_EndFigure(sink, figureBegin == dummy_D2D1_FIGURE_BEGIN_FILLED ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN);

      /* there is no MoveTo, so BeginFigure acts as one */
      figureBegin = getFigureBegin(path, path_n, p);
      dummy_ID2D1GeometrySink_BeginFigure(sink, pt, figureBegin);
      begin_picture = 1;
      break;
    case CD_PATH_LINETO:
      if (i + 1*2 > n) return ret;
      pt.x = type2float(points[i++]);
      pt.y = type2float(points[i++]);

      if (begin_picture)
        dummy_ID2D1GeometrySink_AddLine(sink, pt);
      else
      {
        figureBegin = getFigureBegin(path, path_n, p);
        dummy_ID2D1GeometrySink_BeginFigure(sink, pt, figureBegin);
        begin_picture = 1;
      }
      break;
    case CD_PATH_ARC:
      if (i + 3*2 > n) return ret;
      /* same as cdGetArcPath (notice the interger fix by 1000) */
      cx = type2float(points[i++]);
      cy = type2float(points[i++]);
      w = type2float(points[i++]);
      h = type2float(points[i++]);
      a1 = type2float(points[i++] / 1000.);
      a2 = type2float(points[i++] / 1000.);
      if (invert_yaxis)
      {
        a1 *= -1;
        a2 *= -1;
      }
      baseAngle = (float)(a1);
      sweepAngle = (float)(a2 - a1);
      base_rads = baseAngle * (PI / 180.0f);

      /* arc start point */
      pt.x = cx + (w / 2.f) * cosf(base_rads);
      pt.y = cy + (h / 2.f) * sinf(base_rads);

      if (begin_picture)
        dummy_ID2D1GeometrySink_AddLine(sink, pt);
      else
      {
        figureBegin = getFigureBegin(path, path_n, p);
        dummy_ID2D1GeometrySink_BeginFigure(sink, pt, figureBegin);
        begin_picture = 1;
      }

      if (sweepAngle == 360.0f)
      {
        d2dInitArcSegment(&arc_seg, cx, cy, w / 2.f, h / 2.f, baseAngle, 180.0f);
        dummy_ID2D1GeometrySink_AddArc(sink, &arc_seg);
        d2dInitArcSegment(&arc_seg, cx, cy, w / 2.f, h / 2.f, baseAngle + 180.0f, 180.0f);
        dummy_ID2D1GeometrySink_AddArc(sink, &arc_seg);
      }
      else
      {
        d2dInitArcSegment(&arc_seg, cx, cy, w / 2.f, h / 2.f, baseAngle, sweepAngle);
        dummy_ID2D1GeometrySink_AddArc(sink, &arc_seg);
      }
      break;
    case CD_PATH_CURVETO:
      if (i + 3*2 > n) return ret;
      segment.point1.x = type2float(points[i++]);
      segment.point1.y = type2float(points[i++]);

      if (!begin_picture)
      {
        figureBegin = getFigureBegin(path, path_n, p);
        dummy_ID2D1GeometrySink_BeginFigure(sink, segment.point1, figureBegin);
        begin_picture = 1;
      }

      segment.point2.x = type2float(points[i++]);
      segment.point2.y = type2float(points[i++]);
      segment.point3.x = type2float(points[i++]);
      segment.point3.y = type2float(points[i++]);

      dummy_ID2D1GeometrySink_AddBezier(sink, &segment);
      break;
    case CD_PATH_CLOSE:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      break;
    case CD_PATH_CLIP:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dSetClipGeometry(canvas, g);
      ret = 1;  /* clipping was set */

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_FILL: 
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dFillGeometry(canvas->target, fillBrush, g);

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_FILLSTROKE:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dFillGeometry(canvas->target, fillBrush, g);
      d2dDrawGeometry(canvas->target, drawBrush, g, fStrokeWidth, hStrokeStyle);

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_STROKE:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, figureBegin == dummy_D2D1_FIGURE_BEGIN_FILLED ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dDrawGeometry(canvas->target, drawBrush, g, fStrokeWidth, hStrokeStyle);

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    }
  }

  if (sink)
  {
    dummy_ID2D1GeometrySink_Close(sink);
    dummy_ID2D1GeometrySink_Release(sink);
  }
  if (g)
    dummy_ID2D1PathGeometry_Release(g);

  return ret;
}

int d2dPolyPathF(d2dCanvas *canvas, dummy_ID2D1Brush *drawBrush, dummy_ID2D1Brush *fillBrush, double* points, int points_n, int* path, int path_n, int invert_yaxis, int fill_mode, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle)
{
  dummy_ID2D1PathGeometry* g;
  dummy_ID2D1GeometrySink* sink;
  dummy_D2D1_POINT_2F pt;
  dummy_D2D1_FIGURE_BEGIN figureBegin = dummy_D2D1_FIGURE_BEGIN_HOLLOW;
  dummy_D2D1_ARC_SEGMENT arc_seg;
  dummy_D2D1_BEZIER_SEGMENT segment;
  float cx, cy, w, h, a1, a2;
  float baseAngle, sweepAngle, base_rads;
  int i, begin_picture = 0, p, n = 2 * points_n, ret = 0;

  g = createPolyGeometry(&sink, fill_mode);
  if (!g)
    return ret;

  i = 0;
  for (p = 0; p<path_n; p++)
  {
    switch (path[p])
    {
    case CD_PATH_NEW:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, figureBegin == dummy_D2D1_FIGURE_BEGIN_FILLED ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      if (g)
        dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_MOVETO:
      if (i + 1 * 2 > n) return ret;
      pt.x = type2float(points[i++]);
      pt.y = type2float(points[i++]);

      /* if BeginFigure called, then call EndFigure */
      if (begin_picture)
        dummy_ID2D1GeometrySink_EndFigure(sink, figureBegin == dummy_D2D1_FIGURE_BEGIN_FILLED ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN);

      /* there is no MoveTo, so BeginFigure acts as one */
      figureBegin = getFigureBegin(path, path_n, p);
      dummy_ID2D1GeometrySink_BeginFigure(sink, pt, figureBegin);
      begin_picture = 1;
      break;
    case CD_PATH_LINETO:
      if (i + 1 * 2 > n) return ret;
      pt.x = type2float(points[i++]);
      pt.y = type2float(points[i++]);

      if (begin_picture)
        dummy_ID2D1GeometrySink_AddLine(sink, pt);
      else
      {
        figureBegin = getFigureBegin(path, path_n, p);
        dummy_ID2D1GeometrySink_BeginFigure(sink, pt, figureBegin);
        begin_picture = 1;
      }
      break;
    case CD_PATH_ARC:
      if (i + 3 * 2 > n) return ret;
      /* same as cdfGetArcPath (no integer fix here) */
      cx = type2float(points[i++]);
      cy = type2float(points[i++]);
      w = type2float(points[i++]);
      h = type2float(points[i++]);
      a1 = type2float(points[i++]);
      a2 = type2float(points[i++]);
      if (invert_yaxis)
      {
        a1 *= -1;
        a2 *= -1;
      }
      baseAngle = (float)(a1);
      sweepAngle = (float)(a2 - a1);
      base_rads = baseAngle * (PI / 180.0f);

      /* arc start point */
      pt.x = cx + (w / 2.f) * cosf(base_rads);
      pt.y = cy + (h / 2.f) * sinf(base_rads);

      if (begin_picture)
        dummy_ID2D1GeometrySink_AddLine(sink, pt);
      else
      {
        figureBegin = getFigureBegin(path, path_n, p);
        dummy_ID2D1GeometrySink_BeginFigure(sink, pt, figureBegin);
        begin_picture = 1;
      }

      if (sweepAngle == 360.0f)
      {
        d2dInitArcSegment(&arc_seg, cx, cy, w / 2.f, h / 2.f, baseAngle, 180.0f);
        dummy_ID2D1GeometrySink_AddArc(sink, &arc_seg);
        d2dInitArcSegment(&arc_seg, cx, cy, w / 2.f, h / 2.f, baseAngle + 180.0f, 180.0f);
        dummy_ID2D1GeometrySink_AddArc(sink, &arc_seg);
      }
      else
      {
        d2dInitArcSegment(&arc_seg, cx, cy, w / 2.f, h / 2.f, baseAngle, sweepAngle);
        dummy_ID2D1GeometrySink_AddArc(sink, &arc_seg);
      }
      break;
    case CD_PATH_CURVETO:
      if (i + 3 * 2 > n) return ret;
      segment.point1.x = type2float(points[i++]);
      segment.point1.y = type2float(points[i++]);

      if (!begin_picture)
      {
        figureBegin = getFigureBegin(path, path_n, p);
        dummy_ID2D1GeometrySink_BeginFigure(sink, segment.point1, figureBegin);
        begin_picture = 1;
      }

      segment.point2.x = type2float(points[i++]);
      segment.point2.y = type2float(points[i++]);
      segment.point3.x = type2float(points[i++]);
      segment.point3.y = type2float(points[i++]);

      dummy_ID2D1GeometrySink_AddBezier(sink, &segment);
      break;
    case CD_PATH_CLOSE:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      break;
    case CD_PATH_CLIP:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dSetClipGeometry(canvas, g);
      ret = 1;  /* clipping was set */

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_FILL:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dFillGeometry(canvas->target, fillBrush, g);

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_FILLSTROKE:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, dummy_D2D1_FIGURE_END_CLOSED);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dFillGeometry(canvas->target, fillBrush, g);
      d2dDrawGeometry(canvas->target, drawBrush, g, fStrokeWidth, hStrokeStyle);

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    case CD_PATH_STROKE:
      if (begin_picture)
      {
        begin_picture = 0;
        dummy_ID2D1GeometrySink_EndFigure(sink, figureBegin == dummy_D2D1_FIGURE_BEGIN_FILLED ? dummy_D2D1_FIGURE_END_CLOSED : dummy_D2D1_FIGURE_END_OPEN);
      }
      if (sink)
      {
        dummy_ID2D1GeometrySink_Close(sink);
        dummy_ID2D1GeometrySink_Release(sink);
        sink = NULL;
      }

      d2dDrawGeometry(canvas->target, drawBrush, g, fStrokeWidth, hStrokeStyle);

      /* reset the geometry */
      dummy_ID2D1PathGeometry_Release(g);

      g = createPolyGeometry(&sink, fill_mode);
      if (!g)
        return ret;

      break;
    }
  }

  if (sink)
  {
    dummy_ID2D1GeometrySink_Close(sink);
    dummy_ID2D1GeometrySink_Release(sink);
  }
  if (g)
    dummy_ID2D1PathGeometry_Release(g);

  return ret;
}
