#if !defined(GENIMAGE_H)
#define GENIMAGE_H


int_fast8_t init_image_gen();



long make_double_star(const char *ID_name, long l1, long l2, double intensity_1, double intensity_2, double separation, double position_angle);
/* creates a double star */

long make_disk(const char *ID_name, long l1, long l2, double x_center, double y_center, double radius);
/* creates a disk */

long make_subpixdisk(const char *ID_name, long l1, long l2, double x_center, double y_center, double radius);
 /* creates a disk */

long make_subpixdisk_perturb(const char *ID_name, long l1, long l2, double x_center, double y_center, double radius, long n, double *ra, double *ka, double *pa);

long make_square(const char *ID_name, long l1, long l2, double x_center, double y_center, double radius);
/* creates a square */

long make_rectangle(const char *ID_name, long l1, long l2, double x_center, double y_center, double radius1, double radius2);

long make_line(const char *IDname, long l1, long l2, double x1, double y1, double x2, double y2, double t);
long make_lincoordinate(const char *IDname, long l1, long l2, double x_center, double y_center, double angle);
long make_hexagon(const char *IDname, long l1, long l2, double x_center, double y_center, double radius);

long make_hexsegpupil(const char *IDname, long size, double radius, double gap, double step);
long IMAGE_gen_segments2WFmodes(const char *prefix, long ndigit, const char *IDout);
long make_jacquinot_pupil(const char *ID_name, long l1, long l2, double x_center, double y_center, double width, double height);

long make_sectors(const char *ID_name, long l1, long l2, double x_center, double y_center, double step, long NB_sectors);

long make_rnd(const char *ID_name, long l1, long l2, const char *options);
long make_rnd_double(const char *ID_name, long l1, long l2, const char *options);
/*int make_rnd1(const char *ID_name, long l1, long l2, const char *options);*/

long make_gauss(const char *ID_name, long l1, long l2, double a, double A);

long make_2axis_gauss(const char *ID_name, long l1, long l2, double a, double A, double E, double PA);

long make_cluster(const char *ID_name, long l1, long l2, const char *options);

long make_galaxy(const char *ID_name, long l1, long l2, double S_radius, double S_L0, double S_ell, double S_PA, double E_radius, double E_L0, double E_ell, double E_PA);

long make_Egalaxy(const char *ID_name, long l1, long l2, const char *options);

// make image of EZ disk
long gen_image_EZdisk(const char *ID_name, long size, double InnerEdge, double Index, double Incl);

long make_slopexy(const char *ID_name, long l1,long l2, double sx, double sy);

long make_dist(const char *ID_name, long l1,long l2, double f1, double f2);

long make_PosAngle(const char *ID_name, long l1,long l2, double f1, double f2);

long make_psf_from_profile(const char *profile_name, const char *ID_name, long l1, long l2);

long make_offsetHyperGaussian(long size, double a, double b, long n, char* IDname);

long make_cosapoedgePupil(long size, double a, double b, const char *IDname);

long make_2Dgridpix(const char *IDname, long xsize, long ysize, double pitchx, double pitchy, double offsetx, double offsety);

long make_tile(const char *IDin_name, long size, const char *IDout_name);

long image_gen_im2coord(const char *IDin_name, int axis, const char *IDout_name);

#endif
