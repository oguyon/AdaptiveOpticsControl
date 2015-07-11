#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sched.h>


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

 #endif


#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"

#include "cudacomp/cudacomp.h"

# ifdef _OPENMP
# include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
# endif





extern DATA data;


#ifdef HAVE_CUDA
int deviceCount;

GPUMATMULTCONF gpumatmultconf[10]; // supports up to 10 configurations


cudaError_t error;
cublasStatus_t stat;
float cublasSgemv_alpha = 1.0;
float cublasSgemv_beta  = 0.0;





#endif



// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//

#ifdef HAVE_CUDA
int CUDACOMP_test_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)
        GPUcomp_test(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
    else
        return 1;
}
#endif




int init_cudacomp()
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
    strcpy(data.cmd[data.NBcmd].example,"cudacomptest");
    strcpy(data.cmd[data.NBcmd].Ccall,"GPUcomp_test(long NBact, long NBmodes, long WFSsize, long GPUcnt)");
    data.NBcmd++;
		
    
#endif
    // add atexit functions here

    return 0;
}









#ifdef HAVE_CUDA
int CUDACOMP_init()
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
    long ID;
    
    
    printf("LOADING MATRIX TO GPU ... ");
    fflush(stdout);

    // TEST
    ID = create_2Dimage_ID("mgputest", gpumatmultconf[index].N, gpumatmultconf[index].M);
    for(n=0;n<gpumatmultconf[index].N;n++)
        for(m=0;m<gpumatmultconf[index].M;m++)
            data.image[ID].array.F[m*gpumatmultconf[index].N+n] = gpumatmultconf[index].cMat[m*gpumatmultconf[index].N+n];
    save_fits("mgputest","!MgpuTest.fits");
    delete_image_ID("mgputest");
    

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
        cudaSetDevice(device);
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

int GPU_loop_MultMat_setup(int index, char *IDcontrM_name, char *IDwfsim_name, char *IDoutdmmodes_name, long NBGPUs, int orientation, int USEsem, int initWFSref)
{
    long IDcontrM, IDwfsim, IDwfsref;
    long *sizearraytmp;
    int device;
    struct cudaDeviceProp deviceProp;
    int n, m;
    char sname[200];

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
            printf("[0] [%ld] M = %d\n", IDcontrM, gpumatmultconf[index].M);
            printf("[0] [%ld] N = %d\n", IDcontrM, gpumatmultconf[index].N);
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

            printf("[1] [%ld] M = %d\n", IDcontrM, gpumatmultconf[index].M);
            printf("[1] [%ld] N = %d\n", IDcontrM, gpumatmultconf[index].N);
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
            
            if(data.image[IDwfsim].md[0].size[0]*data.image[IDwfsim].md[0].size[1]!=gpumatmultconf[index].N)
            {
                printf("ERROR: CONTRmat and WFSvec size not compatible: %ld %d\n", data.image[IDwfsim].md[0].size[0]*data.image[IDwfsim].md[0].size[1], gpumatmultconf[index].N);
                fflush(stdout);
                sleep(2);
                exit(0);
            }
        }
        else
        {
            printf("[1] Input vector size: %ld \n", data.image[IDwfsim].md[0].size[0]);
            if(data.image[IDwfsim].md[0].size[0]!=gpumatmultconf[index].N)
            {
                printf("ERROR: CONTRmat and WFSvec size not compatible: %ld %d\n", data.image[IDwfsim].md[0].size[0], gpumatmultconf[index].N);
                fflush(stdout);
                sleep(2);
                exit(0);
            }
        }


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
            if(data.image[gpumatmultconf[index].IDout].md[0].size[0] * data.image[gpumatmultconf[index].IDout].md[0].size[1] != gpumatmultconf[index].M)
            {
                printf("ERROR: CONTRmat and WFSvec size not compatible: %ld %d\n", data.image[gpumatmultconf[index].IDout].md[0].size[0] * data.image[gpumatmultconf[index].IDout].md[0].size[1], gpumatmultconf[index].M);
                printf("gpumatmultconf[index].IDout = %ld\n", gpumatmultconf[index].IDout);
                list_image_ID();
                fflush(stdout);
                sleep(2);
                exit(0);
            }
        }

        gpumatmultconf[index].dmVecTMP = data.image[gpumatmultconf[index].IDout].array.F;




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
        }


        gpumatmultconf[index].NBstreams = deviceCount;
        if(NBGPUs<deviceCount)
            gpumatmultconf[index].NBstreams = NBGPUs;
        



        gpumatmultconf[index].Nsize = (int*) malloc(sizeof(long)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].Noffset = (int*) malloc(sizeof(long)*gpumatmultconf[index].NBstreams);
        gpumatmultconf[index].Noffset[0] = 0;
        for(device=1; device<gpumatmultconf[index].NBstreams; device++)
        {
            gpumatmultconf[index].Noffset[device] = gpumatmultconf[index].Noffset[device-1] + (long) (gpumatmultconf[index].N/gpumatmultconf[index].NBstreams);
            gpumatmultconf[index].Nsize[device-1] = gpumatmultconf[index].Noffset[device] - gpumatmultconf[index].Noffset[device-1];
        }
        gpumatmultconf[index].Nsize[gpumatmultconf[index].NBstreams-1] = gpumatmultconf[index].N-gpumatmultconf[index].Noffset[gpumatmultconf[index].NBstreams-1];

        printf("-----------------------------------------------------\n");
        for(device=0; device<gpumatmultconf[index].NBstreams; device++)
            printf("DEVICE %d  :  %5d -> %5d  (%d)\n", device, gpumatmultconf[index].Noffset[device], gpumatmultconf[index].Noffset[device]+gpumatmultconf[index].Nsize[device], gpumatmultconf[index].Nsize[device]);
        printf("-----------------------------------------------------\n");




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

        gpumatmultconf[index].refWFSinit = (int*) malloc(sizeof(int)*gpumatmultconf[index].NBstreams);


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

            sprintf(sname, "i%d_gpu%d_sem1", index, device);
            if ((gpumatmultconf[index].semptr1[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr1[device], 1, 0);

            sprintf(sname, "i%d_gpu%d_sem2", index, device);
            if ((gpumatmultconf[index].semptr2[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr2[device], 1, 0);

            sprintf(sname, "i%d_gpu%d_sem3", index, device);
            if ((gpumatmultconf[index].semptr3[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr3[device], 1, 0);

            sprintf(sname, "i%d_gpu%d_sem4", index, device);
            if ((gpumatmultconf[index].semptr4[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr4[device], 1, 0);

            sprintf(sname, "i%d_gpu%d_sem5", index, device);
            if ((gpumatmultconf[index].semptr5[device] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                perror("semaphore initilization");
                exit(0);
            }
            sem_init(gpumatmultconf[index].semptr5[device], 1, 0);

        }




        for (device = 0; device < gpumatmultconf[index].NBstreams; device++)
        {
            cudaSetDevice(device);
            cudaStreamCreate( &gpumatmultconf[index].stream[device]);
        }

        for(device=0; device<gpumatmultconf[index].NBstreams; device++)
        {
            cudaSetDevice(device);

            // ALLOCATE MEMORY ON DEVICE

            error = cudaMalloc((void **) &gpumatmultconf[index].d_cMat[device], sizeof(float)*gpumatmultconf[index].M*gpumatmultconf[index].Nsize[device]);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_cMat returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }


            error = cudaMalloc((void **) &gpumatmultconf[index].d_wfsVec[device], sizeof(float)*gpumatmultconf[index].Nsize[device]);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_wfsVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }

            error = cudaMalloc((void **) &gpumatmultconf[index].d_wfsRef[device], sizeof(float)*gpumatmultconf[index].Nsize[device]);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_wfsRef returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }

            error = cudaMalloc((void **) &gpumatmultconf[index].d_dmVec[device], sizeof(float)*gpumatmultconf[index].M);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_dmVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }

            error = cudaMalloc((void **) &gpumatmultconf[index].d_dmRef[device], sizeof(float)*gpumatmultconf[index].M);
            if (error != cudaSuccess)
            {
                printf("cudaMalloc d_dmVec returned error code %d, line(%d)\n", error, __LINE__);
                exit(EXIT_FAILURE);
            }


            stat = cublasCreate(&gpumatmultconf[index].handle[device]);
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

        printf("...\n");
        fflush(stdout);
    }
    
    for(device=0; device<gpumatmultconf[index].NBstreams; device++)
       gpumatmultconf[index].refWFSinit[device] = initWFSref;


    return(0);
}



// increments status by 4
int GPU_loop_MultMat_execute(int index, int *status, int *GPUstatus, float alpha, float beta)
{
    int m;
    int ptn;
    int statustot;


    cublasSgemv_alpha = alpha;
    cublasSgemv_beta = beta;

    *status = *status + 1;  // ->9

    if(index==0) /// main CM multiplication loop
    {
        //	gpumatmultconf[index].NBstreams = 6;
        if(gpumatmultconf[index].CM_cnt != data.image[gpumatmultconf[index].CM_ID].md[0].cnt0)
            if(data.image[gpumatmultconf[index].CM_ID].md[0].write == 0)
            {
                printf("New CM detected (cnt : %ld)\n", data.image[gpumatmultconf[index].CM_ID].md[0].cnt0);
                GPUloadCmat(index);
                gpumatmultconf[index].CM_cnt = data.image[gpumatmultconf[index].CM_ID].md[0].cnt0;
            }
    }


    // index is the matrix multiplication index (unique to each matrix multiplication stream operation)
    // ptn is the thread number = GPU device number
    if((gpumatmultconf[index].sem==0)||(gpumatmultconf[index].gpuinit==0))
    {
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
    *status = *status + 1;  // -> 10


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
    }



    // SUM RESULTS FROM SEPARATE GPUs
    *status = *status + 1;  // -> 11

    data.image[gpumatmultconf[index].IDout].md[0].write = 0;

    for(m=0; m<gpumatmultconf[index].M; m++)
        gpumatmultconf[index].dmVecTMP[m] = 0.0; //gpumatmultconf[index].NBstreams+0.35;

    for(ptn=0; ptn<gpumatmultconf[index].NBstreams; ptn++)
    {
        for(m=0; m<gpumatmultconf[index].M; m++)
            gpumatmultconf[index].dmVecTMP[m] += gpumatmultconf[index].dmVec_part[ptn][m];
    }
    if(data.image[gpumatmultconf[index].IDout].sem == 1)
        sem_post(data.image[gpumatmultconf[index].IDout].semptr);
        
    if(data.image[gpumatmultconf[index].IDout].sem1 == 1)
        sem_post(data.image[gpumatmultconf[index].IDout].semptr1);

    if(data.image[gpumatmultconf[index].IDout].semlog == 1)
        sem_post(data.image[gpumatmultconf[index].IDout].semptrlog);

    data.image[gpumatmultconf[index].IDout].md[0].write = 0;
    data.image[gpumatmultconf[index].IDout].md[0].cnt0++;

    *status = *status + 1; // -> 12

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

    return(0);
}






/* 
 *
 * sequence of events :
 * 
 * wait semptr1
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
    char *ptr0; // source
    char *ptr1; // dest
    float *ptr0f; // test
    int *ptrstat;
    long IDtest;
    int k;
    int kmax = 10;
    char imname[200];
    char fname[200];
    long long iter;
    long long itermax = 1;
    float imtot;
    float alphatmp;
    float betatmp;


    thdata = (THDATA*) ptr;
    device = thdata->thread_no;
    index = thdata->cindex;

    ptrstat = (int*) ((char*) thdata->status + sizeof(int)*device + sizeof(int)*10*index);



    *ptrstat = 1;




    ptr0 = (char*) gpumatmultconf[index].wfsVec;
    ptr0 += sizeof(float)*gpumatmultconf[index].Noffset[device];
    ptr0f = (float*) ptr0;

    cudaSetDevice(device);

    cublasSetStream( gpumatmultconf[index].handle[device], gpumatmultconf[index].stream[device] );



    if(gpumatmultconf[index].sem==1)
        itermax = -1;
    else
        itermax = 1;

    iter = 0;
    while(iter != itermax)
    {


        // copy DM reference to output to prepare computation:   d_dmVec <- d_dmRef
        error = cudaMemcpy(gpumatmultconf[index].d_dmVec[device], gpumatmultconf[index].d_dmRef[device], sizeof(float)*gpumatmultconf[index].M, cudaMemcpyDeviceToDevice);
        if (error != cudaSuccess)
        {
            printf("cudaMemcpy d_wfsVec wfsVec returned error code %d, line(%d)\n", error, __LINE__);
            exit(EXIT_FAILURE);
        }


        // TEST
        //   printf("[%d] TEST : STORING DM ref\n", device);
        //   fflush(stdout);
        /*    stat = cublasGetVector(gpumatmultconf[index].M, sizeof(float), gpumatmultconf[index].d_dmRef[device], 1, gpumatmultconf[index].dmRef_part[device], 1);
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

            imtot = 0.0;
            for(m=0; m<gpumatmultconf[index].M; m++)
                imtot += gpumatmultconf[index].dmRef_part[device][m]*gpumatmultconf[index].dmRef_part[device][m];
            printf("[%d]  [%g %g]  RMS DM ref = %g\n", device, cublasSgemv_alpha, cublasSgemv_beta, sqrt(imtot));
        */



        *ptrstat = 2; // wait for image
        if(gpumatmultconf[index].sem==1)
        {
            //printf("GPU SEMAPHORE :  WAITING FOR SEM1     index %d   device %d ...\n", index, device);
            //fflush(stdout);
            sem_wait(gpumatmultconf[index].semptr1[device]);
            //printf("GPU SEMAPHORE :  WAITING FOR SEM1     index %d   device %d -> MOVING FORWARD\n", index, device);
            //fflush(stdout);
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

            /*    imtot = 0.0;
                for(n=0; n<gpumatmultconf[index].Nsize[device]; n++)
                    imtot += ptr0f[n];
                printf("[%d] TOTAL wfsREF = %g\n", device, imtot);
            */
            // initialization: compute dm ref from wfs ref
            /*
                     d_wfsRef -> wfsRef_part
                    stat = cublasGetVector(gpumatmultconf[index].Nsize[device], sizeof(float), gpumatmultconf[index].d_wfsVec[device], 1, gpumatmultconf[index].wfsRef_part[device], 1);
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



                    sprintf(imname, "test_wfsRef_part%d", device);
                    IDtest = create_2Dimage_ID(imname, gpumatmultconf[index].Nsize[device], 1);
                    printf("[%d] Compute new reference response [1a/3]\n", device);
                    fflush(stdout);
                    for(n=0; n<gpumatmultconf[index].Nsize[device]; n++)
                        data.image[IDtest].array.F[n] = gpumatmultconf[index].wfsRef_part[device][n];
                    printf("[%d] Compute new reference response [1b/3]\n", device);
                    fflush(stdout);
                    sprintf(fname, "!test_wfsRef_part%d.fits", device);
                    save_fits(imname, fname);
            */
            //        printf("[%d] Compute new reference response [2/3]\n", device);
            //       fflush(stdout);


            // input : gpumatmultconf[index].d_wfsRef[device]
            // ouput : gpumatmultconf[index].d_dmRef[device]

            alphatmp = cublasSgemv_alpha;
            betatmp = cublasSgemv_beta;

            cublasSgemv_alpha = 1.0;
            cublasSgemv_beta = 0.0;
            stat = cublasSgemv(gpumatmultconf[index].handle[device], CUBLAS_OP_N, gpumatmultconf[index].M, gpumatmultconf[index].Nsize[device], &cublasSgemv_alpha, gpumatmultconf[index].d_cMat[device], gpumatmultconf[index].M, gpumatmultconf[index].d_wfsVec[device], 1, &cublasSgemv_beta, gpumatmultconf[index].d_dmRef[device], 1);
            if (stat != CUBLAS_STATUS_SUCCESS)
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
            cublasSgemv_alpha = alphatmp;
            cublasSgemv_beta = betatmp;

            gpumatmultconf[index].refWFSinit[device] = 1;



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

            /* sprintf(imname, "test_dmRef_part%d", device);
             IDtest = create_2Dimage_ID(imname, gpumatmultconf[index].M, 1);
             for(m=0; m<gpumatmultconf[index].M; m++)
                 data.image[IDtest].array.F[m] = gpumatmultconf[index].dmRef_part[device][m];
             sprintf(fname, "!test_dmRef_part%d.fits", device);
             save_fits(imname, fname);*/
        }
        else
        {
            //     printf("GPU_alpha = %g   GPU_beta = %g\n", cublasSgemv_alpha, cublasSgemv_beta);
            //    fflush(stdout);





            *ptrstat = 4; // compute

            if(gpumatmultconf[index].sem==1)
            {
                //printf("GPU SEMAPHORE :  POSTING SEM2     index %d   device %d\n", index, device);
                //fflush(stdout);
                sem_post(gpumatmultconf[index].semptr2[device]);
            }




            // d_wfsRef -> wfsRef_part
            /*      stat = cublasGetVector(gpumatmultconf[index].Nsize[device], sizeof(float), gpumatmultconf[index].d_wfsVec[device], 1, gpumatmultconf[index].wfsRef_part[device], 1);
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
                  imtot = 0.0;
                  for(n=0; n<gpumatmultconf[index].Nsize[device]; n++)
                      imtot += gpumatmultconf[index].wfsRef_part[device][n];
                  printf("[%d]   %g   TOT WFS  ref = %g\n", device, cublasSgemv_alpha, imtot);
            */






            stat = cublasSgemv(gpumatmultconf[index].handle[device], CUBLAS_OP_N, gpumatmultconf[index].M, gpumatmultconf[index].Nsize[device], &cublasSgemv_alpha, gpumatmultconf[index].d_cMat[device], gpumatmultconf[index].M, gpumatmultconf[index].d_wfsVec[device], 1, &cublasSgemv_beta, gpumatmultconf[index].d_dmVec[device], 1);

            if (stat != CUBLAS_STATUS_SUCCESS)
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


            if(gpumatmultconf[index].sem==1)
            {
                //printf("GPU SEMAPHORE :  POSTING SEM3     index %d   device %d\n", index, device);
                //fflush(stdout);
                sem_post(gpumatmultconf[index].semptr3[device]);
            }



            *ptrstat = 5; // transfer result


            //cudaMemcpy(cpu.r, gpu.r, gpumatmultconf[index].M * sizeof(float), cudaMemcpyDeviceToHost);

            if(gpumatmultconf[index].sem==1)
            {
                //printf("GPU SEMAPHORE :  WAITING FOR SEM4     index %d   device %d ...\n", index, device);
                //fflush(stdout);
                sem_wait(gpumatmultconf[index].semptr4[device]);
                //printf("GPU SEMAPHORE :  WAITING FOR SEM4     index %d   device %d -> MOVING FORWARD\n", index, device);
                //fflush(stdout);
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

            //    *ptrstat = 6;
        }
        if(gpumatmultconf[index].sem==1)
        {
            //printf("GPU SEMAPHORE :  POSTING SEM5     index %d   device %d\n", index, device);
            //fflush(stdout);
            sem_post(gpumatmultconf[index].semptr5[device]);
        }
        *ptrstat = 6;

        // START MODE VALUES COMPUTATION HERE
        
        
        
        iter++;
    }


    pthread_exit(0);
}













int GPUcomp_test(long NBact, long NBmodes, long WFSsize, long GPUcnt)
{
    long ID_contrM;
    long ID_WFS;
    long ID_cmd_modes;
    long *cmsize;
    long *wfssize;
    long *cmdmodessize;
    int status;
    int *GPUstatus;
    long iter;
    long NBiter = 10000;
    double time1sec, time2sec;
    struct timespec tnow;


    printf("Testing GPU matrix multiplication speed\n");

    GPUstatus = (int*) malloc(sizeof(int)*100);

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

    GPU_loop_MultMat_setup(0, data.image[ID_contrM].md[0].name, data.image[ID_WFS].md[0].name, data.image[ID_cmd_modes].md[0].name, GPUcnt, 0, 0, 1);

    clock_gettime(CLOCK_REALTIME, &tnow);
    time1sec = 1.0*((long) tnow.tv_sec) + 1.0e-9*tnow.tv_nsec;

    for(iter=0; iter<NBiter; iter++)
        GPU_loop_MultMat_execute(0, &status, GPUstatus, 1.0, 0.0);

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

    free(GPUstatus);

    return(0);
}


#endif










