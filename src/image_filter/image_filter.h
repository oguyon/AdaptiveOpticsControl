#if !defined(FILTER_H)
#define FILTER_H

int init_image_filter();

int median_filter(char *ID_name, char *out_name, int filter_size);

long FILTER_percentile_interpol_fast(char *ID_name, char *IDout_name, double perc, long boxrad);

long FILTER_percentile_interpol(char *ID_name, char *IDout_name, double perc, double sigma);

long gauss_filter(char *ID_name, char *out_name, float sigma, int filter_size);

int gauss_3Dfilter(char *ID_name, char *out_name, float sigma, int filter_size);

int f_filter(char *ID_name, char *ID_out, float f1, float f2);

long fconvolve(char *ID_in, char *ID_ke, char *ID_out);

long fconvolve_padd(char *ID_in, char *ID_ke, long paddsize, char *ID_out);

int fconvolve_1(char *name_in, char *kefft, char *name_out);

int fconvolveblock(char *name_in, char *name_ke, char *name_out, long blocksize);

int film_scanner_vsripes_remove(char *IDname, char *IDout, long l1, long l2);

int filter_fit2DcosKernel(char *IDname, float radius);

long filter_CubePercentile(char *IDcin_name, float perc, char *IDout_name);

long filter_CubePercentileLimit(char *IDcin_name, float perc, float limit, char *IDout_name);

#endif
