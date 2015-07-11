#ifndef _ARITH_H
#define _ARITH_H

long arith_set_pixel(char *ID_name, double value, long x, long y);

long arith_set_row(char *ID_name, double value, long y);

long arith_set_col(char *ID_name, double value, long x);

long arith_image_zero(char *ID_name);

int arith_image_crop(char *ID_name, char *ID_out, long *start, long *end, long cropdim);

int arith_image_extract2D(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart);

int arith_image_extract3D(char *in_name, char *out_name, long size_x, long size_y, long size_z, long xstart, long ystart, long zstart);

long arith_image_merge3D(char *ID_name1, char *ID_name2, char *IDout_name);

double arith_image_total(char *ID_name);
double arith_image_mean(char *ID_name);
double arith_image_min(char *ID_name);
double arith_image_max(char *ID_name);

double arith_image_percentile(char *ID_name, double fraction);
double arith_image_median(char *ID_name);

long arith_image_dx(char *ID_name, char *IDout_name);
long arith_image_dy(char *ID_name, char *IDout_name);



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

int arith_image_acos(char *ID_name, char *ID_out);
int arith_image_asin(char *ID_name, char *ID_out);
int arith_image_atan(char *ID_name, char *ID_out);
int arith_image_ceil(char *ID_name, char *ID_out);
int arith_image_cos(char *ID_name, char *ID_out);
int arith_image_cosh(char *ID_name, char *ID_out);
int arith_image_exp(char *ID_name, char *ID_out);
int arith_image_fabs(char *ID_name, char *ID_out);
int arith_image_floor(char *ID_name, char *ID_out);
int arith_image_ln(char *ID_name, char *ID_out);
int arith_image_log(char *ID_name, char *ID_out);
int arith_image_sqrt(char *ID_name, char *ID_out);
int arith_image_sin(char *ID_name, char *ID_out);
int arith_image_sinh(char *ID_name, char *ID_out);
int arith_image_tan(char *ID_name, char *ID_out);
int arith_image_tanh(char *ID_name, char *ID_out);






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

int arith_image_acos_inplace(char *ID_name);
int arith_image_asin_inplace(char *ID_name);
int arith_image_atan_inplace(char *ID_name);
int arith_image_ceil_inplace(char *ID_name);
int arith_image_cos_inplace(char *ID_name);
int arith_image_cosh_inplace(char *ID_name);
int arith_image_exp_inplace(char *ID_name);
int arith_image_fabs_inplace(char *ID_name);
int arith_image_floor_inplace(char *ID_name);
int arith_image_ln_inplace(char *ID_name);
int arith_image_log_inplace(char *ID_name);
int arith_image_sqrt_inplace(char *ID_name);
int arith_image_sin_inplace(char *ID_name);
int arith_image_sinh_inplace(char *ID_name);
int arith_image_tan_inplace(char *ID_name);
int arith_image_tanh_inplace(char *ID_name);




/* Functions for bison / flex    */ 
double Ptrunc(double a, double b, double c);
int arith_image_function_im_im__d_d(char *ID_name, char *ID_out, double (*pt2function)(double));
int arith_image_function_imd_im__dd_d(char *ID_name, double v0, char *ID_out, double (*pt2function)(double, double));
int arith_image_function_imdd_im__ddd_d(char *ID_name, double v0, double v1, char *ID_out, double (*pt2function)(double, double, double));


/* ------------------------------------------------------------------------- */
/* predefined functions    image, image  -> image                                                    */
/* ------------------------------------------------------------------------- */


int arith_image_fmod_byID(long ID1, long ID2, long IDout);
int arith_image_pow_byID(long ID1, long ID2, char *IDout);
int arith_image_add_byID(long ID1, long ID2, long IDout);
int arith_image_sub_byID(long ID1, long ID2, long IDout);
int arith_image_mult_byID(long ID1, long ID2, long IDout);
int arith_image_div_byID(long ID1, long ID2, long IDout);
int arith_image_minv_byID(long ID1, long ID2, long IDout);
int arith_image_maxv_byID(long ID1, long ID2, long IDout);

int arith_image_fmod(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_pow(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_add(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_sub(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_mult(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_div(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_minv(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_maxv(char *ID1_name, char *ID2_name, char *ID_out);




int arith_image_fmod_inplace_byID(long ID1, long ID2);
int arith_image_pow_inplace_byID(long ID1, long ID2);
int arith_image_add_inplace_byID(long ID1, long ID2);
int arith_image_sub_inplace_byID(long ID1, long ID2);
int arith_image_mult_inplace_byID(long ID1, long ID2);
int arith_image_div_inplace_byID(long ID1,  long ID2);
int arith_image_minv_inplace_byID(long ID1, long ID2);
int arith_image_maxv_inplace_byID(long ID1, long ID2);

int arith_image_fmod_inplace(char *ID1_name, char *ID2_name);  // ID1 is output
int arith_image_pow_inplace(char *ID1_name, char *ID2_name);
int arith_image_add_inplace(char *ID1_name, char *ID2_name);
int arith_image_sub_inplace(char *ID1_name, char *ID2_name);
int arith_image_mult_inplace(char *ID1_name, char *ID2_name);
int arith_image_div_inplace(char *ID1_name, char *ID2_name);
int arith_image_minv_inplace(char *ID1_name, char *ID2_name);
int arith_image_maxv_inplace(char *ID1_name, char *ID2_name);








/* ------------------------------------------------------------------------- */
/* complex image, complex image  -> complex image                            */
/* ------------------------------------------------------------------------- */

int arith_image_Cadd_byID(long ID1, long ID2, long IDout);
int arith_image_Csub_byID(long ID1, long ID2, long IDout);
int arith_image_Cmult_byID(long ID1, long ID2, long IDout);
int arith_image_Cdiv_byID(long ID1, long ID2, long IDout);

int arith_image_Cadd(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_Csub(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_Cmult(char *ID1_name, char *ID2_name, char *ID_out);
int arith_image_Cdiv(char *ID1_name, char *ID2_name, char *ID_out);





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

int arith_image_cstfmod(char *ID_name, double f1, char *ID_out);
int arith_image_cstadd(char *ID_name, double f1, char *ID_out);
int arith_image_cstsub(char *ID_name, double f1, char *ID_out);
int arith_image_cstsubm(char *ID_name, double f1, char *ID_out);
int arith_image_cstmult(char *ID_name, double f1, char *ID_out);
int arith_image_cstdiv(char *ID_name, double f1, char *ID_out);
int arith_image_cstpow(char *ID_name, double f1, char *ID_out);
int arith_image_cstmaxv(char *ID_name, double f1, char *ID_out);
int arith_image_cstminv(char *ID_name, double f1, char *ID_out);



int arith_image_cstfmod_inplace_byID(long ID, double f1);
int arith_image_cstadd_inplace_byID(long ID, double f1);
int arith_image_cstsub_inplace_byID(long ID, double f1);
int arith_image_cstmult_inplace_byID(long ID, double f1);
int arith_image_cstdiv_inplace_byID(long ID, double f1);
int arith_image_cstpow_inplace_byID(long ID, double f1);
int arith_image_cstmaxv_inplace_byID(long ID, double f1);
int arith_image_cstminv_inplace_byID(long ID, double f1);

int arith_image_cstfmod_inplace(char *ID_name, double f1);
int arith_image_cstadd_inplace(char *ID_name, double f1);
int arith_image_cstsub_inplace(char *ID_name, double f1);
int arith_image_cstmult_inplace(char *ID_name, double f1);
int arith_image_cstdiv_inplace(char *ID_name, double f1);
int arith_image_cstpow_inplace(char *ID_name, double f1);
int arith_image_cstmaxv_inplace(char *ID_name, double f1);
int arith_image_cstminv_inplace(char *ID_name, double f1);


/* ------------------------------------------------------------------------- */
/* complex image, complex image  -> complex image                            */
/* ------------------------------------------------------------------------- */

int arith_image_trunc_byID(long ID, double f1, double f2, long IDout);
int arith_image_trunc_inplace_byID(long IDname, double f1, double f2);

int arith_image_trunc(char *ID_name, double f1, double f2, char *ID_out);
int arith_image_trunc_inplace(char *ID_name, double f1, double f2);





//int arith_image_translate(char *ID_name, char *ID_out, double xtransl, double ytransl);

int execute_arith(char *cmd);

#endif
