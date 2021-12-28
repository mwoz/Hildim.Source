PROJNAME = cd
LIBNAME = cdgl
OPT = YES

SRC = drv/cdgl.c

INCLUDES = . sim

USE_FTGL = Yes
USE_FREETYPE = Yes
USE_OPENGL = Yes
USE_CD = YES
CD = ..

DEPENDDIR = dep

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  ifeq ($(findstring dll, $(TEC_UNAME)), )
    DEFINES += FTGL_LIBRARY_STATIC
  endif
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  STDINCS = $(X11_INC)
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
  LDIR += $(GTK)/lib
endif
