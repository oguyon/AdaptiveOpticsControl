#if !defined(GENIMAGE_H)
#define GENIMAGE_H


int init_image_gen();



long make_double_star(char *ID_name, long l1, long l2, double intensity_1, double intensity_2, double separation, double position_angle);
/* creates a double star */

long make_disk(char *ID_name, long l1, long l2, double x_center, double y_center, double radius);
/* creates a disk */

long make_subpixdisk(char *ID_name, long l1, long l2, double x_center, double y_center, double radius);
 /* creates a disk */

long make_subpixdisk_perturb(char *ID_name, long l1, long l2, double x_center, double y_center, double radius, long n, double *ra, double *ka, double *pa);

long make_square(char *ID_name, long l1, long l2, double x_center, double y_center, double radius);
/* creates a square */

long make_rectangle(char *ID_name, long l1, long l2, double x_center, double y_center, double radius1, double radius2);

long make_line(char *IDname, long l1, long l2, double x1, double y1, double x2, double y2, double t);
long make_lincoordinate(char *IDname, long l1, long l2, double x_center, double y_center, double angle);
long make_hexagon(char *IDname, long l1, long l2, double x_center, double y_center, double radius);

long make_hexsegpupil(char *IDname, long size, double radius, double gap, double step);
long IMAGE_gen_segments2WFmodes(char *prefix, long ndigit, char *IDout);
long make_jacquinot_pupil(char *ID_name, long l1, long l2, double x_center, double y_center, double width, double height);

long make_sectors(char *ID_name, long l1, long l2, double x_center, double y_center, double step, long NB_sectors);

long make_rnd(char *ID_name, long l1, long l2, char *options);
long make_rnd_double(char *ID_name, long l1, long l2, char *options);
/*int make_rnd1(char *ID_name, long l1, long l2, char *options);*/

long make_gauss(char *ID_name, long l1, long l2, double a, double A);

long make_2axis_gauss(char *ID_name, long l1, long l2, double a, double A, double E, double PA);

long make_cluster(char *ID_name, long l1, long l2, char *options);

long make_galaxy(char *ID_name, long l1, long l2, double S_radius, double S_L0, double S_ell, double S_PA, double E_radius, double E_L0, double E_ell, double E_PA);

long make_Egalaxy(char *ID_name, long l1, long l2, char *options);

// make image of EZ disk
long gen_image_EZdisk(char *ID_name, long size, double InnerEdge, double Index, double Incl);

long make_slopexy(char *ID_name, long l1,long l2, double sx, double sy);

long make_dist(char *ID_name, long l1,long l2, double f1, double f2);

long make_PosAngle(char *ID_name, long l1,long l2, double f1, double f2);

long make_psf_from_profile(char *profile_name, char *ID_name, long l1, long l2);

long make_offsetHyperGaussian(long size, double a, double b, long n, char* IDname);

long make_cosapoedgePupil(long size, double a, double b, char *IDname);

long make_2Dgridpix(char *IDname, long xsize, long ysize, double pitchx, double pitchy, double offsetx, double offsety);

long make_tile(char *IDin_name, long size, char *IDout_name);

long image_gen_im2coord(char *IDin_name, int axis, char *IDout_name);

#endif
