#ifndef _PSF_H
#define _PSF_H



int_fast8_t init_psf();




long PSF_makeChromatPSF(const char *amp_name, const char *pha_name, float coeff1, float coeff2, long NBstep, float ApoCoeff, const char *out_name);

int PSF_finddiskcent(const char *ID_name, float rad, float *result);

int PSF_finddiskcent_alone(const char *ID_name, float rad);

int PSF_measurePhotocenter(const char *ID_name);

float measure_enc_NRJ(const char *ID_name, float xcenter, float ycenter, float fraction);

int measure_enc_NRJ1(const char *ID_name, float xcenter, float ycenter, const char *filename);

float measure_FWHM(const char *ID_name, float xcenter, float ycenter, float step, long nb_step);

int center_PSF(const char *ID_name, double *xcenter, double *ycenter, long box_size);

int fast_center_PSF(const char *ID_name, double *xcenter, double *ycenter, long box_size);

int center_PSF_alone(const char *ID_name);

int center_star(const char *ID_in_name, double *x_star, double *y_star);

float get_sigma(const char *ID_name, float x, float y, const char *options);

float get_sigma_alone(const char *ID_name);

int extract_psf(const char *ID_name, const char *out_name, long size);

long extract_psf_photcent(const char *ID_name, const char *out_name, long size);

int psf_variance(const char *ID_out_m, const char *ID_out_v, const char *options);

int combine_2psf(const char *ID_name, const char *ID_name1, const char *ID_name2, float radius, float index);

float psf_measure_SR(const char *ID_name, float factor, float r1, float r2);

long PSF_coaddbest(const char *IDcin_name, const char *IDout_name, float r_pix);

int PSF_sequence_measure(const char *IDin_name, float PSFsizeEst, const char *outfname);

#endif
