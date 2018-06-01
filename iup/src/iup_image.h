/** \file
 * \brief Image Resources (not exported API)
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_IMAGE_H 
#define __IUP_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

void* iupdrvImageCreateIcon(Ihandle *ih);
void* iupdrvImageCreateCursor(Ihandle *ih);
void* iupdrvImageCreateImage(Ihandle *ih, const char* bgcolor, int make_inactive);

enum {IUPIMAGE_IMAGE, IUPIMAGE_ICON, IUPIMAGE_CURSOR};
void* iupdrvImageLoad(const char* name, int type);
void  iupdrvImageDestroy(void* handle, int type);
int   iupdrvImageGetInfo(void* handle, int *w, int *h, int *bpp);  /* only for IUPIMAGE_IMAGE */
void  iupdrvImageGetData(void* handle, unsigned char* imgdata);   /* only for IUPIMAGE_IMAGE */

void* iupImageGetIcon(const char* name);
void* iupImageGetCursor(const char* name);
void* iupImageGetImage(const char* name, Ihandle* parent, int make_inactive, const char* bgcolor);
void iupImageGetInfo(const char* name, int *w, int *h, int *bpp);
void iupImageRemoveFromCache(Ihandle* ih, void* handle);

Ihandle* iupImageGetHandle(const char* name);

typedef struct _iupColor { 
  unsigned char r, g, b, a; 
} iupColor;

int iupImageInitColorTable(Ihandle *ih, iupColor* colors, int *colors_count);
void iupImageInitNonBgColors(Ihandle* ih, unsigned char *colors);
void iupImageColorMakeInactive(unsigned char *r, unsigned char *g, unsigned char *b, 
                               unsigned char bg_r, unsigned char bg_g, unsigned char bg_b);
int iupImageNormBpp(int bpp);

#define iupALPHABLEND(_src,_dst,_alpha) (unsigned char)(((_src) * (_alpha) + (_dst) * (255 - (_alpha))) / 255)

/* In Windows, RAW data is a DIB handle. 
   imgdata here is bottom-up arranged and has separated planes
   Used only by IupGetImageNativeHandle and IupGetNativeHandleImage */
void* iupdrvImageCreateImageRaw(int width, int height, int bpp, iupColor* colors, int colors_count, unsigned char *imgdata);
int iupdrvImageGetRawInfo(void* handle, int *w, int *h, int *bpp, iupColor* colors, int *colors_count);
void iupdrvImageGetRawData(void* handle, unsigned char* imgdata);

void iupImageSetHandleFromLoaded(const char* name, void* handle);
Ihandle* iupImageGetImageFromName(const char* name);

void iupImageStockInit(void);
void iupImageStockFinish(void);
typedef Ihandle* (*iupImageStockCreateFunc)(void);
void iupImageStockSet(const char *name, iupImageStockCreateFunc func, const char* native_name);
void iupImageStockSetNoResize(const char *name, iupImageStockCreateFunc func, const char* native_name);
void iupImageStockLoadAll(void);  /* Used only in IupView */
int iupImageStockGetSize(void);
void iupImageStockGet(const char* name, Ihandle* *ih, const char* *native_name);


#ifdef __cplusplus
}
#endif

#endif
