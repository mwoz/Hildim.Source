#ifndef _CGM_TXT_GET_H_
#define _CGM_TXT_GET_H_

#ifdef __cplusplus
extern "C" {
#endif

char* cgm_txt_get_sep(tCGM* cgm, char*);
void cgm_txt_skip_sep(tCGM* cgm);
void cgm_txt_skip_com(tCGM* cgm);
void cgm_txt_skip_parentheses(tCGM* cgm);
int cgm_txt_get_ter_noerr(tCGM* cgm);
int cgm_txt_get_ter(tCGM* cgm);

int cgm_txt_get_i(tCGM* cgm, long *);  /* integer */
int cgm_txt_get_ci(tCGM* cgm, unsigned long *);  /* color index */
int cgm_txt_get_cd(tCGM* cgm, unsigned long *, unsigned long *, unsigned long *);  /* color direct (red, green, blue) */
int cgm_txt_get_e(tCGM* cgm, short *, const char **);  /* enumerate */
int cgm_txt_get_r(tCGM* cgm, double *);  /* real */
int cgm_txt_get_s(tCGM* cgm, char **);  /* string */
int cgm_txt_get_vdc(tCGM* cgm, double *);   /* virtual device coordinate */
int cgm_txt_get_p(tCGM* cgm, double *, double *);  /* (vdc, vdc) */
int cgm_txt_get_co(tCGM* cgm, tColor *);  /* cd or ci */

#ifdef __cplusplus
}
#endif

#endif
