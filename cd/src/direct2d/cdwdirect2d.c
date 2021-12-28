/** \file
 * \brief GDI+ Control
 *
 * See Copyright Notice in cd.h
 */
 
#include "cd.h"
#include "cd_private.h"
#include "cd_d2d.h"
#include <stdlib.h>
#include <memory.h>

void cdInitDirect2D(void)
{
  d2dStartup();
}

void cdFinishDirect2D(void)
{
  d2dShutdown();
}
