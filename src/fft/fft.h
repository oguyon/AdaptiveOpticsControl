
#if !defined(FFT_H)
#define FFT_H

int import_wisdom();

int fft_setoffsets(long o1, long o2);

int init_fftw_plans(int mode);

int init_fftw_plans0();

int export_wisdom();

int permut(char *ID_name);

//void permutfliphv(char *ID_name);

int do1dfft(char *in_name, char *out_name);

int do1drfft(char *in_name, char *out_name);

int do1dffti(char *in_name, char *out_name);

int do2dfft(char *in_name, char *out_name);

int do2dffti(char *in_name, char *out_name);

int pupfft(char *ID_name_ampl, char *ID_name_pha, char *ID_name_ampl_out, char *ID_name_pha_out, char *options);

int do2drfft(char *in_name, char *out_name);

int do2drffti(char *in_name, char *out_name);

long fft_correlation(char *ID_name1, char *ID_name2, char *ID_nameout);

int autocorrelation(char *ID_name, char *ID_out);

int fftzoom(char *ID_name, char *ID_out, long factor);

int fftczoom(char *ID_name, char *ID_out, long factor);

int test_fftspeed(int nmax);

long fft_DFT( char *IDin_name, char *IDinmask_name, char *IDout_name, char *IDoutmask_name, double Zfactor, int dir, long kin);

long fft_DFTinsertFPM( char *pupin_name, char *fpmz_name, double zfactor, char *pupout_name);

long fft_DFTinsertFPM_re( char *pupin_name, char *fpmz_name, double zfactor, char *pupout_name);

int fft_image_translate(char *ID_name, char *ID_out, double xtransl, double ytransl);

#endif
