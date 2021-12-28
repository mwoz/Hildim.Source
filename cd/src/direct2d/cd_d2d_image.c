#include "cd_d2d.h"
#include "cd.h"

/* According to MSDN, GUID_WICPixelFormat32bppPBGRA is the recommended pixel
* format for cooperation with Direct2D. Note we define it here manually to
* avoid need to link with UUID.LIB. */
const GUID wic_cd_pixel_format =
{ 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10 } };


static void bufferMap2Bitmap(BYTE* Scan0, INT dstStride, UINT width, UINT height, const BYTE* map, const long* palette)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = (height - 1 - j) * width;
    const BYTE* map_line = map + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int map_index = map_line[i];
      long color = palette[map_index];

      int offset_data = i * 4;
      line_data[offset_data + 0] = cdBlue(color);
      line_data[offset_data + 1] = cdGreen(color);
      line_data[offset_data + 2] = cdRed(color);
      line_data[offset_data + 3] = cdAlpha(color);
    }
  }
}

static void bufferRGB2Bitmap(BYTE* Scan0, INT dstStride, UINT width, UINT height, const unsigned char *red, const unsigned char *green, const unsigned char *blue)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = (height - 1 - j) * width;
    const BYTE* r_line = red + line_offset;
    const BYTE* g_line = green + line_offset;
    const BYTE* b_line = blue + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      unsigned char r = r_line[i];
      unsigned char g = g_line[i];
      unsigned char b = b_line[i];

      int offset_data = i * 4;
      line_data[offset_data + 0] = b;  /* Blue */
      line_data[offset_data + 1] = g;  /* Green */
      line_data[offset_data + 2] = r;  /* Red */
      line_data[offset_data + 3] = 255;
    }
  }
}

static void bufferRGBA2Bitmap(BYTE* Scan0, INT dstStride, UINT width, UINT height, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = (height - 1 - j) * width;
    const BYTE* r_line = red + line_offset;
    const BYTE* g_line = green + line_offset;
    const BYTE* b_line = blue + line_offset;
    const BYTE* a_line = alpha + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      unsigned char r = r_line[i];
      unsigned char g = g_line[i];
      unsigned char b = b_line[i];
      unsigned char a = a_line[i];

      /* pre-multiplied alpha */
      int offset_data = i * 4;
      line_data[offset_data + 0] = (b * a) / 255;  /* Blue */
      line_data[offset_data + 1] = (g * a) / 255;  /* Green */
      line_data[offset_data + 2] = (r * a) / 255;  /* Red */
      line_data[offset_data + 3] = a;  /* Alpha */
    }
  }
}

static void pattern2Bitmap(BYTE* Scan0, INT dstStride, UINT width, UINT height, const long *pattern)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = (height - 1 - j) * width;
    const long* pat_line = pattern + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int offset_data = i * 4;
      unsigned char r = cdRed(pat_line[i]);
      unsigned char g = cdGreen(pat_line[i]);
      unsigned char b = cdBlue(pat_line[i]);
      unsigned char a = cdAlpha(pat_line[i]);

      line_data[offset_data + 0] = (b * a) / 255; /* Blue */
      line_data[offset_data + 1] = (g * a) / 255; /* Green */
      line_data[offset_data + 2] = (r * a) / 255; /* Red */
      line_data[offset_data + 3] = a;
    }
  }
}

static void stipple2Bitmap(BYTE* Scan0, INT dstStride, UINT width, UINT height, const BYTE* map, const int back_opacity, const long foreground, const long background)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = (height - 1 - j) * width;
    const BYTE* map_line = map + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int offset_data = i * 4;
      unsigned char r, g, b, a;

      int map_index = map_line[i];
      int color;
      if (map_index == 0)
        color = background;
      else
        color = foreground;

      r = cdRed(color);
      g = cdGreen(color);
      b = cdBlue(color);

      if (map_index == 0 && back_opacity)
        a = 0;  /* fully transparent if back_opacity is transparent */
      else
        a = cdAlpha(color);

      line_data[offset_data + 0] = (b * a) / 255; /* Blue */
      line_data[offset_data + 1] = (g * a) / 255; /* Green */
      line_data[offset_data + 2] = (r * a) / 255; /* Red */
      line_data[offset_data + 3] = a;
    }
  }
}

IWICBitmap* d2dCreateImageFromBufferRGB(UINT uWidth, UINT uHeight, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha)
{
  IWICBitmap* wic_bitmap = NULL;
  HRESULT hr;
  WICRect rect;
  IWICBitmapLock *bitmap_lock = NULL;
  UINT cbBufferSize = 0;
  UINT dstStride = 0;
  BYTE *Scan0 = NULL;

  if (wic_cd_factory == NULL) {
    return NULL;
  }

  hr = IWICImagingFactory_CreateBitmap(wic_cd_factory, uWidth, uHeight, &wic_cd_pixel_format, WICBitmapCacheOnDemand, &wic_bitmap);   /* GUID_WICPixelFormat32bppPBGRA - pre-multiplied alpha, BGRA order */
  if (FAILED(hr)) {
    return NULL;
  }

  rect.X = 0;
  rect.Y = 0;
  rect.Width = uWidth;
  rect.Height = uHeight;

  hr = IWICBitmap_Lock(wic_bitmap, &rect, WICBitmapLockWrite, &bitmap_lock);
  if (FAILED(hr)) {
    IWICBitmap_Release(wic_bitmap);
    return NULL;
  }

  IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
  IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

  if (alpha)
    bufferRGBA2Bitmap(Scan0, dstStride, uWidth, uHeight, red, green, blue, alpha);
  else
    bufferRGB2Bitmap(Scan0, dstStride, uWidth, uHeight, red, green, blue);
 
  IWICBitmapLock_Release(bitmap_lock);
  return wic_bitmap;
}

IWICBitmap* d2dCreateImageFromBufferMap(UINT uWidth, UINT uHeight, const unsigned char *map, const long* cPalette)
{
  IWICBitmap* wic_bitmap = NULL;
  HRESULT hr;
  WICRect rect;
  IWICBitmapLock *bitmap_lock = NULL;
  UINT cbBufferSize = 0;
  UINT dstStride = 0;
  BYTE *Scan0 = NULL;

  if (wic_cd_factory == NULL) {
    return NULL;
  }

  hr = IWICImagingFactory_CreateBitmap(wic_cd_factory, uWidth, uHeight, &wic_cd_pixel_format, WICBitmapCacheOnDemand, &wic_bitmap);   /* GUID_WICPixelFormat32bppPBGRA - pre-multiplied alpha, BGRA order */
  if (FAILED(hr)) {
    return NULL;
  }

  rect.X = 0;
  rect.Y = 0;
  rect.Width = uWidth;
  rect.Height = uHeight;

  hr = IWICBitmap_Lock(wic_bitmap, &rect, WICBitmapLockWrite, &bitmap_lock);
  if (FAILED(hr)) {
    IWICBitmap_Release(wic_bitmap);
    return NULL;
  }

  IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
  IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

  bufferMap2Bitmap(Scan0, dstStride, uWidth, uHeight, map, cPalette);

  IWICBitmapLock_Release(bitmap_lock);
  return wic_bitmap;
}

IWICBitmap* d2dCreateImageFromHatch(int style, int hsize, int back_opacity, long foreground, long background)
{
  dummy_ID2D1RenderTarget *target;
  IWICBitmap *wic_bitmap;
  dummy_D2D1_RENDER_TARGET_PROPERTIES props;
  dummy_ID2D1Brush *brush;
  int hhalf = hsize / 2;

  wic_bitmap = d2dCreateImage(hsize, hsize);
  if (!wic_bitmap)
    return NULL;

  props.type = dummy_D2D1_RENDER_TARGET_TYPE_DEFAULT;
  props.pixelFormat.format = dummy_DXGI_FORMAT_B8G8R8A8_UNORM;
  props.pixelFormat.alphaMode = dummy_D2D1_ALPHA_MODE_PREMULTIPLIED;
  props.dpiX = 0.0f;
  props.dpiY = 0.0f;
  props.usage = 0;
  props.minLevel = dummy_D2D1_FEATURE_LEVEL_DEFAULT;

  dummy_ID2D1Factory_CreateWicBitmapRenderTarget(d2d_cd_factory, wic_bitmap, &props, &target);

  dummy_ID2D1RenderTarget_BeginDraw(target);

  if (back_opacity == CD_OPAQUE)
  {
    brush = d2dCreateSolidBrush(target, background);
    d2dFillRect(target, brush, 0.0f, 0.0f, (float)hsize, (float)hsize);
    dummy_ID2D1Brush_Release(brush);
  }

  brush = d2dCreateSolidBrush(target, foreground);

  switch (style)
  {
  case CD_HORIZONTAL:
    d2dDrawLine(target, brush, 0.0f, (float)hhalf, (float)hsize, (float)hhalf, 1.0f, NULL);
    break;
  case CD_VERTICAL:
    d2dDrawLine(target, brush, (float)hhalf, 0.0f, (float)hhalf, (float)hsize, 1.0f, NULL);
    break;
  case CD_BDIAGONAL:
    d2dDrawLine(target, brush, 0.0f, (float)hsize, (float)hsize, 0.0f, 1.0f, NULL);
    break;
  case CD_FDIAGONAL:
    d2dDrawLine(target, brush, 0.0f, 0.0f, (float)hsize, (float)hsize, 1.0f, NULL);
    break;
  case CD_CROSS:
    d2dDrawLine(target, brush, (float)hsize, 0.0f, (float)hsize, (float)hsize, 1.0f, NULL);
    d2dDrawLine(target, brush, 0.0f, (float)hhalf, (float)hsize, (float)hhalf, 1.0f, NULL);
    break;
  case CD_DIAGCROSS:
    d2dDrawLine(target, brush, 0.0f, 0.0f, (float)hsize, (float)hsize, 1.0f, NULL);
    d2dDrawLine(target, brush, (float)hsize, 0.0f, 0.0f, (float)hsize, 1.0f, NULL);
    break;
  }

  dummy_ID2D1Brush_Release(brush);

  dummy_ID2D1RenderTarget_EndDraw(target, NULL, NULL);

  dummy_ID2D1RenderTarget_Release(target);

  return wic_bitmap;
}

IWICBitmap* d2dCreateImageFromPattern(UINT uWidth, UINT uHeight, const long *pattern)
{
  IWICBitmap* wic_bitmap = NULL;
  HRESULT hr;
  WICRect rect;
  IWICBitmapLock *bitmap_lock = NULL;
  UINT cbBufferSize = 0;
  UINT dstStride = 0;
  BYTE *Scan0 = NULL;

  if (wic_cd_factory == NULL) {
    return NULL;
  }

  hr = IWICImagingFactory_CreateBitmap(wic_cd_factory, uWidth, uHeight, &wic_cd_pixel_format, WICBitmapCacheOnDemand, &wic_bitmap);   /* GUID_WICPixelFormat32bppPBGRA - pre-multiplied alpha, BGRA order */
  if (FAILED(hr)) {
    return NULL;
  }

  rect.X = 0;
  rect.Y = 0;
  rect.Width = uWidth;
  rect.Height = uHeight;

  hr = IWICBitmap_Lock(wic_bitmap, &rect, WICBitmapLockWrite, &bitmap_lock);
  if (FAILED(hr)) {
    IWICBitmap_Release(wic_bitmap);
    return NULL;
  }

  IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
  IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

  pattern2Bitmap(Scan0, dstStride, uWidth, uHeight, pattern);

  IWICBitmapLock_Release(bitmap_lock);
  return wic_bitmap;
}

IWICBitmap* d2dCreateImageFromStipple(UINT uWidth, UINT uHeight, const unsigned char *stipple, int back_opacity, long foreground, long background)
{
  IWICBitmap* wic_bitmap = NULL;
  HRESULT hr;
  WICRect rect;
  IWICBitmapLock *bitmap_lock = NULL;
  UINT cbBufferSize = 0;
  UINT dstStride = 0;
  BYTE *Scan0 = NULL;

  if (wic_cd_factory == NULL) {
    return NULL;
  }

  hr = IWICImagingFactory_CreateBitmap(wic_cd_factory, uWidth, uHeight, &wic_cd_pixel_format, WICBitmapCacheOnDemand, &wic_bitmap);   /* GUID_WICPixelFormat32bppPBGRA - pre-multiplied alpha, BGRA order */
  if (FAILED(hr)) {
    return NULL;
  }

  rect.X = 0;
  rect.Y = 0;
  rect.Width = uWidth;
  rect.Height = uHeight;

  hr = IWICBitmap_Lock(wic_bitmap, &rect, WICBitmapLockWrite, &bitmap_lock);
  if (FAILED(hr)) {
    IWICBitmap_Release(wic_bitmap);
    return NULL;
  }

  IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
  IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

  stipple2Bitmap(Scan0, dstStride, uWidth, uHeight, stipple, back_opacity, foreground, background);

  IWICBitmapLock_Release(bitmap_lock);
  return wic_bitmap;
}

IWICBitmap *d2dCreateImage(UINT uWidth, UINT uHeight)
{
  IWICBitmap* wic_bitmap = NULL;
  HRESULT hr;
  hr = IWICImagingFactory_CreateBitmap(wic_cd_factory, uWidth, uHeight, &wic_cd_pixel_format, WICBitmapCacheOnDemand, &wic_bitmap);
  if (FAILED(hr)) {
    return NULL;
  }
  return wic_bitmap;
}

void d2dDestroyImage(IWICBitmap *wic_bitmap)
{
  IWICBitmap_Release(wic_bitmap);
}

void d2dBitBltImage(dummy_ID2D1RenderTarget *target, IWICBitmap *wic_bitmap, const dummy_D2D1_RECT_F* pDestRect, const dummy_D2D1_RECT_F* pSourceRect, dummy_D2D1_BITMAP_INTERPOLATION_MODE mode)
{
  dummy_ID2D1Bitmap* bitmap;
  HRESULT hr;
  dummy_D2D1_RECT_F pDest;

  /* Compensation for the translation in the base transformation matrix.
  * This is to fit the image precisely into the pixel grid the canvas
  * when there is no custom transformation applied.
  */
  pDest.left = pDestRect->left - D2D_BASEDELTA_X;
  pDest.top = pDestRect->top - D2D_BASEDELTA_X;
  pDest.right = pDestRect->right - D2D_BASEDELTA_X;
  pDest.bottom = pDestRect->bottom - D2D_BASEDELTA_X;

  hr = dummy_ID2D1RenderTarget_CreateBitmapFromWicBitmap(target, (IWICBitmapSource*)wic_bitmap, NULL, &bitmap);
  if (FAILED(hr))
    return;
  dummy_ID2D1RenderTarget_DrawBitmap(target, bitmap, &pDest, 1.0f, mode, pSourceRect);
  dummy_ID2D1Bitmap_Release(bitmap);
}

void d2dBitBltBitmap(dummy_ID2D1RenderTarget *target, dummy_ID2D1Bitmap *bitmap, const dummy_D2D1_RECT_F* pDestRect, const dummy_D2D1_RECT_F* pSourceRect, dummy_D2D1_BITMAP_INTERPOLATION_MODE mode)
{
  dummy_D2D1_RECT_F pDest;

  /* Compensation for the translation in the base transformation matrix.
  * This is to fit the image precisely into the pixel grid the canvas
  * when there is no custom transformation applied.
  */
  pDest.left = pDestRect->left - D2D_BASEDELTA_X;
  pDest.top = pDestRect->top - D2D_BASEDELTA_X;
  pDest.right = pDestRect->right - D2D_BASEDELTA_X;
  pDest.bottom = pDestRect->bottom - D2D_BASEDELTA_X;

  dummy_ID2D1RenderTarget_DrawBitmap(target, bitmap, &pDest, 1.0f, mode, pSourceRect);
}
