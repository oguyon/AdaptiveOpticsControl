#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sched.h>
#include <assert.h>
#include <sys/stat.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_multifit.h>

#include <fitsio.h> 

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_blas.h>

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
    return 0;, optres[ii]
}
#else
#include <time.h>
#endif


#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "statistic/statistic.h"
#include "info/info.h"
#include "linopt_imtools/linopt_imtools.h"

#include "linARfilterPred/linARfilterPred.h"

#ifdef HAVE_CUDA
#include "cudacomp/cudacomp.h"
#endif

extern DATA data;




// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string (not image)
// 4: existing image
// 5: string




int_fast8_t LINARFILTERPRED_LoadASCIIfiles_cli()
{
	if(CLI_checkarg(1,1)+CLI_checkarg(2,1)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,5)==0)
		{
			LINARFILTERPRED_LoadASCIIfiles(data.cmdargtoken[1].val.numf, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.string);
		}
	else
		return(1);
}


int_fast8_t LINARFILTERPRED_SelectBlock_cli()
{

	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,2)+CLI_checkarg(4,3)==0)
		{
			LINARFILTERPRED_SelectBlock(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string);
		}
	else
		return(1);
}


int_fast8_t LINARFILTERPRED_Build_LinPredictor_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,3)+CLI_checkarg(7,2)+CLI_checkarg(8,1)==0)
		LINARFILTERPRED_Build_LinPredictor(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.string, 1, data.cmdargtoken[7].val.numl, data.cmdargtoken[8].val.numf);
	else
       return 1;

  return(0);
}


int_fast8_t LINARFILTERPRED_Apply_LinPredictor_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,1)+CLI_checkarg(4,3)==0)
		LINARFILTERPRED_Apply_LinPredictor(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.string);
	else
		return 1;

  return(0);
}

int_fast8_t LINARFILTERPRED_ScanGain_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,1)==0)
    LINARFILTERPRED_ScanGain(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numf);
	else
       return 1;

  return(0);
}


int_fast8_t LINARFILTERPRED_PF_updatePFmatrix_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,5)+CLI_checkarg(3,1)==0)
		LINARFILTERPRED_PF_updatePFmatrix(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf);
	else
       return 1;

  return(0);
}


int_fast8_t LINARFILTERPRED_PF_RealTimeApply_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,4)+CLI_checkarg(5,2)+CLI_checkarg(6,5)+CLI_checkarg(7,2)+CLI_checkarg(8,2)+CLI_checkarg(9,2)+CLI_checkarg(10,2)+CLI_checkarg(11,1)+CLI_checkarg(12,2)==0)
		LINARFILTERPRED_PF_RealTimeApply(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.string, data.cmdargtoken[7].val.numl, data.cmdargtoken[8].val.numl, data.cmdargtoken[9].val.numl, data.cmdargtoken[10].val.numl, data.cmdargtoken[11].val.numf, data.cmdargtoken[12].val.numl);
	else
       return 1;

  return(0);
}






int_fast8_t init_linARfilterPred()
{
    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "linear auto-regressive predictive filters");
    data.NBmodule++;


	
	

    strcpy(data.cmd[data.NBcmd].key,"pfloadascii");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_LoadASCIIfiles_cli;
    strcpy(data.cmd[data.NBcmd].info,"load ascii files to PF input");
    strcpy(data.cmd[data.NBcmd].syntax,"<tstart> <dt> <NBpt> <NBfr> <output>");
    strcpy(data.cmd[data.NBcmd].example,"pfloadascii 200.0 0.001 10000 4 pfin");
    strcpy(data.cmd[data.NBcmd].Ccall,"long LINARFILTERPRED_LoadASCIIfiles(double tstart, double dt, long NBpt, long NBfr, const char *IDoutname)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"mselblock");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_SelectBlock_cli;
    strcpy(data.cmd[data.NBcmd].info,"select modes belonging to a block");
    strcpy(data.cmd[data.NBcmd].syntax,"<input mode values> <block map> <selected block> <output>");
    strcpy(data.cmd[data.NBcmd].example,"mselblock modevals blockmap 23 blk23modevals");
    strcpy(data.cmd[data.NBcmd].Ccall,"long LINARFILTERPRED_SelectBlock(const char *IDin_name, const char *IDblknb_name, long blkNB, const char *IDout_name)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"mkARpfilt");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_Build_LinPredictor_cli;
    strcpy(data.cmd[data.NBcmd].info,"Make linear auto-regressive filter");
    strcpy(data.cmd[data.NBcmd].syntax,"<input data> <PForder> <PFlag> <SVDeps> <regularization param> <output filters> <LOOPmode> <LOOPgain>");
    strcpy(data.cmd[data.NBcmd].example,"mkARpfilt indata 5 2.4 0.0001 0.0 outPF 0 0.1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int LINARFILTERPRED_Build_LinPredictor(const char *IDin_name, long PForder, float PFlag, double SVDeps, double RegLambda, const char *IDoutPF, int outMode, int LOOPmode, float LOOPgain)");
    data.NBcmd++;

  /*  strcpy(data.cmd[data.NBcmd].key,"applyPfiltRT");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_Apply_LinPredictor_RT_cli;
    strcpy(data.cmd[data.NBcmd].info,"Apply real-time linear predictive filter");
    strcpy(data.cmd[data.NBcmd].syntax,"<input data> <predictor filter> <output>");
    strcpy(data.cmd[data.NBcmd].example,"applyPfiltRT indata Pfilt outPF");
    strcpy(data.cmd[data.NBcmd].Ccall,"long LINARFILTERPRED_Apply_LinPredictor_RT(const char *IDfilt_name, const char *IDin_name, const char *IDout_name)");
    data.NBcmd++;
*/

    strcpy(data.cmd[data.NBcmd].key,"applyARpfilt");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_Apply_LinPredictor_cli;
    strcpy(data.cmd[data.NBcmd].info,"Apply linear auto-regressive filter");
    strcpy(data.cmd[data.NBcmd].syntax,"<input data> <predictor> <PFlag> <prediction>");
    strcpy(data.cmd[data.NBcmd].example,"applyARpfilt indata Pfilt 2.4 outPF");
    strcpy(data.cmd[data.NBcmd].Ccall,"long LINARFILTERPRED_Apply_LinPredictor(const char *IDfilt_name, const char *IDin_name, float PFlag, const char *IDout_name)");
    data.NBcmd++;

	
	strcpy(data.cmd[data.NBcmd].key,"mscangain");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_ScanGain_cli;
    strcpy(data.cmd[data.NBcmd].info,"scan gain");
    strcpy(data.cmd[data.NBcmd].syntax,"<input mode values> <multiplicative factor (leak)> <latency [frame]>");
    strcpy(data.cmd[data.NBcmd].example,"mscangain olwfsmeas 0.98 2.65");
    strcpy(data.cmd[data.NBcmd].Ccall,"LINARFILTERPRED_ScanGain(char* IDin_name, float multfact, float framelag)");
    data.NBcmd++;


	strcpy(data.cmd[data.NBcmd].key,"linARPFMupdate");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_PF_updatePFmatrix_cli;
    strcpy(data.cmd[data.NBcmd].info,"update predictive filter matrix");
    strcpy(data.cmd[data.NBcmd].syntax,"<input 3D predictor> <output 2D matrix> <update coeff>");
    strcpy(data.cmd[data.NBcmd].example,"linARPFMupdate outPF PFMat 0.1");
    strcpy(data.cmd[data.NBcmd].Ccall,"long LINARFILTERPRED_PF_updatePFmatrix(const char *IDPF_name, const char *IDPFM_name, float alpha)");
    data.NBcmd++;

	strcpy(data.cmd[data.NBcmd].key,"linARapplyRT");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = LINARFILTERPRED_PF_RealTimeApply_cli;
    strcpy(data.cmd[data.NBcmd].info,"Real-time apply predictive filter");
    strcpy(data.cmd[data.NBcmd].syntax,"<input open loop coeffs stream> <offset index> <trigger semaphore index> <2D predictive matrix> <filter order> <output stream> <nbGPU> <loop> <NBiter> <savemode> <timelag> <PFindex>");
    strcpy(data.cmd[data.NBcmd].example,"linARapplyRT modevalOL 0 2 PFmat 5 outPFmodeval 0 0 0 0 1.8 0");
    strcpy(data.cmd[data.NBcmd].Ccall,"long LINARFILTERPRED_PF_RealTimeApply(const char *IDmodevalOL_name, long IndexOffset, int semtrig, const char *IDPFM_name, long NBPFstep, const char *IDPFout_name, int nbGPU, long loop, long NBiter, int SAVEMODE, float tlag, long PFindex)");
    data.NBcmd++;

    // add atexit functions here

    return 0;
}




int NBwords(const char sentence[ ])
{
    int counted = 0; // result

    // state:
    const char* it = sentence;
    int inword = 0;

    do switch(*it) {
        case '\0': 
        case ' ': case '\t': case '\n': case '\r': 
            if (inword) { inword = 0; counted++; }
            break;
        default: inword = 1;
    } while(*it++);

    return counted;
}




//
// load ascii file(s) into image cube
// resamples sequence(s) of data points
//
// INPUT FILES HAVE TO BE NAMED seq000.dat, seq001.dat etc...
//
// file starts at tstart, sampling = dt 
// NBpt per file
// NBfr files
//
long LINARFILTERPRED_LoadASCIIfiles(double tstart, double dt, long NBpt, long NBfr, const char *IDoutname)
{
	FILE *fp;
	long NBfiles;
	double runtime;
	char fname[200];
	struct stat fstat;
	int fOK;
	long NBvarin[200];
	long fcnt;
	FILE* fparray[200];
	long kk;
	size_t linesiz=0;
	char *linebuf=0;
	ssize_t linelen=0;
	int ret;
	long vcnt;
	double ftime0[200];
	double var0[200][200];
	double ftime1[200];
	double var1[200][200];
	double varC[200][200];
	float alpha;
	long nbvar;
	long fr;
	char imoutname[200];
	FILE *fpout;
	long IDout[200];
	int HPfilt = 1; // high pass filter
	float HPgain = 0.005;
	
	long ii;
	long kkpt, kkfr;
	

	runtime = tstart;
	
	fOK = 1;
	NBfiles = 0;
	nbvar = 0;
	while (fOK == 1)
	{
		sprintf(fname, "seq%03ld.dat", NBfiles);
		if( stat (fname, &fstat) == 0 )
		{
			printf("Found file %s\n", fname);
			fflush(stdout);
			fp = fopen(fname, "r");
			linelen = getline(&linebuf, &linesiz, fp);
			fclose(fp);  
			NBvarin[NBfiles] = NBwords(linebuf)-1;
			free(linebuf);
			linebuf = NULL;
			printf("   NB variables = %ld\n", NBvarin[NBfiles]);
			nbvar += NBvarin[NBfiles];
			NBfiles++;
		}
		else
		{
			printf("No more files\n");
			fflush(stdout);
			fOK = 0;
		}
	}
	printf("NBfiles = %ld\n", NBfiles);
	
	
	
	for(fcnt=0;fcnt<NBfiles;fcnt++)
		{
			sprintf(fname, "seq%03ld.dat", fcnt);
			printf("   %03ld  OPENING FILE %s\n", fcnt, fname);
			fflush(stdout);
			fparray[fcnt] = fopen(fname, "r");
		}
	
	
	
	kk = 0; // time
	runtime = tstart;
	

	for(fcnt=0;fcnt<NBfiles;fcnt++)
		{
			ret = fscanf(fparray[fcnt], "%lf", &ftime0[fcnt]);
			for(vcnt=0; vcnt<NBvarin[fcnt]; vcnt++)
				ret = fscanf(fparray[fcnt], "%lf", &var0[fcnt][vcnt]);
			ret = fscanf(fparray[fcnt], "\n");

			ret = fscanf(fparray[fcnt], "%lf", &ftime1[fcnt]);
			for(vcnt=0; vcnt<NBvarin[fcnt]; vcnt++)
				ret = fscanf(fparray[fcnt], "%lf", &var1[fcnt][vcnt]);
			ret = fscanf(fparray[fcnt], "\n");
		
	
	
		printf("FILE %ld :  \n", fcnt);
		printf(" time :    %20f  %20f\n", ftime0[fcnt], ftime1[fcnt]);
		fflush(stdout);
		
		for(vcnt=0; vcnt < NBvarin[fcnt]; vcnt++)
			{
				printf("    variable %3ld   :   %20f  %20f\n", vcnt, var0[fcnt][vcnt], var1[fcnt][vcnt]);
				varC[fcnt][vcnt] = var0[fcnt][vcnt];
			}
		printf("\n");
	}
	
	
	for(fr=0; fr<NBfr; fr++)
	{
		sprintf(imoutname, "%s_%03ld", IDoutname, fr);
		IDout[fr] = create_3Dimage_ID(imoutname, nbvar, 1, NBpt);
	}
	
	fpout = fopen("out.txt", "w");

	kk = 0;
	kkpt = 0;
	kkfr = 0;
	while(kkfr<NBfr)
		{
			fprintf(fpout, "%20f", runtime);
			
			ii = 0;
			for(fcnt=0;fcnt<NBfiles;fcnt++)
				{
					while(ftime1[fcnt]<runtime)
					{
						ftime0[fcnt] = ftime1[fcnt];
						for(vcnt=0; vcnt<NBvarin[fcnt]; vcnt++)
							var0[fcnt][vcnt] = var1[fcnt][vcnt];

						ret = fscanf(fparray[fcnt], "%lf", &ftime1[fcnt]);
						for(vcnt=0; vcnt<NBvarin[fcnt]; vcnt++)
							ret = fscanf(fparray[fcnt], "%lf", &var1[fcnt][vcnt]);
						ret = fscanf(fparray[fcnt], "\n");
					}
					if(kk==0)
						for(vcnt=0; vcnt < NBvarin[fcnt]; vcnt++)
							varC[fcnt][vcnt] = var0[fcnt][vcnt];
							
					alpha = (runtime - ftime0[fcnt]) / (ftime1[fcnt] - ftime0[fcnt]);
					for(vcnt=0; vcnt<NBvarin[fcnt]; vcnt++)
						{
							fprintf(fpout, " %20f", (1.0-alpha)*var0[fcnt][vcnt] + alpha*var1[fcnt][vcnt] - varC[fcnt][vcnt]);					
							varC[fcnt][vcnt] = (1.0-HPgain)*varC[fcnt][vcnt] + HPgain * ((1.0-alpha)*var0[fcnt][vcnt] + alpha*var1[fcnt][vcnt]);

							data.image[IDout[kkfr]].array.F[kkpt*nbvar + ii] = (1.0-alpha)*var0[fcnt][vcnt] + alpha*var1[fcnt][vcnt] - varC[fcnt][vcnt];
							ii++;
						}
				}
			
			fprintf(fpout, "\n");
			
			kk++;
			kkpt++;
			runtime += dt;
			if(kkpt == NBpt)
				{
					kkpt = 0;
					kkfr++;
				}
		}
	
	fclose(fpout);
	
	for(fcnt=0;fcnt<NBfiles;fcnt++)
		fclose(fparray[fcnt]);
	
	return(NBfiles);
}










// select block on first dimension 
long LINARFILTERPRED_SelectBlock(const char *IDin_name, const char *IDblknb_name, long blkNB, const char *IDout_name)
{
	long IDin, IDblknb;
	long naxis, axis;
	long m;
	long NBmodes1;
	long *sizearray;
	long xsize, ysize, zsize;
	long cnt;
	long ii, jj, kk;
	long IDout;
	char imname[200];
	long mmax;
	
	printf("Selecting block %ld ...\n", blkNB);
	fflush(stdout);
	
	IDin = image_ID(IDin_name);
	IDblknb = image_ID(IDblknb_name);
	naxis = data.image[IDin].md[0].naxis;
	mmax = data.image[IDblknb].md[0].size[0];
	
	if(data.image[IDin].md[0].size[0] != data.image[IDblknb].md[0].size[0])
		{			
			printf("WARNING: block index file and telemetry have different sizes\n");
			fflush(stdout);
			mmax = data.image[IDin].md[0].size[0];
			if(data.image[IDblknb].md[0].size[0]<mmax)
				mmax = data.image[IDblknb].md[0].size[0];
		}
	



	NBmodes1 = 0;
	for(m=0;m<mmax;m++)
		{
			if(data.image[IDblknb].array.U[m] == blkNB)
				NBmodes1++;
		}

	
	sizearray = (long*) malloc(sizeof(long)*naxis);
	
	for(axis=0;axis<naxis;axis++)
		sizearray[axis] = data.image[IDin].md[0].size[axis];
	sizearray[0] = NBmodes1;


	IDout = create_image_ID(IDout_name, naxis, sizearray, FLOAT, 0, 0);
	
	
	xsize = data.image[IDin].md[0].size[0];
	if(naxis>1)
		ysize = data.image[IDin].md[0].size[1];
	else
		ysize = 1;
	if(naxis>2)
		zsize = data.image[IDin].md[0].size[2];
	else
		zsize = 1;
	
	
	
	cnt = 0;

	for(jj=0;jj<ysize;jj++)
		for(kk=0;kk<zsize;kk++)
			for(ii=0;ii<mmax;ii++)
				if(data.image[IDblknb].array.U[ii] == blkNB)
						{
							//printf("%ld / %ld   cnt = %8ld / %ld\n", ii, xsize, cnt, NBmodes1*ysize*zsize);
							//fflush(stdout);
							data.image[IDout].array.F[cnt] = data.image[IDin].array.F[kk*xsize*ysize + jj*ysize + ii];
							cnt++;
						}		
	
	free(sizearray);
	
		
	return(IDout);
}






//
// IDin_name is a 2D or 3D image
//
// optional: inmask selects input pixels to be used
//           outmask selects output pixel(s) to be used
// default: use all channels as both input and output
//
// Note: if atmospheric wavefronts, data should be piston-free
//
// outMode
//	0: do not write individual filters
//	1: write individual filters
// (note: output filter cube always written)
//
//
// if LOOPmode = 1, operate in a loop, and re-run filter computation everytime IDin_name changes
//

long LINARFILTERPRED_Build_LinPredictor(const char *IDin_name, long PForder, float PFlag, double SVDeps, double RegLambda, const char *IDoutPF_name, int outMode, int LOOPmode, float LOOPgain)
{
    long IDin, IDmatA, IDout, IDinmask, IDoutmask;
    long nbspl; // number of samples
    long NBpixin, NBpixout;
    long NBmvec, NBmvec1;
    long mvecsize;
    long xsize, ysize;
    long ii, jj;
    long *pixarray_x;
    long *pixarray_y;
    long *pixarray_xy;

    long *outpixarray_x;
    long *outpixarray_y;
    long *outpixarray_xy;


    double *ave_inarray;
    int REG = 0;  // 1 if regularization
    long m, m1, pix, k0, dt;
    int Save = 1;
    long xysize;
    long IDmatC;
    int use_magma = 1; // use MAGMA library if available
    int magmacomp = 0;

    long IDfiltC;
    float *valfarray;
    float alpha;
    long PFpix;
    char filtname[200];
    char filtfname[200];
    long ID_Pfilt;
    float val, val0;
    long ind1;
    int ret;
    long IDoutPF2D;
    long IDoutPF3D;
    char IDoutPF_name3D[500];

    long NB_SVD_Modes;

    int DC_MODE = 0; // 1 if average value of each mode is removed



    long NBiter, iter;
    long semtrig = 2;
    long *imsizearray;
    float gain;

    char fname[200];

    time_t t;
    struct tm *uttime;
    struct timespec timenow;



    if(LOOPmode==0)
    {
        gain = 1.0;
        NBiter = 1;
    }

    else
    {
        NBiter = 100000000;
        gain = LOOPgain;
    }

    sprintf(IDoutPF_name3D, "%s_3D", IDoutPF_name);


    // =========== SELECT INPUT VALUES =======================

    IDin = image_ID(IDin_name);

    switch (data.image[IDin].md[0].naxis) {

    case 2 :
        nbspl = data.image[IDin].md[0].size[1];
        xsize = data.image[IDin].md[0].size[0];
        ysize = 1;
        break;

    case 3 :
        nbspl = data.image[IDin].md[0].size[2];
        xsize = data.image[IDin].md[0].size[0];
        ysize = data.image[IDin].md[0].size[1];
        break;

    default :
        printf("Invalid image size\n");
        break;
    }
    xysize = xsize*ysize;
    printf("xysize = %ld\n", xysize);



    pixarray_x = (long*) malloc(sizeof(long)*xsize*ysize);
    pixarray_y = (long*) malloc(sizeof(long)*xsize*ysize);
    pixarray_xy = (long*) malloc(sizeof(long)*xsize*ysize);
    ave_inarray = (double*) malloc(sizeof(double)*xsize*ysize);

    IDinmask = image_ID("inmask");
    if(IDinmask==-1)
    {
        NBpixin = 0; //xsize*ysize;

        for(ii=0; ii<xsize; ii++)
            for(jj=0; jj<ysize; jj++)
            {
                pixarray_x[NBpixin] = ii;
                pixarray_y[NBpixin] = jj;
                pixarray_xy[NBpixin] = jj*xsize+ii;
                NBpixin ++;
            }
    }
    else
    {
        NBpixin = 0;
        for(ii=0; ii<xsize; ii++)
            for(jj=0; jj<ysize; jj++)
                if(data.image[IDinmask].array.F[jj*xsize+ii] > 0.5)
                {
                    pixarray_x[NBpixin] = ii;
                    pixarray_y[NBpixin] = jj;
                    pixarray_xy[NBpixin] = jj*xsize+ii;
                    NBpixin ++;
                }
    }
    printf("NBpixin = %ld\n", NBpixin);




    // =========== SELECT OUTPUT VALUES =======================

    outpixarray_x = (long*) malloc(sizeof(long)*xsize*ysize);
    outpixarray_y = (long*) malloc(sizeof(long)*xsize*ysize);
    outpixarray_xy = (long*) malloc(sizeof(long)*xsize*ysize);

    IDoutmask = image_ID("outmask");
    if(IDoutmask==-1)
    {
        NBpixout = 0; //xsize*ysize;

        for(ii=0; ii<xsize; ii++)
            for(jj=0; jj<ysize; jj++)
            {
                outpixarray_x[NBpixout] = ii;
                outpixarray_y[NBpixout] = jj;
                outpixarray_xy[NBpixout] = jj*xsize+ii;
                NBpixout ++;
            }
    }
    else
    {
        NBpixout = 0;
        for(ii=0; ii<xsize; ii++)
            for(jj=0; jj<ysize; jj++)
                if(data.image[IDoutmask].array.F[jj*xsize+ii] > 0.5)
                {
                    outpixarray_x[NBpixout] = ii;
                    outpixarray_y[NBpixout] = jj;
                    outpixarray_xy[NBpixout] = jj*xsize+ii;
                    NBpixout ++;
                }
    }



    // ===================== BUILD DATA MATRIX ============================
    // build data matrix
    NBmvec = nbspl - PForder - (int) (PFlag) - 1;
    mvecsize = NBpixin * PForder; // size of each sample vector for AR filter, excluding regularization

    if(REG==0) // no regularization
    {
        printf("NBmvec   = %ld  -> %ld \n", NBmvec, NBmvec);
        NBmvec1 = NBmvec;
        IDmatA = create_2Dimage_ID("PFmatD", NBmvec, mvecsize);
    }
    else // with regularization
    {
        printf("NBmvec   = %ld  -> %ld \n", NBmvec, NBmvec + mvecsize);
        NBmvec1 = NBmvec + mvecsize;
        IDmatA = create_2Dimage_ID("PFmatD", NBmvec + mvecsize, mvecsize);
    }

    IDmatA = image_ID("PFmatD");


    // each column (ii = cst) is a measurement
    // m index is measurement
    // dt*NBpixin+pix index is pixel

    printf("mvecsize = %ld  (%ld x %ld)\n", mvecsize, PForder, NBpixin);
    printf("NBpixin = %ld\n", NBpixin);
    printf("NBpixout = %ld\n", NBpixout);
    printf("NBmvec1 = %ld\n", NBmvec1);
    printf("PForder = %ld\n", PForder);

    printf("xysize = %ld\n", xysize);
    printf("IDin = %ld\n\n", IDin);
    list_image_ID();


    if(DC_MODE == 1) // remove average
    {
        for(pix=0; pix<NBpixin; pix++)
        {
            ave_inarray[pix] = 0.0;
            for(m=0; m<nbspl; m++)
                ave_inarray[pix] += data.image[IDin].array.F[m*xysize+pixarray_xy[pix]];
            ave_inarray[pix] /= nbspl;
        }
    }
    else
    {
        for(pix=0; pix<NBpixin; pix++)
            ave_inarray[pix] = 0.0;
    }











    // ================= LOOP STARTS HERE ===============

    if(LOOPmode == 1)
        COREMOD_MEMORY_image_set_semflush(IDin_name, semtrig);


    for(iter=0; iter<NBiter; iter++)
    {
        if(LOOPmode == 1)
            sem_wait(data.image[IDin].semptr[semtrig]);



        if(LOOPmode == 0)
        {
            for(m=0; m<NBmvec1; m++)
            {
                k0 = m + PForder-1; // dt=0 index
                for(pix=0; pix<NBpixin; pix++)
                    for(dt=0; dt<PForder; dt++)
                        data.image[IDmatA].array.F[(NBpixin*dt+pix)*NBmvec1 + m] = data.image[IDin].array.F[(k0-dt)*xysize + pixarray_xy[pix]] - ave_inarray[pix];
            }
            free(ave_inarray);
        }
        else
        {
            for(m=0; m<NBmvec1; m++)
            {
                k0 = m + PForder-1; // dt=0 index
                for(pix=0; pix<NBpixin; pix++)
                    for(dt=0; dt<PForder; dt++)
                        data.image[IDmatA].array.F[(NBpixin*dt+pix)*NBmvec1 + m] = data.image[IDin].array.F[(k0-dt)*xysize + pixarray_xy[pix]];
            }
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
        //list_image_ID();



        // ===================== COMPUTE RECONSTRUCTION MATRIX ============================
        printf("Compute reconstruction matrix\n");
        fflush(stdout);


        NB_SVD_Modes = 10000;
#ifdef HAVE_MAGMA
        CUDACOMP_magma_compute_SVDpseudoInverse("PFmatD", "PFmatC", SVDeps, NB_SVD_Modes, "PF_VTmat", LOOPmode);
#else
        linopt_compute_SVDpseudoInverse("PFmatD", "PFmatC", SVDeps, NB_SVD_Modes, "PF_VTmat");
#endif

        if(Save==1)
        {
            save_fits("PF_VTmat", "!PF_VTmat.fits");
            save_fits("PFmatC", "!PFmatC.fits");
        }
        IDmatC = image_ID("PFmatC");



        // ===================== COMPUTE FILTERS ============================
        printf("Compute filters\n");
        fflush(stdout);




        ret = system("mkdir -p pixfilters");

        // 3D FILTER MATRIX - contains all pixels
        // axis 0 [ii] : input mode
        // axis 1 [jj] : reconstructed mode
        // axis 2 [kk] : time step


        // 2D Filter - contains only used input and output
        // axis 0 [ii1] : input mode x time step
        // axis 1 [jj1] : output mode

        if( LOOPmode == 0 )
        {
            IDoutPF2D = create_2Dimage_ID(IDoutPF_name, NBpixin*PForder, NBpixout);
            IDoutPF3D = create_3Dimage_ID(IDoutPF_name3D, xysize, xysize, PForder);
        }

        else
        {
            if(iter==0) // create 2D and 3D filters as shared memory
            {
                imsizearray = (long*) malloc(sizeof(long)*2);
                imsizearray[0] = NBpixin*PForder;
                imsizearray[1] = NBpixout;
                IDoutPF2D = create_image_ID(IDoutPF_name, 2, imsizearray, FLOAT, 1, 1);
                free(imsizearray);
                COREMOD_MEMORY_image_set_semflush(IDoutPF_name, -1);


                imsizearray = (long*) malloc(sizeof(long)*3);
                imsizearray[0] = xysize;
                imsizearray[1] = xysize;
                imsizearray[2] = PForder;
                IDoutPF3D = create_image_ID(IDoutPF_name3D, 3, imsizearray, FLOAT, 1, 1);
                free(imsizearray);
                COREMOD_MEMORY_image_set_semflush(IDoutPF_name3D, -1);
            }
            else
            {
                IDoutPF2D = image_ID(IDoutPF_name);
                IDoutPF3D = image_ID(IDoutPF_name3D);
            }
        }


        IDoutmask = image_ID("outmask");

        if(iter==0)
            valfarray = (float*) malloc(sizeof(float)*NBmvec);



        data.image[IDoutPF2D].md[0].write = 1;
        data.image[IDoutPF3D].md[0].write = 1;

        alpha = PFlag - ((long) PFlag);
        for(PFpix=0; PFpix<NBpixout; PFpix++) // PFpix is the pixel for which the filter is created (axis 1 in cube, jj)
        {
            if(LOOPmode==0)
            {   // INDIVIDUAL FILTERS
                sprintf(filtname, "PFfilt_%06ld_%03ld_%03ld", outpixarray_xy[PFpix], outpixarray_x[PFpix], outpixarray_y[PFpix]);
                sprintf(filtfname, "!./pixfilters/PFfilt_%06ld_%03ld_%03ld.fits", outpixarray_xy[PFpix], outpixarray_x[PFpix], outpixarray_y[PFpix]);
                ID_Pfilt = create_3Dimage_ID(filtname, xsize, ysize, PForder);
            }

            // fill in valfarray

            for(m=0; m<NBmvec; m++)
            {
                k0 = m + PForder -1;
                k0 += (long) PFlag;

                valfarray[m] = (1.0-alpha)*data.image[IDin].array.F[(k0)*xysize + outpixarray_xy[PFpix]] + alpha*data.image[IDin].array.F[(k0+1)*xysize + outpixarray_xy[PFpix]];
            }


            for(pix=0; pix<NBpixin; pix++)
            {
                for(dt=0; dt<PForder; dt++)
                {
                    val = 0.0;
                    ind1 = (NBpixin*dt+pix)*NBmvec1;
                    for(m=0; m<NBmvec; m++)
                        val += data.image[IDmatC].array.F[ind1+m] * valfarray[m];

                    val0 = data.image[IDoutPF3D].array.F[dt*xysize*xysize  + outpixarray_xy[PFpix]*xysize + pixarray_xy[pix]];
                    val = (1.0-gain)*val0 + gain*val;


                    data.image[IDoutPF2D].array.F[PFpix*(PForder*NBpixin) + dt*NBpixin + pix] = val;

                    data.image[IDoutPF3D].array.F[dt*xysize*xysize  + outpixarray_xy[PFpix]*xysize + pixarray_xy[pix]] = val;

                }
            }




            if(LOOPmode==0)
            {
                for(pix=0; pix<NBpixin; pix++)
                    for(dt=0; dt<PForder; dt++)
                    {
                        val = 0.0;
                        ind1 = (NBpixin*dt+pix)*NBmvec1;
                        for(m=0; m<NBmvec; m++)
                            val += data.image[IDmatC].array.F[ind1+m] * valfarray[m];

                        data.image[ID_Pfilt].array.F[xysize*dt + pixarray_xy[pix]] =  val;
                    }

                save_fits(filtname, filtfname);
            }
        }




        COREMOD_MEMORY_image_set_sempost_byID(IDoutPF2D, -1);
        data.image[IDoutPF2D].md[0].cnt0++;
        data.image[IDoutPF2D].md[0].write = 0;

        COREMOD_MEMORY_image_set_sempost_byID(IDoutPF3D, -1);
        data.image[IDoutPF3D].md[0].cnt0++;
        data.image[IDoutPF3D].md[0].write = 0;

		if(LOOPmode==1) // log filter
		{
			ret = system("mkdir -p ./PredictiveFilters/");
			   /// measure time
            t = time(NULL);
            uttime = gmtime(&t);
			clock_gettime(CLOCK_REALTIME, &timenow);

            sprintf(fname,"!./PredictiveFilters/%s_%02d:%02d:%02ld.%09ld.fits", IDoutPF_name, uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
			save_fits(IDoutPF_name, fname);
		}
		else
		{
			save_fits(IDoutPF_name, "!_outPF.fits");
			save_fits(IDoutPF_name3D, "!_outPF3D.fits");
        }
        
        printf("DONE\n");
        fflush(stdout);

    }









    free(valfarray);

    free(pixarray_x);
    free(pixarray_y);
    free(pixarray_xy);

    free(outpixarray_x);
    free(outpixarray_y);
    free(outpixarray_xy);

    return(IDoutPF2D);
}












//
// real-time apply predictive filter
// 
// filter can be smaller than input telemetry but needs to include contiguous pixels at the beginning of the input telemetry
//
long LINARFILTERPRED_Apply_LinPredictor_RT(const char *IDfilt_name, const char *IDin_name, const char *IDout_name)
{
	long IDout;
	long IDin;
	long IDfilt;
	long PForder;
	long NBpix_in;
	long NBpix_out;
	long *imsizearray;
	int semtrig = 7;
	
	float *inarray;
	float *outarray;
	
	long ii; // input index
	long jj; // output index6
	long kk; // time step index
	
	
	
	IDfilt = image_ID(IDfilt_name);
	IDin = image_ID(IDin_name);
		
	PForder = data.image[IDfilt].md[0].size[2];
	NBpix_in = data.image[IDfilt].md[0].size[0];
	NBpix_out = data.image[IDfilt].md[0].size[1];
	
	if(data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1] != NBpix_in)
		{
			printf("ERROR: lin predictor engine: filter input size does not match input telemetry\n");
			exit(0);
		}
	
	
	
	printf("Create prediction output %s\n", IDout_name);
	fflush(stdout);
	imsizearray = (long*) malloc(sizeof(long)*2);
	imsizearray[0] = NBpix_out;
	imsizearray[1] = 1;
	IDout = create_image_ID(IDout_name, 2, imsizearray, FLOAT, 1, 1);
	free(imsizearray);
	COREMOD_MEMORY_image_set_semflush(IDout_name, -1);
	printf("Done\n");
	fflush(stdout);
	
	inarray = (float*) malloc(sizeof(float)*NBpix_in*PForder);
	outarray = (float*) malloc(sizeof(float)*NBpix_out);
	
	
	while(sem_trywait(data.image[IDin].semptr[semtrig])==0) {}
	while(1)
	{
		// initialize output array to zero
		for(jj=0;jj<NBpix_out;jj++)
			outarray[jj] = 0.0;
		
		// shift input buffer entries back one time step
		for(kk=PForder-1;kk>0;kk--)
			for(ii=0;ii<NBpix_in;ii++)
				inarray[kk*NBpix_in+ii] = inarray[(kk-1)*NBpix_in+ii];
		
		// multiply input by prediction matrix .. except for measurement yet to come
		for(jj=0;jj<NBpix_out;jj++)
			for(ii=0;ii<NBpix_in;ii++)
				for(kk=1;kk<PForder;kk++)
					outarray[jj] += data.image[IDfilt].array.F[kk*NBpix_in*NBpix_out + jj*NBpix_in + ii] * inarray[kk*NBpix_in+ii];	
		
		sem_wait(data.image[IDin].semptr[semtrig]);
		
		// write new input in inarray vector
		for(ii=0;ii<NBpix_in;ii++)
			inarray[ii] = data.image[IDin].array.F[ii];
		
		
		// multiply input by prediction matrix
		for(jj=0;jj<NBpix_out;jj++)
			for(ii=0;ii<NBpix_in;ii++)
				outarray[jj] += data.image[IDfilt].array.F[jj*NBpix_in + ii] * inarray[ii];	

		data.image[IDout].md[0].write = 1;
		for(jj=0;jj<NBpix_out;jj++)
			data.image[IDout].array.F[jj] = outarray[jj];
		COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
		data.image[IDout].md[0].cnt0 ++;
        data.image[IDout].md[0].write = 0;
	}

	free(inarray);
	free(outarray);
	
	return(IDout);
}














//
// 
// out : prediction
//
// ADDITIONAL OUTPUTS:
// outf : time-shifted measurement
//


long LINARFILTERPRED_Apply_LinPredictor(const char *IDfilt_name, const char *IDin_name, float PFlag, const char *IDout_name)
{
	long IDout;
	long IDin;
	long IDfilt;
	long xsize, ysize, xysize;
	long nbspl;
	long PForder;
	long step;
	long kk;
	float alpha;
	long PFlagl;
	long ii, iip;
	float valp, valf;
	
	long IDoutf;
	
		
	
	IDin = image_ID(IDin_name);
	IDfilt = image_ID(IDfilt_name);
	
	switch (data.image[IDin].md[0].naxis) {
		
		case 2 :
		nbspl = data.image[IDin].md[0].size[1];
		xsize = data.image[IDin].md[0].size[0];
		ysize = 1;
		IDout = create_2Dimage_ID(IDout_name, xsize, nbspl);
		IDoutf = create_2Dimage_ID("outf", xsize, nbspl);
		break;
		
		case 3 :
		nbspl = data.image[IDin].md[0].size[2];
		xsize = data.image[IDin].md[0].size[0];
		ysize = data.image[IDin].md[0].size[1];
		IDout = create_3Dimage_ID(IDout_name, xsize, ysize, nbspl);
		IDoutf = create_3Dimage_ID("outf", xsize, ysize, nbspl);
		break;
		
		default :
		printf("Invalid image size\n");
		break;
	}
	xysize = xsize*ysize;
	
	PForder = data.image[IDfilt].md[0].size[2];
	
	if((data.image[IDfilt].md[0].size[0]!=xysize)||(data.image[IDfilt].md[0].size[1]!=xysize))
		{
			printf("ERROR: filter \"%s\" size is incorrect\n", IDfilt_name);
			exit(0);
		}
	
	alpha = PFlag - ((long) PFlag);
	PFlagl = (long) PFlag;
	
	for(kk=PForder;kk<nbspl;kk++) // time step
	{
		for(iip=0;iip<xysize;iip++) // predicted variable
			{
				valp = 0.0; // prediction 
				for(step=0;step<PForder;step++)
					{
						for(ii=0;ii<xsize*ysize;ii++) // input variable
							valp += data.image[IDfilt].array.F[xysize*xysize*step + iip*xysize + ii] * data.image[IDin].array.F[(kk-step)*xysize + ii];
					}
				data.image[IDout].array.F[kk*xysize+iip] = valp;
			
			
				valf = 0.0;
				if(kk+PFlag+1<nbspl)
					valf = (1.0-alpha) * data.image[IDin].array.F[(kk+PFlagl)*xysize+iip] + alpha * data.image[IDin].array.F[(kk+PFlagl+1)*xysize+iip];
				data.image[IDoutf].array.F[kk*xysize+iip] = valf;
			}
	}
	
	
	return(IDout);
}



//
// IDin_name is a 2 or 3D image, open-loop disturbance
// last axis is time (step)
// this optimization asssumes no correlation in noise
//
float LINARFILTERPRED_ScanGain(char* IDin_name, float multfact, float framelag)
{
	float gain;
	float gainmax = 1.1;
	float residual;
	float optgainblock;
	float residualblock;
	float residualblock0;
	float gainstep = 0.01;
	long IDin;
	
	long nbstep;
	long step, step0, step1;
	
	long framelag0;
	long framelag1;
	float alpha;
	
	float *actval_array; // actuator value
	float actval;
	
	long nbvar;
	long axis, naxis;
	
	double *errval;
	double errvaltot;
	long cnt;
	
	FILE *fp;
	char fname[200];
	float mval;
	long ii;
	float tmpv;
	
	int TEST = 0;
	float TESTperiod = 20.0;
	
	// results
	float *optgain;
	float *optres;
	float *res0;
	int optinit = 0;
	
	
	if(framelag<1.00000001)
		{
			printf("ERROR: framelag should be be > 1\n");
			exit(0);
		}
	
	IDin = image_ID(IDin_name);
	naxis = data.image[IDin].md[0].naxis;

	nbvar = 1;
	for(axis=0;axis<naxis-1;axis++)
		nbvar *= data.image[IDin].md[0].size[axis];
	errval = (double*) malloc(sizeof(double)*nbvar);
	
	nbstep = data.image[IDin].md[0].size[naxis-1];

	framelag0 = (long) framelag;
	framelag1 = framelag0+1;
	alpha = framelag-framelag0;

	printf("alpha = %f    nbvar = %ld\n", alpha, nbvar);
	
	list_image_ID();
	if(TEST==1)
		{
			for(ii=0;ii<nbvar;ii++)
			for(step=0;step<nbstep;step++)
				data.image[IDin].array.F[step*nbvar+ii] = 1.0*sin(2.0*M_PI*step/TESTperiod);
		}
	
	
	actval_array = (float*) malloc(sizeof(float)*nbstep);
	
	
	optgain = (float*) malloc(sizeof(float)*nbvar);
	optres = (float*) malloc(sizeof(float)*nbvar);
	res0 = (float*) malloc(sizeof(float)*nbvar);
	
	sprintf(fname, "gainscan.txt");
	
	gain = 0.2;
	ii = 0;
	fp = fopen(fname, "w");
	residualblock = 1.0e20;
	optgainblock = 0.0;
	for(gain=0;gain<gainmax;gain+=gainstep)
		{
			fprintf(fp, "%5.3f", gain);
		
			errvaltot = 0.0;
			for(ii=0;ii<nbvar;ii++)
			{
				errval[ii] = 0.0;
				cnt = 0.0;
				for(step=0;step<framelag1+2;step++)
					actval_array[step] = 0.0;
				for(step=framelag1; step<nbstep; step++)
					{
						step0 = step - framelag0;
						step1 = step - framelag1;

						actval = (1.0-alpha)*actval_array[step0] + alpha*actval_array[step1];
						mval = ((1.0-alpha)*data.image[IDin].array.F[step0*nbvar+ii] + alpha*data.image[IDin].array.F[step1*nbvar+ii]) - actval;
						actval_array[step] = multfact*(actval_array[step-1] + gain * mval);						
						tmpv = data.image[IDin].array.F[step*nbvar+ii] - actval_array[step];
						errval[ii] += tmpv*tmpv;
						cnt++;		
					}
				errval[ii] = sqrt(errval[ii]/cnt);
				fprintf(fp, " %10f", errval[ii]);
				errvaltot += errval[ii]*errval[ii];
			
				if(optinit==0)
				{
					optgain[ii] = gain;
					optres[ii] = errval[ii];
					res0[ii] = errval[ii];
				}
				else
				{
					if(errval[ii]<optres[ii])
						{
							optres[ii] = errval[ii];
							optgain[ii] = gain;
						}
				}
			}
			
			if(optinit==0)
				residualblock0 = errvaltot;
			
			optinit = 1;
			fprintf(fp, "%10f\n", errvaltot);	
			
			if(errvaltot < residualblock)
			{
				residualblock = errvaltot;
				optgainblock = gain;
			}
			
		}
	fclose(fp);
	
	free(actval_array);
	free(errval);
	
	for(ii=0;ii<nbvar;ii++)
		printf("MODE %4ld    optimal gain = %5.2f     residual = %.6f -> %.6f \n", ii, optgain[ii], res0[ii], optres[ii]);
	
	printf("\noptimal block gain = %f     residual = %.6f -> %.6f\n\n", optgainblock, sqrt(residualblock0), sqrt(residualblock));
	printf("RMS per mode = %f -> %f\n", sqrt(residualblock0/nbvar), sqrt(residualblock/nbvar));
	
	free(optgain);
	free(optres);
	free(res0);
	
	return(optgainblock);
}



//
// IDPF_name and IDPFM_name should be pre-loaded
//
long LINARFILTERPRED_PF_updatePFmatrix(const char *IDPF_name, const char *IDPFM_name, float alpha)
{
	long IDPF, IDPFM;
	long inmode, NBmode, outmode, NBmode2;
	long tstep, NBtstep;

	long *sizearray;
	long naxis;
	
	
	
	// IDPF should be square
	IDPF = image_ID(IDPF_name);
	NBmode = data.image[IDPF].md[0].size[0];
	NBmode2 = NBmode*NBmode;
	assert( data.image[IDPF].md[0].size[0] == data.image[IDPF].md[0].size[1]);
	NBtstep = data.image[IDPF].md[0].size[2];
	
	sizearray = (long*) malloc(sizeof(long)*2);
	sizearray[0] = NBmode*NBtstep;
	sizearray[1] = NBmode;
	naxis = 2;
	
	IDPFM = image_ID(IDPFM_name);
	
	if(IDPFM==-1)
		{
			printf("Creating shared mem image %s  [ %ld  x  %ld ]\n", IDPFM_name, sizearray[0], sizearray[1]);
			fflush(stdout);
			IDPFM = create_image_ID(IDPFM_name, naxis, sizearray, FLOAT, 1, 0);
		}
	free(sizearray);
	
	
	data.image[IDPFM].md[0].write = 1;
	for(outmode=0; outmode<NBmode; outmode++)
		{
			for(tstep=0;tstep<NBtstep;tstep++)
				for(inmode=0; inmode<NBmode; inmode++)
					data.image[IDPFM].array.F[outmode*(NBmode*NBtstep) + tstep*NBmode+inmode] = (1.0-alpha)*data.image[IDPFM].array.F[outmode*(NBmode*NBtstep) + tstep*NBmode+inmode] + alpha * data.image[IDPF].array.F[tstep*NBmode2 + outmode*NBmode + inmode];
		}	
	COREMOD_MEMORY_image_set_sempost_byID(IDPFM, -1);
	data.image[IDPFM].md[0].write = 0;
	data.image[IDPFM].md[0].cnt0++;


	return(IDPFM);
}



//
// IDmodevalIN_name : open loop modal coefficients
// IndexOffset      : predicted mode start at this input index 
// semtrig          : semaphore trigger index in input input
// IDPFM_name       : predictive filter matrix
// IDPFout_name     : prediction
//
//  NBiter: run for fixed number of itearationg
//  SAVEMODE:   0 no file output
//  			1	write txt and FITS output
//				2	write FITS telemetry with prediction: replace output measurements with predictions
// 
//	tlag is only used if SAVEMODE = 2	
//  used outmask to identify outputs 
//
long LINARFILTERPRED_PF_RealTimeApply(const char *IDmodevalIN_name, long IndexOffset, int semtrig, const char *IDPFM_name, long NBPFstep, const char *IDPFout_name, int nbGPU, long loop, long NBiter, int SAVEMODE, float tlag, long PFindex)
{
	long IDmodevalIN;
	long NBmodeIN, NBmodeIN0, NBmodeOUT, mode;
	long IDPFM;
	
	long IDINbuff;
	long tstep;
	long *sizearray;
	long naxis;

	long IDPFout;
	long ii;
	
	int *GPUsetPF;
	char GPUsetfname[200];
	int gpuindex;
    int_fast8_t status;
    int_fast8_t GPUstatus[100];
    FILE *fp;
	int ret;

	time_t t;
    struct tm *uttime;
    struct timespec timenow;
	double timesec, timesec0;
	long IDsave;
	
	FILE *fpout;
	long iter;
	long kk;
	
	long IDinmask;
	long *inmaskindex;
	long NBinmaskpix;
	
	
	long tlag0;
	float tlagalpha = 0.0;

	long IDoutmask;
	long *outmaskindex;
	long NBoutmaskpix;
	long kk0, kk1;
	float val, val0, val1;
	long ii0, ii1;
	
	long IDmasterout;
	char imname[200];
	
	
	IDmodevalIN = image_ID(IDmodevalIN_name);
	NBmodeIN0 = data.image[IDmodevalIN].md[0].size[0];
	
	IDPFM = image_ID(IDPFM_name);
	NBmodeOUT = data.image[IDPFM].md[0].size[1]; 

	sprintf(imname, "aol%ld_modevalPF", loop);
	IDmasterout = image_ID(imname);
	
	IDinmask = image_ID("inmask");
	if(IDinmask!=-1)
	{	
		NBinmaskpix = 0;
		for(ii=0;ii<data.image[IDinmask].md[0].size[0];ii++)
			if(data.image[IDinmask].array.F[ii] > 0.5)
				NBinmaskpix ++;
			
		inmaskindex = (long*) malloc(sizeof(long)*NBinmaskpix);
		NBinmaskpix = 0;
		for(ii=0;ii<data.image[IDinmask].md[0].size[0];ii++)
			if(data.image[IDinmask].array.F[ii] > 0.5)
				{
					inmaskindex[NBinmaskpix] = ii;
					NBinmaskpix++;
				}
		//printf("Number of active input modes  = %ld\n", NBinmaskpix);
	}
	else
	{
		NBinmaskpix = NBmodeIN0;
		printf("no input mask -> assuming NBinmaskpix = %ld\n", NBinmaskpix);
		IDinmask = create_2Dimage_ID("inmask", NBinmaskpix, 1);
		for(ii=0;ii<data.image[IDinmask].md[0].size[0];ii++)
			data.image[IDinmask].array.F[ii] = 1.0;
		inmaskindex = (long*) malloc(sizeof(long)*NBinmaskpix);
		for(ii=0;ii<data.image[IDinmask].md[0].size[0];ii++)
			inmaskindex[NBinmaskpix] = ii;
	}
	NBmodeIN = NBinmaskpix;
	
	NBPFstep = data.image[IDPFM].md[0].size[0]/NBmodeIN; 
	
	printf("Number of input modes         = %ld\n", NBmodeIN0);
	printf("Number of active input modes  = %ld\n", NBmodeIN);
	printf("Number of output modes        = %ld\n", NBmodeOUT);
	printf("Number of time steps          = %ld\n", NBPFstep);
	if(IDmasterout!=-1)
		printf("Writing result in master output stream %s  (%ld)\n", imname, IDmasterout);






	if((SAVEMODE>0)||(IDmasterout!=-1))
	{
		IDoutmask = image_ID("outmask");
		if(IDoutmask == -1)
			{
				printf("ERROR: outmask image required\n");
				exit(0);
			}
		NBoutmaskpix = 0;
		for(ii=0;ii<data.image[IDoutmask].md[0].size[0];ii++)
			if(data.image[IDoutmask].array.F[ii] > 0.5)
				NBoutmaskpix ++;
			
		outmaskindex = (long*) malloc(sizeof(long)*NBoutmaskpix);
		NBoutmaskpix = 0;
		for(ii=0;ii<data.image[IDoutmask].md[0].size[0];ii++)
			if(data.image[IDoutmask].array.F[ii] > 0.5)
				{
					outmaskindex[NBoutmaskpix] = ii;
					NBoutmaskpix++;
				}
		if(NBoutmaskpix != NBmodeOUT)
			{
				printf("ERROR: NBoutmaskpix (%ld)   !=   NBmodeOUT (%ld)\n", NBoutmaskpix, NBmodeOUT);
				list_image_ID();
				exit(0);
			}
	}

	
	
	IDINbuff = create_2Dimage_ID("INbuffer", NBmodeIN, NBPFstep);
	
	sizearray = (long*) malloc(sizeof(long)*2);
	sizearray[0] = NBmodeOUT;
	sizearray[1] = 1;
	naxis = 2;
	IDPFout = image_ID(IDPFout_name);

	if(IDPFout==-1)
		IDPFout = create_image_ID(IDPFout_name, naxis, sizearray, FLOAT, 1, 0);
	free(sizearray);
	
	
	if(nbGPU>0)
		{
			GPUsetPF = (int*) malloc(sizeof(int)*nbGPU);
			
			for(gpuindex=0;gpuindex<nbGPU;gpuindex++)
			{
				sprintf(GPUsetfname, "./conf/conf_PFb%ldGPU%ddevice.txt", PFindex, gpuindex); 
				fp = fopen(GPUsetfname, "r");
				if(fp==NULL)
					{
						printf("ERROR: file %s not found\n", GPUsetfname);
						exit(0);
					}
				ret = fscanf(fp, "%d", &GPUsetPF[gpuindex]);
				fclose(fp);
			}
			printf("USING %d GPUs: ", nbGPU);
			for(gpuindex=0;gpuindex<nbGPU;gpuindex++)
				printf(" %d", GPUsetPF[gpuindex]);
			printf("\n\n"); 
		}
	else
		printf("Using CPU\n");
	
	
	
	
	


	iter = 0;
	if(SAVEMODE>0)
		if(NBiter>50000)
			NBiter=50000;
		
		
	if(SAVEMODE == 1)
		IDsave = create_2Dimage_ID("testPFsave", 1+NBmodeIN0+NBmodeOUT, NBiter);
	if(SAVEMODE == 2)
		IDsave = create_3Dimage_ID("testPFTout", NBmodeIN0, 1, NBiter);
	
	
//	t = time(NULL);
//    uttime = gmtime(&t);			
//	clock_gettime(CLOCK_REALTIME, &timenow);
//	timesec0 = 3600.0*uttime->tm_hour  + 60.0*uttime->tm_min + 1.0*(timenow.tv_sec % 60) + 1.0e-9*timenow.tv_nsec;
 
	

	while(iter!=NBiter)
	{
	//	printf("iter %5ld / %5ld", iter, NBiter);
	//	fflush(stdout);
		
		sem_wait(data.image[IDmodevalIN].semptr[semtrig]);
	//	printf("\n");
	//	fflush(stdout);
			
		// fill in buffer
		for(mode=0; mode<NBmodeIN; mode++)
			data.image[IDINbuff].array.F[mode] = data.image[IDmodevalIN].array.F[IndexOffset + inmaskindex[mode]];


		if(nbGPU>0)
		{
	
			#ifdef HAVE_CUDA
			if(iter==0)
				GPU_loop_MultMat_setup(0, IDPFM_name, "INbuffer", IDPFout_name, nbGPU, GPUsetPF, 0, 1, 1, loop);
			GPU_loop_MultMat_execute(0, &status, &GPUstatus[100], 1.0, 0.0, 0);
			#endif
		//	list_image_ID();
		}
		else
		{
			// compute output : matrix vector mult
			data.image[IDPFout].md[0].write = 1;
			for(mode=0;mode<NBmodeOUT;mode++)
			{
				data.image[IDPFout].array.F[mode] = 0.0;
				for(ii=0;ii<NBmodeIN*NBPFstep;ii++)
					data.image[IDPFout].array.F[mode] += data.image[IDINbuff].array.F[ii] * data.image[IDPFM].array.F[mode*data.image[IDPFM].md[0].size[0]+ii];
			}
			COREMOD_MEMORY_image_set_sempost_byID(IDPFout, -1);
			data.image[IDPFout].md[0].write = 0;
			data.image[IDPFout].md[0].cnt0++;
		}
		
		if(iter==0)
			{
				 /// measure time
				t = time(NULL);
				uttime = gmtime(&t);			
				clock_gettime(CLOCK_REALTIME, &timenow);
				timesec0 = 1.0*timenow.tv_sec + 1.0e-9*timenow.tv_nsec;				
			
			
			// fprintf(fp, "%02d:%02d:%02ld.%09ld ", uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
			
			}	
		
		if(SAVEMODE == 1)
			{
		//		printf("	Saving step (mode = 1) ...");
		//		fflush(stdout);
				
				t = time(NULL);
                uttime = gmtime(&t);
				clock_gettime(CLOCK_REALTIME, &timenow);
				timesec = 1.0*timenow.tv_sec + 1.0e-9*timenow.tv_nsec;
				
				kk = 0;
				data.image[IDsave].array.F[iter*(1+NBmodeIN0+NBmodeOUT)] = (float) (timesec - timesec0);
				//printf(" [%f] ", data.image[IDsave].array.F[iter*(1+NBmodeIN0+NBmodeOUT)]);
				kk++;
				for(mode=0;mode<NBmodeIN0;mode++)
					{
						data.image[IDsave].array.F[iter*(1+NBmodeIN0+NBmodeOUT) + kk] = data.image[IDmodevalIN].array.F[IndexOffset + mode];
						kk++;
					}
				for(mode=0;mode<NBmodeOUT;mode++)
					{
						data.image[IDsave].array.F[iter*(1+NBmodeIN0+NBmodeOUT) + kk] = data.image[IDPFout].array.F[mode];
						kk++;
					}
			//	printf(" done\n");
			//	fflush(stdout);
			}
		if(SAVEMODE == 2)
			{
			//	printf("	Saving step (mode = 2) ...");
			//	fflush(stdout);

				for(mode=0;mode<NBmodeIN0;mode++)
					data.image[IDsave].array.F[iter*NBmodeIN0 + mode] = data.image[IDmodevalIN].array.F[IndexOffset + mode];
				for(mode=0;mode<NBmodeOUT;mode++)
					data.image[IDsave].array.F[iter*NBmodeIN0 + outmaskindex[mode]] = data.image[IDPFout].array.F[mode];
			//	printf(" done\n");
			//	fflush(stdout);
			}
	
	
	
		if(IDmasterout!=-1)
		{
			data.image[IDmasterout].md[0].write = 1;
			for(mode=0;mode<NBmodeOUT;mode++)
				data.image[IDmasterout].array.F[outmaskindex[mode]] = data.image[IDPFout].array.F[mode];
			COREMOD_MEMORY_image_set_sempost_byID(IDmasterout, -1);
			data.image[IDmasterout].md[0].write = 0;
			data.image[IDmasterout].md[0].cnt0++;
		}
	
	
	
	
		iter++;
	
		if(iter<NBiter)
			{				
				// do this now to save time when semaphore is posted
				for(tstep=NBPFstep-1; tstep>0; tstep--)
					{
						// tstep-1 -> tstep
						for(mode=0; mode<NBmodeIN; mode++)
							data.image[IDINbuff].array.F[NBmodeIN*tstep + mode] = data.image[IDINbuff].array.F[NBmodeIN*(tstep-1) + mode];
					}
			}
	}
	printf("LOOP done\n");
	fflush(stdout);
	
	

	
	// output ASCII file
	if(SAVEMODE == 1)
	{
		printf("SAVING DATA [1] ...");
		fflush(stdout);
		
		printf("IDsave = %ld     %ld  %ld\n", IDsave, 1+NBmodeIN0+NBmodeOUT, NBmodeOUT);
		list_image_ID();

		
	//	for(mode=0;mode<NBmodeOUT;mode++)
		//	printf("output %4ld -> %5ld\n", outmaskindex[mode]);
		
		fpout = fopen("testPFsave.dat", "w");
		for(iter=0;iter<NBiter;iter++)
		{
			fprintf(fpout, "%5ld ", iter);
			for(kk=0;kk<(1+NBmodeIN0+NBmodeOUT);kk++)
				fprintf(fpout, "%10f ", data.image[IDsave].array.F[iter*(1+NBmodeIN0+NBmodeOUT) + kk] );
				
		
		
			tlag0 = (long) tlag;
			tlagalpha = tlag-tlag0;
			
			ii0 = iter - (tlag0+1);
			ii1 = iter - (tlag0);				
			
			for(mode=0;mode<NBmodeOUT;mode++)
			{
				if(ii0>-1)
				{
					val0 = data.image[IDsave].array.F[ii0*(1+NBmodeIN0+NBmodeOUT) + 1+NBmodeIN0+mode];
					val1 = data.image[IDsave].array.F[ii1*(1+NBmodeIN0+NBmodeOUT) + 1+NBmodeIN0+mode];
				}
				val = tlagalpha*val0 + (1.0-tlagalpha)*val1;
				fprintf(fpout, "%10f ", val);
			}
			fprintf(fpout, "\n");
		}
		fclose(fpout);
	
		printf(" done\n");
		fflush(stdout);
	}
	
	free(inmaskindex);
	
	
	
	
	
	
	
	if(SAVEMODE==2) // time shift predicted output into FITS output
		{
			tlag0 = (long) tlag;
			tlagalpha = tlag-tlag0;
			for(kk=NBiter-1; kk>tlag0; kk--)
				{
					kk0 = kk-(tlag0+1);
					kk1 = kk-(tlag0);
										
					for(mode=0;mode<NBmodeOUT;mode++)
						{
							val0 = data.image[IDmodevalIN].array.F[kk0*NBmodeIN0 + outmaskindex[mode]];
							val1 = data.image[IDmodevalIN].array.F[kk1*NBmodeIN0 + outmaskindex[mode]];
							val = tlagalpha*val0 + (1.0-tlagalpha)*val1;
							
							data.image[IDsave].array.F[kk*NBmodeIN0 + outmaskindex[mode]] = val;							
						}
				}

			save_fits("testPFTout", "!testPFTout.fits");			
		}
	
	
	if(SAVEMODE>0)
		free(outmaskindex);
	
	
	return(IDPFout);
}










