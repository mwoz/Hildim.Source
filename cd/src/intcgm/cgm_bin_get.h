#ifndef _CGM_BIN_GET_H_
#define _CGM_BIN_GET_H_

#ifdef __cplusplus
extern "C" {
#endif

int cgm_bin_get_ci(tCGM* cgm, unsigned long *);  /* color index */
int cgm_bin_get_cd(tCGM* cgm, unsigned long *, unsigned long *, unsigned long *);  /* color direct (red, green, blue) */
int cgm_bin_get_e(tCGM* cgm, short *);  /* enumerate */
int cgm_bin_get_i(tCGM* cgm, long *);  /* integer */
int cgm_bin_get_r(tCGM* cgm, double *);  /* real */
int cgm_bin_get_s(tCGM* cgm, char **);  /* string */
int cgm_bin_get_vdc(tCGM* cgm, double *);  /* virtual device coordinate */
int cgm_bin_get_p(tCGM* cgm, double *, double *);  /* (vdc, vdc) */
int cgm_bin_get_co(tCGM* cgm, tColor *);  /* cd or ci */
int cgm_bin_get_rf(tCGM* cgm, double *);  /* real as floating point always */

int cgm_bin_get_ix(tCGM* cgm, long *);  /* index */
int cgm_bin_get_pixel(tCGM* cgm, tColor *, int);  /* clist pixel, co using local precision */
int cgm_bin_get_c(tCGM* cgm, unsigned char *);  /* single byte */

typedef int(*cgmGetData)(tCGM* cgm, void *);
cgmGetData cgm_bin_get_samplefunc(long sample_type);


#ifdef __cplusplus
}
#endif

#endif
