#if !defined(INFO_H)
#define INFO_H


int init_info();



int kbdhit(void);

int print_header(const char *str, char c);

struct timespec info_time_diff(struct timespec start, struct timespec end);

long brighter(const char *ID_name, double value);
/* number of pixels brighter than value */

int img_nbpix_flux(const char *ID_name);

float img_percentile_float(const char *ID_name, float p);

double img_percentile_double(const char *ID_name, double p);

double img_percentile(const char *ID_name, double p);

int img_histoc(const char *ID_name, const char *fname);

int make_histogram(const char *ID_name, const char *ID_out_name, double min, double max, long nbsteps);

double ssquare(const char *ID_name);

double rms_dev(const char *ID_name);

int info_image_stats(const char *ID_name, const char *options);

long info_cubestats(const char *ID_name, const char *IDmask_name, const char *outfname);

double img_min(const char *ID_name);

double img_max(const char *ID_name);

int profile(const char *ID_name, const char *outfile, double xcenter, double ycenter, double step, long nb_step);

int profile2im(const char *profile_name, long nbpoints, long size, double xcenter, double ycenter, double radius, const char *out);

int printpix(const char *ID_name, const char *filename);

double background_photon_noise(const char *ID_name);

int test_structure_function(const char *ID_name, long NBpoints, const char *fname);

int full_structure_function(const char *ID_name, long NBpoints, const char *ID_out);

int fft_structure_function(const char *ID_in, const char *ID_out);

#endif
