#ifndef _BASIC_H
#define _BASIC_H


int_fast8_t init_image_basic();


int basic_naninf2zero(const char *ID_name);

int basic_lmin_im(const char *ID_name, const char *out_name);

int basic_lmax_im(const char *ID_name, const char *out_name);

long basic_add(const char *ID1_name, const char *ID2_name, const char *ID3_name, long off1, long off2);
long basic_add3D(const char *ID_name1, const char *ID_name2, const char *ID_name_out, long off1, long off2, long off3);

long basic_diff(const char *ID1_name, const char *ID2_name, const char *ID3_name, long off1, long off2);

long basic_extract(const char *ID_in_name, const char *ID_out_name, long n1, long n2, long n3, long n4);

int basic_trunc_circ(const char *ID_name, float f1);

long basic_expand(const char *ID_name, const char *ID_name_out, int n1, int n2);
long basic_expand3D(const char *ID_name, const char *ID_name_out, int n1, int n2, int n3);

long basic_zoom2(const char *ID_name, const char *ID_name_out);

long basic_contract(const char *ID_name, const char *ID_name_out, int n1, int n2);

long basic_contract3D(const char *ID_name, const char *ID_name_out, int n1, int n2, int n3);

long basic_average_column(const char *ID_name, const char *IDout_name);

long basic_padd(const char *ID_name, const char *ID_name_out, int n1, int n2);

long basic_fliph(const char *ID_name);

long basic_flipv(const char *ID_name);

long basic_fliphv(const char *ID_name);

int basic_median(const char *ID_name, const char *options);

long basic_renorm_max(const char *ID_name);

long basic_rotate(const char *ID_name, const char *IDout_name, float angle);

int basic_rotate90(const char *ID_name, const char *ID_out_name);

int basic_rotate_int(const char *ID_name, const char *ID_out_name, long nbstep);

int basic_translate(const char *ID_name, const char *ID_out, float xtransl, float ytransl);

float basic_correlation(const char *ID_name1, const char *ID_name2);

long IMAGE_BASIC_get_assym_component(const char *ID_name, const char *ID_out_name, float xcenter, float ycenter, const char *options);

long IMAGE_BASIC_get_sym_component(const char *ID_name, const char *ID_out_name, float xcenter, float ycenter);

int basic_rotate2(const char *ID_name_in, const char *ID_name_out, float angle);

int basic_rotate3(const char *ID_name_in, const char *ID_name_out, float angle);

int basic_stretch(const char *name_in, const char *name_out, float coeff, long Xcenter, long Ycenter);

int basic_stretch_range(const char *name_in, const char *name_out, float coeff1, float coeff2, long Xcenter, long Ycenter, long NBstep, float ApoCoeff);

int basic_stretchc(const char *name_in, const char *name_out, float coeff);

int gauss_histo_image(const char *ID_name, const char *ID_out_name, float sigma, float center);

long load_fitsimages(const char *strfilter);

long load_fitsimages_cube(const char *strfilter, const char *ID_out_name);

long basic_cube_center(const char *ID_in_name, const char *ID_out_name);

long cube_average(const char *ID_in_name, const char *ID_out_name, float alpha);

long cube_collapse(const char *ID_in_name, const char *ID_out_name);

long basic_addimagesfiles(const char *strfilter, const char *outname);

long basic_pasteimages(const char *prefix, long NBcol, const char *IDout_name);

long basic_aveimagesfiles(const char *strfilter, const char *outname);

long basic_addimages(const char *prefix, const char *ID_out);

long basic_averageimages(const char *prefix, const char *ID_out);

long basic_resizeim(const char *imname_in, const char *imname_out, long xsizeout, long ysizeout);
long image_basic_3Dto2D(const char *IDname);
long image_basic_SwapAxis2D(const char *IDin_name, const char *IDout_name);

long basic_tableto2Dim(const char *fname, float xmin, float xmax, float ymin, float ymax, long xsize, long ysize, const char *ID_name, float convsize);

long basic_2Dextrapolate_nearestpixel(const char *IDin_name, const char *IDmask_name, const char *IDout_name);

double basic_measure_transl( const char *ID_name1, const char *ID_name2, long tmax);

// Operations on image streams

/** @brief Average an image stream */
long IMAGE_BASIC_streamaverage(const char *IDname, long NBcoadd, const char *IDoutname, int mode, int semindex);

long IMAGE_BASIC_streamfeed(const char *IDname, const char *streamname, float frequ);

long IMAGE_BASIC_streamrecord(const char *streamname, long NBframes, const char *IDname);

#endif
