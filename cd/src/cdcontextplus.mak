PROJNAME = cd
LIBNAME = cdcontextplus
OPT = YES

DEPENDDIR = dep

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRCDIR = gdiplus
  SRC = cdwemfp.cpp cdwimgp.cpp cdwinp.cpp cdwnativep.cpp cdwprnp.cpp cdwdbufp.cpp cdwclpp.cpp cdwgdiplus.c

  INCLUDES = . gdiplus
  LIBS = gdiplus
  CHECK_GDIPLUS = Yes
else
  INCLUDES = . sim
  USE_FREETYPE = Yes
  
  ifdef GTK_DEFAULT
    ifdef USE_GTK3
      DEFINES += USE_GTK3
    endif
    CHECK_GTK = Yes
    SRC = cairo/cdcairoplus.c 
  else
    CHECK_XRENDER = Yes
    SRC = xrender/cdxrender.c xrender/cdxrplus.c
    LIBS = Xrender Xft
    USE_X11 = Yes
    INCLUDES += x11
  endif
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

USE_CD = YES
CD = ..
