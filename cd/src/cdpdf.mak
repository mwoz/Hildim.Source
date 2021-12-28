PROJNAME = cd
LIBNAME = cdpdf
OPT = YES

SRC = drv/cdpdf.c

PDFLIB ?= $(TECTOOLS_HOME)/pdflib7

INCLUDES = . sim $(PDFLIB)/include
DEPENDDIR = dep
LINK_PDFLIB = Yes

USE_CD = YES
CD = ..

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
