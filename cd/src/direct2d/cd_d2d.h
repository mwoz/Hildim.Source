/** \file
 * \brief Windows direct2d Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CD_D2D_H
#define __CD_D2D_H

#define COBJMACROS
#define UNICODE
#define _UNICODE

#include <windows.h>                                                                                                     
#include <wincodec.h>

#if 0
// To Use Standard Direct 2D headers
#define D2D_USE_C_DEFINITIONS
#include <d2d1.h>
#define dummy_ 
#else
#include "dummy/d2d1.h"
#include "dummy/dwrite.h"
#endif

#define D2D_CANVASTYPE_DC           0
#define D2D_CANVASTYPE_HWND         1
#define D2D_CANVASTYPE_IMAGE        2
#define D2D_CANVASTYPE_TARGET       3

#define CANVAS_NOGDICOMPAT       0x0001
#define CANVAS_LAYOUTRTL         0x0002

#define D2D_CANVASFLAG_RECTCLIP     0x1
#define D2D_CANVASFLAG_POLYCLIP     0x2

#define D2D_BASEDELTA_X             0.5f
#define D2D_BASEDELTA_Y             0.5f

#define PI 3.14159265358979323846f

#define type2float(_x) ((float)(_x))

extern HMODULE d2d_cd_dll;
extern HMODULE wic_cd_dll;
extern HMODULE dwrite_cd_dll;

extern dummy_ID2D1Factory* d2d_cd_factory;
extern IWICImagingFactory* wic_cd_factory;
extern dummy_IDWriteFactory* dwrite_cd_factory;

typedef struct _d2dCanvas {
  WORD type;    /* D2D_CANVASTYPE_* */
  WORD flags;    /* D2D_CANVASFLAG_* */
  dummy_ID2D1RenderTarget* target;
} d2dCanvas;


typedef struct _d2dFont {
  dummy_IDWriteTextFormat* tf;
  dummy_DWRITE_FONT_METRICS metrics;
} d2dFont;

typedef struct _d2dFontMetrics {
  float fEmHeight;        /* Typically height of letter 'M' or 'H' */
  float fAscent;          /* Height of char cell above the base line. */
  float fDescent;         /* Height of char cell below the base line. */
  float fLeading;         /* Distance of two base lines in multi-line text. */
                          /* Usually: fEmHeight < fAscent + fDescent <= fLeading */
} d2dFontMetrics;

FLOAT d2dFloatMax(void);
dummy_D2D1_RECT_F d2dInfiniteRect(void);

void d2dStartup(void);
void d2dShutdown(void);

d2dCanvas* d2dCreateCanvasWithWindow(HWND hWnd, DWORD dwFlags);
d2dCanvas* d2dCreateCanvasWithHDC(HDC hDC, const RECT* pRect, DWORD dwFlags);
d2dCanvas* d2dCreateCanvasWithImage(IWICBitmap *wic_bitmap);
d2dCanvas* d2dCreateCanvasWithTarget(dummy_ID2D1RenderTarget* target);
void d2dCanvasDestroy(d2dCanvas* d2d_canvas);

void d2dResetTransform(dummy_ID2D1RenderTarget* target);
void d2dGetTransform(dummy_ID2D1RenderTarget* target, dummy_D2D1_MATRIX_3X2_F* matrix);
void d2dRotateWorld(dummy_ID2D1RenderTarget *target, float cx, float cy, float fAngle);
void d2dApplyTransform(dummy_ID2D1RenderTarget* target, const dummy_D2D1_MATRIX_3X2_F* matrix);
void d2dSetClipRect(d2dCanvas *canvas, double x1, double y1, double x2, double y2);
void d2dSetClipGeometry(d2dCanvas *canvas, dummy_ID2D1PathGeometry* g);
void d2dResetClip(d2dCanvas* c);
void d2dInitArcSegment(dummy_D2D1_ARC_SEGMENT* arc_seg, float cx, float cy, float rx, float ry, float base_angle, float sweep_angle);

IWICBitmap* d2dCreateImageFromBufferRGB(UINT uWidth, UINT uHeight, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha);
IWICBitmap* d2dCreateImageFromBufferMap(UINT uWidth, UINT uHeight, const unsigned char *map, const long* cPalette);
IWICBitmap* d2dCreateImageFromHatch(int style, int hsize, int back_opacity, long foreground, long background);
IWICBitmap* d2dCreateImageFromStipple(UINT uWidth, UINT uHeight, const unsigned char *stipple, int opacity, long foreground, long background);
IWICBitmap* d2dCreateImageFromPattern(UINT uWidth, UINT uHeight, const long *pattern);
IWICBitmap* d2dCreateImage(UINT uWidth, UINT uHeight);
void d2dDestroyImage(IWICBitmap *wic_bitmap);
void d2dBitBltImage(dummy_ID2D1RenderTarget *target, IWICBitmap *wic_bitmap, const dummy_D2D1_RECT_F* pDestRect, const dummy_D2D1_RECT_F* pSourceRect, dummy_D2D1_BITMAP_INTERPOLATION_MODE mode);
void d2dBitBltBitmap(dummy_ID2D1RenderTarget *target, dummy_ID2D1Bitmap *bitmap, const dummy_D2D1_RECT_F* pDestRect, const dummy_D2D1_RECT_F* pSourceRect, dummy_D2D1_BITMAP_INTERPOLATION_MODE mode);

d2dFont* d2dCreateFont(const LOGFONTW* pLogFont);
void d2dDestroyFont(d2dFont* font);
void d2dFontGetMetrics(d2dFont *font, d2dFontMetrics *pMetrics);
void d2dFontMeasureString(d2dFont *hFont, const WCHAR* pszText, int iTextLength, int *w, int *h);

dummy_ID2D1Brush* d2dCreateSolidBrush(dummy_ID2D1RenderTarget *target, long color);
dummy_ID2D1Brush* d2dCreateImageBrush(dummy_ID2D1RenderTarget *target, IWICBitmap* wic_bitmap);
dummy_ID2D1Brush* d2dCreateLinearGradientBrush(dummy_ID2D1RenderTarget *target, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, long foreground, long background);
dummy_ID2D1Brush* d2dCreateRadialGradientBrush(dummy_ID2D1RenderTarget *target, FLOAT cx, FLOAT cy, FLOAT ox, FLOAT oy, FLOAT rx, FLOAT ry, long foreground, long background);
dummy_ID2D1StrokeStyle *d2dSetLineStyle(int line_style, int line_cap, int line_join);
dummy_ID2D1StrokeStyle *d2dSetCustomLineStyle(int *dashes, int dashes_count, float line_width, int line_cap, int line_join);
void d2dInitColor(dummy_D2D1_COLOR_F* c, long color);

dummy_ID2D1PathGeometry* d2dCreateArcGeometry(float cx, float cy, float rx, float ry, float base_angle, float sweep_angle, int pie);
dummy_ID2D1PathGeometry* d2dCreatePolygonGeometry(int* points, int count, int mode, int fill_mode);
dummy_ID2D1PathGeometry* d2dCreatePolygonGeometryF(double* points, int count, int mode, int fill_mode);
dummy_ID2D1PathGeometry* d2dCreateBoxGeometry(double xmin, double xmax, double ymin, double ymax);

void d2dDrawText(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x, float y, float w, float h, const WCHAR* pszText, int iTextLength, d2dFont *font);
void d2dDrawLine(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x0, float y0, float x1, float y1, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle);
void d2dDrawRect(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x0, float y0, float x1, float y1, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle);
void d2dFillRect(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x0, float y0, float x1, float y1);
void d2dDrawArc(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float xc, float yc, float w, float h, double a1, double a2, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle);
void d2dFillArc(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float xc, float yc, float w, float h, double a1, double a2, int pie);
void d2dDrawGeometry(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, dummy_ID2D1PathGeometry* g, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle);
void d2dFillGeometry(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, dummy_ID2D1PathGeometry* g);

int d2dPolyPath(d2dCanvas *canvas, dummy_ID2D1Brush *drawBrush, dummy_ID2D1Brush *fillBrush, int* points, int points_n, int* path, int path_n, int invert_yaxis, int fill_mode, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle);
int d2dPolyPathF(d2dCanvas *canvas, dummy_ID2D1Brush *drawBrush, dummy_ID2D1Brush *fillBrush, double* points, int points_n, int* path, int path_n, int invert_yaxis, int fill_mode, float fStrokeWidth, dummy_ID2D1StrokeStyle *hStrokeStyle);

#endif
