/** \file
 * \brief Windows GDI+ Printer Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwin_d2d.h"
#include "cdprint.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

TCHAR* cdwStrToSystem(const char* str, int utf8mode);

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdwd2dKillCanvas(ctxcanvas);

  EndPage(ctxcanvas->hDC);
  EndDoc(ctxcanvas->hDC);

  DeleteDC(ctxcanvas->hDC);  /* Use DeleteDC for printer */

  free(ctxcanvas);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  EndPage(ctxcanvas->hDC);

  StartPage(ctxcanvas->hDC);

  cdwd2dUpdateCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  char *data_str = (char*)data;
  char docname[256] = "CD - Canvas Draw Document";
  int dialog = 0;
  RECT rect;
  HDC hDC;
  PRINTDLG pd;
  DOCINFOW docInfo;

  if (data_str == NULL)
    return;

  if (data_str[0] != 0)
  {
    const char *ptr = strstr(data_str, "-d");

    if (ptr != NULL)
      dialog = 1;

    if (data_str[0] != '-')
    {
      strcpy(docname, data_str);

      if (dialog)
        docname[ptr - data_str - 1] = 0;
    }
  }

  ZeroMemory(&pd, sizeof(PRINTDLG));
  pd.lStructSize = sizeof(PRINTDLG);
  pd.nCopies = 1;

  if (dialog)
  {
    pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_COLLATE | PD_NOPAGENUMS | PD_NOSELECTION;
    pd.hwndOwner = GetForegroundWindow();
  }
  else
  {
    pd.Flags = PD_RETURNDC | PD_RETURNDEFAULT;
  }

  if (!PrintDlg(&pd))
  {
    if (pd.hDevMode)
      GlobalFree(pd.hDevMode);
    if (pd.hDevNames)
      GlobalFree(pd.hDevNames);
    return;
  }

  hDC = pd.hDC;

  ZeroMemory(&docInfo, sizeof(docInfo));
  docInfo.cbSize = sizeof(docInfo);
  docInfo.lpszDocName = cdwStrToSystem(docname, 0);

  StartDocW(hDC, &docInfo);

  canvas->w = GetDeviceCaps(hDC, HORZRES);
  canvas->h = GetDeviceCaps(hDC, VERTRES);
  canvas->bpp = GetDeviceCaps(hDC, BITSPIXEL);

  /* Inicializa driver WIN32 */
  ctxcanvas = cdwd2dCreateCanvas(canvas, NULL, hDC);

  rect.left = 0;
  rect.top = 0;
  rect.right = ctxcanvas->canvas->w;
  rect.bottom = ctxcanvas->canvas->h;

  /* TODO: this is NOT working */
  ctxcanvas->d2d_canvas = d2dCreateCanvasWithHDC(ctxcanvas->hDC, &rect, 0);
  if (!ctxcanvas->d2d_canvas)
  {
    cdwd2dKillCanvas(ctxcanvas);
    canvas->ctxcanvas = NULL;

    if (pd.hDevMode)
      GlobalFree(pd.hDevMode);
    if (pd.hDevNames)
      GlobalFree(pd.hDevNames);

    return;
  }

  dummy_ID2D1RenderTarget_BeginDraw(ctxcanvas->d2d_canvas->target);

  ctxcanvas->hDC = hDC;

  StartPage(hDC);

  if (pd.hDevMode)
    GlobalFree(pd.hDevMode);
  if (pd.hDevNames)
    GlobalFree(pd.hDevNames);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwd2dInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxFlush = cdflush;
}

static cdContext cdPrinterContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_YAXIS |
  CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV),
  CD_CTX_DEVICE | CD_CTX_PLUS,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextDirect2DPrinter(void)
{
  return &cdPrinterContext;
}

/* TODO: Use PrintControl interface available in Windows 8 */
