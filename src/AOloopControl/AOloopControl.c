#include <fitsio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <sched.h>
#include <ncurses.h>
#include <semaphore.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>




#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "linopt_imtools/linopt_imtools.h"
#include "AOloopControl/AOloopControl.h"
#include "image_filter/image_filter.h"
#include "info/info.h"
#include "ZernikePolyn/ZernikePolyn.h"


#ifdef HAVE_CUDA
#include "cudacomp/cudacomp.h"
#endif


# ifdef _OPENMP
# include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
# endif

#define MAX_MBLOCK 12


int AOLCOMPUTE_TOTAL_ASYNC_THREADinit = 0;
sem_t AOLCOMPUTE_TOTAL_ASYNC_sem_name;
int AOLCOMPUTE_TOTAL_INIT = 0; // toggles to 1 AFTER total for first image is computed


int AOLCOMPUTE_DARK_SUBTRACT_THREADinit = 0;
int COMPUTE_DARK_SUBTRACT_NBTHREADS = 1;
sem_t AOLCOMPUTE_DARK_SUBTRACT_sem_name[32];
sem_t AOLCOMPUTE_DARK_SUBTRACT_RESULT_sem_name[32];

int COMPUTE_GPU_SCALING = 0; // perform scaling inside GPU instead of CPU
int initWFSref_GPU = 0;
float GPU_alpha = 0.0;
float GPU_beta = 0.0;

long ti; // thread index

int MATRIX_COMPUTATION_MODE = 0; 
// 0: compute sequentially modes and DM commands
// 1: use combined control matrix



int wcol, wrow; // window size


long aoconfID_WFS = -1;
int WFSatype;
long aoconfID_WFSdark = -1;
long aoconfID_WFS0 = -1;
long aoconfID_WFS1 = -1;
long aoconfID_WFS2 = -1;
long aoconfID_refWFS = -1;
long aoconfID_DM = -1;
long aoconfID_DMRM = -1;
long aoconfID_DMmodes = -1;
long aoconfID_DMdisp = -1;  // to notify DMcomb that DM maps should be summed

// Fourier Modes
long aoconfID_cmd_modes = -1;
long aoconfID_meas_modes = -1; // measured
long aoconfID_RMS_modes = -1;
long aoconfID_AVE_modes = -1;
long aoconfID_GAIN_modes = -1;
long aoconfID_LIMIT_modes = -1;
long aoconfID_MULTF_modes = -1;




long aoconfID_cmd_modesRM = -1;

long aoconfID_respM = -1;
long aoconfID_contrM = -1; // pixels -> modes
long aoconfID_contrMc = -1; // combined control matrix: pixels -> DM actuators
long aoconfID_meas_act = -1;
long aoconfID_contrMc_active = -1;

long aoconfIDlog0 = -1;
long aoconfIDlog1 = -1;


int *WFS_active_map; // used to map WFS pixels into active array
int *DM_active_map; // used to map DM actuators into active array
long aoconfID_meas_act_active;
long aoconfID_WFS2_active;


int RMACQUISITION = 0;  // toggles to 1 when resp matrix is being acquired


long refWFScnt0 = -1;



// variables used by functions 
char Average_cam_frames_dname[200];
long Average_cam_frames_IDdark = -1;
long Average_cam_frames_nelem = 1;









extern DATA data;


#define NB_AOloopcontrol 10 // max number of loops
long LOOPNUMBER = 0; // current loop index
int AOloopcontrol_meminit = 0;
int AOlooploadconf_init = 0;

#define AOconfname "/tmp/AOconf.shm"
AOLOOPCONTROL_CONF *AOconf; // configuration - this can be an array
int *AOconf_loaded = 0;
int *AOconf_fd; 

float *arrayftmp;
unsigned short *arrayutmp;
int avcamarraysInit = 0;

float normfloorcoeff = 1.0;



float IMTOTAL = 0.0;











// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//


int AOloopControl_makeTemplateAOloopconf_cli()
{
    if(CLI_checkarg(1,2)==0)
        AOloopControl_makeTemplateAOloopconf(data.cmdargtoken[1].val.numl);
    else
        return 1;
}


int AOloopControl_mkModes_cli()
{ 
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)+CLI_checkarg(7,1)+CLI_checkarg(8,1)+CLI_checkarg(9,2)==0)
    AOloopControl_mkModes(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf, data.cmdargtoken[7].val.numf, data.cmdargtoken[8].val.numf, data.cmdargtoken[9].val.numl);
  else
    return 1;      
}



int AOloopControl_camimage_extract2D_sharedmem_loop_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,2)+CLI_checkarg(6,2)==0)
    {
      AOloopControl_camimage_extract2D_sharedmem_loop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string , data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numl);
      return 0;
    }
  else
    return 1;
}


int AOloopControl_loadconfigure_cli()
{
  if(CLI_checkarg(1,2)==0)
    {
      AOloopControl_loadconfigure(data.cmdargtoken[1].val.numl, 1);
      return 0;
    }
  else
    return 1;
}

int AOloopControl_setLoopNumber_cli()
{
  if(CLI_checkarg(1,2)==0)
    {
      AOloopControl_setLoopNumber(data.cmdargtoken[1].val.numl);
      return 0;
    }
  else
    return 1;
}


int AOloopControl_setgain_cli()
{
  if(CLI_checkarg(1,1)==0)
    {
      AOloopControl_setgain(data.cmdargtoken[1].val.numf);
      return 0;
    }
  else
    return 1;
}


int AOloopControl_setWFSnormfloor_cli()
{
  if(CLI_checkarg(1,1)==0)
    {
      AOloopControl_setWFSnormfloor(data.cmdargtoken[1].val.numf);
      return 0;
    }
  else
    return 1;
}

int AOloopControl_setmaxlimit_cli()
{
  if(CLI_checkarg(1,1)==0)
    {
      AOloopControl_setmaxlimit(data.cmdargtoken[1].val.numf);
      return 0;
    }
  else
    return 1;
}

int AOloopControl_setmult_cli()
{
  if(CLI_checkarg(1,1)==0)
    {
      AOloopControl_setmult(data.cmdargtoken[1].val.numf);
      return 0;
    }
  else
    return 1;
}

int Measure_zonalRM_cli()
{
    if(CLI_checkarg(1,1)+CLI_checkarg(2,1)+CLI_checkarg(3,2)+CLI_checkarg(4,3)+CLI_checkarg(5,3)+CLI_checkarg(6,3)+CLI_checkarg(7,3)==0)
    {
        Measure_zonalRM(LOOPNUMBER, data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.string, data.cmdargtoken[6].val.string, data.cmdargtoken[7].val.string);
        return 0;
    }
    else
        return 1;
}




int AOloopControl_Measure_WFScam_PeriodicError_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,3)==0)
    {
      AOloopControl_Measure_WFScam_PeriodicError(LOOPNUMBER, data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}




int AOloopControl_Measure_Resp_Matrix_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,1)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,2)==0)
    {
      Measure_Resp_Matrix(LOOPNUMBER, data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl);
      return 0;
    }
  else
    return 1;
}


int AOloopControl_compute_CombinedControlMatrix_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,4)+CLI_checkarg(4,4)+CLI_checkarg(5,3)+CLI_checkarg(6,3)==0)
    {
      compute_CombinedControlMatrix(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.string, data.cmdargtoken[6].val.string);
      return 0;
    }
  else
    return 1;
}
//long compute_CombinedControlMatrix(char *IDcmat_name, char *IDmodes_name, char* IDwfsmask_name, char *IDdmmask_name, char *IDcmatc_name, char *IDcmatc_active_name)


int AOloopControl_setframesAve_cli()
{
  if(CLI_checkarg(1,2)==0)
    {
      AOloopControl_setframesAve(data.cmdargtoken[1].val.numl);
      return 0;
    }
  else
    return 1;
}


int AOloopControl_computeCM_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,4)+CLI_checkarg(3,3)+CLI_checkarg(4,1)+CLI_checkarg(5,2)+CLI_checkarg(6,1)==0)
    {
      compute_ControlMatrix(LOOPNUMBER, data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, "evecM", data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numf);
      save_fits("evecM","!evecM.fits");
      delete_image_ID("evecM");
    }
  else
    return 1;
}


int AOloopControl_loadCM_cli()
{
  if(CLI_checkarg(1,3)==0)
    {
      AOloopControl_loadCM(LOOPNUMBER, data.cmdargtoken[1].val.string);
      return 0;
    }
  else
    return 1;
}




int AOloopControl_loopstep_cli()
{
  if(CLI_checkarg(1,2)==0)
    {
      AOloopControl_loopstep(LOOPNUMBER, data.cmdargtoken[1].val.numl);
      return 0;
    }
  else
    return 1;
}




int AOloopControl_loopMonitor_cli()
{
 if(CLI_checkarg(1,1)+CLI_checkarg(2,2)==0)
   {
     AOloopControl_loopMonitor(LOOPNUMBER, data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.numl);
     return 0;
   }
 else
   {
     AOloopControl_loopMonitor(LOOPNUMBER, 1.0, 8);
     return 0;
   }
}


int AOloopControl_setgainrange_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,1)==0)
    {
      AOloopControl_setgainrange(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf);
      return 0;
    }
  else
    return 1; 
}


int AOloopControl_setlimitrange_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,1)==0)
    {
      AOloopControl_setlimitrange(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf);
      return 0;
    }
  else
    return 1; 
}


int AOloopControl_setmultfrange_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,1)==0)
    {
      AOloopControl_setmultfrange(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf);
      return 0;
    }
  else
    return 1; 
}



int AOloopControl_setgainblock_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,1)==0)
    {
      AOloopControl_setgainblock(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf);
      return 0;
    }
  else
    return 1; 
}


int AOloopControl_setlimitblock_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,1)==0)
    {
      AOloopControl_setlimitblock(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf);
      return 0;
    }
  else
    return 1; 
}


int AOloopControl_setmultfblock_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,1)==0)
    {
      AOloopControl_setmultfblock(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf);
      return 0;
    }
  else
    return 1; 
}




int AOloopControl_InjectMode_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,1)==0)
    {
      AOloopControl_InjectMode(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf);
      return 0;
    }
  else
    return 1; 
}



int AOloopControl_scanGainBlock_cli()
{
  if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,2)==0)
    {
      AOloopControl_scanGainBlock(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numl);
      return 0;
    }
  else
    return 1; 
}



int AOloopControl_setparam_cli()
{
    if(CLI_checkarg(1,3)+CLI_checkarg(2,1)==0)
    {
        AOloopControl_setparam(LOOPNUMBER, data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf);
        return 0;
    }
    else
        return 1;
}





int init_AOloopControl()
{
  FILE *fp;
  int r;

  if((fp=fopen("loopnb.txt","r"))!=NULL)
    {
      r = fscanf(fp,"%ld", &LOOPNUMBER);
      printf("LOOP NUMBER = %ld\n", LOOPNUMBER);
      fclose(fp);
    }
  else
    LOOPNUMBER = 0;
  

  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "AO loop control");
  data.NBmodule++;


  strcpy(data.cmd[data.NBcmd].key,"aolmkconf");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_makeTemplateAOloopconf_cli;
  strcpy(data.cmd[data.NBcmd].info,"make template configuration file");
  strcpy(data.cmd[data.NBcmd].syntax,"<loopnb [long]>");
  strcpy(data.cmd[data.NBcmd].example,"aolmkconf 2");
  strcpy(data.cmd[data.NBcmd].Ccall,"long AOloopControl_makeTemplateAOloopconf(long loopnb)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolmkmodes");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_mkModes_cli;
  strcpy(data.cmd[data.NBcmd].info,"make control modes");
  strcpy(data.cmd[data.NBcmd].syntax,"<output modes> <size> <max CPA> <delta CPA> <cx> <cy> <r0> <r1> <masking mode>");
  strcpy(data.cmd[data.NBcmd].example,"aolmkmodes modes 50 5.0 0.8 1");
  strcpy(data.cmd[data.NBcmd].Ccall,"long AOloopControl_mkModes(char *ID_name, long msize, float CPAmax, float deltaCPA, double xc, double yx, double r0, double r1, int MaskMode)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"cropshim");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_camimage_extract2D_sharedmem_loop_cli;
  strcpy(data.cmd[data.NBcmd].info,"crop shared mem image");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <output image>");
  strcpy(data.cmd[data.NBcmd].example,"cropshim imin imout");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_camimage_extract2D_sharedmem_loop(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolloadconf");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loadconfigure_cli;
  strcpy(data.cmd[data.NBcmd].info,"load AO loop configuration");
  strcpy(data.cmd[data.NBcmd].syntax,"<loop #>");
  strcpy(data.cmd[data.NBcmd].example,"AOlooploadconf 1");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_loadconfigure(long loopnb, 1)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolmeaszrm");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = Measure_zonalRM_cli;
  strcpy(data.cmd[data.NBcmd].info,"measure zonal resp mat, WFS ref, DM and WFS response maps");
  strcpy(data.cmd[data.NBcmd].syntax,"<ampl [float]> <delay second [float]> <nb frames per position [long]> <output image [string]> <output WFS ref [string]>  <output WFS response map [string]>  <output DM response map [string]>");
  strcpy(data.cmd[data.NBcmd].example,"aolmeaszrm 0.05 0.02 20 zrm wfsref wfsmap dmmap");
  strcpy(data.cmd[data.NBcmd].Ccall,"long Measure_zonalRM(long loop, double ampl, double delays, long NBave, char *zrespm_name, char *WFSref_name, char *WFSmap_name, char *DMmap_name)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolacqwfscampe");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_Measure_WFScam_PeriodicError_cli;
  strcpy(data.cmd[data.NBcmd].info,"acquire WFS camera periodic error");
  strcpy(data.cmd[data.NBcmd].syntax,"<nbframes [long]> <nb pha [long]> <outcube>");
  strcpy(data.cmd[data.NBcmd].example,"aolacqwfscampe 10000 100 wfscampe");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_Measure_WFScam_PeriodicError(long loop, long NBframes, long NBpha, char *IDout_name)");
  data.NBcmd++;



  strcpy(data.cmd[data.NBcmd].key,"aolacqresp");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_Measure_Resp_Matrix_cli;
  strcpy(data.cmd[data.NBcmd].info,"acquire AO response matrix and WFS reference");
  strcpy(data.cmd[data.NBcmd].syntax,"<ave# [long]> <ampl [float]> <nbloop [long]> <frameDelay [long]> <NBiter [long]>");
  strcpy(data.cmd[data.NBcmd].example,"aolacqresp 50 0.1 5 2");
  strcpy(data.cmd[data.NBcmd].Ccall,"int Measure_Resp_Matrix(long loop, long NbAve, float amp, long nbloop, long fDelay, long NBiter)");
  data.NBcmd++;


strcpy(data.cmd[data.NBcmd].key,"aolcompcmatc");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_compute_CombinedControlMatrix_cli;
  strcpy(data.cmd[data.NBcmd].info,"compute combined control matrix");
  strcpy(data.cmd[data.NBcmd].syntax,"<modal control matrix> <modes> <wfs mask> <dm mask> <combined cmat> <combined cmat, only active elements>");
  strcpy(data.cmd[data.NBcmd].example,"aolcompcmatc cmat fmodes wfsmask dmmask cmatc cmatcact");
  strcpy(data.cmd[data.NBcmd].Ccall,"long compute_CombinedControlMatrix(char *IDcmat_name, char *IDmodes_name, char* IDwfsmask_name, char *IDdmmask_name, char *IDcmatc_name, char *IDcmatc_active_name)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolrun");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_run;
  strcpy(data.cmd[data.NBcmd].info,"run AO loop");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"AOlooprun");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_run()");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolkill");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loopkill;
  strcpy(data.cmd[data.NBcmd].info,"kill AO loop");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aolkill");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setLoopNumber()");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolnb");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setLoopNumber_cli;
  strcpy(data.cmd[data.NBcmd].info,"set AO loop #");
  strcpy(data.cmd[data.NBcmd].syntax,"<loop nb>");
  strcpy(data.cmd[data.NBcmd].example,"AOloopnb 0");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setLoopNumber(long loop)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolon");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loopon;
  strcpy(data.cmd[data.NBcmd].info,"turn loop on");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aolon");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_loopon()");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolstep");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loopstep_cli;
  strcpy(data.cmd[data.NBcmd].info,"turn loop on for N steps");
  strcpy(data.cmd[data.NBcmd].syntax,"<nbstep>");
  strcpy(data.cmd[data.NBcmd].example,"aolstep");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_loopstep(long loop, long NBstep)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aoloff");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loopoff;
  strcpy(data.cmd[data.NBcmd].info,"turn loop off");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aoloff");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_loopoff()");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolreset");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loopreset;
  strcpy(data.cmd[data.NBcmd].info,"reset loop, and turn it off");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aolreset");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_loopreset()");
  data.NBcmd++;

/*  strcpy(data.cmd[data.NBcmd].key,"aollogon");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_logon;
  strcpy(data.cmd[data.NBcmd].info,"turn log on");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aollogon");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_logon()");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aollogoff");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_logoff;
  strcpy(data.cmd[data.NBcmd].info,"turn log off");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aollogoff");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_logoff()");
  data.NBcmd++;
  */
  
  strcpy(data.cmd[data.NBcmd].key,"aolsetgain");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setgain_cli;
  strcpy(data.cmd[data.NBcmd].info,"set gain");
  strcpy(data.cmd[data.NBcmd].syntax,"<gain value>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetgain 0.1");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setgain(float gain)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolsetwfsnormf");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setWFSnormfloor_cli;
  strcpy(data.cmd[data.NBcmd].info,"set WFS normalization floor");
  strcpy(data.cmd[data.NBcmd].syntax,"<floor value (total flux)>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetwfsnormf 10000.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setWFSnormfloor(float WFSnormfloor)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolsetmaxlim");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setmaxlimit_cli;
  strcpy(data.cmd[data.NBcmd].info,"set max limit for AO mode correction");
  strcpy(data.cmd[data.NBcmd].syntax,"<limit value>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetmaxlim 0.01");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setmaxlimit(float maxlimit)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolsetmult");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setmult_cli;
  strcpy(data.cmd[data.NBcmd].info,"set mult coeff for AO mode correction");
  strcpy(data.cmd[data.NBcmd].syntax,"<mult value>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetmult 0.98");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setmult(float multcoeff)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolsetnbfr");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setframesAve_cli;
  strcpy(data.cmd[data.NBcmd].info,"set number of frames to be averaged");
  strcpy(data.cmd[data.NBcmd].syntax,"<nb frames>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetnbfr 10");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setframesAve(long nbframes)");
  data.NBcmd++;
  

  strcpy(data.cmd[data.NBcmd].key,"aolcmmake");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_computeCM_cli;
  strcpy(data.cmd[data.NBcmd].info,"make control matrix");
  strcpy(data.cmd[data.NBcmd].syntax,"<NBmodes removed> <RespMatrix> <ContrMatrix> <beta> <nbremovedstep> <eigenvlim>");
  strcpy(data.cmd[data.NBcmd].example,"aolcmmake 8 respm cmat");
  strcpy(data.cmd[data.NBcmd].Ccall,"int compute_ControlMatrix(long loop, long NB_MODE_REMOVED, char *ID_Rmatrix_name, char *ID_Cmatrix_name, char *ID_VTmatrix_name, double Beta, long NB_MODE_REMOVED_STEP, float eigenvlim)");
  data.NBcmd++;

 
  strcpy(data.cmd[data.NBcmd].key,"aolloadcm");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loadCM_cli;
  strcpy(data.cmd[data.NBcmd].info,"load new control matrix from file");
  strcpy(data.cmd[data.NBcmd].syntax,"<fname>");
  strcpy(data.cmd[data.NBcmd].example,"aolloadcm cm32.fits");
  strcpy(data.cmd[data.NBcmd].Ccall,"long AOloopControl_loadCM(long loop, char *CMfname)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"aolmon");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_loopMonitor_cli;
  strcpy(data.cmd[data.NBcmd].info,"monitor loop");
  strcpy(data.cmd[data.NBcmd].syntax,"<frequ> <Nbcols>");
  strcpy(data.cmd[data.NBcmd].example,"aolmon 10.0 3");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_loopMonitor(long loop, double frequ)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"aolsetgainr");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setgainrange_cli;
  strcpy(data.cmd[data.NBcmd].info,"set modal gains");
  strcpy(data.cmd[data.NBcmd].syntax,"<modemin [long]> <modemax [long]> <gainval>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetgainr 20 30 0.2");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setgainrange(long m0, long m1, float gainval)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolsetlimitr");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setlimitrange_cli;
  strcpy(data.cmd[data.NBcmd].info,"set modal limits");
  strcpy(data.cmd[data.NBcmd].syntax,"<modemin [long]> <modemax [long]> <limval>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetlimitr 20 30 0.02");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setlimitrange(long m0, long m1, float gainval)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolsetmultfr");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setmultfrange_cli;
  strcpy(data.cmd[data.NBcmd].info,"set modal multf");
  strcpy(data.cmd[data.NBcmd].syntax,"<modemin [long]> <modemax [long]> <multfval>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetmultfr 10 30 0.98");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setmultfrange(long m0, long m1, float multfval)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolsetgainb");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setgainblock_cli;
  strcpy(data.cmd[data.NBcmd].info,"set modal gains by block");
  strcpy(data.cmd[data.NBcmd].syntax,"<block [long]> <gainval>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetgainb 2 0.2");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setgainblock(long m0, long m1, float gainval)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolsetlimitb");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setlimitblock_cli;
  strcpy(data.cmd[data.NBcmd].info,"set modal limits by block");
  strcpy(data.cmd[data.NBcmd].syntax,"<block [long]> <limval>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetlimitb 2 0.02");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setlimitblock(long m0, long m1, float gainval)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolsetmultfb");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setmultfblock_cli;
  strcpy(data.cmd[data.NBcmd].info,"set modal multf by block");
  strcpy(data.cmd[data.NBcmd].syntax,"<block [long]> <multfval>");
  strcpy(data.cmd[data.NBcmd].example,"aolsetmultfb 2 0.98");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setmultfblock(long mb, float multfval)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolresetrms");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_resetRMSperf;
  strcpy(data.cmd[data.NBcmd].info,"reset RMS performance monitor");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aolresetrms");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_resetRMSperf()");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"aolscangainb");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_scanGainBlock_cli;
  strcpy(data.cmd[data.NBcmd].info,"scan gain for block");
  strcpy(data.cmd[data.NBcmd].syntax,"<blockNB> <NBAOsteps> <gainstart> <gainend> <NBgainpts>");
  strcpy(data.cmd[data.NBcmd].example,"aolscangainb");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_scanGainBlock(long NBblock, long NBstep, float gainStart, float gainEnd, long NBgain)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolstatusstats");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_statusStats;
  strcpy(data.cmd[data.NBcmd].info,"measures distribution of status values");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aolstatusstats");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_statusStats()");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolinjectmode");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_InjectMode_cli;
  strcpy(data.cmd[data.NBcmd].info,"inject single mode error into RM channel");
  strcpy(data.cmd[data.NBcmd].syntax,"<index> <ampl>");
  strcpy(data.cmd[data.NBcmd].example,"aolinjectmode 20 0.1");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_InjectMode()");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolautotune");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_AutoTune;
  strcpy(data.cmd[data.NBcmd].info,"auto tuning of loop parameters");
  strcpy(data.cmd[data.NBcmd].syntax,"no arg");
  strcpy(data.cmd[data.NBcmd].example,"aolautotune");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_AutoTune()");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"aolset");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = AOloopControl_setparam_cli;
  strcpy(data.cmd[data.NBcmd].info,"set parameter");
  strcpy(data.cmd[data.NBcmd].syntax,"<parameter> <value>");
  strcpy(data.cmd[data.NBcmd].example,"aolset");
  strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_setparam(long loop, char *key, double value)");
  data.NBcmd++;

  
  
  

  // add atexit functions here
  // atexit((void*) SCEXAO_DM_unloadconf);
  
  return 0;
}







/** \brief Creates a blank configuration
 *
 *
 *
 */

long AOloopControl_makeTemplateAOloopconf(long loopnb)
{
    FILE *fp;
    char fname[256];
    int r;
    char command[256];
    char line[256];

    // make configuration directory
    r = system("mkdir -p ./conf/");

    /*  sprintf(fname, "./conf/AOloop.conf");

       fp = fopen(fname, "w");
       fprintf(fp, "logsize         1000            number of consecutive entries in single log file\n");
       fprintf(fp, "logdir          ./\n");
       fprintf(fp, "NBMblocks	3		number of modes blocks\n");
       fclose(fp);
      */


    // LOOP NAME
    printf("Enter loop name: ");
    if (fgets(line, sizeof(line), stdin)) {
        printf("LOOP NAME = %s\n", line);
    }
    fp = fopen("./conf/conf_LOOPNAME.txt", "w");
    fprintf(fp, "%s", line);
    fclose(fp);


    return(0);
}





/*** \brief creates AO control modes
 *
 *
 * creates image "modesfreqcpa" which contains CPA value for each mode
 *
 *
 * if Mmask exists, measure xc, yc from it, otherwise use values given to function
 *
 * MaskMode = 0  : tapered masking
 * MaskMode = 1  : STRICT masking
 *
 */

long AOloopControl_mkModes(char *ID_name, long msize, float CPAmax, float deltaCPA, double xc, double yc, double r0, double r1, int MaskMode)
{
    long ID0;
    long ID;
    long k, ii, jj;

    long IDmask;

    double ave;
    double offset;
    double totm;
    double totvm;

    double a0=0.88;
    double b0=40.0;

    double a1=1.2;
    double b1=12.0;

    double x, y, r, PA, xc1, yc1;
    double val0, val1, rms;

    long IDtm, IDem;
    long IDeModes;

    long kelim = 20;
    double coeff;
    long citer;
    long NBciter = 200;
    long IDg;


    long NBZ;
    long IDz;

    long zindex[10];
    double zcpa[10];  /// CPA for each Zernike (somewhat arbitrary... used to sort modes in CPA)
    long IDfreq;

    long IDmfcpa; /// modesfreqcpa ID
    int ret;

    long mblock, m;
    long NBmblock;
    float CPAblocklim[MAX_MBLOCK]; // defines CPA limits for blocks
    long MBLOCK_NBmode[MAX_MBLOCK]; // number of blocks
    long MBLOCK_ID[MAX_MBLOCK];
    float MBLOCK_CPA[MAX_MBLOCK];
    float cpa;

    char *ptr0;
    char *ptr1;

    char imname[200];
    char fname[200];
    char command[1000];
    
    float value, value0, value1;
    long msize2;
    long m0, mblock0;

    long iter;


    /// if Mmask exists, use it, otherwise create it
    IDmask = image_ID("Mmask");

    if(IDmask==-1)
    {
        IDmask = create_2Dimage_ID("Mmask", msize, msize);
        for(ii=0; ii<msize; ii++)
            for(jj=0; jj<msize; jj++)
            {
                x = 1.0*ii-xc;
                y = 1.0*jj-yc;
                r = sqrt(x*x+y*y)/r1;
                val1 = 1.0-exp(-pow(a1*r,b1));
                r = sqrt(x*x+y*y)/r0;
                val0 = exp(-pow(a0*r,b0));
                data.image[IDmask].array.F[jj*msize+ii] = val0*val1;
            }
        //     save_fits("Mmask", "!Mmask.fits");
        xc1 = xc;
        yc1 = yc;
    }
    else /// extract xc and yc from mask
    {
        xc1 = 0.0;
        yc1 = 0.0;
        totm = 0.0;
        for(ii=0; ii<msize; ii++)
            for(jj=0; jj<msize; jj++)
            {
                xc1 += 1.0*ii*data.image[IDmask].array.F[jj*msize+ii];
                yc1 += 1.0*jj*data.image[IDmask].array.F[jj*msize+ii];
                totm += data.image[IDmask].array.F[jj*msize+ii];
            }
        xc1 /= totm;
        yc1 /= totm;
    }

    totm = arith_image_total("Mmask");
    msize = data.image[IDmask].md[0].size[0];




    

    NBZ = 5; /// 3: tip, tilt, focus

    zindex[0] = 1; // tip
    zcpa[0] = 0.0;

    zindex[1] = 2; // tilt
    zcpa[1] = 0.0;

    zindex[2] = 4; // focus
    zcpa[2] = 0.25;

    zindex[3] = 3; // astig
    zcpa[3] = 0.4;

    zindex[4] = 5; // astig
    zcpa[4] = 0.4;

    zindex[5] = 7; // coma
    zcpa[5] = 0.6;

    zindex[6] = 8; // coma
    zcpa[6] = 0.6;

    zindex[7] = 6; // trefoil
    zcpa[7] = 1.0;

    zindex[8] = 9; // trefoil
    zcpa[8] = 1.0;

    zindex[9] = 12;
    zcpa[9] = 1.5;

    
    NBZ = 0;
    for(m=0;m<10;m++)
    {
        if(zcpa[m]<CPAmax)
            NBZ++;
    }



    linopt_imtools_makeCPAmodes("CPAmodes", msize, CPAmax, deltaCPA, 0.5*msize, 1.2, 0);
    ID0 = image_ID("CPAmodes");

    IDfreq	= image_ID("cpamodesfreq");
    //  list_image_ID();


    printf("  %ld %ld %ld\n", msize, msize, data.image[ID0].md[0].size[2]-1 );
    ID = create_3Dimage_ID(ID_name, msize, msize, data.image[ID0].md[0].size[2]-1+NBZ);

    IDmfcpa = create_2Dimage_ID("modesfreqcpa", data.image[ID0].md[0].size[2]-1+NBZ, 1);

    /*** Create TTF first */
    zernike_init();
    for(k=0; k<NBZ; k++)
    {
        data.image[IDmfcpa].array.F[k] = zcpa[k];
        for(ii=0; ii<msize; ii++)
            for(jj=0; jj<msize; jj++)
            {
                x = 1.0*ii-xc1;
                y = 1.0*jj-yc1;
                r = sqrt(x*x+y*y)/r1;
                PA = atan2(y,x);
                data.image[ID].array.F[k*msize*msize+jj*msize+ii] = Zernike_value(zindex[k], r, PA);
            }
    }
    for(k=0; k<data.image[ID0].md[0].size[2]-1; k++)
    {
        data.image[IDmfcpa].array.F[k+NBZ] = data.image[IDfreq].array.F[k+1];
        for(ii=0; ii<msize*msize; ii++)
            data.image[ID].array.F[(k+NBZ)*msize*msize+ii] = data.image[ID0].array.F[(k+1)*msize*msize+ii];
    }


    for(k=0; k<data.image[ID0].md[0].size[2]-1+NBZ; k++)
    {
        /// Remove excluded modes
        IDeModes = image_ID("emodes");
        if(IDeModes!=-1)
        {
            IDtm = create_2Dimage_ID("tmpmode", msize, msize);

            for(ii=0; ii<msize*msize; ii++)
                data.image[IDtm].array.F[ii] = data.image[ID].array.F[k*msize*msize+ii];
            linopt_imtools_image_fitModes("tmpmode", "emodes", "Mmask", 1.0e-5, "lcoeff", 0);
            linopt_imtools_image_construct("emodes", "lcoeff", "em00");
            delete_image_ID("lcoeff");
            IDem = image_ID("em00");

            coeff = 1.0-exp(-pow(1.0*k/kelim,6.0));
            if(k>2.0*kelim)
                coeff = 1.0;
            for(ii=0; ii<msize*msize; ii++)
                data.image[ID].array.F[k*msize*msize+ii] = data.image[IDtm].array.F[ii] - coeff*data.image[IDem].array.F[ii];

            delete_image_ID("em00");
            delete_image_ID("tmpmode");
        }


        ave = 0.0;
        totvm = 0.0;
        for(ii=0; ii<msize*msize; ii++)
        {
            //	  data.image[ID].array.F[k*msize*msize+ii] = data.image[ID0].array.F[(k+1)*msize*msize+ii];
            totvm += data.image[ID].array.F[k*msize*msize+ii]*data.image[IDmask].array.F[ii];
        }
        offset = totvm/totm;

        for(ii=0; ii<msize*msize; ii++)
        {
            data.image[ID].array.F[k*msize*msize+ii] -= offset;
            data.image[ID].array.F[k*msize*msize+ii] *= data.image[IDmask].array.F[ii];
        }

        offset = 0.0;
        for(ii=0; ii<msize*msize; ii++)
            offset += data.image[ID].array.F[k*msize*msize+ii];

        rms = 0.0;
        for(ii=0; ii<msize*msize; ii++)
        {
            data.image[ID].array.F[k*msize*msize+ii] -= offset/msize/msize;
            rms += data.image[ID].array.F[k*msize*msize+ii]*data.image[ID].array.F[k*msize*msize+ii];
        }
        rms = sqrt(rms/totm);
        printf("Mode %ld   RMS = %lf\n", k, rms);
        for(ii=0; ii<msize*msize; ii++)
            data.image[ID].array.F[k*msize*msize+ii] /= rms;
    }


    for(k=0; k<data.image[ID0].md[0].size[2]-1+NBZ; k++)
    {
        rms = 0.0;
        for(ii=0; ii<msize*msize; ii++)
        {
            data.image[ID].array.F[k*msize*msize+ii] -= offset/msize/msize;
            rms += data.image[ID].array.F[k*msize*msize+ii]*data.image[ID].array.F[k*msize*msize+ii];
        }
        rms = sqrt(rms/totm);
        printf("Mode %ld   RMS = %lf\n", k, rms);
    }


    if(MaskMode==0)
    {
        for(citer=0; citer<NBciter; citer++)
        {
            printf("Convolution [%3ld/%3ld]\n", citer, NBciter);
            gauss_filter(ID_name, "modeg", 4.0*pow(1.0*(NBciter-citer)/NBciter,0.5), 5);
            IDg = image_ID("modeg");
            for(k=0; k<data.image[ID].md[0].size[2]; k++)
            {
                for(ii=0; ii<msize*msize; ii++)
                    if(data.image[IDmask].array.F[ii]<0.98)
                        data.image[ID].array.F[k*msize*msize+ii] = data.image[IDg].array.F[k*msize*msize+ii];
            }
            delete_image_ID("modeg");
        }
    }



    CPAblocklim[0] = 0.1; // tip and tilt
    CPAblocklim[1] = 0.3; // focus
    CPAblocklim[2] = 1.6; // other Zernikes
    CPAblocklim[3] = 3.0;
    CPAblocklim[4] = 5.0;
    CPAblocklim[5] = 7.0;
    CPAblocklim[6] = 9.0;
    CPAblocklim[7] = 11.0;
    CPAblocklim[8] = 13.0;
    CPAblocklim[9] = 15.0;
    CPAblocklim[10] = 17.0;
    CPAblocklim[11] = 100.0;

    for(mblock=0; mblock<MAX_MBLOCK; mblock++)
        MBLOCK_NBmode[mblock] = 0;



    NBmblock = 0;
    for(m=0; m<data.image[ID].md[0].size[2]; m++)
    {
        cpa = data.image[IDmfcpa].array.F[m];
        mblock = 0;
        while (cpa > CPAblocklim[mblock])
        {
            //    printf("[%ld  %f %f -> +]\n", mblock, cpa, CPAblocklim[mblock]);
            mblock++;
        }

        MBLOCK_NBmode[mblock]++;

        if(mblock>NBmblock)
            NBmblock = mblock;

        //    printf("%ld %f  -> %ld\n", m, cpa, mblock);
    }

    NBmblock++;

    for(mblock=0; mblock<NBmblock; mblock++)
    {
        sprintf(imname, "fmodes_%03ld", mblock);
        MBLOCK_ID[mblock] = create_3Dimage_ID(imname, msize, msize, MBLOCK_NBmode[mblock]);
        MBLOCK_ID[mblock] = image_ID(imname);
    }
    list_image_ID();


    for(mblock=0; mblock<MAX_MBLOCK; mblock++)
        MBLOCK_NBmode[mblock] = 0;

    for(m=0; m<data.image[ID].md[0].size[2]; m++)
    {

        cpa = data.image[IDmfcpa].array.F[m];
        mblock = 0;
        while (cpa > CPAblocklim[mblock])
            mblock++;

        ptr0 = (char*) data.image[ID].array.F;
        ptr0 += m*sizeof(float)*msize*msize;

        ptr1 = (char*) data.image[MBLOCK_ID[mblock]].array.F;
        ptr1 += MBLOCK_NBmode[mblock]*sizeof(float)*msize*msize;

        memcpy(ptr1, ptr0, sizeof(float)*msize*msize);

        MBLOCK_NBmode[mblock]++;
    }


    // remove previous modes from each block
    msize2 = msize*msize;
    for(mblock=1; mblock<NBmblock; mblock++)
    {
        for(m=0; m<MBLOCK_NBmode[mblock]; m++)
        {
            for(mblock0=0; mblock0<mblock; mblock0++)
            {
                for(m0=0; m0<MBLOCK_NBmode[mblock0]; m0++)
                {
                    value = 0.0;
                    value0 = 0.0;

                    for(ii=0; ii<msize2; ii++)
                    {
                        value += data.image[MBLOCK_ID[mblock]].array.F[m*msize2+ii]*data.image[MBLOCK_ID[mblock0]].array.F[m0*msize2+ii];
                        value0 += data.image[MBLOCK_ID[mblock0]].array.F[m0*msize2+ii]*data.image[MBLOCK_ID[mblock0]].array.F[m0*msize2+ii];
                    }
                    value1 = 0.0;
                    for(ii=0; ii<msize2; ii++)
                    {
                        data.image[MBLOCK_ID[mblock]].array.F[m*msize2+ii] -= value/value0*data.image[MBLOCK_ID[mblock0]].array.F[m0*msize2+ii];
                        value1 += data.image[MBLOCK_ID[mblock]].array.F[m*msize2+ii]*data.image[MBLOCK_ID[mblock]].array.F[m*msize2+ii];
                    }
                    rms = sqrt(value1/totm);
                    for(ii=0; ii<msize2; ii++)
                        data.image[MBLOCK_ID[mblock]].array.F[m*msize2+ii] /= rms;
                }
            }
        }
    }

    sprintf(command, "echo \"%ld\" > ./tmp/fmodes_nb.txt", NBmblock);
    ret = system(command);
    
    ret = system("rm ./tmp/fmodes_*.fits");
    for(mblock=0; mblock<NBmblock; mblock++)
    {
        sprintf(imname, "fmodes_%03ld", mblock);
        sprintf(fname, "!./tmp/fmodes_%03ld.fits", mblock);
        save_fits(imname, fname);
    }

    return(ID);
}







//
// every time im_name changes (counter increments), crop it to out_name in shared memory
//
int AOloopControl_camimage_extract2D_sharedmem_loop(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart)
{
    long iiin,jjin, iiout, jjout;
    long IDin, IDout;
    int atype;
    long *sizeout;
    long long cnt0;
    long IDmask;
    long sizeoutxy;
    long ii;


    sizeout = (long*) malloc(sizeof(long)*2);
    sizeout[0] = size_x;
    sizeout[1] = size_y;
    sizeoutxy = size_x*size_y;

    IDin = image_ID(in_name);
    atype = data.image[IDin].md[0].atype;

    // Create shared memory output image
    IDout = create_image_ID(out_name, 2, sizeout, atype, 1, 0);

    // Check if there is a mask
    IDmask = image_ID("csmask");
    if(IDmask!=-1)
        if((data.image[IDmask].md[0].size[0]!=size_x)||(data.image[IDmask].md[0].size[1]!=size_y))
        {
            printf("ERROR: csmask has wrong size\n");
            exit(0);
        }


    cnt0 = -1;

    switch (atype) {
    case USHORT :
        while(1)
        {
            usleep(10); // OK FOR NOW (NOT USED BY FAST WFS)
            if(data.image[IDin].md[0].cnt0!=cnt0)
            {
                data.image[IDout].md[0].write = 1;
                cnt0 = data.image[IDin].md[0].cnt0;
                for(iiout=0; iiout<size_x; iiout++)
                    for(jjout=0; jjout<size_y; jjout++)
                    {
                        iiin = xstart + iiout;
                        jjin = ystart + jjout;
                        data.image[IDout].array.U[jjout*size_x+iiout] = data.image[IDin].array.U[jjin*data.image[IDin].md[0].size[0]+iiin];
                    }
                if(IDmask!=-1)
                    for(ii=0; ii<sizeoutxy; ii++)
                        data.image[IDout].array.U[ii] *= (int) data.image[IDmask].array.F[ii];

                data.image[IDout].md[0].cnt0 = cnt0;
                data.image[IDout].md[0].write = 0;
            }
        }
        break;
    case FLOAT :
        while(1)
        {
            usleep(50); // OK FOR NOW (NOT USED BY FAST WFS)
            if(data.image[IDin].md[0].cnt0!=cnt0)
            {
                data.image[IDout].md[0].write = 1;
                cnt0 = data.image[IDin].md[0].cnt0;
                for(iiout=0; iiout<size_x; iiout++)
                    for(jjout=0; jjout<size_y; jjout++)
                    {
                        iiin = xstart + iiout;
                        jjin = ystart + jjout;
                        data.image[IDout].array.F[jjout*size_x+iiout] = data.image[IDin].array.F[jjin*data.image[IDin].md[0].size[0]+iiin];
                    }
                if(IDmask!=-1)
                    for(ii=0; ii<sizeoutxy; ii++)
                        data.image[IDout].array.F[ii] *= data.image[IDmask].array.F[ii];
                data.image[IDout].md[0].cnt0 = cnt0;
                data.image[IDout].md[0].write = 0;
            }
        }
        break;
    default :
        printf("ERROR: DATA TYPE NOT SUPPORTED\n");
        exit(0);
        break;
    }
    free(sizeout);

    return(0);
}






/** \brief Computes control matrix using SVD
 *
 *        Conventions:
 * 				m: number of actuators (= NB_MODES);
 * 				n: number of sensors  (= # of pixels)
 *	works even for m != n
 *
 *
 *
 */

int compute_ControlMatrix(long loop, long NB_MODE_REMOVED, char *ID_Rmatrix_name, char *ID_Cmatrix_name, char *ID_VTmatrix_name, double Beta, long NB_MODE_REMOVED_STEP, float eigenvlim)
{
    FILE *fp;
    long ID;
    long ii1, jj1, k, ii;
    gsl_matrix *matrix_D; /* this is the response matrix */
    gsl_matrix *matrix_Ds; /* this is the pseudo inverse of D */
    gsl_matrix *matrix_Dtra;
    gsl_matrix *matrix_DtraD;
    gsl_matrix *matrix_DtraDinv;
    gsl_matrix *matrix_DtraD_evec;
    gsl_matrix *matrix1;
    gsl_matrix *matrix2;
    gsl_vector *matrix_DtraD_eval;
    gsl_eigen_symmv_workspace *w;

    gsl_matrix *matrix_save;

    long m;
    long n;
    long ID_Rmatrix, ID_Cmatrix, ID_VTmatrix;
    long *arraysizetmp;

    long IDmodes, IDeigenmodes;
    long xsize_modes, ysize_modes, zsize_modes;
    long IDeigenmodesResp;
    long kk, kk1;
    long ID_RMmask;

    double *CPAcoeff; /// gain applied to modes to enhance low orders in SVD

    char fname[200];
    long NB_MR;  /// number of modes removed

    long NB_MODE_REMOVED1;
    float eigenvmin=0.0;
    long NBMODES_REMOVED_EIGENVLIM = 0;


    long MB_MR_start;
    long MB_MR_end;
    long MB_MR_step;

    int ret;
    char command[200];


    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);


    arraysizetmp = (long*) malloc(sizeof(long)*3);


    ID_Rmatrix = image_ID(ID_Rmatrix_name);


    n = data.image[ID_Rmatrix].md[0].size[0]*data.image[ID_Rmatrix].md[0].size[1]; //AOconf[loop].NBDMmodes;
    m = data.image[ID_Rmatrix].md[0].size[2]; //AOconf[loop].sizeWFS;


    ID_RMmask = image_ID("RMmask");
    if(ID_RMmask!=-1) // apply mask to response matrix
    {
        for(kk=0; kk<m; kk++)
        {
            for(ii=0; ii<n; ii++)
                data.image[ID_Rmatrix].array.F[kk*n+ii] *= data.image[ID_RMmask].array.F[ii];
        }
    }



    /** in this procedure, m=number of actuators/modes, n=number of WFS elements */
    //  long m = smao[0].NBmode;
    // long n = smao[0].NBwfselem;

    printf("m = %ld actuators (modes), n = %ld sensors\n", m, n);
    fflush(stdout);

    NB_MODE_REMOVED1 = m-1;

    matrix_DtraD_eval = gsl_vector_alloc (m);
    matrix_D = gsl_matrix_alloc (n,m);
    matrix_Ds = gsl_matrix_alloc (m,n);
    matrix_Dtra = gsl_matrix_alloc (m,n);
    matrix_DtraD = gsl_matrix_alloc (m,m);
    matrix_DtraDinv = gsl_matrix_alloc (m,m);
    matrix_DtraD_evec = gsl_matrix_alloc (m,m);


    CPAcoeff = (double*) malloc(sizeof(double)*m);
    
     if(Beta>0.000001)
    {
        ID = load_fits("modesfreqcpa.fits", "modesfreqcpa", 1);
        if(ID==-1)
        {
            for(k=0; k<m; k++)
                CPAcoeff[k] = 1.0;
        }
        else
        {
            for(k=0; k<m; k++)
            {
                CPAcoeff[k] =  exp(-data.image[ID].array.F[k]*Beta);
                printf("%5ld %5.3f %g\n", k, data.image[ID].array.F[k], CPAcoeff[k]);
            }
        }
    }
    else 
    {
        for(k=0; k<m; k++)
            CPAcoeff[k] = 1.0;
    }


    /* write matrix_D */
    for(k=0; k<m; k++)
    {
        for(ii=0; ii<n; ii++)
            gsl_matrix_set (matrix_D, ii, k, data.image[ID_Rmatrix].array.F[k*n+ii]*CPAcoeff[k]);
    }
    /* compute DtraD */
    gsl_blas_dgemm (CblasTrans, CblasNoTrans, 1.0, matrix_D, matrix_D, 0.0, matrix_DtraD);


    /* compute the inverse of DtraD */

    /* first, compute the eigenvalues and eigenvectors */
    w =   gsl_eigen_symmv_alloc (m);
    matrix_save = gsl_matrix_alloc (m,m);
    gsl_matrix_memcpy(matrix_save, matrix_DtraD);
    gsl_eigen_symmv (matrix_save, matrix_DtraD_eval, matrix_DtraD_evec, w);
    gsl_matrix_free(matrix_save);
    gsl_eigen_symmv_free(w);
    gsl_eigen_symmv_sort (matrix_DtraD_eval, matrix_DtraD_evec, GSL_EIGEN_SORT_ABS_DESC);

    printf("Eigenvalues\n");
    fflush(stdout);

    // Write eigenvalues
    if((fp=fopen("eigenv.dat","w"))==NULL)
    {
        printf("ERROR: cannot create file \"eigenv.dat\"\n");
        exit(0);
    }
    for(k=0; k<m; k++)
        fprintf(fp,"%ld %g\n", k, gsl_vector_get(matrix_DtraD_eval,k));
    fclose(fp);

    eigenvmin = eigenvlim*gsl_vector_get(matrix_DtraD_eval,0);

    NBMODES_REMOVED_EIGENVLIM = 0;
    for(k=0; k<m; k++)
    {
        printf("Mode %ld eigenvalue = %g\n", k, gsl_vector_get(matrix_DtraD_eval,k));
        if(gsl_vector_get(matrix_DtraD_eval,k) < eigenvmin)
            NBMODES_REMOVED_EIGENVLIM++;
    }




    /** Write rotation matrix to go from DM modes to eigenmodes */
    arraysizetmp[0] = m;
    arraysizetmp[1] = m;
    ID_VTmatrix = create_image_ID(ID_VTmatrix_name, 2, arraysizetmp, FLOAT, 0, 0);
    for(ii=0; ii<m; ii++) // modes
        for(k=0; k<m; k++) // modes
            data.image[ID_VTmatrix].array.F[k*m+ii] = (float) gsl_matrix_get( matrix_DtraD_evec, k, ii);


    /// Compute eigenmodes responses
    IDeigenmodesResp = create_3Dimage_ID("eigenmodesrespM", data.image[ID_Rmatrix].md[0].size[0], data.image[ID_Rmatrix].md[0].size[1], data.image[ID_Rmatrix].md[0].size[2]);
    printf("Computing eigenmode responses .... \n");
    for(kk=0; kk<m; kk++) /// eigen mode index
    {
        printf("\r eigenmode %4ld / %4ld   ", kk, m);
        fflush(stdout);
        for(kk1=0; kk1<m; kk1++)
        {
            for(ii=0; ii<n; ii++)
                data.image[IDeigenmodesResp].array.F[kk*n + ii] += data.image[ID_VTmatrix].array.F[kk1*m+kk]*data.image[ID_Rmatrix].array.F[kk1*n + ii];
        }
    }
    sprintf(fname, "!eigenmodesrespM_%4.2f.fits", Beta);
    save_fits("eigenmodesrespM", fname);
    printf("\n");


    /// if modesM exists, compute eigenmodes using rotation matrix
    IDmodes = image_ID("modesM");
    if(IDmodes!=-1)
    {
        xsize_modes = data.image[IDmodes].md[0].size[0];
        ysize_modes = data.image[IDmodes].md[0].size[1];
        zsize_modes = data.image[IDmodes].md[0].size[2];
        if(zsize_modes != m)
            printf("ERROR: zsize (%ld) of modesM does not match expected size (%ld)\n", zsize_modes, m);
        else
        {
            IDeigenmodes = create_3Dimage_ID("eigenmodesM", xsize_modes, ysize_modes, m);
            //	list_image_ID();
            printf("Computing eigenmodes .... \n");
            for(kk=0; kk<m; kk++) /// eigen mode index
            {
                printf("\r eigenmode %4ld / %4ld   ", kk, m);
                fflush(stdout);
                for(kk1=0; kk1<m; kk1++)
                {
                    for(ii=0; ii<xsize_modes*ysize_modes; ii++)
                        data.image[IDeigenmodes].array.F[kk*xsize_modes*ysize_modes + ii] += data.image[ID_VTmatrix].array.F[kk1*m+kk]*data.image[IDmodes].array.F[kk1*xsize_modes*ysize_modes + ii];
                }
            }
            printf("\n");
        }
        sprintf(fname, "!eigenmodesM_%4.2f.fits", Beta);
        save_fits("eigenmodesM", fname);
    }




    /// second, build the "inverse" of the diagonal matrix of eigenvalues (matrix1)
    matrix1 = gsl_matrix_alloc (m, m);
    matrix2 = gsl_matrix_alloc (m, m);
    arraysizetmp[0] = AOconf[loop].sizexWFS;
    arraysizetmp[1] = AOconf[loop].sizeyWFS;
    arraysizetmp[2] = m;
    ID_Cmatrix = create_image_ID(ID_Cmatrix_name, 3, arraysizetmp, FLOAT, 0, 0);

    printf("COMPUTING CMAT .... \n");


    if(NB_MODE_REMOVED_STEP==0)
    {
        MB_MR_start = NBMODES_REMOVED_EIGENVLIM;
        MB_MR_end = NBMODES_REMOVED_EIGENVLIM+1;
        MB_MR_step = 10;
    }
    else
    {
        MB_MR_start = 0;
        MB_MR_end = NB_MODE_REMOVED1;
        MB_MR_step = NB_MODE_REMOVED_STEP;
    }

    for(NB_MR=MB_MR_start; NB_MR<MB_MR_end; NB_MR+=MB_MR_step)
    {
        printf("\r Number of modes removed : %5ld / %5ld  (step %ld)  ", NB_MR, NB_MODE_REMOVED1, NB_MODE_REMOVED_STEP);
        fflush(stdout);
        for(ii1=0; ii1<m; ii1++)
            for(jj1=0; jj1<m; jj1++)
            {
                if(ii1==jj1)
                {
                    if((m-ii1-1)<NB_MR)
                        gsl_matrix_set(matrix1, ii1, jj1, 0.0);
                    else
                        gsl_matrix_set(matrix1, ii1, jj1, 1.0/gsl_vector_get(matrix_DtraD_eval,ii1));
                }
                else
                    gsl_matrix_set(matrix1, ii1, jj1, 0.0);
            }


        printf("-");
        fflush(stdout);

        /* third, compute the "inverse" of DtraD */
        gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, matrix_DtraD_evec, matrix1, 0.0, matrix2);
        printf("-");
        fflush(stdout);
        gsl_blas_dgemm (CblasNoTrans, CblasTrans, 1.0, matrix2, matrix_DtraD_evec, 0.0, matrix_DtraDinv);
        printf("-");
        fflush(stdout);
        gsl_blas_dgemm (CblasNoTrans, CblasTrans, 1.0, matrix_DtraDinv, matrix_D, 0.0, matrix_Ds);

        /* write result */
        printf("write result to ID %ld   [%ld %ld]\n", ID_Cmatrix, n, m);
        fflush(stdout);
        list_image_ID();

        for(ii=0; ii<n; ii++) // sensors
            for(k=0; k<m; k++) // actuator modes
                data.image[ID_Cmatrix].array.F[k*n+ii] = (float) gsl_matrix_get(matrix_Ds, k, ii)*CPAcoeff[k];


        if(NB_MODE_REMOVED_STEP==0)
        {
            save_fits(ID_Cmatrix_name, "!cmat.fits");
            sprintf(command, "echo \"%ld\" > ./cmat.NB_MODES_RM.txt", NBMODES_REMOVED_EIGENVLIM);
            ret = system(command);
            sprintf(command, "echo \"%ld\" > ./cmat.NB_MODES.txt",  m);
            ret = system(command);
       }
        else
        {
            sprintf(fname, "!cmat_%4.2f_%03ld.fits", Beta, NB_MR);
            printf("  SAVING -> %s\n", fname);
            fflush(stdout);
            save_fits(ID_Cmatrix_name, fname);
        }
    }

    printf("\n\n");

    gsl_matrix_free(matrix1);
    gsl_matrix_free(matrix2);

    gsl_vector_free(matrix_DtraD_eval);
    gsl_matrix_free(matrix_D);
    gsl_matrix_free(matrix_Ds);
    gsl_matrix_free(matrix_Dtra);
    gsl_matrix_free(matrix_DtraD);
    gsl_matrix_free(matrix_DtraDinv);
    gsl_matrix_free(matrix_DtraD_evec);

    free(arraysizetmp);

    free(CPAcoeff);


    return(ID_Cmatrix);
}


















/*** mode = 0 or 1. if mode == 1, simply connect */

int AOloopControl_InitializeMemory(int mode)
{
    int SM_fd;
    struct stat file_stat;
    int create = 0;
    int result;
    long loop;
    long *sizearray;
    char cntname[200];

    SM_fd = open(AOconfname, O_RDWR);
    if(SM_fd==-1)
    {
        printf("Cannot import file -> creating file\n");
        create = 1;
    }
    else
    {
        fstat(SM_fd, &file_stat);
        printf("File %s size: %zd\n", AOconfname, file_stat.st_size);
        if(file_stat.st_size!=sizeof(AOLOOPCONTROL_CONF)*NB_AOloopcontrol)
        {
            printf("File size wrong -> recreating file\n");
            create = 1;
            close(SM_fd);
        }
    }

    if(create==1)
    {
        SM_fd = open(AOconfname, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

        if (SM_fd == -1) {
            perror("Error opening file for writing");
            exit(0);
        }

        result = lseek(SM_fd, sizeof(AOLOOPCONTROL_CONF)*NB_AOloopcontrol-1, SEEK_SET);
        if (result == -1) {
            close(SM_fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(0);
        }

        result = write(SM_fd, "", 1);
        if (result != 1) {
            close(SM_fd);
            perror("Error writing last byte of the file");
            exit(0);
        }
    }

    AOconf = (AOLOOPCONTROL_CONF*) mmap(0, sizeof(AOLOOPCONTROL_CONF)*NB_AOloopcontrol, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
    if (AOconf == MAP_FAILED) {
        close(SM_fd);
        perror("Error mmapping the file");
        exit(0);
    }

    if((mode==0)||(create==1))
    {
        AOconf[loop].on = 0;
        AOconf[loop].cnt = 0;
        AOconf[loop].cntmax = 0;
		AOconf[loop].init_CMc = 0;
        sprintf(cntname, "aol%ld_logdata", loop); // contains loop count (cnt0) and loop gain
        if((AOconf[loop].logdataID = image_ID(cntname))==-1)
        {
            sizearray = (long*) malloc(sizeof(long)*2);
            sizearray[0] = 1;
            sizearray[1] = 1;
            AOconf[loop].logdataID = create_image_ID(cntname, 2, sizearray, FLOAT, 1, 0);
            free(sizearray);
        }
    }

    if(create==1)
    {
        for(loop=0; loop<NB_AOloopcontrol; loop++)
        {
            AOconf[loop].init = 0;
            AOconf[loop].on = 0;
            AOconf[loop].cnt = 0;
            AOconf[loop].cntmax = 0;
            AOconf[loop].maxlimit = 0.3;
            AOconf[loop].mult = 1.00;
            AOconf[loop].gain = 0.0;
            AOconf[loop].WFSnormfloor = 0.0;
            AOconf[loop].framesAve = 1;
            AOconf[loop].NBMblocks = 3;
            AOconf[loop].GPUusesem = 1;
        }
    }
    else
    {
        for(loop=0; loop<NB_AOloopcontrol; loop++)
            if(AOconf[loop].init == 1)
            {
                printf("LIST OF ACTIVE LOOPS:\n");
                printf("----- Loop %ld   (%s) ----------\n", loop, AOconf[loop].name);
                printf("  WFS:  %s  [%ld]  %ld x %ld\n", AOconf[loop].WFSname, aoconfID_WFS, AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
                printf("   DM:  %s  [%ld]  %ld x %ld\n", AOconf[loop].DMname, aoconfID_DM, AOconf[loop].sizexDM, AOconf[loop].sizeyDM);
                printf("DM RM:  %s  [%ld]  %ld x %ld\n", AOconf[loop].DMnameRM, aoconfID_DM, AOconf[loop].sizexDM, AOconf[loop].sizeyDM);
            }
    }

    AOloopcontrol_meminit = 1;


    return 0;
}


void *compute_function_imtotal( void *ptr )
{
    long ii;
    long nelem;

    nelem = data.image[aoconfID_WFS0].md[0].size[0]*data.image[aoconfID_WFS0].md[0].size[1];

    while(1)
    {
        sem_wait(&AOLCOMPUTE_TOTAL_ASYNC_sem_name);
        IMTOTAL = 0.0;
        for(ii=0; ii<nelem; ii++)
            IMTOTAL += data.image[aoconfID_WFS0].array.F[ii];
    }
}


void *compute_function_dark_subtract( void *ptr )
{
    long ii, iistart, iiend;
    long nelem;
    long *index;
    int sval;
    long threadindex;

    nelem = data.image[aoconfID_WFS0].md[0].size[0]*data.image[aoconfID_WFS0].md[0].size[1];
    index = (long*) ptr;
    threadindex = *index;

    iistart = (long) ((threadindex)*nelem/COMPUTE_DARK_SUBTRACT_NBTHREADS);
    iiend = (long) ((threadindex+1)*nelem/COMPUTE_DARK_SUBTRACT_NBTHREADS);

    while(1)
    {
        sem_wait(&AOLCOMPUTE_DARK_SUBTRACT_sem_name[threadindex]);

        switch ( WFSatype ) {
        case USHORT :
            for(ii=iistart; ii<iiend; ii++)
                data.image[aoconfID_WFS0].array.F[ii] = ((float) arrayutmp[ii]) - data.image[Average_cam_frames_IDdark].array.F[ii];
            break;
        case FLOAT :
            for(ii=iistart; ii<iiend; ii++)
                data.image[aoconfID_WFS0].array.F[ii] = ((float) arrayftmp[ii]) - data.image[Average_cam_frames_IDdark].array.F[ii];
            break;
        default :
            printf("ERROR: WFS data type not recognized\n");
            exit(0);
            break;
        }

        sem_post(&AOLCOMPUTE_DARK_SUBTRACT_RESULT_sem_name[threadindex]);
    }
}








/** Read image from WFS camera
 *
 * supports ring buffer
 * puts image from camera buffer aoconfID_WFS into aoconfID_WFS1 (supplied by user)
 *
 * RM = 1 if response matrix
 *
 * image is normalized by dividing by (total + AOconf[loop].WFSnormfloor)
 */

int Average_cam_frames(long loop, long NbAve, int RM)
{
    long imcnt;
    long ii;
    double totalinv;
    char name[200];
    long slice;
    char *ptrv;
    long double tmplv1;
    double tmpf;
    long IDdark;
    char dname[200];
    long nelem;
    pthread_t thread_computetotal_id;
    pthread_t thread_dark_subtract[20];
    float resulttotal;
    int sval0, sval;
    void *status = 0;


    WFSatype = data.image[aoconfID_WFS].md[0].atype;



    if(avcamarraysInit==0)
    {
        arrayftmp = (float*) malloc(sizeof(float)*AOconf[loop].sizeWFS);
        arrayutmp = (unsigned short*) malloc(sizeof(unsigned short)*AOconf[loop].sizeWFS);

        sprintf(Average_cam_frames_dname, "aol%ld_wfsdark", loop);
        Average_cam_frames_IDdark = image_ID(Average_cam_frames_dname);
        Average_cam_frames_nelem = AOconf[loop].sizeWFS;

        avcamarraysInit = 1;
    }

    if(NbAve>1)
        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[aoconfID_WFS0].array.F[ii] = 0.0;

    if(RM==0)
        AOconf[loop].status = 2;  // 2: WAIT FOR IMAGE
    else
        data.status1 = 2;


    if(data.image[aoconfID_WFS].md[0].naxis==2) // single buffer
    {
        switch (WFSatype) {
        case FLOAT :
            imcnt = 0;
            while(imcnt<NbAve)
            {
                usleep(50); // OK FOR NOW (not using single buffer in fast WFS)
                if(data.image[aoconfID_WFS].md[0].write == 0)
                {
                    if(AOconf[loop].WFScnt!=data.image[aoconfID_WFS].md[0].cnt0)
                    {
                        AOconf[loop].WFScnt = data.image[aoconfID_WFS].md[0].cnt0;
                        memcpy (arrayftmp, data.image[aoconfID_WFS].array.F, sizeof(float)*AOconf[loop].sizeWFS);
                        if(NbAve>1)
                        {

                            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                                data.image[aoconfID_WFS0].array.F[ii] += arrayftmp[ii];
                        }
                        else
                            memcpy(data.image[aoconfID_WFS0].array.F, arrayftmp,  sizeof(float)*AOconf[loop].sizeWFS);
                        imcnt++;
                    }
                }
            }
            break;
        case USHORT :
            imcnt = 0;
            while(imcnt<NbAve)
            {
                usleep(50); // OK FOR NOW (not using single buffer in fast WFS)
                if(data.image[aoconfID_WFS].md[0].write == 0)
                {
                    if(AOconf[loop].WFScnt!=data.image[aoconfID_WFS].md[0].cnt0)
                    {
                        AOconf[loop].WFScnt = data.image[aoconfID_WFS].md[0].cnt0;
                        memcpy (arrayutmp, data.image[aoconfID_WFS].array.U, sizeof(unsigned short)*AOconf[loop].sizeWFS);
                        if(NbAve>1)
                        {
                            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                                data.image[aoconfID_WFS0].array.F[ii] += arrayutmp[ii];
                        }
                        else
                        {
                            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                                data.image[aoconfID_WFS0].array.F[ii] = arrayutmp[ii];
                        }
                        imcnt++;
                    }
                }
            }
            break;
        default :
            printf("ERROR: DATA TYPE NOT SUPPORTED\n");
            exit(0);
            break;
        }
    }
    else // ring buffer mode, only works with NbAve = 1
    {
   //    printf("RING BUFFER\n");
   //     fflush(stdout);
        if(data.image[aoconfID_WFS].sem==0)
        {
            if(RM==0)
            {
                while(AOconf[loop].WFScnt==data.image[aoconfID_WFS].md[0].cnt0) // test if new frame exists
                {
                    usleep(5);
                    // do nothing, wait
                }
            }
            else
            {
                while(AOconf[loop].WFScntRM==data.image[aoconfID_WFS].md[0].cnt0) // test if new frame exists
                {
                    usleep(5);
                    // do nothing, wait
                }
            }
        }
        else
        {
            printf("Waiting for semaphore to post .... ");
            fflush(stdout);
            sem_wait(data.image[aoconfID_WFS].semptr);
            printf(" done\n");
            fflush(stdout);
        }

        slice = data.image[aoconfID_WFS].md[0].cnt1;
        if(slice==-1)
            slice = data.image[aoconfID_WFS].md[0].size[2];

        switch (WFSatype) {
        case FLOAT :
            ptrv = (char*) data.image[aoconfID_WFS].array.F;
            ptrv += sizeof(float)*slice* AOconf[loop].sizeWFS;
            memcpy(arrayftmp, ptrv,  sizeof(float)*AOconf[loop].sizeWFS);
            break;
        case USHORT :
            ptrv = (char*) data.image[aoconfID_WFS].array.U;
            ptrv += sizeof(unsigned short)*slice* AOconf[loop].sizeWFS;
            memcpy (arrayutmp, ptrv, sizeof(unsigned short)*AOconf[loop].sizeWFS);
            break;
        default :
            printf("ERROR: DATA TYPE NOT SUPPORTED\n");
            exit(0);
            break;
        }
        if(RM==0)
            AOconf[loop].WFScnt = data.image[aoconfID_WFS].md[0].cnt0;
        else
            AOconf[loop].WFScntRM = data.image[aoconfID_WFS].md[0].cnt0;
    }
    AOconf[loop].status = 3;  // 3: DARK SUBTRACT


    // Dark subtract and compute total
    //sprintf(dname, "aol%ld_wfsdark", loop);
    //IDdark = image_ID(dname);
    //nelem = AOconf[loop].sizeWFS;

    if((loop==0)||(RMACQUISITION == 1)) // single thread, in CPU
    {
        switch ( WFSatype ) {
            case USHORT :
# ifdef _OPENMP
        #pragma omp parallel num_threads(8) if (Average_cam_frames_nelem>OMP_NELEMENT_LIMIT)
        {
# endif

# ifdef _OPENMP
            #pragma omp for
# endif
            for(ii=0; ii<Average_cam_frames_nelem; ii++)
                data.image[aoconfID_WFS0].array.F[ii] = ((float) arrayutmp[ii]) - data.image[Average_cam_frames_IDdark].array.F[ii];
# ifdef _OPENMP
        }
# endif
            break;
            case FLOAT :
# ifdef _OPENMP
        #pragma omp parallel num_threads(8) if (Average_cam_frames_nelem>OMP_NELEMENT_LIMIT)
        {
# endif

# ifdef _OPENMP
            #pragma omp for
# endif
            for(ii=0; ii<Average_cam_frames_nelem; ii++)
                data.image[aoconfID_WFS0].array.F[ii] = arrayftmp[ii] - data.image[Average_cam_frames_IDdark].array.F[ii];
# ifdef _OPENMP
        }
# endif
            break;
            default :
                printf("ERROR: WFS data type not recognized\n");
                exit(0);
                break;
        }
            
    }
    else
    {
        if(AOLCOMPUTE_DARK_SUBTRACT_THREADinit==0)
        {
            ti = 0;

            while(ti<COMPUTE_DARK_SUBTRACT_NBTHREADS)
            {
                pthread_create( &thread_dark_subtract[ti], NULL, compute_function_dark_subtract, (void*) &ti);
                sem_init(&AOLCOMPUTE_DARK_SUBTRACT_sem_name[ti], 0, 0);
                sem_init(&AOLCOMPUTE_DARK_SUBTRACT_RESULT_sem_name[ti], 0, 0); 
                usleep(100);
                ti++;   
           }
//            sem_init(&AOLCOMPUTE_DARK_SUBTRACT_sem_name, 0, 0);
  //          sem_init(&AOLCOMPUTE_DARK_SUBTRACT_RESULT_sem_name, 0, 0);             
            AOLCOMPUTE_DARK_SUBTRACT_THREADinit = 1;
        }


        for(ti=0; ti<COMPUTE_DARK_SUBTRACT_NBTHREADS; ti++)
            {
                sem_getvalue(&AOLCOMPUTE_DARK_SUBTRACT_sem_name[ti], &sval0);
                sem_post(&AOLCOMPUTE_DARK_SUBTRACT_sem_name[ti]);
                sem_getvalue(&AOLCOMPUTE_DARK_SUBTRACT_sem_name[ti], &sval);
//                printf("[%ld] 00 posting thread    %d -> %d\n", ti, sval0, sval);
//              printf("[%ld] waiting on result thread\n", ti);
                sem_wait(&AOLCOMPUTE_DARK_SUBTRACT_RESULT_sem_name[ti]);
            }
    }


    //  if(IDdark!=-1)
    // {
    //    for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
    //       data.image[aoconfID_WFS0].array.F[ii] -= data.image[IDdark].array.F[ii];
    //}

    AOconf[loop].status = 4; // 4: COMPUTE TOTAL OF IMAGE


    // Normalize

    if((AOconf[loop].AOLCOMPUTE_TOTAL_ASYNC==0)||(AOLCOMPUTE_TOTAL_INIT==0)||(RMACQUISITION == 1)) // do it in main thread
    {
        AOconf[loop].WFStotalflux = arith_image_total(data.image[aoconfID_WFS0].md[0].name);
        AOLCOMPUTE_TOTAL_INIT = 1;
        IMTOTAL = AOconf[loop].WFStotalflux;
    }
    else  // do it in other threads
    {
        AOconf[loop].WFStotalflux = IMTOTAL; // from last loop
        if(AOLCOMPUTE_TOTAL_ASYNC_THREADinit==0)
        {
            pthread_create( &thread_computetotal_id, NULL, compute_function_imtotal, NULL);
            AOLCOMPUTE_TOTAL_ASYNC_THREADinit = 1;
            sem_init(&AOLCOMPUTE_TOTAL_ASYNC_sem_name, 0, 0);
        }
        sem_post(&AOLCOMPUTE_TOTAL_ASYNC_sem_name);
    }


    AOconf[loop].status = 5;  // 5: NORMALIZE WFS IMAGE

    data.image[aoconfID_WFS0].md[0].cnt0 ++;

    nelem = AOconf[loop].sizeWFS;

    totalinv=1.0/(AOconf[loop].WFStotalflux + AOconf[loop].WFSnormfloor*AOconf[loop].sizeWFS);
    GPU_alpha = totalinv;
    
    normfloorcoeff = AOconf[loop].WFStotalflux/(AOconf[loop].WFStotalflux+AOconf[loop].WFSnormfloor*AOconf[loop].sizeWFS);
    GPU_beta = -normfloorcoeff;

    
    //GPU_beta = -1.0; // test

 //   printf("----------- alpha = %g     beta = %g\n", GPU_alpha, GPU_beta);
  //  fflush(stdout);

 
 
    if(COMPUTE_GPU_SCALING==0)  // normalize WFS image by totalinv
    {
    data.image[aoconfID_WFS1].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel num_threads(8) if (nelem>OMP_NELEMENT_LIMIT)
        {
# endif

# ifdef _OPENMP
            #pragma omp for
# endif
            for(ii=0; ii<nelem; ii++)
                data.image[aoconfID_WFS1].array.F[ii] = data.image[aoconfID_WFS0].array.F[ii]*totalinv;
# ifdef _OPENMP
        }
# endif
        data.image[aoconfID_WFS1].md[0].cnt0 ++;
        data.image[aoconfID_WFS1].md[0].write = 0;
    }


    return(0);
}




















long AOloopControl_MakeDMModes(long loop, long NBmodes, char *IDname)
{
    long ID;
    long IDtmp;
    long ii, jj;
    double x, y;
    long size = AOconf[loop].sizexDM;
    long m;
    float rpix;
    long size2;

    size2 = size*size;
    rpix = 0.5*size;

    ID = create_3Dimage_ID_float(IDname, size, size, NBmodes);

    for(m=0; m<NBmodes; m++)
    {
        IDtmp = mk_zer("zertmp", size, m+1, rpix);
        for(ii=0; ii<size2; ii++)
            data.image[ID].array.F[size2*m+ii] = data.image[IDtmp].array.F[ii];
        delete_image_ID("zertmp");
    }

    return(ID);
}





long AOloopControl_loadCM(long loop, char *CMfname)
{
    long ID = -1;
    int vOK;
    char name[200];
    long ID0;
    long ii;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(0);

    if( (ID = load_fits(CMfname, "tmpcontrM", 1)) != -1 )
    {

        // check size is OK
        vOK = 1;
        if(data.image[ID].md[0].naxis!=3)
        {
            printf("Control matrix has wrong dimension\n");
            vOK = 0;
        }
        if(data.image[ID].md[0].atype!=FLOAT)
        {
            printf("Control matrix has wrong type\n");
            vOK = 0;
        }
        if(vOK==1)
        {
            if(data.image[ID].md[0].size[0]!=AOconf[loop].sizexWFS)
            {
                printf("Control matrix has wrong x size : is %ld, should be %ld\n", data.image[ID].md[0].size[0], AOconf[loop].sizexWFS);
                vOK = 0;
            }
            if(data.image[ID].md[0].size[1]!=AOconf[loop].sizeyWFS)
            {
                printf("Control matrix has wrong y size\n");
                vOK = 0;
            }
            if(data.image[ID].md[0].size[2]!=AOconf[loop].NBDMmodes)
            {
                printf("Control matrix has wrong z size\n");
                vOK = 0;
            }
        }


        if(vOK==1)
        {
            AOconf[loop].init_CM = 1;
            sprintf(name, "ContrM_%ld", loop);
            ID = image_ID(name);
            if(ID==-1)
                ID = read_sharedmem_image(name);
            ID0 = image_ID("tmpcontrM");
            data.image[ID].md[0].write  = 1;
            for(ii=0; ii<AOconf[loop].sizexWFS*AOconf[loop].sizeyWFS*AOconf[loop].NBDMmodes; ii++)
                data.image[ID].array.F[ii] = data.image[ID0].array.F[ii];
            data.image[ID].md[0].write  = 0;
            data.image[ID].md[0].cnt0++;
        }
        delete_image_ID("tmpcontrM");
    }

    return(ID);
}






long AOloopControl_2Dloadcreate_shmim(char *name, char *fname, long xsize, long ysize)
{
    long ID;
    int CreateSMim;
    int sizeOK;
    long *sizearray;
    char command[500];
    int r;
    long ID1;


    ID = image_ID(name);
    sizearray = (long*) malloc(sizeof(long)*2);

    if(ID==-1)
    {
        CreateSMim = 0;
        ID = read_sharedmem_image(name);
        if(ID!=-1)
        {
            sizeOK = COREMOD_MEMORY_check_2Dsize(name, xsize, ysize);
            if(sizeOK==0)
            {
                printf("\n========== EXISTING %s HAS WRONG SIZE -> CREATING BLANK %s ===========\n\n", name, name);
                delete_image_ID(name);
                sprintf(command, "rm /tmp/%s.im.shm", name);
                r = system(command);
                CreateSMim = 1;
            }
        }
        else
            CreateSMim = 1;

        if(CreateSMim == 1)
        {
            sizearray[0] =  xsize;
            sizearray[1] =  ysize;
            printf("Creating %s   [%ld x %ld]\n", name, sizearray[0], sizearray[1]);
            fflush(stdout);
            ID = create_image_ID(name, 2, sizearray, FLOAT, 1, 0);
        }
    }
    free(sizearray);

    if(ID==-1)
    {
        printf("ERROR: could not load/create %s\n", name);
        exit(0);
    }
    else
    {
        ID1 = load_fits(fname, "tmp2Dim", 1);
        if(ID1!=-1)
        {
            sizeOK = COREMOD_MEMORY_check_2Dsize("tmp2Dim", xsize, ysize);
            if(sizeOK==1)
            {
                memcpy(data.image[ID].array.F, data.image[ID1].array.F, sizeof(float)*xsize*ysize);
                printf("loaded file \"%s\" to shared memory \"%s\"\n", fname, name);
            }
            else
                printf("File \"%s\" has wrong size (should be 2-D %ld x %ld,  is %ld-D %ld x %ld): ignoring\n", fname, xsize, ysize, data.image[ID1].md[0].naxis, data.image[ID1].md[0].size[0], data.image[ID1].md[0].size[1]);
            delete_image_ID("tmp2Dim");
        }
    }

    return ID;
}






long AOloopControl_3Dloadcreate_shmim(char *name, char *fname, long xsize, long ysize, long zsize)
{
    long ID;
    int CreateSMim;
    int sizeOK;
    long *sizearray;
    char command[500];
    int r;
    long ID1;

    ID = image_ID(name);
    sizearray = (long*) malloc(sizeof(long)*3);

    if(ID==-1)
    {
        CreateSMim = 0;
          ID = read_sharedmem_image(name);
       if(ID!=-1)
        {
            sizeOK = COREMOD_MEMORY_check_3Dsize(name, xsize, ysize, zsize);
            if(sizeOK==0)
            {
 //               printf("\n========== EXISTING %s HAS WRONG SIZE -> CREATING BLANK %s ===========\n\n", name, name);
                delete_image_ID(name);
                sprintf(command, "rm /tmp/%s.im.shm", name);
                r = system(command);
                CreateSMim = 1;
            }
        }
        else
            CreateSMim = 1;

        if(CreateSMim == 1)
        {
            sizearray[0] = xsize;
            sizearray[1] = ysize;
            sizearray[2] = zsize;
//            printf("Creating %s   [%ld x %ld x %ld]\n", name, sizearray[0], sizearray[1], sizearray[2]);
//            fflush(stdout);
           ID = create_image_ID(name, 3, sizearray, FLOAT, 1, 0);
        }
    }
    free(sizearray);


    if(ID==-1)
    {
        printf("ERROR: could not load/create %s\n", name);
        exit(0);
   }
    else
    {
        ID1 = load_fits(fname, "tmp3Dim", 1);

       if(ID1!=-1)
        {

            sizeOK = COREMOD_MEMORY_check_3Dsize("tmp3Dim", xsize, ysize, zsize);
            if(sizeOK==1)
            {
                memcpy(data.image[ID].array.F, data.image[ID1].array.F, sizeof(float)*xsize*ysize*zsize);
   //             printf("loaded file \"%s\" to shared memory \"%s\"\n", fname, name);
            }
            else
                printf("File \"%s\" has wrong size (should be 3-D %ld x %ld, x %ld  is %ld-D %ld x %ld x %ld): ignoring\n", fname, xsize, ysize, zsize, data.image[ID1].md[0].naxis, data.image[ID1].md[0].size[0], data.image[ID1].md[0].size[1], data.image[ID1].md[0].size[2]);
            delete_image_ID("tmp3Dim");
        }
    }
    

    return ID;
}





//
// load / setup configuration
// mode = 1 loads from ./conf/ directory to shared memory
// mode = 0 simply connects to shared memory
//
int AOloopControl_loadconfigure(long loop, int mode)
{
    FILE *fp;
    char content[200];
    char name[200];
    char fname[200];
    long ID;
    long *sizearray;
    int vOK;
    int kw;
    long k;
    int r;
    int sizeOK;
    char command[500];
    int CreateSMim;
    long ID1tmp, ID2tmp;
    long ii;


    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(0);



    // printf("mode = %d\n", mode); // not used yet


    // Name definitions for shared memory

    sprintf(name, "aol%ld_dmC", loop);
    printf("DM file name : %s\n", name);
    strcpy(AOconf[loop].DMname, name);

    sprintf(name, "aol%ld_dmRM", loop);
    printf("DM RM file name : %s\n", name);
    strcpy(AOconf[loop].DMnameRM, name);

    sprintf(name, "aol%ld_wfsim", loop);
    printf("WFS file name: %s\n", name);
    strcpy(AOconf[loop].WFSname, name);



    sprintf(name, "aol%ld_DMmodes", loop);
    printf("DMmodes file name: %s\n", name);
    strcpy(AOconf[loop].DMMODESname, name);

    sprintf(name, "aol%ld_RespM", loop);
    printf("respM file name: %s\n", name);
    strcpy(AOconf[loop].respMname, name);

    sprintf(name, "aol%ld_ContrM", loop);
    printf("contrM file name: %s\n", name);
    strcpy(AOconf[loop].contrMname, name);






    sizearray = (long*) malloc(sizeof(long)*3);


    // READ LOOP NAME

    if((fp=fopen("./conf/conf_LOOPNAME.txt","r"))==NULL)
    {
        printf("ERROR: file ./conf/conf_LOOPNAME.txt missing\n");
        exit(0);
    }
    r = fscanf(fp, "%s", content);
    printf("loop name : %s\n", content);
    fclose(fp);
    fflush(stdout);
    strcpy(AOconf[loop].name, content);


    // USE GPUs ?

    if((fp=fopen("./conf/conf_GPU.txt","r"))==NULL)
    {
        printf("WARNING: file ./conf/conf_GPU.txt missing\n");
        printf("Using CPU only\n");
        AOconf[loop].GPU = 0;
    }
    else
    {
        r = fscanf(fp, "%s", content);
        printf("GPU : %d\n", atoi(content));
        fclose(fp);
        fflush(stdout);
        AOconf[loop].GPU = atoi(content);
    }

    // Skip CPU image scaling and go straight to GPUs ?

    if((fp=fopen("./conf/conf_GPUall.txt","r"))==NULL)
    {
        printf("WARNING: file ./conf/conf_GPUall.txt missing\n");
        printf("Using CPU for image scaling\n");
        AOconf[loop].GPUall = 0;
    }
    else
    {
        r = fscanf(fp, "%s", content);
        printf("GPUall : %d\n", atoi(content));
        fclose(fp);
        fflush(stdout);
        AOconf[loop].GPUall = atoi(content);
    }


   // TOTAL image done in separate thread ?
    AOconf[loop].AOLCOMPUTE_TOTAL_ASYNC = 0;
    if((fp=fopen("./conf/conf_COMPUTE_TOTAL_ASYNC.txt","r"))==NULL)
    {
        printf("WARNING: file ./conf/conf_COMPUTE_TOTAL_ASYNC.txt missing\n");
        printf("using default: %d\n", AOconf[loop].AOLCOMPUTE_TOTAL_ASYNC);
    }
    else
    {
        r = fscanf(fp, "%s", content);
        printf("AOLCOMPUTE_TOTAL_ASYNC : %d\n", atoi(content));
        fclose(fp);
        fflush(stdout);
        AOconf[loop].AOLCOMPUTE_TOTAL_ASYNC = atoi(content);
    }


    // CMatrix mult mode
    // 0 : WFS signal -> Mode coeffs -> DM act values  (2 sequential matrix multiplications)
    // 1 : WFS signal -> DM act values  (1 combined matrix multiplication)

    if((fp=fopen("./conf/conf_CMmode.txt","r"))==NULL)
    {
        printf("WARNING: file ./conf/conf_CMmode.txt missing\n");
        printf("Using combined matrix\n");
        MATRIX_COMPUTATION_MODE = 1;  // by default, use combined matrix
    }
    else
    {
        r = fscanf(fp, "%s", content);
        printf("Matrix mult mode : %d\n", atoi(content));
        fclose(fp);
        fflush(stdout);
        MATRIX_COMPUTATION_MODE = atoi(content);
    }


    // this image is read to notify when new dm displacement is ready
    aoconfID_DMdisp = read_sharedmem_image("dmdisp");



    // Connect to WFS camera
    // This is where the size of the WFS is fixed
    aoconfID_WFS = read_sharedmem_image(AOconf[loop].WFSname);
    AOconf[loop].sizexWFS = data.image[aoconfID_WFS].md[0].size[0];
    AOconf[loop].sizeyWFS = data.image[aoconfID_WFS].md[0].size[1];
    AOconf[loop].sizeWFS = AOconf[loop].sizexWFS*AOconf[loop].sizeyWFS;



    // The AOloopControl_xDloadcreate_shmim functions work as follows:
    // If file already loaded, use it (we assume it's already been properly loaded)
    // If not, attempt to read it from shared memory
    // If not available in shared memory, create it in shared memory
    // if "fname" exists, attempt to load it into the shared memory image

    sprintf(name, "aol%ld_wfsdark", loop);
    sprintf(fname, "./conf/dark.fits");
    aoconfID_WFSdark = AOloopControl_2Dloadcreate_shmim(name, fname, AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);


    sprintf(name, "aol%ld_imWFS0", loop);
    aoconfID_WFS0 = AOloopControl_2Dloadcreate_shmim(name, " ", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);

    sprintf(name, "aol%ld_imWFS1", loop);
    aoconfID_WFS1 = AOloopControl_2Dloadcreate_shmim(name, " ", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);

    sprintf(name, "aol%ld_imWFS2", loop);
    aoconfID_WFS2 = AOloopControl_2Dloadcreate_shmim(name, " ", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);

    sprintf(name, "aol%ld_refWFSim", loop);
    sprintf(fname, "./conf/refwfs.fits");
    aoconfID_refWFS = AOloopControl_2Dloadcreate_shmim(name, fname, AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    AOconf[loop].init_refWFS = 1;



    // Connect to DM
    // Here the DM size is fixed
    //


    aoconfID_DM = image_ID(AOconf[loop].DMname);
    if(aoconfID_DM==-1)
    {
        printf("connect to %s\n", AOconf[loop].DMname);
        aoconfID_DM = read_sharedmem_image(AOconf[loop].DMname);
        if(aoconfID_DM==-1)
        {
            printf("ERROR: cannot connect to shared memory %s\n", AOconf[loop].DMname);
            exit(0);
        }
    }
    AOconf[loop].sizexDM = data.image[aoconfID_DM].md[0].size[0];
    AOconf[loop].sizeyDM = data.image[aoconfID_DM].md[0].size[1];
    AOconf[loop].sizeDM = AOconf[loop].sizexDM*AOconf[loop].sizeyDM;



    aoconfID_DMRM = image_ID(AOconf[loop].DMnameRM);
    if(aoconfID_DMRM==-1)
    {
        printf("connect to %s\n", AOconf[loop].DMnameRM);
        aoconfID_DMRM = read_sharedmem_image(AOconf[loop].DMnameRM);
        if(aoconfID_DMRM==-1)
        {
            printf("ERROR: cannot connect to shared memory %s\n", AOconf[loop].DMnameRM);
            exit(0);
        }
    }







    // Load DM modes (will exit if not successful)

    aoconfID_DMmodes = image_ID(AOconf[loop].DMMODESname); // if already exists, trust it and adopt it

    if(aoconfID_DMmodes==-1) // If not, check file
    {

        printf("Checking file ./conf/fmodes.fits\n");

        // GET SIZE FROM FILE
        ID1tmp = load_fits("./conf/fmodes.fits", "tmp3Dim", 1);
        if(ID1tmp==-1)
        {
            printf("ERROR: no file \"./conf/fmodes.fits\"\n");
            exit(0);
        }

        // check size
        if(data.image[ID1tmp].md[0].naxis != 3)
        {
            printf("ERROR: File \"./conf/fmodes.fits\" is not a 3D image (cube)\n");
            exit(0);
        }
        if(data.image[ID1tmp].md[0].size[0] != AOconf[loop].sizexDM)
        {
            printf("ERROR: File \"./conf/fmodes.fits\" has wrong x size: should be %ld, is %ld\n", AOconf[loop].sizexDM, data.image[ID1tmp].md[0].size[0]);
            exit(0);
        }
        if(data.image[ID1tmp].md[0].size[1] != AOconf[loop].sizeyDM)
        {
            printf("ERROR: File \"./conf/fmodes.fits\" has wrong y size: should be %ld, is %ld\n", AOconf[loop].sizeyDM, data.image[ID1tmp].md[0].size[1]);
            exit(0);
        }
        AOconf[loop].NBDMmodes = data.image[ID1tmp].md[0].size[2];

        printf("NUMBER OF MODES = %ld\n", AOconf[loop].NBDMmodes);

        // try to read it from shared memory
        ID2tmp = read_sharedmem_image(AOconf[loop].DMMODESname);
        vOK = 0;
        if(ID2tmp != -1) // if shared memory exists, check its size
        {
            vOK = 1;
            if(data.image[ID2tmp].md[0].naxis != 3)
            {
                printf("ERROR: Shared memory File %s is not a 3D image (cube)\n", AOconf[loop].DMMODESname);
                vOK = 0;
            }
            if(data.image[ID2tmp].md[0].size[0] != AOconf[loop].sizexDM)
            {
                printf("ERROR: Shared memory File %s has wrong x size: should be %ld, is %ld\n", AOconf[loop].DMMODESname, AOconf[loop].sizexDM, data.image[ID2tmp].md[0].size[0]);
                vOK = 0;
            }
            if(data.image[ID2tmp].md[0].size[1] != AOconf[loop].sizeyDM)
            {
                printf("ERROR: Shared memory File %s has wrong y size: should be %ld, is %ld\n", AOconf[loop].DMMODESname, AOconf[loop].sizeyDM, data.image[ID2tmp].md[0].size[1]);
                vOK = 0;
            }
            if(data.image[ID2tmp].md[0].size[2] != AOconf[loop].NBDMmodes)
            {
                printf("ERROR: Shared memory File %s has wrong y size: should be %ld, is %ld\n", AOconf[loop].DMMODESname, AOconf[loop].NBDMmodes, data.image[ID2tmp].md[0].size[2]);
                vOK = 0;
            }

            if(vOK==1) // if size is OK, adopt it
                aoconfID_DMmodes = ID2tmp;
            else // if not, erase shared memory
            {
                printf("SHARED MEM IMAGE HAS WRONG SIZE -> erasing it\n");
                delete_image_ID(AOconf[loop].DMMODESname);
            }
        }


        if(vOK==0) // create shared memory
        {

            sizearray[0] = AOconf[loop].sizexDM;
            sizearray[1] = AOconf[loop].sizeyDM;
            sizearray[2] = AOconf[loop].NBDMmodes;
            printf("Creating %s   [%ld x %ld x %ld]\n", AOconf[loop].DMMODESname, sizearray[0], sizearray[1], sizearray[2]);
            fflush(stdout);
            aoconfID_DMmodes = create_image_ID(AOconf[loop].DMMODESname, 3, sizearray, FLOAT, 1, 0);
        }

        // put modes into shared memory

        switch (data.image[ID1tmp].md[0].atype) {
        case FLOAT :
            memcpy(data.image[aoconfID_DMmodes].array.F, data.image[ID1tmp].array.F, sizeof(float)*AOconf[loop].sizexDM*AOconf[loop].sizeyDM*AOconf[loop].NBDMmodes);
            break;
        case DOUBLE :
            for(ii=0; ii<AOconf[loop].sizexDM*AOconf[loop].sizeyDM*AOconf[loop].NBDMmodes; ii++)
                data.image[aoconfID_DMmodes].array.F[ii] = data.image[ID1tmp].array.D[ii];
            break;
        default :
            printf("ERROR: TYPE NOT RECOGNIZED FOR MODES\n");
            exit(0);
            break;
        }

        delete_image_ID("tmp3Dim");
    }






    /*
        if(aoconfID_DMmodes!=-1)
        {

            printf("reading from shared memory %s   [%ld x %ld x %ld]\n", name, data.image[aoconfID_DMmodes].md[0].size[0], data.image[aoconfID_DMmodes].md[0].size[1], data.image[aoconfID_DMmodes].md[0].size[2]);
            AOconf[loop].NBDMmodes = data.image[aoconfID_DMmodes].md[0].size[2];
        }

        aoconfID_DMmodes = load_fits("./conf/fmodes.fits", "tmp3Dim");
        if(aoconfID_DMmodes!=-1)
        {
            if(data.image[aoconfID_DMmodes].md[0].naxis != 3)
            {
                printf("ERROR: File \"./conf/fmodes.fits\" is not a 3D image (cube)\n");
                exit(0);
            }
            if(data.image[aoconfID_DMmodes].md[0].size[0] != AOconf[loop].sizexDM)
            {
                printf("ERROR: File \"./conf/fmodes.fits\" has wrong x size: should be %ld, is %ld\n", AOconf[loop].sizexDM, data.image[aoconfID_DMmodes].md[0].size[0]);
                exit(0);
            }
            if(data.image[aoconfID_DMmodes].md[0].size[1] != AOconf[loop].sizexDM)
            {
                printf("ERROR: File \"./conf/fmodes.fits\" has wrong y size: should be %ld, is %ld\n", AOconf[loop].sizeyDM, data.image[aoconfID_DMmodes].md[0].size[1]);
                exit(0);
            }
        }




        if(aoconfID_DMmodes == -1)
        {
            printf("ERROR: NO DMmodes\n");
            exit(0);
        }



        // VERIFY DM MODES SIZE
        vOK = 0;
        if(aoconfID_DMmodes != -1)
        {
            vOK = 1;
            if(data.image[aoconfID_DMmodes].md[0].naxis != 3)
            {
                printf("DM modes has wrong dimension\n");
                vOK = 0;
            }
            if(data.image[aoconfID_DMmodes].md[0].atype != FLOAT)
            {
                printf("DM modes has wrong type\n");
                vOK = 0;
            }
            if(vOK==1)
            {
                if(data.image[aoconfID_DMmodes].md[0].size[0]!=AOconf[loop].sizexDM)
                {
                    printf("DM modes has wrong x size : is %ld, should be %ld\n", data.image[ID].md[0].size[0], AOconf[loop].sizexDM);
                    vOK = 0;
                }
                if(data.image[aoconfID_DMmodes].md[0].size[1]!=AOconf[loop].sizeyDM)
                {
                    printf("DM modes has wrong y size : is %ld, should be %ld\n", data.image[ID].md[0].size[0], AOconf[loop].sizexDM);
                    vOK = 0;
                }
            }
            if(vOK==1)
            {
                AOconf[loop].NBDMmodes = data.image[aoconfID_DMmodes].md[0].size[2];
                printf("%ld DM modes\n", AOconf[loop].NBDMmodes);
            }
        }
        if(vOK == 0)
        {
            printf("\n");
            printf("========== ERROR: NEED DM MODES TO START AO LOOP ===========\n");
            printf("\n");
            exit(0);
        }
    */






    // modes blocks

//    if(read_config_parameter(config_fname, "NBMblocks", content)==0)
//        AOconf[loop].NBMblocks = 1;
//    else
//        AOconf[loop].NBMblocks = atoi(content);
    AOconf[loop].NBMblocks = 5;
    printf("NBMblocks : %ld\n", AOconf[loop].NBMblocks);
    fflush(stdout);



    if(AOconf[loop].NBMblocks==1)
        AOconf[loop].indexmaxMB[0] = AOconf[loop].NBDMmodes;
    else
    {
        for(k=0; k<AOconf[loop].NBMblocks; k++)
            AOconf[loop].indexmaxMB[k] = (long) (pow(1.0*(k+1.0)/AOconf[loop].NBMblocks,2.0)*AOconf[loop].NBDMmodes);
        AOconf[loop].indexmaxMB[AOconf[loop].NBMblocks-1] = AOconf[loop].NBDMmodes;
    }


    for(k=0; k<AOconf[loop].NBMblocks; k++)
    {
        AOconf[loop].gainMB[k] = 1.0;
        AOconf[loop].limitMB[k] = 1.0;
        AOconf[loop].multfMB[k] = 1.0;
    }


    // Allocate / create logging data files/memory
    /*   if(read_config_parameter(config_fname, "logdir", content)==0)
       {
           printf("parameter logdir missing\n");
           exit(0);
       }
       strcpy(AOconf[loop].logdir, content);
       if(read_config_parameter(config_fname, "logsize", content)==0)
       {
           printf("parameter logsize missing\n");
           exit(0);
       }
       AOconf[loop].logsize = atol(content);*/
    // time [s]       (1)
    // gains          ( AOconf[loop].NBDMmodes )
    // ID_cmd_modes   ( AOconf[loop].NBDMmodes )
    // ID_meas_modes  ( AOconf[loop].NBDMmodes )

    /*    sizearray[0] = 1+3*AOconf[loop].NBDMmodes;
        sizearray[1] = AOconf[loop].logsize;
        sprintf(name, "loop%ldlog0", loop);
        aoconfIDlog0 = create_image_ID(name, 2, sizearray, FLOAT, 1, 10);
        ID = aoconfIDlog0;
        data.image[ID].md[0].NBkw = 1;
        kw = 0;
        strcpy(data.image[ID].kw[kw].name, "TIMEORIGIN");
        data.image[ID].kw[kw].type = 'L';
        data.image[ID].kw[kw].value.numl = 0;
        strcpy(data.image[ID].kw[kw].comment, "time offset [sec]");

        sprintf(name, "loop%ldlog1", loop);
        aoconfIDlog1 = create_image_ID(name, 2, sizearray, FLOAT, 1, 10);
        ID = aoconfIDlog1;
        data.image[ID].md[0].NBkw = 1;
        kw = 0;
        strcpy(data.image[ID].kw[kw].name, "TIMEORIGIN");
        data.image[ID].kw[kw].type = 'L';
        data.image[ID].kw[kw].value.numl = 0;
        strcpy(data.image[ID].kw[kw].comment, "time offset [sec]");
    */


    /*    AOconf[loop].logcnt = 0;
        AOconf[loop].logfnb = 0;
        strcpy(AOconf[loop].userLOGstring, "");
    */
    // AOconf[loop].ID_DMmodes = AOloopControl_MakeDMModes(loop, 5, name);

    printf("%ld modes\n", AOconf[loop].NBDMmodes);




    // REMOVE ??
    // load ref WFS image
    sprintf(name, "aol%ld_refWFSim", loop);
    aoconfID_refWFS = AOloopControl_2Dloadcreate_shmim(name, "./conf/refwfs.fits", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);



    // Load/create modal command vector memory
    sprintf(name, "aol%ld_DMmode_cmd", loop);
    ID = image_ID(name);
    aoconfID_cmd_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_cmd_modes].array.F[k] = 0.0;

    sprintf(name, "aol%ld_DMmode_meas", loop);
    ID = image_ID(name);
    aoconfID_meas_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_meas_modes].array.F[k] = 0.0;

    sprintf(name, "aol%ld_DMmode_AVE", loop);
    ID = image_ID(name);
    aoconfID_AVE_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_AVE_modes].array.F[k] = 0.0;

    sprintf(name, "aol%ld_DMmode_RMS", loop);
    ID = image_ID(name);
    aoconfID_RMS_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_RMS_modes].array.F[k] = 0.0;

    sprintf(name, "aol%ld_DMmode_GAIN", loop);
    ID = image_ID(name);
    aoconfID_GAIN_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_GAIN_modes].array.F[k] = 1.0;

    sprintf(name, "aol%ld_DMmode_LIMIT", loop);
    ID = image_ID(name);
    aoconfID_LIMIT_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_LIMIT_modes].array.F[k] = 1.0;

    sprintf(name, "aol%ld_DMmode_MULTF", loop);
    ID = image_ID(name);
    aoconfID_MULTF_modes = AOloopControl_2Dloadcreate_shmim(name, "", AOconf[loop].NBDMmodes, 1);
    if(ID==-1)
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            data.image[aoconfID_MULTF_modes].array.F[k] = 1.0;


    AOconf[loop].init_RM = 0;
    aoconfID_respM = AOloopControl_3Dloadcreate_shmim(AOconf[loop].respMname, "./conf/respm.fits", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, AOconf[loop].NBDMmodes);
    AOconf[loop].init_RM = 1;

  
    AOconf[loop].init_CM = 0;
    aoconfID_contrM = AOloopControl_3Dloadcreate_shmim(AOconf[loop].contrMname, "./conf/cmat.fits", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, AOconf[loop].NBDMmodes);
    AOconf[loop].init_CM = 1;
  
    free(sizearray);


    printf("   init_WFSref    %d\n", AOconf[loop].init_refWFS);
    printf("   init_RM        %d\n", AOconf[loop].init_RM);
    printf("   init_CM        %d\n", AOconf[loop].init_CM);

    AOconf[loop].init = 1;


    return(0);

}












int set_DM_modes(long loop)
{
    long k;
    long i, j;
    float *arrayf;
    double a;
    long cnttest;

    if(AOconf[loop].GPU == 0)
    {
        arrayf = (float*) malloc(sizeof(float)*AOconf[loop].sizeDM);
        for(j=0; j<AOconf[loop].sizeDM; j++)
            arrayf[j] = 0.0;

        for(i=0; i<AOconf[loop].sizeDM; i++)
            for(k=0; k < AOconf[loop].NBDMmodes; k++)
                arrayf[i] += data.image[aoconfID_cmd_modes].array.F[k] * data.image[aoconfID_DMmodes].array.F[k*AOconf[loop].sizeDM+i];

        data.image[aoconfID_DM].md[0].write = 1;
        memcpy (data.image[aoconfID_DM].array.F, arrayf, sizeof(float)*AOconf[loop].sizeDM);
        if(data.image[aoconfID_DM].sem == 1)
            sem_post(data.image[aoconfID_DM].semptr);
        data.image[aoconfID_DM].md[0].cnt0++;
        data.image[aoconfID_DM].md[0].write = 0;

        free(arrayf);
    }
    else
    {
#ifdef HAVE_CUDA
        GPU_loop_MultMat_setup(1, data.image[aoconfID_DMmodes].md[0].name, data.image[aoconfID_cmd_modes].md[0].name, data.image[aoconfID_DM].md[0].name, AOconf[loop].GPU, 1, AOconf[loop].GPUusesem, 1);
        AOconf[loop].status = 15;
        GPU_loop_MultMat_execute(1, &AOconf[loop].status, &AOconf[loop].GPUstatus[0], 1.0, 0.0);
#endif
    }

    if(aoconfID_DMdisp!=-1)
        if(data.image[aoconfID_DMdisp].sem1 == 1)
            sem_post(data.image[aoconfID_DMdisp].semptr1);

    AOconf[loop].DMupdatecnt ++;

    return(0);
}







int set_DM_modesRM(long loop)
{
    long k;
    long i, j;
    float *arrayf;



    arrayf = (float*) malloc(sizeof(float)*AOconf[loop].sizeDM);



    //printf("============ %ld %ld   %ld\n", aoconfID_cmd_modesRM, aoconfID_DMmodes, AOconf[loop].sizeDM);
    //fflush(stdout);
    //list_image_ID();


    for(j=0; j<AOconf[loop].sizeDM; j++)
        arrayf[j] = 0.0;

    for(k=0; k < AOconf[loop].NBDMmodes; k++)
    {
        for(i=0; i<AOconf[loop].sizeDM; i++)
            arrayf[i] += data.image[aoconfID_cmd_modesRM].array.F[k] * data.image[aoconfID_DMmodes].array.F[k*AOconf[loop].sizeDM+i];
    }


    data.image[aoconfID_DMRM].md[0].write = 1;
    memcpy (data.image[aoconfID_DMRM].array.F, arrayf, sizeof(float)*AOconf[loop].sizeDM);
    data.image[aoconfID_DMRM].md[0].cnt0++;
    data.image[aoconfID_DMRM].md[0].write = 0;

    free(arrayf);
    AOconf[loop].DMupdatecnt ++;


    return(0);
}







/** Measures zonal response matrix
 * -> collapses it to DM response map and WFS response map 
 * (both maps show amplitude of actuator effect on WFS) */

long Measure_zonalRM(long loop, double ampl, double delays, long NBave, char *zrespm_name, char *WFSref_name, char *WFSmap_name, char *DMmap_name)
{
    long ID_WFSmap, ID_WFSref, ID_DMmap, IDmapcube, IDzrespm, IDzrespmn, ID_WFSrefn;
    long act, j, ii, kk;
    double value;
    long delayus;
    float *arrayf;
    char fname[200];
    char name[200];
    long IDpos, IDneg;
    float tot, v1, rms;
    long *sizearray;

    long NBiter = 30;
    long iter;
    float *arraypix;
    long i;
    long istart, iend, icnt;
    long cntn;
    double tmpv;

    arraypix = (float*) malloc(sizeof(float)*NBiter);
    sizearray = (long*) malloc(sizeof(long)*3);

    delayus = (long) (1000000.0*delays);

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(0);


    //  sprintf(fname, "./conf/AOloop.conf");
    AOloopControl_loadconfigure(LOOPNUMBER, 1);
 

    printf("Importing DM response matrix channel shared memory ...\n");
    aoconfID_DMRM = read_sharedmem_image(AOconf[loop].DMnameRM);

    printf("Importing WFS camera image shared memory ... \n");
    aoconfID_WFS = read_sharedmem_image(AOconf[loop].WFSname);




    sprintf(name, "aol%ld_imWFS1RM", loop);
    sizearray[0] = AOconf[loop].sizexWFS;
    sizearray[1] = AOconf[loop].sizeyWFS;
    aoconfID_WFS1 = create_image_ID(name, 2, sizearray, FLOAT, 1, 0);


    arrayf = (float*) malloc(sizeof(float)*AOconf[loop].sizeDM);

    sizearray[0] = AOconf[loop].sizexDM;
    sizearray[1] = AOconf[loop].sizeyDM;
    ID_DMmap = create_image_ID(DMmap_name, 2, sizearray, FLOAT, 1, 5);

    IDpos = create_2Dimage_ID("wfsposim", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    IDneg = create_2Dimage_ID("wfsnegim", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);


    sizearray[0] = AOconf[loop].sizexWFS;
    sizearray[1] = AOconf[loop].sizeyWFS;
    sizearray[2] = AOconf[loop].sizeDM;
    ID_WFSmap = create_image_ID(WFSmap_name, 2, sizearray, FLOAT, 1, 5);
    ID_WFSref = create_image_ID("tmpwfsref", 2, sizearray, FLOAT, 1, 5);
    ID_WFSrefn = create_image_ID(WFSref_name, 2, sizearray, FLOAT, 1, 5);
    IDzrespm = create_image_ID("zrespm", 3, sizearray, FLOAT, 0, 5); // Zonal response matrix
    IDzrespmn = create_image_ID(zrespm_name, 3, sizearray, FLOAT, 0, 5); // Zonal response matrix normalized
 
    list_image_ID();

    cntn = 0;
    for(iter=0; iter<NBiter; iter++)
    {
        for(act=0; act<AOconf[loop].sizeDM; act++)
        {
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            {
                data.image[IDpos].array.F[ii] = 0.0;
                data.image[IDneg].array.F[ii] = 0.0;
            }


            for(j=0; j<AOconf[loop].sizeDM; j++)
                arrayf[j] = 0.0;

            arrayf[act] = ampl;

            data.image[aoconfID_DMRM].md[0].write = 1;
            memcpy (data.image[aoconfID_DMRM].array.F, arrayf, sizeof(float)*AOconf[loop].sizeDM);
            data.image[aoconfID_DMRM].md[0].cnt0++;
            data.image[aoconfID_DMRM].md[0].write = 0;
            AOconf[loop].DMupdatecnt ++;

            usleep(delayus);

            for(kk=0; kk<NBave; kk++)
            {
                Average_cam_frames(loop, 1, 0);
                for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                    data.image[IDpos].array.F[ii] += data.image[aoconfID_WFS1].array.F[ii];
            }
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                {
                    data.image[IDzrespm].array.F[act*AOconf[loop].sizeWFS+ii] += data.image[IDpos].array.F[ii];
                    data.image[ID_WFSref].array.F[ii] += data.image[IDpos].array.F[ii];
                }          



            for(j=0; j<AOconf[loop].sizeDM; j++)
                arrayf[j] = 0.0;

            arrayf[act] = -ampl;

            data.image[aoconfID_DMRM].md[0].write = 1;
            memcpy (data.image[aoconfID_DMRM].array.F, arrayf, sizeof(float)*AOconf[loop].sizeDM);
            data.image[aoconfID_DMRM].md[0].cnt0++;
            data.image[aoconfID_DMRM].md[0].write = 0;
            AOconf[loop].DMupdatecnt ++;

            usleep(delayus);

            for(kk=0; kk<NBave; kk++)
            {
                Average_cam_frames(loop, 1, 0);
                for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                    data.image[IDneg].array.F[ii] += data.image[aoconfID_WFS1].array.F[ii];
            }
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                {
                    data.image[IDzrespm].array.F[act*AOconf[loop].sizeWFS+ii] -= data.image[IDneg].array.F[ii];
                    data.image[ID_WFSref].array.F[ii] += data.image[IDneg].array.F[ii];
                }

            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            {
                data.image[IDpos].array.F[ii] = 0.0;
                data.image[IDneg].array.F[ii] = 0.0;
            }


       
        }
        cntn += NBave;
 
        for(act=0; act<AOconf[loop].sizeDM; act++)
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                data.image[IDzrespmn].array.F[act*AOconf[loop].sizeWFS+ii] = data.image[IDzrespm].array.F[act*AOconf[loop].sizeWFS+ii]/ampl/NBave;
        sprintf(fname, "!%s.fits", zrespm_name);
        save_fits(zrespm_name, fname);

        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[ID_WFSrefn].array.F[ii] = data.image[ID_WFSref].array.F[ii]/NBave/cntn;
        sprintf(fname, "!%s.fits", WFSref_name);
        save_fits(WFSref_name, fname);
    

        for(act=0; act<AOconf[loop].sizeDM; act++)
        {
            rms = 0.0;
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            {
                tmpv = data.image[IDzrespmn].array.F[act*AOconf[loop].sizeWFS+ii];
                rms += tmpv*tmpv;
            }
            data.image[ID_DMmap].array.F[act] = rms;
        }
        sprintf(fname, "!%s.fits", DMmap_name);
        save_fits(DMmap_name, fname);

        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
        {
            rms = 0.0;
            for(act=0; act<AOconf[loop].sizeDM; act++)
            {
                tmpv = data.image[IDzrespmn].array.F[act*AOconf[loop].sizeWFS+ii];
                rms += tmpv*tmpv;
            }
            data.image[ID_WFSmap].array.F[ii] = rms;
        }
        sprintf(fname, "!%s.fits", WFSmap_name);
        save_fits(WFSmap_name, fname);
    }

    free(arrayf);
    free(sizearray);
    free(arraypix);

    delete_image_ID("tmpwfsref");

    return(ID_WFSmap);
}























/** Record periodic camera signal (to be used if there is a periodic camera error)
 *
 * folds the signal onto one period
 *
 */

int AOloopControl_Measure_WFScam_PeriodicError(long loop, long NBframes, long NBpha, char *IDout_name)
{
    FILE *fp;
    char fname[200];
    long ii, jj, kk, kk1, kkmax;
    long IDrc, IDrefim;
    long IDout;

    double period; /// in frames
    double period_start = 1000.0;
    double period_end = 1200.0;
    double period_step;
    double pha;
    long *phacnt;
    long phal;
    long double rmsval;
    double rmsvalmin, rmsvalmax;
    double periodopt;
    long cnt;
    long p, p0, p1, pmin, pmax;

    double intpart;
    double tmpv1;

    double *coarsermsarray;
    double rmsvalmin1;
    int lOK;
    double level1, level2, level3;
    long pp1, pp2, pp3;
    int level1OK, level2OK, level3OK;

    long kw;
    char kname[200];
    char comment[200];


    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(0);


    IDrc = create_3Dimage_ID("Rcube", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, NBframes);
    IDout = create_3Dimage_ID(IDout_name, AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, NBpha);

    printf("SETTING UP... (loop %ld)\n", LOOPNUMBER);
    fflush(stdout);

  //  sprintf(fname, "./conf/AOloop.conf");
    AOloopControl_loadconfigure(LOOPNUMBER, 1);
    //exit(0);

    printf("Importing WFS camera image shared memory ... \n");
    aoconfID_WFS = read_sharedmem_image(AOconf[loop].WFSname);



    for(kk=0; kk<NBframes; kk++)
    {
        Average_cam_frames(loop, 1, 0);
        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[IDrc].array.F[kk*AOconf[loop].sizeWFS+ii] = data.image[aoconfID_WFS1].array.F[ii];
    }

    save_fits("Rcube", "!Rcube.fits");

    IDrefim = create_2Dimage_ID("refim", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    for(kk=0; kk<NBframes; kk++)
    {
        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[IDrefim].array.F[ii] += data.image[IDrc].array.F[kk*AOconf[loop].sizeWFS+ii];
    }
    for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
        data.image[IDrefim].array.F[ii] /= NBframes;

    for(kk=0; kk<NBframes; kk++)
    {
        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[IDrc].array.F[kk*AOconf[loop].sizeWFS+ii] -= data.image[IDrefim].array.F[ii];
    }
    save_fits("Rcube", "!R1cube.fits");


    /** find periodicity ( coarse search ) */
    fp = fopen("wfscampe_coarse.txt","w");
    fclose(fp);

    pmax = (long) NBframes/2;
    pmin = 0;
    rmsvalmin = 1.0e20;



    rmsvalmax = 0.0;
    p0 = 200;
    coarsermsarray = (double*) malloc(sizeof(double)*pmax);
    for(p=p0; p<pmax; p++)
    {
        rmsval = 0.0;
        kkmax = 100;
        if(kkmax+pmax>NBframes)
        {
            printf("ERROR: pmax, kkmax not compatible\n");
            exit(0);
        }

        for(kk=0; kk<kkmax; kk++)
        {
            kk1 = kk+p;
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            {
                tmpv1 = data.image[IDrc].array.F[kk*AOconf[loop].sizeWFS+ii] - data.image[IDrc].array.F[kk1*AOconf[loop].sizeWFS+ii];
                rmsval += tmpv1*tmpv1;
            }
        }
        rmsval = sqrt(rmsval/kkmax/AOconf[loop].sizeWFS);

        if(rmsval<rmsvalmin)
        {
            rmsvalmin = rmsval;
            pmin = p;
        }
        if(rmsval>rmsvalmax)
            rmsvalmax = rmsval;

        coarsermsarray[p] = rmsval;

        printf("%20ld  %20g     [ %20ld  %20g ]\n", p, (double) rmsval, pmin, rmsvalmin);
        fp = fopen("wfscampe_coarse.txt","a");
        fprintf(fp, "%20ld %20g\n", p, (double) rmsval);
        fclose(fp);
    }

    level1 = rmsvalmin + 0.2*(rmsvalmax-rmsvalmin);
    level1OK = 0; /// toggles to 1 when curve first goes above level1

    level2 = rmsvalmin + 0.8*(rmsvalmax-rmsvalmin);
    level2OK = 0; /// toggles to 1 when curve first goes above level2 after level1OK

    level3 = rmsvalmin + 0.2*(rmsvalmax-rmsvalmin);
    level3OK = 0; /// toggles to 1 when curve first goes above level3 after level2OK

    p = p0;
    p1 = 0;
    lOK = 0;
    rmsvalmin1 = rmsvalmax;
    while((lOK==0)&&(p<pmax))
    {
        if(level1OK==0)
            if(coarsermsarray[p]>level1)
            {
                level1OK = 1;
                pp1 = p;
            }

        if((level1OK==1)&&(level2OK==0))
            if(coarsermsarray[p]>level2)
            {
                level2OK = 1;
                pp2 = p;
            }

        if((level1OK==1)&&(level2OK==1)&&(level3OK==0))
            if(coarsermsarray[p]<level3)
            {
                pp3 = p;
                level3OK = 1;
            }

        if((level1OK==1)&&(level2OK==1)&&(level3OK==1))
        {
            if(coarsermsarray[p] < rmsvalmin1)
            {
                rmsvalmin1 = coarsermsarray[p];
                p1 = p;
            }

            if(coarsermsarray[p]>level2)
                lOK = 1;
        }
        p++;
    }

    free(coarsermsarray);


    printf("APPROXIMATE PERIOD = %ld   [%ld %ld %ld]  [%f %f %f]\n", p1, pp1, pp2, pp3, level1, level2, level3);

    /** find periodicity ( fine search ) */

    periodopt = 0.0;
    rmsvalmax = 0.0;

    fp = fopen("wfscampe.txt","w");
    fclose(fp);

    period_start = 1.0*p1 - 15.0;
    period_end = 1.0*p1 + 15.0;

    phacnt = (long*) malloc(sizeof(long)*NBpha);
    period_step = (period_end-period_start)/300.0;
    for(period=period_start; period<period_end; period += period_step)
    {
        for(kk=0; kk<NBpha; kk++)
            phacnt[kk] = 0;

        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[IDout].array.F[phal*AOconf[loop].sizeWFS+ii] = 0.0;

        for(kk=0; kk<NBframes; kk++)
        {
            pha = 1.0*kk/period;
            pha = modf(pha, &intpart);
            phal = (long) (1.0*NBpha*pha);

            if(phal>NBpha-1)
                phal = NBpha-1;
            if(phal<0)
                phal = 0;

            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                data.image[IDout].array.F[phal*AOconf[loop].sizeWFS+ii] += data.image[IDrc].array.F[kk*AOconf[loop].sizeWFS+ii];

            phacnt[phal]++;
        }

        rmsval = 0.0;
        cnt = 0;
        for(kk=0; kk<NBpha; kk++)
        {
            if(phacnt[kk]>0)
            {
                cnt++;
                for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                {
                    data.image[IDout].array.F[kk*AOconf[loop].sizeWFS+ii] /= phacnt[kk];
                    rmsval = data.image[IDout].array.F[kk*AOconf[loop].sizeWFS+ii]*data.image[IDout].array.F[kk*AOconf[loop].sizeWFS+ii];
                }
            }
        }


        rmsval = sqrt(rmsval/AOconf[loop].sizeWFS/cnt);
        if(rmsval>rmsvalmax)
        {
            rmsvalmax = rmsval;
            periodopt = period;
        }
        printf("%20f  %20g     [ %20f  %20g ]\n", period, (double) rmsval, periodopt, rmsvalmax);
        fp = fopen("wfscampe.txt","a");
        fprintf(fp, "%20f %20g\n", period, (double) rmsval);
        fclose(fp);
    }

    printf("EXACT PERIOD = %f\n", periodopt);

    kw = 0;
    sprintf(kname, "PERIOD");
    strcpy(data.image[IDout].kw[kw].name, kname);
    data.image[IDout].kw[kw].type = 'D';
    data.image[IDout].kw[kw].value.numf = (double) periodopt;
    sprintf(comment, "WFS cam error period");
    strcpy(data.image[IDout].kw[kw].comment, comment);


    /// building phase cube
    period = periodopt;

    for(kk=0; kk<NBpha; kk++)
        phacnt[kk] = 0;
    for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
        data.image[IDout].array.F[phal*AOconf[loop].sizeWFS+ii] = 0.0;

    for(kk=0; kk<NBframes; kk++)
    {
        pha = 1.0*kk/period;
        pha = modf(pha, &intpart);
        phal = (long) (1.0*NBpha*pha);

        if(phal>NBpha-1)
            phal = NBpha-1;
        if(phal<0)
            phal = 0;

        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[IDout].array.F[phal*AOconf[loop].sizeWFS+ii] += data.image[IDrc].array.F[kk*AOconf[loop].sizeWFS+ii];
        phacnt[phal]++;
    }

    rmsval = 0.0;
    cnt = 0;
    for(kk=0; kk<NBpha; kk++)
    {
        if(phacnt[kk]>0)
        {
            cnt++;
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            {
                data.image[IDout].array.F[kk*AOconf[loop].sizeWFS+ii] /= phacnt[kk];
            }
        }
    }



    free(phacnt);

    return(0);
}







/** remove WFS camera periodic error
 *
 * pha: phase from 0.0 to 1.0
 */

int AOloopControl_Remove_WFScamPE(char *IDin_name, char *IDcorr_name, double pha)
{
    long IDin;
    long IDcorr;
    long phal;
    long xsize, ysize, zsize, xysize;
    long ii;


    IDin = image_ID(IDin_name);
    IDcorr = image_ID(IDcorr_name);

    xsize = data.image[IDcorr].md[0].size[0];
    ysize = data.image[IDcorr].md[0].size[1];
    zsize = data.image[IDcorr].md[0].size[2];
    xysize = xsize*ysize;

    phal = (long) (1.0*pha*zsize);
    if(phal>zsize-1)
        phal -= zsize;



    for(ii=0; ii<xysize; ii++) {
        data.image[IDin].array.F[ii] -= data.image[IDcorr].array.F[xysize*phal+ii];
    }


    return(0);
}











/** measures response matrix AND reference */
// scan delay up to fDelay

int Measure_Resp_Matrix(long loop, long NbAve, float amp, long nbloop, long fDelay, long NBiter)
{
    long NBloops;
    long kloop;
    long delayus = 10000; // delay in us
    long ii, i, imax;
    int Verbose = 0;
    long k1, k, k2;
    char fname[200];
    char name0[200];
    char name[200];

    long kk;
    long RespMatNBframes;
    long IDrmc;
    long kc;

    long IDeigenmodes;

    long frameDelay = 0;
    long frameDelayMax = 50;
    long double RMsig;
    long double RMsigold;
    long kc0;
    FILE *fp;
    long NBexcl = 2; // number of frames excluded between DM mode changes
    long kc0min, kc0max;
    long IDrmtest;
    int vOK;

    long iter;
    long IDrmi;
    float beta = 0.0;
    float gain = 0.0001;
    long IDrmcumul;
    long IDrefi;
    long IDrefcumul;

    long *sizearray;

    long IDrespM;
    long IDrefWFS;

    int r;

    long IDoptsignal; // optical signal for each mode, cumulative
    long IDoptsignaln; // optical signal for each mode, normalize
    long IDmcoeff; // multiplicative gain to amplify low-oder modes
    long IDoptcnt;
    double rmsval;
    char signame[200];

    double normcoeff, normcoeffcnt;


    int AdjustAmplitude = 0;
    char command[2000];

    float valave;
    long IDrmc1;



    RMACQUISITION = 1;


    printf("ACQUIRE RESPONSE MATRIX - loop = %ld, NbAve = %ld, amp = %f, nbloop = %ld, fDelay = %ld, NBiter = %ld\n", loop, NbAve, amp, nbloop, fDelay, NBiter);

    sizearray = (long*) malloc(sizeof(long)*3);

  
  
     if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(0);
 //   sprintf(fname, "./conf/AOloop.conf");
    AOloopControl_loadconfigure(LOOPNUMBER, 1);


    // create output
    IDrefWFS = create_2Dimage_ID("refwfsacq", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    IDrespM = create_3Dimage_ID_float("respmacq", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, AOconf[loop].NBDMmodes);




    IDoptsignal = create_2Dimage_ID("optsig", AOconf[loop].NBDMmodes, 1);
    IDoptsignaln = create_2Dimage_ID("optsign", AOconf[loop].NBDMmodes, 1);
    IDmcoeff = create_2Dimage_ID("mcoeff", AOconf[loop].NBDMmodes, 1);
    IDoptcnt = create_2Dimage_ID("optsigcnt", AOconf[loop].NBDMmodes, 1);

    for(k=0; k<AOconf[loop].NBDMmodes; k++)
    {
        data.image[IDoptcnt].array.F[k] = 0.0;
        data.image[IDoptsignal].array.F[k] = 0.0;
        data.image[IDoptsignaln].array.F[k] = 0.0;
        data.image[IDmcoeff].array.F[k] = 1.0;
    }


    RespMatNBframes = 2*AOconf[loop].NBDMmodes*NbAve;  // *nbloop
    printf("%ld frames total\n", RespMatNBframes);
    fflush(stdout);

    IDrmc = create_3Dimage_ID("RMcube", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, RespMatNBframes); // this is the main cube





    IDrmi = create_3Dimage_ID("RMiter", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, AOconf[loop].NBDMmodes);    // Response matrix for 1 iteration
    IDrmcumul = create_3Dimage_ID("RMcumul", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, AOconf[loop].NBDMmodes);  // Cumulative Response matrix

    IDrefi = create_2Dimage_ID("REFiter", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    IDrefcumul = create_2Dimage_ID("REFcumul", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);



    /// local arrays for image acquision
    //	aoconfID_WFS = create_2Dimage_ID("RMwfs", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    aoconfID_WFS0 = create_2Dimage_ID("RMwfs0", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    aoconfID_WFS1 = create_2Dimage_ID("RMwfs1", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
    aoconfID_WFS2 = create_2Dimage_ID("RMwfs2", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);


    aoconfID_cmd_modesRM = create_2Dimage_ID("RMmodesloc", AOconf[loop].NBDMmodes, 1);


    for(iter=0; iter<NBiter; iter++)
    {
        if (file_exist ("stopRM.txt"))
        {
            r = system("rm stopRM.txt");
            iter = NBiter;
        }
        else
        {
            NBloops = nbloop;


            // initialize reference to zero
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                data.image[IDrefi].array.F[ii] = 0.0;

            for(ii=0; ii<AOconf[loop].sizeWFS*RespMatNBframes; ii++)
                data.image[IDrmc].array.F[ii] = 0.0;


            printf("\n");
            printf("Testing (in measure_resp_matrix function) :,  NBloops = %ld, NBmode = %ld\n",  NBloops, AOconf[loop].NBDMmodes);
            fflush(stdout);
            sleep(1);

            for(k2 = 0; k2 < AOconf[loop].NBDMmodes; k2++)
                data.image[aoconfID_cmd_modesRM].array.F[k2] = 0.0;

                  

            // set DM to last mode, neg 
            k1 = AOconf[loop].NBDMmodes-1;
            data.image[aoconfID_cmd_modesRM].array.F[k1] = -amp*data.image[IDmcoeff].array.F[k1];
            set_DM_modesRM(loop);


            usleep(delayus); 

            for (kloop = 0; kloop < NBloops; kloop++)
            {
                kc = 0;
                if(Verbose)
                {
                    printf("\n Loop %ld / %ld (%f)\n", kloop, NBloops, amp);
                    fflush(stdout);
                }


                for(k1 = 0; k1 < AOconf[loop].NBDMmodes; k1++)
                {
                    for(k2 = 0; k2 < AOconf[loop].NBDMmodes; k2++)
                        data.image[aoconfID_cmd_modesRM].array.F[k2] = 0.0;

                    // positive
                    data.image[aoconfID_cmd_modesRM].array.F[k1] = amp*data.image[IDmcoeff].array.F[k1];
                    set_DM_modesRM(loop);




                    for(kk=0; kk<NbAve; kk++)
                    {
                        Average_cam_frames(loop, 1, 1);


                        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                        {
                            data.image[IDrefi].array.F[ii] += data.image[aoconfID_WFS1].array.F[ii];
                            data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii] += data.image[aoconfID_WFS1].array.F[ii];
                        }
                        kc++;
                    }


                    // negative
                    data.image[aoconfID_cmd_modesRM].array.F[k1] = 0.0-amp*data.image[IDmcoeff].array.F[k1];
                    set_DM_modesRM(loop);

                   

                    for(kk=0; kk<NbAve; kk++)
                    {
                        Average_cam_frames(loop, 1, 1);

                        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                        {
                            data.image[IDrefi].array.F[ii] += data.image[aoconfID_WFS1].array.F[ii];
                            data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii] += data.image[aoconfID_WFS1].array.F[ii];
                        }
                        kc++;
                    }
                }
            }

            for(ii=0; ii<AOconf[loop].sizeWFS*RespMatNBframes; ii++)
                data.image[IDrmc].array.F[ii] /= NBloops;


            // set DM to zero
            for(k2 = 0; k2 < AOconf[loop].NBDMmodes; k2++)
                data.image[aoconfID_cmd_modesRM].array.F[k2] = 0.0;
            set_DM_modesRM(loop);

            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                data.image[IDrefi].array.F[ii] /= RespMatNBframes*NBloops; 



            // SAVE RMCUBE
            //    save_fits("RMcube", "!RMcube.fits");

            // remove average
            if(1)
            {
                IDrmc1 = create_3Dimage_ID("RMcube1", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, RespMatNBframes); // this is the main cube, average removed

                for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                {
                    valave = 0.0;
                    for(kc=0; kc<RespMatNBframes; kc++)
                        valave += data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii];
                    valave /= RespMatNBframes;
                    for(kc=0; kc<RespMatNBframes; kc++)
                        data.image[IDrmc1].array.F[kc*AOconf[loop].sizeWFS+ii] = data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii] - valave;
                }
                save_fits("RMcube1", "!RMcube1.fits");
            }




            IDrmtest = create_3Dimage_ID("rmtest", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS, AOconf[loop].NBDMmodes);


            kc0 = fDelay;

            // initialize RM to zero
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                for(k=0; k<AOconf[loop].NBDMmodes; k++)
                    data.image[IDrmtest].array.F[k*AOconf[loop].sizeWFS+ii] = 0.0;

            // initialize reference to zero
            kc = kc0;

            for(k1 = 0; k1 < AOconf[loop].NBDMmodes; k1++)
            {
                // positive
                kc += NBexcl;
                if(kc > data.image[IDrmc].md[0].size[2]-1)
                    kc -= data.image[IDrmc].md[0].size[2];
                for(kk=NBexcl; kk<NbAve-NBexcl; kk++)
                {
                    for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                        {
                            data.image[IDrmtest].array.F[k1*AOconf[loop].sizeWFS+ii] += data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii];
                       //     data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii] += 1.0;
                        }
                    kc++;
                    if(kc > data.image[IDrmc].md[0].size[2]-1)
                        kc -= data.image[IDrmc].md[0].size[2];
                }
                kc+=NBexcl;
                if(kc > data.image[IDrmc].md[0].size[2]-1)
                    kc -= data.image[IDrmc].md[0].size[2];

                // negative
                kc+=NBexcl;
                if(kc > data.image[IDrmc].md[0].size[2]-1)
                    kc -= data.image[IDrmc].md[0].size[2];               
                for(kk=NBexcl; kk<NbAve-NBexcl; kk++)
                {
                    for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                        {
                            data.image[IDrmtest].array.F[k1*AOconf[loop].sizeWFS+ii] -= data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii];
                          //  data.image[IDrmc].array.F[kc*AOconf[loop].sizeWFS+ii] -= 1.0;
                        }
                    kc++;
                    if(kc > data.image[IDrmc].md[0].size[2]-1)
                        kc -= data.image[IDrmc].md[0].size[2];
                }
                kc+=NBexcl;
                if(kc > data.image[IDrmc].md[0].size[2]-1)
                    kc -= data.image[IDrmc].md[0].size[2];
            }

          //  save_fits("RMcube", "!RMcube2.fits");
          //  exit(0);
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                for(k1=0; k1<AOconf[loop].NBDMmodes; k1++)
                    data.image[IDrmi].array.F[k1*AOconf[loop].sizeWFS+ii] = data.image[IDrmtest].array.F[k1*AOconf[loop].sizeWFS+ii];


            //        save_fl_fits("rmtest", "!rmtest.fits");
            delete_image_ID("rmtest");




            printf("%ld %ld  %ld  %ld\n", IDrefcumul, IDrmcumul, IDrefWFS, IDrespM);


            beta = (1.0-gain)*beta + gain;
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            {
                data.image[IDrefcumul].array.F[ii] = (1.0-gain)*data.image[IDrefcumul].array.F[ii] + gain*data.image[IDrefi].array.F[ii];

                data.image[IDrefWFS].array.F[ii] = data.image[IDrefcumul].array.F[ii]/beta;



                for(k1=0; k1<AOconf[loop].NBDMmodes; k1++)
                {
                    data.image[IDrmcumul].array.F[k1*AOconf[loop].sizeWFS+ii] = (1.0-gain)*data.image[IDrmcumul].array.F[k1*AOconf[loop].sizeWFS+ii] + gain*data.image[IDrmi].array.F[k1*AOconf[loop].sizeWFS+ii];
                    data.image[IDrespM].array.F[k1*AOconf[loop].sizeWFS+ii] = data.image[IDrmcumul].array.F[k1*AOconf[loop].sizeWFS+ii]/beta;
                }
            }

            for(k1=0; k1<AOconf[loop].NBDMmodes; k1++)
            {
                rmsval = 0.0;
                for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                    rmsval += data.image[IDrespM].array.F[k1*AOconf[loop].sizeWFS+ii]*data.image[IDrespM].array.F[k1*AOconf[loop].sizeWFS+ii];

                data.image[IDoptsignal].array.F[k1] += rmsval;
                data.image[IDoptcnt].array.F[k1] += 1.0;

                data.image[IDoptsignaln].array.F[k1] = data.image[IDoptsignal].array.F[k1]/data.image[IDoptcnt].array.F[k1];
            }
            save_fits("optsignaln","!./tmp/RM_optsign.fits");

            sprintf(signame, "./tmp/RM_optsign_%06ld.txt", iter);

            normcoeff = 0.0;
            normcoeffcnt = 0.0;
            for(k1=AOconf[loop].NBDMmodes/2; k1<AOconf[loop].NBDMmodes; k1++)
            {
                normcoeff += data.image[IDoptsignaln].array.F[k1];
                normcoeffcnt += 1.0;
            }
            normcoeff /= normcoeffcnt;



            if(AdjustAmplitude==1)
                for(k1=0; k1<AOconf[loop].NBDMmodes; k1++)
                {
                    data.image[IDmcoeff].array.F[k1] = 0.8*data.image[IDmcoeff].array.F[k1] + 0.2/(data.image[IDoptsignaln].array.F[k1]/normcoeff);
                    if(data.image[IDmcoeff].array.F[k1]>5.0)
                        data.image[IDmcoeff].array.F[k1] = 5.0;
                }

            fp = fopen(signame, "w");
            for(k1=0; k1<AOconf[loop].NBDMmodes; k1++)
                fprintf(fp, "%ld  %g  %g  %g\n", k1, data.image[IDoptsignaln].array.F[k1], data.image[IDoptcnt].array.F[k1], data.image[IDmcoeff].array.F[k1]*amp);
            fclose(fp);
            r = system("cp ./tmp/RM_outsign%06ld.txt ./tmp/RM_outsign.txt");

            save_fits("refwfsacq", "!./tmp/refwfs.fits");
            save_fits("respmacq", "!./tmp/respm.fits");
        }
    }


    fp = fopen("./tmp/rmparams.txt", "w");
    fprintf(fp, "%5ld       NbAve: number of WFS frames per averaging\n", NbAve);
    fprintf(fp, "%f	        amp: nominal DM amplitude (RMS)\n", amp);
    fprintf(fp, "%ld        iter: number of iterations\n", iter);
    fprintf(fp, "%ld        nbloop: number of loops per iteration\n", nbloop);
    fprintf(fp, "%ld        fDelay: delay number of frames\n", fDelay);
    fclose(fp);



    printf("Done\n");
    free(sizearray);

    return(0);
}













int ControlMatrixMultiply( float *cm_array, float *imarray, long m, long n, float *outvect)
{
    long i;

    cblas_sgemv (CblasRowMajor, CblasNoTrans, m, n, 1.0, cm_array, n, imarray, 1, 0.0, outvect, 1);

    return(0);
}




// computes combined control matrix
//
//


long compute_CombinedControlMatrix(char *IDcmat_name, char *IDmodes_name, char* IDwfsmask_name, char *IDdmmask_name, char *IDcmatc_name, char *IDcmatc_active_name)
{
    long ID;
    struct timespec t1;
    struct timespec t2;
    struct timespec tdiff;
    double tdiffv;

    float *matrix_cmp;
    long wfselem, act, mode;
    //    long n_sizeDM, n_NBDMmodes, n_sizeWFS;
    float *matrix_Mc, *matrix_DMmodes;
    long act_active, wfselem_active;
    int chunk = 10;

    long IDwfsmask, IDdmmask;
    long sizexWFS, sizeyWFS, sizeWFS, sizeWFS_active;
    long ii, ii1;
    long sizexDM, sizeyDM;
    long sizeDM_active;
    long *sizearray;
    long IDcmat;
    long IDcmatc;
    long IDmodes;
    long NBDMmodes;
    long sizeDM;
    long IDcmatc_active;

    

    printf("COMPUTING COMBINED CONTROL MATRIX .... \n");
    fflush(stdout);

    clock_gettime(CLOCK_REALTIME, &t1);


    list_image_ID();

    // initialize size of arrays
    IDwfsmask = image_ID(IDwfsmask_name);
    sizexWFS = data.image[IDwfsmask].md[0].size[0];
    sizeyWFS = data.image[IDwfsmask].md[0].size[1];
    sizeWFS = sizexWFS*sizeyWFS;

    IDdmmask = image_ID(IDdmmask_name);
    sizexDM = data.image[IDdmmask].md[0].size[0];
    sizeyDM = data.image[IDdmmask].md[0].size[1];
    sizeDM = sizexDM*sizeyDM;
 
    IDmodes = image_ID(IDmodes_name);
    NBDMmodes = data.image[IDmodes].md[0].size[2];
    
    // allocate array for combined matrix
    sizearray = (long*) malloc(sizeof(long)*3);
    sizearray[0] = sizexWFS;
    sizearray[1] = sizeyWFS;
    sizearray[2] = sizeDM;
    IDcmatc = create_image_ID(IDcmatc_name, 3, sizearray, FLOAT, 1, 0);
    free(sizearray);


    list_image_ID();
    printf("PREPARE MATRIX MULT\n");
    fflush(stdout);

    

    // init matrix_Mc
    matrix_Mc = (float*) malloc(sizeof(float)*sizeWFS*sizeDM);
    memcpy(matrix_Mc, data.image[IDcmatc].array.F, sizeof(float)*sizeWFS*sizeDM);

    // copy modal control matrix to matrix_cmp
    IDcmat = image_ID(IDcmat_name);
    matrix_cmp = (float*) malloc(sizeof(float)*sizeWFS*NBDMmodes);
    memcpy(matrix_cmp, data.image[IDcmat].array.F, sizeof(float)*sizeWFS*NBDMmodes);

    // copy modes matrix to matrix_DMmodes
    matrix_DMmodes = (float*) malloc(sizeof(float)*NBDMmodes*sizeDM);
    memcpy(matrix_DMmodes, data.image[IDmodes].array.F, sizeof(float)*NBDMmodes*sizeDM);
   
    printf("START MATRIX MULT\n");
    fflush(stdout);

   

// computing combine matrix (full size)
# ifdef _OPENMP
   #pragma omp parallel shared(matrix_Mc, matrix_cmp, matrix_DMmodes ,chunk) private( mode, act, wfselem)
    {
        #pragma omp for schedule (static)
# endif
        for(mode=0; mode<NBDMmodes; mode++)
        {
          //  printf("mode %6ld   [%ld %ld %ld] \n", mode, NBDMmodes, sizeDM, sizeWFS);
          //  fflush(stdout);
            for(act=0; act<sizeDM; act++)
                {
                    for(wfselem=0; wfselem<sizeWFS; wfselem++)
                    {
                    //   printf("-   [%ld/%ld %ld/%ld %ld/%ld] \n", mode, act, wfselem);
                    // fflush(stdout);
                        matrix_Mc[act*sizeWFS+wfselem] += matrix_cmp[mode*sizeWFS+wfselem]*matrix_DMmodes[mode*sizeDM+act];
                    }
                }
        }
# ifdef _OPENMP
    }
# endif
    memcpy(data.image[IDcmatc].array.F, matrix_Mc, sizeof(float)*sizeWFS*sizeDM);

 
    printf("REDUCE MATRIX SIZE\n");
    fflush(stdout);


   WFS_active_map = (int*) malloc(sizeof(int)*sizeWFS);
    ii1 = 0;
    for(ii=0; ii<sizeWFS; ii++)
        if(data.image[IDwfsmask].array.F[ii]>0.1)
        {
            WFS_active_map[ii1] = ii;
            ii1++;
        }
    sizeWFS_active = ii1;
    aoconfID_WFS2_active = create_2Dimage_ID("wfs2active", sizeWFS_active, 1);


    DM_active_map = (int*) malloc(sizeof(int)*sizeDM);
    ii1 = 0;
    for(ii=0; ii<sizeDM; ii++)
        if(data.image[IDdmmask].array.F[ii]>0.1)
        {
            DM_active_map[ii1] = ii;
            ii1++;
        }
    sizeDM_active = ii1;
    aoconfID_meas_act_active = create_2Dimage_ID("meas_act_active", sizeDM_active, 1);





// reduce matrix size to active elements
    IDcmatc_active = create_2Dimage_ID("cmatc_active", sizeWFS_active, sizeDM_active);
    for(act_active=0; act_active<sizeDM_active; act_active++)
    {
        for(wfselem_active=0; wfselem_active<sizeWFS_active; wfselem_active++)
        {
            act = DM_active_map[act_active];
            wfselem = WFS_active_map[wfselem_active];
            data.image[IDcmatc_active].array.F[act_active*sizeWFS_active+wfselem_active] = matrix_Mc[act*sizeWFS+wfselem];
        }
    }
    free(matrix_Mc);
    free(matrix_DMmodes);

    printf("Keeping only active pixels / actuators : %ld x %ld   ->   %ld x %ld\n", sizeWFS, sizeDM, sizeWFS_active, sizeDM_active);






    clock_gettime(CLOCK_REALTIME, &t2);
    tdiff = info_time_diff(t1, t2);
    tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
    printf("\n");
    printf("TIME TO COMPUTE MATRIX = %f sec\n", tdiffv);


    return(ID);
}









int AOcompute(long loop)
{
    float total = 0.0;
    long k, k1, k2;
    long ii;
    long i;
    long m, n;
    long index;
    //  long long wcnt;
    // long long wcntmax;
    long cnttest;
    double a;
    long *sizearray;

    float *matrix_cmp;
    long wfselem, act, mode;

    struct timespec t1;
    struct timespec t2;
    struct timespec tdiff;
    double tdiffv;

    int chunk = 10;
    float *matrix_Mc, *matrix_DMmodes;
    long n_sizeDM, n_NBDMmodes, n_sizeWFS;

    long ii1;
    long IDmask;
    long act_active, wfselem_active;
    float *matrix_Mc_active;
    long IDcmatca_shm;
    char imname[200];
        
    float imtot;

    // get dark-subtracted image
    AOconf[loop].status = 1;  // 1: READING IMAGE
    Average_cam_frames(loop, AOconf[loop].framesAve, 0);

    AOconf[loop].status = 6;  // 6: REMOVING REF


    if(COMPUTE_GPU_SCALING==0)
    {
        data.image[aoconfID_WFS2].md[0].write = 1;
        for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
            data.image[aoconfID_WFS2].array.F[ii] = data.image[aoconfID_WFS1].array.F[ii] - normfloorcoeff*data.image[aoconfID_refWFS].array.F[ii];
        cnttest = data.image[aoconfID_meas_modes].md[0].cnt0;
        data.image[aoconfID_WFS2].md[0].cnt0 ++;
        data.image[aoconfID_WFS2].md[0].write = 0;
    }

    //  save_fits(data.image[aoconfID_WFS].md[0].name, "!testim.fits");
    // sleep(5);

    AOconf[loop].status = 7; // MULTIPLYING BY CONTROL MATRIX -> MODE VALUES


    //printf("init_CMc = %d\n", AOconf[loop].init_CMc);
    //fflush(stdout);
    if((AOconf[loop].init_CMc == 0)&&(MATRIX_COMPUTATION_MODE==1)) // compute combined control matrix or matrices
    {
        printf("COMPUTING COMBINED CONTROL MATRIX .... \n");
        fflush(stdout);
        list_image_ID();
        
        
        clock_gettime(CLOCK_REALTIME, &t1);



        printf("Build up map for active regions...\n");
        fflush(stdout);
        
        WFS_active_map = (int*) malloc(sizeof(int)*AOconf[loop].sizeWFS);
        IDmask = image_ID("wfsmask");
        if(IDmask==-1)
        {
            IDmask = create_2Dimage_ID("wfsmask", AOconf[loop].sizexWFS, AOconf[loop].sizeyWFS);
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                data.image[IDmask].array.F[ii] = 1.0;
        }

        if(IDmask != -1)
        {
            ii1 = 0;
            for(ii=0; ii<AOconf[loop].sizeWFS; ii++)
                if(data.image[IDmask].array.F[ii]>0.1)
                {
                    WFS_active_map[ii1] = ii;
                    ii1++;
                }
            AOconf[loop].sizeWFS_active = ii1;
        }
        aoconfID_WFS2_active = create_2Dimage_ID("wfs2active", AOconf[loop].sizeWFS_active, 1);


        DM_active_map = (int*) malloc(sizeof(int)*AOconf[loop].sizeDM);
        IDmask = image_ID("dmmask");
        if(IDmask==-1)
        {
            IDmask = create_2Dimage_ID("dmmask", AOconf[loop].sizexDM, AOconf[loop].sizeyDM);
            for(ii=0; ii<AOconf[loop].sizeDM; ii++)
                data.image[IDmask].array.F[ii] = 1.0;
        }
        if(IDmask != -1)
        {
            ii1 = 0;
            for(ii=0; ii<AOconf[loop].sizeDM; ii++)
                if(data.image[IDmask].array.F[ii]>0.1)
                {
                    DM_active_map[ii1] = ii;
                    ii1++;
                }
            AOconf[loop].sizeDM_active = ii1;
        }


        aoconfID_meas_act_active = create_2Dimage_ID("meas_act_active", AOconf[loop].sizeDM_active, 1);
       // printf("AOconf[loop].sizeDM_active = %ld\n", AOconf[loop].sizeDM_active);
       // exit(0);

        aoconfID_contrMc = image_ID("cmatc");
        aoconfID_contrMc_active = image_ID("cmatcact");

        if(aoconfID_contrMc==-1)
        {
            printf("Build  cmatc ...\n");
            fflush(stdout);
 
            sizearray = (long*) malloc(sizeof(long)*3);
            sizearray[0] = AOconf[loop].sizexWFS;
            sizearray[1] = AOconf[loop].sizeyWFS;
            sizearray[2] = AOconf[loop].sizeDM;
            aoconfID_contrMc = create_image_ID("cmatc", 3, sizearray, FLOAT, 1, 0);
            free(sizearray);



            // control matrix scaled by gains
            matrix_cmp = (float*) malloc(sizeof(float)*AOconf[loop].sizeWFS*AOconf[loop].sizeDM);
            for(mode=0; mode<AOconf[loop].NBDMmodes; mode++)
                for(wfselem=0; wfselem<AOconf[loop].sizeWFS; wfselem++)
                    matrix_cmp[mode*AOconf[loop].sizeWFS+wfselem] = data.image[aoconfID_contrM].array.F[mode*AOconf[loop].sizeWFS+wfselem]*data.image[aoconfID_GAIN_modes].array.F[mode];
            printf("\n");

            n_sizeDM = AOconf[loop].sizeDM;
            n_NBDMmodes = AOconf[loop].NBDMmodes;
            n_sizeWFS = AOconf[loop].sizeWFS;
            matrix_Mc = (float*) malloc(sizeof(float)*AOconf[loop].sizeWFS*AOconf[loop].sizeDM);
            memcpy(matrix_Mc, data.image[aoconfID_contrMc].array.F, sizeof(float)*AOconf[loop].sizeWFS*AOconf[loop].sizeDM);
            matrix_DMmodes = (float*) malloc(sizeof(float)*AOconf[loop].NBDMmodes*AOconf[loop].sizeDM);
            memcpy(matrix_DMmodes, data.image[aoconfID_DMmodes].array.F, sizeof(float)*AOconf[loop].NBDMmodes*AOconf[loop].sizeDM);

# ifdef _OPENMP
            #pragma omp parallel shared(matrix_Mc, matrix_cmp, matrix_DMmodes ,chunk) private( mode, act, wfselem)
            {
                #pragma omp for schedule (static)
# endif
                for(mode=0; mode<n_NBDMmodes; mode++)
                {
                 //   printf("mode %6ld    \n", mode);
                  //  fflush(stdout);
                    for(act=0; act<n_sizeDM; act++)
                        for(wfselem=0; wfselem<n_sizeWFS; wfselem++)
                            matrix_Mc[act*n_sizeWFS+wfselem] += matrix_cmp[mode*n_sizeWFS+wfselem]*matrix_DMmodes[mode*n_sizeDM+act];
                }
# ifdef _OPENMP
            }
# endif
            memcpy(data.image[aoconfID_contrMc].array.F, matrix_Mc, sizeof(float)*AOconf[loop].sizeWFS*AOconf[loop].sizeDM);
            free(matrix_Mc);
            free(matrix_DMmodes);
      }

        if(aoconfID_contrMc_active==-1)
        {
            printf("Build  cmatcactive ...\n");
            fflush(stdout);
            
            matrix_Mc = (float*) malloc(sizeof(float)*AOconf[loop].sizeWFS*AOconf[loop].sizeDM);
            memcpy(matrix_Mc, data.image[aoconfID_contrMc].array.F, sizeof(float)*AOconf[loop].sizeWFS*AOconf[loop].sizeDM);
          
            aoconfID_contrMc_active = create_3Dimage_ID("cmatc_active", AOconf[loop].sizeWFS_active, 1, AOconf[loop].sizeDM_active);
            
            list_image_ID();
            n_sizeWFS = AOconf[loop].sizeWFS;
            
            for(act_active=0; act_active<AOconf[loop].sizeDM_active; act_active++)
            {
                for(wfselem_active=0; wfselem_active<AOconf[loop].sizeWFS_active; wfselem_active++)
                {
                    act = DM_active_map[act_active];
                    wfselem = WFS_active_map[wfselem_active];
                 //   printf("[%ld]  %ld / %ld -> %ld / %ld     %ld / %ld  -> %ld / %ld\n", aoconfID_contrMc_active, act_active, AOconf[loop].sizeDM_active, act, AOconf[loop].sizeDM, wfselem_active, AOconf[loop].sizeWFS_active, wfselem, AOconf[loop].sizeWFS);
                   // fflush(stdout);
                    data.image[aoconfID_contrMc_active].array.F[act_active*AOconf[loop].sizeWFS_active+wfselem_active] = matrix_Mc[act*n_sizeWFS+wfselem];
                }
            }
            free(matrix_Mc);
 
            printf("Keeping only active pixels / actuators : %ld x %ld   ->   %ld x %ld\n", AOconf[loop].sizeWFS, AOconf[loop].sizeDM, AOconf[loop].sizeWFS_active, AOconf[loop].sizeDM_active);
            fflush(stdout);
        }
        else
        {
            printf("USING PRE_COMPUTED COMBINED ACTIVE CONTROL MATRIX \n");
        }


        if(aoconfID_meas_act==-1)
        {
            sizearray = (long*) malloc(sizeof(long)*2);
            sizearray[0] = AOconf[loop].sizexDM;
            sizearray[1] = AOconf[loop].sizeyDM;
            aoconfID_meas_act = create_image_ID("measact", 2, sizearray, FLOAT, 1, 0);
            free(sizearray);
        }
        save_fits("cmatc", "!cmatc.fits");
        AOconf[loop].init_CMc = 1;
        printf(" done\n");
        fflush(stdout);

        clock_gettime(CLOCK_REALTIME, &t2);
        tdiff = info_time_diff(t1, t2);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
        printf("\n");
        printf("TIME TO COMPUTE MATRIX = %f sec\n", tdiffv);
        
        printf("CREATING SHARED MEMORY CMATCACT  from ID %ld\n", aoconfID_contrMc_active);
        sizearray = (long*) malloc(sizeof(long)*2);
        
        sizearray[0] = data.image[aoconfID_contrMc_active].md[0].size[0];
        if(data.image[aoconfID_contrMc_active].md[0].naxis==2)
            sizearray[1] = data.image[aoconfID_contrMc_active].md[0].size[1];
        else
            sizearray[1] = data.image[aoconfID_contrMc_active].md[0].size[1]*data.image[aoconfID_contrMc_active].md[0].size[2];
        sprintf(imname, "aol%ld_cmatca", loop);
        IDcmatca_shm = create_image_ID(imname, 2, sizearray, FLOAT, 1, 0);
        free(sizearray);
        memcpy(data.image[IDcmatca_shm].array.F, data.image[aoconfID_contrMc_active].array.F, sizeof(float)*data.image[aoconfID_contrMc_active].md[0].size[0]*data.image[aoconfID_contrMc_active].md[0].size[1]);
    }


    if(AOconf[loop].GPU == 0)
    {
        if(MATRIX_COMPUTATION_MODE==0)  // goes explicitely through modes, slow but useful for tuning
        {
            ControlMatrixMultiply( data.image[aoconfID_contrM].array.F, data.image[aoconfID_WFS2].array.F, AOconf[loop].NBDMmodes, AOconf[loop].sizeWFS, data.image[aoconfID_meas_modes].array.F);
            data.image[aoconfID_meas_modes].md[0].cnt0 ++;
        }
        else
        {
            ControlMatrixMultiply( data.image[aoconfID_contrMc].array.F, data.image[aoconfID_WFS2].array.F, AOconf[loop].sizeDM, AOconf[loop].sizeWFS, data.image[aoconfID_meas_act].array.F);
            data.image[aoconfID_meas_modes].md[0].cnt0 ++;
        }
    }
    else
    {
#ifdef HAVE_CUDA
        if(MATRIX_COMPUTATION_MODE==0)  // goes explicitely through modes, slow but useful for tuning
        {
            GPU_loop_MultMat_setup(0, data.image[aoconfID_contrM].md[0].name, data.image[aoconfID_WFS2].md[0].name, data.image[aoconfID_meas_modes].md[0].name, AOconf[loop].GPU, 0, AOconf[loop].GPUusesem, 1);
            AOconf[loop].status = 8; // execute
            GPU_loop_MultMat_execute(0, &AOconf[loop].status, &AOconf[loop].GPUstatus[0], 1.0, 0.0);
        }
        else // direct pixel -> actuators linear transformation
        {
            if(1==0)
            {
                GPU_loop_MultMat_setup(0, data.image[aoconfID_contrMc].md[0].name, data.image[aoconfID_WFS2].md[0].name, data.image[aoconfID_meas_act].md[0].name, AOconf[loop].GPU, 0, AOconf[loop].GPUusesem, 1);
                AOconf[loop].status = 8; // execute
                GPU_loop_MultMat_execute(0, &AOconf[loop].status, &AOconf[loop].GPUstatus[0], 1.0, 0.0);
            }
            else // only use active pixels and actuators
            {
                // re-map input vector

                if(COMPUTE_GPU_SCALING==1)
                {
                    // save reference (TEST)
                   // save_fits(data.image[aoconfID_WFS0].md[0].name, "!test_imtogpu.fits");
                    
                    for(wfselem_active=0; wfselem_active<AOconf[loop].sizeWFS_active; wfselem_active++)
                        data.image[aoconfID_WFS2_active].array.F[wfselem_active] = data.image[aoconfID_WFS0].array.F[WFS_active_map[wfselem_active]];
                    data.image[aoconfID_WFS2_active].md[0].cnt0++;
                }
                else
                {
                    for(wfselem_active=0; wfselem_active<AOconf[loop].sizeWFS_active; wfselem_active++)
                        data.image[aoconfID_WFS2_active].array.F[wfselem_active] = data.image[aoconfID_WFS2].array.F[WFS_active_map[wfselem_active]];
                    data.image[aoconfID_WFS2_active].md[0].cnt0++;
                }
                //printf("Vector wfs re-mapped\n");

                if(COMPUTE_GPU_SCALING==1)
                    {
                        if(data.image[aoconfID_refWFS].md[0].cnt0 != refWFScnt0)
                            {
                                printf("NEW REFERENCE WFS DETECTED  [ %ld %ld ]\n", data.image[aoconfID_refWFS].md[0].cnt0, refWFScnt0);
                                fflush(stdout);
                                initWFSref_GPU = 0;
                                refWFScnt0 = data.image[aoconfID_refWFS].md[0].cnt0;
                            }
                        if(initWFSref_GPU==0) // initialize WFS reference
                        {           
                       //     imtot = 0.0;                 
                            
                            // save reference (TEST)
                        //    save_fits(data.image[aoconfID_refWFS].md[0].name, "!test_reftogpu.fits");
                            
                            for(wfselem_active=0; wfselem_active<AOconf[loop].sizeWFS_active; wfselem_active++)
                                {
                                    data.image[aoconfID_WFS2_active].array.F[wfselem_active] = data.image[aoconfID_refWFS].array.F[WFS_active_map[wfselem_active]];
                     //               imtot += data.image[aoconfID_refWFS].array.F[wfselem_active];
                                }
                            data.image[aoconfID_WFS2_active].md[0].cnt0++;
                         //   printf("imtot = %g\n", imtot);
                            fflush(stdout);
                        }
                    }

                // perform matrix mult
                //GPU_loop_MultMat_setup(0, data.image[aoconfID_contrMc_active].md[0].name, data.image[aoconfID_WFS2_active].md[0].name, data.image[aoconfID_meas_act_active].md[0].name, AOconf[loop].GPU, 0, AOconf[loop].GPUusesem);
               
                
            /*    list_image_ID();
                printf("%s %s %s\n", data.image[IDcmatca_shm].md[0].name, data.image[aoconfID_WFS2_active].md[0].name, data.image[aoconfID_meas_act_active].md[0].name);
                exit(0); // TESTING
              */  
                
                GPU_loop_MultMat_setup(0, data.image[IDcmatca_shm].md[0].name, data.image[aoconfID_WFS2_active].md[0].name, data.image[aoconfID_meas_act_active].md[0].name, AOconf[loop].GPU, 0, AOconf[loop].GPUusesem, initWFSref_GPU );
                
                
                initWFSref_GPU = 1;
                AOconf[loop].status = 8; // execute

                if(COMPUTE_GPU_SCALING==1)
                {
                    GPU_loop_MultMat_execute(0, &AOconf[loop].status, &AOconf[loop].GPUstatus[0], GPU_alpha, GPU_beta);
                }
                else
                    GPU_loop_MultMat_execute(0, &AOconf[loop].status, &AOconf[loop].GPUstatus[0], 1.0, 0.0);


                // re-map output vector
                for(act_active=0; act_active<AOconf[loop].sizeDM_active; act_active++)
                    data.image[aoconfID_meas_act].array.F[DM_active_map[act_active]] = data.image[aoconfID_meas_act_active].array.F[act_active];
            }

        }
#endif
    }

    AOconf[loop].status = 13; // MULTIPLYING BY GAINS
    if(MATRIX_COMPUTATION_MODE==0)
    {
        AOconf[loop].RMSmodes = 0;
        for(k=0; k<AOconf[loop].NBDMmodes; k++)
            AOconf[loop].RMSmodes += data.image[aoconfID_meas_modes].array.F[k]*data.image[aoconfID_meas_modes].array.F[k];

        AOconf[loop].RMSmodesCumul += AOconf[loop].RMSmodes;
        AOconf[loop].RMSmodesCumulcnt ++;


        for(k=0; k<AOconf[loop].NBDMmodes; k++)
        {
            data.image[aoconfID_RMS_modes].array.F[k] = 0.99*data.image[aoconfID_RMS_modes].array.F[k] + 0.01*data.image[aoconfID_meas_modes].array.F[k]*data.image[aoconfID_meas_modes].array.F[k];
            data.image[aoconfID_AVE_modes].array.F[k] = 0.99*data.image[aoconfID_AVE_modes].array.F[k] + 0.01*data.image[aoconfID_meas_modes].array.F[k];

            data.image[aoconfID_cmd_modes].array.F[k] -= AOconf[loop].gain * data.image[aoconfID_GAIN_modes].array.F[k] * data.image[aoconfID_meas_modes].array.F[k];

            if(data.image[aoconfID_cmd_modes].array.F[k] < -AOconf[loop].maxlimit * data.image[aoconfID_LIMIT_modes].array.F[k])
                data.image[aoconfID_cmd_modes].array.F[k] = -AOconf[loop].maxlimit * data.image[aoconfID_LIMIT_modes].array.F[k];

            if(data.image[aoconfID_cmd_modes].array.F[k] > AOconf[loop].maxlimit * data.image[aoconfID_LIMIT_modes].array.F[k])
                data.image[aoconfID_cmd_modes].array.F[k] = AOconf[loop].maxlimit * data.image[aoconfID_LIMIT_modes].array.F[k];

            data.image[aoconfID_cmd_modes].array.F[k] *= AOconf[loop].mult * data.image[aoconfID_MULTF_modes].array.F[k];

            // update total gain
            //     data.image[aoconfID_GAIN_modes].array.F[k+AOconf[loop].NBDMmodes] = AOconf[loop].gain * data.image[aoconfID_GAIN_modes].array.F[k];
        }


        data.image[aoconfID_cmd_modes].md[0].cnt0 ++;
    }


    return(0);
}






///
/// main routine
///

int AOloopControl_run()
{
    FILE *fp;
    char fname[200];
    long loop;
    int vOK;
    long ii;
    long ID;
    long j, m;
    struct tm *uttime;
    time_t t;
    struct timespec *thetime = (struct timespec *)malloc(sizeof(struct timespec));
    char logfname[1000];
    char command[1000];
    int r;
    int RT_priority = 90; //any number from 0-99
    struct sched_param schedpar;
    double a;
    long cnttest;
    float tmpf1;


    schedpar.sched_priority = RT_priority;
    // r = seteuid(euid_called); //This goes up to maximum privileges
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    // r = seteuid(euid_real);//Go back to normal privileges

    loop = LOOPNUMBER;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(0);

    

    printf("SETTING UP...\n");
 //   sprintf(fname, "./conf/AOloop.conf");
    AOloopControl_loadconfigure(LOOPNUMBER, 1);

    COMPUTE_GPU_SCALING = AOconf[loop].GPUall; 

    vOK = 1;
    if(AOconf[loop].init_refWFS==0)
    {
        printf("ERROR: CANNOT RUN LOOP WITHOUT WFS REFERENCE\n");
        vOK = 0;
    }
    if(AOconf[loop].init_CM==0)
    {
        printf("ERROR: CANNOT RUN LOOP WITHOUT CONTROL MATRIX\n");
        vOK = 0;
    }

    AOconf[loop].init_CMc = 0;

    if(vOK==1)
    {

        AOconf[loop].kill = 0;
        AOconf[loop].on = 0;
        printf("\n");
        while( AOconf[loop].kill == 0)
        {
            printf(" WAITING                    \r");
            fflush(stdout);
            usleep(1000);



            while(AOconf[loop].on == 1)
            {
                AOcompute(loop);

                AOconf[loop].status = 14;

                if(MATRIX_COMPUTATION_MODE==0)  // 2-step : WFS -> mode coeffs -> DM act
                {
                    if(fabs(AOconf[loop].gain)>1.0e-6)
                        set_DM_modes(loop);
                }
                else // 1 step: WFS -> DM act
                {
                    data.image[aoconfID_DM].md[0].write = 1;

                    for(ii=0; ii<AOconf[loop].sizeDM; ii++)
                    {
                        data.image[aoconfID_DM].array.F[ii] -= AOconf[loop].gain * data.image[aoconfID_meas_act].array.F[ii];
                        data.image[aoconfID_DM].array.F[ii] *= AOconf[loop].mult;
                    }
                    if(data.image[aoconfID_DM].sem == 1)
                        sem_post(data.image[aoconfID_DM].semptr);
                    data.image[aoconfID_DM].md[0].cnt0++;
                    data.image[aoconfID_DM].md[0].write = 0;
                    // inform dmdisp that new command is ready in one of the channels
                    if(aoconfID_DMdisp!=-1)
                        if(data.image[aoconfID_DMdisp].sem1 == 1)
                            sem_post(data.image[aoconfID_DMdisp].semptr1);

                    AOconf[loop].DMupdatecnt ++;
                }

                AOconf[loop].status = 20; //  LOGGING, part 1
                AOconf[loop].status = 21; //  (13->) LOGGING, part 2
                AOconf[loop].cnt++;

                data.image[AOconf[loop].logdataID].md[0].cnt0 = AOconf[loop].cnt;
                data.image[AOconf[loop].logdataID].array.F[0] = AOconf[loop].gain;


                if(AOconf[loop].cnt == AOconf[loop].cntmax)
                    AOconf[loop].on = 0;
            }

        }
    }

    free(thetime);


    return(0);
}










int AOloopControl_printloopstatus(long loop, long nbcol)
{
    long k, kmax;
    long col;
    float val;
    long nbl = 0;
    float AVElim = 0.01;
    float RMSlim = 0.01;

    printw("loop number %ld    ", loop);


    if(AOconf[loop].on == 1)
        printw("loop is ON     ");
    else
        printw("loop is OFF    ");

    /*  if(AOconf[loop].logon == 1)
          printw("log is ON   ");
      else
          printw("log is OFF  ");

    */


    printw("STATUS = %d  ", AOconf[loop].status);

    kmax = (wrow-3)*(nbcol);
    printw("Gain = %f   maxlim = %f     GPU = %d    kmax=%ld\n", AOconf[loop].gain, AOconf[loop].maxlimit, AOconf[loop].GPU, kmax);
    printw("WFS norm floor = %f\n", AOconf[loop].WFSnormfloor);
    nbl++;

    printw("CNT : %lld  / %lld\n", AOconf[loop].cnt, AOconf[loop].cntmax);
    nbl++;




    for(k=0; k<AOconf[loop].NBMblocks; k++)
    {
        if(k==0)
            printw("MODE BLOCK %ld   [ %4ld - %4ld ]  %4.2f  %4.2f  %4.2f\n", k, (long) 0, AOconf[loop].indexmaxMB[k], AOconf[loop].gainMB[k], AOconf[loop].limitMB[k], AOconf[loop].multfMB[k]);
        else
            printw("MODE BLOCK %ld   [ %4ld - %4ld ]  %4.2f  %4.2f  %4.2f\n", k, AOconf[loop].indexmaxMB[k-1], AOconf[loop].indexmaxMB[k], AOconf[loop].gainMB[k], AOconf[loop].limitMB[k], AOconf[loop].multfMB[k]);
        nbl++;
    }


    printw("            MODAL RMS (ALL MODES) : %6.4lf     AVERAGE :  %8.6lf       ( %20g / %8lld )\n", sqrt(AOconf[loop].RMSmodes), sqrt(AOconf[loop].RMSmodesCumul/AOconf[loop].RMSmodesCumulcnt), AOconf[loop].RMSmodesCumul, AOconf[loop].RMSmodesCumulcnt);


    print_header(" MODES ", '-');
    nbl++;




    if(kmax>AOconf[loop].NBDMmodes)
        kmax = AOconf[loop].NBDMmodes;

    col = 0;
    for(k=0; k<kmax; k++)
    {
        attron(A_BOLD);
        printw("%4ld ", k);
        attroff(A_BOLD);

        printw("[%4.2f %4.2f %5.3f] ", data.image[aoconfID_GAIN_modes].array.F[k], data.image[aoconfID_LIMIT_modes].array.F[k], data.image[aoconfID_MULTF_modes].array.F[k]);

        // print current value on DM
        val = data.image[aoconfID_cmd_modes].array.F[k];
        if(fabs(val)>0.99*AOconf[loop].maxlimit)
        {
            attron(A_BOLD | COLOR_PAIR(2));
            printw("%7.4f ", val);
            attroff(A_BOLD | COLOR_PAIR(2));
        }
        else
        {
            if(fabs(val)>0.99*AOconf[loop].maxlimit*data.image[aoconfID_LIMIT_modes].array.F[k])
            {
                attron(COLOR_PAIR(1));
                printw("%7.4f ", val);
                attroff(COLOR_PAIR(1));
            }
            else
                printw("%7.4f ", val);
        }

        // last reading from WFS
        printw("%7.4f ", data.image[aoconfID_meas_modes].array.F[k]);


        // Time average
        val = data.image[aoconfID_AVE_modes].array.F[k];
        if(fabs(val)>AVElim)
        {
            attron(A_BOLD | COLOR_PAIR(2));
            printw("%7.4f ", val);
            attroff(A_BOLD | COLOR_PAIR(2));
        }
        else
            printw("%7.4f ", val);


        // RMS variation
        val = data.image[aoconfID_RMS_modes].array.F[k];
        if(fabs(val)>RMSlim)
        {
            attron(A_BOLD | COLOR_PAIR(2));
            printw("%7.4f ", val);
            attroff(A_BOLD | COLOR_PAIR(2));
        }
        else
            printw("%7.4f ", val);

        col++;
        if(col==nbcol)
        {
            col = 0;
            printw("\n");
        }
        else
            printw(" | ");
    }


    return(0);
}





int AOloopControl_loopMonitor(long loop, double frequ, long nbcol)
{
    char name[200];

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    printf("MEMORY HAS BEEN INITIALIZED\n");
    fflush(stdout);

    // load arrays that are required
    if(aoconfID_cmd_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_cmd", loop);
        aoconfID_cmd_modes = read_sharedmem_image(name);
    }

    if(aoconfID_meas_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_meas", loop);
        aoconfID_meas_modes = read_sharedmem_image(name);
    }


    if(aoconfID_RMS_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_RMS", loop);
        aoconfID_RMS_modes = read_sharedmem_image(name);
    }

    if(aoconfID_AVE_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_AVE", loop);
        aoconfID_AVE_modes = read_sharedmem_image(name);
    }

    if(aoconfID_GAIN_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_GAIN", loop);
        aoconfID_GAIN_modes = read_sharedmem_image(name);
    }

    if(aoconfID_LIMIT_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_LIMIT", loop);
        aoconfID_LIMIT_modes = read_sharedmem_image(name);
    }

    if(aoconfID_MULTF_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_MULTF", loop);
        aoconfID_MULTF_modes = read_sharedmem_image(name);
    }


    initscr();
    getmaxyx(stdscr, wrow, wcol);


    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    while( !kbdhit() )
    {
        usleep((long) (1000000.0/frequ));
        clear();
        attron(A_BOLD);
        print_header(" PRESS ANY KEY TO STOP MONITOR ", '-');
        attroff(A_BOLD);

        AOloopControl_printloopstatus(loop, nbcol);

        refresh();
    }
    endwin();

    return 0;
}




int AOloopControl_statusStats()
{
    long k;
    long NBkiter = 200000;
    long statusmax = 22;
    long *statuscnt;
    float usec0, usec1;
    int st;
    int RT_priority = 91; //any number from 0-99
    struct sched_param schedpar;
    const char *statusdef[22];
    int gpu;
    int nbgpu;
    struct timespec t1;
    struct timespec t2;
    struct timespec tdiff;
    double tdiffv;
    long *statusgpucnt;
    long *statusgpucnt2;
    double loopiterus;
    long long loopcnt;

    statusdef[0] = "";
    statusdef[1] = "READING IMAGE";
    statusdef[2] = "WAIT FOR IMAGE";
    statusdef[3] = "DARK SUBTRACT";
    statusdef[4] = "COMPUTE WFS IMAGE TOTAL";
    statusdef[5] = "NORMALIZE WFS IMAGE";
    statusdef[6] = "SUBTRACT REFERENCE";
    statusdef[7] = "MULTIPLYING BY CONTROL MATRIX -> MODE VALUES : SETUP";
    statusdef[8] = "START CONTROL MATRIX MULTIPLICATION: CHECK IF NEW CM EXISTS";
    statusdef[9] = "CONTROL MATRIX MULT: CREATE COMPUTING THREADS";
    statusdef[10] = "CONTROL MATRIX MULT: WAIT FOR THREADS TO COMPLETE";
    statusdef[11] = "CONTROL MATRIX MULT: COMBINE TRHEADS RESULTS";
    statusdef[12] = "CONTROL MATRIX MULT: INCREMENT COUNTER AND EXIT FUNCTION";
    statusdef[13] = "MULTIPLYING BY GAINS";
    statusdef[14] = "ENTER SET DM MODES";
    statusdef[15] = "START DM MODES MATRIX MULTIPLICATION";
    statusdef[16] = "MATRIX MULT: CREATE COMPUTING THREADS";
    statusdef[17] = "MATRIX MULT: WAIT FOR THREADS TO COMPLETE";
    statusdef[18] = "MATRIX MULT: COMBINE TRHEADS RESULTS";
    statusdef[19] = "MATRIX MULT: INCREMENT COUNTER AND EXIT FUNCTION";
    statusdef[20] = "LOG DATA, PART 1";
    statusdef[21] = "LOG DATA, PART 2";

    usec0 = 50.0;
    usec1 = 150.0;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    schedpar.sched_priority = RT_priority;
    sched_setscheduler(0, SCHED_FIFO, &schedpar);

    nbgpu = AOconf[LOOPNUMBER].GPU;


    printf("Measuring loop status distribution \n");
    fflush(stdout);

    statuscnt = (long*) malloc(sizeof(long)*statusmax);
    statusgpucnt = (long*) malloc(sizeof(long)*nbgpu*10);
    statusgpucnt2 = (long*) malloc(sizeof(long)*nbgpu*10);


    for(st=0; st<statusmax; st++)
        statuscnt[st] = 0;

    for(st=0; st<nbgpu*10; st++)
    {
        statusgpucnt[st] = 0;
        statusgpucnt2[st] = 0;
    }

    loopcnt = AOconf[LOOPNUMBER].cnt;
    clock_gettime(CLOCK_REALTIME, &t1);
    for(k=0; k<NBkiter; k++)
    {
        usleep((long) (usec0+usec1*(1.0*k/NBkiter)));
        st = AOconf[LOOPNUMBER].status;
        if(st<statusmax)
            statuscnt[st]++;
        for(gpu=0; gpu<AOconf[LOOPNUMBER].GPU; gpu++)
        {
            // 1st matrix mult
            st = 10*gpu + AOconf[LOOPNUMBER].GPUstatus[gpu];
            statusgpucnt[st]++;

            // 2nd matrix mult
            st = 10*gpu + AOconf[LOOPNUMBER].GPUstatus[10+gpu];
            statusgpucnt2[st]++;
        }
    }
    loopcnt = AOconf[LOOPNUMBER].cnt - loopcnt;
    clock_gettime(CLOCK_REALTIME, &t2);
    tdiff = info_time_diff(t1, t2);
    tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
    printf("\n");
    loopiterus = 1.0e6*tdiffv/loopcnt;
    printf("Time diff = %f sec \n", tdiffv);
    printf("Loop freq = %8.2f Hz   -> single interation = %8.3f us\n", 1.0*loopcnt/tdiffv, loopiterus);
    printf("\n");

    for(st=0; st<statusmax; st++)
        printf("STATUS %2d     %5.2f %%    [   %6ld  /  %6ld  ]   [ %9.3f us] %s\n", st, 100.0*statuscnt[st]/NBkiter, statuscnt[st], NBkiter, loopiterus*statuscnt[st]/NBkiter , statusdef[st]);


    if(AOconf[LOOPNUMBER].GPU!=0)
    {
        printf("\n");
        printf("          ----1--------2--------3--------4--------5--------6----\n");
        printf("                   wait im | ->GPU |     COMPUTE     |   ->CPU  \n");
        printf("          ------------------------------------------------------\n");

        for(gpu=0; gpu<AOconf[LOOPNUMBER].GPU; gpu++)
        {
            printf("GPU %2d  : ", gpu);
            printf("  %5.2f %%",  100.0*statusgpucnt[10*gpu+1]/NBkiter);
            printf("  %5.2f %%",  100.0*statusgpucnt[10*gpu+2]/NBkiter);
            printf("  %5.2f %%",  100.0*statusgpucnt[10*gpu+3]/NBkiter);
            printf("  %5.2f %%",  100.0*statusgpucnt[10*gpu+4]/NBkiter);
            printf("  %5.2f %%",   100.0*statusgpucnt[10*gpu+5]/NBkiter);
            printf("  %5.2f %%\n",  100.0*statusgpucnt[10*gpu+6]/NBkiter);
        }
        for(gpu=0; gpu<AOconf[LOOPNUMBER].GPU; gpu++)
        {
            printf("GPU %2d  : ", gpu);
            printf(" %5.2f us",  loopiterus*statusgpucnt[10*gpu+1]/NBkiter);
            printf(" %5.2f us",  loopiterus*statusgpucnt[10*gpu+2]/NBkiter);
            printf(" %5.2f us",  loopiterus*statusgpucnt[10*gpu+3]/NBkiter);
            printf(" %5.2f us",  loopiterus*statusgpucnt[10*gpu+4]/NBkiter);
            printf(" %5.2f us",   loopiterus*statusgpucnt[10*gpu+5]/NBkiter);
            printf(" %5.2f us\n",  loopiterus*statusgpucnt[10*gpu+6]/NBkiter);
        }

        printf("\n");
        if(MATRIX_COMPUTATION_MODE == 0)
        {
            printf("          ----1--------2--------3--------4--------5--------6----\n");
            for(gpu=0; gpu<AOconf[LOOPNUMBER].GPU; gpu++)
            {
                printf("GPU %2d  : ", gpu);
                printf("  %5.2f %%",  100.0*statusgpucnt2[10*gpu+1]/NBkiter);
                printf("  %5.2f %%",  100.0*statusgpucnt2[10*gpu+2]/NBkiter);
                printf("  %5.2f %%",  100.0*statusgpucnt2[10*gpu+3]/NBkiter);
                printf("  %5.2f %%",  100.0*statusgpucnt2[10*gpu+4]/NBkiter);
                printf("  %5.2f %%",   100.0*statusgpucnt2[10*gpu+5]/NBkiter);
                printf("  %5.2f %%\n",  100.0*statusgpucnt2[10*gpu+6]/NBkiter);
            }
        }
    }
    free(statuscnt);
    free(statusgpucnt);
    free(statusgpucnt2);


    return 0;
}








int AOloopControl_showparams(long loop)
{
  printf("loop number %ld\n", loop);

  if(AOconf[loop].on == 1)
    printf("loop is ON\n");
  else
    printf("loop is OFF\n");
    
    printf("Gain = %f   maxlim = %f\n  multcoeff = %f  GPU = %d\n", AOconf[loop].gain, AOconf[loop].maxlimit, AOconf[loop].mult, AOconf[loop].GPU);
    printf("WFS norm floor = %f\n", AOconf[loop].WFSnormfloor);

  return 0;
}



int AOloopControl_setLoopNumber(long loop)
{
  printf("LOOPNUMBER = %ld\n", loop);
  LOOPNUMBER = loop;
  
  /** append process name with loop number */
  

  return 0;
}



int AOloopControl_loopkill()
{
    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    AOconf[LOOPNUMBER].kill = 1;

    return 0;
}





int AOloopControl_loopon()
{
    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    AOconf[LOOPNUMBER].cntmax = AOconf[LOOPNUMBER].cnt-1;

    AOconf[LOOPNUMBER].on = 1;
    AOloopControl_showparams(LOOPNUMBER);

    return 0;
}




int AOloopControl_loopstep(long loop, long NBstep)
{
    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    AOconf[loop].cntmax = AOconf[loop].cnt + NBstep;
    AOconf[LOOPNUMBER].RMSmodesCumul = 0.0;
    AOconf[LOOPNUMBER].RMSmodesCumulcnt = 0;

    //  printf("\nLOOP %ld STEP    %lld %ld %lld\n\n", loop, AOconf[loop].cnt, NBstep, AOconf[loop].cntmax);
    //fflush(stdout);

    AOconf[loop].on = 1;

    while(AOconf[loop].on==1)
        usleep(100); // THIS WAITING IS OK

    // AOloopControl_showparams(loop);

    return 0;
}





int AOloopControl_loopoff()
{
    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    AOconf[LOOPNUMBER].on = 0;
    AOloopControl_showparams(LOOPNUMBER);

    return 0;
}




int AOloopControl_loopreset()
{
    char name[200];
    long k;
    long mb;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_cmd_modes==-1)
    {
        sprintf(name, "DMmode_cmd_%ld", LOOPNUMBER);
        aoconfID_cmd_modes = read_sharedmem_image(name);
    }

    AOconf[LOOPNUMBER].on = 0;
    for(k=0; k<AOconf[LOOPNUMBER].NBDMmodes; k++)
        data.image[aoconfID_cmd_modes].array.F[k] = 0.0;

    for(mb=0; mb<AOconf[LOOPNUMBER].NBMblocks; mb)
    {
        AOloopControl_setgainblock(mb, 0.0);
        AOloopControl_setlimitblock(mb, 0.01);
        AOloopControl_setmultfblock(mb, 0.95);
    }

    return 0;
}





int AOloopControl_setgain(float gain)
{
  if(AOloopcontrol_meminit==0)
    AOloopControl_InitializeMemory(1);

  AOconf[LOOPNUMBER].gain = gain;
  AOloopControl_showparams(LOOPNUMBER);

  return 0;
}

int AOloopControl_setWFSnormfloor(float WFSnormfloor)
{
    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    AOconf[LOOPNUMBER].WFSnormfloor = WFSnormfloor;
    printf("SHOWING PARAMETERS ...\n");
    fflush(stdout);
    AOloopControl_showparams(LOOPNUMBER);
    printf("DONE ...\n");
    fflush(stdout);
   
    return 0;
}


int AOloopControl_setmaxlimit(float maxlimit)
{
  if(AOloopcontrol_meminit==0)
    AOloopControl_InitializeMemory(1);

  AOconf[LOOPNUMBER].maxlimit = maxlimit;
  AOloopControl_showparams(LOOPNUMBER);

  return 0;
}

int AOloopControl_setmult(float multcoeff)
{
  if(AOloopcontrol_meminit==0)
    AOloopControl_InitializeMemory(1);

  AOconf[LOOPNUMBER].mult = multcoeff;
  AOloopControl_showparams(LOOPNUMBER);

  return 0;
}


int AOloopControl_setframesAve(long nbframes)
{
  if(AOloopcontrol_meminit==0)
    AOloopControl_InitializeMemory(1);

  AOconf[LOOPNUMBER].framesAve = nbframes;
  AOloopControl_showparams(LOOPNUMBER);

  return 0;
}



int AOloopControl_setgainrange(long m0, long m1, float gainval)
{
    long k;
    long kmax;
    char name[200];

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_GAIN_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_GAIN", LOOPNUMBER);
        aoconfID_GAIN_modes = read_sharedmem_image(name);
    }

    kmax = m1+1;
    if(kmax>AOconf[LOOPNUMBER].NBDMmodes)
        kmax = AOconf[LOOPNUMBER].NBDMmodes-1;

    for(k=m0; k<kmax; k++)
        data.image[aoconfID_GAIN_modes].array.F[k] = gainval;

    return 0;
}




int AOloopControl_setlimitrange(long m0, long m1, float limval)
{
    long k;
    long kmax;
    char name[200];

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_LIMIT_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_LIMIT", LOOPNUMBER);
        aoconfID_LIMIT_modes = read_sharedmem_image(name);
    }

    kmax = m1+1;
    if(kmax>AOconf[LOOPNUMBER].NBDMmodes)
        kmax = AOconf[LOOPNUMBER].NBDMmodes-1;

    for(k=m0; k<kmax; k++)
        data.image[aoconfID_LIMIT_modes].array.F[k] = limval;

    return 0;
}



int AOloopControl_setmultfrange(long m0, long m1, float multfval)
{
    long k;
    long kmax;
    char name[200];

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_MULTF_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_MULTF", LOOPNUMBER);
        aoconfID_MULTF_modes = read_sharedmem_image(name);
    }

    kmax = m1+1;
    if(kmax>AOconf[LOOPNUMBER].NBDMmodes)
        kmax = AOconf[LOOPNUMBER].NBDMmodes-1;

    for(k=m0; k<kmax; k++)
        data.image[aoconfID_MULTF_modes].array.F[k] = multfval;

    return 0;
}



int AOloopControl_setgainblock(long mb, float gainval)
{
    long k;
    char name[200];
    long kmin, kmax;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_GAIN_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_GAIN", LOOPNUMBER);
        aoconfID_GAIN_modes = read_sharedmem_image(name);
    }

    if(mb<AOconf[LOOPNUMBER].NBMblocks)
    {
        if(mb==0)
            kmin = 0;
        else
            kmin = AOconf[LOOPNUMBER].indexmaxMB[mb-1];
        kmax = AOconf[LOOPNUMBER].indexmaxMB[mb];

        AOconf[LOOPNUMBER].gainMB[mb] = gainval;

        for(k=kmin; k<kmax; k++)
            data.image[aoconfID_GAIN_modes].array.F[k] = gainval;
    }

    return 0;
}



int AOloopControl_setlimitblock(long mb, float limitval)
{
    long k;
    char name[200];
    long kmin, kmax;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_LIMIT_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_LIMIT", LOOPNUMBER);
        aoconfID_LIMIT_modes = read_sharedmem_image(name);
    }

    if(mb<AOconf[LOOPNUMBER].NBMblocks)
    {
        if(mb==0)
            kmin = 0;
        else
            kmin = AOconf[LOOPNUMBER].indexmaxMB[mb-1];
        kmax = AOconf[LOOPNUMBER].indexmaxMB[mb];

        AOconf[LOOPNUMBER].limitMB[mb] = limitval;

        for(k=kmin; k<kmax; k++)
            data.image[aoconfID_LIMIT_modes].array.F[k] = limitval;
    }

    return 0;
}



int AOloopControl_setmultfblock(long mb, float multfval)
{
    long k;
    char name[200];
    long kmin, kmax;

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_MULTF_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_MULTF", LOOPNUMBER);
        aoconfID_MULTF_modes = read_sharedmem_image(name);
    }

    if(mb<AOconf[LOOPNUMBER].NBMblocks)
    {
        if(mb==0)
            kmin = 0;
        else
            kmin = AOconf[LOOPNUMBER].indexmaxMB[mb-1];
        kmax = AOconf[LOOPNUMBER].indexmaxMB[mb];

        AOconf[LOOPNUMBER].multfMB[mb] = multfval;

        for(k=kmin; k<kmax; k++)
            data.image[aoconfID_MULTF_modes].array.F[k] = multfval;
    }

    return 0;
}





int AOloopControl_resetRMSperf()
{
  long k;
  char name[200];
  long kmin, kmax;

  if(AOloopcontrol_meminit==0)
    AOloopControl_InitializeMemory(1);

  AOconf[LOOPNUMBER].RMSmodesCumul = 0.0;
  AOconf[LOOPNUMBER].RMSmodesCumulcnt = 0;

  return 0;
}



int AOloopControl_scanGainBlock(long NBblock, long NBstep, float gainStart, float gainEnd, long NBgain)
{
    long k, kg;
    float gain;
    float bestgain= 0.0;
    float bestval = 10000000.0;
    float val;
    char name[200];


    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_cmd_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_cmd", LOOPNUMBER);
        aoconfID_cmd_modes = read_sharedmem_image(name);
    }


    printf("Block: %ld, NBstep: %ld, gain: %f->%f (%ld septs)\n", NBblock, NBstep, gainStart, gainEnd, NBgain);

    for(kg=0; kg<NBgain; kg++)
    {
        for(k=0; k<AOconf[LOOPNUMBER].NBDMmodes; k++)
            data.image[aoconfID_cmd_modes].array.F[k] = 0.0;

        gain = gainStart + 1.0*kg/(NBgain-1)*(gainEnd-gainStart);
        AOloopControl_setgainblock(NBblock, gain);
        AOloopControl_loopstep(LOOPNUMBER, NBstep);
        val = sqrt(AOconf[LOOPNUMBER].RMSmodesCumul/AOconf[LOOPNUMBER].RMSmodesCumulcnt);
        printf("%2ld  %6.4f  %10.8lf\n", kg, gain, val);

        if(val<bestval)
        {
            bestval = val;
            bestgain = gain;
        }
    }
    printf("BEST GAIN = %f\n", bestgain);

    AOloopControl_setgainblock(NBblock, bestgain);

    return(0);
}



int AOloopControl_InjectMode( long index, float ampl )
{
    long i;
    float *arrayf;
    char name[200];

    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_DMmodes==-1)
    {
        sprintf(name, "aol%ld_DMmodes", LOOPNUMBER);
        aoconfID_DMmodes = read_sharedmem_image(name);
    }

    if(aoconfID_DMRM==-1)
        aoconfID_DMRM = read_sharedmem_image(AOconf[LOOPNUMBER].DMnameRM);


    if((index<0)||(index>AOconf[LOOPNUMBER].NBDMmodes-1))
    {
        printf("Invalid mode index... must be between 0 and %ld\n", AOconf[LOOPNUMBER].NBDMmodes);
    }
    else
    {
        arrayf = (float*) malloc(sizeof(float)*AOconf[LOOPNUMBER].sizeDM);

        for(i=0; i<AOconf[LOOPNUMBER].sizeDM; i++)
            arrayf[i] = ampl*data.image[aoconfID_DMmodes].array.F[index*AOconf[LOOPNUMBER].sizeDM+i];



        data.image[aoconfID_DMRM].md[0].write = 1;
        memcpy (data.image[aoconfID_DMRM].array.F, arrayf, sizeof(float)*AOconf[LOOPNUMBER].sizeDM);
        data.image[aoconfID_DMRM].md[0].cnt0++;
        data.image[aoconfID_DMRM].md[0].write = 0;

        free(arrayf);
        AOconf[LOOPNUMBER].DMupdatecnt ++;
    }

    return(0);
}




int AOloopControl_AutoTune()
{
    long block;
    float gainStart = 0.0;
    float gainEnd = 1.0;

    long NBgain = 10;
    long NBstep = 10000;
    float gain;
    char name[200];
    long k, kg;
    float bestgain= 0.0;
    float bestval = 10000000.0;
    float val;

    int gOK;



    if(AOloopcontrol_meminit==0)
        AOloopControl_InitializeMemory(1);

    if(aoconfID_cmd_modes==-1)
    {
        sprintf(name, "aol%ld_DMmode_cmd", LOOPNUMBER);
        aoconfID_cmd_modes = read_sharedmem_image(name);
    }

    // initialize
    for(block=0; block<AOconf[LOOPNUMBER].NBMblocks; block++)
    {
        AOloopControl_setgainblock(block, 0.0);
        AOloopControl_setlimitblock(block, 0.1);
        AOloopControl_setmultfblock(block, 0.8);
    }


    for(block=0; block<AOconf[LOOPNUMBER].NBMblocks; block++)
    {
        // tune block gain
        gOK = 1;
        gain = gainStart;
        bestval = 100000000.0;
        while((gOK==1)&&(gain<gainEnd))
        {
            for(k=0; k<AOconf[LOOPNUMBER].NBDMmodes; k++)
                data.image[aoconfID_cmd_modes].array.F[k] = 0.0;

            gain += 0.01;
            gain *= 1.1;

            AOloopControl_setgainblock(block, gain);
            AOloopControl_loopstep(LOOPNUMBER, NBstep);
            val = sqrt(AOconf[LOOPNUMBER].RMSmodesCumul/AOconf[LOOPNUMBER].RMSmodesCumulcnt);
            printf("%2ld  %6.4f  %10.8lf\n", kg, gain, val);

            if(val<bestval)
            {
                bestval = val;
                bestgain = gain;
            }
            else
                gOK = 0;
        }
        printf("BLOCK %ld  : BEST GAIN = %f\n", block, bestgain);

        AOloopControl_setgainblock(block, bestgain);
    }


    return(0);
}




int AOloopControl_setparam(long loop, char *key, double value)
{
    int pOK=0;
    char kstring[200];

    strcpy(kstring, "PEperiod");
    if((strncmp (key, kstring, strlen(kstring)) == 0)&&(pOK==0))
    {
        //AOconf[loop].WFScamPEcorr_period = (long double) value;
        pOK = 1;
    }

    if(pOK==0)
        printf("Parameter not found\n");




    return (0);
}













