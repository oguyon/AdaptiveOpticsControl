/**
 * @file    AOloopControl.h
 * @brief   Function prototypes for Adaptive Optics Control loop engine
 * 
 * AO engine uses stream data structure
 * 
 * @author  O. Guyon
 * @date    17 Jun 2017
 *
 * @bug No known bugs. 
 * 
 */

#ifndef _AOLOOPCONTROL_H
#define _AOLOOPCONTROL_H




/** @brief Initialize AOloopControl command line interface. */
int_fast8_t init_AOloopControl();



/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 1. INITIALIZATION, configurations                                        
 * 
 * Allocate memory, import/export configurations
 * 
 */
/* =============================================================================================== */
/* =============================================================================================== */



/** @brief Load configuation parameters from disk */
static int_fast8_t AOloopControl_loadconfigure(long loop, int mode, int level);

/** @brief Initialize memory - function called within C code only (no CLI call) */
static int_fast8_t AOloopControl_InitializeMemory();







/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 2. LOW LEVEL UTILITIES & TOOLS    
 *  Useful tools */
/* =============================================================================================== */
/* =============================================================================================== */


/* =============================================================================================== */
/** @name AOloopControl - 2.1. LOW LEVEL UTILITIES & TOOLS - LOAD DATA STREAMS                     */
/* =============================================================================================== */

long AOloopControl_2Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize);

long AOloopControl_3Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize, long zsize);


/* =============================================================================================== */
/** @name AOloopControl - 2.2. LOW LEVEL UTILITIES & TOOLS - DATA STREAMS PROCESSING               */
/* =============================================================================================== */

int_fast8_t AOloopControl_AveStream(const char *IDname, double alpha, const char *IDname_out_ave, const char *IDname_out_AC, const char *IDname_out_RMS);

long AOloopControl_frameDelay(const char *IDin_name, const char *IDkern_name, const char *IDout_name, int insem);

/** @brief Re-arrange a 3D cube into an array of images into a single 2D frame */
long AOloopControl_stream3Dto2D(const char *in_name, const char *out_name, int NBcols, int insem);


/* =============================================================================================== */
/** @name AOloopControl - 2.3. LOW LEVEL UTILITIES & TOOLS - MISC COMPUTATION ROUTINES             */
/* =============================================================================================== */

// compute cross product between two 3D arrays
static long AOloopControl_CrossProduct(const char *ID1_name, const char *ID2_name, const char *IDout_name);

// compute sum of image pixels
static void *compute_function_imtotal( void *ptr );

static void *compute_function_dark_subtract( void *ptr );

long AOloopControl_mkSimpleZpokeM( long dmxsize, long dmysize, char *IDout_name);

long AOloopControl_dm2opdmaploop(char *DMdisp_name, char *OPDmap_name, int semindex);




/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 3. WFS INPUT
 *  Read camera imates */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_camimage_extract2D_sharedmem_loop(const char *in_name, const char *dark_name, const char *out_name, long size_x, long size_y, long xstart, long ystart);

int_fast8_t Read_cam_frame(long loop, int RM, int normalize, int PixelStreamMode, int InitSem);




/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 4. ACQUIRING CALIBRATION
 *  Measure system response */
/* =============================================================================================== */
/* =============================================================================================== */

/**
 * @brief Acquire WFS response to a series of DM pattern.
 *
 * 
 * @param[in]  loop            Loop index
 * @param[in]  delayfr         Integer delay [frame]
 * @param[in]  delayRM1us      Fractional delay [us]
 * @param[in]  NBave           Number of frames averaged per DM state
 * @param[in]  NBexcl          Number of frames excluded
 * @param[in]  IDpokeC_name    Poke pattern
 * @param[out] IDoutC_name     Output cube
 * @param[in]  normalize       Normalize flag
 * @param[in]  AOinitMode      AO structure initialization flag
 * @param[in\  NBcycle         Number of cycles averaged
 * 
 * AOinitMode = 0:  create AO shared mem struct
 * AOinitMode = 1:  connect only to AO shared mem struct
 * 
 * INPUT : DMpoke_name : set of DM patterns
 * OUTPUT : WFSmap_name : WFS response maps
 * 
 * USR1 signal will stop acquisition immediately
 * USR2 signal completes current cycles and stops acquisition
 * 
 * @return IDoutC
 * 
 */
long AOloopControl_Measure_WFSrespC(long loop, long delayfr, long delayRM1us, long NBave, long NBexcl, const char *IDpokeC_name, const char *IDoutC_name, int normalize, int AOinitMode, long NBcycle);


long AOloopControl_Measure_WFS_linResponse(long loop, float ampl, long delayfr, long delayRM1us, long NBave, long NBexcl, const char *IDpokeC_name, const char *IDrespC_name, const char *IDwfsref_name, int normalize, int AOinitMode, long NBcycle);


long AOloopControl_Measure_zonalRM(long loop, double ampl, long delayfr, long delayRM1us, long NBave, long NBexcl, const char *zrespm_name, const char *WFSref_name, const char *WFSmap_name, const char *DMmap_name, long mode, int normalize, int AOinitMode, long NBcycle);


int_fast8_t Measure_Resp_Matrix(long loop, long NbAve, float amp, long nbloop, long fDelay, long NBiter);


long AOloopControl_RespMatrix_Fast(const char *DMmodes_name, const char *dmRM_name, const char *imWFS_name, long semtrig, float HardwareLag, float loopfrequ, float ampl, const char *outname);






/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 5. COMPUTING CALIBRATION
 *  Compute control matrix, modes */
/* =============================================================================================== */
/* =============================================================================================== */

long AOloopControl_mkHadamardModes(const char *DMmask_name, const char *outname);

long AOloopControl_Hadamard_decodeRM(const char *inname, const char *Hmatname, const char *indexname, const char *outname);

long AOloopControl_mkloDMmodes(const char *ID_name, long msizex, long msizey, float CPAmax, float deltaCPA, double xc, double yc, double r0, double r1, int MaskMode);

int_fast8_t AOloopControl_mkCalib_map_mask(long loop, const char *zrespm_name, const char *WFSmap_name, const char *DMmap_name, float dmmask_perclow, float dmmask_coefflow, float dmmask_perchigh, float dmmask_coeffhigh, float wfsmask_perclow, float wfsmask_coefflow, float wfsmask_perchigh, float wfsmask_coeffhigh);

int_fast8_t AOloopControl_Process_zrespM(long loop, const char *IDzrespm0_name, const char *IDwfsref_name, const char *IDzrespm_name, const char *WFSmap_name, const char *DMmap_name);

int_fast8_t AOloopControl_ProcessZrespM_medianfilt(long loop, const char *zrespm_name, const char *WFSref0_name, const char *WFSmap_name, const char *DMmap_name, double rmampl, int normalize);

long AOloopControl_mkCM(const char *respm_name, const char *cm_name, float SVDlim);

long AOloopControl_mkSlavedAct(const char *IDmaskRM_name, float pixrad, const char *IDout_name);

static long AOloopControl_DMedgeDetect(const char *IDmaskRM_name, const char *IDout_name);

static long AOloopControl_DMextrapolateModes(const char *IDin_name, const char *IDmask_name, const char *IDcpa_name, const char *IDout_name);

long AOloopControl_DMslaveExt(const char *IDin_name, const char *IDmask_name, const char *IDsl_name, const char *IDout_name, float r0);

long AOloopControl_mkModes(const char *ID_name, long msizex, long msizey, float CPAmax, float deltaCPA, double xc, double yx, double r0, double r1, int MaskMode, int BlockNB, float SVDlim);

long AOloopControl_mkModes_Simple(const char *IDin_name, long NBmblock, long Cmblock, float SVDlim);

int_fast8_t compute_ControlMatrix(long loop, long NB_MODE_REMOVED, const char *ID_Rmatrix_name, const char *ID_Cmatrix_name, const char *ID_VTmatrix_name, double Beta, long NB_MODE_REMOVED_STEP, float eigenvlim);

long compute_CombinedControlMatrix(const char *IDcmat_name, const char *IDmodes_name, const char* IDwfsmask_name, const char *IDdmmask_name, const char *IDcmatc_name, const char *IDcmatc_active_name);

long AOloopControl_loadCM(long loop, const char *CMfname);



/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 6. REAL TIME COMPUTING ROUTINES
 *  calls CPU and GPU processing */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_WFSzpupdate_loop(const char *IDzpdm_name, const char *IDzrespM_name, const char *IDwfszp_name);

int_fast8_t AOloopControl_WFSzeropoint_sum_update_loop(long loopnb, const char *ID_WFSzp_name, int NBzp, const char *IDwfsref0_name, const char *IDwfsref_name);

int_fast8_t AOloopControl_run();

int_fast8_t ControlMatrixMultiply( float *cm_array, float *imarray, long m, long n, float *outvect);

int_fast8_t set_DM_modes(long loop);

int_fast8_t set_DM_modesRM(long loop);

int_fast8_t AOcompute(long loop, int normalize);

int_fast8_t AOloopControl_CompModes_loop(const char *ID_CM_name, const char *ID_WFSref_name, const char *ID_WFSim_name, const char *ID_WFSimtot_name, const char *ID_coeff_name);

int_fast8_t AOloopControl_GPUmodecoeffs2dm_filt_loop(const char *modecoeffs_name, const char *DMmodes_name, int semTrigg, const char *out_name, int GPUindex, long loop, int offloadMode);

long AOloopControl_sig2Modecoeff(const char *WFSim_name, const char *IDwfsref_name, const char *WFSmodes_name, const char *outname);

long AOloopControl_computeWFSresidualimage(long loop, float alpha);

long AOloopControl_ComputeOpenLoopModes(long loop);

int_fast8_t AOloopControl_AutoTuneGains(long loop, const char *IDout_name);

long AOloopControl_dm2dm_offload(const char *streamin, const char *streamout, float twait, float offcoeff, float multcoeff);




/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 7. PREDICTIVE CONTROL
 *  Predictive control using WFS telemetry */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_mapPredictiveFilter(const char *IDmodecoeff_name, long modeout, double delayfr);

double AOloopControl_testPredictiveFilter(const char *IDtrace_name, long mode, double delayfr, long filtsize, const char *IDfilt_name, double SVDeps);

long AOloopControl_builPFloop_WatchInput(long loop, long PFblock);





/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 8.   LOOP CONTROL INTERFACE
 *  Set parameters */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_setLoopNumber(long loop);

int_fast8_t AOloopControl_setparam(long loop, const char *key, double value);


/* =============================================================================================== */
/** @name AOloopControl - 8.1. LOOP CONTROL INTERFACE - MAIN CONTROL : LOOP ON/OFF START/STOP/STEP/RESET
 *  Set parameters */
/* =============================================================================================== */

int_fast8_t AOloopControl_loopon();

int_fast8_t AOloopControl_loopoff();

int_fast8_t AOloopControl_loopkill();

int_fast8_t AOloopControl_loopstep(long loop, long NBstep);

int_fast8_t AOloopControl_loopreset();


/* =============================================================================================== */
/** @name AOloopControl - 8.2. LOOP CONTROL INTERFACE - DATA LOGGING                               */
/* =============================================================================================== */

int_fast8_t AOloopControl_logon();

int_fast8_t AOloopControl_logoff();


/* =============================================================================================== */
/** @name AOloopControl - 8.3. LOOP CONTROL INTERFACE - PRIMARY DM WRITE                           */
/* =============================================================================================== */

int_fast8_t AOloopControl_DMprimaryWrite_on();

int_fast8_t AOloopControl_DMprimaryWrite_off();


/* =============================================================================================== */
/** @name AOloopControl - 8.4. LOOP CONTROL INTERFACE - INTEGRATOR AUTO TUNING                     */
/* =============================================================================================== */

int_fast8_t AOloopControl_AUTOTUNE_LIMITS_on();

int_fast8_t AOloopControl_AUTOTUNE_LIMITS_off();

int_fast8_t AOloopControl_set_AUTOTUNE_LIMITS_delta(float AUTOTUNE_LIMITS_delta);

int_fast8_t AOloopControl_set_AUTOTUNE_LIMITS_perc(float AUTOTUNE_LIMITS_perc);

int_fast8_t AOloopControl_set_AUTOTUNE_LIMITS_mcoeff(float AUTOTUNE_LIMITS_mcoeff);

int_fast8_t AOloopControl_AUTOTUNE_GAINS_on();

int_fast8_t AOloopControl_AUTOTUNE_GAINS_off();

/* =============================================================================================== */
/** @name AOloopControl - 8.5. LOOP CONTROL INTERFACE - PREDICTIVE FILTER ON/OFF                   */
/* =============================================================================================== */

int_fast8_t AOloopControl_ARPFon();

int_fast8_t AOloopControl_ARPFoff();

/* =============================================================================================== */
/** @name AOloopControl - 8.6. LOOP CONTROL INTERFACE - TIMING PARAMETERS                          */
/* =============================================================================================== */

int_fast8_t AOloopControl_set_loopfrequ(float loopfrequ);

int_fast8_t AOloopControl_set_hardwlatency_frame(float hardwlatency_frame);

int_fast8_t AOloopControl_set_complatency_frame(float complatency_frame);

int_fast8_t AOloopControl_set_wfsmextrlatency_frame(float wfsmextrlatency_frame);

/* =============================================================================================== */
/** @name AOloopControl - 8.7. LOOP CONTROL INTERFACE - CONTROL LOOP PARAMETERS                    */
/* =============================================================================================== */

int_fast8_t AOloopControl_setgain(float gain);

int_fast8_t AOloopControl_setARPFgain(float gain);

int_fast8_t AOloopControl_setWFSnormfloor(float WFSnormfloor);

int_fast8_t AOloopControl_setmaxlimit(float maxlimit);

int_fast8_t AOloopControl_setmult(float multcoeff);

int_fast8_t AOloopControl_setframesAve(long nbframes);

int_fast8_t AOloopControl_set_modeblock_gain(long loop, long blocknb, float gain, int add);// modal blocks

int_fast8_t AOloopControl_scanGainBlock(long NBblock, long NBstep, float gainStart, float gainEnd, long NBgain);




/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 9. STATUS / TESTING / PERF MEASUREMENT
 *  Measure loop behavior */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_printloopstatus(long loop, long nbcol, long IDmodeval_dm, long IDmodeval, long IDmodevalave, long IDmodevalrms, long ksize);

int_fast8_t AOloopControl_loopMonitor(long loop, double frequ, long nbcol);

int_fast8_t AOloopControl_statusStats(int updateconf);

int_fast8_t AOloopControl_resetRMSperf();

int_fast8_t AOloopControl_showparams(long loop);

int_fast8_t AOcontrolLoop_TestDMSpeed(const char *dmname, long delayus, long NBpts, float ampl);

int_fast8_t AOcontrolLoop_TestSystemLatency(const char *dmname, char *wfsname, float OPDamp, long NBiter);

long AOloopControl_blockstats(long loop, const char *IDout_name);

int_fast8_t AOloopControl_InjectMode( long index, float ampl );

long AOloopControl_TestDMmodeResp(const char *DMmodes_name, long index, float ampl, float fmin, float fmax, float fmultstep, float avetime, long dtus, const char *DMmask_name, const char *DMstream_in_name, const char *DMstream_out_name, const char *IDout_name);

long AOloopControl_TestDMmodes_Recovery(const char *DMmodes_name, float ampl, const char *DMmask_name, const char *DMstream_in_name, const char *DMstream_out_name, const char *DMstream_meas_name, long tlagus, long NBave, const char *IDout_name, const char *IDoutrms_name, const char *IDoutmeas_name, const char *IDoutmeasrms_name);

long AOloopControl_mkTestDynamicModeSeq(const char *IDname_out, long NBpt, long NBmodes);

int_fast8_t AOloopControl_AnalyzeRM_sensitivity(const char *IDdmmodes_name, const char *IDdmmask_name, const char *IDwfsref_name, const char *IDwfsresp_name, const char *IDwfsmask_name, float amplimitnm, float lambdanm, const char *foutname);





/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 10. FOCAL PLANE SPECKLE MODULATION / CONTROL
 *  custom FP AO routines */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_OptimizePSF_LO(const char *psfstream_name, const char *IDmodes_name, const char *dmstream_name, long delayframe, long NBframes);

int_fast8_t AOloopControl_DMmodulateAB(const char *IDprobeA_name, const char *IDprobeB_name, const char *IDdmstream_name, const char *IDrespmat_name, const char *IDwfsrefstream_name, double delay, long NBprobes);




/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 11. PROCESS LOG FILES
 *  process log files */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_logprocess_modeval(const char *IDname);














/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 12. OBSOLETE ?                                                           */ 
/* =============================================================================================== */
/* =============================================================================================== */

// "old" blocks (somewhat obsolete)
int_fast8_t AOloopControl_setgainrange(long m0, long m1, float gainval);
int_fast8_t AOloopControl_setlimitrange(long m0, long m1, float limval);
int_fast8_t AOloopControl_setmultfrange(long m0, long m1, float multfval);
int_fast8_t AOloopControl_setgainblock(long mb, float gainval);
int_fast8_t AOloopControl_setlimitblock(long mb, float limitval);
int_fast8_t AOloopControl_setmultfblock(long mb, float multfval);

int_fast8_t AOloopControl_AutoTune();



#endif
