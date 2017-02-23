#include <stdint.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#include <gsl/gsl_math.h>
#include <gsl/gsl_multimin.h>

#include <fitsio.h>

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_tools/COREMOD_tools.h"

#include "image_filter/image_filter.h"
#include "fft/fft.h"
#include "info/info.h"
#include "statistic/statistic.h"
#include "OpticsMaterials/OpticsMaterials.h"
#include "image_filter/image_filter.h"
#include "image_gen/image_gen.h"

#include "OptSystProp/OptSystProp.h"
#include "AOsystSim/AOsystSim.h"

extern DATA data;





// non-null pixels in DM influence function (to speed up computing time)
int DMifpixarray_init = 0;
float *DMifpixarray_val;
long *DMifpixarray_index; // which actuator
long *DMifpixarray_pixindex; // which pixel
long DMifpixarray_NBpix;
long DMifpixarray_NBpix0;
long *dmifpixactarray;
long *dmifpixactarray_dmact;
long *dmifpixactarray_ii;
        

long NBprobesG = 3;
int CENTERprobe=1; // 1 if center probe included
long NBoptVar;
long double probe_re[100]; // 100 probes maximum
long double probe_im[100];
long double nprobe_re[100]; // 100 probes maximum
long double nprobe_im[100];


long double probe_tflux[100]; // test point computed flux
long double Cflux = 1.0; // coherent flux, ph for CA unity circle
long double probe_nmflux[100]; // measured flux (noisy)
long double probe_nmnoise[100]; // measured flux (noisy)


double tmpvalue1, tmpvalue2;



OPTSYST *optsystsim;


// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string (not image)
// 4: existing image
// 5: string


int_fast8_t AOsystSim_simpleAOfilter_cli(){
  if(CLI_checkarg(1,3)+CLI_checkarg(1,3)==0)
    AOsystSim_simpleAOfilter(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
  return(0);}

int_fast8_t AOsystSim_fitTelPup_cli(){
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)==0)
    AOsystSim_fitTelPup(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
  return(0);}

int_fast8_t AOsystSim_mkWF_cli(){
  if(CLI_checkarg(1,5)==0)    {
        AOsystSim_mkWF(data.cmdargtoken[1].val.string);
        return 0;
    }    else        return 1;}

int_fast8_t AOsystSim_PyrWFS_cli(){
  if(CLI_checkarg(1,5)==0)    {
        AOsystSim_PyrWFS(data.cmdargtoken[1].val.string);
        return 0;    }    else        return 1;}

int_fast8_t AOsystSim_DM_cli(){
  if(CLI_checkarg(1,5)==0)    {
        AOsystSim_DM(data.cmdargtoken[1].val.string);
        return 0;    }    else        return 1;}

int_fast8_t AOsystSim_coroLOWFS_cli(){
  if(CLI_checkarg(1,5)==0)    {
        AOsystSim_coroLOWFS(data.cmdargtoken[1].val.string);
        return 0;    }    else        return 1;}

int_fast8_t AOsystSim_run_cli(){
  if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,2)==0)    {
        AOsystSim_run(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl);
        return 0;    }    else        return 1;}
        
int_fast8_t AOsystSim_FPWFS_mkprobes_CLI(){
    if(CLI_checkarg(1,3)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,1)+CLI_checkarg(6,1)+CLI_checkarg(7,1)+CLI_checkarg(8,1)+CLI_checkarg(9,2)==0)    {
        AOsystSim_FPWFS_mkprobes(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf, data.cmdargtoken[7].val.numf, data.cmdargtoken[8].val.numf, data.cmdargtoken[9].val.numl);
        return 0;    }    else        return 1;}

int_fast8_t AOsystSim_FPWFS_sensitivityAnalysis_cli(){
     if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)    {   
        AOsystSim_FPWFS_sensitivityAnalysis(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);        return 0;    }    else        return 1;      }



int init_AOsystSim()
{
    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "conversion between image format, I/O");
    data.NBmodule++;

    strcpy(data.cmd[data.NBcmd].key,"AOsimfilt");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_simpleAOfilter_cli;
    strcpy(data.cmd[data.NBcmd].info,"AO simple filtering");
    strcpy(data.cmd[data.NBcmd].syntax,"<input WF> <output WF>");
    strcpy(data.cmd[data.NBcmd].example,"AOsimfilt wfin wfout");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_simpleAOfilter(const char *IDin_name, const char *IDout_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"AOsystsfitpup");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_fitTelPup_cli;
    strcpy(data.cmd[data.NBcmd].info,"fit telescope pupil");
    strcpy(data.cmd[data.NBcmd].syntax,"tel pupil file");
    strcpy(data.cmd[data.NBcmd].example,"AOsystfitpup");
    strcpy(data.cmd[data.NBcmd].Ccall,"AOsystSim_fitTelPup(const char *ID_name)");
    data.NBcmd++;


	

    strcpy(data.cmd[data.NBcmd].key,"AOsimmkWF");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_mkWF_cli;
    strcpy(data.cmd[data.NBcmd].info,"make WF series for AO simulation");
    strcpy(data.cmd[data.NBcmd].syntax,"<configuration file name>");
    strcpy(data.cmd[data.NBcmd].example,"AOsystmkWF WF.conf");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_mkWF(const char *CONF_FNAME)");
    data.NBcmd++;
	
    strcpy(data.cmd[data.NBcmd].key,"AOsimPyrWFS");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_PyrWFS_cli;
    strcpy(data.cmd[data.NBcmd].info,"run pyramid WFS for AO simulation");
    strcpy(data.cmd[data.NBcmd].syntax,"<configuration file name>");
    strcpy(data.cmd[data.NBcmd].example,"AOsimPyrWFS PyrWFS.conf");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_PyrWFS(const char *CONF_FNAME)");
    data.NBcmd++;
	
    strcpy(data.cmd[data.NBcmd].key,"AOsimDM");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_DM_cli;
    strcpy(data.cmd[data.NBcmd].info,"run DM for AO simulation");
    strcpy(data.cmd[data.NBcmd].syntax,"<configuration file name>");
    strcpy(data.cmd[data.NBcmd].example,"AOsimDM PyrWFS.conf");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_DM(const char *CONF_FNAME)");
    data.NBcmd++;
	
    strcpy(data.cmd[data.NBcmd].key,"AOsimcoroLOWFS");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_coroLOWFS_cli;
    strcpy(data.cmd[data.NBcmd].info,"run coro and LOWFS for AO simulation");
    strcpy(data.cmd[data.NBcmd].syntax,"<configuration file name>");
    strcpy(data.cmd[data.NBcmd].example,"AOsimcoroLOWFS LOWFS.conf");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_coroLOWFS(const char *CONF_FNAME)");
    data.NBcmd++;
	
	
	

    strcpy(data.cmd[data.NBcmd].key,"AOsystsim");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_run_cli;
    strcpy(data.cmd[data.NBcmd].info,"run fake AO system");
    strcpy(data.cmd[data.NBcmd].syntax,"<syncmode> <DMindex> <delayus>");
    strcpy(data.cmd[data.NBcmd].example,"AOsystsim");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_run(int syncmode, long DMindex, long delayus)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"AOsystexaosim");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_extremeAO_contrast_sim;
    strcpy(data.cmd[data.NBcmd].info,"run extremeAO analysis");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"AOsystexaosim");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_extremeAO_contrast_sim()");
    data.NBcmd++;
    
    strcpy(data.cmd[data.NBcmd].key,"AOsystmkABprobes");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_FPWFS_mkprobes_CLI;
    strcpy(data.cmd[data.NBcmd].info,"make AB probes for focal plane sensing and coherence measurement");
    strcpy(data.cmd[data.NBcmd].syntax, "AOsystmkABprobes prA prB 50 50 0.7 0.1 0.7 0.1 1");
    strcpy(data.cmd[data.NBcmd].example,"AOsystmkABprobes");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_FPWFS_mkprobes(const char *IDprobeA_name, const char *IDprobeB_name, long dmxsize, long dmysize, double CPAmax, double CPArmin, double CPArmax, double RMSampl, long modegeom)");
    data.NBcmd++;
 
    strcpy(data.cmd[data.NBcmd].key,"AOsystFPWFSan");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOsystSim_FPWFS_sensitivityAnalysis_cli;
    strcpy(data.cmd[data.NBcmd].info,"run focal plane WFS sensitivity analysis");
    strcpy(data.cmd[data.NBcmd].syntax,"<mapmode> <mode> <optmode> <NBprobes>");
    strcpy(data.cmd[data.NBcmd].example,"AOsystFPWFSan");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOsystSim_FPWFS_sensitivityAnalysis(int mapmode, int mode, int optmode, int NBprobes)");
    data.NBcmd++;

    // add atexit functions here

    return 0;
}





/** \brief simple AO filtering model using Fourier analysis
 *         simulates WFS integration, delay, noise (as a function of spatial frequency)
 *
 *  Open loop model
 * 
 *  AO simple filter is modeled by several 2D maps in spatial frequency:
 *  aosf_noise : noise per spatial frequency
 *  aosf_mult : signal throughput per spatial frequency
 *  aosf_gain : loop gain to be applied per spatial frequency
 */


int AOsystSim_simpleAOfilter(const char *IDin_name, const char *IDout_name)
{
    long IDin, IDmask, IDout, IDdm, IDwfe;
    long *sizearray;
    long cnt0;
    long ii, jj;
    double x, y, r;
    long ID, IDin1;

    long IDaosf_noise;
    long IDaosf_mult;
    long IDaosf_gain;

    float loopgain = 0.001;
    float multr0 = 0.2;

    float wfsetime = 0.001; /**< WFS exposure time */
    float timedelay = 0.0005;/**< time delay between wavefront sensor measurement (end of exposure) and DM correction [sec] */
    long size2;
    double tnowdouble = 0.0;
    double time_wfse0 = 0.0;
    long wfsecnt = 0;

    double rmask;
    double r1;
    double WFrms0 = 0.0;
    double WFrms = 0.0;
    long WFrmscnt = 0;

    double noiselevel = 0.3; // noise level per WFS measurement [um] - only white noise is currently supported

    /** wavefront correction buffer to implement time delay */
    long IDwfcbuff;
    long wfcbuff_size = 10000;
    double *wfcbuff_time;
    long *wfcbuff_status; // 1 : waiting to be applied
    long k, k0, k1;
    long wfsecnt1;

    double dmmovetime = 0.001; /**< time it takes for DM to move */
    long dmmoveNBpt = 10;

    sizearray = (long*) malloc(sizeof(long)*2);

    IDin = read_sharedmem_image(IDin_name);  /**< turbulence channel */
    sizearray[0] = data.image[IDin].md[0].size[0];
    sizearray[1] = data.image[IDin].md[0].size[1];
    size2 = sizearray[0]*sizearray[1];

    IDwfcbuff = create_3Dimage_ID("wfcbuff", sizearray[0], sizearray[1], wfcbuff_size);
    wfcbuff_time = (double*) malloc(sizeof(double)*wfcbuff_size);
    wfcbuff_status = (long*) malloc(sizeof(long)*wfcbuff_size);
    for(k=0; k<wfcbuff_size; k++)
        wfcbuff_status[k] = 0;

    rmask = 0.4*sizearray[0];

    IDaosf_noise = create_2Dimage_ID("aosf_noise", sizearray[0], sizearray[1]);
    IDaosf_mult = create_2Dimage_ID("aosf_mult", sizearray[0], sizearray[1]);
    IDaosf_gain = create_2Dimage_ID("aosf_gain", sizearray[0], sizearray[1]);

    IDmask = create_2Dimage_ID("aosf_mask", sizearray[0], sizearray[1]);


    for(ii=0; ii<sizearray[0]; ii++)
        for(jj=0; jj<sizearray[1]; jj++)
        {
            x = 1.0*ii-0.5*sizearray[0];
            y = 1.0*jj-0.5*sizearray[1];
            r = sqrt(x*x+y*y);
            r1 = (r-rmask)/(0.5*sizearray[0]-rmask);

            if(r1<0.0)
                data.image[IDmask].array.F[jj*sizearray[0]+ii] = 1.0;
            else if (r1>1.0)
                data.image[IDmask].array.F[jj*sizearray[0]+ii] = 0.0;
            else
                data.image[IDmask].array.F[jj*sizearray[0]+ii] = 0.5*(cos(r1*M_PI)+1.0);

            data.image[IDaosf_gain].array.F[jj*sizearray[0]+ii] = loopgain;
            data.image[IDaosf_mult].array.F[jj*sizearray[0]+ii] = exp(-pow(r/(multr0*sizearray[0]),8.0));
            if(r>multr0*sizearray[0])
                data.image[IDaosf_mult].array.F[jj*sizearray[0]+ii] = 0.0;
            data.image[IDaosf_noise].array.F[jj*sizearray[0]+ii] = 0.0;
        }
    save_fits("aosf_noise", "!aosf_noise.fits");
    save_fits("aosf_mult", "!aosf_mult.fits");
    save_fits("aosf_gain", "!aosf_gain.fits");

    permut("aosf_mult");
    permut("aosf_noise");
    permut("aosf_gain");




    IDdm = create_image_ID("aofiltdm", 2, sizearray, FLOAT, 1, 0);
    IDwfe = create_image_ID("aofiltwfe", 2, sizearray, FLOAT, 1, 0);

    IDout = create_image_ID(IDout_name, 2, sizearray, FLOAT, 1, 0);
    strcpy(data.image[IDout].kw[0].name, "TIME");
    data.image[IDout].kw[0].type = 'D';
    data.image[IDout].kw[0].value.numf = 0.0;
    strcpy(data.image[IDout].kw[0].comment, "Physical time [sec]");

    IDin1 = create_image_ID("aofiltin", 2, sizearray, FLOAT, 1, 0);

    printf("%s -> %s\n", IDin_name, IDout_name);
    cnt0 = -1;
    time_wfse0 = data.image[IDin].kw[0].value.numf; /** start of WFS exposure */
    wfsecnt = 0;
    wfsecnt1 = 0;
    while(1)
    {
        usleep(10);
        if(data.image[IDin].md[0].cnt0!=cnt0)
        {
            /** input masking */
            for(ii=0; ii<size2; ii++)
                data.image[IDin1].array.F[ii] = data.image[IDin].array.F[ii] * data.image[IDmask].array.F[ii];

            /** Wavefront correction & measurement */
            WFrms0 = 0.0;
            WFrms = 0.0;
            WFrmscnt = 0;
            for(ii=0; ii<sizearray[0]*sizearray[1]; ii++)
            {
                data.image[IDout].array.F[ii] = data.image[IDin1].array.F[ii] - data.image[IDdm].array.F[ii];
                if(data.image[IDmask].array.F[ii]>0.99)
                {
                    WFrms0 += data.image[IDin1].array.F[ii]*data.image[IDin1].array.F[ii];
                    WFrms += data.image[IDout].array.F[ii]*data.image[IDout].array.F[ii];
                    WFrmscnt ++;
                }
            }
            WFrms0 = sqrt(WFrms0/WFrmscnt);
            WFrms = sqrt(WFrms/WFrmscnt);
            data.image[IDout].kw[0].value.numf = tnowdouble;


            tnowdouble = data.image[IDin].kw[0].value.numf;
            printf("\r Time : %.6lf    WFSexposure time:  %.6f   [%5ld]    %10.5lf -> %10.5lf ", tnowdouble, tnowdouble-time_wfse0, wfsecnt, WFrms0, WFrms);
            fflush(stdout);
            cnt0 = data.image[IDin].md[0].cnt0;

            do2drfft(IDout_name, "aosf_tmpfft");
            ID = image_ID("aosf_tmpfft");

            for(ii=0; ii<sizearray[0]*sizearray[1]; ii++)
            {
                data.image[ID].array.CF[ii].re *= data.image[IDaosf_mult].array.F[ii]/size2;
                data.image[ID].array.CF[ii].im *= data.image[IDaosf_mult].array.F[ii]/size2;
            }

            do2dffti("aosf_tmpfft", "testo");
            delete_image_ID("aosf_tmpfft");
            ID = image_ID("testo");

            /** Wavefront estimation */
            for(ii=0; ii<sizearray[0]*sizearray[1]; ii++)
                data.image[IDwfe].array.F[ii] += data.image[ID].array.CF[ii].re;
            delete_image_ID("testo");
            wfsecnt ++;

            if((tnowdouble-time_wfse0) > wfsetime)
            {
                if(wfsecnt>0)
                    for(ii=0; ii<sizearray[0]*sizearray[1]; ii++)
                        data.image[IDwfe].array.F[ii] /= wfsecnt;


                /** write entries in buffer */
                for(k1=0; k1<dmmoveNBpt; k1++)
                {
                    k0 = 0;
                    while((wfcbuff_status[k0]==1)&&(k0<wfcbuff_size))
                        k0++;
                    if(k0>wfcbuff_size-1)
                    {
                        printf("\n WFC buffer full [%ld]\n", k0);
                        exit(0);
                    }

                    for(ii=0; ii<size2; ii++)
                        data.image[IDwfcbuff].array.F[k0*size2+ii] = data.image[IDwfe].array.F[ii]/dmmoveNBpt;
                    wfcbuff_time[k0] = tnowdouble + timedelay + 1.0*k1/(dmmoveNBpt-1)*dmmovetime;
                    wfcbuff_status[k0] = 1;
                }

                for(ii=0; ii<sizearray[0]*sizearray[1]; ii++)
                    data.image[IDwfe].array.F[ii] = 0.0;
                time_wfse0 += wfsetime;
                wfsecnt = 0;
                wfsecnt1++;
            }



            for(k=0; k<wfcbuff_size; k++)
            {
                if(wfcbuff_status[k]==1)
                    if(wfcbuff_time[k]<tnowdouble)
                    {
                        //			printf("Reading WFC buffer slice %ld  [%5ld %5ld]\n", k, wfsecnt1, wfsecnt);
                        /** update DM shape */
                        for(ii=0; ii<sizearray[0]*sizearray[1]; ii++)
                            data.image[IDdm].array.F[ii] += data.image[IDaosf_gain].array.F[ii]*data.image[IDwfcbuff].array.F[k*size2+ii];
                        wfcbuff_status[k] = 0;
                    }
            }
            data.image[IDout].md[0].cnt0 = cnt0;
        }
    }

    delete_image_ID("circbuff");

    free(sizearray);
    free(wfcbuff_time);
    free(wfcbuff_status);

    return(0);
}




















// all sizes in actuators on DM

long AOsystSim_mkTelPupDM(const char *ID_name, long msize, double xc, double yc, double rin, double rout, double pupPA, double spiderPA, double spideroffset, double spiderthick, double stretchx)
{
    long ID, IDz;
    long ii,  jj;
    double x, y, x1, y1, r;
    long binfact = 8;
    long size;
    double PA;
    double val;
    long IDindex, IDi;
    long index;
    long ii1, jj1;


    size = msize*binfact;

    ID = create_2Dimage_ID(ID_name, msize, msize);
    IDi = create_3Dimage_ID("TPind", msize, msize, 5);

    IDz = create_2Dimage_ID("telpupDMz", size, size);
    IDindex = create_2Dimage_ID("telpupDMzindex", size, size);
    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            index = 0;
            val = 1.0;

            x = (1.0*ii/binfact);
            y = (1.0*jj/binfact);
            x -= xc;
            y -= yc;
            r = sqrt(x*x+y*y);
            PA = atan2(y, x);
            PA += pupPA;
            x = r*cos(PA)*stretchx;
            y = r*sin(PA);

            if(r>rout)
                val = 0.0;
            if(r<rin)
                val = 0.0;

            x1 = x-spideroffset-spiderthick/2.0;
            y1 = y;
            PA = atan2(y1,x1);
            if(fabs(PA)<spiderPA)
                index = 1;


            x1 = x+spideroffset+spiderthick/2.0;
            y1 = y;
            PA = atan2(y1,-x1);
            if(fabs(PA)<spiderPA)
                index = 2;




            x1 = x+spideroffset-spiderthick/2.0;
            y1 = y;
            PA = atan2(x1,y1);
            if((fabs(PA)<M_PI/2-spiderPA)&&(x<0))
                index = 3;

            x1 = -x+spideroffset-spiderthick/2.0;
            y1 = y;
            PA = atan2(x1,y1);
            if((fabs(PA)<M_PI/2-spiderPA)&&(x>0))
                index = 3;





            x1 = x+spideroffset-spiderthick/2.0;
            y1 = -y;
            PA = atan2(x1,y1);
            if((fabs(PA)<M_PI/2-spiderPA)&&(x<0))
                index = 4;

            x1 = -x+spideroffset-spiderthick/2.0;
            y1 = -y;
            PA = atan2(x1,y1);
            if((fabs(PA)<M_PI/2-spiderPA)&&(x>0))
                index = 4;

            if(index==0)
                val = 0.0;
            data.image[IDz].array.F[jj*size+ii] = val;
            data.image[IDindex].array.F[jj*size+ii] = index*val;

            ii1 = (long) (ii/binfact);
            jj1 = (long) (jj/binfact);


            data.image[ID].array.F[jj1*msize+ii1] += val/binfact/binfact;

            if(val>0.5)
            {
                data.image[IDi].array.F[jj1*msize+ii1] = 1;
                data.image[IDi].array.F[index*msize*msize+jj1*msize+ii1] = 1;
            }
        }

    delete_image_ID("telpupDMz");
    delete_image_ID("telpupDMzindex");

    save_fits("TPind", "!TPind.fits");

    return(ID);
}




/** fits DM illumination to pupil geometry */

long AOsystSim_fitTelPup(const char *ID_name, const char *IDtelpup_name)
{
    FILE *fp;
    long ID, ID1, IDt;
    long IDtelpup;
    double xc, yc, pupPA, spiderPA, spideroffset, spiderthick, rin, rout, stretchx;
    long size;
    double vp10, vp90;
    double rms;
    long ii, jj;
    double v1;

    double xc_min, yc_min, pupPA_min, rout_min, stretchx_min, spiderPA_min, spideroffset_min;
    double xc_max, yc_max, pupPA_max, rout_max, stretchx_max, spiderPA_max, spideroffset_max;

    double xc1, yc1, pupPA1, rout1, stretchx1, spiderPA1, spideroffset1;
    long nbstep = 2;
    double rms1;

    double xc_range, yc_range, pupPA_range, rout_range, stretchx_range, spiderPA_range, spideroffset_range;
    long iter;
    long NBiter = 5;

    /** compensate for illumination gradient */
    double coeffx, coeffy, coeffx1, coeffy1;
    double x, y;
    double val, val1;

    double eps = 1.0e-8;


    coeffx = 0.0;
    coeffy = 0.0;

    rout = 22.15;
    rin = 0.315*rout;
    pupPA = -0.105;
    spiderPA = 0.85;
    spiderthick = 0.06*rout;
    spideroffset = 0.15*rout;


    xc = 24.629630;
    yc = 23.518519;
    stretchx = 1.048148;

    rout1 = rout;
    xc1 = xc;
    yc1 = yc;
    pupPA1 = pupPA;
    stretchx1 = stretchx;
    spiderPA1 = spiderPA;
    spideroffset1 = spideroffset;

    /** set percentiles */
    ID = image_ID(ID_name);
    size = data.image[ID].md[0].size[0];


    vp10 = img_percentile_float(ID_name, 0.1);
    vp90 = img_percentile_float(ID_name, 0.9);
    printf("%f %f\n", vp10, vp90);
    for(ii=0; ii<data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1]; ii++)
        data.image[ID].array.F[ii] = (data.image[ID].array.F[ii]-vp10)/(vp90-vp10);



    /** compensate for image gradient */
    ID1 = create_2Dimage_ID("tmpftpim", size, size);
    val1 = 1000000000000000.0;
    for(coeffx=-1.5; coeffx < 1.5; coeffx += 0.05)
        for(coeffy=-1.5; coeffy < 1.5; coeffy += 0.05)
        {
            for(ii=0; ii<size; ii++)
                for(jj=0; jj<size; jj++)
                {
                    x = 1.0*ii/size;
                    y = 1.0*jj/size;
                    data.image[ID1].array.F[jj*size+ii] =  data.image[ID].array.F[jj*size+ii]*(1.0+coeffx*(x-0.5))*(1.0+coeffy*(y-0.5));
                }
            val = img_percentile_float("tmpftpim", 0.9) - img_percentile_float("tmpftpim", 0.7);
            if(val<val1)
            {
                val1 = val;
                coeffx1 = coeffx;
                coeffy1 = coeffy;
            }
            printf("     %f %f   %g       ( %f %f %g )\n", coeffx, coeffy, val, coeffx1, coeffy1, val1);
        }

    printf("COEFF : %f %f\n", coeffx1, coeffy1);
    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            x = 1.0*ii/size;
            y = 1.0*jj/size;
            data.image[ID].array.F[jj*size+ii] =  data.image[ID].array.F[jj*size+ii]*(1.0+coeffx1*(x-0.5))*(1.0+coeffy1*(y-0.5));
        }



    vp10 = img_percentile_float(ID_name, 0.1);
    vp90 = img_percentile_float(ID_name, 0.9);
    printf("%f %f\n", vp10, vp90);



    for(ii=0; ii<data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1]; ii++)
        data.image[ID].array.F[ii] = (data.image[ID].array.F[ii]-vp10)/(vp90-vp10);

    for(ii=0; ii<size*size; ii++)
    {
        if(data.image[ID].array.F[ii]>0.05)
            data.image[ID].array.F[ii] = pow(data.image[ID].array.F[ii], 0.5);
        else
            data.image[ID].array.F[ii] = 0.0;
    }

    vp10 = img_percentile_float(ID_name, 0.1);
    vp90 = img_percentile_float(ID_name, 0.9);
    for(ii=0; ii<data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1]; ii++)
        data.image[ID].array.F[ii] = (data.image[ID].array.F[ii]-vp10)/(vp90-vp10);




    rout_min = rout1*0.9;
    rout_max = rout1*1.1;
    xc_min = xc1-2.0;
    xc_max = xc1+2.0;
    yc_min = yc1-2.0;
    yc_max = yc1+2.0;
    pupPA_min = pupPA1-0.1;
    pupPA_max = pupPA1+0.1;
    stretchx_min = stretchx1*0.95;
    stretchx_max = stretchx1*1.05;
    spideroffset_min = spideroffset1*0.9;
    spideroffset_max = spideroffset1*1.1;
    spiderPA_min = spiderPA1-0.1;
    spiderPA_max = spiderPA1+0.1;

    fp = fopen("pupfit.txt", "w");
    fclose(fp);


    rms1 = 1000000000000000000000.0;
    for(iter=0; iter<NBiter; iter++)
    {
        for(spiderPA=spiderPA_min; spiderPA<spiderPA_max+eps; spiderPA += (spiderPA_max-spiderPA_min)/nbstep)
            for(spideroffset=spideroffset_min; spideroffset<spideroffset_max+eps; spideroffset += (spideroffset_max-spideroffset_min)/nbstep)
                for(stretchx=stretchx_min; stretchx<stretchx_max+eps; stretchx += (stretchx_max-stretchx_min)/nbstep)
                    for(rout=rout_min; rout<rout_max+eps; rout += (rout_max-rout_min)/nbstep)
                        for(xc=xc_min; xc<xc_max+eps; xc += (xc_max-xc_min)/nbstep)
                            for(yc=yc_min; yc<yc_max+eps; yc += (yc_max-yc_min)/nbstep)
                                for(pupPA=pupPA_min; pupPA<pupPA_max+eps; pupPA += (pupPA_max-pupPA_min)/nbstep)
                                {
                                    rin = 0.305*rout;
                                    spiderthick = 0.06*rout;
                                    spideroffset = 0.165*rout;
                                    AOsystSim_mkTelPupDM("testpup", size, xc, yc, rin, rout, pupPA, spiderPA, spideroffset, spiderthick, stretchx);
                                    IDt = image_ID("testpup");
                                    // list_image_ID();
                                    // save_fits("testpup", "!testpup.fits");

                                    rms = 0.0;
                                    for(ii=0; ii<size*size; ii++)
                                    {
                                        v1 = data.image[ID].array.F[ii] - data.image[IDt].array.F[ii];
                                        rms += v1*v1;
                                    }

                                    rms = sqrt(rms/size/size);

                                    if(rms<rms1)
                                    {
                                        rms1 = rms;
                                        rout1 = rout;
                                        xc1 = xc;
                                        yc1 = yc;
                                        pupPA1 = pupPA;
                                        stretchx1 = stretchx;
                                        spiderPA1 = spiderPA;
                                        spideroffset1 = spideroffset;
                                    }
                                    delete_image_ID("testpup");
                                    printf("%f %f %f  %f -> rms = %g\n", rout, xc, yc, pupPA, rms);
                                    fp = fopen("pupfit.txt", "a");
                                    fprintf(fp,"%f %f %f %f %g\n", rout, xc, yc, pupPA, rms);
                                    fclose(fp);
                                }

        printf("ITERATION %ld    %f %f  %f  %f  ->  %f\n", iter, rout1, xc1, yc1, pupPA1, rms1);


        spideroffset_range = spideroffset_max-spideroffset_min;
        spideroffset_range /= 3.0;
        spideroffset_min = spideroffset1 - 0.5*spideroffset_range;
        spideroffset_max = spideroffset1 + 0.5*spideroffset_range;

        spiderPA_range = spiderPA_max-spiderPA_min;
        spiderPA_range /= 3.0;
        spiderPA_min = spiderPA1 - 0.5*spiderPA_range;
        spiderPA_max = spiderPA1 + 0.5*spiderPA_range;

        stretchx_range = stretchx_max-stretchx_min;
        stretchx_range /= 3.0;
        stretchx_min = stretchx1 - 0.5*stretchx_range;
        stretchx_max = stretchx1 + 0.5*stretchx_range;

        rout_range = rout_max-rout_min;
        rout_range /= 3.0;
        rout_min = rout1 - 0.5*rout_range;
        rout_max = rout1 + 0.5*rout_range;

        xc_range = xc_max-xc_min;
        xc_range /= 3.0;
        xc_min = xc1 - 0.5*xc_range;
        xc_max = xc1 + 0.5*xc_range;

        yc_range = yc_max-yc_min;
        yc_range /= 3.0;
        yc_min = yc1 - 0.5*yc_range;
        yc_max = yc1 + 0.5*yc_range;

        pupPA_range = xc_max-xc_min;
        pupPA_range /= 3.0;
        pupPA_min = xc1 - 0.5*xc_range;
        pupPA_max = xc1 + 0.5*pupPA_range;
    }



    printf("BEST SOLUTION: \n");
    printf("     xc            %f\n", xc1);
    printf("     yc            %f\n", yc1);
    printf("     rout          %f\n", rout1);
    printf("     pupPA         %f\n", pupPA1);
    printf("     spiderPA      %f\n", spiderPA1);
    printf("     spideroffset  %f\n", spideroffset1);
    printf("     stretchx      %f\n", stretchx1);

    rout = rout1;
    rin = 0.315*rout;
    spiderthick = 0.06*rout;

    AOsystSim_mkTelPupDM(IDtelpup_name, size, xc1, yc1, rin, rout, pupPA1, spiderPA1, spideroffset1, spiderthick, stretchx1);

    return(ID);
}



/** \brief DM control signals to DMshape
 *
 *
 */

int AOsystSim_DMshape(const char *IDdmctrl_name, const char *IDdmifc_name, const char *IDdm_name)
{
    long IDdmctrl, IDdmifc, IDdm;
    long dmsizex, dmsizey, DMnbact;
    long ii, jj;
    long dmact;
    double eps=1.0e-12;
    long k;
    long kk;

    IDdmctrl = image_ID(IDdmctrl_name);

    IDdmifc = image_ID(IDdmifc_name);
    dmsizex = data.image[IDdmifc].md[0].size[0];
    dmsizey = data.image[IDdmifc].md[0].size[1];
    DMnbact = data.image[IDdmifc].md[0].size[2];

  
    if(DMifpixarray_init==0)
    {
        DMifpixarray_NBpix = 0.0;
        for(dmact=0; dmact<DMnbact; dmact++)
            for(ii=0; ii<dmsizex*dmsizey; ii++)
                if(fabs(data.image[IDdmifc].array.F[dmact*dmsizex*dmsizey+ii])>eps)
                    DMifpixarray_NBpix++;
        
        dmifpixactarray = (long*) malloc(sizeof(long)*DMifpixarray_NBpix);
        dmifpixactarray_dmact = (long*) malloc(sizeof(long)*DMifpixarray_NBpix);
        dmifpixactarray_ii = (long*) malloc(sizeof(long)*DMifpixarray_NBpix);
        
        
        
        DMifpixarray_NBpix = 0.0;
        for(dmact=0; dmact<DMnbact; dmact++)
            for(ii=0; ii<dmsizex*dmsizey; ii++)
                if(fabs(data.image[IDdmifc].array.F[dmact*dmsizex*dmsizey+ii])>eps)
                    {
                        dmifpixactarray[DMifpixarray_NBpix] = dmact*dmsizex*dmsizey+ii;
                        dmifpixactarray_dmact[DMifpixarray_NBpix] = dmact;
                        dmifpixactarray_ii[DMifpixarray_NBpix] = ii;
                        DMifpixarray_NBpix++;
                    }
        
        if(DMifpixarray_val!=NULL)
            {
                printf("WARNING: array DMifpixarray_val is not NULL");
                fflush(stdout);
            }
        if((DMifpixarray_val = (float*) malloc(sizeof(float)*DMifpixarray_NBpix))==NULL)
            {
                printf("ERROR: could not allocate array DMifpixarray_val\n");
                exit(0);
            }
       
        DMifpixarray_index = (long*) malloc(sizeof(long)*DMifpixarray_NBpix);
        DMifpixarray_pixindex = (long*) malloc(sizeof(long)*DMifpixarray_NBpix);
        DMifpixarray_NBpix0 = DMifpixarray_NBpix;
        DMifpixarray_init = 1;
    }
       
       
/*    DMifpixarray_NBpix = 0;
    for(dmact=0; dmact<DMnbact; dmact++)
        for(ii=0; ii<dmsizex*dmsizey; ii++)
            if(fabs(data.image[IDdmifc].array.F[dmact*dmsizex*dmsizey+ii])>eps)
                {
                    DMifpixarray_val[DMifpixarray_NBpix] = data.image[IDdmifc].array.F[dmact*dmsizex*dmsizey+ii];
                    DMifpixarray_index[DMifpixarray_NBpix] = dmact;
                    DMifpixarray_pixindex[DMifpixarray_NBpix] = ii;
                    DMifpixarray_NBpix++;
                }
*/                
                
    for(kk=0;kk<DMifpixarray_NBpix0;kk++)
    {
        DMifpixarray_val[kk] = data.image[IDdmifc].array.F[dmifpixactarray[kk]];
        DMifpixarray_index[kk] = dmifpixactarray_dmact[kk];
        DMifpixarray_pixindex[kk] = dmifpixactarray_ii[kk];
    }
    
  //  printf("Used pix = %ld / %ld\n", DMifpixarray_NBpix, DMnbact*dmsizex*dmsizey);
  
   
    IDdm = image_ID(IDdm_name);
    if(IDdm==-1)
        IDdm = create_2Dimage_ID(IDdm_name, dmsizex, dmsizey);
   
 
    for(ii=0; ii<dmsizex*dmsizey; ii++)
        data.image[IDdm].array.F[ii] = 0.0;
/*
    for(dmact=0; dmact<DMnbact; dmact++)
    {
        for(ii=0; ii<dmsizex*dmsizey; ii++)
            data.image[IDdm].array.F[ii] += data.image[IDdmctrl].array.F[dmact]*data.image[IDdmifc].array.F[dmact*dmsizex*dmsizey+jj*dmsizex+ii];
    }
*/

    for(k=0;k<DMifpixarray_NBpix;k++)
        data.image[IDdm].array.F[DMifpixarray_pixindex[k]] += data.image[IDdmctrl].array.F[DMifpixarray_index[k]] * DMifpixarray_val[k];
    
    return (0);
}




int AOsystSim_WFSsim_Pyramid(const char *inWFc_name, const char *outWFSim_name, double modampl, long modnbpts)
{
    long ID_inWFc, ID_outWFSim;
    long arraysize;
    long *imsize;
    long IDa, IDp;
    long ID_inWFccp;
    long arraysize2;
    long IDpyramp, IDpyrpha;
    double lenssize;
    double pcoeff = 1.4;
    double x, y;
    long ii, jj;
    char pnamea[200];
    char pnamep[200];
    char pfnamea[200];
    char pfnamep[200];

    long PYRMOD_nbpts = 16;
    long pmodpt;
    double PYRMOD_rad = 5.0;
    double xc, yc, PA;
    long ID_outWFSim_tmp;
    
    PYRMOD_nbpts = modnbpts;
    PYRMOD_rad = modampl;
    
    
    
    ID_inWFc = image_ID(inWFc_name);
    arraysize = data.image[ID_inWFc].md[0].size[0];
    arraysize2 = arraysize*arraysize;
    lenssize = 0.4*arraysize;


    for(pmodpt=0; pmodpt<PYRMOD_nbpts; pmodpt++)
    {
        sprintf(pnamea, "pyramp_%03ld", pmodpt);
        sprintf(pnamep, "pyrpha_%03ld", pmodpt);
        IDpyramp = image_ID(pnamea);
        IDpyrpha = image_ID(pnamep);
        if((IDpyramp==-1)||(IDpyrpha==-1))
        {
            imsize = (long*) malloc(sizeof(long)*2);
            imsize[0] = arraysize;
            imsize[1] = arraysize;
            IDpyramp = create_image_ID(pnamea, 2, imsize, FLOAT, 0, 0);
            IDpyrpha = create_image_ID("pyrpha0", 2, imsize, FLOAT, 0, 0);
            free(imsize);

            PA = 2.0*M_PI*pmodpt/PYRMOD_nbpts;
            xc = PYRMOD_rad * cos(PA);
            yc = PYRMOD_rad * sin(PA);

            for(ii=0; ii<arraysize; ii++)
                for(jj=0; jj<arraysize; jj++)
                {
                    x = 1.0*(ii-arraysize/2) - xc;
                    y = 1.0*(jj-arraysize/2) - yc;

                    data.image[IDpyrpha].array.F[jj*arraysize+ii] = pcoeff*(fabs(x)+fabs(y));
                    if((fabs(x)>lenssize)||(fabs(y)>lenssize))
                        data.image[IDpyramp].array.F[jj*arraysize+ii] = 0.0;
                    else
                        data.image[IDpyramp].array.F[jj*arraysize+ii] = 1.0;
                }
            gauss_filter("pyrpha0", pnamep, 1.0, 10);
            delete_image_ID("pyrpha0");

            sprintf(pfnamea, "!pyramp_%03ld.fits", pmodpt);
            sprintf(pfnamep, "!pyrpha_%03ld.fits", pmodpt);

            printf("SAVING: %s -> %s\n", pnamea, pfnamea);
            save_fits(pnamea, pfnamea);
            save_fits(pnamep, pfnamep);
        }
    }


    ID_outWFSim = image_ID(outWFSim_name);
    if(ID_outWFSim==-1)
    {
        imsize = (long*) malloc(sizeof(long)*2);
        imsize[0] = arraysize;
        imsize[1] = arraysize;
        ID_outWFSim = create_image_ID(outWFSim_name, 2, imsize, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(outWFSim_name, 7);
        free(imsize);
    }

    ID_outWFSim_tmp = image_ID("outpwfsimtmp");
    if(ID_outWFSim_tmp==-1)
    {
        imsize = (long*) malloc(sizeof(long)*2);
        imsize[0] = arraysize;
        imsize[1] = arraysize;
        ID_outWFSim_tmp = create_image_ID("outpwfsimtmp", 2, imsize, FLOAT, 1, 0);
        free(imsize);
    }

    ID_inWFccp = image_ID("pyrwfcin");
    if(ID_inWFccp==-1)
    {
        imsize = (long*) malloc(sizeof(long)*2);
        imsize[0] = arraysize;
        imsize[1] = arraysize;
        ID_inWFccp = create_image_ID("pyrwfcin", 2, imsize, COMPLEX_FLOAT, 0, 0);
        free(imsize);
    }


    data.image[ID_outWFSim].md[0].write = 1;
    for(ii=0; ii<arraysize2; ii++)
        data.image[ID_outWFSim_tmp].array.F[ii] = 0.0;

    for(pmodpt=0; pmodpt<PYRMOD_nbpts; pmodpt++)
    {
        memcpy(data.image[ID_inWFccp].array.CF, data.image[ID_inWFc].array.CF, sizeof(complex_float)*arraysize*arraysize);



        permut("pyrwfcin");
        do2dfft("pyrwfcin","pyrpsfcin");
        permut("pyrpsfcin");
        mk_amph_from_complex("pyrpsfcin", "pyrpsfa", "pyrpsfp", 0);
        delete_image_ID("pyrpsfcin");

        sprintf(pnamea, "pyramp_%03ld", pmodpt);
        sprintf(pnamep, "pyrpha_%03ld", pmodpt);
        IDpyramp = image_ID(pnamea);
        IDpyrpha = image_ID(pnamep);
        IDa = image_ID("pyrpsfa");
        IDp = image_ID("pyrpsfp");

        for(ii=0; ii<arraysize2; ii++)
        {
            data.image[IDa].array.F[ii] *= data.image[IDpyramp].array.F[ii];
            data.image[IDp].array.F[ii] += data.image[IDpyrpha].array.F[ii];
        }


        mk_complex_from_amph("pyrpsfa", "pyrpsfp", "pyrpsfc", 0);
        delete_image_ID("pyrpsfa");
        delete_image_ID("pyrpsfp");

        permut("pyrpsfc");
        do2dfft("pyrpsfc","pyrwfs_pupc");
        delete_image_ID("pyrpsfc");
        permut("pyrwfs_pupc");
        mk_amph_from_complex("pyrwfs_pupc", "pyrwfs_pupa", "pyrwfs_pupp", 0);

        delete_image_ID("pyrwfs_pupp");
        delete_image_ID("pyrwfs_pupc");

        IDa = image_ID("pyrwfs_pupa");

        for(ii=0; ii<arraysize2; ii++)
            data.image[ID_outWFSim_tmp].array.F[ii] += data.image[IDa].array.F[ii]*data.image[IDa].array.F[ii]/PYRMOD_nbpts;
        delete_image_ID("pyrwfs_pupa");
    }
    memcpy(data.image[ID_outWFSim].array.F, data.image[ID_outWFSim_tmp].array.F, sizeof(float)*arraysize*arraysize);
    data.image[ID_outWFSim].md[0].cnt0++;
    data.image[ID_outWFSim].md[0].write = 0;

    return (0);
}




//AOsystSim_runWFS(2, "aosimwfsim");

int AOsystSim_runWFS(long index, const char *IDout_name)
{
    long cnt0;
    long IDinamp;
    char imnameamp[200];
    char imnamepha[200];
    int ret;

    ret = sprintf(imnameamp, "WFamp0_%03ld", index);
    ret = sprintf(imnamepha, "WFpha0_%03ld", index);
    IDinamp = image_ID(imnameamp);

    cnt0 = 0;

    while(1)
    {
        while(cnt0 == data.image[IDinamp].md[0].cnt0)
            usleep(50);
        cnt0 = data.image[IDinamp].md[0].cnt0;

        mk_complex_from_amph(imnameamp, imnamepha, "_tmpwfc", 0);
        AOsystSim_WFSsim_Pyramid("_tmpwfc", IDout_name, 0.0, 1);

        delete_image_ID("_tmpwfc");
    }


    return(0);
}

















int AOsystSim_mkWF_mkCONF(const char *fname)
{
    FILE *fp;

    fp = fopen(fname, "w");
    fprintf(fp, "# ======== AO simulation : creating atmospheric wavefronts stream =================\n");
	fprintf(fp, "# script relies on pre-computed physical atmospheric wavefronts\n");
	fprintf(fp, "\n\n");
	fprintf(fp, "MODE             0                # 0: read pre-computed WFs, 1: empty WFs\n");
    fprintf(fp, "WFDIR            ./atmwf          # atmospheric wavefronts directory\n");
    fprintf(fp, "\n");
    fprintf(fp, "LAMBDANM         1650             # wavelength [nm]\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== OUTPUT TYPE (OPD unit = um) ====================\n");
    fprintf(fp, "OUT0FITSFILE         2                # 0: none, 1 if FITS file output OPD only, 2 if OPD+AMP\n");
    fprintf(fp, "OUT0FITSFILENAMEOPD  wf0opd.fits      # FITS file name output (OPD)\n");
    fprintf(fp, "OUT0FITSFILENAMEAMP  wf0amp.fits      # FITS file name output (AMP)\n");
    fprintf(fp, "OUTPHYSTIME          aosim_phystime   # physical time [s]\n");
    fprintf(fp, "OUT0STREAM           2                # 1 if shared memory stream OPD only, 2 if OPD+AMP \n");
    fprintf(fp, "OUT0STREAMNAMEOPD    wf0opd           # output WF stream name (OPD)\n");
    fprintf(fp, "OUT0STREAMNAMEAMP    wf0amp           # output WF stream name (AMP)\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== PROCESS TRIGGER ==================================\n");
    fprintf(fp, "TRIGGERMODE           2                # 0: file, 1: semaphore from stream, 2: time interval\n");
    fprintf(fp, "TRIGGERFILE           wfup.txt         # update file name (TRIGGERMODE=0,1\n");
    fprintf(fp, "TRIGGERDT             0.05             # update interval (TRIGGERMODE=0,2)\n");
    fprintf(fp, "TRIGGER0STREAM        WFSinst          # trigger stream\n");
    fprintf(fp, "TRIGGER0SEM           5                # trigger semaphore in TRIGGERSTREAM\n");
    fprintf(fp, "TRIGGER1STREAM        dm05dispmap      # trigger stream\n");
    fprintf(fp, "TRIGGER1SEM           5                # trigger semaphore in TRIGGERSTREAM\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== PARAMETERS ======================================\n");
    fprintf(fp, "DT                    0.001            # time interval between computed wavefronts\n");
    fprintf(fp, "OPDMFACT              1.0              # OPD multiplicative factor\n");
    fprintf(fp, "AMPMFACT              1.0              # AMP multiplicative factor\n");
    fprintf(fp, "ARRAYSIZE             128              # output size [pix]\n");
    fprintf(fp, "PIXSCALEMODE          2                # 0: adopt input WF pixel scale, 1: custom pixel scale, 2: bin\n");
    fprintf(fp, "PIXSCALECUSTOM        0.1              # custom pixel size\n");
    fprintf(fp, "PIXBINFACTOR          4                # bin factor\n");
    fprintf(fp, "PUPDIAMM              8                # Pupil diameter [m]\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== POST-PROCESSING =============================\n");
	fprintf(fp, "DM0MODE               1                # 0 if no DM0, 1 if OPD DMn\n");
	fprintf(fp, "DM0NAME               dm05dispmap      # DM displacement map stream\n");
	fprintf(fp, "OUT1FITSFILE          0                # 1 if FITS file output\n");
	fprintf(fp, "OUT1FITSFILENAMEOPD  wf1opd.fits       # FITS file name output (OPD)\n");
    fprintf(fp, "OUT1FITSFILENAMEAMP  wf1amp.fits       # FITS file name output (AMP)\n");
    fprintf(fp, "OUT1STREAM           1                 # 1 if shared memory stream OPD only, 2 if OPD+AMP \n");
    fprintf(fp, "OUT1STREAMNAMEOPD     wf1opd           # output WF stream name (OPD)\n");
    fprintf(fp, "OUT1STREAMNAMEAMP     wf1amp           # output WF stream name (AMP)\n");
    fprintf(fp, "\n");       
    fprintf(fp, "\n");
    fclose(fp);

    return(0);
}



int AOsystSim_mkWF(const char *CONF_FNAME)
{
    FILE *fp;
    FILE *fp1;
    char fname[200];
    char wf_fname[500];

	int MODE;
    char WFDIR[500];
    float LAMBDA;
    char conf_atmWFconf_fname[500];

    int OUT0FITSFILE;
    char OUT0FITSFILENAMEOPD[200];
    char OUT0FITSFILENAMEAMP[200];
   
	char OUTPHYSTIME[200];
    int OUT0STREAM;
    char OUT0STREAMNAMEOPD[200];
    char OUT0STREAMNAMEAMP[200];

    int TRIGGERMODE;
    char TRIGGERFILE[200];
    float TRIGGERDT;
    char TRIGGER0STREAM[200];
    int TRIGGER0SEM;
    char TRIGGER1STREAM[200];
    int TRIGGER1SEM;

    float DT;
    float OPDMFACT;
    float AMPMFACT;
    long ARRAYSIZE;
    int PIXSCALEMODE;
    float PIXSCALECUSTOM;
    float PIXSCALE;
    int PIXBINFACTOR;
    float PUPDIAM;

    int DM0MODE;
    char DM0NAME[200];
    int OUT1FITSFILE;
    char OUT1FITSFILENAMEOPD[200];
    char OUT1FITSFILENAMEAMP[200];
    int OUT1STREAM;
    char OUT1STREAMNAMEOPD[200];
    char OUT1STREAMNAMEAMP[200];


	struct timespec ts;

    // read from atm turbb config file
    float wfin_TIME_STEP;
    long wfin_NB_TSPAN;
    float wfin_TIME_SPAN;
    char wfin_PREFIX[200];
    float wfin_PUPIL_SCALE;
    long wfin_WFsize;

    long ii, jj, ii1, jj1;
    int OKf;
    int OK;
    long k, k1;
    long kmax = 3;
    char wfimname_pha[200];
    char wfimname_amp[200];
    long *sizearray;

    long IDwf0, IDwf1;
    long IDwf0amp, IDwf1amp;
    long ID0, ID1, ID0amp, ID1amp, kk0, kk1;
    float alpha;

    double phystime;
    float frame_f;
    long frame_n;
    long NBframes;
    char command[200];
    int ret;

    long IDopd0, IDamp0, IDopd1, IDamp1;
    long iioffset, jjoffset;
    long iistart, iiend, jjstart, jjend, iisize, jjsize, csize;
    long IDbin_re, IDbin_im, IDbin_amp, IDbin_opd;
    long i, j;
    float opd, amp, re, im;
    int AMPfile = 1;
    long IDampmask;
    float pupscale;
    long ii1start0, jj1start0, ii1end0, jj1end0; // compute window in output array
    long ii1start, jj1start, ii1end, jj1end;
    long off1;
    long ID;

	long IDdm0opd;
	long IDphystime;

	int fOK;
	long knext;
	
	

    printf("AOsystSim mkWFS...\n");

	
	
    // INPUT WF
	if((fp=fopen(CONF_FNAME,"r"))==NULL)
    {
        sprintf(fname, "%s.default", CONF_FNAME);
        printf("configuration file %s not found. Creating default template as %s\n", CONF_FNAME, fname);
        AOsystSim_mkWF_mkCONF(fname);
        exit(0);
    }
    else
        fclose(fp);
		

    read_config_parameter(CONF_FNAME, "WFDIR", WFDIR);
    printf("WFDIR = %s\n", WFDIR);
    sprintf(conf_atmWFconf_fname, "%s/WFsim.conf", WFDIR);
    printf("WFsim.conf   : %s\n", conf_atmWFconf_fname);
    read_config_parameter(conf_atmWFconf_fname, "SWF_FILE_PREFIX", wfin_PREFIX);

    // OUTPUT PARAMETERS - WAVELENGTH
    LAMBDA = 1.0e-9*read_config_parameter_float(CONF_FNAME, "LAMBDANM");
    printf("LAMBDA = %.3f nm\n", LAMBDA*1.0e9);

    // OUTPUT STREAM 0
    OUT0FITSFILE = read_config_parameter_long(CONF_FNAME, "OUT0FITSFILE");
    read_config_parameter(CONF_FNAME, "OUT0FITSFILENAMEOPD", OUT0FITSFILENAMEOPD);
    read_config_parameter(CONF_FNAME, "OUT0FITSFILENAMEAMP", OUT0FITSFILENAMEAMP);
	
	read_config_parameter(CONF_FNAME, "OUTPHYSTIME", OUTPHYSTIME);
    OUT0STREAM = read_config_parameter_long(CONF_FNAME, "OUT0STREAM");
    read_config_parameter(CONF_FNAME, "OUT0STREAMNAMEOPD", OUT0STREAMNAMEOPD);
    read_config_parameter(CONF_FNAME, "OUT0STREAMNAMEAMP", OUT0STREAMNAMEAMP);

    // TRIGGER
    TRIGGERMODE = read_config_parameter_long(CONF_FNAME, "TRIGGERMODE");
    TRIGGERDT =  read_config_parameter_float(CONF_FNAME, "TRIGGERDT");
    read_config_parameter(CONF_FNAME, "TRIGGERFILE", TRIGGERFILE);
    read_config_parameter(CONF_FNAME, "TRIGGER0STREAM", TRIGGER0STREAM);
    TRIGGER0SEM = read_config_parameter_long(CONF_FNAME, "TRIGGER0SEM");
    read_config_parameter(CONF_FNAME, "TRIGGER1STREAM", TRIGGER1STREAM);
    TRIGGER1SEM = read_config_parameter_long(CONF_FNAME, "TRIGGER1SEM");

    // TIMING OUTPUT
    DT = read_config_parameter_float(CONF_FNAME, "DT");
    printf("DT = %.3f ms\n", DT*1.0e3);
    OPDMFACT = read_config_parameter_float(CONF_FNAME, "OPDMFACT");
    AMPMFACT = read_config_parameter_float(CONF_FNAME, "AMPMFACT");


    // GEOMETRY
    ARRAYSIZE =  read_config_parameter_long(CONF_FNAME, "ARRAYSIZE");
    PIXSCALEMODE =  read_config_parameter_long(CONF_FNAME, "PIXSCALEMODE");
    PIXSCALECUSTOM =  read_config_parameter_float(CONF_FNAME, "PIXSCALECUSTOM");
    PIXBINFACTOR =  read_config_parameter_long(CONF_FNAME, "PIXBINFACTOR");
    PUPDIAM =  read_config_parameter_float(CONF_FNAME, "PUPDIAMM");

    // OUTPUT STREAM 1
    DM0MODE =  read_config_parameter_long(CONF_FNAME, "DM0MODE");
    read_config_parameter(CONF_FNAME, "DM0NAME", DM0NAME);
    OUT1FITSFILE = read_config_parameter_long(CONF_FNAME, "OUT1FITSFILE");
    read_config_parameter(CONF_FNAME, "OUT1FITSFILENAMEOPD", OUT1FITSFILENAMEOPD);
    read_config_parameter(CONF_FNAME, "OUT1FITSFILENAMEAMP", OUT1FITSFILENAMEAMP);
    OUT1STREAM = read_config_parameter_long(CONF_FNAME, "OUT1STREAM");
    read_config_parameter(CONF_FNAME, "OUT1STREAMNAMEOPD", OUT1STREAMNAMEOPD);
    read_config_parameter(CONF_FNAME, "OUT1STREAMNAMEAMP", OUT1STREAMNAMEAMP);


    // TIMING INPUT
    wfin_TIME_STEP = read_config_parameter_float(conf_atmWFconf_fname, "WFTIME_STEP");
    printf("Input WF time step = %f\n", wfin_TIME_STEP);

    wfin_NB_TSPAN = read_config_parameter_long(conf_atmWFconf_fname, "NB_TSPAN");
    printf("Input WF NB_TSPAN = %ld\n", wfin_NB_TSPAN);

    wfin_TIME_SPAN = read_config_parameter_float(conf_atmWFconf_fname, "TIME_SPAN");
    printf("Input WF TIME_SPAN = %f\n", wfin_TIME_SPAN);

    NBframes = (long) ((wfin_TIME_SPAN+0.1*wfin_TIME_STEP)/wfin_TIME_STEP);
    printf("NBframes = %ld\n", NBframes);

    wfin_PUPIL_SCALE = read_config_parameter_float(conf_atmWFconf_fname, "PUPIL_SCALE");
    printf("Input WF PUPIL_SCALE = %f\n", wfin_PUPIL_SCALE);
    fp1 = fopen("pupscale.txt", "w");
    fprintf(fp1, "%f", wfin_PUPIL_SCALE);
    fclose(fp1);

    wfin_WFsize = read_config_parameter_long(conf_atmWFconf_fname, "WFsize");
    printf("Input WF WFsize = %ld\n", wfin_WFsize);
	
	
	
	sizearray = (long*) malloc(sizeof(long)*2);
	sizearray[0] = 1;
	sizearray[1] = 1;
	IDphystime = create_image_ID(OUTPHYSTIME, 2, sizearray, FLOAT, 1, 0);
	phystime = 0;
	data.image[IDphystime].array.F[0] = phystime;
	free(sizearray);


    if(PIXSCALEMODE!=2)
        PIXBINFACTOR =1;

    iioffset = (wfin_WFsize/PIXBINFACTOR - ARRAYSIZE)/2;
    if(iioffset<0)
        iioffset = 0;
    jjoffset = iioffset;

    pupscale = wfin_PUPIL_SCALE*PIXBINFACTOR;

    sizearray = (long*) malloc(sizeof(long)*2);
    sizearray[0] = ARRAYSIZE;
    sizearray[1] = ARRAYSIZE;



    if(OUT0STREAM>0)
    {
        IDopd0 = create_image_ID(OUT0STREAMNAMEOPD, 2, sizearray, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(OUT0STREAMNAMEOPD, 10);
    }
    else
        IDopd0 = create_image_ID(OUT0STREAMNAMEOPD, 2, sizearray, FLOAT, 0, 0);



    if(OUT0STREAM>0) // create amp stream if OUT0STREAM=1 or 2, but will only update it if OUT0STREAM=2
    {
        IDamp0 = create_image_ID(OUT0STREAMNAMEAMP, 2, sizearray, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(OUT0STREAMNAMEAMP, 10);
    }
    else
        IDopd0 = create_image_ID(OUT0STREAMNAMEAMP, 2, sizearray, FLOAT, 0, 0);
   

    if(DM0MODE>0)
    {
        if(OUT1STREAM>0)
        {
            IDopd1 = create_image_ID(OUT1STREAMNAMEOPD, 2, sizearray, FLOAT, 1, 0);
            COREMOD_MEMORY_image_set_createsem(OUT1STREAMNAMEOPD, 10);
        }
        else
            IDopd1 = create_image_ID(OUT1STREAMNAMEOPD, 2, sizearray, FLOAT, 0, 0);

        if(OUT1STREAM>0) // create amp stream if OUT0STREAM=1 or 2, but will only update it if OUT0STREAM=2
        {
            IDamp1 = create_image_ID(OUT1STREAMNAMEAMP, 2, sizearray, FLOAT, 1, 0);
            COREMOD_MEMORY_image_set_createsem(OUT1STREAMNAMEAMP, 10);
        }
        else
            IDopd1 = create_image_ID(OUT1STREAMNAMEAMP, 2, sizearray, FLOAT, 0, 0);
    }

    free(sizearray);

	if(DM0MODE>0)
	{
		IDdm0opd = read_sharedmem_image(DM0NAME);
		if(IDdm0opd==-1)
			{
				printf("ERROR: stream %s could not be loaded\n", DM0NAME);
				exit(0);
			}
		if( (data.image[IDdm0opd].md[0].size[0]!=ARRAYSIZE) || (data.image[IDdm0opd].md[0].size[1]!=ARRAYSIZE) )
			{
				printf("ERROR: stream %s has wrong size: is %ld x %ld, should be %ld x %ld\n", DM0NAME, data.image[IDdm0opd].md[0].size[0], data.image[IDdm0opd].md[0].size[1], ARRAYSIZE, ARRAYSIZE);
				exit(0);
			}
	}

    OK = 1;
    
    k = 0;
    k1 = 0;
    
    kmax = 3;
    AMPfile = 1;


	// compute kmax
	fOK = 1;
	kmax = 0;
	while(fOK == 1)
	{
		sprintf(wf_fname,"%s/%s%08ld.%09ld.pha.fits", WFDIR, wfin_PREFIX, kmax, (long) (1.0e12*LAMBDA+0.5));
		fOK = file_exists(wf_fname);
		kmax++;
	}	
	kmax -= 2;
	
	printf("kmax = %ld\n", kmax);
	//exit(0);


    IDampmask = make_disk("pupmask", ARRAYSIZE, ARRAYSIZE, 0.5*ARRAYSIZE, 0.5*ARRAYSIZE, 0.5*PUPDIAM/pupscale);
    for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)
        data.image[IDamp0].array.F[ii] = data.image[IDampmask].array.F[ii];
    ii1start0 = ARRAYSIZE;
    jj1start0 = ARRAYSIZE;
    ii1end0 = 0;
    jj1end0 = 0;
    for(ii1=0; ii1<ARRAYSIZE; ii1++)
        for(jj1=0; jj1<ARRAYSIZE; jj1++)
        {
            if(data.image[IDampmask].array.F[jj1*ARRAYSIZE+ii1]>1.0e-6)
            {
                if(ii1>ii1end0)
                    ii1end0 = ii1;
                if(ii1<ii1start0)
                    ii1start0 = ii1;
                if(jj1>jj1end0)
                    jj1end0 = jj1;
                if(jj1<jj1start0)
                    jj1start0 = jj1;
            }
        }
    ii1end ++;
    jj1end ++;


	
    while(1)
    {
        frame_f = (phystime - wfin_TIME_SPAN*k1)/wfin_TIME_STEP;
        frame_n = (long) frame_f;
        alpha = frame_f-frame_n;

        if(frame_n > (NBframes-1))
        {
            k++;
            if(k==kmax)
				k = 0;
            
            k1++;
            frame_n -= NBframes;
            frame_f -= wfin_TIME_SPAN;
        }

        sprintf(wfimname_pha, "wf%08ld_pha", k);
        sprintf(wfimname_amp, "wf%08ld_amp", k);
        if(image_ID(wfimname_pha)==-1)
        {
            sprintf(wf_fname,"%s/%s%08ld.%09ld.pha.fits", WFDIR, wfin_PREFIX, k, (long) (1.0e12*LAMBDA+0.5));
            printf("Loading WF file name : %s\n", wf_fname);
            sprintf(wfimname_pha, "wf%08ld_pha", k);
            IDwf0 = load_fits(wf_fname, wfimname_pha, 1);

            sprintf(wf_fname,"%s/%s%08ld.%09ld.amp.fits", WFDIR, wfin_PREFIX, k, (long) (1.0e12*LAMBDA+0.5));
            printf("Loading WF file name : %s\n", wf_fname);
            sprintf(wfimname_amp, "wf%08ld_amp", k);
            IDwf0amp = load_fits(wf_fname, wfimname_amp, 1);
            if(IDwf0amp==-1)
                AMPfile = 0;
        }
        IDwf0 = image_ID(wfimname_pha);
        if(AMPfile==1)
            IDwf0amp = image_ID(wfimname_amp);
            

		knext = k+1;
		if(k==kmax)
			knext = 0;

        sprintf(wfimname_pha, "wf%08ld_pha", knext);
        sprintf(wfimname_amp, "wf%08ld_amp", knext);
        if(image_ID(wfimname_pha)==-1)
        {
            sprintf(wf_fname,"%s/%s%08ld.%09ld.pha.fits", WFDIR, wfin_PREFIX, knext, (long) (1.0e12*LAMBDA+0.5));
            printf("Loading WF file name : %s\n", wf_fname);
            sprintf(wfimname_pha, "wf%08ld_pha", knext);
            IDwf1 = load_fits(wf_fname, wfimname_pha, 1);

            sprintf(wf_fname,"%s/%s%08ld.%09ld.amp.fits", WFDIR, wfin_PREFIX, knext, (long) (1.0e12*LAMBDA+0.5));
            printf("Loading WF file name : %s\n", wf_fname);
            sprintf(wfimname_amp, "wf%08ld_amp", knext);
            IDwf1amp = load_fits(wf_fname, wfimname_amp, 1);
            if(IDwf1amp==-1)
                AMPfile = 0;
        }
        IDwf1 = image_ID(wfimname_pha);
        if(AMPfile==1)
            IDwf1amp = image_ID(wfimname_amp);

        ID0 = IDwf0;
		if(AMPfile==1)
			ID0amp = IDwf0amp;
			
			
        if(frame_n+1 == NBframes)
        {
            kk0 = frame_n;
            kk1 = 0;
            ID1 = IDwf1;
            ID1amp = IDwf1amp;
        }
        else
        {
            ID1 = IDwf0;
            ID1amp = IDwf0amp;
            kk0 = frame_n;
            kk1 = kk0+1;
        }
        
        
        
        

/*        if(k>0) // delete (only for long sequence)
        {
            sprintf(wfimname_pha, "wf%08ld_pha", k-1);
            if(image_ID(wfimname_pha)!=-1)
                delete_image_ID(wfimname_pha);
            if(AMPfile==1)
            {
                sprintf(wfimname_pha, "wf%08ld_amp", k-1);
                if(image_ID(wfimname_amp)!=-1)
                    delete_image_ID(wfimname_amp);
            }
        }
*/


        printf("%.9f  %8ld  %12.6f  %6ld      %5ld %5ld  %.5f\n", phystime, k, frame_f, frame_n, kk0, kk1, alpha);


        // BIN
        if((PIXSCALEMODE==2)&&(PIXBINFACTOR!=1))
        {
            csize = wfin_WFsize/PIXBINFACTOR;

            IDbin_re = image_ID("tmpre");
            if(IDbin_re==-1)
                IDbin_re = create_2Dimage_ID("tmpre", csize, csize);

            IDbin_im = image_ID("tmpim");
            if(IDbin_im==-1)
                IDbin_im = create_2Dimage_ID("tmpim", csize, csize);

            IDbin_amp = image_ID("tmpamp");
            if(IDbin_amp==-1)
                IDbin_amp = create_2Dimage_ID("tmpamp", csize, csize);

            IDbin_opd = image_ID("tmpopd");
            if(IDbin_opd==-1)
                IDbin_opd = create_2Dimage_ID("tmpopd", csize, csize);

            ii1start = (csize - ARRAYSIZE)/2 + ii1start0;
            if(ii1start<0)
                ii1start = 0;
            ii1end = (csize - ARRAYSIZE)/2 + ii1end0;
            if(ii1end>csize)
                ii1end = csize;

            jj1start = (csize - ARRAYSIZE)/2 + jj1start0;
            if(jj1start<0)
                jj1start = 0;
            jj1end = (csize - ARRAYSIZE)/2 + jj1end0;
            if(jj1end>csize)
                jj1end = csize;

            off1 = (csize - ARRAYSIZE)/2; // offset between ii1 and output array

            if(AMPfile==1)
            {
                for(ii1=ii1start; ii1<ii1end; ii1++)
                    for(jj1=jj1start; jj1<jj1end; jj1++)
                    {
                        if(data.image[IDampmask].array.F[(jj1-off1)*ARRAYSIZE+(ii1-off1)]>1.0e-6)
                        {
                            data.image[IDbin_re].array.F[jj1*csize+ii1] = 0.0;
                            data.image[IDbin_im].array.F[jj1*csize+ii1] = 0.0;
                            for(i=0; i<PIXBINFACTOR; i++)
                                for(j=0; j<PIXBINFACTOR; j++)
                                {
                                    ii = PIXBINFACTOR*ii1+i;
                                    jj = PIXBINFACTOR*jj1+j;
                                    opd = OPDMFACT * ((1.0-alpha) * data.image[ID0].array.F[kk0*wfin_WFsize*wfin_WFsize+jj*wfin_WFsize+ii] + alpha * data.image[ID1].array.F[kk1*wfin_WFsize*wfin_WFsize+jj*wfin_WFsize+ii])/(2.0*M_PI)*LAMBDA*1.0e6;
                                    amp = ((1.0-alpha) * data.image[ID0amp].array.F[kk0*wfin_WFsize*wfin_WFsize+jj*wfin_WFsize+ii] + alpha * data.image[ID1amp].array.F[kk1*wfin_WFsize*wfin_WFsize+jj*wfin_WFsize+ii]);
                                    re = amp*cos(1.0e-6*opd/LAMBDA*2.0*M_PI);
                                    im = amp*sin(1.0e-6*opd/LAMBDA*2.0*M_PI);
                                    data.image[IDbin_re].array.F[jj1*csize+ii1] += re;
                                    data.image[IDbin_im].array.F[jj1*csize+ii1] += im;
                                    data.image[IDbin_opd].array.F[jj1*csize+ii1] += opd;
                                }
                            re = data.image[IDbin_re].array.F[jj1*csize+ii1];
                            im = data.image[IDbin_im].array.F[jj1*csize+ii1];
                            data.image[IDbin_amp].array.F[jj1*csize+ii1] = sqrt(re*re+im*im)/PIXBINFACTOR/PIXBINFACTOR;
                            data.image[IDbin_opd].array.F[jj1*csize+ii1] /= PIXBINFACTOR*PIXBINFACTOR;
                        }
                    }
            }
            else
            {
                for(ii1=ii1start; ii1<ii1end; ii1++)
                    for(jj1=jj1start; jj1<jj1end; jj1++)
                    {
                        if(data.image[IDampmask].array.F[(jj1-off1)*ARRAYSIZE+(ii1-off1)]>1.0e-6)
                        {   data.image[IDbin_re].array.F[jj1*csize+ii1] = 0.0;
                            data.image[IDbin_im].array.F[jj1*csize+ii1] = 0.0;
                            for(i=0; i<PIXBINFACTOR; i++)
                                for(j=0; j<PIXBINFACTOR; j++)
                                {
                                    ii = PIXBINFACTOR*ii1+i;
                                    jj = PIXBINFACTOR*jj1+j;
                                    opd = OPDMFACT * ((1.0-alpha) * data.image[ID0].array.F[kk0*wfin_WFsize*wfin_WFsize+jj*wfin_WFsize+ii] + alpha * data.image[ID1].array.F[kk1*wfin_WFsize*wfin_WFsize+jj*wfin_WFsize+ii])/(2.0*M_PI)*LAMBDA*1.0e6;
                                    amp = 1.0;
                                    re = amp*cos(1.0e-6*opd/LAMBDA*2.0*M_PI);
                                    im = amp*sin(1.0e-6*opd/LAMBDA*2.0*M_PI);
                                    data.image[IDbin_re].array.F[jj1*csize+ii1] += re;
                                    data.image[IDbin_im].array.F[jj1*csize+ii1] += im;
                                    data.image[IDbin_opd].array.F[jj1*csize+ii1] += opd;
                                }
                            re = data.image[IDbin_re].array.F[jj1*csize+ii1];
                            im = data.image[IDbin_im].array.F[jj1*csize+ii1];
                            data.image[IDbin_amp].array.F[jj1*csize+ii1] = sqrt(re*re+im*im)/PIXBINFACTOR/PIXBINFACTOR;
                            data.image[IDbin_opd].array.F[jj1*csize+ii1] /= PIXBINFACTOR*PIXBINFACTOR;
                        }
                    }
            }
        }




        // WRITE PIXELS
        iistart = 0;
        iiend = ARRAYSIZE;
        jjstart = 0;
        jjend = ARRAYSIZE;

        iisize = wfin_WFsize/PIXBINFACTOR;
        jjsize = wfin_WFsize/PIXBINFACTOR;

        iistart = (ARRAYSIZE/2)-(iisize/2);
        iiend = (ARRAYSIZE/2)+(iisize/2);
        if(iistart<0)
            iistart = 0;
        if(iiend>ARRAYSIZE)
            iiend = ARRAYSIZE;

        jjstart = iistart;
        jjend = iiend;






        data.image[IDopd0].md[0].write = 1;
        if(PIXBINFACTOR==1)
        {
            for(ii=iistart; ii<iiend; ii++)
                for(jj=jjstart; jj<jjend; jj++)
                {
                    ii1 = ii + iioffset;
                    jj1 = jj + jjoffset;
                    data.image[IDopd0].array.F[jj*ARRAYSIZE+ii] = OPDMFACT * ((1.0-alpha) * data.image[ID0].array.F[kk0*wfin_WFsize*wfin_WFsize+jj1*wfin_WFsize+ii1] + alpha * data.image[ID1].array.F[kk1*wfin_WFsize*wfin_WFsize+jj1*wfin_WFsize+ii1]);
                }
        }
        else
        {
            for(ii=iistart; ii<iiend; ii++)
                for(jj=jjstart; jj<jjend; jj++)
                {
                    ii1 = (ii-iistart) + iioffset;
                    jj1 = (jj-jjstart) + jjoffset;
                    data.image[IDopd0].array.F[jj*ARRAYSIZE+ii] = data.image[IDbin_opd].array.F[jj1*csize+ii1];
                }
        }
        data.image[IDopd0].md[0].cnt0++;
        data.image[IDopd0].md[0].write = 0;
        COREMOD_MEMORY_image_set_sempost(OUT0STREAMNAMEOPD, -1);






        if(OUT0STREAM==2)
        {
            data.image[IDamp0].md[0].write = 1;

            if(PIXBINFACTOR==1)
            {
                if(AMPfile==1)
                {
                    for(ii=iistart; ii<iiend; ii++)
                        for(jj=jjstart; jj<jjend; jj++)
                        {
                            ii1 = ii + iioffset;
                            jj1 = jj + jjoffset;
                            data.image[IDamp0].array.F[jj*ARRAYSIZE+ii] = AMPMFACT * data.image[IDampmask].array.F[jj*ARRAYSIZE+ii] * ((1.0-alpha) * data.image[ID0amp].array.F[kk0*wfin_WFsize*wfin_WFsize+jj1*wfin_WFsize+ii1] + alpha * data.image[ID1amp].array.F[kk1*wfin_WFsize*wfin_WFsize+jj1*wfin_WFsize+ii1]);
                        }
                }
                else
                {
                    for(ii=iistart; ii<iiend; ii++)
                        for(jj=jjstart; jj<jjend; jj++)
                        {
                            ii1 = ii + iioffset;
                            jj1 = jj + jjoffset;
                            data.image[IDamp0].array.F[jj*ARRAYSIZE+ii] = AMPMFACT * data.image[IDampmask].array.F[jj*ARRAYSIZE+ii];
                        }
                }
            }
            else
            {
                if(AMPfile==1)
                {
                    for(ii=iistart; ii<iiend; ii++)
                        for(jj=jjstart; jj<jjend; jj++)
                        {
                            ii1 = ii + iioffset;
                            jj1 = jj + jjoffset;
                            data.image[IDamp0].array.F[jj*ARRAYSIZE+ii] = AMPMFACT * data.image[IDampmask].array.F[jj*ARRAYSIZE+ii] * data.image[IDbin_amp].array.F[jj1*csize+ii1] * ((1.0-alpha) * data.image[ID0amp].array.F[kk0*wfin_WFsize*wfin_WFsize+jj1*wfin_WFsize+ii1] + alpha * data.image[ID1amp].array.F[kk1*wfin_WFsize*wfin_WFsize+jj1*wfin_WFsize+ii1]);
                        }
                }
                else
                {
                    for(ii=iistart; ii<iiend; ii++)
                        for(jj=jjstart; jj<jjend; jj++)
                        {
                            ii1 = (ii-iistart) + iioffset;
                            jj1 = (jj-jjstart) + jjoffset;
                            data.image[IDamp0].array.F[jj*ARRAYSIZE+ii] = AMPMFACT * data.image[IDampmask].array.F[jj*ARRAYSIZE+ii] * data.image[IDbin_amp].array.F[jj1*csize+ii1];
                        }
                }


            }
            data.image[IDamp0].md[0].cnt0++;
            data.image[IDamp0].md[0].write = 0;
            COREMOD_MEMORY_image_set_sempost(OUT0STREAMNAMEAMP, -1);
        }


        if(OUT0FITSFILE>0)
        {
            sprintf(command, "rm %s", OUT0FITSFILENAMEOPD);
            ret = system(command);
            sprintf(fname, "!%s", OUT0FITSFILENAMEOPD);
            save_fits(OUT0STREAMNAMEOPD, fname);
        }
        if((OUT0FITSFILE==2)&&(OUT0STREAM==2))
        {
            sprintf(command, "rm %s", OUT0FITSFILENAMEAMP);
            ret = system(command);
            sprintf(fname, "!%s", OUT0FITSFILENAMEAMP);
            save_fits(OUT0STREAMNAMEAMP, fname);
        }


        if(DM0MODE>0)
        {
			data.image[IDopd1].md[0].write = 1;
			data.image[IDamp1].md[0].write = 1;

            for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)                  
				data.image[IDopd1].array.F[ii] = data.image[IDopd0].array.F[ii] - 2.0*data.image[IDdm0opd].array.F[ii];
			memcpy((char*) data.image[IDamp1].array.F, (char*) data.image[IDamp0].array.F, sizeof(float)*ARRAYSIZE*ARRAYSIZE);
	        data.image[IDopd1].md[0].cnt0++;
	        data.image[IDamp1].md[0].cnt0++;
			data.image[IDopd1].md[0].write = 0;
			data.image[IDamp1].md[0].write = 0;
			COREMOD_MEMORY_image_set_sempost(OUT1STREAMNAMEOPD, -1);
			COREMOD_MEMORY_image_set_sempost(OUT1STREAMNAMEAMP, -1);			
			
			
			
        }


        phystime += DT;
        data.image[IDphystime].md[0].write = 0;
        data.image[IDphystime].array.F[0] = phystime;
		COREMOD_MEMORY_image_set_sempost_byID(IDphystime, -1);
        data.image[IDphystime].md[0].cnt0 ++;
        data.image[IDphystime].md[0].write = 0;

        switch(TRIGGERMODE) {
        case 0:
            OKf = 0;
            while(OKf==0)
            {
                usleep((long) (1.0e6*TRIGGERDT));
                if(file_exists(TRIGGERFILE)==1)
                {
                    sprintf(command, "rm %s", TRIGGERFILE);
                    ret = system(command);
                    OKf = 1;
                }
            }
            break;
        case 1:
            ID = image_ID(TRIGGER0STREAM);
            while(ID==-1)

            {
                usleep(1000000);
                ID = read_sharedmem_image(TRIGGER0STREAM);
            }
            COREMOD_MEMORY_image_set_semwait(TRIGGER0STREAM, TRIGGER0SEM);
            
            ID = image_ID(TRIGGER1STREAM);
            while(ID==-1)
            {
                usleep(1000000);
                ID = read_sharedmem_image(TRIGGER1STREAM);
            }
            COREMOD_MEMORY_image_set_semwait(TRIGGER1STREAM, TRIGGER1SEM);            
            break;
        case 2:
            usleep((long) (1.0e6*TRIGGERDT));
            break;
        default:
            printf("TRIGGERMODE = %d not valid\n", TRIGGERMODE);
            exit(0);
            break;
        }

		if(k>kmax)
			k = 0;

    }

    list_image_ID();

    return(0);
}














int AOsystSim_PyrWFS_mkCONF(const char *fname)
{
    FILE *fp;

    fp = fopen(fname, "w");
    fprintf(fp, "\n");
    fprintf(fp, "LAMBDANM          800             # wavelength [nm]\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== INPUT TYPE (OPD unit = um) ====================\n");
    fprintf(fp, "INMODE                   0                 # 0:stream, 1:file system (FITS)\n");
    fprintf(fp, "INSTREAMNAMEOPD          wf1opd            # input OPD stream\n");
    fprintf(fp, "INSTREAMNAMEAMP          wf1amp            # input AMP stream\n");
    fprintf(fp, "INSEMCHAN                5                 # input semaphore channel (using OPD input)\n");
    fprintf(fp, "INFITSFILENAMEOPD        inwfopd.fits      # input FITS file name (OPD)\n");
    fprintf(fp, "INFITSFILENAMEAMP        inwfamp.fits      # input FITS file name (AMP)\n");
    fprintf(fp, "INTRIGGERFILE            inwf.txt          # input trigger file\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== OUTPUT TYPE ===================================\n");
    fprintf(fp, "OUTMODE                  0                 # 0: stream, 1: file system\n");
    fprintf(fp, "OUTSTREAMNAME            pWFSim            # output image stream name \n");
    fprintf(fp, "OUTINSTSTREAMNAME        WFSinst           # output instantaneous image stream name \n");
    fprintf(fp, "OUTFITSFILENAME          PyrWFS_im.fits    # FITS file name output (intensity)\n");
    fprintf(fp, "OUTTRIGGERFILE           outpyr.txt        # output trigger file\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== FREQUENCY, GEOMETRY =============================\n");
	fprintf(fp, "ARRAYSIZE                256               # array size for computations\n");
	fprintf(fp, "PYRMODMODE               1                 # 0: no modulation, 1: modulation\n");
	fprintf(fp, "PYRMODAMP                0.0               # Modulation radius [l/D]\n");
	fprintf(fp, "PYRMODNBPT               1                 # number of pyramid modulation points\n");
	fprintf(fp, "PYRAPERTURE              50.0              # spatial filter aperture [l/D]\n");
	fprintf(fp, "PUPPIXDIAM               100.0             # pupil diameter [pix] - used to compute l/D scale\n");
	fprintf(fp, "PYRPUPSEP                1.2               # separation between pupil imagees (relative to pup diam)\n");
	fprintf(fp, "OUTBINFACT               2                 # output binning factor\n");
	fprintf(fp, "OUTARRAYSIZE             120               # output array size\n");
	fprintf(fp, "\n");
    fclose(fp);

    return(0);
}



int AOsystSim_PyrWFS(const char *CONF_FNAME)
{
    FILE *fp;
    char fname[200];
    long *sizearray;
    long k;
    long kmax = 100000000;

    float LAMBDA;
    int INMODE;
    char INSTREAMNAMEOPD[200];
    char INSTREAMNAMEAMP[200];
    int INSEMCHAN;
    char INFITSFILENAMEOPD[200];
    char INFITSFILENAMEAMP[200];
    char INTRIGGERFILE[200];
    int OUTMODE;
    char OUTSTREAMNAME[200];
    char OUTINSTSTREAMNAME[200];
    char OUTFITSFILENAME[200];
    char OUTTRIGGERFILE[200];
    long ARRAYSIZE;
    int PYRMODMODE;
    float PYRMODAMP;
    long PYRMODNBPT;
    float PYRAPERTURE;
    long PUPPIXDIAM;
    float PYRPUPSEP;
    long OUTBINFACT;
    long OUTARRAYSIZE;


    long IDinOPD, IDinAMP;
    long wfinsize;
    char command[200];
    int ret;
    long ii, jj, ii1, jj1;
    long IDout;
    int OKf;
    long IDpupamp, IDpuppha, IDfoca, IDfocp, IDfocamp, IDfocpha;
    long offset;

    long pmodpt;
    long IDpyr_amp, IDpyr_pha;
    float pcoeff = 100.0*M_PI;
    float x, y, PA, xc, yc;

    float fpscale = 1.0; // pix per l/D
    long IDpyrpupi, IDpupa;
    long i, j;
    long outoffset;
    int WRITEOUT;
    long IDoutinst;



    printf("AOsystSim PyrWFS...\n");


    // INPUT WF
    if((fp=fopen(CONF_FNAME,"r"))==NULL)
    {
        sprintf(fname, "%s.default", CONF_FNAME);
        printf("configuration file %s not found. Creating default template as %s\n", CONF_FNAME, fname);
        AOsystSim_PyrWFS_mkCONF(fname);
        exit(0);
    }
    else
        fclose(fp);

    LAMBDA = 1.0e-9*read_config_parameter_float(CONF_FNAME, "LAMBDANM");

    INMODE = read_config_parameter_long(CONF_FNAME, "INMODE");
    read_config_parameter(CONF_FNAME, "INSTREAMNAMEOPD", INSTREAMNAMEOPD);
    read_config_parameter(CONF_FNAME, "INSTREAMNAMEAMP", INSTREAMNAMEAMP);
    INSEMCHAN = read_config_parameter_long(CONF_FNAME, "INSEMCHAN");
    read_config_parameter(CONF_FNAME, "INFITSFILENAMEOPD", INFITSFILENAMEOPD);
    read_config_parameter(CONF_FNAME, "INFITSFILENAMEAMP", INFITSFILENAMEAMP);
    read_config_parameter(CONF_FNAME, "INTRIGGERFILE", INTRIGGERFILE);

    // OUTPUT STREAM
    OUTMODE = read_config_parameter_long(CONF_FNAME, "OUTMODE");
    read_config_parameter(CONF_FNAME, "OUTSTREAMNAME", OUTSTREAMNAME);
    read_config_parameter(CONF_FNAME, "OUTINSTSTREAMNAME", OUTINSTSTREAMNAME);
    read_config_parameter(CONF_FNAME, "OUTFITSFILENAME", OUTFITSFILENAME);
    read_config_parameter(CONF_FNAME, "OUTTRIGGERFILE", OUTTRIGGERFILE);

    ARRAYSIZE = read_config_parameter_long(CONF_FNAME, "ARRAYSIZE");
    PYRMODMODE = read_config_parameter_long(CONF_FNAME, "PYRMODMODE");
    PYRMODAMP = read_config_parameter_float(CONF_FNAME, "PYRMODAMP");
    PYRMODNBPT = read_config_parameter_long(CONF_FNAME, "PYRMODNBPT");
    PYRAPERTURE = read_config_parameter_float(CONF_FNAME, "PYRAPERTURE");
    PUPPIXDIAM = read_config_parameter_float(CONF_FNAME, "PUPPIXDIAM");
    PYRPUPSEP = read_config_parameter_float(CONF_FNAME, "PYRPUPSEP");
    OUTBINFACT = read_config_parameter_long(CONF_FNAME, "OUTBINFACT");
    OUTARRAYSIZE = read_config_parameter_long(CONF_FNAME, "OUTARRAYSIZE");


    fpscale = 1.0*ARRAYSIZE/PUPPIXDIAM;

    // PREPARE PYRAMID CUBE
    if(PYRMODMODE==0)
    {
        PYRMODNBPT = 1;
        PYRMODAMP = 0.0;
    }

    printf("PYRAPERTURE = %f  -> %f pix\n", PYRAPERTURE, PYRAPERTURE*fpscale);
    printf("PYTPUPSEP = %f  ->  %f\n", PYRPUPSEP, M_PI*PUPPIXDIAM*PYRPUPSEP);


    IDpyr_amp = create_3Dimage_ID("pyramp", ARRAYSIZE, ARRAYSIZE, PYRMODNBPT);
    IDpyr_pha = create_3Dimage_ID("pyrpha", ARRAYSIZE, ARRAYSIZE, PYRMODNBPT);

    for(pmodpt=0; pmodpt<PYRMODNBPT; pmodpt++)
    {
        PA = 2.0*M_PI*pmodpt/PYRMODNBPT;
        xc = PYRMODAMP * cos(PA);
        yc = PYRMODAMP * sin(PA);

        for(ii=0; ii<ARRAYSIZE; ii++)
            for(jj=0; jj<ARRAYSIZE; jj++)
            {
                x = 1.0*(ii-ARRAYSIZE/2) - xc*fpscale;
                y = 1.0*(jj-ARRAYSIZE/2) - yc*fpscale;

                data.image[IDpyr_pha].array.F[pmodpt*ARRAYSIZE*ARRAYSIZE+jj*ARRAYSIZE+ii] = M_PI*PUPPIXDIAM*PYRPUPSEP/ARRAYSIZE*(fabs(x)+fabs(y));
                if((fabs(ii-ARRAYSIZE/2)>PYRAPERTURE*fpscale)||(fabs(jj-ARRAYSIZE/2)>PYRAPERTURE*fpscale))
                    data.image[IDpyr_amp].array.F[pmodpt*ARRAYSIZE*ARRAYSIZE+jj*ARRAYSIZE+ii] = 0.0;
                else
                    data.image[IDpyr_amp].array.F[pmodpt*ARRAYSIZE*ARRAYSIZE+jj*ARRAYSIZE+ii] = 1.0;
            }
    }
    save_fits("pyrpha", "!pyrpha.fits");
    save_fits("pyramp", "!pyramp.fits");


    switch( INMODE ) {
    case 0:
        IDinOPD = read_sharedmem_image(INSTREAMNAMEOPD);
        while(IDinOPD==-1)
        {
			usleep(1000000);
			IDinOPD = read_sharedmem_image(INSTREAMNAMEOPD);
		}
        IDinAMP = read_sharedmem_image(INSTREAMNAMEAMP);
        while(IDinAMP==-1)
        {
			usleep(1000000);
			IDinAMP = read_sharedmem_image(INSTREAMNAMEAMP);
		}
        break;
    case 1:
        IDinOPD = load_fits(INFITSFILENAMEOPD, "inOPD", 1);
        IDinAMP = load_fits(INFITSFILENAMEAMP, "inAMP", 1);
        break;
    default:
        printf("INMODE value %d not valid\n", INMODE);
        exit(0);
        break;
    }

    sizearray = (long*) malloc(sizeof(long)*2);
    sizearray[0] = OUTARRAYSIZE; //data.image[IDinOPD].md[0].size[0];
    sizearray[1] = OUTARRAYSIZE; //data.image[IDinOPD].md[0].size[1];
    wfinsize = data.image[IDinOPD].md[0].size[0];
    if(OUTMODE==0)
    {
        IDout = create_image_ID(OUTSTREAMNAME, 2, sizearray, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(OUTSTREAMNAME, 10);

        sizearray[0] = ARRAYSIZE;
        sizearray[1] = ARRAYSIZE;
		IDoutinst = create_image_ID(OUTINSTSTREAMNAME, 2, sizearray, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(OUTINSTSTREAMNAME, 10);
    }
    else
    {
        IDout = create_image_ID(OUTSTREAMNAME, 2, sizearray, FLOAT, 0, 0);
		IDoutinst = create_image_ID(OUTINSTSTREAMNAME, 2, sizearray, FLOAT, 0, 0);
    }
    free(sizearray);





    // CREATE COMPUTING ARRAYS
    IDpupamp = create_2Dimage_ID("pupamp", ARRAYSIZE, ARRAYSIZE);
    IDpuppha = create_2Dimage_ID("puppha", ARRAYSIZE, ARRAYSIZE);

    IDfocamp = create_2Dimage_ID("focamp", ARRAYSIZE, ARRAYSIZE); // after pyramid
    IDfocpha = create_2Dimage_ID("focpha", ARRAYSIZE, ARRAYSIZE);

    IDpyrpupi = create_2Dimage_ID("pyrpupi", ARRAYSIZE, ARRAYSIZE);

    offset = (ARRAYSIZE-wfinsize)/2;

    pmodpt = 0;
    k = 0;
    while(k<kmax)
    {
        // compute focal plane CA
        for(ii=0; ii<wfinsize; ii++)
            for(jj=0; jj<wfinsize; jj++)
            {
                ii1 = ii+offset;
                jj1 = jj+offset;
                data.image[IDpuppha].array.F[jj1*ARRAYSIZE+ii1] = 2.0*M_PI*(1.0e-6*data.image[IDinOPD].array.F[jj*wfinsize+ii])/LAMBDA;
                data.image[IDpupamp].array.F[jj1*ARRAYSIZE+ii1] = data.image[IDinAMP].array.F[jj*wfinsize+ii];
            }

        mk_complex_from_amph("pupamp", "puppha", "wfc", 0);
        permut("wfc");
        do2dfft("wfc", "imc");
        delete_image_ID("wfc");
        permut("imc");
        mk_amph_from_complex("imc", "ima", "imp", 0);
        delete_image_ID("imc");
        IDfoca = image_ID("ima");
        IDfocp = image_ID("imp");




        for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)
        {
            data.image[IDfocamp].array.F[ii] = data.image[IDfoca].array.F[ii] * data.image[IDpyr_amp].array.F[pmodpt*ARRAYSIZE*ARRAYSIZE+ii];
            data.image[IDfocpha].array.F[ii] = data.image[IDfocp].array.F[ii] + data.image[IDpyr_pha].array.F[pmodpt*ARRAYSIZE*ARRAYSIZE+ii];
        }
        mk_complex_from_amph("focamp", "focpha", "focc", 0);
        permut("focc");
        do2dfft("focc", "pupc");
        delete_image_ID("focc");
        permut("pupc");
        mk_amph_from_complex("pupc", "pupa", "pupp", 0);
        IDpupa = image_ID("pupa");
        delete_image_ID("pupc");
        delete_image_ID("pupp");
        //for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)
          //  data.image[IDpyrpupi].array.F[ii] += data.image[IDpupa].array.F[ii]*data.image[IDpupa].array.F[ii];
       

		data.image[IDoutinst].md[0].write = 1;
		for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)
            data.image[IDoutinst].array.F[ii] = data.image[IDpupa].array.F[ii]*data.image[IDpupa].array.F[ii];
        data.image[IDoutinst].md[0].cnt0++;
        data.image[IDoutinst].md[0].write = 0;
		if(OUTMODE==0)
            COREMOD_MEMORY_image_set_sempost(OUTINSTSTREAMNAME, -1);
		delete_image_ID("pupa");

		for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)
            data.image[IDpyrpupi].array.F[ii] += data.image[IDoutinst].array.F[ii];

        pmodpt++;
        if(pmodpt==PYRMODNBPT)
        {
            pmodpt = 0;
            WRITEOUT = 1;
        }
        else
            WRITEOUT = 0;

	

        if(WRITEOUT==1)        // WRITE PIXELS
        {
            outoffset = (ARRAYSIZE-OUTBINFACT*OUTARRAYSIZE)/2;

            data.image[IDout].md[0].write = 1;
            for(ii=0; ii<OUTARRAYSIZE*OUTARRAYSIZE; ii++)
                data.image[IDout].array.F[ii] = 0.0;

            for(ii=0; ii<OUTARRAYSIZE; ii++)
                for(jj=0; jj<OUTARRAYSIZE; jj++)
                {
                    for(i=0; i<OUTBINFACT; i++)
                        for(j=0; j<OUTBINFACT; j++)
                        {
                            ii1 = ii*OUTBINFACT+i + outoffset;
                            jj1 = jj*OUTBINFACT+j + outoffset;
                            data.image[IDout].array.F[jj*OUTARRAYSIZE+ii] += data.image[IDpyrpupi].array.F[jj1*ARRAYSIZE+ii1];
                        }
                }


            data.image[IDout].md[0].cnt0++;
            data.image[IDout].md[0].write = 0;
            if(OUTMODE==0)
                COREMOD_MEMORY_image_set_sempost(OUTSTREAMNAME, -1);


            if(OUTMODE==1)
            {
                sprintf(command, "rm %s", OUTFITSFILENAME);
                ret = system(command);
                sprintf(fname, "!%s", OUTFITSFILENAME);
                save_fits(OUTSTREAMNAME, fname);
                sprintf(command, "touch %s", OUTTRIGGERFILE);
                ret = system(command);
            }
			for(ii=0; ii<ARRAYSIZE*ARRAYSIZE; ii++)
                data.image[IDpyrpupi].array.F[ii] = 0.0;
        }

        switch( INMODE ) {
        case 0:
            COREMOD_MEMORY_image_set_semwait(INSTREAMNAMEOPD, INSEMCHAN);
            break;
        case 1:
            OKf = 0;
            while(OKf==0)
            {
                usleep(10000);
                if(file_exists(INTRIGGERFILE)==1)
                {
                    sprintf(command, "rm %s", INTRIGGERFILE);
                    ret = system(command);
                    OKf = 1;
                }
            }
            break;
        default:
            printf("INMODE value %d not valid\n", INMODE);
            exit(0);
            break;
        }
    }


    free(sizearray);

    return(0);
}








//
// from DM dispacements to 2D DM map
// 
int AOsystSim_DM_mkCONF(const char *fname)
{
    FILE *fp;

    fp = fopen(fname, "w");
    fprintf(fp, "\n");       
    fprintf(fp, "# ============== AOsim: DM process =============================\n");
    fprintf(fp, "# DM process is triggered by WF OPD, not by DM command\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== INPUT & TRIGGER (OPD unit = um) ===============\n");
    fprintf(fp, "INMODE                   0                 # 0:stream, 1:file system (FITS)\n");
    fprintf(fp, "INSTREAMNAMEDM           dm05disp          # input DM command stream\n");
    fprintf(fp, "INFITSFILENAMEDM         dm05disp.fits     # input FITS file name (DM command)\n");
    fprintf(fp, "INTRIGSTREAMNAME         wf1opd            # input trigger stream\n");
    fprintf(fp, "INTRIGSEMCHAN            6                 # input semaphore channel (using OPD input)\n");
    fprintf(fp, "INTRIGGERFILE            inwf.txt          # input trigger file\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== OUTPUT TYPE ===================================\n");
    fprintf(fp, "OUTMODE                  0                 # 0: stream, 1: file system\n");
    fprintf(fp, "OUTSTREAMNAMEDM          dm05dispmap       # output WF stream name \n");
    fprintf(fp, "OUTFITSFILENAMEDM        dm05dispmap.fits  # FITS file name output [um]\n");
    fprintf(fp, "OUTTRIGGERFILE           outdm.txt         # output trigger file\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== GEOMETRY, TIME =============================\n");
	fprintf(fp, "ARRAYSIZE                128               # array size, pix\n");
	fprintf(fp, "DMRAD                    52                # DM radius on array\n");
	fprintf(fp, "DMDT                     0.0003            # DM time sampling\n");
	fprintf(fp, "NBTSAMPLES               100               # number of time samples\n");
	fprintf(fp, "DMLAGSTART               0.0003            # time lag start\n");
	fprintf(fp, "DMTIMECST                0.0001            # DM time constant\n");
	fprintf(fp, "\n");

    fclose(fp);

    return(0);
}



int AOsystSim_DM(const char *CONF_FNAME)
{
    FILE *fp;
    char fname[200];
    long *sizearray;
    long k;
    long kmax = 100000000;

    int INMODE;
    char INSTREAMNAMEDM[200];
    char INFITSFILENAMEDM[200];  
    char INTRIGSTREAMNAME[200];
    int INTRIGSEMCHAN;
    char INTRIGGERFILE[200];

    int OUTMODE;
    char OUTSTREAMNAMEDM[200];
    char OUTFITSFILENAMEDM[200];
    char OUTTRIGGERFILE[200];
    char OUTSTREAMNAMEOPD[200];
    char OUTSTREAMNAMEAMP[200];
    char OUTFITSFILENAMEOPD[200];
    char OUTFITSFILENAMEAMP[200];
   
    long ARRAYSIZE;
    float DMRAD;
    float DMDT;
    long NBTSAMPLES;
	float DMLAGSTART;
	float DMTIMECST;
    
	long DMsize;
	long IDout, IDout_tmp;
	int ret;
	char command[200];
	int OKf;
	long IDinTRIG, IDinDM;

	long DMnbact;
	long *imsize;
	long DMif;
	long ii, jj;
	float x, y;
	float sig;
	long mx, my;
	float rx, ry, rxif, ryif, u, t, v00, v01, v10, v11;
	long rxi, ryi;
	long IDif, IDifc;
	float ii0f,jj0f;
	double dmifscale = 4.0; // scale magnification between full DM map and DM influence function

	long IDdmdispC, IDdmdispC1;
	long act;

	long NBdmifcarray;
	float dmif_limit = 0.0001; // small value threshold [um]
	float *dmifcarray_value; // non-null values of DM influence function cube
	long *dmifcarray_act; // actuator index
	long *dmifcarray_iijj; // pixel index
	long kk, kkstart;
	float alpha;
	
	long nelement;



    printf("AOsystSim DM...\n");


    if((fp=fopen(CONF_FNAME,"r"))==NULL)
    {
        sprintf(fname, "%s.default", CONF_FNAME);
        printf("configuration file %s not found. Creating default template as %s\n", CONF_FNAME, fname);
        AOsystSim_DM_mkCONF(fname);
        exit(0);
    }
    else
        fclose(fp);


    INMODE = read_config_parameter_long(CONF_FNAME, "INMODE");
    read_config_parameter(CONF_FNAME, "INSTREAMNAMEDM", INSTREAMNAMEDM);            // IDinDM
	read_config_parameter(CONF_FNAME, "INFITSFILENAMEDM", INFITSFILENAMEDM);        
	read_config_parameter(CONF_FNAME, "INTRIGSTREAMNAME", INTRIGSTREAMNAME);          // IDinTRIG
    INTRIGSEMCHAN = read_config_parameter_long(CONF_FNAME, "INTRIGSEMCHAN");
	read_config_parameter(CONF_FNAME, "INTRIGGERFILE", INTRIGGERFILE);    
  
    // OUTPUT STREAM
    OUTMODE = read_config_parameter_long(CONF_FNAME, "OUTMODE");
	read_config_parameter(CONF_FNAME, "OUTSTREAMNAMEDM", OUTSTREAMNAMEDM);
	read_config_parameter(CONF_FNAME, "OUTFITSFILENAMEDM", OUTFITSFILENAMEDM);
	read_config_parameter(CONF_FNAME, "OUTTRIGGERFILE", OUTTRIGGERFILE);
	read_config_parameter(CONF_FNAME, "OUTSTREAMNAMEOPD", OUTSTREAMNAMEOPD);
	read_config_parameter(CONF_FNAME, "OUTSTREAMNAMEAMP", OUTSTREAMNAMEAMP);
	read_config_parameter(CONF_FNAME, "OUTFITSFILENAMEOPD", OUTFITSFILENAMEOPD);
	read_config_parameter(CONF_FNAME, "OUTFITSFILENAMEAMP", OUTFITSFILENAMEAMP);
 
 	ARRAYSIZE = read_config_parameter_long(CONF_FNAME, "ARRAYSIZE");
	DMRAD = read_config_parameter_float(CONF_FNAME, "DMRAD");
	DMDT = read_config_parameter_float(CONF_FNAME, "DMDT");
	NBTSAMPLES = read_config_parameter_long(CONF_FNAME, "NBTSAMPLES");
	DMLAGSTART = read_config_parameter_float(CONF_FNAME, "DMLAGSTART");
	DMTIMECST = read_config_parameter_float(CONF_FNAME, "DMTIMECST");




    switch( INMODE ) {
    case 0:
		IDinDM = read_sharedmem_image(INSTREAMNAMEDM);
		while(IDinDM==-1)
			{
				usleep(100000);
				IDinDM = read_sharedmem_image(INSTREAMNAMEDM);
			}
    
        IDinTRIG = read_sharedmem_image(INTRIGSTREAMNAME);
        while(IDinTRIG==-1)
			{
				usleep(100000);
				IDinTRIG = read_sharedmem_image(INTRIGSTREAMNAME);
			}
        break;
    case 1:
        break;
    default:
        printf("INMODE value %d not valid\n", INMODE);
        exit(0);
        break;
    }

	

    sizearray = (long*) malloc(sizeof(long)*2);
    sizearray[0] = ARRAYSIZE; 
    sizearray[1] = ARRAYSIZE; 
    DMsize = data.image[IDinDM].md[0].size[0];
    if(OUTMODE==0)
    {
        IDout = create_image_ID(OUTSTREAMNAMEDM, 2, sizearray, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(OUTSTREAMNAMEDM, 10);
    }
    else
        IDout = create_image_ID(OUTSTREAMNAMEDM, 2, sizearray, FLOAT, 0, 0);
        
    IDout_tmp = create_image_ID("tmpDMshape", 2, sizearray, FLOAT, 0, 0);
    free(sizearray);



	// CREATE DM TEMPORAL RESPONSE CUBE
	IDdmdispC = create_3Dimage_ID("dmdispC", DMsize, DMsize, NBTSAMPLES);
	IDdmdispC1 = create_3Dimage_ID("dmdispC1", DMsize, DMsize, NBTSAMPLES);



   // MAKE DM INFLUENCE FUNCTIONS

    // DM influence functions stored as a data cube
    DMnbact = DMsize*DMsize;
    imsize = (long*) malloc(sizeof(long)*3);
    imsize[0] = ARRAYSIZE;
    imsize[1] = ARRAYSIZE;
    imsize[2] = DMnbact;
    IDif = create_image_ID("dmif0", 2, imsize, FLOAT, 0, 0);
    // construct DM influence function (for 1 actuator)
    // step 1: square
    // actuator size = dmifscale*(arraysize*2.0*dmrad/DMsize) [pix]
    printf("DM size = %ld x %ld actuators\n", DMsize, DMsize);
    printf("actuator pix size = %f pix\n", dmifscale*(2.0*DMRAD/DMsize));

    list_image_ID();
    for(ii=0; ii<ARRAYSIZE; ii++)
        for(jj=0; jj<ARRAYSIZE; jj++)
        {
            x = (1.0*ii-0.5*ARRAYSIZE); // [pix]
            y = (1.0*jj-0.5*ARRAYSIZE); // [pix]
            x /= dmifscale*(2.0*DMRAD/DMsize);
            y /= dmifscale*(2.0*DMRAD/DMsize);
            if((fabs(x)<0.5)&&(fabs(y)<0.5))
                data.image[IDif].array.F[jj*ARRAYSIZE+ii] = 1.0;
            else
                data.image[IDif].array.F[jj*ARRAYSIZE+ii] = 0.0;
        }
    printf("convolve\n");
    fflush(stdout);
    // convolve dmif
    save_fits("dmif0", "!dmif0.fits");
    sig = 0.5*dmifscale*(2.0*DMRAD/DMsize);
    printf("gauss filter   %lf %ld\n", sig, (long) (2.0*sig));
    fflush(stdout);
    gauss_filter("dmif0", "dmif", sig, (long) (2.0*sig));
    list_image_ID();
    delete_image_ID("dmif0");
    IDif = image_ID("dmif");

    save_fits("dmif", "!dmif.fits");

    IDifc = create_image_ID("dmifc", 3, imsize, FLOAT, 0, 0);
    printf("\n");


    printf("dmifc = %ld   %ld %ld %ld    %ld %ld\n", IDifc, data.image[IDifc].md[0].size[0], data.image[IDifc].md[0].size[1], data.image[IDifc].md[0].size[2], ARRAYSIZE, ARRAYSIZE);


	NBdmifcarray = 0;
    for(mx=0; mx<DMsize; mx++)
        for(my=0; my<DMsize; my++)
        {
            printf("\r actuator %2ld %2ld    ", mx, my);
            fflush(stdout);
            // actuator center coordinates
            // center: mx = DMsize/2-0.5  4 -> 1.5
            ii0f = 0.5*ARRAYSIZE + 2.0*(mx-DMsize/2)/DMsize*DMRAD;
            jj0f = 0.5*ARRAYSIZE + 2.0*(my-DMsize/2)/DMsize*DMRAD;
            for(ii=0; ii<ARRAYSIZE; ii++)
                for(jj=0; jj<ARRAYSIZE; jj++)
                {
                    rx = 1.0*ii-ii0f;
                    ry = 1.0*jj-jj0f;
                    rx *= dmifscale;
                    ry *= dmifscale;
                    rxif = rx + ARRAYSIZE/2;
                    ryif = ry + ARRAYSIZE/2;
                    rxi = (long) (rxif);
                    ryi = (long) (ryif);
                    u = rxif - rxi;
                    t = ryif - ryi;

                    if((rxi>0)&&(rxi<ARRAYSIZE-1)&&(ryi>0)&&(ryi<ARRAYSIZE-1))
                    {
                        v00 = data.image[IDif].array.F[ryi*ARRAYSIZE+rxi];
                        v01 = data.image[IDif].array.F[ryi*ARRAYSIZE+rxi+1];
                        v10 = data.image[IDif].array.F[(ryi+1)*ARRAYSIZE+rxi];
                        v11 = data.image[IDif].array.F[(ryi+1)*ARRAYSIZE+rxi+1];
                        data.image[IDifc].array.F[(my*DMsize+mx)*ARRAYSIZE*ARRAYSIZE + jj*ARRAYSIZE + ii] = (1.0-u)*(1.0-t)*v00 + (1.0-u)*t*v10 + u*(1.0-t)*v01 + u*t*v11;
						if(fabs(data.image[IDifc].array.F[(my*DMsize+mx)*ARRAYSIZE*ARRAYSIZE + jj*ARRAYSIZE + ii])>dmif_limit)
							NBdmifcarray++;
                    }
                }
        }
    free(imsize);
    save_fits("dmifc","!dmifc.fits");
    printf("\n");

	
	dmifcarray_value = (float*) malloc(sizeof(float)*NBdmifcarray);
	dmifcarray_act = (long*) malloc(sizeof(long)*NBdmifcarray);
	dmifcarray_iijj = (long*) malloc(sizeof(long)*NBdmifcarray);


	NBdmifcarray = 0;
    for(mx=0; mx<DMsize; mx++)
        for(my=0; my<DMsize; my++)
        {
			for(ii=0; ii<ARRAYSIZE; ii++)
                for(jj=0; jj<ARRAYSIZE; jj++)
                {
					if(fabs(data.image[IDifc].array.F[(my*DMsize+mx)*ARRAYSIZE*ARRAYSIZE + jj*ARRAYSIZE + ii])>dmif_limit)
						{
							dmifcarray_value[NBdmifcarray] = data.image[IDifc].array.F[(my*DMsize+mx)*ARRAYSIZE*ARRAYSIZE + jj*ARRAYSIZE + ii];
							dmifcarray_act[NBdmifcarray] = my*DMsize+mx;
							dmifcarray_iijj[NBdmifcarray] = jj*ARRAYSIZE + ii;
							NBdmifcarray++;
						}
							
				}
		}





    k = 0;
    while(k<kmax)
    {		

        // WRITE PIXELS
        
        // APPLY DM SHAPE (which was computed on the last iteration)  
		
		memcpy ((char*) data.image[IDdmdispC1].array.F, (char*) data.image[IDdmdispC].array.F, sizeof(float)*DMsize*DMsize*NBTSAMPLES);  
		memcpy ((char*) data.image[IDdmdispC].array.F, (char*) data.image[IDdmdispC1].array.F + sizeof(float)*DMsize*DMsize, sizeof(float)*DMsize*DMsize*(NBTSAMPLES-1));  


        kkstart = (long) (DMLAGSTART/DMDT);
        alpha = pow(0.5, DMDT/DMTIMECST);
  
      
        for(kk=kkstart; kk<NBTSAMPLES; kk++)
			for(ii=0;ii<DMsize*DMsize;ii++)
				data.image[IDdmdispC].array.F[kk*DMsize*DMsize+ii] = data.image[IDinDM].array.F[ii] - alpha*(data.image[IDinDM].array.F[ii] - data.image[IDdmdispC].array.F[kk*DMsize*DMsize+ii]);
        
  
        
        // COMPUTE NEW DM SHAPE
        nelement = ARRAYSIZE*ARRAYSIZE;
        memset(data.image[IDout_tmp].array.F, '\0', sizeof(float)*nelement);
		for(kk=0;kk<NBdmifcarray;kk++)
			data.image[IDout_tmp].array.F[dmifcarray_iijj[kk]] += data.image[IDdmdispC].array.F[dmifcarray_act[kk]] * dmifcarray_value[kk];            
 
        
        data.image[IDout].md[0].write = 1;
		memcpy (data.image[IDout].array.F, data.image[IDout_tmp].array.F, sizeof(float)*nelement);
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
        if(OUTMODE==0)
            COREMOD_MEMORY_image_set_sempost(OUTSTREAMNAMEDM, -1);


        if(OUTMODE==1)
        {
            sprintf(command, "rm %s", OUTFITSFILENAMEDM);
            ret = system(command);
            sprintf(fname, "!%s", OUTFITSFILENAMEDM);
            save_fits(OUTSTREAMNAMEDM, fname);
            sprintf(command, "touch %s", OUTTRIGGERFILE);
            ret = system(command);
        }

        switch( INMODE ) {
        case 0:
            COREMOD_MEMORY_image_set_semwait(INTRIGSTREAMNAME, INTRIGSEMCHAN);
            break;
        case 1:
            OKf = 0;
            while(OKf==0)
            {
                usleep(10000);
                if(file_exists(INTRIGGERFILE)==1)
                {
                    sprintf(command, "rm %s", INTRIGGERFILE);
                    ret = system(command);
                    OKf = 1;
                }
            }
            break;
        default:
            printf("INMODE value %d not valid\n", INMODE);
            exit(0);
            break;
        }
		k++;
    }


    free(sizearray);
    free(dmifcarray_value);
    free(dmifcarray_act);
    free(dmifcarray_iijj);

    return(0);
}












//
// from DM dispacements to 2D DM map
// 
int AOsystSim_coroLOWFS_mkCONF(const char *fname)
{
    FILE *fp;

    fp = fopen(fname, "w");
    fprintf(fp, "\n");       
    fprintf(fp, "# ============== AOsim: coro LOWFS process =============================\n");
    fprintf(fp, "# LOWFS process is triggered by WF OPD\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== INPUT & TRIGGER (OPD unit = um) ===============\n");
    fprintf(fp, "INMODE                   0                 # 0:stream, 1:file system (FITS)\n");
    fprintf(fp, "INAMPSTREAMNAME          wf1amp            # input AMP stream\n");
    fprintf(fp, "INOPDSTREAMNAME          wf1opd            # input OPD stream\n");
    fprintf(fp, "INAMPFITSFILENAME        wf1amp.fits       # input FITS file name\n");
    fprintf(fp, "INOPDFITSFILENAME        wf1opd.fits       # input FITS file name\n");
    fprintf(fp, "INTRIGSTREAMNAME         wf1opd            # input trigger stream\n");
    fprintf(fp, "INTRIGSEMCHAN            4                 # input semaphore channel (using OPD input)\n");
    fprintf(fp, "INTRIGGERFILE            inwf.txt          # input trigger file\n");
    fprintf(fp, "INPHYSTIME               aosim_phystime    # physical time\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== OUTPUT TYPE ===================================\n");
    fprintf(fp, "OUTMODE                  0                       # 0: stream, 1: file system\n");
    fprintf(fp, "OUTLOWFSARRAYSIZE        16                      # output LOWFS array size, pix\n");
    fprintf(fp, "OUTLOWFSSTREAMNAME       aosim_imcamLOWFS        # output WF stream name \n");
    fprintf(fp, "OUTLOWFSFITSFILENAME     aosim_imcamLOWFS.fits   # FITS file name output [um]\n");
    fprintf(fp, "OUTTRIGGERFILE           outLOWFS.txt            # output trigger file\n");
    fprintf(fp, "\n");
    fprintf(fp, "# ============== GEOMETRY, OPTICAL DESIGN =============================\n");
    fprintf(fp, "LAMBDAum                 1.65              # wavelength [um]\n");
	fprintf(fp, "ARRAYSIZE                256               # array size, pix, used for internal computations\n");
	fprintf(fp, "COROFPMAMP               corofpmamp        # coronagraph focal plane mask amplitude\n");
	fprintf(fp, "COROFPMPHA               corofpmpha        # coronagraph focal plane mask amplitude\n");
	fprintf(fp, "LYOTSTOPREFLAMP          lyotstopreflamp   # LYOT stop reflectivity map (amp)\n");
	fprintf(fp, "LYOTSTOPREFLPHA          lyotstopreflpha   # LYOT stop reflectivity map (pha)\n");
	fprintf(fp, "LYOTSTOPTRANSMAMP        lyotstoptransmamp # LYOT stop transmission map (amp)\n");
	fprintf(fp, "LYOTSTOPTRANSMPHA        lyotstoptransmpha # LYOT stop transmission map (pha)\n");
	fprintf(fp, "\n");
	fprintf(fp, "# ============== LOWFS =============================\n");
	fprintf(fp, "LOWFSOPDMAP              lowfsopdmap       # LOWFS defocus map [um]\n");
	fprintf(fp, "LOWFSCAMETIME            0.01              # LOWFS camera exposure time [s]\n");
	

	fprintf(fp, "\n");

    fclose(fp);

    return(0);
}




int AOsystSim_coroLOWFS(const char *CONF_FNAME)
{
    FILE *fp;
    char fname[200];
    long *sizearray;
    long k;
    long kmax = 100000000;

    int INMODE;
    char INAMPSTREAMNAME[200];
    char INOPDSTREAMNAME[200];
    char INAMPFITSFILENAME[200];  
    char INOPDFITSFILENAME[200];  
    char INTRIGSTREAMNAME[200];
    int INTRIGSEMCHAN;
    char INTRIGGERFILE[200];
    char INPHYSTIME[200];

    int OUTMODE;
    long OUTLOWFSARRAYSIZE;
    char OUTLOWFSSTREAMNAME[200];
    char OUTLOWFSFITSFILENAME[200];
    char OUTTRIGGERFILE[200];
	char COROFPMAMP[200];
	char COROFPMPHA[200];
	char LYOTSTOPREFLAMP[200];
	char LYOTSTOPREFLPHA[200];
	char LYOTSTOPTRANSMAMP[200];
	char LYOTSTOPTRANSMPHA[200];

	char LOWFSOPDMAP[200];
	float LOWFSCAMETIME;
	float LOWFScamstarttime = 0.0;
	long LOWFScamcnt = 0;
	
    long ARRAYSIZE;
    long ARRAYSIZE2;
    double LAMBDA;
  	   
	long DMsize;
	long IDoutLOWFS, IDout_tmp;
	int ret;
	char command[200];
	int OKf;
	long IDinTRIG, IDinOPD, IDinAMP;

	long *imsize;
	long xsizein, ysizein;
	long ii, jj;
	long ii1, jj1;
	long iioffset, jjoffset;

	long IDfpmamp, IDfpmpha;
	long ID_LS_ramp, ID_LS_rpha; // Lyot stop reflected amplitude and phase
	long ID_LS_tamp, ID_LS_tpha; // Lyot stop transmitted amplitude and phase
	long ID_lowfsopd;

	long IDwfa, IDwfp;
	long IDfoc0a, IDfoc0p;
	long IDfoc1a, IDfoc1p;
	long IDpup1a, IDpup1p;
	long IDpup1ta, IDpup1tp;
	long IDpup1ra, IDpup1rp;
	long IDfoclowfsa;
	long IDimlowfs;

	long IDphystime;
	long IDimcamlowfstmp;


    printf("AOsystSim coro LOWFS...\n");


    if((fp=fopen(CONF_FNAME,"r"))==NULL)
    {
        sprintf(fname, "%s.default", CONF_FNAME);
        printf("configuration file %s not found. Creating default template as %s\n", CONF_FNAME, fname);
        AOsystSim_coroLOWFS_mkCONF(fname);
        exit(0);
    }
    else
        fclose(fp);


    INMODE = read_config_parameter_long(CONF_FNAME, "INMODE");
    read_config_parameter(CONF_FNAME, "INAMPSTREAMNAME", INAMPSTREAMNAME);            // IDinAMP
    read_config_parameter(CONF_FNAME, "INOPDSTREAMNAME", INOPDSTREAMNAME);            // IDinOPD
	read_config_parameter(CONF_FNAME, "INAMPFITSFILENAME", INAMPFITSFILENAME);        
	read_config_parameter(CONF_FNAME, "INOPDFITSFILENAME", INOPDFITSFILENAME);        
	read_config_parameter(CONF_FNAME, "INTRIGSTREAMNAME", INTRIGSTREAMNAME);          // IDinTRIG
    INTRIGSEMCHAN = read_config_parameter_long(CONF_FNAME, "INTRIGSEMCHAN");
	read_config_parameter(CONF_FNAME, "INTRIGGERFILE", INTRIGGERFILE);    
	read_config_parameter(CONF_FNAME, "INPHYSTIME", INPHYSTIME); 
  
    // OUTPUT STREAM
    OUTMODE = read_config_parameter_long(CONF_FNAME, "OUTMODE");
    OUTLOWFSARRAYSIZE = read_config_parameter_long(CONF_FNAME, "OUTLOWFSARRAYSIZE");
	read_config_parameter(CONF_FNAME, "OUTLOWFSSTREAMNAME", OUTLOWFSSTREAMNAME);
	read_config_parameter(CONF_FNAME, "OUTLOWFSFITSFILENAME", OUTLOWFSFITSFILENAME);
	read_config_parameter(CONF_FNAME, "OUTTRIGGERFILE", OUTTRIGGERFILE);

	LAMBDA = 1.0e-6 * read_config_parameter_float(CONF_FNAME, "LAMBDAum");
 	ARRAYSIZE = read_config_parameter_long(CONF_FNAME, "ARRAYSIZE");
 	ARRAYSIZE2 = ARRAYSIZE*ARRAYSIZE;
 	read_config_parameter(CONF_FNAME, "COROFPMAMP", COROFPMAMP);
 	read_config_parameter(CONF_FNAME, "COROFPMPHA", COROFPMPHA);
 	read_config_parameter(CONF_FNAME, "LYOTSTOPREFLAMP", LYOTSTOPREFLAMP);
 	read_config_parameter(CONF_FNAME, "LYOTSTOPREFLPHA", LYOTSTOPREFLPHA);
 	read_config_parameter(CONF_FNAME, "LYOTSTOPTRANSMAMP", LYOTSTOPTRANSMAMP);
 	read_config_parameter(CONF_FNAME, "LYOTSTOPTRANSMPHA", LYOTSTOPTRANSMPHA);

	read_config_parameter(CONF_FNAME, "LOWFSOPDMAP", LOWFSOPDMAP);
	LOWFSCAMETIME = read_config_parameter_float(CONF_FNAME, "LOWFSCAMETIME");
	
	
	IDphystime = read_sharedmem_image(INPHYSTIME);
	

    switch( INMODE ) {
    case 0:
		IDinOPD = read_sharedmem_image(INOPDSTREAMNAME);
		while(IDinOPD==-1)
			{
				usleep(100000);
				IDinOPD = read_sharedmem_image(INOPDSTREAMNAME);
			}
    
		IDinAMP = read_sharedmem_image(INAMPSTREAMNAME);
		while(IDinOPD==-1)
			{
				usleep(100000);
				IDinOPD = read_sharedmem_image(INAMPSTREAMNAME);
			}
    
        IDinTRIG = read_sharedmem_image(INTRIGSTREAMNAME);
        while(IDinTRIG==-1)
			{
				usleep(100000);
				IDinTRIG = read_sharedmem_image(INTRIGSTREAMNAME);
			}
        break;
    case 1:
        break;
    default:
        printf("INMODE value %d not valid\n", INMODE);
        exit(0);
        break;
    }

	if((IDfpmamp = image_ID(COROFPMAMP))==-1)
		{
			printf("ERROR: image %s not loaded\n", COROFPMAMP);
			exit(0);
		}
	
	if((IDfpmpha = image_ID(COROFPMPHA))==-1)
		{
			printf("ERROR: image %s not loaded\n", COROFPMPHA);
			exit(0);
		}
	
	if((ID_LS_ramp = image_ID(LYOTSTOPREFLAMP))==-1)
		{
			printf("ERROR: image %s not loaded\n", LYOTSTOPREFLAMP);
			exit(0);
		}

	if((ID_LS_rpha = image_ID(LYOTSTOPREFLPHA))==-1)
		{
			printf("ERROR: image %s not loaded\n", LYOTSTOPREFLPHA);
			exit(0);
		}

	if((ID_LS_tamp = image_ID(LYOTSTOPTRANSMAMP))==-1)
		{
			printf("ERROR: image %s not loaded\n", LYOTSTOPTRANSMAMP);
			exit(0);
		}

	if((ID_LS_tpha = image_ID(LYOTSTOPTRANSMPHA))==-1)
		{
			printf("ERROR: image %s not loaded\n", LYOTSTOPTRANSMPHA);
			exit(0);
		}

	if((ID_lowfsopd = image_ID(LOWFSOPDMAP))==-1)
		{
			printf("ERROR: image %s not loaded\n", LOWFSOPDMAP);
			exit(0);
		}

	
	

    sizearray = (long*) malloc(sizeof(long)*2);
    sizearray[0] = OUTLOWFSARRAYSIZE; 
    sizearray[1] = OUTLOWFSARRAYSIZE; 
    DMsize = data.image[IDinOPD].md[0].size[0];
    if(OUTMODE==0)
    {
        IDoutLOWFS = create_image_ID(OUTLOWFSSTREAMNAME, 2, sizearray, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(OUTLOWFSSTREAMNAME, 10);
    }
    else
        IDoutLOWFS = create_image_ID(OUTLOWFSSTREAMNAME, 2, sizearray, FLOAT, 0, 0);
	
	IDimlowfs = create_image_ID("aosim_imlowfs", 2, sizearray, FLOAT, 1, 0);

    sizearray[0] = ARRAYSIZE; 
    sizearray[1] = ARRAYSIZE; 

	IDfoc1a = create_image_ID("aosim_foc1_amp", 2, sizearray, FLOAT, 1, 0);
	IDfoc1p = create_image_ID("aosim_foc1_pha", 2, sizearray, FLOAT, 1, 0);
    
	IDpup1ta = create_image_ID("aosim_pup1t_amp", 2, sizearray, FLOAT, 1, 0);
	IDpup1tp = create_image_ID("aosim_pup1t_pha", 2, sizearray, FLOAT, 1, 0);
    
	IDpup1ra = create_image_ID("aosim_pup1r_amp", 2, sizearray, FLOAT, 1, 0);
	IDpup1rp = create_image_ID("aosim_pup1r_pha", 2, sizearray, FLOAT, 1, 0);
    
    free(sizearray);


	xsizein = data.image[IDinOPD].md[0].size[0];
	ysizein = data.image[IDinOPD].md[0].size[1];
	IDwfa = create_2Dimage_ID("aosim_wfa", ARRAYSIZE, ARRAYSIZE);
	IDwfp = create_2Dimage_ID("aosim_wfp", ARRAYSIZE, ARRAYSIZE);	
	iioffset = (ARRAYSIZE - xsizein)/2;
	jjoffset = (ARRAYSIZE - ysizein)/2;

	IDimcamlowfstmp = create_2Dimage_ID("aosim_imcamlowfstmp", ARRAYSIZE, ARRAYSIZE);	
	LOWFScamstarttime = data.image[IDphystime].array.F[0];

    k = 0;
    while(k<kmax)
    {		
  
        
        // COMPUTE FOCAL PLANE COMPLEX AMPLITUDE INCIDENT ON FOCAL PLANE MASK
		for(ii=0;ii<xsizein;ii++)
			for(jj=0;jj<ysizein;jj++)
				{
					ii1 = ii + iioffset;
					jj1 = jj + jjoffset;
					data.image[IDwfa].array.F[jj1*ARRAYSIZE+ii1] = data.image[IDinAMP].array.F[jj*xsizein+ii];
					data.image[IDwfp].array.F[jj1*ARRAYSIZE+ii1] = 1.0*M_PI*data.image[IDinOPD].array.F[jj*xsizein+ii]*1e-6/LAMBDA;
				}

		// loc_  for local files (not shared)
        mk_complex_from_amph("aosim_wfa", "aosim_wfp", "loc_aosim_wfc", 0);
        permut("loc_aosim_wfc");
        do2dfft("loc_aosim_wfc", "loc_aosim_fc0");
        permut("loc_aosim_fc0");
        mk_amph_from_complex("loc_aosim_fc0", "aosim_foc0_amp", "aosim_foc0_pha", 1);
        delete_image_ID("loc_aosim_wfc");
        delete_image_ID("loc_aosim_fc0");
		

        // APPLY FOCAL PLANE MASK
        IDfoc0a = image_ID("aosim_foc0_amp");
        IDfoc0p = image_ID("aosim_foc0_pha");
        data.image[IDfoc1a].md[0].write = 1;
        data.image[IDfoc1p].md[0].write = 1;
        for(ii=0;ii<ARRAYSIZE2;ii++)
			{
				data.image[IDfoc1a].array.F[ii] = data.image[IDfoc0a].array.F[ii] * data.image[IDfpmamp].array.F[ii];
				data.image[IDfoc1p].array.F[ii] = data.image[IDfoc0p].array.F[ii] + data.image[IDfpmpha].array.F[ii];
			}
		COREMOD_MEMORY_image_set_sempost_byID(IDfoc1a, -1);
		COREMOD_MEMORY_image_set_sempost_byID(IDfoc1p, -1);
		data.image[IDfoc1a].md[0].cnt0++;
		data.image[IDfoc1p].md[0].cnt0++;
		data.image[IDfoc1a].md[0].write = 0;
        data.image[IDfoc1p].md[0].write = 0;
        
        // COMPUTE pre-LYOT CA  
		mk_complex_from_amph("aosim_foc1_amp", "aosim_foc1_pha", "loc_aosim_foc1_c", 0);
		permut("loc_aosim_foc1_c");
		do2dfft("loc_aosim_foc1_c", "loc_aosim_pup1_c");
		delete_image_ID("loc_aosim_foc1_c");
		permut("loc_aosim_pup1_c");
		mk_amph_from_complex("loc_aosim_pup1_c", "aosim_pup1_amp", "aosim_pup1_pha", 1);
		delete_image_ID("loc_aosim_pup1_c");
	
		IDpup1a = image_ID("aosim_pup1_amp");
		IDpup1p = image_ID("aosim_pup1_pha");
		
		
		// COMPUTE pup1t
        data.image[IDpup1ta].md[0].write = 1;
        data.image[IDpup1tp].md[0].write = 1;
        for(ii=0;ii<ARRAYSIZE2;ii++)
			{
				data.image[IDpup1ta].array.F[ii] = data.image[IDpup1a].array.F[ii] * data.image[ID_LS_tamp].array.F[ii];
				data.image[IDpup1tp].array.F[ii] = data.image[IDpup1p].array.F[ii] + data.image[ID_LS_tpha].array.F[ii];
			}
		COREMOD_MEMORY_image_set_sempost_byID(IDpup1ta, -1);
		COREMOD_MEMORY_image_set_sempost_byID(IDpup1tp, -1);
		data.image[IDpup1ta].md[0].cnt0++;
		data.image[IDpup1tp].md[0].cnt0++;
		data.image[IDpup1ta].md[0].write = 0;
        data.image[IDpup1tp].md[0].write = 0;
 
 
		// COMPUTE foc2
		mk_complex_from_amph("aosim_pup1t_amp", "aosim_pup1t_pha", "loc_aosim_pup1t_c", 0);
        permut("loc_aosim_pup1t_c");
        do2dfft("loc_aosim_pup1t_c", "loc_aosim_foc2_c");
        permut("loc_aosim_foc2_c");
        mk_amph_from_complex("loc_aosim_foc2_c", "aosim_foc2_amp", "aosim_foc2_pha", 1);
        delete_image_ID("loc_aosim_pup1t_c");
        delete_image_ID("loc_aosim_foc2_c");
 
 
 
		// COMPUTE pup1r
        data.image[IDpup1ra].md[0].write = 1;
        data.image[IDpup1rp].md[0].write = 1;
        for(ii=0;ii<ARRAYSIZE2;ii++)
			{
				data.image[IDpup1ra].array.F[ii] = data.image[IDpup1a].array.F[ii] * data.image[ID_LS_ramp].array.F[ii];
				data.image[IDpup1rp].array.F[ii] = data.image[IDpup1p].array.F[ii] + data.image[ID_LS_rpha].array.F[ii] + 2.0*M_PI*data.image[ID_lowfsopd].array.F[ii]*1.0e-6/LAMBDA;
			}
		COREMOD_MEMORY_image_set_sempost_byID(IDpup1ra, -1);
		COREMOD_MEMORY_image_set_sempost_byID(IDpup1rp, -1);
		data.image[IDpup1ra].md[0].cnt0++;
		data.image[IDpup1rp].md[0].cnt0++;
		data.image[IDpup1ra].md[0].write = 0;
        data.image[IDpup1rp].md[0].write = 0;
 
		
		// COMPUTE foclowfs
		mk_complex_from_amph("aosim_pup1r_amp", "aosim_pup1r_pha", "loc_aosim_pup1r_c", 0);
        permut("loc_aosim_pup1r_c");
        do2dfft("loc_aosim_pup1r_c", "loc_aosim_foc1r_c");
        permut("loc_aosim_foc1r_c");
        mk_amph_from_complex("loc_aosim_foc1r_c", "aosim_foclowfs_amp", "aosim_foclowfs_pha", 1);
        delete_image_ID("loc_aosim_pup1r_c");
        delete_image_ID("loc_aosim_foc1r_c");
	
		// COMPUTE imlowfs
		IDfoclowfsa = image_ID("aosim_foclowfs_amp");
		data.image[IDimlowfs].md[0].write = 1;
		for(ii=0;ii<OUTLOWFSARRAYSIZE;ii++)
			for(jj=0;jj<OUTLOWFSARRAYSIZE;jj++)
				{
					ii1 = ii+(ARRAYSIZE-OUTLOWFSARRAYSIZE)/2;
					jj1 = jj+(ARRAYSIZE-OUTLOWFSARRAYSIZE)/2;
					data.image[IDimlowfs].array.F[OUTLOWFSARRAYSIZE*jj+ii] = data.image[IDfoclowfsa].array.F[jj1*ARRAYSIZE+ii1]*data.image[IDfoclowfsa].array.F[jj1*ARRAYSIZE+ii1];
					data.image[IDimcamlowfstmp].array.F[OUTLOWFSARRAYSIZE*jj+ii] += data.image[IDimlowfs].array.F[OUTLOWFSARRAYSIZE*jj+ii];
				}
		COREMOD_MEMORY_image_set_sempost_byID(IDimlowfs, -1);
		data.image[IDimlowfs].md[0].cnt0++;
		data.image[IDimlowfs].md[0].write = 0;
		LOWFScamcnt++;
	
		printf("%8ld  TIME: %16f  %16f   %16f/%16f\n", k, data.image[IDphystime].array.F[0], LOWFScamstarttime, data.image[IDphystime].array.F[0]-LOWFScamstarttime, LOWFSCAMETIME);
		fflush(stdout);
		
		if(data.image[IDphystime].array.F[0]-LOWFScamstarttime > LOWFSCAMETIME)
			{
				data.image[IDoutLOWFS].md[0].write = 1;
				data.image[IDimcamlowfstmp].md[0].write = 1;
				for(ii=0;ii<ARRAYSIZE2;ii++)
					data.image[IDoutLOWFS].array.F[ii] = data.image[IDimcamlowfstmp].array.F[ii]/LOWFScamcnt;
				for(ii=0;ii<ARRAYSIZE2;ii++)
					data.image[IDimcamlowfstmp].array.F[ii] = 0.0;
				COREMOD_MEMORY_image_set_sempost_byID(IDoutLOWFS, -1);
				COREMOD_MEMORY_image_set_sempost_byID(IDimcamlowfstmp, -1);
				data.image[IDoutLOWFS].md[0].cnt0++;
				data.image[IDimcamlowfstmp].md[0].cnt0++;;
				data.image[IDoutLOWFS].md[0].write = 0;
				data.image[IDimcamlowfstmp].md[0].write = 0;			
				LOWFScamcnt = 0;
				LOWFScamstarttime = data.image[IDphystime].array.F[0];
			}
		
		
		

        if(OUTMODE==1)
        {
            sprintf(command, "rm %s", OUTLOWFSFITSFILENAME);
            ret = system(command);
            sprintf(fname, "!%s", OUTLOWFSFITSFILENAME);
            save_fits(OUTLOWFSSTREAMNAME, fname);
            sprintf(command, "touch %s", OUTTRIGGERFILE);
            ret = system(command);
        }

        switch( INMODE ) {
        case 0:
            COREMOD_MEMORY_image_set_semwait(INTRIGSTREAMNAME, INTRIGSEMCHAN);
            break;
        case 1:
            OKf = 0;
            while(OKf==0)
            {
                usleep(10000);
                if(file_exists(INTRIGGERFILE)==1)
                {
                    sprintf(command, "rm %s", INTRIGGERFILE);
                    ret = system(command);
                    OKf = 1;
                }
            }
            break;
        default:
            printf("INMODE value %d not valid\n", INMODE);
            exit(0);
            break;
        }


		k++;
    }


   

    return(0);
}































/** \brief simplified AO system simulator (DM command -> WFS image part)
 *
 * creates a DM map(s) and a WF error input
 * When either DM map or WF error input changes, compute intensity outputs (images)
 *
 * syncmode:
 * 0: sync to turbulence
 * 1: sync to DM
 * 2: sync to both
 * default: use delayus
 *
 */

int AOsystSim_run(int syncmode, long DMindex, long delayus)
{
    long arraysize = 128;
    long ii, jj, ii1, jj1;
    double puprad, dmrad;
    double dmifscale = 2.0; // scale magnification between full DM map and DM influence function
    long *imsize;
    long DMsize = 50; // default
    long DMnbact;
    long mx, my;
    double x, y, rx, ry, ii1ld, rld;
    long rxi, ryi;
    double u, t, v00, v01, v10, v11, ii0f, jj0f, rxif, ryif;
    long IDif, IDifc;
    double sig;
    long k;
    long IDdmctrl;
    long IDpupm;
    int elem;
    long IDdm0shape;
    long IDfocmask;
    double r;
    double dftzoomfact = 2.0;
    long IDturb;
    long *dmsizearray;
    long ID;
    long IDout;
    long *IDarray;
    long iter;

    long *dhsizearray;
    long IDdh, IDdhmask;
    long dhxsize, dhysize, dhsize;
    long dhxoffset, dhyoffset;
    long IDre, IDim;
    char imdhname[200];
    char name[200];
    int COROmode = 0; // 1 if coronagraph

    float pupradcoeff = 0.17;
    float dmradcoeff = 0.20;
    float iwald = 2.0;

    char imnameamp[200];
    char imnamepha[200];
    int ret;
    long index;

    puprad = pupradcoeff*arraysize;
    dmrad = dmradcoeff*arraysize;


    // INITIALIZE DM CONTROL ARRAY IF DOESN'T EXIST

    sprintf(name, "dm%02lddisp", DMindex);
    IDdmctrl = image_ID(name);
    if(IDdmctrl==-1)
        IDdmctrl = read_sharedmem_image(name);
    if(IDdmctrl==-1)
    {
        dmsizearray = (long*) malloc(sizeof(long)*2);
        dmsizearray[0] = DMsize;
        dmsizearray[1] = DMsize;
        IDdmctrl = create_image_ID(name, 2, dmsizearray, FLOAT, 1, 0);
        free(dmsizearray);
        COREMOD_MEMORY_image_set_createsem(name, 2);
    }
    else
    {
        DMsize = data.image[IDdmctrl].md[0].size[0];
        COREMOD_MEMORY_image_set_createsem(name, 2);
    }



    for(k=0; k<DMsize*DMsize; k++)
        data.image[IDdmctrl].array.F[k] = ran1()*2.0e-8;

    dmifscale = 0.5*DMsize;


    // MAKE DM INFLUENCE FUNCTIONS

    // DM influence functions stored as a data cube
    DMnbact = DMsize*DMsize;
    imsize = (long*) malloc(sizeof(long)*3);
    imsize[0] = arraysize;
    imsize[1] = arraysize;
    imsize[2] = DMnbact;
    IDif = create_image_ID("dmif0", 2, imsize, FLOAT, 0, 0);
    // construct DM influence function (for 1 actuator)
    // step 1: square
    // actuator size = dmifscale*(arraysize*2.0*dmrad/DMsize) [pix]
    printf("DM size = %ld x %ld actuators\n", DMsize, DMsize);
    printf("actuator pix size = %f pix\n", dmifscale*(2.0*dmrad/DMsize));

    list_image_ID();
    for(ii=0; ii<arraysize; ii++)
        for(jj=0; jj<arraysize; jj++)
        {
            x = (1.0*ii-0.5*arraysize); // [pix]
            y = (1.0*jj-0.5*arraysize); // [pix]
            x /= dmifscale*(2.0*dmrad/DMsize);
            y /= dmifscale*(2.0*dmrad/DMsize);
            if((fabs(x)<0.5)&&(fabs(y)<0.5))
                data.image[IDif].array.F[jj*arraysize+ii] = 1.0e-6;
            else
                data.image[IDif].array.F[jj*arraysize+ii] = 0.0;
        }
    printf("convolve\n");
    fflush(stdout);
    // convolve dmif
    save_fits("dmif0", "!dmif0.fits");
    sig = 0.5*dmifscale*(2.0*dmrad/DMsize);
    printf("gauss filter   %lf %ld\n", sig, (long) (2.0*sig));
    fflush(stdout);
    gauss_filter("dmif0", "dmif", sig, (long) (2.0*sig));
    list_image_ID();
    delete_image_ID("dmif0");
    IDif = image_ID("dmif");


    list_image_ID();
    save_fits("dmif", "!dmif.fits");

    IDifc = create_image_ID("dmifc", 3, imsize, FLOAT, 0, 0);
    printf("\n");


    list_image_ID();
    printf("dmifc = %ld   %ld %ld %ld    %ld %ld\n", IDifc, data.image[IDifc].md[0].size[0], data.image[IDifc].md[0].size[1], data.image[IDifc].md[0].size[2], arraysize, arraysize);



    for(mx=0; mx<DMsize; mx++)
        for(my=0; my<DMsize; my++)
        {
            printf("\r actuator %2ld %2ld    ", mx, my);
            fflush(stdout);
            // actuator center coordinates
            // center: mx = DMsize/2-0.5  4 -> 1.5
            ii0f = 0.5*arraysize + 2.0*(mx-DMsize/2)/DMsize*dmrad;
            jj0f = 0.5*arraysize + 2.0*(my-DMsize/2)/DMsize*dmrad;
            for(ii=0; ii<arraysize; ii++)
                for(jj=0; jj<arraysize; jj++)
                {
                    rx = 1.0*ii-ii0f;
                    ry = 1.0*jj-jj0f;
                    rx *= dmifscale;
                    ry *= dmifscale;
                    rxif = rx+arraysize/2;
                    ryif = ry+arraysize/2;
                    rxi = (long) (rxif);
                    ryi = (long) (ryif);
                    u = rxif - rxi;
                    t = ryif - ryi;

                    if((rxi>0)&&(rxi<arraysize-1)&&(ryi>0)&&(ryi<arraysize-1))
                    {
                        v00 = data.image[IDif].array.F[ryi*arraysize+rxi];
                        v01 = data.image[IDif].array.F[ryi*arraysize+rxi+1];
                        v10 = data.image[IDif].array.F[(ryi+1)*arraysize+rxi];
                        v11 = data.image[IDif].array.F[(ryi+1)*arraysize+rxi+1];
                        data.image[IDifc].array.F[(my*DMsize+mx)*arraysize*arraysize + jj*arraysize + ii] = (1.0-u)*(1.0-t)*v00 + (1.0-u)*t*v10 + u*(1.0-t)*v01 + u*t*v11;
                    }
                }
        }
    free(imsize);
    save_fits("dmifc","!dmifc.fits");
    printf("\n");




    // INITIALIZE TURBULENCE SCREEN

    imsize = (long*) malloc(sizeof(long)*2);
    imsize[0] = arraysize;
    imsize[1] = arraysize;
    IDturb = create_image_ID("WFturb", 2, imsize, FLOAT, 1, 0);
    free(imsize);
    COREMOD_MEMORY_image_set_createsem("WFturb", 2);
    list_image_ID();

    sprintf(name, "dm%02lddisp", DMindex);
    AOsystSim_DMshape(name, "dmifc", "dm2Ddisp");
    IDdm0shape = image_ID("dm2Ddisp");
    save_fits("dm2Ddisp", "!dm2Ddisp.fits");





    // INITIALIZE OPTICAL SYSTEM

    optsystsim = (OPTSYST*) malloc(sizeof(OPTSYST)*1);
    optsystsim[0].nblambda = 1;
    optsystsim[0].lambdaarray[0] = 1.6e-6;
    optsystsim[0].beamrad = 0.008; // 8mm
    optsystsim[0].size = arraysize;
    optsystsim[0].pixscale = optsystsim[0].beamrad/50.0;
    optsystsim[0].DFTgridpad = 0;


    optsystsim[0].NB_asphsurfm = 2;
    optsystsim[0].NB_asphsurfr = 0;
    optsystsim[0].NBelem = 100; // to be updated later


    // 0: INPUT PUPIL
    IDpupm = make_disk("pupmask", arraysize, arraysize, 0.5*arraysize, 0.5*arraysize, puprad);
    elem = 0;
    optsystsim[0].elemtype[elem] = 1; // pupil mask
    optsystsim[0].elemarrayindex[elem] = IDpupm;
    optsystsim[0].elemZpos[elem] = 0.0;
    elem++;



    // 1: Turbulence screen
    optsystsim[0].elemtype[elem] = 3; // reflective surface
    optsystsim[0].elemarrayindex[elem] = 0; // index
    optsystsim[0].ASPHSURFMarray[0].surfID = IDturb;
    optsystsim[0].elemZpos[elem] = 0.0;
    elem++;

    // 2: DM 0
    optsystsim[0].elemtype[elem] = 3; // reflective surface
    optsystsim[0].elemarrayindex[elem] = 1; // index
    optsystsim[0].ASPHSURFMarray[1].surfID = IDdm0shape;
    optsystsim[0].elemZpos[elem] = 0.0;
    optsystsim[0].keepMem[elem] = 1;
    elem++;



    if(COROmode==1)    // FOCAL PLANE MASK
    {
        IDfocmask = create_2DCimage_ID("focpm", arraysize, arraysize);
        for(ii=0; ii<arraysize; ii++)
            for(jj=0; jj<arraysize; jj++)
            {
                x = 1.0*ii-0.5*arraysize;
                y = 1.0*jj-0.5*arraysize;
                r = sqrt(x*x+y*y);
                if(r<20.0*dftzoomfact)
                {
                    data.image[IDfocmask].array.CF[jj*arraysize+ii].re = 1.0;  // 1-(CA) : 1.0=opaque 0=transmissive 2.0=phase shifting
                    data.image[IDfocmask].array.CF[jj*arraysize+ii].im = 0.0;
                }
                else
                {
                    data.image[IDfocmask].array.CF[jj*arraysize+ii].re = 0.0;
                    data.image[IDfocmask].array.CF[jj*arraysize+ii].im = 0.0;
                }
            }

        optsystsim[0].elemtype[elem] = 5; // focal plane mask
        optsystsim[0].FOCMASKarray[0].fpmID = IDfocmask;
        optsystsim[0].FOCMASKarray[0].zfactor = dftzoomfact;
        optsystsim[0].FOCMASKarray[0].mode = 1;
        optsystsim[0].elemZpos[elem] = optsystsim[0].elemZpos[elem-1]; // plane from which FT is done
        elem++;
    }

    optsystsim[0].NBelem = elem;

    optsystsim[0].SAVE = 1;

    // propagate
    OptSystProp_run(optsystsim, 0, 0, optsystsim[0].NBelem, "./testconf/", 1);

    ID = image_ID("psfi0");
    imsize = (long*) malloc(sizeof(long)*2);
    imsize[0] = data.image[ID].md[0].size[0];
    imsize[1] = data.image[ID].md[0].size[1];
    imsize[2] = data.image[ID].md[0].size[2];
    IDout = create_image_ID("aosimpsfout", 3, imsize, FLOAT, 1, 0);
    free(imsize);

    COREMOD_MEMORY_image_set_createsem("aosimpsfout", 2);
    data.image[IDout].md[0].write = 1;
    memcpy(data.image[IDout].array.F, data.image[ID].array.F, sizeof(FLOAT)*data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1]*data.image[ID].md[0].size[2]);
    data.image[IDout].md[0].cnt0++;
    data.image[IDout].md[0].write = 0;
    COREMOD_MEMORY_image_set_sempost("aosimpsfout", -1);

    IDarray = (long*) malloc(sizeof(long)*2);
    IDarray[0] = image_ID("WFturb");
    sprintf(name, "dm%02lddisp", DMindex);
    IDarray[1] = image_ID(name);

    sprintf(imdhname, "dhfield");
    dhsize = (long) (0.5/pupradcoeff * DMsize*(pupradcoeff/dmradcoeff)*0.5);
    dhxsize = dhsize;
    dhysize = dhsize*2;
    dhxoffset = arraysize/2;
    dhyoffset = arraysize/2 - dhsize;
    dhsizearray = (long*) malloc(sizeof(long)*2);
    dhsizearray[0] = dhxsize*2;
    dhsizearray[1] = dhysize;

    IDdhmask = create_2Dimage_ID("dhmask", dhxsize, dhysize);
    for(ii=0; ii<dhxsize; ii++)
        for(jj=0; jj<dhysize; jj++)
        {
            data.image[IDdhmask].array.F[jj*dhxsize+ii] = 1.0;
            ii1 = (ii + dhxoffset) - arraysize/2;
            jj1 = (jj + dhyoffset) - arraysize/2;
            ii1ld = 2.0*ii1*pupradcoeff;
            r = sqrt(ii1*ii1+jj1*jj1);
            rld = r*pupradcoeff*2.0;
            if((ii1ld<0.5)||(rld<iwald))
                data.image[IDdhmask].array.F[jj*dhxsize+ii] = 0.0;
        }

    save_fits("dhmask", "!dhmask.fits");

    iter = 0;
    while(1)
    {
        //        printf("ITERATION %6ld   \n", iter);
        //      fflush(stdout);
        sprintf(name, "dm%02lddisp", DMindex);
        printf("Compute DM shape ...\n");
        fflush(stdout);
        AOsystSim_DMshape(name, "dmifc", "dm2Ddisp");
        printf("done\n");
        fflush(stdout);

        printf("Computing propagation ...\n");
        fflush(stdout);
        OptSystProp_run(optsystsim, 0, 0, optsystsim[0].NBelem, "./testconf/", 1);
        printf("done\n");
        fflush(stdout);

        // PYWFS code
        index = 2;
        ret = sprintf(imnameamp, "WFamp0_%03ld", index);
        ret = sprintf(imnamepha, "WFpha0_%03ld", index);
        mk_complex_from_amph(imnameamp, imnamepha, "_tmpwfc", 0);
        AOsystSim_WFSsim_Pyramid("_tmpwfc", "aosimwfsim", 0.0, 1);
        delete_image_ID("_tmpwfc");

        COREMOD_MEMORY_image_set_sempost("aosimwfsim", 0);


        ID = image_ID("psfi0");
        data.image[IDout].md[0].write = 1;
        memcpy(data.image[IDout].array.F, data.image[ID].array.F, sizeof(FLOAT)*data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1]*data.image[ID].md[0].size[2]);
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
        COREMOD_MEMORY_image_set_sempost("aosimpsfout", -1);



        // CREATE DARK HOLE FIELD
        IDre = image_ID("psfre0");
        IDim = image_ID("psfim0");
        IDdh = create_image_ID(imdhname, 2, dhsizearray, FLOAT, 1, 0);
        data.image[IDdh].md[0].write = 1;
        for(ii=0; ii<dhxsize; ii++)
            for(jj=0; jj<dhysize; jj++)
            {
                data.image[IDdh].array.F[jj*(2*dhxsize)+ii] = data.image[IDre].array.F[(jj+dhyoffset)*arraysize + (ii+dhxoffset)] * data.image[IDdhmask].array.F[jj*dhxsize+ii];
                data.image[IDdh].array.F[jj*(2*dhxsize)+(ii+dhxsize)] = data.image[IDim].array.F[(jj+dhyoffset)*arraysize + (ii+dhxoffset)] * data.image[IDdhmask].array.F[jj*dhxsize+ii];
            }
        data.image[IDdh].md[0].cnt0++;
        data.image[IDdh].md[0].write = 0;


        switch (syncmode) {
        case 0 : // sync to turbulence
            waitforsemID((void*) IDarray[0]);
            break;
        case 1 : // sync to DM
            waitforsemID((void*) IDarray[1]);
            break;
        case 2 :
            COREMOD_MEMORY_image_set_semwait_OR_IDarray(IDarray, 2);
            break;
        default :
            printf("WAITING %ld us\n", delayus);
            usleep(delayus);
            break;
        }

        COREMOD_MEMORY_image_set_semflush_IDarray(IDarray, 2);
        iter++;
    }

    free(dhsizearray);

    return(0);
}












int_fast8_t AOsystSim_extremeAO_contrast_sim()
{
    EXAOSIMCONF *exaosimconf;
    long CN2layer;
    double tmpv1, tmpv2, tmpv3, tmpv4;
    double tmpC;

    double lambda_V = 0.545e-6;
    double zeropt_V = 9.9690e10;

    double lambda_R = 0.638e-6;
    double zeropt_R = 7.2384e10;

    double lambda_I = 0.797e-6;
    double zeropt_I = 4.5825e10;

    double lambda_J = 1.22e-6;
    double zeropt_J = 1.9422e10;

    double lambda_H = 1.63e-6;
    double zeropt_H = 9.4440e9;

    double lambda_K = 2.19e-6;
    double zeropt_K = 4.3829e9;

    double lambda_L = 3.45e-6;
    double zeropt_L = 1.2292e9;

    double zeroptWFS;
    double zeroptWFSsci;

    double sourcemag_wfs;
    double sourcemag_sci;
    double lambdaBwfs = 0.4; // dlambda/lambda
    double lambdaBsci = 0.4; // dlambda/lambda
    double systemEfficiency = 0.3;

    double coeff;
    FILE *fp;
    double tmpA, tmpB;
    double dsa;

    double nwfs,nsci; // refractive indices

    double tobs = 3600.0; // observation time
    double crosstime;

    double WFStlim = 0.0000; // WFS min exposure time
    double sciWFStlim = 0.0000; // sci WFS min exposure time
    double IWAld = 1.3;
    int OK;
    double att2;


    exaosimconf = (EXAOSIMCONF*) malloc(sizeof(EXAOSIMCONF));

    // initialization
    exaosimconf[0].lambda0 = 0.55e-6;
    exaosimconf[0].lambdai = 1.6e-6;
    exaosimconf[0].lambdawfs = 0.8e-6;
    exaosimconf[0].D = 30.0;
    exaosimconf[0].r0 = 0.15;
    exaosimconf[0].windspeed = 10.0;
    exaosimconf[0].betapWFS = sqrt(2.0);
    exaosimconf[0].betaaWFS = sqrt(2.0);
    exaosimconf[0].betapWFSsci = 2.0;
    exaosimconf[0].betaaWFSsci = 2.0;
    exaosimconf[0].framedelay = 1.5;
    nwfs = OPTICSMATERIALS_n( OPTICSMATERIALS_code("Air"), exaosimconf[0].lambdawfs);
    nsci = OPTICSMATERIALS_n( OPTICSMATERIALS_code("Air"), exaosimconf[0].lambdai);

    printf("n = %f %f\n", nwfs, nsci);

    for(CN2layer=0; CN2layer<20; CN2layer++)
    {
        exaosimconf[0].CN2layer_h[CN2layer] = 0.0;
        exaosimconf[0].CN2layer_coeff[CN2layer] = 0.0;
    }
    exaosimconf[0].CN2layer_h[0] = 500.0;
    exaosimconf[0].CN2layer_coeff[0] = 0.2283;
    exaosimconf[0].CN2layer_h[1] = 1000.0;
    exaosimconf[0].CN2layer_coeff[1] = 0.0883;
    exaosimconf[0].CN2layer_h[2] = 2000.0;
    exaosimconf[0].CN2layer_coeff[2] = 0.0666;
    exaosimconf[0].CN2layer_h[3] = 4000.0;
    exaosimconf[0].CN2layer_coeff[3] = 0.1458;
    exaosimconf[0].CN2layer_h[4] = 8000.0;
    exaosimconf[0].CN2layer_coeff[4] = 0.3350;
    exaosimconf[0].CN2layer_h[5] = 16000.0;
    exaosimconf[0].CN2layer_coeff[5] = 0.1350;


    zeroptWFS = zeropt_I;
    exaosimconf[0].lambdawfs = lambda_I;

    zeroptWFSsci = zeropt_H;
    exaosimconf[0].lambdai = lambda_H;

    sourcemag_wfs = 8.0;
    sourcemag_sci = 6.0;
    exaosimconf[0].Fwfs = zeroptWFS*1.0e6*(exaosimconf[0].lambdawfs*lambdaBwfs)*pow(100.0, -0.2*sourcemag_wfs)*lambdaBwfs*systemEfficiency;
    exaosimconf[0].Fsci = zeroptWFSsci*1.0e6*(exaosimconf[0].lambdai*lambdaBsci)*pow(100.0, -0.2*sourcemag_sci)*lambdaBsci*systemEfficiency;

    exaosimconf[0].alpha_arcsec=0.25;

    fp = fopen("result.out.txt", "w");

    for(exaosimconf[0].alpha_arcsec=0.010; exaosimconf[0].alpha_arcsec<0.25; exaosimconf[0].alpha_arcsec+=0.001)
    {
        OK = 0;
        while(OK==0)
        {
            exaosimconf[0].alpha = exaosimconf[0].alpha_arcsec/3600/180*M_PI;
            exaosimconf[0].alpha_ld = exaosimconf[0].alpha / (exaosimconf[0].lambdai / exaosimconf[0].D);
            exaosimconf[0].f = exaosimconf[0].alpha/exaosimconf[0].lambdai;
            if(exaosimconf[0].alpha_ld>IWAld)
                OK = 1;
            else
                exaosimconf[0].alpha_arcsec+=0.001;
        }

        if(0) // SHWFS
        {
            dsa=0.15;
            exaosimconf[0].betapWFS = 1.48/(exaosimconf[0].f*dsa)*sqrt(1.0+(dsa*dsa/exaosimconf[0].r0/exaosimconf[0].r0));
        }

        exaosimconf[0].f_wfs = exaosimconf[0].alpha/exaosimconf[0].lambdawfs;
        exaosimconf[0].f_0 = exaosimconf[0].alpha/exaosimconf[0].lambda0;

        exaosimconf[0].hf = 0.22 * exaosimconf[0].lambda0 / (pow(exaosimconf[0].f, 11.0/6.0) * exaosimconf[0].D * pow(exaosimconf[0].r0, 5.0/6.0));

        exaosimconf[0].X = 0.0;
        exaosimconf[0].dX = 0.0;
        exaosimconf[0].dY = 0.0;
        for(CN2layer=0; CN2layer<20; CN2layer++)
        {
            tmpv1 = cos( M_PI * exaosimconf[0].CN2layer_h[CN2layer] * exaosimconf[0].f * exaosimconf[0].f * exaosimconf[0].lambdai);
            tmpv2 = cos( M_PI * exaosimconf[0].CN2layer_h[CN2layer] * exaosimconf[0].f * exaosimconf[0].f * exaosimconf[0].lambdawfs);
            tmpv3 = sin( M_PI * exaosimconf[0].CN2layer_h[CN2layer] * exaosimconf[0].f * exaosimconf[0].f * exaosimconf[0].lambdai);
            tmpv4 = sin( M_PI * exaosimconf[0].CN2layer_h[CN2layer] * exaosimconf[0].f * exaosimconf[0].f * exaosimconf[0].lambdawfs);
            exaosimconf[0].X += exaosimconf[0].CN2layer_coeff[CN2layer] * tmpv1 * tmpv1;
            exaosimconf[0].dX += exaosimconf[0].CN2layer_coeff[CN2layer] * (tmpv1-tmpv2) * (tmpv1-tmpv2);
            exaosimconf[0].dY += exaosimconf[0].CN2layer_coeff[CN2layer] * (tmpv3-tmpv4) * (tmpv3-tmpv4);
        }
        exaosimconf[0].Y = sqrt(1.0 - exaosimconf[0].X*exaosimconf[0].X);

        exaosimconf[0].C0 = pow(M_PI*exaosimconf[0].hf/exaosimconf[0].lambdai, 2.0) * exaosimconf[0].X;
        exaosimconf[0].C1 = pow(M_PI*exaosimconf[0].hf/exaosimconf[0].lambdai, 2.0) * exaosimconf[0].Y;

        tmpA = 2.0*M_PI*exaosimconf[0].hf*exaosimconf[0].windspeed*exaosimconf[0].f*exaosimconf[0].framedelay;
        tmpB = exaosimconf[0].lambdawfs/M_PI * exaosimconf[0].betapWFS / sqrt(exaosimconf[0].Fwfs * M_PI) / exaosimconf[0].D;
        exaosimconf[0].twfs_opt = pow(0.5*tmpB/tmpA*tmpB/tmpA, 1.0/3.0);
        exaosimconf[0].twfs = exaosimconf[0].twfs_opt;

        if(exaosimconf[0].twfs<WFStlim)
            exaosimconf[0].twfs = WFStlim;

        exaosimconf[0].hfca = tmpA * exaosimconf[0].twfs;   // lag
        exaosimconf[0].hfcb = tmpB / sqrt(exaosimconf[0].twfs);  // noise

        exaosimconf[0].hfc = sqrt(exaosimconf[0].hfca*exaosimconf[0].hfca + exaosimconf[0].hfcb*exaosimconf[0].hfcb);

        exaosimconf[0].C2 = pow(M_PI*exaosimconf[0].hfc/exaosimconf[0].lambdai, 2.0);
        exaosimconf[0].C2_wfs = exaosimconf[0].C2 * pow( exaosimconf[0].lambdai/exaosimconf[0].lambdawfs,2.0);

        exaosimconf[0].twfs_opt_amp = exaosimconf[0].twfs_opt*pow(exaosimconf[0].X/exaosimconf[0].Y, 1.0/3.0)*pow(exaosimconf[0].betaaWFS/exaosimconf[0].betapWFS,2.0/3.0);
        exaosimconf[0].C3 = exaosimconf[0].C2*pow(exaosimconf[0].Y/exaosimconf[0].X, 1.0/3.0)*pow(exaosimconf[0].betaaWFS/exaosimconf[0].betapWFS,4.0/3.0);

        exaosimconf[0].C4 = exaosimconf[0].C0*exaosimconf[0].dX;
        exaosimconf[0].C5 = exaosimconf[0].C1*exaosimconf[0].dY;

        exaosimconf[0].C6 = exaosimconf[0].C0 * pow((nwfs-nsci)/(1.0-0.5*(nwfs+nsci)), 2.0);



        printf("alpha_ld = %.2lf l/D\n", exaosimconf[0].alpha_ld);
        printf("f = %f  -> p = %g m\n", exaosimconf[0].f, 1.0/exaosimconf[0].f);
        printf("X = %g\n", exaosimconf[0].X);
        printf("Y = %g\n", exaosimconf[0].Y);
        printf("dX = %g\n", exaosimconf[0].dX);
        printf("dY = %g\n", exaosimconf[0].dY);
        printf("OPTIMAL WFS exposure time = %g sec  -> %.3lf kHz\n", exaosimconf[0].twfs_opt, 0.001/exaosimconf[0].twfs_opt);
        printf("single frequ WF error: %g -> %g  (%g + %g)\n", exaosimconf[0].hf, exaosimconf[0].hfc, exaosimconf[0].hfca, exaosimconf[0].hfcb);
        printf("WFS total flux per frame = %lf ph\n", exaosimconf[0].twfs*exaosimconf[0].Fwfs*exaosimconf[0].D*exaosimconf[0].D/4.0);
        crosstime = exaosimconf[0].D/exaosimconf[0].windspeed;
        printf("C0 contrast = %20g\n", exaosimconf[0].C0);
        printf("C1 contrast = %20g\n", exaosimconf[0].C1);
        printf("C2 contrast = %20g \n", exaosimconf[0].C2);
        printf("   WFS  lag = %20g    %20g\n", pow(M_PI*exaosimconf[0].hfca/exaosimconf[0].lambdai, 2.0), pow(M_PI*exaosimconf[0].hfca/exaosimconf[0].lambdai, 2.0)/sqrt(tobs/crosstime)*2.0);
        printf("   WFS noise= %20g    %20g\n", pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0), pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0)/sqrt(tobs/exaosimconf[0].twfs)*2.0);
        printf("C3 contrast = %20g    %20g\n", exaosimconf[0].C3, exaosimconf[0].C3/sqrt(tobs/crosstime)*2.0);
        printf("C4 contrast = %20g    %20g\n", exaosimconf[0].C4, exaosimconf[0].C4/sqrt(tobs/crosstime)*2.0);
        printf("C5 contrast = %20g    %20g\n", exaosimconf[0].C5, exaosimconf[0].C5/sqrt(tobs/crosstime)*2.0);
        printf("C6 contrast = %20g    %20g\n", exaosimconf[0].C6, exaosimconf[0].C6/sqrt(tobs/crosstime)*2.0);
        exaosimconf[0].Csum = exaosimconf[0].C2+exaosimconf[0].C3+exaosimconf[0].C4+exaosimconf[0].C5+exaosimconf[0].C6;
        exaosimconf[0].Csum_detection = pow(M_PI*exaosimconf[0].hfca/exaosimconf[0].lambdai, 2.0)/sqrt(tobs/crosstime)*2.0 + pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0)/sqrt(tobs/exaosimconf[0].twfs)*2.0 + exaosimconf[0].C3/sqrt(tobs/crosstime)*2.0 + exaosimconf[0].C4/sqrt(tobs/crosstime)*2.0 + exaosimconf[0].C5/sqrt(tobs/crosstime)*2.0+exaosimconf[0].C6/sqrt(tobs/crosstime)*2.0;
        printf("TOTAL CONTRAST = %20g   %20g\n", exaosimconf[0].Csum, exaosimconf[0].Csum_detection);
        printf("WFS speckle  contrast = %g    ph per WFS speckle per frame = %g\n", exaosimconf[0].C2_wfs, exaosimconf[0].C2_wfs*exaosimconf[0].twfs_opt*exaosimconf[0].Fwfs*exaosimconf[0].D*exaosimconf[0].D/4.0);
        printf("Time lag speckle lifetime = %.4f sec (intensity), %.4f sec (complex amplitude)\n", exaosimconf[0].D/exaosimconf[0].windspeed, (1.0/exaosimconf[0].f)/exaosimconf[0].windspeed/2.0/M_PI);


        // NEAR-IR LOOP


        // time lag attenuation
        tmpA = 2.0*M_PI*exaosimconf[0].hfca*exaosimconf[0].windspeed*exaosimconf[0].f*exaosimconf[0].framedelay;
        tmpB = exaosimconf[0].lambdai/M_PI * exaosimconf[0].betapWFSsci / sqrt(exaosimconf[0].Fsci * M_PI) / exaosimconf[0].D;
        exaosimconf[0].twfssci_opt = pow(0.5*tmpB/tmpA*tmpB/tmpA, 1.0/3.0);
        exaosimconf[0].twfssci = exaosimconf[0].twfssci_opt;
        if(exaosimconf[0].twfssci<sciWFStlim)
            exaosimconf[0].twfssci = sciWFStlim;
        exaosimconf[0].TL_hfca = tmpA * exaosimconf[0].twfssci;   // lag
        exaosimconf[0].TL_hfcb = tmpB / sqrt(exaosimconf[0].twfssci);  // noise
        exaosimconf[0].TL_hfc = sqrt(exaosimconf[0].TL_hfca*exaosimconf[0].TL_hfca + exaosimconf[0].TL_hfcb*exaosimconf[0].TL_hfcb);
        exaosimconf[0].C7 = pow(M_PI*exaosimconf[0].TL_hfc/exaosimconf[0].lambdai, 2.0);

        exaosimconf[0].C8 = (exaosimconf[0].C7+pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0))*pow(exaosimconf[0].Y/exaosimconf[0].X, 1.0/3.0)*pow(exaosimconf[0].betaaWFS/exaosimconf[0].betapWFS,4.0/3.0);
        att2 = (exaosimconf[0].C7+pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0))/exaosimconf[0].C2;


        exaosimconf[0].C9 = att2*exaosimconf[0].C4;
        exaosimconf[0].C10 = att2*exaosimconf[0].C5;


        // refractive index chromaticity attenuation
        tmpA = 2.0*M_PI*exaosimconf[0].hf*(nwfs-nsci)/(1.0-0.5*(nwfs+nsci))*exaosimconf[0].windspeed*exaosimconf[0].f*exaosimconf[0].framedelay;
        tmpB = exaosimconf[0].lambdai/M_PI * exaosimconf[0].betapWFSsci / sqrt(exaosimconf[0].Fsci * M_PI) / exaosimconf[0].D;
        exaosimconf[0].twfssci_opt = pow(0.5*tmpB/tmpA*tmpB/tmpA, 1.0/3.0);
        exaosimconf[0].twfssci = exaosimconf[0].twfssci_opt;
        if(exaosimconf[0].twfssci<sciWFStlim)
            exaosimconf[0].twfssci = sciWFStlim;
        exaosimconf[0].RIC_hfca = tmpA * exaosimconf[0].twfssci;   // lag
        exaosimconf[0].RIC_hfcb = tmpB / sqrt(exaosimconf[0].twfssci);  // noise
        exaosimconf[0].RIC_hfc = sqrt(exaosimconf[0].RIC_hfca*exaosimconf[0].RIC_hfca + exaosimconf[0].RIC_hfcb*exaosimconf[0].RIC_hfcb);
        exaosimconf[0].C11 = pow(M_PI*exaosimconf[0].RIC_hfc/exaosimconf[0].lambdai, 2.0);


        exaosimconf[0].Csum2 = 0.0;
        exaosimconf[0].Csum2 += exaosimconf[0].C7;
        exaosimconf[0].Csum2 += pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0);
        exaosimconf[0].Csum2 += exaosimconf[0].C8;
        exaosimconf[0].Csum2 += exaosimconf[0].C9;

        exaosimconf[0].Csum2ave = 0.0;
        exaosimconf[0].Csum2ave += exaosimconf[0].C7/sqrt(tobs/exaosimconf[0].twfssci)*2.0;
        exaosimconf[0].Csum2ave += pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0)/sqrt(tobs/exaosimconf[0].twfs)*2.0;
        exaosimconf[0].Csum2ave += exaosimconf[0].C8/sqrt(tobs/exaosimconf[0].twfssci)*2.0;
        exaosimconf[0].Csum2ave += exaosimconf[0].C9/sqrt(tobs/exaosimconf[0].twfssci)*2.0;
        // #1 : arcsec
        // #2 : C0
        // #3 : C1
        // #4 : C2
        // #5 : C3
        // #6 : C4
        // #7 : C5
        // #8 : C6
        // #9 : Csum
        // #10 : Csum_detection [5 sig]
        // #11 : wfs etime
        // #12 : ph/frame

        // #13 : C7  time lag correction residual (C2 ->)
        // #14 : C8 (C3->)
        // #15 : C9 (C4->)
        // #16 : C10 (C5->)
        // #17 : C11 (C6->)
        // #18 : RAW CONTRAST
        // #19 : Detection limit [5 sig]
        // #20 : Photon noise limit 1hr [5 sig]
        // #21 : wfs etime
        // #22 : near-IR ph/speckle/frame

        // #23 : photon noise limit loop 1


        fprintf(fp, "%f %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g\n", exaosimconf[0].alpha_arcsec, log10(exaosimconf[0].C0), log10(exaosimconf[0].C1), log10(exaosimconf[0].C2), log10(exaosimconf[0].C3), log10(exaosimconf[0].C4), log10(exaosimconf[0].C5), log10(exaosimconf[0].C6), log10(exaosimconf[0].Csum), log10(exaosimconf[0].Csum_detection*5), exaosimconf[0].twfs, exaosimconf[0].twfs*exaosimconf[0].Fwfs*exaosimconf[0].D*exaosimconf[0].D/4.0, log10(exaosimconf[0].C7), log10(exaosimconf[0].C8), log10(exaosimconf[0].C9), log10(exaosimconf[0].C10), log10(exaosimconf[0].C11), log10(exaosimconf[0].Csum2), log10(exaosimconf[0].Csum2ave*5), log10(5.0*exaosimconf[0].Csum2/sqrt(tobs*exaosimconf[0].Fsci*exaosimconf[0].D*exaosimconf[0].D/4.0*exaosimconf[0].Csum2)), exaosimconf[0].twfssci, exaosimconf[0].twfssci*exaosimconf[0].Fsci*exaosimconf[0].D*exaosimconf[0].D/4.0*exaosimconf[0].Csum2, log10(5.0*exaosimconf[0].Csum/sqrt(tobs*exaosimconf[0].Fsci*exaosimconf[0].D*exaosimconf[0].D/4.0*exaosimconf[0].Csum)));
        printf("Nphoton Sci = %g\n",tobs*exaosimconf[0].Fsci*exaosimconf[0].D*exaosimconf[0].D/4.0*exaosimconf[0].Csum2);
        //pow(M_PI*exaosimconf[0].hfca/exaosimconf[0].lambdai, 2.0), pow(M_PI*exaosimconf[0].hfcb/exaosimconf[0].lambdai, 2.0));
    }
    fclose(fp);

    fp = fopen("printcmd", "w");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fclose(fp);


    free(exaosimconf);

    return(0);
}











double f_eval (const gsl_vector *v, void *params)
{
    double *p = (double *)params;
    long double value;
    long k;
    long pr;

    long double ptre_test;
    long double ptim_test;
    long double Iflux_test;
    long double are_test = 1.0;
    long double aim_test = 0.0;
    long double e_test = 1.0;
    long double ai;
    long double re, im, x, y, tmp1;


    ptre_test = gsl_vector_get(v, 0);
    ptim_test = gsl_vector_get(v, 1);
    Iflux_test = gsl_vector_get(v, 2);

    are_test = 1.0;
    aim_test = 0.0;
    e_test = 1.0;
    if(NBoptVar>3)
        are_test = gsl_vector_get(v, 3);
    if(NBoptVar>4)
    {
        aim_test = gsl_vector_get(v, 4);
        e_test = gsl_vector_get(v, 5);
    }

    value = 0.0;
    for(pr=0; pr<NBprobesG; pr++)
    {
        re = probe_re[pr] - ptre_test;
        im = probe_im[pr] - ptim_test;
        if(NBoptVar>3)
        {
            x = re*are_test + im*aim_test;
            y = -re*aim_test + im*are_test;
            y = y*e_test;
            probe_tflux[pr] = Iflux_test + Cflux*(x*x+y*y);
        }
        else
            probe_tflux[pr] = Iflux_test + Cflux*(re*re+im*im);

        tmp1 = (probe_tflux[pr]-probe_nmflux[pr])/probe_nmnoise[pr];
        value += tmp1*tmp1;
    }

    tmpvalue1 = value;

    // penalize large (pte)
    value += pow(0.01*ptre_test*ptre_test, 4.0);
    // penalize large (ptim)
    value += pow(0.01*ptim_test*ptim_test, 4.0);


    if(NBoptVar>4)
    {
        ai = are_test*are_test + aim_test*aim_test;
        value += 0.0001*pow((ai-1.0)*(ai-1.0), 4.0);

        tmpvalue2 = value;

        // penalize large (e-1)
        value += 0.01*pow((e_test-1.0)*(e_test-1.0)*1.0, 8.0);
    }
    // printf("%lf %lf %lf    %lf %lf %lf   -> %g\n", (double) ptre_test, (double) ptim_test, (double) Iflux_test, (double) are_test, (double) aim_test, (double) e_test, value);
    // fflush(stdout);
    // usleep(100000);
    //exit(0);

    // value = tmpvalue1;// no regularization

    return ((double) value);

}




long AOsystSim_FPWFS_imsimul(double probeamp, double sepx, double sepy, double contrast, double wferramp, double totFlux, double DMgainErr, double RON, double CnoiseFloor)
{
    long size = 256;
    double puprad = 50.0;
    long IDpupa;
    long IDwf0; // initial WF
    long IDwfA; // probe A
    long IDwfB; // probe B
    long IDwf;
    long IDpsfC; // PSF cube
    long pr;
    long IDa;
    long ii, jj, ii1, jj1;
    double coeffA, coeffB;
    double x, y, x1, y1;
    double sincx, sincy;
    double CPAx, CPAy;
    double CPAstep;
    long ID, ID1;
    long xsize, ysize, xmin, xmax, ymin, ymax;

    double tot1 = 0.0;
    double peak = 0.0;

    // define WFS probe area
    double CPAxmin, CPAxmax, CPAymin, CPAymax;
    double pixscaleld;
    long IDtmp;
    char imname[200];
    char fname[200];
    double probeAmultcoeff = 1.0;
    double probeBmultcoeff = 1.0;
    double probeBphaseoffset = 1.0*M_PI/2.0;

    double dmactgain;
    long IDnoise;
    double val;



    printf("Creating images .... Contrast = %g  \n", contrast);
    fflush(stdout);


    IDpupa = make_subpixdisk("pupa", size, size, 0.5*size, 0.5*size, puprad);


    IDwf0 = image_ID("wf0");
    if(IDwf0==-1)
    {
        IDwf0 = create_2Dimage_ID("wf0", size, size);
        for(ii=0; ii<size*size; ii++)
            data.image[IDwf0].array.F[ii] = wferramp*(1.0-2.0*ran1());
        save_fl_fits("wf0", "!wf0.fits");
    }


    IDwfA = create_2Dimage_ID("wfA", size, size);
    IDwfB = create_2Dimage_ID("wfB", size, size);
    IDwf = create_2Dimage_ID("wf", size, size);

    IDpsfC = create_3Dimage_ID("psfC", size, size, NBprobesG);

    // initialize wf0

    // initialize wfA and wfB
    CPAxmin = 4.0;
    CPAxmax = 20.0;
    CPAymin = 4.0;
    CPAymax = 20.0;
    CPAstep = 0.2;

    probeAmultcoeff = 1.0; // 1.1
    probeBmultcoeff = 1.0; // 0.9
    probeBphaseoffset = 1.0*M_PI/2.0; // 0.8


    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            x = (1.0*ii-0.5*size)/puprad;
            y = (1.0*jj-0.5*size)/puprad;
            for(CPAx=CPAxmin; CPAx<CPAxmax; CPAx += CPAstep)
                for(CPAy=CPAymin; CPAy<CPAymax; CPAy += CPAstep)
                {
                    data.image[IDwfA].array.F[jj*size+ii] += probeAmultcoeff*probeamp*CPAstep*CPAstep*cos(M_PI*(x*CPAx+y*CPAy));
                    data.image[IDwfB].array.F[jj*size+ii] += probeBmultcoeff*probeamp*CPAstep*CPAstep*cos(M_PI*(x*CPAx+y*CPAy)+probeBphaseoffset);
                }
        }

    // DM gain error
    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            dmactgain = 1.0 + DMgainErr*(1.0-2.0*ran1());
            data.image[IDwfA].array.F[jj*size+ii]*= dmactgain;
            data.image[IDwfB].array.F[jj*size+ii]*= dmactgain;
        }


    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            data.image[IDwfA].array.F[jj*size+ii] *= data.image[IDpupa].array.F[jj*size+ii];
            data.image[IDwfB].array.F[jj*size+ii] *= data.image[IDpupa].array.F[jj*size+ii];
        }
    save_fl_fits("wfA", "!wfA.fits");
    save_fl_fits("wfB", "!wfB.fits");
    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            x = (1.0*ii-0.5*size)/puprad;
            y = (1.0*jj-0.5*size)/puprad;
            data.image[IDpupa].array.F[jj*size+ii] *= exp(-8.0*(x*x+y*y));
        }
    save_fl_fits("pupa", "!pupa.fits");


    for(pr=0; pr<NBprobesG; pr++)
    {
        if(pr==0)
        {
            coeffA = 0.0;
            coeffB = 0.0;
        }
        else
        {
            coeffA = cos(2.0*M_PI/(NBprobesG-1)*(pr-1)); //nprobe_re[pr];
            coeffB = sin(2.0*M_PI/(NBprobesG-1)*(pr-1)); //nprobe_im[pr];
        }

        for(ii=0; ii<size*size; ii++)
            data.image[IDwf].array.F[ii] = data.image[IDwf0].array.F[ii] + coeffA*data.image[IDwfA].array.F[ii] + coeffB*data.image[IDwfB].array.F[ii];

        sprintf(fname, "!DMprobe%02ld.fits", pr);
        save_fl_fits("wf", fname);

        mk_complex_from_amph("pupa", "wf", "wfc", 0);
        permut("wfc");
        do2dfft("wfc", "imc");
        permut("imc");
        mk_amph_from_complex("imc", "ima", "imp", 0);
        delete_image_ID("imc");
        delete_image_ID("imp");
        IDa = image_ID("ima");
        for(ii=0; ii<size*size; ii++)
        {
            data.image[IDpsfC].array.F[pr*size*size+ii] = data.image[IDa].array.F[ii]*data.image[IDa].array.F[ii];
            tot1 += data.image[IDpsfC].array.F[pr*size*size+ii];
        }
        delete_image_ID("ima");
    }

    // ADD COMPANIONS
    printf("Adding companions\n");
    fflush(stdout);
    IDtmp = create_3Dimage_ID("tmp3dim", size, size, NBprobesG);
    for(ii=0; ii<size*size*NBprobesG; ii++)
        data.image[IDtmp].array.F[ii] = data.image[IDpsfC].array.F[ii];

    for(pr=0; pr<NBprobesG; pr++)
    {
        for(ii=0; ii<size; ii++)
            for(jj=0; jj<size; jj++)
            {
                // companion #1
                ii1 = ii + (long) sepx;
                jj1 = jj + (long) sepy;
                if((ii1>0)&&(ii1<size)&&(jj1>0)&&(jj1<size))
                    data.image[IDpsfC].array.F[pr*size*size+jj1*size+ii1] += contrast * data.image[IDtmp].array.F[pr*size*size+jj*size+ii];

                // companion #2, 3.333x fainter, -10pix in x, +10 pix in y
                ii1 = ii + (long) sepx - 10;
                jj1 = jj + (long) sepy + 10;
                if((ii1>0)&&(ii1<size)&&(jj1>0)&&(jj1<size))
                    data.image[IDpsfC].array.F[pr*size*size+jj1*size+ii1] += 0.3*contrast * data.image[IDtmp].array.F[pr*size*size+jj*size+ii];

                // companion #3, 10x fainter, +10pix in x, +10 pix in y
                ii1 = ii + (long) sepx + 10;
                jj1 = jj + (long) sepy + 10;
                if((ii1>0)&&(ii1<size)&&(jj1>0)&&(jj1<size))
                    data.image[IDpsfC].array.F[pr*size*size+jj1*size+ii1] += 0.1*contrast * data.image[IDtmp].array.F[pr*size*size+jj*size+ii];

                // companion #4, 33.333x fainter, +10pix in x, -10 pix in y
                ii1 = ii + (long) sepx + 10;
                jj1 = jj + (long) sepy - 10;
                if((ii1>0)&&(ii1<size)&&(jj1>0)&&(jj1<size))
                    data.image[IDpsfC].array.F[pr*size*size+jj1*size+ii1] += 0.03*contrast * data.image[IDtmp].array.F[pr*size*size+jj*size+ii];

                // companion #5, 100x fainter, -10pix in x, -10 pix in y
                ii1 = ii + (long) sepx - 10;
                jj1 = jj + (long) sepy - 10;
                if((ii1>0)&&(ii1<size)&&(jj1>0)&&(jj1<size))
                    data.image[IDpsfC].array.F[pr*size*size+jj1*size+ii1] += 0.01*contrast * data.image[IDtmp].array.F[pr*size*size+jj*size+ii];

            }
    }
    delete_image_ID("tmp3dim");


    for(ii=0; ii<size*size*NBprobesG; ii++)
        data.image[IDpsfC].array.F[ii] *= totFlux/tot1;
    peak = 0.0;
    for(ii=0; ii<size*size; ii++)
        if(data.image[IDpsfC].array.F[ii]>peak)
            peak = data.image[IDpsfC].array.F[ii];


    pixscaleld = size/(2.0*puprad);
    xmin = 0.5*size + pixscaleld*CPAxmin;
    xmax = 0.5*size + pixscaleld*CPAxmax;
    ymin = 0.5*size + pixscaleld*CPAymin;
    ymax = 0.5*size + pixscaleld*CPAymax;
    xsize = xmax-xmin;
    ysize = ymax-ymin;


    // CREATE PROBE AMPLITUDE IMAGE IN FOCAL PLANE





    printf("Cropping and adding photon noise\n");
    fflush(stdout);

    ID = create_3Dimage_ID("psfCcrop", xsize, ysize, NBprobesG);
    tot1 = 0.0;
    for(pr=0; pr<NBprobesG; pr++)
        for(ii1=0; ii1<xsize; ii1++)
            for(jj1=0; jj1<ysize; jj1++)
            {
                ii = ii1+xmin;
                jj = jj1+ymin;
                data.image[ID].array.F[pr*xsize*ysize+jj1*xsize+ii1] = data.image[IDpsfC].array.F[pr*size*size+jj*size+ii];

                tot1 += data.image[ID].array.F[pr*xsize*ysize+jj1*xsize+ii1];
            }

    // CREATE PROBE AMPLITUDE IMAGE IN FOCAL PLANE
    ID1 = create_2Dimage_ID("psfprobeampC", xsize, ysize);


    for(ii1=0; ii1<xsize; ii1++)
        for(jj1=0; jj1<ysize; jj1++)
        {
            tot1 = 0.0;
            for(pr=CENTERprobe; pr<NBprobesG; pr++)
                tot1 += data.image[ID].array.F[pr*xsize*ysize+jj1*xsize+ii1]/peak;
            tot1 /= (NBprobesG-CENTERprobe);
            data.image[ID1].array.F[jj1*xsize+ii1] = tot1 - data.image[ID].array.F[jj1*xsize+ii1]/peak;
        }
    save_fl_fits("psfprobeampC", "!psfprobeampC.fits");

    // noise image
    IDnoise = create_3Dimage_ID("psfCcropnCn", xsize, ysize, NBprobesG);

    put_poisson_noise("psfCcrop", "psfCcropn");
    // add readout noise
    ID = image_ID("psfCcropn");
    for(ii1=0; ii1<xsize; ii1++)
        for(jj1=0; jj1<ysize; jj1++)
            data.image[ID].array.F[jj1*xsize+ii1] += RON * gauss();


    save_fl_fits("psfCcrop", "!psfCcrop.fits");
    save_fl_fits("psfCcropn", "!psfCcropn.fits");

    ID = image_ID("psfCcropn");
    ID1 =  create_3Dimage_ID("psfCcropnC", xsize, ysize, NBprobesG);
    for(pr=0; pr<NBprobesG; pr++)
        for(ii1=0; ii1<xsize; ii1++)
            for(jj1=0; jj1<ysize; jj1++)
            {
                data.image[ID1].array.F[pr*xsize*ysize+jj1*xsize+ii1] = data.image[ID].array.F[pr*xsize*ysize+jj1*xsize+ii1]/peak;

                val = data.image[ID].array.F[pr*xsize*ysize+jj1*xsize+ii1];
                if(val<1.0)
                    val = 1.0;

                data.image[IDnoise].array.F[pr*xsize*ysize+jj1*xsize+ii1] = sqrt( val + RON*RON )/peak ;  // assuming photon noise + readout noise
                if(data.image[IDnoise].array.F[pr*xsize*ysize+jj1*xsize+ii1] < CnoiseFloor)
                    data.image[IDnoise].array.F[pr*xsize*ysize+jj1*xsize+ii1]  = CnoiseFloor;
            }
    save_fl_fits("psfCcropnC", "!psfCcropnC.fits");
    printf("Saving psfCcropnCn\n");
    save_fl_fits("psfCcropnCn", "!psfCcropnCnoise.fits");

    for(pr=0; pr<NBprobesG; pr++)
    {
        sprintf(imname, "psfC_%03ld", pr);
        ID = create_2Dimage_ID(imname, size, size);
        for(ii=0; ii<size; ii++)
            for(jj=0; jj<size; jj++)
            {
                data.image[ID].array.F[jj*size+ii] = data.image[IDpsfC].array.F[pr*size*size+jj*size+ii]/peak;
            }

        sprintf(fname, "!psfC_%03ld.fits", pr);
        save_fl_fits(imname, fname);
    }

    printf("PSFs creation is complete\n");
    fflush(stdout);

    return(ID1);
}






// modegeom:
//
// last digit
// 0  : horizontal
// 1  : vertical
// 2  : quadrants #1
// 3  : quadrants #2
//
//
//
//
int AOsystSim_FPWFS_mkprobes(const char *IDprobeA_name, const char *IDprobeB_name, long dmxsize, long dmysize, double CPAmax, double CPArmin, double CPArmax, double RMSampl, long modegeom)
{
    long IDdmA, IDdmB;
    double CPAstep = 0.1;
    double x, y, r;
    double CPAx, CPAy, CPAr;
    double CPAxmin, CPAxmax;
    double CPAymin, CPAymax;
    long dmsize;
    long ii, jj;
    double rms;
    long ID;
    long imsize;
    double pha;

    IDdmA = create_2Dimage_ID(IDprobeA_name, dmxsize, dmysize);
    IDdmB = create_2Dimage_ID(IDprobeB_name, dmxsize, dmysize);

    switch (modegeom) {
    case 0 :    // mode 1: horizontal
        CPAxmin = 0.5;
        CPAxmax = CPAmax;
        CPAymin = -CPAmax;
        CPAymax = CPAmax;
        break;
    case 1 : // mode 2: vertical
        CPAxmin = -CPAmax;
        CPAxmax = CPAmax;
        CPAymin = 0.5;
        CPAymax = CPAmax;
        break;
    case 2 : // quadrants #1
        CPAxmin = 0.5;
        CPAxmax = CPAmax;
        CPAymin = 0.5;
        CPAymax = CPAmax;
        break;
    case 3 : // quadrants #1
        CPAxmin = 0.5;
        CPAxmax = CPAmax;
        CPAymin = -0.5;
        CPAymax = -CPAmax;
        break;
    default :
        printf("ERROR: mode not supported\n");
        exit(0);
        break;
    }

    dmsize = dmxsize;
    if(dmysize>dmxsize)
        dmsize = dmysize;

    for(CPAx=CPAxmin; CPAx<CPAmax; CPAx += CPAstep)
        for(CPAy=CPAymin; CPAy<CPAymax; CPAy += CPAstep)
        {
            CPAr = sqrt(CPAx*CPAx+CPAy*CPAy);
            if((CPAr>CPArmin)&&(CPAr<CPArmax))
            {
                pha = -1.6*CPAx;
                for(ii=0; ii<dmxsize; ii++)
                    for(jj=0; jj<dmysize; jj++)
                    {
                        x = 2.0*(1.0*ii-0.5*dmxsize)/dmsize;
                        y = 2.0*(1.0*jj-0.5*dmysize)/dmsize;
                        data.image[IDdmA].array.F[jj*dmxsize+ii] += cos(M_PI*(x*CPAx+y*CPAy)+pha);
                        data.image[IDdmB].array.F[jj*dmxsize+ii] += cos(M_PI*(x*CPAx+y*CPAy)+M_PI/2+pha);
                    }
            }
        }

    rms = 0.0;
    for(ii=0; ii<dmxsize*dmysize; ii++)
        rms += data.image[IDdmA].array.F[ii]*data.image[IDdmA].array.F[ii];
    rms = sqrt(rms/(dmxsize*dmysize));

    for(ii=0; ii<dmxsize*dmysize; ii++)
    {
        data.image[IDdmA].array.F[ii] *= RMSampl/rms;
        data.image[IDdmB].array.F[ii] *= RMSampl/rms;
    }


    // TEST
    imsize = 5*dmsize;
    ID = make_disk("pupa", imsize, imsize, 0.5*imsize, 0.5*imsize, 0.45*dmsize);
    for(ii=0; ii<imsize; ii++)
        for(jj=0; jj<imsize; jj++)
        {
            x = (1.0*ii-0.5*imsize)/(0.45*dmsize);
            y = (1.0*jj-0.5*imsize)/(0.45*dmsize);
            r = sqrt(x*x+y*y);
            data.image[ID].array.F[jj*imsize+ii] *= 1.0; //exp(-r*r*4.0);
            if(r<0.3)
                data.image[ID].array.F[jj*imsize+ii] = 0.0;
        }
    ID = create_2Dimage_ID("pupp", imsize, imsize);
    for(ii=0; ii<dmxsize; ii++)
        for(jj=0; jj<dmysize; jj++)
            data.image[ID].array.F[(jj+(imsize-dmysize)/2)*imsize+(ii+(imsize-dmxsize)/2)] = data.image[IDdmA].array.F[jj*dmxsize+ii];

    mk_complex_from_amph("pupa", "pupp", "pupc", 0);
    permut("pupc");
    do2dfft("pupc","focc");
    permut("focc");
    mk_amph_from_complex("focc","foca","focp", 0);
    save_fits("pupa", "!test_pupa.fits");
    save_fits("pupp", "!test_pupp_A.fits");
    save_fits("foca", "!test_foca_A.fits");
    delete_image_ID("pupc");
    delete_image_ID("focc");
    delete_image_ID("foca");
    delete_image_ID("focp");



    ID = image_ID("pupp");
    for(ii=0; ii<dmxsize; ii++)
        for(jj=0; jj<dmysize; jj++)
            data.image[ID].array.F[(jj+(imsize-dmysize)/2)*imsize+(ii+(imsize-dmxsize)/2)] = data.image[IDdmB].array.F[jj*dmxsize+ii];
    mk_complex_from_amph("pupa", "pupp", "pupc", 0);
    permut("pupc");
    do2dfft("pupc","focc");
    permut("focc");
    mk_amph_from_complex("focc","foca","focp", 0);
    save_fits("pupp", "!test_pupp_B.fits");
    save_fits("foca", "!test_foca_B.fits");
    delete_image_ID("pupc");
    delete_image_ID("focc");
    delete_image_ID("foca");
    delete_image_ID("focp");


    ID = image_ID("pupp");
    for(ii=0; ii<dmxsize; ii++)
        for(jj=0; jj<dmysize; jj++)
            data.image[ID].array.F[(jj+(imsize-dmysize)/2)*imsize+(ii+(imsize-dmxsize)/2)] = -data.image[IDdmA].array.F[jj*dmxsize+ii];
    mk_complex_from_amph("pupa", "pupp", "pupc", 0);
    permut("pupc");
    do2dfft("pupc","focc");
    permut("focc");
    mk_amph_from_complex("focc","foca","focp", 0);
    save_fits("pupp", "!test_pupp_mA.fits");
    save_fits("foca", "!test_foca_mA.fits");
    delete_image_ID("pupc");
    delete_image_ID("focc");
    delete_image_ID("foca");
    delete_image_ID("focp");


    ID = image_ID("pupp");
    for(ii=0; ii<dmxsize; ii++)
        for(jj=0; jj<dmysize; jj++)
            data.image[ID].array.F[(jj+(imsize-dmysize)/2)*imsize+(ii+(imsize-dmxsize)/2)] = -data.image[IDdmB].array.F[jj*dmxsize+ii];
    mk_complex_from_amph("pupa", "pupp", "pupc", 0);
    permut("pupc");
    do2dfft("pupc","focc");
    permut("focc");
    mk_amph_from_complex("focc","foca","focp", 0);
    save_fits("pupp", "!test_pupp_mB.fits");
    save_fits("foca", "!test_foca_mB.fits");
    delete_image_ID("pupc");
    delete_image_ID("focc");
    delete_image_ID("foca");
    delete_image_ID("focp");




    ID = image_ID("pupp");
    for(ii=0; ii<dmxsize; ii++)
        for(jj=0; jj<dmysize; jj++)
            data.image[ID].array.F[(jj+(imsize-dmysize)/2)*imsize+(ii+(imsize-dmxsize)/2)] = 0.0;
    mk_complex_from_amph("pupa", "pupp", "pupc", 0);
    permut("pupc");
    do2dfft("pupc","focc");
    permut("focc");
    mk_amph_from_complex("focc","foca","focp", 0);
    save_fits("pupp", "!test_pupp_00.fits");
    save_fits("foca", "!test_foca_00.fits");
    delete_image_ID("pupc");
    delete_image_ID("focc");
    delete_image_ID("foca");
    delete_image_ID("focp");




    return(0);
}


//
// explore optimal theoretical sensitivity for focal plane WFS
//
// optmode:
// 1: 3 free parameters: ptre, ptim, Iflux
// 2: 6 free parameters: ptre, ptim, Iflux, are, aim, e
//
int AOsystSim_FPWFS_sensitivityAnalysis(int mapmode, int mode, int optmode, int NBprobes)
{
    // conventions
    // CA : complex amplitude
    //
    // CA of point to be measured is uniformly distributed in unit circle

    char imname[200];
    double ptre, ptim; // real/imaginary components of point to be probed

    double probe_noise_prop;
    double ProbeNoise;

    double probe_mflux[100]; // measured flux (ideal, no noise)
    double probe_mflux_dre[100]; // derivative against ptre
    double probe_mflux_dim[100]; // derivative against ptim
    double probe_mflux_dI[100]; // derivative against Iflux

    double probeamp;
    int pr;
    double Iflux = 0.0; // incoherent flux
    double re, im;
    double eps = 1.0e-8;

    double re_re, re_im, im_re, im_im;
    long k, ks;
    double tmp1;
    int execmode;

    // solver
    double ptre_test, ptim_test, Iflux_test, are_test, aim_test, e_test;
    double ptre_best, ptim_best, Iflux_best, are_best, aim_best, e_best;
    double value, bvalue, value0;
    double x, y;

    double optampl;
    long ksmax = 100000000;

    double mapampl = 2.0;
    long mapsize = 41; // map size (linear)
    long mapxsize, mapysize;
    long kx, ky, kz;

    long IDmap;

    // output solution
    long IDmap_ptre;
    long IDmap_ptim;
    long IDmap_ptre_in;
    long IDmap_ptim_in;
    long IDmap_Iflux;
    long IDmap_are;
    long IDmap_aim;
    long IDmap_e;

    long mapz;
    long mapzsize;


    long st;
    long optvar;
    const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
    gsl_multimin_fminimizer *s = NULL;
    gsl_vector *ss, *xvect;
    gsl_multimin_function feval_func;
    int status;
    long iter;
    long iterMax = 100000;
    double optsize;
    double bestvalue;
    double tmp;

    double FLUXph = 1.0e4; // total number of photon per cycle

    int imSimulMode = 0; // 1 if real image simulated
    long IDpsfC;

    long avecnt;
    double ave, rms;
    long ii, jj;
    long kxtest = 21;
    long kytest = 11;

    long IDprobampC = -1;

    long vID;
    double probeampl;
    double WFerr;
    double contrast;
    double totFlux;

    int initmap = 0;

    long IDmap_Iflux_ave, IDmap_Iflux_rms;
    int NewWF = 0;
    long IDpsfCnoise;

    double RON = 1.0; // detector readout noise (phe-)
    long IDmap_Iflux_rmsn;
    long IDmap_Iflux_rmsn1;
    long IDmap_CA_rms, IDmap_CA_rmsn;
    double dx, dy, val;
    double CnoiseFloor;


    NBprobesG = NBprobes;



    if(mapmode==2) // complex amplitude field
        imSimulMode = 1;

    mapzsize = 10;
    if((vID=variable_ID("mapzsize"))!=-1)
    {
        mapzsize = (long) (0.1+data.variable[vID].value.f);
        printf("mapzsize = %ld\n", mapzsize);
    }

    if((vID=variable_ID("NewWF"))!=-1)
    {
        NewWF = (int) (0.1+data.variable[vID].value.f);
        printf("NewWF = %d\n", NewWF);
    }

    ProbeNoise = 0.2;
    if((vID=variable_ID("ProbeNoise"))!=-1)
    {
        ProbeNoise = data.variable[vID].value.f;
        printf("ProbeNoise = %g\n", ProbeNoise);
    }

    RON = 1.0;
    if((vID=variable_ID("RON"))!=-1)
    {
        RON = data.variable[vID].value.f;
        printf("RON = %f\n", RON);
    }

    CnoiseFloor = 1.0e-5;
    if((vID=variable_ID("CnoiseFloor"))!=-1)
    {
        CnoiseFloor = data.variable[vID].value.f;
        printf("CnoiseFloor = %g\n", CnoiseFloor);
    }

    CENTERprobe = 1;
    if((vID=variable_ID("CENTERprobe"))!=-1)
    {
        CENTERprobe = (int) (0.1+data.variable[vID].value.f);
        printf("CENTERprobe = %d\n", CENTERprobe);
    }




    if(imSimulMode == 1)
    {
        probeampl = 0.0001;
        if((vID=variable_ID("probeampl"))!=-1)
        {
            probeampl = data.variable[vID].value.f;
            printf("probeamp = %f rad\n", probeampl);
        }

        contrast = 5.0e-8;
        if((vID=variable_ID("contrast"))!=-1)
        {
            contrast = data.variable[vID].value.f;
            printf("contrast = %e\n", contrast);
        }

        WFerr = 0.000;
        if((vID=variable_ID("WFerr"))!=-1)
        {
            WFerr = data.variable[vID].value.f;
            printf("WFerr = %f rad\n", WFerr);
        }

        mapmode = 2; // use existing file
    }


    totFlux= 1e12;
    if((vID=variable_ID("totFlux"))!=-1)
    {
        totFlux = data.variable[vID].value.f;
        FLUXph = data.variable[vID].value.f;
        printf("totFlux = %f ph\n", totFlux);
    }



    if(mapmode==2)
        CENTERprobe = 1;


    probeamp = 1.0;
    if(CENTERprobe==1) // include center probe
    {
        probe_re[pr] = 0.0;
        probe_im[pr] = 0.0;
        for(pr=1; pr<NBprobes; pr++)
        {
            probe_re[pr] = probeamp*cos(2.0*M_PI/(NBprobes-1)*(pr-1));
            probe_im[pr] = probeamp*sin(2.0*M_PI/(NBprobes-1)*(pr-1));
        }
    }
    else
    {
        for(pr=0; pr<NBprobes; pr++)
        {
            probe_re[pr] = probeamp*cos(2.0*M_PI/(NBprobes)*pr);
            probe_im[pr] = probeamp*sin(2.0*M_PI/(NBprobes)*pr);
        }
    }


    execmode = 1;





    if(mapmode==0)
    {
        mapsize = 1;
    }

    mapxsize = mapsize;
    mapysize = mapsize;







    switch (mode) {
    case 1 :    // mode 1: perfectly calibrated system -> no probe noise
        probe_noise_prop = 0.0;
        for(pr=0; pr<NBprobes; pr++)
        {
            nprobe_re[pr] = probe_re[pr];
            nprobe_im[pr] = probe_im[pr];
        }
        break;
    case 2 : // mode 2: uncorrelated probes noise
        probe_noise_prop = ProbeNoise;
        for(pr=0; pr<NBprobes; pr++)
        {
            nprobe_re[pr] = probe_re[pr] + probe_noise_prop*probeamp*(2.0*ran1()-1.0);
            nprobe_im[pr] = probe_im[pr] + probe_noise_prop*probeamp*(2.0*ran1()-1.0);
        }
        break;
    case 3 : // noise on probe axes
        probe_noise_prop = ProbeNoise;
        re_re = 1.0 + (2.0*ran1()-1.0)*probe_noise_prop;
        re_im = 0.0 + (2.0*ran1()-1.0)*probe_noise_prop;
        im_re = 0.0 + (2.0*ran1()-1.0)*probe_noise_prop;
        im_im = 1.0 + (2.0*ran1()-1.0)*probe_noise_prop;
        for(pr=0; pr<NBprobes; pr++)
        {
            nprobe_re[pr] = probe_re[pr]*re_re + probe_im[pr]*im_re;
            nprobe_im[pr] = probe_re[pr]*re_im + probe_im[pr]*im_im;
        }
        break;
    default :
        printf("ERROR: mode not supported\n");
        execmode = 0;
        break;
    }


    printf("noise mode  = %d (%f)\n", mode, probe_noise_prop);
    printf("mapmode     = %d\n", mapmode);
    printf("mapsize     = %ld   %ld   %ld\n", mapxsize, mapysize, mapzsize);
    printf("execmode    = %d\n", execmode);
    printf("mapzsize    = %ld\n", mapzsize);
    printf("imSimulMode = %d\n", imSimulMode);


    if(IDprobampC==-1)
        IDprobampC = create_2Dimage_ID("psfprobeamp", mapxsize, mapysize);
    for(ii=0; ii<mapxsize*mapysize; ii++)
        data.image[IDprobampC].array.F[ii] = 1.0;

    printf("\n\n");



    for(mapz=0; mapz<mapzsize; mapz++)
    {

        if(imSimulMode == 1)
        {
            IDpsfC = AOsystSim_FPWFS_imsimul(probeampl, 30.0, 30.0, contrast, WFerr, totFlux, probe_noise_prop, RON, CnoiseFloor); // computes data cube
            IDpsfCnoise = image_ID("psfCcropnCn");
            IDprobampC = image_ID("psfprobeampC");
            save_fl_fits("psfC", "!psfC.fits");

            ave = 0.0;
            avecnt = 0;

            for(pr=1; pr<NBprobes; pr++)
            {
                for(ii=0; ii<data.image[IDpsfC].md[0].size[0]; ii++)
                    for(jj=0; jj<data.image[IDpsfC].md[0].size[1]; jj++)
                    {
                        ave += data.image[IDpsfC].array.F[pr*data.image[IDpsfC].md[0].size[0]*data.image[IDpsfC].md[0].size[1]+jj*data.image[IDpsfC].md[0].size[0]+ii];
                        avecnt++;
                    }
            }
            ave /= avecnt;
            printf("ave = %g\n", ave);

            mapmode = 2; // use existing file
        }


        if(mapmode == 2)
        {
            mapxsize = data.image[IDpsfC].md[0].size[0];
            mapysize = data.image[IDpsfC].md[0].size[1];
        }

        if(initmap==0)
        {
            if(mapmode>0)
            {
                IDmap = create_3Dimage_ID("WFSerrmap", mapxsize, mapysize, mapzsize);
                IDmap_ptre = create_3Dimage_ID("WFSsol_ptre", mapxsize, mapysize, mapzsize);
                IDmap_ptim = create_3Dimage_ID("WFSsol_ptim", mapxsize, mapysize, mapzsize);
                IDmap_ptre_in = create_3Dimage_ID("WFSsol_ptre_in", mapxsize, mapysize, mapzsize);
                IDmap_ptim_in = create_3Dimage_ID("WFSsol_ptim_in", mapxsize, mapysize, mapzsize);
                IDmap_Iflux = create_3Dimage_ID("WFSsol_Iflux", mapxsize, mapysize, mapzsize);
                IDmap_are = create_3Dimage_ID("WFSsol_are", mapxsize, mapysize, mapzsize);
                IDmap_aim = create_3Dimage_ID("WFSsol_aim", mapxsize, mapysize, mapzsize);
                IDmap_e = create_3Dimage_ID("WFSsol_e", mapxsize, mapysize, mapzsize);
            }
            else
                IDmap = -1;
            initmap = 1;
        }





        if(mapmode==0)
        {
            printf("Preparing optimization ... \n");
            fflush(stdout);
        }
        switch (optmode) {
        case 0 :
            NBoptVar = 3;
            break;
        case 1 :
            NBoptVar = 4;
            break;
        case 2 :
            NBoptVar = 6;
            break;
        }

        xvect = gsl_vector_alloc (NBoptVar);
        ss = gsl_vector_alloc (NBoptVar);
        feval_func.n = NBoptVar;  /* number of function components */
        feval_func.f = &f_eval;
        feval_func.params = (void *) NULL;
        s = gsl_multimin_fminimizer_alloc (T, NBoptVar);


        if(execmode>0)
        {
            if((mapmode==0)&&((kx==kxtest)&&(ky==kytest)))
                for(pr=0; pr<NBprobes; pr++)
                    printf("PROBE %2d : %10lf %10lf\n", pr, (double) probe_re[pr], (double) probe_im[pr]);


            for(kx=0; kx<mapxsize; kx++)
                for(ky=0; ky<mapysize; ky++)
                {
                    printf("\r slice %ld    pixel %5ld %5ld       ", mapz, kx, ky);
                    fflush(stdout);
                    if(mapmode==0)
                    {
                        ptre = 2.0*ran1()-1.0;
                        ptim = 2.0*ran1()-1.0;
                    }
                    else
                    {
                        ptre = mapampl*(2.0*kx/(mapxsize-1)-1.0);
                        ptim = mapampl*(2.0*ky/(mapysize-1)-1.0);
                    }
                    if(mapmode==2)
                    {
                        ptre = 0.0;
                        ptim = 0.0;
                    }

                    if((mapmode==0)||((kx==kxtest)&&(ky==kytest))||((kx==kxtest+1)&&(ky==kytest)))
                        printf("\n\n ptre, ptim = %g, %g\n", ptre, ptim);
                    // compute flux



                    for(pr=0; pr<NBprobes; pr++)
                    {
                        // ideal measurements (no noise)
                        re = probe_re[pr] - ptre;
                        im = probe_im[pr] - ptim;
                        probe_mflux[pr] = Iflux + Cflux*(re*re+im*im);


                        // noisy measurements
                        re = nprobe_re[pr] - ptre;
                        im = nprobe_im[pr] - ptim;
                        val = fast_poisson(((Iflux + Cflux*(re*re+im*im))*FLUXph)/NBprobes) + RON*gauss();
                        probe_nmflux[pr] = ( val ) / (FLUXph/NBprobes);
                        if(val<1.0)
                            val = 1.0;
                        probe_nmnoise[pr] = sqrt(val);
                        if((mapmode==0)&&(((kx==kxtest)&&(ky==kytest))||((kx==kxtest+1)&&(ky==kytest))))
                        {
                            printf("M0    NO NOISE:  probe %3d  normalized flux = %8.5lf  (%g ph)\n", pr, (double) (probe_mflux[pr]/Cflux), (double) probe_mflux[pr]);
                            printf("M0  WITH NOISE:             normalized flux = %8.5lf  (%g ph)\n",     (double) (probe_nmflux[pr]/Cflux), (double) probe_nmflux[pr]);
                            fflush(stdout);
                        }


                        if(imSimulMode == 1)
                        {
                            probe_nmflux[pr] = data.image[IDpsfC].array.F[pr*mapxsize*mapysize+ky*mapxsize+kx] / data.image[IDprobampC].array.F[ky*mapxsize+kx]; // unit : normalized contrast
                            probe_nmnoise[pr] = data.image[IDpsfCnoise].array.F[pr*mapxsize*mapysize+ky*mapxsize+kx] / data.image[IDprobampC].array.F[ky*mapxsize+kx];
                            if(((kx==kxtest)&&(ky==kytest))||((kx==kxtest+1)&&(ky==kytest)))
                                printf("PROBE %d   -> %g\n", pr, (double) probe_nmflux[pr]);
                        }

                    }

                    if((mapmode==0)&&((kx==kxtest)&&(ky==kytest))||((kx==kxtest+1)&&(ky==kytest)))
                        fflush(stdout);

                    // SOLVER


                    for(optvar=0; optvar<NBoptVar; optvar++)
                        gsl_vector_set (xvect, optvar, 0.0);

                    gsl_vector_set (xvect, 0, ptre);
                    gsl_vector_set (xvect, 1, ptim);

                    if(NBoptVar>3)
                        gsl_vector_set (xvect, 3, 1.0);


                    if(NBoptVar>4)
                    {
                        gsl_vector_set (xvect, 4, 0.0);
                        gsl_vector_set (xvect, 5, 1.0);
                    }

                    /* Set initial step sizes  */
                    gsl_vector_set_all (ss, 0.1);

                    /* Initialize method and iterate */
                    iter = 0;
                    bestvalue = 100000.0;


                    gsl_multimin_fminimizer_set (s, &feval_func, xvect, ss);


                    do
                    {
                        iter++;
                        status = gsl_multimin_fminimizer_iterate(s);
                        if (status)
                            break;

                        optsize = gsl_multimin_fminimizer_size (s);
                        // printf("Iteration %ld   %g\n", iter, optsize);
                        status = gsl_multimin_test_size (optsize, 1.0/pow(10.0, 10.0-4.0*iter/iterMax)); //1.0e-10);


                        //printf ("............[%05ld] ->  %e   (%e)\n", iter, s->fval, bestvalue);
                        if (status == GSL_SUCCESS)
                        {
                            if(((kx==kxtest)&&(ky==kytest))||((kx==kxtest+1)&&(ky==kytest)))
                            {
                                printf ("  ->  %e [%5ld]  (%e)", s->fval, iter, bestvalue);
                                printf("\n");
                            }
                            ptre_best = gsl_vector_get(s->x, 0);
                            ptim_best = gsl_vector_get(s->x, 1);
                            Iflux_best = gsl_vector_get(s->x, 2);
                            if(NBoptVar>3)
                                are_best = gsl_vector_get(s->x, 3);
                            if(NBoptVar>4)
                            {
                                aim_best = gsl_vector_get(s->x, 4);
                                e_best = gsl_vector_get(s->x, 5);

                                if(e_best>1.0)
                                {
                                    e_best = 1.0/e_best;
                                    tmp = are_best;
                                    are_best = aim_best;
                                    aim_best = -tmp;
                                }
                            }



                            if(s->fval < bestvalue)
                                bestvalue = s->fval;
                        }
                    }
                    while (status == GSL_CONTINUE && iter < iterMax );
                    if(iter>iterMax-1) {
                        printf("Max number of iteration reached (optsize = %g)\n", optsize);
                    }

                    ptre_best = gsl_vector_get(s->x, 0);
                    ptim_best = gsl_vector_get(s->x, 1);
                    Iflux_best = gsl_vector_get(s->x, 2);
                    if(NBoptVar>3)
                        are_best = gsl_vector_get(s->x, 3);
                    if(NBoptVar>4)
                    {
                        aim_best = gsl_vector_get(s->x, 4);
                        e_best = gsl_vector_get(s->x, 5);
                        if(e_best>1.0)
                        {
                            e_best = 1.0/e_best;
                            tmp = are_best;
                            are_best = aim_best;
                            aim_best = -tmp;
                        }
                    }

                    if((mapmode==0)||((kx==kxtest)&&(ky==kytest))||((kx==kxtest+1)&&(ky==kytest)))
                    {
                        printf("\n\n");
                        printf("[%3ld %3ld] OPTIMAL SOLUTION  [ %6ld / %6ld  %g ]: \n", kx, ky, iter, iterMax, optsize);
                        printf("       ptre = %.18f\n", ptre_best);
                        printf("       ptim = %.18f\n", ptim_best);
                        printf("      Iflux = %.18f\n", Iflux_best);
                        if(NBoptVar>3)
                            printf("        are = %.18f\n", are_best);
                        if(NBoptVar>4)
                        {
                            printf("        aim = %.18f\n", aim_best);
                            printf("          e = %.18f\n", e_best);
                        }
                        printf("OPT VALUES :  %g  %g  %g\n", tmpvalue1, tmpvalue2, bestvalue);
                        printf("\n\n");
                    }


                    if(mapmode>0)
                    {

                        data.image[IDmap].array.F[mapz*mapxsize*mapysize+ky*mapxsize+kx] = Iflux_best*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                        data.image[IDmap_ptre].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = ptre_best*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                        data.image[IDmap_ptim].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = ptim_best*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                        data.image[IDmap_ptre_in].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = ptre*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                        data.image[IDmap_ptim_in].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = ptim*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                        data.image[IDmap_Iflux].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = Iflux_best*data.image[IDprobampC].array.F[ky*mapxsize+kx];
                        if(NBoptVar>3)
                            data.image[IDmap_are].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = are_best*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                        if(NBoptVar>4)
                        {
                            data.image[IDmap_aim].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = aim_best*sqrt(data.image[IDprobampC].array.F[ky*mapxsize+kx]);
                            data.image[IDmap_e].array.F[mapz*mapysize*mapxsize+ky*mapxsize+kx] = e_best;
                        }
                    }
                }
        }
        if(mapmode>0)
        {
            save_fits("WFSerrmap", "!WFSerrmap.fits");
            save_fits("WFSsol_ptre", "!WFSsol_ptre.fits");
            save_fits("WFSsol_ptim", "!WFSsol_ptim.fits");
            save_fits("WFSsol_ptre_in", "!WFSsol_ptre_in.fits");
            save_fits("WFSsol_ptim_in", "!WFSsol_ptim_in.fits");
            save_fits("WFSsol_Iflux", "!WFSsol_Iflux.fits");
            if(NBoptVar>3)
                save_fits("WFSsol_are", "!WFSsol_are.fits");
            if(NBoptVar>4)
            {
                save_fits("WFSsol_aim", "!WFSsol_aim.fits");
                save_fits("WFSsol_e", "!WFSsol_e.fits");
            }
        }


        delete_image_ID("pupa");

        if(NewWF==1)
            delete_image_ID("wf0");

        delete_image_ID("wfA");
        delete_image_ID("wfB");
        delete_image_ID("wf");
        delete_image_ID("psfC");
        delete_image_ID("wfc");
        delete_image_ID("psfCcrop");
        delete_image_ID("psfprobeampC");
        delete_image_ID("psfCcropn");
        delete_image_ID("psfCcropnC");
        for(pr=0; pr<NBprobes; pr++)
        {
            sprintf(imname, "psfC_%03d", pr);
            delete_image_ID(imname);
        }

        list_image_ID();



        if((mapz>2)&&(mapmode>0))
        {
            IDmap_Iflux_ave = image_ID("WFSsol_Iflux_ave");
            if(IDmap_Iflux_ave==-1)
                IDmap_Iflux_ave = create_2Dimage_ID("WFSsol_Iflux_ave", mapxsize, mapysize);

            IDmap_Iflux_rms = image_ID("WFSsol_Iflux_rms");
            if(IDmap_Iflux_rms==-1)
                IDmap_Iflux_rms = create_2Dimage_ID("WFSsol_Iflux_rms", mapxsize, mapysize);




            for(kx=0; kx<mapxsize; kx++)
                for(ky=0; ky<mapysize; ky++)
                {
                    ave = 0.0;
                    rms = 0.0;
                    for(kz=0; kz<mapz; kz++)
                    {
                        ave += data.image[IDmap_Iflux].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx];
                        rms += data.image[IDmap_Iflux].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx]*data.image[IDmap_Iflux].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx];
                    }
                    ave /= mapz;
                    rms /= mapz;
                    rms -= ave*ave;
                    rms = sqrt(rms);
                    data.image[IDmap_Iflux_ave].array.F[ky*mapxsize+kx] = ave;
                    data.image[IDmap_Iflux_rms].array.F[ky*mapxsize+kx] = rms;
                }
            save_fits("WFSsol_Iflux_ave", "!WFSsol_Iflux_ave.fits");
            save_fits("WFSsol_Iflux_rms", "!WFSsol_Iflux_rms.fits");





            if(imSimulMode == 0)
            {
                // INCOHERENT COMPONENT ERROR

                IDmap_Iflux_rmsn = image_ID("WFSsol_Iflux_rmsn");
                if(IDmap_Iflux_rmsn==-1)
                    IDmap_Iflux_rmsn = create_2Dimage_ID("WFSsol_Iflux_rmsn", mapxsize, mapysize);

                IDmap_Iflux_rmsn1 = image_ID("WFSsol_Iflux_rmsn1");
                if(IDmap_Iflux_rmsn1==-1)
                    IDmap_Iflux_rmsn1 = create_2Dimage_ID("WFSsol_Iflux_rmsn1", mapxsize, mapysize);


                for(kx=0; kx<mapxsize; kx++)
                    for(ky=0; ky<mapysize; ky++)
                    {
                        ptre = mapampl*(2.0*kx/mapxsize-1.0);
                        ptim = mapampl*(2.0*ky/mapysize-1.0);

                        data.image[IDmap_Iflux_rmsn].array.F[ky*mapxsize+kx] = data.image[IDmap_Iflux_rms].array.F[ky*mapxsize+kx] / sqrt(1.0/FLUXph);
                        data.image[IDmap_Iflux_rmsn1].array.F[ky*mapxsize+kx] = data.image[IDmap_Iflux_rms].array.F[ky*mapxsize+kx] / sqrt(1.0/FLUXph) / sqrt(ptre*ptre+ptim*ptim);
                    }
                save_fits("WFSsol_Iflux_rmsn", "!WFSsol_Iflux_rmsn.fits");
                save_fits("WFSsol_Iflux_rmsn1", "!WFSsol_Iflux_rmsn1.fits");



                // COHERENT COMPONENT ERROR

                IDmap_CA_rms = image_ID("WFSsol_CA_rms");
                if(IDmap_CA_rms==-1)
                    IDmap_CA_rms = create_2Dimage_ID("WFSsol_CA_rms", mapxsize, mapysize);

                for(kx=0; kx<mapxsize; kx++)
                    for(ky=0; ky<mapysize; ky++)
                    {
                        ave = 0.0;
                        rms = 0.0;
                        for(kz=0; kz<mapz; kz++)
                        {
                            dx = data.image[IDmap_ptre].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx]-data.image[IDmap_ptre_in].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx];
                            dy = data.image[IDmap_ptim].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx]-data.image[IDmap_ptim_in].array.F[kz*mapysize*mapxsize+ky*mapxsize+kx];
                            val = dx*dx+dy*dy;
                            ave += val;
                            rms += val*val;
                        }
                        ave /= mapz;
                        rms /= mapz;
                        rms -= ave*ave;
                        rms = sqrt(rms);
                        data.image[IDmap_CA_rms].array.F[ky*mapxsize+kx] = sqrt(ave);
                    }
                save_fits("WFSsol_CA_rms", "!WFSsol_CA_rms.fits");

                IDmap_CA_rmsn = image_ID("WFSsol_CA_rmsn");
                if(IDmap_CA_rmsn==-1)
                    IDmap_CA_rmsn = create_2Dimage_ID("WFSsol_CA_rmsn", mapxsize, mapysize);

                for(kx=0; kx<mapxsize; kx++)
                    for(ky=0; ky<mapysize; ky++)
                        data.image[IDmap_CA_rmsn].array.F[ky*mapxsize+kx] = data.image[IDmap_CA_rms].array.F[ky*mapxsize+kx] * sqrt(FLUXph);

                save_fits("WFSsol_CA_rmsn", "!WFSsol_CA_rmsn.fits");
            }
        }

    }

    return(0);
}

















