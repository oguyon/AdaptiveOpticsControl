#ifndef _WFPROPAGATEMODULE_H
#define _WFPROPAGATEMODULE_H

int Fresnel_propagate_wavefront(char *in, char *out, double PUPIL_SCALE, double z, double lambda);

int Init_Fresnel_propagate_wavefront(char *Cim, long size, double PUPIL_SCALE, double z, double lambda, double FPMASKRAD);

int Fresnel_propagate_wavefront1(char *in, char *out, char *Cin);

long Fresnel_propagate_cube(char *IDcin_name, char *IDout_name_amp, char *IDout_name_pha, double PUPIL_SCALE, double zstart, double zend, long NBzpts, double lambda);

long WFpropagate_run();

#endif
