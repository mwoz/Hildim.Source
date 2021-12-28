#include <stdlib.h>
#include <string.h>

#include "cgm_types.h"
#include "cgm_txt_get.h"


char* cgm_txt_get_sep(tCGM* cgm, char* chr)
{
  fscanf(cgm->fp, "%[ \r\n\t\v\f,]", chr);
  return chr;
}

void cgm_txt_skip_sep(tCGM* cgm)
{
  char chr[1024];
  cgm_txt_get_sep(cgm, chr);
}

void cgm_txt_skip_com(tCGM* cgm)
{
  char chr[1024], c;

  while((c =(char)fgetc(cgm->fp)) == '%')
  {
    fscanf(cgm->fp, "%[^%]%%", chr);
    cgm_txt_get_sep(cgm, chr);
  }

  ungetc(c, cgm->fp);
}

void cgm_txt_skip_parentheses(tCGM* cgm)
{
  char chr[1024];
  cgm_txt_get_sep(cgm, chr);
  fscanf(cgm->fp, "%[()]", chr);
}

int cgm_txt_get_ter_noerr(tCGM* cgm)
{
  char c = 0;

  cgm_txt_skip_com(cgm);

  cgm_txt_skip_sep(cgm);

  fscanf(cgm->fp, "%c", &c);

  if (c=='/' || c==';') 
    return 0;  /* found, stop while */

  ungetc(c, cgm->fp);

  return 1; /* not found, continue */
}

int cgm_txt_get_ter(tCGM* cgm)
{
  cgm_txt_get_ter_noerr(cgm);  /* ignore returned value */
  return CGM_OK;
}

int cgm_txt_get_i(tCGM* cgm, long *i)
{
  cgm_txt_skip_sep(cgm);

  cgm_txt_skip_com(cgm);

  if(fscanf(cgm->fp, "%ld", i)) 
    return CGM_OK;

  return CGM_ERR_READ;
}

int cgm_txt_get_ci(tCGM* cgm, unsigned long *ci)
{
  return cgm_txt_get_i(cgm,(long*)ci);
}

#define txt_get_cd cgm_txt_get_ci

int cgm_txt_get_cd(tCGM* cgm, unsigned long *r, unsigned long *g, unsigned long *b)
{
  if(txt_get_cd(cgm, r)) 
    return CGM_ERR_READ;

  if(txt_get_cd(cgm, g)) 
    return CGM_ERR_READ;

  if(txt_get_cd(cgm, b)) 
    return CGM_ERR_READ;

  return CGM_OK;
}

int cgm_txt_get_e(tCGM* cgm, short *e, const char **el)
{
  char chr[1024] = "";
  int i;
  char *pt;

  cgm_txt_skip_sep(cgm);

  cgm_txt_skip_com(cgm);

  fscanf(cgm->fp, "%[^ \r\n\t\v\f,/;%\"\']", chr);

  cgm_strupper(chr);

  pt = strtok(chr,"_$");
  while(pt)
  {
    pt = strtok(NULL, "_$");
    if (pt)
      strcat(chr, pt);
  }

  for(i=0; el[i]!=NULL; i++)
  {
    if(strcmp(chr, el[i]) == 0)
    {
      *e =(short)i;
      return CGM_OK;
    }
  }

  return CGM_ERR_READ;
}

int cgm_txt_get_r(tCGM* cgm, double *f)
{
  cgm_txt_skip_sep(cgm);

  cgm_txt_skip_com(cgm);

  if(fscanf(cgm->fp, "%lg", f)) 
    return CGM_OK;

  return CGM_ERR_READ;
}

int cgm_txt_get_s(tCGM* cgm, char **str)
{
  char c, delim;
  int block = 80;
  int i = 0, ok=1;

  cgm_txt_skip_sep(cgm);

  cgm_txt_skip_com(cgm);

  delim =(char)fgetc(cgm->fp);

  if(delim != '"' && delim != '\'') 
    return CGM_ERR_READ;

  *str =(char *) malloc(block*sizeof(char));
  strcpy(*str, "");

  do
  {
    c = (char)fgetc(cgm->fp);
    if (c == (char)EOF)
    {
      ok = 0;
      return feof(cgm->fp)? CGM_OK: CGM_ERR_READ;
    }

    if(c==delim)
    {
      c=(char)fgetc(cgm->fp);
      if (c == (char)EOF)
      {
        ok = 0;
        return feof(cgm->fp)? CGM_OK: CGM_ERR_READ;
      }

      if(c==delim)
        (*str)[i++] = c;
      else 
      {
        ungetc(c,cgm->fp);
        break;
      }
    }
    else
      (*str)[i++] = c;

    if((i+1)==block)
    {
      block *= 2;
      *str =(char *) realloc(*str, block*sizeof(char));
    }
  } while(ok);

  (*str)[i] = '\0';

  return CGM_OK;
}

int cgm_txt_get_vdc(tCGM* cgm, double *vdc)
{
  long l;

  if(cgm->vdc_type==CGM_INTEGER)
  {
    if(cgm_txt_get_i(cgm, &l)) 
      return CGM_ERR_READ;
    *vdc =(double) l;
    return CGM_OK;
  }
  else
    return cgm_txt_get_r(cgm, vdc);
}

int cgm_txt_get_p(tCGM* cgm, double *x, double *y)
{
  cgm_txt_skip_parentheses(cgm);

  if(cgm_txt_get_vdc(cgm, x)) 
    return CGM_ERR_READ;

  if(cgm_txt_get_vdc(cgm, y)) 
    return CGM_ERR_READ;

  cgm_txt_skip_parentheses(cgm);

  return CGM_OK;
}

int cgm_txt_get_co(tCGM* cgm, tColor *co)
{
  if(cgm->color_mode == CGM_INDEXED)
  {
    if(cgm_txt_get_ci(cgm, &(co->index))) 
      return CGM_ERR_READ;
  }
  else
  {
    if(cgm_txt_get_cd(cgm, &(co->rgb.red), &(co->rgb.green), &(co->rgb.blue))) 
      return CGM_ERR_READ;
  }

  return CGM_OK; 
}
