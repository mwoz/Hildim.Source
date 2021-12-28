/** 
 * CGM Parser for the CD toolkit
 * Tecgraf: Computer Graphics Technology Group, PUC-Rio, Brazil
 * http://www.tecgraf.puc-rio.br/cd	
 * mailto:cd@tecgraf.puc-rio.br
 *
 * Based on INTCGM implemented by Camilo Freire.
 * New implementation by Antonio Scuri & Rafael Rieder.
 *
 * Version 1.0 - 22/Sep/2012
 *
 * See Copyright Notice at the end of this file
 */

#ifndef _CGM_PLAY_H_
#define _CGM_PLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _tCGM tCGM;

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} cgmRGB;

typedef struct {
  double x;
  double y;
} cgmPoint;

typedef struct {
  int w, h;
  cgmRGB *pattern;
} cgmPattern;

enum { CGM_LINES, CGM_CLOSEDLINES, CGM_FILL, CGM_BEZIER };  /* polygon options */
enum { CGM_PIE, CGM_CHORD, CGM_OPENARC };  /* arc options */

typedef struct { 
  /* Delimiter Elements */
  void (*BeginMetafile)(const char* name, void* userdata);
  void (*EndMetafile)(void* userdata);
  void (*BeginPicture)(const char* name, void* userdata);
  void (*EndPicture)(void* userdata);
  void (*BeginPictureBody)(void* userdata);

  /* Picture Descriptor Elements */
  void (*DeviceExtent)(cgmPoint *first, cgmPoint *second, void* userdata);
  void (*ScaleMode)(int metric, double *factor, void* userdata);
  void (*BackgroundColor)(cgmRGB color, void* userdata);

  /* Control Elements */
  void (*Transparency)(int transp, cgmRGB color, void* userdata);
  void (*ClipRectangle)(cgmPoint, cgmPoint, void* userdata);
  void (*ClipIndicator)(int clip, void* userdata);

  /* Graphical Primitive Elements */
  void (*PolyLine)(int n, cgmPoint* pt, void* userdata);
  void (*PolyMarker)(int n, cgmPoint* pt, void* userdata);
  void (*Polygon)(int n, cgmPoint* pt, int fill, void* userdata);
  void (*Rectangle)(cgmPoint point1, cgmPoint point2, void* userdata);
  void (*Circle)(cgmPoint center, double radius, void* userdata);
  void (*CircularArc)(cgmPoint center, double radius, double angle1, double angle2, int arc, void* userdata);
  void (*Ellipse)(cgmPoint center, cgmPoint first, cgmPoint second, void* userdata);
  void (*EllipticalArc)(cgmPoint center, cgmPoint first, cgmPoint second, double angle1, double angle2, int arc, void* userdata);
  void (*Text)(const char* text, cgmPoint point, void* userdata);
  void (*CellArray)(cgmPoint corner1, cgmPoint corner2, cgmPoint corner3, int w, int h, unsigned char* rgb, void* userdata);

  /* Attributes */
  void (*TextAttrib)(const char* horiz_align, const char* vert_align, const char* font, double height, cgmRGB color, cgmPoint base_dir, void* userdata);
  void (*LineAttrib)(const char *type, const char *cap, const char *join, double width, const char *mode, cgmRGB color, void* userdata);
  void (*MarkerAttrib)(const char *type, double size, const char* mode, cgmRGB color, void* userdata);
  void (*FillAttrib)(const char* type, cgmRGB color, const char* hatch, cgmPattern* pat, void* userdata);  

  /* Utilities */
  int(*Counter)(double percent, void* userdata);
} cgmPlayFuncs;

/* Notes:
  CGM version 1 is fully supported.
  CGM version 2 and 3 are partially supported.
  All modules are independent from the CD and can be reused.
  All methods are optional and can be NULL.
  Call Order
    BeginMetafile
      BeginPicture
        (Picture Descriptor Elements)
        BeginPictureBody
        (Control, Graphical and Attribute Elements)
      EndPicture
    EndMetafile
  Attribute Methods are not called in file order.
    They are called right before a Graphical Primitive method.
  DeviceExtent
    first,second defines axis and angle orientation
    (0,0)-(1,1) defines a bottom-top / left-right / counter-clock wise orientation
    first and second can be changed by the method
  Scale Mode (metric=on[1]/off[0])
    metric (coordinate * factor = coordinate_mm) 
    off = abstract (pixels, factor is always 1)
    factor can be changed by the method
  Background Color
    can be used to fill the background
  Transparency (transp=on[1]/off[0])
    same as background opacity. The given color should be used when transp=off.
  ClipIndicator (clip=on[1]/off[0])
  CellArray (rgb=packed data rgbrgbrgb...)
    corner1 => (0,0)
    corner2 => (w-1,h-1)
    corner3 => (w-1,0)
  Ellipse,EllipticalArc
    first,second define the major and minor axis
    angle is oriented from second to first in the positive angular direction
  LineAttrib 
    (type=SOLID,DASH,DOT,DASH_DOT,DASH_DOT_DOT)
    (mode=ABSOLUTE,SCALED,FRACTIONAL,MM)
    absolute => pixels
    scaled => scale factor to be applied by the interpreter to a device-dependent "nominal" measure
    fractional => fraction of the horizontal dimension of the default device viewport
    mm => millimetres
  MarkerAttrib 
    (type=DOT,PLUS,ASTERISK,CIRCLE,CROSS)
    (mode=ABS,SCALED,FRACTIONAL,MM)
  FillAttrib (type=HOLLOW,SOLID,PATTERN,HATCH)
    hath is NULL if type!=HATCH
    pat is NULL if type!=PATTERN 
  Hatches (hatch=HORIZONTAL,VERTICAL,POSITIVE_SLOPE,NEGATIVE_SLOPE,HV_CROSS,SLOPE_CROSS)
    horizontal equally spaced parallel lines
    vertical equally spaced parallel lines
    positive slope equally spaced parallel lines
    negative slope equally spaced parallel lines
    horizontal/vertical crosshatch
    positive slope/negative slope crosshatch
  Horizontal Text Alignment (hor=LEFT,CENTER,RIGHT)
  Vertical Text Alignment (ver=TOP,CAP,CENTER,BASELINE,BOTTOM)
  Text Height
    are in coordinates units
  Common Fonts:
    TIMES_ROMAN
    TIMES_ITALIC
    TIMES_BOLD
    TIMES_BOLD_ITALIC
    HELVETICA
    HELVETICA_OBLIQUE
    HELVETICA_BOLD
    HELVETICA_BOLD_OBLIQUE
    COURIER
    COURIER_BOLD
    COURIER_ITALIC
    COURIER_BOLD_ITALIC
  Counter
    if file is sucessfully open then percent=0 and percent=100 will alway be called.
*/

/* returned values */
#define CGM_OK 0
#define CGM_ERR_OPEN 1
#define CGM_ERR_READ 2
#define CGM_ABORT_COUNTER -1

int cgmPlay(const char* filename, void* userdata, cgmPlayFuncs* funcs);


#ifdef __cplusplus
}
#endif

/******************************************************************************
Copyright (C) 1994-2012 Tecgraf, PUC-Rio.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#endif
