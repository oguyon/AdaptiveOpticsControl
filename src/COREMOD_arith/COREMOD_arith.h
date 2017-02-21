#ifndef _ARITH_H
#define _ARITH_H


int init_COREMOD_arith();



long arith_set_pixel(const char *ID_name, double value, long x, long y);
long arith_set_pixel_1Drange(const char *ID_name, double value, long x, long y);

long arith_set_row(const char *ID_name, double value, long y);

long arith_set_col(const char *ID_name, double value, long x);

long arith_image_zero(const char *ID_name);

int arith_image_crop(const char *ID_name, const char *ID_out, long *start, long *end, long cropdim);

int arith_image_extract2D(const char *in_name, const char *out_name, long size_x, long size_y, long xstart, long ystart);

int arith_image_extract3D(const char *in_name, const char *out_name, long size_x, long size_y, long size_z, long xstart, long ystart, long zstart);

long arith_image_merge3D(const char *ID_name1, const char *ID_name2, const char *IDout_name);

double arith_image_total(const char *ID_name);
double arith_image_mean(const char *ID_name);
double arith_image_min(const char *ID_name);
double arith_image_max(const char *ID_name);

double arith_image_percentile(const char *ID_name, double fraction);
double arith_image_median(const char *ID_name);

long arith_image_dx(const char *ID_name, const char *IDout_name);
long arith_image_dy(const char *ID_name, const char *IDout_name);



/* ------------------------------------------------------------------------- */
/* image  -> image                                                           */
/* ------------------------------------------------------------------------- */



int arith_image_acos_byID(long ID, long IDout);
int arith_image_asin_byID(long ID, long IDout);
int arith_image_atan_byID(long ID, long IDout);
int arith_image_ceil_byID(long ID_name, long IDout);
int arith_image_cos_byID(long ID, long IDout);
int arith_image_cosh_byID(long ID, long IDout);
int arith_image_exp_byID(long ID, long IDout);
int arith_image_fabs_byID(long ID, long IDout);
int arith_image_floor_byID(long ID, long IDout);
int arith_image_ln_byID(long ID, long IDout);
int arith_image_log_byID(long ID, long IDout);
int arith_image_sqrt_byID(long ID, long IDout);
int arith_image_sin_byID(long ID, long IDout);
int arith_image_sinh_byID(long ID, long IDout);
int arith_image_tan_byID(long ID, long IDout);
int arith_image_tanh_byID(long ID, long IDout);

int arith_image_acos(const char *ID_name, const char *ID_out);
int arith_image_asin(const char *ID_name, const char *ID_out);
int arith_image_atan(const char *ID_name, const char *ID_out);
int arith_image_ceil(const char *ID_name, const char *ID_out);
int arith_image_cos(const char *ID_name, const char *ID_out);
int arith_image_cosh(const char *ID_name, const char *ID_out);
int arith_image_exp(const char *ID_name, const char *ID_out);
int arith_image_fabs(const char *ID_name, const char *ID_out);
int arith_image_floor(const char *ID_name, const char *ID_out);
int arith_image_ln(const char *ID_name, const char *ID_out);
int arith_image_log(const char *ID_name, const char *ID_out);
int arith_image_sqrt(const char *ID_name, const char *ID_out);
int arith_image_sin(const char *ID_name, const char *ID_out);
int arith_image_sinh(const char *ID_name, const char *ID_out);
int arith_image_tan(const char *ID_name, const char *ID_out);
int arith_image_tanh(const char *ID_name, const char *ID_out);






int arith_image_acos_inplace_byID(long ID);
int arith_image_asin_inplace_byID(long ID);
int arith_image_atan_inplace_byID(long ID);
int arith_image_ceil_inplace_byID(long ID);
int arith_image_cos_inplace_byID(long ID);
int arith_image_cosh_inplace_byID(long ID);
int arith_image_exp_inplace_byID(long ID);
int arith_image_fabs_inplace_byID(long ID);
int arith_image_floor_inplace_byID(long ID);
int arith_image_ln_inplace_byID(long ID);
int arith_image_log_inplace_byID(long ID);
int arith_image_sqrt_inplace_byID(long ID);
int arith_image_sin_inplace_byID(long ID);
int arith_image_sinh_inplace_byID(long ID);
int arith_image_tan_inplace_byID(long ID);
int arith_image_tanh_inplace_byID(long ID);

int arith_image_acos_inplace(const char *ID_name);
int arith_image_asin_inplace(const char *ID_name);
int arith_image_atan_inplace(const char *ID_name);
int arith_image_ceil_inplace(const char *ID_name);
int arith_image_cos_inplace(const char *ID_name);
int arith_image_cosh_inplace(const char *ID_name);
int arith_image_exp_inplace(const char *ID_name);
int arith_image_fabs_inplace(const char *ID_name);
int arith_image_floor_inplace(const char *ID_name);
int arith_image_ln_inplace(const char *ID_name);
int arith_image_log_inplace(const char *ID_name);
int arith_image_sqrt_inplace(const char *ID_name);
int arith_image_sin_inplace(const char *ID_name);
int arith_image_sinh_inplace(const char *ID_name);
int arith_image_tan_inplace(const char *ID_name);
int arith_image_tanh_inplace(const char *ID_name);




/* Functions for bison / flex    */ 
double Ppositive(double a);
double Ptrunc(double a, double b, double c);
int arith_image_function_im_im__d_d(const char *ID_name, const char *ID_out, double (*pt2function)(double));
int arith_image_function_imd_im__dd_d(const char *ID_name, double v0, const char *ID_out, double (*pt2function)(double, double));
int arith_image_function_imdd_im__ddd_d(const char *ID_name, double v0, double v1, const char *ID_out, double (*pt2function)(double, double, double));


/* ------------------------------------------------------------------------- */
/* predefined functions    image, image  -> image                                                    */
/* ------------------------------------------------------------------------- */


int arith_image_fmod_byID(long ID1, long ID2, long IDout);
int arith_image_pow_byID(long ID1, long ID2, const char *IDout);
int arith_image_add_byID(long ID1, long ID2, long IDout);
int arith_image_sub_byID(long ID1, long ID2, long IDout);
int arith_image_mult_byID(long ID1, long ID2, long IDout);
int arith_image_div_byID(long ID1, long ID2, long IDout);
int arith_image_minv_byID(long ID1, long ID2, long IDout);
int arith_image_maxv_byID(long ID1, long ID2, long IDout);

int arith_image_fmod(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_pow(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_add(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_sub(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_mult(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_div(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_minv(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_maxv(const char *ID1_name, const char *ID2_name, const char *ID_out);




int arith_image_fmod_inplace_byID(long ID1, long ID2);
int arith_image_pow_inplace_byID(long ID1, long ID2);
int arith_image_add_inplace_byID(long ID1, long ID2);
int arith_image_sub_inplace_byID(long ID1, long ID2);
int arith_image_mult_inplace_byID(long ID1, long ID2);
int arith_image_div_inplace_byID(long ID1,  long ID2);
int arith_image_minv_inplace_byID(long ID1, long ID2);
int arith_image_maxv_inplace_byID(long ID1, long ID2);

int arith_image_fmod_inplace(const char *ID1_name, const char *ID2_name);  // ID1 is output
int arith_image_pow_inplace(const char *ID1_name, const char *ID2_name);
int arith_image_add_inplace(const char *ID1_name, const char *ID2_name);
int arith_image_sub_inplace(const char *ID1_name, const char *ID2_name);
int arith_image_mult_inplace(const char *ID1_name, const char *ID2_name);
int arith_image_div_inplace(const char *ID1_name, const char *ID2_name);
int arith_image_minv_inplace(const char *ID1_name, const char *ID2_name);
int arith_image_maxv_inplace(const char *ID1_name, const char *ID2_name);








/* ------------------------------------------------------------------------- */
/* complex image, complex image  -> complex image                            */
/* ------------------------------------------------------------------------- */

int arith_image_Cadd_byID(long ID1, long ID2, long IDout);
int arith_image_Csub_byID(long ID1, long ID2, long IDout);
int arith_image_Cmult_byID(long ID1, long ID2, long IDout);
int arith_image_Cdiv_byID(long ID1, long ID2, long IDout);

int arith_image_Cadd(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_Csub(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_Cmult(const char *ID1_name, const char *ID2_name, const char *ID_out);
int arith_image_Cdiv(const char *ID1_name, const char *ID2_name, const char *ID_out);





/* ------------------------------------------------------------------------- */
/* image, double  -> image                                                */
/* ------------------------------------------------------------------------- */

int arith_image_cstfmod_byID(long ID, double f1, long IDout);
int arith_image_cstadd_byID(long ID, double f1, long IDout);
int arith_image_cstsub_byID(long ID, double f1, long IDout);
int arith_image_cstsubm_byID(long ID, double f1, long IDout);
int arith_image_cstmult_byID(long ID, double f1, long IDout);
int arith_image_cstdiv_byID(long ID, double f1, long IDout);
int arith_image_cstpow_byID(long ID, double f1, long IDout);
int arith_image_cstmaxv_byID(long ID, double f1, long IDout);
int arith_image_cstminv_byID(long ID, double f1, long IDout);

int arith_image_cstfmod(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstadd(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstsub(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstsubm(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstmult(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstdiv(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstpow(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstmaxv(const char *ID_name, double f1, const char *ID_out);
int arith_image_cstminv(const char *ID_name, double f1, const char *ID_out);



int arith_image_cstfmod_inplace_byID(long ID, double f1);
int arith_image_cstadd_inplace_byID(long ID, double f1);
int arith_image_cstsub_inplace_byID(long ID, double f1);
int arith_image_cstmult_inplace_byID(long ID, double f1);
int arith_image_cstdiv_inplace_byID(long ID, double f1);
int arith_image_cstpow_inplace_byID(long ID, double f1);
int arith_image_cstmaxv_inplace_byID(long ID, double f1);
int arith_image_cstminv_inplace_byID(long ID, double f1);

int arith_image_cstfmod_inplace(const char *ID_name, double f1);
int arith_image_cstadd_inplace(const char *ID_name, double f1);
int arith_image_cstsub_inplace(const char *ID_name, double f1);
int arith_image_cstmult_inplace(const char *ID_name, double f1);
int arith_image_cstdiv_inplace(const char *ID_name, double f1);
int arith_image_cstpow_inplace(const char *ID_name, double f1);
int arith_image_cstmaxv_inplace(const char *ID_name, double f1);
int arith_image_cstminv_inplace(const char *ID_name, double f1);


/* ------------------------------------------------------------------------- */
/* complex image, complex image  -> complex image                            */
/* ------------------------------------------------------------------------- */

int arith_image_trunc_byID(long ID, double f1, double f2, long IDout);
int arith_image_trunc_inplace_byID(long IDname, double f1, double f2);

int arith_image_trunc(const char *ID_name, double f1, double f2, const char *ID_out);
int arith_image_trunc_inplace(const char *ID_name, double f1, double f2);

long arith_make_slopexy(const char *ID_name, long l1,long l2, double sx, double sy);



//int arith_image_translate(const char *ID_name, const char *ID_out, double xtransl, double ytransl);

int execute_arith(const char *cmd);

#endif
