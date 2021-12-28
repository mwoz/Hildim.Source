PROJNAME = cd
LIBNAME = cdim
OPT = YES

SRC = drv/cdim.c

INCLUDES = .

USE_IM = YES
USE_CD = YES
CD = ..
DEPENDDIR = dep

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
