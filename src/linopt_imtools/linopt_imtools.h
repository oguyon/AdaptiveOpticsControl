#ifndef _LINOPTIMTOOLS_H
#define _LINOPTIMTOOLS_H


// creating modal basis

long linopt_imtools_makeCosRadModes(char *ID_name, long size, long kmax, float radius, float radfactlim);

long linopt_imtools_makeCPAmodes(char *ID_name, long size, float CPAmax, float deltaCPA, float radius, float radfactlim, int writeMfile);




long linopt_imtools_mask_to_pixtable(char *IDmask_name, char *IDpixindex_name, char *IDpixmult_name);

long linopt_imtools_Image_to_vec(char *ID_name, char *IDpixindex_name, char *IDpixmult_name, char *IDvec_name);

long linopt_imtools_vec_to_2DImage(char *IDvec_name, char *IDpixindex_name, char *IDpixmult_name, char *ID_name, long xsize, long ysize);

long linopt_imtools_image_construct(char *IDmodes_name, char *IDcoeff_name, char *ID_name);

long linopt_imtools_image_fitModes(char *ID_name, char *IDmodes_name, char *IDmask_name, double SVDeps, char *IDcoeff_name, int reuse);



double linopt_imtools_match_slow(char *ID_name, char *IDref_name, char *IDmask_name, char *IDsol_name, char *IDout_name);

double linopt_imtools_match(char *ID_name, char *IDref_name, char *IDmask_name, char *IDsol_name, char *IDout_name);

#endif
