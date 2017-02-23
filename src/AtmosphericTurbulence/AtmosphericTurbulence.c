#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#ifdef __MACH__
#include <mach/mach_time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
int clock_gettime(int clk_id, struct timespec *t){
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t time;
    time = mach_absolute_time();
    double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
    double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
    t->tv_sec = seconds;
    t->tv_nsec = nseconds;
    return 0;
}
#else
#include <time.h>
#endif



#include "CLIcore.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_arith/COREMOD_arith.h"

#include "fft/fft.h"
#include "image_basic/image_basic.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "image_filter/image_filter.h"
#include "info/info.h"
#include "image_gen/image_gen.h"
#include "statistic/statistic.h"
#include "linopt_imtools/linopt_imtools.h"
#include "AtmosphericTurbulence/AtmosphericTurbulence.h"
#include "psf/psf.h"
#include "WFpropagate/WFpropagate.h"
#include "OpticsMaterials/OpticsMaterials.h"
#include "AtmosphereModel/AtmosphereModel.h"

#include "cudacomp/cudacomp.h"


#ifdef _OPENMP
#include <omp.h>
#endif



#define SWAP(x,y)  temp=(x);x=(y);y=temp;

#define PI 3.14159265358979323846264338328


extern DATA data;

char CONFFILE[200] = "WFsim.conf";
 
//int TimeDayOfYear;
//float TimeLocalSolarTime;


float SiteLat;
float SiteLong;
float SiteAlt;
//float CO2_ppm;

//float SiteH2OMethod;
//float SiteTPW;
//float SiteRH;
//float SitePWSH;
//float alpha1H2O;






// constants
double C_me = 9.10938291e-31; // electon mass [kg]
double C_e0 = 8.854187817620e-12; // Vacuum permittivity [F.m-1]
double C_Na = 6.0221413e23; // Avogadro number
double C_e = 1.60217657e-19; // electron charge [C]
double C_ls = 2.686777447e25; // Loschmidt constant


double rhocoeff = 1.0;








// CONFIGURATION

// ------------ TURBULENCE AND ATMOSPHERE PARAMETERS -------------------------------------------
float CONF_LAMBDA;
float CONF_SEEING;
char CONF_TURBULENCE_PROF_FILE[200];
float CONF_ZANGLE;
float CONF_SOURCE_Xpos;
float CONF_SOURCE_Ypos;


// ------------ LOW_FIDELITY OUTPUT AT REF LAMBDA ------------------------------------------------

int CONF_WFOUTPUT = 1;
char CONF_WF_FILE_PREFIX[200];
int CONF_SHM_OUTPUT = 0;


// ------------- HIGH FIDELITY OUTPUT WAVELENGTH ---------------------------------

int CONF_MAKE_SWAVEFRONT = 0;
//float CONF_SLAMBDA;
int CONF_SWF_WRITE2DISK = 0;
char CONF_SWF_FILE_PREFIX[200];

int CONF_SHM_SOUTPUT = 0;
char CONF_SHM_SPREFIX[100];
int CONF_SHM_SOUTPUTM = 0; // 1: output in [meter]


// ------------ OUTPUT PARAMETERS --------------------------------------------
long CONF_WFsize;
float CONF_PUPIL_SCALE;


// ------------- TIMING ---------------------------------------
int CONF_ATMWF_REALTIME;
double CONF_ATMWF_REALTIMEFACTOR = 1.0;
float CONF_WFTIME_STEP;
float CONF_TIME_SPAN;
long CONF_NB_TSPAN;
long CONF_SIMTDELAY = 0;
int CONF_WAITFORSEM = 0;
char CONF_WAITSEMIMNAME[100];


// ------------ COMPUTATION PARAMETERS, MODES --------------------------------
int CONF_SKIP_EXISTING;
long CONF_WF_RAW_SIZE;
long CONF_MASTER_SIZE;


// ------------ WAVEFRONT AMPLITUDE -------------------------------------------
int CONF_FRESNEL_PROPAGATION;
int CONF_WAVEFRONT_AMPLITUDE;
float CONF_FRESNEL_PROPAGATION_BIN;













// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//


int_fast8_t make_AtmosphericTurbulence_wavefront_series_cli()
{
	if(CLI_checkarg(1,1)+CLI_checkarg(2,2)+CLI_checkarg(3,2)==0)
		make_AtmosphericTurbulence_wavefront_series(data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl);
	else
		return 1;
}


int_fast8_t make_AtmosphericTurbulence_vonKarmanWind_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,1)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,2)+CLI_checkarg(6,3)==0)
        make_AtmosphericTurbulence_vonKarmanWind(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.string);
    else
        return 1;
}   


int_fast8_t AtmosphericTurbulence_mkmastert_cli()
{
  
  if(CLI_checkarg(1,3)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)==0)
    {
      make_master_turbulence_screen(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, 1.0*data.cmdargtoken[4].val.numf, 1.0*data.cmdargtoken[5].val.numf, 0);
    }
  else
    return 1;
}

int_fast8_t AtmosphericTurbulence_makeHV_CN2prof_cli()
{
  if(CLI_checkarg(1,1)+CLI_checkarg(2,1)+CLI_checkarg(3,1)+CLI_checkarg(4,2)+CLI_checkarg(5,3)==0)
    {
      AtmosphericTurbulence_makeHV_CN2prof(data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.string);
    }
  else
    return 1;
}





int_fast8_t AtmosphericTurbulence_measure_wavefront_series_expoframes_cli()
{
    if(CLI_checkarg(1,1)+CLI_checkarg(2,3)==0)
    {
        measure_wavefront_series_expoframes(data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.string);
    }
    else
        return(1);
    
}


int_fast8_t AtmosphericTurbulence_mkTestTTseq_cli()
{
   if(CLI_checkarg(1,1)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,2)+CLI_checkarg(6,1)+CLI_checkarg(7,2)==0)
    {
		AtmosphericTurbulence_mkTestTTseq(data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numf, data.cmdargtoken[7].val.numl);
    }
    else
        return(1);
}




int_fast8_t AtmosphericTurbulence_Build_LinPredictor_Full_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)==0)
    {
        AtmosphericTurbulence_Build_LinPredictor_Full(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf);
    }
    else
        return(1);
}


int_fast8_t AtmosphericTurbulence_Apply_LinPredictor_Full_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,4)+CLI_checkarg(3,4)+CLI_checkarg(4,2)+CLI_checkarg(5,1)+CLI_checkarg(6,3)+CLI_checkarg(7,3)==0)
    {
        AtmosphericTurbulence_Apply_LinPredictor_Full(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.string, data.cmdargtoken[7].val.string);
    }
    else
        return(1);
}

int_fast8_t AtmosphericTurbulence_LinPredictor_filt_2DKernelExtract_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,2)+CLI_checkarg(4,3)==0)
    {
       AtmosphericTurbulence_LinPredictor_filt_2DKernelExtract(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string);
    }
    else
        return(1);
}



int_fast8_t AtmosphericTurbulence_LinPredictor_filt_Expand_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)==0)
    {
       AtmosphericTurbulence_LinPredictor_filt_Expand(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
    }
    else
        return(1);
}


int_fast8_t AtmosphericTurbulence_Build_LinPredictor_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,1)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,2)+CLI_checkarg(6,2)+CLI_checkarg(7,2)+CLI_checkarg(8,1)==0)
    {
        AtmosphericTurbulence_Build_LinPredictor(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numl, data.cmdargtoken[7].val.numl, data.cmdargtoken[8].val.numf);
    }
    else
        return(1);
    
}

int_fast8_t AtmosphericTurbulence_psfCubeContrast_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,3)==0)
	{
		AtmosphericTurbulence_psfCubeContrast(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
	}
	else
        return(1);
    
}

int_fast8_t AtmosphericTurbulence_Test_LinPredictor_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,1)+CLI_checkarg(3,4)+CLI_checkarg(4,2)+CLI_checkarg(5,2)+CLI_checkarg(6,2)+CLI_checkarg(7,1)==0)
    {
        AtmosphericTurbulence_Test_LinPredictor(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numl, data.cmdargtoken[7].val.numf);
    }
    else
        return(1);
}



int init_AtmosphericTurbulence()
{
    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "Atmospheric Turbulence");
    data.NBmodule++;


    strcpy(data.cmd[data.NBcmd].key,"mkwfs");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = make_AtmosphericTurbulence_wavefront_series_cli;
    strcpy(data.cmd[data.NBcmd].info,"make wavefront series");
    strcpy(data.cmd[data.NBcmd].syntax,"<wavelength [nm]> <precision 0=single, 1=double> <computation mode>");
    strcpy(data.cmd[data.NBcmd].example,"mkwfs 1650.0 1 1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int make_AtmosphericTurbulence_wavefront_series(float slambdaum, long WFprecision, int compmode)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"mkvonKarmanWind");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = make_AtmosphericTurbulence_vonKarmanWind_cli;
    strcpy(data.cmd[data.NBcmd].info,"make vonKarman wind model");
    strcpy(data.cmd[data.NBcmd].syntax,"<pixsize> <pixscale [m/pix]> <sigma windspeed [m/s]> <scale [m]> <size [long]> <output name>");
    strcpy(data.cmd[data.NBcmd].example,"mkvonKarmanWind 8192 0.1 20.0 50.0 512 vKmodel");
    strcpy(data.cmd[data.NBcmd].Ccall,"long make_AtmosphericTurbulence_vonKarmanWind(long vKsize, float pixscale, float sigmawind, float Lwind, long size, const char *IDout_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"mkmastert");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_mkmastert_cli;
    strcpy(data.cmd[data.NBcmd].info,"make 2 master phase screens");
    strcpy(data.cmd[data.NBcmd].syntax,"<screen0> <screen1> <size> <outerscale> <innerscale>");
    strcpy(data.cmd[data.NBcmd].example,"mkmastert scr0 scr1 2048 50.0 2.0");
    strcpy(data.cmd[data.NBcmd].Ccall,"int make_master_turbulence_screen(const char *ID_name1, const char *ID_name2, long size, float outercale, float innercale, long WFprecision)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"mkHVturbprof");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_makeHV_CN2prof_cli;
    strcpy(data.cmd[data.NBcmd].info,"make Hufnager-Valley turbulence profile");
    strcpy(data.cmd[data.NBcmd].syntax,"<high wind speed [m/s]> <r0 [m]> <site alt [m]> <NBlayers> <output file>");
    strcpy(data.cmd[data.NBcmd].example,"mkHVturbprof 21.0 0.15 4200 100 turbHV.prof");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AtmosphericTurbulence_makeHV_CN2prof(double wspeed, double r0, double sitealt, long NBlayer, const char *outfile)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"atmturbmeasexpo");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_measure_wavefront_series_expoframes_cli;
    strcpy(data.cmd[data.NBcmd].info,"Measure long exposure time PSF from wavefront series");
    strcpy(data.cmd[data.NBcmd].syntax,"<etime [s]> <out name>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbmeasexpo 1.0 outpsf");
    strcpy(data.cmd[data.NBcmd].Ccall,"int measure_wavefront_series_expoframes(float etime, const char *outfile)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"atmturbmktestTTs");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_mkTestTTseq_cli;
    strcpy(data.cmd[data.NBcmd].info,"make test TT sequence");
    strcpy(data.cmd[data.NBcmd].syntax,"<dt [s]> <number of pts per block> <number of blocks> <measurement noise> <accelerometer mode> <accelerometer noise> <mode>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbmktestTTs 0.001 1000 10 0.1 0 0.0 0");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AtmosphericTurbulence_mkTestTTseq(double dt, long NBpts, long NBblocks, double measnoise, int ACCnmode, double ACCnoise, int MODE)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"atmturbwfpredictf");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_Build_LinPredictor_Full_cli;
    strcpy(data.cmd[data.NBcmd].info,"build full linear predictor from wavefront series");
    strcpy(data.cmd[data.NBcmd].syntax,"<input WF series (cube)> <mask image> <predictor order> <predictor time lag> <SVD eps> <RegLambda>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbwfpredictf wfin wfmask 20 3.5 0.001 0.0");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AtmosphericTurbulence_Build_LinPredictor_Full(const char *WFin_name, const char *WFmask_name, int PForder, float PFlag, double SVDeps, double Rlambda)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"atmturbwfpapply");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_Apply_LinPredictor_Full_cli;
    strcpy(data.cmd[data.NBcmd].info,"Apply full linear predictor from wavefront series");
    strcpy(data.cmd[data.NBcmd].syntax,"<mode> <input WF series (cube)> <mask image> <predictor order> <predictor time lag> <predicted future values> <measured future values>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbwfpapply 0 wfin wfmask 20 3.5 outp outf");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AtmosphericTurbulence_Apply_LinPredictor_Full(int MODE, const char *WFin_name, const char *WFmask_name, int PForder, float PFlag, const char *WFoutp_name, const char *WFoutf_name)");
    data.NBcmd++;

	strcpy(data.cmd[data.NBcmd].key,"atmturbwfp2Dkern");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_LinPredictor_filt_2DKernelExtract_cli;
    strcpy(data.cmd[data.NBcmd].info,"collapse WF predictor into 2D kernel");
    strcpy(data.cmd[data.NBcmd].syntax,"<input WF filter (cube)> <mask image> <kernel radius> <output kernel name>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbwfp2Dkern wfpfilt wfmask 20 wfpkern");
    strcpy(data.cmd[data.NBcmd].Ccall,"long AtmosphericTurbulence_LinPredictor_filt_2DKernelExtract(const char *IDfilt_name, const char *IDmask_name, long krad, const char *IDkern_name)");
	data.NBcmd++;
	
	strcpy(data.cmd[data.NBcmd].key,"atmturbwfpexp");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_LinPredictor_filt_Expand_cli;
    strcpy(data.cmd[data.NBcmd].info,"Expand 3D filter cube into pixel-based 3D cube filters");
    strcpy(data.cmd[data.NBcmd].syntax,"<input WF filter (cube)> <mask image>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbwfpexp wfpfilt wfmask ");
    strcpy(data.cmd[data.NBcmd].Ccall,"long AtmosphericTurbulence_LinPredictor_filt_Expand(const char *IDfilt_name, const char *IDmask_name)");
	data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"atmturbwfpredict");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_Build_LinPredictor_cli;
    strcpy(data.cmd[data.NBcmd].info,"build linear predictor from wavefront series");
    strcpy(data.cmd[data.NBcmd].syntax,"<number steps input> <noise level [rad]> <predictor z size> <predictor xy radius> <lambda [um]>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbwfpredict 1000 0.01 5 5 64 64 1.65");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AtmosphericTurbulence_Build_LinPredictor(long NB_WFstep, double WFphaNoise, long WFP_NBstep, long WFP_xyrad, long WFPiipix, long WFPjjpix, float slambdaum)");
    data.NBcmd++;

	strcpy(data.cmd[data.NBcmd].key,"atmturbmkpsfcc");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_psfCubeContrast_cli;
    strcpy(data.cmd[data.NBcmd].info,"measure contrast performance of WF cube");
    strcpy(data.cmd[data.NBcmd].syntax,"<input WF cube> <mask> <output psf cube>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbmkpsfcc wfc mask psfc");
    strcpy(data.cmd[data.NBcmd].Ccall,"long AtmosphericTurbulence_psfCubeContrast(const char *IDwfc_name, const char *IDmask_name, const char *IDpsfc_name)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"atmturbwfptest");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AtmosphericTurbulence_Test_LinPredictor_cli;
    strcpy(data.cmd[data.NBcmd].info,"Test linear predictor on wavefront series");
    strcpy(data.cmd[data.NBcmd].syntax,"<number steps input> <noise level [rad]> <predictor name> <lag> <iipix> <jjpix>");
    strcpy(data.cmd[data.NBcmd].example,"atmturbwfptest 1000 0.01 wfpfilt 1 32 54");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AtmosphericTurbulence_Test_LinPredictor(long NB_WFstep, double WFphaNoise, const char *IDWFPfilt_name, long WFPlag, long WFPiipix, long WFPjjpix)");
    data.NBcmd++;

    return 0;
}



















int AtmosphericTurbulence_change_configuration_file(const char *fname)
{
  sprintf(CONFFILE, "%s", fname);
  
  return(0);
}






//
// pixscale [m/pix]
// sigmawind [m/s]
// Lwind [m]
//
long make_AtmosphericTurbulence_vonKarmanWind(long vKsize, float pixscale, float sigmawind, float Lwind, long size, const char *IDout_name)
{
    long ID, IDc;
    long ii, jj;
    double dx, dy, r;
    double Lwindpix;
    double rms = 0.0;
    
    double sigmau, sigmav, sigmaw;
    
    IDc = create_3Dimage_ID(IDout_name, vKsize, 1, 3);
    sigmau = sigmawind;
    sigmav = sigmawind;
    sigmaw = sigmawind;
    
    // longitudinal (u)
    make_rnd("tmppha0",vKsize, 1,"");
    arith_image_cstmult("tmppha0", 2.0*PI,"tmppha");
    delete_image_ID("tmppha0");
  
    printf("vK wind outer scale = %f m\n", Lwind);
    printf("pixscale            = %f m\n", pixscale);
    printf("Image size          = %f m\n", vKsize*pixscale);

  
    ID = create_2Dimage_ID("tmpamp0", vKsize, 1);
    for(ii=0; ii<vKsize; ii++)
        {
            dx = 1.0*ii-vKsize/2;
            r = sqrt(dx*dx); // period = (size*pixscale)/r
            // spatial frequency = 2 PI / period
            data.image[ID].array.F[ii] = sqrt( 1.0/pow(1.0 + pow(1.339*2.0*M_PI*r/(vKsize*pixscale)*Lwind,2.0), 5.0/6.0) );
        }
    
    
    make_rnd("tmpg", vKsize, 1,"-gauss");
    arith_image_mult("tmpg", "tmpamp0", "tmpamp");
    save_fits("tmpamp0","!vKwind_tmpamp0.fits");
    delete_image_ID("tmpamp0");
    delete_image_ID("tmpg");
    arith_set_pixel("tmpamp", 0.0, vKsize/2, 0); 
    mk_complex_from_amph("tmpamp", "tmppha", "tmpc", 0);
    delete_image_ID("tmpamp");
    delete_image_ID("tmppha");
    permut("tmpc");
    do2dfft("tmpc","tmpcf");
    delete_image_ID("tmpc");
    mk_reim_from_complex("tmpcf", "tmpo1", "tmpo2", 0);
    delete_image_ID("tmpcf");
    delete_image_ID("tmpo2");
    ID = image_ID("tmpo1");
    rms = 0.0;
    for(ii=0; ii<vKsize; ii++)
        rms += data.image[ID].array.F[ii]*data.image[ID].array.F[ii];
    rms = sqrt(rms/vKsize);
    
    for(ii=0; ii<vKsize; ii++)
        data.image[IDc].array.F[ii] = data.image[ID].array.F[ii]/rms*sigmau;
    delete_image_ID("tmpo1");

  
     // tangential (v)
    make_rnd("tmppha0", vKsize, 1, "");
    arith_image_cstmult("tmppha0", 2.0*PI,"tmppha");
    delete_image_ID("tmppha0");
  
    ID = create_2Dimage_ID("tmpamp0", vKsize, 1);
    for(ii=0; ii<vKsize; ii++)
        {
            dx = 1.0*ii-vKsize/2;
            r = sqrt(dx*dx); 
            data.image[ID].array.F[ii] = sqrt( (1.0 + 8.0/3.0*pow(2.678*2.0*M_PI*r/(vKsize*pixscale)*Lwind,2.0)) /pow(1.0 + pow(2.678*2.0*M_PI*r/(vKsize*pixscale)*Lwind,2.0), 11.0/6.0) );
        }
    make_rnd("tmpg", vKsize, 1,"-gauss");
    arith_image_mult("tmpg", "tmpamp0", "tmpamp");
    delete_image_ID("tmpamp0");
    delete_image_ID("tmpg");
    arith_set_pixel("tmpamp", 0.0, vKsize/2, 0);
    mk_complex_from_amph("tmpamp", "tmppha", "tmpc", 0);
    delete_image_ID("tmpamp");
    delete_image_ID("tmppha");
    permut("tmpc");
    do2dfft("tmpc","tmpcf");
    delete_image_ID("tmpc");
    mk_reim_from_complex("tmpcf", "tmpo1", "tmpo2", 0);
    delete_image_ID("tmpcf");
    delete_image_ID("tmpo2");
    ID = image_ID("tmpo1");
    rms = 0.0;
    for(ii=0; ii<vKsize; ii++)
        rms += data.image[ID].array.F[ii]*data.image[ID].array.F[ii];
    rms = sqrt(rms/(vKsize));
    
    for(ii=0; ii<vKsize; ii++)
        data.image[IDc].array.F[vKsize+ii] = data.image[ID].array.F[ii]/rms*sigmav;
    delete_image_ID("tmpo1");
      
      
      
      
    // vertical (w)
    make_rnd("tmppha0", vKsize, 1, "");
    arith_image_cstmult("tmppha0", 2.0*PI,"tmppha");
    delete_image_ID("tmppha0");
  
    ID = create_2Dimage_ID("tmpamp0", vKsize, 1);
    for(ii=0; ii<vKsize; ii++)
        {
            dx = 1.0*ii-size/2;
            r = sqrt(dx*dx);
            data.image[ID].array.F[ii] = sqrt( (1.0 + 8.0/3.0*pow(2.678*2.0*M_PI*r/(vKsize*pixscale)*Lwind,2.0)) /pow(1.0 + pow(2.678*2.0*M_PI*r/(vKsize*pixscale)*Lwind,2.0), 11.0/6.0) );
        }
    make_rnd("tmpg", vKsize, 1,"-gauss");
    arith_image_mult("tmpg", "tmpamp0", "tmpamp");
    delete_image_ID("tmpamp0");
    delete_image_ID("tmpg");
    arith_set_pixel("tmpamp", 0.0, vKsize/2, 0); 
    mk_complex_from_amph("tmpamp", "tmppha", "tmpc", 0);
    delete_image_ID("tmpamp");
    delete_image_ID("tmppha");
    permut("tmpc");
    do2dfft("tmpc","tmpcf");
    delete_image_ID("tmpc");
    mk_reim_from_complex("tmpcf", "tmpo1", "tmpo2", 0);
    delete_image_ID("tmpcf");
    delete_image_ID("tmpo2");
    ID = image_ID("tmpo1");
    rms = 0.0;
    for(ii=0; ii<vKsize; ii++)
        rms += data.image[ID].array.F[ii]*data.image[ID].array.F[ii];
    rms = sqrt(rms/vKsize);
    
    for(ii=0; ii<size; ii++)
        data.image[IDc].array.F[vKsize*2+ii] = data.image[ID].array.F[ii]/rms*sigmav;
    delete_image_ID("tmpo1");

    
    
    
    return(IDc);
}




//
// innerscale and outerscale in pixel
// von Karman spectrum
//
int make_master_turbulence_screen(const char *ID_name1, const char *ID_name2, long size, float outerscale, float innerscale, long WFprecision)
{
    long ID,ii,jj;
    double value,C1,C2;
    long cnt;
    long Dlim = 3;
    long IDv;

    int OUTERSCALE_MODE = 1; // 1 if outer scale
    double OUTERscale_f0;
    double INNERscale_f0;
    double dx, dy, r;
    double rlim = 0.0;
    int RLIMMODE = 0;
    double iscoeff;


    printf("Make turbulence screen, precision = %ld\n", WFprecision);
    fflush(stdout);

    /*  IDv = variable_ID("OUTERSCALE");
      if(IDv!=-1)
        {
          outerscale = data.variable[IDv].value.f;
          printf("Outer scale = %f pix\n", outerscale);
        }
     */

    IDv = variable_ID("RLIM");
    if(IDv!=-1)
    {
        RLIMMODE = 1;
        rlim = data.variable[IDv].value.f;
        printf("R limit = %f pix\n",rlim);
    }

    OUTERscale_f0 = 1.0*size/outerscale; // [1/pix] in F plane
    INNERscale_f0 = (5.92/(2.0*M_PI))*size/innerscale;

    if(WFprecision==0)
        make_rnd("tmppha", size, size, "");
    else
        make_rnd_double("tmppha", size, size, "");



    arith_image_cstmult("tmppha", 2.0*PI,"tmppha1");
    delete_image_ID("tmppha");
    //  make_dist("tmpd",size,size,size/2,size/2);
	if(WFprecision==0)
		ID = create_2Dimage_ID("tmpd",size, size);
	else
		ID = create_2Dimage_ID_double("tmpd",size, size);

    if(WFprecision==0)
    {
        for(ii=0; ii<size; ii++)
            for(jj=0; jj<size; jj++)
            {
                dx = 1.0*ii-size/2;
                dy = 1.0*jj-size/2;

                if(RLIMMODE==1)
                {
                    r = sqrt(dx*dx + dy*dy);
                    if(r<rlim)
                        data.image[ID].array.F[jj*size+ii] = 0.0;
                    else
                        data.image[ID].array.F[jj*size+ii] = sqrt(dx*dx + dy*dy + OUTERscale_f0*OUTERscale_f0);
                }
                else
                    data.image[ID].array.F[jj*size+ii] = sqrt(dx*dx + dy*dy + OUTERscale_f0*OUTERscale_f0);
            }
    }
    else
    {
        for(ii=0; ii<size; ii++)
            for(jj=0; jj<size; jj++)
            {
                dx = 1.0*ii-size/2;
                dy = 1.0*jj-size/2;

                if(RLIMMODE==1)
                {
                    r = sqrt(dx*dx + dy*dy);
                    if(r<rlim)
                        data.image[ID].array.D[jj*size+ii] = 0.0;
                    else
                        data.image[ID].array.D[jj*size+ii] = sqrt(dx*dx + dy*dy + OUTERscale_f0*OUTERscale_f0);
                }
                else
                    data.image[ID].array.D[jj*size+ii] = sqrt(dx*dx + dy*dy + OUTERscale_f0*OUTERscale_f0);
            }
    }



    if(WFprecision==0)
        make_rnd("tmpg", size, size, "-gauss");
    else
        make_rnd_double("tmpg", size, size, "-gauss");


	

    ID = image_ID("tmpg");
    if(WFprecision==0)
	{
		for(ii=0; ii<size; ii++)
			for(jj=0; jj<size; jj++)
			{
				dx = 1.0*ii-size/2;
				dy = 1.0*jj-size/2;
				iscoeff = exp(-(dx*dx+dy*dy)/INNERscale_f0/INNERscale_f0);
				data.image[ID].array.F[jj*size+ii] *= sqrt(iscoeff); // power -> amplitude : sqrt
			}
	}
	else
	{
		for(ii=0; ii<size; ii++)
			for(jj=0; jj<size; jj++)
			{
				dx = 1.0*ii-size/2;
				dy = 1.0*jj-size/2;
				iscoeff = exp(-(dx*dx+dy*dy)/INNERscale_f0/INNERscale_f0);
				data.image[ID].array.D[jj*size+ii] *= sqrt(iscoeff); // power -> amplitude : sqrt
			}
	}


    arith_image_cstpow("tmpd", 11.0/6.0, "tmpd1");
    delete_image_ID("tmpd");
    arith_image_div("tmpg", "tmpd1", "tmpamp");
    delete_image_ID("tmpg");
    delete_image_ID("tmpd1");
    arith_set_pixel("tmpamp", 0.0, size/2, size/2);
    mk_complex_from_amph("tmpamp", "tmppha1", "tmpc", 0);
    delete_image_ID("tmpamp");
    delete_image_ID("tmppha1");
    permut("tmpc");

    do2dfft("tmpc","tmpcf");
    delete_image_ID("tmpc");
    mk_reim_from_complex("tmpcf", "tmpo1", "tmpo2", 0);
    delete_image_ID("tmpcf");


    /* compute the scaling factor in the power law of the structure function */
    fft_structure_function("tmpo1", "strf");
    ID = image_ID("strf");

    value = 0.0;
    cnt = 0;
    if(data.image[ID].md[0].atype == FLOAT)
    {
        for(ii = 1; ii<Dlim; ii++)
            for(jj = 1; jj<Dlim; jj++)
            {
                value += log10(data.image[ID].array.F[jj*size+ii])-5.0/3.0*log10(sqrt(ii*ii+jj*jj));
                cnt++;
            }
    }
    else
    {
        for(ii = 1; ii<Dlim; ii++)
            for(jj = 1; jj<Dlim; jj++)
            {
                value += log10(data.image[ID].array.D[jj*size+ii])-5.0/3.0*log10(sqrt(ii*ii+jj*jj));
                cnt++;
            }
    }
    // save_fl_fits("strf","!strf.fits");
    delete_image_ID("strf");
    C1 = pow(10.0,value/cnt);




    fft_structure_function("tmpo2", "strf");
    ID = image_ID("strf");
    value = 0.0;
    cnt = 0;
    if(data.image[ID].md[0].atype == FLOAT)
    {
        for(ii=1; ii<Dlim; ii++)
            for(jj=1; jj<Dlim; jj++)
            {
                value += log10(data.image[ID].array.F[jj*size+ii])-5.0/3.0*log10(sqrt(ii*ii+jj*jj));
                cnt++;
            }
    }
    else
    {
        for(ii=1; ii<Dlim; ii++)
            for(jj=1; jj<Dlim; jj++)
            {
                value += log10(data.image[ID].array.D[jj*size+ii])-5.0/3.0*log10(sqrt(ii*ii+jj*jj));
                cnt++;
            }
    }

    delete_image_ID("strf");
    C2 = pow(10.0,value/cnt);

    printf("C1, C2 =   %f %f\n", C1, C2);

    arith_image_cstmult("tmpo1", 1.0/sqrt(C1),ID_name1);
    arith_image_cstmult("tmpo2", 1.0/sqrt(C2),ID_name2);
    delete_image_ID("tmpo1");
    delete_image_ID("tmpo2");

    return(0);
}






int make_master_turbulence_screen_pow(const char *ID_name1, const char *ID_name2, long size, float power)
{
    long ID,ii,jj;
    float value,C1,C2;
    long cnt;
    long Dlim = 3;

    make_rnd("tmppha",size,size,"");
    arith_image_cstmult("tmppha",2.0*PI,"tmppha1");
    delete_image_ID("tmppha");
    make_dist("tmpd",size,size,size/2,size/2);
    make_rnd("tmpg",size,size,"-gauss");

    arith_image_cstpow("tmpd",power,"tmpd1");
    delete_image_ID("tmpd");
    arith_image_div("tmpg","tmpd1","tmpamp");
    delete_image_ID("tmpg");
    delete_image_ID("tmpd1");
    arith_set_pixel("tmpamp",0.0,size/2,size/2);
    mk_complex_from_amph("tmpamp","tmppha1","tmpc", 0);
    delete_image_ID("tmpamp");
    delete_image_ID("tmppha1");
    permut("tmpc");
    do2dfft("tmpc","tmpcf");
    delete_image_ID("tmpc");
    mk_reim_from_complex("tmpcf","tmpo1","tmpo2", 0);
    delete_image_ID("tmpcf");

    /* compute the scaling factor in the power law of the structure function */
    fft_structure_function("tmpo1","strf");
    ID=image_ID("strf");
    value = 0.0;
    cnt = 0;
    for(ii=1; ii<Dlim; ii++)
        for(jj=1; jj<Dlim; jj++)
        {
            value += log10(data.image[ID].array.F[jj*size+ii])-power*log10(sqrt(ii*ii+jj*jj));
            /*	printf("%ld %ld %f\n",ii,jj,log10(data.image[ID].array.F[jj*size+ii])-5.0/3.0*log10(sqrt(ii*ii+jj*jj)));*/
            cnt++;
        }
    delete_image_ID("strf");
    C1=pow(10.0,value/cnt);

    fft_structure_function("tmpo2","strf");
    ID=image_ID("strf");
    value = 0.0;
    cnt = 0;
    for(ii=1; ii<Dlim; ii++)
        for(jj=1; jj<Dlim; jj++)
        {
            value += log10(data.image[ID].array.F[jj*size+ii])-power*log10(sqrt(ii*ii+jj*jj));
            cnt++;
        }
    delete_image_ID("strf");
    C2=pow(10.0,value/cnt);
    /*  printf("%f %f\n",C1,C2);*/
    arith_image_cstmult("tmpo1",1.0/sqrt(C1),ID_name1);
    arith_image_cstmult("tmpo2",1.0/sqrt(C2),ID_name2);
    delete_image_ID("tmpo1");
    delete_image_ID("tmpo2");

    return(0);
}






int contract_wavefront_cube(const char *ina_file, const char *inp_file, const char *outa_file, const char *outp_file, int factor)
{
    /* contracts the wavefront series by a factor of 2^factor */
    long IDamp,IDpha,IDoutamp,IDoutpha;
    long ii,jj,kk;
    long i,j;
    long naxes[3];
    long naxes_out[3];
    float re, im, amp, pha;
    float P;
    long LARGE = 10000;
    float pharef, ampref;
    int pfactor;

    pfactor=1;
    for(i=0; i<factor; i++)
        pfactor *= 2;

    load_fits(inp_file, "tmpwfp", 1);
    IDpha=image_ID("tmpwfp");
    load_fits(ina_file, "tmpwfa", 1);
    IDamp=image_ID("tmpwfa");
    naxes[0] = data.image[IDpha].md[0].size[0];
    naxes[1] = data.image[IDpha].md[0].size[1];
    naxes[2] = data.image[IDpha].md[0].size[2];
    naxes_out[0] = data.image[IDpha].md[0].size[0]/pfactor;
    naxes_out[1] = data.image[IDpha].md[0].size[1]/pfactor;
    naxes_out[2] = data.image[IDpha].md[0].size[2];
    IDoutpha = create_3Dimage_ID("tmpwfop",naxes_out[0],naxes_out[1],naxes_out[2]);
    IDoutamp = create_3Dimage_ID("tmpwfoa",naxes_out[0],naxes_out[1],naxes_out[2]);

    ii=0;
    jj=0;
    kk=0;
    amp = 0.0;
    pha = 0.0;

    //  # ifdef _OPENMP
    //  #pragma omp parallel
    //  {
    //  # endif

    //  # ifdef _OPENMP
    //  #pragma omp for private(kk,ii,jj,i,j,amp,pha,ampref,pharef,re,im,P) collapse(3)
    //  # endif
    for(kk=0; kk<naxes[2]; kk++)
    {
        for(ii=0; ii<naxes[0]/pfactor; ii++)
            for(jj=0; jj<naxes[1]/pfactor; jj++)
            {
                re=0.0;
                im=0.0;
                pharef = 0.0;
                ampref = 0.0;
                for(i=0; i<pfactor; i++)
                    for(j=0; j<pfactor; j++)
                    {
                        amp = data.image[IDamp].array.F[kk*naxes[0]*naxes[1]+(pfactor*jj+j)*naxes[0]+pfactor*ii+i];
                        pha = data.image[IDpha].array.F[kk*naxes[0]*naxes[1]+(pfactor*jj+j)*naxes[0]+pfactor*ii+i];
                        pharef += data.image[IDamp].array.F[kk*naxes[0]*naxes[1]+(pfactor*jj+j)*naxes[0]+pfactor*ii+i]*data.image[IDpha].array.F[kk*naxes[0]*naxes[1]+(pfactor*jj+j)*naxes[0]+pfactor*ii+i];
                        ampref += data.image[IDamp].array.F[kk*naxes[0]*naxes[1]+(pfactor*jj+j)*naxes[0]+pfactor*ii+i];
                        re += amp*cos(pha);
                        im += amp*sin(pha);
                    }
                amp = sqrt(re*re+im*im);
                pha = atan2(im,re);
                pharef /= ampref;
                P = 2.0*PI*( ((long) (0.5+1.0*LARGE+(pharef-pha)/2.0/PI)) - LARGE);
                if(ampref<0.01)
                    P = 0.0;
                data.image[IDoutpha].array.F[kk*naxes_out[0]*naxes_out[1]+jj*naxes_out[0]+ii] = pha+P;
                data.image[IDoutamp].array.F[kk*naxes_out[0]*naxes_out[1]+jj*naxes_out[0]+ii] = amp/pfactor/pfactor;
            }
    }

    //  # ifdef _OPENMP
    //  }
    //  # endif


    save_fl_fits("tmpwfop",outp_file);
    save_fl_fits("tmpwfoa",outa_file);

    delete_image_ID("tmpwfa");
    delete_image_ID("tmpwfp");
    delete_image_ID("tmpwfoa");
    delete_image_ID("tmpwfop");


    return(0);
}


int contract_wavefront_cube_phaseonly(const char *inp_file, const char *outp_file, int factor)
{
    /* contracts the wavefront series by a factor of 2^factor */
    long IDpha,IDoutpha;
    long ii,jj,kk;
    long i,j;
    long naxes[3];
    long naxes_out[3];
    float re,im,amp,pha;
    float P;
    long LARGE = 10000;
    float pharef,ampref;
    int pfactor;
    long l1,l2,l3,l4;

    //  printf("CONTRACT, FACTOR = %d\n",factor);
    //fflush(stdout);

    pfactor=1;
    for(i=0; i<factor; i++)
        pfactor *= 2;

    load_fits(inp_file, "tmpwfp", 1);
    IDpha=image_ID("tmpwfp");
    naxes[0] = data.image[IDpha].md[0].size[0];
    naxes[1] = data.image[IDpha].md[0].size[1];
    naxes[2] = data.image[IDpha].md[0].size[2];
    naxes_out[0] = data.image[IDpha].md[0].size[0]/pfactor;
    naxes_out[1] = data.image[IDpha].md[0].size[1]/pfactor;
    naxes_out[2] = data.image[IDpha].md[0].size[2];
    IDoutpha = create_3Dimage_ID("tmpwfop",naxes_out[0],naxes_out[1],naxes_out[2]);

    ii=0;
    jj=0;
    kk=0;
    amp = 0.0;
    pha = 0.0;
    ampref = 1.0*pfactor*pfactor;

# ifdef _OPENMP
    #pragma omp parallel for shared(naxes,naxes_out,data,pfactor,IDoutpha,IDpha,ampref) private(kk,ii,jj,i,j,amp,pha,pharef,re,im,P,l1,l2,l3,l4)
# endif
    for(kk=0; kk<naxes[2]; kk++)
    {
        l1 = kk*naxes_out[0]*naxes_out[1];
        l2 = kk*naxes[0]*naxes[1];
        for(ii=0; ii<naxes[0]/pfactor; ii++)
            for(jj=0; jj<naxes[1]/pfactor; jj++)
            {
                re=0.0;
                im=0.0;
                pharef = 0.0;
                l4=l2+pfactor*ii;
                for(j=0; j<pfactor; j++)
                {
                    l3=l4+(pfactor*jj+j)*naxes[0];
                    for(i=0; i<pfactor; i++)
                    {
                        pha = data.image[IDpha].array.F[l3+i];
                        pharef += data.image[IDpha].array.F[l3+i];
                        re += cos(pha);
                        im += sin(pha);
                    }
                }
                /*	    amp = sqrt(re*re+im*im);*/
                pha = atan2(im,re);
                pharef /= ampref;
                P = 2.0*PI*( ((long) (0.5+1.0*LARGE+(pharef-pha)/2.0/PI)) - LARGE);
                if(ampref<0.01)
                    P = 0.0;
                data.image[IDoutpha].array.F[l1+jj*naxes_out[0]+ii] = pha+P;
            }

    }

    save_fl_fits("tmpwfop",outp_file);

    delete_image_ID("tmpwfp");
    delete_image_ID("tmpwfop");


    return(0);
}


// P [Pa]
// T [C]
// RH [%]

// ref: Techniques and Topics in Flow Measurement
// By Frank E. Jones
//
// coeffs An, Bn, Cn adopted from Nitrogen
//
// Compressibility factors:
// Davies 1991
//
// returns compressibility
// rhocoeff is the (proportional) increase of particle density vs. STP 
//
double Z_Air(double P, double T, double RH)
{
    double Z, Z0, rho, rho0;
    double tc; // C
    double rhocoeff1;
    double An = 4.446e-6; 
    double Bn = 6.4e-13;
    double Cn = -1.07e-16;
    double P0 = 101325.0;
    
    double TK;
    double f; // enhancement factor
    double Psv; // water vapor saturation
    double xv;
    
    double A = 1.2378847e-5;
    double B = -1.9121316e-2;
    double C = 33.93711047;
    double D = -6.3431645e3;
    
    double alpha = 1.00062;
    double beta = 3.14e-8;
    double gamma = 5.6e-7;
    
    double a0 = 1.58123e-6;
    double a1 = -2.9331e-8;
    double a2 = 1.1043e-10;
    double b0 = 5.707e-6;
    double b1 = -2.051e-8;
    double c0 = 1.9898e-4;
    double c1 = -2.376e-6;
    double d = 1.83e-11;
    double e = -0.765e-8;
    
    double RH0 = 0.0;
    double T0, TK0, f0, Psv0, xv0;
    
    TK = T + 273.15;

    Z = 1.00001 - 5.8057e-9*P + 2.6402e-16*P*P - 3.3297e-7*T + 1.2420e-10*P*T - 2.0158e-18*P*P*T + 2.4925e-9*T*T - 6.2873e-13*P*T*T + 5.4174e-21*P*P*T*T - 3.5e-7*RH - 5.0e-9*RH*RH;
    
    Psv = exp(A*TK*TK + B*TK + C + D/TK);
    printf("Water vapor saturation pressure Psv0 = %f Pa\n", Psv);
    f = alpha + beta*P + gamma*T*T;
    printf("enhancement factor f = %f\n", f);
    xv = RH*0.01 * f * Psv/P;
    Z = 1.0 - P/TK * (a0 + a1*T + a2*T*T + (c0+b1*T)*xv + (c0+c1*T)*xv*xv) + P*P/TK/TK*(d+e*xv*xv);
    
    Z0 = 1.00001 - 5.8057e-9*P0 + 2.6402e-16*P0*P0;
    
    T0 = 0.0;
    TK0 = 273.15;
    Psv0 = exp(A*TK0*TK0 + B*TK0 + C + D/TK0);
    printf("Water vapor saturation pressure Psv0 = %f Pa\n", Psv0);
    f0 = alpha + beta*P0 + gamma*T0*T0;
    printf("enhancement factor f0 = %f\n", f0);
    xv0 = RH0*0.01 * f0 * Psv0/P0;
    Z0 = 1.0 - P0/TK0 * (a0 + a1*T0 + a2*T0*T0 + (c0+b1*T0)*xv0 + (c0+c1*T0)*xv0*xv0) + P0*P0/TK0/TK0*(d+e*xv0*xv0);
    
    
    rhocoeff1 = 101325.0/P;
    rhocoeff1 *= (T+273.15)/273.15;  // particle count ratio : more particles at lower temp
    rhocoeff1 *= Z/Z0;
    
    rho0 = C_ls/C_Na; // [mol.m^-3]
    
    rhocoeff = rhocoeff1;
    
    printf("Z = %.8f   Z0 = %.8f\n", Z, Z0);
    
    rho = rho0/rhocoeff1;
    rhocoeff = rhocoeff1; // * (1.0 + Bn/An*rho0 + Cn/An*rho0*rho0) / (1.0 + Bn/An*rho + Cn/An*rho*rho) / (1.0 + Bn/An*rho + Cn/An*rho*rho);
                
    return(Z);
}




double Z_N2(double P, double T)
{
    double Z, Z0, rho, rho0;
    double tc; // C
    double rhocoeff1;
    double An = 4.446e-6; // Achterman et al. 1986
    double Bn = 6.4e-13;
    double Cn = -1.07e-16;
    
    tc = T-273.15;

    // J. D. Wright, “Gas property equations for the NIST fluid flow group gas flow measurement calibration services” (National Institute of Standards and Technology, 2004), http://www.cstl.nist.gov/div836/836.01/PDFs/2004/Gas_Properties.pdf.)
    Z = 1.0 - 101325.0 * (P/101325.0) * (0.449805 - 0.01177*tc + 0.00006*tc*tc)*1e-8;
    Z0 = 1.0 - 101325.0 * (0.449805)*1e-8;
    
    rhocoeff1 = 101325.0/P;
    rhocoeff1 *= T/273.15;  // particle count ratio : more particles at lower temp
    rhocoeff1 *= Z/Z0;
    rho0 = C_ls/C_Na; // [mol.m^-3]
    rho = rho0/rhocoeff1;
    rhocoeff = rhocoeff1 * (1.0 + Bn/An*rho0 + Cn/An*rho0*rho0) / (1.0 + Bn/An*rho + Cn/An*rho*rho) / (1.0 + Bn/An*rho + Cn/An*rho*rho);
                
    return(Z);
}






int AtmosphericTurbulence_ReadConf()
{
    char KEYWORD[200];
    char CONTENT[200];
    
    
   // ------------ TURBULENCE AND ATMOSPHERE PARAMETERS -------------------------------------------
    
    strcpy(KEYWORD,"TURBULENCE_REF_WAVEL");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_LAMBDA = atof(CONTENT)*0.000001;
    
    strcpy(KEYWORD,"TURBULENCE_SEEING");
    read_config_parameter(CONFFILE,KEYWORD,CONTENT);
    CONF_SEEING = atof(CONTENT);

    strcpy(KEYWORD,"TURBULENCE_PROF_FILE");
    read_config_parameter(CONFFILE, KEYWORD, CONF_TURBULENCE_PROF_FILE);
 
    strcpy(KEYWORD,"ZENITH_ANGLE");
    read_config_parameter(CONFFILE,KEYWORD,CONTENT);
    CONF_ZANGLE = atof(CONTENT);

    strcpy(KEYWORD,"SOURCE_XPOS");
    read_config_parameter(CONFFILE,KEYWORD,CONTENT);
    CONF_SOURCE_Xpos = atof(CONTENT);

    strcpy(KEYWORD,"SOURCE_YPOS");
    read_config_parameter(CONFFILE,KEYWORD,CONTENT);
    CONF_SOURCE_Ypos = atof(CONTENT);

 
    
    // ------------ LOW_FIDELITY OUTPUT AT REF LAMBDA ------------------------------------------------
    
    strcpy(KEYWORD,"WFOUTPUT");
    if(read_config_parameter_exists(CONFFILE, KEYWORD)==1)
    {
        read_config_parameter(CONFFILE,KEYWORD, CONTENT);
        CONF_WFOUTPUT = atoi(CONTENT);
    }
    
    strcpy(KEYWORD,"WF_FILE_PREFIX");
    read_config_parameter(CONFFILE,KEYWORD, CONF_WF_FILE_PREFIX);

    strcpy(KEYWORD,"SHM_OUTPUT");
    if(read_config_parameter_exists(CONFFILE, KEYWORD)==1)
    {
        read_config_parameter(CONFFILE, KEYWORD, CONTENT);
        CONF_SHM_OUTPUT = atoi(CONTENT);
    }

    
    // ------------- HIGH FIDELITY OUTPUT WAVELENGTH ---------------------------------
    
    strcpy(KEYWORD,"MAKE_SWAVEFRONT");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_MAKE_SWAVEFRONT = atoi(CONTENT);

//    strcpy(KEYWORD,"SLAMBDA");
 //   read_config_parameter(CONFFILE, KEYWORD, CONTENT);
 //   CONF_SLAMBDA = atof(CONTENT)*0.000001;

    strcpy(KEYWORD,"SWF_WRITE2DISK");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_SWF_WRITE2DISK = atoi(CONTENT);

    strcpy(KEYWORD,"SWF_FILE_PREFIX");
    read_config_parameter(CONFFILE, KEYWORD, CONF_SWF_FILE_PREFIX);

    strcpy(KEYWORD,"SHM_SOUTPUT");
    if(read_config_parameter_exists(CONFFILE, KEYWORD)==1)
    {
        read_config_parameter(CONFFILE, KEYWORD, CONTENT);
        CONF_SHM_SOUTPUT = atoi(CONTENT);
    }
    strcpy(KEYWORD,"SHM_SPREFIX");
    read_config_parameter(CONFFILE,KEYWORD, CONF_SHM_SPREFIX);

    strcpy(KEYWORD,"SHM_SOUTPUTM");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_SHM_SOUTPUTM = atoi(CONTENT);


    // ------------ OUTPUT PARAMETERS --------------------------------------------
    
    strcpy(KEYWORD,"PUPIL_SCALE");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_PUPIL_SCALE = atof(CONTENT);
    
    strcpy(KEYWORD,"WFsize");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_WFsize = atol(CONTENT);

 
    
    // ------------- TIMING ---------------------------------------
    
    strcpy(KEYWORD,"REALTIME");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_ATMWF_REALTIME = atoi(CONTENT);
    
    strcpy(KEYWORD,"REALTIMEFACTOR");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_ATMWF_REALTIMEFACTOR = atof(CONTENT);

    strcpy(KEYWORD, "WFTIME_STEP");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_WFTIME_STEP = atof(CONTENT);

    strcpy(KEYWORD, "TIME_SPAN");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_TIME_SPAN = atof(CONTENT);

    strcpy(KEYWORD, "NB_TSPAN");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_NB_TSPAN = atol(CONTENT);

    strcpy(KEYWORD, "SIMTDELAY");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_SIMTDELAY = atol(CONTENT);

    
    strcpy(KEYWORD, "WAITFORSEM");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_WAITFORSEM = atoi(CONTENT);
    
    strcpy(KEYWORD, "WAITSEMIMNAME");
    read_config_parameter(CONFFILE, KEYWORD, CONF_WAITSEMIMNAME);
    
    
    
    // ------------ COMPUTATION PARAMETERS, MODES --------------------------------
    
    
    strcpy(KEYWORD,"SKIP_EXISTING");
    if(read_config_parameter_exists(CONFFILE,KEYWORD)==1)
    {
        read_config_parameter(CONFFILE,KEYWORD,CONTENT);
        CONF_SKIP_EXISTING = 1;
    }
    else
        CONF_SKIP_EXISTING = 0;

    strcpy(KEYWORD,"WF_RAW_SIZE");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_WF_RAW_SIZE = atol(CONTENT);
 
    strcpy(KEYWORD,"MASTER_SIZE");
    read_config_parameter(CONFFILE, KEYWORD,CONTENT);
    CONF_MASTER_SIZE = atol(CONTENT);



    //  ------------ WAVEFRONT AMPLITUDE -------------------------------------------

    
    strcpy(KEYWORD,"WAVEFRONT_AMPLITUDE");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_WAVEFRONT_AMPLITUDE = atoi(CONTENT);

	if(CONF_WAVEFRONT_AMPLITUDE == 1)
	{
		strcpy(KEYWORD,"FRESNEL_PROPAGATION");
		read_config_parameter(CONFFILE, KEYWORD, CONTENT);
		CONF_FRESNEL_PROPAGATION = atoi(CONTENT);
	}
	else
		CONF_FRESNEL_PROPAGATION = 0;
		
    strcpy(KEYWORD,"FRESNEL_PROPAGATION_BIN");
    read_config_parameter(CONFFILE, KEYWORD, CONTENT);
    CONF_FRESNEL_PROPAGATION_BIN = atof(CONTENT);

    // ------------ POSTPROCESSING --------------
 

    strcpy(KEYWORD,"PUPIL_AMPL_FILE");
    if(read_config_parameter_exists(CONFFILE,KEYWORD)==1)
    {
        read_config_parameter(CONFFILE, KEYWORD, CONTENT);
        load_fits(CONTENT, "ST_pa", 1);
    }

    strcpy(KEYWORD,"PUPIL_PHA_FILE");
    if(read_config_parameter_exists(CONFFILE,KEYWORD)==1)
    {
        read_config_parameter(CONFFILE,KEYWORD,CONTENT);
        load_fits(CONTENT, "ST_pp", 1);
    }


    
    return 0;
}





// compmode = 0 : compute atmosphere model only, no turbulence
// compmode = 1 : full computation

int make_AtmosphericTurbulence_wavefront_series(float slambdaum, long WFprecision, int compmode)
{
    //  fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
    int status;
    long  fpixel = 1, naxis = 3, nelement;

    long naxes_MASTER[2];
    /*  long ID;*/
    long ID1,ID2;
    long tspan;
    char fname_a[200];
    char fname1[200];
    char fname1_a[200];
    char fname2[200];
    char tmpafname[200];
    char tmppfname[200];

    // float SODIUM_ALT;
    double layer_scale = 1.0;

    long master;
    long NBMASTERS;
    long *ID_TM;
    long *ID_TML;
    long IDout_array_pha;
    long IDout_array_amp;
    complex_float *array;
    complex_double *array_double;

    // phase only
    long ID_array1;
    long ID_sarray1;
    long ID_carray1;

    // phase + amplitude
    long ID_array2;
    long ID_sarray2;
    long ID_carray2;

    double re,im;

    double SLAMBDA; // wagvelength [um]

    long IDout_sarray_pha;
    long IDout_sarray_amp;
    complex_float *sarray;
    complex_double *sarray_double;
    double Scoeff;
    double Nlambda, Nslambda, l;


    // cone effect wavefront
    /*
        int make_cwavefront = 0;
        char CWF_FILE_PREFIX[100];
        complex_float *carray;
    */

    long NBLAYERS; /* number of layers */
    long layer;
    FILE *fp;
    char command[200];
    char line[2000];
    char word[200];
    char fname[200];
    char name[200];
    double *LAYER_ALT;
    double *LAYER_CN2;
    double *LAYER_SPD;
    double *LAYER_DIR;
    double *LAYER_OUTERSCALE;
    double *LAYER_INNERSCALE;
    double *LAYER_SIGMAWSPEED;
    double *LAYER_LWIND;
    int stop;

    long *naxes;
    long *naxesout;
    double *xpos;
    double *ypos;
    double *xpos0;
    double *ypos0;
    long *xposfcnt; // counter to keep trak of modulo
    long *yposfcnt;
    double *vxpix;
    double *vypix;
    double vpix,PA;
    long vindex;
    long frame;
    long NBFRAMES;
    double fl1, fl2, fl3, fl4, fl5, fl6, fl7, fl8;
    long ii, jj, iim, jjm, ii1, jj1;
    double value;
    double coeff = 0.0;

    double *alt_bin_sep;
    long NB_alt_bin_sep;
    double minaltd;
    double *SLAYER_ALT;
    double *SLAYER_CN2;
    long NBSLAYERS;
    int OK;
    long i,j,index;
    long *super_layer_index;

    int contraction_factor;
    int pfactor;
    //   float OuterScale;
    double CN2total;
    double tmp, tmpf, h, Rindex, Roffset, RoffsetS;

    long xref,yref,xrefm,yrefm,iimax,jjmax,y1;
    long ID;
    long start_tspan=0;

    double tot,p1,p2,tmp1;
    long dpix,cnt,k,r0cnt;
    double r0,r0tot;

    long IDshmpha, IDshmamp;
    long IDshmspha, IDshmsamp;

    //
    // US standard atmosphere density, normalized to sea level, 2km step
    // log10 of density, 2km step
    //
    double StdAtmDens[44] = {0.000000, -0.085107, -0.174693, -0.268526, -0.367316, -0.471661, -0.594121, -0.730392, -0.866722, -1.003203, -1.139185, -1.278509, -1.416593, -1.553349, -1.688809, -1.823082, -1.956197, -2.093072, -2.227379, -2.358485, -2.486619, -2.611739, -2.734220, -2.854125, -2.968550, -3.076566, -3.182071, -3.282703, -3.385361, -3.490222, -3.597335, -3.706660, -3.818623, -3.933104, -4.050311, -4.170053, -4.293230, -4.421899, -4.552842, -4.686219, -4.822140, -4.960707, -5.101812, -5.245839};
    double dens_N2, dens_O2, dens_Ar, dens_H2O, dens_CO2, dens_Ne, dens_He, dens_CH4, dens_Kr, dens_H2, dens_O3, dens_N, dens_O, dens_H; // [cm-3]
    double lambda;
    double xN2, xO2, xAr, xH2O, xCO2, xNe, xHe, xCH4, xKr, xH2, xO3, xN, xO, xH;
    double P, T, TC, Pw, CO2ppm, denstot, xtot;
    double LoschmidtConstant =  2.6867805e25;
    double logD;
    double x;
    long i0,i1;
    double iimf, jjmf, iifrac, jjfrac, value_re, value_im;
    double pha;

    int r;
    double Temp;
    double RH;
    double Z, Z0;


    // timing
    struct timespec tnow;
    double tnowdouble;
    int kw;


    // optimal phase unwrapping
    double peakpha_re, peakpha_im, peakpha;
    long IDpeakpha_re, IDpeakpha_im, IDpeakpha;
    long IDpeakpha_re_bin, IDpeakpha_im_bin, IDpeakpha_bin, IDpeakpha_bin_ch;
    long xsizepeakpha;
    long ysizepeakpha;
    long ii2, jj2;
    long index1, index2;
    double pv1, pv2;
    long chiter = 0;
    long chcnt = 0;

    double plim = 0.0;
    double pcoeff2 = 0.15;
    long chcnt0cnt = 0;


    long WSPEEDsize;
    double WSPEEDpixscale;
    long vKwindsize;


	int BICUBIC = 1; // 0 if bilinear
	double a00, a01, a02, a03;
	double a10, a11, a12, a13;
	double a20, a21, a22, a23;
	double a30, a31, a32, a33;
	double p00, p01, p02, p03;
	double p10, p11, p12, p13;
	double p20, p21, p22, p23;
	double p30, p31, p32, p33;
	long iim0, iim1, iim2, iim3, jjm0, jjm1, jjm2, jjm3;
	double x2, x3, y, y2, y3;
	
	FILE *fpxypos;










    SLAMBDA = 1.0e-6*slambdaum;


    naxes = (long*) malloc(sizeof(long)*3);
    naxesout = (long*) malloc(sizeof(long)*3);



    ID1=-1;
    ID2=-1;
    ID_sarray1=-1;
    sarray = NULL;
    sarray_double = NULL;
    //  ID_carray1=-1;
    //  carray = NULL;






    printf("Making the wavefront series...\n");
    fflush(stdout);



    AtmosphericTurbulence_ReadConf();
    naxesout[0] = CONF_WFsize;
    naxesout[1] = CONF_WFsize;
    naxes[0] = CONF_WF_RAW_SIZE;
    naxes[1] = CONF_WF_RAW_SIZE;





    printf("Building reference atmosphere model ...\n");
    // create atm.txt file with concentrations as function of altitude
    // load RIA (Refractive index and Absorption) files if available
    AtmosphereModel_Create_from_CONF(CONFFILE, slambdaum*1e-6);




    // SOME TESTING
    //  AtmosphereModel_RefractionPath(1.5, CONF_ZANGLE, 0); //79.999/180.0*M_PI);//CONF_ZANGLE);


    /*   fp = fopen("Rprof.txt", "w");
       for(h=0; h<100000.0; h+=10.0)
           fprintf(fp, "%8g %.16f %.16f\n", h, AtmosphereModel_stdAtmModel_N(h, 0.6e-6, 0), AtmosphereModel_stdAtmModel_N(h, 1.6e-6, 0));
       fclose(fp);
    */



    // TESTING VALUES IN CIDDOR 1996


    if(0) // dry, table 1, 10 C, 100kPa -> Ciddor value = 1.000277747
    {
        P = 100000.0;
        Pw = 0.0;
        CO2ppm = 450.0;
        TC = 10.0;
    }
    else
    {
        // wet case #1
        P = 102993.0;
        Pw = 641.0;
        CO2ppm = 450.0;
        TC = 19.173;

        if(1)
        {
            // wet case #2
            P = 103006.0;
            Pw = 642.0;
            CO2ppm = 440.0;
            TC = 19.173;
        }
    }


    T = TC + 273.15;

    // dry air composition (Harisson 1965)
    dens_N2 = 780840e-6 * LoschmidtConstant*1e-6;
    dens_O2 = 209844e-6 * LoschmidtConstant*1e-6; // assuming CO2 -> O2
    dens_Ar = 9340e-6 * LoschmidtConstant*1e-6;
    dens_Ne = 18.18e-6 * LoschmidtConstant*1e-6;
    dens_He = 5.24e-6 * LoschmidtConstant*1e-6;
    dens_CH4 = 1.774e-6 * LoschmidtConstant*1e-6;
    dens_Kr = 1.14e-6 * LoschmidtConstant*1e-6;
    dens_H2 = 0.56e-6 * LoschmidtConstant*1e-6;
    dens_O3 = 0.0;
    dens_N = 0.0;
    dens_O = 0.0;
    dens_H = 0.0;
    dens_O2 -= CO2ppm*1e-6 * LoschmidtConstant*1e-6;
    dens_CO2 = CO2ppm*1e-6 * LoschmidtConstant*1e-6;
    denstot = dens_N2 + dens_O2 + dens_Ar + dens_Ne + dens_He + dens_CH4 + dens_Kr + dens_H2 + dens_O3 + dens_N + dens_O + dens_H + dens_CO2;
    dens_H2O = denstot * (Pw/P) / (1.0 - (Pw/P)); // - 1e-6*CO2ppm);
    //dens_CO2 = CO2ppm*1e-6 * LoschmidtConstant*1e-6;


    //dens_CO2 = denstot * CO2ppm*1e-6 / (1.0 - (Pw/P) - 1e-6*CO2ppm);

    denstot = dens_N2 + dens_O2 + dens_Ar + dens_Ne + dens_He + dens_CH4 + dens_Kr + dens_H2 + dens_O3 + dens_N + dens_O + dens_H + dens_H2O + dens_CO2;
    xN2 = dens_N2/denstot;
    xO2 = dens_O2/denstot;
    xAr = dens_Ar/denstot;
    xNe = dens_Ne/denstot;
    xHe = dens_H2/denstot;
    xCH4 = dens_CH4/denstot;
    xKr = dens_Kr/denstot;
    xH2 = dens_H2/denstot;
    xO3 = dens_O3/denstot;
    xN = dens_N/denstot;
    xO = dens_O/denstot;
    xH = dens_H/denstot;

    xCO2 = dens_CO2/denstot;
    xH2O = dens_H2O/denstot;

    xtot = xN2 + xO2 + xAr + xNe + xHe + xCH4 + xKr + xH2 + xO3 + xN + xO + xH + xCO2 + xH2O;
    xN2 /= xtot;
    xO2 /= xtot;
    xAr /= xtot;
    xNe /= xtot;
    xHe /= xtot;
    xCH4 /= xtot;
    xKr /= xtot;
    xH2 /= xtot;
    xO3 /= xtot;
    xN /= xtot;
    xO /= xtot;
    xH /= xtot;
    xCO2 /= xtot;
    xH2O /= xtot;

    printf("CO2 ppm = %.6f\n", xCO2*1e6);
    printf("H2O ppm = %.6f (goal: %.6f)\n", xH2O*1e6, Pw/P);

    RH = 0.0;
    Z0 = Z_Air(101325.0, 0.0, 0.0);
    Z = Z_Air(P, TC, RH); //
    //LL *= rhocoeff;
    printf("rhocoeff = %f %f\n", 1.0/rhocoeff, P/101325.0 * (273.15/T));
    printf("Z = %f\n", Z);
    printf("Z0 = %f\n", Z0);


    //denstot = 1.0/rhocoeff * LoschmidtConstant*1e-6;
    denstot =  (1.0/Z) * P/101325.0 * (273.15/T) * LoschmidtConstant*1e-6; // approximation - does not take into account CHANGE of Z with pressure, temperature

    dens_N2 = xN2 * denstot;
    dens_O2 = xO2 * denstot;
    dens_Ar = xAr * denstot;
    dens_Ne = xNe * denstot;
    dens_He = xHe * denstot;
    dens_CH4 = xCH4 * denstot;
    dens_Kr = xKr * denstot;
    dens_H2 = xH2 * denstot;
    dens_O3 = xO3 * denstot;
    dens_N = xN * denstot;
    dens_O = xO * denstot;
    dens_H = xH * denstot;
    dens_CO2 = xCO2 * denstot;
    dens_H2O = xH2O * denstot;



    printf("denstot = %g part/cm3      %f x LScst\n", denstot, denstot*1.0e6/LoschmidtConstant);
    lambda = 0.633e-6;
    printf("633nm test values\n");
    printf("HARISSON MODEL:    n = %.12g\n", 1.0 + AirMixture_N(lambda, dens_N2, dens_O2, dens_Ar, dens_H2O, dens_CO2, dens_Ne, dens_He, dens_CH4, dens_Kr, dens_H2, dens_O3, dens_N, dens_O, dens_H));

    printf("STD MODEL, 1 atm : n = %.12g\n", AtmosphereModel_stdAtmModel_N(0.0, lambda, 1));



    fp = fopen("Rlambda.txt", "w");
    for(l=1e-6; l<5e-6; l*=1.0+1e-5)
        fprintf(fp, "%.16f %.16f %.16f %.16f\n", l, AtmosphereModel_stdAtmModel_N(10, l, 0), AtmosphereModel_stdAtmModel_N(1000, l, 0), AtmosphereModel_stdAtmModel_N(4200, l, 0));
    fclose(fp);



    fp = fopen("AtmRefrac.txt", "w");
    for(l=0.4e-6; l<2.0e-6; l+=0.01e-6)
        fprintf(fp, "%.16f %.16f\n", l, asin(sin(CONF_ZANGLE)/(1.0+AtmosphereModel_stdAtmModel_N(SiteAlt, l, 0))));
    fclose(fp);


	if(compmode==0)
		return 0;


    printf("CONF_ZANGLE = %f  alt = %f\n", CONF_ZANGLE, SiteAlt);



    // for(Temp=173.0; Temp<373.0; Temp+=5.0)
    //   printf("T= %lf K    Ps(H2O) [Pa] = %g\n", Temp, AtmosphereModel_H2O_Saturation(Temp));



    //    Scoeff = LAMBDA/SLAMBDA;
    //   Nlambda = 0.0000834213+0.0240603/(130.0-1.0/pow(CONF_LAMBDA*1000000.0,2.0))+0.00015997/(38.9-1.0/pow(CONF_LAMBDA*1000000.0,2.0));
    //   Nslambda = 0.0000834213+0.0240603/(130.0-1.0/pow(SLAMBDA*1000000.0,2.0))+0.00015997/(38.9-1.0/pow(SLAMBDA*1000000.0,2.0));

    //printf("method 1 : %f %f\n", Nlambda, Nslambda);

    Nlambda = AtmosphereModel_stdAtmModel_N(0.0, CONF_LAMBDA, 0);
    Nslambda = AtmosphereModel_stdAtmModel_N(0.0, SLAMBDA, 0);
    Scoeff =  CONF_LAMBDA/SLAMBDA * Nslambda/Nlambda; // multiplicative coefficient to go from reference lambda phase to science lambda phase


    //  printf("Scoeff is %f (%f)\n",Scoeff,Nslambda/Nlambda);
    //   fflush(stdout);


    // printf("Zenith angle = %f rad\n", CONF_ZANGLE);



    //  pfactor = naxes[0]/CONF_WFsize;
    pfactor = 1;
    contraction_factor = 4;
    if(naxes[0]/CONF_WFsize==1)
        contraction_factor = 0;
    if(naxes[0]/CONF_WFsize==2)
        contraction_factor = 1;
    if(naxes[0]/CONF_WFsize==4)
        contraction_factor = 2;
    if(naxes[0]/CONF_WFsize==8)
        contraction_factor = 3;

    if(contraction_factor==4)
    {
        printf("ERROR: unknown contraction factor\n");
        fflush(stdout);
        exit(0);
    }

    /*  ID=image_ID("ST_pa");*/



    NBFRAMES = (long) (1.0*CONF_TIME_SPAN/CONF_WFTIME_STEP+0.5);
    printf("%.16f  %.16f  ->  %ld\n", CONF_TIME_SPAN, CONF_WFTIME_STEP, NBFRAMES);



    naxes[2] = NBFRAMES;
    naxesout[2] = NBFRAMES;

    nelement = naxes[0] * naxes[1] * naxes[2];
    printf("Allocating memory...\n");
    fflush(stdout);

    // OUTPUT ARRAYS
    if (WFprecision == 0 ) // single precision
    {
        IDout_array_pha = create_3Dimage_ID("outarraypha", naxesout[0], naxesout[1], naxesout[2]);
        IDout_array_amp = create_3Dimage_ID("outarrayamp", naxesout[0], naxesout[1], naxesout[2]);
        IDout_sarray_pha = create_3Dimage_ID("outsarraypha", naxesout[0], naxesout[1], naxesout[2]);
        IDout_sarray_amp = create_3Dimage_ID("outsarrayamp", naxesout[0], naxesout[1], naxesout[2]);
        for(ii=0; ii<naxesout[0]*naxesout[1]*naxesout[2]; ii++)
        {
            data.image[IDout_array_amp].array.F[ii] = 1.0;
            data.image[IDout_array_pha].array.F[ii] = 0.0;

            data.image[IDout_sarray_amp].array.F[ii] = 1.0;
            data.image[IDout_sarray_pha].array.F[ii] = 0.0;
        }
    }
    else // double precision 
    {
        IDout_array_pha = create_3Dimage_ID_double("outarraypha", naxesout[0], naxesout[1], naxesout[2]);
        IDout_array_amp = create_3Dimage_ID_double("outarrayamp", naxesout[0], naxesout[1], naxesout[2]);
        IDout_sarray_pha = create_3Dimage_ID_double("outsarraypha", naxesout[0], naxesout[1], naxesout[2]);
        IDout_sarray_amp = create_3Dimage_ID_double("outsarrayamp", naxesout[0], naxesout[1], naxesout[2]);
        for(ii=0; ii<naxesout[0]*naxesout[1]*naxesout[2]; ii++)
        {
            data.image[IDout_array_amp].array.D[ii] = 1.0;
            data.image[IDout_array_pha].array.D[ii] = 0.0;

            data.image[IDout_sarray_amp].array.D[ii] = 1.0;
            data.image[IDout_sarray_pha].array.D[ii] = 0.0;
        }
    }



    naxes_MASTER[0] = CONF_MASTER_SIZE;
    naxes_MASTER[1] = CONF_MASTER_SIZE;

    master=0;
    stop=1;
    while(stop)
    {
		if(WFprecision == 0)
			sprintf(fname,"t%03ld_%ld_f.fits", master, CONF_MASTER_SIZE);
		else
			sprintf(fname,"t%03ld_%ld_d.fits", master, CONF_MASTER_SIZE);
        if(!file_exists(fname))
        {
            stop=0;
        }
        else
            master++;
    }

    NBMASTERS = master;
    printf("%ld turbulence master files\n",NBMASTERS);
    fflush(stdout);



    if((fp=fopen(CONF_TURBULENCE_PROF_FILE,"r"))==NULL)
    {
        printf("Cannot open turbulence profile file \"%s\"\n", CONF_TURBULENCE_PROF_FILE);
        exit(1);
    }
    NBLAYERS=0;
    while(fgets(line, 2000, fp)!=NULL)
    {
        sscanf(line,"%s",word);
        if(isdigit(word[0]))
        {
            NBLAYERS+=1;
        }
    }
    fclose(fp);

    LAYER_ALT = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_CN2 = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_SPD = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_DIR = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_OUTERSCALE = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_INNERSCALE = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_SIGMAWSPEED = (double*) malloc(NBLAYERS*sizeof(double));
    LAYER_LWIND = (double*) malloc(NBLAYERS*sizeof(double));

    if((fp=fopen(CONF_TURBULENCE_PROF_FILE, "r"))==NULL)
    {
        printf("Cannot open turbulence profile file \"%s\"\n", CONF_TURBULENCE_PROF_FILE);
        exit(1);
    }
    layer=0;
    while(fgets(line,2000,fp)!=NULL)
    {
        sscanf(line,"%s",word);
        if(isdigit(word[0]))
        {
            sscanf(line,"%lf %lf %lf %lf %lf %lf %lf %lf", &fl1, &fl2, &fl3, &fl4, &fl5, &fl6, &fl7, &fl8);
            if(fl1>SiteAlt-0.1)
            {
                LAYER_ALT[layer] = fl1;
                LAYER_CN2[layer] = fl2;
                LAYER_SPD[layer] = fl3;
                LAYER_DIR[layer] = fl4;
                LAYER_OUTERSCALE[layer] = fl5;
                LAYER_INNERSCALE[layer] = fl6;
                LAYER_SIGMAWSPEED[layer] = fl7;
                LAYER_LWIND[layer] = fl8;
                layer+=1;
            }
        }
    }
    fclose(fp);

    /* CN2 normalisation for 1024x1024 -> 256x256*/
    /* S<0.7 : x=(S-0.1)/0.06*2
    S>0.7 : x=(S-0.38)/0.033*2 */
    /*  if(seeing<0.7)
    CN2total = (seeing-0.1)/0.03;
    else
    CN2total = (seeing-0.38)/0.0165;*/
    /* S  = sqrt(CN2/16.666)*0.443 = 0.1085 sqrt(CN2) */

    CN2total = 1.0; //84.926*seeing*seeing;

    tmp = 0;
    for(layer=0; layer<NBLAYERS; layer++)
        tmp += LAYER_CN2[layer];
    for(layer=0; layer<NBLAYERS; layer++)
        LAYER_CN2[layer] *= CN2total/tmp;



    for(layer=0; layer<NBLAYERS; layer++)
    {
        printf("Turbulence layer %ld : alt = %f m   CN2 = %f   V = %f m/s   Angle = %f rad   outerscale = %f m    innerscale = %f m  sigmaWind = %f m/s  Lwind = %f m\n", layer, LAYER_ALT[layer], LAYER_CN2[layer], LAYER_SPD[layer], LAYER_DIR[layer], LAYER_OUTERSCALE[layer], LAYER_INNERSCALE[layer], LAYER_SIGMAWSPEED[layer], LAYER_LWIND[layer]);
    }





    SLAYER_ALT = (double*) malloc(NBLAYERS*sizeof(double));
    SLAYER_CN2 = (double*) malloc(NBLAYERS*sizeof(double));
    NBSLAYERS = NBLAYERS;
    for(layer=0; layer<NBSLAYERS; layer++)
    {
        SLAYER_ALT[layer] = LAYER_ALT[layer];
        SLAYER_CN2[layer] = LAYER_CN2[layer];
    }


    /// temporary arrays for phase unwrapping
    if (WFprecision == 0 ) // single precision
    {
        IDpeakpha_re = create_2Dimage_ID("peakphare", naxesout[0], naxesout[1]);
        IDpeakpha_im = create_2Dimage_ID("peakphaim", naxesout[0], naxesout[1]);
        IDpeakpha = create_2Dimage_ID("peakpha", naxesout[0], naxesout[1]);

        xsizepeakpha = (long) (naxesout[0]/20);
        ysizepeakpha = (long) (naxesout[1]/20);
        IDpeakpha_re_bin = create_2Dimage_ID("peakphare_bin", xsizepeakpha, ysizepeakpha);
        IDpeakpha_im_bin = create_2Dimage_ID("peakphaim_bin", xsizepeakpha, ysizepeakpha);
        IDpeakpha_bin = create_2Dimage_ID("peakpha_bin", xsizepeakpha, ysizepeakpha);
        IDpeakpha_bin_ch = create_2Dimage_ID("peakpha_bin_ch", xsizepeakpha, ysizepeakpha);
    }
    else
    {
        IDpeakpha_re = create_2Dimage_ID_double("peakphare", naxesout[0], naxesout[1]);
        IDpeakpha_im = create_2Dimage_ID_double("peakphaim", naxesout[0], naxesout[1]);
        IDpeakpha = create_2Dimage_ID_double("peakpha", naxesout[0], naxesout[1]);

        xsizepeakpha = (long) (naxesout[0]/20);
        ysizepeakpha = (long) (naxesout[1]/20);
        IDpeakpha_re_bin = create_2Dimage_ID_double("peakphare_bin", xsizepeakpha, ysizepeakpha);
        IDpeakpha_im_bin = create_2Dimage_ID_double("peakphaim_bin", xsizepeakpha, ysizepeakpha);
        IDpeakpha_bin = create_2Dimage_ID_double("peakpha_bin", xsizepeakpha, ysizepeakpha);
        IDpeakpha_bin_ch = create_2Dimage_ID_double("peakpha_bin_ch", xsizepeakpha, ysizepeakpha);
    }


    OK=0;
    while(OK==0)
    {
        printf("--------------------\n");
        for(i=0; i<NBSLAYERS; i++)
            printf("Super layer %ld/%ld  alt: %f  CN2: %f\n", i, NBSLAYERS,SLAYER_ALT[i], SLAYER_CN2[i]);

        /* look for minimum altitude difference */
        minaltd = LAYER_ALT[NBLAYERS-1];
        index = 0;
        for(i=0; i<NBSLAYERS-1; i++)
        {
            value = SLAYER_ALT[i+1]-SLAYER_ALT[i];
            if(value < minaltd)
            {
                minaltd = value;
                index = i;
            }
        }
        printf("minimumum distance between layers: %.2f m  (%ld %ld)   (CONF_FRESNEL_PROPAGATION_BIN = %.2f)\n", minaltd, i, i+1, CONF_FRESNEL_PROPAGATION_BIN);
        if((minaltd>CONF_FRESNEL_PROPAGATION_BIN)||(NBSLAYERS==1))
        {
            OK=1;
        }
        else
        {
            /* group SLAYERs i and i+1 */
            printf("Group slayers %ld and %ld\n", index, index+1);
            SLAYER_ALT[index] = (SLAYER_CN2[index]*SLAYER_ALT[index]+SLAYER_CN2[index+1]*SLAYER_ALT[index+1])/(SLAYER_CN2[index]+SLAYER_CN2[index+1]);
            SLAYER_CN2[index] = SLAYER_CN2[index] + SLAYER_CN2[index+1];
            for(i=index+1; i<NBSLAYERS-1; i++)
            {
                SLAYER_ALT[i] = SLAYER_ALT[i+1];
                SLAYER_CN2[i] = SLAYER_CN2[i+1];
            }
            NBSLAYERS-=1;
        }
    }
    for(i=0; i<NBSLAYERS; i++)
    {
        printf("Super layer %ld  alt: %f  CN2: %g\n", i, SLAYER_ALT[i], SLAYER_CN2[i]);
    }

    NB_alt_bin_sep = NBSLAYERS-1;
    alt_bin_sep = (double*) malloc(NB_alt_bin_sep*sizeof(double));
    for(i=0; i<NB_alt_bin_sep; i++)
    {
        alt_bin_sep[i] = 0.5*(SLAYER_ALT[i]+SLAYER_ALT[i+1]);
        printf("Super layer %ld Altitude threshhold : %.2f m\n", i, alt_bin_sep[i]);
    }

    free(SLAYER_CN2);


    super_layer_index = (long*) malloc(NBLAYERS*sizeof(long));
    for(layer=0; layer<NBLAYERS; layer++)
    {
        index=0;
        for(i=0; i<NB_alt_bin_sep; i++)
            if((alt_bin_sep[i]<LAYER_ALT[layer])&&(alt_bin_sep[i+1]>LAYER_ALT[layer]))
                index=i+1;
        if(LAYER_ALT[layer]>alt_bin_sep[NB_alt_bin_sep-1])
            index = NB_alt_bin_sep;
        super_layer_index[layer]=index;
        printf("Layer %ld belongs to superlayer %ld/%ld\n",layer,super_layer_index[layer],NBSLAYERS);
    }


    frame=0;
    xpos = (double*) malloc(sizeof(double)*NBLAYERS);
    ypos = (double*) malloc(sizeof(double)*NBLAYERS);
    xpos0 = (double*) malloc(sizeof(double)*NBLAYERS);
    ypos0 = (double*) malloc(sizeof(double)*NBLAYERS);
    xposfcnt = (long*) malloc(sizeof(long)*NBLAYERS);
    yposfcnt = (long*) malloc(sizeof(long)*NBLAYERS);

    vxpix = (double*) malloc(sizeof(double)*NBLAYERS);
    vypix = (double*) malloc(sizeof(double)*NBLAYERS);

    h = 0.0;
    printf("\n\n");
    printf("Refractivity = %g -> %g     %g -> %g\n", CONF_LAMBDA, AtmosphereModel_stdAtmModel_N(h, CONF_LAMBDA, 0), SLAMBDA, AtmosphereModel_stdAtmModel_N(h, SLAMBDA, 0));


    printf("Computing refraction and position offset for CONF_ZANGLE = %f\n", CONF_ZANGLE);
    for(layer=0; layer<NBLAYERS; layer++)
    {
        xposfcnt[layer] = 0;
        yposfcnt[layer] = 0;

        xpos[layer] = 0.05*CONF_MASTER_SIZE;
        ypos[layer] = 0.05*CONF_MASTER_SIZE;

        // layer shift due to atmospheric refraction
        // computes here what the offset is at a the reference wavelength
        Roffset = 0.0;
        for(h=SiteAlt; h<LAYER_ALT[layer]; h += 1.0)
        {
            Rindex = 1.0 + AtmosphereModel_stdAtmModel_N(h, CONF_LAMBDA, 0);
            tmpf = sin(CONF_ZANGLE)/sqrt(Rindex*Rindex-sin(CONF_ZANGLE)*sin(CONF_ZANGLE));
            tmpf -= sin(CONF_ZANGLE)/sqrt(1.0-sin(CONF_ZANGLE)*sin(CONF_ZANGLE));
            Roffset += tmpf;
            //           printf("h = %12f   Rindex = %12f   Roffset = %12f\n", h, Rindex, Roffset);
        }

        // we compute here the offset at the science wavelength
        RoffsetS = 0.0;
        for(h=SiteAlt; h<LAYER_ALT[layer]; h += 1.0)
        {
            Rindex = 1.0 + AtmosphereModel_stdAtmModel_N(h, SLAMBDA, 0);
            tmpf = sin(CONF_ZANGLE)/sqrt(Rindex*Rindex-sin(CONF_ZANGLE)*sin(CONF_ZANGLE));
            tmpf -= sin(CONF_ZANGLE)/sqrt(1.0-sin(CONF_ZANGLE)*sin(CONF_ZANGLE));
            RoffsetS += tmpf;
        }

        // the refractive offset is the difference between the reference and science wavelength offsets
        ypos[layer] += (RoffsetS-Roffset)/(CONF_PUPIL_SCALE/pfactor);

        // add here offset due to source position
        // SOURCE_Xpos and SOURCE_Ypos are in radian
        xpos[layer] += CONF_SOURCE_Xpos*LAYER_ALT[layer]/(CONF_PUPIL_SCALE/pfactor);
        ypos[layer] += CONF_SOURCE_Ypos*LAYER_ALT[layer]/(CONF_PUPIL_SCALE/pfactor);

        xpos0[layer] = CONF_SOURCE_Xpos*LAYER_ALT[layer]/(CONF_PUPIL_SCALE/pfactor); // for realtime mode
        ypos0[layer] = CONF_SOURCE_Ypos*LAYER_ALT[layer]/(CONF_PUPIL_SCALE/pfactor);

        vxpix[layer] = LAYER_SPD[layer]*cos(LAYER_DIR[layer])/(CONF_PUPIL_SCALE/pfactor); /* pixel coordinate speed, in pixel per sec, x axis */
        vypix[layer] = LAYER_SPD[layer]*sin(LAYER_DIR[layer])/(CONF_PUPIL_SCALE/pfactor); /* pixel coordinate speed, in pixel per sec, y axis */

        printf("------ layer %5ld, SPEED = %12f x %12f pix/step, offset = %12f m  [ %12f m  %12f m ] ----------\n",layer, vxpix[layer], vypix[layer], RoffsetS-Roffset, RoffsetS, Roffset);
    }



    printf("NBMASTERS = %ld\n",NBMASTERS);
    if(NBMASTERS<NBLAYERS)
        NBMASTERS = NBLAYERS;
    ID_TM = (long*) malloc(sizeof(long)*NBMASTERS);
    for(i=0; i<NBMASTERS; i++)
    {
		if(WFprecision == 0)
			sprintf(fname,"t%03ld_%ld_f.fits", i, CONF_MASTER_SIZE);
        else
			sprintf(fname,"t%03ld_%ld_d.fits", i, CONF_MASTER_SIZE);			

        sprintf(fname1, "TM%ld",i);
        if(load_fits(fname, fname1, 1)==-1)
        {
            sprintf(fname1,"TM%ld", i);
            printf("CREATING %s   (%f - %f)\n", fname, LAYER_OUTERSCALE[i]/CONF_PUPIL_SCALE, LAYER_INNERSCALE[i]/CONF_PUPIL_SCALE);
            make_master_turbulence_screen(fname1, "tursctmp", CONF_MASTER_SIZE, LAYER_OUTERSCALE[i]/CONF_PUPIL_SCALE, LAYER_INNERSCALE[i]/CONF_PUPIL_SCALE, WFprecision);

			if(WFprecision == 0)
				save_fl_fits(fname1, fname);
            else
				save_db_fits(fname1, fname);
            delete_image_ID("tursctmp");
        }
        ID_TM[i] = image_ID(fname1);
    }
    ID_TML = (long*) malloc(sizeof(long)*NBLAYERS);
    j=0;
    for(i=0; i<NBLAYERS; i++)
    {
        ID_TML[i] = ID_TM[j];
        if(j==NBMASTERS)
        {
            printf("ERROR: number of master turbulence phase screens (%ld) is too small\n",NBMASTERS);
            exit(0);
        }
        j++;
    }







    // Measure r0 (pix) for each master turbulence screen
    dpix = 50;
    r0tot = 0.0;
    r0cnt = 0;
    for(k=0; k<NBMASTERS; k++)
    {
        cnt = 0;
        tot = 0.0;
        if(WFprecision == 0)
        {
            for(ii=0; ii<CONF_MASTER_SIZE-dpix; ii++)
                for(jj=0; jj<CONF_MASTER_SIZE; jj++)
                {
                    p1 = data.image[ID_TM[k]].array.F[jj*CONF_MASTER_SIZE+ii];
                    p2 = data.image[ID_TM[k]].array.F[jj*CONF_MASTER_SIZE+ii+dpix];
                    tot += (p1-p2)*(p1-p2);
                    cnt++;
                }
        }
        else
        {
            for(ii=0; ii<CONF_MASTER_SIZE-dpix; ii++)
                for(jj=0; jj<CONF_MASTER_SIZE; jj++)
                {
                    p1 = data.image[ID_TM[k]].array.D[jj*CONF_MASTER_SIZE+ii];
                    p2 = data.image[ID_TM[k]].array.D[jj*CONF_MASTER_SIZE+ii+dpix];
                    tot += (p1-p2)*(p1-p2);
                    cnt++;
                }
        }
        r0 = 1.0*dpix*pow((tot/cnt)/6.88,-3.0/5.0);
        printf("TURBULENCE MASTER %ld    r0 = %g pix\n",k,r0);
        r0tot += r0;
        r0cnt++;
    }
    r0 = r0tot/r0cnt;
    printf("r0 = %g pix -> %g pix\n", r0, CONF_LAMBDA/(CONF_SEEING/3600.0/180.0*PI)/CONF_PUPIL_SCALE*pfactor);

    // renormalize turbulence screens such that a single screen has the right r0
    tmp1 = pow(r0/(CONF_LAMBDA/(CONF_SEEING/3600.0/180.0*PI)/CONF_PUPIL_SCALE*pfactor),5.0/6.0);
    if(WFprecision == 0)
    {
        for(k=0; k<NBMASTERS; k++)
            for(ii=0; ii<CONF_MASTER_SIZE*CONF_MASTER_SIZE; ii++)
                data.image[ID_TM[k]].array.F[ii] *= tmp1;
    }
    else
    {
        for(k=0; k<NBMASTERS; k++)
            for(ii=0; ii<CONF_MASTER_SIZE*CONF_MASTER_SIZE; ii++)
                data.image[ID_TM[k]].array.D[ii] *= tmp1;
    }

    r0tot = 0.0;
    r0cnt = 0;
    for(k=0; k<NBMASTERS; k++)
    {
        cnt = 0;
        tot = 0.0;
        if(WFprecision == 0)
        {
            for(ii=0; ii<CONF_MASTER_SIZE-dpix; ii++)
                for(jj=0; jj<CONF_MASTER_SIZE; jj++)
                {
                    p1 = data.image[ID_TM[k]].array.F[jj*CONF_MASTER_SIZE+ii];
                    p2 = data.image[ID_TM[k]].array.F[jj*CONF_MASTER_SIZE+ii+dpix];
                    tot += (p1-p2)*(p1-p2);
                    cnt++;
                }
        }
        else
        {
            for(ii=0; ii<CONF_MASTER_SIZE-dpix; ii++)
                for(jj=0; jj<CONF_MASTER_SIZE; jj++)
                {
                    p1 = data.image[ID_TM[k]].array.D[jj*CONF_MASTER_SIZE+ii];
                    p2 = data.image[ID_TM[k]].array.D[jj*CONF_MASTER_SIZE+ii+dpix];
                    tot += (p1-p2)*(p1-p2);
                    cnt++;
                }
        }
        r0 = 1.0*dpix*pow((tot/cnt)/6.88,-3.0/5.0);
        printf("TURBULENCE MASTER %ld    r0 = %g pix\n",k,r0);
        r0tot += r0;
        r0cnt++;
    }
    r0 = r0tot/r0cnt;
    printf("r0 = %g pix\n",r0);


    // target seeing = seeing [arcsec]
    // ref lambda = CONF_LAMBDA [m]
    // if single screen:
    // r0[m] = CONF_LAMBDA[m]/seeing[rad]
    // r0[pix] = CONF_LAMBDA*1.0e-6/(seeing/3600.0/180.0*PI)/CONF_PUPIL_SCALE
    // multiply by (r0/r0goal)^6/5


    // each layer coeff mult by sqrt(fracCN2/cos(CONF_ZANGLE)))


    for(i=0; i<NBLAYERS; i++)
    {
        //      coeff = 3.645*183.8115*pow(10.0,-12)/LAMBDA/LAMBDA*CONF_PUPIL_SCALE*PUPIL_SCALE*sqrt(LAYER_CN2[i]);
        //if(pfactor==2)
        //	coeff *= 1.0/(pfactor*pfactor*pfactor*pfactor*pfactor*pfactor);
        if(WFprecision == 0)
        {
            for(ii=0; ii<CONF_MASTER_SIZE*CONF_MASTER_SIZE; ii++)
                data.image[ID_TML[i]].array.F[ii] *= sqrt(LAYER_CN2[i]/cos(CONF_ZANGLE));
        }
        else
        {
            for(ii=0; ii<CONF_MASTER_SIZE*CONF_MASTER_SIZE; ii++)
                data.image[ID_TML[i]].array.D[ii] *= sqrt(LAYER_CN2[i]/cos(CONF_ZANGLE));
        }
        printf("Layer %ld, coeff = %g\n",i,sqrt(LAYER_CN2[i]/cos(CONF_ZANGLE)));
    }



    if(WFprecision == 0)
    {
        ID_array1 = create_2Dimage_ID("array1",naxes[0],naxes[1]);
        if(CONF_MAKE_SWAVEFRONT==1)
            ID_sarray1 = create_2Dimage_ID("sarray1",naxes[0],naxes[1]);
    }
    else
    {
        ID_array1 = create_2Dimage_ID_double("array1",naxes[0],naxes[1]);
        if(CONF_MAKE_SWAVEFRONT==1)
            ID_sarray1 = create_2Dimage_ID_double("sarray1",naxes[0],naxes[1]);
    }



    if(CONF_WAVEFRONT_AMPLITUDE==1) // includes sub pixel translation
    {
        if(WFprecision == 0)
        {
            ID_array2 = create_2DCimage_ID("array2", naxes[0], naxes[1]);
            if(CONF_MAKE_SWAVEFRONT==1)
                ID_sarray2 = create_2DCimage_ID("sarray2", naxes[0], naxes[1]);
        }
        else
        {
            ID_array2 = create_2DCimage_ID_double("array2", naxes[0], naxes[1]);
            if(CONF_MAKE_SWAVEFRONT==1)
                ID_sarray2 = create_2DCimage_ID_double("sarray2", naxes[0], naxes[1]);
        }
    }



    if(WFprecision == 0)
    {
        if((array = (complex_float*) malloc(NBFRAMES*naxes[0]*naxes[1]*sizeof(complex_float)))==NULL)
        {
            printf("Memory allocation error (\"array\" in make_AtmosphericTurbulence_wavefront_series)\n");
            printf("Decrease the size of the wavefront cube\n");
            exit(0);
        }

        if(CONF_MAKE_SWAVEFRONT==1)
        {
            if((sarray = (complex_float*) malloc(NBFRAMES*naxes[0]*naxes[1]*sizeof(complex_float)))==NULL)
            {
                printf("Memory allocation error (\"sarray\" in make_AtmosphericTurbulence_wavefront_series)\n");
                printf("Decrease the size of the wavefront cube\n");
                exit(0);
            }
        }
    }
    else
    {
        if((array_double = (complex_double*) malloc(NBFRAMES*naxes[0]*naxes[1]*sizeof(complex_double)))==NULL)
        {
            printf("Memory allocation error (\"array_double\" in make_AtmosphericTurbulence_wavefront_series)\n");
            printf("Decrease the size of the wavefront cube\n");
            exit(0);
        }

        if(CONF_MAKE_SWAVEFRONT==1)
        {
            if((sarray_double = (complex_double*) malloc(NBFRAMES*naxes[0]*naxes[1]*sizeof(complex_double)))==NULL)
            {
                printf("Memory allocation error (\"sarray_double\" in make_AtmosphericTurbulence_wavefront_series)\n");
                printf("Decrease the size of the wavefront cube\n");
                exit(0);
            }
        }
    }





    if(CONF_SHM_OUTPUT == 1)
    {
        if(WFprecision == 0)
        {
            IDshmpha = create_image_ID("atmwfpha", 2, naxesout, FLOAT, 1, 0);
            IDshmamp = create_image_ID("atmwfamp", 2, naxesout, FLOAT, 1, 0);
        }
        else
        {
            IDshmpha = create_image_ID("atmwfpha", 2, naxesout, DOUBLE, 1, 0);
            IDshmamp = create_image_ID("atmwfamp", 2, naxesout, DOUBLE, 1, 0);
        }
    }


    if(CONF_SHM_SOUTPUT == 1)
    {
        sprintf(fname, "%spha", CONF_SHM_SPREFIX);
        if(WFprecision == 0)
            IDshmspha = create_image_ID(fname, 2, naxesout, FLOAT, 1, 0);
        else
            IDshmspha = create_image_ID(fname, 2, naxesout, DOUBLE, 1, 0);

        sprintf(fname, "%samp", CONF_SHM_SPREFIX);
        if(WFprecision == 0)
            IDshmsamp = create_image_ID(fname, 2, naxesout, FLOAT, 1, 0);
        else
            IDshmsamp = create_image_ID(fname, 2, naxesout, DOUBLE, 1, 0);

        kw = 0;
        strcpy(data.image[IDshmspha].kw[kw].name, "TIME");
        data.image[IDshmspha].kw[kw].type = 'D';
        data.image[IDshmspha].kw[kw].value.numf = 0.0;
        strcpy(data.image[IDshmspha].kw[kw].comment, "Physical time [sec]");

        kw = 0;
        strcpy(data.image[IDshmsamp].kw[kw].name, "TIME");
        data.image[IDshmsamp].kw[kw].type = 'D';
        data.image[IDshmsamp].kw[kw].value.numf = 0.0;
        strcpy(data.image[IDshmsamp].kw[kw].comment, "Physical time [sec]");
    }





    printf("SKIP_EXISTING = %d\n", CONF_SKIP_EXISTING);
    if(CONF_SKIP_EXISTING==1)
    {
        start_tspan = 0;
        OK = 1;
        while(OK==1)
        {
            sprintf(fname1,"%s%08ld.%09ld.pha.fits", CONF_WF_FILE_PREFIX, start_tspan, (long) (SLAMBDA*1e12+0.5)); // lambda in pm
            sprintf(fname2,"%s%08ld.%09ld.pha.fits", CONF_SWF_FILE_PREFIX, start_tspan, (long) (SLAMBDA*1e12+0.5));
            printf("TESTING FILE %s ... ", fname2);
            if(file_exists(fname2)==1)
            {
                start_tspan ++;
                printf("exists\n");
                OK = 1;
            }
            else
            {
                printf("does not exist\n");
                OK = 0;
            }
        }
    }
    printf("Start TSPAN = %ld\n", start_tspan);


    WSPEEDsize = naxes[0];
    WSPEEDpixscale = CONF_PUPIL_SCALE;
    vKwindsize = 20000.0/WSPEEDpixscale; // 20 km

    for(layer=0; layer<NBLAYERS; layer++)
    {
        sprintf(fname, "wspeed_%03ld.fits", layer);
        sprintf(name, "wspeed_%03ld", layer);
        ID = load_fits(fname, name, 1);
        if(ID==-1)
        {
            printf("COMPUTE WIND SPEED SCATTER - LAYER %ld   sigma = %f m/s\n", layer, LAYER_SIGMAWSPEED[layer]);
            make_AtmosphericTurbulence_vonKarmanWind(vKwindsize, WSPEEDpixscale, LAYER_SIGMAWSPEED[layer], LAYER_LWIND[layer], WSPEEDsize, name);
            sprintf(fname, "!wspeed_%03ld.fits", layer);
            save_fits(name, fname);


            // print wind speed as a function of time and position
            sprintf(fname, "wspeed_%03ld.txt", layer);
            ID = image_ID(name);
            if((fp=fopen(fname,"w"))==NULL)
            {
                printf("ERROR: cannot create file \"%s\"\n", fname);
                exit(0);
            }
            for(ii=0; ii<vKwindsize; ii++)
                fprintf(fp, "%6ld   %20.16f   %20.16f  %.16g  %.16g  %.16g\n", ii, WSPEEDpixscale*ii, WSPEEDpixscale*ii*LAYER_SPD[layer], data.image[ID].array.F[ii], data.image[ID].array.F[vKwindsize+ii], data.image[ID].array.F[2*vKwindsize+ii]);
            fclose(fp);
        }
    }



    vindex = 0;




    printf("WAVEFRONT_AMPLITUDE = %d\n", CONF_WAVEFRONT_AMPLITUDE);

	fpxypos = fopen("xypos.log", "w");
    fclose(fpxypos);

	fpxypos = fopen("xypos3.log", "w");
    fclose(fpxypos);

    for(tspan=start_tspan; tspan<CONF_NB_TSPAN; tspan++)
    {
        for(frame=0; frame<NBFRAMES; frame++)
        {
            vindex ++;
            if(CONF_MAKE_SWAVEFRONT==1)
            {
                if(WFprecision == 0)
                {
                    for(ii=0; ii<naxes[0]; ii++)
                        for(jj=0; jj<naxes[1]; jj++)
                        {
                            data.image[ID_array1].array.F[jj*naxes[0]+ii] = 0.0;
                            data.image[ID_sarray1].array.F[jj*naxes[0]+ii] = 0.0;
                        }
                    if(CONF_WAVEFRONT_AMPLITUDE==1)
                        for(ii=0; ii<naxes[0]; ii++)
                            for(jj=0; jj<naxes[1]; jj++)
                            {
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].re = 1.0;
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].im = 0.0;
                                data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].re = 1.0;
                                data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].im = 0.0;
                            }
                }
                else
                {
                    for(ii=0; ii<naxes[0]; ii++)
                        for(jj=0; jj<naxes[1]; jj++)
                        {
                            data.image[ID_array1].array.D[jj*naxes[0]+ii] = 0.0;
                            data.image[ID_sarray1].array.D[jj*naxes[0]+ii] = 0.0;
                        }
                    if(CONF_WAVEFRONT_AMPLITUDE==1)
                        for(ii=0; ii<naxes[0]; ii++)
                            for(jj=0; jj<naxes[1]; jj++)
                            {
                                data.image[ID_array2].array.CD[jj*naxes[0]+ii].re = 1.0;
                                data.image[ID_array2].array.CD[jj*naxes[0]+ii].im = 0.0;
                                data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].re = 1.0;
                                data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].im = 0.0;
                            }
                }
            }
            else
            {
                if(WFprecision == 0)
                {
                    for(ii=0; ii<naxes[0]; ii++)
                        for(jj=0; jj<naxes[1]; jj++)
                            data.image[ID_array1].array.F[jj*naxes[0]+ii] = 0.0;
                    if(CONF_WAVEFRONT_AMPLITUDE==1)
                        for(ii=0; ii<naxes[0]; ii++)
                            for(jj=0; jj<naxes[1]; jj++)
                            {
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].re = 1.0;
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].im = 0.0;
                            }
                }
                else
                {
                    for(ii=0; ii<naxes[0]; ii++)
                        for(jj=0; jj<naxes[1]; jj++)
                            data.image[ID_array1].array.D[jj*naxes[0]+ii] = 0.0;
                    if(CONF_WAVEFRONT_AMPLITUDE==1)
                        for(ii=0; ii<naxes[0]; ii++)
                            for(jj=0; jj<naxes[1]; jj++)
                            {
                                data.image[ID_array2].array.CD[jj*naxes[0]+ii].re = 1.0;
                                data.image[ID_array2].array.CD[jj*naxes[0]+ii].im = 0.0;
                            }
                }
            }

            usleep(CONF_SIMTDELAY);


            if(CONF_WAITFORSEM==1) // wait for semaphore to advance to next WF step
            {
                printf("WAITING for semaphore #0 \"%s\" ...\n", CONF_WAITSEMIMNAME);
                COREMOD_MEMORY_image_set_semwait(CONF_WAITSEMIMNAME, 0);
                printf("Done\n");
            }


            clock_gettime(CLOCK_REALTIME, &tnow);
            tnowdouble = 1.0*tnow.tv_sec + 1.0e-9*tnow.tv_nsec;
            tnowdouble *= CONF_ATMWF_REALTIMEFACTOR;
            for(layer=NBLAYERS-1; layer!=-1; layer--)
            {
                if(CONF_ATMWF_REALTIME==0)
                {
                    tnowdouble = (tspan*NBFRAMES+frame)*CONF_WFTIME_STEP;
                    printf("\rLayer %2ld/%2ld, Frame %4ld/%4ld, File %6ld/%6ld  [TIME = %10.4f s]  ", layer, NBLAYERS, frame, NBFRAMES,tspan, CONF_NB_TSPAN,(tspan*NBFRAMES+frame)*CONF_WFTIME_STEP);
                }
                else
                    printf("\rLayer %2ld/%2ld, Frame %4ld/%4ld, File %6ld/%6ld  [PHYSICAL TIME = %.9lf s]  ",layer,NBLAYERS, frame, NBFRAMES, tspan, CONF_NB_TSPAN, tnowdouble);
                fflush(stdout);

                // recompute Scoeff for this layer
                Nlambda = AtmosphereModel_stdAtmModel_N(LAYER_ALT[layer], CONF_LAMBDA, 0);
                Nslambda = AtmosphereModel_stdAtmModel_N(LAYER_ALT[layer], SLAMBDA, 0);
                Scoeff =  CONF_LAMBDA/SLAMBDA * Nslambda/Nlambda; // multiplicative coefficient to go from reference lambda phase to science lambda phase


                if(layer!=NBLAYERS-1)
                {
                    if(super_layer_index[layer+1]!=super_layer_index[layer])
                    {
                        if(CONF_FRESNEL_PROPAGATION==1)
                        {
                            Fresnel_propagate_wavefront("array2", "array2p", CONF_PUPIL_SCALE/pfactor,(SLAYER_ALT[super_layer_index[layer+1]]-SLAYER_ALT[super_layer_index[layer]])/cos(CONF_ZANGLE), CONF_LAMBDA);
                            delete_image_ID("array2");
                            chname_image_ID("array2p", "array2");
                        }
                        
                        ID_array2 = image_ID("array2");
                        if(CONF_MAKE_SWAVEFRONT==1)
                        {
                            if(CONF_FRESNEL_PROPAGATION==1)
                            {
                                //				printf("FRESNEL PROPAGATION\n");  //TEST
                                //			fflush(stdout);

                                Fresnel_propagate_wavefront("sarray2", "sarray2p", CONF_PUPIL_SCALE/pfactor, (SLAYER_ALT[super_layer_index[layer+1]]-SLAYER_ALT[super_layer_index[layer]])/cos(CONF_ZANGLE), SLAMBDA);
                                delete_image_ID("sarray2");
                                chname_image_ID("sarray2p", "sarray2");
                            }
                            ID_sarray2 = image_ID("sarray2");
                        }
                    }
                }


                // layer_scale = (SODIUM_ALT-LAYER_ALT[layer])/SODIUM_ALT;

                vpix = 0.0; //0.1*sin(11.0*vindex*(layer+3))*sqrt(vxpix[layer]*vxpix[layer]+vypix[layer]*vypix[layer]);
                PA = sin(10.0*vindex*(layer+2));

                if(CONF_ATMWF_REALTIME==1) // real time
                {
                    xpos[layer] = xpos0[layer] + vxpix[layer]*tnowdouble + 1.0*xposfcnt[layer]*naxes_MASTER[0];
                    ypos[layer] = ypos0[layer] + vypix[layer]*tnowdouble + 1.0*yposfcnt[layer]*naxes_MASTER[0];
                }
                else // non real time
                {

                    xpos[layer] += vxpix[layer]*CONF_WFTIME_STEP;
                    ypos[layer] += vypix[layer]*CONF_WFTIME_STEP;
                }



                xref = (long) (xpos[layer]);
                yref = (long) (ypos[layer]);

                while(xpos[layer]<0)
                {
                    xpos[layer] += 1.0*naxes_MASTER[0];
                    xposfcnt[layer]++;
                    xref = (long) (xpos[layer]);
                }
                while(xpos[layer]>1.0*naxes_MASTER[0])
                {
                    xpos[layer] -= 1.0*naxes_MASTER[0];
                    xposfcnt[layer]--;
                    xref = (long) (xpos[layer]);
                }

                while(ypos[layer]<0)
                {
                    ypos[layer] += 1.0*naxes_MASTER[1];
                    yposfcnt[layer]++;
                    yref = (long) (ypos[layer]);
                }
                while(ypos[layer]>1.0*naxes_MASTER[1])
                {
                    ypos[layer] -= 1.0*naxes_MASTER[1];
                    yposfcnt[layer]--;
                    yref = (long) (ypos[layer]);
                }

                
                if(xref==naxes_MASTER[0])
                    xref = 0;
                if(yref==naxes_MASTER[1])
                    yref = 0;
                iimax = naxes_MASTER[0]-xref;
                jjmax = naxes_MASTER[1]-yref;
                if(iimax>naxes[0])
                    iimax = naxes[0];
                if(jjmax>naxes[1])
                    jjmax = naxes[1];
                xrefm = xref-naxes_MASTER[0];
                yrefm = yref-naxes_MASTER[1];


                /* make wavefront */
                if(WFprecision == 0) // floating point precision
                {
                   if(BICUBIC==0)            
                   {
					   // bilinear interpolation
						for(ii=0; ii<naxes[0]; ii++)
						for(jj=0; jj<naxes[1]; jj++)
                        {
                            iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                            jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
                            iim = (long) (iimf);
                            jjm = (long) (jjmf);
                            iifrac = iimf-iim;
                            jjfrac = jjmf-jjm;
                            iim1 = iim+1;
                            jjm1 = jjm+1;
                            if(iim==CONF_MASTER_SIZE)
                                iim = 0;
                            if(jjm==CONF_MASTER_SIZE)
                                jjm = 0;
                            if(iim1>CONF_MASTER_SIZE-1)
                                iim1 -= CONF_MASTER_SIZE;
                            if(jjm1>CONF_MASTER_SIZE-1)
                                jjm1 -= CONF_MASTER_SIZE;

                            value = (1.0-iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim];
                            value += (1.0-iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim];
                            value += (iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim1];
                            value += (iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim1];

                            data.image[ID_array1].array.F[jj*naxes[0]+ii] += value;
                            if(CONF_WAVEFRONT_AMPLITUDE==1)
                            {
                                re = data.image[ID_array2].array.CF[jj*naxes[0]+ii].re;
                                im = data.image[ID_array2].array.CF[jj*naxes[0]+ii].im;
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
                            }
                        }
                   
						//fpxypos = fopen("xypos.log", "a");
						//fprintf(fpxypos, "%5ld %4ld    %10.8f %10.8f      %5ld %10.8f %5ld %10.8f    %10.8f %10.8f  %.18g        %.18g   %.18g   %.18g   %.18g\n", vindex, layer, xpos[layer], ypos[layer], iim, iifrac, jjm, jjfrac, 1.0*iim+iifrac, 1.0*jjm+jjfrac, value, data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim], data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim], data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim1], data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim1]);
						//fclose(fpxypos);
					}
					else 
					{
						// bicubic interpolation
						for(ii=0; ii<naxes[0]; ii++)
                        for(jj=0; jj<naxes[1]; jj++)
                        {
                            iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                            jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
							
							iim = (long) (iimf);
                            jjm = (long) (jjmf);
							
							x = iimf-iim;
                            y = jjmf-jjm;
                            
                            
                            iim0 = iim - 1;
                            iim1 = iim;
                            iim2 = iim + 1;
                            iim3 = iim + 2;
							if(iim1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(iim2>CONF_MASTER_SIZE-1)
								iim2 -= CONF_MASTER_SIZE;
							if(iim3>CONF_MASTER_SIZE-1)
								iim3 -= CONF_MASTER_SIZE;
							if(iim0<0)
								iim0 += CONF_MASTER_SIZE;
								
                            jjm0 = jjm - 1;
                            jjm1 = jjm;
                            jjm2 = jjm + 1;
                            jjm3 = jjm + 2;
							if(jjm1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(jjm2>CONF_MASTER_SIZE-1)
								jjm2 -= CONF_MASTER_SIZE;
							if(jjm3>CONF_MASTER_SIZE-1)
								jjm3 -= CONF_MASTER_SIZE;
							if(jjm0<0)
								jjm0 += CONF_MASTER_SIZE;
							
							/*assert(iim0>=0);
							assert(iim1>=0);
							assert(iim2>=0);
							assert(iim3>=0);
							assert(iim0<CONF_MASTER_SIZE);
							assert(iim1<CONF_MASTER_SIZE);
							assert(iim2<CONF_MASTER_SIZE);
							assert(iim3<CONF_MASTER_SIZE);
							assert(jjm0>=0);
							assert(jjm1>=0);
							assert(jjm2>=0);
							assert(jjm3>=0);
							assert(jjm0<CONF_MASTER_SIZE);
							assert(jjm1<CONF_MASTER_SIZE);
							assert(jjm2<CONF_MASTER_SIZE);
							assert(jjm3<CONF_MASTER_SIZE);
							*/
							
							p00 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim0];
							p01 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim0];
							p02 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim0];
							p03 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim0];

							p10 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim1];
							p11 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim1];
							p12 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim1];
							p13 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim1];

							p20 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim2];
							p21 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim2];
							p22 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim2];
							p23 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim2];

							p30 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim3];
							p31 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim3];
							p32 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim3];
							p33 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim3];
							
							
							a00 = p11;
							a01 = -.5*p10 + .5*p12;
							a02 = p10 - 2.5*p11 + 2*p12 - .5*p13;
							a03 = -.5*p10 + 1.5*p11 - 1.5*p12 + .5*p13;
							a10 = -.5*p01 + .5*p21;
							a11 = .25*p00 - .25*p02 - .25*p20 + .25*p22;
							a12 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + .5*p20 - 1.25*p21 + p22 - .25*p23;
							a13 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .25*p20 + .75*p21 - .75*p22 + .25*p23;
							a20 = p01 - 2.5*p11 + 2*p21 - .5*p31;
							a21 = -.5*p00 + .5*p02 + 1.25*p10 - 1.25*p12 - p20 + p22 + .25*p30 - .25*p32;
							a22 = p00 - 2.5*p01 + 2*p02 - .5*p03 - 2.5*p10 + 6.25*p11 - 5*p12 + 1.25*p13 + 2*p20 - 5*p21 + 4*p22 - p23 - .5*p30 + 1.25*p31 - p32 + .25*p33;
							a23 = -.5*p00 + 1.5*p01 - 1.5*p02 + .5*p03 + 1.25*p10 - 3.75*p11 + 3.75*p12 - 1.25*p13 - p20 + 3*p21 - 3*p22 + p23 + .25*p30 - .75*p31 + .75*p32 - .25*p33;
							a30 = -.5*p01 + 1.5*p11 - 1.5*p21 + .5*p31;
							a31 = .25*p00 - .25*p02 - .75*p10 + .75*p12 + .75*p20 - .75*p22 - .25*p30 + .25*p32;
							a32 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + 1.5*p10 - 3.75*p11 + 3*p12 - .75*p13 - 1.5*p20 + 3.75*p21 - 3*p22 + .75*p23 + .5*p30 - 1.25*p31 + p32 - .25*p33;
							a33 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .75*p10 + 2.25*p11 - 2.25*p12 + .75*p13 + .75*p20 - 2.25*p21 + 2.25*p22 - .75*p23 - .25*p30 + .75*p31 - .75*p32 + .25*p33;
							
							x2 = x*x;
							x3 = x2*x;
							y2 = y*y;
							y3 = y2*y;
							
							value = (a00 + a01 * y + a02 * y2 + a03 * y3) + (a10 + a11 * y + a12 * y2 + a13 * y3) * x + (a20 + a21 * y + a22 * y2 + a23 * y3) * x2 + (a30 + a31 * y + a32 * y2 + a33 * y3) * x3;							
						
						
							data.image[ID_array1].array.F[jj*naxes[0]+ii] += value;
							if(CONF_WAVEFRONT_AMPLITUDE==1)
                            {
                                re = data.image[ID_array2].array.CF[jj*naxes[0]+ii].re;
                                im = data.image[ID_array2].array.CF[jj*naxes[0]+ii].im;
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
                                data.image[ID_array2].array.CF[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
                            }

						
						//fpxypos = fopen("xypos3.log", "a");
						//fprintf(fpxypos, "%5ld %4ld    %10.8f %10.8f      %5ld %10.8f %5ld %10.8f    %10.8f %10.8f  %.18g        %.18g   %.18g   %.18g   %.18g\n", vindex, layer, xpos[layer], ypos[layer], iim, x, jjm, y, 1.0*iim+x, 1.0*jjm+y, value, data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim], data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim], data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim1], data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim1]);
						//fclose(fpxypos);
						}
					}
					
                }
                else // double precision
                {
                   if(BICUBIC==0)                
                   {
					   // bilinear interpolation
					   for(ii=0; ii<naxes[0]; ii++)
                       for(jj=0; jj<naxes[1]; jj++)
                        {
                            iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                            jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
                            iim = (long) (iimf);
                            jjm = (long) (jjmf);
                            iifrac = iimf-iim;
                            jjfrac = jjmf-jjm;
                            iim1 = iim+1;
                            jjm1 = jjm+1;
                            if(iim==CONF_MASTER_SIZE)
                                iim = 0;
                            if(jjm==CONF_MASTER_SIZE)
                                jjm = 0;
                            if(iim1>CONF_MASTER_SIZE-1)
                                iim1 -= CONF_MASTER_SIZE;
                            if(jjm1>CONF_MASTER_SIZE-1)
                                jjm1 -= CONF_MASTER_SIZE;

                            value = (1.0-iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.D[jjm*naxes_MASTER[0]+iim];
                            value += (1.0-iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim];
                            value += (iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim1];
                            value += (iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.D[jjm*naxes_MASTER[0]+iim1];

                            data.image[ID_array1].array.D[jj*naxes[0]+ii] += value;
                            if(CONF_WAVEFRONT_AMPLITUDE==1)
                            {
                                re = data.image[ID_array2].array.CD[jj*naxes[0]+ii].re;
                                im = data.image[ID_array2].array.CD[jj*naxes[0]+ii].im;
                                data.image[ID_array2].array.CD[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
                                data.image[ID_array2].array.CD[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
                            }
                        }
					}
					else
					{
						// bicubic interpolation
						for(ii=0; ii<naxes[0]; ii++)
                        for(jj=0; jj<naxes[1]; jj++)
                        {
                            iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                            jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
							
							iim = (long) (iimf);
                            jjm = (long) (jjmf);
							
							x = iimf-iim;
                            y = jjmf-jjm;
                            
                            
                            iim0 = iim - 1;
                            iim1 = iim;
                            iim2 = iim + 1;
                            iim3 = iim + 2;
							if(iim1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(iim2>CONF_MASTER_SIZE-1)
								iim2 -= CONF_MASTER_SIZE;
							if(iim3>CONF_MASTER_SIZE-1)
								iim3 -= CONF_MASTER_SIZE;
							if(iim0<0)
								iim0 += CONF_MASTER_SIZE;
								
                            jjm0 = jjm - 1;
                            jjm1 = jjm;
                            jjm2 = jjm + 1;
                            jjm3 = jjm + 2;
							if(jjm1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(jjm2>CONF_MASTER_SIZE-1)
								jjm2 -= CONF_MASTER_SIZE;
							if(jjm3>CONF_MASTER_SIZE-1)
								jjm3 -= CONF_MASTER_SIZE;
							if(jjm0<0)
								jjm0 += CONF_MASTER_SIZE;

							/*assert(iim0>=0);
							assert(iim1>=0);
							assert(iim2>=0);
							assert(iim3>=0);
							assert(iim0<CONF_MASTER_SIZE);
							assert(iim1<CONF_MASTER_SIZE);
							assert(iim2<CONF_MASTER_SIZE);
							assert(iim3<CONF_MASTER_SIZE);
							assert(jjm0>=0);
							assert(jjm1>=0);
							assert(jjm2>=0);
							assert(jjm3>=0);
							assert(jjm0<CONF_MASTER_SIZE);
							assert(jjm1<CONF_MASTER_SIZE);
							assert(jjm2<CONF_MASTER_SIZE);
							assert(jjm3<CONF_MASTER_SIZE);
					*/
							p00 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim0];
							p01 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim0];
							p02 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim0];
							p03 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim0];

							p10 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim1];
							p11 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim1];
							p12 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim1];
							p13 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim1];

							p20 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim2];
							p21 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim2];
							p22 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim2];
							p23 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim2];

							p30 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim3];
							p31 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim3];
							p32 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim3];
							p33 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim3];
							
							
							a00 = p11;
							a01 = -.5*p10 + .5*p12;
							a02 = p10 - 2.5*p11 + 2*p12 - .5*p13;
							a03 = -.5*p10 + 1.5*p11 - 1.5*p12 + .5*p13;
							a10 = -.5*p01 + .5*p21;
							a11 = .25*p00 - .25*p02 - .25*p20 + .25*p22;
							a12 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + .5*p20 - 1.25*p21 + p22 - .25*p23;
							a13 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .25*p20 + .75*p21 - .75*p22 + .25*p23;
							a20 = p01 - 2.5*p11 + 2*p21 - .5*p31;
							a21 = -.5*p00 + .5*p02 + 1.25*p10 - 1.25*p12 - p20 + p22 + .25*p30 - .25*p32;
							a22 = p00 - 2.5*p01 + 2*p02 - .5*p03 - 2.5*p10 + 6.25*p11 - 5*p12 + 1.25*p13 + 2*p20 - 5*p21 + 4*p22 - p23 - .5*p30 + 1.25*p31 - p32 + .25*p33;
							a23 = -.5*p00 + 1.5*p01 - 1.5*p02 + .5*p03 + 1.25*p10 - 3.75*p11 + 3.75*p12 - 1.25*p13 - p20 + 3*p21 - 3*p22 + p23 + .25*p30 - .75*p31 + .75*p32 - .25*p33;
							a30 = -.5*p01 + 1.5*p11 - 1.5*p21 + .5*p31;
							a31 = .25*p00 - .25*p02 - .75*p10 + .75*p12 + .75*p20 - .75*p22 - .25*p30 + .25*p32;
							a32 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + 1.5*p10 - 3.75*p11 + 3*p12 - .75*p13 - 1.5*p20 + 3.75*p21 - 3*p22 + .75*p23 + .5*p30 - 1.25*p31 + p32 - .25*p33;
							a33 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .75*p10 + 2.25*p11 - 2.25*p12 + .75*p13 + .75*p20 - 2.25*p21 + 2.25*p22 - .75*p23 - .25*p30 + .75*p31 - .75*p32 + .25*p33;
							
							x2 = x*x;
							x3 = x2*x;
							y2 = y*y;
							y3 = y2*y;
							
							value = (a00 + a01 * y + a02 * y2 + a03 * y3) + (a10 + a11 * y + a12 * y2 + a13 * y3) * x + (a20 + a21 * y + a22 * y2 + a23 * y3) * x2 + (a30 + a31 * y + a32 * y2 + a33 * y3) * x3;							
						
					
							data.image[ID_array1].array.D[jj*naxes[0]+ii] += value;
							if(CONF_WAVEFRONT_AMPLITUDE==1)
							{
								re = data.image[ID_array2].array.CD[jj*naxes[0]+ii].re;
								im = data.image[ID_array2].array.CD[jj*naxes[0]+ii].im;
								data.image[ID_array2].array.CD[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
								data.image[ID_array2].array.CD[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
							}
						}
					}
                }



                /* make swavefront */
                if(CONF_MAKE_SWAVEFRONT==1)
                {
                    if(WFprecision == 0)
                    {
						if(BICUBIC==0)
						{   
							for(ii=0; ii<naxes[0]; ii++)
                            for(jj=0; jj<naxes[1]; jj++)
                            {
                                iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                                jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
                                iim = (long) (iimf);
                                jjm = (long) (jjmf);
                                iifrac = iimf-iim;
                                jjfrac = jjmf-jjm;
                                iim1 = iim+1;
                                jjm1 = jjm+1;
                                if(iim==CONF_MASTER_SIZE)
                                    iim = 0;
                                if(jjm==CONF_MASTER_SIZE)
                                    jjm = 0;
                                if(iim1>CONF_MASTER_SIZE-1)
                                    iim1 -= CONF_MASTER_SIZE;
                                if(jjm1>CONF_MASTER_SIZE-1)
                                    jjm1 -= CONF_MASTER_SIZE;

                                value = (1.0-iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim];
                                value += (1.0-iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim];
                                value += (iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim1];
                                value += (iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.F[jjm*naxes_MASTER[0]+iim1];

                                value *= Scoeff;  // multiplicative coeff to go from ref lambda to science lambda

                                data.image[ID_sarray1].array.F[jj*naxes[0]+ii] += value;

                                if(CONF_WAVEFRONT_AMPLITUDE==1)
                                {
                                    re = data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].re;
                                    im = data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].im;
                                    data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
                                    data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
                                }
                            }
						}
						else
						{
							// bicubic interpolation
							for(ii=0; ii<naxes[0]; ii++)
							for(jj=0; jj<naxes[1]; jj++)
							{
                            iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                            jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
							
							iim = (long) (iimf);
                            jjm = (long) (jjmf);
							
							x = iimf-iim;
                            y = jjmf-jjm;
                            
                            
                            iim0 = iim - 1;
                            iim1 = iim;
                            iim2 = iim + 1;
                            iim3 = iim + 2;
							if(iim1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(iim2>CONF_MASTER_SIZE-1)
								iim2 -= CONF_MASTER_SIZE;
							if(iim3>CONF_MASTER_SIZE-1)
								iim3 -= CONF_MASTER_SIZE;
							if(iim0<0)
								iim0 += CONF_MASTER_SIZE;
								
                            jjm0 = jjm - 1;
                            jjm1 = jjm;
                            jjm2 = jjm + 1;
                            jjm3 = jjm + 2;
							if(jjm1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(jjm2>CONF_MASTER_SIZE-1)
								jjm2 -= CONF_MASTER_SIZE;
							if(jjm3>CONF_MASTER_SIZE-1)
								jjm3 -= CONF_MASTER_SIZE;
							if(jjm0<0)
								jjm0 += CONF_MASTER_SIZE;

							/*assert(iim0>=0);
							assert(iim1>=0);
							assert(iim2>=0);
							assert(iim3>=0);
							assert(iim0<CONF_MASTER_SIZE);
							assert(iim1<CONF_MASTER_SIZE);
							assert(iim2<CONF_MASTER_SIZE);
							assert(iim3<CONF_MASTER_SIZE);
							assert(jjm0>=0);
							assert(jjm1>=0);
							assert(jjm2>=0);
							assert(jjm3>=0);
							assert(jjm0<CONF_MASTER_SIZE);
							assert(jjm1<CONF_MASTER_SIZE);
							assert(jjm2<CONF_MASTER_SIZE);
							assert(jjm3<CONF_MASTER_SIZE);
					*/
							p00 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim0];
							p01 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim0];
							p02 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim0];
							p03 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim0];

							p10 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim1];
							p11 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim1];
							p12 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim1];
							p13 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim1];

							p20 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim2];
							p21 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim2];
							p22 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim2];
							p23 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim2];

							p30 = data.image[ID_TML[layer]].array.F[jjm0*naxes_MASTER[0]+iim3];
							p31 = data.image[ID_TML[layer]].array.F[jjm1*naxes_MASTER[0]+iim3];
							p32 = data.image[ID_TML[layer]].array.F[jjm2*naxes_MASTER[0]+iim3];
							p33 = data.image[ID_TML[layer]].array.F[jjm3*naxes_MASTER[0]+iim3];
							
							
							a00 = p11;
							a01 = -.5*p10 + .5*p12;
							a02 = p10 - 2.5*p11 + 2*p12 - .5*p13;
							a03 = -.5*p10 + 1.5*p11 - 1.5*p12 + .5*p13;
							a10 = -.5*p01 + .5*p21;
							a11 = .25*p00 - .25*p02 - .25*p20 + .25*p22;
							a12 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + .5*p20 - 1.25*p21 + p22 - .25*p23;
							a13 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .25*p20 + .75*p21 - .75*p22 + .25*p23;
							a20 = p01 - 2.5*p11 + 2*p21 - .5*p31;
							a21 = -.5*p00 + .5*p02 + 1.25*p10 - 1.25*p12 - p20 + p22 + .25*p30 - .25*p32;
							a22 = p00 - 2.5*p01 + 2*p02 - .5*p03 - 2.5*p10 + 6.25*p11 - 5*p12 + 1.25*p13 + 2*p20 - 5*p21 + 4*p22 - p23 - .5*p30 + 1.25*p31 - p32 + .25*p33;
							a23 = -.5*p00 + 1.5*p01 - 1.5*p02 + .5*p03 + 1.25*p10 - 3.75*p11 + 3.75*p12 - 1.25*p13 - p20 + 3*p21 - 3*p22 + p23 + .25*p30 - .75*p31 + .75*p32 - .25*p33;
							a30 = -.5*p01 + 1.5*p11 - 1.5*p21 + .5*p31;
							a31 = .25*p00 - .25*p02 - .75*p10 + .75*p12 + .75*p20 - .75*p22 - .25*p30 + .25*p32;
							a32 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + 1.5*p10 - 3.75*p11 + 3*p12 - .75*p13 - 1.5*p20 + 3.75*p21 - 3*p22 + .75*p23 + .5*p30 - 1.25*p31 + p32 - .25*p33;
							a33 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .75*p10 + 2.25*p11 - 2.25*p12 + .75*p13 + .75*p20 - 2.25*p21 + 2.25*p22 - .75*p23 - .25*p30 + .75*p31 - .75*p32 + .25*p33;
							
							x2 = x*x;
							x3 = x2*x;
							y2 = y*y;
							y3 = y2*y;
							
							value = (a00 + a01 * y + a02 * y2 + a03 * y3) + (a10 + a11 * y + a12 * y2 + a13 * y3) * x + (a20 + a21 * y + a22 * y2 + a23 * y3) * x2 + (a30 + a31 * y + a32 * y2 + a33 * y3) * x3;							
						
						
							value *= Scoeff;  // multiplicative coeff to go from ref lambda to science lambda
						
							data.image[ID_sarray1].array.F[jj*naxes[0]+ii] += value;
						
							if(CONF_WAVEFRONT_AMPLITUDE==1)
							{
								re = data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].re;
								im = data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].im;
								data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
								data.image[ID_sarray2].array.CF[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
							}
							}
						}
                    }
                    else // double precision
                    {
						if(BICUBIC==0)
						{   
							for(ii=0; ii<naxes[0]; ii++)
                            for(jj=0; jj<naxes[1]; jj++)
                            {
                                iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                                jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
                                iim = (long) (iimf);
                                jjm = (long) (jjmf);
                                iifrac = iimf-iim;
                                jjfrac = jjmf-jjm;
                                iim1 = iim+1;
                                jjm1 = jjm+1;
                                if(iim==CONF_MASTER_SIZE)
                                    iim = 0;
                                if(jjm==CONF_MASTER_SIZE)
                                    jjm = 0;
                                if(iim1>CONF_MASTER_SIZE-1)
                                    iim1 -= CONF_MASTER_SIZE;
                                if(jjm1>CONF_MASTER_SIZE-1)
                                    jjm1 -= CONF_MASTER_SIZE;

                                value = (1.0-iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.D[jjm*naxes_MASTER[0]+iim];
                                value += (1.0-iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim];
                                value += (iifrac)*(jjfrac)*data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim1];
                                value += (iifrac)*(1.0-jjfrac)*data.image[ID_TML[layer]].array.D[jjm*naxes_MASTER[0]+iim1];

                                value *= Scoeff;  // multiplicative coeff to go from ref lambda to science lambda

                                data.image[ID_sarray1].array.D[jj*naxes[0]+ii] += value;

                                if(CONF_WAVEFRONT_AMPLITUDE==1)
                                {
                                    re = data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].re;
                                    im = data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].im;
                                    data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
                                    data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
                                }
                            }
                        }
                        else
                        {
							// bicubic interpolation
							for(ii=0; ii<naxes[0]; ii++)
							for(jj=0; jj<naxes[1]; jj++)
							{
                            iimf = fmod((xpos[layer]+ii), 1.0*naxes_MASTER[0]);
                            jjmf = fmod((ypos[layer]+jj), 1.0*naxes_MASTER[1]);
							
							iim = (long) (iimf);
                            jjm = (long) (jjmf);
							
							x = iimf-iim;
                            y = jjmf-jjm;
                            
                            
                            iim0 = iim - 1;
                            iim1 = iim;
                            iim2 = iim + 1;
                            iim3 = iim + 2;
							if(iim1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(iim2>CONF_MASTER_SIZE-1)
								iim2 -= CONF_MASTER_SIZE;
							if(iim3>CONF_MASTER_SIZE-1)
								iim3 -= CONF_MASTER_SIZE;
							if(iim0<0)
								iim0 += CONF_MASTER_SIZE;
								
                            jjm0 = jjm - 1;
                            jjm1 = jjm;
                            jjm2 = jjm + 1;
                            jjm3 = jjm + 2;
							if(jjm1>CONF_MASTER_SIZE-1)
								iim1 -= CONF_MASTER_SIZE;
							if(jjm2>CONF_MASTER_SIZE-1)
								jjm2 -= CONF_MASTER_SIZE;
							if(jjm3>CONF_MASTER_SIZE-1)
								jjm3 -= CONF_MASTER_SIZE;
							if(jjm0<0)
								jjm0 += CONF_MASTER_SIZE;

							/*assert(iim0>=0);
							assert(iim1>=0);
							assert(iim2>=0);
							assert(iim3>=0);
							assert(iim0<CONF_MASTER_SIZE);
							assert(iim1<CONF_MASTER_SIZE);
							assert(iim2<CONF_MASTER_SIZE);
							assert(iim3<CONF_MASTER_SIZE);
							assert(jjm0>=0);
							assert(jjm1>=0);
							assert(jjm2>=0);
							assert(jjm3>=0);
							assert(jjm0<CONF_MASTER_SIZE);
							assert(jjm1<CONF_MASTER_SIZE);
							assert(jjm2<CONF_MASTER_SIZE);
							assert(jjm3<CONF_MASTER_SIZE);
							*/

					
							p00 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim0];
							p01 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim0];
							p02 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim0];
							p03 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim0];

							p10 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim1];
							p11 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim1];
							p12 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim1];
							p13 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim1];

							p20 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim2];
							p21 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim2];
							p22 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim2];
							p23 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim2];

							p30 = data.image[ID_TML[layer]].array.D[jjm0*naxes_MASTER[0]+iim3];
							p31 = data.image[ID_TML[layer]].array.D[jjm1*naxes_MASTER[0]+iim3];
							p32 = data.image[ID_TML[layer]].array.D[jjm2*naxes_MASTER[0]+iim3];
							p33 = data.image[ID_TML[layer]].array.D[jjm3*naxes_MASTER[0]+iim3];
							
							
							a00 = p11;
							a01 = -.5*p10 + .5*p12;
							a02 = p10 - 2.5*p11 + 2*p12 - .5*p13;
							a03 = -.5*p10 + 1.5*p11 - 1.5*p12 + .5*p13;
							a10 = -.5*p01 + .5*p21;
							a11 = .25*p00 - .25*p02 - .25*p20 + .25*p22;
							a12 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + .5*p20 - 1.25*p21 + p22 - .25*p23;
							a13 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .25*p20 + .75*p21 - .75*p22 + .25*p23;
							a20 = p01 - 2.5*p11 + 2*p21 - .5*p31;
							a21 = -.5*p00 + .5*p02 + 1.25*p10 - 1.25*p12 - p20 + p22 + .25*p30 - .25*p32;
							a22 = p00 - 2.5*p01 + 2*p02 - .5*p03 - 2.5*p10 + 6.25*p11 - 5*p12 + 1.25*p13 + 2*p20 - 5*p21 + 4*p22 - p23 - .5*p30 + 1.25*p31 - p32 + .25*p33;
							a23 = -.5*p00 + 1.5*p01 - 1.5*p02 + .5*p03 + 1.25*p10 - 3.75*p11 + 3.75*p12 - 1.25*p13 - p20 + 3*p21 - 3*p22 + p23 + .25*p30 - .75*p31 + .75*p32 - .25*p33;
							a30 = -.5*p01 + 1.5*p11 - 1.5*p21 + .5*p31;
							a31 = .25*p00 - .25*p02 - .75*p10 + .75*p12 + .75*p20 - .75*p22 - .25*p30 + .25*p32;
							a32 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + 1.5*p10 - 3.75*p11 + 3*p12 - .75*p13 - 1.5*p20 + 3.75*p21 - 3*p22 + .75*p23 + .5*p30 - 1.25*p31 + p32 - .25*p33;
							a33 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .75*p10 + 2.25*p11 - 2.25*p12 + .75*p13 + .75*p20 - 2.25*p21 + 2.25*p22 - .75*p23 - .25*p30 + .75*p31 - .75*p32 + .25*p33;
							
							x2 = x*x;
							x3 = x2*x;
							y2 = y*y;
							y3 = y2*y;
							
							value = (a00 + a01 * y + a02 * y2 + a03 * y3) + (a10 + a11 * y + a12 * y2 + a13 * y3) * x + (a20 + a21 * y + a22 * y2 + a23 * y3) * x2 + (a30 + a31 * y + a32 * y2 + a33 * y3) * x3;							
						
						
							value *= Scoeff;  // multiplicative coeff to go from ref lambda to science lambda
						
							data.image[ID_sarray1].array.D[jj*naxes[0]+ii] += value;
						
							if(CONF_WAVEFRONT_AMPLITUDE==1)
							{
								re = data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].re;
								im = data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].im;
								data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].re = re*cos(value)-im*sin(value);
								data.image[ID_sarray2].array.CD[jj*naxes[0]+ii].im = re*sin(value)+im*cos(value);
							}
							}
						}
                    }
                }
            }
           
            
            

            // REFERENCE LAMBDA
            if(WFprecision == 0)
            {
            for(ii=0; ii<naxesout[0]; ii++)
                for(jj=0; jj<naxesout[1]; jj++)
                {
                    ii1 = ii+(naxes[0]-naxesout[0])/2;
                    jj1 = jj+(naxes[1]-naxesout[1])/2;
                    data.image[IDout_array_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = data.image[ID_array1].array.F[jj1*naxes[0]+ii1];
                }
            }
            else
            {
            for(ii=0; ii<naxesout[0]; ii++)
                for(jj=0; jj<naxesout[1]; jj++)
                {
                    ii1 = ii+(naxes[0]-naxesout[0])/2;
                    jj1 = jj+(naxes[1]-naxesout[1])/2;
                    data.image[IDout_array_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = data.image[ID_array1].array.D[jj1*naxes[0]+ii1];
                }
            }

            if(WFprecision == 0)
            {
            if(CONF_WAVEFRONT_AMPLITUDE==1)
            {
                for(ii=0; ii<naxesout[0]; ii++)
                    for(jj=0; jj<naxesout[1]; jj++)
                    {
                        ii1 = ii+(naxes[0]-naxesout[0])/2;
                        jj1 = jj+(naxes[1]-naxesout[1])/2;
                        array[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re = data.image[ID_array2].array.CF[jj1*naxes[0]+ii1].re;
                        array[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im = data.image[ID_array2].array.CF[jj1*naxes[0]+ii1].im;

                        re = array[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re;
                        im = array[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im;
                        data.image[IDout_array_amp].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = sqrt(re*re+im*im);
                        pha = atan2(im,re);
                        data.image[IDout_array_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = pha + 2.0*M_PI*((long) (data.image[IDout_array_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii]/2.0/M_PI+1000.5) - 1000.0);
                    }
            }
			}
			else
            {
                for(ii=0; ii<naxesout[0]; ii++)
                    for(jj=0; jj<naxesout[1]; jj++)
                    {
                        ii1 = ii+(naxes[0]-naxesout[0])/2;
                        jj1 = jj+(naxes[1]-naxesout[1])/2;
                        array_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re = data.image[ID_array2].array.CD[jj1*naxes[0]+ii1].re;
                        array_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im = data.image[ID_array2].array.CD[jj1*naxes[0]+ii1].im;

                        re = array_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re;
                        im = array_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im;
                        data.image[IDout_array_amp].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = sqrt(re*re+im*im);
                        pha = atan2(im,re);
                        data.image[IDout_array_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = pha + 2.0*M_PI*((long) (data.image[IDout_array_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii]/2.0/M_PI+1000.5) - 1000.0);
                    }
            }
			




            // WRITE CURRENT WF TO SHARED MEMORY

            if(CONF_SHM_OUTPUT == 1)
            {
                if(CONF_WAVEFRONT_AMPLITUDE==0)
                {
                    if(WFprecision == 0)
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                            data.image[IDshmpha].array.F[ii] = data.image[ID_array1].array.F[frame*naxesout[0]*naxesout[1]+ii];
                    }
                    else
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                            data.image[IDshmpha].array.D[ii] = data.image[ID_array1].array.D[frame*naxesout[0]*naxesout[1]+ii];
                    }
                }
                else
                {
                    if(WFprecision == 0)
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                        {
                            data.image[IDshmpha].array.F[ii] = data.image[IDout_array_pha].array.F[frame*naxesout[0]*naxesout[1]+ii];
                            data.image[IDshmamp].array.F[ii] = data.image[IDout_array_amp].array.F[frame*naxesout[0]*naxesout[1]+ii];
                        }
                    }
                    else
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                        {
                            data.image[IDshmpha].array.D[ii] = data.image[IDout_array_pha].array.D[frame*naxesout[0]*naxesout[1]+ii];
                            data.image[IDshmamp].array.D[ii] = data.image[IDout_array_amp].array.D[frame*naxesout[0]*naxesout[1]+ii];
                        }
                    }
                }
            }



            // SCIENCE LAMBDA
            if(CONF_MAKE_SWAVEFRONT==1)
            {
                if(WFprecision == 0)
                {
                    for(ii=0; ii<naxesout[0]; ii++)
                        for(jj=0; jj<naxesout[1]; jj++)
                        {
                            ii1 = ii+(naxes[0]-naxesout[0])/2;
                            jj1 = jj+(naxes[1]-naxesout[1])/2;
                            data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = data.image[ID_sarray1].array.F[jj1*naxes[0]+ii1];
                        }
                }
                else
                {
                    for(ii=0; ii<naxesout[0]; ii++)
                        for(jj=0; jj<naxesout[1]; jj++)
                        {
                            ii1 = ii+(naxes[0]-naxesout[0])/2;
                            jj1 = jj+(naxes[1]-naxesout[1])/2;
                            data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = data.image[ID_sarray1].array.D[jj1*naxes[0]+ii1];
                        }
                }


                if(CONF_WAVEFRONT_AMPLITUDE==1)
                {
                    if(WFprecision == 0)
                    {
                        for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                        {
                            data.image[IDpeakpha_re_bin].array.F[ii2] = 0.0;
                            data.image[IDpeakpha_im_bin].array.F[ii2] = 0.0;
                            data.image[IDpeakpha_bin].array.F[ii2] = 0.0;
                            data.image[IDpeakpha_bin_ch].array.F[ii2] = 0.0;
                        }
                    }
                    else
                    {
                        for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                        {
                            data.image[IDpeakpha_re_bin].array.D[ii2] = 0.0;
                            data.image[IDpeakpha_im_bin].array.D[ii2] = 0.0;
                            data.image[IDpeakpha_bin].array.D[ii2] = 0.0;
                            data.image[IDpeakpha_bin_ch].array.D[ii2] = 0.0;
                        }
                    }

                    peakpha_re = 0.0;
                    peakpha_im = 0.0;

                    if(WFprecision == 0)
                    {
                        for(ii=0; ii<naxesout[0]; ii++)
                            for(jj=0; jj<naxesout[1]; jj++)
                            {
                                ii1 = ii+(naxes[0]-naxesout[0])/2;
                                jj1 = jj+(naxes[1]-naxesout[1])/2;
                                sarray[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re = data.image[ID_sarray2].array.CF[jj1*naxes[0]+ii1].re;
                                sarray[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im = data.image[ID_sarray2].array.CF[jj1*naxes[0]+ii1].im;
                                re = sarray[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re;
                                im = sarray[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im;
                                data.image[IDout_sarray_amp].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = sqrt(re*re+im*im);
                                pha = atan2(im,re);
                                data.image[IDpeakpha_re].array.F[jj*naxesout[0]+ii] = cos(data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii]-pha);
                                data.image[IDpeakpha_im].array.F[jj*naxesout[0]+ii] = sin(data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii]-pha);
                                data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = pha;
                                ii2 = (long) (1.0*ii/naxesout[0]*xsizepeakpha);
                                jj2 = (long) (1.0*jj/naxesout[1]*ysizepeakpha);

                                if((ii2<xsizepeakpha)&&(jj2<ysizepeakpha))
                                {
                                    data.image[IDpeakpha_re_bin].array.F[jj2*xsizepeakpha+ii2] += cos(data.image[ID_sarray1].array.F[jj1*naxes[0]+ii1]-pha);
                                    data.image[IDpeakpha_im_bin].array.F[jj2*xsizepeakpha+ii2] += sin(data.image[ID_sarray1].array.F[jj1*naxes[0]+ii1]-pha);
                                }
                            }
                    }
                    else
                    {
                        for(ii=0; ii<naxesout[0]; ii++)
                            for(jj=0; jj<naxesout[1]; jj++)
                            {
                                ii1 = ii+(naxes[0]-naxesout[0])/2;
                                jj1 = jj+(naxes[1]-naxesout[1])/2;
                                sarray_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re = data.image[ID_sarray2].array.CD[jj1*naxes[0]+ii1].re;
                                sarray_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im = data.image[ID_sarray2].array.CD[jj1*naxes[0]+ii1].im;
                                re = sarray_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re;
                                im = sarray_double[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im;
                                data.image[IDout_sarray_amp].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = sqrt(re*re+im*im);
                                pha = atan2(im,re);
                                data.image[IDpeakpha_re].array.D[jj*naxesout[0]+ii] = cos(data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii]-pha);
                                data.image[IDpeakpha_im].array.D[jj*naxesout[0]+ii] = sin(data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii]-pha);
                                data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = pha;
                                ii2 = (long) (1.0*ii/naxesout[0]*xsizepeakpha);
                                jj2 = (long) (1.0*jj/naxesout[1]*ysizepeakpha);

                                if((ii2<xsizepeakpha)&&(jj2<ysizepeakpha))
                                {
                                    data.image[IDpeakpha_re_bin].array.D[jj2*xsizepeakpha+ii2] += cos(data.image[ID_sarray1].array.D[jj1*naxes[0]+ii1]-pha);
                                    data.image[IDpeakpha_im_bin].array.D[jj2*xsizepeakpha+ii2] += sin(data.image[ID_sarray1].array.D[jj1*naxes[0]+ii1]-pha);
                                }
                            }
                    }


                    //peakpha = atan2(peakpha_im, peakpha_re);
                    //printf("peak pha = %lf\n", peakpha/2.0/M_PI);


                    peakpha = 0.0;
                    if(WFprecision == 0)
                    {
                        for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                        {
                            data.image[IDpeakpha_bin].array.F[ii2] = atan2(data.image[IDpeakpha_im_bin].array.F[ii2], data.image[IDpeakpha_re_bin].array.F[ii2]);
                            //	while(data.image[IDpeakpha_bin].array.F[ii2]<0.0)
                            //	data.image[IDpeakpha_bin].array.F[ii2] += 2.0*M_PI;
                        }
                    }
                    else
                    {
                        for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                        {
                            data.image[IDpeakpha_bin].array.D[ii2] = atan2(data.image[IDpeakpha_im_bin].array.D[ii2], data.image[IDpeakpha_re_bin].array.D[ii2]);
                        }
                    }

                    chcnt = 1;
                    chcnt0cnt = 0;
                    chiter = 0;
                    plim = 1.0;
                    while((plim>0.51)&&(chiter<1000)&&(chcnt0cnt<5))
                    {
                        chiter++;
                        chcnt = 0;
                        if(WFprecision == 0)
                        {
                            for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                                data.image[IDpeakpha_bin_ch].array.F[ii2] = 0.0;
                        }
                        else
                        {
                            for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                                data.image[IDpeakpha_bin_ch].array.D[ii2] = 0.0;
                        }

                        if(WFprecision == 0)
                        {
                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += 0.2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = (jj2+1)*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += 0.2;
                                    }
                                }


                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = (jj2+1)*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += 0.2;
                                    }
                                }


                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = (jj2+1)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += 0.2;
                                    }
                                }


                            pcoeff2;


                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }


                            for(ii2=0; ii2<xsizepeakpha; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2+1;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2+2;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = (jj2+1)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = (jj2+1)*xsizepeakpha+ii2+2;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.F[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.F[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.F[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.F[index2] += pcoeff2;
                                    }
                                }



                            plim = 0.0;
                            for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                                if(fabs(data.image[IDpeakpha_bin_ch].array.F[ii2])>plim)
                                    plim = fabs(data.image[IDpeakpha_bin_ch].array.F[ii2]);
                            plim -= 0.001;

                            if(plim<0.5)
                                plim = 0.5;

                            //	plim = 2.39;

                            //                        save_fits("peakpha_bin", "!peakpha_bin.fits");

                            for(ii2=0; ii2<xsizepeakpha; ii2++)
                                for(jj2=0; jj2<ysizepeakpha; jj2++)
                                {
                                    if(data.image[IDpeakpha_bin_ch].array.F[jj2*xsizepeakpha+ii2]>plim)
                                    {
                                        if((ii2>1)&&(jj2>1)&&(ii2<xsizepeakpha-2)&&(jj2<ysizepeakpha-2))
                                            chcnt ++;
                                        data.image[IDpeakpha_bin].array.F[jj2*xsizepeakpha+ii2] += 2.0*M_PI;
                                    }
                                    if(data.image[IDpeakpha_bin_ch].array.F[jj2*xsizepeakpha+ii2]<-plim)
                                    {
                                        if((ii2>1)&&(jj2>1)&&(ii2<xsizepeakpha-2)&&(jj2<ysizepeakpha-2))
                                            chcnt ++;
                                        data.image[IDpeakpha_bin].array.F[jj2*xsizepeakpha+ii2] -= 2.0*M_PI;
                                    }

                                }
                        }
                        else
                        {
                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += 0.2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = (jj2+1)*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += 0.2;
                                    }
                                }


                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = (jj2+1)*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += 0.2;
                                    }
                                }


                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = (jj2+1)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= 0.2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= 0.2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += 0.2;
                                    }
                                }


                            pcoeff2;


                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha; jj2++)
                                {
                                    index1 = jj2*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }


                            for(ii2=0; ii2<xsizepeakpha; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2+1;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-1; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+1;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2+2;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-2; jj2++)
                                {
                                    index1 = (jj2+2)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = (jj2+1)*xsizepeakpha+ii2;
                                    index2 = jj2*xsizepeakpha+ii2+2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }

                            for(ii2=0; ii2<xsizepeakpha-2; ii2++)
                                for(jj2=0; jj2<ysizepeakpha-1; jj2++)
                                {
                                    index1 = (jj2+1)*xsizepeakpha+ii2+2;
                                    index2 = jj2*xsizepeakpha+ii2;
                                    pv1 = data.image[IDpeakpha_bin].array.D[index1];
                                    pv2 = data.image[IDpeakpha_bin].array.D[index2];
                                    if(pv2>pv1+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] += pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] -= pcoeff2;
                                    }
                                    if(pv1>pv2+M_PI)
                                    {
                                        data.image[IDpeakpha_bin_ch].array.D[index1] -= pcoeff2;
                                        data.image[IDpeakpha_bin_ch].array.D[index2] += pcoeff2;
                                    }
                                }



                            plim = 0.0;
                            for(ii2=0; ii2<xsizepeakpha*ysizepeakpha; ii2++)
                                if(fabs(data.image[IDpeakpha_bin_ch].array.D[ii2])>plim)
                                    plim = fabs(data.image[IDpeakpha_bin_ch].array.D[ii2]);
                            plim -= 0.001;

                            if(plim<0.5)
                                plim = 0.5;

                            //	plim = 2.39;

                            //                        save_fits("peakpha_bin", "!peakpha_bin.fits");

                            for(ii2=0; ii2<xsizepeakpha; ii2++)
                                for(jj2=0; jj2<ysizepeakpha; jj2++)
                                {
                                    if(data.image[IDpeakpha_bin_ch].array.D[jj2*xsizepeakpha+ii2]>plim)
                                    {
                                        if((ii2>1)&&(jj2>1)&&(ii2<xsizepeakpha-2)&&(jj2<ysizepeakpha-2))
                                            chcnt ++;
                                        data.image[IDpeakpha_bin].array.D[jj2*xsizepeakpha+ii2] += 2.0*M_PI;
                                    }
                                    if(data.image[IDpeakpha_bin_ch].array.D[jj2*xsizepeakpha+ii2]<-plim)
                                    {
                                        if((ii2>1)&&(jj2>1)&&(ii2<xsizepeakpha-2)&&(jj2<ysizepeakpha-2))
                                            chcnt ++;
                                        data.image[IDpeakpha_bin].array.D[jj2*xsizepeakpha+ii2] -= 2.0*M_PI;
                                    }

                                }


                        }
                        //                    printf("chiter = %ld    [%ld]  %f\n", chiter, chcnt, plim);
                        //                      save_fits("peakpha_bin_ch", "!peakpha_bin_ch.fits");

                        if(chcnt==0)
                            chcnt0cnt++;
                        else
                            chcnt0cnt = 0;
                    }


                    /*          list_image_ID();
                              save_fits("peakpha_bin", "!peakpha2_bin.fits");
                              save_fits("peakphare_bin", "!peakpha_re_bin.fits");
                              save_fits("peakphaim_bin", "!peakpha_im_bin.fits");
                    */

                    if(WFprecision == 0)
                    {
                        for(ii=0; ii<naxesout[0]; ii++)
                            for(jj=0; jj<naxesout[1]; jj++)
                            {
                                ii2 = (long) (1.0*ii/naxesout[0]*xsizepeakpha);
                                jj2 = (long) (1.0*jj/naxesout[1]*ysizepeakpha);

                                if((ii2<xsizepeakpha)&&(jj2<ysizepeakpha))
                                    peakpha = data.image[IDpeakpha_bin].array.F[jj2*xsizepeakpha+ii2];

                                ii1 = ii+(naxes[0]-naxesout[0])/2;
                                jj1 = jj+(naxes[1]-naxesout[1])/2;

                                pha = data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii];
                                data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = pha + 2.0*M_PI*((long) ((data.image[ID_sarray1].array.F[jj1*naxes[0]+ii1]-pha-peakpha)/2.0/M_PI+1000.5) - 1000.0);
                            }
                    }
                    else
                    {
                        for(ii=0; ii<naxesout[0]; ii++)
                            for(jj=0; jj<naxesout[1]; jj++)
                            {
                                ii2 = (long) (1.0*ii/naxesout[0]*xsizepeakpha);
                                jj2 = (long) (1.0*jj/naxesout[1]*ysizepeakpha);

                                if((ii2<xsizepeakpha)&&(jj2<ysizepeakpha))
                                    peakpha = data.image[IDpeakpha_bin].array.D[jj2*xsizepeakpha+ii2];

                                ii1 = ii+(naxes[0]-naxesout[0])/2;
                                jj1 = jj+(naxes[1]-naxesout[1])/2;

                                pha = data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii];
                                data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii] = pha + 2.0*M_PI*((long) ((data.image[ID_sarray1].array.D[jj1*naxes[0]+ii1]-pha-peakpha)/2.0/M_PI+1000.5) - 1000.0);
                            }
                    }
                }
            }



            /*
                        if(make_cwavefront==1)
                        {
                            for(ii=0; ii<naxesout[0]; ii++)
                                for(jj=0; jj<naxesout[1]; jj++)
                                {
                                    ii1 = ii+(naxes[0]-naxesout[0])/2;
                                    jj1 = jj+(naxes[1]-naxesout[1])/2;
                                    carray[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].re = data.image[ID_carray2].array.CF[jj1*naxes[0]+ii1].re;
                                    carray[frame*naxesout[0]*naxesout[1]+jj*naxesout[0]+ii].im = data.image[ID_carray2].array.CF[jj1*naxes[0]+ii1].im;
                                }
                        }

            */


            // WRITE CURRENT WF TO SHARED MEMORY
            if((CONF_SHM_SOUTPUT == 1)&&(CONF_MAKE_SWAVEFRONT==1))
            {
                switch (CONF_SHM_SOUTPUTM) {
                case 1 :
                    coeff = SLAMBDA/2.0/M_PI;
                    break;
                case 2 :
                    coeff = SLAMBDA/2.0/M_PI*1e6;
                    break;
                default :
                    coeff = 1.0;
                    break;
                }



                if(CONF_WAVEFRONT_AMPLITUDE==0)
                {
                    data.image[IDshmspha].md[0].write = 1;
                    data.image[IDshmspha].kw[0].value.numf = tnowdouble;
                    if(WFprecision == 0)
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                            data.image[IDshmspha].array.F[ii] = data.image[ID_sarray1].array.F[frame*naxesout[0]*naxesout[1]+ii]*coeff;
                    }
                    else
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                            data.image[IDshmspha].array.D[ii] = data.image[ID_sarray1].array.D[frame*naxesout[0]*naxesout[1]+ii]*coeff;
                    }
                    data.image[IDshmspha].md[0].cnt0++;
                    data.image[IDshmspha].md[0].write = 0;
                }
                else
                {
                    data.image[IDshmspha].md[0].write = 1;
                    data.image[IDshmsamp].md[0].write = 1;
                    data.image[IDshmspha].kw[0].value.numf = tnowdouble;
                    data.image[IDshmsamp].kw[0].value.numf = tnowdouble;
                    if(WFprecision == 0)
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                        {
                            data.image[IDshmspha].array.F[ii] = data.image[IDout_sarray_pha].array.F[frame*naxesout[0]*naxesout[1]+ii]*coeff;
                            data.image[IDshmsamp].array.F[ii] = data.image[IDout_sarray_amp].array.F[frame*naxesout[0]*naxesout[1]+ii];
                        }
                    }
                    else
                    {
                        for(ii=0; ii<naxesout[0]*naxesout[1]; ii++)
                        {
                            data.image[IDshmspha].array.D[ii] = data.image[IDout_sarray_pha].array.D[frame*naxesout[0]*naxesout[1]+ii]*coeff;
                            data.image[IDshmsamp].array.D[ii] = data.image[IDout_sarray_amp].array.D[frame*naxesout[0]*naxesout[1]+ii];
                        }
                    }
                    data.image[IDshmspha].md[0].cnt0++;
                    data.image[IDshmsamp].md[0].cnt0++;
                    data.image[IDshmspha].md[0].write = 0;
                    data.image[IDshmsamp].md[0].write = 0;
                }

            }
        }


        if(CONF_WFOUTPUT==1) // WRITE REFERENCE LAMBDA
        {
            sprintf(fname1,"!%s%08ld.%09ld.pha.fits", CONF_WF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
            sprintf(fname2,"!%s%08ld.%09ld.amp.fits", CONF_WF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));

            if(WFprecision == 0)
                save_fl_fits("outarraypha", fname1);
            else
                save_db_fits("outarraypha", fname1);


            if(CONF_WAVEFRONT_AMPLITUDE==1)
                if(WFprecision == 0)
                    save_fl_fits("outarrayamp",fname2);
                else
                    save_db_fits("outarrayamp",fname2);
        }
        else if (CONF_WFOUTPUT==0)
        {
            // CREATE EMPTY FILES
            sprintf(fname1,"%s%08ld.%09ld.pha.fits", CONF_WF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
            sprintf(fname2,"%s%08ld.%09ld.amp.fits", CONF_WF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
            sprintf(command,"touch %s",fname1);
            r = system(command);
            if(CONF_WAVEFRONT_AMPLITUDE==1)
            {
                sprintf(command,"touch %s", fname2);
                r = system(command);
            }
        }


        if((CONF_MAKE_SWAVEFRONT==1)&&(CONF_SWF_WRITE2DISK==1)) // WRITE SCIENCE LAMBDA
        {
            printf("WRITING WAVEFRONT FILE ...");
            fflush(stdout);
            sprintf(fname1,"!%s%08ld.%09ld.pha.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
            sprintf(fname2,"!%s%08ld.%09ld.amp.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));

            if(WFprecision == 0)
                save_fl_fits("outsarraypha", fname1);
            else
                save_db_fits("outsarraypha", fname1);

            printf(" - ");
            fflush(stdout);

            if(CONF_WAVEFRONT_AMPLITUDE==1)
                if(WFprecision == 0)
                    save_fl_fits("outsarrayamp", fname2);
                else
                    save_db_fits("outsarrayamp", fname2);

            printf("\n");
            fflush(stdout);
        }
        
    }

    delete_image_ID("array1");
    if(WFprecision == 0)
        free(array);
    else
        free(array_double);

    if(CONF_MAKE_SWAVEFRONT==1)
    {
        delete_image_ID("sarray1");
        if(WFprecision == 0)
            free(sarray);
        else
            free(sarray_double);
    }
    /*  if(make_cwavefront==1)
      {
          delete_image_ID("carray1");
          free(carray);
      }*/

    free(SLAYER_ALT);
    free(super_layer_index);
    free(xpos);
    free(ypos);
    free(xpos0);
    free(ypos0);
    free(xposfcnt);
    free(yposfcnt);
    free(vxpix);
    free(vypix);


    free(LAYER_ALT);
    free(LAYER_CN2);
    free(LAYER_SPD);
    free(LAYER_DIR);
    free(LAYER_OUTERSCALE);
    free(LAYER_INNERSCALE);
    free(LAYER_SIGMAWSPEED);
    free(LAYER_LWIND);
    free(ID_TM);
    free(ID_TML);

    free(naxes);
    free(naxesout);

    return(0);
}




















int contract_wavefront_series(const char *in_prefix, const char *out_prefix, long NB_files)
{
    /* contracts the wavefront series by a factor of 2 */
    char fname[200];
    long IDamp,IDpha,IDoutamp,IDoutpha;
    long ii,jj,kk;
    long i,j;
    long naxes[3];
    long naxes_out[3];
    float re,im,amp,pha;
    long index;
    float P;
    long LARGE = 10000;
    float pharef,ampref;
	double SLAMBDA = 1.65e-6;

    for(index=0; index<NB_files; index++)
    {
        printf("INDEX = %ld/%ld\n",index,NB_files);
        sprintf(fname,"%s%08ld.%09ld.pha.fits", in_prefix, index, (long) (1.0e12*SLAMBDA+0.5));
        load_fits(fname, "tmpwfp", 1);
        IDpha=image_ID("tmpwfp");
        sprintf(fname,"%s%08ld.%09ld.amp.fits",in_prefix, index, (long) (1.0e12*SLAMBDA+0.5));
        load_fits(fname, "tmpwfa", 1);
        IDamp=image_ID("tmpwfa");
        naxes[0] = data.image[IDpha].md[0].size[0];
        naxes[1] = data.image[IDpha].md[0].size[1];
        naxes[2] = data.image[IDpha].md[0].size[2];
        naxes_out[0] = data.image[IDpha].md[0].size[0]/2;
        naxes_out[1] = data.image[IDpha].md[0].size[1]/2;
        naxes_out[2] = data.image[IDpha].md[0].size[2];
        IDoutpha = create_3Dimage_ID("tmpwfop",naxes_out[0],naxes_out[1],naxes_out[2]);
        IDoutamp = create_3Dimage_ID("tmpwfoa",naxes_out[0],naxes_out[1],naxes_out[2]);

        ii=0;
        jj=0;
        kk=0;
        amp = 0.0;
        pha = 0.0;
        for(kk=0; kk<naxes[2]; kk++)
        {
            for(ii=0; ii<naxes[0]/2; ii++)
                for(jj=0; jj<naxes[1]/2; jj++)
                {
                    re=0.0;
                    im=0.0;
                    pharef = 0.0;
                    ampref = 0.0;
                    for(i=0; i<2; i++)
                        for(j=0; j<2; j++)
                        {
                            amp = data.image[IDamp].array.F[kk*naxes[0]*naxes[1]+(2*jj+j)*naxes[0]+2*ii+i];
                            pha = data.image[IDpha].array.F[kk*naxes[0]*naxes[1]+(2*jj+j)*naxes[0]+2*ii+i];
                            pharef += data.image[IDamp].array.F[kk*naxes[0]*naxes[1]+(2*jj+j)*naxes[0]+2*ii+i]*data.image[IDpha].array.F[kk*naxes[0]*naxes[1]+(2*jj+j)*naxes[0]+2*ii+i];
                            ampref += data.image[IDamp].array.F[kk*naxes[0]*naxes[1]+(2*jj+j)*naxes[0]+2*ii+i];
                            re += amp*cos(pha);
                            im += amp*sin(pha);
                        }
                    amp = sqrt(re*re+im*im);
                    pha = atan2(im,re);
                    pharef /= ampref;
                    P = 2.0*PI*( ((long) (0.5+1.0*LARGE+(pharef-pha)/2.0/PI)) - LARGE);
                    if(ampref<0.01)
                        P = 0.0;
                    data.image[IDoutpha].array.F[kk*naxes_out[0]*naxes_out[1]+jj*naxes_out[0]+ii] = pha+P;
                    data.image[IDoutamp].array.F[kk*naxes_out[0]*naxes_out[1]+jj*naxes_out[0]+ii] = amp/4.0;
                }
        }
        sprintf(fname,"%s%8ld.%09ld.pha.fits", out_prefix, index, (long) (1.0e12*SLAMBDA+0.1));
        replace_char(fname,' ','0');
        save_fl_fits("tmpwfop",fname);
        sprintf(fname,"%s%8ld.%09ld.amp.fits",out_prefix, index, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fname,' ','0');
        save_fl_fits("tmpwfoa",fname);

        delete_image_ID("tmpwfa");
        delete_image_ID("tmpwfp");
        delete_image_ID("tmpwfoa");
        delete_image_ID("tmpwfop");
    }

    return(0);
}



//
// analyze WF series: PSF FWHM and aperture photometry
//

int measure_wavefront_series(float factor)
{
    float FOCAL_SCALE;
    double tmp;
    long ID,IDpsf,IDamp,IDpha;
    long IDpupamp;
    long tspan;

    long ID_array1;
    float amp,pha;

    char fnameamp[200];
    char fnamepha[200];

    long naxes[3];
    long frame;
    long NBFRAMES;
    long ii,jj;
    int amplitude_on;

    double puprad = 0.035; // meter
    double pupradpix;
    double psfflux;
    double psfflux1; // within 1 arcsec radius
    double psfflux2; // within 2 arcsec radius
    double psfflux5; // within 5 arcsec radius
    double psfflux10; // within 10 arcsec radius
    double psfflux20; // within 20 arcsec radius

    double dx, dy, r;
    FILE *fpphot;

	double SLAMBDA = 1.65e-6;


    AtmosphericTurbulence_ReadConf();

    
    pupradpix = puprad/CONF_PUPIL_SCALE;
    printf("pupradpix = %f m\n",pupradpix);


    FOCAL_SCALE = CONF_LAMBDA/CONF_WFsize/CONF_PUPIL_SCALE/PI*180.0*3600.0; /* in arcsecond per pixel */
    printf("Scale is %f arcsecond per pixel (%ld pixels)\n", FOCAL_SCALE, CONF_WFsize);
    
    
    IDpupamp = image_ID("ST_pa");
    if (IDpupamp==-1)
        {
            printf("ERROR: pupil amplitude map not loaded");
            exit(0);
        }
    naxes[0]=data.image[ID].md[0].size[0];
    naxes[1]=data.image[ID].md[0].size[1];





    NBFRAMES = (long) (CONF_TIME_SPAN/CONF_WFTIME_STEP);
    naxes[2]=NBFRAMES;

    ID_array1 = create_2DCimage_ID("array1", naxes[0], naxes[1]);

    IDpsf = create_2Dimage_ID("PSF", naxes[0], naxes[1]);

    fpphot = fopen("phot.txt","w");
    fclose(fpphot);

    for(tspan=0; tspan<CONF_NB_TSPAN; tspan++)
    {
        printf("%ld/%ld\n", tspan, CONF_NB_TSPAN);
        sprintf(fnamepha,"%s%8ld.%09ld.pha.fits", CONF_WF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fnamepha,' ','0');
        sprintf(fnameamp,"%s%8ld.%09ld.amp.fits", CONF_WF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fnameamp,' ','0');
        IDpha = load_fits(fnamepha, "wfpha", 1);
        if(amplitude_on==1)
            IDamp = load_fits(fnameamp, "wfamp", 1);

        for(frame=0; frame<NBFRAMES; frame++)
        {
            psfflux = 0.0;
            psfflux1 = 0.0;
            psfflux2 = 0.0;
            psfflux5 = 0.0;
            psfflux10 = 0.0;
            psfflux20 = 0.0;
            if(amplitude_on==1)
                for(ii=0; ii<naxes[0]; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                    {
                        amp = data.image[IDamp].array.F[frame*naxes[0]*naxes[1]+jj*naxes[0]+ii]*data.image[IDpupamp].array.F[jj*naxes[0]+ii];
                        pha = factor*data.image[IDpha].array.F[frame*naxes[0]*naxes[1]+jj*naxes[0]+ii];
                        data.image[ID_array1].array.CF[jj*naxes[0]+ii].re = amp*cos(pha);
                        data.image[ID_array1].array.CF[jj*naxes[0]+ii].im = amp*sin(pha);
                        psfflux += amp*amp;
                    }
            else
                for(ii=0; ii<naxes[0]; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                    {
                        amp = data.image[IDpupamp].array.F[jj*naxes[0]+ii];
                        pha = factor*data.image[IDpha].array.F[frame*naxes[0]*naxes[1]+jj*naxes[0]+ii];
                        data.image[ID_array1].array.CF[jj*naxes[0]+ii].re = amp*cos(pha);
                        data.image[ID_array1].array.CF[jj*naxes[0]+ii].im = amp*sin(pha);
                    }

            do2dfft("array1","im_c");
            permut("im_c");
            ID=image_ID("im_c");
            for(ii=0; ii<naxes[0]; ii++)
                for(jj=0; jj<naxes[1]; jj++)
                {
                    dx = 1.0*ii-naxes[0]/2;
                    dy = 1.0*jj-naxes[1]/2;
                    r = sqrt(dx*dx+dy*dy);
                    tmp = (data.image[ID].array.CF[jj*naxes[0]+ii].re*data.image[ID].array.CF[jj*naxes[0]+ii].re+data.image[ID].array.CF[jj*naxes[0]+ii].im*data.image[ID].array.CF[jj*naxes[0]+ii].im);
                    data.image[IDpsf].array.F[jj*naxes[0]+ii] += tmp;
                    if(r<1.0/FOCAL_SCALE)
                        psfflux1 += tmp;
                    if(r<2.0/FOCAL_SCALE)
                        psfflux2 += tmp;
                    if(r<5.0/FOCAL_SCALE)
                        psfflux5 += tmp;
                    if(r<10.0/FOCAL_SCALE)
                        psfflux10 += tmp;
                    if(r<20.0/FOCAL_SCALE)
                        psfflux20 += tmp;
                }
            delete_image_ID("im_c");
            printf("%.6f %.4f %.4f %.4f %.4f %.4f %.4f\n", (tspan*NBFRAMES+frame)*CONF_WFTIME_STEP, psfflux, psfflux1, psfflux2, psfflux5, psfflux10, psfflux20);
            fpphot = fopen("phot.txt","a");
            fprintf(fpphot,"%.6f %.4f %.4f %.4f %.4f %.4f %.4f\n", (tspan*NBFRAMES+frame)*CONF_WFTIME_STEP, psfflux, psfflux1, psfflux2, psfflux5, psfflux10, psfflux20);
            fclose(fpphot);
        }

        delete_image_ID("wfamp");
        delete_image_ID("wfpha");
    }
    delete_image_ID("array1");
    save_fl_fits("PSF","!PSF.fits");
    tmp = measure_FWHM("PSF",1.0*naxes[0]/2,1.0*naxes[1]/2,1.0,naxes[0]/2);
    printf("FWHM = %f arcseconds (%f pixels)\n",FOCAL_SCALE*tmp,tmp);

    return(0);
}




//
// make TT test sequence
// if ACCmode == 1, include accelerometer
//
// MODE = 0 : 30 sine waves
// MODE = 1 : repeating waveform (non-periodic) 
//
int AtmosphericTurbulence_mkTestTTseq(double dt, long NBpts, long NBblocks, double measnoise, int ACCmode, double ACCnoise, int MODE)
{ 
	long IDout, IDoutn;
	long block;
	
	double tsim = 0.0; // running time	
	double fv1;
	double fv2 = 0.0;
	char imname[200];
	char fname[200];

	char imnamen[200];
	char fnamen[200];

	double x, y;
	double vx, vy; // speed
	double xold, yold, vxold, vyold;
	double ax, ay; // acceleration
	double xn, yn;
	double axn, ayn; // acceleration
	
	double ameasnoise;
	
	long NBfrequ;
	double *farray_frequ;
	double *farray_amp;
	double *farray_PA;
	double *farray_pha;
	long frequ;
	
	long ii;
	
	FILE *fp;
	
	
	
	long NBpt_wf0 = 40;
	long i;
	long wfi0;
	long WFon0;
	double *xwaveform0;
	double *ywaveform0;

	long NBpt_wf1 = 100;
	long wfi1;
	long WFon1;
	double *xwaveform1;
	double *ywaveform1;

	double amp;
	
	
	
	// create waveform
	xwaveform0 = (double*) malloc(sizeof(double)*NBpt_wf0);
	ywaveform0 = (double*) malloc(sizeof(double)*NBpt_wf0);
	for(i=0;i<NBpt_wf0;i++)
	{
		x = 1.0*i/NBpt_wf0;
		xwaveform0[i] = 5.0*(1.0-x)*pow(sin(2.0*M_PI*x*2.0), 5.0);
		ywaveform0[i] = -5.0*4.0*exp(pow((-2.0*x),3.0)) * exp(-pow(2.0*(1.0-x), 20.0));
	}
	
	xwaveform1 = (double*) malloc(sizeof(double)*NBpt_wf1);
	ywaveform1 = (double*) malloc(sizeof(double)*NBpt_wf1);
	for(i=0;i<NBpt_wf1;i++)
	{
		x = 1.0*i/NBpt_wf1;
		amp = 10*sqrt(x)*exp(-x*x*8);
		xwaveform1[i] = amp*sin(2.0*M_PI*x*4.0);
		ywaveform1[i] = amp*cos(2.0*M_PI*x*4.0);
	}
	
	

	
	
	
	NBfrequ = 20;
	farray_frequ = (double*) malloc(sizeof(double)*NBfrequ);
	farray_amp = (double*) malloc(sizeof(double)*NBfrequ);
	farray_PA = (double*) malloc(sizeof(double)*NBfrequ);
	farray_pha = (double*) malloc(sizeof(double)*NBfrequ);
	
	farray_frequ[0] = 15.0;
	farray_amp[0] = 1.0;
	farray_PA[0] = 0.0;
	farray_pha[0] = 0.0;
	
	for(frequ=1; frequ<NBfrequ; frequ++)
		{
			farray_frequ[frequ] = 1.1*farray_frequ[frequ-1];
			farray_amp[frequ] = farray_amp[frequ-1];
			farray_PA[frequ] = 1.0*frequ;
			farray_pha[frequ] = farray_pha[frequ-1];
		}
	
	fp = fopen("testTTseq.txt", "w");

	WFon0 = 0;
	wfi0 = 0;
	fv1 = 0.0;
	WFon1 = 0;
	wfi1 = 0;
	
	
	
	for(block=0; block < NBblocks; block++)
		{
			sprintf(imname, "testTT%05ld", block);
			sprintf(fname, "!%s.fits", imname);
	
			sprintf(imnamen, "testTTn%05ld", block);
			sprintf(fnamen, "!%s.fits", imnamen);
			
			if(ACCmode==0)
				{
					IDout = create_3Dimage_ID(imname, 2, 2, NBpts);
					IDoutn = create_3Dimage_ID(imnamen, 2, 2, NBpts);
				}
			else
				{
					IDout = create_3Dimage_ID(imname, 4, 2, NBpts);
					IDoutn = create_3Dimage_ID(imnamen, 4, 2, NBpts);
				}
			
				
			xold = 0.0;
			yold = 0.0;
			vxold = 0.0;
			vyold = 0.0;
			
		//	printf("---------------------------fv1 = %f\n", fv1);
		//	printf(" LOOP START2   fv1 = %f\n", fv1);



			for(ii=0;ii<NBpts;ii++)
			{
			//	printf(" LOOP START %ld/%ld  fv1 = %f\n", ii, NBpts, fv1);
						
				x = 0.0;
				y = 0.0;
				ax = 0.0;
				ay = 0.0;
				
				if(MODE==0)
					for(frequ=0; frequ<NBfrequ; frequ++)
					{
						x += farray_amp[frequ] * sin(2.0*M_PI*(tsim*farray_frequ[frequ])+farray_pha[frequ]) * cos(farray_PA[frequ]);
						y += farray_amp[frequ] * sin(2.0*M_PI*(tsim*farray_frequ[frequ])+farray_pha[frequ]) * sin(farray_PA[frequ]);
						ax -= 0.0002*farray_amp[frequ] * cos(farray_PA[frequ]) * sin(2.0*M_PI*(tsim*farray_frequ[frequ])+farray_pha[frequ]) * farray_frequ[frequ] * farray_frequ[frequ];
						ay -= 0.0002*farray_amp[frequ] * sin(farray_PA[frequ]) * sin(2.0*M_PI*(tsim*farray_frequ[frequ])+farray_pha[frequ]) * farray_frequ[frequ] * farray_frequ[frequ];
					}
				
				if(MODE==1)
					{
						if(ii==1060)
							WFon0 = 1;
						
						if( (WFon0==0)&&((ii>1200)||(ii<900)) )
							{
								if(  fabs(sin(cos(1800.123456*fv1))) < 0.05  )
									WFon0 = 1;
								wfi0 = 0;
							}
						if(WFon0==1)
							{
								x += xwaveform0[wfi0];
								y += ywaveform0[wfi0];
								wfi0++;
							}
						if(wfi0==NBpt_wf0-1)
							{
								wfi0 = 0;
								WFon0 = 0;
							}
						
						
						
						if(ii==1110)
							WFon1 = 1;
						if( (WFon1==0) &&((ii>1200)||(ii<900)) )
							{
								if(  fabs(sin(cos(1700.123456*fv1))) < 0.05  )
									WFon1 = 1;
								wfi1 = 0;
							}
						if(WFon1==1)
							{
								x += xwaveform1[wfi1];
								y += ywaveform1[wfi1];
								wfi1++;
							}
						if(wfi1==NBpt_wf1-1)
							{
								wfi1 = 0;
								WFon1 = 0;
							}
																			
					}
				
				vx = x-xold;
				vy = y-yold;
				xold = x;
				yold = y;
				
				ax = -5.5*(vxold-vx);
				ay = -5.5*(vyold-vy);
					

				xn = x + measnoise*gauss();
				yn = y + measnoise*gauss();
				axn = ax + ACCnoise*gauss();
				ayn = ay + ACCnoise*gauss();
				fprintf(fp, "%16lf %16f %16f %16f %16f   %16f %16f %16f %16f\n", tsim, x, y, xn, yn, ax, ay, axn, ayn);

				vxold = vx;
				vyold = vy;


				if(ACCmode==0)
				{
				data.image[IDout].array.F[4*ii] = x;
				data.image[IDout].array.F[4*ii+1] = y;

				data.image[IDout].array.F[4*ii+2] = -x;
				data.image[IDout].array.F[4*ii+3] = -y;



				data.image[IDoutn].array.F[4*ii] = xn;
				data.image[IDoutn].array.F[4*ii+1] = yn;

				data.image[IDoutn].array.F[4*ii+2] = -xn;
				data.image[IDoutn].array.F[4*ii+3] = -yn;
				}
				else
				{
				data.image[IDout].array.F[8*ii] = x;
				data.image[IDout].array.F[8*ii+1] = y;
				data.image[IDout].array.F[8*ii+2] = ax;
				data.image[IDout].array.F[8*ii+3] = ay;

				data.image[IDout].array.F[8*ii+4] = -x;
				data.image[IDout].array.F[8*ii+5] = -y;
				data.image[IDout].array.F[8*ii+6] = -ax;
				data.image[IDout].array.F[8*ii+7] = -ay;



				data.image[IDoutn].array.F[8*ii] = xn;
				data.image[IDoutn].array.F[8*ii+1] = yn;
				data.image[IDoutn].array.F[8*ii+2] = axn;
				data.image[IDoutn].array.F[8*ii+3] = ayn;

				data.image[IDoutn].array.F[8*ii+4] = -xn;
				data.image[IDoutn].array.F[8*ii+5] = -yn;
				data.image[IDoutn].array.F[8*ii+6] = -axn;
				data.image[IDoutn].array.F[8*ii+7] = -ayn;
				}
			
				tsim += dt;
	//			printf("=== %f + %f -> ", fval1, t);

			//	printf("   %f ===\n", fval1);
				fv1 = fv1 + tsim;
				//printf(" LOOP END   %f  fv1 = %f\n", tsim, fv1);
			}
			
			
			save_fits(imname, fname);
			printf("%s -> %s\n", imnamen, fnamen);
			save_fits(imnamen, fnamen);
		}
	fclose(fp);
	
	free(farray_frequ);
	free(farray_amp);
	free(farray_pha);
	
	free(xwaveform0);
	free(ywaveform0);
	free(xwaveform1);
	free(ywaveform1);
	
	return(0);
}





//
// build full predictor (all pixels of WF)
//
int AtmosphericTurbulence_Build_LinPredictor_Full(const char *WFin_name, const char *WFmask_name, int PForder, float PFlag, double SVDeps, double RegLambda)
{
	long ID_WFin;
	long NBmvec; // number of entries in data matrix
	long ID_WFmask;
	long xsize, ysize, zsize;
	long xysize;
	
	long IDmatA; // data matrix
	long NBpix;
	long pix;
	long PFpix;
	
	long mvecsize;
	long *pixarray_x;
	long *pixarray_y;
	long *pixarray_xy;
	
	long IDmatC;
	
	int Save = 1;
	char filtname[200];
	char filtfname[200];
	double val, valf;
	long k0;
	long ID_Pfilt;
	float alpha;
	long ii, jj, kk, dt, m;
	int ret;
	long IDfiltC;
	double tot, totm;
	
	int REG = 0;  // 1 if regularization
	long m1, NBmvec1;
	int use_magma = 0;
	
	if(RegLambda>1e-20)
		REG = 1;
	
	float *valfarray;
	long ind1;
	
	
	
	ID_WFin = image_ID(WFin_name);
	xsize = data.image[ID_WFin].md[0].size[0];
	ysize = data.image[ID_WFin].md[0].size[1];
	zsize = data.image[ID_WFin].md[0].size[2];
	xysize = xsize*ysize;
	
	ID_WFmask = image_ID(WFmask_name);
	NBpix = 0;
	for(ii=0;ii<xsize*ysize;ii++)
		if(data.image[ID_WFmask].array.F[ii] > 0.5)
			NBpix++;
	pixarray_x = (long*) malloc(sizeof(long)*NBpix);
	pixarray_y = (long*) malloc(sizeof(long)*NBpix);
	pixarray_xy = (long*) malloc(sizeof(long)*NBpix);
	



	// PRE_PROCESS WAVEFRONTS : REMOVE PISTON TERM

	totm = 0.0;
	for(ii=0;ii<xsize;ii++)
		for(jj=0;jj<ysize;jj++)
			if(data.image[ID_WFmask].array.F[jj*xsize+ii] > 0.5)
				totm += 1.0;

	for(kk=0;kk<zsize;kk++)
	{
		tot = 0.0;
		for(ii=0;ii<xsize;ii++)
			for(jj=0;jj<ysize;jj++)
			{
				data.image[ID_WFin].array.F[kk*xysize+jj*xsize+ii] *= data.image[ID_WFmask].array.F[jj*xsize+ii];
				tot += data.image[ID_WFin].array.F[kk*xysize+jj*xsize+ii];
			}
			for(ii=0;ii<xsize;ii++)
				for(jj=0;jj<ysize;jj++)
					if(data.image[ID_WFmask].array.F[jj*xsize+ii] > 0.5)
						data.image[ID_WFin].array.F[kk*xysize+jj*xsize+ii] -= tot/totm;
		}
	if(Save==1)
		save_fits(WFin_name, "!wfinm.fits");
	
	
	
	// LOAD PIXELS COORDINATES INTO ARRAYS
	
	NBpix = 0;
	for(ii=0;ii<xsize;ii++)
		for(jj=0;jj<ysize;jj++)
			if(data.image[ID_WFmask].array.F[jj*xsize+ii] > 0.5)
				{
					pixarray_x[NBpix] = ii;
					pixarray_y[NBpix] = jj;
					pixarray_xy[NBpix] = jj*xsize+ii;
					NBpix++;
				}
	printf("NBpix = %ld\n", NBpix);
	
	
	
	
	ret = system("mkdir -p pixfilters");
	
	// build data matrix
	NBmvec = zsize - PForder - (int) (PFlag) - 1;
	mvecsize = NBpix * PForder;
	
	if(REG==0)
		IDmatA = create_2Dimage_ID("PFmatD", NBmvec, mvecsize);
    else
		IDmatA = create_2Dimage_ID("PFmatD", NBmvec + mvecsize, mvecsize);
		
    
    
    // each column is a measurement
    // m index is measurement
    // dt*NBpix+pix index is pixel
    
    
    // CREATE DATA MATRIX
    
	if(REG==0)
		{
			printf("NBmvec   = %ld  -> %ld \n", NBmvec, NBmvec);
			NBmvec1 = NBmvec;
		}
	else
		{
			printf("NBmvec   = %ld  -> %ld \n", NBmvec, NBmvec + mvecsize);
			NBmvec1 = NBmvec + mvecsize;
		}
    
    printf("mvecsize = %ld  (%d x %ld)\n", mvecsize, PForder, NBpix);


	for(m=0; m<NBmvec; m++)
	{
		k0 = m + PForder-1; // dt=0 index
		for(pix=0; pix<NBpix; pix++)
			for(dt=0; dt<PForder; dt++)		
				data.image[IDmatA].array.F[(NBpix*dt+pix)*NBmvec1+m] = data.image[ID_WFin].array.F[(k0-dt)*xysize + pixarray_xy[pix]];
	}
	if(REG==1)
		{
			for(m=0; m<mvecsize; m++)
				{
					m1 = NBmvec + m;
					data.image[IDmatA].array.F[(m)*NBmvec1+(NBmvec+m)] = RegLambda;
				}
		}

	
	if(Save == 1)
		save_fits("PFmatD", "!PFmatD.fits");
	list_image_ID();
	


	printf("Compute reconstruction matrix\n");
	fflush(stdout);

	
	#ifdef HAVE_MAGMA
		CUDACOMP_magma_compute_SVDpseudoInverse("PFmatD", "PFmatC", SVDeps, 100000, "PF_VTmat", 0);
	#else
		linopt_compute_SVDpseudoInverse("PFmatD", "PFmatC", SVDeps, 100000, "PF_VTmat");
	#endif
	
	if(0){
		save_fits("PFmatD", "!test_PFmatD.fits");
		save_fits("PFmatC", "!test_PFmatC.fits");
		save_fits("PF_VTmat", "!test_PF_VTmat.fits");
			#ifdef HAVE_MAGMA
		CUDACOMP_magma_compute_SVDpseudoInverse("PFmatD", "PFmatC_magma", SVDeps, 100000, "PF_VTmat_magma", 0);
		#else
		linopt_compute_SVDpseudoInverse("PFmatD", "PFmatC_magma", SVDeps, 100000, "PF_VTmat_magma");
		#endif
		
		list_image_ID();
		save_fits("PFmatC_magma", "!test_PFmatC_magma.fits");
		save_fits("PF_VTmat_magma", "!test_PF_VTmat_magma.fits");	
		exit(0);
	}

    if(Save==1)
        save_fits("PFmatC", "!PFmatC.fits");
    IDmatC = image_ID("PFmatC");

	printf("Compute filters\n");
	fflush(stdout);
	
	IDfiltC = create_3Dimage_ID("filtC", NBpix, NBpix, PForder);	
	
	
	valfarray = (float*) malloc(sizeof(float)*NBmvec);
	
	alpha = PFlag - ((long) PFlag);
	for(PFpix=0; PFpix<NBpix; PFpix++) // PFpix is the pixel for which the filter is created
	{
		sprintf(filtname, "PFfilt_%06ld_%03ld_%03ld", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);			
		sprintf(filtfname, "!./pixfilters/PFfilt_%06ld_%03ld_%03ld.fits", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);	
		ID_Pfilt = create_3Dimage_ID(filtname, xsize, ysize, PForder);
		
		
		// fill in valfarray
		
		for(m=0; m<NBmvec; m++)
			{
				k0 = m + PForder -1;
				k0 += (long) PFlag;
				
				valfarray[m] = (1.0-alpha)*data.image[ID_WFin].array.F[(k0)*xysize + pixarray_xy[PFpix]] + alpha*data.image[ID_WFin].array.F[(k0+1)*xysize + pixarray_xy[PFpix]];
			}
		
		
		for(pix=0; pix<NBpix; pix++)
			{
				for(dt=0; dt<PForder; dt++)		
					{
						val = 0.0;
						ind1 = (NBpix*dt+pix)*NBmvec1;
						for(m=0; m<NBmvec; m++)
							val += data.image[IDmatC].array.F[ind1+m] * valfarray[m];

						data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]] =  val;
						data.image[IDfiltC].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] = val;
					}
			}
		save_fits(filtname, filtfname);	
	}
	
	free(valfarray);
	free(pixarray_x);
	free(pixarray_y);
	
	return(0);
}




//
// extract translation-invariant kernel from predictive AR filter and expand into individual filters
//
long AtmosphericTurbulence_LinPredictor_filt_2DKernelExtract(const char *IDfilt_name, const char *IDmask_name, long krad, const char *IDkern_name)
{
	long IDkern;
	long IDfilt;
	long IDmask;
	long IDkern_cnt;
	char filtname[200];
	char filtfname[200];

	long PForder;
	long xksize, yksize;
	long PFpix, NBpix, pix;
	long iifilt, jjfilt;
	long NBpix1;
	long ii, jj;
	long xsize, ysize, xysize;
	
	long *pixarray_x;
	long *pixarray_y;
	long *pixarray_xy;
	
	long dt;
	long dii, djj;
	
	long IDfiltC1, IDfiltC1n, IDfiltC2, IDfiltC2n;
	long ID_Pfilt, IDfiltC1cnt;
	long double tmp1, tmp2;
	double gain;
	
	
	IDfilt = image_ID(IDfilt_name);
	
	IDmask = image_ID(IDmask_name);
	xsize = data.image[IDmask].md[0].size[0];
	ysize = data.image[IDmask].md[0].size[1];
	xysize = xsize*ysize;
	NBpix = data.image[IDfilt].md[0].size[0];
	PForder = data.image[IDfilt].md[0].size[2];
	
	
	pixarray_x = (long*) malloc(sizeof(long)*NBpix);
	pixarray_y = (long*) malloc(sizeof(long)*NBpix);
	pixarray_xy = (long*) malloc(sizeof(long)*NBpix);

	NBpix1 = 0;
	for(ii=0;ii<xsize;ii++)
		for(jj=0;jj<ysize;jj++)
			if(data.image[IDmask].array.F[jj*xsize+ii] > 0.5)
				{
					pixarray_x[NBpix1] = ii;
					pixarray_y[NBpix1] = jj;
					pixarray_xy[NBpix1] = jj*xsize+ii;
					NBpix1++;
				}
	printf("NBpix1 = %ld / %ld\n", NBpix1, NBpix);
	
	
	
	xksize = 2*krad+1;
	yksize = 2*krad+1;
	IDkern = create_3Dimage_ID(IDkern_name, xksize, yksize, PForder);
	IDkern_cnt = create_3Dimage_ID("kerncnt", xksize, yksize, PForder);
	
	// extract 2D kernel
	for(PFpix=0; PFpix<NBpix; PFpix++) // PFpix is the pixel for which the filter is created
	{
		iifilt = pixarray_x[PFpix];
		jjfilt = pixarray_y[PFpix];
		
		for(dt=0; dt<PForder; dt++)	
		{
			for(pix=0; pix<NBpix; pix++)
				{
					ii = pixarray_x[pix];
					jj = pixarray_y[pix];
					
					dii = ii-iifilt;
					djj = jj-jjfilt;
					
					if(dii*dii+djj*djj<krad*krad)
						{
							data.image[IDkern].array.F[dt*xksize*yksize + (djj+krad)*xksize + dii+krad] += data.image[IDfilt].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
							data.image[IDkern_cnt].array.F[dt*xksize*yksize + (djj+krad)*xksize + dii+krad] += 1.0;
						}
				}
		}
	}
	
	
	for(ii=0; ii<xksize; ii++)
		for(jj=0; jj<yksize; jj++)
			for(dt=0; dt<PForder; dt++)	
				data.image[IDkern].array.F[dt*xksize*yksize + jj*xksize + ii] /= (data.image[IDkern_cnt].array.F[dt*xksize*yksize + jj*xksize + ii] + 1.0e-8);
	
	
	
	// expand 2D kernel into new filter
	IDfiltC1 = create_3Dimage_ID("filtCk", NBpix, NBpix, PForder);
	IDfiltC1cnt = create_3Dimage_ID("filtCkcnt", NBpix, NBpix, PForder);
	
	IDfiltC1n = create_2Dimage_ID("filtCkn", xsize, ysize);
	for(PFpix=0; PFpix<NBpix; PFpix++)
		{		
			tmp1 = 0.0;
			iifilt = pixarray_x[PFpix];
			jjfilt = pixarray_y[PFpix];
			for(dt=0; dt<PForder; dt++)	
				{
					for(pix=0; pix<NBpix; pix++)
						{
							ii = pixarray_x[pix];
							jj = pixarray_y[pix];
					
							dii = ii-iifilt;
							djj = jj-jjfilt;
					
							if((dii*dii+djj*djj)<(krad*krad))
								{
									data.image[IDfiltC1].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] = data.image[IDkern].array.F[dt*xksize*yksize + (djj+krad)*xksize + dii+krad];
									data.image[IDfiltC1cnt].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] = 1.0;
								}
							
							
						}
				}			
		}
	
		
	// remove offset 
	IDfiltC2 = create_3Dimage_ID("filtCk2", NBpix, NBpix, PForder);
	IDfiltC2n = create_2Dimage_ID("filtC2n", xsize, ysize);
	for(dt=0; dt<PForder; dt++)	
	{
		for(PFpix=0; PFpix<NBpix; PFpix++)
			{
				tmp1 = 0.0;
				tmp2 = 0.0;
				
				for(pix=0; pix<NBpix; pix++)
					{
						tmp1 += data.image[IDfiltC1cnt].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
						tmp2 += data.image[IDfiltC1].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
					}
				tmp1 = 1.0*NBpix - tmp1;
			
				
				for(pix=0; pix<NBpix; pix++)
				{
					data.image[IDfiltC1].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] -= (1.0-data.image[IDfiltC1cnt].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix])*(tmp2/tmp1);

					// non piston-compensated 
					data.image[IDfiltC2].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] = data.image[IDfiltC1].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] + tmp2/tmp1;
				}
			}
	}			
	
	
	for(PFpix=0; PFpix<NBpix; PFpix++)
		{
			tmp1 = 0.0;	
			for(dt=0; dt<PForder; dt++)	
				for(pix=0; pix<NBpix; pix++)
					tmp1 += data.image[IDfiltC2].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
			data.image[IDfiltC2n].array.F[pixarray_xy[PFpix]] = tmp1;
		}
			
	
	// rescale edge gain effect
	for(dt=0; dt<PForder; dt++)	
	{
		for(PFpix=0; PFpix<NBpix; PFpix++)
			{
				if(data.image[IDfiltC2n].array.F[pixarray_xy[PFpix]]>0.01)
					gain = 1.0/data.image[IDfiltC2n].array.F[pixarray_xy[PFpix]];
				tmp1 = 0.0;
				tmp2 = 0.0;
				for(pix=0; pix<NBpix; pix++)
				{
					data.image[IDfiltC2].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] *= gain;
					tmp1 += 1.0;
					tmp2 += data.image[IDfiltC2].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
				}
				
			
				
				for(pix=0; pix<NBpix; pix++)
					data.image[IDfiltC2].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix] -= (tmp2/tmp1);
				
			}
	}			
	
	
	
	
	
	// expand into individual filters
	for(PFpix=0; PFpix<NBpix; PFpix++)
	{
		sprintf(filtname, "PFfilt_%06ld_%03ld_%03ld", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);	
		sprintf(filtfname, "!./pixfilters/PFfilt_%06ld_%03ld_%03ld.fits", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);	
		ID_Pfilt = create_3Dimage_ID(filtname, xsize, ysize, PForder);
		tmp1 = 0.0;
		for(dt=0; dt<PForder; dt++)	
			for(pix=0; pix<NBpix; pix++)
				{
					data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]] = data.image[IDfiltC2].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
					tmp1 += data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]];
				}
		save_fits(filtname, filtfname);
		data.image[IDfiltC1n].array.F[pixarray_xy[PFpix]] = tmp1;
	}
	
	
	free(pixarray_x);
	free(pixarray_y);
	free(pixarray_xy);
	
	return(IDkern);
}





//
// expand into individual filters
// also provides some statistical analysis on filter
//
long AtmosphericTurbulence_LinPredictor_filt_Expand(const char *IDfilt_name, const char *IDmask_name)
{
	long IDfilt;
	long IDmask;
	char filtname[200];
	char filtfname[200];
	long ID_Pfilt;
	long PForder;
	long xksize, yksize;
	long PFpix, NBpix, pix;
	long iifilt, jjfilt;
	long NBpix1;
	long ii, jj;
	long xsize, ysize, xysize;
	
	long *pixarray_x;
	long *pixarray_y;
	long *pixarray_xy;
	
	long dt;
	
	long IDtau;
	long IDnorm1, IDnorm2;
	double tau; // effective time averaging
	double norm1, norm2;
	
	IDfilt = image_ID(IDfilt_name);
	
	IDmask = image_ID(IDmask_name);
	xsize = data.image[IDmask].md[0].size[0];
	ysize = data.image[IDmask].md[0].size[1];
	xysize = xsize*ysize;
	NBpix = data.image[IDfilt].md[0].size[0];
	PForder = data.image[IDfilt].md[0].size[2];
	
	
	pixarray_x = (long*) malloc(sizeof(long)*NBpix);
	pixarray_y = (long*) malloc(sizeof(long)*NBpix);
	pixarray_xy = (long*) malloc(sizeof(long)*NBpix);

	NBpix1 = 0;
	for(ii=0;ii<xsize;ii++)
		for(jj=0;jj<ysize;jj++)
			if(data.image[IDmask].array.F[jj*xsize+ii] > 0.5)
				{
					pixarray_x[NBpix1] = ii;
					pixarray_y[NBpix1] = jj;
					pixarray_xy[NBpix1] = jj*xsize+ii;
					NBpix1++;
				}
	printf("NBpix1 = %ld / %ld\n", NBpix1, NBpix);
	
	
	IDnorm1 = create_2Dimage_ID("filtmap_norm1", xsize, ysize);
	IDnorm2 = create_2Dimage_ID("filtmap_norm2", xsize, ysize);
	IDtau = create_2Dimage_ID("filtmap_tau", xsize, ysize);
	
	
	// expand into individual filters
	for(PFpix=0; PFpix<NBpix; PFpix++)
	{
		sprintf(filtname, "PFfilt_%06ld_%03ld_%03ld", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);	
		sprintf(filtfname, "!./pixfilters/PFfilt_%06ld_%03ld_%03ld.fits", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);	
		ID_Pfilt = create_3Dimage_ID(filtname, xsize, ysize, PForder);

		tau = 0.0;
		norm1 = 0.0;
		norm2 = 0.0;

		for(dt=0; dt<PForder; dt++)	
			for(pix=0; pix<NBpix; pix++)
				{
					data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]] = data.image[IDfilt].array.F[dt*NBpix*NBpix  + PFpix*NBpix + pix];
					
					norm1 += fabs(data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]]);
					norm2 += data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]]*data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]];
					tau += data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]]*data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]]*dt;
				}
		tau /= norm2;
		norm2 = sqrt(norm2);
		
		data.image[IDnorm1].array.F[pixarray_xy[PFpix]] = norm1;
		data.image[IDnorm2].array.F[pixarray_xy[PFpix]] = norm2;
		data.image[IDtau].array.F[pixarray_xy[PFpix]] = tau;

		save_fits(filtname, filtfname);
	}
	
	
	
	
	free(pixarray_x);
	free(pixarray_y);
	free(pixarray_xy);
	
	return(0);
}











//
// Apply full predictor (all pixels of WF)
//
// MODE = 0: apply pixfilter filters
// MODE = 1: simple WF averaging (temporal only)
//
// outp : prediction
// outf : time-shifted measurement
// outft : actual future value
// reft : true wavefront (no WFS noise)
//
// -> outp_res  (prediction residual)
// -> outf_res  (future measurement residual)
// -> outl_res  (last measurement residual)
//
int AtmosphericTurbulence_Apply_LinPredictor_Full(int MODE, const char *WFin_name, const char *WFmask_name, int PForder, float PFlag, const char *WFoutp_name, const char *WFoutf_name)
{
	long ID_WFin;
	long NBmvec; // number of entries in data matrix
	long ID_WFmask;
	long xsize, ysize, zsize;
	long xysize;
	
	long NBpix;
	long pix;
	long PFpix;

	long *pixarray_x;
	long *pixarray_y;
	long *pixarray_xy;
	
	
	char filtname[200];
	char filtfname[200];
	double val, valf, valp;
	long ID_Pfilt;
	float alpha;
	long ii, jj, kk, dt, m;
	int ret;
	
	double tot, totm;
	long IDoutp, IDoutf;
	long step;
	long PFlagl;
	
	long IDreft; 
	long IDoutft; // truth (if exists)
	double valft;
	
	long IDoutp_res;
	long IDoutf_res;
	long IDoutl_res;
	
	
	
	
	ID_WFin = image_ID(WFin_name);
	xsize = data.image[ID_WFin].md[0].size[0];
	ysize = data.image[ID_WFin].md[0].size[1];
	zsize = data.image[ID_WFin].md[0].size[2];
	xysize = xsize*ysize;
	
	IDoutp = create_3Dimage_ID(WFoutp_name, xsize, ysize, zsize);
	IDoutf = create_3Dimage_ID(WFoutf_name, xsize, ysize, zsize);
	IDreft = image_ID("reft");
	if(IDreft!=-1)
		{
			IDoutft = create_3Dimage_ID("outft", xsize, ysize, zsize);
			IDoutp_res = create_3Dimage_ID("outp_res", xsize, ysize, zsize);
			IDoutf_res = create_3Dimage_ID("outf_res", xsize, ysize, zsize);
			IDoutl_res = create_3Dimage_ID("outl_res", xsize, ysize, zsize);
		}
	
	ID_WFmask = image_ID(WFmask_name);
	NBpix = 0;
	for(ii=0;ii<xsize*ysize;ii++)
		if(data.image[ID_WFmask].array.F[ii] > 0.5)
			NBpix++;
	pixarray_x = (long*) malloc(sizeof(long)*NBpix);
	pixarray_y = (long*) malloc(sizeof(long)*NBpix);
	pixarray_xy = (long*) malloc(sizeof(long)*NBpix);
	
	
	totm = 0.0;
	for(ii=0;ii<xsize;ii++)
		for(jj=0;jj<ysize;jj++)
			if(data.image[ID_WFmask].array.F[jj*xsize+ii] > 0.5)
				totm += 1.0;

	NBpix = 0;
	for(ii=0;ii<xsize;ii++)
		for(jj=0;jj<ysize;jj++)
			if(data.image[ID_WFmask].array.F[jj*xsize+ii] > 0.5)
				{
					pixarray_x[NBpix] = ii;
					pixarray_y[NBpix] = jj;
					pixarray_xy[NBpix] = jj*xsize+ii;
					NBpix++;
				}
	printf("NBpix = %ld\n", NBpix);
	
	
	
	alpha = PFlag - ((long) PFlag);
	PFlagl = (long) PFlag;
	
	printf("Read and Apply filters\n");
	fflush(stdout);
	for(PFpix=0; PFpix<NBpix; PFpix++) 
	{
		sprintf(filtname, "PFfilt_%06ld_%03ld_%03ld", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);			
		if(MODE==0)
		{		
			sprintf(filtfname, "./pixfilters/PFfilt_%06ld_%03ld_%03ld.fits", pixarray_xy[PFpix], pixarray_x[PFpix], pixarray_y[PFpix]);
			ID_Pfilt = load_fits(filtfname, filtname, 1);
		}
		else
		{
			ID_Pfilt = create_3Dimage_ID(filtname, xsize, ysize, PForder);

			for(step=0;step<PForder;step++)
				data.image[ID_Pfilt].array.F[xysize*step + pixarray_y[PFpix]*xsize+pixarray_x[PFpix]] = 1.0/PForder;
		}
		
		for(kk=PForder;kk<zsize;kk++)
			{
				valp = 0.0;
				for(step=0;step<PForder;step++)
					{
						for(ii=0;ii<xsize*ysize;ii++)
							valp += data.image[ID_Pfilt].array.F[xysize*step+ii]*data.image[ID_WFin].array.F[(kk-step)*xysize + ii];
					}
					
				valf = 0.0;
				if(kk+PFlag+1<zsize)
					valf = (1.0-alpha) * data.image[ID_WFin].array.F[(kk+PFlagl)*xysize+pixarray_xy[PFpix]] + alpha * data.image[ID_WFin].array.F[(kk+PFlagl+1)*xysize+pixarray_xy[PFpix]];
			
				valft = 0.0;
				if(kk+PFlag+1<zsize)
					valft = (1.0-alpha) * data.image[IDreft].array.F[(kk+PFlagl)*xysize+pixarray_xy[PFpix]] + alpha * data.image[IDreft].array.F[(kk+PFlagl+1)*xysize+pixarray_xy[PFpix]];
		
		
				data.image[IDoutp].array.F[kk*xysize+pixarray_xy[PFpix]] = valp;
				data.image[IDoutf].array.F[kk*xysize+pixarray_xy[PFpix]] = valf;
				
				if(IDreft!=-1)
				{					
					valft = 0.0;
					if(kk+PFlag+1<zsize)
						valft = (1.0-alpha) * data.image[IDreft].array.F[(kk+PFlagl)*xysize+pixarray_xy[PFpix]] + alpha * data.image[IDreft].array.F[(kk+PFlagl+1)*xysize+pixarray_xy[PFpix]];
					data.image[IDoutft].array.F[kk*xysize+pixarray_xy[PFpix]] = valft;

					data.image[IDoutp_res].array.F[kk*xysize+pixarray_xy[PFpix]] = valp-valft;					
					data.image[IDoutf_res].array.F[kk*xysize+pixarray_xy[PFpix]] = valf-valft;		
					data.image[IDoutl_res].array.F[kk*xysize+pixarray_xy[PFpix]] = data.image[ID_WFin].array.F[kk*xysize + pixarray_xy[PFpix]]-valft;					
				}
			}
		delete_image_ID(filtname);
	
	}	
	
	


	free(pixarray_x);
	free(pixarray_y);
	free(pixarray_xy);

	return(0);
}










// use past and near pixels to predict current pixel value ( single pixel )

int AtmosphericTurbulence_Build_LinPredictor(long NB_WFstep, double WFphaNoise, long WFPlag, long WFP_NBstep, long WFP_xyrad, long WFPiipix, long WFPjjpix, float slambdaum)
{
    int GHA = 0;

    long ii0, jj0;
    long IDpha_measured;
    long IDpupamp;
    long frame;
    long NBFRAMES;
    long WFPxsize, WFPysize;
    double pha;
    long ii1, jj1, ii, jj;
    long tspan;
    long IDpha;
    long k;
    long naxes[2];
    char fname[200];
    char fnameamp[200];
    char fnamepha[200];


    long mvecsize; // measurement vector size
    long *mvecdx;
    long *mvecdy;
    long *mvecdz;
    long NBmvec; // number of measurement vectors

    long IDmatA;
    long l, m;
    long k0;
    long IDmatC;
    double val;
    long ID_WFPfilt;

    int Save = 1;

    // General Hessian Algorithm
    double GHA_eta = 1e-7; // learning rate
    long GHA_NBmodes = 10;
    long ID_GHA_x; // input vector [ n ]
    long ID_GHA_y; // output vector [ m ]
    long ID_GHA_z;
    long ID_GHA_UT; // V [n x n]
    long ID_GHA_NT; // V S+ [m x n]
    long ID_GHA_zzT; // [m x m]
    long ID_GHA_V; // [n x n]
    long ID_GHA_sval; // [m]
    long ID_GHA_Mest; // [m x n]
    long ID_GHA_WFPfilt;
    long GHA_n; // size of input vector = n
    long GHA_m; // size of output vector = m
    double dval, dval0;
    long ll;
    double sval;
    long offset;
    long GHAiter;
    long GHA_NBiter = 50;
    long double errval = 0.0;
    long ID_GHA_matA;
    long double cntval;
    long long cnt;
    long k1;


    long IDpupmask;
    double maxPixSpeed = 0.3; // max wind speed in pix / frame

    double SVDeps = 1e-8;
    long vID;
    
    double SLAMBDA;
    
    
    
    SLAMBDA = 1.0e-6*slambdaum;
    
    
    if((vID=variable_ID("SVDeps"))!=-1)
    {
        SVDeps = data.variable[vID].value.f;
        printf("SVDeps = %f\n", SVDeps);
    }


    printf("WFP lag    = %ld\n", WFPlag);
    printf("WFP rad    = %ld\n", WFP_xyrad);
    printf("WFP NBstep = %ld\n", WFP_NBstep);

    printf("NOISE      = %f\n", WFphaNoise);
    fflush(stdout);

    AtmosphericTurbulence_ReadConf();

    // select center pixel
    ii0 = WFPiipix;
    jj0 = WFPjjpix;

    WFPxsize = 1+2*WFP_xyrad;
    WFPysize = 1+2*WFP_xyrad;
    printf("WFP_xyrad = %ld\n", WFP_xyrad);
    IDpha_measured = create_3Dimage_ID("WFP_pham", WFPxsize, WFPysize, NB_WFstep);

    IDpupamp = image_ID("ST_pa");
    if (IDpupamp==-1)
    {
        printf("ERROR: pupil amplitude map not loaded");
        exit(0);
    }

    naxes[0] = data.image[IDpupamp].md[0].size[0];
    naxes[1] = data.image[IDpupamp].md[0].size[1];
    NBFRAMES = (long) (CONF_TIME_SPAN/CONF_WFTIME_STEP);

    printf("NBFRAMES = %ld\n", NBFRAMES);


    IDpupmask = image_ID("pupmask");
    if (IDpupmask==-1)
    {
        printf("ERROR: pupil mask not loaded");
        exit(0);
    }



    if(GHA==1)
    {
        // prepare GHA
        // matrix convention
        // km x kn matrix
        // size[0] = km
        // size[1] = kn
        // m<n
        GHA_m = 1;
        GHA_n = WFPxsize*WFPysize*(WFP_NBstep-WFPlag);
        ID_GHA_x = create_2Dimage_ID("GHA_x", GHA_n, 1); // input vector
        ID_GHA_y = create_2Dimage_ID("GHA_y", GHA_m, 1); // output vector
        ID_GHA_z = create_2Dimage_ID("GHA_z", GHA_m, 1); // z = UT y
        ID_GHA_UT = create_2Dimage_ID("GHA_UT", GHA_m, GHA_m);
        ID_GHA_NT = create_2Dimage_ID("GHA_NT", GHA_m, GHA_n);  // m x n matrix
        ID_GHA_zzT = create_2Dimage_ID("GHA_zzT", GHA_m, GHA_m);  // m x m matrix


        // initialization: set GHA_UT to Identity square matrix
        for(ii=0; ii<GHA_m; ii++)
            for(jj=0; jj<GHA_m; jj++)
                data.image[ID_GHA_UT].array.F[jj*GHA_m+ii] = 0.0;
        for(ii=0; ii<GHA_m; ii++)
            data.image[ID_GHA_UT].array.F[ii*GHA_m+ii] = 1.0;

        // set NT elements
        for(ii=0; ii<GHA_m; ii++)
            for(jj=0; jj<GHA_n; jj++)
                data.image[ID_GHA_NT].array.F[jj*GHA_m+ii] = 0.0;
        for(ii=0; ii<GHA_m; ii++)
            data.image[ID_GHA_NT].array.F[ii*GHA_m+ii] = 1.0;
        //data.image[ID_GHA_NT].array.F[10] = 1.0;
    }


    tspan = 0;
    k = 0;
    printf("\n\n");
    cnt = 0;
    cntval = 0.0;
    while(k<NB_WFstep)
    {
        printf("\r %4ld/%4ld   tspan = %4ld   ", k, NB_WFstep, tspan);
        fflush(stdout);
        //        printf("%ld/%ld\n", tspan, CONF_NB_TSPAN);
        sprintf(fnamepha,"%s%8ld.%09ld.pha.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fnamepha,' ','0');
        sprintf(fnameamp,"%s%8ld.%09ld.amp.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fnameamp,' ','0');
        IDpha = load_fits(fnamepha, "wfpha", 1);

        for(frame=0; frame<NBFRAMES; frame++)
        {
            printf("\r      %4ld/%4ld   tspan = %4ld   ", k, NB_WFstep, tspan);
            fflush(stdout);
            cnt = 0;
            cntval = 0.0;
            if(k<NB_WFstep)
            {
                for(ii=0; ii<WFPxsize; ii++)
                    for(jj=0; jj<WFPysize; jj++)
                    {
                        ii1 = ii0 + (ii-WFP_xyrad);
                        jj1 = jj0 + (jj-WFP_xyrad);
                        if((ii1>0)&&(ii1<CONF_WFsize)&&(jj1>0)&&(jj1<CONF_WFsize))
                            pha = data.image[IDpha].array.F[frame*naxes[0]*naxes[1]+jj1*naxes[0]+ii1];
                        else
                            pha = 0;
                        data.image[IDpha_measured].array.F[k*WFPxsize*WFPysize+jj*WFPxsize+ii] = gauss()*WFphaNoise + pha;
                        cnt++;
                        cntval += data.image[IDpha_measured].array.F[k*WFPxsize*WFPysize+jj*WFPxsize+ii];
                    }
                //            for(ii=0; ii<WFPxsize*WFPysize; ii++)
                //                  data.image[IDpha_measured].array.F[k*WFPxsize*WFPysize+ii] -= cntval/cnt;
            }
            k++;
        }
        delete_image_ID("wfpha");
        tspan++;
    }




    //    for(ii=0; ii<WFPxsize*WFPysize*NB_WFstep; ii++)
    //      data.image[IDpha_measured].array.F[ii] -= cntval/cnt;

    if(Save==1)
        save_fits("WFP_pham", "!WFP_pham.fits");





    mvecsize = WFPxsize*WFPysize*(WFP_NBstep-WFPlag);
    mvecdx = (long*) malloc(sizeof(long)*mvecsize);
    mvecdy = (long*) malloc(sizeof(long)*mvecsize);
    mvecdz = (long*) malloc(sizeof(long)*mvecsize);
    NBmvec = NB_WFstep-WFP_NBstep;

    // indexes
    // m = measurement number
    // l = pixel index

    l = 0;
    for(k=WFPlag; k<WFP_NBstep; k++)
    {
            
        for(ii=0; ii<WFPxsize; ii++)
            for(jj=0; jj<WFPysize; jj++)
            {
                mvecdx[l] = ii-WFP_xyrad;
                mvecdy[l] = jj-WFP_xyrad;
                mvecdz[l] = k;
                
                if((data.image[IDpupmask].array.F[(jj0+mvecdy[l])*naxes[0] + (ii0+mvecdx[l])]>0.1) && (sqrt(mvecdx[l]*mvecdx[l]+mvecdy[l]*mvecdy[l])<2.0+maxPixSpeed*(k+1)) )
                    l++;
            }
    }

    printf("lmax = %ld / %ld\n", l, mvecsize);
    mvecsize = l;
    

    IDmatA = create_2Dimage_ID("WFPmatA", NBmvec, mvecsize);
    // each column is a measurement
    // m index is measurement
    // l index is pixel
    for(m=0; m<NBmvec; m++)
    {
        k0 = m+WFP_NBstep;
        for(l=0; l<mvecsize; l++)
            data.image[IDmatA].array.F[l*NBmvec+m] = data.image[IDpha_measured].array.F[(k0-mvecdz[l])*WFPxsize*WFPysize+(mvecdy[l]+WFP_xyrad)*WFPxsize+(mvecdx[l]+WFP_xyrad)];
    }




    if(GHA==1)
    {
        // for GHA: compute differences
        ID_GHA_matA = create_2Dimage_ID("GHAmatA", NBmvec, mvecsize);
        for(k=0; k<NB_WFstep; k++)
        {
            k1 = k + 50; // 50 frames offset
            if(k1>NB_WFstep-1)
                k1 -= NB_WFstep;
            for(l=0; l<mvecsize; l++)
                data.image[ID_GHA_matA].array.F[l*NBmvec+k] = data.image[IDmatA].array.F[l*NBmvec+k] - data.image[IDmatA].array.F[l*NBmvec+k1];
        }
    }


    if(Save==1)
    {
        save_fits("WFPmatA", "!WFPmatA.fits");
        if(GHA==1)
            save_fits("GHAmatA", "!GHAmatA.fits");
    }

    if(GHA==1)
    {
        // run GHA
        printf("\n");
        printf("RUNNING GHA  %ld x %ld  [%ld]... \n", GHA_m, GHA_n, NBmvec);
        fflush(stdout);
        for(GHAiter=0; GHAiter<GHA_NBiter; GHAiter++)
        {
            errval = 0.0;
            for(k=0; k<NBmvec; k++)
            {
                //printf("\n %ld\n", k);
                //fflush(stdout);

                // initialize input vector x
                for(ii=0; ii<GHA_n; ii++)
                    data.image[ID_GHA_x].array.F[ii] = data.image[ID_GHA_matA].array.F[ii*NBmvec+k];

                // initialize output vector y
                for(ii=0; ii<GHA_m; ii++)
                    data.image[ID_GHA_y].array.F[ii] = data.image[ID_GHA_x].array.F[60];
                //data.image[IDpha_measured].array.F[(k+WFP_NBstep)*WFPxsize*WFPysize+WFP_xyrad*WFPxsize+WFP_xyrad];
                //data.image[ID_GHA_x].array.F[24];

                // Compute vector z = UT y
                for(ii=0; ii<GHA_m; ii++)
                    data.image[ID_GHA_z].array.F[ii] = 0.0;

                for(ii=0; ii<GHA_m; ii++)
                    for(jj=0; jj<GHA_m; jj++)
                        data.image[ID_GHA_z].array.F[ii] += data.image[ID_GHA_UT].array.F[jj*GHA_m+ii] * data.image[ID_GHA_y].array.F[jj];

                // compute LT[zzT]
                for(ii=0; ii<GHA_m; ii++)
                    for(jj=0; jj<GHA_m; jj++)
                        if(jj<=ii)
                            data.image[ID_GHA_zzT].array.F[jj*GHA_m+ii] = data.image[ID_GHA_z].array.F[ii] * data.image[ID_GHA_z].array.F[jj];

                // update UT
                for(ii=0; ii<GHA_m; ii++)
                    for(jj=0; jj<GHA_m; jj++)
                    {
                        dval = 0.0;
                        dval = data.image[ID_GHA_z].array.F[ii]*data.image[ID_GHA_y].array.F[jj];  // z yT
                        dval0 = 0.0;
                        for(ll=0; ll<GHA_m; ll++)
                            dval0 += data.image[ID_GHA_zzT].array.F[ll*GHA_m+ii] * data.image[ID_GHA_UT].array.F[jj*GHA_m+ll];
                        dval -= dval0;

                        data.image[ID_GHA_UT].array.F[jj*GHA_m+ii] += GHA_eta * dval;
                    }

                // update NT
                errval = 0.0;
                for(ii=0; ii<GHA_m; ii++)
                    for(jj=0; jj<GHA_n; jj++)
                    {
                        dval = 0.0;
                        dval = data.image[ID_GHA_z].array.F[ii]*data.image[ID_GHA_x].array.F[jj];  // z xT
                        dval0 = 0.0;
                        for(ll=0; ll<GHA_m; ll++)
                            dval0 += data.image[ID_GHA_zzT].array.F[ll*GHA_m+ii] * data.image[ID_GHA_NT].array.F[jj*GHA_m+ll];
                        dval -= dval0;

                        errval += dval*dval;

                        data.image[ID_GHA_NT].array.F[jj*GHA_m+ii] += GHA_eta*dval;
                    }
                //  printf("%05ld   z = %g    U = %g     NT0 = %g\n", k, data.image[ID_GHA_z].array.F[0], data.image[ID_GHA_UT].array.F[0], data.image[ID_GHA_NT].array.F[0]);
            }
            printf("%3ld  errval = %.18lf    %.10f\n", GHAiter, (double) errval, data.image[ID_GHA_NT].array.F[0]);
            fflush(stdout);
        }
        printf("done\n");
        fflush(stdout);

        if(Save==1)
        {
            save_fits("GHA_UT", "!GHA_UT.fits");
            save_fits("GHA_NT", "!GHA_NT.fits");
        }

        printf("Computing Matrix M\n");
        fflush(stdout);

        // separate vectors V and singular values
        ID_GHA_V = create_2Dimage_ID("GHA_V", GHA_n, GHA_m);
        ID_GHA_sval = create_2Dimage_ID("GHA_sval", GHA_m, 1);
        for(jj=0; jj<GHA_m; jj++)
        {
            val = 0.0;
            for(ii=0; ii<GHA_n; ii++)
            {
                dval = data.image[ID_GHA_NT].array.F[ii*GHA_m+jj];
                val += dval*dval;
                data.image[ID_GHA_V].array.F[jj*GHA_n+ii] = dval;
            }
            val = sqrt(val);
            for(ii=0; ii<GHA_n; ii++)
                data.image[ID_GHA_V].array.F[jj*GHA_n+ii] /= val;
            printf("Singular value %3ld = %g\n", jj, 1.0/val);
            data.image[ID_GHA_sval].array.F[jj] = 1.0/val;
        }
        save_fits("GHA_V", "!GHA_V.fits");

        // compute Mest
        ID_GHA_Mest = create_2Dimage_ID("GHA_Mest", GHA_m, GHA_n);

        for(ll=0; ll<GHA_m; ll++) // singular value index
        {
            sval = data.image[ID_GHA_sval].array.F[ll];
            for(jj=0; jj<GHA_n; jj++)
                for(ii=0; ii<GHA_m; ii++)
                {
                    data.image[ID_GHA_Mest].array.F[jj*GHA_m+ii] += data.image[ID_GHA_UT].array.F[ii*GHA_m+ll]*sval*data.image[ID_GHA_V].array.F[ll*GHA_n+jj];
                }
        }


        ID_GHA_WFPfilt = create_3Dimage_ID("GHA_WFPfilt", WFPxsize, WFPysize, WFP_NBstep);
        offset = WFPxsize*WFPysize*WFPlag;
        for(k=0; k<WFPxsize*WFPysize*(WFP_NBstep-WFPlag); k++)
            data.image[ID_GHA_WFPfilt].array.F[offset+k] =  data.image[ID_GHA_Mest].array.F[k];
        save_fits("GHA_WFPfilt", "!GHA_WFPfilt.fits");

    }
    //exit(0);



    linopt_compute_SVDpseudoInverse("WFPmatA", "WFPmatC", SVDeps, 10000, "WFP_VTmat");
    if(Save==1)
        save_fits("WFPmatC", "!WFPmatC.fits");
    IDmatC = image_ID("WFPmatC");

    ID_WFPfilt = create_3Dimage_ID("WFPfilt", WFPxsize, WFPysize, WFP_NBstep);

    for(l=0; l<mvecsize; l++)
    {
        val = 0.0;
        for(m=0; m<NBmvec; m++)
            val += data.image[IDmatC].array.F[l*NBmvec+m] * data.image[IDpha_measured].array.F[(m+WFP_NBstep)*WFPxsize*WFPysize + WFP_xyrad*WFPxsize + WFP_xyrad];
        // printf("%5ld  ->  %5ld / %5ld     %5ld / %5ld    %5ld / %5ld\n", l, mvecdz[l], WFP_NBstep, mvecdy[l]+WFP_xyrad, WFPysize, mvecdx[l]+WFP_xyrad, WFPxsize);
        data.image[ID_WFPfilt].array.F[WFPxsize*WFPysize*mvecdz[l]+WFPxsize*(mvecdy[l]+WFP_xyrad)+(mvecdx[l]+WFP_xyrad)] =  val;
    }
    sprintf(fname, "!WFPfilt_lag%ld_rad%ld_%03ld_%03ld.fits", WFPlag, WFP_xyrad, WFPiipix, WFPjjpix);
    save_fits("WFPfilt", fname);
    save_fits("WFPfilt", "!WFPfilt.fits");

    for(k=WFPlag; k<WFP_NBstep; k++)
    {
        val = 0.0;
        for(ii=0; ii<WFPxsize*WFPysize; ii++)
            val += data.image[ID_WFPfilt].array.F[WFPxsize*WFPysize*k+ii]*data.image[ID_WFPfilt].array.F[WFPxsize*WFPysize*k+ii];
        printf("%5ld  %.10f\n", k, val);
    }
    list_image_ID();

    free(mvecdx);
    free(mvecdy);
    free(mvecdz);

    return 0;
}



long AtmosphericTurbulence_psfCubeContrast(const char *IDwfc_name, const char *IDmask_name, const char *IDpsfc_name)
{
	long IDwfc, IDmask, IDpsfc, IDa, IDtmp, IDm, IDpsfCc;
	long xsize, ysize, zsize, xysize, xsize1, ysize1, zsize1, xysize1;
	long IDpsf;
	long zfactor = 2;
	long ii, jj, ii1, jj1, kk;
	long IDretmp, IDimtmp;
	float peakC;
	
	long IDpsfc_ave, IDpsfCc_ave;
	
	long cnt0, cnt1;
	long IDpsfc_ave0, IDpsfCc_ave0;
	long IDpsfc_ave1, IDpsfCc_ave1;
	
	
	IDwfc = image_ID(IDwfc_name);
	xsize = data.image[IDwfc].md[0].size[0];
	ysize = data.image[IDwfc].md[0].size[1];
	zsize = data.image[IDwfc].md[0].size[2];
	xysize = xsize*ysize;
	
	IDmask = image_ID(IDmask_name);
	
	xsize1 = zfactor*xsize;
	ysize1 = zfactor*ysize;
	zsize1 = zsize;
	xysize1 = xsize1*ysize1;
	
	IDm = create_2Dimage_ID("wfmask1", xsize1, ysize1);
	for(ii=0;ii<xsize;ii++)
			for(jj=0;jj<ysize;jj++)
			{
				ii1 = ii + (xsize1-xsize)/2;
				jj1 = jj + (ysize1-ysize)/2;
				data.image[IDm].array.F[jj1*xsize1+ii1] = data.image[IDmask].array.F[jj*xsize+ii];
			}

	
	IDpsfc = create_3Dimage_ID(IDpsfc_name, xsize1, ysize1, zsize1);
	IDpsfCc = create_3Dimage_ID("psfCc", xsize1, ysize1, zsize1);
	
	IDtmp = create_2Dimage_ID("wftmp", xsize1, ysize1);
	IDretmp = create_2Dimage_ID("wfretmp", xsize1, ysize1);
	IDimtmp = create_2Dimage_ID("wfimtmp", xsize1, ysize1);
	
	IDpsfc_ave = create_2Dimage_ID("psfc_ave", xsize1, ysize1);
	IDpsfCc_ave = create_2Dimage_ID("psfCc_ave", xsize1, ysize1);
	IDpsfc_ave0 = create_2Dimage_ID("psfc_ave0", xsize1, ysize1);
	IDpsfCc_ave0 = create_2Dimage_ID("psfCc_ave0", xsize1, ysize1);
	IDpsfc_ave1 = create_2Dimage_ID("psfc_ave1", xsize1, ysize1);
	IDpsfCc_ave1 = create_2Dimage_ID("psfCc_ave1", xsize1, ysize1);

	cnt0 = 0;
	cnt1 = 0;
	list_image_ID();
	printf("\n");
	for(kk=0;kk<zsize;kk++)
	{
		printf("\r %5ld / %5ld  ",kk, zsize);
		fflush(stdout);
		for(ii=0;ii<xsize;ii++)
			for(jj=0;jj<ysize;jj++)
			{
				ii1 = ii + (xsize1-xsize)/2;
				jj1 = jj + (ysize1-ysize)/2;
				data.image[IDtmp].array.F[jj1*xsize1+ii1] = data.image[IDwfc].array.F[kk*xysize + jj*xsize + ii];
			}
		mk_complex_from_amph("wfmask1", "wftmp", "wfctmp", 0);
		permut("wfctmp");
		do2dfft("wfctmp","focc");
		delete_image_ID("wfctmp");
		permut("focc");
		mk_amph_from_complex("focc", "foca", "focp", 0);
		delete_image_ID("focc");
		delete_image_ID("focp");
		IDa = image_ID("foca");
		peakC = 0.0;
		for(ii=0;ii<xysize1;ii++)
			{
				data.image[IDpsfc].array.F[xysize1*kk+ii] = data.image[IDa].array.F[ii]*data.image[IDa].array.F[ii];
				if(data.image[IDpsfc].array.F[xysize1*kk+ii]>peakC)
					peakC = data.image[IDpsfc].array.F[xysize1*kk+ii];
			}
		for(ii=0;ii<xysize1;ii++)
			data.image[IDpsfc].array.F[xysize1*kk+ii] /= peakC;
		delete_image_ID("foca");
	
		for(ii=0;ii<xysize1;ii++)
			data.image[IDpsfc_ave].array.F[ii] += data.image[IDpsfc].array.F[xysize1*kk+ii]/zsize;
		
		for(ii=0;ii<xsize;ii++)
			for(jj=0;jj<ysize;jj++)
			{
				ii1 = ii + (xsize1-xsize)/2;
				jj1 = jj + (ysize1-ysize)/2;
				data.image[IDretmp].array.F[jj1*xsize1+ii1] = data.image[IDm].array.F[jj1*xsize1+ii1] * (cos(data.image[IDwfc].array.F[kk*xysize + jj*xsize + ii])-1.0);
				data.image[IDretmp].array.F[jj1*xsize1+ii1] = data.image[IDm].array.F[jj1*xsize1+ii1] * sin(data.image[IDwfc].array.F[kk*xysize + jj*xsize + ii]);
			}
		mk_complex_from_reim("wfretmp", "wfimtmp", "wfctmp", 0);
		permut("wfctmp");
		do2dfft("wfctmp","focc");
		delete_image_ID("wfctmp");
		permut("focc");
		mk_amph_from_complex("focc", "foca", "focp", 0);
		delete_image_ID("focc");
		delete_image_ID("focp");
		IDa = image_ID("foca");
		for(ii=0;ii<xysize1;ii++)
			data.image[IDpsfCc].array.F[xysize1*kk+ii] = (data.image[IDa].array.F[ii]*data.image[IDa].array.F[ii])/peakC;
		delete_image_ID("foca");
		
		for(ii=0;ii<xysize1;ii++)
			data.image[IDpsfCc_ave].array.F[ii] += data.image[IDpsfCc].array.F[xysize1*kk+ii]/zsize;

		if(kk<zsize/2)
		{
			cnt0++;
			for(ii=0;ii<xysize1;ii++)
			{
				data.image[IDpsfc_ave0].array.F[ii] += data.image[IDpsfc].array.F[xysize1*kk+ii];
				data.image[IDpsfCc_ave0].array.F[ii] += data.image[IDpsfCc].array.F[xysize1*kk+ii];
			}
		}
		else
		{
			cnt1++;
			for(ii=0;ii<xysize1;ii++)
			{
				data.image[IDpsfc_ave1].array.F[ii] += data.image[IDpsfc].array.F[xysize1*kk+ii];
				data.image[IDpsfCc_ave1].array.F[ii] += data.image[IDpsfCc].array.F[xysize1*kk+ii];
			}
		}
	}
	delete_image_ID("wfmask1");
	delete_image_ID("wftmp");

	for(ii=0;ii<xysize1;ii++)
	{
		data.image[IDpsfc_ave0].array.F[ii] /= cnt0;
		data.image[IDpsfc_ave1].array.F[ii] /= cnt1;
		data.image[IDpsfCc_ave0].array.F[ii] /= cnt0;
		data.image[IDpsfCc_ave1].array.F[ii] /= cnt1;		
	}
	
	printf("\n");
	
	return(IDpsfc);
}



int AtmosphericTurbulence_Test_LinPredictor(long NB_WFstep, double WFphaNoise, const char *IDWFPfilt_name, long WFPlag, long WFPiipix, long WFPjjpix, float slambdaum)
{
    long WFP_xyrad;
    long WFP_NBstep;
    long IDWFPfilt;
    long WFPxsize, WFPysize;
    long ii0, jj0;
    char fnamephaout[200];
    char fnamephaoutres[200];
    long tspan;
    long k, k1;
    char fnamepha[200];
    long IDpha;
    long frame;
    long ii, jj, ii1, jj1, ii2, jj2;
    double pha;
    long IDbuff, IDbuff1;
    long IDphaout, IDphaoutres;
    double val;
    long NBFRAMES;
    FILE *fp;
    double valgain100, valgain050, valgain025, valgain013, valgain006, valgain003, vallag;
    double rms100, rms050, rms025, rms013, rms006, rms003, rmslag;
    double err, rms;
    double rmst; // pure temporal error
    long rmscnt;
	double SLAMBDA;
	
	SLAMBDA = 1.0e-6*slambdaum;

    IDWFPfilt = image_ID(IDWFPfilt_name);
    if(IDWFPfilt==-1)
    {
        printf("ERROR: image \"%s\" does not exist\n", IDWFPfilt_name);
        exit(0);
    }

    WFPxsize = data.image[IDWFPfilt].md[0].size[0];
    WFPysize = data.image[IDWFPfilt].md[0].size[1];
    WFP_NBstep = data.image[IDWFPfilt].md[0].size[2];

    WFP_xyrad = (long) (0.5*WFPxsize);

    printf("WFP_xyrad = %ld\n", WFP_xyrad);

    AtmosphericTurbulence_ReadConf();
    NBFRAMES = (long) (CONF_TIME_SPAN/CONF_WFTIME_STEP);
    //    IDphaout = create_3Dimage_ID("wfphaout", CONF_WFsize, CONF_WFsize, NBFRAMES);
    //    IDphaoutres = create_3Dimage_ID("wfphaoutres", CONF_WFsize, CONF_WFsize, NBFRAMES);
    IDbuff = create_3Dimage_ID("wfpbuffer", CONF_WFsize, CONF_WFsize, WFP_NBstep);
    IDbuff1 = create_3Dimage_ID("wfpbuffer1", CONF_WFsize, CONF_WFsize, WFP_NBstep); // no noise

    fp = fopen("WFPtest.log", "w");

    valgain100 = 0.0;
    valgain050 = 0.0;
    valgain025 = 0.0;
    valgain013 = 0.0;
    valgain006 = 0.0;
    valgain003 = 0.0;
    vallag = 0.0;
    
    rms = 0.0;
    rms100 = 0.0;
    rms050 = 0.0;
    rms025 = 0.0;
    rms013 = 0.0;
    rms006 = 0.0;
    rms003 = 0.0;
    rmslag = 0.0;
    
    rmscnt = 0;
    
    tspan = 0;
    k = 0;
    printf("\n\n");
    while(k<NB_WFstep)
    {
        printf("\r %4ld/%4ld   tspan = %4ld   ", k, NB_WFstep, tspan);
        fflush(stdout);
        sprintf(fnamepha, "%s%08ld.%09ld.pha.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));

        IDpha = load_fits(fnamepha, "wfpha", 1);


        sprintf(fnamephaout, "!%s%08ld.out.pha.fits", CONF_SWF_FILE_PREFIX, tspan);
        sprintf(fnamephaoutres, "!%s%08ld.outres.pha.fits", CONF_SWF_FILE_PREFIX, tspan);


        for(frame=0; frame<NBFRAMES; frame++)
        {
            printf("\r      %4ld/%4ld   tspan = %4ld   ", k, NB_WFstep, tspan);
            fflush(stdout);
            if(k<NB_WFstep)
            {
                // write buffer slice 0
                for(ii=0; ii<CONF_WFsize*CONF_WFsize; ii++)
                    {
                        data.image[IDbuff].array.F[ii] = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+ii] + gauss()*WFphaNoise;
                        data.image[IDbuff1].array.F[ii] = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+ii];
                    }
                    
                // estimation
                //for(ii=0;ii<CONF_WFsize;ii++)
                //  for(jj=0;jj<CONF_WFsize;jj++)
                // {
                ii = WFPiipix;
                jj = WFPjjpix;
                val = 0.0;
                for(ii1=0; ii1<WFPxsize; ii1++)
                    for(jj1=0; jj1<WFPysize; jj1++)
                    {
                        ii2 = ii + ii1 - WFP_xyrad;
                        jj2 = jj + jj1 - WFP_xyrad;
                        if((ii2>-1)&&(ii2<CONF_WFsize)&&(jj2>-1)&&(jj2<CONF_WFsize))
                            for(k1=0; k1<WFP_NBstep; k1++)
                                val += data.image[IDbuff].array.F[k1*CONF_WFsize*CONF_WFsize+CONF_WFsize*jj2+ii2] * data.image[IDWFPfilt].array.F[k1*WFPxsize*WFPysize+jj1*WFPxsize+ii1];
                    }


                //data.image[IDphaout].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii] = val;
                //data.image[IDphaoutres].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii] = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii] - val;

                valgain100 = data.image[IDbuff].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];
                valgain050 = valgain050*(1.00-0.50) + 0.50*data.image[IDbuff].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];
                valgain025 = valgain025*(1.00-0.25) + 0.25*data.image[IDbuff].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];
                valgain013 = valgain013*(1.00-0.13) + 0.13*data.image[IDbuff].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];
                valgain006 = valgain006*(1.00-0.06) + 0.06*data.image[IDbuff].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];
                valgain003 = valgain003*(1.00-0.03) + 0.03*data.image[IDbuff].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];
                vallag = data.image[IDbuff1].array.F[WFPlag*CONF_WFsize*CONF_WFsize+CONF_WFsize*WFPjjpix+WFPiipix];

                fprintf(fp, "%5ld  %20f  %20f  %20f  %20f  %20f  %20f  %20f  %20f  %20f\n", k, data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii], val, data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-val, valgain100, valgain050, valgain025, valgain013, valgain006, valgain003);



                if(k>100)
                {
                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-valgain100;
                    rms100 += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-valgain050;
                    rms050 += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-valgain025;
                    rms025 += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-valgain013;
                    rms013 += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-valgain006;
                    rms006 += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-valgain003;
                    rms003 += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-val;
                    rms += err*err;

                    err = data.image[IDpha].array.F[frame*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii]-vallag;
                    rmslag += err*err;

                    rmscnt ++;
                }


                // move by 1 slice
                for(k1=WFP_NBstep-1; k1>0; k1--)
                    for(ii=0; ii<CONF_WFsize*CONF_WFsize; ii++)
                        {
                            data.image[IDbuff].array.F[k1*CONF_WFsize*CONF_WFsize+ii] = data.image[IDbuff].array.F[(k1-1)*CONF_WFsize*CONF_WFsize+ii];
                            data.image[IDbuff1].array.F[k1*CONF_WFsize*CONF_WFsize+ii] = data.image[IDbuff1].array.F[(k1-1)*CONF_WFsize*CONF_WFsize+ii];
                        }
            }
            k++;
        }
        //        save_fits("wfphaout", fnamephaout);
        //       save_fits("wfphaoutres", fnamephaoutres);
        tspan++;
    }

    fclose(fp);


    rms = sqrt(rms/rmscnt);
    rms100 = sqrt(rms100/rmscnt);
    rms050 = sqrt(rms050/rmscnt);
    rms025 = sqrt(rms025/rmscnt);
    rms013 = sqrt(rms013/rmscnt);
    rms006 = sqrt(rms006/rmscnt);
    rms003 = sqrt(rms003/rmscnt);
    rmslag = sqrt(rmslag/rmscnt);
    
    printf("\nRMS : %10f    [%8f]  %10f %10f %10f %10f %10f %10f\n", rms, rmslag, rms100, rms050, rms025, rms013, rms006, rms003);

    return(0);
}




int measure_wavefront_series_expoframes(float etime, const char *outfile)
{
    float FOCAL_SCALE;
    float tmp,tmp1;
    long ID,IDpsf,IDamp,IDpha;
    long tspan;
    FILE *fp;
    char command[200];
    float frac = 0.5;

    long ID_array1;
    float amp,pha;

    char fnameamp[200];
    char fnamepha[200];
    char fname[200];

    long naxes[3];
    long frame;
    long NBFRAMES;
    long ii,jj;
    float etime_left;
    long frame_number;
    double *xcenter;
    double *ycenter;
    long amplitude_on;

    long zoomfactor = 2;
    long naxesout[2];
    long xoffset, yoffset;

	double SLAMBDA = 1.65e-6;


    xcenter = (double*) malloc(sizeof(double));
    ycenter = (double*) malloc(sizeof(double));

    printf("Frame exposure time is %f s\n",etime);

    AtmosphericTurbulence_ReadConf();




    FOCAL_SCALE = CONF_LAMBDA/CONF_WFsize/CONF_PUPIL_SCALE/PI*180.0*3600.0*zoomfactor; /* in arcsecond per pixel */
    printf("Scale is %f arcsecond per pixel (%ld pixels)\n",FOCAL_SCALE, CONF_WFsize);

    ID = image_ID("ST_pa");
    if (ID==-1)
        {
            printf("ERROR: pupil amplitude map not loaded");
            exit(0);
        }
 
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];
    IDamp = ID;

    naxesout[0] = naxes[0]*zoomfactor;
    naxesout[1] = naxes[1]*zoomfactor;
    xoffset = (naxesout[0]-naxes[0])/2;
    yoffset = (naxesout[1]-naxes[1])/2;


    NBFRAMES = (long) (CONF_TIME_SPAN/CONF_WFTIME_STEP);
    naxes[2] = NBFRAMES;

    ID_array1 = create_2DCimage_ID("array1", naxesout[0], naxesout[1]);

    IDpsf = create_2Dimage_ID("PSF", naxesout[0], naxesout[1]);
    frame_number = 0;
    etime_left = etime;

    sprintf(command,"rm -rf %s",outfile);
    if(system(command)==-1)
    {
        printf("ERROR: system(\"%s\"), %s line %d\n",command,__FILE__,__LINE__);
        exit(0);
    }

 

    if((fp=fopen(outfile,"w"))==NULL)
    {
        printf("Cannot create file %s\n",outfile);
        exit(0);
    }
    fclose(fp);


    for(tspan=0; tspan<CONF_NB_TSPAN; tspan++)
    {
        printf("%ld/%ld\n", tspan, CONF_NB_TSPAN);
        sprintf(fnamepha,"%s%8ld.%09ld.pha.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fnamepha,' ','0');
        sprintf(fnameamp,"%s%8ld.%09ld.amp.fits", CONF_SWF_FILE_PREFIX, tspan, (long) (1.0e12*SLAMBDA+0.5));
        replace_char(fnameamp,' ','0');
        IDpha = load_fits(fnamepha, "wfpha", 1);
        if(amplitude_on==1)
        {
            printf("reading amp\n");
            fflush(stdout);
            IDamp = load_fits(fnameamp, "wfamp", 1);
        }

        for(frame=0; frame<NBFRAMES; frame++)
        {
            if(amplitude_on==1)
                for(ii=0; ii<naxes[0]; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                    {
                        amp = data.image[IDamp].array.F[frame*naxes[0]*naxes[1]+jj*naxes[0]+ii];
                        pha = data.image[IDpha].array.F[frame*naxes[0]*naxes[1]+jj*naxes[0]+ii];
                        data.image[ID_array1].array.CF[(jj+yoffset)*naxesout[0]+ii+xoffset].re = amp*cos(pha);
                        data.image[ID_array1].array.CF[(jj+yoffset)*naxesout[0]+ii+xoffset].im = amp*sin(pha);
                    }
            else
                for(ii=0; ii<naxes[0]; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                    {
                        amp = data.image[IDamp].array.F[jj*naxes[0]+ii];
                        pha = data.image[IDpha].array.F[frame*naxes[0]*naxes[1]+jj*naxes[0]+ii];
                        data.image[ID_array1].array.CF[(jj+yoffset)*naxesout[0]+ii+xoffset].re = amp*cos(pha);
                        data.image[ID_array1].array.CF[(jj+yoffset)*naxesout[0]+ii+xoffset].im = amp*sin(pha);
                    }

            do2dfft("array1","im_c");
            permut("im_c");
            ID=image_ID("im_c");
            for(ii=0; ii<naxesout[0]; ii++)
                for(jj=0; jj<naxesout[1]; jj++)
                {
                    data.image[IDpsf].array.F[jj*naxesout[0]+ii] += (data.image[ID].array.CF[jj*naxesout[0]+ii].re*data.image[ID].array.CF[jj*naxesout[0]+ii].re+data.image[ID].array.CF[jj*naxesout[0]+ii].im*data.image[ID].array.CF[jj*naxesout[0]+ii].im);
                }
            delete_image_ID("im_c");

            etime_left -= CONF_WFTIME_STEP;
            if(etime_left<0)
            {
                sprintf(fname,"!PSF%04ld.fits",frame_number);
                save_fl_fits("PSF",fname);
                gauss_filter("PSF","PSFg",3,10);
                center_PSF("PSFg", xcenter, ycenter, naxesout[0]/2);
                tmp = measure_FWHM("PSFg",xcenter[0],ycenter[0],1.0,naxesout[0]/2);
                printf("%ld FWHM %f arcseconds (%f pixels)\n",frame_number,FOCAL_SCALE*tmp,tmp);
                tmp1 = measure_enc_NRJ("PSF", xcenter[0], ycenter[0], frac);
                printf("Encircled energy (%f) is %f arcseconds\n", frac, tmp1*FOCAL_SCALE);
                delete_image_ID("PSFg");
                if((fp=fopen(outfile,"a"))==NULL)
                {
                    printf("Cannot open file %s\n",outfile);
                    exit(0);
                }
                fprintf(fp,"%ld %f %f %f %f\n",frame_number,FOCAL_SCALE*tmp,2.0*FOCAL_SCALE*tmp1,xcenter[0],ycenter[0]);
                fclose(fp);

                arith_image_zero("PSF");
                etime_left = etime;
                frame_number++;
                printf("Working on frame %ld\n",frame_number);
            }

        }
        delete_image_ID("wfamp");
        delete_image_ID("wfpha");
    }
    free(xcenter);
    free(ycenter);
    delete_image_ID("array1");
    save_fl_fits("PSF","!PSF");
    tmp = measure_FWHM("PSF", 1.0*naxesout[0]/2, 1.0*naxesout[1]/2,1.0, naxesout[0]/2);
    printf("FWHM = %f arcseconds (%f pixels)\n", FOCAL_SCALE*tmp, tmp);

    return(0);
}







int frame_select_PSF(const char *logfile, long NBfiles, float frac)
{
    /* logfile has the following format:
       <PSF file name> <FWHM> <Enc.ener.0.5> <centerx> <centery>
    */
    /* outputs the following files :
       PSFc:    coadded PSF (no centering/filtering)
       PSFcc:   coadded PSF with centering
       PSFccsf: coadded PSF with centering and selection on FWHM (frac= fraction of the frames kept)
       PSFccse: coadded PSF with centering and selection on Enc.ener.(frac= fraction of the frames kept)
    */
    FILE *fp;
    int OK;
    long i;
    float *FWHM;
    float *ENCE;
    float *xcen;
    float *ycen;
    char fname[200];
    float Xcenter,Ycenter;
    float Xcenter_c,Ycenter_c;
    float Xcenter_cc,Ycenter_cc;
    float Xcenter_ccsf,Ycenter_ccsf;
    float Xcenter_ccse,Ycenter_ccse;
    float limit;
    long cnt;
    float fwhm1,fwhm2,fwhm3,fwhm4;
    float ence1,ence2,ence3,ence4;
    float fs = 0.01128;

    Xcenter_c = 128;
    Ycenter_c = 128;

    FWHM = (float*) malloc(sizeof(float)*NBfiles);
    ENCE = (float*) malloc(sizeof(float)*NBfiles);
    xcen = (float*) malloc(sizeof(float)*NBfiles);
    ycen = (float*) malloc(sizeof(float)*NBfiles);

    /* make PSFc */
    if((fp=fopen(logfile,"r"))==NULL)
    {
        printf("ERROR: cannot open file \"%s\"\n",logfile);
        exit(0);
    }
    for(i=0; i<NBfiles; i++)
    {
        if(fscanf(fp,"%s %f %f %f %f\n",fname,&FWHM[i],&ENCE[i],&xcen[i],&ycen[i])!=5)
        {
            printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
            exit(0);
        }

        if(i==0)
            load_fits(fname, "PSFc", 1);
        else
        {
            load_fits(fname, "tmppsf", 1);
            execute_arith("PSFc=PSFc+tmppsf");
            delete_image_ID("tmppsf");
        }
    }
    fclose(fp);

    /* make PSFcc */
    Xcenter = 0.0;
    Ycenter = 0.0;
    if((fp=fopen(logfile,"r"))==NULL)
    {
        printf("ERROR: cannot open file \"%s\"\n",logfile);
        exit(0);
    }
    for(i=0; i<NBfiles; i++)
    {
        if(fscanf(fp,"%s %f %f %f %f\n",fname,&FWHM[i],&ENCE[i],&xcen[i],&ycen[i])!=5)
        {
            printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
            exit(0);
        }

        if(i==0)
        {
            load_fits(fname, "PSFcc", 1);
            Xcenter = xcen[0];
            Ycenter = ycen[0];
        }
        else
        {
            load_fits(fname, "tmppsf" , 1);
            basic_add("PSFcc","tmppsf","nPSFcc",Xcenter-xcen[i],Ycenter-ycen[i]);
            if(Xcenter<xcen[i])
                Xcenter = xcen[i];
            if(Ycenter<ycen[i])
                Ycenter = ycen[i];
            delete_image_ID("PSFcc");
            delete_image_ID("tmppsf");
            chname_image_ID("nPSFcc","PSFcc");
        }
    }
    fclose(fp);
    Xcenter_cc = Xcenter;
    Ycenter_cc = Ycenter;

    /* make PSFccsf */
    quick_sort_float(FWHM, NBfiles);
    limit = FWHM[(long) (frac*NBfiles)];
    Xcenter = 0.0;
    Ycenter = 0.0;
    if((fp=fopen(logfile,"r"))==NULL)
    {
        printf("ERROR: cannot open file \"%s\"\n",logfile);
        exit(0);
    }
    OK = 0;
    cnt = 0;
    for(i=0; i<NBfiles; i++)
    {
        if(fscanf(fp,"%s %f %f %f %f\n",fname,&FWHM[i],&ENCE[i],&xcen[i],&ycen[i])!=5)
        {
            printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
            exit(0);
        }

        if(FWHM[i]<limit)
        {
            cnt++;
            if(OK==0)
            {
                load_fits(fname, "PSFccsf", 1);
                Xcenter = xcen[i];
                Ycenter = ycen[i];
                OK=1;
            }
            else
            {
                load_fits(fname, "tmppsf", 1);
                basic_add("PSFccsf","tmppsf","nPSFccsf",Xcenter-xcen[i],Ycenter-ycen[i]);
                if(Xcenter<xcen[i])
                    Xcenter = xcen[i];
                if(Ycenter<ycen[i])
                    Ycenter = ycen[i];
                delete_image_ID("PSFccsf");
                delete_image_ID("tmppsf");
                chname_image_ID("nPSFccsf","PSFccsf");
            }
        }
    }
    fclose(fp);
    printf("PSFccsf: %ld frames kept\n",cnt);
    Xcenter_ccsf = Xcenter;
    Ycenter_ccsf = Ycenter;

    /* make PSFccse */
    quick_sort_float(ENCE, NBfiles);
    limit = ENCE[(long) (frac*NBfiles)];
    Xcenter = 0.0;
    Ycenter = 0.0;
    if((fp=fopen(logfile,"r"))==NULL)
    {
        printf("ERROR: cannot open file \"%s\"\n",logfile);
        exit(0);
    }
    OK = 0;
    cnt = 0;
    for(i=0; i<NBfiles; i++)
    {
        if(fscanf(fp,"%s %f %f %f %f\n",fname,&FWHM[i],&ENCE[i],&xcen[i],&ycen[i])!=5)
        {
            printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
            exit(0);
        }
        if(ENCE[i]<limit)
        {
            cnt++;
            if(OK==0)
            {
                load_fits(fname, "PSFccse", 1);
                Xcenter = xcen[i];
                Ycenter = ycen[i];
                OK=1;
            }
            else
            {
                load_fits(fname, "tmppsf", 1);
                basic_add("PSFccse","tmppsf","nPSFccse",Xcenter-xcen[i],Ycenter-ycen[i]);
                if(Xcenter<xcen[i])
                    Xcenter = xcen[i];
                if(Ycenter<ycen[i])
                    Ycenter = ycen[i];
                delete_image_ID("PSFccse");
                delete_image_ID("tmppsf");
                chname_image_ID("nPSFccse","PSFccse");
            }
        }
    }
    fclose(fp);
    printf("PSFccse : %ld frames kept\n",cnt);
    Xcenter_ccse = Xcenter;
    Ycenter_ccse = Ycenter;


    /* quality evaluation */
    fwhm1 = measure_FWHM("PSFc",Xcenter_c,Ycenter_c,1.0,128);
    fwhm2 = measure_FWHM("PSFcc",Xcenter_cc,Ycenter_cc,1.0,128);
    fwhm3 = measure_FWHM("PSFccsf",Xcenter_ccsf,Ycenter_ccsf,1.0,128);
    fwhm4 = measure_FWHM("PSFccse",Xcenter_ccse,Ycenter_ccse,1.0,128);
    ence1 = measure_enc_NRJ("PSFc",Xcenter_c,Ycenter_c,0.5);
    ence2 = measure_enc_NRJ("PSFcc",Xcenter_cc,Ycenter_cc,0.5);
    ence3 = measure_enc_NRJ("PSFccsf",Xcenter_ccsf,Ycenter_ccsf,0.5);
    ence4 = measure_enc_NRJ("PSFccse",Xcenter_ccse,Ycenter_ccse,0.5);

    printf("PSFc   :  %f %f\n",fwhm1*fs,2.0*ence1*fs);
    printf("PSFcc  :  %f %f\n",fwhm2*fs,2.0*ence2*fs);
    printf("PSFccsf:  %f %f\n",fwhm3*fs,2.0*ence3*fs);
    printf("PSFccse:  %f %f\n",fwhm4*fs,2.0*ence4*fs);

    free(FWHM);
    free(ENCE);
    free(xcen);
    free(ycen);

    return(0);
}




// explore long exposure PSF structure
double AtmosphericTurbulence_makePSF(double Kp, double Ki, double Kd, double Kdgain)
{
    FILE *fp;
    char wf_file_name[200];
    double PupScale = 0.01;



    // SIMULATION PARAMETERS

    long WFSLAMBDA = 600; // [nm]
    double SLAMBDA = 1.65e-6;
    double zeroptWFS = 8.354e10; // [ph/s/um/m2] (600 nm)
    //double zeroptWFS = 9.444e9; // [ph/s/um/m2] (1600 nm)


    long SCILAMBDA = 1600; // [nm]
    double zeroptSCI = 9.444e9; // [ph/s/um/m2] (H band)



    double TelDiam = 30.0;
    double etime = 10.0; // end of exposure [s]
    double etimestart = 0.01; // loop closing delay before start of exposure [s]
    double dtime = 0.00001; // internal time step, 0.1 ms
    double WFS_SamplingTime = 0.00025; // WFS sampling time [sec]
    double WFS_Delay = 0.0002; // [sec]  DELAY SHOULD BE SMALLER THAN SAMPLING TIME

    double bandpassWFS = 0.1; // [um]
    double bandpassSCI = 0.1; // [um]
    double throughputWFS = 0.1;
    double throughputSCI = 0.1;
    double magnWFS = 6.0;
    double magnSCI = 6.0;
    double FLUXSCI; // [ph/s]
    double FLUXWFS; // [ph/s]

    double rtime; // running time

    // input parameters
    float FOCAL_SCALE;

    long BINWF = 2; // 2 for 30m telescope
    long WFsize1; // after binning by BINWF

    long size = 512;
    long BINFACTOR = 8; // 8 for 8m telescope
    long sizeb;

    char WFDIRECTORY[200];
    char fnameamp[200];
    char fnamepha[200];
    char fname[200];

    long NBFRAMES;
    long CubeSize;
    long frame0, frame1, frameindex0, frameindex1, cubeindex0, cubeindex1;
    long frame;
    long ii, jj, ii1, jj1;
    long IDac0, IDac1, IDpc0, IDpc1;
    long IDacs0, IDacs1, IDpcs0, IDpcs1;
    char imname[200];
    double framefrac, framef;
    double amp, pha, val0, val1;
    double wfstime, wfstime1;
    long wfscnt;

    int WFSdelayWait;

    long IDtelpup;
    long IDatm_amp, IDatm_opd;
    long IDatm_amp_sci, IDatm_opd_sci;
    long IDwfs_opd, IDwfs_amp, IDwfs_mes_opd, IDwfs_mes_opd_prev;
    long IDwfs_mes_opd_derivative, IDwfs_mes_opd_integral;
    long IDdm_opd, IDdm_opd_tmp;
    long IDsci_amp, IDsci_opd;
    double tot;

    double RMSwf;

    long IDpupa, IDpupp;
    long ID, IDpsfcumul, ID1, IDpsfcumul1, IDre, IDim;
    double peak, re, im, reave, imave, tot0;
    double re_sci, im_sci, errpha, pha_ave, pha_ave_sci;
    double tmpd;
    long i, j;
    long cnt;

    long IDtmp;


    int PIDok = 0;
    double value = 0.0;
    long valuecnt = 0;



    AtmosphericTurbulence_ReadConf();

    FLUXSCI = zeroptSCI*M_PI*TelDiam*TelDiam/4.0*bandpassSCI*throughputSCI/pow(2.511886,magnSCI);
    printf("FLUX SCI = %g ph/s\n", FLUXSCI);
    FLUXWFS = zeroptWFS*M_PI*TelDiam*TelDiam/4.0*bandpassWFS*throughputWFS/pow(2.511886,magnWFS);
    printf("FLUX WFS = %g ph/s\n", FLUXWFS);

    sprintf(WFDIRECTORY,"/media/data1/WFsim/AtmSim_0.01");
    sprintf(CONFFILE, "%s/WF%04ld/AOsim.conf", WFDIRECTORY, WFSLAMBDA);
    AtmosphericTurbulence_change_configuration_file(CONFFILE);

    printf("Frame exposure time is %f s\n", etime);

    
    printf("PUPIL SCALE = %f m\n", CONF_PUPIL_SCALE);
    printf("PUPIL DIAM = %f pix\n", TelDiam/CONF_PUPIL_SCALE);


    WFsize1 = CONF_WFsize/BINWF;

    FOCAL_SCALE = SLAMBDA/CONF_WFsize/CONF_PUPIL_SCALE/PI*180.0*3600.0/BINWF; /* in arcsecond per pixel */
    printf("Scale is %f arcsecond per pixel (%ld pixels)\n", FOCAL_SCALE, CONF_WFsize);



    CubeSize = (long) (CONF_TIME_SPAN/CONF_WFTIME_STEP-0.001);
    NBFRAMES = (long) (1.0*etime/CONF_WFTIME_STEP);

    printf("CUBE SIZE = %ld\n", CubeSize);


    // MAKE PUPIL MASK
    IDtelpup = make_disk("TelPup", WFsize1, WFsize1, WFsize1/2, WFsize1/2, TelDiam*0.5/CONF_PUPIL_SCALE/BINWF);


    IDatm_opd = create_2Dimage_ID("atmopd", WFsize1, WFsize1); // Atmosphere OPD at WFS lambda
    IDatm_amp = create_2Dimage_ID("atmamp", WFsize1, WFsize1); // Atmosphere amplitude at WFS lambda

    IDatm_opd_sci = create_2Dimage_ID("atmopdsci", WFsize1, WFsize1); // Atmosphere OPD at SCI lambda
    IDatm_amp_sci = create_2Dimage_ID("atmampsci", WFsize1, WFsize1); // Atmosphere amplitude at SCI lambda

    IDwfs_opd = create_2Dimage_ID("wfsopd", WFsize1, WFsize1); // WFS OPD (corrected by DM)
    IDwfs_amp = create_2Dimage_ID("wfsamp", WFsize1, WFsize1); // WFS amplitude (corrected by DM)

    IDdm_opd_tmp = create_2Dimage_ID("dmopdtmp", WFsize1, WFsize1); // DM opd (before being sent to DM)
    IDdm_opd = create_2Dimage_ID("dmopd", WFsize1, WFsize1); // DM opd

    IDwfs_mes_opd = create_2Dimage_ID("wfsmesopd", WFsize1, WFsize1); // measured WFS OPD
    IDwfs_mes_opd_prev = create_2Dimage_ID("wfsmesopdprev", WFsize1, WFsize1); // previous measured WFS OPD
    IDwfs_mes_opd_derivative = create_2Dimage_ID("wfsmesopdder", WFsize1, WFsize1); // derivative of measured WFS OPD
    IDwfs_mes_opd_integral = create_2Dimage_ID("wfsmesopdint", WFsize1, WFsize1); // integral of measured WFS OPD

    IDsci_opd = create_2Dimage_ID("sciopd", WFsize1, WFsize1); // SCI OPD (corrected by DM)
    IDsci_amp = create_2Dimage_ID("sciamp", WFsize1, WFsize1); // SCI amplitude (corrected by DM)

    sizeb = size*BINFACTOR;
    IDpupa = create_2Dimage_ID("pupa", sizeb, sizeb);
    IDpupp = create_2Dimage_ID("pupp", sizeb, sizeb);
    IDpsfcumul = create_2Dimage_ID("PSFcumul", size, size);
    IDpsfcumul1 = create_2Dimage_ID("PSFcumul1", size, size);


    frame = 0;
    frame = 0;
    rtime = 0.0;
    framefrac = 0.0;
    framef = 0.0;
    cubeindex0 = 0;
    cubeindex1 = 0;
    frameindex0 = 0;
    frameindex1 = 1;

    /*  sprintf(fnamepha, "%s/WF0800/WF4096/wf_%08ld.pha", WFDIRECTORY, cubeindex1);
    sprintf(fnameamp, "%s/WF0800/WF4096/wf_%08ld.amp", WFDIRECTORY, cubeindex1);
    ID_WFphaC = load_fits(fnamepha, "WFphaC");
    ID_WFampC = load_fits(fnameamp, "WFampC");
    */

    wfstime = 0.0;
    for(ii=0; ii<WFsize1*WFsize1; ii++)
    {
        data.image[IDwfs_opd].array.F[ii] = 0.0;
        data.image[IDdm_opd].array.F[ii] = 0.0;
        data.image[IDwfs_mes_opd].array.F[ii] = 0.0;
        data.image[IDwfs_mes_opd_prev].array.F[ii] = 0.0;
        data.image[IDwfs_mes_opd_derivative].array.F[ii] = 0.0;
        data.image[IDwfs_mes_opd_integral].array.F[ii] = 0.0;
    }
    wfscnt = 0;
    WFSdelayWait = 1;

    fp = fopen("result.log", "w");
    fclose(fp);

    while (rtime < etime)
    {
        framef = rtime/CONF_WFTIME_STEP;
        frame = (long) framef;
        framefrac = framef-frame;

        // compute current OPD map for WFS
        frame0 = frame;
        frame1 = frame+1;
        frameindex0 = frame0-cubeindex0*CubeSize;
        frameindex1 = frame1-cubeindex1*CubeSize;

        printf("time = %2.10g s  (%04ld %04ld) (%04ld %04ld) %1.6f\n", rtime, frameindex0, cubeindex0, frameindex1, cubeindex1, framefrac);

        while(frameindex0>CubeSize-1)
        {
            cubeindex0 ++;
            frameindex0 -= CubeSize;
        }

        while(frameindex1>CubeSize-1)
        {
            cubeindex1 ++;
            frameindex1 -= CubeSize;
        }


        sprintf(imname, "wfa%08ld",cubeindex0);
        IDac0 = image_ID(imname);
        if(IDac0 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.amp.fits", WFDIRECTORY, WFSLAMBDA, cubeindex0, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDac0 = load_fits(fname, imname, 1);
        }
        sprintf(imname, "wfa%08ld",cubeindex1);
        IDac1 = image_ID(imname);
        if(IDac1 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.amp.fits", WFDIRECTORY, WFSLAMBDA, cubeindex1, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDac1 = load_fits(fname, imname, 1);
        }

        sprintf(imname, "wfp%08ld",cubeindex0);
        IDpc0 = image_ID(imname);
        if(IDpc0 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.pha.fits", WFDIRECTORY, WFSLAMBDA, cubeindex0, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDpc0 = load_fits(fname, imname, 1);
        }
        sprintf(imname, "wfp%08ld",cubeindex1);
        IDpc1 = image_ID(imname);
        if(IDpc1 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.pha.fits", WFDIRECTORY, WFSLAMBDA, cubeindex1, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDpc1 = load_fits(fname, imname, 1);
        }




        sprintf(imname, "swfa%08ld",cubeindex0);
        IDacs0 = image_ID(imname);
        if(IDacs0 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.amp.fits", WFDIRECTORY, SCILAMBDA, cubeindex0, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDacs0 = load_fits(fname, imname, 1);
        }
        sprintf(imname, "swfa%08ld",cubeindex1);
        IDacs1 = image_ID(imname);
        if(IDacs1 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.amp.fits", WFDIRECTORY, SCILAMBDA, cubeindex1, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDacs1 = load_fits(fname, imname, 1);
        }

        sprintf(imname, "swfp%08ld",cubeindex0);
        IDpcs0 = image_ID(imname);
        if(IDpcs0 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.pha.fits", WFDIRECTORY, SCILAMBDA, cubeindex0, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDpcs0 = load_fits(fname, imname, 1);
        }
        sprintf(imname, "swfp%08ld",cubeindex1);
        IDpcs1 = image_ID(imname);
        if(IDpcs1 == -1)
        {
            sprintf(fname, "%s/WF%04ld/WF4096/wf_%08ld.%09ld.pha.fits", WFDIRECTORY, SCILAMBDA, cubeindex1, (long) (1.0e12*SLAMBDA+0.5));
            printf("LOADING %s\n", fname);
            IDpcs1 = load_fits(fname, imname, 1);
        }



        if(cubeindex0>0)
        {
            sprintf(imname, "wfa%08ld",cubeindex0-1);
            IDtmp = image_ID(imname);
            if(IDtmp!=-1)
                delete_image_ID(imname);

            sprintf(imname, "wfp%08ld",cubeindex0-1);
            IDtmp = image_ID(imname);
            if(IDtmp!=-1)
                delete_image_ID(imname);

            sprintf(imname, "swfa%08ld",cubeindex0-1);
            IDtmp = image_ID(imname);
            if(IDtmp!=-1)
                delete_image_ID(imname);

            sprintf(imname, "swfp%08ld",cubeindex0-1);
            IDtmp = image_ID(imname);
            if(IDtmp!=-1)
                delete_image_ID(imname);
        }


        if(BINWF==1)
        {
            for(ii=0; ii<CONF_WFsize*CONF_WFsize; ii++)
            {
                val0 = data.image[IDac0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+ii];
                val1 = data.image[IDac1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+ii];
                amp = (1.0-framefrac)*val0 + framefrac*val1;

                val0 = data.image[IDpc0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+ii];
                val1 = data.image[IDpc1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+ii];
                pha = (1.0-framefrac)*val0 + framefrac*val1;


                data.image[IDatm_amp].array.F[ii] = amp;
                data.image[IDatm_opd].array.F[ii] = pha/2.0/M_PI*SLAMBDA;


                val0 = data.image[IDacs0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+ii];
                val1 = data.image[IDacs1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+ii];
                amp = (1.0-framefrac)*val0 + framefrac*val1;

                val0 = data.image[IDpcs0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+ii];
                val1 = data.image[IDpcs1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+ii];
                pha = (1.0-framefrac)*val0 + framefrac*val1;

                data.image[IDatm_amp_sci].array.F[ii] = amp;
                data.image[IDatm_opd_sci].array.F[ii] = pha/2.0/M_PI*(1.0e-9*SCILAMBDA);
            }
        }
        else
        {
            for(ii1=0; ii1<WFsize1; ii1++)
                for(jj1=0; jj1<WFsize1; jj1++)
                {
                    re = 0.0;
                    im = 0.0;
                    re_sci = 0.0;
                    im_sci = 0.0;
                    pha_ave = 0.0;
                    pha_ave_sci = 0.0;

                    for(i=0; i<BINWF; i++)
                        for(j=0; j<BINWF; j++)
                        {
                            ii = ii1*BINWF+i;
                            jj = jj1*BINWF+j;

                            val0 = data.image[IDac0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            val1 = data.image[IDac1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            amp = (1.0-framefrac)*val0 + framefrac*val1;

                            val0 = data.image[IDpc0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            val1 = data.image[IDpc1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            pha = (1.0-framefrac)*val0 + framefrac*val1;

                            re += amp*cos(pha);
                            im += amp*sin(pha);
                            pha_ave += pha;


                            val0 = data.image[IDacs0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            val1 = data.image[IDacs1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            amp = (1.0-framefrac)*val0 + framefrac*val1;

                            val0 = data.image[IDpcs0].array.F[frameindex0*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            val1 = data.image[IDpcs1].array.F[frameindex1*CONF_WFsize*CONF_WFsize+jj*CONF_WFsize+ii];
                            pha = (1.0-framefrac)*val0 + framefrac*val1;

                            re_sci += amp*cos(pha);
                            im_sci += amp*sin(pha);
                            pha_ave_sci += pha;
                        }
                    re /= (BINWF*BINWF);
                    im /= (BINWF*BINWF);
                    re_sci /= (BINWF*BINWF);
                    im_sci /= (BINWF*BINWF);
                    pha_ave /= (BINWF*BINWF);
                    pha_ave_sci /= (BINWF*BINWF);

                    data.image[IDatm_amp].array.F[jj1*WFsize1+ii1] = sqrt(re*re+im*im);
                    pha = pha_ave;
                    errpha = atan2(im,re)-pha_ave; // close to -2PI, 0, 2PI etc...
                    errpha = modf(errpha/(2.0*M_PI),&tmpd);
                    if(errpha>0.5)
                        errpha -= 1.0;
                    if(errpha<-0.5)
                        errpha += 1.0;
                    pha += errpha*2.0*M_PI;
                    data.image[IDatm_opd].array.F[jj1*WFsize1+ii1] = pha/2.0/M_PI*SLAMBDA;


                    data.image[IDatm_amp_sci].array.F[jj1*WFsize1+ii1] = sqrt(re_sci*re_sci+im_sci*im_sci);
                    pha = pha_ave_sci;
                    errpha = atan2(im_sci,re_sci)-pha_ave_sci; // close to -2PI, 0, 2PI etc...
                    errpha = modf(errpha/(2.0*M_PI),&tmpd);
                    if(errpha>0.5)
                        errpha -= 1.0;
                    if(errpha<-0.5)
                        errpha += 1.0;
                    pha += errpha*2.0*M_PI;
                    data.image[IDatm_opd_sci].array.F[jj1*WFsize1+ii1] = pha/2.0/M_PI*(1.0e-9*SCILAMBDA);
                }
        }

        //    save_fl_fits("atmopdsci","!test_atmopdsci.fits");
        //save_fl_fits("atmampsci","!test_atmampsci.fits");


        // Apply DM
        for(ii=0; ii<WFsize1*WFsize1; ii++)
        {
            data.image[IDwfs_opd].array.F[ii] = data.image[IDatm_opd].array.F[ii] - data.image[IDdm_opd].array.F[ii];
            data.image[IDsci_opd].array.F[ii] = data.image[IDatm_opd_sci].array.F[ii] - data.image[IDdm_opd].array.F[ii];
        }


        // WFS integration
        for(ii=0; ii<WFsize1*WFsize1; ii++)
            data.image[IDwfs_mes_opd].array.F[ii] += data.image[IDwfs_opd].array.F[ii];
        wfscnt++;



        if(wfstime>WFS_SamplingTime) // WFS measurement
        {

            // ADD WFS NOISE HERE
            //
            //

            if(PIDok == 1)
            {
                for(ii=0; ii<WFsize1*WFsize1; ii++)
                {
                    data.image[IDwfs_mes_opd].array.F[ii] /= wfscnt; // AVERAGE OVER WFS INTEGRATION TIME
                    data.image[IDwfs_mes_opd_derivative].array.F[ii] = (1.0-Kdgain)*data.image[IDwfs_mes_opd_derivative].array.F[ii] + Kdgain*(data.image[IDwfs_mes_opd].array.F[ii]-data.image[IDwfs_mes_opd_prev].array.F[ii])/WFS_SamplingTime;
                    data.image[IDwfs_mes_opd_integral].array.F[ii] += WFS_SamplingTime*data.image[IDwfs_mes_opd].array.F[ii];
                }
            }
            else
            {
                for(ii=0; ii<WFsize1*WFsize1; ii++)
                {
                    data.image[IDwfs_mes_opd].array.F[ii] /= wfscnt; // AVERAGE OVER WFS INTEGRATION TIME
                    data.image[IDwfs_mes_opd_derivative].array.F[ii] = 0.0;
                    data.image[IDwfs_mes_opd_integral].array.F[ii] += WFS_SamplingTime*data.image[IDwfs_mes_opd].array.F[ii];
                }
                PIDok = 1;
            }




            for(ii=0; ii<WFsize1*WFsize1; ii++)
            {
                data.image[IDdm_opd_tmp].array.F[ii] += Kp*data.image[IDwfs_mes_opd].array.F[ii] + Kd*data.image[IDwfs_mes_opd_derivative].array.F[ii]*WFS_SamplingTime;
                data.image[IDwfs_mes_opd].array.F[ii] = 0.0;
            }
            wfscnt = 0.0;
            wfstime = 0.0;
            WFSdelayWait = 1; // start wait
            wfstime1 = 0.0;



            for(ii=0; ii<WFsize1*WFsize1; ii++)
                data.image[IDwfs_mes_opd_prev].array.F[ii] = data.image[IDwfs_mes_opd].array.F[ii]; // PREVIOUS WFS MEASUREMENT
        }

        if((wfstime1>WFS_Delay)&&(WFSdelayWait==1))
        {
            printf("UPDATE DM SHAPE\n");
            WFSdelayWait = 0;
            for(ii=0; ii<WFsize1*WFsize1; ii++)
                data.image[IDdm_opd].array.F[ii] = data.image[IDdm_opd_tmp].array.F[ii];
        }

        // MEASURE WF QUALITY
        val0 = 0.0;
        val1 = 0.0;
        for(ii=0; ii<WFsize1*WFsize1; ii++)
        {
            val0 += data.image[IDwfs_opd].array.F[ii]*data.image[IDtelpup].array.F[ii];
            val1 += data.image[IDtelpup].array.F[ii];
        }
        for(ii=0; ii<WFsize1*WFsize1; ii++) // REMOVE PISTON
            data.image[IDwfs_opd].array.F[ii] -= val0/val1;

        val0 = 0.0;
        val1 = 0.0;
        for(ii=0; ii<WFsize1*WFsize1; ii++) // COMPUTE RMS WF QUALITY
        {
            val0 += data.image[IDsci_opd].array.F[ii]*data.image[IDsci_opd].array.F[ii]*data.image[IDtelpup].array.F[ii];
            val1 += data.image[IDtelpup].array.F[ii];
        }
        RMSwf = sqrt(val0/val1);

        //    tp("0.0");

        printf("WAVEFRONT QUALITY = %g m\n", RMSwf);
        fp = fopen("result.log", "a");
        fprintf(fp,"%10.10g %g\n", rtime, RMSwf);
        fclose(fp);

        if(rtime>etimestart)
        {
            value += RMSwf;
            valuecnt ++;
        }


        // MAKING FOCAL PLANE IMAGE
        //printf("%ld %ld\n", sizeb, WFsize1);
        // list_image_ID();
        //    printf("Image identifiers: %ld %ld %ld %ld %ld\n", IDpupa, IDpupp, IDtelpup, IDatm_amp_sci, IDsci_opd);
        fflush(stdout);

        tot = 0.0;
        for(ii=0; ii<sizeb; ii++)
            for(jj=0; jj<sizeb; jj++)
            {
                ii1 = WFsize1/2-sizeb/2+ii;
                jj1 = WFsize1/2-sizeb/2+jj;
                if((ii1>-1)&&(jj1>-1)&&(ii1<WFsize1)&&(jj1<WFsize1))
                {
                    data.image[IDpupa].array.F[jj*sizeb+ii] = data.image[IDtelpup].array.F[jj1*WFsize1+ii1]*data.image[IDatm_amp_sci].array.F[jj1*WFsize1+ii1];
                    tot += data.image[IDpupa].array.F[jj*sizeb+ii];
                    data.image[IDpupp].array.F[jj*sizeb+ii] = data.image[IDtelpup].array.F[jj1*WFsize1+ii1]*2.0*M_PI*data.image[IDsci_opd].array.F[jj1*WFsize1+ii1]/(1.0e-9*SCILAMBDA);
                }
            }

        //      tp("0.5");

        mk_reim_from_amph("pupa", "pupp", "pupre", "pupim", 0);
        basic_contract("pupre","pupre1",BINFACTOR,BINFACTOR);
        basic_contract("pupim","pupim1",BINFACTOR,BINFACTOR);
        delete_image_ID("pupre");
        delete_image_ID("pupim");
        mk_complex_from_reim("pupre1", "pupim1", "pupc", 0);
        permut("pupc");
        do2dfft("pupc","focc");
        permut("focc");
        delete_image_ID("pupc");
        mk_amph_from_complex("focc", "foca", "focp", 0);
        delete_image_ID("focc");
        delete_image_ID("focp");
        execute_arith("foci=foca*foca");
        delete_image_ID("foca");

        ID = image_ID("foci");
        IDpsfcumul = image_ID("PSFcumul");


        //      tp("1.0");


        // MAKING CORONAGRAPHIC FOCAL PLANE IMAGE
        // USING SIMPLE CORONAGRAPH MODEL REMOVING PERFECTLY MODE 0
        IDre = image_ID("pupre1");
        IDim = image_ID("pupim1");
        peak = 0.0;
        for(ii=0; ii<size*size; ii++)
        {
            re = data.image[IDre].array.F[ii];
            im = data.image[IDim].array.F[ii];
            amp = re*re+im*im;
            if(amp>peak)
                peak = amp;
        }
        reave = 0.0;
        imave = 0.0;
        cnt = 0;
        for(ii=0; ii<size*size; ii++)
        {
            re = data.image[IDre].array.F[ii];
            im = data.image[IDim].array.F[ii];
            reave += re;
            imave += im;
            amp = re*re+im*im;
            if(amp>0.2*peak)
                cnt ++;
        }
        reave /= cnt;
        imave /= cnt;
        for(ii=0; ii<size*size; ii++)
        {
            re = data.image[IDre].array.F[ii];
            im = data.image[IDim].array.F[ii];
            amp = re*re+im*im;
            if(amp>0.2*peak)
            {
                data.image[IDre].array.F[ii] -= reave;
                data.image[IDim].array.F[ii] -= imave;
            }
        }
        mk_complex_from_reim("pupre1", "pupim1", "pupc", 0);
        permut("pupc");
        do2dfft("pupc","focc");
        permut("focc");
        delete_image_ID("pupc");
        mk_amph_from_complex("focc", "foca", "focp", 0);
        delete_image_ID("focc");
        delete_image_ID("focp");
        execute_arith("focic=foca*foca");
        delete_image_ID("foca");
        delete_image_ID("pupre1");
        delete_image_ID("pupim1");


        ID = image_ID("foci");
        ID1 = image_ID("focic");
        peak = 1.0;
        if(rtime>etimestart)
        {
            peak = 0.0;
            for(ii=0; ii<size*size; ii++)
            {
                data.image[IDpsfcumul].array.F[ii] += data.image[ID].array.F[ii];
                data.image[IDpsfcumul1].array.F[ii] += data.image[ID1].array.F[ii];
                if(data.image[IDpsfcumul].array.F[ii]>peak)
                    peak = data.image[IDpsfcumul].array.F[ii];
            }
        }


        // SAVING RESULT
        if(1)
        {
            sprintf(fname, "!psf/psf_%1.10lf.fits", rtime);
            save_fl_fits("foci",fname);
            printf("tot = %g\n", tot);


            if(rtime>etimestart)
            {
                tot = (rtime-etimestart)*FLUXSCI; // total flux in image [ph]
                arith_image_cstmult("PSFcumul",1.0/peak,"PSFcumuln");
                save_fl_fits("PSFcumuln", "!PSFcumul.fits"); // normalized in contrast
                tot0 = arith_image_total("PSFcumuln");
                arith_image_cstmult_inplace("PSFcumuln",tot/tot0); // normalized to tot
                put_poisson_noise("PSFcumuln","PSFcumulnn");
                arith_image_cstmult_inplace("PSFcumulnn",tot0/tot); // re-normalized in contrast
                delete_image_ID("PSFcumuln");
                save_fl_fits("PSFcumulnn", "!PSFcumul_n.fits");

                arith_image_cstmult("PSFcumul1",1.0/peak,"PSFcumul1n");
                save_fl_fits("PSFcumul1n", "!PSFcumul1.fits");
                arith_image_cstmult_inplace("PSFcumul1n",tot/tot0);
                put_poisson_noise("PSFcumul1n","PSFcumul1nn");
                arith_image_cstmult_inplace("PSFcumul1nn",tot0/tot);
                delete_image_ID("PSFcumul1n");
                save_fl_fits("PSFcumul1nn", "!PSFcumul1_n.fits");
            }
        }

        rtime += dtime;
        wfstime += dtime;
        wfstime1 += dtime;

    }


    delete_image_ID("TelPup");
    delete_image_ID("atmopd");
    delete_image_ID("atmamp");
    delete_image_ID("atmopdsci");
    delete_image_ID("atmampsci");
    delete_image_ID("wfsopd");
    delete_image_ID("wfsamp");
    delete_image_ID("dmopdtmp");
    delete_image_ID("dmopd");
    delete_image_ID("wfsmesopd");
    delete_image_ID("wfsmesopdprev");
    delete_image_ID("wfsmesopdder");
    delete_image_ID("wfsmesopdint");
    delete_image_ID("sciopd");
    delete_image_ID("sciamp");
    delete_image_ID("pupa");
    delete_image_ID("pupp");
    delete_image_ID("PSFcumul");
    delete_image_ID("PSFcumul1");
    delete_image_ID("PSFcumulnn");
    delete_image_ID("PSFcumul1nn");
    delete_image_ID("focic");
    delete_image_ID("foci");



    sprintf(imname, "wfa%08ld",cubeindex0);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "wfp%08ld",cubeindex0);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "swfa%08ld",cubeindex0);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "swfp%08ld",cubeindex0);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "wfa%08ld",cubeindex1);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "wfp%08ld",cubeindex1);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "swfa%08ld",cubeindex1);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);

    sprintf(imname, "swfp%08ld",cubeindex1);
    IDtmp = image_ID(imname);
    if(IDtmp!=-1)
        delete_image_ID(imname);



    return(value/valuecnt);
}



// custom AO related processing of a series of WF

int AtmosphericTurbulence_WFprocess()
{
    FILE *fp;
    char wf_file_name[200];
    long ID;
    long k, kk;
    double time;
    long size, sizec;
    int OK;
    char command[500];
    double pupsize;


    double Kp, Ki, Kd, Kdgain;
    double val;
    long iter;

	double SLAMBDA = 1.65e-6;

    int r;



    // Std loop (no PID)
    Kp = 0.5;
    Ki = 0.0;
    Kd = 0.0;
    val = AtmosphericTurbulence_makePSF(Kp, Ki, Kd, Kdgain);
    exit(0);



    printf("CUSTOM PROCESSING\n");
    for(iter=15; iter<1000; iter++)
    {
        Kp = ran1(); //0.35; // loop gain
        Ki = ran1(); //0.0;
        Kd = ran1(); //0.1;
        Kdgain = ran1(); //0.5;

        Kp = 0.3;
        Ki = 0.0;
        Kd = 0.0;// ran1();
        //Kdgain = 0.5;


        val = AtmosphericTurbulence_makePSF(Kp, Ki, Kd, Kdgain);

        fp = fopen("res.log.txt","a");
        fprintf(fp, "%ld %e %e %e %e %e\n", iter, Kp, Ki, Kd, Kdgain, val);
        fclose(fp);
        sprintf(command, "cp result.log result_%05ld.log", iter);
        r = system(command);

        list_image_ID();
        exit(0);
    }

    OK = 1;
    k = 0;
    while(OK==1)
    {
        sprintf(wf_file_name,"wf550_%08ld.%09ld.pha.fits",k, (long) (1.0e12*SLAMBDA+0.5));
        ID = load_fits(wf_file_name, "tpmwfc", 1);
        if(ID==-1)
        {
            OK = 0;
        }
        else
        {
            size = data.image[ID].md[0].size[0];
            sizec = data.image[ID].md[0].size[2];
            printf("%ld  %ld %ld\n",k,size,sizec);

            for(kk=0; kk<sizec; kk++)
            {

            }

            delete_image_ID("tmpwfc");
        }
        k++;
    }

    return(0);
}


int AtmosphericTurbulence_makeHV_CN2prof(double wspeed, double r0, double sitealt, long NBlayer, const char *outfile)
{
    FILE *fp;
    double h;
    double CN2;
    double hstep, hmax;
    double A, A0;
    double CN2sum = 0.0;
    double r0val;
    double lambda = 0.55e-6;
    double Acoeff = 1.0;
    long iter;
    double Astep;
    long k;
    double l0, L0;
    double *layerarray_h;
    double *layerarray_CN2frac;
    double *layerarray_Wspeed;
    double *layerarray_sigmaWindSpeed;
    double *layerarray_Lwind;
    
    hmax = 30000.0;
    hstep = 1.0;
    A0 = 1.7e-14;

    Astep = 1.0;
    for(iter=0;iter<30;iter++)
    {
        A = A0*Acoeff;
        CN2sum = 0.0;
        for(h=sitealt; h<hmax; h+=hstep)
        {
            CN2 = 5.94e-53*pow(wspeed/27.0, 2.0)*pow(h,10.0)*exp(-h/1000.0) + 2.7e-16*exp(-h/1500.0) + A*exp(-(h-sitealt)/100.0);
            CN2sum += CN2/hstep;
        }
        r0val = 1.0/pow( 0.423*pow( 2.0*M_PI/lambda, 2.0)*CN2sum, 3.0/5.0);

      //  printf("Acoeff = %12f -> r0 = %10f m -> seeing = %10f arcsec\n", Acoeff, r0val, (lambda/r0val)/M_PI*180.0*3600.0);

        if(r0val>r0)
            Acoeff *= 1.0+Astep;
        else
            Acoeff /= 1.0+Astep;
        Astep *= 0.8;
    }


    fp = fopen("conf_turb.txt", "w");
    fprintf(fp, "%f\n", (lambda/r0val)/M_PI*180.0*3600.0);
    fclose(fp);

    layerarray_h = (double*) malloc(sizeof(double)*NBlayer);
    layerarray_CN2frac = (double*) malloc(sizeof(double)*NBlayer);
    layerarray_Wspeed = (double*) malloc(sizeof(double)*NBlayer);
    layerarray_sigmaWindSpeed = (double*) malloc(sizeof(double)*NBlayer);
    layerarray_Lwind = (double*) malloc(sizeof(double)*NBlayer);
    
    for(k=0;k<NBlayer;k++)
        {
            layerarray_h[k] = sitealt + pow(1.0*k/(NBlayer-1), 2.0)*(hmax-sitealt);
            layerarray_CN2frac[k] = 0.0;
        }

    hstep = 1.0;
    for(h=sitealt; h<hmax; h+=hstep)
        {
            CN2 = 5.94e-53*pow(wspeed/27.0, 2.0)*pow(h,10.0)*exp(-h/1000.0) + 2.7e-16*exp(-h/1500.0) + A*exp(-(h-sitealt)/100.0);
            k  = (long) (sqrt((h-sitealt)/(hmax-sitealt))*(1.0*NBlayer-1.0)+0.5);
            layerarray_CN2frac[k] += CN2;
        }
    
    fp = fopen(outfile, "w");
    fprintf(fp, "# altitude(m)   relativeCN2     speed(m/s)   direction(rad) outerscale[m] innerscale[m] sigmaWsp[m/s] Lwind[m]\n");
    fprintf(fp, "\n");
    for(k=0;k<NBlayer;k++)
        {
            layerarray_CN2frac[k] /= CN2sum;
            
            l0 = 0.008 + 0.072*pow(layerarray_h[k]/20000.0,1.6);
            
            if(layerarray_h[k]<14000.0)
                L0 = pow(10.0, 2.0 - 0.9*(layerarray_h[k]/14000.0) );
            else
                L0 = pow(10.0, 1.1 + 0.3*(layerarray_h[k]-14000.0)/6000.0);
            
            layerarray_Wspeed[k] = wspeed*(0.3+0.8*sqrt(1.0*k/(1.0+NBlayer)));
            layerarray_sigmaWindSpeed[k] = 0.1*layerarray_Wspeed[k];
            layerarray_Lwind[k] = 500.0;
            
            fprintf(fp, "%12f  %12f  %12f  %12f  %12f  %12f  %12f  %12f\n", layerarray_h[k], layerarray_CN2frac[k], layerarray_Wspeed[k], 2.0*M_PI*k/(1.0+NBlayer), L0, l0, layerarray_sigmaWindSpeed[k], layerarray_Lwind[k]);
        }
    fclose(fp);
    
    free(layerarray_h);
    free(layerarray_CN2frac);
    free(layerarray_Wspeed);
    free(layerarray_sigmaWindSpeed);
    free(layerarray_Lwind);
    
    return(0);
}


