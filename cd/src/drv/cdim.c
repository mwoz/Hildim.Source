/** \file
 * \brief Image RGB Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h> 
#include <memory.h> 
#include <math.h> 
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <im.h>
#include <im_image.h>

#include "cd.h"
#include "wd.h"
#include "cd_private.h"
#include "cdirgb.h"
#include "cdim.h"


void cdCanvasPatternImImage(cdCanvas* canvas, const imImage* image)
{
  long* pattern;
  int i;

  assert(canvas);
  assert(image);
  if (!_cdCheckCanvas(canvas)) return;

  if (image->data_type != IM_BYTE)
    return;

  pattern = malloc(image->count * sizeof(long));
  memset(pattern, 0, image->count * sizeof(long));

  if (image->color_space == IM_RGB)
  {
    unsigned char* r = (unsigned char*)image->data[0];
    unsigned char* g = (unsigned char*)image->data[1];
    unsigned char* b = (unsigned char*)image->data[2];
    unsigned char* a = NULL;

    if (image->has_alpha)
      a = (unsigned char*)image->data[3];

    for (i = 0; i < image->count; i++)
    {
      if (image->has_alpha)
        pattern[i] = cdEncodeColorAlpha(r[i], g[i], b[i], a[i]);
      else
        pattern[i] = cdEncodeColor(r[i], g[i], b[i]);
    }
  }
  else if (image->color_space == IM_MAP || image->color_space == IM_GRAY || image->color_space == IM_BINARY)
  {
    unsigned char* map = (unsigned char*)image->data[0];

    for (i = 0; i < image->count; i++)
    {
      pattern[i] = image->palette[map[i]];
    }
  }

  cdCanvasPattern(canvas, image->width, image->height, pattern);
  free(pattern);
}

void cdCanvasStippleImImage(cdCanvas* canvas, const imImage* image)
{
  unsigned char* stipple;

  assert(canvas);
  assert(image);
  if (!_cdCheckCanvas(canvas)) return;

  if (image->data_type != IM_BYTE || image->color_space != IM_BINARY)
    return;

  stipple = (unsigned char*)image->data[0];

  cdCanvasStipple(canvas, image->width, image->height, stipple);
}

void cdCanvasPutImImage(cdCanvas* canvas, const imImage* image, int x, int y, int w, int h)
{                          
  if (image->data_type != IM_BYTE)
    return;

  if (image->color_space == IM_RGB)                                      
  {                                                                       
    if (image->has_alpha)                                                
      cdCanvasPutImageRectRGBA(canvas, image->width, image->height,    
                        (unsigned char*)image->data[0],                  
                        (unsigned char*)image->data[1],                  
                        (unsigned char*)image->data[2],                  
                        (unsigned char*)image->data[3],                  
                        x, y, w, h, 0, 0, 0, 0);      
   else                                                                  
      cdCanvasPutImageRectRGB(canvas, image->width, image->height,     
                        (unsigned char*)image->data[0],                  
                        (unsigned char*)image->data[1],                  
                        (unsigned char*)image->data[2],                  
                        x, y, w, h, 0, 0, 0, 0);      
   }                                                                       
  else if (image->color_space == IM_MAP || image->color_space == IM_GRAY || image->color_space == IM_BINARY)
    cdCanvasPutImageRectMap(canvas, image->width, image->height,       
                        (unsigned char*)image->data[0], image->palette,   
                        x, y, w, h, 0, 0, 0, 0);        
}    

void cdCanvasGetImImage(cdCanvas* canvas, imImage* image, int x, int y)
{
  cdCanvasGetImageRGB(canvas,
                      (unsigned char*)image->data[0],
                      (unsigned char*)image->data[1],
                      (unsigned char*)image->data[2],
                      x, y, image->width, image->height);
}

void cdfCanvasPutImImage(cdCanvas* canvas, const imImage* image, double x, double y, double w, double h)
{
  if (image->data_type != IM_BYTE)
    return;

  if (image->color_space == IM_RGB)
  {
    if (image->has_alpha)
      cdfCanvasPutImageRectRGBA(canvas, image->width, image->height,
      (unsigned char*)image->data[0],
      (unsigned char*)image->data[1],
      (unsigned char*)image->data[2],
      (unsigned char*)image->data[3],
      x, y, w, h, 0, 0, 0, 0);
    else
      cdfCanvasPutImageRectRGB(canvas, image->width, image->height,
      (unsigned char*)image->data[0],
      (unsigned char*)image->data[1],
      (unsigned char*)image->data[2],
      x, y, w, h, 0, 0, 0, 0);
  }
  else if (image->color_space == IM_MAP || image->color_space == IM_GRAY || image->color_space == IM_BINARY)
    cdfCanvasPutImageRectMap(canvas, image->width, image->height,
    (unsigned char*)image->data[0], image->palette,
    x, y, w, h, 0, 0, 0, 0);
}

void wdCanvasPutImImage(cdCanvas* canvas, const imImage* image, double x, double y, double w, double h)
{
  if (image->data_type != IM_BYTE)
    return;

  if (image->color_space == IM_RGB)
  {
    if (image->has_alpha)
      wdCanvasPutImageRectRGBA(canvas, image->width, image->height,
      (unsigned char*)image->data[0],
      (unsigned char*)image->data[1],
      (unsigned char*)image->data[2],
      (unsigned char*)image->data[3],
      x, y, w, h, 0, 0, 0, 0);
    else
      wdCanvasPutImageRectRGB(canvas, image->width, image->height,
      (unsigned char*)image->data[0],
      (unsigned char*)image->data[1],
      (unsigned char*)image->data[2],
      x, y, w, h, 0, 0, 0, 0);
  }
  else if (image->color_space == IM_MAP || image->color_space == IM_GRAY || image->color_space == IM_BINARY)
    wdCanvasPutImageRectMap(canvas, image->width, image->height,
    (unsigned char*)image->data[0], image->palette,
    x, y, w, h, 0, 0, 0, 0);
}

void wdCanvasGetImImage(cdCanvas* canvas, imImage* image, double x, double y)
{
  wdCanvasGetImageRGB(canvas,
                       (unsigned char*)image->data[0],
                       (unsigned char*)image->data[1],
                       (unsigned char*)image->data[2],
                       x, y, image->width, image->height);
}

static void(*cdcreatecanvasIMAGERGB)(cdCanvas* canvas, void *data) = NULL;

static void cdcreatecanvas(cdCanvas* canvas, imImage* image)
{
  char data_s[100];
  double res = 0;
  const char* res_unit;

  if (image->color_space != IM_RGB || image->data_type != IM_BYTE)
    return;

  res_unit = imImageGetAttribString(image, "ResolutionUnit");
  if (res_unit)
  {
    double xres = imImageGetAttribReal(image, "XResolution", 0);
    if (xres)
    {
      /* to DPM */
      if (res_unit[0] == 'D' &&
          res_unit[1] == 'P' &&
          res_unit[2] == 'I')
          res = xres / (10. * 2.54);
      else  /* DPC */
        res = xres / 10.0;
    }
  }

  if (!res)
  {

    if (image->has_alpha)
      sprintf(data_s, "%dx%d %p %p %p %p -a", image->width, image->height,
                                              image->data[0], image->data[1], image->data[2], image->data[3]);
    else
      sprintf(data_s, "%dx%d %p %p %p", image->width, image->height,
                                        image->data[0], image->data[1], image->data[2]);
  }
  else
  {
    if (image->has_alpha)
      sprintf(data_s, "%dx%d %p %p %p %p -r%g -a", image->width, image->height,
                                                   image->data[0], image->data[1], image->data[2], image->data[3], res);
    else
      sprintf(data_s, "%dx%d %p %p %p -r%g", image->width, image->height,
                                             image->data[0], image->data[1], image->data[2], res);
  }

  cdcreatecanvasIMAGERGB(canvas, data_s);
}

static cdContext cdImImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY |  
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | 
                 CD_CAP_PALETTE ),
  CD_CTX_IMAGE,
  cdcreatecanvas,
  NULL,
  NULL,             
  NULL, 
};

cdContext* cdContextImImage(void)
{
  if (cdImImageContext.cxInitTable == NULL)
  {
    cdContext* irgb_context = cdContextImageRGB();
    cdImImageContext.cxInitTable = irgb_context->cxInitTable;

    cdcreatecanvasIMAGERGB = irgb_context->cxCreateCanvas;
  }

  return &cdImImageContext;
}
