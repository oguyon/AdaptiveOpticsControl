#ifndef _OPTICSMATERIALS_H
#define _OPTICSMATERIALS_H

int init_OpticsMaterials();


int OPTICSMATERIALS_code(char *name);

char* OPTICSMATERIALS_name(int code);

double OPTICSMATERIALS_n( int material, double lambda);

// phase offset as function of mask thickness and lambda
double OPTICSMATERIALS_pha_lambda( int material, double z, double lambda );

#endif
