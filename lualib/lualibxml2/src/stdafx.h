// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdlib.h>
#undef near
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}



// TODO: reference additional headers your program requires here
