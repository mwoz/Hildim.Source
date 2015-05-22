/** \file
 * \brief iupexpander control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_childtree.h"
#include "iup_draw.h"
#include "iup_image.h"


#define IEXPAND_BUTTON_SIZE 16
#define IEXPAND_HANDLE_SIZE 20
#define IEXPAND_SPACING   3
#define IEXPAND_BACK_MARGIN  2

enum { IEXPANDER_LEFT, IEXPANDER_RIGHT, IEXPANDER_TOP, IEXPANDER_BOTTOM };
enum { IEXPANDER_CLOSE, IEXPANDER_OPEN, IEXPANDER_OPEN_FLOAT };

struct _IcontrolData
{
  /* attributes */
  int position;
  int state;
  int barSize;

  int highlight,
      extra_buttons,
      extra_buttons_state[4],
      auto_show;
  Ihandle* timer;
};


static void iExpanderOpenCloseChild(Ihandle* ih, int refresh, int callcb, int state)
{
  Ihandle *child = ih->firstchild->brother;

  if (callcb)
  {
    IFni cb = (IFni)IupGetCallback(ih, "OPENCLOSE_CB");
    if (cb)
    {
      int ret = cb(ih, state);
      if (ret == IUP_IGNORE)
        return;
    }
  }

  ih->data->state = state;

  IupUpdate(ih->firstchild);

  if (child)
  {
    if (ih->data->state == IEXPANDER_CLOSE)
      IupSetAttribute(child, "VISIBLE", "NO");
    else
      IupSetAttribute(child, "VISIBLE", "YES");

    if (refresh)
      IupRefresh(child); /* this will recompute the layout of the hole dialog */
  }

  if (callcb)
  {
    IFn cb = IupGetCallback(ih, "ACTION");
    if (cb)
      cb(ih);
  }
}

static int iExpanderGetBarSize(Ihandle* ih)
{
  int bar_size;
  if (ih->data->barSize == -1)
  {
    iupdrvFontGetCharSize(ih, NULL, &bar_size); 

    if (bar_size < IEXPAND_HANDLE_SIZE)
      bar_size = IEXPAND_HANDLE_SIZE;

    if (ih->data->position == IEXPANDER_TOP)
    {
      char* title = iupAttribGetStr(ih, "TITLE");
      char* image = iupAttribGetStr(ih, "IMAGE");
      if (image)
      {
        int image_h = 0;
        iupImageGetInfo(image, NULL, &image_h, NULL);
        bar_size = iupMAX(bar_size, image_h);
      }

      if (title || image || ih->data->extra_buttons != 0)
        bar_size += 2 * IEXPAND_BACK_MARGIN;
    }
  }
  else
    bar_size = ih->data->barSize;

  return bar_size;
}

/*****************************************************************************\
|* Callbacks of canvas bar                                                   *|
\*****************************************************************************/

static void iExpanderDrawTriangle(IdrawCanvas *dc, int x, int y, unsigned char r, unsigned char g, unsigned char b, int dir)
{
  int points[6];

  /* fix for smooth triangle */
  int delta = (IEXPAND_HANDLE_SIZE - 2*IEXPAND_SPACING)/2;

  switch(dir)
  {
  case IEXPANDER_LEFT:  /* arrow points left */
    x += IEXPAND_SPACING;  /* fix center */
    points[0] = x + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING - delta;
    points[1] = y + IEXPAND_SPACING;
    points[2] = x + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING - delta;
    points[3] = y + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING;
    points[4] = x + IEXPAND_SPACING;
    points[5] = y + IEXPAND_HANDLE_SIZE/2;
    break;
  case IEXPANDER_TOP:    /* arrow points top */
    y += IEXPAND_SPACING;  /* fix center */
    points[0] = x + IEXPAND_SPACING;
    points[1] = y + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING - (delta-1);
    points[2] = x + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING;
    points[3] = y + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING - (delta-1);
    points[4] = x + IEXPAND_HANDLE_SIZE/2;
    points[5] = y + IEXPAND_SPACING;
    break;
  case IEXPANDER_RIGHT:  /* arrow points right */
    x += IEXPAND_SPACING;  /* fix center */
    points[0] = x + IEXPAND_SPACING;
    points[1] = y + IEXPAND_SPACING;
    points[2] = x + IEXPAND_SPACING;
    points[3] = y + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING;
    points[4] = x + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING - delta;
    points[5] = y + IEXPAND_HANDLE_SIZE/2;
    break;
  case IEXPANDER_BOTTOM:  /* arrow points bottom */
    y += IEXPAND_SPACING;  /* fix center */
    points[0] = x + IEXPAND_SPACING;
    points[1] = y + IEXPAND_SPACING;
    points[2] = x + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING;
    points[3] = y + IEXPAND_SPACING;
    points[4] = x + IEXPAND_HANDLE_SIZE/2;
    points[5] = y + IEXPAND_HANDLE_SIZE - IEXPAND_SPACING - (delta-1);

    /* fix for simmetry */
    iupDrawLine(dc, x+IEXPAND_SPACING, y+IEXPAND_SPACING, x+IEXPAND_HANDLE_SIZE-IEXPAND_SPACING, y+IEXPAND_SPACING, r, g, b, IUP_DRAW_STROKE);
    break;
  }

  iupDrawPolygon(dc, points, 3, r, g, b, IUP_DRAW_FILL);
}

static void iExpanderDrawSmallTriangle(IdrawCanvas *dc, int x, int y, unsigned char r, unsigned char g, unsigned char b, int dir)
{
  int points[6];
  int size = IEXPAND_HANDLE_SIZE-2;
  int space = IEXPAND_SPACING+1;

  /* fix for smooth triangle */
  int delta = (size - 2*space)/2;

  switch(dir)
  {
  case IEXPANDER_RIGHT:  /* arrow points right */
    x += space-1;  /* fix center */
    y += 1;
    points[0] = x + space;
    points[1] = y + space;
    points[2] = x + space;
    points[3] = y + size - space;
    points[4] = x + size - space - delta;
    points[5] = y + size/2;
    break;
  case IEXPANDER_BOTTOM:  /* arrow points bottom */
    y += space;  /* fix center */
    points[0] = x + space;
    points[1] = y + space;
    points[2] = x + size - space;
    points[3] = y + space;
    points[4] = x + size/2;
    points[5] = y + size - space - (delta-1);

    /* fix for simmetry */
    iupDrawLine(dc, x+space, y+space, x+size-space, y+space, r, g, b, IUP_DRAW_STROKE);
    break;
  }

  iupDrawPolygon(dc, points, 3, r, g, b, IUP_DRAW_FILL);
}

static void iExpanderDrawArrow(IdrawCanvas *dc, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char bg_r, unsigned char bg_g, unsigned char bg_b, int dir)
{
  unsigned char sr, sg, sb;

  sr = (r+bg_r)/2;
  sg = (g+bg_g)/2;
  sb = (b+bg_b)/2;

  /* to smooth the arrow border */
  switch(dir)
  {
  case IEXPANDER_LEFT:  /* arrow points left */
    iExpanderDrawTriangle(dc, x-1, y, sr, sg, sb, dir);
    break;
  case IEXPANDER_TOP:    /* arrow points top */
    iExpanderDrawTriangle(dc, x, y-1, sr, sg, sb, dir);
    break;
  case IEXPANDER_RIGHT:  /* arrow points right */
    iExpanderDrawTriangle(dc, x+1, y, sr, sg, sb, dir);
    break;
  case IEXPANDER_BOTTOM:  /* arrow points bottom */
    iExpanderDrawTriangle(dc, x, y+1, sr, sg, sb, dir);
    break;
  }

  iExpanderDrawTriangle(dc, x, y, r, g, b, dir);
}

static void iExpanderDrawSmallArrow(IdrawCanvas *dc, unsigned char r, unsigned char g, unsigned char b, unsigned char bg_r, unsigned char bg_g, unsigned char bg_b, int dir, int y_offset)
{
  unsigned char sr, sg, sb;

  sr = (r+bg_r)/2;
  sg = (g+bg_g)/2;
  sb = (b+bg_b)/2;

  /* to smooth the arrow border */
  switch(dir)
  {
  case IEXPANDER_RIGHT:  /* arrow points right */
    iExpanderDrawSmallTriangle(dc, 2 + IEXPAND_BACK_MARGIN, 0 + IEXPAND_BACK_MARGIN + y_offset, sr, sg, sb, dir);
    iExpanderDrawSmallTriangle(dc, 1 + IEXPAND_BACK_MARGIN, 0 + IEXPAND_BACK_MARGIN + y_offset, r, g, b, dir);
    break;
  case IEXPANDER_BOTTOM:  /* arrow points bottom */
    iExpanderDrawSmallTriangle(dc, 0 + IEXPAND_BACK_MARGIN, 1 + IEXPAND_BACK_MARGIN + y_offset, sr, sg, sb, dir);
    iExpanderDrawSmallTriangle(dc, 0 + IEXPAND_BACK_MARGIN, 0 + IEXPAND_BACK_MARGIN + y_offset, r, g, b, dir);
    break;
  }
}

static void iExpanderAddHighlight(unsigned char *r, unsigned char *g, unsigned char *b)
{
  int i = (*r+*g+*b)/3;
  if (i < 128)
  {
    *r = (*r+255)/2;
    *g = (*g+255)/2;
    *b = (*b+255)/2;
  }
  else
  {
    *r = (*r+0)/2;
    *g = (*g+0)/2;
    *b = (*b+0)/2;
  }
}

static void iExpanderDrawExtraButton(Ihandle* ih, IdrawCanvas *dc, int button, int x, int y, int height)
{
  char* image = iupAttribGetId(ih, "IMAGEEXTRA", button);
  int active = IupGetInt(ih, "ACTIVE");
  int img_width = 0, img_height = 0;

  if (!image)
    return;

  if (ih->data->extra_buttons_state[button] == 1)
  {
    char* impress = iupAttribGetId(ih, "IMAGEEXTRAPRESS", button);
    if (impress) image = impress;
  }
  else if (ih->data->extra_buttons_state[button] == -1)
  {
    char* imhighlight = iupAttribGetId(ih, "IMAGEEXTRAHIGHLIGHT", button);
    if (imhighlight) image = imhighlight;
  }

  iupImageGetInfo(image, NULL, &img_height, NULL);
  if (height > img_height)
    y += (height - img_height) / 2;

  iupDrawImage(dc, image, !active, x, y, &img_width, &img_height);
}

static int iExpanderAction_CB(Ihandle* bar)
{
  Ihandle *ih = bar->parent;
  IdrawCanvas *dc = iupDrawCreateCanvas(bar);
  unsigned char r=0, g=0, b=0;
  unsigned char bg_r=0, bg_g=0, bg_b=0;
  int draw_bgcolor = 1;
  char* title = iupAttribGetStr(ih, "TITLE");
  char* image = iupAttribGetStr(ih, "IMAGE");
  char* bgcolor = iupAttribGetStr(ih, "BACKCOLOR");
  if (!bgcolor)
  {
    bgcolor = iupBaseNativeParentGetBgColorAttrib(ih);
    draw_bgcolor = 0;
  }
  
  iupStrToRGB(bgcolor, &bg_r, &bg_g, &bg_b);
  iupStrToRGB(IupGetAttribute(ih, "FORECOLOR"), &r, &g, &b);

  iupDrawParentBackground(dc);

  if (draw_bgcolor)
    iupDrawRectangle(dc, IEXPAND_BACK_MARGIN, IEXPAND_BACK_MARGIN, bar->currentwidth - IEXPAND_BACK_MARGIN, bar->currentheight - IEXPAND_BACK_MARGIN, bg_r, bg_g, bg_b, IUP_DRAW_FILL);

  if (ih->data->position == IEXPANDER_TOP && (title || image || ih->data->extra_buttons!=0))
  {
    /* left align image/handler+title */
    int txt_offset = IEXPAND_HANDLE_SIZE;

    if (image)
    {
      int active = IupGetInt(ih, "ACTIVE");
      int img_width = 0, img_height = 0;
      int y_offset = 0;

      if (ih->data->state != IEXPANDER_CLOSE)
      {
        char* imopen = iupAttribGetStr(ih, "IMAGEOPEN");
        if (imopen) image = imopen;

        if (ih->data->highlight)
        {
          char* imhighlight = iupAttribGetStr(ih, "IMAGEOPENHIGHLIGHT");
          if (imhighlight) image = imhighlight;
        }
      }
      else if (ih->data->highlight)
      {
        char* imhighlight = iupAttribGetStr(ih, "IMAGEHIGHLIGHT");
        if (imhighlight) image = imhighlight;
      }

      iupImageGetInfo(image, NULL, &img_height, NULL);
      if (bar->currentheight > img_height)
        y_offset = (bar->currentheight - img_height) / 2;

      iupDrawImage(dc, image, !active, IEXPAND_BACK_MARGIN, y_offset, &img_width, &img_height);

      txt_offset = iupMAX(txt_offset, img_width);
    }
    else
    {
      int y_offset = 0;
      if (bar->currentheight > IEXPAND_HANDLE_SIZE + 2 * IEXPAND_BACK_MARGIN)
        y_offset = (bar->currentheight - IEXPAND_HANDLE_SIZE - 2 * IEXPAND_BACK_MARGIN) / 2;

      if (ih->data->highlight)
        iExpanderAddHighlight(&r, &g, &b);

      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawSmallArrow(dc, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_RIGHT, y_offset);
      else
        iExpanderDrawSmallArrow(dc, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_BOTTOM, y_offset);
    }

    if (title)
    {
      /* left align everything */
      int len, charheight;
      iupStrNextLine(title, &len);  /* get the length of the first line */
      iupdrvFontGetCharSize(ih, NULL, &charheight);
      iupDrawText(dc, title, len, txt_offset + IEXPAND_SPACING, (bar->currentheight - charheight) / 2, r, g, b, IupGetAttribute(ih, "FONT"));
    }

    if (ih->data->extra_buttons != 0)
    {
      /* right align extra buttons */
      int y = IEXPAND_SPACING + IEXPAND_BACK_MARGIN,
        height = bar->currentheight - 2 * (IEXPAND_SPACING + IEXPAND_BACK_MARGIN);

      iExpanderDrawExtraButton(ih, dc, 1, bar->currentwidth - (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN, y, height);

      if (ih->data->extra_buttons > 1)
        iExpanderDrawExtraButton(ih, dc, 2, bar->currentwidth - 2 * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN, y, height);

      if (ih->data->extra_buttons == 3)
        iExpanderDrawExtraButton(ih, dc, 3, bar->currentwidth - 3 * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN, y, height);
    }
  }
  else
  {
    /* center align the handler */
    int x, y;

    if (ih->data->highlight)
      iExpanderAddHighlight(&r, &g, &b);

    switch(ih->data->position)
    {
    case IEXPANDER_LEFT:
      x = 0;
      y = (bar->currentheight - IEXPAND_HANDLE_SIZE)/2;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_RIGHT);
      else
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_LEFT);
      break;
    case IEXPANDER_TOP:
      x = (bar->currentwidth - IEXPAND_HANDLE_SIZE)/2;
      y = 0;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_BOTTOM);
      else
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_TOP);
      break;
    case IEXPANDER_RIGHT:
      x = 0;
      y = (bar->currentheight - IEXPAND_HANDLE_SIZE)/2;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_LEFT);
      else
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_RIGHT);
      break;
    case IEXPANDER_BOTTOM:
      x = (bar->currentwidth - IEXPAND_HANDLE_SIZE)/2;
      y = 0;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_TOP);
      else
        iExpanderDrawArrow(dc, x, y, r, g, b, bg_r, bg_g, bg_b, IEXPANDER_BOTTOM);
      break;
    }
  }

  iupDrawFlush(dc);

  iupDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static int iExpanderGlobalMotion_cb(int x, int y)
{
  int child_x, child_y;
  Ihandle* ih = (Ihandle*)IupGetGlobal("_IUP_EXPANDER_GLOBAL");
  Ihandle *child = ih->firstchild->brother;

  if (ih->data->state != IEXPANDER_OPEN_FLOAT)
  {
    IupSetGlobal("_IUP_EXPANDER_GLOBAL", NULL);
    IupSetFunction("GLOBALMOTION_CB", IupGetFunction("_IUP_OLD_GLOBALMOTION_CB"));
    IupSetFunction("_IUP_OLD_GLOBALMOTION_CB", NULL);
    IupSetGlobal("INPUTCALLBACKS", "No");
    return IUP_DEFAULT;
  }

  child_x = 0, child_y = 0;
  iupdrvClientToScreen(ih->firstchild, &child_x, &child_y);
  if (x > child_x && x < child_x+ih->firstchild->currentwidth &&
      y > child_y && y < child_y+ih->firstchild->currentheight)
    return IUP_DEFAULT;  /* ignore if inside the bar */

  child_x = 0, child_y = 0;
  iupdrvClientToScreen(child, &child_x, &child_y);
  if (x < child_x || x > child_x+child->currentwidth ||
      y < child_y || y > child_y+child->currentheight)
  {
    iExpanderOpenCloseChild(ih, 0, 1, IEXPANDER_CLOSE);
    IupSetGlobal("_IUP_EXPANDER_GLOBAL", NULL);
    IupSetFunction("GLOBALMOTION_CB", IupGetFunction("_IUP_OLD_GLOBALMOTION_CB"));
    IupSetFunction("_IUP_OLD_GLOBALMOTION_CB", NULL);
    IupSetGlobal("INPUTCALLBACKS", "No");
  }

  return IUP_DEFAULT;
}

static int iExpanderTimer_cb(Ihandle* timer)
{
  Ihandle* ih = (Ihandle*)iupAttribGet(timer, "_IUP_EXPANDER");
  Ihandle *child = ih->firstchild->brother;

  /* run timer just once each time */
  IupSetAttribute(timer, "RUN", "No");

  /* just show child on top,
     that's why child must be a native container when using autoshow. */
  iExpanderOpenCloseChild(ih, 0, 1, IEXPANDER_OPEN_FLOAT);
  IupRefreshChildren(ih);
  IupSetAttribute(child, "ZORDER", "TOP"); 

  /* now monitor mouse move */
  IupSetGlobal("INPUTCALLBACKS", "Yes");
  IupSetFunction("_IUP_OLD_GLOBALMOTION_CB", IupGetFunction("GLOBALMOTION_CB"));
  IupSetGlobal("_IUP_EXPANDER_GLOBAL", (char*)ih);
  IupSetFunction("GLOBALMOTION_CB", (Icallback)iExpanderGlobalMotion_cb);
  return IUP_DEFAULT;
}

static int iExpanderLeaveWindow_cb(Ihandle* bar)
{
  Ihandle* ih = bar->parent;
  if (ih->data->highlight)
  {
    ih->data->highlight = 0;

    if (ih->data->extra_buttons_state[1] != 0) ih->data->extra_buttons_state[1] = 0;
    if (ih->data->extra_buttons_state[2] != 0) ih->data->extra_buttons_state[2] = 0;
    if (ih->data->extra_buttons_state[3] != 0) ih->data->extra_buttons_state[3] = 0;

    /* redraw bar */
    IupUpdate(ih->firstchild);

    if (ih->data->auto_show)
    {
      if (IupGetInt(ih->data->timer, "RUN"))
        IupSetAttribute(ih->data->timer, "RUN", "No");
    }
  }
  return IUP_DEFAULT;
}

static int iExpanderEnterWindow_cb(Ihandle* bar)
{
  Ihandle* ih = bar->parent;
  if (!ih->data->highlight)
  {
    ih->data->highlight = 1;

    /* redraw bar */
    IupUpdate(ih->firstchild);

    if (ih->data->auto_show &&
        ih->firstchild->brother &&
        ih->data->state == IEXPANDER_CLOSE)
      IupSetAttribute(ih->data->timer, "RUN", "Yes");
  }
  return IUP_DEFAULT;
}

static int iExpanderMotion_CB(Ihandle* bar, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;

  /* called only when EXTRABUTTONS is used */

  if (ih->data->position != IEXPANDER_TOP)
    return IUP_DEFAULT;

  if (y >= IEXPAND_SPACING + IEXPAND_BACK_MARGIN && y <= bar->currentheight - IEXPAND_SPACING - IEXPAND_BACK_MARGIN)
  {
    int old_state[4];
    old_state[1] = ih->data->extra_buttons_state[1];
    old_state[2] = ih->data->extra_buttons_state[2];
    old_state[3] = ih->data->extra_buttons_state[3];

    if ((x >= bar->currentwidth - (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN) &&
      (x < bar->currentwidth - IEXPAND_SPACING - IEXPAND_BACK_MARGIN))
    {
      if (ih->data->extra_buttons_state[1] == 0)
        ih->data->extra_buttons_state[1] = -1;  /* highlight if not pressed */
    }
    else
    {
      if (ih->data->extra_buttons_state[1] != 0)
        ih->data->extra_buttons_state[1] = 0;
    }

    if (ih->data->extra_buttons > 1)
    {
      if ((x >= bar->currentwidth - 2 * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN) &&
        (x < bar->currentwidth - (IEXPAND_BUTTON_SIZE + 2 * IEXPAND_SPACING) - IEXPAND_BACK_MARGIN))
      {
        if (ih->data->extra_buttons_state[2] == 0)
          ih->data->extra_buttons_state[2] = -1;  /* highlight if not pressed */
      }
      else
      {
        if (ih->data->extra_buttons_state[2] != 0)
          ih->data->extra_buttons_state[2] = 0;
      }
    }

    if (ih->data->extra_buttons == 3)
    {
      if ((x >= bar->currentwidth - 3 * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN) &&
          (x < bar->currentwidth - (2 * IEXPAND_BUTTON_SIZE + 3 * IEXPAND_SPACING) - IEXPAND_BACK_MARGIN))
      {
        if (ih->data->extra_buttons_state[3] == 0)
          ih->data->extra_buttons_state[3] = -1;  /* highlight if not pressed */
      }
      else
      {
        if (ih->data->extra_buttons_state[3] != 0)
          ih->data->extra_buttons_state[3] = 0;
      }
    }

    if (old_state[1] != ih->data->extra_buttons_state[1] ||
        old_state[2] != ih->data->extra_buttons_state[2] ||
        old_state[3] != ih->data->extra_buttons_state[3])
      IupUpdate(bar);
  }

  (void)status;
  return IUP_DEFAULT;
}

static int iExpanderCallExtraButtonCb(Ihandle* ih, int button, int pressed)
{
  int old_state = ih->data->extra_buttons_state[button];
  ih->data->extra_buttons_state[button] = pressed;

  /* redraw only if state changed */
  if (old_state != ih->data->extra_buttons_state[button])
    IupUpdate(ih->firstchild);

  if (!pressed)
    pressed = pressed;

  /* if pressed always call,
     if not pressed, call only if was pressed */
  if (pressed || old_state==1)
  {
    IFnii cb = (IFnii)IupGetCallback(ih, "EXTRABUTTON_CB");
    if (cb)
      cb(ih, button, pressed);
  }

  return IUP_DEFAULT;
}

static int iExpanderButton_CB(Ihandle* bar, int button, int pressed, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;

  if (button != IUP_BUTTON1)
    return IUP_DEFAULT;

  if (ih->data->auto_show && ih->firstchild)
  {
    if (IupGetInt(ih->data->timer, "RUN"))
      IupSetAttribute(ih->data->timer, "RUN", "No");
  }

  if (ih->data->position == IEXPANDER_TOP && ih->data->extra_buttons != 0)
  {
    if (y >= IEXPAND_SPACING + IEXPAND_BACK_MARGIN && y <= bar->currentheight - IEXPAND_SPACING - IEXPAND_BACK_MARGIN)
    {
      if ((x >= bar->currentwidth - (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN) &&
          (x < bar->currentwidth - IEXPAND_SPACING - IEXPAND_BACK_MARGIN))
        return iExpanderCallExtraButtonCb(ih, 1, pressed);

      if (ih->data->extra_buttons > 1)
      {
        if ((x >= bar->currentwidth - 2 * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN) &&
            (x < bar->currentwidth - (IEXPAND_BUTTON_SIZE + 2 * IEXPAND_SPACING) - IEXPAND_BACK_MARGIN))
          return iExpanderCallExtraButtonCb(ih, 2, pressed);
      }

      if (ih->data->extra_buttons == 3)
      {
        if ((x >= bar->currentwidth - 3 * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING) - IEXPAND_BACK_MARGIN) &&
            (x < bar->currentwidth - (2 * IEXPAND_BUTTON_SIZE + 3 * IEXPAND_SPACING) - IEXPAND_BACK_MARGIN))
          return iExpanderCallExtraButtonCb(ih, 3, pressed);
      }
    }
  }

  if (pressed)
  {
    /* Update the state: OPEN ==> collapsed, CLOSE ==> expanded */
     iExpanderOpenCloseChild(ih, 1, 1, ih->data->state==IEXPANDER_OPEN? IEXPANDER_CLOSE: IEXPANDER_OPEN);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}


/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/


static char* iExpanderGetClientSizeAttrib(Ihandle* ih)
{
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    width -= bar_size;
  else
    height -= bar_size;

  if (width < 0) width = 0;
  if (height < 0) height = 0;
  return iupStrReturnIntInt(width, height, 'x');
}

static int iExpanderSetPositionAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "LEFT"))
    ih->data->position = IEXPANDER_LEFT;
  else if (iupStrEqualNoCase(value, "RIGHT"))
    ih->data->position = IEXPANDER_RIGHT;
  else if (iupStrEqualNoCase(value, "BOTTOM"))
    ih->data->position = IEXPANDER_BOTTOM;
  else  /* Default = TOP */
    ih->data->position = IEXPANDER_TOP;

  return 0;  /* do not store value in hash table */
}

static int iExpanderSetBarSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    ih->data->barSize = -1;
  else
    iupStrToInt(value, &ih->data->barSize);  /* must manually update layout */
  return 0; /* do not store value in hash table */
}

static char* iExpanderGetBarSizeAttrib(Ihandle* ih)
{
  int bar_size = iExpanderGetBarSize(ih);
  return iupStrReturnInt(bar_size);
}

static int iExpanderPostRedrawSetAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  IupUpdate(ih->firstchild);
  return 1;  /* store value in hash table */
}

static int iExpanderSetStateAttrib(Ihandle* ih, const char* value)
{
  int state;
  if (iupStrEqualNoCase(value, "OPEN"))
    state = IEXPANDER_OPEN;
  else
    state = IEXPANDER_CLOSE;

  if (ih->data->state == state)
    return 0;

  iExpanderOpenCloseChild(ih, 1, 0, state);

  return 0; /* do not store value in hash table */
}

static char* iExpanderGetStateAttrib(Ihandle* ih)
{
  if (ih->data->state)
    return "OPEN";
  else
    return "CLOSE";
}

static int iExpanderSetAutoShowAttrib(Ihandle* ih, const char* value)
{
  ih->data->auto_show = iupStrBoolean(value);
  if (ih->data->auto_show)
  {
    if (!ih->data->timer)
    {
      ih->data->timer = IupTimer();
      IupSetAttribute(ih->data->timer, "TIME", "1000");  /* 1 second */
      IupSetCallback(ih->data->timer, "ACTION_CB", iExpanderTimer_cb);
      iupAttribSet(ih->data->timer, "_IUP_EXPANDER", (char*)ih);  /* 1 second */
    }
  }
  else
  {
    if (ih->data->timer)
      IupSetAttribute(ih->data->timer, "RUN", "NO");
  }
  return 0; /* do not store value in hash table */
}

static char* iExpanderGetAutoShowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->auto_show);
}

static int iExpanderSetExtraButtonsAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    ih->data->extra_buttons = 0;
  else
  {
    iupStrToInt(value, &(ih->data->extra_buttons));
    if (ih->data->extra_buttons < 0)
      ih->data->extra_buttons = 0;
    else if (ih->data->extra_buttons > 3)
      ih->data->extra_buttons = 3;

    if (ih->data->extra_buttons != 0)
      IupSetCallback(ih->firstchild, "MOTION_CB", (Icallback)iExpanderMotion_CB);
  }
  return 0; /* do not store value in hash table */
}

static char* iExpanderGetExtraButtonsAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->extra_buttons);
}


/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/


static void iExpanderComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int child_expand = 0,
      natural_w, natural_h;
  Ihandle *child = ih->firstchild->brother;
  int bar_size = iExpanderGetBarSize(ih);

  /* bar */
  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    natural_w = bar_size;
    natural_h = IEXPAND_HANDLE_SIZE;
  }
  else
  {
    natural_w = IEXPAND_HANDLE_SIZE;
    natural_h = bar_size;

    if (ih->data->position == IEXPANDER_TOP)
    {
      /* if IMAGE is defined assume that will cover all the canvas area */
      char* value = iupAttribGetStr(ih, "IMAGE");
      if (value)
      {
        int image_w = 0;
        iupImageGetInfo(value, &image_w, NULL, NULL);
        natural_w = iupMAX(natural_w, image_w);
      }

      /* if TITLE and IMAGE are both defined then 
         IMAGE is only the handle */

      value = iupAttribGetStr(ih, "TITLE");
      if (value)
      {
        int title_size = 0;
        iupdrvFontGetMultiLineStringSize(ih, value, &title_size, NULL);
        natural_w += title_size + IEXPAND_SPACING;
      }

      if (ih->data->extra_buttons != 0)
        natural_w += ih->data->extra_buttons * (IEXPAND_BUTTON_SIZE + IEXPAND_SPACING);

      natural_w += 2 * IEXPAND_BACK_MARGIN;
    }
  }

  if (child)
  {
    /* update child natural bar_size first */
    iupBaseComputeNaturalSize(child);

    if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    {
      if (ih->data->state == IEXPANDER_OPEN)  /* only open, not float */
        natural_w += child->naturalwidth;
      natural_h = iupMAX(natural_h, child->naturalheight);
    }
    else
    {
      natural_w = iupMAX(natural_w, child->naturalwidth);
      if (ih->data->state == IEXPANDER_OPEN)  /* only open, not float */
        natural_h += child->naturalheight;
    }

    if (ih->data->state == IEXPANDER_OPEN)
      child_expand = child->expand;
    else
    {
      if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
        child_expand = child->expand & IUP_EXPAND_HEIGHT;  /* only vertical allowed */
      else
        child_expand = child->expand & IUP_EXPAND_WIDTH;  /* only horizontal allowed */
    }
  }

  *children_expand = child_expand;
  *w = natural_w;
  *h = natural_h;
}

static void iExpanderSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  Ihandle *child = ih->firstchild->brother;
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    /* bar */
    ih->firstchild->currentwidth  = bar_size;
    ih->firstchild->currentheight = ih->currentheight;

    if (ih->currentwidth < bar_size)
      ih->currentwidth = bar_size;

    width = ih->currentwidth - bar_size;
  }
  else /* IEXPANDER_TOP OR IEXPANDER_BOTTOM */
  {
    /* bar */
    ih->firstchild->currentwidth  = ih->currentwidth;
    ih->firstchild->currentheight = bar_size;

    if (ih->currentheight < bar_size)
      ih->currentheight = bar_size;

    height = ih->currentheight - bar_size;
  }

  if (child)
  {
    if (ih->data->state == IEXPANDER_OPEN)
      iupBaseSetCurrentSize(child, width, height, shrink);
    else if (ih->data->state == IEXPANDER_OPEN_FLOAT)  /* simply set to the natural size */
      iupBaseSetCurrentSize(child, child->currentwidth, child->currentheight, shrink);
  }
}

static void iExpanderSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  Ihandle *child = ih->firstchild->brother;
  int bar_size = iExpanderGetBarSize(ih);

  /* always position bar */
  if (ih->data->position == IEXPANDER_LEFT)
  {
    iupBaseSetPosition(ih->firstchild, x, y);
    x += bar_size;
  }
  else if (ih->data->position == IEXPANDER_RIGHT)
    iupBaseSetPosition(ih->firstchild, x + ih->currentwidth - bar_size, y);
  else if (ih->data->position == IEXPANDER_BOTTOM)
    iupBaseSetPosition(ih->firstchild, x, y + ih->currentheight - bar_size);
  else /* IEXPANDER_TOP */
  {
    iupBaseSetPosition(ih->firstchild, x, y);
    y += bar_size;
  }

  if (child)
  {
    if (ih->data->state == IEXPANDER_OPEN)
      iupBaseSetPosition(child, x, y);
    else if (ih->data->state == IEXPANDER_OPEN_FLOAT)
    {
      if (ih->data->position == IEXPANDER_RIGHT)
        x -= child->currentwidth;
      else if (ih->data->position == IEXPANDER_BOTTOM)
        y -= child->currentheight;

      iupBaseSetPosition(child, x, y);
    }
  }
}

static void iExpanderChildAddedMethod(Ihandle* ih, Ihandle* child)
{
  iExpanderOpenCloseChild(ih, 0, 0, ih->data->state);
  (void)child;
}

static int iExpanderCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->position = IEXPANDER_TOP;
  ih->data->state = IEXPANDER_OPEN;
  ih->data->barSize = -1;

  bar = IupCanvas(NULL);
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */
  bar->flags |= IUP_INTERNAL;

  IupSetAttribute(bar, "CANFOCUS", "NO");
  IupSetAttribute(bar, "BORDER", "NO");
  IupSetAttribute(bar, "EXPAND", "NO");

  /* Setting callbacks */
  IupSetCallback(bar, "BUTTON_CB", (Icallback) iExpanderButton_CB);
  IupSetCallback(bar, "ACTION",    (Icallback) iExpanderAction_CB);
  IupSetCallback(bar, "ENTERWINDOW_CB", (Icallback)iExpanderEnterWindow_cb);
  IupSetCallback(bar, "LEAVEWINDOW_CB", (Icallback)iExpanderLeaveWindow_cb);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  return IUP_NOERROR;
}

static void iExpanderDestroyMethod(Ihandle* ih)
{
  if (ih->data->timer)
    IupDestroy(ih->data->timer);
}

Iclass* iupExpanderNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "expander";
  ic->format = "h";   /* one ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY+2;  /* canvas+child */
  ic->is_interactive = 0;

  /* Class functions */
  ic->New     = iupExpanderNewClass;
  ic->Create  = iExpanderCreateMethod;
  ic->Destroy = iExpanderDestroyMethod;
  ic->Map     = iupBaseTypeVoidMapMethod;
  ic->ChildAdded = iExpanderChildAddedMethod;

  ic->ComputeNaturalSize     = iExpanderComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iExpanderSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition    = iExpanderSetChildrenPositionMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "ACTION", "");
  iupClassRegisterCallback(ic, "OPENCLOSE_CB", "i");
  iupClassRegisterCallback(ic, "EXTRABUTTON_CB", "ii");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iExpanderGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);

  /* IupExpander only */
  iupClassRegisterAttribute(ic, "BARPOSITION", NULL, iExpanderSetPositionAttrib, IUPAF_SAMEASSYSTEM, "TOP", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BARSIZE", iExpanderGetBarSizeAttrib, iExpanderSetBarSizeAttrib, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STATE", iExpanderGetStateAttrib, iExpanderSetStateAttrib, IUPAF_SAMEASSYSTEM, "OPEN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FORECOLOR", NULL, iExpanderPostRedrawSetAttrib, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKCOLOR", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AUTOSHOW", iExpanderGetAutoShowAttrib, iExpanderSetAutoShowAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXTRABUTTONS", iExpanderGetExtraButtonsAttrib, iExpanderSetExtraButtonsAttrib, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGE", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEHIGHLIGHT", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEOPEN", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEOPENHIGHLIGHT", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGEEXTRA1", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAPRESS1", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAHIGHLIGHT1", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRA2", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAPRESS2", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAHIGHLIGHT2", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRA3", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAPRESS3", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAHIGHLIGHT3", NULL, iExpanderPostRedrawSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupExpander(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("expander", children);
}
