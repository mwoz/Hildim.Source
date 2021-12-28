#include "cd_d2d.h"
#include <math.h>
#include <malloc.h>

#define ABS(a)           ((a) > 0 ? (a) : -(a))

static int (WINAPI* fn_GetUserDefaultLocaleName)(WCHAR*, int) = NULL;

static void default_user_locale(WCHAR buffer[LOCALE_NAME_MAX_LENGTH])
{
  if (fn_GetUserDefaultLocaleName == NULL) 
  {
    /* We need locale name for creation of dummy_IDWriteTextFormat. This
    * functions is available since Vista (which covers all systems with
    * Direct2D and DirectWrite). */
    HMODULE dll_kernel32 = GetModuleHandleA("KERNEL32.DLL");
    if (dll_kernel32 != NULL)
      fn_GetUserDefaultLocaleName = (int (WINAPI*)(WCHAR*, int))GetProcAddress(dll_kernel32, "GetUserDefaultLocaleName");
  }

  if (fn_GetUserDefaultLocaleName != NULL) {
    if (fn_GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH) > 0)
      return;
  }

  buffer[0] = L'\0';
}

static dummy_IDWriteTextFormat* create_text_format(const WCHAR* locale_name, const LOGFONTW* logfont, dummy_DWRITE_FONT_METRICS* metrics)
{
  dummy_IDWriteTextFormat* tf = NULL;
  dummy_IDWriteGdiInterop* gdi_interop;
  dummy_IDWriteFont* font;
  dummy_IDWriteFontFamily* family;
  dummy_IDWriteLocalizedStrings* family_names;
  UINT32 family_name_buffer_size;
  WCHAR* family_name_buffer;
  float font_size;
  HRESULT hr;

  hr = dummy_IDWriteFactory_GetGdiInterop(dwrite_cd_factory, &gdi_interop);
  if (FAILED(hr)) {
    goto err_IDWriteFactory_GetGdiInterop;
  }

  hr = dummy_IDWriteGdiInterop_CreateFontFromLOGFONT(gdi_interop, logfont, &font);
  if (FAILED(hr)) {
    goto err_IDWriteGdiInterop_CreateFontFromLOGFONT;
  }

  dummy_IDWriteFont_GetMetrics(font, metrics);

  hr = dummy_IDWriteFont_GetFontFamily(font, &family);
  if (FAILED(hr)) {
    goto err_IDWriteFont_GetFontFamily;
  }

  hr = dummy_IDWriteFontFamily_GetFamilyNames(family, &family_names);
  if (FAILED(hr)) {
    goto err_IDWriteFontFamily_GetFamilyNames;
  }

  hr = dummy_IDWriteLocalizedStrings_GetStringLength(family_names, 0, &family_name_buffer_size);
  if (FAILED(hr)) {
    goto err_IDWriteLocalizedStrings_GetStringLength;
  }

  family_name_buffer = (WCHAR*)_malloca(sizeof(WCHAR)* (family_name_buffer_size + 1));
  if (family_name_buffer == NULL) {
    goto err_malloca;
  }

  hr = dummy_IDWriteLocalizedStrings_GetString(family_names, 0,
                                               family_name_buffer, family_name_buffer_size + 1);
  if (FAILED(hr)) {
    goto err_IDWriteLocalizedStrings_GetString;
  }

  if (logfont->lfHeight < 0) {
    font_size = (float)-logfont->lfHeight;
  }
  else if (logfont->lfHeight > 0) {
    font_size = ((float)metrics->ascent + (float)metrics->descent)
      / (float)metrics->designUnitsPerEm;
  }
  else {
    font_size = 12.0f;
  }

  hr = dummy_IDWriteFactory_CreateTextFormat(dwrite_cd_factory, family_name_buffer,
                                             NULL, dummy_IDWriteFont_GetWeight(font), dummy_IDWriteFont_GetStyle(font),
                                             dummy_IDWriteFont_GetStretch(font), font_size, locale_name, &tf);
  if (FAILED(hr)) {
    goto err_IDWriteFactory_CreateTextFormat;
  }

err_IDWriteFactory_CreateTextFormat:
err_IDWriteLocalizedStrings_GetString :
  _freea(family_name_buffer);
err_malloca:
err_IDWriteLocalizedStrings_GetStringLength :
  dummy_IDWriteLocalizedStrings_Release(family_names);
err_IDWriteFontFamily_GetFamilyNames:
  dummy_IDWriteFontFamily_Release(family);
err_IDWriteFont_GetFontFamily:
  dummy_IDWriteFont_Release(font);
err_IDWriteGdiInterop_CreateFontFromLOGFONT:
  dummy_IDWriteGdiInterop_Release(gdi_interop);
err_IDWriteFactory_GetGdiInterop:
  return tf;
}

static dummy_IDWriteTextLayout* create_text_layout(dummy_IDWriteTextFormat* tf, float w, float h, const WCHAR* str, int len)
{
  dummy_IDWriteTextLayout* layout;
  HRESULT hr;
  int tla;

  if (len < 0)
    len = (int)wcslen(str);

  hr = dummy_IDWriteFactory_CreateTextLayout(dwrite_cd_factory, str, len, tf, w, h, &layout);
  if (FAILED(hr)) {
    return NULL;
  }

  tla = dummy_DWRITE_TEXT_ALIGNMENT_LEADING;
  dummy_IDWriteTextLayout_SetTextAlignment(layout, tla);

  tla = dummy_DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
  dummy_IDWriteTextLayout_SetParagraphAlignment(layout, tla);

  dummy_IDWriteTextLayout_SetWordWrapping(layout, dummy_DWRITE_WORD_WRAPPING_NO_WRAP);

  return layout;
}

void d2dDrawText(dummy_ID2D1RenderTarget *target, dummy_ID2D1Brush *brush, float x, float y, float w, float h, const WCHAR* pszText, int iTextLength, d2dFont *font)
{
  dummy_D2D1_POINT_2F origin;
  dummy_IDWriteTextLayout* layout;

  origin.x = x;
  origin.y = y;

  layout = create_text_layout(font->tf, w, h, pszText, iTextLength);
  if (layout == NULL) {
    return;
  }

  dummy_ID2D1RenderTarget_DrawTextLayout(target, origin, layout, brush, 0);

  dummy_IDWriteTextLayout_Release(layout);
}

d2dFont* d2dCreateFont(const LOGFONTW* pLogFont)
{
  WCHAR user_locale[LOCALE_NAME_MAX_LENGTH];
  WCHAR* locales[3];
  d2dFont* font;
  int i;

  locales[0] = user_locale;
  locales[1] = L"";
  locales[2] = L"en-us";

  font = (d2dFont*)malloc(sizeof(d2dFont));
  if (font == NULL) {
    return NULL;
  }

  default_user_locale(user_locale);

  /* Direct 2D seems to not understand "MS Shell Dlg" and "MS Shell Dlg 2"
  * so we skip the attempts to use it. */
  if (wcscmp(pLogFont->lfFaceName, L"MS Shell Dlg") != 0 &&
      wcscmp(pLogFont->lfFaceName, L"MS Shell Dlg 2") != 0) 
  {
    for (i = 0; i < 3; i++) 
    {
      font->tf = create_text_format(locales[i], pLogFont, &font->metrics);
      if (font->tf != NULL)
        return font;
    }
  }

  free(font);
  return NULL;
}

void d2dDestroyFont(d2dFont* font)
{
  dummy_IDWriteTextFormat_Release(font->tf);
  free(font);
}

void d2dFontGetMetrics(d2dFont *font, d2dFontMetrics *pMetrics)
{
  float factor;

  if (font == NULL) 
  {
    /* Treat NULL as "no font". This simplifies paint code when font
    * creation fails. */
    pMetrics->fEmHeight = 0.0f;
    pMetrics->fAscent = 0.0f;
    pMetrics->fDescent = 0.0f;
    pMetrics->fLeading = 0.0f;

    return;
  }

  pMetrics->fEmHeight = dummy_IDWriteTextFormat_GetFontSize(font->tf);

  factor = (pMetrics->fEmHeight / (float)font->metrics.designUnitsPerEm);

  pMetrics->fAscent = factor * (float)font->metrics.ascent;
  pMetrics->fDescent = factor * (float)ABS(font->metrics.descent);
  pMetrics->fLeading = factor * (float)(font->metrics.ascent + ABS(font->metrics.descent) + font->metrics.lineGap);
}

void d2dFontMeasureString(d2dFont *hFont, const WCHAR* pszText, int iTextLength, int *w, int *h)
{
  d2dFont* font = (d2dFont*)hFont;
  dummy_IDWriteTextLayout* layout;
  dummy_DWRITE_TEXT_METRICS tm;

  layout = create_text_layout(font->tf, 10000, 10000, pszText, iTextLength);
  if (layout == NULL) {
    return;
  }

  dummy_IDWriteTextLayout_GetMetrics(layout, &tm);

  if (w) *w = (int)tm.width;
  if (h) *h = (int)tm.height;

  dummy_IDWriteTextLayout_Release(layout);
}
