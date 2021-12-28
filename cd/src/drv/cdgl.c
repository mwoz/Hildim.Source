/** \file
 * \brief OpenGL Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined (OSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <FTGL/ftgl.h>

#include "cd.h"
#include "cd_private.h"
#include "cdgl.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_HATCHES  6
#define HATCH_WIDTH  8
#define HATCH_HEIGHT 8

/* 
** 6 predefined patterns to be accessed through cdHatch(
CD_HORIZONTAL | CD_VERTICAL | CD_FDIAGONAL | CD_BDIAGONAL |
CD_CROSS      | CD_DIAGCROSS)

*/
static char hatches[NUM_HATCHES][8] = {
  {0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00},  /* HORIZONTAL */
  {0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22},  /* VERTICAL */
  {0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04},  /* FDIAGONAL */
  {0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20},  /* BDIAGONAL */
  {0x22,0x22,0xFF,0x22,0x22,0x22,0xFF,0x22},  /* CROSS */
  {0x18,0x18,0x24,0x42,0x81,0x81,0x42,0x24}   /* DIAGCROSS */
};


typedef struct _cdglFontCache
{
  char filename[10240];
  int size;
  FTGLfont *font;
} cdglFontCache;


struct _cdCtxImage
{
  unsigned int w, h;
  GLubyte* img;     /* always RGBA 32bpp */
  GLuint texture;
};

struct _cdCtxCanvas
{
  cdCanvas* canvas;

  FTGLfont *font;

  double rotate_angle;
  int rotate_center_x;
  int rotate_center_y;

  char* utf8_buffer;
  int utf8mode, utf8_buffer_len;

  cdglFontCache* gl_fonts;
  int gl_fonts_count;
  int gl_fonts_max;

  int texture_filter;
};

/******************************************************/

static FTGLfont* cdglGetFont(cdCtxCanvas *ctxcanvas, const char* filename, int size, int res)
{
  int i;
  FTGLfont* font;

  if (!ctxcanvas->gl_fonts)
  {
    ctxcanvas->gl_fonts_max = 10;
    ctxcanvas->gl_fonts_count = 0;
    ctxcanvas->gl_fonts = malloc(sizeof(cdglFontCache)*ctxcanvas->gl_fonts_max);
  }

  /* search for an existent font */
  for (i = 0; i < ctxcanvas->gl_fonts_count; i++)
  {
    if (cdStrEqualNoCase(ctxcanvas->gl_fonts[i].filename, filename) &&
        ctxcanvas->gl_fonts[i].size == size)
      return ctxcanvas->gl_fonts[i].font;
  }

  /* not found, create a new font and add it to the cache */

  font = ftglCreateTextureFont(filename);
  if (!font)
    return NULL;

  ftglSetFontFaceSize(font, size, res);

  if (ctxcanvas->gl_fonts_count == ctxcanvas->gl_fonts_max)
  {
    ctxcanvas->gl_fonts_max += 10;
    ctxcanvas->gl_fonts = realloc(ctxcanvas->gl_fonts, sizeof(cdglFontCache)*ctxcanvas->gl_fonts_max);
  }

  ctxcanvas->gl_fonts[ctxcanvas->gl_fonts_count].font = font;
  strcpy(ctxcanvas->gl_fonts[ctxcanvas->gl_fonts_count].filename, filename);
  ctxcanvas->gl_fonts[ctxcanvas->gl_fonts_count].size = size;

  ctxcanvas->gl_fonts_count++;
  return font;
}

static void cdglStrConvertToUTF8(cdCtxCanvas *ctxcanvas, const char* str, int len)
{
  /* FTGL multibyte strings are always UTF-8 */
  ctxcanvas->utf8_buffer = cdStrConvertToUTF8(str, len, ctxcanvas->utf8_buffer, &(ctxcanvas->utf8_buffer_len), ctxcanvas->utf8mode);
}

static void cdglGetImageData(GLubyte* glImage, unsigned char *r, unsigned char *g, unsigned char *b, int w, int h)
{
  int y, x;
  unsigned char *pixline_data;
  int rowstride, channels = 3;

  rowstride = w * channels;

  /* planes are separated in image data */
  for (y = 0; y < h; y++)
  {
    int lineoffset = y * w;
    pixline_data = glImage + y * rowstride;
    for (x = 0; x < w; x++)
    {
      int pos = x*channels;
      r[lineoffset + x] = pixline_data[pos];
      g[lineoffset + x] = pixline_data[pos + 1];
      b[lineoffset + x] = pixline_data[pos + 2];
    }
  }
}

static GLubyte* cdglCreateImage(int xmin, int ymin, int width, int height, const unsigned char *rgba, int image_width)
{
  GLubyte* pixline_data;
  GLubyte* glImage;
  const unsigned char* line_data;
  int rowstride = width * 4;
  int x, y;

  glImage = (GLubyte*)malloc(rowstride * height);

  xmin *= 4;
  image_width *= 4;

  for (y = 0; y < height; y++)
  {
    pixline_data = glImage + y * rowstride;
    line_data = rgba + (y + ymin) * image_width + xmin;

    for (x = 0; x < rowstride; x++)
      pixline_data[x] = line_data[x];
  }

  return glImage;
}

static GLubyte* cdglCreateImageRGBA(int xmin, int ymin, int width, int height, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int image_width)
{
  GLubyte* pixline_data;
  GLubyte* glImage;
  int x, y;
  int channels = a ? 4 : 3;
  int rowstride = width * channels;
  int lineoffset;

  glImage = (GLubyte*)malloc(rowstride * height);

  /* planes are separated in image data */
  for (y = 0; y < height; y++)
  {
    pixline_data = glImage + y * rowstride;
    lineoffset = (y + ymin) * image_width + xmin;

    for (x = 0; x<width; x++)
    {
      int pos = x*channels;
      pixline_data[pos] = r[lineoffset + x];
      pixline_data[pos + 1] = g[lineoffset + x];
      pixline_data[pos + 2] = b[lineoffset + x];

      if (a)
        pixline_data[pos + 3] = a[lineoffset + x];
    }
  }

  return glImage;
}

static GLubyte* cdglCreateImageMap(int xmin, int ymin, int width, int height, const long* colors, const unsigned char *map, int image_width)
{
  const GLubyte *line_data;
  GLubyte *pixline_data;
  GLubyte *glImage;
  int x, y, channels = 3;
  int rowstride = width * channels;

  glImage = (GLubyte*)malloc(rowstride * height);

  for (y = 0; y < height; y++)
  {
    pixline_data = glImage + y * rowstride;
    line_data = map + (y + ymin) * image_width + xmin;

    for (x = 0; x<width; x++)
    {
      GLubyte index = line_data[x];
      long c = colors[index];
      GLubyte *r = &pixline_data[channels*x],
        *g = r + 1,
        *b = g + 1;

      *r = cdRed(c);
      *g = cdGreen(c);
      *b = cdBlue(c);
    }
  }

  return glImage;
}

#if 0
static GLubyte* cdglCreateImagePattern(int width, int height, const long* pattern)
{
  const long *line_data;
  GLubyte *pixline_data;
  GLubyte *glImage;
  int x, y, channels = 4;
  int rowstride = width * channels;

  glImage = (GLubyte*)malloc(rowstride * height);

  for (y = 0; y < height; y++)
  {
    pixline_data = glImage + y * rowstride;
    line_data = pattern + y * width;

    for (x = 0; x<width; x++)
    {
      long c = line_data[x];
      GLubyte *r = &pixline_data[channels*x],
        *g = r + 1,
        *b = g + 1,
        *a = b + 1;

      *r = cdRed(c);
      *g = cdGreen(c);
      *b = cdBlue(c);
      *a = cdAlpha(c);
    }
  }

  return glImage;
}

static GLubyte* cdglCreateImageStipple(int width, int height, const char* stipple, long fgcolor, long bgcolor, int back_opacity)
{
  const char *line_data;
  GLubyte *pixline_data;
  GLubyte *glImage;
  int x, y, channels = 4;
  int rowstride = width * channels;

  glImage = (GLubyte*)malloc(rowstride * height);

  for (y = 0; y < height; y++)
  {
    pixline_data = glImage + y * rowstride;
    line_data = stipple + y * width;

    for (x = 0; x<width; x++)
    {
      char s = line_data[x];
      GLubyte *r = &pixline_data[channels*x],
        *g = r + 1,
        *b = g + 1,
        *a = b + 1;

      if (s)
      {
        *r = cdRed(fgcolor);
        *g = cdGreen(fgcolor);
        *b = cdBlue(fgcolor);
        *a = cdAlpha(fgcolor);
      }
      else
      {
        *r = cdRed(bgcolor);
        *g = cdGreen(bgcolor);
        *b = cdBlue(bgcolor);
        if (back_opacity == CD_TRANSPARENT)
          *a = cdAlpha(0); /* actually 255 */
        else
          *a = cdAlpha(bgcolor);
      }
    }
  }

  return glImage;
}
#endif

static int iGLIsOpenGL2orMore(void)
{
  const char* glversion = (const char*)glGetString(GL_VERSION);
  int major = 1, minor = 0;
  sscanf(glversion, "%d.%d", &major, &minor);
  if (major > 1)
    return 1;
  else
    return 0;
}

static GLuint cdglCreateTexture(void)
{
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  return texture;
}

static int cdglBeginTexture(cdCtxCanvas *ctxcanvas, GLuint texture)
{
  int smooth = glIsEnabled(GL_POLYGON_SMOOTH);
  if (smooth) glDisable(GL_POLYGON_SMOOTH);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ctxcanvas->texture_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ctxcanvas->texture_filter);

  return smooth;
}

static void cdglEndTexture(int smooth)
{
  glDisable(GL_TEXTURE_2D);
  if (smooth) glEnable(GL_POLYGON_SMOOTH);
}

static void cdglDrawTextureImage(cdCtxCanvas *ctxcanvas, GLuint texture, double x, double y, double w, double h)
{
  double x2 = x + w - 1;
  double y2 = y + h - 1;

  int smooth = cdglBeginTexture(ctxcanvas, texture);

  glBegin(GL_QUADS);
  glTexCoord2d(0.0, 0.0); glVertex2d(x, y);
  glTexCoord2d(1.0, 0.0); glVertex2d(x2 + 0.375, y);
  glTexCoord2d(1.0, 1.0); glVertex2d(x2 + 0.375, y2 + 0.375);
  glTexCoord2d(0.0, 1.0); glVertex2d(x, y2 + 0.375);
  glEnd();

  cdglEndTexture(smooth);
}

static void cdglPutImage(cdCtxCanvas *ctxcanvas, int rw, int rh, const unsigned char* glImage, int format, int x, int y, int w, int h)
{
  if (iGLIsOpenGL2orMore())
  {
    /* Texture is faster(?), will follow the transformations, follow clipping,
    but have limitations.
    Its maximum size is 3379 (GL_MAX_TEXTURE_SIZE).
    In OpenGL 1.x its size must be a power of two.
    */
    GLuint texture = cdglCreateTexture();

    glTexImage2D(GL_TEXTURE_2D, 0, format, rw, rh, 0, format, GL_UNSIGNED_BYTE, glImage);

    cdglDrawTextureImage(ctxcanvas, texture, x, y, w, h);

    glDeleteTextures(1, &texture);
  }
  else
  {
    /* glDrawPixels is not affected by transformations.
    glRasterPos2i will be clipped if outside the canvas
    and the image will not be drawn.
    So we will avoid these two functions and use textures by default.
    */
    if (w != rw || h != rh)
      glPixelZoom((GLfloat)w / (GLfloat)rw, (GLfloat)h / (GLfloat)rh);

    glRasterPos2i(x, y);
    glDrawPixels(rw, rh, format, GL_UNSIGNED_BYTE, glImage);

    if (w != rw || h != rh)
      glPixelZoom(1.0f, 1.0f);
  }
}

static void cdglfPutImage(cdCtxCanvas *ctxcanvas, int rw, int rh, const unsigned char* glImage, int format, double x, double y, double w, double h)
{
  if (iGLIsOpenGL2orMore())
  {
    /* Texture is faster(?), will follow the transformations, follow clipping,
    but have limitations.
    Its maximum size is 3379 (GL_MAX_TEXTURE_SIZE).
    In OpenGL 1.x its size must be a power of two.
    */
    GLuint texture = cdglCreateTexture();

    glTexImage2D(GL_TEXTURE_2D, 0, format, rw, rh, 0, format, GL_UNSIGNED_BYTE, glImage);

    cdglDrawTextureImage(ctxcanvas, texture, x, y, w, h);

    glDeleteTextures(1, &texture);
  }
  else
  {
    /* glDrawPixels is not affected by transformations.
    glRasterPos2i will be clipped if outside the canvas
    and the image will not be drawn.
    So we will avoid these two functions and use textures by default.
    */
    if (w != rw || h != rh)
      glPixelZoom((GLfloat)w / (GLfloat)rw, (GLfloat)h / (GLfloat)rh);

    glRasterPos2d(x, y);
    glDrawPixels(rw, rh, format, GL_UNSIGNED_BYTE, glImage);

    if (w != rw || h != rh)
      glPixelZoom(1.0f, 1.0f);
  }
}

/******************************************************/

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  /* SIZE attribute MUST be updated when the canvas window is resized */
  cdCanvas* canvas = ctxcanvas->canvas;
  glViewport(0, 0, canvas->w, canvas->h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, canvas->w, 0, canvas->h, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.375, 0.375, 0.0);  /* render all primitives at integer positions */

  /* An optimum compromise that allows all primitives to be specified at integer positions,
     while still ensuring predictable rasterization, is to translate x and y by 0.375.
     Such a translation keeps polygon and pixel image edges safely away from the centers of pixels,
     while moving line vertices close enough to the pixel centers.
     From: OpenGL Programming Guide (Red Book) - Appendix G "Programming Tips" - OpenGL Correctness Tips
  */

  return CD_OK;
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->gl_fonts)
  {
    int i;
    for (i = 0; i<ctxcanvas->gl_fonts_count; i++)
      ftglDestroyFont(ctxcanvas->gl_fonts[i].font);

    ctxcanvas->gl_fonts_max = 0;
    ctxcanvas->gl_fonts_count = 0;
    free(ctxcanvas->gl_fonts);
    ctxcanvas->gl_fonts = NULL;
  }

  if (ctxcanvas->utf8_buffer)
    free(ctxcanvas->utf8_buffer);

  free(ctxcanvas);
}

/******************************************************/

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  glFlush();
  (void)ctxcanvas;
}

/******************************************************/

static int cdclip(cdCtxCanvas *ctxcanvas, int clip_mode)
{
  switch (clip_mode)
  {
  case CD_CLIPOFF:
    if(glIsEnabled(GL_SCISSOR_TEST))
      glDisable(GL_SCISSOR_TEST);
    break;
  case CD_CLIPAREA:
    {
      glEnable(GL_SCISSOR_TEST);
      glScissor(ctxcanvas->canvas->clip_rect.xmin, ctxcanvas->canvas->clip_rect.ymin,
                ctxcanvas->canvas->clip_rect.xmax - ctxcanvas->canvas->clip_rect.xmin + 1,
                ctxcanvas->canvas->clip_rect.ymax - ctxcanvas->canvas->clip_rect.ymin + 1);
      break;
    }
  case CD_CLIPPOLYGON:
    clip_mode = ctxcanvas->canvas->clip_mode;
    break;
  case CD_CLIPREGION:
    clip_mode = ctxcanvas->canvas->clip_mode;
    break;
  }

  return clip_mode;
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
  {
    ctxcanvas->canvas->clip_rect.xmin = xmin;
    ctxcanvas->canvas->clip_rect.ymin = ymin;
    ctxcanvas->canvas->clip_rect.xmax = xmax;
    ctxcanvas->canvas->clip_rect.ymax = ymax;
    cdclip(ctxcanvas, CD_CLIPAREA);
  }
}


/******************************************************/

static int cdwritemode(cdCtxCanvas *ctxcanvas, int write_mode)
{
  switch (write_mode)
  {
  case CD_REPLACE:
    if(glIsEnabled(GL_COLOR_LOGIC_OP))
      glDisable(GL_COLOR_LOGIC_OP);
    break;
  case CD_XOR:
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);
    break;
  case CD_NOT_XOR:
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_EQUIV);
    break;
  }

  (void)ctxcanvas;
  return write_mode;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int hatch_style)
{
  GLubyte pattern[128]; /* 32x32 / 8 (1 bit per pixel) */
  int x, y, pos = 0;
 
  glEnable(GL_POLYGON_STIPPLE);
 
  for (y = 0; y < 128; y+=8)
  {
    for (x = 0; x < 8; x++)
      pattern[x+y] = hatches[hatch_style][pos];
    pos++;

    if(pos > 7) /* repeat the pattern */
      pos = 0;
  }
  glPolygonStipple(pattern);

  (void)ctxcanvas;
  return hatch_style;
}

static int cdinteriorstyle(cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
  case CD_HOLLOW:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case CD_SOLID:
  case CD_HATCH :
  case CD_STIPPLE:
  case CD_PATTERN:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  }

  switch (style)
  {
  case CD_STIPPLE:
  case CD_PATTERN:
  case CD_HOLLOW:
  case CD_SOLID:
    if(glIsEnabled(GL_POLYGON_STIPPLE))
      glDisable(GL_POLYGON_STIPPLE);
    break;
  case CD_HATCH:
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
    break;
  }

  return style;
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long int *pattern)
{
  (void)pattern;
  (void)m;
  (void)n;
  cdinteriorstyle(ctxcanvas, CD_SOLID);
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  (void)stipple;
  (void)m;
  (void)n;
  cdinteriorstyle(ctxcanvas, CD_SOLID);
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    if(glIsEnabled(GL_LINE_STIPPLE))
      glDisable(GL_LINE_STIPPLE);
    return style;
    break;
  case CD_DASHED:
  case CD_DOTTED:
  case CD_DASH_DOT:
  case CD_DASH_DOT_DOT:
  case CD_CUSTOM:
    glEnable(GL_LINE_STIPPLE);
    break;
  }

  switch (style)
  {
  case CD_DASHED:
    glLineStipple(1, 0x3F);   /* 00111111 */
    break;
  case CD_DOTTED:
    glLineStipple(1, 0x33);   /* 00110011 */
    break;
  case CD_DASH_DOT:
    glLineStipple(1, 0x33F);  /* 001100111111 */
    break;
  case CD_DASH_DOT_DOT:
    glLineStipple(1, 0x333F); /* 0011001100111111 */
    break;
  case CD_CUSTOM:
    /* style patterns more than 16 bits are not drawn completely */
    glLineStipple(1, (GLushort)*ctxcanvas->canvas->line_dashes);
    break;
  }

  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  if (width == 0) 
    width = 1;

  glLineWidth((GLfloat)width);

  (void)ctxcanvas;
  return width;
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  char filename[10240];
  FTGLfont* font;
  int res;

  /* try the pre-defined names and pre-defined style suffix */
  if (!cdGetFontFileNameDefault(type_face, style, filename))
  {
    /* try to find the file in the native system */
    if (!cdGetFontFileNameSystem(type_face, style, filename))
    {
      /* try the type_face as file name,
         here assume type_face is a full path */
      strcpy(filename, type_face);
    }
  }

  size = cdGetFontSizePoints(ctxcanvas->canvas, size);
  res = (int)(ctxcanvas->canvas->xres*25.4);

  font = cdglGetFont(ctxcanvas, filename, size, res);
  if (!font)
    return 0;

  ctxcanvas->font = font;
  return 1;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  if(!ctxcanvas->font)
    return;

  if (max_width) *max_width = cdRound(ftglGetFontMaxWidth(ctxcanvas->font));
  if (height)    *height = cdRound(ftglGetFontLineHeight(ctxcanvas->font));
  if (ascent)    *ascent = cdRound(ftglGetFontAscender(ctxcanvas->font));
  if (descent)   *descent = cdRound(-ftglGetFontDescender(ctxcanvas->font));
}

static long int cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{
  (void)ctxcanvas;

  glColor4ub(cdRed(color), 
             cdGreen(color), 
             cdBlue(color), 
             cdAlpha(color));

  return color;
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  GLclampf r = (GLclampf)cdRed(ctxcanvas->canvas->background)/255.0f; 
  GLclampf g = (GLclampf)cdGreen(ctxcanvas->canvas->background)/255.0f;
  GLclampf b = (GLclampf)cdBlue(ctxcanvas->canvas->background)/255.0f;
  GLclampf a = (GLclampf)cdAlpha(ctxcanvas->canvas->background)/255.0f;

  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    cdclip(ctxcanvas, CD_CLIPOFF);

  glClearColor(r, g, b, a);

  glClear(GL_COLOR_BUFFER_BIT);

  /* restore the foreground color */
  glColor4ub(cdRed(ctxcanvas->canvas->foreground), 
             cdGreen(ctxcanvas->canvas->foreground), 
             cdBlue(ctxcanvas->canvas->foreground), 
             cdAlpha(ctxcanvas->canvas->foreground));

  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    cdclip(ctxcanvas, CD_CLIPAREA);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  glBegin(GL_LINES);
    glVertex2d(x1, y1);
    glVertex2d(x2, y2);
  glEnd();

  (void)ctxcanvas;
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  glBegin(GL_LINES);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glEnd();

  (void)ctxcanvas;
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  glBegin(GL_LINE_LOOP);
    glVertex2d(xmin, ymin);
    glVertex2d(xmax, ymin);
    glVertex2d(xmax, ymax);
    glVertex2d(xmin, ymax);
  glEnd();

  (void)ctxcanvas;
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  glBegin(GL_LINE_LOOP);
  glVertex2i(xmin, ymin);
  glVertex2i(xmax, ymin);
  glVertex2i(xmax, ymax);
  glVertex2i(xmin, ymax);
  glEnd();

  (void)ctxcanvas;
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  /* must disable polygon smooth or fill may get diagonal lines */
  int smooth = glIsEnabled(GL_POLYGON_SMOOTH);
  if (smooth) glDisable(GL_POLYGON_SMOOTH);

  if (ctxcanvas->canvas->back_opacity == CD_OPAQUE && glIsEnabled(GL_POLYGON_STIPPLE))
  {
    /* draw twice, one with background color only, and one with foreground color */
    glDisable(GL_POLYGON_STIPPLE);
    glColor4ub(cdRed(ctxcanvas->canvas->background), 
               cdGreen(ctxcanvas->canvas->background), 
               cdBlue(ctxcanvas->canvas->background), 
               cdAlpha(ctxcanvas->canvas->background));

    glBegin(GL_QUADS);
      glVertex2d(xmin, ymin);
      glVertex2d(xmax, ymin);
      glVertex2d(xmax, ymax);
      glVertex2d(xmin, ymax);
    glEnd();

    /* restore the foreground color */
    glColor4ub(cdRed(ctxcanvas->canvas->foreground), 
               cdGreen(ctxcanvas->canvas->foreground), 
               cdBlue(ctxcanvas->canvas->foreground), 
               cdAlpha(ctxcanvas->canvas->foreground));
    glEnable(GL_POLYGON_STIPPLE);
  }

  glBegin(GL_QUADS);
    glVertex2d(xmin, ymin);
    glVertex2d(xmax, ymin);
    glVertex2d(xmax, ymax);
    glVertex2d(xmin, ymax);
  glEnd();

  if (smooth) glEnable(GL_POLYGON_SMOOTH);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  /* must disable polygon smooth or fill may get diagonal lines */
  int smooth = glIsEnabled(GL_POLYGON_SMOOTH);
  if (smooth) glDisable(GL_POLYGON_SMOOTH);

  if (ctxcanvas->canvas->back_opacity == CD_OPAQUE && glIsEnabled(GL_POLYGON_STIPPLE))
  {
    /* draw twice, one with background color only, and one with foreground color */
    glDisable(GL_POLYGON_STIPPLE);
    glColor4ub(cdRed(ctxcanvas->canvas->background),
               cdGreen(ctxcanvas->canvas->background),
               cdBlue(ctxcanvas->canvas->background),
               cdAlpha(ctxcanvas->canvas->background));

    glBegin(GL_QUADS);
    glVertex2i(xmin, ymin);
    glVertex2i(xmax, ymin);
    glVertex2i(xmax, ymax);
    glVertex2i(xmin, ymax);
    glEnd();

    /* restore the foreground color */
    glColor4ub(cdRed(ctxcanvas->canvas->foreground),
               cdGreen(ctxcanvas->canvas->foreground),
               cdBlue(ctxcanvas->canvas->foreground),
               cdAlpha(ctxcanvas->canvas->foreground));
    glEnable(GL_POLYGON_STIPPLE);
  }

  glBegin(GL_QUADS);
  glVertex2i(xmin, ymin);
  glVertex2i(xmax, ymin);
  glVertex2i(xmax, ymax);
  glVertex2i(xmin, ymax);
  glEnd();

  if (smooth) glEnable(GL_POLYGON_SMOOTH);
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  int stipple = 0;
  int w, h, baseline;
  double x_origin = x;
  double y_origin = y;

  if (!ctxcanvas->font)
    return;

  cdglStrConvertToUTF8(ctxcanvas, s, len);
  w = cdRound(ftglGetFontAdvance(ctxcanvas->font, ctxcanvas->utf8_buffer));
  h = cdRound(ftglGetFontLineHeight(ctxcanvas->font));
  baseline = h - cdRound(ftglGetFontAscender(ctxcanvas->font));

  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_BASE_RIGHT:
    case CD_NORTH_EAST:
    case CD_EAST:
    case CD_SOUTH_EAST:
      x = x - w;
      break;
    case CD_BASE_CENTER:
    case CD_CENTER:
    case CD_NORTH:
    case CD_SOUTH:
      x = x - w/2;
      break;
    case CD_BASE_LEFT:
    case CD_NORTH_WEST:
    case CD_WEST:
    case CD_SOUTH_WEST:
      x = x;
      break;
  }

  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_BASE_LEFT:
    case CD_BASE_CENTER:
    case CD_BASE_RIGHT:
      y = y;
      break;
    case CD_SOUTH_EAST:
    case CD_SOUTH_WEST:
    case CD_SOUTH:
      y = y + baseline;
      break;
    case CD_NORTH_EAST:
    case CD_NORTH:
    case CD_NORTH_WEST:
      y = y - h + baseline;
      break;
    case CD_CENTER:
    case CD_EAST:
    case CD_WEST:
      y = y - h/2 + baseline;
      break;
  }

  if (ctxcanvas->canvas->text_orientation != 0)
  {
    double angle = CD_DEG2RAD * ctxcanvas->canvas->text_orientation;
    double cos_angle = cos(angle);
    double sin_angle = sin(angle);
    cdfRotatePoint(ctxcanvas->canvas, x, y, x_origin, y_origin, &x, &y, sin_angle, cos_angle);
  }

  if(glIsEnabled(GL_POLYGON_STIPPLE))
  {
    stipple = 1;
    glDisable(GL_POLYGON_STIPPLE);
  }

  glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(ctxcanvas->canvas->text_orientation, 0, 0, 1);
    if (ctxcanvas->canvas->text_orientation == 0) ftglSetNearestFilter(ctxcanvas->font, 1);
    ftglRenderFont(ctxcanvas->font, ctxcanvas->utf8_buffer, FTGL_RENDER_ALL);
    glPopMatrix();

  if(stipple)
    glEnable(GL_POLYGON_STIPPLE);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, s, len);
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  if (!ctxcanvas->font)
    return;

  cdglStrConvertToUTF8(ctxcanvas, s, len);

  if (width)  *width = cdRound(ftglGetFontAdvance(ctxcanvas->font, ctxcanvas->utf8_buffer));
  if (height) *height = cdRound(ftglGetFontLineHeight(ctxcanvas->font));
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;
  int smooth = 0;
  
  if (mode == CD_CLIP)
    return;

  if (mode == CD_BEZIER)
  {
    int prec = 100;
    double* points = (double*)malloc(n * 3 * sizeof(double));

    for(i = 0; i < n; i++)
    {
      points[i*3+0] = (double)poly[i].x;
      points[i*3+1] = (double)poly[i].y;
      points[i*3+2] = 0;
    }

    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, n, points);
    glEnable(GL_MAP1_VERTEX_3);
    glMapGrid1f(prec, 0.0, 1.0);
    glEvalMesh1(GL_LINE, 0, prec);
    glDisable(GL_MAP1_VERTEX_3);

    free(points);
    return;
  }

  if (mode == CD_PATH)
  {
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    glBegin(GL_LINE_LOOP);
    break;
  case CD_OPEN_LINES :
    glBegin(GL_LINE_STRIP);
    break;
  case CD_FILL :
    /* must disable polygon smooth or fill may get diagonal lines */
    smooth = glIsEnabled(GL_POLYGON_SMOOTH);
    if (smooth) glDisable(GL_POLYGON_SMOOTH);

    if(ctxcanvas->canvas->back_opacity == CD_OPAQUE && glIsEnabled(GL_POLYGON_STIPPLE))
    {
      /* draw twice, one with background color only, and one with foreground color */
      glDisable(GL_POLYGON_STIPPLE);
      glColor4ub(cdRed(ctxcanvas->canvas->background), 
                 cdGreen(ctxcanvas->canvas->background), 
                 cdBlue(ctxcanvas->canvas->background), 
                 cdAlpha(ctxcanvas->canvas->background));

      glBegin(GL_POLYGON);
      for(i = 0; i < n; i++)
        glVertex2i(poly[i].x, poly[i].y);
      glEnd();

      /* restore the foreground color */
      glColor4ub(cdRed(ctxcanvas->canvas->foreground), 
                 cdGreen(ctxcanvas->canvas->foreground), 
                 cdBlue(ctxcanvas->canvas->foreground), 
                 cdAlpha(ctxcanvas->canvas->foreground));
      glEnable(GL_POLYGON_STIPPLE);
    }

    glBegin(GL_POLYGON);
    break;
  }

  for(i = 0; i < n; i++)
    glVertex2i(poly[i].x, poly[i].y);
  glEnd();

  if (smooth) glEnable(GL_POLYGON_SMOOTH);

  (void)ctxcanvas;
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;
  int smooth = 0;

  if (mode == CD_CLIP)
    return;

  if (mode == CD_BEZIER)
  {
    int prec = 100;
    double* points = (double*)malloc(n * 3 * sizeof(double));

    for(i = 0; i < n; i++)
    {
      points[i*3+0] = poly[i].x;
      points[i*3+1] = poly[i].y;
      points[i*3+2] = 0;
    }

    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, n, points);
    glEnable(GL_MAP1_VERTEX_3);
    glMapGrid1d(prec, 0.0, 1.0);
    glEvalMesh1(GL_LINE, 0, prec);
    glDisable(GL_MAP1_VERTEX_3);

    free(points);
    return;
  }

  if (mode == CD_PATH)
  {
    cdfSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    glBegin(GL_LINE_LOOP);
    break;
  case CD_OPEN_LINES :
    glBegin(GL_LINE_STRIP);
    break;
  case CD_FILL :
    /* must disable polygon smooth or fill may get diagonal lines */
    smooth = glIsEnabled(GL_POLYGON_SMOOTH);
    if (smooth) glDisable(GL_POLYGON_SMOOTH);

    if (ctxcanvas->canvas->back_opacity == CD_OPAQUE && glIsEnabled(GL_POLYGON_STIPPLE))
    {
      glDisable(GL_POLYGON_STIPPLE);
      glColor4ub(cdRed(ctxcanvas->canvas->background), 
                 cdGreen(ctxcanvas->canvas->background), 
                 cdBlue(ctxcanvas->canvas->background), 
                 cdAlpha(ctxcanvas->canvas->background));

      glBegin(GL_POLYGON);
      for(i = 0; i < n; i++)
        glVertex2d(poly[i].x, poly[i].y);
      glEnd();

      /* restore the foreground color */
      glColor4ub(cdRed(ctxcanvas->canvas->foreground), 
                 cdGreen(ctxcanvas->canvas->foreground), 
                 cdBlue(ctxcanvas->canvas->foreground), 
                 cdAlpha(ctxcanvas->canvas->foreground));
      glEnable(GL_POLYGON_STIPPLE);
    }

    glBegin(GL_POLYGON);
    break;
  }

  for(i = 0; i < n; i++)
    glVertex2d(poly[i].x, poly[i].y);
  glEnd();

  if (smooth) glEnable(GL_POLYGON_SMOOTH);

  (void)ctxcanvas;
}

/******************************************************/

static void cdgetimagergb(cdCtxCanvas *ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  GLubyte* glImage = (GLubyte*)malloc((w*3)*h);  /* each pixel uses 3 bytes (RGB) */

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, glImage);
  if (!glImage)
    return;

  cdglGetImageData(glImage, r, g, b, w, h);

  (void)ctxcanvas;

  free(glImage);
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, 
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  GLubyte* glImage;
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glImage = cdglCreateImageRGBA(xmin, ymin, rw, rh, r, g, b, NULL, iw);
  if (!glImage)
    return;

  cdglPutImage(ctxcanvas, rw, rh, glImage, GL_RGB, x, y, w, h);

  free(glImage);

  (void)ih;
  (void)ctxcanvas;
}

static void cdfputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, 
                              double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  GLubyte* glImage;
  int rw = xmax - xmin + 1;
  int rh = ymax - ymin + 1;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glImage = cdglCreateImageRGBA(xmin, ymin, rw, rh, r, g, b, NULL, iw);
  if (!glImage)
    return;

  cdglfPutImage(ctxcanvas, rw, rh, glImage, GL_RGB, x, y, w, h);

  free(glImage);

  (void)ih;
  (void)ctxcanvas;
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, 
                               int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int blend = 1;
  GLubyte* glImage;
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glImage = cdglCreateImageRGBA(xmin, ymin, rw, rh, r, g, b, a, iw);
  if (!glImage)
    return;

  if (!glIsEnabled(GL_BLEND))
  {
    blend = 0;
    glEnable(GL_BLEND);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  cdglPutImage(ctxcanvas, rw, rh, glImage, GL_RGBA, x, y, w, h);

  if (!blend)
    glDisable(GL_BLEND);

  free(glImage);

  (void)ih;
  (void)ctxcanvas;
}

static void cdfputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, 
                                double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int blend = 1;
  GLubyte* glImage;
  int rw = xmax - xmin + 1;
  int rh = ymax - ymin + 1;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glImage = cdglCreateImageRGBA(xmin, ymin, rw, rh, r, g, b, a, iw);
  if (!glImage)
    return;

  if (!glIsEnabled(GL_BLEND))
  {
    blend = 0;
    glEnable(GL_BLEND);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  cdglfPutImage(ctxcanvas, rw, rh, glImage, GL_RGBA, x, y, w, h);

  if (!blend)
    glDisable(GL_BLEND);

  free(glImage);

  (void)ih;
  (void)ctxcanvas;
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, 
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  GLubyte* glImage;
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glImage = cdglCreateImageMap(xmin, ymin, rw, rh, colors, index, iw);
  if (!glImage)
    return;

  cdglPutImage(ctxcanvas, rw, rh, glImage, GL_RGB, x, y, w, h);

  free(glImage);

  (void)ih;
  (void)ctxcanvas;
}

static void cdfputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, 
                               double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  GLubyte* glImage;
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glImage = cdglCreateImageMap(xmin, ymin, rw, rh, colors, index, iw);
  if (!glImage)
    return;

  cdglfPutImage(ctxcanvas, rw, rh, glImage, GL_RGB, x, y, w, h);

  free(glImage);

  (void)ih;
  (void)ctxcanvas;
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  glColor4ub(cdRed(color), 
             cdGreen(color), 
             cdBlue(color), 
             cdAlpha(color));

  /* Draw pixel */
  glPointSize(1);
  glBegin(GL_POINTS);
    glVertex2i(x, y);
  glEnd();

  /* restore the foreground color */
  glColor4ub(cdRed(ctxcanvas->canvas->foreground), 
             cdGreen(ctxcanvas->canvas->foreground), 
             cdBlue(ctxcanvas->canvas->foreground), 
             cdAlpha(ctxcanvas->canvas->foreground));

  (void)ctxcanvas;
}

static void cdfpixel(cdCtxCanvas *ctxcanvas, double x, double y, long int color)
{
  glColor4ub(cdRed(color), 
             cdGreen(color), 
             cdBlue(color), 
             cdAlpha(color));

  /* Draw pixel */
  glPointSize(1);
  glBegin(GL_POINTS);
    glVertex2d(x, y);
  glEnd();

  /* restore the foreground color */
  glColor4ub(cdRed(ctxcanvas->canvas->foreground), 
             cdGreen(ctxcanvas->canvas->foreground), 
             cdBlue(ctxcanvas->canvas->foreground), 
             cdAlpha(ctxcanvas->canvas->foreground));

  (void)ctxcanvas;
}

static cdCtxImage *cdcreateimage (cdCtxCanvas *ctxcanvas, int w, int h)
{
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));

  ctximage->w = w;
  ctximage->h = h;

  ctximage->img = (GLubyte*)malloc(w*h*4);  /* each pixel uses 4 bytes (RGBA) */
  if (iGLIsOpenGL2orMore())
    ctximage->texture = cdglCreateTexture();
  else
    ctximage->texture = 0;

  if (!ctximage->img)
  {
    free(ctximage);
    return (void*)0;
  }

  (void)ctxcanvas;
  return (void*)ctximage;
}

static void cdgetimage (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glReadPixels(x, y - ctximage->h + 1, ctximage->w, ctximage->h, GL_RGBA, GL_UNSIGNED_BYTE, ctximage->img);

  if (ctximage->texture)
  {
    glBindTexture(GL_TEXTURE_2D, ctximage->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ctximage->w, ctximage->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ctximage->img);
  }

  (void)ctxcanvas;
}

static void cdputimagerect (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  int rw = xmax - xmin + 1;
  int rh = ymax - ymin + 1;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if (rw == (int)ctximage->w && rh == (int)ctximage->h)
  {
    if (ctximage->texture)
      cdglDrawTextureImage(ctxcanvas, ctximage->texture, x, y, ctximage->w, ctximage->h);
    else
    {
      glRasterPos2i(x, y);
      glDrawPixels(ctximage->w, ctximage->h, GL_RGBA, GL_UNSIGNED_BYTE, ctximage->img);
    }
  }
  else
  {
    GLubyte* glImage = cdglCreateImage(xmin, ymin, rw, rh, ctximage->img, ctximage->w);
    if (!glImage)
      return;

    cdglPutImage(ctxcanvas, rw, rh, glImage, GL_RGBA, x, y, rw, rh);

    free(glImage);
  }

  (void)ctxcanvas;
}

static void cdkillimage (cdCtxImage *ctximage)
{
  if (ctximage->texture)
    glDeleteTextures(1, &(ctximage->texture));

  free(ctximage->img);
  free(ctximage);
}

static void cdscrollarea (cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  glRasterPos2i(xmin+dx, ymin+dy);
  glCopyPixels(xmin, ymin, xmax-xmin+1, ymax-ymin+1, GL_RGBA);

  (void)ctxcanvas;
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    GLdouble transformMTX[4][4];

    transformMTX[0][0] = matrix[0];   transformMTX[0][1] = matrix[1];   transformMTX[0][2] = 0.0;         transformMTX[0][3] = 0.0;
    transformMTX[1][0] = matrix[2];   transformMTX[1][1] = matrix[3];   transformMTX[1][2] = 0.0;         transformMTX[1][3] = 0.0;
    transformMTX[2][0] = 0.0;         transformMTX[2][1] = 0.0;         transformMTX[2][2] = 1.0;         transformMTX[2][3] = 0.0;
    transformMTX[3][0] = matrix[4];   transformMTX[3][1] = matrix[5];   transformMTX[3][2] = 0.0;         transformMTX[3][3] = 1.0;

    glLoadIdentity();
    glMultMatrixd(&transformMTX[0][0]);
  }
  else
    glLoadIdentity();

  (void)ctxcanvas;
}

/******************************************************************/

static void set_alpha_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
  {
    glDisable(GL_BLEND);
  }
  else
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  (void)ctxcanvas;
}

static char* get_alpha_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;

  if (glIsEnabled(GL_BLEND))
    return "1";
  else
    return "0";
}

static cdAttribute alpha_attrib =
{
  "ALPHA",
  set_alpha_attrib,
  get_alpha_attrib
};

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
  {
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
  }
  else
  {
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  }

  (void)ctxcanvas;
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;

  if (glIsEnabled(GL_LINE_SMOOTH))
    return "1";
  else
    return "0";
}

static cdAttribute aa_attrib =
{
  "ANTIALIAS",
  set_aa_attrib,
  get_aa_attrib
};

static void set_interp_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && (cdStrEqualNoCase(data, "FAST") || cdStrEqualNoCase(data, "NEAREST")))
    ctxcanvas->texture_filter = GL_NEAREST;
  else 
    ctxcanvas->texture_filter = GL_LINEAR;
}

static char* get_interp_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->texture_filter == GL_LINEAR)
    return "BILINEAR";
  else 
    return "NEAREST";
}

static cdAttribute interp_attrib =
{
  "IMGINTERP",
  set_interp_attrib,
  get_interp_attrib
};

static void set_utf8mode_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
    ctxcanvas->utf8mode = 0;
  else
    ctxcanvas->utf8mode = 1;
}

static char* get_utf8mode_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->utf8mode)
    return "1";
  else
    return "0";
}

static cdAttribute utf8mode_attrib =
{
  "UTF8MODE",
  set_utf8mode_attrib,
  get_utf8mode_attrib
}; 

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    sscanf(data, "%lg %d %d", &ctxcanvas->rotate_angle,
                              &ctxcanvas->rotate_center_x,
                              &ctxcanvas->rotate_center_y);

    cdCanvasTransformTranslate(ctxcanvas->canvas, ctxcanvas->rotate_center_x, ctxcanvas->rotate_center_y);
    cdCanvasTransformRotate(ctxcanvas->canvas, ctxcanvas->rotate_angle);
    cdCanvasTransformTranslate(ctxcanvas->canvas, -ctxcanvas->rotate_center_x, -ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;

    cdCanvasTransform(ctxcanvas->canvas, NULL);
  }
}

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->rotate_angle)
    return NULL;

  sprintf(data, "%g %d %d", ctxcanvas->rotate_angle,
                            ctxcanvas->rotate_center_x,
                            ctxcanvas->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
};

static void set_size_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    cdCanvas* canvas = ctxcanvas->canvas;
    sscanf(data, "%dx%d %lg", &canvas->w, &canvas->h, &(canvas->xres));
    canvas->yres = canvas->xres;
    canvas->w_mm = ((double)canvas->w) / canvas->xres;
    canvas->h_mm = ((double)canvas->h) / canvas->yres;
  }
}

static cdAttribute size_attrib =
{
  "SIZE",
  set_size_attrib,
  NULL
};

static char* get_version_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return (char*)glGetString(GL_VERSION);
}

static cdAttribute version_attrib =
{
  "GLVERSION",
  NULL,
  get_version_attrib
};

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  int w = 0, h = 0;
  double res = 3.78;
  char* str_data = (char*)data;

  sscanf(str_data, "%dx%d %lg", &w, &h, &res);
  if (w == 0) w = 1;
  if (h == 0) h = 1;

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  canvas->w = w;
  canvas->h = h;
  canvas->xres = res;
  canvas->yres = res;
  canvas->bpp = 24;

  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;

  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  ctxcanvas->utf8_buffer = NULL;
  ctxcanvas->texture_filter = GL_LINEAR;

  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &version_attrib);
  cdRegisterAttribute(canvas, &size_attrib);
  cdRegisterAttribute(canvas, &alpha_attrib);
  cdRegisterAttribute(canvas, &aa_attrib);
  cdRegisterAttribute(canvas, &utf8mode_attrib);
  cdRegisterAttribute(canvas, &interp_attrib);

  cdCanvasSetAttribute(canvas, "ALPHA", "1");
  cdCanvasSetAttribute(canvas, "ANTIALIAS", "1");
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;
  
  canvas->cxPixel  = cdpixel;
  canvas->cxLine   = cdline;
  canvas->cxPoly   = cdpoly;
  canvas->cxRect   = cdrect;
  canvas->cxBox    = cdbox;
  canvas->cxArc = cdSimArc;
  canvas->cxSector = cdSimSector;
  canvas->cxChord = cdSimChord;

  canvas->cxText = cdtext;
  canvas->cxFont = cdfont;
  canvas->cxGetFontDim  = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxWriteMode = cdwritemode;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxForeground = cdforeground;
  canvas->cxTransform  = cdtransform;

  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfSimArc;
  canvas->cxFSector = cdfSimSector;
  canvas->cxFChord = cdfSimChord;
  canvas->cxFText = cdftext;

  canvas->cxScrollArea = cdscrollarea;
  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxFPutImageRectRGB = cdfputimagerectrgb;
  canvas->cxFPutImageRectRGBA = cdfputimagerectrgba;
  canvas->cxFPutImageRectMap = cdfputimagerectmap;
  canvas->cxFPixel = cdfpixel;

  canvas->cxActivate = cdactivate;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdGLContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_PALETTE | CD_CAP_LINEJOIN | CD_CAP_LINECAP |
                 CD_CAP_REGION | CD_CAP_STIPPLE | CD_CAP_PATTERN),
  CD_CTX_WINDOW,
  cdcreatecanvas,
  cdinittable,
  NULL,              
  NULL,
};

cdContext* cdContextGL(void)
{
  return &cdGLContext;
}
