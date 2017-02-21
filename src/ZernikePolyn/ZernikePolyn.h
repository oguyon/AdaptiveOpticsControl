#ifndef _ZERNIKEPOLYN_H
#define _ZERNIKEPOLYN_H



typedef struct /* structure to store Zernike coefficients */
{
  int init;
  long ZERMAX;
  long *Zer_n;
  long *Zer_m;
  double *R_array;
} ZERNIKE;




int_fast8_t init_ZernikePolyn();



double fact(int n);

int zernike_init();

long Zernike_n(long i);

long Zernike_m(long i);

double Zernike_value(long j, double r, double PA);

long mk_zer(const char *ID_name, long SIZE, long zer_nb, float rpix);

long mk_zer_unbounded(const char *ID_name, long SIZE, long zer_nb, float rpix);

int mk_zer_series(const char *ID_name, long SIZE, long zer_nb, float rpix);

long mk_zer_seriescube(const char *ID_namec, long SIZE, long zer_nb, float rpix);

double get_zer(const char *ID_name, long zer_nb, double radius);

double get_zer_crop(const char *ID_name, long zer_nb, double radius, double radius1);

int get_zerns(const char *ID_name, long max_zer, double radius);

int get_zern_array(const char *ID_name, long max_zer, double radius, double *array);

int remove_zerns(const char *ID_name, const char *ID_name_out, int max_zer, double radius);

long ZERNIKEPOLYN_rmPiston(const char *ID_name, const char *IDmask_name);

int remove_TTF(const char *ID_name, const char *ID_name_out, double radius);

double fit_zer(const char *ID_name, long maxzer_nb, double radius, double *zvalue, double *residual);

#endif
