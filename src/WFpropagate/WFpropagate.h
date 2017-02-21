#ifndef _WFPROPAGATEMODULE_H
#define _WFPROPAGATEMODULE_H


int_fast8_t init_WFpropagate();


int Fresnel_propagate_wavefront(const char *in, const char *out, double PUPIL_SCALE, double z, double lambda);

int Init_Fresnel_propagate_wavefront(const char *Cim, long size, double PUPIL_SCALE, double z, double lambda, double FPMASKRAD, int Precision);

int Fresnel_propagate_wavefront1(const char *in, const char *out, const char *Cin);

long Fresnel_propagate_cube(const char *IDcin_name, const char *IDout_name_amp, const char *IDout_name_pha, double PUPIL_SCALE, double zstart, double zend, long NBzpts, double lambda);

long WFpropagate_run();

#endif
