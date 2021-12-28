/** \file
 * \brief PPTX library
 *
 * See Copyright Notice in cd.h
 */

#ifndef __PPTX_H
#define __PPTX_H

typedef struct _pptxPresentation pptxPresentation;

pptxPresentation *pptxCreatePresentation(double width_mm, double height_mm, int width, int height);
int pptxKillPresentation(pptxPresentation *presentation, const char* filename);

int pptxOpenSlide(pptxPresentation *presentation);
void pptxCloseSlide(pptxPresentation *presentation);

void pptsBeginMasterFile(pptxPresentation *presentation);
void pptsEndMasterFile(pptxPresentation *presentation);
void pptxSetImportedMasterSlideFile(pptxPresentation* presentation, char* masterSlideFile);

void pptxBeginPath(pptxPresentation *presentation, int xmin, int ymin, int w, int h);
void pptxMoveTo(pptxPresentation *presentation, int x, int y);
void pptxLineTo(pptxPresentation *presentation, int x, int y);
void pptxArcTo(pptxPresentation *presentation, int h, int w, double stAng, double swAng);
void pptxBezierLineTo(pptxPresentation *presentation, int c1x, int c1y, int c2x, int c2y, int c3x, int c3y);
void pptxClosePath(pptxPresentation *presentation);
void pptxBeginSector(pptxPresentation *presentation, const char *geomType, int xmin, int ymin, int w, int h, double angle1, double angle2);
void pptxNoFill(pptxPresentation *presentation);
void pptxSolidFill(pptxPresentation *presentation, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
void pptxHatchLine(pptxPresentation *presentation, const char* style, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned char bRed, unsigned char bGreen, unsigned char bBlue, unsigned char bAlpha);
void pptxPattern(pptxPresentation *presentation, const unsigned char *rgba_data, int width, int height);
void pptxStipple(pptxPresentation *presentation, const unsigned char *rgba_data, int width, int height);
void pptxEndLine(pptxPresentation *presentation, int width, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, const char* lineStyle, int nDashes, int *dashes);
void pptxEndFill(pptxPresentation *presentation);
void pptxText(pptxPresentation *presentation, int xmin, int ymin, int w, int h, double rotAngle, int bold, int italic, int underline, int strikeout, int size, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, const char* typeface, const char* text);
void pptxPixel(pptxPresentation *presentation, int x, int y, int width, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
void pptxImageRGB(pptxPresentation *presentation, int iw, int ih, const unsigned char *rgb_data, int x, int y, int w, int h);
void pptxImageRGBA(pptxPresentation *presentation, int iw, int ih, const unsigned char *rgba_data, int x, int y, int w, int h);

#endif
