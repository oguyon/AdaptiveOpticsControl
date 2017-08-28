#ifndef _AtmosphericTurbulence_H
#define _AtmosphericTurbulence_H


int init_AtmosphericTurbulence();

int AtmosphericTurbulence_change_configuration_file(const char *fname);

long make_AtmosphericTurbulence_vonKarmanWind(long vKsize, float pixscale, float sigmawind, float Lwind, long size, const char *IDout_name);

int make_master_turbulence_screen(const char *ID_name1, const char *ID_name2, long size, float outerscale, float innerscale, long WFprecision);

int make_master_turbulence_screen_pow(const char *ID_name1, const char *ID_name2, long size, float power);

//int unwrap_phase_screen(const char *ID_name);

int contract_wavefront_series(const char *in_prefix, const char *out_prefix, long NB_files);

int contract_wavefront_cube(const char *ina_file, const char *inp_file, const char *outa_file, const char *outp_file, int factor);

int contract_wavefront_cube_phaseonly(const char *inp_file, const char *outp_file, int factor);

int make_AtmosphericTurbulence_wavefront_series(float slambdaum, long WFprecision, int compmode);

int measure_wavefront_series(float factor);
 
int AtmosphericTurbulence_mkTestTTseq(double dt, long NBpts, long NBblocks, double measnoise, int ACCnmode, double ACCnoise, int MODE);

int AtmosphericTurbulence_Build_LinPredictor_Full(const char *WFin_name, const char *WFmask_name, int PForder, float PFlag, double SVDeps, double RegLambda);
int AtmosphericTurbulence_Apply_LinPredictor_Full(int MODE, const char *WFin_name, const char *WFmask_name, int PForder, float PFlag, const char *WFoutp_name, const char *WFoutf_name);
long AtmosphericTurbulence_LinPredictor_filt_2DKernelExtract(const char *IDfilt_name, const char *IDmask_name, long krad, const char *IDkern_name);
long AtmosphericTurbulence_LinPredictor_filt_Expand(const char *IDfilt_name, const char *IDmask_name);

int AtmosphericTurbulence_Build_LinPredictor(long NB_WFstep, double WFphaNoise, long WFPlag, long WFP_NBstep, long WFP_xyrad, long WFPiipix, long WFPjjpix, float slambdaum);
long AtmosphericTurbulence_psfCubeContrast(const char *IDwfc_name, const char *IDmask_name, const char *IDpsfc_name);
int AtmosphericTurbulence_Test_LinPredictor(long NB_WFstep, double WFphaNoise, const char *IDWFPfilt_name, long WFPlag, long WFPiipix, long WFPjjpix, float slambdaum);

int measure_wavefront_series_expoframes(float etime, const char *outfile);

int frame_select_PSF(const char *logfile, long NBfiles, float frac);

int AtmosphericTurbulence_WFprocess();

int AtmosphericTurbulence_makeHV_CN2prof(double wspeed, double r0, double sitealt, long NBlayer, const char *outfile);

#endif
