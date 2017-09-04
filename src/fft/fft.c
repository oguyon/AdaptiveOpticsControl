/**
 * @file    fft.c
 * @brief   Fourier Transform
 * 
 * Wrapper to fftw
 *  
 * @author  O. Guyon
 * @date    7 Jul 2017
 *
 * 
 * @bug No known bugs.
 * 
 */



#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


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






#include <fftw3.h>



//#ifdef _OPENMP
# ifdef HAVE_LIBGOMP
#include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
#endif

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "COREMOD_tools/COREMOD_tools.h"

#include "fft/fft.h"


#define PI 3.14159265358979323846264338328

#define SWAP(x,y)  tmp=(x);x=(y);y=tmp;
#define CSWAP(x,y)  tmp=(x.re);x.re=(y.re);y.re=tmp;tmp=(x.im);x.im=(y.im);y.im=tmp;

#define SBUFFERSIZE 1000

#define FFTWOPTMODE FFTW_ESTIMATE
//#define FFTWOPTMODE FFTW_MEASURE
//#define FFTWOPTMODE FFTW_PATIENT
//#define FFTWOPTMODE FFTW_EXHAUSTIVE


//#define FFTWMT 1
static int NB_FFTW_THREADS = 2;

extern DATA data;




// Forward references

/*
int init_fftw_plans0 ( );
int fft_setNthreads(int nt);
int import_wisdom();
int export_wisdom();
int test_fftspeed(int nmax);
long fft_correlation(char *ID_name1, char *ID_name2, char *ID_nameout);
*/


// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//


int_fast8_t fft_permut_cli()
{
    if(CLI_checkarg(1,4)==0)
        permut(data.cmdargtoken[1].val.string);

    return 0;
}


//int do2dfft(char *in_name, char *out_name);

int_fast8_t fft_do1dfft_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)==0)
    {
        do1dfft(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
        return 0;
    }
    else
        return 1;
}

int_fast8_t fft_do1drfft_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)==0)
    {
        do1drfft(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
        return 0;
    }
    else
        return 1;
}


int_fast8_t fft_do2dfft_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)==0)
    {
        do2dfft(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
        return 0;
    }
    else
        return 1;
}




int_fast8_t test_fftspeed_cli()
{
    if(CLI_checkarg(1,2)==0)
        test_fftspeed((int) data.cmdargtoken[1].val.numl);

    return 0;
}


int_fast8_t fft_image_translate_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,1)+CLI_checkarg(4,1)==0)
        fft_image_translate(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf);
    else
        return 1;

    return 0;
}



int_fast8_t fft_correlation_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,3)==0)
        fft_correlation(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
    else
        return 1;
}








int_fast8_t init_fft()
{

# ifdef FFTWMT
    printf("Multi-threaded fft enabled, max threads = %d\n",omp_get_max_threads());
    fftwf_init_threads();
    fftwf_plan_with_nthreads(omp_get_max_threads());
# endif


    // FFTW init

    // load fftw wisdom
    import_wisdom();


    fftwf_set_timelimit(1000.0);
    fftw_set_timelimit(1000.0);


    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "FFTW wrapper");
    data.NBmodule++;



    strcpy(data.cmd[data.NBcmd].key,"initfft");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = init_fftw_plans0;
    strcpy(data.cmd[data.NBcmd].info,"init FFTW");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"initfft");
    strcpy(data.cmd[data.NBcmd].Ccall,"int init_fftw_plans0()");
    data.NBcmd++;
    
    
    strcpy(data.cmd[data.NBcmd].key,"dofft");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = fft_do2dfft_cli;
    strcpy(data.cmd[data.NBcmd].info,"perform FFT");
    strcpy(data.cmd[data.NBcmd].syntax,"<input> <output>");
    strcpy(data.cmd[data.NBcmd].example,"fofft in out");
    strcpy(data.cmd[data.NBcmd].Ccall,"int do2dfft(const char *in_name, const char *out_name)");
    data.NBcmd++;

    
    strcpy(data.cmd[data.NBcmd].key,"do1Dfft");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = fft_do1dfft_cli;
    strcpy(data.cmd[data.NBcmd].info,"perform 1D complex->complex FFT");
    strcpy(data.cmd[data.NBcmd].syntax,"<input> <output>");
    strcpy(data.cmd[data.NBcmd].example,"do1dfft in out");
    strcpy(data.cmd[data.NBcmd].Ccall,"int do1dfft(const char *in_name, const char *out_name)");
    data.NBcmd++;
     
       
    strcpy(data.cmd[data.NBcmd].key,"do1Drfft");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = fft_do1drfft_cli;
    strcpy(data.cmd[data.NBcmd].info,"perform 1D real->complex FFT");
    strcpy(data.cmd[data.NBcmd].syntax,"<input> <output>");
    strcpy(data.cmd[data.NBcmd].example,"do1drfft in out");
    strcpy(data.cmd[data.NBcmd].Ccall,"int do1drfft(const char *in_name, const char *out_name)");
    data.NBcmd++;
     

    strcpy(data.cmd[data.NBcmd].key,"permut");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = fft_permut_cli;
    strcpy(data.cmd[data.NBcmd].info,"permut image quadrants");
    strcpy(data.cmd[data.NBcmd].syntax,"<image>");
    strcpy(data.cmd[data.NBcmd].example,"permut im1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int permut(const char *ID_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"testfftspeed");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = test_fftspeed_cli;
    strcpy(data.cmd[data.NBcmd].info,"test FFTW speed");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"testfftspeed");
    strcpy(data.cmd[data.NBcmd].Ccall,"int test_fftwspeed(int nmax)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"transl");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = fft_image_translate_cli;
    strcpy(data.cmd[data.NBcmd].info,"translate image");
    strcpy(data.cmd[data.NBcmd].syntax,"<imagein> <imageout> <xtransl> <ytransl>");
    strcpy(data.cmd[data.NBcmd].example,"transl im1 im2 2.3 -2.1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int fft_image_translate(const char *ID_name, const char *ID_out, double xtransl, double ytransl)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"fcorrel");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = fft_correlation_cli;
    strcpy(data.cmd[data.NBcmd].info,"correlate two images");
    strcpy(data.cmd[data.NBcmd].syntax,"<imagein1> <imagein2> <correlout>");
    strcpy(data.cmd[data.NBcmd].example,"fcorrel im1 im2 outim");
    strcpy(data.cmd[data.NBcmd].Ccall,"long fft_correlation(const char *ID_name1, const char *ID_name2, const char *ID_nameout)");
    data.NBcmd++;

    return 0;
}




int free_fft()
{
# ifdef FFTWMT
    fftwf_cleanup_threads();
# endif

# ifndef FFTWMT
    fftwf_cleanup();
# endif

    return 0;
}











int fft_setNthreads(int nt)
{
 //   printf("set number of thread to %d (FFTWMT)\n",nt);
# ifdef FFTWMT
    fftwf_cleanup_threads();
    fftwf_cleanup();

    //  printf("Multi-threaded fft enabled, max threads = %d\n",nt);
    fftwf_init_threads();
    fftwf_plan_with_nthreads(nt);
# endif


    import_wisdom();

    return(0);
}

int import_wisdom()
{
    FILE *fp;
    char wisdom_file_single[SBUFFERSIZE];
    char wisdom_file_double[SBUFFERSIZE];
    char warnmessg[SBUFFERSIZE];
    int n;



# ifdef FFTWMT
    n = snprintf(wisdom_file_single, SBUFFERSIZE, "%s/fftwf_mt_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    n = snprintf(wisdom_file_double, SBUFFERSIZE, "%s/fftw_mt_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
# endif

# ifndef FFTWMT
    n = snprintf(wisdom_file_single, SBUFFERSIZE, "%s/fftwf_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    n = snprintf(wisdom_file_double, SBUFFERSIZE, "%s/fftw_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
# endif


    if((fp = fopen(wisdom_file_single,"r"))==NULL)
    {
        n = snprintf(warnmessg,SBUFFERSIZE,"No single precision wisdom file in %s\n FFTs will not be optimized, and may run slower than if a wisdom file is used\n type \"initfft\" to create the wisdom file (this will take time)", wisdom_file_single);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printWARNING(__FILE__,__func__,__LINE__,warnmessg);
    } else  {
        if (fftwf_import_wisdom_from_file(fp)==0)
            printERROR(__FILE__,__func__,__LINE__,"Error reading wisdom");
        fclose(fp);
    }


    if((fp = fopen(wisdom_file_double,"r"))==NULL)
    {
        n = snprintf(warnmessg,SBUFFERSIZE,"No double precision wisdom file in %s\n FFTs will not be optimized, and may run slower than if a wisdom file is used\n type \"initfft\" to create the wisdom file (this will take time)", wisdom_file_double);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printWARNING(__FILE__,__func__,__LINE__,warnmessg);
    } else  {
        if (fftw_import_wisdom_from_file(fp)==0)
            printERROR(__FILE__,__func__,__LINE__,"Error reading wisdom");
        fclose(fp);
    }


    return(0);
}



int export_wisdom()
{
    FILE *fp;
    char wisdom_file_single[SBUFFERSIZE];
    char wisdom_file_double[SBUFFERSIZE];
    char errmessg[SBUFFERSIZE];
    int n;
    char command[200];
    int ret;
    
    sprintf(command, "mkdir -p %s", FFTCONFIGDIR);
    ret = system(command);
    
# ifdef FFTWMT
    n = snprintf(wisdom_file_single, SBUFFERSIZE, "%s/fftwf_mt_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    n = snprintf(wisdom_file_double, SBUFFERSIZE, "%s/fftw_mt_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
# endif

# ifndef FFTWMT
    n = snprintf(wisdom_file_single, SBUFFERSIZE, "%s/fftwf_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    n = snprintf(wisdom_file_double, SBUFFERSIZE, "%s/fftw_wisdom.dat", FFTCONFIGDIR);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
# endif


    if((fp = fopen(wisdom_file_single, "w"))==NULL)
    {
        n = snprintf(errmessg, SBUFFERSIZE, "Error creating wisdom file \"%s\"", wisdom_file_single);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errmessg);
        exit(0);
    }
    fftwf_export_wisdom_to_file(fp);
    fclose(fp);

    if((fp = fopen(wisdom_file_double, "w"))==NULL)
    {
        n = snprintf(errmessg, SBUFFERSIZE, "Error creating wisdom file \"%s\"", wisdom_file_double);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errmessg);
        exit(0);
    }
    fftw_export_wisdom_to_file(fp);
    fclose(fp);





    return(0);
}


/*^-----------------------------------------------------------------------------
|
|
|
|
|
|
|
+-----------------------------------------------------------------------------*/
int_fast8_t init_fftw_plans(int mode)
{
    int n;
    int size;

    fftwf_complex *inf = NULL;
    fftwf_complex *outf = NULL;
    float *rinf = NULL;

    fftw_complex *ind = NULL;
    fftw_complex *outd = NULL;
    double *rind = NULL;

    unsigned int plan_mode;


    printf("Optimization of FFTW\n");
    printf("The optimization is done for 2D complex to complex FFTs, with size equal to 2^n x 2^n\n");
    printf("You can kill the optimization anytime, and resume later where it previously stopped.\nAfter each size is optimized, the result is saved\n");
    printf("It might be a good idea to run this overnight or when your computer is not busy\n");

    fflush(stdout);

    size = 1;

    //  plan_mode = FFTWOPTMODE;
    plan_mode = FFTW_EXHAUSTIVE;

    for(n=0; n<14; n++)
    {
        if(mode==0)
        {
            printf("Optimizing 2D FFTs - size = %d\n",size);
            fflush(stdout);
        }
        rinf = (float*) fftwf_malloc(size*size*sizeof(float));
        inf = (fftwf_complex*) fftwf_malloc(size*size*sizeof(fftwf_complex));
        outf = (fftwf_complex*) fftwf_malloc(size*size*sizeof(fftwf_complex));

        fftwf_plan_dft_2d(size, size, inf, outf, FFTW_FORWARD, plan_mode);
        fftwf_plan_dft_2d(size, size, inf, outf, FFTW_BACKWARD, plan_mode);
        fftwf_plan_dft_r2c_2d(size, size, rinf, outf, plan_mode);

        fftwf_free(inf);
        fftwf_free(rinf);
        fftwf_free(outf);


        rind = (double*) fftw_malloc(size*size*sizeof(double));
        ind = (fftw_complex*) fftw_malloc(size*size*sizeof(fftw_complex));
        outd = (fftw_complex*) fftw_malloc(size*size*sizeof(fftw_complex));

        fftw_plan_dft_2d(size, size, ind, outd, FFTW_FORWARD, plan_mode);
        fftw_plan_dft_2d(size, size, ind, outd, FFTW_BACKWARD, plan_mode);
        fftw_plan_dft_r2c_2d(size, size, rind, outd, plan_mode);

        fftw_free(ind);
        fftw_free(rind);
        fftw_free(outd);


        size*=2;
        if(mode==0)
            export_wisdom();
    }
    size = 1;
    for(n=0; n<15; n++)
    {
        if(mode==0)
        {
            printf("Optimizing 1D FFTs - size = %d\n",size);
            fflush(stdout);
        }
        rinf = (float*) fftwf_malloc(size*sizeof(float));
        inf = (fftwf_complex*) fftwf_malloc(size*sizeof(fftwf_complex));
        outf = (fftwf_complex*) fftwf_malloc(size*sizeof(fftwf_complex));

        fftwf_plan_dft_1d(size, inf, outf, FFTW_FORWARD, plan_mode);
        fftwf_plan_dft_1d(size, inf, outf, FFTW_BACKWARD, plan_mode);
        fftwf_plan_dft_r2c_1d(size, rinf, outf, plan_mode);

        fftwf_free(inf);
        fftwf_free(rinf);
        fftwf_free(outf);



        rind = (double*) fftw_malloc(size*sizeof(double));
        ind = (fftw_complex*) fftw_malloc(size*sizeof(fftw_complex));
        outd = (fftw_complex*) fftw_malloc(size*sizeof(fftw_complex));

        fftw_plan_dft_1d(size, ind, outd, FFTW_FORWARD, plan_mode);
        fftw_plan_dft_1d(size, ind, outd, FFTW_BACKWARD, plan_mode);
        fftw_plan_dft_r2c_1d(size, rind, outd, plan_mode);

        fftw_free(ind);
        fftw_free(rind);
        fftw_free(outd);


        size*=2;
        if(mode==0)
            export_wisdom();
    }


    export_wisdom();

    return(0);
}




int_fast8_t init_fftw_plans0()
{
    init_fftw_plans(0);

    return(0);
}




int permut(const char *ID_name)
{
    double tmp;
    long naxes0, naxes1, naxes2;
    int ID;
    long xhalf,yhalf;
    long ii,jj,kk;
    long i;
    long naxis;
    int atype;
    int OK=0;

    //  printf("permut image %s ...", ID_name);
    // fflush(stdout);

    ID = image_ID(ID_name);
    naxis = data.image[ID].md[0].naxis;

    naxes0 = data.image[ID].md[0].size[0];
    if(naxis>1)
        naxes1 = data.image[ID].md[0].size[1];
    if(naxis>2)
        naxes2 = data.image[ID].md[0].size[2];
    else
        naxes2 = 1;

    //  printf(" [%ld %ld %ld] ", naxes0, naxes1, naxes2);


    atype = data.image[ID].md[0].atype;

    tmp=0;

    if(atype==_DATATYPE_FLOAT)
    {
        if(naxis==1)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            for (ii = 0; ii < xhalf; ii++)
                SWAP(data.image[ID].array.F[ii],data.image[ID].array.F[ii+xhalf])
            }
        if(naxis==2)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for (jj = 0; jj < yhalf; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    SWAP(data.image[ID].array.F[jj*naxes0+ii],data.image[ID].array.F[(jj+yhalf)*naxes0+(ii+xhalf)])
                }
            for (jj = yhalf; jj < naxes1; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    SWAP(data.image[ID].array.F[jj*naxes0+ii],data.image[ID].array.F[(jj-yhalf)*naxes0+(ii+xhalf)])
                }
        }
        if(naxis==3)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for (jj = 0; jj < yhalf; jj++)
                for (ii = 0; ii < xhalf; ii++)
                {
                    for(kk=0; kk<naxes2; kk++)
                        SWAP(data.image[ID].array.F[kk*naxes0*naxes1+jj*naxes0+ii],data.image[ID].array.F[kk*naxes0*naxes1+(jj+yhalf)*naxes0+(ii+xhalf)])
                    }
            for (jj = yhalf; jj < naxes1; jj++)
                for (ii = 0; ii < xhalf; ii++)
                {
                    for(kk=0; kk<naxes2; kk++)
                        SWAP(data.image[ID].array.F[kk*naxes0*naxes1+jj*naxes0+ii],data.image[ID].array.F[kk*naxes0*naxes1+(jj-yhalf)*naxes0+(ii+xhalf)])
                    }
        }
    }

    if(atype==_DATATYPE_DOUBLE)
    {
        if(naxis==1)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            for (ii = 0; ii < xhalf; ii++)
                SWAP(data.image[ID].array.D[ii],data.image[ID].array.D[ii+xhalf])
            }
        if(naxis==2)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for (jj = 0; jj < yhalf; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    SWAP(data.image[ID].array.D[jj*naxes0+ii],data.image[ID].array.D[(jj+yhalf)*naxes0+(ii+xhalf)])
                }
            for (jj = yhalf; jj < naxes1; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    SWAP(data.image[ID].array.D[jj*naxes0+ii],data.image[ID].array.D[(jj-yhalf)*naxes0+(ii+xhalf)])
                }
        }
        if(naxis==3)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for (jj = 0; jj < yhalf; jj++)
                for (ii = 0; ii < xhalf; ii++)
                {
                    for(kk=0; kk<naxes2; kk++)
                        SWAP(data.image[ID].array.D[kk*naxes0*naxes1+jj*naxes0+ii],data.image[ID].array.D[kk*naxes0*naxes1+(jj+yhalf)*naxes0+(ii+xhalf)])
                    }
            for (jj = yhalf; jj < naxes1; jj++)
                for (ii = 0; ii < xhalf; ii++)
                {
                    for(kk=0; kk<naxes2; kk++)
                        SWAP(data.image[ID].array.D[kk*naxes0*naxes1+jj*naxes0+ii],data.image[ID].array.D[kk*naxes0*naxes1+(jj-yhalf)*naxes0+(ii+xhalf)])
                    }
        }
    }

    if(atype==_DATATYPE_COMPLEX_FLOAT)
    {
        if(naxis==1)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            for (ii = 0; ii < xhalf; ii++)
                CSWAP(data.image[ID].array.CF[ii], data.image[ID].array.CF[ii+xhalf])
            }
        if(naxis==2)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for (jj = 0; jj < yhalf; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    CSWAP(data.image[ID].array.CF[jj*naxes0+ii], data.image[ID].array.CF[(jj+yhalf)*naxes0+(ii+xhalf)])
                }
            for (jj = yhalf; jj < naxes1; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    CSWAP(data.image[ID].array.CF[jj*naxes0+ii], data.image[ID].array.CF[(jj-yhalf)*naxes0+(ii+xhalf)])
                }
        }
        if(naxis==3)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for(kk=0; kk<naxes2; kk++)
                for (jj = 0; jj < yhalf; jj++)
                    for (ii = 0; ii < xhalf; ii++)
                    {
                        CSWAP(data.image[ID].array.CF[kk*naxes0*naxes1+jj*naxes0+ii],data.image[ID].array.CF[kk*naxes0*naxes1+(jj+yhalf)*naxes0+(ii+xhalf)])
                    }
            printf(" - ");
            fflush(stdout);

            for(kk=0; kk<naxes2; kk++)
                for (jj = yhalf; jj < naxes1; jj++)
                    for (ii = 0; ii < xhalf; ii++)
                    {
                        CSWAP(data.image[ID].array.CF[kk*naxes0*naxes1+jj*naxes0+ii],data.image[ID].array.CF[kk*naxes0*naxes1+(jj-yhalf)*naxes0+(ii+xhalf)])
                    }
        }
    }

   if(atype==_DATATYPE_COMPLEX_DOUBLE)
    {
        if(naxis==1)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            for (ii = 0; ii < xhalf; ii++)
                CSWAP(data.image[ID].array.CD[ii], data.image[ID].array.CD[ii+xhalf])
            }
        if(naxis==2)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for (jj = 0; jj < yhalf; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    CSWAP(data.image[ID].array.CD[jj*naxes0+ii], data.image[ID].array.CD[(jj+yhalf)*naxes0+(ii+xhalf)])
                }
            for (jj = yhalf; jj < naxes1; jj++)
                for (ii = 0; ii < xhalf; ii++) {
                    CSWAP(data.image[ID].array.CD[jj*naxes0+ii], data.image[ID].array.CD[(jj-yhalf)*naxes0+(ii+xhalf)])
                }
        }
        if(naxis==3)
        {
            OK=1;
            xhalf = (long) (naxes0/2);
            yhalf = (long) (naxes1/2);
            for(kk=0; kk<naxes2; kk++)
                for (jj = 0; jj < yhalf; jj++)
                    for (ii = 0; ii < xhalf; ii++)
                    {
                        CSWAP(data.image[ID].array.CD[kk*naxes0*naxes1+jj*naxes0+ii], data.image[ID].array.CD[kk*naxes0*naxes1+(jj+yhalf)*naxes0+(ii+xhalf)])
                    }
            printf(" - ");
            fflush(stdout);

            for(kk=0; kk<naxes2; kk++)
                for (jj = yhalf; jj < naxes1; jj++)
                    for (ii = 0; ii < xhalf; ii++)
                    {
                        CSWAP(data.image[ID].array.CD[kk*naxes0*naxes1+jj*naxes0+ii], data.image[ID].array.CD[kk*naxes0*naxes1+(jj-yhalf)*naxes0+(ii+xhalf)])
                    }
        }
    }



    if(OK==0)
        printf("Error : data format not supported by permut\n");


    //  printf(" done\n");
    // fflush(stdout);


    return(0);
}



int array_index(long size)
{
    int i;

    switch (size) {
    case 1:
        i=0;
        break;
    case 2:
        i=1;
        break;
    case 4:
        i=2;
        break;
    case 8:
        i=3;
        break;
    case 16:
        i=4;
        break;
    case 32:
        i=5;
        break;
    case 64:
        i=6;
        break;
    case 128:
        i=7;
        break;
    case 256:
        i=8;
        break;
    case 512:
        i=9;
        break;
    case 1024:
        i=10;
        break;
    case 2048:
        i=11;
        break;
    case 4096:
        i=12;
        break;
    case 8192:
        i=13;
        break;
    case 16384:
        i=14;
        break;
    default:
        i=100;
    }

    return(i);
}





/* 1d complex -> complex fft */
// supports single and double precisions
long FFT_do1dfft(const char *in_name, const char *out_name, int dir)
{
    int *naxes;
    uint32_t *naxesl;
    long naxis;
    long IDin, IDout;
    long i;
    int OK=0;
    fftwf_plan plan;
    fftw_plan plan_double;
	long jj;
	fftwf_complex *inptr, *outptr;
	fftw_complex *inptr_double, *outptr_double;
	int atype;

    IDin=image_ID(in_name);
    naxis=data.image[IDin].md[0].naxis;
    naxes = (int *) malloc(naxis*sizeof(int));
    naxesl = (uint32_t *) malloc(naxis*sizeof(uint32_t));
    for (i=0; i<naxis; i++)
    {
        naxesl[i]= data.image[IDin].md[0].size[i];
        naxes[i]= (int) data.image[IDin].md[0].size[i];
    }
	atype = data.image[IDin].md[0].atype;
    IDout = create_image_ID(out_name, naxis, naxesl, atype, data.SHARED_DFT, data.NBKEWORD_DFT);

    if(naxis==1)
    {
        if(array_index(naxes[0])!=100)
        {
            OK=1;
            if(atype == _DATATYPE_COMPLEX_FLOAT)
				{
					plan = fftwf_plan_dft_1d(naxes[0], (fftwf_complex*) data.image[IDin].array.CF, (fftwf_complex*) data.image[IDout].array.CF, dir, FFTWOPTMODE);
					fftwf_execute(plan);
					fftwf_destroy_plan(plan);
				}
            else
				{
					plan_double = fftw_plan_dft_1d(naxes[0], (fftw_complex*) data.image[IDin].array.CD, (fftw_complex*) data.image[IDout].array.CD, dir, FFTWOPTMODE);
					fftw_execute(plan_double);
					fftw_destroy_plan(plan_double);
				}
        }
        else
        {
            OK=1;
            if(atype == _DATATYPE_COMPLEX_FLOAT)
				{
					plan = fftwf_plan_dft_1d(naxes[0], (fftwf_complex*) data.image[IDin].array.CF, (fftwf_complex*) data.image[IDout].array.CF, dir, FFTWOPTMODE);
					fftwf_execute(plan);
					fftwf_destroy_plan(plan);
				}
            else
				{
					plan_double =  fftw_plan_dft_1d(naxes[0], (fftw_complex*) data.image[IDin].array.CD, (fftw_complex*) data.image[IDout].array.CD, dir, FFTWOPTMODE);
					fftw_execute(plan_double);
					fftw_destroy_plan(plan_double);
				}
        }
    }

    if(naxis==2)
    {
        if((naxes[1]==1)&&(array_index(naxes[0])!=100))
        {
            OK=1;
            if(atype == _DATATYPE_COMPLEX_FLOAT)
            {
				inptr = (fftwf_complex*) data.image[IDin].array.CF;
				outptr = (fftwf_complex*) data.image[IDout].array.CF;
				plan = fftwf_plan_dft_1d(naxes[0], inptr, outptr, dir, FFTWOPTMODE);
				fftwf_execute(plan);
				fftwf_destroy_plan(plan);
			}
			else
			{
				inptr_double = (fftw_complex*) data.image[IDin].array.CD;
				outptr_double = (fftw_complex*) data.image[IDout].array.CD;
				plan_double = fftw_plan_dft_1d(naxes[0], inptr_double, outptr_double, dir, FFTWOPTMODE);
				fftw_execute(plan_double);
				fftw_destroy_plan(plan_double);
			}
        }
        else
        {
            OK=1;
            if(atype == _DATATYPE_COMPLEX_FLOAT)
            {
				inptr = (fftwf_complex*) malloc(sizeof(fftwf_complex)*naxes[0]);
				outptr = (fftwf_complex*) malloc(sizeof(fftwf_complex)*naxes[0]);
				plan = fftwf_plan_dft_1d(naxes[0], inptr, outptr, dir, FFTWOPTMODE);
            
				for(jj=0;jj<naxes[1];jj++)
				{
					memcpy((char*) inptr, (char*) data.image[IDin].array.CF + sizeof(fftwf_complex)*jj*naxes[0], sizeof(fftwf_complex)*naxes[0]);
					fftwf_execute(plan);
					memcpy((char*) data.image[IDout].array.CF + sizeof(complex_float)*jj*naxes[0], outptr, sizeof(fftwf_complex)*naxes[0]);
				}
				fftwf_destroy_plan(plan);
				free(inptr);
				free(outptr);
			}
			else
			{
				inptr_double = (fftw_complex*) malloc(sizeof(fftw_complex)*naxes[0]);
				outptr_double = (fftw_complex*) malloc(sizeof(fftw_complex)*naxes[0]);
				plan_double = fftw_plan_dft_1d(naxes[0], inptr_double, outptr_double, dir, FFTWOPTMODE);
            
				for(jj=0;jj<naxes[1];jj++)
				{
					memcpy((char*) inptr_double, (char*) data.image[IDin].array.CD + sizeof(fftw_complex)*jj*naxes[0], sizeof(fftw_complex)*naxes[0]);
					fftw_execute(plan_double);
					memcpy((char*) data.image[IDout].array.CD + sizeof(complex_double)*jj*naxes[0], outptr_double, sizeof(fftw_complex)*naxes[0]);
				}
				fftw_destroy_plan(plan_double);
				free(inptr_double);
				free(outptr_double);
			}
        }
    }

    if(OK==0)
    {
        printf("Error : image dimension not appropriate for FFT\n");
    }
    free(naxes);
    free(naxesl);

    return(IDout);
}



/* 1d real -> complex fft */
// supports single and double precision
long do1drfft(const char *in_name, const char *out_name)
{
    int *naxes;
    uint32_t *naxesl;
    uint32_t *naxesout;
    long naxis;
    long IDin, IDout, IDtmp;
    long i;
    int OK=0;
    long ii,jj;
    fftwf_plan plan;
	fftw_plan plan_double;
	fftwf_complex *outptr;
	fftw_complex *outptr_double;
	float *inptr;
	double *inptr_double;
	int atype;
	
    char ffttmpname[SBUFFERSIZE];
    int n;

    IDin = image_ID(in_name);
    naxis = data.image[IDin].md[0].naxis;
    naxes = (int *) malloc(naxis*sizeof(int));
    naxesl = (uint32_t *) malloc(naxis*sizeof(uint32_t));
    naxesout = (uint32_t *) malloc(naxis*sizeof(uint32_t));

	atype = data.image[IDin].md[0].atype;

    for (i=0; i<naxis; i++)
    {
        naxesl[i]= data.image[IDin].md[0].size[i];
        naxes[i]= (int) data.image[IDin].md[0].size[i];
        naxesout[i]=data.image[IDin].md[0].size[i];
        if(i==0)
            {
				naxesout[i] = data.image[IDin].md[0].size[i]/2+1;
			}
    }


    n = snprintf(ffttmpname,SBUFFERSIZE,"_ffttmpname_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
   // IDtmp = create_image_ID(ffttmpname, naxis, naxestmp, CDtype, data.SHARED_DFT, data.NBKEWORD_DFT);


	if(atype == _DATATYPE_FLOAT)
		IDout = create_image_ID(out_name, naxis, naxesout, _DATATYPE_COMPLEX_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);
	else
		IDout = create_image_ID(out_name, naxis, naxesout, _DATATYPE_COMPLEX_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT);

    if(naxis==2)
    {
        if((naxes[1]==1)&&(array_index(naxes[0])!=100))
        {
            OK=1;
            if(atype == _DATATYPE_FLOAT)
            {
				plan = fftwf_plan_dft_r2c_1d(naxes[0], data.image[IDin].array.F, (fftwf_complex*) data.image[IDout].array.CF, FFTWOPTMODE);
				fftwf_execute(plan);
				fftwf_destroy_plan(plan);
			}
			else
			{
				plan_double = fftw_plan_dft_r2c_1d(naxes[0], data.image[IDin].array.D, (fftw_complex*) data.image[IDout].array.CD, FFTWOPTMODE);
				fftw_execute(plan_double);
				fftw_destroy_plan(plan_double);
			}
        }
        else
        {
            OK=1;
            if(atype == _DATATYPE_FLOAT)
            {
				inptr = (float*) malloc(sizeof(float)*naxes[0]);
				outptr = (fftwf_complex*) malloc(sizeof(fftwf_complex)*naxes[0]);
				plan = fftwf_plan_dft_r2c_1d(naxes[0], inptr, outptr, FFTWOPTMODE);
            
				for(jj=0;jj<naxes[1];jj++)
				{
					memcpy((char*) inptr, (char*) data.image[IDin].array.F + sizeof(float)*jj*naxes[0], sizeof(float)*naxes[0]);
					fftwf_execute(plan);
					memcpy((char*) data.image[IDout].array.CF + sizeof(complex_float)*jj*naxesout[0], outptr, sizeof(fftwf_complex)*naxesout[0]);
				}
				fftwf_destroy_plan(plan);
				free(inptr);
				free(outptr);
			}
			else
			{
				inptr_double = (double*) malloc(sizeof(double)*naxes[0]);
				outptr_double = (fftw_complex*) malloc(sizeof(fftw_complex)*naxes[0]);
				plan_double = fftw_plan_dft_r2c_1d(naxes[0], inptr_double, outptr_double, FFTWOPTMODE);
            
				for(jj=0;jj<naxes[1];jj++)
				{
					memcpy((char*) inptr_double, (char*) data.image[IDin].array.D + sizeof(double)*jj*naxes[0], sizeof(double)*naxes[0]);
					fftw_execute(plan_double);
					memcpy((char*) data.image[IDout].array.CD + sizeof(complex_double)*jj*naxesout[0], outptr_double, sizeof(fftw_complex)*naxesout[0]);
				}
				fftw_destroy_plan(plan_double);
				free(inptr_double);
				free(outptr_double);
			}
        }
    }

    if(OK==0)
    {
        printf("Error : image dimension not appropriate for FFT\n");
    }
    free(naxes);
    free(naxesl);
    free(naxesout);
   // delete_image_ID(ffttmpname);

    return(IDout);
}




long do1dfft(const char *in_name, const char *out_name)
{
	long IDout;
	
	IDout = FFT_do1dfft(in_name, out_name, -1);
	
	return(IDout);
}


long do1dffti(const char *in_name, const char *out_name)
{
	long IDout;
	
	IDout = FFT_do1dfft(in_name, out_name, 1);
	
	return(IDout);
}






/* 2d complex fft */
// supports single and double precisions
long FFT_do2dfft(const char *in_name, const char *out_name, int dir)
{
    int *naxes;
    uint32_t *naxesl;
    long naxis;
    long IDin,IDout;
    long i;
    int OK=0;
    fftwf_plan plan;
    fftw_plan plan_double;
    long tmp1;
    long IDcpy;

    char ffttmpcpyname[SBUFFERSIZE];
    int n;
    long nextID;
	int atype;


	
    IDin = image_ID(in_name);
    naxis = data.image[IDin].md[0].naxis;
    naxes = (int *) malloc(naxis*sizeof(int));
    naxesl = (uint32_t *) malloc(naxis*sizeof(uint32_t));

    for (i=0; i<naxis; i++)
    {
        naxesl[i]= (long) data.image[IDin].md[0].size[i];
        naxes[i]= (int) data.image[IDin].md[0].size[i];
    }

	

	atype = data.image[IDin].md[0].atype;
    IDout = create_image_ID(out_name, naxis, naxesl, atype, data.SHARED_DFT, data.NBKEWORD_DFT);



    // need to swap first 2 axis for fftw
    if(naxis>1)
    {
        tmp1 = naxes[0];
        naxes[0] = naxes[1];
        naxes[1] = tmp1;
    }
    
    

    if(naxis==2)
    {
        OK=1;
        
        if(atype == _DATATYPE_COMPLEX_FLOAT)
        {
			plan = fftwf_plan_dft_2d(naxes[0],naxes[1], (fftwf_complex*) data.image[IDin].array.CF, (fftwf_complex*) data.image[IDout].array.CF, dir, FFTWOPTMODE);
			if(plan==NULL)
			{
				//	  if ( Debug > 2)
				fprintf(stdout,"New FFT size [do2dfft %d x %d]: optimizing ...",naxes[1],naxes[0]);
				fflush(stdout);

				n = snprintf(ffttmpcpyname,SBUFFERSIZE,"_ffttmpcpyname_%d",(int) getpid());
				if(n >= SBUFFERSIZE)
					printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
				copy_image_ID(in_name, ffttmpcpyname, 0);

				plan = fftwf_plan_dft_2d(naxes[0],naxes[1], (fftwf_complex*) data.image[IDin].array.CF, (fftwf_complex*) data.image[IDout].array.CF, dir, FFTWOPTMODE);
				copy_image_ID(ffttmpcpyname, in_name, 0);
				delete_image_ID(ffttmpcpyname);
				export_wisdom();
				fprintf(stdout,"\n");
			}
			fftwf_execute(plan);
			fftwf_destroy_plan(plan);
		}
		else
		{
			plan_double = fftw_plan_dft_2d(naxes[0], naxes[1], (fftw_complex*) data.image[IDin].array.CD, (fftw_complex*) data.image[IDout].array.CD, dir, FFTWOPTMODE);
			if(plan_double==NULL)
			{
				//	  if ( Debug > 2)
				fprintf(stdout,"New FFT size [do2dfft %d x %d]: optimizing ...",naxes[1], naxes[0]);
				fflush(stdout);

				n = snprintf(ffttmpcpyname,SBUFFERSIZE,"_ffttmpcpyname_%d",(int) getpid());
				if(n >= SBUFFERSIZE)
					printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
				copy_image_ID(in_name, ffttmpcpyname, 0);

				plan_double = fftw_plan_dft_2d(naxes[0], naxes[1], (fftw_complex*) data.image[IDin].array.CD, (fftw_complex*) data.image[IDout].array.CD, dir, FFTWOPTMODE);
				copy_image_ID(ffttmpcpyname, in_name, 0);
				delete_image_ID(ffttmpcpyname);
				export_wisdom();
				fprintf(stdout,"\n");
			}
			fftw_execute(plan_double);
			fftw_destroy_plan(plan_double);
		}
    }



    if(naxis==3)
    {
        OK=1;
		if(atype == _DATATYPE_COMPLEX_FLOAT)
        {
			plan = fftwf_plan_many_dft(2,naxes,naxes[2],(fftwf_complex*) data.image[IDin].array.CF,NULL, 1, naxes[0]*naxes[1],(fftwf_complex*) data.image[IDout].array.CF,NULL, 1 ,naxes[0]*naxes[1], dir, FFTWOPTMODE);
			if(plan==NULL)
			{
				//if ( Debug > 2)
				fprintf(stdout,"New FFT size [do2dfft %d x %d x %d]: optimizing ...",naxes[1],naxes[0],naxes[2]);
				fflush(stdout);

				n = snprintf(ffttmpcpyname,SBUFFERSIZE,"_ffttmpcpyname_%d",(int) getpid());
				if(n >= SBUFFERSIZE)
					printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
				copy_image_ID(in_name, ffttmpcpyname, 0);

				plan = fftwf_plan_many_dft(2,naxes,naxes[2],(fftwf_complex*) data.image[IDin].array.CF,NULL, 1, naxes[0]*naxes[1],(fftwf_complex*) data.image[IDout].array.CF,NULL, 1,naxes[0]*naxes[1], dir, FFTWOPTMODE);
				copy_image_ID(ffttmpcpyname, in_name, 0);
				delete_image_ID(ffttmpcpyname);
				export_wisdom();
				fprintf(stdout,"\n");
			}
			fftwf_execute(plan);
			fftwf_destroy_plan(plan);
		}
		else
		{
			plan_double = fftw_plan_many_dft(2, naxes, naxes[2],(fftw_complex*) data.image[IDin].array.CD, NULL, 1, naxes[0]*naxes[1], (fftw_complex*) data.image[IDout].array.CD, NULL, 1 ,naxes[0]*naxes[1], dir, FFTWOPTMODE);
			if(plan_double==NULL)
			{
				//if ( Debug > 2)
				fprintf(stdout,"New FFT size [do2dfft %d x %d x %d]: optimizing ...",naxes[1],naxes[0],naxes[2]);
				fflush(stdout);

				n = snprintf(ffttmpcpyname, SBUFFERSIZE, "_ffttmpcpyname_%d", (int) getpid());
				if(n >= SBUFFERSIZE)
					printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
				copy_image_ID(in_name, ffttmpcpyname, 0);

				plan_double = fftw_plan_many_dft(2, naxes, naxes[2], (fftw_complex*) data.image[IDin].array.CD, NULL, 1, naxes[0]*naxes[1],(fftw_complex*) data.image[IDout].array.CD,NULL, 1, naxes[0]*naxes[1], dir, FFTWOPTMODE);
				copy_image_ID(ffttmpcpyname, in_name, 0);
				delete_image_ID(ffttmpcpyname);
				export_wisdom();
				fprintf(stdout,"\n");
			}
			fftw_execute(plan_double);
			fftw_destroy_plan(plan_double);
		}
    }


    if(OK==0)
        printf("Error : image dimension not appropriate for FFT\n");

    free(naxes);


    return(IDout);
}




long do2dfft(const char *in_name, const char *out_name)
{
	long IDout;
	
	IDout = FFT_do2dfft(in_name, out_name, -1);
	
	return(IDout);
}


long do2dffti(const char *in_name, const char *out_name)
{
	long IDout;
	
	IDout = FFT_do2dfft(in_name, out_name, 1);
	
	return(IDout);
}















/* inv = 0 for direct fft and 1 for inverse fft */
/* direct = focal plane -> pupil plane  equ. fft2d(..,..,..,1) */
/* inverse = pupil plane -> focal plane equ. fft2d(..,..,..,0) */
/* options :  -reim  takes real/imaginary input and creates real/imaginary output
               -inv  for inverse fft (inv=1) */
int pupfft(const char *ID_name_ampl, const char *ID_name_pha, const char *ID_name_ampl_out, const char *ID_name_pha_out, const char *options)
{
    int reim;
    int inv;

    char Ctmpname[SBUFFERSIZE];
    char C1tmpname[SBUFFERSIZE];
    int n;

    reim = 0;
    inv = 0;

    if(strstr(options,"-reim")!=NULL)
    {
        /*	printf("taking real / imaginary input/output\n");*/
        reim = 1;
    }

    if(strstr(options,"-inv")!=NULL)
    {
        /*printf("doing the inverse Fourier transform\n");*/
        inv = 1;
    }


    n = snprintf(Ctmpname,SBUFFERSIZE,"_Ctmp_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    if (reim==0)
    {
        mk_complex_from_amph(ID_name_ampl, ID_name_pha, Ctmpname, 0);
    }
    else
    {
        mk_complex_from_reim(ID_name_ampl, ID_name_pha, Ctmpname, 0);
    }

    permut(Ctmpname);

    n = snprintf(C1tmpname,SBUFFERSIZE,"_C1tmp_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    if(inv==0)
        do2dfft(Ctmpname, C1tmpname); /* equ. fft2d(..,1) */
    else
        do2dffti(Ctmpname, C1tmpname); /* equ. fft2d(..,0) */

    delete_image_ID(Ctmpname);

    if (reim==0)
    {
        /* if this line is removed, the program crashes... why ??? */
        /*	list_image_ID(data); */
        mk_amph_from_complex(C1tmpname,ID_name_ampl_out,ID_name_pha_out, 0);
    }
    else
    {
        mk_reim_from_complex(C1tmpname,ID_name_ampl_out,ID_name_pha_out, 0);
    }

    delete_image_ID(C1tmpname);

    permut(ID_name_ampl_out);
    permut(ID_name_pha_out);

    return(0);
}





/* real fft : real to complex */
// supports single and double precisions
long FFT_do2drfft(const char *in_name, const char *out_name, int dir)
{
    int *naxes;  // int format for fftw 
    uint32_t *naxesl;
    uint32_t *naxestmp;

    long naxis;
    long IDin,IDout,IDtmp;
    long i;
    int OK=0;
    long idist;
    long ii,jj,kk;
    fftwf_plan plan;
    fftw_plan plan_double;
    long tmp1;

    char ffttmpname[SBUFFERSIZE];
    char ffttmpcpyname[SBUFFERSIZE];
    int n;
    
    int atype;
    int atypeout;


    IDin = image_ID(in_name);
    
    atype = data.image[IDin].md[0].atype;
    naxis = data.image[IDin].md[0].naxis;
    naxes = (int *) malloc(naxis*sizeof(int));
    naxesl = (uint32_t *) malloc(naxis*sizeof(uint32_t));
    naxestmp = (uint32_t *) malloc(naxis*sizeof(uint32_t));




    for (i=0; i<naxis; i++)
    {
        naxes[i] = (int) data.image[IDin].md[0].size[i];
        naxesl[i] = (uint32_t) data.image[IDin].md[0].size[i];
        naxestmp[i] = data.image[IDin].md[0].size[i];
        if(i==0)
            naxestmp[i] = data.image[IDin].md[0].size[i]/2+1;
    }

    n = snprintf(ffttmpname,SBUFFERSIZE,"_ffttmp_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

	if(atype==_DATATYPE_FLOAT)
		atypeout = _DATATYPE_COMPLEX_FLOAT;
	else
		atypeout = _DATATYPE_COMPLEX_DOUBLE;

    IDtmp = create_image_ID(ffttmpname, naxis, naxestmp, atypeout, data.SHARED_DFT, data.NBKEWORD_DFT);

    IDout = create_image_ID(out_name, naxis, naxesl, atypeout, data.SHARED_DFT, data.NBKEWORD_DFT);
    
    if(naxis==2)
    {
        OK=1;
        if(atype==_DATATYPE_FLOAT)
        {
            plan = fftwf_plan_dft_r2c_2d((int) naxes[1], (int) naxes[0], data.image[IDin].array.F, (fftwf_complex*) data.image[IDtmp].array.CF, FFTWOPTMODE);
            if(plan==NULL)
            {
                // if ( Debug > 2)
                fprintf(stdout,"New FFT size [do2drfft %d x %d]: optimizing ...",naxes[1], naxes[0]);
                fflush(stdout);

                n = snprintf(ffttmpcpyname, SBUFFERSIZE, "_ffttmpcpy_%d", (int) getpid());
                if(n >= SBUFFERSIZE)
                    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
                copy_image_ID(in_name, ffttmpcpyname, 0);

                plan = fftwf_plan_dft_r2c_2d(naxes[1], naxes[0], data.image[IDin].array.F, (fftwf_complex*) data.image[IDtmp].array.CF, FFTWOPTMODE);
                copy_image_ID(ffttmpcpyname, in_name, 0);
                delete_image_ID(ffttmpcpyname);
                export_wisdom();
                fprintf(stdout,"\n");
            }
            fftwf_execute(plan);
            fftwf_destroy_plan(plan);

            if(dir == -1)
            {
                for(ii=0; ii<naxes[0]/2+1; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                        data.image[IDout].array.CF[jj*naxes[0]+ii] = data.image[IDtmp].array.CF[jj*naxestmp[0]+ii];
                 
                for(ii=1; ii<naxes[0]/2+1; ii++)
                {
                    jj=0;
                    data.image[IDout].array.CF[jj*naxes[0]+(naxes[0]-ii)].re = data.image[IDtmp].array.CF[jj*naxestmp[0]+ii].re;
                    data.image[IDout].array.CF[jj*naxes[0]+(naxes[0]-ii)].im = -data.image[IDtmp].array.CF[jj*naxestmp[0]+ii].im;
                    for(jj=1; jj<naxes[1]; jj++)
                    {
                        data.image[IDout].array.CF[jj*naxes[0]+(naxes[0]-ii)].re = data.image[IDtmp].array.CF[(naxes[1]-jj)*naxestmp[0]+ii].re;
                        data.image[IDout].array.CF[jj*naxes[0]+(naxes[0]-ii)].im = -data.image[IDtmp].array.CF[(naxes[1]-jj)*naxestmp[0]+ii].im;
                    }
                }
            }
        }
        else
        {
            plan_double = fftw_plan_dft_r2c_2d(naxes[1], naxes[0], data.image[IDin].array.D, (fftw_complex*) data.image[IDtmp].array.CD, FFTWOPTMODE);
            if(plan_double==NULL)
            {
                // if ( Debug > 2)
                fprintf(stdout,"New FFT size [do2drfft %d x %d]: optimizing ...",naxes[1], naxes[0]);
                fflush(stdout);

                n = snprintf(ffttmpcpyname,SBUFFERSIZE,"_ffttmpcpy_%d",(int) getpid());
                if(n >= SBUFFERSIZE)
                    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
                copy_image_ID(in_name, ffttmpcpyname, 0);

                plan_double = fftw_plan_dft_r2c_2d(naxes[1], naxes[0], data.image[IDin].array.D, (fftw_complex*) data.image[IDtmp].array.CD, FFTWOPTMODE);
                copy_image_ID(ffttmpcpyname, in_name, 0);
                delete_image_ID(ffttmpcpyname);
                export_wisdom();
                fprintf(stdout,"\n");
            }
            fftw_execute(plan_double);
            fftw_destroy_plan(plan_double);

            if(dir == -1)
            {
                for(ii=0; ii<naxes[0]/2+1; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                        data.image[IDout].array.CD[jj*naxes[0]+ii] = data.image[IDtmp].array.CD[jj*naxestmp[0]+ii];

                for(ii=1; ii<naxes[0]/2+1; ii++)
                {
                    jj=0;
                    data.image[IDout].array.CD[jj*naxes[0]+(naxes[0]-ii)].re = data.image[IDtmp].array.CD[jj*naxestmp[0]+ii].re;
                    data.image[IDout].array.CD[jj*naxes[0]+(naxes[0]-ii)].im = -data.image[IDtmp].array.CD[jj*naxestmp[0]+ii].im;
                    for(jj=1; jj<naxes[1]; jj++)
                    {
                        data.image[IDout].array.CD[jj*naxes[0]+(naxes[0]-ii)].re = data.image[IDtmp].array.CD[(naxes[1]-jj)*naxestmp[0]+ii].re;
                        data.image[IDout].array.CD[jj*naxes[0]+(naxes[0]-ii)].im = -data.image[IDtmp].array.CD[(naxes[1]-jj)*naxestmp[0]+ii].im;
                    }
                }
            }
        }
    }
    if(naxis==3)
    {
        OK=1;
        idist = naxes[0]*naxes[1];

        // swapping first 2 axis
        tmp1 = naxes[0];
        naxes[0] = naxes[1];
        naxes[1] = tmp1;

        if(atype==_DATATYPE_FLOAT)
        {
            plan = fftwf_plan_many_dft_r2c(2, naxes, naxes[2], data.image[IDin].array.F, NULL, 1, naxes[0]*naxes[1], (fftwf_complex*) data.image[IDout].array.CF,NULL,1,naxes[0]*naxes[1],FFTWOPTMODE);
            if(plan==NULL)
            {
                //	  if ( Debug > 2) fprintf(stdout,"New FFT size [do2drfft %d x %d x %d]: optimizing ...",naxes[1],naxes[0],naxes[2]);
                fflush(stdout);

                n = snprintf(ffttmpcpyname,SBUFFERSIZE,"_ffttmpcpy_%d",(int) getpid());
                if(n >= SBUFFERSIZE)
                    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
                copy_image_ID(in_name, ffttmpcpyname, 0);

                plan = fftwf_plan_many_dft_r2c(2, naxes, naxes[2], data.image[IDin].array.F, NULL, 1, naxes[0]*naxes[1], (fftwf_complex*) data.image[IDout].array.CF,NULL,1,naxes[0]*naxes[1],FFTWOPTMODE);

                copy_image_ID(ffttmpcpyname, in_name, 0);
                delete_image_ID(ffttmpcpyname);
                export_wisdom();
                fprintf(stdout,"\n");
            }

            fftwf_execute(plan);
            fftwf_destroy_plan(plan);

            if(dir==-1)
            {
                // unswapping first 2 axis
                tmp1 = naxes[0];
                naxes[0] = naxes[1];
                naxes[1] = tmp1;

                for(ii=0; ii<naxes[0]/2+1; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                        for(kk=0; kk<naxes[2]; kk++)
                        {
                            data.image[IDout].array.CF[naxes[0]*naxes[1]*kk+jj*naxes[0]+ii] = data.image[IDtmp].array.CF[naxestmp[0]*naxestmp[1]*kk+jj*naxestmp[0]+ii];
                            if(ii!=0)
                                data.image[IDout].array.CF[naxes[0]*naxes[1]*kk+jj*naxes[0]+(naxes[0]-ii)] = data.image[IDtmp].array.CF[naxestmp[0]*naxestmp[1]*kk+jj*naxestmp[0]+ii];
                        }
            }
        }
        else
        {
            plan_double = fftw_plan_many_dft_r2c(2, naxes, naxes[2], data.image[IDin].array.D, NULL, 1, naxes[0]*naxes[1], (fftw_complex*) data.image[IDout].array.CD, NULL, 1, naxes[0]*naxes[1], FFTWOPTMODE);
            if(plan==NULL)
            {
                //	  if ( Debug > 2) fprintf(stdout,"New FFT size [do2drfft %d x %d x %d]: optimizing ...",naxes[1],naxes[0],naxes[2]);
                //				fflush(stdout);

                n = snprintf(ffttmpcpyname,SBUFFERSIZE,"_ffttmpcpy_%d",(int) getpid());
                if(n >= SBUFFERSIZE)
                    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
                copy_image_ID(in_name, ffttmpcpyname, 0);

                plan_double = fftw_plan_many_dft_r2c(2, naxes, naxes[2], data.image[IDin].array.D, NULL, 1, naxes[0]*naxes[1], (fftw_complex*) data.image[IDout].array.CD, NULL, 1, naxes[0]*naxes[1], FFTWOPTMODE);

                copy_image_ID(ffttmpcpyname, in_name, 0);
                delete_image_ID(ffttmpcpyname);
                export_wisdom();
                fprintf(stdout,"\n");
            }

            fftw_execute(plan_double);
            fftw_destroy_plan(plan_double);

            if(dir==-1)
            {
                // unswapping first 2 axis
                tmp1 = naxes[0];
                naxes[0] = naxes[1];
                naxes[1] = tmp1;

                for(ii=0; ii<naxes[0]/2+1; ii++)
                    for(jj=0; jj<naxes[1]; jj++)
                        for(kk=0; kk<naxes[2]; kk++)
                        {
                            data.image[IDout].array.CD[naxes[0]*naxes[1]*kk+jj*naxes[0]+ii] = data.image[IDtmp].array.CD[naxestmp[0]*naxestmp[1]*kk+jj*naxestmp[0]+ii];
                            if(ii!=0)
                                data.image[IDout].array.CD[naxes[0]*naxes[1]*kk+jj*naxes[0]+(naxes[0]-ii)] = data.image[IDtmp].array.CD[naxestmp[0]*naxestmp[1]*kk+jj*naxestmp[0]+ii];
                        }
            }
        }
    }

    if(OK==0)
        printf("Error : image dimension not appropriate for FFT\n");

    delete_image_ID(ffttmpname);

    free(naxestmp);
    free(naxesl);
    free(naxes);

    return(IDout);
}




long do2drfft(const char *in_name, const char *out_name)
{
	long IDout;
	
	IDout = FFT_do2drfft(in_name, out_name, -1);

	return(IDout);
}



long do2drffti(const char *in_name, const char *out_name)
{
	long IDout;
	
	IDout = FFT_do2drfft(in_name, out_name, 1);

	return(IDout);
}










long fft_correlation(const char *ID_name1, const char *ID_name2, const char *ID_nameout)
{
    long ID1, ID2,IDout;
    long nelement;

    char ft1name[SBUFFERSIZE];
    char ft2name[SBUFFERSIZE];
    char fta1name[SBUFFERSIZE];
    char fta2name[SBUFFERSIZE];
    char ftp1name[SBUFFERSIZE];
    char ftp2name[SBUFFERSIZE];
    char fta12name[SBUFFERSIZE];
    char ftp12name[SBUFFERSIZE];
    char fftname[SBUFFERSIZE];
    char fft1name[SBUFFERSIZE];
    char fft1pname[SBUFFERSIZE];
    int n;

    ID1 = image_ID(ID_name1);
    nelement = data.image[ID1].md[0].nelement;


    n = snprintf(ft1name,SBUFFERSIZE,"_ft1_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    do2drfft(ID_name1, ft1name);

    n = snprintf(ft2name,SBUFFERSIZE,"_ft2_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    do2drfft(ID_name2, ft2name);

    n = snprintf(fta1name,SBUFFERSIZE,"_%s_a_%d",ft1name,(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    n = snprintf(ftp1name,SBUFFERSIZE,"_%s_p_%d",ft1name,(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    n = snprintf(fta2name,SBUFFERSIZE,"_%s_a_%d",ft2name,(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    n = snprintf(ftp2name,SBUFFERSIZE,"_%s_p_%d",ft2name,(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    n = snprintf(fta12name,SBUFFERSIZE,"_%s_12a_%d",ft1name,(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    n = snprintf(ftp12name,SBUFFERSIZE,"_%s_12p_%d",ft1name,(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    mk_amph_from_complex(ft1name, fta1name, ftp1name, 0);
    mk_amph_from_complex(ft2name, fta2name, ftp2name, 0);


    delete_image_ID(ft1name);
    delete_image_ID(ft2name);

    arith_image_mult(fta1name, fta2name, fta12name);
    arith_image_sub(ftp1name, ftp2name, ftp12name);
    delete_image_ID(fta1name);
    delete_image_ID(fta2name);
    delete_image_ID(ftp1name);
    delete_image_ID(ftp2name);

    arith_image_cstmult_inplace(fta12name,1.0/sqrt(nelement)/(1.0*nelement));

    n = snprintf(fftname,SBUFFERSIZE,"_fft_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    mk_complex_from_amph(fta12name, ftp12name, fftname, 0);
    delete_image_ID(fta12name);
    delete_image_ID(ftp12name);

    n = snprintf(fft1name,SBUFFERSIZE,"_fft1_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    do2dfft(fftname, fft1name);
    delete_image_ID(fftname);

    n = snprintf(fft1pname,SBUFFERSIZE,"_fft1p_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    mk_amph_from_complex(fft1name, ID_nameout, fft1pname, 0);
    permut(ID_nameout);
    
    delete_image_ID(fft1name); 
    delete_image_ID(fft1pname);

    IDout = image_ID(ID_nameout);


    return(IDout);
}


int autocorrelation(const char *ID_name, const char *ID_out)
{
    long ID;
    long nelement;

    char atmp1name[SBUFFERSIZE];
    char aampname[SBUFFERSIZE];
    char aphaname[SBUFFERSIZE];
    char sqaampname[SBUFFERSIZE];
    char sqaamp1name[SBUFFERSIZE];
    int n;


    ID = image_ID(ID_name);
    nelement = data.image[ID].md[0].nelement;

    n = snprintf(atmp1name,SBUFFERSIZE,"_atmp1_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    do2drfft(ID_name, atmp1name);
    
    n = snprintf(aampname,SBUFFERSIZE,"_aamp_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    n = snprintf(aphaname,SBUFFERSIZE,"_apha_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    mk_amph_from_complex(atmp1name, aampname, aphaname, 0);

    n = snprintf(sqaampname,SBUFFERSIZE,"_sqaamp_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    arith_image_mult(aampname, aampname, sqaampname);
    delete_image_ID(aampname);
    delete_image_ID(aphaname);
    delete_image_ID(atmp1name);

    n = snprintf(sqaamp1name,SBUFFERSIZE,"_sqaamp1_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    arith_image_cstmult(sqaampname,1.0/sqrt(nelement)/(1.0*nelement),sqaamp1name);
    delete_image_ID(sqaampname);
    
    do2drfft(sqaamp1name, atmp1name);
    mk_reim_from_complex(atmp1name, ID_out, aphaname, 0);
    delete_image_ID(sqaamp1name);
    delete_image_ID(atmp1name);
    delete_image_ID(aphaname);


    return(0);
}




int fftczoom(const char *ID_name, const char *ID_out, long factor)
{
    long ID,ID1;
    long naxes[2];
    long ii,jj;
    double coeff;

    char tmpzname[SBUFFERSIZE];
    char tmpz1name[SBUFFERSIZE];
    int n;

    ID=image_ID(ID_name);

    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    coeff = 1.0/(factor*factor*naxes[0]*naxes[1]);
    permut(ID_name);

    n = snprintf(tmpzname,SBUFFERSIZE,"_tmpz_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    do2dfft(ID_name, tmpzname);


    permut(ID_name);
    permut(tmpzname);
    ID = image_ID(tmpzname);

    n = snprintf(tmpz1name,SBUFFERSIZE,"_tmpz1_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    ID1 = create_2DCimage_ID(tmpz1name,factor*naxes[0],factor*naxes[1]);

    for(ii=0; ii<naxes[0]; ii++)
        for(jj=0; jj<naxes[1]; jj++)
        {
            data.image[ID1].array.CF[(jj+factor*naxes[1]/2-naxes[1]/2)*naxes[0]*factor+(ii+factor*naxes[0]/2-naxes[0]/2)].re = data.image[ID].array.CF[jj*naxes[0]+ii].re*coeff;
            data.image[ID1].array.CF[(jj+factor*naxes[1]/2-naxes[1]/2)*naxes[0]*factor+(ii+factor*naxes[0]/2-naxes[0]/2)].im = data.image[ID].array.CF[jj*naxes[0]+ii].im*coeff;
        }
    delete_image_ID(tmpzname);

    permut(tmpz1name);
    do2dffti(tmpz1name, ID_out);
    permut(ID_out);
    delete_image_ID(tmpz1name);

    return(0);
}




int fftzoom(const char *ID_name, const char *ID_out, long factor)
{
    long ID,ID1;
    long naxes[2];
    long ii,jj;
    double coeff;

    char tmpzname[SBUFFERSIZE];
    char tmpz1name[SBUFFERSIZE];
    char tmpz2name[SBUFFERSIZE];
    char tbename[SBUFFERSIZE];
    int n;

    ID = image_ID(ID_name);

    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    coeff = 1.0/(factor*factor*naxes[0]*naxes[1]);
    permut(ID_name);

    n = snprintf(tmpzname,SBUFFERSIZE,"_tmpz_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    do2drfft(ID_name,tmpzname);

    permut(ID_name);
    permut(tmpzname);
    ID = image_ID(tmpzname);

    n = snprintf(tmpz1name,SBUFFERSIZE,"_tmpz1_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    ID1 = create_2DCimage_ID(tmpz1name,factor*naxes[0],factor*naxes[1]);

    for(ii=0; ii<naxes[0]; ii++)
        for(jj=0; jj<naxes[1]; jj++)
        {
            data.image[ID1].array.CF[(jj+factor*naxes[1]/2-naxes[1]/2)*naxes[0]*factor+(ii+factor*naxes[0]/2-naxes[0]/2)].re = data.image[ID].array.CF[jj*naxes[0]+ii].re*coeff;
            data.image[ID1].array.CF[(jj+factor*naxes[1]/2-naxes[1]/2)*naxes[0]*factor+(ii+factor*naxes[0]/2-naxes[0]/2)].im = data.image[ID].array.CF[jj*naxes[0]+ii].im*coeff;
        }
    delete_image_ID(tmpzname);

    permut(tmpz1name);

    n = snprintf(tmpz2name,SBUFFERSIZE,"_tmpz2_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    do2dffti(tmpz1name, tmpz2name);

    permut(tmpz2name);
    delete_image_ID(tmpz1name);

    n = snprintf(tbename,SBUFFERSIZE,"_tbe_%d",(int) getpid());
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    mk_reim_from_complex(tmpz2name,ID_out,tbename, 0);

    delete_image_ID(tbename);
    delete_image_ID(tmpz2name);

    return(0);
}






/** @brief Test FFT speed (fftw)
 * 
 */ 

int test_fftspeed(int nmax)
{
    int n;
    long size;
    int nbiter,iter;

    struct timespec tS0;
    struct timespec tS1;
    struct timespec tS2;
    double ti0,ti1,ti2;
    double dt1;
    struct timeval tv;
    int nb_threads=1;
    int nb_threads_max = 8;

    /*  printf("%ld ticks per second\n",CLOCKS_PER_SEC);*/
    nbiter = 10000;
    size=2;

    printf("Testing complex FFT, nxn pix\n");

    printf("size(pix)");
# ifdef FFTWMT
    for(nb_threads=1; nb_threads<nb_threads_max; nb_threads++)
        printf("%13d",nb_threads);
# endif
    printf("\n");


    size = 2;
    for(n=0; n<nmax; n++)
    {
        printf("%9ld",size);
# ifdef FFTWMT
        for(nb_threads=1; nb_threads<nb_threads_max; nb_threads++)
        {
            fft_setNthreads(nb_threads);
# endif

#if _POSIX_TIMERS > 0
            clock_gettime(CLOCK_REALTIME, &tS0);
#else
            gettimeofday(&tv, NULL);
            tS0.tv_sec = tv.tv_sec;
            tS0.tv_nsec = tv.tv_usec*1000;
#endif

		

            //	  clock_gettime(CLOCK_REALTIME, &tS0);
            for(iter=0; iter<nbiter; iter++)
            {
                create_2DCimage_ID("tmp", size, size);
                do2dfft("tmp", "tmpf");
                delete_image_ID("tmp");
                delete_image_ID("tmpf");
            }

#if _POSIX_TIMERS > 0
            clock_gettime(CLOCK_REALTIME, &tS1);
#else
            gettimeofday(&tv, NULL);
            tS1.tv_sec = tv.tv_sec;
            tS1.tv_nsec = tv.tv_usec*1000;
#endif
            //	  clock_gettime(CLOCK_REALTIME, &tS1);

            for(iter=0; iter<nbiter; iter++)
            {
                create_2DCimage_ID("tmp",size,size);
                delete_image_ID("tmp");
            }

#if _POSIX_TIMERS > 0
            clock_gettime(CLOCK_REALTIME, &tS2);
#else
            gettimeofday(&tv, NULL);
            tS2.tv_sec = tv.tv_sec;
            tS2.tv_nsec = tv.tv_usec*1000;
#endif
            //clock_gettime(CLOCK_REALTIME, &tS2);


            ti0 = 1.0*tS0.tv_sec+0.000000001*tS0.tv_nsec;
            ti1 = 1.0*tS1.tv_sec+0.000000001*tS1.tv_nsec;
            ti2 = 1.0*tS2.tv_sec+0.000000001*tS2.tv_nsec;
            dt1 = 1.0*(ti1-ti0)-1.0*(ti2-ti1);

            dt1 /= nbiter;

            printf("%10.3f ms",dt1*1000.0);
            //printf("Complex FFT %ldx%ld [%d threads] : %f ms  [%ld]\n",size,size,nb_threads,dt1*1000.0,nbiter);
            fflush(stdout);
# ifdef FFTWMT
        }
# endif
        printf("\n");
        nbiter = 0.1/dt1;
        if(nbiter<2)
            nbiter = 2;
        size = size*2;
    }

    return(0);
}






/* ----------------- CUSTOM DFT ------------- */

//
// Zfactor is zoom factor
// dir = -1 for FT, 1 for inverse FT
// kin in selects slice in IDin_name if this is a cube
//
long fft_DFT( const char *IDin_name, const char *IDinmask_name, const char *IDout_name, const char *IDoutmask_name, double Zfactor, int dir, long kin)
{
    long IDin;
    long IDout;
    long IDinmask;
    long IDoutmask;

    long NBptsin;
    long NBptsout;

    uint_fast16_t xsize, ysize;
    uint_fast16_t ii, jj, k, kout;
    double val;
    double re, im;
    float pha;

    uint_fast16_t *iiinarray;
    uint_fast16_t *jjinarray;
    double *xinarray;
    double *yinarray;
    double *valinamp;
    double *valinpha;
    float *cosvalinpha;
    float *sinvalinpha;

    uint_fast16_t *iioutarray;
    uint_fast16_t *jjoutarray;
    double *xoutarray;
    double *youtarray;

	float cospha, sinpha;

	long IDcosXX, IDcosYY, IDsinXX, IDsinYY;

	// list of active coordinates
	uint_fast16_t *iiinarrayActive;
	uint_fast16_t *jjinarrayActive;
	uint_fast16_t *iioutarrayActive;
	uint_fast16_t *jjoutarrayActive;	
	uint_fast16_t pixiiin, pixiiout, pixjjin, pixjjout;
	
	uint_fast8_t pixact;
	uint_fast16_t NBpixact_iiin, NBpixact_jjin;
	uint_fast16_t NBpixact_iiout, NBpixact_jjout;

	float *XinarrayActive;
	float *YinarrayActive;
	float *XoutarrayActive;
	float *YoutarrayActive;
	uint_fast16_t iiin, jjin, iiout, jjout;

	float cosXX, sinXX, cosYY, sinYY, cosXY, sinXY;





    IDin = image_ID(IDin_name);

    IDinmask = image_ID(IDinmask_name);
    xsize = data.image[IDinmask].md[0].size[0];
    ysize = data.image[IDinmask].md[0].size[1];
   	iiinarrayActive = (uint_fast16_t *) malloc(sizeof(uint_fast16_t)*xsize);
	jjinarrayActive = (uint_fast16_t *) malloc(sizeof(uint_fast16_t)*ysize);
	iioutarrayActive = (uint_fast16_t *) malloc(sizeof(uint_fast16_t)*xsize);
	jjoutarrayActive = (uint_fast16_t *) malloc(sizeof(uint_fast16_t)*ysize);


    
    
    
    NBptsin = 0;
    NBpixact_iiin = 0;
    for(ii=0; ii<xsize; ii++)
    {
		pixact = 0;
        for(jj=0; jj<ysize; jj++)
        {
            val = data.image[IDinmask].array.F[jj*xsize+ii];
            if(val>0.5)
            {
				pixact = 1;
                NBptsin ++;
			}
        }
        if(pixact==1)
			{
				iiinarrayActive[NBpixact_iiin] = ii;
				NBpixact_iiin++;
			}
	}

	NBpixact_jjin = 0;
    for(jj=0; jj<ysize; jj++)
    {
		pixact = 0;
        for(ii=0; ii<xsize; ii++)
        {
            val = data.image[IDinmask].array.F[jj*xsize+ii];
            if(val>0.5)
				pixact = 1;
        }
        if(pixact==1)
			{
				jjinarrayActive[NBpixact_jjin] = jj;
				NBpixact_jjin++;
			}
	}
	
    XinarrayActive = (float *) malloc(sizeof(float)*NBpixact_iiin);
    YinarrayActive = (float *) malloc(sizeof(float)*NBpixact_jjin);

	for(pixiiin=0; pixiiin<NBpixact_iiin; pixiiin++)
	{
		iiin = iiinarrayActive[pixiiin];
		XinarrayActive[pixiiin] = (1.0*iiin/xsize-0.5);
	}
	for(pixjjin=0; pixjjin<NBpixact_jjin; pixjjin++)
	{
		jjin = jjinarrayActive[pixjjin];
		YinarrayActive[pixjjin] = (1.0*jjin/ysize-0.5);
	}

    printf("DFT (factor %f, slice %ld):  %ld input points (%ld %ld)-> ", Zfactor, kin, NBptsin, NBpixact_iiin, NBpixact_jjin);

    iiinarray = (uint_fast16_t *) malloc(sizeof(uint_fast16_t)*NBptsin);
    jjinarray = (uint_fast16_t *) malloc(sizeof(uint_fast16_t)*NBptsin);
    xinarray = (double *) malloc(sizeof(double)*NBptsin);
    yinarray = (double *) malloc(sizeof(double)*NBptsin);
    valinamp = (double *) malloc(sizeof(double)*NBptsin);
    valinpha = (double *) malloc(sizeof(double)*NBptsin);
    cosvalinpha = (float *) malloc(sizeof(float)*NBptsin);
    sinvalinpha = (float *) malloc(sizeof(float)*NBptsin);
    k = 0;



    for(ii=0; ii<xsize; ii++)
        for(jj=0; jj<ysize; jj++)
        {
            val = data.image[IDinmask].array.F[jj*xsize+ii];
            if(val>0.5)
            {
                iiinarray[k] = ii;
                jjinarray[k] = jj;
                xinarray[k] = 1.0*ii/xsize-0.5;
                yinarray[k] = 1.0*jj/xsize-0.5;
                re = data.image[IDin].array.CF[kin*xsize*ysize+jj*xsize+ii].re;
                im = data.image[IDin].array.CF[kin*xsize*ysize+jj*xsize+ii].im;
                valinamp[k] = sqrt(re*re+im*im);
                valinpha[k] = atan2(im,re);
                cosvalinpha[k] = cosf(valinpha[k]);
                sinvalinpha[k] = sinf(valinpha[k]);
                k++;
            }
        }




    IDoutmask = image_ID(IDoutmask_name);

    NBptsout = 0;
    NBpixact_iiout = 0;
    for(ii=0; ii<xsize; ii++)
    {
		pixact = 0;
	    for(jj=0; jj<ysize; jj++)
        {
            val = data.image[IDoutmask].array.F[jj*xsize+ii];
            if(val>0.5)
            {
				pixact = 1;
                NBptsout ++;
			}
        }
        if(pixact==1)
			{
				iioutarrayActive[NBpixact_iiout] = ii;
				NBpixact_iiout++;
			}
	}
	
	NBpixact_jjout = 0;
    for(jj=0; jj<ysize; jj++)
    {
		pixact = 0;
        for(ii=0; ii<xsize; ii++)
        {
            val = data.image[IDoutmask].array.F[jj*xsize+ii];
            if(val>0.5)
				pixact = 1;
        }
        if(pixact==1)
			{
				jjoutarrayActive[NBpixact_jjout] = jj;
				NBpixact_jjout++;
			}
	}
	XoutarrayActive = (float *) malloc(sizeof(float)*NBpixact_iiout);
    YoutarrayActive = (float *) malloc(sizeof(float)*NBpixact_jjout);
	
	for(pixiiout=0; pixiiout<NBpixact_iiout; pixiiout++)
	{
		iiout = iioutarrayActive[pixiiout];
		XoutarrayActive[pixiiout] = (1.0/Zfactor) * (1.0*iiout/xsize-0.5) * xsize;
	}
	
	for(pixjjout=0; pixjjout<NBpixact_jjout; pixjjout++)
	{
		jjout = jjoutarrayActive[pixjjout];
		YoutarrayActive[pixjjout] = (1.0/Zfactor) * (1.0*jjout/ysize-0.5) * ysize;
	}	
	
    printf("%ld output points (%ld %ld) \n", NBptsout, NBpixact_iiout, NBpixact_jjout);



    iioutarray = (uint_fast32_t *) malloc(sizeof(uint_fast32_t)*NBptsout);
    jjoutarray = (uint_fast32_t *) malloc(sizeof(uint_fast32_t)*NBptsout);
    xoutarray = (double *) malloc(sizeof(double)*NBptsout);
    youtarray = (double *) malloc(sizeof(double)*NBptsout);
    kout = 0;
    for(ii=0; ii<xsize; ii++)
        for(jj=0; jj<ysize; jj++)
        {
            val = data.image[IDoutmask].array.F[jj*xsize+ii];
            if(val>0.5)
            {
                iioutarray[kout] = ii;
                jjoutarray[kout] = jj;
                xoutarray[kout] = (1.0/Zfactor) * (1.0*ii/xsize-0.5) * xsize;
                youtarray[kout] = (1.0/Zfactor) * (1.0*jj/ysize-0.5) * ysize;
                kout++;
            }
        }

    IDout = create_2DCimage_ID(IDout_name, xsize, ysize);




	IDcosXX = create_2Dimage_ID("_cosXX", xsize, xsize);
	IDsinXX = create_2Dimage_ID("_sinXX", xsize, xsize);
	IDcosYY = create_2Dimage_ID("_cosYY", ysize, ysize);
	IDsinYY = create_2Dimage_ID("_sinYY", ysize, ysize);


printf(" <");
fflush(stdout);

    //# ifdef _OPENMP
# ifdef HAVE_LIBGOMP
    #pragma omp parallel default(shared) private(pixiiout, pixiiin, iiout, iiin, pha, cospha, sinpha)
    {
        #pragma omp for
# endif
        for(pixiiout=0; pixiiout<NBpixact_iiout; pixiiout++)
        {
			iiout = iioutarrayActive[pixiiout];
            for(pixiiin=0; pixiiin<NBpixact_iiin; pixiiin++)
            {
				iiin = iiinarrayActive[pixiiin];
                pha = 2.0*dir*M_PI*(XinarrayActive[pixiiin]*XoutarrayActive[pixiiout]);
				cospha = cosf(pha);
				sinpha = sinf(pha);
                
                data.image[IDcosXX].array.F[iiout*xsize+iiin] = cospha;
                data.image[IDsinXX].array.F[iiout*xsize+iiin] = sinpha;
               
            }
        }
# ifdef HAVE_LIBGOMP
        // # ifdef _OPENMP
    }
# endif

printf("> ");
fflush(stdout);




printf(" <");
fflush(stdout);

    //# ifdef _OPENMP
# ifdef HAVE_LIBGOMP
    #pragma omp parallel default(shared) private(pixjjout, pixjjin, jjout, jjin, pha, cospha, sinpha)
    {
        #pragma omp for
# endif
        for(pixjjout=0; pixjjout<NBpixact_jjout; pixjjout++)
        {
			jjout = jjoutarrayActive[pixjjout];
            for(pixjjin=0; pixjjin<NBpixact_jjin; pixjjin++)
            {
				jjin = jjinarrayActive[pixjjin];
                pha = 2.0*dir*M_PI*(YinarrayActive[pixjjin]*YoutarrayActive[pixjjout]);
				cospha = cosf(pha);
				sinpha = sinf(pha);
                
                data.image[IDcosYY].array.F[jjout*ysize+jjin] = cospha;
                data.image[IDsinYY].array.F[jjout*ysize+jjin] = sinpha;
               
            }
        }
# ifdef HAVE_LIBGOMP
        // # ifdef _OPENMP
    }
# endif

printf("> ");
fflush(stdout);




	


printf("<<");
fflush(stdout);


# ifdef HAVE_LIBGOMP
    #pragma omp parallel default(shared) private(kout, k, pha, re, im, cospha, sinpha, iiin, jjin, iiout, jjout, cosXX, cosYY, sinXX, sinYY, cosXY, sinXY)
    {
        #pragma omp for
# endif
        for(kout=0; kout<NBptsout; kout++)
        {
			iiout = iioutarray[kout];
			jjout = jjoutarray[kout];
			
            re = 0.0;
            im = 0.0;
            for(k=0; k<NBptsin; k++)
            {
				iiin = iiinarray[k];
				jjin = jjinarray[k];
				
				cosXX = data.image[IDcosXX].array.F[iiout*xsize + iiin];
				cosYY = data.image[IDcosYY].array.F[jjout*ysize + jjin];
				
				sinXX = data.image[IDsinXX].array.F[iiout*xsize + iiin];
				sinYY = data.image[IDsinYY].array.F[jjout*ysize + jjin];
				
				cosXY = cosXX*cosYY - sinXX*sinYY;
				sinXY = sinXX*cosYY + cosXX*sinYY;
				
				cospha = cosvalinpha[k]*cosXY - sinvalinpha[k]*sinXY;
				sinpha = sinvalinpha[k]*cosXY + cosvalinpha[k]*sinXY;
				
				
                re += valinamp[k]*cospha;
                im += valinamp[k]*sinpha;
            }
            data.image[IDout].array.CF[jjoutarray[kout]*xsize+iioutarray[kout]].re = re/Zfactor;
            data.image[IDout].array.CF[jjoutarray[kout]*xsize+iioutarray[kout]].im = im/Zfactor;
        }
# ifdef HAVE_LIBGOMP
    }
# endif


printf(">>");
fflush(stdout);

	free(cosvalinpha);
	free(sinvalinpha);
	
	delete_image_ID("_cosXX");
	delete_image_ID("_sinXX");
	delete_image_ID("_cosYY");
	delete_image_ID("_sinYY");

	free(XinarrayActive);
	free(YinarrayActive);
	free(XoutarrayActive);
	free(YoutarrayActive);

	free(iiinarrayActive);
	free(jjinarrayActive);
	free(iioutarrayActive);
	free(jjoutarrayActive);

    free(iiinarray);
    free(jjinarray);
    free(xinarray);
    free(yinarray);
    free(valinamp);
    free(valinpha);


    free(iioutarray);
    free(jjoutarray);
    free(xoutarray);
    free(youtarray);

    return(IDout);
}





//
// pupil convolution by complex focal plane mask of limited support
// typically used with fpmz = zoomed copy of 1-fpm
// high resolution focal plane mask using DFT
// zoom factor
//
// force computation over pixels >0.5 in _DFTmask00 if it exists
//
long fft_DFTinsertFPM( const char *pupin_name, const char *fpmz_name, double zfactor, const char *pupout_name)
{
    double eps = 1.0e-16;
    long ID, ID1;
    long IDpupin_mask;
    long IDfpmz;
    long IDfpmz_mask;
    long xsize, ysize, zsize;
    long IDin, IDout;
    long ii, jj, k;
    double re, im, rein, imin, amp, pha, ampin, phain, amp2;
    double x, y, r;
    double total = 0;
    long IDout2D;
    int FORCE_IMZERO = 0;
    double imresidual = 0.0;
    double tx, ty, tcx, tcy;
    long size2;

	long ID_DFTmask00;


    if(variable_ID("_FORCE_IMZERO")!=-1)
    {
        FORCE_IMZERO = 1;
        printf("---------------FORCING IMAGINARY PART TO ZERO-------------\n");
    }


	ID_DFTmask00 = image_ID("_DFTmask00");


    printf("zfactor = %f\n", zfactor);

    IDin = image_ID(pupin_name);
    xsize = data.image[IDin].md[0].size[0];
    ysize = data.image[IDin].md[0].size[1];
    if(data.image[IDin].md[0].naxis > 2)
        zsize = data.image[IDin].md[0].size[2];
    else
        zsize = 1;
    printf("zsize = %ld\n", zsize);
    size2 = xsize*ysize;

    IDout = create_3DCimage_ID(pupout_name, xsize, ysize, zsize);

    for(k=0; k<zsize; k++) // increment slice (= wavelength)
    {
        IDpupin_mask = create_2Dimage_ID("_DFTpupmask", xsize, ysize);
        for(ii=0; ii<xsize*ysize; ii++)
        {
            re = data.image[IDin].array.CF[k*size2+ii].re;
            im = data.image[IDin].array.CF[k*size2+ii].im;
            amp2 = re*re+im*im;
            if(amp2>eps)
                data.image[IDpupin_mask].array.F[ii] = 1.0;
            else
                data.image[IDpupin_mask].array.F[ii] = 0.0;
        }
        
        if(ID_DFTmask00 != -1)
			for(ii=0; ii<xsize*ysize; ii++)
			{
				if(data.image[ID_DFTmask00].array.F[ii] > 0.5)
					data.image[IDpupin_mask].array.F[ii] = 1.0;
			}

        IDfpmz = image_ID(fpmz_name);
        IDfpmz_mask = create_2Dimage_ID("_fpmzmask", xsize, ysize);
        for(ii=0; ii<xsize*ysize; ii++)
        {
            re = data.image[IDfpmz].array.CF[k*size2+ii].re;
            im = data.image[IDfpmz].array.CF[k*size2+ii].im;
            amp2 = re*re+im*im;
            if(amp2>eps)
                data.image[IDfpmz_mask].array.F[ii] = 1.0;
            else
                data.image[IDfpmz_mask].array.F[ii] = 0.0;
        }

	//	save_fits("_DFTpupmask", "!_DFTpupmask.fits");



        fft_DFT( pupin_name, "_DFTpupmask", "_foc0", "_fpmzmask", zfactor, -1, k);

        ID = image_ID("_foc0");
        total = 0.0;
        tx = 0.0;
        ty = 0.0;
        tcx = 0.0;
        tcy = 0.0;
        for(ii=0; ii<xsize; ii++)
            for(jj=0; jj<ysize; jj++)
            {
                x = 1.0*ii-0.5*xsize;
                y = 1.0*jj-0.5*ysize;
                re = data.image[IDfpmz].array.CF[k*size2+jj*xsize+ii].re;
                im = data.image[IDfpmz].array.CF[k*size2+jj*xsize+ii].im;
                amp = sqrt(re*re+im*im);
                pha = atan2(im,re);

                rein = data.image[ID].array.CF[jj*xsize+ii].re;
                imin = data.image[ID].array.CF[jj*xsize+ii].im;
                ampin = sqrt(rein*rein+imin*imin);
                phain = atan2(imin, rein);

                ampin *= amp;
                total += ampin*ampin;
                phain += pha;

                data.image[ID].array.CF[jj*xsize+ii].re = ampin*cos(phain);
                data.image[ID].array.CF[jj*xsize+ii].im = ampin*sin(phain);

                tx += x*ampin*sin(phain)*ampin;
                ty += y*ampin*sin(phain)*ampin;
                tcx += x*x*ampin*ampin;
                tcy += y*y*ampin*ampin;
            }
        printf("TX TY = %.18lf %.18lf", tx/tcx, ty/tcy);
        if(FORCE_IMZERO==1) // Remove tip-tilt in focal plane mask imaginary part
        {
            tx = 0.0;
            ty = 0.0;
            for(ii=0; ii<xsize; ii++)
                for(jj=0; jj<ysize; jj++)
                {
                    x = 1.0*ii-0.5*xsize;
                    y = 1.0*jj-0.5*ysize;

                    re = data.image[ID].array.CF[jj*xsize+ii].re;
                    im = data.image[ID].array.CF[jj*xsize+ii].im;
                    amp = sqrt(re*re+im*im);

                    data.image[ID].array.CF[jj*xsize+ii].im -= amp*(x*tx/tcx + y*ty/tcy);
                    tx += x*data.image[ID].array.CF[jj*xsize+ii].im*amp;
                    ty += y*data.image[ID].array.CF[jj*xsize+ii].im*amp;
                }
            printf("  ->   %.18lf %.18lf", tx/tcx, ty/tcy);

            mk_amph_from_complex("_foc0","_foc0_amp","_foc0_pha", 0);
            save_fl_fits("_foc0_amp", "!_foc_amp.fits");
            save_fl_fits("_foc0_pha", "!_foc_pha.fits");
            delete_image_ID("_foc0_amp");
            delete_image_ID("_foc0_pha");
        }
        printf("\n");


        data.FLOATARRAY[0] = (float) total;

        /*  if(FORCE_IMZERO==1) // Remove tip-tilt in focal plane mask imaginary part
        {
        imresidual = 0.0;
        ID = image_ID("_foc0");
        ID1 = create_2Dimage_ID("imresidual", xsize, ysize);
        for(ii=0; ii<xsize*ysize; ii++)
        {
        data.image[ID1].array.F[ii] = data.image[ID].array.CF[ii].im;
        imresidual += data.image[ID].array.CF[ii].im*data.image[ID].array.CF[ii].im;
        data.image[ID].array.CF[ii].im = 0.0;
        }
        printf("IM RESIDUAL = %lf\n", imresidual);
        save_fl_fits("imresidual", "!imresidual.fits");
        delete_image_ID("imresidual");
        }
        */

        if(0) // TEST
        {
            mk_amph_from_complex("_foc0", "tmp_foc0_a", "tmp_foc0_p", 0);
            save_fl_fits("tmp_foc0_a", "!_DFT_foca");
            save_fl_fits("tmp_foc0_p", "!_DFT_focp");
            delete_image_ID("tmp_foc0_a");
            delete_image_ID("tmp_foc0_p");
        }


        /* for(ii=0; ii<xsize; ii++)
        for(jj=0; jj<ysize; jj++)
         {
         x = 1.0*ii-xsize/2;
         y = 1.0*jj-ysize/2;
         r = sqrt(x*x+y*y);
         if(r<150.0)
         data.image[IDpupin_mask].array.F[jj*xsize+ii] = 1.0;
         }*/

        fft_DFT( "_foc0", "_fpmzmask", "_pupout2D", "_DFTpupmask", zfactor, 1, 0);

	//	save_fits("_DFTpupmask", "!test_DFTpupmask.fits");//TEST

        IDout2D = image_ID("_pupout2D");
        for(ii=0; ii<xsize*ysize; ii++)
        {
            data.image[IDout].array.CF[k*xsize*ysize+ii].re = data.image[IDout2D].array.CF[ii].re/(xsize*ysize);
            data.image[IDout].array.CF[k*xsize*ysize+ii].im = data.image[IDout2D].array.CF[ii].im/(xsize*ysize);
        }
        delete_image_ID("_pupout2D");
        delete_image_ID("_foc0");

        delete_image_ID("_DFTpupmask");
        delete_image_ID("_fpmzmask");
    }


    return(IDout);
}


//
// pupil convolution by real focal plane mask of limited support
// typically used with fpmz = zoomed copy of 1-fpm
// high resolution focal plane mask using DFT
// zoom factor
//
//
//
long fft_DFTinsertFPM_re( const char *pupin_name, const char *fpmz_name, double zfactor, const char *pupout_name)
{
    double eps = 1.0e-10;
    long ID;
    long IDpupin_mask;
    long IDfpmz;
    long IDfpmz_mask;
    long xsize, ysize;
    long IDin, IDout;
    long ii, jj;
    double re, im, rein, imin, amp, pha, ampin, phain, amp2;
    double x, y, r;
    double total = 0;
	char fname[200];
	long ID_DFTmask00;

    IDin = image_ID(pupin_name);
    xsize = data.image[IDin].md[0].size[0];
    ysize = data.image[IDin].md[0].size[1];


	ID_DFTmask00 = image_ID("_DFTmask00");

    printf("zfactor = %f\n", zfactor);

    IDpupin_mask = create_2Dimage_ID("_DFTpupmask", xsize, ysize);
    for(ii=0; ii<xsize*ysize; ii++)
    {
        re = data.image[IDin].array.CF[ii].re;
        im = data.image[IDin].array.CF[ii].im;
        amp2 = re*re+im*im;
        if(amp2>eps)
            data.image[IDpupin_mask].array.F[ii] = 1.0;
        else
            data.image[IDpupin_mask].array.F[ii] = 0.0;
    }
    //  save_fl_fits("_DFTpupmask", "!_DFTpupmask.fits");

       if(ID_DFTmask00 != -1)
			for(ii=0; ii<xsize*ysize; ii++)
			{
				if(data.image[ID_DFTmask00].array.F[ii] > 0.5)
					data.image[IDpupin_mask].array.F[ii] = 1.0;
			}



    IDfpmz = image_ID(fpmz_name);
    IDfpmz_mask = create_2Dimage_ID("_fpmzmask", xsize, ysize);
    for(ii=0; ii<xsize*ysize; ii++)
    {
        amp = fabs(data.image[IDfpmz].array.F[ii]);
        if(amp>eps)
            data.image[IDfpmz_mask].array.F[ii] = 1.0;
        else
            data.image[IDfpmz_mask].array.F[ii] = 0.0;
    }

    fft_DFT( pupin_name, "_DFTpupmask", "_foc0", "_fpmzmask", zfactor, -1, 0);

    ID = image_ID("_foc0");
    total = 0.0;
    for(ii=0; ii<xsize*ysize; ii++)
    {
        amp = data.image[IDfpmz].array.F[ii];

        rein = data.image[ID].array.CF[ii].re;
        imin = data.image[ID].array.CF[ii].im;
        ampin = sqrt(rein*rein+imin*imin);
        phain = atan2(imin, rein);

        ampin *= amp;
        total += ampin*ampin;

        data.image[ID].array.CF[ii].re = ampin*cos(phain);
        data.image[ID].array.CF[ii].im = ampin*sin(phain);
    }

    data.FLOATARRAY[0] = (float) total;


    if(1) // TEST
    {
        mk_amph_from_complex("_foc0", "tmp_foc0_a", "tmp_foc0_p", 0);
        sprintf(fname, "!%s/_DFT_foca", data.SAVEDIR);
		save_fl_fits("tmp_foc0_a", fname);
        sprintf(fname, "!%s/_DFT_focp", data.SAVEDIR);		
        save_fl_fits("tmp_foc0_p", fname);
        delete_image_ID("tmp_foc0_a");
        delete_image_ID("tmp_foc0_p");
    }

    /* for(ii=0; ii<xsize; ii++)
      for(jj=0; jj<ysize; jj++)
        {
    x = 1.0*ii-xsize/2;
    y = 1.0*jj-ysize/2;
    r = sqrt(x*x+y*y);
    if(r<150.0)
      data.image[IDpupin_mask].array.F[jj*xsize+ii] = 1.0;
      }*/

    fft_DFT( "_foc0", "_fpmzmask", pupout_name, "_DFTpupmask", zfactor, 1, 0);

    IDout = image_ID(pupout_name);
    for(ii=0; ii<xsize*ysize; ii++)
    {
        data.image[IDout].array.CF[ii].re /= xsize*ysize;
        data.image[IDout].array.CF[ii].im /= xsize*ysize;
    }

    delete_image_ID("_foc0");

    delete_image_ID("_DFTpupmask");
    delete_image_ID("_fpmzmask");

    return(IDout);
}


/*^-----------------------------------------------------------------------------
| int
| arith_image_translate :
|
|   char *ID_name       :
|   char *ID_out        :
|   double xtransl   :
|   double ytransl   :
|
| COMMENT:  Inclusion of this routine requires inclusion of modules:
|           fft, gen_image
+-----------------------------------------------------------------------------*/
int fft_image_translate(const char *ID_name, const char *ID_out, double xtransl, double ytransl)
{
    long ID;
    long naxes[2];
    //  int n0,n1;

    fprintf( stdout, "[arith_image_translate %f %f]\n", xtransl, ytransl);

    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];
    // n0 = (int) ((log10(naxes[0])/log10(2))+0.01);
    // n1 = (int) ((log10(naxes[0])/log10(2))+0.01);

    //  if ((n0==n1)&&(naxes[0]==(int) pow(2,n0))&&(naxes[1]==(int) pow(2,n1)))
    // {
    do2drfft(ID_name,"ffttmp1");
    mk_amph_from_complex("ffttmp1","amptmp","phatmp", 0);
    delete_image_ID("ffttmp1");
    arith_make_slopexy("sltmp", naxes[0], naxes[1], xtransl*2.0*M_PI/naxes[0], ytransl*2.0*M_PI/naxes[1]);
    permut("sltmp");
    arith_image_add("phatmp","sltmp","phatmp1");
    delete_image_ID("phatmp");
    delete_image_ID("sltmp");
    mk_complex_from_amph("amptmp", "phatmp1", "ffttmp2", 0);
    delete_image_ID("amptmp");
    delete_image_ID("phatmp1");
    do2dffti("ffttmp2", "ffttmp3");
    delete_image_ID("ffttmp2");
    mk_reim_from_complex("ffttmp3","retmp","imtmp", 0);
    arith_image_cstmult("retmp", 1.0/naxes[0]/naxes[1], ID_out);
    delete_image_ID("ffttmp3");
    delete_image_ID("retmp");
    delete_image_ID("imtmp");
    // }
    // else
    //{
    // printf("Error: image size does not allow translation\n");
    //}

    return(0);
}


