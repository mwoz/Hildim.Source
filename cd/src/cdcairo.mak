# Windows Only

PROJNAME = cd
LIBNAME = cdcairo
OPT = YES

DEPENDDIR = dep

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRCCAIRO = cdcairodbuf.c cdcairopdf.c cdcairosvg.c cdcairo.c \
             cdcairoimg.c cdcairoirgb.c cdcairops.c
  SRCCAIRO := $(addprefix cairo/, $(SRCCAIRO))
  
  SRC = $(SRCCAIRO) 

  SRC += cairo/cdcaironative_win32.c cairo/cdcairoprn_win32.c cairo/cdcairoemf.c

  INCLUDES = . cairo
  LIBS = pangocairo-1.0 cairo pango-1.0 glib-2.0 gobject-2.0
  
  VCPKG_BASE = /lng/vcpkg/installed
  ifneq ($(findstring _64, $(TEC_UNAME)), )
    VCPKG = $(VCPKG_BASE)/x64-windows
  else
    VCPKG = $(VCPKG_BASE)/x86-windows
  endif
  
  INCLUDES += $(VCPKG)/include
  LDIR = $(VCPKG)/lib
endif

USE_CD = YES
CD = ..
