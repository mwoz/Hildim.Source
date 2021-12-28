#include <stdlib.h>
#include <string.h>

#include "cgm_types.h"
#include "cgm_bin_get.h"


static int bin_get_bit(tCGM* cgm, unsigned char *b)
{
  if(cgm->buff.pc==0 || cgm->buff.pc==8)
  {
    *b = cgm->buff.data[cgm->buff.bc];
    cgm->buff.bc++;

    if(cgm->buff.bc > cgm->buff.len)
      return CGM_ERR_READ;

    cgm->buff.pc=0;
  }
  else
    *b = cgm->buff.data[cgm->buff.bc-1];

  switch(cgm->buff.pc)
  {
  case 0:
    *b = (*b | 0x0080) >> 7;
    break;
  case 1:
    *b = (*b | 0x0040) >> 6;
    break;
  case 2:
    *b = (*b | 0x0020) >> 5;
    break;
  case 3:
    *b = (*b | 0x0010) >> 4;
    break;
  case 4:
    *b = (*b | 0x0008) >> 3;
    break;
  case 5:
    *b = (*b | 0x0004) >> 2;
    break;
  case 6:
    *b = (*b | 0x0002) >> 1;
    break;
  case 7:
    *b = (*b | 0x0001);
    break;
  default:
    return CGM_ERR_READ;
  }

  cgm->buff.pc++;

  return CGM_OK;
}

static int bin_get_2bit(tCGM* cgm, unsigned char *b)
{
  if(cgm->buff.pc==0 || cgm->buff.pc==8)
  {
    *b = cgm->buff.data[cgm->buff.bc];
    cgm->buff.bc++;

    if(cgm->buff.bc > cgm->buff.len) 
      return CGM_ERR_READ;

    cgm->buff.pc=0;
  }
  else
    *b = cgm->buff.data[cgm->buff.bc-1];

  switch(cgm->buff.pc)
  {
  case 0:
    *b = (*b | 0x00C0) >> 6;
    break;
  case 2:
    *b = (*b | 0x0030) >> 4;
    break;
  case 4:
    *b = (*b | 0x000C) >> 2;
    break;
  case 6:
    *b = (*b | 0x0003);
    break;
  default:
    return CGM_ERR_READ;
  }

  cgm->buff.pc += 2;

  return CGM_OK;
}


static int bin_get_4bit(tCGM* cgm, unsigned char *b)
{
  if(cgm->buff.pc==0 || cgm->buff.pc==8)
  {
    *b = cgm->buff.data[cgm->buff.bc];
    cgm->buff.bc++;

    if(cgm->buff.bc > cgm->buff.len) 
      return CGM_ERR_READ;

    cgm->buff.pc=0;
  }
  else
    *b = cgm->buff.data[cgm->buff.bc-1];

  switch(cgm->buff.pc)
  {
  case 0:
    *b =(*b | 0x00F0) >> 4;
    break;
  case 4:
    *b =(*b | 0x000F);
    break;
  default:
    return CGM_ERR_READ;
  }

  cgm->buff.pc += 4;

  return CGM_OK;
}

int cgm_bin_get_c(tCGM* cgm, unsigned char *b)
{
  *b = cgm->buff.data[cgm->buff.bc];
  cgm->buff.bc++;

  if(cgm->buff.bc > cgm->buff.len) 
    return CGM_ERR_READ;

  return CGM_OK;   
}

static int cgm_bin_get_i8(tCGM* cgm, signed char *b)
{
  unsigned char b1;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;

  *b =(signed char) b1;

  return CGM_OK;
}

static int cgm_bin_get_i16(tCGM* cgm, short *b)
{
  unsigned char b1, b2;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;

  *b =(b1<<8) | b2;

  return CGM_OK;
}

static int cgm_bin_get_i24(tCGM* cgm, long *b)
{
  unsigned char b1, b2, b3;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b3)) 
    return CGM_ERR_READ;

  *b =(b1<<16) |(b2<<8) | b3;

  return CGM_OK;
}

static int cgm_bin_get_i32(tCGM* cgm, long *b)
{
  unsigned char b1, b2, b3, b4;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b3)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b4)) 
    return CGM_ERR_READ;

  *b =(b1<<24) |(b2<<16) |(b3<<8) | b4;

  return CGM_OK;
}

static int cgm_bin_get_u8(tCGM* cgm, unsigned char *b)
{
  if(cgm_bin_get_c(cgm, b)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

static int cgm_bin_get_u16(tCGM* cgm, unsigned short *b)
{
  unsigned char b1, b2;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;

  *b =(b1<<8) | b2;

  return CGM_OK;
}

static int cgm_bin_get_u24(tCGM* cgm, unsigned long *b)
{
  unsigned char b1, b2, b3;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b3)) 
    return CGM_ERR_READ;

  *b =(b1<<16) |(b2<<8) | b3;

  return CGM_OK;
}

static int cgm_bin_get_u32(tCGM* cgm, unsigned long *b)
{
  unsigned char b1, b2, b3, b4;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b3)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b4)) 
    return CGM_ERR_READ;

  *b =(b1<<24) |(b2<<16) |(b3<<8) | b4;

  return CGM_OK;
}

static int cgm_bin_get_fl32(tCGM* cgm, float *b)
{
  unsigned char b1, b2, b3, b4;
  union {
    float f;
    long  l;
  } r;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b3)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b4)) 
    return CGM_ERR_READ;

  r.l =(b1<<24) |(b2<<16) |(b3<<8) | b4;
  *b = r.f;

  return CGM_OK;
}

static int cgm_bin_get_fl64(tCGM* cgm, double *b)
{
  unsigned char b1, b2, b3, b4, b5, b6, b7, b8;
  union {
    double d;
    long   l[2];
  } r;

  if(cgm_bin_get_c(cgm, &b1)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b2)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b3)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b4)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b5)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b6)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b7)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_c(cgm, &b8)) 
    return CGM_ERR_READ;

  r.l[1] =(b1<<24) |(b2<<16) |(b3<<8) | b4;
  r.l[0] =(b5<<24) |(b6<<16) |(b7<<8) | b8;
  *b = r.d;

  return CGM_OK;
}

static int cgm_bin_get_fx32(tCGM* cgm, float *b)
{
  short si;
  unsigned short ui;

  if(cgm_bin_get_i16(cgm, &si)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_u16(cgm, &ui)) 
    return CGM_ERR_READ;

  *b =(float)(si +(ui / 65536.0));

  return CGM_OK;
}

static int cgm_bin_get_fx64(tCGM* cgm, double *b)
{
  long si, ui;

  if(cgm_bin_get_i32(cgm, &si)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_i32(cgm, &ui)) 
    return CGM_ERR_READ;

  *b = si +((unsigned short) ui /(65536.0 * 65536.0));

  return CGM_OK;
}

cgmGetData cgm_bin_get_samplefunc(long sample_type)
{
  cgmGetData getdata = NULL;

  switch(sample_type)
  {
  case 0:
    getdata =(cgmGetData) cgm_bin_get_i16;
    break;
  case 1:
    getdata =(cgmGetData) cgm_bin_get_i32;
    break;
  case 2:
    getdata =(cgmGetData) cgm_bin_get_fl32;
    break;
  case 3:
    getdata =(cgmGetData) cgm_bin_get_i8;
    break;
  case 4:
    getdata =(cgmGetData) cgm_bin_get_i16;
    break;
  case 5:
    getdata =(cgmGetData) cgm_bin_get_i8;
    break;
  }
  return getdata;
}

int cgm_bin_get_ci(tCGM* cgm, unsigned long *ci)
{
  unsigned char c;
  unsigned short i;

  switch(cgm->cix_prec)
  {
  case 0: 
    if(cgm_bin_get_u8 (cgm, &c)) 
      return CGM_ERR_READ; 
    *ci =(unsigned long) c;
    break;
  case 1: 
    if(cgm_bin_get_u16(cgm, &i)) 
      return CGM_ERR_READ;
    *ci =(unsigned long) i;
    break;
  case 2: 
    if(cgm_bin_get_u24(cgm, ci)) 
      return CGM_ERR_READ;
    break;
  case 3: 
    if(cgm_bin_get_u32(cgm, ci)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int bin_get_cd(tCGM* cgm, unsigned long *cd)
{
  unsigned char c;
  unsigned short i;

  switch(cgm->cd_prec)
  {
  case 0: 
    if(cgm_bin_get_u8 (cgm, &c)) 
      return CGM_ERR_READ;
    *cd =(unsigned long) c;
    break;
  case 1: 
    if(cgm_bin_get_u16(cgm, &i)) 
      return CGM_ERR_READ;
    *cd =(unsigned long) i;
    break;
  case 2: 
    if(cgm_bin_get_u24(cgm, cd)) 
      return CGM_ERR_READ;
    break;
  case 3: 
    if(cgm_bin_get_u32(cgm, cd)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

int cgm_bin_get_cd(tCGM* cgm, unsigned long *r, unsigned long *g, unsigned long *b)
{
  if(bin_get_cd(cgm, r)) 
    return CGM_ERR_READ;
  if(bin_get_cd(cgm, g)) 
    return CGM_ERR_READ;
  if(bin_get_cd(cgm, b)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

int cgm_bin_get_ix(tCGM* cgm, long *ix)
{
  signed char c;
  short i;

  switch(cgm->ix_prec.b_prec)
  {
  case 0: 
    if(cgm_bin_get_i8 (cgm, &c)) 
      return CGM_ERR_READ;
    *ix =(long) c;
    break;
  case 1: 
    if(cgm_bin_get_i16(cgm, &i)) 
      return CGM_ERR_READ;
    *ix =(long) i;
    break;
  case 2: 
    if(cgm_bin_get_i24(cgm, ix)) 
      return CGM_ERR_READ;
    break;
  case 3: 
    if(cgm_bin_get_i32(cgm, ix)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

int cgm_bin_get_e(tCGM* cgm, short *e)
{
  return cgm_bin_get_i16(cgm, e);
}

int cgm_bin_get_i(tCGM* cgm, long *li)
{
  signed char c;
  short i;

  switch(cgm->int_prec.b_prec)
  {
  case 0: 
    if(cgm_bin_get_i8 (cgm, &c)) 
      return CGM_ERR_READ;
    *li =(long) c;
    break;
  case 1: 
    if(cgm_bin_get_i16(cgm, &i)) 
      return CGM_ERR_READ;
    *li =(long) i;
    break;
  case 2: 
    if(cgm_bin_get_i24(cgm, li)) 
      return CGM_ERR_READ;
    break;
  case 3: 
    if(cgm_bin_get_i32(cgm, li)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

#ifdef UNUSED
int cgm_bin_get_u(tCGM* cgm, unsigned long *ui)
{
  unsigned char c;
  unsigned short i;

  switch(cgm->int_prec.b_prec)
  {
  case 0: 
    if(cgm_bin_get_u8 (cgm, &c)) 
      return CGM_ERR_READ;
    *ui =(unsigned long) c;
    break;
  case 1: 
    if(cgm_bin_get_u16(cgm, &i)) 
      return CGM_ERR_READ;
    *ui =(unsigned long) i;
    break;
  case 2: 
    if(cgm_bin_get_u24(cgm, ui)) 
      return CGM_ERR_READ;
    break;
  case 3: 
    if(cgm_bin_get_u32(cgm, ui)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}
#endif

int cgm_bin_get_rf(tCGM* cgm, double *d)
{
  /* Special case were it is always encoded as floating point
     regardless of the value of the fixed/floating flag. */
  if(cgm->real_prec.b_prec==1)
  {
    if(cgm_bin_get_fl64(cgm, d) ) 
      return CGM_ERR_READ;
  }
  else
  {
    float f;
    if (cgm_bin_get_fl32(cgm, &f)) 
      return CGM_ERR_READ;
    if(f<1e-20) f = 1;
    *d = f;
  }

  return CGM_OK;
}

int cgm_bin_get_r(tCGM* cgm, double *d)
{
  float f;

  switch(cgm->real_prec.b_prec)
  {
  case 0: 
    if(cgm_bin_get_fl32(cgm, &f)) 
      return CGM_ERR_READ;
    *d =(double) f;
    break;
  case 1: 
    if(cgm_bin_get_fl64(cgm, d)) 
      return CGM_ERR_READ;
    break;
  case 2: 
    if(cgm_bin_get_fx32(cgm, &f)) 
      return CGM_ERR_READ;
    *d =(double) f;
    break;
  case 3: 
    if(cgm_bin_get_fx64(cgm, d)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

int cgm_bin_get_s(tCGM* cgm, char **str)
{
  register unsigned i = 0;
  unsigned char l;
  unsigned short l1;
  unsigned short cont;
  char *s = NULL;

  cont = 1;

  if(cgm_bin_get_u8(cgm, &l)) 
    return CGM_ERR_READ;

  l1 = l;

  while(cont)
  {
    if(l > 254)
    {
      if(cgm_bin_get_u16(cgm, &l1)) 
        return CGM_ERR_READ;
      cont =(l1 & 0x8000);
      l1 &= 0x7fff;
    }
    else
      cont = 0;

    s =(char *)realloc((unsigned char *)s,(sizeof(char) * l1) + 1);

    for(i=0; i<l1; i++)
    {
      unsigned char k;
      if(cgm_bin_get_c(cgm, &k)) 
      {
        free(s);
        return CGM_ERR_READ;
      }
      s[i] =(char) k;
    }

  }
  s[i] = '\0';

  *str = s;

  return CGM_OK;
}

int cgm_bin_get_vdc(tCGM* cgm, double *vdc)
{
  signed char c;
  short i;
  long l;
  float f;

  if(cgm->vdc_type == CGM_INTEGER)
  {
    switch(cgm->vdc_int.b_prec)
    {
    case 0: 
      if(cgm_bin_get_i8 (cgm, &c)) 
        return CGM_ERR_READ;
      *vdc =(double) c;
      break;
    case 1: 
      if(cgm_bin_get_i16(cgm,  &i)) 
        return CGM_ERR_READ;
      *vdc =(double) i;
      break;
    case 2: 
      if(cgm_bin_get_i24(cgm, &l)) 
        return CGM_ERR_READ;
      *vdc =(double) l;
      break;
    case 3: 
      if(cgm_bin_get_i32(cgm, &l)) 
        return CGM_ERR_READ;
      *vdc =(double) l;
      break;
    default:
      return CGM_ERR_READ;
    }
  }
  else
  {
    switch(cgm->vdc_real.b_prec)
    {
    case 0: 
      if(cgm_bin_get_fl32(cgm, &f)) 
        return CGM_ERR_READ;
      *vdc =(double) f;
      break;
    case 1: 
      if(cgm_bin_get_fl64(cgm, vdc)) 
        return CGM_ERR_READ;
      break;
    case 2: 
      if(cgm_bin_get_fx32(cgm, &f)) 
        return CGM_ERR_READ;
      *vdc =(double) f;
      break;
    case 3: 
      if(cgm_bin_get_fx64(cgm, vdc)) 
        return CGM_ERR_READ;
      break;
    default:
      return CGM_ERR_READ;
    }
  }

  return CGM_OK;
}

int cgm_bin_get_p(tCGM* cgm, double *x, double *y)
{
  if(cgm_bin_get_vdc(cgm, x)) 
    return CGM_ERR_READ;
  if(cgm_bin_get_vdc(cgm, y)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

int cgm_bin_get_co(tCGM* cgm, tColor *co)
{
  if(cgm->color_mode == CGM_INDEXED) /* indexed */
  {
    if(cgm_bin_get_ci(cgm, &(co->index))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_bin_get_cd(cgm, &(co->rgb.red), &(co->rgb.green), &(co->rgb.blue))) 
      return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int bin_get_pixeli(tCGM* cgm, unsigned long *ci, int localp)
{
  unsigned char c;
  unsigned short i;

  if(localp==0)
  {
    if(cgm->cix_prec==0) localp = 8;
    else if(cgm->cix_prec==1) localp = 16;
    else if(cgm->cix_prec==2) localp = 24;
    else if(cgm->cix_prec==3) localp = 32;
  }

  switch(localp)
  {
  case 1: 
    if(bin_get_bit(cgm, &c)) 
      return CGM_ERR_READ;
    *ci =(unsigned long) c;
    break;
  case 2: 
    if(bin_get_2bit(cgm, &c))
      *ci =(unsigned long) c;
    return CGM_ERR_READ;
    break;
  case 4: 
    if(bin_get_4bit(cgm, &c))
      *ci =(unsigned long) c;
    return CGM_ERR_READ;
    break;
  case 8: 
    if(cgm_bin_get_u8 (cgm, &c)) 
      return CGM_ERR_READ; 
    *ci =(unsigned long) c;
    break;
  case 16: 
    if(cgm_bin_get_u16(cgm, &i)) 
      return CGM_ERR_READ;
    *ci =(unsigned long) i;
    break;
  case 24: 
    if(cgm_bin_get_u24(cgm, ci)) 
      return CGM_ERR_READ;
    break;
  case 32: 
    if(cgm_bin_get_u32(cgm, ci)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int bin_get_pixeld(tCGM* cgm, unsigned long *cd, int localp)
{
  unsigned char c;
  unsigned short i;

  if(localp==0)
  {
    if(cgm->cd_prec==0) localp = 8;
    else if(cgm->cd_prec==1) localp = 16;
    else if(cgm->cd_prec==2) localp = 24;
    else if(cgm->cd_prec==3) localp = 32;
  }

  switch(localp)
  {
  case  1: 
    if(bin_get_bit (cgm, &c)) 
      return CGM_ERR_READ;
    *cd =(unsigned long) c;
    break;
  case  2: 
    if(bin_get_2bit(cgm, &c)) 
      return CGM_ERR_READ;
    *cd =(unsigned long) c;
    break;
  case  4: 
    if(bin_get_4bit(cgm, &c))
      *cd =(unsigned long) c;

    return CGM_ERR_READ;
    break;
  case  8: 
    if(cgm_bin_get_u8 (cgm, &c)) 
      return CGM_ERR_READ;
    *cd =(unsigned long) c;
    break;
  case 16: 
    if(cgm_bin_get_u16(cgm, &i)) 
      return CGM_ERR_READ;
    *cd =(unsigned long) i;
    break;
  case 24: 
    if(cgm_bin_get_u24(cgm, cd)) 
      return CGM_ERR_READ;
    break;
  case 32: 
    if(cgm_bin_get_u32(cgm, cd)) 
      return CGM_ERR_READ;
    break;
  default:
    return CGM_ERR_READ;
  }

  return CGM_OK;
}

static int bin_get_pixelrgb(tCGM* cgm, unsigned long *r, unsigned long *g, unsigned long *b, int localp)
{
  if(bin_get_pixeld(cgm, r, localp)) 
    return CGM_ERR_READ;
  if(bin_get_pixeld(cgm, g, localp)) 
    return CGM_ERR_READ;
  if(bin_get_pixeld(cgm, b, localp)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

int cgm_bin_get_pixel(tCGM* cgm, tColor *co, int localp)
{
  if(cgm->color_mode == CGM_INDEXED) /* indexed */
  {
    if(bin_get_pixeli(cgm, &(co->index), localp)) 
      return CGM_ERR_READ;
  }
  else
  {
    if(bin_get_pixelrgb(cgm, &(co->rgb.red), &(co->rgb.green), &(co->rgb.blue), localp)) 
      return CGM_ERR_READ;
  }

  return CGM_OK;
}
