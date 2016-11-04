#ifndef _BASIC_H
#define _BASIC_H


int init_image_basic();


int basic_naninf2zero(char *ID_name);

int basic_lmin_im(char *ID_name, char *out_name);

int basic_lmax_im(char *ID_name, char *out_name);

long basic_add(char *ID1_name, char *ID2_name, char *ID3_name, long off1, long off2);

long basic_diff(char *ID1_name, char *ID2_name, char *ID3_name, long off1, long off2);

long basic_extract(char *ID_in_name, char *ID_out_name, long n1, long n2, long n3, long n4);

int basic_trunc_circ(char *ID_name, float f1);

long basic_expand(char *ID_name, char *ID_name_out, int n1, int n2);
long basic_expand3D(char *ID_name, char *ID_name_out, int n1, int n2, int n3);

long basic_zoom2(char *ID_name, char *ID_name_out);

long basic_contract(char *ID_name, char *ID_name_out, int n1, int n2);

long basic_contract3D(char *ID_name, char *ID_name_out, int n1, int n2, int n3);

long basic_average_column(char *ID_name, char *IDout_name);

long basic_padd(char *ID_name, char *ID_name_out, int n1, int n2);

long basic_fliph(char *ID_name);

long basic_flipv(char *ID_name);

long basic_fliphv(char *ID_name);

int basic_median(char *ID_name, char *options);

long basic_renorm_max(char *ID_name);

long basic_rotate(char *ID_name, char *IDout_name, float angle);

int basic_rotate90(char *ID_name, char *ID_out_name);

int basic_rotate_int(char *ID_name, char *ID_out_name, long nbstep);

int basic_translate(char *ID_name, char *ID_out, float xtransl, float ytransl);

float basic_correlation(char *ID_name1, char *ID_name2);

long IMAGE_BASIC_get_assym_component(char *ID_name, char *ID_out_name, float xcenter, float ycenter, char *options);

long IMAGE_BASIC_get_sym_component(char *ID_name, char *ID_out_name, float xcenter, float ycenter);

int basic_rotate2(char *ID_name_in, char *ID_name_out, float angle);

int basic_rotate3(char *ID_name_in, char *ID_name_out, float angle);

int basic_stretch(char *name_in, char *name_out, float coeff, long Xcenter, long Ycenter);

int basic_stretch_range(char *name_in, char *name_out, float coeff1, float coeff2, long Xcenter, long Ycenter, long NBstep, float ApoCoeff);

int basic_stretchc(char *name_in, char *name_out, float coeff);

int gauss_histo_image(char *ID_name, char *ID_out_name, float sigma, float center);

long load_fitsimages(char *strfilter);

long load_fitsimages_cube(char *strfilter, char *ID_out_name);

long basic_cube_center(char *ID_in_name, char *ID_out_name);

long cube_average(char *ID_in_name, char *ID_out_name, float alpha);

long cube_collapse(char *ID_in_name, char *ID_out_name);

long basic_addimagesfiles(char *strfilter, char *outname);

long basic_pasteimages(char *prefix, long NBcol, char *IDout_name);

long basic_aveimagesfiles(char *strfilter, char *outname);

long basic_addimages(char *prefix, char *ID_out);

long basic_averageimages(char *prefix, char *ID_out);

long basic_resizeim(char *imname_in, char *imname_out, long xsizeout, long ysizeout);
long image_basic_3Dto2D(char *IDname);
long image_basic_SwapAxis2D(char *IDin_name, char *IDout_name);

long basic_tableto2Dim(char *fname, float xmin, float xmax, float ymin, float ymax, long xsize, long ysize, char *ID_name, float convsize);

long basic_2Dextrapolate_nearestpixel(char *IDin_name, char *IDmask_name, char *IDout_name);

double basic_measure_transl( char *ID_name1, char *ID_name2, long tmax);

// Operations on image streams
long IMAGE_BASIC_streamaverage(char *IDname, long NBcoadd, char *IDoutname, int mode);

long IMAGE_BASIC_streamfeed(char *IDname, char *streamname, float frequ);

long IMAGE_BASIC_streamrecord(char *streamname, long NBframes, char *IDname);

#endif
