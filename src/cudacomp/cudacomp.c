#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <sched.h>
#include <signal.h> 


#include <semaphore.h>

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



#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>

#include <assert.h>



#ifdef HAVE_CUDA

#include <cuda_runtime_api.h>
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <device_types.h>
#include <pthread.h>
#include <cusolverDn.h>

#endif


#ifdef HAVE_MAGMA

#include "magma.h"
#include "magma_v2.h"
#include "magma_lapack.h"

#endif



#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "info/info.h"
#include "cudacomp/cudacomp.h"

#include "linopt_imtools/linopt_imtools.h" // for testing


#define min(a,b) (((a)<(b))?(a):(b))


# ifdef _OPENMP
# include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
# endif

int FORCESEMINIT = 1;




extern DATA data;
extern pid_t CLIPID;


static struct timespec tnow;
static struct timespec tdiff;
static double tdiffv;

static int IDtimerinit = 0;
static long IDtiming = -1; // index to image where timing should be written




#ifdef HAVE_CUDA
static int deviceCount;

GPUMATMULTCONF gpumatmultconf[20]; // supports up to 20 configurations per process


static cudaError_t error;
static cublasStatus_t stat;
static float cublasSgemv_alpha = 1.0;
static float cublasSgemv_beta  = 0.0;


#endif




// MAGMA global variables

#ifdef HAVE_MAGMA

static int INIT_MAGMA = 0;

// queue for default magma device
static magma_queue_t   magmaqueue;

static long MAGMAloop_iter = 0;

static double *magma_h_A;
static double *magma_d_A;
static double *magma_d_AtA;
static double *magma_h_AtA;
static double *magma_w1; // eigenvalues
static double *magma_h_R;
static double *magma_h_work;
static double *magma_d_VT1;
static double *magma_h_VT1;
static double *magma_d_M2;
static double *magma_d_Ainv;
static double *magma_h_Ainv;
static double *magma_h_M2;


static float *magmaf_h_A;
static float *magmaf_d_A;
static float *magmaf_d_AtA;
static float *magmaf_h_AtA;
static float *magmaf_w1; // eigenvalues
static float *magmaf_h_R;
static float *magmaf_h_work;
static float *magmaf_d_VT1;
static float *magmaf_h_VT1;
static float *magmaf_d_M2;
static float *magmaf_d_Ainv;
static float *magmaf_h_Ainv;
static float *magmaf_h_M2;


static magma_int_t magma_aux_iwork[1];
static magma_int_t magma_lwork, magma_liwork;
static magma_int_t *magma_iwork;


#endif





// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string 
//

#ifdef HAVE_CUDA

int_fast8_t CUDACOMP_test_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)
        GPUcomp_test(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
    else
        return 1;
}

int_fast8_t CUDACOMP_Coeff2Map_Loop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,2)+CLI_checkarg(4,4)==0)
        CUDACOMP_Coeff2Map_Loop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, 0, " ");
    else
        return 1;
}


int_fast8_t CUDACOMP_Coeff2Map_offset_Loop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,2)+CLI_checkarg(4,4)+CLI_checkarg(5,4)==0)
        CUDACOMP_Coeff2Map_Loop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, 1, data.cmdargtoken[5].val.string);
    else
        return 1;
}


int_fast8_t CUDACOMP_extractModesLoop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,5)+CLI_checkarg(3,4)+CLI_checkarg(4,4)+CLI_checkarg(5,5)+CLI_checkarg(6,5)+CLI_checkarg(7,2)+CLI_checkarg(8,2)+CLI_checkarg(9,2)+CLI_checkarg(10,2)+CLI_checkarg(11,2)+CLI_checkarg(12,2)+CLI_checkarg(13,2)==0)
        CUDACOMP_extractModesLoop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.string, data.cmdargtoken[6].val.string, data.cmdargtoken[7].val.numl, data.cmdargtoken[8].val.numl, data.cmdargtoken[9].val.numl, data.cmdargtoken[10].val.numl, data.cmdargtoken[11].val.numl, data.cmdargtoken[12].val.numl, data.cmdargtoken[13].val.numl);
    else
        return 1;
}
#endif



int_fast8_t init_cudacomp()
{
    long i;
#ifdef HAVE_CUDA
    for(i=0; i<10; i++) {
        gpumatmultconf[i].init = 0;
        gpumatmultconf[i].alloc = 0;
    }
#endif

    strcpy(data.module[data.NBmodule].name,__FILE__);
    strcpy(data.module[data.NBmodule].info,"CUDA wrapper for AO loop");
    data.NBmodule++;


#ifdef HAVE_CUDA
    strcpy(data.cmd[data.NBcmd].key,"cudacompinit");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = CUDACOMP_init;
    strcpy(data.cmd[data.NBcmd].info,"init CUDA comp");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"cudacompinit");
    strcpy(data.cmd[data.NBcmd].Ccall,"int CUDACOMP_init()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"cudacomptest");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = CUDACOMP_test_cli;
    strcpy(data.cmd[data.NBcmd].info,"test CUDA comp");
    strcpy(data.cmd[data.NBcmd].syntax,"<NB actuators [long]> <NB modes [long]> <NB pixels [long]> <NB GPU [long]>");
    strcpy(data.cmd[data.NBcmd].example,"cudacomptest 1000 20 1000 1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int GPUcomp_test(long NBact, long NBmodes, long WFSsize, long GPUcnt)");
    data.NBcmd++;
        
    strcpy(data.cmd[data.NBcmd].key,"cudacoeff2map");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = CUDACOMP_Coeff2Map_Loop_cli;
    strcpy(data.cmd[data.NBcmd].info,"CUDA multiply vector by modes");
    strcpy(data.cmd[data.NBcmd].syntax,"<modes> <coeffs vector> <GPU index [long]> <output map>");
    strcpy(data.cmd[data.NBcmd].example,"cudacoeff2map modes coeff 4 outmap");
    strcpy(data.cmd[data.NBcmd].Ccall,"int CUDACOMP_Coeff2Map_Loop(const char *IDmodes_name, const char *IDcoeff_name, int GPUindex, const char *IDoutmap_name, int offsetmode, const char *IDoffset_name)");
    data.NBcmd++;
    
    strcpy(data.cmd[data.NBcmd].key,"cudacoeffo2map");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = CUDACOMP_Coeff2Map_offset_Loop_cli;
    strcpy(data.cmd[data.NBcmd].info,"CUDA multiply vector by modes and add offset");
    strcpy(data.cmd[data.NBcmd].syntax,"<modes> <coeffs vector> <GPU index [long]> <output map> <offset image>");
    strcpy(data.cmd[data.NBcmd].example,"cudacoeffo2map modes coeff 4 outmap offsetim");
    strcpy(data.cmd[data.NBcmd].Ccall,"int CUDACOMP_Coeff2Map_Loop(const char *IDmodes_name, const char *IDcoeff_name, int GPUindex, const char *IDoutmap_name, int offsetmode, const char *IDoffset_name)");
    data.NBcmd++;
    
    strcpy(data.cmd[data.NBcmd].key,"cudaextrmodes");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = CUDACOMP_extractModesLoop_cli;
    strcpy(data.cmd[data.NBcmd].info,"CUDA extract mode values loop. Note that intot and refout parameters can be NULL");
    strcpy(data.cmd[data.NBcmd].syntax,"<inval stream> <intot stream> <modes> <refin val> <refout_val> <outmode vals> <GPU index [long]> <PROCESS flag> <TRACEMODE flag> <MODE norm flag> <input semaphore> <axis orientation> <twait [us]>");
    strcpy(data.cmd[data.NBcmd].example,"cudaextrmodes inmap inmaptot modes imref imoutref modeval 3 1 1 1 3 0 0");
    strcpy(data.cmd[data.NBcmd].Ccall,"int CUDACOMP_extractModesLoop(const char *in_stream, const char *intot_stream, const char *IDmodes_name, const char *IDrefin_name, const char *IDrefout_name, const char *IDmodes_val_name, int GPUindex, int PROCESS, int TRACEMODE, int MODENORM, int insem, int axmode, long twait)");
    data.NBcmd++;
    

#endif
    // add atexit functions here

    return 0;
}










#ifdef HAVE_CUDA
int_fast8_t CUDACOMP_init()
{
    int device;
    struct cudaDeviceProp deviceProp;

    cudaGetDeviceCount(&deviceCount);
    printf("%d devices found\n", deviceCount);
    printf("\n");
    for (device = 0; device < deviceCount; ++device) {
        cudaGetDeviceProperties(&deviceProp, device);
        printf("Device %d [ %20s ]  has compute capability %d.%d.\n",
               device, deviceProp.name, deviceProp.major, deviceProp.minor);
        printf("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n", (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
        printf("  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
        printf("\n");
        #ifdef HAVE_MAGMA
		printf("Using MAGMA library\n");
		magma_print_environment();
		#endif

        printf("\n");
    }

    return(0);
}

















////////////////////////////////////////////////////////////////////////////////
//! Compute reference data set matrix multiply on CPU
//! dmVec = cMat * wfsVec
////////////////////////////////////////////////////////////////////////////////


void matrixMulCPU(float *cMat, float *wfsVec, float *dmVec, int M, int N)
{
    long n, m;
    float sum;
    long index;
    long i;

    printf("Conventional mat mult %d %d\n", M, N);
    for(m=0; m<M; m++)
    {
        dmVec[m] = 0.0;
        for(n=0; n<N; n++)
        {
            index = m*N+n;
            dmVec[m] += cMat[index]*wfsVec[n];
        }
        //cMat[n*M+m]*wfsVec[n];
    }

    printf("cMat  : ");
    for(i=0; i<5; i++)
        printf("%f ", cMat[i]);
    printf(" ... ");
    for(i=N*M-5; i<N*M; i++)
        printf("%f ", cMat[i]);
    printf("\n");

    printf("wfsVec: ");
    for(n=0; n<5; n++)
        printf("%f ", wfsVec[n]);
    printf(" ... ");
    for(n=N-5; n<N; n++)
        printf("%f ", wfsVec[n]);
    printf("\n");

}







int GPUloadCmat(int index)
{
    int device;
    int n, m;
   // long ID;
    
    
    printf("LOADING MATRIX TO GPU ... ");
    fflush(stdout);

    // TEST
  //  ID = create_2Dimage_ID("mgputest", gpumatmultconf[index].N, gpumatmultconf[index].M);
   // for(n=0;n<gpumatmultconf[index].N;n++)
    //    for(m=0;m<gpumatmultconf[index].M;m++)
     //       data.image[ID].array.F[m*gpumatmultconf[index].N+n] = gpumatmultconf[index].cMat[m*gpumatmultconf[index].N+n];
    //save_fits("mgputest","!MgpuTest.fits");
    //delete_image_ID("mgputest");
    

    for(device = 0; device < gpumatmultconf[index].NBstreams; device++)
    {
        for (n=gpumatmultconf[index].Noffset[device]; n<gpumatmultconf[index].Noffset[device]+gpumatmultconf[index].Nsize[device]; n++) {
            if(gpumatmultconf[index].orientation==0)
            {
                for (m=0; m<gpumatmultconf[index].M; m++) {
                    gpumatmultconf[index].cMat_part[device][(n-gpumatmultconf[index].Noffset[device])*gpumatmultconf[index].M+m] = gpumatmultconf[index].cMat[m*gpumatmultconf[index].N+n];
                }
            }
            else
            {
                for (m=0; m<gpumatmultconf[index].M; m++) {
                    gpumatmultconf[index].cMat_part[device][(n-gpumatmultconf[index].Noffset[device])*gpumatmultconf[index].M+m] = gpumatmultconf[index].cMat[n*gpumatmultconf[index].M+m];
                }
            }
        }
    }

    for(device=0; device<gpumatmultconf[index].NBstreams; device++)
    {
        cudaSetDevice(gpumatmultconf[index].GPUdevice[device]);
        error = cublasSetMatrix (gpumatmultconf[index].M, gpumatmultconf[index].Nsize[device], sizeof(float), gpumatmultconf[index].cMat_part[device], gpumatmultconf[index].M, gpumatmultconf[index].d_cMat[device], gpumatmultconf[index].M);
        if (error != cudaSuccess)
        {
            printf("cudblasSetMatrix returned error code %d, line(%d)\n", error, __LINE__);
            exit(EXIT_FAILURE);
        }
    }
    printf("done\n");
    fflush(stdout);

    return(0);
}






/** setup matrix multiplication using multiple GPUs */
/*
 * 
 *  IDoutdmmodes_name  = alpha * IDcontrM_name x IDwfsim_name
 * 
 * upon setup, IDwfsim_name is the WFS ref and initWFSref = 0
 * 
*/

int GPU_loop_MultMat_setup(int index, const char *IDcontrM_name, const char *IDwfsim_name, const char *IDoutdmmodes_name, long NBGPUs, int *GPUdevice, int orientation, int USEsem, int initWFSref, long loopnb)
{
    long IDcontrM, IDwfsim, IDwfsref;
    long *sizearraytmp;
    int device;
    struct cudaDeviceProp deviceProp;
    int n, m;
    char sname[200];
    char name[200];
    int ptn;

    long cnt0;
    long cnt;

    long NBiter = 100000;
    long iter = 0;
    
    int cmatdim = 2; // 2D or 3D
    

    

     
    if(gpumatmultconf[index].init == 0)
    {

        printf("STARTING SETUP %d .....\n", index);
        fflush(stdout);
    
 
        if(IDtimerinit == 0)
            {
                sprintf(name, "aol%ld_looptiming", loopnb);
                IDtiming = image_ID(name);
            
            if(IDtiming==-1)
                IDtiming = create_2Dimage_ID(name, 50, 1);
            }
 
 
        
        if(gpumatmultconf[index].alloc == 1)
        {
            GPU_loop_MultMat_free(index);
            gpumatmultconf[index].alloc = 0;
        }

        if(USEsem==1)
            gpumatmultconf[index].sem = 1;
        else
            gpumatmultconf[index].sem = 0;

        printf("USEsem = %d\n", USEsem);
        fflush(stdout);
        


        gpumatmultconf[index].orientation = orientation;

        printf("input CM name : %s\n", IDcontrM_name);
        fflush(stdout);
        //sleep(2);
        gpumatmultconf[index].CM_ID = image_ID(IDcontrM_name);

        printf("CM_ID = %ld\n", gpumatmultconf[index].CM_ID);
        fflush(stdout);
        //	sleep(2);

        gpumatmultconf[index].CM_cnt = data.image[gpumatmultconf[index].CM_ID].md[0].cnt0;



        /// Load Control Matrix
        IDcontrM = image_ID(IDcontrM_name);
        //		printf("control matrix loaded: IDcontrM = %ld\n", IDcontrM);
        //        fflush(stdout);
        //	sleep(2);


        if(orientation==0)
        {
            if(data.image[IDcontrM].md[0].naxis==3)
                {
                    gpumatmultconf[index].M = data.image[IDcontrM].md[0].size[2];
                    gpumatmultconf[index].N = data.image[IDcontrM].md[0].size[0] * data.image[IDcontrM].md[0].size[1];
                    cmatdim = 3;
                }
            else
                {
                    gpumatmultconf[index].M = data.image[IDcontrM].md[0].size[1];
                    gpumatmultconf[index].N = data.image[IDcontrM].md[0].size[0];
                    cmatdim = 2;
               }
            printf("[0] [%ld] M = %d\n", IDcontrM, (int) gpumatmultconf[index].M);
            printf("[0] [%ld] N = %d\n", IDcontrM, (int) gpumatmultconf[index].N);
        }
        else
        {
            if(data.image[IDcontrM].md[0].naxis==3)
            {
                gpumatmultconf[index].M = data.image[IDcontrM].md[0].size[0] * data.image[IDcontrM].md[0].size[1];
                gpumatmultconf[index].N = data.image[IDcontrM].md[0].size[2];
                cmatdim = 3;
            }
            else
            {
                  gpumatmultconf[index].M = data.image[IDcontrM].md[0].size[0];
                gpumatmultconf[index].N = data.image[IDcontrM].md[0].size[1];              
                cmatdim = 2;
            }

            printf("[1] [%ld] M = %d\n", IDcontrM, (int) gpumatmultconf[index].M);
            printf("[1] [%ld] N = %d\n", IDcontrM, (int) gpumatmultconf[index].N);
        }

        gpumatmultconf[index].cMat =  data.image[IDcontrM].array.F;
        

        /// Load Input vectors
        IDwfsim = image_ID(IDwfsim_name);
        gpumatmultconf[index].wfsVec = data.image[IDwfsim].array.F;
        IDwfsref = image_ID(IDwfsim_name);
        gpumatmultconf[index].wfsRef = data.image[IDwfsref].array.F;

        if(orientation == 0)
        {            
            printf("[0] Input vector size: %ld %ld\n", data.image[IDwfsim].md[0].size[0], data.image[IDwfsim].md[0].size[1]);
            
            if(data.image[IDwfsim].md[0].size[0]*data.image[IDwfsim].md[0].size[1] != (int) gpumatmultconf[index].N)
            {
                printf("ERROR: CONTRmat and WFSvec size not compatible: %ld %d\n", data.image[IDwfsim].md[0].size[0]*data.image[IDwfsim].md[0].size[1], (int) gpumatmultconf[index].N);
                fflush(stdout);
                sleep(2);
                exit(0);
            }
        }
        else
        {
            printf("[1] Input vector size: %ld \n", data.image[IDwfsim].md[0].size[0]);
            if(data.image[IDwfsim].md[0].size[0] != (int) gpumatmultconf[index].N)
            {
                printf("ERROR: CONTRmat and WFSvec size not compatible: %ld %d\n", data.image[IDwfsim].md[0].size[0], (int) gpumatmultconf[index].N);
                fflush(stdout);
                sleep(2);
                exit(0);
            }
        }
        

		printf("Setting up gpumatmultconf\n");
		fflush(stdout);
		
        if((gpumatmultconf[index].IDout = image_ID(IDoutdmmodes_name)) == -1)
        {
            sizearraytmp = (long*) malloc(sizeof(long)*2);
            sizearraytmp[0] = gpumatmultconf[index].M;
            sizearraytmp[1] = 1;
            gpumatmultconf[index].IDout = create_image_ID(IDoutdmmodes_name, 2, sizearraytmp, FLOAT, 1, 10);
            free(sizearraytmp);
        }
        else
        {
            if(data.image[gpumatmultconf[index].IDout].md[0].size[0] * data.image[gpumatmultconf[index].IDout].md[0].size[1] != (int) gpumatmultconf[index].M)
            {
                printf("ERROR: CONTRmat and WFSvec size not compatible: %ld %d\n", data.image[gpumatmultconf[index].IDout].md[0].size[0] * data.image[gpumatmultconf[index].IDout].md[0].size[1], (int) gpumatmultconf[index].M);
                printf("gpumatmultconf[index].IDout = %ld\n", gpumatmultconf[index].IDout);
                list_image_ID();
                fflush(stdout);
                sleep(2);
                exit(0);
            }
        }

        gpumatmultconf[index].dmVecTMP = data.image[gpumatmultconf[index].IDout].array.F;


		printf("Scanning for GPU devices ...\n");
		fflush(stdout);

        cudaGetDeviceCount(&deviceCount);
        printf("%d devices found\n", deviceCount);
        fflush(stdout);
        
        printf("\n");
        for (device = 0; device < deviceCount; ++device) {
            cudaGetDeviceProperties(&deviceProp, device);
            printf("Device %d [ %20s ]  has compute capability %d.%d.\n",
                   device, deviceProp.name, deviceProp.major, deviceProp.minor);
            printf("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n", (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
            printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
            printf("  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
            printf("\n");
        }

		printf("Done scanning for GPU devices\n");
		fflush(stdout);

        gpumatmultconf[index].NBstreams = deviceCount;
        if(NBGPUs<deviceCount)
            gpumatmultconf[index].NBstreams = NBGPUs;
        



        gpumatmultconf[index].Nsize = (uint_fast32_t*) malloc(sizeof(long)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].Noffset = (uint_fast32_t*) malloc(sizeof(long)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].Noffset[0] = 0;
        for(device=1; device<gpumatmultconf[index].NBstreams; device++)
        {
            gpumatmultconf[index].Noffset[device] = gpumatmultconf[index].Noffset[device-1] + (long) (gpumatmultconf[index].N/gpumatmultconf[index].NBstreams);
            gpumatmultconf[index].Nsize[device-1] = gpumatmultconf[index].Noffset[device] - gpumatmultconf[index].Noffset[device-1];
        }
        gpumatmultconf[index].Nsize[gpumatmultconf[index].NBstreams-1] = gpumatmultconf[index].N-gpumatmultconf[index].Noffset[gpumatmultconf[index].NBstreams-1];
     
     
        printf("Allocating physical GPU(s) to stream(s) (index %d, NBGPU(s) = %ld)\n", index, NBGPUs);
        printf("%d stream(s)\n", gpumatmultconf[index].NBstreams);
        fflush(stdout);
     
        gpumatmultconf[index].GPUdevice = (int*) malloc(sizeof(int)*NBGPUs);
            
        printf("- - - - - - - - -\n");
        fflush(stdout);    
        
        for (device = 0; device < gpumatmultconf[index].NBstreams; device++)
        {
            printf("stream %2d  ->  GPU device %2d\n", device, GPUdevice[device]);
            fflush(stdout);
            gpumatmultconf[index].GPUdevice[device] = GPUdevice[device];
        }

        printf("-----------------------------------------------------\n");
        fflush(stdout);
        for(device=0; device<gpumatmultconf[index].NBstreams; device++)
        {
            printf("DEVICE %2d  [%2d]:  %5d -> %5d  (%d)\n", device, gpumatmultconf[index].GPUdevice[device], (int) gpumatmultconf[index].Noffset[device], (int) (gpumatmultconf[index].Noffset[device]+gpumatmultconf[index].Nsize[device]), (int) gpumatmultconf[index].Nsize[device]);
            fflush(stdout);
        }
        printf("-----------------------------------------------------\n");
        fflush(stdout);



        // device (GPU)
        gpumatmultconf[index].d_cMat = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].d_wfsVec = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].d_dmVec = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].d_wfsRef = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams); // WFS reference
        gpumatmultconf[index].d_dmRef = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);  // DM reference

        gpumatmultconf[index].stream = (cudaStream_t*) malloc(sizeof(cudaStream_t)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].handle = (cublasHandle_t*) malloc(sizeof(cublasHandle_t)*gpumatmultconf[index].NBstreams);


        // host (computer)
        gpumatmultconf[index].cMat_part = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].wfsVec_part = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].dmVec_part = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].wfsRef_part = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams); // WFS reference
        gpumatmultconf[index].dmRef_part = (float **) malloc(sizeof(float*)*gpumatmultconf[index].NBstreams);  // DM reference (for checking only)

        gpumatmultconf[index].refWFSinit = (int_fast8_t*) malloc(sizeof(int)*gpumatmultconf[index].NBstreams);


        gpumatmultconf[index].semptr1 = (sem_t **) malloc(sizeof(sem_t*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].semptr2 = (sem_t **) malloc(sizeof(sem_t*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].semptr3 = (sem_t **) malloc(sizeof(sem_t*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].semptr4 = (sem_t **) malloc(sizeof(sem_t*)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].semptr5 = (sem_t **) malloc(sizeof(sem_t*)*gpumatmultconf[index].NBstreams);
 

        for(device = 0; device < gpumatmultconf[index].NBstreams; device++)
        {
            gpumatmultconf[index].cMat_part[device] = (float*) malloc(sizeof(float)*gpumatmultconf[index].M*gpumatmultconf[index].Nsize[device]);
            gpumatmultconf[index].wfsVec_part[device] = (float*) malloc(sizeof(float)*gpumatmultconf[index].Nsize[device]);
            gpumatmultconf[index].wfsRef_part[device] = (float*) malloc(sizeof(float)*gpumatmultconf[index].Nsize[device]);
            gpumatmultconf[index].dmVec_part[device] = (float*) malloc(sizeof(float)*gpumatmultconf[index].M);
            gpumatmultconf[index].dmRef_part[device] = (float*) malloc(sizeof(float)*gpumatmultconf[index].M);

            sprintf(sname, "loop%02ld_i%02d_gpu%02d_sem1", loopnb, index, GPUdevice[device]);
            if ((gpumatmultconf[index].semptr1[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr1[device], 1, 0);

            sprintf(sname, "loop%02ld_i%02d_gpu%02d_sem2", loopnb, index, GPUdevice[device]);
            if ((gpumatmultconf[index].semptr2[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr2[device], 1, 0);

            sprintf(sname, "loop%02ld_i%02d_gpu%02d_sem3", loopnb, index, GPUdevice[device]);
            if ((gpumatmultconf[index].semptr3[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr3[device], 1, 0);

            sprintf(sname, "loop%02ld_i%02d_gpu%02d_sem4", loopnb, index, GPUdevice[device]);
            if ((gpumatmultconf[index].semptr4[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr4[device], 1, 0);

            sprintf(sname, "loop%02ld_i%02d_gpu%02d_sem5", loopnb, index, GPUdevice[device]);
            if ((gpumatmultconf[index].semptr5[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr5[device], 1, 0);

        }
        

        for (device = 0; device < gpumatmultconf[index].NBstreams; device++)
        {
            cudaSetDevice(GPUdevice[device]);
            cudaStreamCreate( &gpumatmultconf[index].stream[device]);
        }

        for(device=0; device<gpumatmultconf[index].NBstreams; device++)
        {
            cudaSetDevice(GPUdevice[device]);

            // ALLOCATE MEMORY ON DEVICE

            error = cudaMalloc((void **) &gpumatmultconf[index].d_cMat[device], sizeof(float)*gpumatmultconf[index].M*gpumatmultconf[index].Nsize[device]);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_cMat returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
            else
            {
				printf("ALLOCATED gpumatmultconf[%d].d_cMat[%d] size %d x %d\n", index, device, (int) gpumatmultconf[index].M, (int) gpumatmultconf[index].Nsize[device]);
			}


            error = cudaMalloc((void **) &gpumatmultconf[index].d_wfsVec[device], sizeof(float)*gpumatmultconf[index].Nsize[device]);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_wfsVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
            else
            {
				printf("ALLOCATED gpumatmultconf[%d].d_wfsVec[%d] size %d\n", index, device, (int) gpumatmultconf[index].Nsize[device]);
			}

            error = cudaMalloc((void **) &gpumatmultconf[index].d_wfsRef[device], sizeof(float)*gpumatmultconf[index].Nsize[device]);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_wfsRef returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
            else
            {
				printf("ALLOCATED gpumatmultconf[%d].d_wfsRef[%d] size %d\n", index, device, (int) gpumatmultconf[index].Nsize[device]);
			}

            error = cudaMalloc((void **) &gpumatmultconf[index].d_dmVec[device], sizeof(float)*gpumatmultconf[index].M);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_dmVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
            else
            {
				printf("ALLOCATED gpumatmultconf[%d].d_dmVec[%d] size %d\n", index, device, (int) gpumatmultconf[index].M);
			}

            error = cudaMalloc((void **) &gpumatmultconf[index].d_dmRef[device], sizeof(float)*gpumatmultconf[index].M);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_dmVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
			else
            {
				printf("ALLOCATED gpumatmultconf[%d].d_dmRef[%d] size %d\n", index, device, (int) gpumatmultconf[index].M);
			}

            stat = cublasCreate(&gpumatmultconf[index].handle[device]);
            printf("INITIALIZED CUBLAS handle index=%d device=%d\n", index, device);
			fflush(stdout);
            if (stat != CUBLAS_STATUS_SUCCESS) {
                printf ("CUBLAS initialization failed\n");
                return EXIT_FAILURE;
            }

        }

        for(device = 0; device < gpumatmultconf[index].NBstreams; device++)
            for (n=gpumatmultconf[index].Noffset[device]; n<gpumatmultconf[index].Noffset[device]+gpumatmultconf[index].Nsize[device]; n++)
                {
                    gpumatmultconf[index].wfsVec_part[device][n-gpumatmultconf[index].Noffset[device]] = gpumatmultconf[index].wfsVec[n];
                    gpumatmultconf[index].wfsRef_part[device][n-gpumatmultconf[index].Noffset[device]] = gpumatmultconf[index].wfsRef[n];
                }

    // copy memory to devices
    for(device=0; device<gpumatmultconf[index].NBstreams; device++)
        {
            error = cudaMemcpy(gpumatmultconf[index].d_wfsVec[device], gpumatmultconf[index].wfsVec_part[device], sizeof(float)*gpumatmultconf[index].Nsize[device], cudaMemcpyHostToDevice);
            if (error != cudaSuccess)
            {
                printf("cudaMemcpy d_wfsVec wfsVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
            
            
            
            printf("COPY wfsRef_part to d_wfsRef\n");
            fflush(stdout);
            error = cudaMemcpy(gpumatmultconf[index].d_wfsRef[device], gpumatmultconf[index].wfsRef_part[device], sizeof(float)*gpumatmultconf[index].Nsize[device], cudaMemcpyHostToDevice);
            if (error != cudaSuccess)
            {
                printf("cudaMemcpy d_wfsRef wfsRef returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }
        }

        GPUloadCmat(index);


        printf("SETUP %d DONE, READY TO START COMPUTATIONS  ", index);
        fflush(stdout);

        gpumatmultconf[index].iret = (int*) malloc(sizeof(int)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].thdata = (THDATA*) malloc(sizeof(THDATA)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].threadarray = (pthread_t*) malloc(sizeof(pthread_t)*gpumatmultconf[index].NBstreams);
    
        for(m=0; m<gpumatmultconf[index].M; m++)
            gpumatmultconf[index].dmVecTMP[m] = 0.0;

        cnt = 0;
        iter = 0;
        gpumatmultconf[index].init = 1;

        printf(". . . \n");
        fflush(stdout);
    }
    
    for(device=0; device<gpumatmultconf[index].NBstreams; device++)
       gpumatmultconf[index].refWFSinit[device] = initWFSref;
    
   // printf("CONFIGURATION DONE \n");
   // fflush(stdout);
    
    return(0);
}







// increments status by 4
int GPU_loop_MultMat_execute(int index, int_fast8_t *status, int_fast8_t *GPUstatus, float alpha, float beta, int timing)
{
    int m;
    int ptn;
    int statustot;
    int semval;
    long cnt;


    cublasSgemv_alpha = alpha;
    cublasSgemv_beta = beta;

	// flush semaphores
    for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
    {
        sem_getvalue(gpumatmultconf[index].semptr1[ptn], &semval);
        for(cnt=0; cnt<semval; cnt++)
            sem_trywait(gpumatmultconf[index].semptr1[ptn]);

        sem_getvalue(gpumatmultconf[index].semptr2[ptn], &semval);
        for(cnt=0; cnt<semval; cnt++)
            sem_trywait(gpumatmultconf[index].semptr2[ptn]);

        sem_getvalue(gpumatmultconf[index].semptr3[ptn], &semval);
        for(cnt=0; cnt<semval; cnt++)
            sem_trywait(gpumatmultconf[index].semptr3[ptn]);

        sem_getvalue(gpumatmultconf[index].semptr4[ptn], &semval);
        for(cnt=0; cnt<semval; cnt++)
            sem_trywait(gpumatmultconf[index].semptr4[ptn]);

        sem_getvalue(gpumatmultconf[index].semptr5[ptn], &semval);
        for(cnt=0; cnt<semval; cnt++)
            sem_trywait(gpumatmultconf[index].semptr5[ptn]);
    }


    if(timing==1)
    {
        *status = *status + 1;  // ->7
        clock_gettime(CLOCK_REALTIME, &tnow);
        tdiff = info_time_diff(data.image[IDtiming].md[0].wtime, tnow);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
        data.image[IDtiming].array.F[*status] = tdiffv;
    }

//    if((index==0)||(index==2)) /// main CM multiplication loop
//    {

        if(gpumatmultconf[index].CM_cnt != data.image[gpumatmultconf[index].CM_ID].md[0].cnt0)
            if(data.image[gpumatmultconf[index].CM_ID].md[0].write == 0)
            {
                printf("New CM detected (cnt : %ld)\n", data.image[gpumatmultconf[index].CM_ID].md[0].cnt0);
                GPUloadCmat(index);
                gpumatmultconf[index].CM_cnt = data.image[gpumatmultconf[index].CM_ID].md[0].cnt0;
            }
 //   }



    // index is the matrix multiplication index (unique to each matrix multiplication stream operation)
    // ptn is the thread number = GPU device number


    //    if((gpumatmultconf[index].sem==0)||



    if(gpumatmultconf[index].gpuinit==0)
    {
        printf("GPU pthread create, index = %d    %d %d\n", index, gpumatmultconf[index].sem, gpumatmultconf[index].gpuinit);//TEST
        fflush(stdout);

        for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
        {
            gpumatmultconf[index].thdata[ptn].thread_no = ptn;
            gpumatmultconf[index].thdata[ptn].numl0 = ptn*ptn;
            gpumatmultconf[index].thdata[ptn].cindex = index;
            gpumatmultconf[index].thdata[ptn].status = GPUstatus;
            gpumatmultconf[index].iret[ptn] = pthread_create( &gpumatmultconf[index].threadarray[ptn], NULL, compute_function, (void*) &gpumatmultconf[index].thdata[ptn]);
            if(gpumatmultconf[index].iret[ptn])
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n", gpumatmultconf[index].iret[ptn]);
                exit(EXIT_FAILURE);
            }
        }
        gpumatmultconf[index].gpuinit = 1;
    }

    if(timing == 1)
    {
        *status = *status + 1;  // -> 8
        clock_gettime(CLOCK_REALTIME, &tnow);
        tdiff = info_time_diff(data.image[IDtiming].md[0].wtime, tnow);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
        data.image[IDtiming].array.F[*status] = tdiffv;
    }


    if(gpumatmultconf[index].sem==0)
    {
        for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
            pthread_join( gpumatmultconf[index].threadarray[ptn], NULL);
    }
    else
    {
        for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
        {
            sem_post(gpumatmultconf[index].semptr1[ptn]); // START COMPUTATION
            sem_post(gpumatmultconf[index].semptr4[ptn]);
        }

        for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
            sem_wait(gpumatmultconf[index].semptr5[ptn]); // WAIT FOR RESULT

        // for safety, set semaphores to zerosem_getvalue(data.image[IDarray[i]].semptr[s], &semval);
        if(FORCESEMINIT==1)
            for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
            {
                sem_getvalue(gpumatmultconf[index].semptr5[ptn], &semval);
                for(cnt=0; cnt<semval; cnt++)
                    sem_trywait(gpumatmultconf[index].semptr5[ptn]);
            }

    }


    // SUM RESULTS FROM SEPARATE GPUs
    if(timing == 1)
    {
        *status = *status + 1;  // -> 9
        clock_gettime(CLOCK_REALTIME, &tnow);
        tdiff = info_time_diff(data.image[IDtiming].md[0].wtime, tnow);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
        data.image[IDtiming].array.F[*status] = tdiffv;
    }

    data.image[gpumatmultconf[index].IDout].md[0].write = 1;

    for(m=0; m<gpumatmultconf[index].M; m++)
        gpumatmultconf[index].dmVecTMP[m] = 0.0;



    for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
    {
        for(m=0; m<gpumatmultconf[index].M; m++)
            gpumatmultconf[index].dmVecTMP[m] += gpumatmultconf[index].dmVec_part[ptn][m];
    }

    COREMOD_MEMORY_image_set_sempost_byID(gpumatmultconf[index].IDout, -1);


    /*  if(data.image[gpumatmultconf[index].IDout].sem > 0)
       {
           sem_getvalue(data.image[gpumatmultconf[index].IDout].semptr[0], &semval);
           if(semval<SEMAPHORE_MAXVAL)
               sem_post(data.image[gpumatmultconf[index].IDout].semptr[0]);
       }


       if(data.image[gpumatmultconf[index].IDout].sem > 1)
           {
               sem_getvalue(data.image[gpumatmultconf[index].IDout].semptr[1], &semval);
               if(semval<SEMAPHORE_MAXVAL)
                   sem_post(data.image[gpumatmultconf[index].IDout].semptr[1]);
           }
    */


    data.image[gpumatmultconf[index].IDout].md[0].write = 0;
    data.image[gpumatmultconf[index].IDout].md[0].cnt0++;

    if(timing == 1)
    {
        *status = *status + 1; // -> 10
        clock_gettime(CLOCK_REALTIME, &tnow);
        tdiff = info_time_diff(data.image[IDtiming].md[0].wtime, tnow);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
        data.image[IDtiming].array.F[*status] = tdiffv;
    }

    return(0);
}










int GPU_loop_MultMat_free(int index)
{
    int device;

    cudaFree(gpumatmultconf[index].d_cMat);
    cudaFree(gpumatmultconf[index].d_dmVec);
    cudaFree(gpumatmultconf[index].d_wfsVec);
    cudaFree(gpumatmultconf[index].d_wfsRef);
    cudaFree(gpumatmultconf[index].d_dmRef);
    free(gpumatmultconf[index].stream);

    for(device=0; device<gpumatmultconf[index].NBstreams; device++)
    {
        // free memory for stream
        cublasDestroy(gpumatmultconf[index].handle[device]);
        free(gpumatmultconf[index].cMat_part[device]);
        free(gpumatmultconf[index].wfsVec_part[device]);
        free(gpumatmultconf[index].dmVec_part[device]);
    }

    free(gpumatmultconf[index].cMat_part);
    free(gpumatmultconf[index].dmVec_part);
    free(gpumatmultconf[index].wfsVec_part);

    free(gpumatmultconf[index].Nsize);
    free(gpumatmultconf[index].Noffset);

    free(gpumatmultconf[index].iret);
    free(gpumatmultconf[index].threadarray);
    free(gpumatmultconf[index].thdata);

    free(gpumatmultconf[index].refWFSinit);

    free(gpumatmultconf[index].GPUdevice);

    return(0);
}






/* 
 *
 * sequence of events :
 * 
 * wait semptr1              (wait for input image data)
 * transfer input CPU -> GPU  
 * post semptr2
 * COMPUTE
 * post semptr3
 * wait semptr4
 * 
 * 
 * 
 *  
 */


void *compute_function( void *ptr )
{
    THDATA *thdata;
    int device;
    int n, m;
    int index;
    const char *ptr0; // source
    const char *ptr1; // dest
    float *ptr0f; // test
    int *ptrstat;
    long IDtest;
    int k;
    int kmax = 10;
    char fname[200];
    long long iter;
    long long itermax = 1;
    float imtot;
    float alphatmp;
    float betatmp;
    int semval;
    long cnt;
    FILE *fptest;
    long ii;

    float alpharef, betaref;

    thdata = (THDATA*) ptr;
    device = thdata->thread_no;
    index = thdata->cindex;

    ptrstat = (int*) ((char*) thdata->status + sizeof(int)*device + sizeof(int)*10*index);

    *ptrstat = 1;


	
	
	
    ptr0 = (char*) gpumatmultconf[index].wfsVec;
    ptr0 += sizeof(float)*gpumatmultconf[index].Noffset[device];
    ptr0f = (float*) ptr0;

    if((index==0)||(index==2))
        cudaSetDevice(gpumatmultconf[index].GPUdevice[device]);

    cublasSetStream( gpumatmultconf[index].handle[device], gpumatmultconf[index].stream[device] );



    if(gpumatmultconf[index].sem==1)
        itermax = -1;
    else
        itermax = 1;

    iter = 0;
    while(iter != itermax)
    {
		//printf("====================================== gpumatmultconf[index].M = %d\n", gpumatmultconf[index].M);
		//fflush(stdout);
		
		
        // copy DM reference to output to prepare computation:   d_dmVec <- d_dmRef
        error = cudaMemcpy(gpumatmultconf[index].d_dmVec[device], gpumatmultconf[index].d_dmRef[device], sizeof(float)*gpumatmultconf[index].M, cudaMemcpyDeviceToDevice);
        if (error != cudaSuccess)
        {
            printf("cudaMemcpy d_wfsVec wfsVec returned error code %d, line(%d)\n", error, __LINE__);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        *ptrstat = 2; // wait for image
        if(gpumatmultconf[index].sem==1)
        {
            sem_wait(gpumatmultconf[index].semptr1[device]);

            if(FORCESEMINIT==1)
            {
                sem_getvalue(gpumatmultconf[index].semptr1[device], &semval);
                for(cnt=0; cnt<semval; cnt++)
                    sem_trywait(gpumatmultconf[index].semptr1[device]);
            }
        }

        *ptrstat = 3; // transfer: prt0 -> d_wfsVec
        stat = cublasSetVector(gpumatmultconf[index].Nsize[device], sizeof(float), (float*) ptr0, 1, gpumatmultconf[index].d_wfsVec[device], 1);
        if (stat != CUBLAS_STATUS_SUCCESS)
        {
            fprintf(stderr, "!!!! device access error (read C)\n");
            if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
            if(stat == CUBLAS_STATUS_INVALID_VALUE)
                printf("   CUBLAS_STATUS_INVALID_VALUE\n");
            if(stat == CUBLAS_STATUS_MAPPING_ERROR)
                printf("   CUBLAS_STATUS_MAPPING_ERROR\n");
            exit(EXIT_FAILURE);
        }


        if(gpumatmultconf[index].refWFSinit[device] == 0) // compute DM reference (used when reference changes)
        {
            *ptrstat = 4; // compute

            if(gpumatmultconf[index].sem==1)
                sem_post(gpumatmultconf[index].semptr2[device]);


          //  printf("%d  device %d (GPU %d): compute reference product\n", index, device, gpumatmultconf[index].GPUdevice[device]);
          //  fflush(stdout);

            //            alphatmp = cublasSgemv_alpha;
            //            betatmp = cublasSgemv_beta;

            // MOVE THIS TO CPU AS A SEPARATE THREAD TO AVOID LOOP PAUSE ??
            //        cublasSgemv_alpha = 1.0;
            //        cublasSgemv_beta = 0.0;
            alpharef = 1.0;
            betaref = 0.0;
            stat = cublasSgemv(gpumatmultconf[index].handle[device], CUBLAS_OP_N, gpumatmultconf[index].M, gpumatmultconf[index].Nsize[device], &alpharef, gpumatmultconf[index].d_cMat[device], gpumatmultconf[index].M, gpumatmultconf[index].d_wfsVec[device], 1, &betaref, gpumatmultconf[index].d_dmRef[device], 1);
            if (stat != CUBLAS_STATUS_SUCCESS)
            {
                printf("cublasSgemv returned error code %d, line(%d)\n", stat, __LINE__);
                fflush(stdout);
                if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                if(stat == CUBLAS_STATUS_INVALID_VALUE)
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                if(stat == CUBLAS_STATUS_ARCH_MISMATCH)
                    printf("   CUBLAS_STATUS_ARCH_MISMATCH\n");
                if(stat == CUBLAS_STATUS_EXECUTION_FAILED)
                    printf("   CUBLAS_STATUS_EXECUTION_FAILED\n");

                printf("device %d of index %d\n", device, index);
                printf("GPU device                          = %d\n", gpumatmultconf[index].GPUdevice[device]);

                printf("CUBLAS_OP_N                         = %d\n", CUBLAS_OP_N);
                printf("alpha                               = %f\n", alpharef);
                printf("alpha                               = %f\n", betaref);
                printf("gpumatmultconf[index].M             = %d\n", (int) gpumatmultconf[index].M);
                printf("gpumatmultconf[index].Nsize[device] = %d\n", (int) gpumatmultconf[index].Nsize[device]);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            //          cublasSgemv_alpha = alphatmp;
            //          cublasSgemv_beta = betatmp;

            gpumatmultconf[index].refWFSinit[device] = 1;


            if(gpumatmultconf[index].sem==1)
                sem_post(gpumatmultconf[index].semptr3[device]);

            *ptrstat = 5; // transfer result

            if(gpumatmultconf[index].sem==1)
            {
                sem_wait(gpumatmultconf[index].semptr4[device]);
                if(FORCESEMINIT==1)
                {
                    sem_getvalue(gpumatmultconf[index].semptr4[device], &semval);
                    for(cnt=0; cnt<semval; cnt++)
                        sem_trywait(gpumatmultconf[index].semptr4[device]);
                }
            }


            // copy d_dmRef -> dmRef_part
            stat = cublasGetVector(gpumatmultconf[index].M, sizeof(float), gpumatmultconf[index].d_dmRef[device], 1, gpumatmultconf[index].dmRef_part[device], 1);
            if (stat != CUBLAS_STATUS_SUCCESS)
            {
                fprintf(stderr, "!!!! device access error (read C)\n");
                if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                if(stat == CUBLAS_STATUS_INVALID_VALUE)
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                if(stat == CUBLAS_STATUS_MAPPING_ERROR)
                    printf("   CUBLAS_STATUS_MAPPING_ERROR\n");
                exit(EXIT_FAILURE);
            }

            // TEST

        /*    sprintf(fname, "gputest%d.txt", device);
            if((fptest = fopen(fname, "w"))==NULL)
            {
                printf("ERROR: cannot create file \"%s\"\n", fname);
                exit(0);
            }
            printf("Writing test file \"%s\"\n", fname);
            fflush(stdout);
            for(ii=0; ii<gpumatmultconf[index].M; ii++)
                fprintf(fptest, "%ld %f\n", ii, gpumatmultconf[index].dmRef_part[device][ii]);
            fclose(fptest);
*/
            if(gpumatmultconf[index].sem==1)
                sem_post(gpumatmultconf[index].semptr5[device]);

            *ptrstat = 6;
        }
        else
        {
            *ptrstat = 4; // compute

            if(gpumatmultconf[index].sem==1)
                sem_post(gpumatmultconf[index].semptr2[device]);

            stat = cublasSgemv(gpumatmultconf[index].handle[device], CUBLAS_OP_N, gpumatmultconf[index].M, gpumatmultconf[index].Nsize[device], &cublasSgemv_alpha, gpumatmultconf[index].d_cMat[device], gpumatmultconf[index].M, gpumatmultconf[index].d_wfsVec[device], 1, &cublasSgemv_beta, gpumatmultconf[index].d_dmVec[device], 1);

            if (stat != CUBLAS_STATUS_SUCCESS)
            {
                printf("cublasSgemv returned error code %d, line(%d)\n", stat, __LINE__);
                fflush(stdout);
                if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                if(stat == CUBLAS_STATUS_INVALID_VALUE)
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                if(stat == CUBLAS_STATUS_ARCH_MISMATCH)
                    printf("   CUBLAS_STATUS_ARCH_MISMATCH\n");
                if(stat == CUBLAS_STATUS_EXECUTION_FAILED)
                    printf("   CUBLAS_STATUS_EXECUTION_FAILED\n");

                printf("device %d of index %d\n", device, index);
                printf("GPU device : %d\n", gpumatmultconf[index].GPUdevice[device]);

                printf("alpha = %f\n", cublasSgemv_alpha);
                printf("alpha = %f\n", cublasSgemv_beta);
                printf("gpumatmultconf[index].Nsize[device] = %d\n", (int) gpumatmultconf[index].Nsize[device]);
                fflush(stdout);

                exit(EXIT_FAILURE);
            }


            if(gpumatmultconf[index].sem==1)
                sem_post(gpumatmultconf[index].semptr3[device]);

            *ptrstat = 5; // transfer result

            if(gpumatmultconf[index].sem==1)
            {
                sem_wait(gpumatmultconf[index].semptr4[device]);
                if(FORCESEMINIT==1)
                {
                    sem_getvalue(gpumatmultconf[index].semptr4[device], &semval);
                    for(cnt=0; cnt<semval; cnt++)
                        sem_trywait(gpumatmultconf[index].semptr4[device]);
                }
            }

            // result is on gpumatmultconf[index].d_dmVec[device]
            stat = cublasGetVector(gpumatmultconf[index].M, sizeof(float), gpumatmultconf[index].d_dmVec[device], 1, gpumatmultconf[index].dmVec_part[device], 1);
            if (stat != CUBLAS_STATUS_SUCCESS)
            {
                fprintf(stderr, "!!!! device access error (read C)\n");
                if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                if(stat == CUBLAS_STATUS_INVALID_VALUE)
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                if(stat == CUBLAS_STATUS_MAPPING_ERROR)
                    printf("   CUBLAS_STATUS_MAPPING_ERROR\n");
                exit(EXIT_FAILURE);
            }
        }
        if(gpumatmultconf[index].sem==1)
            sem_post(gpumatmultconf[index].semptr5[device]);

        *ptrstat = 6;

        // START MODE VALUES COMPUTATION HERE



        iter++;
    }


    pthread_exit(0);
}






#ifdef HAVE_MAGMA






/******************* CPU memory */
#define TESTING_MALLOC_CPU( ptr, type, size )                              \
    if ( MAGMA_SUCCESS !=                                                  \
            magma_malloc_cpu( (void**) &ptr, (size)*sizeof(type) )) {      \
        fprintf( stderr, "!!!! magma_malloc_cpu failed for: %s\n", #ptr ); \
        magma_finalize();                                                  \
        exit(-1);                                                          \
    }

#define TESTING_FREE_CPU( ptr ) magma_free_cpu( ptr )


/******************* Pinned CPU memory */
#ifdef HAVE_CUBLAS
    // In CUDA, this allocates pinned memory.
    #define TESTING_MALLOC_PIN( ptr, type, size )                                 \
        if ( MAGMA_SUCCESS !=                                                     \
                magma_malloc_pinned( (void**) &ptr, (size)*sizeof(type) )) {      \
            fprintf( stderr, "!!!! magma_malloc_pinned failed for: %s\n", #ptr ); \
            magma_finalize();                                                     \
            exit(-1);                                                             \
        }
    
    #define TESTING_FREE_PIN( ptr ) magma_free_pinned( ptr )
#else
    // For OpenCL, we don't support pinned memory yet.
    #define TESTING_MALLOC_PIN( ptr, type, size )                              \
        if ( MAGMA_SUCCESS !=                                                  \
                magma_malloc_cpu( (void**) &ptr, (size)*sizeof(type) )) {      \
            fprintf( stderr, "!!!! magma_malloc_cpu failed for: %s\n", #ptr ); \
            magma_finalize();                                                  \
            exit(-1);                                                          \
        }
    
    #define TESTING_FREE_PIN( ptr ) magma_free_cpu( ptr )
#endif


/******************* GPU memory */
#ifdef HAVE_CUBLAS
    // In CUDA, this has (void**) cast.
    #define TESTING_MALLOC_DEV( ptr, type, size )                              \
        if ( MAGMA_SUCCESS !=                                                  \
                magma_malloc( (void**) &ptr, (size)*sizeof(type) )) {          \
            fprintf( stderr, "!!!! magma_malloc failed for: %s\n", #ptr );     \
            magma_finalize();                                                  \
            exit(-1);                                                          \
        }
#else
    // For OpenCL, ptr is cl_mem* and there is no cast.
    #define TESTING_MALLOC_DEV( ptr, type, size )                              \
        if ( MAGMA_SUCCESS !=                                                  \
                magma_malloc( &ptr, (size)*sizeof(type) )) {                   \
            fprintf( stderr, "!!!! magma_malloc failed for: %s\n", #ptr );     \
            magma_finalize();                                                  \
            exit(-1);                                                          \
        }
#endif

#define TESTING_FREE_DEV( ptr ) magma_free( ptr )










//
// Computes control matrix
// Conventions:
//   m: number of actuators 
//   n: number of sensors  
int CUDACOMP_magma_compute_SVDpseudoInverse_old(const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, long MaxNBmodes, const char *ID_VTmatrix_name) /* works for m != n */
{
	long *arraysizetmp;
	magma_int_t M, N, min_mn;
	long m, n, ii, jj, k;
	long ID_Rmatrix;
	long ID_Cmatrix;
	int atype;
	
	magma_int_t lda, ldu, ldv;
	float dummy[1];
	float *a, *h_R;         // a, h_R - mxn  matrices
	float *U, *VT;			// u - mxm matrix , vt - nxn  matrix  on the  host
	float *S1;              //  vectors  of  singular  values
	magma_int_t  info;
	magma_int_t  ione   = 1;
	float  work[1], error = 1.;				// used in  difference  computations
	float  mone =  -1.0, *h_work;           //  h_work  - workspace
	magma_int_t  lwork;                     //  workspace  size
	real_Double_t gpu_time , cpu_time;
	
	FILE *fp;
	char fname[200];
	long ID_VTmatrix;
	float egvlim;
	long maxMode, MaxNBmodes1, mode;

	
    arraysizetmp = (long*) malloc(sizeof(long)*3);

    ID_Rmatrix = image_ID(ID_Rmatrix_name);
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

	M = n;
	N = m;

	lda = M;
	ldu = M;
	ldv = N;
	
	min_mn=min(M,N);
	
	//printf("INITIALIZE MAGMA\n");
	//fflush(stdout);
	
    /* in this procedure, m=number of actuators/modes, n=number of WFS elements */
 //   printf("magma :    M = %ld , N = %ld\n", (long) M, (long) N);
    //fflush(stdout);



	magma_init (); // initialize Magma
	//  Allocate  host  memory
	magma_smalloc_cpu (&a, lda*N);                  // host  memory  for a
	magma_smalloc_cpu (&VT, ldv*N);                 // host  memory  for vt
	magma_smalloc_cpu (&U, M*M);                    // host  memory  for u
	magma_smalloc_cpu (&S1, min_mn );               // host  memory  for s1
	magma_smalloc_pinned (&h_R, lda*N);             // host  memory  for r
	magma_int_t  nb = magma_get_sgesvd_nb(M,N);     // opt. block  size
	lwork = (M+N)*nb + 3* min_mn;
	magma_smalloc_pinned (&h_work, lwork);          // host  mem. for  h_work





	
	
	
	// write input h_R matrix
	 if(atype==FLOAT)
    {
        for(k=0; k<m; k++)
            for(ii=0; ii<n; ii++)
                h_R[k*n+ii] =  data.image[ID_Rmatrix].array.F[k*n+ii];
    }
    else
    {
        for(k=0; k<m; k++)
            for(ii=0; ii<n; ii++)
                h_R[k*n+ii] = data.image[ID_Rmatrix].array.D[k*n+ii];
    }

	//printf("M = %ld   N = %ld\n", (long) M, (long) N);
	//printf("=============== lwork = %ld\n", (long) lwork);
	gpu_time = magma_wtime ();
	magma_sgesvd( MagmaSomeVec, MagmaAllVec, M, N, h_R, lda, S1, U, ldu, VT, ldv, h_work, lwork, &info );
	gpu_time = magma_wtime() - gpu_time;
	if (info != 0) {
                printf("magma_sgesvd returned error %d: %s.\n",
                       (int) info, magma_strerror( info ));
            }
            
	//printf("sgesvd gpu time: %7.5f\n", gpu_time );   
     
     
     // Write eigenvalues
    sprintf(fname, "eigenv.dat.magma");
    if((fp=fopen(fname, "w"))==NULL)
      {
        printf("ERROR: cannot create file \"%s\"\n", fname);
        exit(0);
      }
    for(k=0; k<min_mn; k++)
      fprintf(fp,"%5ld %20g %20g\n", k, S1[k], S1[k]/S1[0] );
    fclose(fp);

 
    egvlim = SVDeps * S1[0];
        
	MaxNBmodes1 = MaxNBmodes;
	if(MaxNBmodes1>M)
		MaxNBmodes1 = M;
	if(MaxNBmodes1>N)
		MaxNBmodes1 = N;
	mode = 0;
	while( (mode<MaxNBmodes1) && (S1[mode]>egvlim) )
		mode++;
	MaxNBmodes1 = mode;
	
	//printf("Keeping %ld modes  (SVDeps = %g)\n", MaxNBmodes1, SVDeps);
    // Write rotation matrix 
    arraysizetmp[0] = m;
    arraysizetmp[1] = m;
    if(atype==FLOAT)
    {
        ID_VTmatrix = create_image_ID(ID_VTmatrix_name, 2, arraysizetmp, FLOAT, 0, 0);
        for(ii=0; ii<m; ii++) // modes
            for(k=0; k<m; k++) // modes
                data.image[ID_VTmatrix].array.F[k*m+ii] = (float) VT[k*m+ii];
    }
    else
    {
        ID_VTmatrix = create_image_ID(ID_VTmatrix_name, 2, arraysizetmp, DOUBLE, 0, 0);
        for(ii=0; ii<m; ii++) // modes
            for(k=0; k<m; k++) // modes
                data.image[ID_VTmatrix].array.D[k*m+ii] = (double) VT[k*m+ii];
    }

     
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

   // compute pseudo-inverse
   // M+ = V Sig^-1 UT
  /* for(ii=0;ii<M;ii++)
		for(jj=0;jj<N;jj++)
			for(mode=0;mode<MaxNBmodes1-1;mode++)
				{
					data.image[ID_Cmatrix].array.F[jj*M+ii] += VT[jj*N+mode]*U[mode*M+ii]/S1[mode];
				}
   */
     
            
    magma_free_cpu(a);                                        // free  host  memory
	magma_free_cpu(VT);                                       // free  host  memory
	magma_free_cpu(S1);                                       // free  host  memory
	magma_free_cpu(U);                                        // free  host  memory
	magma_free_pinned(h_work );                  // free  host  memory
	magma_free_pinned(h_R);                        // free  host  memory       
            
	magma_finalize ();                               //  finalize  Magma

	free(arraysizetmp);

//    printf("[CM magma SVD done]\n");
 //   fflush(stdout);

    return(ID_Cmatrix);
}












//
// Computes control matrix
// Conventions:
//   m: number of actuators
//   n: number of sensors
//	assumes n>m
//
// NOTE: NUMERICALLY STABLE FOR SVDeps >1e-3
//
// use LOOPmode = 1 for computing the same size SVD, same input and output location
//
int CUDACOMP_magma_compute_SVDpseudoInverse(const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, long MaxNBmodes, const char *ID_VTmatrix_name, int LOOPmode) /* works for m != n */
{
    long ID_Rmatrix;
    int atype;
    long *arraysizetmp;
    int size; // variable for memory allocations
    long n, m, N, M;
    long ii, jj, k;
    magma_int_t info;

    double aux_work[1];
    float auxf_work[1];

    long ID_A, ID_AtA, ID_VT, ID_Ainv;

    // Timing
    int testmode = 0;
    int timing = 1;
    struct timespec t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    double t01d, t12d, t23d, t34d, t45d, t56d, t67d, t78d, t89d, t09d;
    struct timespec tdiff;


    FILE *fp;
    char fname[200];

    long maxMode, MaxNBmodes1, mode;
    double egvlim;
    long nbmodesremoved;
    long ID_M2;

    long ID_Cmatrix;



    int MAGMAfloat = 0; // 1 if single precision



    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t0);



    arraysizetmp = (long*) malloc(sizeof(long)*3);



    ID_Rmatrix = image_ID(ID_Rmatrix_name);
    atype = data.image[ID_Rmatrix].md[0].atype;


    if(data.image[ID_Rmatrix].md[0].naxis==3)
    {
        n = data.image[ID_Rmatrix].md[0].size[0]*data.image[ID_Rmatrix].md[0].size[1];
        m = data.image[ID_Rmatrix].md[0].size[2];
       // printf("3D image -> %ld %ld\n", n, m);
       // fflush(stdout);
    }
    else
    {
        n = data.image[ID_Rmatrix].md[0].size[0];
        m = data.image[ID_Rmatrix].md[0].size[1];
      //  printf("2D image -> %ld %ld\n", n, m);
       // fflush(stdout);
    }

    M = n;
    N = m;
    /*	if(M<N)
    		{
    			printf("ERROR: this routine needs M > N\n");
    			exit(0);
    		}
    	*/

    /* in this procedure, m=number of actuators/modes, n=number of WFS elements */


   // printf("magma :    M = %ld , N = %ld\n", (long) M, (long) N);
  //  fflush(stdout);


    if(INIT_MAGMA==0)
    {
        printf("INITIALIZE MAGMA\n");
        fflush(stdout);
        magma_init(); // initialize Magma
        //flops_init();
        magma_print_environment();

        INIT_MAGMA = 1;
    }



    if(MAGMAloop_iter == 0)
    {
        if(MAGMAfloat==0)
        {
            TESTING_MALLOC_CPU( magma_h_A, double, M*N);
            TESTING_MALLOC_DEV( magma_d_A, double, M*N);

            TESTING_MALLOC_CPU( magma_h_AtA, double, N*N);
            TESTING_MALLOC_DEV( magma_d_AtA, double, N*N);

            TESTING_MALLOC_CPU( magma_h_VT1, double, N*N);
            TESTING_MALLOC_DEV( magma_d_VT1, double, N*N);
            TESTING_MALLOC_DEV( magma_d_M2, double, N*N);
        }
        else
        {
            TESTING_MALLOC_CPU( magmaf_h_A, float, M*N);
            TESTING_MALLOC_DEV( magmaf_d_A, float, M*N);

            TESTING_MALLOC_CPU( magmaf_h_AtA, float, N*N);
            TESTING_MALLOC_DEV( magmaf_d_AtA, float, N*N);

            TESTING_MALLOC_CPU( magmaf_h_VT1, float, N*N);
            TESTING_MALLOC_DEV( magmaf_d_VT1, float, N*N);
            TESTING_MALLOC_DEV( magmaf_d_M2, float, N*N);
        }
    }




    //printf("MAGMA READY\n");
    //fflush(stdout);


    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t1);




    // write input h_A matrix
    if(atype==FLOAT)
    {
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<n*m; ii++)
                magmaf_h_A[ii] =  data.image[ID_Rmatrix].array.F[ii];
        }
        else
        {
            for(ii=0; ii<n*m; ii++)
                magma_h_A[ii] =  data.image[ID_Rmatrix].array.F[ii];
        }
    }
    else
    {
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<n*m; ii++)
                magmaf_h_A[ii] = data.image[ID_Rmatrix].array.D[ii];
        }
        else
        {
            for(ii=0; ii<n*m; ii++)
                magma_h_A[ii] = data.image[ID_Rmatrix].array.D[ii];
        }
    }

    if(testmode==1)
    {
        ID_A = create_2Dimage_ID("mA", M, N);
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<M*N; ii++)
                data.image[ID_A].array.F[ii] = magmaf_h_A[ii];
        }
        else
        {
            for(ii=0; ii<M*N; ii++)
                data.image[ID_A].array.F[ii] = magma_h_A[ii];
        }

        save_fits("mA", "!test_mA.fits");
        delete_image_ID("mA");
    }


    if(MAGMAloop_iter == 0)
        magma_queue_create(0, &magmaqueue);


    if(MAGMAfloat==1)
        magma_ssetmatrix( M, N, magmaf_h_A, M, magmaf_d_A, M, magmaqueue);
    else
        magma_dsetmatrix( M, N, magma_h_A, M, magma_d_A, M, magmaqueue);


    if(LOOPmode==0)
    {
        if(MAGMAfloat==1)
            TESTING_FREE_CPU( magmaf_h_A );
        else
            TESTING_FREE_CPU( magma_h_A );
    }

    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t2);

    // COMPUTE trans(A) x A
    if(MAGMAfloat==1)
        magma_sgemm(  MagmaTrans, MagmaNoTrans, N, N, M, 1.0, magmaf_d_A, M, magmaf_d_A, M, 0.0,  magmaf_d_AtA, N, magmaqueue);
    else
        magma_dgemm(  MagmaTrans, MagmaNoTrans, N, N, M, 1.0, magma_d_A, M, magma_d_A, M, 0.0,  magma_d_AtA, N, magmaqueue);



    if(testmode == 1)
    {
        if(MAGMAfloat==1)
            magma_sgetmatrix( N, N, magmaf_d_AtA, N, magmaf_h_AtA, N, magmaqueue);
        else
            magma_dgetmatrix( N, N, magma_d_AtA, N, magma_h_AtA, N, magmaqueue);


        ID_AtA = create_2Dimage_ID("mAtA", N, N);
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<N*N; ii++)
                data.image[ID_AtA].array.F[ii] = magmaf_h_AtA[ii];
        }
        else
        {
            for(ii=0; ii<N*N; ii++)
                data.image[ID_AtA].array.F[ii] = magma_h_AtA[ii];
        }
        save_fits("mAtA", "!test_mAtA.fits");
    }



    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t3);

    // COMPUTE eigenvalues and eigenvectors of trans(A) x A

    if(MAGMAloop_iter==0)
    {
        // get workspace size
        if(MAGMAfloat==1)
        {
            magma_ssyevd_gpu( MagmaVec, MagmaLower, N, NULL, N, NULL, NULL, N, auxf_work,  -1, magma_aux_iwork, -1, &info );
            magma_lwork  = (magma_int_t) MAGMA_S_REAL( auxf_work[0] );
        }
        else
        {
            magma_dsyevd_gpu( MagmaVec, MagmaLower, N, NULL, N, NULL, NULL, N, aux_work,  -1, magma_aux_iwork, -1, &info );
            magma_lwork  = (magma_int_t) MAGMA_S_REAL( aux_work[0] );
        }

        magma_liwork = magma_aux_iwork[0];

      //  printf("workspace size : %ld  %ld\n", (long) magma_lwork, (long) magma_liwork);
    }



    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t4);


    // allocate & compute
    if(MAGMAloop_iter == 0)
    {
        if(MAGMAfloat==1)
        {
            TESTING_MALLOC_CPU( magma_iwork,  magma_int_t, magma_liwork );
            TESTING_MALLOC_PIN( magmaf_h_work, float, magma_lwork  );
            TESTING_MALLOC_CPU( magmaf_w1,     float,             N      );
            TESTING_MALLOC_PIN( magmaf_h_R,    float, N*N  );
        }
        else
        {
            TESTING_MALLOC_CPU( magma_iwork,  magma_int_t, magma_liwork );
            TESTING_MALLOC_PIN( magma_h_work, double, magma_lwork  );
            TESTING_MALLOC_CPU( magma_w1,     double,             N      );
            TESTING_MALLOC_PIN( magma_h_R,    double, N*N  );
        }
    }



    // THIS ROUTINE IS GOOD TO EIGENVALUES ABOUT 1e-6 OF PEAK EIGENVALUE IF USING FLOAT, MUCH BETTER WITH DOUBLE
    if(MAGMAfloat==1)
        magma_ssyevd_gpu( MagmaVec, MagmaLower, N, magmaf_d_AtA, N, magmaf_w1, magmaf_h_R, N, magmaf_h_work, magma_lwork, magma_iwork, magma_liwork, &info );
    else
        magma_dsyevd_gpu( MagmaVec, MagmaLower, N, magma_d_AtA, N, magma_w1, magma_h_R, N, magma_h_work, magma_lwork, magma_iwork, magma_liwork, &info );

    if(LOOPmode == 0)
    {
        TESTING_FREE_CPU( magma_iwork  );

        if(MAGMAfloat==1)
            TESTING_FREE_PIN( magmaf_h_R    );
        else
            TESTING_FREE_PIN( magma_h_R    );

		if(MAGMAfloat==1)
			TESTING_FREE_PIN( magma_h_work );
		else
			TESTING_FREE_PIN( magmaf_h_work );
    }




    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t5);



    // Write eigenvalues
    sprintf(fname, "eigenv.dat");
    if((fp=fopen(fname, "w"))==NULL)
    {
        printf("ERROR: cannot create file \"%s\"\n", fname);
        exit(0);
    }
    if(MAGMAfloat==1)
    {
        for(k=0; k<m; k++)
            fprintf(fp,"%5ld %20.8g  %20.8f  %20.8f\n", k, magmaf_w1[m-k-1], magmaf_w1[m-k-1]/magmaf_w1[m-1], SVDeps*SVDeps);
    }
    else
    {
        for(k=0; k<m; k++)
            fprintf(fp,"%5ld %20.8g  %20.8f  %20.8f\n", k, magma_w1[m-k-1], magma_w1[m-k-1]/magma_w1[m-1], SVDeps*SVDeps);
    }
    fclose(fp);


	
	if(MAGMAfloat==1)
		egvlim = SVDeps*SVDeps* magmaf_w1[m-1];
    else
    	egvlim = SVDeps*SVDeps* magma_w1[m-1];
    
    MaxNBmodes1 = MaxNBmodes;
    if(MaxNBmodes1>m)
        MaxNBmodes1 = m;
    if(MaxNBmodes1>n)
        MaxNBmodes1 = n;
    mode = 0;
    
    if(MAGMAfloat==1)
    {
		while( (mode<MaxNBmodes1) && (magmaf_w1[m-mode-1]>egvlim) )
			mode++;
    }
    else
    {
		while( (mode<MaxNBmodes1) && (magma_w1[m-mode-1]>egvlim) )
			mode++;
	}
	
//    printf("Keeping %ld modes  (SVDeps = %g -> %g, MaxNBmodes = %ld -> %ld)\n", mode, SVDeps, egvlim, MaxNBmodes, MaxNBmodes1);

    fp = fopen("SVDmodes.log", "w");
    fprintf(fp, "%6ld %6ld\n", mode, MaxNBmodes1);
    fclose(fp);
    MaxNBmodes1 = mode;
    //printf("Keeping %ld modes  (SVDeps = %g)\n", MaxNBmodes1, SVDeps);

    if(MAGMAfloat==1)
        magma_sgetmatrix( N, N, magmaf_d_AtA, N, magmaf_h_AtA, N, magmaqueue);
    else
        magma_dgetmatrix( N, N, magma_d_AtA, N, magma_h_AtA, N, magmaqueue);

    //if(testmode == 1)
    //{
    ID_VT = create_2Dimage_ID(ID_VTmatrix_name, N, N);

    if(MAGMAfloat==1)
    {
        for(ii=0; ii<N; ii++)
            for(jj=0; jj<N; jj++)
                data.image[ID_VT].array.F[jj*N+ii] = magmaf_h_AtA[(N-ii-1)*N+jj];
    }
    else
    {
        for(ii=0; ii<N; ii++)
            for(jj=0; jj<N; jj++)
                data.image[ID_VT].array.F[jj*N+ii] = magma_h_AtA[(N-ii-1)*N+jj];
    }


    if(MAGMAfloat==1)
    {
        for(ii=0; ii<N; ii++)
            for(jj=0; jj<N; jj++)
            {
                if(N-jj-1<MaxNBmodes1)
                    magmaf_h_VT1[ii*N+jj] = magmaf_h_AtA[jj*N+ii]/magmaf_w1[jj];
                else
                    magmaf_h_VT1[ii*N+jj] = 0.0;
            }
        magma_ssetmatrix( N, N, magmaf_h_VT1, N, magmaf_d_VT1, N, magmaqueue);
    }
    else
    {
        for(ii=0; ii<N; ii++)
            for(jj=0; jj<N; jj++)
            {
                if(N-jj-1<MaxNBmodes1)
                    magma_h_VT1[ii*N+jj] = magma_h_AtA[jj*N+ii]/magma_w1[jj];
                else
                    magma_h_VT1[ii*N+jj] = 0.0;
            }
        magma_dsetmatrix( N, N, magma_h_VT1, N, magma_d_VT1, N, magmaqueue);
    }


    if(LOOPmode == 0)
    {
        if(MAGMAfloat==1)
        {
            TESTING_FREE_CPU( magmaf_h_VT1 );
            TESTING_FREE_CPU( magmaf_w1 );
            TESTING_FREE_CPU( magmaf_h_AtA );
        }
        else
        {
            TESTING_FREE_CPU( magma_h_VT1 );
            TESTING_FREE_CPU( magma_w1 );
            TESTING_FREE_CPU( magma_h_AtA );
        }
    }



    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t6);

    // compute M2 = VT1 VT
    if(MAGMAfloat==1)
        magma_sgemm(  MagmaTrans, MagmaTrans, N, N, N, 1.0, magmaf_d_VT1, N, magmaf_d_AtA, N, 0.0,  magmaf_d_M2, N, magmaqueue);
    else
        magma_dgemm(  MagmaTrans, MagmaTrans, N, N, N, 1.0, magma_d_VT1, N, magma_d_AtA, N, 0.0,  magma_d_M2, N, magmaqueue);

    if(testmode == 1)
    {
        ID_M2 = create_2Dimage_ID("mM2", N, N);
        if(MAGMAfloat==1)
        {
            TESTING_MALLOC_CPU( magmaf_h_M2, float, N*N);
            magma_sgetmatrix( N, N, magmaf_d_M2, N, magmaf_h_M2, N, magmaqueue);

            for(ii=0; ii<N; ii++)
                for(jj=0; jj<N; jj++)
                    data.image[ID_M2].array.F[jj*N+ii] = magmaf_h_M2[jj*N+ii];
        }
        else
        {
            TESTING_MALLOC_CPU( magma_h_M2, double, N*N);
            magma_dgetmatrix( N, N, magma_d_M2, N, magma_h_M2, N, magmaqueue);

            for(ii=0; ii<N; ii++)
                for(jj=0; jj<N; jj++)
                    data.image[ID_M2].array.F[jj*N+ii] = magma_h_M2[jj*N+ii];
        }

        save_fits("mM2", "!test_mM2.fits");


        //	magma_dsetmatrix( N, N, h_M2, N, d_M2, N, magmaqueue);
        if(MAGMAfloat==1)
            TESTING_FREE_CPU( magmaf_h_M2 );
        else
            TESTING_FREE_CPU( magma_h_M2 );
    }

    if(LOOPmode == 0)
    {
        if(MAGMAfloat==1)
        {
            TESTING_FREE_DEV( magmaf_d_VT1 );
            TESTING_FREE_DEV( magmaf_d_AtA );
        }
        else
        {
            TESTING_FREE_DEV( magma_d_VT1 );
            TESTING_FREE_DEV( magma_d_AtA );
        }
    }

    // d_A d_M2
    //  w1

    // compute M3 = M2 A
    if(MAGMAloop_iter==0)
    {
        if(MAGMAfloat==1)
        {
            TESTING_MALLOC_DEV( magmaf_d_Ainv, float, N*M);
        }
        else
        {
            TESTING_MALLOC_DEV( magma_d_Ainv, double, N*M);
        }
    }

    if(MAGMAfloat==1)
        magma_sgemm(  MagmaNoTrans, MagmaNoTrans, M, N, N, 1.0, magmaf_d_A, M, magmaf_d_M2, N, 0.0, magmaf_d_Ainv, M, magmaqueue);
    else
        magma_dgemm(  MagmaNoTrans, MagmaNoTrans, M, N, N, 1.0, magma_d_A, M, magma_d_M2, N, 0.0, magma_d_Ainv, M, magmaqueue);


    if(LOOPmode==0)
    {
        if(MAGMAfloat==1)
        {
			TESTING_FREE_DEV( magmaf_d_A);
		}
        else
        {
            TESTING_FREE_DEV( magma_d_A);
		}
	}


    if(MAGMAloop_iter==0)
    {
        if(MAGMAfloat==1)
        {
		    TESTING_MALLOC_CPU( magmaf_h_Ainv, float, N*M);
        }
        else
        {
		    TESTING_MALLOC_CPU( magma_h_Ainv, double, N*M);
		}
    }

    if(LOOPmode==0)
    {
        if(MAGMAfloat==1)
            TESTING_FREE_DEV( magmaf_d_M2 );
        else
            TESTING_FREE_DEV( magma_d_M2 );
    }




    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t7);

    if(MAGMAfloat==1)
        magma_sgetmatrix( M, N, magmaf_d_Ainv, M, magmaf_h_Ainv, M, magmaqueue);
    else
        magma_dgetmatrix( M, N, magma_d_Ainv, M, magma_h_Ainv, M, magmaqueue);



    if(testmode == 1)
    {
        ID_Ainv = create_2Dimage_ID("mAinv", M, N);
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<M; ii++)
                for(jj=0; jj<N; jj++)
                    data.image[ID_Ainv].array.F[jj*M+ii] = magmaf_h_Ainv[jj*M+ii];
        }
        else
        {
            for(ii=0; ii<M; ii++)
                for(jj=0; jj<N; jj++)
                    data.image[ID_Ainv].array.F[jj*M+ii] = magma_h_Ainv[jj*M+ii];
        }
        save_fits("mAinv", "!test_mAinv.fits");
    }

    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t8);


    if(MAGMAloop_iter==0)
    {
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
    }
    else
        ID_Cmatrix = image_ID(ID_Cmatrix_name);

    /* write result */
    // 	M = n;
    //	N = m;
    if(atype==FLOAT)
    {
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<M*N; ii++)
                data.image[ID_Cmatrix].array.F[ii] = magmaf_h_Ainv[ii];
        }
        else
        {
            for(ii=0; ii<M*N; ii++)
                data.image[ID_Cmatrix].array.F[ii] = (float) magma_h_Ainv[ii];
        }
    }
    else
    {
        // sensors : M
        // actuator modes: N
        if(MAGMAfloat==1)
        {
            for(ii=0; ii<M*N; ii++)
                data.image[ID_Cmatrix].array.D[ii] = magmaf_h_Ainv[ii];
        }
        else
        {
            for(ii=0; ii<M*N; ii++)
                data.image[ID_Cmatrix].array.D[ii] = magma_h_Ainv[ii];
        }
    }


    if(timing==1)
        clock_gettime(CLOCK_REALTIME, &t9);


    if(LOOPmode==0)
    {
        if(MAGMAfloat==1)
        {
            TESTING_FREE_DEV( magmaf_d_Ainv );
            TESTING_FREE_CPU( magmaf_h_Ainv );
        }
        else
        {
            TESTING_FREE_DEV( magma_d_Ainv );
            TESTING_FREE_CPU( magma_h_Ainv );
        }
    }





    if(LOOPmode==0)
    {
        magma_queue_destroy( magmaqueue );
        magma_finalize ();                               //  finalize  Magma
    }

    free(arraysizetmp);


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

        tdiff = info_time_diff(t7, t8);
        t78d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

        tdiff = info_time_diff(t8, t9);
        t89d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

        tdiff = info_time_diff(t0, t9);
        t09d = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;


      //  printf("%6ld  Timing info: \n", MAGMAloop_iter);
      //  printf("  0-1	%12.3f ms\n", t01d*1000.0);
      //  printf("  1-2	%12.3f ms\n", t12d*1000.0);
      //  printf("  2-3	%12.3f ms\n", t23d*1000.0);
      //  printf("  3-4	%12.3f ms\n", t34d*1000.0);
      //  printf("  4-5	%12.3f ms\n", t45d*1000.0);
      //  printf("  5-6	%12.3f ms\n", t56d*1000.0);
      //  printf("  6-7	%12.3f ms\n", t67d*1000.0);
      //  printf("  7-8	%12.3f ms\n", t78d*1000.0);
      //  printf("  8-9	%12.3f ms\n", t89d*1000.0);
      //  printf("\n");
      //  printf(" TOTAL  %12.3f ms\n", t09d*1000.0);
    }



   // printf("\n\n");


    if(LOOPmode == 1)
        MAGMAloop_iter++;

    return(ID_Cmatrix);
}




#endif











//
// Computes control matrix
// Conventions:
//   n: number of actuators (= NB_MODES)
//   m: number of sensors  (= # of pixels)
// assumes m = n

int GPU_SVD_computeControlMatrix(int device, const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, const char *ID_VTmatrix_name)
{
    cusolverDnHandle_t  cudenseH = NULL;
    cublasHandle_t cublasH = NULL;
    cublasStatus_t cublas_status = CUBLAS_STATUS_SUCCESS;
    cusolverStatus_t cusolver_status = CUSOLVER_STATUS_SUCCESS;
    struct cudaDeviceProp deviceProp;
    int k;

    long ID_Rmatrix, ID_Cmatrix, ID_VTmatrix;
    int atype;
    int m;
    int n;
    long *arraysizetmp;
    int lda, ldu, ldvt;
    

    float *d_A = NULL; // linear memory of GPU
    float *h_A = NULL;
    float *d_S = NULL; // linear memory of GPU
    float *d_U = NULL; // linear memory of GPU
    float *h_U1 = NULL;    
    float *d_VT = NULL; // linear memory of GPU
    float *d_M = NULL; // linear memory of GPU
    float *d_U1 = NULL; // linear memory of GPU
    float *d_Work = NULL; // linear memory of GPU
    cudaError_t cudaStat = cudaSuccess;
    int *devInfo = NULL; // info in gpu (device copy)
    int Lwork;
    float *rwork;

    float *Sarray;
    float *Aarray;
    long i;
    FILE *fp;
    char fname[200];

    int info_gpu;

    double time1sec, time2sec;
    struct timespec tnow;


    long ID_leftSV; // left singular vectors
    float val;
    long ii, jj;
    float alpha = 1.0;
    float beta = 0.0;
    long ID;
    
    float *h_M;
    long cnt0;
 

    cudaGetDeviceCount(&deviceCount);
    printf("%d devices found\n", deviceCount);
    fflush(stdout);
    printf("\n");
    for (k = 0; k < deviceCount; ++k) {
        cudaGetDeviceProperties(&deviceProp, k);
        printf("Device %d [ %20s ]  has compute capability %d.%d.\n",
               k, deviceProp.name, deviceProp.major, deviceProp.minor);
        printf("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n", (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
        printf("  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
        printf("\n");
    }


    if(device<deviceCount)
    {
        cudaSetDevice(device);
    }
    else
    {
        printf("Invalid Device : %d / %d\n", device, deviceCount);
        exit(0);
    }

    cudaDeviceReset();

    printf("step 1a: create cudense handle ...");
    fflush(stdout);
    cusolver_status = cusolverDnCreate(&cudenseH);
    if (cusolver_status != CUSOLVER_STATUS_SUCCESS) {
        printf ("CUSOLVER initialization failed\n");
        return EXIT_FAILURE;
    }
    printf(" done\n");
    fflush(stdout);


    printf("step 1b: create cublas handle ...");
    fflush(stdout);
    cublas_status = cublasCreate(&cublasH);
    if (cublas_status != CUBLAS_STATUS_SUCCESS) {
        printf ("CUBLAS initialization failed\n");
        return EXIT_FAILURE;
    }
    printf(" done\n");
    fflush(stdout);





    clock_gettime(CLOCK_REALTIME, &tnow);
    time1sec = 1.0*((long) tnow.tv_sec) + 1.0e-9*tnow.tv_nsec;



    list_image_ID();

    arraysizetmp = (long*) malloc(sizeof(long)*3);
    ID_Rmatrix = image_ID(ID_Rmatrix_name);

    atype = data.image[ID_Rmatrix].md[0].atype;
    if(atype!=FLOAT)
    {
        printf("wrong type\n");
        exit(0);
    }

    if(data.image[ID_Rmatrix].md[0].naxis==3)
    {
        m = data.image[ID_Rmatrix].md[0].size[0]*data.image[ID_Rmatrix].md[0].size[1];
        n = data.image[ID_Rmatrix].md[0].size[2];
        printf("3D image -> %d %d\n", m, n);
        fflush(stdout);
    }
    else
    {
        m = data.image[ID_Rmatrix].md[0].size[0];
        n = data.image[ID_Rmatrix].md[0].size[1];
        printf("2D image -> %d %d\n", m, n);
        fflush(stdout);
    }

    if(m!=n)
    {
        printf("ERROR: m must be equal to n\n");
        exit(0);
    }









    cudaStat = cudaMalloc ((void**)&d_A  , sizeof(float) * n * m);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_A returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

    h_A = (float*) malloc(sizeof(float)*m*n);
    
    cudaStat = cudaMemcpy(d_A, data.image[ID_Rmatrix].array.F, sizeof(float)*m*n, cudaMemcpyHostToDevice);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy d_A returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }





    cudaStat = cudaMalloc ((void**)&d_S  , sizeof(float) * n);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_S returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

    cudaStat = cudaMalloc ((void**)&d_U  , sizeof(float) * m * m);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_U returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

    cudaStat = cudaMalloc ((void**)&d_VT  , sizeof(float) * n * n);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_VT returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    
    cudaStat = cudaMalloc ((void**)&devInfo, sizeof(int));
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc devInfo returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

    lda = m;
    ldu = m;
    ldvt = n;
    cusolver_status = cusolverDnSgesvd_bufferSize(cudenseH, m, n, &Lwork );
    if (cusolver_status != CUSOLVER_STATUS_SUCCESS) {
        printf ("CUSOLVER DnSgesvd_bufferSize failed\n");
        return EXIT_FAILURE;
    }

    cudaStat = cudaMalloc((void**)&d_Work, sizeof(float)*Lwork);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_Work returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    
    rwork = (float*) malloc(5*sizeof(float)*n);
    
 
    printf("START GPU COMPUTATION (%d x %d)  buffer size = %d ...", m, n, Lwork);
    fflush(stdout);
    cusolverDnSgesvd (cudenseH, 'A', 'A', m, n, d_A, lda, d_S, d_U, ldu, d_VT, ldvt, d_Work, Lwork, NULL, devInfo);
    printf(" SYNC ");
    fflush(stdout);
    cudaStat = cudaDeviceSynchronize();
    printf(" DONE\n");
    fflush(stdout);

    cudaStat = cudaMemcpy(&info_gpu, devInfo, sizeof(int), cudaMemcpyDeviceToHost);
    printf("after gesvd: info_gpu = %d\n", info_gpu);

 
    ID_VTmatrix = create_2Dimage_ID(ID_VTmatrix_name, n, n);
    cudaStat = cudaMemcpy(data.image[ID_VTmatrix].array.F, d_VT, sizeof(float)*n*n, cudaMemcpyDeviceToHost);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
   
   save_fits(ID_VTmatrix_name, "!matVT0.fits");
  
  
     Sarray = (float*) malloc(sizeof(float)*n);
    //    Aarray = (float*) malloc(sizeof(float)*m*n);
    cudaStat = cudaMemcpy(Sarray, d_S, sizeof(float)*n, cudaMemcpyDeviceToHost);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

    sprintf(fname, "eigenv.dat.gsl");
    if((fp=fopen(fname, "w"))==NULL)
    {
        printf("ERROR: cannot create file \"%s\"\n", fname);
        exit(0);
    }
    for(i=0; i<n; i++)
        fprintf(fp,"%5ld %20g %20g\n", i, Sarray[i], Sarray[i]/Sarray[0]);
    fclose(fp);



    ID = create_2Dimage_ID("matU", m, m);
    cudaMemcpy(data.image[ID].array.F, d_U, sizeof(float)*m*m, cudaMemcpyDeviceToHost);
    save_fits("matU", "!matU.fits");
  
    h_U1 = (float*) malloc(sizeof(float)*m*n);
    cudaStat = cudaMalloc((void**)&d_U1, sizeof(float)*m*n);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_U1 returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    for(ii=0;ii<m;ii++)
        for(jj=0;jj<n;jj++)
            h_U1[jj*m+ii] = data.image[ID].array.F[jj*m+ii];
    cudaMemcpy(d_U1, h_U1, sizeof(float)*m*n, cudaMemcpyHostToDevice);
    free(h_U1);
    
    ID = create_2Dimage_ID("matU1", m, n);
    cudaMemcpy(data.image[ID].array.F, d_U1, sizeof(float)*m*n, cudaMemcpyDeviceToHost);
    save_fits("matU1", "!matU1.fits");


   
   
    printf("SVDeps = %f\n", SVDeps);
    cnt0 = 0;
    // multiply lines of VT by 1/eigenval
    for(ii=0;ii<n;ii++)
    {
        if( Sarray[ii] > Sarray[0]*SVDeps )
            {
                val = 1.0/(Sarray[ii]);
                cnt0++;
            }
        else
            val = 0.0;
        
        for(jj=0;jj<n;jj++)
             data.image[ID_VTmatrix].array.F[jj*n+ii] *= val;
    }
    printf("%ld eigenvalues kept\n", cnt0);
    
    // copy VT back to GPU
   cudaStat = cudaMemcpy(d_VT, data.image[ID_VTmatrix].array.F, sizeof(float)*n*n, cudaMemcpyHostToDevice);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    
    
    cudaStat = cudaMalloc((void**)&d_M, sizeof(float)*n*m);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_M returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    
 
    save_fits(ID_VTmatrix_name, "!matVT.fits");
 
    cudaStat = cublasSgemm(cublasH, CUBLAS_OP_T, CUBLAS_OP_T, n, m, n, &alpha, d_VT, n, d_U, m, &beta, d_M, n);
     if (cudaStat != cudaSuccess)
    {
        printf("cublasSgemm returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

     



    if(data.image[ID_Rmatrix].md[0].naxis==3)
    {
        arraysizetmp[0] = data.image[ID_Rmatrix].md[0].size[0];
        arraysizetmp[1] = data.image[ID_Rmatrix].md[0].size[1];
        arraysizetmp[2] = n;
    }
    else
    {
        arraysizetmp[0] = m;
        arraysizetmp[1] = n;
    }

    
    ID_Cmatrix = create_image_ID(ID_Cmatrix_name, data.image[ID_Rmatrix].md[0].naxis, arraysizetmp, FLOAT, 0, 0);
    
    
 //   cudaStat = cudaMemcpy(data.image[ID_Cmatrix].array.F, d_M, sizeof(float)*m*n, cudaMemcpyDeviceToHost);
    
    h_M = (float*) malloc(sizeof(float)*m*n);
    cudaStat = cudaMemcpy(h_M, d_M, sizeof(float)*m*n, cudaMemcpyDeviceToHost);
    for(ii=0;ii<m;ii++)
        for(jj=0;jj<n;jj++)
            data.image[ID_Cmatrix].array.F[jj*m+ii] = h_M[ii*n+jj];
    
    //cudaStat = cudaMemcpy(data.image[ID_Cmatrix].array.F, d_VT, sizeof(float)*n*n, cudaMemcpyDeviceToHost);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    


    cudaFree(d_A);
    cudaFree(d_S);
    cudaFree(d_U);
    cudaFree(d_VT);
    cudaFree(d_Work);
    cudaFree(devInfo);
    cudaFree(d_M);
    cudaFree(d_U1);

    clock_gettime(CLOCK_REALTIME, &tnow);
    time2sec = 1.0*((long) tnow.tv_sec) + 1.0e-9*tnow.tv_nsec;

    printf("time = %8.3f s\n", 1.0*(time2sec-time1sec));




    if (cublasH ) cublasDestroy(cublasH);
    if (cudenseH) cusolverDnDestroy(cudenseH);

    cudaDeviceReset();

    free(arraysizetmp);
    free(Sarray);
    free(rwork);
    free(h_A);
    free(h_M);
    
    return(0);
}






//
// single GPU
// semaphore input = 3
//
int CUDACOMP_Coeff2Map_Loop(const char *IDmodes_name, const char *IDcoeff_name, int GPUindex, const char *IDoutmap_name, int offsetmode, const char *IDoffset_name)
{
    long NBmodes;
    long IDmodes;
    long IDcoeff;
    long IDoutmap;
    long m;
    int k;
    cublasHandle_t cublasH = NULL;
    cublasStatus_t cublas_status = CUBLAS_STATUS_SUCCESS;
    cudaError_t cudaStat = cudaSuccess;
    struct cudaDeviceProp deviceProp;

    float *d_modes = NULL; // linear memory of GPU
    float *d_coeff = NULL;
    float *d_outmap = NULL;

    float alpha = 1.0;
    float beta = 0.0;
    int loopOK;
    struct timespec ts;
    long iter;
    long long cnt = -1;
    long scnt;
    int semval;
    int semr;
    long ii, kk;

    long IDoffset;


    printf("entering CUDACOMP_Coeff2Map_Loop\n");
    printf("offsetmode = %d\n", offsetmode);
    fflush(stdout);

    if(offsetmode==1)
    {
        beta = 1.0;
        IDoffset = image_ID(IDoffset_name);
        
        if(IDoffset == -1)
        {
            printf("ERROR: image \"%s\" does not exist\n", IDoffset_name);
            exit(0);
        }
    }


    IDcoeff = image_ID(IDcoeff_name);
    NBmodes = 1;
    for(k=0; k<data.image[IDcoeff].md[0].naxis; k++)
        NBmodes *= data.image[IDcoeff].md[0].size[k];

    IDmodes = image_ID(IDmodes_name);
    if(data.image[IDmodes].md[0].naxis==3)
        m = data.image[IDmodes].md[0].size[0]*data.image[IDmodes].md[0].size[1];
    else
        m = data.image[IDmodes].md[0].size[0];



    IDoutmap = image_ID(IDoutmap_name);
    if(IDoutmap==-1)
    {
        printf("ERROR: missing output stream\n");
        exit(0);
    }
    COREMOD_MEMORY_image_set_createsem(IDoutmap_name, 5);


    cudaGetDeviceCount(&deviceCount);
    printf("%d devices found\n", deviceCount);
    fflush(stdout);
    printf("\n");
    for (k = 0; k < deviceCount; ++k) {
        cudaGetDeviceProperties(&deviceProp, k);
        printf("Device %d [ %20s ]  has compute capability %d.%d.\n",
               k, deviceProp.name, deviceProp.major, deviceProp.minor);
        printf("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n", (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
        printf("  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
        printf("\n");
    }


    if(GPUindex<deviceCount)
        cudaSetDevice(GPUindex);
    else
    {
        printf("Invalid Device : %d / %d\n", GPUindex, deviceCount);
        exit(0);
    }


    printf("Create cublas handle ...");
    fflush(stdout);
    cublas_status = cublasCreate(&cublasH);
    if (cublas_status != CUBLAS_STATUS_SUCCESS) {
        printf ("CUBLAS initialization failed\n");
        return EXIT_FAILURE;
    }
    printf(" done\n");
    fflush(stdout);


    // load modes to GPU
    cudaStat = cudaMalloc((void**)&d_modes, sizeof(float)*m*NBmodes);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_DMmodes returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    cudaStat = cudaMemcpy(d_modes, data.image[IDmodes].array.F, sizeof(float)*m*NBmodes, cudaMemcpyHostToDevice);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }


    // create d_outmap
    cudaStat = cudaMalloc((void**)&d_outmap, sizeof(float)*m);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_outmap returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }


    // create d_coeff
    cudaStat = cudaMalloc((void**)&d_coeff, sizeof(float)*NBmodes);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_coeff returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }


    if (sigaction(SIGINT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGBUS, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPIPE, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }


    loopOK = 1;
    iter = 0;

    printf("ENTERING LOOP, %ld modes (offsetmode = %d)\n", NBmodes, offsetmode);
    fflush(stdout);

    while(loopOK == 1)
    {

        if(data.image[IDcoeff].sem==0)
        {
            while(data.image[IDcoeff].md[0].cnt0==cnt) // test if new frame exists
                usleep(5);
            cnt = data.image[IDcoeff].md[0].cnt0;
            semr = 0;
        }
        else
        {


            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
            ts.tv_sec += 1;
            semr = sem_timedwait(data.image[IDcoeff].semptr[3], &ts);


            if(iter == 0)
            {
                //  printf("driving semaphore to zero ... ");
                // fflush(stdout);
                sem_getvalue(data.image[IDcoeff].semptr[2], &semval);
                for(scnt=0; scnt<semval; scnt++)
                    sem_trywait(data.image[IDcoeff].semptr[2]);
                // printf("done\n");
                // fflush(stdout);
            }
        }




        if(semr==0)
        {
            //  printf("Compute\n");
            //  fflush(stdout);

            // send vector back to GPU
            cudaStat = cudaMemcpy(d_coeff, data.image[IDcoeff].array.F, sizeof(float)*NBmodes, cudaMemcpyHostToDevice);
            if (cudaStat != cudaSuccess)
            {
                printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
                exit(EXIT_FAILURE);
            }

            if(offsetmode==1)
            {
                cudaStat = cudaMemcpy(d_outmap, data.image[IDoffset].array.F, sizeof(float)*m, cudaMemcpyHostToDevice);
                if (cudaStat != cudaSuccess)
                {
                    printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
                    exit(EXIT_FAILURE);
                }
            }

            // compute
            cublas_status = cublasSgemv(cublasH, CUBLAS_OP_N, m, NBmodes, &alpha, d_modes, m, d_coeff, 1, &beta, d_outmap, 1);
            if (cudaStat != CUBLAS_STATUS_SUCCESS)
            {
                printf("cublasSgemv returned error code %d, line(%d)\n", stat, __LINE__);
                if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                if(stat == CUBLAS_STATUS_INVALID_VALUE)
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                if(stat == CUBLAS_STATUS_ARCH_MISMATCH)
                    printf("   CUBLAS_STATUS_ARCH_MISMATCH\n");
                if(stat == CUBLAS_STATUS_EXECUTION_FAILED)
                    printf("   CUBLAS_STATUS_EXECUTION_FAILED\n");
                exit(EXIT_FAILURE);
            }

            // copy result
            data.image[IDoutmap].md[0].write = 1;
            cudaStat = cudaMemcpy(data.image[IDoutmap].array.F, d_outmap, sizeof(float)*m, cudaMemcpyDeviceToHost);
            sem_getvalue(data.image[IDoutmap].semptr[0], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[IDoutmap].semptr[0]);
            sem_getvalue(data.image[IDoutmap].semptr[1], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[IDoutmap].semptr[1]);
            data.image[IDoutmap].md[0].cnt0++;
            data.image[IDoutmap].md[0].write = 0;



        }

        if((data.signal_INT == 1)||(data.signal_TERM == 1)||(data.signal_ABRT==1)||(data.signal_BUS==1)||(data.signal_SEGV==1)||(data.signal_HUP==1)||(data.signal_PIPE==1))
            loopOK = 0;

        iter++;

    }


    cudaFree(d_modes);
    cudaFree(d_outmap);
    cudaFree(d_coeff);


    if (cublasH ) cublasDestroy(cublasH);



    return(0);
}





//
// extract mode coefficients from data stream
// modes need to be orthogonal
// single GPU computation
//
// in_stream                  input stream
// intot_strem   [optional]   input normalization stream
// IDmodes_name               Modes
// IDrefin_name               input reference  - to be subtracted
// IDrefout_name [optional]   output reference - to be added
// IDmodes_val_name           ouput
// GPUindex                   GPU index
// PROCESS                    1 if postprocessing
// TRACEMODE                  1 if writing trace
// MODENORM                   1 if input modes should be normalized
// insem                      input semaphore index
// axmode                     0 for normal mode extraction, 1 for expansion
// twait					  if >0, insert time wait [us] at each iteration
//
// IMPORTANT: if IDmodes_val_name exits, use it and do not compute it
//
// if IDrefout_name exists, match output image size to IDrefout_name
//

int CUDACOMP_extractModesLoop(const char *in_stream, const char *intot_stream, const char *IDmodes_name, const char *IDrefin_name, const char *IDrefout_name, const char *IDmodes_val_name, int GPUindex, int PROCESS, int TRACEMODE, int MODENORM, int insem, int axmode, long twait)
{
    long IDin;
    long IDintot;
    long IDmodes;
    long IDref;
    long ID;
    long ID_modeval;
    cublasHandle_t cublasH = NULL;
    cublasStatus_t cublas_status = CUBLAS_STATUS_SUCCESS;
    cudaError_t cudaStat = cudaSuccess;
    struct cudaDeviceProp deviceProp;
    int m, n;
    int k;
    long *arraytmp;

    float *d_modes = NULL; // linear memory of GPU
    float *d_in = NULL;
    float *d_modeval = NULL;

    float alpha = 1.0;
    float beta = 0.0;
    int loopOK;
    struct timespec ts;
    long iter;
    long long cnt = -1;
    long scnt;
    int semval;
    int semr;
    long ii, jj, kk;

    long NBmodes;
    float *normcoeff;

    long IDoutact;
    long *sizearraytmp;

    long ID_modeval_mult;
    int imOK;


    char traceim_name[200];
    long TRACEsize = 2000;
    long TRACEindex = 0;
    long IDtrace;


    int NBaveSTEP = 10; // each step is 2x longer average than previous step
    double stepcoeff;
    double stepcoeff0 = 0.3;
    char process_ave_name[200];
    char process_rms_name[200];
    long IDprocave;
    long IDprocrms;
    long step;

    long semnb;
    double tmpv;

    int INNORMMODE = 0; // 1 if input normalized

    float *modevalarray;
    float *modevalarrayref;

    int initref = 0; // 1 when reference has been processed
    int BETAMODE = 0;
    long IDrefout;

    long refindex;
	long twait1;
    struct timespec t0;
    struct timespec t1;

    int MODEVALCOMPUTE = 1; // 1 if compute, 0 if import


	int RT_priority = 80; //any number from 0-99
    struct sched_param schedpar;



    schedpar.sched_priority = RT_priority;
#ifndef __MACH__
    sched_setscheduler(0, SCHED_FIFO, &schedpar); 
#endif




    IDin = image_ID(in_stream);
    m = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1];
    COREMOD_MEMORY_image_set_createsem(in_stream, 10);

    // total flux
    IDintot = image_ID(intot_stream);

    if(IDintot==-1)
    {
        INNORMMODE = 0;
        IDintot = create_2Dimage_ID("intot_tmp", 1, 1);
        data.image[IDintot].array.F[0] = 1.0;
    }
    else
        INNORMMODE = 1;

    // reference
    IDref = image_ID(IDrefin_name);
    if(IDref==-1)
    {
        IDref = create_2Dimage_ID("_tmprefin", data.image[IDin].md[0].size[0], data.image[IDin].md[0].size[1]);
        for(ii=0; ii<data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]; ii++)
            data.image[IDref].array.F[ii] = 0.0;
    }



    if(axmode==0)
    {
        IDmodes = image_ID(IDmodes_name);
        n = data.image[IDmodes].md[0].size[2];
        NBmodes = n;
    }
    else
    {
        ID = image_ID(IDmodes_name);
        printf("ID = %ld\n", ID);
        fflush(stdout);

        NBmodes = data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1];
        n = NBmodes;
        printf("NBmodes = %ld\n", NBmodes);
        fflush(stdout);

        IDmodes = create_3Dimage_ID("_tmpmodes", data.image[IDin].md[0].size[0], data.image[IDin].md[0].size[1], NBmodes);

        for(ii=0; ii<data.image[IDin].md[0].size[0]; ii++)
            for(jj=0; jj<data.image[IDin].md[0].size[1]; jj++)
            {
                for(kk=0; kk<NBmodes; kk++)
                    data.image[IDmodes].array.F[kk*data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]+jj*data.image[IDin].md[0].size[0]+ii] = data.image[ID].array.F[NBmodes*(jj*data.image[IDin].md[0].size[0]+ii)+kk];
            }
        save_fits("_tmpmodes", "!_test_tmpmodes.fits");
    }



    normcoeff = (float*) malloc(sizeof(float)*NBmodes);

    if(MODENORM==1)
    {
        for(k=0; k<NBmodes; k++)
        {
            normcoeff[k] = 0.0;
            for(ii=0; ii<m; ii++)
                normcoeff[k] += data.image[IDmodes].array.F[k*m+ii] * data.image[IDmodes].array.F[k*m+ii];
        }
    }
    else
        for(k=0; k<NBmodes; k++)
            normcoeff[k] = 1.0;



    modevalarray = (float*) malloc(sizeof(float)*n);
    modevalarrayref = (float*) malloc(sizeof(float)*n);


    arraytmp = (long*) malloc(sizeof(long)*2);

    IDrefout = image_ID(IDrefout_name);
    if(IDrefout==-1)
    {
        arraytmp[0] = NBmodes;
        arraytmp[1] = 1;
    }
    else
    {
        arraytmp[0] = data.image[IDrefout].md[0].size[0];
        arraytmp[1] = data.image[IDrefout].md[0].size[1];
    }


    ID_modeval = image_ID(IDmodes_val_name);
    if(ID_modeval==-1) // CREATE IT
    {
        ID_modeval = create_image_ID(IDmodes_val_name, 2, arraytmp, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(IDmodes_val_name, 10);
        MODEVALCOMPUTE = 1;
    }
    else // USE STREAM, DO NOT COMPUTE IT
    {
        MODEVALCOMPUTE = 0;
        // drive semaphore to zero
        while(sem_trywait(data.image[ID_modeval].semptr[insem])==0) {}
    }

    free(arraytmp);

    if(MODEVALCOMPUTE == 1)
    {
        cudaGetDeviceCount(&deviceCount);
        printf("%d devices found\n", deviceCount);
        fflush(stdout);
        printf("\n");
        for (k = 0; k < deviceCount; ++k) {
            cudaGetDeviceProperties(&deviceProp, k);
            printf("Device %d [ %20s ]  has compute capability %d.%d.\n",
                   k, deviceProp.name, deviceProp.major, deviceProp.minor);
            printf("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n", (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
            printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
            printf("  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
            printf("\n");
        }


        if(GPUindex<deviceCount)
            cudaSetDevice(GPUindex);
        else
        {
            printf("Invalid Device : %d / %d\n", GPUindex, deviceCount);
            exit(0);
        }


        printf("Create cublas handle ...");
        fflush(stdout);
        cublas_status = cublasCreate(&cublasH);
        if (cublas_status != CUBLAS_STATUS_SUCCESS) {
            printf ("CUBLAS initialization failed\n");
            return EXIT_FAILURE;
        }
        printf(" done\n");
        fflush(stdout);


        // load modes to GPU
        cudaStat = cudaMalloc((void**)&d_modes, sizeof(float)*m*NBmodes);
        if (cudaStat != cudaSuccess)
        {
            printf("cudaMalloc d_modes returned error code %d, line %d\n", cudaStat, __LINE__);
            exit(EXIT_FAILURE);
        }
        cudaStat = cudaMemcpy(d_modes, data.image[IDmodes].array.F, sizeof(float)*m*NBmodes, cudaMemcpyHostToDevice);
        if (cudaStat != cudaSuccess)
        {
            printf("cudaMemcpy returned error code %d, line %d\n", cudaStat, __LINE__);
            exit(EXIT_FAILURE);
        }


        // create d_in
        cudaStat = cudaMalloc((void**)&d_in, sizeof(float)*m);
        if (cudaStat != cudaSuccess)
        {
            printf("cudaMalloc d_in returned error code %d, line %d\n", cudaStat, __LINE__);
            exit(EXIT_FAILURE);
        }


        // create d_modeval
        cudaStat = cudaMalloc((void**)&d_modeval, sizeof(float)*NBmodes);
        if (cudaStat != cudaSuccess)
        {
            printf("cudaMalloc d_modeval returned error code %d, line %d\n", cudaStat, __LINE__);
            exit(EXIT_FAILURE);
        }
    }



    if (sigaction(SIGINT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGBUS, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPIPE, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }


    loopOK = 1;
    iter = 0;



    if(TRACEMODE==1)
    {
        sizearraytmp = (long*) malloc(sizeof(long)*2);
        sprintf(traceim_name, "%s_trace", IDmodes_val_name);
        sizearraytmp[0] = TRACEsize;
        sizearraytmp[1] = NBmodes;
        IDtrace = image_ID(traceim_name);
        imOK = 1;
        if(IDtrace == -1)
            imOK = 0;
        else
        {
            if((data.image[IDtrace].md[0].size[0]!=TRACEsize)||(data.image[IDtrace].md[0].size[1]!=NBmodes))
            {
                imOK = 0;
                delete_image_ID(traceim_name);
            }
        }
        if(imOK==0)
            IDtrace = create_image_ID(traceim_name, 2, sizearraytmp, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(traceim_name, 10);
        free(sizearraytmp);
    }



    if(PROCESS==1)
    {
        sizearraytmp = (long*) malloc(sizeof(long)*2);
        sprintf(process_ave_name, "%s_ave", IDmodes_val_name);
        sizearraytmp[0] = NBmodes;
        sizearraytmp[1] = NBaveSTEP;
        IDprocave = image_ID(process_ave_name);
        imOK = 1;
        if(IDprocave == -1)
            imOK = 0;
        else
        {
            if((data.image[IDprocave].md[0].size[0]!=NBmodes)||(data.image[IDprocave].md[0].size[1]!=NBaveSTEP))
            {
                imOK = 0;
                delete_image_ID(process_ave_name);
            }
        }
        if(imOK==0)
            IDprocave = create_image_ID(process_ave_name, 2, sizearraytmp, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(process_ave_name, 10);
        free(sizearraytmp);

        sizearraytmp = (long*) malloc(sizeof(long)*2);
        sprintf(process_rms_name, "%s_rms", IDmodes_val_name);
        sizearraytmp[0] = NBmodes;
        sizearraytmp[1] = NBaveSTEP;
        IDprocrms = image_ID(process_rms_name);
        imOK = 1;
        if(IDprocrms == -1)
            imOK = 0;
        else
        {
            if((data.image[IDprocrms].md[0].size[0]!=NBmodes)||(data.image[IDprocrms].md[0].size[1]!=NBaveSTEP))
            {
                imOK = 0;
                delete_image_ID(process_rms_name);
            }
        }
        if(imOK==0)
            IDprocrms = create_image_ID(process_rms_name, 2, sizearraytmp, FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(process_rms_name, 10);
        free(sizearraytmp);
    }


    initref = 0;


	twait1 = twait;

    while(loopOK == 1)
    {
		clock_gettime(CLOCK_REALTIME, &t0);
				
        if(MODEVALCOMPUTE==1)
        {
            if(refindex != data.image[IDref].md[0].cnt0)
            {
                initref = 0;
                refindex = data.image[IDref].md[0].cnt0;
            }

            if(initref==1)
            {
                if(data.image[IDin].sem==0)
                {
                    while(data.image[IDin].md[0].cnt0==cnt) // test if new frame exists
                        usleep(5);
                    cnt = data.image[IDin].md[0].cnt0;
                    semr = 0;
                }
                else
                {
                    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        perror("clock_gettime");
                        exit(EXIT_FAILURE);
                    }
                    ts.tv_sec += 1;
                    semr = sem_timedwait(data.image[IDin].semptr[insem], &ts);

                    // drive semaphore to zero
                    while(sem_trywait(data.image[IDin].semptr[insem])==0) {}
                }
            }
            else // compute response of reference immediately
            {
                printf("COMPUTE NEW REFERENCE RESPONSE\n");
                semr = 0;
            }



            if(semr==0)
            {
                // load in_stream to GPU
                if(initref==0)
                    cudaStat = cudaMemcpy(d_in, data.image[IDref].array.F, sizeof(float)*m, cudaMemcpyHostToDevice);
                else
                    cudaStat = cudaMemcpy(d_in, data.image[IDin].array.F, sizeof(float)*m, cudaMemcpyHostToDevice);


                if (cudaStat != cudaSuccess)
                {
                    printf("initref = %d    %ld  %ld\n", initref, IDref, IDin);
                    printf("cudaMemcpy returned error code %d, line %d\n", cudaStat, __LINE__);
                    exit(EXIT_FAILURE);
                }

                if(BETAMODE == 1)
                {
                    beta = -1.0;
                    cudaStat = cudaMemcpy(d_modeval, modevalarrayref, sizeof(float)*NBmodes, cudaMemcpyHostToDevice);
                }

                // compute
                cublas_status = cublasSgemv(cublasH, CUBLAS_OP_T, m, NBmodes, &alpha, d_modes, m, d_in, 1, &beta, d_modeval, 1);
                if (cudaStat != CUBLAS_STATUS_SUCCESS)
                {
                    printf("cublasSgemv returned error code %d, line(%d)\n", stat, __LINE__);
                    if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                        printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                    if(stat == CUBLAS_STATUS_INVALID_VALUE)
                        printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                    if(stat == CUBLAS_STATUS_ARCH_MISMATCH)
                        printf("   CUBLAS_STATUS_ARCH_MISMATCH\n");
                    if(stat == CUBLAS_STATUS_EXECUTION_FAILED)
                        printf("   CUBLAS_STATUS_EXECUTION_FAILED\n");
                    exit(EXIT_FAILURE);
                }

                // copy result
                data.image[ID_modeval].md[0].write = 1;


                if(initref==0) // construct reference to be subtracted
                {
                    cudaStat = cudaMemcpy(modevalarrayref, d_modeval, sizeof(float)*NBmodes, cudaMemcpyDeviceToHost);

                    IDrefout = image_ID(IDrefout_name);
                    if(IDrefout != -1)
                        for(k=0; k<NBmodes; k++)
                            modevalarrayref[k] -= data.image[IDrefout].array.F[k];


                    if((INNORMMODE==0)&&(MODENORM==0))
                        BETAMODE = 1; // include ref subtraction in GPU operation
                    else
                        BETAMODE = 0;
                }
                else
                {
                    cudaStat = cudaMemcpy(modevalarray, d_modeval, sizeof(float)*NBmodes, cudaMemcpyDeviceToHost);

                    if(BETAMODE==0)
                    {
                        for(k=0; k<NBmodes; k++)
                            data.image[ID_modeval].array.F[k] = (modevalarray[k]/data.image[IDintot].array.F[0]-modevalarrayref[k])/normcoeff[k];
                    }
                    else
                        for(k=0; k<NBmodes; k++)
                            data.image[ID_modeval].array.F[k] = modevalarray[k];


                    COREMOD_MEMORY_image_set_sempost_byID(ID_modeval, -1);

                    data.image[ID_modeval].md[0].cnt0++;
                    data.image[ID_modeval].md[0].write = 0;
                }
            }
        }
        else // WAIT FOR NEW MODEVAL
        {
			sem_wait(data.image[ID_modeval].semptr[insem]);
		}










        if(TRACEMODE == 1)
        {
            data.image[ID_modeval].md[0].write = 1;

            for(k=0; k<NBmodes; k++)
                data.image[IDtrace].array.F[k*TRACEsize+TRACEindex] = data.image[ID_modeval].array.F[k];
            data.image[IDtrace].md[0].cnt1 = TRACEindex;

            sem_getvalue(data.image[IDtrace].semptr[0], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[IDtrace].semptr[0]);
            sem_getvalue(data.image[IDtrace].semptr[1], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[IDtrace].semptr[1]);
            data.image[IDtrace].md[0].cnt0++;
            data.image[IDtrace].md[0].write = 0;

            TRACEindex++;
            if(TRACEindex>=TRACEsize)
            {
                TRACEindex = 0;
                // copy to tracef shared memory (frozen trace)
            }
        }



        if(PROCESS==1)
        {
            stepcoeff = stepcoeff0;
            data.image[IDprocave].md[0].write = 1;
            for(step=0; step<NBaveSTEP; step++)
            {
                for(k=0; k<NBmodes; k++)
                    data.image[IDprocave].array.F[NBmodes*step+k] = (1.0-stepcoeff)*data.image[IDprocave].array.F[NBmodes*step+k] + stepcoeff*data.image[ID_modeval].array.F[k];
                stepcoeff *= stepcoeff0;
            }
            for(semnb=0; semnb<data.image[IDprocave].sem; semnb++)
            {
                sem_getvalue(data.image[IDprocave].semptr[semnb], &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDprocave].semptr[semnb]);
            }
            data.image[IDprocave].md[0].cnt0++;
            data.image[IDprocave].md[0].write = 0;

            stepcoeff = stepcoeff0;
            data.image[IDprocrms].md[0].write = 1;
            for(step=0; step<NBaveSTEP; step++)
            {
                for(k=0; k<NBmodes; k++)
                {
                    tmpv = data.image[ID_modeval].array.F[k] - data.image[IDprocave].array.F[NBmodes*step+k];
                    tmpv = tmpv*tmpv;
                    data.image[IDprocrms].array.F[NBmodes*step+k] = (1.0-stepcoeff)*data.image[IDprocrms].array.F[NBmodes*step+k] + stepcoeff*tmpv;
                }
                stepcoeff *= stepcoeff0;
            }
            for(semnb=0; semnb<data.image[IDprocrms].sem; semnb++)
            {
                sem_getvalue(data.image[IDprocrms].semptr[semnb], &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDprocrms].semptr[semnb]);
            }
            data.image[IDprocrms].md[0].cnt0++;
            data.image[IDprocrms].md[0].write = 0;
        }


		if(twait>0)
			usleep(twait1);
		
		if(twait1<0)
			twait1 = 0;
		
		clock_gettime(CLOCK_REALTIME, &t1);
        tdiff = info_time_diff(t0, t1);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
	
		if(tdiffv<1.0e-6*twait) 
			twait1 ++;
		else
			twait1 --;
		//printf("timing info : %11.9lf  %ld  %ld\n", tdiffv, );

        if((data.signal_INT == 1)||(data.signal_TERM == 1)||(data.signal_ABRT==1)||(data.signal_BUS==1)||(data.signal_SEGV==1)||(data.signal_HUP==1)||(data.signal_PIPE==1))
        {
            loopOK = 0;
            printf("Exiting loop\n");
            fflush(stdout);
            sleep(1.0);
        }

        initref = 1;
        iter++;
    }

    if(MODEVALCOMPUTE==1)
    {
        cudaFree(d_modes);
        cudaFree(d_in);
        cudaFree(d_modeval);



        if (cublasH ) cublasDestroy(cublasH);
    }

    free(normcoeff);
    free(modevalarray);
    free(modevalarrayref);




    return(0);
}

















// extract mode coefficients from data stream
/*
int CUDACOMP_createModesLoop(const char *DMmodeval_stream, const char *DMmodes, const char *DMact_stream, int GPUindex)
{
    long ID_DMmodeval;
    long ID_DMmodes;
    long ID_DMact;
    cublasHandle_t cublasH = NULL;
    cublasStatus_t cublas_status = CUBLAS_STATUS_SUCCESS;
    cudaError_t cudaStat = cudaSuccess;
    struct cudaDeviceProp deviceProp;
    int m, n;
    int k;
    long *arraytmp;

    float *d_DMmodes = NULL; // linear memory of GPU
    float *d_DMact = NULL;
    float *d_modeval = NULL;

    float alpha = 1.0;
    float beta = 0.0;
    int loopOK;
    struct timespec ts;
    long iter;
    long long cnt = -1;
    long scnt;
    int semval;
    int semr;
    long ii, kk;

    long NBmodes;
    
    float *normcoeff;
    
    

    ID_DMact = image_ID(DMact_stream);
    m = data.image[ID_DMact].md[0].size[0]*data.image[ID_DMact].md[0].size[1];

    ID_DMmodes = image_ID(DMmodes);
    n = data.image[ID_DMmodes].md[0].size[2];
    NBmodes = n;
    normcoeff = (float*) malloc(sizeof(float)*NBmodes);

    for(kk=0;kk<NBmodes;kk++)
        {
            normcoeff[kk] = 0.0;
            for(ii=0;ii<m;ii++)
                normcoeff[kk] += data.image[ID_DMmodes].array.F[kk*m+ii]*data.image[ID_DMmodes].array.F[kk*m+ii];            
            for(ii=0;ii<m;ii++)
                data.image[ID_DMmodes].array.F[kk*m+ii] /= normcoeff[kk];
        }

    //NBmodes = 3;

    arraytmp = (long*) malloc(sizeof(long)*2);
    arraytmp[0] = NBmodes;
    arraytmp[1] = 1;
    ID_modeval = create_image_ID(DMmodes_val, 2, arraytmp, FLOAT, 1, 0);
    free(arraytmp);
    COREMOD_MEMORY_image_set_createsem(DMmodes_val, 2);


    cudaGetDeviceCount(&deviceCount);
    printf("%d devices found\n", deviceCount);
    fflush(stdout);
    printf("\n");
    for (k = 0; k < deviceCount; ++k) {
        cudaGetDeviceProperties(&deviceProp, k);
        printf("Device %d [ %20s ]  has compute capability %d.%d.\n",
               k, deviceProp.name, deviceProp.major, deviceProp.minor);
        printf("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n", (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
        printf("  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
        printf("\n");
    }


    if(GPUindex<deviceCount)
        cudaSetDevice(GPUindex);
    else
    {
        printf("Invalid Device : %d / %d\n", GPUindex, deviceCount);
        exit(0);
    }
 

    printf("Create cublas handle ...");
    fflush(stdout);
    cublas_status = cublasCreate(&cublasH);
    if (cublas_status != CUBLAS_STATUS_SUCCESS) {
        printf ("CUBLAS initialization failed\n");
        return EXIT_FAILURE;
    }
    printf(" done\n");
    fflush(stdout);


    // load DMmodes to GPU
    cudaStat = cudaMalloc((void**)&d_DMmodes, sizeof(float)*m*NBmodes);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_DMmodes returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }
    cudaStat = cudaMemcpy(d_DMmodes, data.image[ID_DMmodes].array.F, sizeof(float)*m*NBmodes, cudaMemcpyHostToDevice);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }


    // create d_DMact
    cudaStat = cudaMalloc((void**)&d_DMact, sizeof(float)*m);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_DMact returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }

    // create d_modeval
    cudaStat = cudaMalloc((void**)&d_modeval, sizeof(float)*NBmodes);
    if (cudaStat != cudaSuccess)
    {
        printf("cudaMalloc d_modeval returned error code %d, line(%d)\n", cudaStat, __LINE__);
        exit(EXIT_FAILURE);
    }


    if (sigaction(SIGINT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGBUS, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPIPE, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }


    loopOK = 1;
    iter = 0;

    while(loopOK == 1)
    {
        if(data.image[ID_DMact].sem==0)
        {
            while(data.image[ID_DMact].md[0].cnt0==cnt) // test if new frame exists
                usleep(5);
            cnt = data.image[ID_DMact].md[0].cnt0;
            semr = 0;
        }
        else
        {
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
            ts.tv_sec += 1;
            semr = sem_timedwait(data.image[ID_DMact].semptr[0], &ts);

            if(iter == 0)
            {
                printf("driving semaphore to zero ... ");
                fflush(stdout);
                sem_getvalue(data.image[ID_DMact].semptr[0], &semval);
                for(scnt=0; scnt<semval; scnt++)
                    sem_trywait(data.image[ID_DMact].semptr[0]);
                printf("done\n");
                fflush(stdout);
            }
        }

        if(semr==0)
        {

            // load DMact to GPU
            cudaStat = cudaMemcpy(d_DMact, data.image[ID_DMact].array.F, sizeof(float)*m, cudaMemcpyHostToDevice);
            if (cudaStat != cudaSuccess)
            {
                printf("cudaMemcpy returned error code %d, line(%d)\n", cudaStat, __LINE__);
                exit(EXIT_FAILURE);
            }

            // compute
            cublas_status = cublasSgemv(cublasH, CUBLAS_OP_T, m, NBmodes, &alpha, d_DMmodes, m, d_DMact, 1, &beta, d_modeval, 1);
            if (cudaStat != CUBLAS_STATUS_SUCCESS)
            {
                printf("cublasSgemv returned error code %d, line(%d)\n", stat, __LINE__);
                if(stat == CUBLAS_STATUS_NOT_INITIALIZED)
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                if(stat == CUBLAS_STATUS_INVALID_VALUE)
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                if(stat == CUBLAS_STATUS_ARCH_MISMATCH)
                    printf("   CUBLAS_STATUS_ARCH_MISMATCH\n");
                if(stat == CUBLAS_STATUS_EXECUTION_FAILED)
                    printf("   CUBLAS_STATUS_EXECUTION_FAILED\n");
                exit(EXIT_FAILURE);
            }

            // copy result
            data.image[ID_modeval].md[0].write = 1;
            cudaStat = cudaMemcpy(data.image[ID_modeval].array.F, d_modeval, sizeof(float)*NBmodes, cudaMemcpyDeviceToHost);
            sem_getvalue(data.image[ID_modeval].semptr[0], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID_modeval].semptr[0]);
            sem_getvalue(data.image[ID_modeval].semptr[1], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID_modeval].semptr[1]);
            data.image[ID_modeval].md[0].cnt0++;
            data.image[ID_modeval].md[0].write = 0;
        }

        if((data.signal_INT == 1)||(data.signal_TERM == 1)||(data.signal_ABRT==1)||(data.signal_BUS==1)||(data.signal_SEGV==1)||(data.signal_HUP==1)||(data.signal_PIPE==1))
            loopOK = 0;
        
        iter++;
    }


    cudaFree(d_DMmodes);
    cudaFree(d_DMact);
    cudaFree(d_modeval);

    if (cublasH ) cublasDestroy(cublasH);

    free(normcoeff);

    return(0);
}

*/






















 


int_fast8_t GPUcomp_test(long NBact, long NBmodes, long WFSsize, long GPUcnt)
{
    long ID_contrM;
    long ID_WFS;
    long ID_cmd_modes;
    long *cmsize;
    long *wfssize;
    long *cmdmodessize;
    int_fast8_t status;
    int_fast8_t GPUstatus[100];
    long iter;
    long NBiter = 50000;
    double time1sec, time2sec;
    struct timespec tnow;
    int *GPUdevices;
    int k;
    double SVDeps = 0.1;

    long n, m;
    long *arraysizetmp;
    long ID, ID_R, ID_C;
    long ii, jj;
    float val;

    if(1==1)
    {
    //printf("Testing SVD on CPU\n");
      // linopt_compute_reconstructionMatrix("Rmat", "Cmat", SVDeps, "VTmat");
    
        create_2Dimage_ID("Rmat", WFSsize, WFSsize);
    
       printf("Testing SVD on GPU\n");
       GPU_SVD_computeControlMatrix(0, "Rmat", "Cmat", SVDeps, "VTmat");
        list_image_ID();
        printf("DONE ... ");
        fflush(stdout);
        
        
       // CHECK RESULT
     /*   arraysizetmp = (long*) malloc(sizeof(long)*3);
        ID_R = image_ID("Rmat");
        ID_C = image_ID("Cmat");

        if(data.image[ID_R].md[0].naxis==3)
        {
            m = data.image[ID_R].md[0].size[0]*data.image[ID_R].md[0].size[1];
            n = data.image[ID_R].md[0].size[2];
            printf("3D image -> %ld %ld\n", m, n);
            fflush(stdout);
        }
        else
        {
            m = data.image[ID_R].md[0].size[0];
            n = data.image[ID_R].md[0].size[1];
            printf("2D image -> %ld %ld\n", m, n);
            fflush(stdout);
        }
        
     
        printf("CHECKING RESULT ... ");
        fflush(stdout);
        
        ID = create_2Dimage_ID("SVDcheck", n, n);
        for(ii=0;ii<n;ii++)
            for(jj=0;jj<n;jj++)
                {
                    val = 0.0;
                    for(k=0;k<m;k++)
                        val += data.image[ID_C].array.F[ii*m+k] * data.image[ID_R].array.F[jj*m+k];
                    data.image[ID].array.F[jj*n+ii] = val;
                }
        save_fits("SVDcheck", "!SVDcheck.fits");
        printf("DONE\n");
        fflush(stdout);*/
    }
    else
    {
        printf("Testing GPU matrix multiplication speed, %ld GPUs\n", GPUcnt);


        GPUdevices = (int*) malloc(sizeof(int)*GPUcnt);
        for(k=0; k<GPUcnt; k++)
            GPUdevices[k] = k+8;

        //    GPUstatus = (int*) malloc(sizeof(int)*100);

        cmsize = (long*) malloc(sizeof(long)*3);
        cmsize[0] = WFSsize;
        cmsize[1] = WFSsize;
        cmsize[2] = NBmodes;
        ID_contrM = create_image_ID("cudatestcm", 3, cmsize, FLOAT, 1, 0);

        wfssize = (long*) malloc(sizeof(long)*2);
        wfssize[0] = WFSsize;
        wfssize[1] = WFSsize;
        ID_WFS = create_image_ID("cudatestwfs", 2, wfssize, FLOAT, 1, 0);

        cmdmodessize = (long*) malloc(sizeof(long)*2);
        cmdmodessize[0] = NBmodes;
        cmdmodessize[1] = 1;
        ID_cmd_modes = create_image_ID("cudatestcmd", 2, cmdmodessize, FLOAT, 1, 0);

        GPU_loop_MultMat_setup(0, data.image[ID_contrM].name, data.image[ID_WFS].name, data.image[ID_cmd_modes].name, GPUcnt, GPUdevices, 0, 1, 1, 0);

        clock_gettime(CLOCK_REALTIME, &tnow);
        time1sec = 1.0*((long) tnow.tv_sec) + 1.0e-9*tnow.tv_nsec;

        for(iter=0; iter<NBiter; iter++)
        {
            status = 0;
            GPU_loop_MultMat_execute(0, &status, &GPUstatus[0], 1.0, 0.0, 1);
        }
        clock_gettime(CLOCK_REALTIME, &tnow);
        time2sec = 1.0*((long) tnow.tv_sec) + 1.0e-9*tnow.tv_nsec;

        printf("Frequ = %12.3f Hz\n", 1.0*NBiter/(time2sec-time1sec));

        printf("done\n");
        fflush(stdout);

        delete_image_ID("cudatestcm");
        delete_image_ID("cudatestwfs");
        delete_image_ID("cudatestcmd");

        free(cmsize);
        free(wfssize);
        free(cmdmodessize);
        free(GPUdevices);
    }

    return(0);
}



#endif










