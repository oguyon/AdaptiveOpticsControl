#ifndef _CUDACOMP_H
#define _CUDACOMP_H


#ifdef HAVE_CUDA

#include <cuda_runtime_api.h>
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <device_types.h>
#include <pthread.h>

#endif

#ifdef HAVE_CUDA

// data passed to each thread
typedef struct
{
    int thread_no;
    long numl0;
    int cindex; // computation index
    int_fast8_t *status; // where to white status
} THDATA;



/** \brief This structure holds the GPU computation setup for matrix multiplication
 *
 *  By declaring an array of these structures,
 * several parallel computations can be executed
 *
 */

typedef struct
{
    int_fast8_t init; /// 1 if initialized
    int_fast8_t *refWFSinit; /// reference init
    int_fast8_t alloc; /// 1 if memory has been allocated
    long CM_ID;
    long CM_cnt;
    long timerID;
    
    uint_fast32_t M;
    uint_fast32_t N;


    // synchronization
    int_fast8_t sem; // if sem = 1, wait for semaphore to perform computation
    int_fast8_t gpuinit;
    
    // one semaphore per thread
    sem_t **semptr1; // command to start matrix multiplication (input)
    sem_t **semptr2; // memory transfer to device completed (output)
    sem_t **semptr3; // computation done (output)
    sem_t **semptr4; // command to start transfer to host (input)
    sem_t **semptr5; // output transfer to host completed (output)

    // computer memory (host)
    float *cMat;
    float **cMat_part;
    float *wfsVec;
    float **wfsVec_part;
    float *wfsRef;
    float **wfsRef_part;
    float *dmVec;
    float *dmVecTMP;
    float **dmVec_part;
    float **dmRef_part;

    // GPU memory (device)
    float **d_cMat;
    float **d_wfsVec;
    float **d_dmVec;
    float **d_wfsRef;
    float **d_dmRef;

    // threads
    THDATA *thdata;
    int *iret;
    pthread_t *threadarray;
    int_fast8_t NBstreams;
    cudaStream_t *stream;
    cublasHandle_t *handle;

    // splitting limits
    uint_fast32_t *Nsize;
    uint_fast32_t *Noffset;

    int *GPUdevice;

    int_fast8_t orientation;

    long IDout;


} GPUMATMULTCONF;
#endif



int_fast8_t init_cudacomp();

int_fast8_t CUDACOMP_init();

#ifdef HAVE_CUDA
void matrixMulCPU(float *cMat, float *wfsVec, float *dmVec, int M, int N);

int GPUloadCmat(int index);

int GPU_loop_MultMat_setup(int index, const char *IDcontrM_name, const char *IDwfsim_name, const char *IDoutdmmodes_name, long NBGPUs, int *GPUdevice, int orientation, int USEsem, int initWFSref, long loopnb);

int GPU_loop_MultMat_execute(int index, int_fast8_t *status, int_fast8_t *GPUstatus, float alpha, float beta, int timing);

int GPU_loop_MultMat_free(int index);


#ifdef HAVE_MAGMA
int CUDACOMP_magma_compute_SVDpseudoInverse(const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, long MaxNBmodes, const char *ID_VTmatrix_name, int LOOPmode);
#endif

void *compute_function( void *ptr );
int CUDACOMP_Coeff2Map_Loop(const char *IDmodes_name, const char *IDcoeff_name, int GPUindex, const char *IDoutmap_name, int offsetmode, const char *IDoffset_name);

int CUDACOMP_extractModesLoop(const char *in_stream, const char *intot_stream, const char *IDmodes_name, const char *IDrefin_name, const char *IDrefout_name, const char *IDmodes_val_name, int GPUindex, int PROCESS, int TRACEMODE, int MODENORM, int insem, int axmode, long twait);

int_fast8_t GPUcomp_test(long NBact, long NBmodes, long WFSsize, long GPUcnt);

#endif


#endif
