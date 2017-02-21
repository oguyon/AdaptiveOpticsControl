#if !defined(FFT_H)
#define FFT_H

int_fast8_t init_fft();


int import_wisdom();

int fft_setoffsets(long o1, long o2);

int_fast8_t init_fftw_plans(int mode);

int_fast8_t init_fftw_plans0();

int export_wisdom();

int permut(const char *ID_name);

//void permutfliphv(const char *ID_name);

long do1dfft(const char *in_name, const char *out_name);

long do1drfft(const char *in_name, const char *out_name);

long do1dffti(const char *in_name, const char *out_name);

long do2dfft(const char *in_name, const char *out_name);

long do2dffti(const char *in_name, const char *out_name);

int pupfft(const char *ID_name_ampl, const char *ID_name_pha, const char *ID_name_ampl_out, const char *ID_name_pha_out, const char *options);

long do2drfft(const char *in_name, const char *out_name);

long do2drffti(const char *in_name, const char *out_name);

long fft_correlation(const char *ID_name1, const char *ID_name2, const char *ID_nameout);

int autocorrelation(const char *ID_name, const char *ID_out);

int fftzoom(const char *ID_name, const char *ID_out, long factor);

int fftczoom(const char *ID_name, const char *ID_out, long factor);

int test_fftspeed(int nmax);

long fft_DFT( const char *IDin_name, const char *IDinmask_name, const char *IDout_name, const char *IDoutmask_name, double Zfactor, int dir, long kin);

long fft_DFTinsertFPM( const char *pupin_name, const char *fpmz_name, double zfactor, const char *pupout_name);

long fft_DFTinsertFPM_re( const char *pupin_name, const char *fpmz_name, double zfactor, const char *pupout_name);

int fft_image_translate(const char *ID_name, const char *ID_out, double xtransl, double ytransl);

#endif
