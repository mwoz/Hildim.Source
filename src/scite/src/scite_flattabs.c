/** \file
* \brief iupflattabs control
*
* See Copyright Notice in "iup.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <lua.h>
#include "lauxlib.h"

#include "iup.h"
#include "iupcbs.h"

#include "../../iup/src/iup_object.h"
#include "../../iup/src/iup_attrib.h"
#include "../../iup/src/iup_str.h"
#include "../../iup/src/iup_drv.h"
#include "../../iup/src/iup_drvfont.h"
#include "../../iup/src/iup_stdcontrols.h"
#include "../../iup/src/iup_layout.h"
#include "../../iup/src/iup_image.h"
#include "../../iup/src/iup_register.h"
#include "../../iup/src/iup_drvdraw.h"
#include "../../iup/srclua5/il.h"
#include "../../iup/src/win/iupwin_handle.h"
#include "../../iup/srccontrols/color/iup_colorhsi.h"
#include "scite_flattabs.h"
#include <windows.h>

#define ITABS_CLOSE_SIZE 13
#define ITABS_CLOSE_SPACING 12
#define ITABS_CLOSE_BORDER 8
#define ITABS_NONE -1
#define ITABS_SB_LEFT -2
#define ITABS_SB_RIGHT -3
#define ITABS_EXTRABUTTON1 -4

#define ITABS_TABID2EXTRABUT(_id) (ITABS_EXTRABUTTON1 - _id + 1)
#define ITABS_EXTRABUT2TABID(_id) (ITABS_EXTRABUTTON1 - _id + 1) /* equal to the above, the conversion is symmetric */

static Ihandle* load_image_expand_down(void) {
	unsigned char imgdata[] = {
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
	  0, 0, 0, 8, 0, 0, 0, 48, 0, 0, 0, 21, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 21, 0, 0, 0, 48, 0, 0, 0, 8,
	  0, 0, 0, 45, 0, 0, 0, 109, 0, 0, 0, 93, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 93, 0, 0, 0, 109, 0, 0, 0, 45,
	  0, 0, 0, 17, 0, 0, 0, 94, 0, 0, 0, 119, 0, 0, 0, 93, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 93, 0, 0, 0, 119, 0, 0, 0, 93, 0, 0, 0, 16,
	  0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 0, 93, 0, 0, 0, 118, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 25, 0, 0, 0, 92, 0, 0, 0, 118, 0, 0, 0, 93, 0, 0, 0, 24, 0, 0, 0, 1,
	  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 0, 92, 0, 0, 0, 118, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 25, 0, 0, 0, 92, 0, 0, 0, 118, 0, 0, 0, 92, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 92, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 92, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 92, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 28, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 28, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 28, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 92, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 101, 0, 0, 0, 101, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 90, 0, 0, 0, 120, 0, 0, 0, 120, 0, 0, 0, 90, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	Ihandle* image = IupImageRGBA(24, 16, imgdata);
	return image;
}

static Ihandle* load_image_expand_up(void) {
	unsigned char imgdata[] = {
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 88, 0, 0, 0, 88, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 92, 0, 0, 0, 119, 0, 0, 0, 119, 0, 0, 0, 92, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 100, 0, 0, 0, 100, 0, 0, 0, 114, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 31, 0, 0, 0, 30, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 28, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 117, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 90, 0, 0, 0, 116, 0, 0, 0, 90, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 92, 0, 0, 0, 117, 0, 0, 0, 92, 0, 0, 0, 26, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 26, 0, 0, 0, 92, 0, 0, 0, 117, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 92, 0, 0, 0, 117, 0, 0, 0, 92, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 92, 0, 0, 0, 117, 0, 0, 0, 92, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 0, 93, 0, 0, 0, 117, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 92, 0, 0, 0, 117, 0, 0, 0, 93, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 0,
	  0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 0, 93, 0, 0, 0, 118, 0, 0, 0, 93, 0, 0, 0, 25, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 25, 0, 0, 0, 93, 0, 0, 0, 118, 0, 0, 0, 93, 0, 0, 0, 23, 0, 0, 0, 1,
	  0, 0, 0, 17, 0, 0, 0, 94, 0, 0, 0, 119, 0, 0, 0, 93, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 93, 0, 0, 0, 119, 0, 0, 0, 94, 0, 0, 0, 16,
	  0, 0, 0, 46, 0, 0, 0, 111, 0, 0, 0, 94, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 0, 94, 0, 0, 0, 111, 0, 0, 0, 46,
	  0, 0, 0, 7, 0, 0, 0, 48, 0, 0, 0, 20, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 20, 0, 0, 0, 48, 0, 0, 0, 7,
	  0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	Ihandle* image = IupImageRGBA(24, 16, imgdata);
	return image;
}
typedef int(*IFnniiiiis)(Ihandle*, Ihandle*, int, int, int, int, int, char*);
typedef int(*IFnniiiiiis)(Ihandle*, Ihandle*, int, int, int, int, int, int, char*);

static void iFlatTabsInitializeImages(void) {
	Ihandle *image;

	unsigned char img_close[ITABS_CLOSE_SIZE * ITABS_CLOSE_SIZE] =
	{
	  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	  0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0,
	  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
	  0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0,
	  0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	  0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0,
	  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
	  0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0,
	  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	};

	image = IupImage(ITABS_CLOSE_SIZE, ITABS_CLOSE_SIZE, img_close);
	IupSetAttribute(image, "0", "BGCOLOR");
	IupSetAttribute(image, "1", "0 0 0");
	IupSetHandle("IMGFLATCLOSE", image);

	image = IupImage(ITABS_CLOSE_SIZE, ITABS_CLOSE_SIZE, img_close);
	IupSetAttribute(image, "0", "BGCOLOR");
	IupSetAttribute(image, "1", "255 255 255");
	IupSetHandle("IMGFLATCLOSEPRESS", image);

	image = load_image_expand_down();
	IupSetHandle("IupFlatExpandDown", image);

	image = load_image_expand_up();
	IupSetHandle("IupFlatExpandUp", image);
}

static int flattab_tab_button_cb(Ihandle *self, Ihandle *p0, int p1, int p2, int p3, int p4, int p5, int p6, char * p7) {
	lua_State *L = iuplua_call_start(self, "tab_button_cb");
	iuplua_pushihandle(L, p0);
	lua_pushinteger(L, p1);
	lua_pushinteger(L, p2);
	lua_pushinteger(L, p3); 
	lua_pushinteger(L, p4);
	lua_pushinteger(L, p5);
	lua_pushinteger(L, p6);
	lua_pushstring(L, p7);
	return iuplua_call(L, 8);
}

static int flattab_tab_motion_cb(Ihandle *self, Ihandle* p0, int p1, int p2, int p3, int p4, int p5, char * p6) {
	lua_State *L = iuplua_call_start(self, "tab_motion_cb");
	iuplua_pushihandle(L, p0);
	lua_pushinteger(L, p1);
	lua_pushinteger(L, p2);
	lua_pushinteger(L, p3);
	lua_pushinteger(L, p4);
	lua_pushinteger(L, p5);
	lua_pushstring(L, p6);
	return iuplua_call(L, 7);
}


static void iFlatTabsGetIconSize(Ihandle* ih, int pos, int *w, int *h) {
	char* image = iupAttribGetId(ih, "TABIMAGE", pos);
	char* title = iupAttribGetId(ih, "TABTITLE", pos);

	*w = 0;
	*h = 0;

	if (image) {
		iupImageGetInfo(image, w, h, NULL);

		if (title) {
			int img_position = iupFlatGetImagePosition(iupAttribGetStr(ih, "TABSIMAGEPOSITION"));
			int spacing = iupAttribGetInt(ih, "TABSIMAGESPACING");
			int text_w, text_h;

			iupDrawGetTextSize(ih, title, &text_w, &text_h);

			if (img_position == IUP_IMGPOS_RIGHT ||
				img_position == IUP_IMGPOS_LEFT) {
				*w += text_w + spacing;
				*h = iupMAX(*h, text_h);
			} else {
				*w = iupMAX(*w, text_w);
				*h += text_h + spacing;
			}
		}
	} else if (title)
		iupDrawGetTextSize(ih, title, w, h);
}

static void iFlatTabsSetTabFont(Ihandle* ih, int pos) {
	char* font = iupAttribGetId(ih, "TABFONT", pos);
	if (font)
		iupAttribSetStr(ih, "DRAWFONT", font);
	else {
		char* tabs_font = iupAttribGet(ih, "TABSFONT");
		iupAttribSetStr(ih, "DRAWFONT", tabs_font);
	}
}

static void iFlatTabsGetTabSize(Ihandle* ih, int fixedwidth, int horiz_padding, int vert_padding, int show_close, int pos, int *tab_w, int *tab_h) {
	iFlatTabsSetTabFont(ih, pos);

	iFlatTabsGetIconSize(ih, pos, tab_w, tab_h);

	if (fixedwidth)
		*tab_w = fixedwidth;

	(*tab_w) += 2 * horiz_padding;
	(*tab_h) += 2 * vert_padding;

	if (show_close)
		(*tab_w) += ITABS_CLOSE_SPACING + ITABS_CLOSE_SIZE + ITABS_CLOSE_BORDER;
}

static int iFlatTabsGetTitleHeight(Ihandle* ih, int *title_width, int scrolled_width) {
	int vert_padding, horiz_padding;
	int title_height = 0, tab_w, tab_h, pos;
	int fixedwidth = iupAttribGetInt(ih, "FIXEDWIDTH");
	int show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
	int scroll_pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");

	iupAttribGetIntInt(ih, "TABSPADDING", &horiz_padding, &vert_padding, 'x');

	if (title_width)
		*title_width = 0;

	for (pos = 0; iupAttribGetId(ih, "TABTITLE", pos); pos++) {
		if (title_width && pos == scroll_pos && scrolled_width)
			*title_width = 0;

		iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close, pos, &tab_w, &tab_h);

		if (tab_h > title_height)
			title_height = tab_h;

		if (title_width)
			*title_width += tab_w;
	}

	return title_height;
}

static void iFlatTabsSetExtraFont(Ihandle* ih, int id) {
	char* font = iupAttribGetId(ih, "EXTRAFONT", id);
	if (font)
		iupAttribSetStr(ih, "DRAWFONT", font);
	else {
		char* tabs_font = iupAttribGet(ih, "TABSFONT");
		iupAttribSetStr(ih, "DRAWFONT", tabs_font);
	}
}

static int iFlatTabsGetExtraWidthId(Ihandle* ih, int i, int img_position, int horiz_padding) {
	char* image = iupAttribGetId(ih, "EXTRAIMAGE", i);
	char* title = iupAttribGetId(ih, "EXTRATITLE", i);

	int w = 0;

	iFlatTabsSetExtraFont(ih, i);

	if (image) {
		iupImageGetInfo(image, &w, NULL, NULL);

		if (title) {
			int spacing = iupAttribGetInt(ih, "TABSIMAGESPACING");
			int text_w, text_h;

			iupDrawGetTextSize(ih, title, &text_w, &text_h);

			if (img_position == IUP_IMGPOS_RIGHT ||
				img_position == IUP_IMGPOS_LEFT)
				w += text_w + spacing;
			else
				w = iupMAX(w, text_w);
		}
	} else if (title)
		iupDrawGetTextSize(ih, title, &w, NULL);

	w += 2 * horiz_padding;

	return w;
}

static int iFlatTabsGetExtraWidth(Ihandle* ih, int extra_buttons, int img_position, int horiz_padding) {
	int extra_width = 0, i;

	if (extra_buttons == 0)
		return 0;

	for (i = 1; i <= extra_buttons; i++) {
		int w = iFlatTabsGetExtraWidthId(ih, i, img_position, horiz_padding);
		extra_width += w;
	}

	return extra_width;
}

static int iFlatTabsGetExtraActive(Ihandle* ih, int id) {
	if (!iupAttribGetId(ih, "EXTRAACTIVE", id))
		return 1; /* default is yes */

	return iupAttribGetBooleanId(ih, "EXTRAACTIVE", id);
}

static void iFlatTabsGetAlignment(const char* alignment, int *horiz_alignment, int *vert_alignment) {
	char value1[30], value2[30];

	if (!alignment)
		return;

	iupStrToStrStr(alignment, value1, value2, ':');

	*horiz_alignment = iupFlatGetHorizontalAlignment(value1);
	*vert_alignment = iupFlatGetVerticalAlignment(value2);
}

static void iFlatTabsDrawScrollLeftButton(IdrawCanvas* dc, const char *tabs_bgcolor, const char *tabs_forecolor, int active, int title_height) {
	int scroll_width = title_height / 2;
	int arrow_size = (scroll_width + 1) / 2;

	int x = (scroll_width - arrow_size) / 2;
	int y = (title_height - arrow_size) / 2;

	iupFlatDrawArrow(dc, x, y, arrow_size, tabs_forecolor, tabs_bgcolor, active, IUPDRAW_ARROW_LEFT);
}

static void iFlatTabsDrawScrollRightButton(IdrawCanvas* dc, const char *tabs_bgcolor, const char *tabs_forecolor, int active, int title_height, int width) {
	int scroll_width = title_height / 2;
	int arrow_size = (scroll_width + 1) / 2;

	int x = width - 1 - scroll_width + (scroll_width - arrow_size) / 2;
	int y = (title_height - arrow_size) / 2;

	iupFlatDrawArrow(dc, x, y, arrow_size, tabs_forecolor, tabs_bgcolor, active, IUPDRAW_ARROW_RIGHT);
}


int iFlatTabsGetLastVisibleAttrib(Ihandle* ih) {
	int pos = 0;
	int tab_x = 0;
	int valuepos = iupAttribGetInt(ih, "VALUEPOS");
	int fixedwidth = iupAttribGetInt(ih, "FIXEDWIDTH");
	int horiz_padding, vert_padding;
	iupAttribGetIntInt(ih, "TABSPADDING", &horiz_padding, &vert_padding, 'x');
	int show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
	int title_width;
	int title_height = iFlatTabsGetTitleHeight(ih, &title_width, 1);
	int iWidth = max(iupAttribGetInt(ih, "_IUPFWIDTH"), ih->currentwidth);
	int img_position = iupFlatGetImagePosition(iupAttribGetStr(ih, "TABSIMAGEPOSITION"));
	int extra_buttons = iupAttribGetInt(ih, "EXTRABUTTONS");

	int extra_width = iFlatTabsGetExtraWidth(ih, extra_buttons, img_position, horiz_padding);

	for (pos = 0; iupAttribGetId(ih, "TABTITLE", pos); pos++) {
		int tab_w, tab_h;
		iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close && (valuepos == pos), pos, &tab_w, &tab_h);  /* this will also set any id based font */

		int scroll_width = title_height / 2;

		if (tab_x + tab_w > iWidth - extra_width - scroll_width) {
			break;
		}
		tab_x += tab_w;
	}
	//return iupStrReturnInt(pos);
	return pos;
}

static int iFlatTabsGetCountAttrib(Ihandle* ih) {
	int pos = 0;
	for (pos = 0; iupAttribGetId(ih, "TABTITLE", pos); pos++);
	return pos;
}

static int iFlatTabsRedraw_CB(Ihandle* ih) {

	Ihandle* prnt = IupGetParent(ih);

	if ((ih->currentwidth < 20) && (ih->currentwidth < prnt->currentwidth) && (iupAttribGetInt(ih, "_IUPFREDRAW") < 1000)) {
		iupAttribSetInt(ih, "_IUPFREDRAW", iupAttribGetInt(ih, "_IUPFREDRAW") + 1);
		IupRefreshChildren(prnt);
		IupUpdateChildren(prnt);

		return IUP_DEFAULT;
	}
	iupAttribSetInt(ih, "_IUPFREDRAW", 0);


	char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");
	char* bgcolormovied = iupAttribGetStr(ih, "BGCOLORMOVIED");
	char* forecolor = iupAttribGetStr(ih, "FORECOLOR");
	char* highcolor = iupAttribGetStr(ih, "HIGHCOLOR");
	char* tabs_bgcolor = iupAttribGet(ih, "TABSBACKCOLOR");
	char* tabs_forecolor = iupAttribGetStr(ih, "TABSFORECOLOR");
	char* tabs_highcolor = iupAttribGetStr(ih, "TABSHIGHCOLOR");
	int img_position = iupFlatGetImagePosition(iupAttribGetStr(ih, "TABSIMAGEPOSITION"));
	char* alignment = iupAttribGetStr(ih, "TABSALIGNMENT");
	char* text_align = iupAttribGetStr(ih, "TABSTEXTALIGNMENT");
	int active = IupGetInt(ih, "ACTIVE");  /* native implementation */
	int spacing = iupAttribGetInt(ih, "TABSIMAGESPACING");
	int horiz_padding, vert_padding, scroll_pos;
	int show_lines = iupAttribGetBoolean(ih, "SHOWLINES");
	int title_width;
	int title_height = iFlatTabsGetTitleHeight(ih, &title_width, 1);

	int fixedwidth = iupAttribGetInt(ih, "FIXEDWIDTH");
	int pos, horiz_alignment, vert_alignment, tab_x = 0;
	long line_color = 0;
	int show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
	int tab_highlighted = iupAttribGetInt(ih, "_IUPFTABS_HIGHLIGHTED");
	int extra_width;
	int extra_buttons = iupAttribGetInt(ih, "EXTRABUTTONS");
	int valuepos = iupAttribGetInt(ih, "VALUEPOS");
	scroll_pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");


	iupAttribGetIntInt(ih, "TABSPADDING", &horiz_padding, &vert_padding, 'x');
	int iWidth = max(iupAttribGetInt(ih, "_IUPFWIDTH"), ih->currentwidth);
	extra_width = iFlatTabsGetExtraWidth(ih, extra_buttons, img_position, horiz_padding);

	if (valuepos > 0 && !iupAttribGetBoolean(ih, "_SCIPAUTOSCROLL")) {
		int t_x = 0, p;
		for (p = valuepos; p >= 0; p--) {
			int tab_w, tab_h;
			iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close && (valuepos == p), p, &tab_w, &tab_h);  /* this will also set any id based font */

			int scroll_width = title_height / 2;

			t_x += tab_w;
			if (t_x + tab_w > iWidth - extra_width - scroll_width) {
				break;
			}
		}
		if (p > scroll_pos) {
			scroll_pos = p;
			iupAttribSetInt(ih, "_IUPFTABS_SCROLLPOS", scroll_pos);
		} else {
			t_x = 0;
			for (p = iFlatTabsGetCountAttrib(ih); p >= 0; p--) {
				int tab_w, tab_h;
				iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close && (valuepos == p), p, &tab_w, &tab_h);  /* this will also set any id based font */

				int scroll_width = title_height / 2;

				t_x += tab_w;
				if (t_x + tab_w > iWidth - extra_width - scroll_width) {
					break;
				}
			}
			//p++;
			if (p < 0)
				p = 0;
			if (p < scroll_pos) {
				scroll_pos = p;
				iupAttribSetInt(ih, "_IUPFTABS_SCROLLPOS", scroll_pos);
			}
		}
	}

	IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);

	if (!tabs_bgcolor)
		tabs_bgcolor = iupBaseNativeParentGetBgColorAttrib(ih);

	/* draw child area background */
	iupFlatDrawBox(dc, 0, iWidth - 1, title_height, ih->currentheight - 1, bgcolor, NULL, 1);

	/* title area background */
	iupFlatDrawBox(dc, 0, iWidth - 1, 0, title_height, tabs_bgcolor, NULL, 1);


	iFlatTabsGetAlignment(alignment, &horiz_alignment, &vert_alignment);

	if (show_lines) {
		char* title_line_color = iupAttribGetStr(ih, "TABSLINECOLOR");
		line_color = iupDrawStrToColor(title_line_color, line_color);

		/* tab bottom horizontal and top children horizontal */
		//iupdrvDrawLine(dc, 0, title_height - 1, iWidth - 1, title_height - 1, line_color, IUP_DRAW_STROKE, -1);
	}

	if (scroll_pos > 0) {
		int scroll_width = title_height / 2;
		tab_x += scroll_width;
	}

	for (pos = scroll_pos; iupAttribGetId(ih, "TABTITLE", pos); pos++) {

		char* tab_image = iupAttribGetId(ih, "TABIMAGE", pos);
		char* tab_title = iupAttribGetId(ih, "TABTITLE", pos);
		char* tab_backcolor = iupAttribGetId(ih, "TABBACKCOLOR", pos);
		char* tab_forecolor = iupAttribGetId(ih, "TABFORECOLOR", pos);
		char* tab_highcolor = iupAttribGetId(ih, "TABHIGHCOLOR", pos);
		char* background_color = NULL;
		int tab_w, tab_h, tab_active, reset_clip;
		char* foreground_color;
		int icon_width, make_inactive = 0;

		if (!active)
			tab_active = active;
		else
			tab_active = 1;

		iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close && (valuepos == pos), pos, &tab_w, &tab_h);  /* this will also set any id based font */

		if (valuepos == pos) {
			/* current tab is always drawn with these colors */
			background_color = ih->data->start? bgcolormovied :bgcolor;
			foreground_color = forecolor;
		} else {
			/* other tabs are drawn with these colors */
			if (tab_backcolor)
				background_color = tab_backcolor;

			foreground_color = tabs_forecolor;
			if (tab_forecolor)
				foreground_color = tab_forecolor;

			if (pos == tab_highlighted) {
				if (highcolor)
					foreground_color = highcolor;
				else
					foreground_color = forecolor;

				if (tabs_highcolor || tab_highcolor) {
					if (tab_highcolor)
						background_color = tab_highcolor;
					else
						background_color = tabs_highcolor;
				}
			}
		}

		if (tab_image) {
			make_inactive = 0;

			if (!tab_active) {
				char* tab_image_inative = iupAttribGetId(ih, "TABIMAGEINACTIVE", pos);
				if (!tab_image_inative)
					make_inactive = 1;
				else
					tab_image = tab_image_inative;
			} else if (pos == tab_highlighted) {
				char* tab_image_highlight = iupAttribGetId(ih, "TABIMAGEHIGHLIGHT", pos);
				if (tab_image_highlight)
					tab_image = tab_image_highlight;
			}
		}

		reset_clip = 0;
		int valuenotdraw = 0;

		if (title_width > iWidth - extra_width) /* has right scroll button */
		{
			int scroll_width = title_height / 2;

			if (tab_x + tab_w > iWidth - extra_width - scroll_width) {
				iupdrvDrawSetClipRect(dc, tab_x, 0, iWidth - extra_width - scroll_width, title_height);
				reset_clip = 1;
				if (pos == valuepos)
					valuenotdraw = 1;
			}
		}

		/* draw tab title background */
		if (background_color)
			iupFlatDrawBox(dc, tab_x, tab_x + tab_w, 0, title_height, background_color, NULL, 1);
		else
			background_color = tabs_bgcolor;
		if (show_lines) {
			if (valuepos == pos)//
			{
				iupdrvDrawLine(dc, tab_x, 0, tab_x + tab_w - 1, 0, line_color, IUP_DRAW_STROKE, -1); /* tab top horizontal */
				iupdrvDrawLine(dc, tab_x, 0, tab_x, title_height - 1, line_color, IUP_DRAW_STROKE, -1); /* tab left vertical */
				iupdrvDrawLine(dc, tab_x + tab_w - 1, 0, tab_x + tab_w - 1, title_height, line_color, IUP_DRAW_STROKE, -1); /* tab right vertical */
			} else if (tab_backcolor) {
				char * border_color = iupAttribGetId(ih, "_TABBORDERCOLOR", pos);
				unsigned char c_r = 0, c_g = 0, c_b = 0;
				iupStrToRGB(border_color, &c_r, &c_g, &c_b);
				long border_linecolor = 0;
				border_linecolor = iupDrawStrToColor(border_color, border_linecolor);
				iupdrvDrawLine(dc, tab_x, 0, tab_x + tab_w - 1, 0, border_linecolor, IUP_DRAW_STROKE, -1); /* tab top horizontal */
				iupdrvDrawLine(dc, tab_x, 0, tab_x, title_height - 1, border_linecolor, IUP_DRAW_STROKE, -1); /* tab left vertical */
				iupdrvDrawLine(dc, tab_x + tab_w - 1, 0, tab_x + tab_w - 1, title_height - 1, border_linecolor, IUP_DRAW_STROKE, -1); /* tab right vertical */
				iupdrvDrawLine(dc, tab_x, title_height, tab_x + tab_w - 1, title_height, border_linecolor, IUP_DRAW_STROKE, -1); /* tab top horizontal */
			}
		}

		icon_width = tab_w;
		if (show_close && (valuepos == pos))
			icon_width -= ITABS_CLOSE_SPACING + ITABS_CLOSE_SIZE + ITABS_CLOSE_BORDER;

		iupFlatDrawIcon(ih, dc, tab_x, 0,
			icon_width, title_height,
			img_position, spacing, horiz_alignment, vert_alignment, horiz_padding, vert_padding,
			tab_image, make_inactive, tab_title, text_align, foreground_color, background_color, tab_active);

		if (show_close && (valuepos == pos)) {
			int close_x = tab_x + tab_w - ITABS_CLOSE_BORDER - ITABS_CLOSE_SIZE;
			int close_y = (title_height - (ITABS_CLOSE_SIZE)) / 2;
			const char* imagename;
			int tab_close_high = iupAttribGetInt(ih, "_IUPFTABS_CLOSEHIGH");
			int tab_close_press = iupAttribGetInt(ih, "_IUPFTABS_CLOSEPRESS");

			if (pos == tab_close_press) {
				background_color = iupAttribGetStr(ih, "CLOSEPRESSCOLOR");
				iupFlatDrawBox(dc, close_x - ITABS_CLOSE_BORDER, close_x + ITABS_CLOSE_SIZE + ITABS_CLOSE_BORDER, close_y - ITABS_CLOSE_BORDER, close_y + ITABS_CLOSE_SIZE + ITABS_CLOSE_BORDER, background_color, NULL, 1);
			} else if (pos == tab_close_high) {
				background_color = iupAttribGetStr(ih, "CLOSEHIGHCOLOR");
				iupFlatDrawBox(dc, close_x - ITABS_CLOSE_BORDER, close_x + ITABS_CLOSE_SIZE + ITABS_CLOSE_BORDER, close_y - ITABS_CLOSE_BORDER, close_y + ITABS_CLOSE_SIZE + ITABS_CLOSE_BORDER, background_color, NULL, 1);
			}

			imagename = iupFlatGetImageName(ih, "CLOSEIMAGE", NULL, pos == tab_close_press, pos == tab_close_high, tab_active, &make_inactive);
			iupdrvDrawImage(dc, imagename, make_inactive, close_x, close_y);
		}

		/* goto next tab area */
		tab_x += tab_w;

		if (reset_clip) {
			iupdrvDrawResetClip(dc);
		}
	}


	if (scroll_pos > 0) {
		char* foreground_color = tabs_forecolor;
		if (tab_highlighted == ITABS_SB_LEFT) {
			if (highcolor)
				foreground_color = highcolor;
			else
				foreground_color = forecolor;
		}

		iFlatTabsDrawScrollLeftButton(dc, tabs_bgcolor, foreground_color, active, title_height);
	}

	if (title_width > iWidth - extra_width) {
		char* foreground_color = tabs_forecolor;
		if (tab_highlighted == ITABS_SB_RIGHT) {
			if (highcolor)
				foreground_color = highcolor;
			else
				foreground_color = forecolor;
		}

		iFlatTabsDrawScrollRightButton(dc, tabs_bgcolor, foreground_color, active, title_height, iWidth - extra_width);
	}

	if (extra_buttons) {
		int i, right_extra_width = 0, extra_id;
		int extra_active, make_inactive, extra_x, extra_w;
		int extra_horiz_alignment, extra_vert_alignment;

		for (i = 1; i <= extra_buttons; i++) {
			const char* extra_image = iupAttribGetId(ih, "EXTRAIMAGE", i);
			char* extra_title = iupAttribGetId(ih, "EXTRATITLE", i);
			char* extra_alignment = iupAttribGetId(ih, "EXTRAALIGNMENT", i);
			char* extra_forecolor = iupAttribGetId(ih, "EXTRAFORECOLOR", i);
			int extra_press = iupAttribGetInt(ih, "_IUPFTABS_EXTRAPRESS");

			extra_horiz_alignment = horiz_alignment;
			extra_vert_alignment = vert_alignment;
			iFlatTabsGetAlignment(extra_alignment, &extra_horiz_alignment, &extra_vert_alignment);

			if (!active)
				extra_active = active;
			else
				extra_active = iFlatTabsGetExtraActive(ih, i);

			if (!extra_forecolor)
				extra_forecolor = tabs_forecolor;

			extra_id = ITABS_EXTRABUT2TABID(i);

			extra_w = iFlatTabsGetExtraWidthId(ih, i, img_position, horiz_padding);  /* this will also set any id based font */

			extra_x = iWidth - right_extra_width - extra_w;

			if (extra_press == extra_id) {
				char* extra_presscolor = iupAttribGetId(ih, "EXTRAPRESSCOLOR", i);
				if (!extra_presscolor)
					extra_presscolor = iupAttribGetStr(ih, "CLOSEPRESSCOLOR"); 

				iupFlatDrawBox(dc, extra_x + horiz_padding / 2, extra_x + extra_w - horiz_padding / 2, vert_padding / 2, title_height - 1 - vert_padding / 2, extra_presscolor, NULL, 1);
			} else if (tab_highlighted == extra_id) {
				char* extra_highcolor = iupAttribGetId(ih, "EXTRAHIGHCOLOR", i);
				if (!extra_highcolor)
					extra_highcolor = iupAttribGetStr(ih, "CLOSEHIGHCOLOR");

				iupFlatDrawBox(dc, extra_x + horiz_padding / 2, extra_x + extra_w - horiz_padding / 2, vert_padding / 2, title_height - 1 - vert_padding / 2, extra_highcolor, NULL, 1);
			}

			extra_image = iupFlatGetImageNameId(ih, "EXTRAIMAGE", i, extra_image, extra_press == extra_id, tab_highlighted == extra_id, extra_active, &make_inactive);

			iupFlatDrawIcon(ih, dc, extra_x, 0,
				extra_w, title_height - 1,
				img_position, spacing, extra_horiz_alignment, extra_vert_alignment, horiz_padding, vert_padding,
				extra_image, make_inactive, extra_title, text_align, extra_forecolor, tabs_bgcolor, extra_active);

			right_extra_width += extra_w;
		}
	}

	/* lines around children */
	if (show_lines) {
		iupdrvDrawLine(dc, 0, title_height, 0, ih->currentheight - 1, line_color, IUP_DRAW_STROKE, -1); /* left children vertical */
		iupdrvDrawLine(dc, iWidth - 1, title_height, iWidth - 1, ih->currentheight - 1, line_color, IUP_DRAW_STROKE, -1); /* right children vertical */
		//iupdrvDrawLine(dc, 0, ih->currentheight - 1, iWidth - 1, ih->currentheight - 1, line_color, IUP_DRAW_STROKE, -1); /* bottom children horizontal */
	}

	iupdrvDrawFlush(dc);

	iupdrvDrawKillCanvas(dc);

	return IUP_DEFAULT;
}

static int iFlatTabsResize_CB(Ihandle* ih, int width, int height) {
	iupAttribSetInt(ih, "_IUPFWIDTH", width);
	iupAttribSetInt(ih, "_SCIPAUTOSCROLL", 0);
	int scroll_pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");
	if (scroll_pos) {
		int title_width;
		iFlatTabsGetTitleHeight(ih, &title_width, 0);

		if (title_width > width) {
			/* tabs are larger than the element, leave scroll_pos as it is */
			return IUP_DEFAULT;
		}

		/* tabs fit in element area, reset scroll_pos */
		iupAttribSetInt(ih, "_IUPFTABS_SCROLLPOS", 0);
	}

	(void)height;
	return IUP_DEFAULT;
}

static int iFlatTabsCallTabChange(Ihandle* ih, int prev_pos, int pos) {
	IFnnn cb = (IFnnn)IupGetCallback(ih, "TABCHANGE_CB");
	int ret = IUP_DEFAULT;

	IFnii cb2 = (IFnii)IupGetCallback(ih, "TABCHANGEPOS_CB");
	if (cb2) {
		ret = cb2(ih, pos, prev_pos);
	}
	return ret;
}

static int iFlatTabsFindTab(Ihandle* ih, int cur_x, int cur_y, int show_close, int *inside_close) {
	int title_width;
	int title_height = iFlatTabsGetTitleHeight(ih, &title_width, 1);

	*inside_close = 0;

	if (cur_y < title_height && cur_y > 0) {
		int pos, horiz_padding, vert_padding, tab_x = 0, scroll_pos;
		int fixedwidth = iupAttribGetInt(ih, "FIXEDWIDTH");
		int img_position = iupFlatGetImagePosition(iupAttribGetStr(ih, "TABSIMAGEPOSITION"));
		int extra_width;
		int extra_buttons = iupAttribGetInt(ih, "EXTRABUTTONS");

		iupAttribGetIntInt(ih, "TABSPADDING", &horiz_padding, &vert_padding, 'x');
		extra_width = iFlatTabsGetExtraWidth(ih, extra_buttons, img_position, horiz_padding);

		scroll_pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");

		if (scroll_pos > 0) {
			int scroll_width = title_height / 2;
			if (cur_x < scroll_width)
				return ITABS_SB_LEFT;

			tab_x += scroll_width;
		}

		if (title_width > ih->currentwidth - extra_width) {
			int scroll_width = title_height / 2;
			if (cur_x > ih->currentwidth - extra_width - scroll_width && cur_x < ih->currentwidth - extra_width)
				return ITABS_SB_RIGHT;
		}

		if (extra_buttons) {
			int i, right_extra_width = 0;
			for (i = 1; i <= extra_buttons; i++) {
				int w = iFlatTabsGetExtraWidthId(ih, i, img_position, horiz_padding);

				if (cur_x > ih->currentwidth - right_extra_width - w && cur_x < ih->currentwidth - right_extra_width)
					return ITABS_EXTRABUT2TABID(i);

				right_extra_width += w;
			}
		}

		int valuepos = iupAttribGetInt(ih, "VALUEPOS");
		for (pos = scroll_pos; iupAttribGetId(ih, "TABTITLE", pos); pos++) {
			int tab_w, tab_h;

			iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close && (valuepos == pos), pos, &tab_w, &tab_h);

			if (cur_x > tab_x && cur_x < tab_x + tab_w) {
				if (show_close && (valuepos == pos)) {
					int close_end = tab_x + tab_w - ITABS_CLOSE_BORDER;
					int close_start = close_end - ITABS_CLOSE_SIZE;
					if (cur_x >= close_start && cur_x <= close_end)
						*inside_close = 1;
				}

				return pos;
			}

			tab_x += tab_w;

			if (tab_x > ih->currentwidth)
				break;
		}
	}

	return ITABS_NONE;
}


/*****************************************************************************************/

static void iFlatTabsToggleExpand(Ihandle* ih) {
	int expand_pos = iupAttribGetInt(ih, "EXPANDBUTTONPOS");
	int expand_state = iupAttribGetBoolean(ih, "EXPANDBUTTONSTATE");
	if (expand_state) {
		int title_height = iFlatTabsGetTitleHeight(ih, NULL, 0);
		iupAttribSetId(ih, "EXTRAIMAGE", expand_pos, "IupFlatExpandDown");
		iupAttribSet(ih, "EXPANDBUTTONSTATE", "No");
		IupSetStrf(ih, "MAXSIZE", "x%d", title_height);
		iupAttribSetInt(ih, "_IUP_FULLHEIGHT", ih->currentheight);
		IupRefresh(ih);
	} else {
		iupAttribSetId(ih, "EXTRAIMAGE", expand_pos, "IupFlatExpandUp");
		iupAttribSet(ih, "EXPANDBUTTONSTATE", "Yes");
		IupSetAttribute(ih, "MAXSIZE", NULL);
		iupAttribSet(ih, "_IUP_FULLHEIGHT", NULL);
		IupRefresh(ih);
	}
}

static int iFlatTabsKillFocus_CB(Ihandle* ih) {
	ih->currentheight = IupGetInt2(ih, "MAXSIZE");
	iupLayoutUpdate(ih);
	IupSetCallback(ih, "KILLFOCUS_CB", NULL);
	return IUP_DEFAULT;
}

static int iFlatTabsButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status) {

	int inside_close;
	int show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
	int tab_found = iFlatTabsFindTab(ih, x, y, show_close, &inside_close);

	if ((button == IUP_BUTTON1 || button == IUP_BUTTON2 || button == IUP_BUTTON3) && pressed) {
		if (tab_found > ITABS_NONE) {
			iupAttribSetInt(ih, "_SCIPAUTOSCROLL", 0);
			if (show_close && inside_close) {
				iupAttribSetInt(ih, "_IUPFTABS_CLOSEPRESS", tab_found);  /* used for press feedback */
				iupdrvPostRedraw(ih);
			} else {

				iupAttribSetInt(ih, "_IUPFTABS_CLOSEPRESS", ITABS_NONE);

				int prev_pos = iupAttribGetInt(ih, "VALUEPOS");
				if (tab_found != prev_pos) {
					iupAttribSetInt(ih, "VALUEPOS", tab_found);
					//int prev_pos = IupGetChildPos(ih, prev_child);
					int ret = iFlatTabsCallTabChange(ih, prev_pos, tab_found);

					iFlatTabsRedraw_CB(ih);
				}

				if (iupAttribGetBoolean(ih, "EXPANDBUTTON") && !iupAttribGetBoolean(ih, "EXPANDBUTTONSTATE")) {
					ih->currentheight = iupAttribGetInt(ih, "_IUP_FULLHEIGHT");
					IupSetAttribute(ih, "ZORDER", "TOP");
					iupLayoutUpdate(ih);
					IupSetCallback(ih, "KILLFOCUS_CB", (Icallback)iFlatTabsKillFocus_CB);
				}
			}
		} else if (tab_found == ITABS_SB_LEFT) {
			int pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");
			pos--;
			iupAttribSetInt(ih, "_IUPFTABS_SCROLLPOS", pos);
			iupAttribSetInt(ih, "_SCIPAUTOSCROLL", 1);
			iupdrvPostRedraw(ih);
		} else if (tab_found == ITABS_SB_RIGHT) {
			int pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");
			pos++;

			iupAttribSetInt(ih, "_IUPFTABS_SCROLLPOS", pos);
			iupAttribSetInt(ih, "_SCIPAUTOSCROLL", 1);
			iupdrvPostRedraw(ih);
		} else if (tab_found <= ITABS_EXTRABUTTON1) {
			IFnii cb = (IFnii)IupGetCallback(ih, "EXTRABUTTON_CB");
			if (cb)
				cb(ih, ITABS_TABID2EXTRABUT(tab_found), 1);

			iupAttribSetInt(ih, "_IUPFTABS_EXTRAPRESS", tab_found);
			iupdrvPostRedraw(ih);
		}
	} else if (button == IUP_BUTTON1 && !pressed) {
		int extra_buttons;
		int tab_found = ITABS_NONE;

		int show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
		if (show_close) {
			int tab_close_press = iupAttribGetInt(ih, "_IUPFTABS_CLOSEPRESS");
			int inside_close;
			tab_found = iFlatTabsFindTab(ih, x, y, show_close, &inside_close);

			iupAttribSetInt(ih, "_IUPFTABS_CLOSEPRESS", ITABS_NONE);

			if (tab_found > ITABS_NONE && inside_close && tab_close_press == tab_found) {
				int ret = IUP_DEFAULT;
				IFni cb = (IFni)IupGetCallback(ih, "TABCLOSE_CB");
				if (cb)
					ret = cb(ih, tab_found);
				if (ret == IUP_DEFAULT) /* hide tab and children */
				  //IupSetAttributeId(ih, "TABVISIBLE", tab_found, "No")
					;
				//!!!TODO
				else
					iupdrvPostRedraw(ih);
			}
		}

		extra_buttons = iupAttribGetInt(ih, "EXTRABUTTONS");
		if (extra_buttons) {
			int extra_press = iupAttribGetInt(ih, "_IUPFTABS_EXTRAPRESS");
			int inside_close;
			if (!show_close)
				tab_found = iFlatTabsFindTab(ih, x, y, show_close, &inside_close);

			iupAttribSetInt(ih, "_IUPFTABS_EXTRAPRESS", ITABS_NONE);

			if (tab_found <= ITABS_EXTRABUTTON1 && iFlatTabsGetExtraActive(ih, tab_found) && extra_press == tab_found) {
				IFnii cb = (IFnii)IupGetCallback(ih, "EXTRABUTTON_CB");
				if (cb)
					cb(ih, ITABS_TABID2EXTRABUT(tab_found), 0);

				if (iupAttribGetBoolean(ih, "EXPANDBUTTON") && ITABS_TABID2EXTRABUT(tab_found) == iupAttribGetInt(ih, "EXPANDBUTTONPOS"))
					iFlatTabsToggleExpand(ih);

				iupdrvPostRedraw(ih);
			}
		}
	}
	if (button == IUP_BUTTON3 && pressed) {
		IFni cb = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");
		if (cb) {
			int show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
			int inside_close;
			int tab_found = iFlatTabsFindTab(ih, x, y, show_close, &inside_close);
			if (tab_found > ITABS_NONE)
				cb(ih, tab_found);
		}
	}
	IFnniiiiiis cb = (IFnniiiiiis)IupGetCallback(ih, "TAB_BUTTON_CB");
	if (cb) {
		Ihandle* ihTarget = NULL;
		int dragTab = tab_found;
		if (!pressed && ih->data->start) {//
			if (x < 0 || y < 0 || ih->currentwidth < x || ih->currentheight < y) {
				POINT p;
				GetCursorPos(&p);
				HWND hwnd = WindowFromPoint(p);
				if (hwnd)
					ihTarget = iupwinHandleGet((InativeHandle*)hwnd);
			} else {
				dragTab = ih->data->dragTab;
			}
		}
		if (pressed) {
			ih->data->dragTab = iupAttribGetInt(ih, "VALUEPOS");
			ih->data->xStart = x;
			ih->data->yStart = y;
			ih->data->start = 0;
			ih->data->xFreeMax = 0;
		} else {
			ih->data->dragTab = ITABS_NONE;
			ih->data->start = 0;
			ih->data->xFreeMax = 0;
		}
		if (cb(ih, ihTarget, button, pressed, x, y, tab_found, dragTab, status) == IUP_IGNORE)
			return IUP_DEFAULT;
	}

	return IUP_DEFAULT;
}

static void iFlatTabsSetTipVisible(Ihandle* ih, const char* tip) {
	int visible = IupGetInt(ih, "TIPVISIBLE");

	/* do not call IupSetAttribute */
	iupAttribSetStr(ih, "TIP", tip);
	iupdrvBaseSetTipAttrib(ih, tip);

	if (visible) {
		IupSetAttribute(ih, "TIPVISIBLE", "No");
		if (tip)
			IupSetAttribute(ih, "TIPVISIBLE", "Yes");
	}
}

static int iFlatTabsCheckTip(Ihandle* ih, const char* new_tip) {
	char* tip = iupAttribGet(ih, "TIP");
	if (!tip && !new_tip)
		return 1;
	if (iupStrEqual(tip, new_tip))
		return 1;
	return 0;
}

static void iFlatTabsResetTip(Ihandle* ih) {
	char* tip = iupAttribGet(ih, "TABSTIP");
	if (!iFlatTabsCheckTip(ih, tip))
		iFlatTabsSetTipVisible(ih, tip);
}

static void iFlatTabsSetTip(Ihandle *ih, const char* tip) {
	if (!iFlatTabsCheckTip(ih, tip))
		iFlatTabsSetTipVisible(ih, tip);
}

static int iFlatTabsMotion_CB(Ihandle *ih, int x, int y, char *status) {
	int tab_found, tab_highlighted, redraw = 0;
	int inside_close, show_close;
	show_close = iupAttribGetBoolean(ih, "SHOWCLOSE");
	tab_found = iFlatTabsFindTab(ih, x, y, show_close, &inside_close);

	IFnniiiiis cb = (IFnniiiiis)IupGetCallback(ih, "TAB_MOTION_CB");
	IFnii cbS = (IFnii)IupGetCallback(ih, "TAB_SHIFT_CB");
	if (cb) {
		Ihandle* ihTarget = NULL;
		int dragTab = tab_found;
		int start = 0;

		if (ih->data->xFreeMax && (ih->data->xFreeMax < x || ih->data->xFree > x))
			ih->data->xFreeMax = 0;

		if (iup_isbutton1(status)) {
			if ((!ih->data->start && abs(x - ih->data->xStart) > 5 || abs(y - ih->data->yStart) > 5) && ih->data->dragTab > -1) {
				ih->data->start = 1;
				start++;
			}
			if (ih->data->start) {
				dragTab = ih->data->dragTab;
				start++;
			}
		}
		if (x < 0 || y < 0 || y > ih->currentheight || x > ih->currentwidth) {
			POINT p;
			GetCursorPos(&p);
			HWND hwnd = WindowFromPoint(p);
			if (hwnd)
				ihTarget = iupwinHandleGet((InativeHandle*)hwnd);
		}
		if (cb(ih, ihTarget, x, y, tab_found, dragTab, start, status) == IUP_IGNORE)
			return IUP_DEFAULT;

		if ((cbS) && start && (tab_found >= 0 || tab_found == -3 || tab_found == -2) && ih->data->dragTab != tab_found && !ih->data->xFreeMax) {
			if (tab_found == -3)
				tab_found = ih->data->dragTab + 1;
			else if (tab_found == -2)
				tab_found = ih->data->dragTab - 1;
			if (cbS(ih, ih->data->dragTab, tab_found) == IUP_IGNORE)
				return IUP_DEFAULT;
			int tab_found_new = iFlatTabsFindTab(ih, x, y, show_close, &inside_close);
			if (tab_found != tab_found_new) {
				int horiz_padding, vert_padding, tab_x = 0;
				int fixedwidth = iupAttribGetInt(ih, "FIXEDWIDTH");
				iupAttribGetIntInt(ih, "TABSPADDING", &horiz_padding, &vert_padding, 'x');
				int tab_w, tab_w2, tab_h;
				iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close, tab_found, &tab_w, &tab_h);
				iFlatTabsGetTabSize(ih, fixedwidth, horiz_padding, vert_padding, show_close, ih->data->dragTab, &tab_w2, &tab_h);

				if (tab_found > ih->data->dragTab) {
					ih->data->xFree = x;
					ih->data->xFreeMax = x - tab_w + tab_w2;
				} else {
					ih->data->xFree = x + tab_w - tab_w2;
					ih->data->xFreeMax = x;
				}
			}
			ih->data->dragTab = tab_found;
			SetCapture((HWND)ih->handle);
		}
	}

	tab_highlighted = iupAttribGetInt(ih, "_IUPFTABS_HIGHLIGHTED");

	if (tab_found == ITABS_NONE)
		iFlatTabsResetTip(ih);
	else {
		if (tab_found > ITABS_NONE) {
			char* tab_tip = iupAttribGetId(ih, "TABTIP", tab_found);
			if (tab_tip)
				iFlatTabsSetTip(ih, tab_tip);
			else
				iFlatTabsResetTip(ih);
		} else {
			char* extra_tip = iupAttribGetId(ih, "EXTRATIP", ITABS_TABID2EXTRABUT(tab_found));
			if (extra_tip)
				iFlatTabsSetTip(ih, extra_tip);
			else
				iFlatTabsResetTip(ih);
		}
	}

	if (tab_found != tab_highlighted && !inside_close) {
		iupAttribSetInt(ih, "_IUPFTABS_HIGHLIGHTED", tab_found);
		redraw = 1;
	}

	if (show_close && tab_found >= ITABS_NONE) {
		int tab_close_high, tab_close_press;

		tab_close_high = iupAttribGetInt(ih, "_IUPFTABS_CLOSEHIGH");
		if (inside_close) {
			if (tab_close_high != tab_found) {
				iupAttribSetInt(ih, "_IUPFTABS_HIGHLIGHTED", ITABS_NONE);
				iupAttribSetInt(ih, "_IUPFTABS_CLOSEHIGH", tab_found);
				redraw = 1;
			}
		} else {
			if (tab_close_high != ITABS_NONE) {
				iupAttribSetInt(ih, "_IUPFTABS_CLOSEHIGH", ITABS_NONE);
				redraw = 1;
			}
		}

		tab_close_press = iupAttribGetInt(ih, "_IUPFTABS_CLOSEPRESS");
		if (tab_close_press != ITABS_NONE && !inside_close) {
			iupAttribSetInt(ih, "_IUPFTABS_CLOSEPRESS", ITABS_NONE);
			redraw = 1;
		}
	}

	if (redraw)
		iupdrvPostRedraw(ih);

	return IUP_DEFAULT;
}

static int iFlatTabsLeaveWindow_CB(Ihandle* ih) {
	int tab_highlighted, tab_close_high, redraw = 0;

	IFn cb = (IFn)IupGetCallback(ih, "FLAT_LEAVEWINDOW_CB");
	if (cb) {
		if (cb(ih) == IUP_IGNORE)
			return IUP_DEFAULT;
	}

	tab_highlighted = iupAttribGetInt(ih, "_IUPFTABS_HIGHLIGHTED");
	if (tab_highlighted != ITABS_NONE) {
		iupAttribSetInt(ih, "_IUPFTABS_HIGHLIGHTED", ITABS_NONE);
		redraw = 1;
	}

	tab_close_high = iupAttribGetInt(ih, "_IUPFTABS_CLOSEHIGH");
	if (tab_close_high != ITABS_NONE) {
		iupAttribSetInt(ih, "_IUPFTABS_CLOSEHIGH", ITABS_NONE);
		redraw = 1;
	}

	iFlatTabsResetTip(ih);

	if (redraw)
		iupdrvPostRedraw(ih);

	return IUP_DEFAULT;
}


/*****************************************************************************************/



static int iFlatTabsSetValuePosAttrib(Ihandle* ih, const char* value) {
	int pos = (int)value;
	pos--;

	if (iFlatTabsGetCountAttrib(ih) <= pos)
		return 0;

	int scroll_pos = iupAttribGetInt(ih, "_IUPFTABS_SCROLLPOS");
	if (pos < scroll_pos)
		iupAttribSetInt(ih, "_IUPFTABS_SCROLLPOS", pos);

	iupAttribSetInt(ih, "_SCIPAUTOSCROLL", 0);
	iupAttribSetInt(ih, "VALUEPOS", pos);
	return 0;
}

static char* iFlatTabsGetValuePosAttrib(Ihandle* ih) {
	int pos = iupAttribGetInt(ih, "VALUEPOS");
	if (pos != -1)
		return iupStrReturnInt(pos);
	return NULL;
}

static int iFlatTabsSetValueAttrib(Ihandle* ih, const char* value) {
	return 0;
}

static int iupFlatTabsDrawSetRedrawAttrib(Ihandle* ih, const char* value) {
	iupdrvRedrawNow(ih);
	return 0;
}

static char* iFlatTabsGetValueAttrib(Ihandle* ih) {
	return "";
}

static char* iFlatTabsGetClientSizeAttrib(Ihandle* ih) {
	int width = ih->currentwidth;
	int height = ih->currentheight;

	height -= iFlatTabsGetTitleHeight(ih, NULL, 0);

	if (iupAttribGetBoolean(ih, "SHOWLINES")) {
		height -= 1;
		width -= 2;
	}

	if (width < 0) width = 0;
	if (height < 0) height = 0;

	return iupStrReturnIntInt(width, height, 'x');
}

static char* iFlatTabsGetBgColorAttrib(Ihandle* ih) {
	if (iupAttribGet(ih, "BGCOLOR"))
		return NULL;  /* get from the hash table */
	else
		return "255 255 255";
}

static int iFlatTabsSetTipAttrib(Ihandle* ih, const char* value) {
	iupAttribSetStr(ih, "TABSTIP", value);
	return iupdrvBaseSetTipAttrib(ih, value);
}

static int iFlatTabsSetActiveAttrib(Ihandle* ih, const char* value) {
	iupdrvPostRedraw(ih);
	return iupBaseSetActiveAttrib(ih, value);
}

static int iFlatTabsUpdateSetAttrib(Ihandle* ih, const char* value) {
	(void)value;
	// if (ih->handle)
		// iupdrvRedrawNow(ih);
	   //iupdrvPostRedraw(ih);

	return 1;
}

static int iupStrToHSI_Int(const char *str, int *h, int *s, int *i) {
	int fh, fs, fi;
	if (!str) return 0;
	if (sscanf(str, "%d %d %d", &fh, &fs, &fi) != 3) return 0;
	if (fh > 359 || fs > 100 || fi > 100) return 0;
	if (fh < 0 || fs < 0 || fi < 0) return 0;
	*h = fh;
	*s = fs;
	*i = fi;
	return 1;
}



static int iFlatTabsSetHueBackColor(Ihandle* ih, int id, const char* value) {
	int h, s, i;
	iupStrToInt(value, &h);	
	if (h == -1) {
		IupStoreAttributeId(ih, "TABBACKCOLOR", id, NULL);
		return 0;
	}


	iupStrToInt(IupGetAttribute(ih, "SATURATION"), &s);
	iupStrToInt(IupGetAttribute(ih, "ILLUMINATION"), &i);
	if (s < 1 || s > 99) s = 50;
	if (i < 1 || i > 99) i = 90;

	unsigned char r, g, b;

	iupColorHSI2RGB(h / 1.0, s / 100.0, i / 100.0, &r, &g, &b);
	IupStoreAttributeId(ih, "TABBACKCOLOR", id, iupStrReturnRGB(r, g, b));

	iupColorHSI2RGB(h / 1.0, s / 100.0, pow((i / 100.0), 1.5), &r, &g, &b);
	IupStoreAttributeId(ih, "_TABBORDERCOLOR", id, iupStrReturnRGB(r, g, b));

	return 0;
}
static int iFlatTabsSetTabFontStyleAttrib(Ihandle* ih, int id, const char* value) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	font = iupAttribGetId(ih, "TABFONT", id);
	if (!font)
		font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "TABFONT", id, "%s, %s %d", typeface, value, size);

	return 0;
}

static char* iFlatTabsGetTabFontStyleAttrib(Ihandle* ih, int id) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];

	char* font = iupAttribGetId(ih, "TABFONT", id);
	if (!font)
		font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnStrf("%s%s%s%s", is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "");
}

static int iFlatTabsSetTabFontSizeAttrib(Ihandle* ih, int id, const char* value) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	font = iupAttribGetId(ih, "TABFONT", id);
	if (!font)
		font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "TABFONT", id, "%s, %s %d", typeface, is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "", value);

	return 0;
}

static char* iFlatTabsGetTabFontSizeAttrib(Ihandle* ih, int id) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];

	char* font = iupAttribGetId(ih, "TABFONT", id);
	if (!font)
		font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnInt(size);
}

static char* iFlatTabsGetTabsFontSizeAttrib(Ihandle* ih) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];

	char* font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnInt(size);
}

static int iFlatTabsSetTabsFontSizeAttrib(Ihandle* ih, const char* value) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttribute(ih, "TABSFONT", "%s, %s%s%s%s %s", typeface, is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "", value);
	return 0;
}

static char* iFlatTabsGetTabsFontStyleAttrib(Ihandle* ih) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];

	char* font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnStrf("%s%s%s%s", is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "");
}

static int iFlatTabsSetTabsFontStyleAttrib(Ihandle* ih, const char* value) {
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	font = iupAttribGet(ih, "TABSFONT");
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttribute(ih, "TABSFONT", "%s, %s %d", typeface, value, size);

	return 0;
}

static int iFlatTabsSetTabsFontAttrib(Ihandle* ih, const char* value) {
	iupdrvSetFontAttrib(ih, value);
	return 1;
}

static int iFlatTabsSetExpandButtonAttrib(Ihandle* ih, const char* value) {
	if (iupStrBoolean(value) && !iupAttribGetBoolean(ih, "EXPANDBUTTON")) {
		int extra_buttons = iupAttribGetInt(ih, "EXTRABUTTONS");
		extra_buttons++;
		iupAttribSetInt(ih, "EXTRABUTTONS", extra_buttons);
		iupAttribSetInt(ih, "EXPANDBUTTONPOS", extra_buttons);

		iupAttribSetId(ih, "EXTRAIMAGE", extra_buttons, "IupFlatExpandUp");
		iupAttribSetId(ih, "EXTRAALIGNMENT", extra_buttons, "ACENTER:ABOTTOM");
	}

	return 1;
}

static int iFlatTabsSetExpandButtonStateAttrib(Ihandle* ih, const char* value) {
	if (iupAttribGetBoolean(ih, "EXPANDBUTTON")) {
		int expand_state = iupAttribGetBoolean(ih, "EXPANDBUTTONSTATE");
		if (iupStrBoolean(value)) {
			if (!expand_state)
				iFlatTabsToggleExpand(ih);
		} else {
			if (expand_state)
				iFlatTabsToggleExpand(ih);
		}
	}
	return 1;
}


/*********************************************************************************/

#define ATTRIB_ID_COUNT 6
const static char* attrib_id[ATTRIB_ID_COUNT] = {
  "TABTITLE",
  "TABIMAGE",
  "TABFORECOLOR",
  "TABBACKCOLOR",
  "TABHIGHCOLOR",
  "TABFONT"
};

static void iFlatTabsComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand) {
	*w = 0;
	*h = iFlatTabsGetTitleHeight(ih, NULL, 0);

	if (iupAttribGetBoolean(ih, "SHOWLINES")) {
		*h += 1;
		*w += 2;
	}
}

static int iFlatTabsCreateMethod(Ihandle* ih, void **params) {
	/* add children */
	if (params) {
		Ihandle** iparams = (Ihandle**)params;
		while (*iparams) {
			IupAppend(ih, *iparams);
			iparams++;
		}
	}
	int w, h;
	int vert_padding, horiz_padding;
	iupAttribGetIntInt(ih, "TABSPADDING", &horiz_padding, &vert_padding, 'x');

	iupDrawGetTextSize(ih, "AaBbCc", &w, &h);
	char msz[10];
	msz[0] = 'x';
	h += 6;
	_itoa(h, msz + 1, 10);

	iupAttribSet(ih, "MAXSIZE", msz);
	iupAttribSetInt(ih, "_IUPFTABS_HIGHLIGHTED", ITABS_NONE);
	iupAttribSetInt(ih, "_IUPFTABS_CLOSEHIGH", ITABS_NONE);
	iupAttribSetInt(ih, "_IUPFTABS_CLOSEPRESS", ITABS_NONE);
	iupAttribSetInt(ih, "_IUPFTABS_EXTRAPRESS", ITABS_NONE);

	IupSetCallback(ih, "ACTION", (Icallback)iFlatTabsRedraw_CB);
	IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatTabsButton_CB);
	IupSetCallback(ih, "MOTION_CB", (Icallback)iFlatTabsMotion_CB);
	IupSetCallback(ih, "LEAVEWINDOW_CB", (Icallback)iFlatTabsLeaveWindow_CB);
	IupSetCallback(ih, "RESIZE_CB", (Icallback)iFlatTabsResize_CB);
	ih->data->dragTab = ITABS_NONE;
	ih->data->xStart = -1;
	ih->data->yStart = -1;
	ih->data->start = 0;
	ih->data->xFreeMax = 0;
	ih->data->xFree = 0;
	return IUP_NOERROR;
}

void IupFlattabsCtrlOpen(void) {
	static int run = 0;
	if (run) return;
	run = 1;
	iupRegisterClass(iupFlattabsCtrlNewClass());
}

Iclass* iupFlattabsCtrlNewClass(void) {
	Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

	ic->name = "flattabs_ctrl";
	ic->format = NULL;
	ic->nativetype = IUP_TYPECANVAS;

	ic->childtype = IUP_CHILDNONE;
	ic->is_interactive = 1;
	ic->has_attrib_id = 1;

	/* Class functions */
	ic->New = iupFlattabsCtrlNewClass;
	ic->Create = iFlatTabsCreateMethod;

	ic->ComputeNaturalSize = iFlatTabsComputeNaturalSizeMethod;

	/* IupFlatTabs Callbacks */
	iupClassRegisterCallback(ic, "TABCHANGEPOS_CB", "ii");
	iupClassRegisterCallback(ic, "RIGHTCLICK_CB", "i");
	iupClassRegisterCallback(ic, "TABCLOSE_CB", "i");
	iupClassRegisterCallback(ic, "EXTRABUTTON_CB", "ii");

	iupClassRegisterCallback(ic, "TAB_BUTTON_CB", "niiiiiis");
	iupClassRegisterCallback(ic, "TAB_MOTION_CB", "niiiiis");
	iupClassRegisterCallback(ic, "TAB_SHIFT_CB", "ii");
	iupClassRegisterCallback(ic, "FLAT_LEAVEWINDOW_CB", "");

	/* Base Container */
	iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLIENTSIZE", iFlatTabsGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	/* Native Container */
	iupClassRegisterAttribute(ic, "CHILDOFFSET", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	/* replace IupCanvas behavior */
	iupClassRegisterReplaceAttribDef(ic, "BORDER", "NO", NULL);
	iupClassRegisterReplaceAttribFlags(ic, "BORDER", IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterReplaceAttribFlags(ic, "SCROLLBAR", IUPAF_READONLY | IUPAF_NO_INHERIT);

	iupClassRegisterReplaceAttribFunc(ic, "ACTIVE", NULL, iFlatTabsSetActiveAttrib);
	iupClassRegisterAttribute(ic, "TIP", NULL, iFlatTabsSetTipAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

	/* IupFlatTabs only */
	iupClassRegisterAttribute(ic, "VALUE", iFlatTabsGetValueAttrib, iFlatTabsSetValueAttrib, NULL, NULL, IUPAF_NO_STRING | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "VALUEPOS", iFlatTabsGetValuePosAttrib, iFlatTabsSetValuePosAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "COUNT", (IattribGetFunc)iFlatTabsGetCountAttrib, NULL, NULL, NULL, IUPAF_NO_STRING | IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "FIXEDWIDTH", NULL, iFlatTabsUpdateSetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABCHANGEONCHECK", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "REDRAW", NULL, iupFlatTabsDrawSetRedrawAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "LASTVISIBLE", (IattribGetFunc)iFlatTabsGetLastVisibleAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);

	/* IupFlatTabs Child only */
	iupClassRegisterAttributeId(ic, "TABTITLE", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABIMAGE", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABFORECOLOR", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABBACKCOLOR", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABBACKCOLORHUE", NULL, iFlatTabsSetHueBackColor, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABHIGHCOLOR", NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABFONT", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABTIP", NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABBUFFERID", NULL, NULL, IUPAF_NO_INHERIT);

	iupClassRegisterAttributeId(ic, "TABFONTSTYLE", iFlatTabsGetTabFontStyleAttrib, iFlatTabsSetTabFontStyleAttrib, IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TABFONTSIZE", iFlatTabsGetTabFontSizeAttrib, iFlatTabsSetTabFontSizeAttrib, IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	/* Visual for current TAB */
	iupClassRegisterAttribute(ic, "BGCOLORMOVIED", iFlatTabsGetBgColorAttrib, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "255 255 255", IUPAF_DEFAULT);   /* inherited */
	iupClassRegisterAttribute(ic, "BGCOLOR", iFlatTabsGetBgColorAttrib, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "255 255 255", IUPAF_DEFAULT);   /* inherited */
	iupClassRegisterAttribute(ic, "FORECOLOR", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "HIGHCOLOR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

	/* Visual for the other TABS */
	iupClassRegisterAttribute(ic, "TABSFORECOLOR", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSBACKCOLOR", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSHIGHCOLOR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "ILLUMINATION", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "SATURATION", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

	/* Visual for all TABS */
	iupClassRegisterAttribute(ic, "TABSFONT", NULL, iFlatTabsSetTabsFontAttrib, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSFONTSTYLE", iFlatTabsGetTabsFontStyleAttrib, iFlatTabsSetTabsFontStyleAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSFONTSIZE", iFlatTabsGetTabsFontSizeAttrib, iFlatTabsSetTabsFontSizeAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "SHOWLINES", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSLINECOLOR", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "180 180 180", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSIMAGEPOSITION", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "LEFT", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSIMAGESPACING", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "2", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSALIGNMENT", NULL, iFlatTabsUpdateSetAttrib, "ACENTER:ACENTER", NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSPADDING", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "10x10", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TABSTEXTALIGNMENT", NULL, NULL, IUPAF_SAMEASSYSTEM, "ALEFT", IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "SHOWCLOSE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLOSEIMAGE", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "IMGFLATCLOSE", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLOSEIMAGEPRESS", NULL, iFlatTabsUpdateSetAttrib, IUPAF_SAMEASSYSTEM, "IMGFLATCLOSEPRESS", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLOSEIMAGEHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLOSEIMAGEINACTIVE", NULL, iFlatTabsUpdateSetAttrib, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLOSEPRESSCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "80 180 245", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "CLOSEHIGHCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 220 245", IUPAF_NO_INHERIT);

	/* Extra Buttons */
	iupClassRegisterAttribute(ic, "EXTRABUTTONS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRATITLE", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAACTIVE", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAFORECOLOR", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAPRESSCOLOR", NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAHIGHCOLOR", NULL, NULL, IUPAF_NO_INHERIT); 
	iupClassRegisterAttributeId(ic, "EXTRAFONT", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAALIGNMENT", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAIMAGE", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAIMAGEPRESS", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAIMAGEHIGHLIGHT", NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRAIMAGEINACTIVE", NULL, (IattribSetIdFunc)iFlatTabsUpdateSetAttrib, IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "EXTRATIP", NULL, NULL, IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "EXPANDBUTTON", NULL, iFlatTabsSetExpandButtonAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "EXPANDBUTTONPOS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "EXPANDBUTTONSTATE", NULL, iFlatTabsSetExpandButtonStateAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);


	/* Default node images */
	if (!IupGetHandle("IMGFLATCLOSE"))
		iFlatTabsInitializeImages();

	return ic;
}

Ihandle* IupFlattabsCtrl(void) {
	return IupCreatev("flattabs_ctrl", NULL);// (void**)children);
}

static int FlattabsCtrl(lua_State *L) {
	Ihandle *ih = IupFlattabsCtrl();//iuplua_checkihandleornil(L, 1)
	iuplua_plugstate(L, ih);
	iuplua_pushihandle_raw(L, ih);
	return 1;
}

int iupFlattabsCtrllua_open(lua_State * L) {
	static int run = 0;	  	  // if (!run)
	iuplua_register(L, FlattabsCtrl, "FlattabsCtrl");
	iuplua_register_cb(L, "TAB_BUTTON_CB", (lua_CFunction)flattab_tab_button_cb, NULL);
	iuplua_register_cb(L, "TAB_MOTION_CB", (lua_CFunction)flattab_tab_motion_cb, NULL);
	iuplua_dostring(L,
		"local ctrl = {  nick = 'flattabs_ctrl',  parent = iup.BOX,  subdir = 'elem',  creation = 'I',  funcname = 'FlattabsCtrl', };function ctrl.createElement(class, param)  return iup.FlattabsCtrl() end; iup.RegisterWidget(ctrl); iup.SetClass(ctrl, 'iupWidget')",
		"flattabs_ctrl.lua");
	run = 1;
	return 0;
}
