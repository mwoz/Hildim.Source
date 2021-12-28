PROJNAME = cd
LIBNAME = cddirect2d
OPT = YES

DEPENDDIR = dep

SRCDIR = direct2d
SRC = cd_d2d.c cd_d2d_draw.c cd_d2d_image.c cd_d2d_text.c cdwdirect2d.c cdwin_d2d.c cdwnative_d2d.c \
      cdwdbuf_d2d.c cdwimg_d2d.c cdwprn_d2d.c cdwimgrgb_d2d.c

INCLUDES = . direct2d

USE_CD = YES
CD = ..
