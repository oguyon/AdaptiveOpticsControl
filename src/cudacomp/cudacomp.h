/**
 * @file    cudacomp.h
 * @brief   Function prototypes for CUDA/MAGMA wrapper 
 * 
 * Also uses MAGMA library
 * 
 * @author  O. Guyon
 * @date    3 Jul 2017
 *
 * @bug Magma can hang on magma_dsyevd_gpu
 * 
 */


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
 * By declaring an array of these structures,
 * several parallel computations can be executed
 *
 */

typedef struct
{
    int_fast8_t init;                   /**< 1 if initialized               */
    int_fast8_t *refWFSinit;            /**< reference init                 */
    int_fast8_t alloc;                  /**< 1 if memory has been allocated */
    long CM_ID;                         
    long CM_cnt;
    long timerID;
    
    uint_fast32_t M;
    uint_fast32_t N;


    /// synchronization
    int_fast8_t sem;                    /**< if sem = 1, wait for semaphore to perform computation */
    int_fast8_t gpuinit;
    
    /// one semaphore per thread
    sem_t **semptr1;                   /**< command to start matrix multiplication (input) */
    sem_t **semptr2;                   /**< memory transfer to device completed (output)   */
    sem_t **semptr3;                   /**< computation done (output)                      */
    sem_t **semptr4;                   /**< command to start transfer to host (input)      */
    sem_t **semptr5;                   /**< output transfer to host completed (output)     */

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






/* =============================================================================================== */
/* =============================================================================================== */
/** @name 1. INITIALIZATION
 *  Initialization functions
 */
///@{
/* =============================================================================================== */
/* =============================================================================================== */


/**
 * @brief Initialize cudacomp module and command line interface.
 */
int_fast8_t init_cudacomp();


/**
 * @brief Initialize CUDA and MAGMA
 * 
 * Finds CUDA devices
 * Initializes CUDA and MAGMA libraries
 * 
 * @return number of CUDA devices found
 * 
 */
int_fast8_t CUDACOMP_init();

int CUDACOMP_printGPUMATMULTCONF(int index);

int_fast8_t GPUcomp_test(long NBact, long NBmodes, long WFSsize, long GPUcnt);

///@}





#ifdef HAVE_CUDA

/* =============================================================================================== */
/* =============================================================================================== */
/** @name 2. LOW-LEVEL MATRIX VECTOR MULTIPLICATION FUNCTIONS 
 *  Multi-GPU Matrix Vector multiplication
 * 
 * 
 */
///@{
/* =============================================================================================== */
/* =============================================================================================== */



/** @brief CPU-based matrix vector multiplication */
void matrixMulCPU(float *cMat, float *wfsVec, float *dmVec, int M, int N);


void *compute_function( void *ptr );


int GPUloadCmat(int index);


/** @brief Setup memory and process for GPU-based matrix-vector multiply */
int GPU_loop_MultMat_setup(int index, const char *IDcontrM_name, const char *IDwfsim_name, const char *IDoutdmmodes_name, long NBGPUs, int *GPUdevice, int orientation, int USEsem, int initWFSref, long loopnb);


int GPU_loop_MultMat_execute(int index, int_fast8_t *status, int_fast8_t *GPUstatus, float alpha, float beta, int timing, int TimerOffsetIndex);


int GPU_loop_MultMat_free(int index);

///@}



#ifdef HAVE_MAGMA
/* =============================================================================================== */
/* =============================================================================================== */
/** @name 3. SINGULAR VALUE DECOMPOSITION, PSEUDO-INVERSE
 *  Call to MAGMA SVD
 */
///@{
/* =============================================================================================== */
/* =============================================================================================== */


long CUDACOMP_MatMatMult_testPseudoInverse(const char *IDmatA_name, const char *IDmatAinv_name, const char *IDmatOut_name);



/**
 * @brief Compute pseudoinverse using MAGMA-based SVD
 */
int CUDACOMP_magma_compute_SVDpseudoInverse_SVD(const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, long MaxNBmodes, const char *ID_VTmatrix_name);



/**
 * @brief Computes matrix pseudo-inverse (AT A)^-1 AT, using eigenvector/eigenvalue decomposition of AT A
 *
 * Regularization (eigenvalue cutoff) is set by parameters SVDeps and MaxNBmodes
 * Both contraints are applied, so that the number of modes is the minimum of both constraints
 * 
 * @param[in]   ID_Rmatrix_name     Input data matrix, can be 2D or 3D
 * @param[out]  ID_Cmatrix_name     Pseudoinverse (result) 
 * @param[in]   SVDeps              SVD eigenvalue limit for pseudoinverse
 * @param[in]   MaxNBmodes          Maximum number of modes kept 
 * @param[out]  ID_VTmatrix_name    VT output matrix
 * @param[in]   LOOPmode            if set to 1, repeat routine as input data is updated
 *
 * @return void
 * 
 * @note When called by AOloopControl module to compute control matrix, N = number of actuators, M = number of sensors 
 * 
 * @warning Requires M>N (tall matrix)
 */
int CUDACOMP_magma_compute_SVDpseudoInverse(const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, long MaxNBmodes, const char *ID_VTmatrix_name, int LOOPmode);



int GPU_SVD_computeControlMatrix(int device, const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, double SVDeps, const char *ID_VTmatrix_name);

///@}

#endif



/* =============================================================================================== */
/* =============================================================================================== */
/** @name 4. HIGH LEVEL FUNCTIONS
 *  
 */
///@{
/* =============================================================================================== */
/* =============================================================================================== */


int CUDACOMP_Coeff2Map_Loop(const char *IDmodes_name, const char *IDcoeff_name, int GPUindex, const char *IDoutmap_name, int offsetmode, const char *IDoffset_name);



/**
 * @brief extract mode coefficients from data stream
 * 
 * modes need to be orthogonal
 * single GPU computation
 * 
 * @param[in]   in_stream            input stream
 * @param[in]   intot_stream         [optional]   input normalization stream
 * @param[in]   IDmodes_name         Modes
 * @param[in]   IDrefin_name         [optional] input reference  - to be subtracted
 * @param[in]   IDrefout_name        [optional] output reference - to be added
 * @param[out]  IDmodes_val_name     ouput stream
 * @param[in]   GPUindex             GPU index
 * @param[in]   PROCESS              1 if postprocessing
 * @param[in]   TRACEMODE            1 if writing trace
 * @param[in]   MODENORM             1 if input modes should be normalized
 * @param[in]   insem                input semaphore index
 * @param[in]   axmode               0 for normal mode extraction, 1 for expansion
 * @param[in]   twait		         if >0, insert time wait [us] at each iteration
 * 
 * @note IMPORTANT: if IDmodes_val_name exits, use it and do not compute it
 * 
 * @note if IDrefout_name exists, match output image size to IDrefout_name
 */
int CUDACOMP_extractModesLoop(const char *in_stream, const char *intot_stream, const char *IDmodes_name, const char *IDrefin_name, const char *IDrefout_name, const char *IDmodes_val_name, int GPUindex, int PROCESS, int TRACEMODE, int MODENORM, int insem, int axmode, long twait);



#endif


#endif
