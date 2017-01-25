#include <fitsio.h> 
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sched.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_multifit.h>

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
static int clock_gettime(int clk_id, struct mach_timespec *t){
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
#include "00CORE/00CORE.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "statistic/statistic.h"
#include "info/info.h"
#include "linopt_imtools/linopt_imtools.h"
#include "cudacomp/cudacomp.h"

extern DATA data;


static long NBPARAM;
static long double C0;
// polynomial coeff (degree = 1)
static long double *polycoeff1 = NULL;
// polynomial coeff (degree = 2)
static long double *polycoeff2 = NULL;
static long dfcnt = 0;

int fmInit = 0;







// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string 


int linopt_compute_linRM_from_inout_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,4)+CLI_checkarg(4,4)==0)
    {
      linopt_compute_linRM_from_inout(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string);
      return 0;
    }
  else
    return 1;
}

int linopt_compute_1Dfit_cli()
{
	if(CLI_checkarg(1,5)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,5)+CLI_checkarg(5,2)==0)
    {
		linopt_compute_1Dfit(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.numl);
      return 0;
    }
  else
    return 1;
}


int linopt_imtools_makeCosRadModes_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)==0)
    {
      linopt_imtools_makeCosRadModes(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf);
      return 0;
    }
  else
    return 1;
}


int linopt_imtools_makeCPAmodes_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)==0)
    {
      linopt_imtools_makeCPAmodes(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf, 1);
      return 0;
    }
  else
    return 1;
}


int linopt_imtools_mask_to_pixtable_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,3)==0)
    {
      linopt_imtools_mask_to_pixtable(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}

int linopt_imtools_Image_to_vec_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,4)+CLI_checkarg(4,3)==0)
    {
      linopt_imtools_Image_to_vec(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string);
      return 0;
    }
  else
    return 1;
}


int linopt_imtools_vec_to_2DImage_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,4)+CLI_checkarg(4,3)+CLI_checkarg(5,2)+CLI_checkarg(6,2)==0)
    {
      linopt_imtools_vec_to_2DImage(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numl);
      return 0;
    }
  else
    return 1;
}


int linopt_imtools_image_construct_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,3)==0)
    {
      linopt_imtools_image_construct(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}



int linopt_imtools_image_construct_stream_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,4)==0)
    {
      linopt_imtools_image_construct_stream(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}



int linopt_compute_SVDdecomp_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,3)==0)
    {
      linopt_compute_SVDdecomp(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}


int linopt_imtools_image_fitModes_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,4)+CLI_checkarg(4,1)+CLI_checkarg(5,3)==0)
    {
      linopt_imtools_image_fitModes(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.string, 0);
      return 0;
    }
  else
    return 1;
}



int init_linopt_imtools()
{
    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "image linear decomposition and optimization tools");
    data.NBmodule++;


    strcpy(data.cmd[data.NBcmd].key,"lincRMiter");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_compute_linRM_from_inout_cli;
    strcpy(data.cmd[data.NBcmd].info,"estimate response matrix from input and output");
    strcpy(data.cmd[data.NBcmd].syntax,"<input cube> <inmask> <output cube> <RM>");
    strcpy(data.cmd[data.NBcmd].example,"lincRMiter inC inmask outC imRM");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_compute_linRM_iter(char *IDinput_name, char *IDinmask_name, char *IDoutput_name, char *IDRM_name)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"linopt1Dfit");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_compute_1Dfit_cli;
    strcpy(data.cmd[data.NBcmd].info,"least-square 1D fit");
    strcpy(data.cmd[data.NBcmd].syntax,"<output data file> <NBpt> <fit order> <output coeff file> <fit MODE>");
    strcpy(data.cmd[data.NBcmd].example,"linopt1Dfit data.txt 1000 10 fitsol.txt 0");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_compute_1Dfit(char *fnamein, long NBpt, long MaxOrder, char *fnameout, int MODE)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"mkcosrmodes");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_makeCosRadModes_cli;
    strcpy(data.cmd[data.NBcmd].info,"make basis of cosine radial modes");
    strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <image size [long]> <kmax [long]> <radius [float]> <overfill factor [float]>");
    strcpy(data.cmd[data.NBcmd].example,"mkcosrmodes cmodes 256 100 80.0 2.0");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_makeCosRadModes(char *ID_name, long size, long kmax, float radius, float radfactlim, int writeMfile)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"mkFouriermodes");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_makeCPAmodes_cli;
    strcpy(data.cmd[data.NBcmd].info,"make basis of Fourier Modes");
    strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <image size> <CPAmax float> <deltaCPA float> <beam radius> <overfill factor>");
    strcpy(data.cmd[data.NBcmd].example,"mkFouriermodes fmodes 256 10.0 0.8 80.0 2.0");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_makeCPAmodes(char *ID_name, long size, float CPAmax, float deltaCPA, float radius, float radfactlim)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"mask2pixtable");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_mask_to_pixtable_cli;
    strcpy(data.cmd[data.NBcmd].info,"make pixel tables from mask");
    strcpy(data.cmd[data.NBcmd].syntax,"<maskname> <pixindex> <pixmult>");
    strcpy(data.cmd[data.NBcmd].example,"mask2pixtable mask pixi pixm");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_mask_to_pixtable(char *IDmask_name, char *IDpixindex_name, char *IDpixmult_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"im2vec");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_Image_to_vec_cli;
    strcpy(data.cmd[data.NBcmd].info,"remap image to vector");
    strcpy(data.cmd[data.NBcmd].syntax,"<imagename> <pixindex> <pixmult> <vecname>");
    strcpy(data.cmd[data.NBcmd].example,"im2vec im pixi pixm vecim");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_Image_to_vec(char *ID_name, char *IDpixindex_name, char *IDpixmult_name, char *IDvec_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"vec2im");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_vec_to_2DImage_cli;
    strcpy(data.cmd[data.NBcmd].info,"remap vector to image");
    strcpy(data.cmd[data.NBcmd].syntax,"<vecname> <pixindex> <pixmult> <imname> <xsize> <ysize>");
    strcpy(data.cmd[data.NBcmd].example,"im2vec vecim pixi pixm im 512 512");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_vec_to_2DImage(char *IDvec_name, char *IDpixindex_name, char *IDpixmult_name, char *ID_name, long xsize, long ysize)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imlinconstruct");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_image_construct_cli;
    strcpy(data.cmd[data.NBcmd].info,"construct image as linear sum of modes");
    strcpy(data.cmd[data.NBcmd].syntax,"<modes> <coeffs> <outim>");
    strcpy(data.cmd[data.NBcmd].example,"imlinconstruct modes coeffs outim");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_image_construct(char *IDmodes_name, char *IDcoeff_name, char *ID_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imlinconstructs");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_image_construct_stream_cli;
    strcpy(data.cmd[data.NBcmd].info,"construct image as linear sum of modes (stream mode)");
    strcpy(data.cmd[data.NBcmd].syntax,"<modes> <coeffs> <outim>");
    strcpy(data.cmd[data.NBcmd].example,"imlinconstructs modes coeffs outim");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_image_construct_stream(char *IDmodes_name, char *IDcoeff_name, char *IDout_name)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"imsvd");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_compute_SVDdecomp_cli;
    strcpy(data.cmd[data.NBcmd].info,"Singular values decomposition");
    strcpy(data.cmd[data.NBcmd].syntax,"<image cube> <SVD modes> <coeffs>");
    strcpy(data.cmd[data.NBcmd].example,"imsvd imc svdm coeffs");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_compute_SVDdecomp(char *IDin_name, char *IDout_name, char *IDcoeff_name)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"imfitmodes");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = linopt_imtools_image_fitModes_cli;
    strcpy(data.cmd[data.NBcmd].info,"fit image as sum of modes");
    strcpy(data.cmd[data.NBcmd].syntax,"<imname> <modes> <mask> <epssvd> <outcoeff>");
    strcpy(data.cmd[data.NBcmd].example,"imfitmodes im modes mask 0.01 outcim");
    strcpy(data.cmd[data.NBcmd].Ccall,"long linopt_imtools_image_fitModes(char *ID_name, char *IDmodes_name, char *IDmask_name, double SVDeps, char *IDcoeff_name, int reuse)");
    data.NBcmd++;



    // add atexit functions here


    return 0;

}


//
// solve for response matrix given a series of input and output
// initial value of RM should be best guess
// inmask = 0 over input that are known to produce no response
//
long linopt_compute_linRM_from_inout(char *IDinput_name, char *IDinmask_name, char *IDoutput_name, char *IDRM_name)
{
	long IDRM;
	long IDin;
	long IDinmask;
	long IDout;
	long insize; // number of input
	long xsizein, ysizein, xsizeout, ysizeout;
	double fitval;
	long kk, ii_in, jj_in, ii_out, jj_out;
	double tot;
	long IDtmp;
	double tmpv1;
	long iter;
	long IDout1;
	double alpha = 0.001;
	
	long *sizearray;
	long IDpokeM; // poke matrix (input)
	long IDoutM; // outputX
	double SVDeps = 1.0e-4;
	
	long NBact, act;
	long *inpixarray;
	long spl; // sample measurement
	long ii;
	long ID_rm;
	int autoMask_MODE = 0; // if 1, automatically measure input mask based on IDinput_name image
	long IDpinv;
	int use_magma = 0;
	
	int ngpu;
	
	ngpu = 0;
	setenv("CUDA_VISIBLE_DEVICES", "3,4", 1 );
	
	
	IDin = image_ID(IDinput_name);
	IDout = image_ID(IDoutput_name);
	IDRM = image_ID(IDRM_name);
	
		
	insize = data.image[IDin].md[0].size[2];
	xsizeout = data.image[IDRM].md[0].size[0];
	ysizeout = data.image[IDRM].md[0].size[1];
	xsizein = data.image[IDin].md[0].size[0];
	ysizein = data.image[IDin].md[0].size[1];
	
	if(autoMask_MODE==0)
		IDinmask = image_ID(IDinmask_name);
	else
		{
			IDinmask = create_2Dimage_ID("_RMmask", xsizein, ysizein);
			for(spl=0;spl<insize;spl++)
				for(ii=0;ii<xsizein*ysizein;ii++)
					if(data.image[IDin].array.F[spl*xsizein*ysizein + ii]>0.5)
						data.image[IDinmask].array.F[ii] = 1.0;
		}
		
	// create pokeM
	NBact = 0;
	for(ii=0;ii<xsizein*ysizein;ii++)
		if(data.image[IDinmask].array.F[ii]>0.5)
			NBact++;
			
	printf("NBact = %ld\n", NBact);
	
	inpixarray = (long*) malloc(sizeof(long)*NBact);
	act = 0;
	for(ii=0;ii<xsizein*ysizein;ii++)
		if(data.image[IDinmask].array.F[ii]>0.5)
			{
				inpixarray[act] = ii;
				act++;
			}
	
	
	
	sizearray = (long*) malloc(sizeof(long)*2);
	
	sizearray[0] = NBact;
	sizearray[1] = insize; // number of measurements

	printf("NBact = %ld\n", NBact);
	for(act=0;act<10;act++)
		printf("act %5ld -> pix %5ld\n", act, inpixarray[act]);
	
	 
	IDpokeM = create_2Dimage_ID("pokeM", NBact, insize);

	for(spl=0;spl<insize;spl++)
		for(act=0;act<NBact;act++)
			data.image[IDpokeM].array.F[NBact*spl+act] = data.image[IDin].array.F[spl*xsizein*ysizein + inpixarray[act]];
	save_fits("pokeM", "!_test_pokeM.fits");

	// compute pokeM pseudo-inverse
   	#ifdef HAVE_MAGMA
		CUDACOMP_magma_compute_SVDpseudoInverse("pokeM", "pokeMinv", SVDeps, insize, "VTmat", 0);
	#else
        linopt_compute_SVDpseudoInverse("pokeM", "pokeMinv", SVDeps, insize, "VTmat");
     #endif   
        
	list_image_ID();
	save_fits("pokeMinv", "!pokeMinv.fits");
	IDpinv = image_ID("pokeMinv");
	
	// multiply measurements by pokeMinv
	ID_rm = create_3Dimage_ID("_respmat", xsizeout, ysizeout, xsizein*ysizein);

	for(act=0;act<NBact;act++)
		{
			for(kk=0;kk<insize;kk++)
				for(ii=0;ii<xsizeout*ysizeout;ii++)
					data.image[ID_rm].array.F[inpixarray[act]*xsizeout*ysizeout + ii] += data.image[IDout].array.F[kk*xsizeout*ysizeout + ii] * data.image[IDpinv].array.F[kk*NBact+act];
		}
	save_fits("_respmat", "!_test_RM.fits");
//exit(0);
	
	
	// COMPUTE SOLUTION QUALITY
	
	IDRM = image_ID("_respmat");
	
	
	IDtmp = create_2Dimage_ID("_tmplicli", xsizeout, ysizeout);
	IDout1 = create_3Dimage_ID("testout", xsizeout, ysizeout, insize);
	
	printf("IDin  = %ld\n", IDin);
	printf("IDout = %ld\n", IDout);
	printf("IDinmask = %ld\n", IDinmask);
	
	// on iteration 0, compute initial fit value
	fitval = 0.0;

	

		for(kk=0;kk<insize;kk++)
		{
			printf("\r kk = %5ld / %5ld    ", kk, insize);
			fflush(stdout);
			
			for(ii_out=0;ii_out<xsizeout; ii_out++)
				for(jj_out=0;jj_out<ysizeout; jj_out++)
					data.image[IDtmp].array.F[jj_out*xsizeout+ii_out] = 0.0;
					
					
			for(ii_in=0;ii_in<xsizein;ii_in++)
				for(jj_in=0;jj_in<ysizein;jj_in++)
					{
						
								//printf("%ld  pix %ld %ld active\n", kk, ii_in, jj_in);
								for(ii_out=0;ii_out<xsizeout; ii_out++)
									for(jj_out=0;jj_out<ysizeout; jj_out++)
										data.image[IDtmp].array.F[jj_out*xsizeout+ii_out] += data.image[IDin].array.F[kk*xsizein*ysizein + jj_in*xsizein + ii_in]*data.image[IDRM].array.F[(jj_in*xsizein + ii_in)*xsizeout*ysizeout+jj_out*xsizeout+ii_out];							
							
					}
			for(ii_out=0;ii_out<xsizeout; ii_out++)
				for(jj_out=0;jj_out<ysizeout; jj_out++)
					{
						tmpv1 = data.image[IDtmp].array.F[jj_out*xsizeout+ii_out] - data.image[IDout].array.F[kk*xsizeout*ysizeout + jj_out*xsizeout+ii_out];
						fitval += tmpv1*tmpv1;
						data.image[IDout1].array.F[kk*xsizeout*ysizeout + jj_out*xsizeout+ii_out] = tmpv1; //data.image[IDtmp].array.F[jj_out*xsizeout+ii_out];
					}
		}
	printf("\n");
	printf("  %5ld    fitval = %.20f\n", kk, sqrt(fitval/xsizeout/ysizeout));
	
	delete_image_ID("_tmplicli");
		
	free(sizearray);
	free(inpixarray);
	
	return(IDout);
}





// r0pix is r=1 in pixel unit

long linopt_imtools_make1Dpolynomials(char *IDout_name, long NBpts, long MaxOrder, float r0pix)
{
	long IDout;
	long xsize, ysize, zsize;
	long ii, kk;
	float r;
	
	xsize = NBpts;
	ysize = 1;
	zsize = MaxOrder;
	
	IDout = create_3Dimage_ID(IDout_name, xsize, ysize, zsize);
	
	for(kk=0;kk<zsize;kk++)
		{
			for(ii=0;ii<xsize;ii++)
				{
					r = 1.0*ii/r0pix;
					data.image[IDout].array.F[kk*xsize+ii] = pow(r, 1.0*kk);
				}
		}
	
	return IDout;
}



// MODE : 
// 0 : polynomial
//
long linopt_compute_1Dfit(char *fnamein, long NBpt, long MaxOrder, char *fnameout, int MODE)
{
	float *xarray;
	float *valarray;
	
	FILE *fp;
	long ii;
	int ret;
	
	long IDin, IDin0;
	long IDmask;
	long IDmodes;
	long NBmodes;
	long m;
	
	float SVDeps = 0.0000001;
	
	long IDout, IDout0;
	double val, vale, err;
	
	long NBiter = 100;
	float gain = 1.0;
	long iter;
	
	
	xarray = (float*) malloc(sizeof(float)*NBpt);
	valarray = (float*) malloc(sizeof(float)*NBpt);
	
	fp = fopen(fnamein, "r");
	for(ii=0;ii<NBpt;ii++)
		ret = fscanf(fp, "%f %f\n", &xarray[ii], &valarray[ii]);		
	fclose(fp);
	
	IDin = create_2Dimage_ID("invect", NBpt, 1);
	IDin0 = create_2Dimage_ID("invect0", NBpt, 1);
	IDmask = create_2Dimage_ID("inmask", NBpt, 1);
	
	for(ii=0;ii<NBpt;ii++)
		{
//			printf("%18.16f  %+18.16f\n", xarray[ii], valarray[ii]);
			data.image[IDin].array.F[ii] = valarray[ii];
			data.image[IDin0].array.F[ii] = valarray[ii];
			data.image[IDmask].array.F[ii] = 1.0;
		}
	
	NBmodes = MaxOrder;
	IDmodes = create_3Dimage_ID("fitmodes", NBpt, 1, NBmodes);
	IDout = create_2Dimage_ID("outcoeff", NBmodes, 1);

	switch (MODE) {
		case 0 :
			for(m=0; m<NBmodes; m++)
			{
				for(ii=0;ii<NBpt;ii++)
				data.image[IDmodes].array.F[m*NBpt+ii] = pow(xarray[ii], 1.0*m);
			}
		break;
		case 1 :
			for(m=0; m<NBmodes; m++)
			{
				for(ii=0;ii<NBpt;ii++)
					data.image[IDmodes].array.F[m*NBpt+ii] = cos(xarray[ii]*M_PI*m);
			}
		break;
		default :
			printf("ERROR: MODE = %d not supported\n", MODE);
			exit(0);
		break;
	}
	
	list_image_ID();
	
	for(iter = 0; iter<NBiter; iter++)
	{
		linopt_imtools_image_fitModes("invect0", "fitmodes", "inmask", SVDeps, "outcoeffim0", 1);
		IDout0 = image_ID("outcoeffim0");
	
	
		for(m=0;m<NBmodes;m++)
			data.image[IDout].array.F[m] += gain*data.image[IDout0].array.F[m];
	
		for(ii=0;ii<NBpt;ii++)
		{	
			err = 0.0;
			val = 0.0;
			for(m=0; m<NBmodes; m++)
				val += data.image[IDout].array.F[m]*data.image[IDmodes].array.F[m*NBpt+ii];
			data.image[IDin0].array.F[ii] = data.image[IDin].array.F[ii] - val;
			err += data.image[IDin0].array.F[ii]*data.image[IDin0].array.F[ii];
		}
		err = sqrt(err/NBpt);
		printf("ITERATION %4ld   residual = %20g   [gain = %20g]\n", iter, err, gain);
		gain *= 0.95;
	}
	
	
	
	
	fp = fopen(fnameout, "w");
	for(m=0;m<NBmodes;m++)
		fprintf(fp, "%4ld %+.8g\n", m, data.image[IDout].array.F[m]);
	fclose(fp);
	
	
	fp = fopen("testout.txt", "w");
	err = 0.0;
	for(ii=0;ii<NBpt;ii++)
		{
			val = 0.0;
			for(m=0; m<NBmodes; m++)
				val += data.image[IDout].array.F[m]*data.image[IDmodes].array.F[m*NBpt+ii];
			vale = valarray[ii] - val;
			err += vale*vale;
			fprintf(fp, "%05ld  %18.16f  %18.16f   %18.16f\n", ii, xarray[ii], valarray[ii], val);
		}
	fclose(fp);
	err = sqrt(err/NBpt);
	
	printf("FIT error = %g m\n", err);
	
	free(xarray);
	free(valarray);
	
	return(IDout);
}





//
// make cosine radial modes
//
long linopt_imtools_makeCosRadModes(char *ID_name, long size, long kmax, float radius, float radfactlim)
{
    long ID;
    long ii, jj;
    float x, y, r;
    long k;
    long size2;
    long IDr;


    size2 = size*size;
    IDr = create_2Dimage_ID("linopt_tmpr", size, size);

    for(ii=0; ii<size; ii++)
    {
        x = (1.0*ii-0.5*size)/radius;
        for(jj=0; jj<size; jj++)
        {
            y = (1.0*jj-0.5*size)/radius;
            r = sqrt(x*x+y*y);
            data.image[IDr].array.F[jj*size+ii] = r;
        }
    }

    ID = create_3Dimage_ID(ID_name, size, size, kmax);

    for(k=0; k<kmax; k++)
        for(ii=0; ii<size2; ii++)
        {
            r = data.image[IDr].array.F[ii];
            if(r<radfactlim)
                data.image[ID].array.F[k*size2+ii] = cos(r*M_PI*k);
        }


    delete_image_ID("linopt_tmpr");

    return(ID);
}



long linopt_imtools_makeCPAmodes(char *ID_name, long size, float CPAmax, float deltaCPA, float radius, float radfactlim, int writeMfile)
{
    long ID;
    long IDx, IDy, IDr;
    float CPAx, CPAy;
    float x, y, r;
    long ii, jj;
    long k, k1;
    long NBmax;
    float *CPAxarray;
    float *CPAyarray;
    float *CPArarray;
    long size2;
    long NBfrequ;
    float y0;
    float ydist;
    float eps;
    FILE *fp;

    long IDfreq;

    eps = 0.1*deltaCPA;
    printf("size       = %ld\n", size);
    printf("CPAmax     = %f\n", CPAmax);
    printf("deltaCPA   = %f\n", deltaCPA);
    printf("radius     = %f\n", radius);
    printf("radfactlim = %f\n", radfactlim);


    size2 = size*size;
    IDx = create_2Dimage_ID("cpa_tmpx", size, size);
    IDy = create_2Dimage_ID("cpa_tmpy", size, size);
    IDr = create_2Dimage_ID("cpa_tmpr", size, size);


    printf("precomputing x, y, r\n");
    fflush(stdout);

    for(ii=0; ii<size; ii++)
    {
        x = (1.0*ii-0.5*size)/radius;
        for(jj=0; jj<size; jj++)
        {
            y = (1.0*jj-0.5*size)/radius;
            r = sqrt(x*x+y*y);
            data.image[IDx].array.F[jj*size+ii] = x;
            data.image[IDy].array.F[jj*size+ii] = y;
            data.image[IDr].array.F[jj*size+ii] = r;
        }
    }


    printf("CPA: max = %f   delta = %f\n", CPAmax, deltaCPA);
    fflush(stdout);
    NBfrequ = 0;
    for(CPAx=0; CPAx<CPAmax; CPAx+=deltaCPA)
        for(CPAy=-CPAmax; CPAy<CPAmax; CPAy+=deltaCPA)
            NBfrequ ++;

    printf("NBfrequ = %ld\n", NBfrequ);
    fflush(stdout);

    CPAxarray = (float*) malloc(sizeof(float)*NBfrequ);
    CPAyarray = (float*) malloc(sizeof(float)*NBfrequ);
    CPArarray = (float*) malloc(sizeof(float)*NBfrequ);


    NBfrequ = 0;
    ydist = 2.0*deltaCPA;
    y0 = 0.0;
    for(CPAx=0; CPAx<CPAmax; CPAx+=deltaCPA)
    {
        for(CPAy=0; CPAy<CPAmax; CPAy+=deltaCPA)
        {
            CPAxarray[NBfrequ] = CPAx;
            CPAyarray[NBfrequ] = CPAy;
            CPArarray[NBfrequ] = sqrt(CPAx*CPAx+CPAy*CPAy);
            NBfrequ++;
        }
        if(CPAx>eps)
        {
            for(CPAy=-deltaCPA; CPAy>-CPAmax; CPAy-=deltaCPA)
            {
                CPAxarray[NBfrequ] = CPAx;
                CPAyarray[NBfrequ] = CPAy;
                CPArarray[NBfrequ] = sqrt(CPAx*CPAx+CPAy*CPAy);
                NBfrequ++;
            }
        }
    }

    //  for(k1=0;k1<NBfrequ;k1++)
    //printf("%ld %f %f %f\n", k1, CPAxarray[k1], CPAyarray[k1], CPArarray[k1]);



    //  printf("sorting\n");
    // fflush(stdout);

    quick_sort3_float(CPArarray, CPAxarray, CPAyarray, NBfrequ);




    NBmax = NBfrequ*2;
    ID = create_3Dimage_ID(ID_name, size, size, NBmax-1);



    if(writeMfile==1)
    {
        fp = fopen("ModesExpr_CPA.txt", "w");
        fprintf(fp, "%4ld %10.5f %10.5f    1.0\n", (long) 0, 0.0, 0.0);
        k1 = 1;
        k = 2;
        while(k<NBmax)
        {
            CPAx = CPAxarray[k1];
            CPAy = CPAyarray[k1];
            if(CPAy<0)
            {
                fprintf(fp, "%4ld %10.5f %10.5f    cos(M_PI*(x*%.5f-y*%.5f))\n", k-1, CPAx, CPAy, CPAx, -CPAy);
                fprintf(fp, "%4ld %10.5f %10.5f    sin(M_PI*(x*%.5f-y*%.5f))\n", k, CPAx, CPAy, CPAx, -CPAy);
            }
            else
            {
                fprintf(fp, "%4ld %10.5f %10.5f    cos(M_PI*(x*%.5f+y*%.5f))\n", k-1, CPAx, CPAy, CPAx, CPAy);
                fprintf(fp, "%4ld %10.5f %10.5f    sin(M_PI*(x*%.5f+y*%.5f))\n", k, CPAx, CPAy, CPAx, CPAy);
            }
            k += 2;
            k1++;
        }

        fclose(fp);
    }

    delete_image_ID("cpamodesfreq");
    IDfreq = create_2Dimage_ID("cpamodesfreq", NBmax-1, 1);


    // mode 0 (piston)
    data.image[IDfreq].array.F[0] = 0.0;
    for(ii=0; ii<size2; ii++)
    {
        x = data.image[IDx].array.F[ii];
        y = data.image[IDy].array.F[ii];
        r = data.image[IDr].array.F[ii];
        if(r<radfactlim)
            data.image[ID].array.F[ii] = 1.0;
    }

    k1 = 1;
    k = 2;
    while(k<NBmax)
    {
        //      printf("\r%5ld / %5ld          ", k, NBmax);
        //      fflush(stdout);
        CPAx = CPAxarray[k1];
        CPAy = CPAyarray[k1];
        // printf("    %ld %f %f\n", k1, CPAx, CPAy);
        for(ii=0; ii<size2; ii++)
        {
            x = data.image[IDx].array.F[ii];
            y = data.image[IDy].array.F[ii];
            r = data.image[IDr].array.F[ii];
            data.image[IDfreq].array.F[k-1] = sqrt(CPAx*CPAx+CPAy*CPAy);
            data.image[IDfreq].array.F[k] = sqrt(CPAx*CPAx+CPAy*CPAy);
            if(r<radfactlim)
            {
                data.image[ID].array.F[(k-1)*size2+ii] = cos(M_PI*(x*CPAx+y*CPAy));
                data.image[ID].array.F[k*size2+ii] = sin(M_PI*(x*CPAx+y*CPAy));
            }
        }
        k+=2;
        k1++;
    }
    //  printf("done \n");
    // fflush(stdout);

    free(CPAxarray);
    free(CPAyarray);
    free(CPArarray);


    delete_image_ID("cpa_tmpx");
    delete_image_ID("cpa_tmpy");
    delete_image_ID("cpa_tmpr");

    // printf("done \n");
    //fflush(stdout);



    return NBmax;
}











/* ------------------------------------------------ */
/*                                                  */
/*   Maps image to array of pixel values using mask */
/*                                                  */
/* ------------------------------------------------ */


// to decompose image into modes:
// STEP 1: create index and mult tables (linopt_imtools_mask_to_pixtable)
//

long linopt_imtools_mask_to_pixtable(char *IDmask_name, char *IDpixindex_name, char *IDpixmult_name)
{
    long NBpix;
    long ii;
    long ID;
    long size;
    float eps = 1.0e-8;
    long k;
    long *sizearray;
    long IDpixindex, IDpixmult;

    ID = image_ID(IDmask_name);
    size = data.image[ID].md[0].nelement;



    NBpix = 0;
    for(ii=0; ii<size; ii++)
        if(data.image[ID].array.F[ii]>eps)
            NBpix++;

    sizearray = (long*) malloc(sizeof(long)*2);
    sizearray[0] = NBpix;
    sizearray[1] = 1;
    IDpixindex = create_image_ID(IDpixindex_name, 2, sizearray, LONG, 0, 0);
    IDpixmult = create_image_ID(IDpixmult_name, 2, sizearray, FLOAT, 0, 0);

    k = 0;
    for(ii=0; ii<size; ii++)
        if(data.image[ID].array.F[ii]>eps)
        {
            data.image[IDpixindex].array.L[k] = ii;
            data.image[IDpixmult].array.F[k] = data.image[ID].array.F[ii];
            k++;
        }

    //  printf("%ld active pixels in mask %s\n", NBpix, IDmask_name);

    return(NBpix);
}


//
//
//
long linopt_imtools_Image_to_vec(char *ID_name, char *IDpixindex_name, char *IDpixmult_name, char *IDvec_name)
{
    long ID;
    long ii;
    long k;
    long IDpixindex, IDpixmult;
    long IDvec;
    long NBpix;
    long naxisin;
    long sizexy;
    long kk;
    int atype;

    ID = image_ID(ID_name);
    naxisin = data.image[ID].md[0].naxis;
    atype = data.image[ID].md[0].atype;


    IDpixindex = image_ID(IDpixindex_name);
    IDpixmult = image_ID(IDpixmult_name);
    NBpix = data.image[IDpixindex].md[0].nelement;

    if(naxisin<3)
    {
        IDvec = create_2Dimage_ID(IDvec_name, NBpix, 1);
        for(k=0; k<NBpix; k++)
            data.image[IDvec].array.F[k] = data.image[IDpixmult].array.F[k] * data.image[ID].array.F[data.image[IDpixindex].array.L[k]];
    }
    else
    {
        sizexy = data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1];
        if(atype==FLOAT)
        {
            IDvec = create_2Dimage_ID(IDvec_name, NBpix, data.image[ID].md[0].size[2]);
            for(kk=0; kk<data.image[ID].md[0].size[2]; kk++)
                for(k=0; k<NBpix; k++)
                    data.image[IDvec].array.F[kk*NBpix+k] = data.image[IDpixmult].array.F[k] * data.image[ID].array.F[kk*sizexy+data.image[IDpixindex].array.L[k]];
        }
        if(atype==COMPLEX_FLOAT)
        {
            IDvec = create_2Dimage_ID(IDvec_name, NBpix*2, data.image[ID].md[0].size[2]);
            for(kk=0; kk<data.image[ID].md[0].size[2]; kk++)
                for(k=0; k<NBpix; k++)
                {
                    data.image[IDvec].array.F[kk*NBpix*2+2*k] = data.image[IDpixmult].array.F[k] * data.image[ID].array.CF[kk*sizexy+data.image[IDpixindex].array.L[k]].re;
                    data.image[IDvec].array.F[kk*NBpix*2+2*k+1] = data.image[IDpixmult].array.F[k] * data.image[ID].array.CF[kk*sizexy+data.image[IDpixindex].array.L[k]].im;
                }
        }

    }

    return(ID);
}



long linopt_imtools_vec_to_2DImage(char *IDvec_name, char *IDpixindex_name, char *IDpixmult_name, char *ID_name, long xsize, long ysize)
{
    long ID;
    long IDvec;
    long k;
    long IDpixindex, IDpixmult;
    long NBpix;

    IDvec = image_ID(IDvec_name);
    IDpixindex = image_ID(IDpixindex_name);
    IDpixmult = image_ID(IDpixmult_name);
    NBpix = data.image[IDpixindex].md[0].nelement;

    ID = create_2Dimage_ID(ID_name, xsize, ysize);

    for(k=0; k<NBpix; k++)
        data.image[ID].array.F[data.image[IDpixindex].array.L[k]] = data.image[IDvec].array.F[k]/data.image[IDpixmult].array.F[k];

    return (ID);
}



// rotation matrix written as SVD_VTm

long linopt_compute_SVDdecomp(char *IDin_name, char *IDout_name, char *IDcoeff_name)
{
    long IDin;
    long IDout;
    long IDcoeff;
    long ii1, jj1, k, ii;
    gsl_matrix *matrix_D; /* input */
    gsl_matrix *matrix_Dtra;
    gsl_matrix *matrix_DtraD;
    gsl_matrix *matrix_DtraD_evec;
    gsl_matrix *matrix1;
    gsl_matrix *matrix2;
    gsl_vector *matrix_DtraD_eval;
    gsl_eigen_symmv_workspace *w;
    gsl_matrix *matrix_save;


    long m;
    long n;
    long *arraysizetmp;

    long IDmodes, IDeigenmodes;
    long xsize_modes, ysize_modes, zsize_modes;
    long IDeigenmodesResp;
    long kk, kk1;
    long ID_RMmask;
    int ret;
    char command[200];
    long ID_VTmatrix;

    arraysizetmp = (long*) malloc(sizeof(long)*3);
  
  
    printf("[SVD start]");
    fflush(stdout);


    IDin = image_ID(IDin_name);


    n = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]; 
    m = data.image[IDin].md[0].size[2]; 

    matrix_DtraD_eval = gsl_vector_alloc (m);
    matrix_D = gsl_matrix_alloc (n,m);
    matrix_Dtra = gsl_matrix_alloc (m,n);
    matrix_DtraD = gsl_matrix_alloc (m,m);
    matrix_DtraD_evec = gsl_matrix_alloc (m,m);

   
    /* write matrix_D */
    for(k=0; k<m; k++)
    {
        for(ii=0; ii<n; ii++)
            gsl_matrix_set (matrix_D, ii, k, data.image[IDin].array.F[k*n+ii]);
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

    IDcoeff = create_2Dimage_ID(IDcoeff_name, m, 1);
    
 
    for(k=0; k<m; k++)
        data.image[IDcoeff].array.F[k] = gsl_vector_get(matrix_DtraD_eval,k);



    /** Write rotation matrix to go from DM modes to eigenmodes */
    arraysizetmp[0] = m;
    arraysizetmp[1] = m;
    ID_VTmatrix = image_ID("SVD_VTm");
    if(ID_VTmatrix!=-1)
        delete_image_ID("SVD_VTm");
    ID_VTmatrix = create_image_ID("SVD_VTm", 2, arraysizetmp, FLOAT, 0, 0);
    for(ii=0; ii<m; ii++) // modes
        for(k=0; k<m; k++) // modes
            data.image[ID_VTmatrix].array.F[k*m+ii] = (float) gsl_matrix_get( matrix_DtraD_evec, k, ii);

    /// Compute SVD decomp
    
    IDout = create_3Dimage_ID(IDout_name, data.image[IDin].md[0].size[0], data.image[IDin].md[0].size[1], data.image[IDin].md[0].size[2]);
    for(kk=0; kk<m; kk++) /// eigen mode index
    {
//        printf("eigenmode %4ld / %4ld  %g\n", kk, m, data.image[IDcoeff].array.F[kk]);
//       fflush(stdout);
        for(kk1=0; kk1<m; kk1++)
        {
            for(ii=0; ii<n; ii++)
                data.image[IDout].array.F[kk*n + ii] += data.image[ID_VTmatrix].array.F[kk1*m+kk]*data.image[IDin].array.F[kk1*n + ii];
        }
    }
    
 //   delete_image_ID("SVD_VTm");

 
    free(arraysizetmp);
    
    gsl_matrix_free(matrix_D);
    gsl_matrix_free(matrix_Dtra);
    gsl_matrix_free(matrix_DtraD);
    gsl_matrix_free(matrix_DtraD_evec);
    gsl_vector_free(matrix_DtraD_eval);

    printf("[SVD done]\n");
    fflush(stdout);

    return(IDout);
}




//
// Computes control matrix
// Conventions:
//   m: number of actuators (= NB_MODES)
//   n: number of sensors  (= # of pixels)
//
// This implementation computes the eigenvalue decomposition of transpose(M) x M, so it is efficient if n>>m, as transpose(M) x M is size m x m
//
int linopt_compute_SVDpseudoInverse(char *ID_Rmatrix_name, char *ID_Cmatrix_name, double SVDeps, long MaxNBmodes, char *ID_VTmatrix_name) /* works even for m != n */
{
    FILE *fp;
    char fname[200];
    long ii1, jj1, k, ii, jj;
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
    double egvlim;
    long nbmodesremoved;

    int atype;

	long maxMode, MaxNBmodes1, mode;
	
	// Timing
	int timing = 1; 
	struct timespec t0, t1, t2, t3, t4, t5, t6, t7;
    double t01d, t12d, t23d, t34d, t45d, t56d, t67d;
	struct timespec tdiff;

	int testmode = 0;
	long ID_AtA;
	long ID;




    printf("[CPU (gsl) SVD start]");
    fflush(stdout);

	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t0);
		

    arraysizetmp = (long*) malloc(sizeof(long)*3);


    ID_Rmatrix = image_ID(ID_Rmatrix_name);
    if(ID_Rmatrix==-1)
		{
			printf("ERROR: matrix %s not found in memory\n", ID_Rmatrix_name);
			exit(0);
		}
    atype = data.image[ID_Rmatrix].md[0].atype;
    if(data.image[ID_Rmatrix].md[0].naxis==3)
    {
        n = data.image[ID_Rmatrix].md[0].size[0]*data.image[ID_Rmatrix].md[0].size[1];
        m = data.image[ID_Rmatrix].md[0].size[2];
        printf("3D image -> %ld %ld\n", n, m);
        fflush(stdout);
    }
    else
    {
        n = data.image[ID_Rmatrix].md[0].size[0];
        m = data.image[ID_Rmatrix].md[0].size[1];
         printf("2D image -> %ld %ld\n", n, m);
        fflush(stdout);
   }

    /* in this procedure, m=number of actuators/modes, n=number of WFS elements */
    //  long m = smao[0].NBmode;
    // long n = smao[0].NBwfselem;

     printf("m = %ld , n = %ld \n", m, n);
      fflush(stdout);

    matrix_DtraD_eval = gsl_vector_alloc (m);
    matrix_D = gsl_matrix_alloc (n,m);
    matrix_Ds = gsl_matrix_alloc (m,n);
    matrix_Dtra = gsl_matrix_alloc (m,n);
    matrix_DtraD = gsl_matrix_alloc (m,m);
    matrix_DtraDinv = gsl_matrix_alloc (m,m);
    matrix_DtraD_evec = gsl_matrix_alloc (m,m);



    /* write matrix_D */
    if(atype==FLOAT)
    {
        for(k=0; k<m; k++)
            for(ii=0; ii<n; ii++)
                gsl_matrix_set (matrix_D, ii, k, data.image[ID_Rmatrix].array.F[k*n+ii]);
    }
    else
    {
        for(k=0; k<m; k++)
            for(ii=0; ii<n; ii++)
                gsl_matrix_set (matrix_D, ii, k, data.image[ID_Rmatrix].array.D[k*n+ii]);
    }

	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t1);


    /* compute DtraD */
    gsl_blas_dgemm (CblasTrans, CblasNoTrans, 1.0, matrix_D, matrix_D, 0.0, matrix_DtraD);


	if(testmode==1)
	{
	// TEST
	ID_AtA = create_2Dimage_ID("AtA", m, m);
    for(ii=0; ii<m; ii++) 
    for(jj=0; jj<m; jj++) 
		data.image[ID_AtA].array.F[jj*m+ii] = (float) gsl_matrix_get( matrix_DtraD, ii, jj);
	save_fits("AtA", "!test_AtA.fits");
	}

	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t2);



    /* compute the inverse of DtraD */

    /* first, compute the eigenvalues and eigenvectors */
    w =   gsl_eigen_symmv_alloc (m);
    matrix_save = gsl_matrix_alloc (m,m);
    gsl_matrix_memcpy(matrix_save, matrix_DtraD);
    gsl_eigen_symmv (matrix_save, matrix_DtraD_eval, matrix_DtraD_evec, w);
    gsl_matrix_free(matrix_save);
    gsl_eigen_symmv_free(w);

	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t3);

    gsl_eigen_symmv_sort (matrix_DtraD_eval, matrix_DtraD_evec, GSL_EIGEN_SORT_ABS_DESC);

	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t4);


    //  printf("Eigenvalues\n");
    //  fflush(stdout);

    // Write eigenvalues
    sprintf(fname, "eigenv.dat");
    if((fp=fopen(fname, "w"))==NULL)
      {
        printf("ERROR: cannot create file \"%s\"\n", fname);
        exit(0);
      }
    for(k=0; k<m; k++)
      fprintf(fp,"%ld %g\n", k, sqrt(gsl_vector_get(matrix_DtraD_eval,k)));
    fclose(fp);
    


    //  for(k=0; k<m; k++)
    //    printf("Mode %ld eigenvalue = %g\n", k, gsl_vector_get(matrix_DtraD_eval,k));
    egvlim = SVDeps*SVDeps * gsl_vector_get(matrix_DtraD_eval, 0);
	MaxNBmodes1 = MaxNBmodes;
	if(MaxNBmodes1>m)
		MaxNBmodes1 = m;
	if(MaxNBmodes1>n)
		MaxNBmodes1 = n;
	mode = 0;
	while( (mode<MaxNBmodes1) && (gsl_vector_get(matrix_DtraD_eval, mode)>egvlim) )
		mode++;
	printf("Keeping %ld modes  (SVDeps = %g-> %g, MaxNBmodes = %ld -> %ld)\n", mode, SVDeps, egvlim, MaxNBmodes, MaxNBmodes1);
	MaxNBmodes1 = mode;
		
    // Write rotation matrix 
    arraysizetmp[0] = m;
    arraysizetmp[1] = m;
    if(atype==FLOAT)
    {
        ID_VTmatrix = create_image_ID(ID_VTmatrix_name, 2, arraysizetmp, FLOAT, 0, 0);
        for(ii=0; ii<m; ii++) // modes
            for(k=0; k<m; k++) // modes
                data.image[ID_VTmatrix].array.F[k*m+ii] = (float) gsl_matrix_get( matrix_DtraD_evec, k, ii);
    }
    else
    {
        ID_VTmatrix = create_image_ID(ID_VTmatrix_name, 2, arraysizetmp, DOUBLE, 0, 0);
        for(ii=0; ii<m; ii++) // modes
            for(k=0; k<m; k++) // modes
                data.image[ID_VTmatrix].array.D[k*m+ii] = gsl_matrix_get( matrix_DtraD_evec, k, ii);
    }

	if(testmode==1)
		save_fits(ID_VTmatrix_name, "!test_VT.fits");

    /* second, build the "inverse" of the diagonal matrix of eigenvalues (matrix1) */
    nbmodesremoved = 0;
    matrix1 = gsl_matrix_alloc (m, m);
    for(ii1=0; ii1<m; ii1++) // mode
        for(jj1=0; jj1<m; jj1++)
        {
            if(ii1==jj1)
            {
                if(ii1>MaxNBmodes1-1)
                {
                    gsl_matrix_set(matrix1, ii1, jj1, 0.0);
                    nbmodesremoved ++;
                }
                else
                    gsl_matrix_set(matrix1, ii1, jj1, 1.0/gsl_vector_get(matrix_DtraD_eval,ii1));
            }
            else
                gsl_matrix_set(matrix1, ii1, jj1, 0.0);
        }
    // printf("%ld modes removed\n", nbmodesremoved);
    // printf("Compute inverse\n");
    // fflush(stdout);
	
	
	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t5);
		
		
    /* third, compute the "inverse" of DtraD */
    matrix2 = gsl_matrix_alloc (m, m);
    gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, matrix_DtraD_evec, matrix1, 0.0, matrix2);
    gsl_blas_dgemm (CblasNoTrans, CblasTrans, 1.0, matrix2, matrix_DtraD_evec, 0.0, matrix_DtraDinv);
    gsl_matrix_free(matrix1);
    gsl_matrix_free(matrix2);
    
    if(testmode==1)
    {
		ID = create_2Dimage_ID("M2", m, m);
		for(ii=0;ii<m;ii++)
			for(jj=0;jj<m;jj++)
				data.image[ID].array.F[jj*m+ii] = gsl_matrix_get(matrix_DtraDinv, ii, jj);
		save_fits("M2", "!test_M2.fits");
	}
    
    
    gsl_blas_dgemm (CblasNoTrans, CblasTrans, 1.0, matrix_DtraDinv, matrix_D, 0.0, matrix_Ds);

    if(data.image[ID_Rmatrix].md[0].naxis==3)
    {
        arraysizetmp[0] = data.image[ID_Rmatrix].md[0].size[0];
        arraysizetmp[1] = data.image[ID_Rmatrix].md[0].size[1];
        arraysizetmp[2] = m;
    }
    else
    {
        arraysizetmp[0] = n;
        arraysizetmp[1] = m;
    }

    if(atype==FLOAT)
        ID_Cmatrix = create_image_ID(ID_Cmatrix_name, data.image[ID_Rmatrix].md[0].naxis, arraysizetmp, FLOAT, 0, 0);
    else
        ID_Cmatrix = create_image_ID(ID_Cmatrix_name, data.image[ID_Rmatrix].md[0].naxis, arraysizetmp, DOUBLE, 0, 0);


	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t6);

    /* write result */
    if(atype==FLOAT)
    {
        for(ii=0; ii<n; ii++) // sensors
            for(k=0; k<m; k++) // actuator modes
                data.image[ID_Cmatrix].array.F[k*n+ii] = (float) gsl_matrix_get(matrix_Ds, k, ii);
    }
    else
    {
        for(ii=0; ii<n; ii++) // sensors
            for(k=0; k<m; k++) // actuator modes
                data.image[ID_Cmatrix].array.D[k*n+ii] = gsl_matrix_get(matrix_Ds, k, ii);
    }


	if(testmode==1)
		save_fits(ID_Cmatrix_name, "!test_Ainv.fits");

	if(timing==1)
		clock_gettime(CLOCK_REALTIME, &t7);


    gsl_vector_free(matrix_DtraD_eval);
    gsl_matrix_free(matrix_D);
    gsl_matrix_free(matrix_Ds);
    gsl_matrix_free(matrix_Dtra);
    gsl_matrix_free(matrix_DtraD);
    gsl_matrix_free(matrix_DtraDinv);
    gsl_matrix_free(matrix_DtraD_evec);

    free(arraysizetmp);

    printf("[CPU pseudo-inverse done]\n");
    fflush(stdout);

	if(timing==1)
	{
		tdiff = info_time_diff(t0, t1);
        t01d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		tdiff = info_time_diff(t1, t2);
        t12d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		tdiff = info_time_diff(t2, t3);
        t23d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		tdiff = info_time_diff(t3, t4);
        t34d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		tdiff = info_time_diff(t4, t5);
        t45d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		tdiff = info_time_diff(t5, t6);
        t56d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		tdiff = info_time_diff(t6, t7);
        t67d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

		printf("Timing info: \n");
		printf("  0-1	%12.3f ms\n", t01d*1000.0);
		printf("  1-2	%12.3f ms\n", t12d*1000.0);
		printf("  2-3	%12.3f ms\n", t23d*1000.0);
		printf("  3-4	%12.3f ms\n", t34d*1000.0);
		printf("  4-5	%12.3f ms\n", t45d*1000.0);
		printf("  5-6	%12.3f ms\n", t56d*1000.0);
		printf("  6-7	%12.3f ms\n", t67d*1000.0);
	}


    return(ID_Cmatrix);
}













long linopt_imtools_image_construct(char *IDmodes_name, char *IDcoeff_name, char *ID_name)
{
    long ID;
    long IDmodes;
    long IDcoeff;
    long ii, jj, kk;
    long xsize, ysize, zsize;
    long sizexy;
    int atype;


    IDmodes = image_ID(IDmodes_name);
    atype = data.image[IDmodes].md[0].atype;

    xsize = data.image[IDmodes].md[0].size[0];
    ysize = data.image[IDmodes].md[0].size[1];
    zsize = data.image[IDmodes].md[0].size[2];

    sizexy = xsize*ysize;



    if(atype==FLOAT)
        ID = create_2Dimage_ID(ID_name, xsize, ysize);
    else
        ID = create_2Dimage_ID_double(ID_name, xsize, ysize);

    IDcoeff = image_ID(IDcoeff_name);


    if(atype==FLOAT)
    {
        for(kk=0; kk<zsize; kk++)
            for(ii=0; ii<sizexy; ii++)
                data.image[ID].array.F[ii] += data.image[IDcoeff].array.F[kk] * data.image[IDmodes].array.F[kk*sizexy+ii];
    }
    else
    {
        for(kk=0; kk<zsize; kk++)
            for(ii=0; ii<sizexy; ii++)
                data.image[ID].array.D[ii] += data.image[IDcoeff].array.D[kk] * data.image[IDmodes].array.D[kk*sizexy+ii];
    }


    return(ID);
}




// FLOAT only
long linopt_imtools_image_construct_stream(char *IDmodes_name, char *IDcoeff_name, char *IDout_name)
{
    long IDout;
    long IDmodes;
    long IDcoeff;
    long ii, jj, kk;
    long xsize, ysize, zsize;
    long sizexy;
    int semval;
    int atype;
    long long cnt = 0;
    int RT_priority = 80; //any number from 0-99
    struct sched_param schedpar;
	int NOSEM = 1; // ignore input semaphore, use counter
   
    
    schedpar.sched_priority = RT_priority;
    #ifndef __MACH__
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    #endif
 
  
  
    IDmodes = image_ID(IDmodes_name);
    atype = data.image[IDmodes].md[0].atype;

    xsize = data.image[IDmodes].md[0].size[0];
    ysize = data.image[IDmodes].md[0].size[1];
    zsize = data.image[IDmodes].md[0].size[2];

    sizexy = xsize*ysize;

	if(variable_ID("NOSEM")!=-1)
		NOSEM = 1;
	else
		NOSEM = 0;

    IDout = image_ID(IDout_name);
    IDcoeff = image_ID(IDcoeff_name);

    while(1==1)
    {
        if((data.image[IDcoeff].sem==0)||(NOSEM==1))
        {
            while(cnt==data.image[IDcoeff].md[0].cnt0) // test if new frame exists
                usleep(5);
            cnt = data.image[IDcoeff].md[0].cnt0;
        }
        else
            sem_wait(data.image[IDcoeff].semptr[0]);

         for(ii=0; ii<sizexy; ii++)
            data.image[IDout].array.F[ii] = 0.0;
                
        data.image[IDout].md[0].write = 1;
        for(kk=0; kk<zsize; kk++)
            for(ii=0; ii<sizexy; ii++)
                data.image[IDout].array.F[ii] += data.image[IDcoeff].array.F[kk] * data.image[IDmodes].array.F[kk*sizexy+ii];
        sem_getvalue(data.image[IDout].semptr[0], &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(data.image[IDout].semptr[0]);

        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
    }

    return(IDout);
}





//
// if reuse = 1, do not recompute pixind, pixmul, respm, recm
//
long linopt_imtools_image_fitModes(char *ID_name, char *IDmodes_name, char *IDmask_name, double SVDeps, char *IDcoeff_name, int reuse)
{
    long ID;
    long IDmodes;
    long IDmask;
    long m, n;

    long IDrecm;
    long IDmvec;
    long IDcoeff;
    long ii, jj;

	int use_magma = 0;


    if((reuse==0)&&(fmInit==1))
    {
        delete_image_ID("_fm_pixind");
        delete_image_ID("_fm_pixmul");
        delete_image_ID("_fm_respm");
        delete_image_ID("_fm_recm");
        delete_image_ID("_fm_vtmat");
    }


    if((reuse==0)||(fmInit==0))
    {
        linopt_imtools_mask_to_pixtable(IDmask_name, "_fm_pixind", "_fm_pixmul");
        linopt_imtools_Image_to_vec(IDmodes_name, "_fm_pixind", "_fm_pixmul", "_fm_respm");
   

	#ifdef HAVE_MAGMA
		CUDACOMP_magma_compute_SVDpseudoInverse("_fm_respm", "_fm_recm", SVDeps, 10000, "_fm_vtmat", 0);
	#else
        linopt_compute_SVDpseudoInverse("_fm_respm", "_fm_recm", SVDeps, 10000, "_fm_vtmat");
   #endif
    }

    
    linopt_imtools_Image_to_vec(ID_name, "_fm_pixind", "_fm_pixmul", "_fm_measvec");
    

    IDmvec = image_ID("_fm_measvec");
    IDrecm = image_ID("_fm_recm");
    m = data.image[IDrecm].md[0].size[1];
    n = data.image[IDrecm].md[0].size[0];
    // printf("m=%ld n=%ld\n", m, n);
    // m = number modes
    // n = number WFS elem

    IDcoeff = create_2Dimage_ID(IDcoeff_name, m, 1);
    cblas_sgemv (CblasRowMajor, CblasNoTrans, m, n, 1.0,  data.image[IDrecm].array.F, n, data.image[IDmvec].array.F, 1, 0.0, data.image[IDcoeff].array.F, 1);

    // for(ii=0;ii<m;ii++)
    //   printf("  coeff %03ld  =  %g\n", ii, data.image[IDcoeff].array.F[ii]);


    delete_image_ID("_fm_measvec");


    if(0) // testing
    {
        printf("========  %s  %s  %s  %lf  %s  %d  ====\n", ID_name, IDmodes_name, IDmask_name, SVDeps, IDcoeff_name, reuse);
        list_image_ID();
        save_fits("_fm_respm", "!fm_respm.fits");
        linopt_imtools_image_construct(IDmodes_name, IDcoeff_name, "testsol");
        save_fits("testsol", "!testsol.fits");
        arith_image_sub(ID_name,"testsol","fitres");
        save_fits("fitres", "!fitres.fits");
        arith_image_mult("fitres", IDmask_name, "fitresm");
        save_fits("fitresm", "!fitresm.fits");
        exit(0);
    }

    fmInit = 1;

    return(IDcoeff);
}



















/* --------------------------------------------------------------- */
/*                                                                 */
/*           Functions for optimization                            */
/*                                                                 */
/* --------------------------------------------------------------- */

double linopt_imtools_opt_f (const gsl_vector *v, void *params)
{
    double value;
    long k,l,n;


    n = NBPARAM;
    value = C0;
    for(k=0; k<n; k++)
        value += polycoeff1[k]*gsl_vector_get(v,k);
    for(k=0; k<n; k++)
        for(l=0; l<n; l++)
            value += polycoeff2[l*n+k] * gsl_vector_get(v,k) * gsl_vector_get(v,l);

    return(value);
}

void linopt_imtools_opt_df (const gsl_vector *v, void *params, gsl_vector *df)
{
    double epsilon = 1.0e-8;
    long i,j;
    double v0,v1,v2;
    gsl_vector *vcp;

    vcp = gsl_vector_alloc (NBPARAM);
    v0 = linopt_imtools_opt_f (v, params);

    for(i=0; i<NBPARAM; i++)
    {
        for(j=0; j<NBPARAM; j++)
            gsl_vector_set(vcp, j, gsl_vector_get(v,j));
        gsl_vector_set(vcp, i, gsl_vector_get(v,i)+epsilon);
        v1 = linopt_imtools_opt_f (vcp, params);
        gsl_vector_set(vcp, i, gsl_vector_get(v,i)-epsilon);
        v2 = linopt_imtools_opt_f (vcp, params);
        gsl_vector_set(df, i, (double) ((v1-v2)/(2.0*epsilon)));
    }

    if(0)
    {
        printf("%ld df = (",dfcnt);
        for(i=0; i<NBPARAM; i++)
            printf(" %g",gsl_vector_get(df,i));
        printf(" )\n");
    }
    dfcnt ++;

    if(dfcnt > 50)
        exit(0);

    gsl_vector_free (vcp);
}

void linopt_imtools_opt_fdf (const gsl_vector *x, void *params, double *f, gsl_vector *df)
{
    *f = linopt_imtools_opt_f(x, params);
    linopt_imtools_opt_df(x, params, df);
}







//
// match a single image (ID_name) to a linear sum of images within IDref_name
// result is a 1D array of coefficients in IDsol_name
//
double linopt_imtools_match_slow(char *ID_name, char *IDref_name, char *IDmask_name, char *IDsol_name, char *IDout_name)
{
    long ID, IDref, IDmask, IDsol, IDout;
    long naxes[2];
    long n; // number of reference frames
    long ii,k,l;

    long double val;
    long double valbest;

    // initial random search
    long riter;
    long riterMax = 1000000;

    long double v0;
    long double *tarray = NULL; // temporary array to store values for fixed pixel


    // ref image coefficients (solutions)
    long double *alpha = NULL;
    long double *alphabest = NULL;
    long double ampl;

    /*
      the optimization problem is first rewritten as a 2nd degree polynomial of alpha values
      val = V0 + SUM_{k=0...n-1}{polycoeff1[k]*alpha[k] + SUM_{k=0...n-1}{l=0...k}{polycoeff2[k,l]*alpha[k]*alpha[l]}
     */

    long iter = 0;
    double *params;
    const gsl_multimin_fdfminimizer_type *T;
    gsl_multimin_fdfminimizer *sminimizer;
    long i;
    gsl_vector *x;
    gsl_multimin_function_fdf opt_func;
    int status;

    //  printf("Input params : %s %s %s\n",ID_name,IDref_name,IDsol_name);

	params = (double*) malloc(sizeof(double)*1);
	params[0] = 0.0;


    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    IDmask = image_ID(IDmask_name);
    IDref = image_ID(IDref_name);
    n = data.image[IDref].md[0].size[2];

    printf("Number of points = %ld x %ld\n",naxes[0]*naxes[1],n);


    alpha = (long double*) malloc(sizeof(long double)*n);
    if(alpha==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"Cannot allocate memory");
        exit(0);
    }
    alphabest = (long double*) malloc(sizeof(long double)*n);
    if(alphabest==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"Cannot allocate memory");
        exit(0);
    }



    polycoeff1 = (long double*) malloc(sizeof(long double)*n);
    if(polycoeff1==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"Cannot allocate memory");
        exit(0);
    }
    polycoeff2 = (long double*) malloc(sizeof(long double)*n*n);
    if(polycoeff2==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"Cannot allocate memory");
        exit(0);
    }

    tarray = (long double*) malloc(sizeof(long double)*n);
    if(tarray==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"Cannot allocate memory");
        exit(0);
    }



    // initialize all coeffs to zero
    C0 = 0.0;
    for(k=0; k<n; k++)
    {
        alpha[k] = 1.0/n;
        polycoeff1[k] = 0.0;
        for(l=0; l<n; l++)
            polycoeff2[l*n+k] = 0.0;
    }

    // compute polynomial coefficients
    for(ii=0; ii<naxes[0]*naxes[1]; ii++)
    {
        v0 = (long double) (data.image[ID].array.F[ii]*data.image[IDmask].array.F[ii]);
        for(k=0; k<n; k++)
            tarray[k] = (long double) (data.image[IDref].array.F[naxes[0]*naxes[1]*k+ii]*data.image[IDmask].array.F[ii]);
        C0 += v0*v0;
        for(k=0; k<n; k++)
            polycoeff1[k] += -2.0*v0*tarray[k];
        for(k=0; k<n; k++)
            for(l=0; l<n; l++)
                polycoeff2[l*n+k] += tarray[k]*tarray[l];
    }

    // find solution
    /*   val = C0 + SUM_{k=0...n-1}{polycoeff1[k]*alpha[k] + SUM_{k=0...n-1}{l=0...k}{polycoeff2[k,l]*alpha[k]*alpha[l]}
     */
    val = C0;
    for(k=0; k<n; k++)
        val += polycoeff1[k]*alpha[k];
    for(k=0; k<n; k++)
        for(l=0; l<n; l++)
            val += polycoeff2[l*n+k]*alpha[k]*alpha[l];


    for(k=0; k<n; k++)
        printf("%g ", (double) alpha[k]);
    printf("-> %g\n", (double) val);
    for(k=0; k<n; k++)
        alphabest[k] = alpha[k];
    valbest = val;





    for(riter=0; riter<riterMax; riter++)
    {
        ampl = pow(ran1(),4.0);
        for(k=0; k<n; k++)
            alpha[k] = alphabest[k] + ampl*(1.0-2.0*ran1())/n;

        val = C0;
        for(k=0; k<n; k++)
            val += polycoeff1[k]*alpha[k];
        for(k=0; k<n; k++)
            for(l=0; l<n; l++)
                val += polycoeff2[l*n+k]*alpha[k]*alpha[l];
        if(val<valbest)
        {
            //printf("[%ld/%ld] ",riter,riterMax);
            //for(k=0;k<n;k++)
            //  printf(" %g ", (double) alpha[k]);
            //printf("-> %g\n", (double) val);
            for(k=0; k<n; k++)
                alphabest[k] = alpha[k];
            valbest = val;
        }
    }

    NBPARAM = n;

    x = gsl_vector_alloc (n);

    for(i=0; i<n; i++)
        gsl_vector_set(x, i, alphabest[i]);
    printf("Value = %g\n", linopt_imtools_opt_f (x, params));



    opt_func.n = n;
    opt_func.f = &linopt_imtools_opt_f;
    opt_func.df = &linopt_imtools_opt_df;
    opt_func.fdf = &linopt_imtools_opt_fdf;
    opt_func.params = &params;

    x = gsl_vector_alloc (n);

    for(i=0; i<n; i++)
        gsl_vector_set(x, i, alphabest[i]);

    T = gsl_multimin_fdfminimizer_vector_bfgs2;
    sminimizer = gsl_multimin_fdfminimizer_alloc (T, n);

    gsl_multimin_fdfminimizer_set (sminimizer, &opt_func, x, 1.0e-5, 0.1);

    do
    {
        iter++;
        dfcnt = 0;
        status = gsl_multimin_fdfminimizer_iterate (sminimizer);
        if (status)
            break;
        status = gsl_multimin_test_gradient (sminimizer->gradient, 1e-5);
        if (status == GSL_SUCCESS)
        {
            printf ("Minimum found at:\n");
            printf ("%5ld : ", iter);
            //for(i=0;i<n;i++)
            // printf("%.8f ",gsl_vector_get(sminimizer->x, i));
            printf ("    %10.8f\n", sminimizer->f);
        }
    }
    while (status == GSL_CONTINUE && iter < 1000);

    for(i=0; i<n; i++)
        alphabest[i] = gsl_vector_get(sminimizer->x, i);

    for(i=0; i<n; i++)
        gsl_vector_set(x, i, alphabest[i]);
    printf("Value after minimization = %g\n", linopt_imtools_opt_f (x, params));

    gsl_multimin_fdfminimizer_free (sminimizer);
    gsl_vector_free (x);


    IDsol = create_2Dimage_ID(IDsol_name,n,1);
    for(i=0; i<n; i++)
        data.image[IDsol].array.F[i] = alphabest[i];



    // compute residual

    IDout = create_2Dimage_ID(IDout_name, naxes[0], naxes[1]);

    for(ii=0; ii<naxes[0]*naxes[1]; ii++)
        data.image[IDout].array.F[ii] = 0.0;
    for (k=0; k<n; k++)
        for(ii=0; ii<naxes[0]*naxes[1]; ii++)
            data.image[IDout].array.F[ii] += alphabest[k]*data.image[IDref].array.F[naxes[0]*naxes[1]*k+ii];


    free(alpha);
    alpha = NULL;
    free(alphabest);
    alphabest = NULL;
    free(polycoeff1);
    polycoeff1 = NULL;
    free(polycoeff2);
    polycoeff2 = NULL;
    free(tarray);
    tarray = NULL;
    
    free (params);

    return((double) val);
}





// match a single image (ID_name) to a linear sum of images within IDref_name
// result is a 1D array of coefficients in IDsol_name
//
// n = number of observations
// p = number of variables
//
// ID_name is input, size (n,1)
// IDsol_name must contain initial solution
//

double linopt_imtools_match(char *ID_name, char *IDref_name, char *IDmask_name, char *IDsol_name, char *IDout_name)
{
    gsl_multifit_linear_workspace *work;
    size_t n, p;
    long ID, IDref, IDmask, IDsol, IDout;
    long naxes[3];
    long i, j, k, ii;
    gsl_matrix *X;
    gsl_vector *y; // measurements
    gsl_vector *c;
    gsl_vector *w;
    gsl_matrix *cov;
    double chisq;

    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];
    n = naxes[0]*naxes[1];
    IDmask = image_ID(IDmask_name);
    IDref = image_ID(IDref_name);
    p = data.image[IDref].md[0].size[2];

    // some verification
    if(IDref==-1)
        printERROR(__FILE__,__func__,__LINE__,"input ref missing\n");
    if(IDmask==-1)
        printERROR(__FILE__,__func__,__LINE__,"input mask missing\n");
    if(data.image[IDmask].md[0].size[0] != data.image[ID].md[0].size[0])
        printERROR(__FILE__,__func__,__LINE__,"mask size[0] is wrong\n");
    if(data.image[IDmask].md[0].size[1] != data.image[ID].md[0].size[1])
        printERROR(__FILE__,__func__,__LINE__,"mask size[1] is wrong\n");


    printf("n,p = %ld %ld\n", (long) n, (long) p);
    fflush(stdout);

    y = gsl_vector_alloc (n); // measurements
    for(i=0; i<n; i++)
        gsl_vector_set(y, i, data.image[ID].array.F[i]);

    w = gsl_vector_alloc (n);
    for(i=0; i<n; i++)
        gsl_vector_set(w, i, data.image[IDmask].array.F[i]);

    X = gsl_matrix_alloc (n, p);
    for(i=0; i<n; i++)
        for(j=0; j<p; j++)
            gsl_matrix_set(X, i, j, data.image[IDref].array.F[j*n+i]);
    c = gsl_vector_alloc (p); // solution (coefficients)
    cov = gsl_matrix_alloc (p, p);

    work = gsl_multifit_linear_alloc (n, p);

    printf("-");
    fflush(stdout);
    gsl_multifit_wlinear (X, w, y, c, cov, &chisq, work);
    printf("-");
    fflush(stdout);

    IDsol = create_2Dimage_ID(IDsol_name, p, 1);
    for(i=0; i<p; i++)
        data.image[IDsol].array.F[i] = gsl_vector_get(c, i);

    gsl_multifit_linear_free (work);
    gsl_vector_free (y);
    gsl_vector_free (w);
    gsl_matrix_free (X);
    gsl_vector_free (c);
    gsl_matrix_free (cov);

    printf(" . ");
    fflush(stdout);

    // compute residual
    IDout = create_2Dimage_ID(IDout_name, naxes[0], naxes[1]);
    for(ii=0; ii<naxes[0]*naxes[1]; ii++)
        data.image[IDout].array.F[ii] = 0.0;
    for (k=0; k<p; k++)
        for(ii=0; ii<naxes[0]*naxes[1]; ii++)
            data.image[IDout].array.F[ii] += data.image[IDsol].array.F[k]*data.image[IDref].array.F[naxes[0]*naxes[1]*k+ii];

    return(chisq);
}


