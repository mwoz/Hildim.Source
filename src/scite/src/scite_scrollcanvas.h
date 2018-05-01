/** \file
 * \brief Canvas Controls Private Declarations
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_CANVAS_H 
#define __IUP_CANVAS_H

#ifdef __cplusplus
extern "C" {
#endif
 
void IupScrollCanvasOpen(void);
Iclass* iupScrollCanvasNewClass(void);

int iupIupScrollCanvaslua_open(lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
