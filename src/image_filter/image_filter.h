#if !defined(FILTER_H)
#define FILTER_H

int init_image_filter();

int median_filter(const char *ID_name, const char *out_name, int filter_size);

long FILTER_percentile_interpol_fast(const char *ID_name, const char *IDout_name, double perc, long boxrad);

long FILTER_percentile_interpol(const char *ID_name, const char *IDout_name, double perc, double sigma);

long gauss_filter(const char *ID_name, const char *out_name, float sigma, int filter_size);

int gauss_3Dfilter(const char *ID_name, const char *out_name, float sigma, int filter_size);

int f_filter(const char *ID_name, const char *ID_out, float f1, float f2);

long fconvolve(const char *ID_in, const char *ID_ke, const char *ID_out);

long fconvolve_padd(const char *ID_in, const char *ID_ke, long paddsize, const char *ID_out);

int fconvolve_1(const char *name_in, const char *kefft, const char *name_out);

int fconvolveblock(const char *name_in, const char *name_ke, const char *name_out, long blocksize);

int film_scanner_vsripes_remove(const char *IDname, const char *IDout, long l1, long l2);

int filter_fit2DcosKernel(const char *IDname, float radius);

long filter_CubePercentile(const char *IDcin_name, float perc, const char *IDout_name);

long filter_CubePercentileLimit(const char *IDcin_name, float perc, float limit, const char *IDout_name);

#endif
