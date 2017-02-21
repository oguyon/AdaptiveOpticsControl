#ifndef _AOLOOPCONTROL_H
#define _AOLOOPCONTROL_H





#define maxNBMB 100
#define MAX_NUMBER_TIMER 100

typedef struct
{

    /* =============================================================================================== */
	/*                                         TIMING                                                  */
	/* =============================================================================================== */
	
	// LOOP Timing info
	float loopfrequ; // Hz

	// Hardware latency = time from DM command issued to WFS response changed
	float hardwlatency; // hardware latency between DM command and WFS response [sec] 
	float hardwlatency_frame; // hardware latency between DM command and WFS response 

	// computation time for direct WFS->DM mode through single matrix multiplication
	float complatency;
	float complatency_frame; // computation latency (main loop) from WFS image reception to DM command output

	// computation time for full computation including open loop computation
	float wfsmextrlatency;
	float wfsmextrlatency_frame; // WFS mode extraction latency

    int_fast8_t status; // loop status for main loop
    int_fast8_t statusM; // loop status for modal loop
    int_fast8_t statusM1; // loop status for modal loop
  
    int_fast8_t GPUstatus[50];
    uint_fast16_t NBtimer; // number of active timers - 1 timer per status value
    struct timespec timer[MAX_NUMBER_TIMER];
    




    /* =============================================================================================== */
	/*                              SETUP & INITIALIZATION STATE                                       */
	/* =============================================================================================== */

    int_fast8_t init; // has been initialized
    uint_fast64_t cnt;
    uint_fast64_t cntmax;
    uint_fast64_t DMupdatecnt;
    int_fast8_t kill; // set to 1 to kill computation loop
    char name[80];

    int_fast8_t init_RM;         // Response Matrix loaded
    int_fast8_t init_CM;         // Control Matrix loaded
    int_fast8_t init_CMc;        // combine control matrix computed
    int_fast8_t initmapping;
    char respMname[80];
    char contrMname[80];




    /* =============================================================================================== */
	/*                                       WFS CAMERA                                                */
	/* =============================================================================================== */

    char WFSname[80];
    float DarkLevel;
    uint_fast32_t sizexWFS;
    uint_fast32_t sizeyWFS;
    uint_fast32_t sizeWFS;
    uint_fast32_t activeWFScnt; // number of active WFS pixels
    uint_fast32_t sizeWFS_active[100]; // only takes into account WFS pixels in use/active for each slice
    uint_fast64_t WFScnt;
    uint_fast64_t WFScntRM;

    int_fast8_t WFSnormalize; // 1 if each WFS frame should be normalized to 1
    float WFSnormfloor;
    float WFStotalflux; // after dark subtraction




    /* =============================================================================================== */
	/*                                    DEFORMABLE MIRROR                                            */
	/* =============================================================================================== */

    char dmCname[80];
    char dmdispname[80];
    char dmRMname[80];
    uint_fast32_t sizexDM;
    uint_fast32_t sizeyDM;
    uint_fast32_t sizeDM;
    uint_fast32_t activeDMcnt; // number of active actuators
    uint_fast32_t sizeDM_active; // only takes into account DM actuators that are active/in use




    /* =============================================================================================== */
	/*                                       CONTROL MODES                                             */
	/* =============================================================================================== */

    char DMmodesname[80];
    uint_fast16_t DMmodesNBblock; // number of mode blocks
    uint_fast16_t NBmodes_block[100]; // number of modes within each block

    // BLOCKS OF MODES
    uint_fast16_t NBMblocks; // number of mode blocks
    uint_fast16_t indexmaxMB[maxNBMB]; 

	uint_fast16_t NBDMmodes;



    int_fast8_t init_wfsref0;    // WFS reference image loaded



    float maxlimit; // maximum absolute value for mode values
    float mult; // multiplication coefficient to be applied at each loop iteration

	

    /* =============================================================================================== */
	/*                                        LOOP CONTROL                                             */
	/* =============================================================================================== */
    int_fast8_t on;  // goes to 1 when loop starts, put to 0 to turn loop off
    float gain; // overall loop gain
    uint_fast16_t framesAve; // number of frames to average
	int_fast8_t DMprimaryWrite_ON; // primary DM write
	
 
	// MODAL AUTOTUNING 
	// limits
	int_fast8_t AUTOTUNE_LIMITS_ON;
	float AUTOTUNE_LIMITS_perc; // percentile limit for autotuning
	float AUTOTUNE_LIMITS_delta; // autotune loop increment 

	int_fast8_t AUTOTUNE_GAINS_ON;
	float AUTOTUNEGAINcoeff; // averaging coefficient (usually about 0.001 for second-level time response)
	float AUTOTUNEGAIN_evolTimescale; // evolution timescale, beyond which errors stop growing
	
 
 
    /* =============================================================================================== */
	/*                                      PREDICTICE CONTROL                                         */
	/* =============================================================================================== */

    int_fast8_t ARPFon; // 1 if auto-regressive predictive filter is ON
	float ARPFgain; 
 



    /* =============================================================================================== */
	/*                                     COMPUTATION MODE                                            */
	/* =============================================================================================== */
    int_fast8_t GPU; // 1 if matrix multiplication  done by GPU
    int_fast8_t GPUall; // 1 if scaling computations done by GPU
    int_fast8_t GPUusesem; // 1 if using semaphores to control GPU
    int_fast8_t AOLCOMPUTE_TOTAL_ASYNC; // 1 if performing image total in separate thread (runs faster, but image total dates from last frame)
    

    

    /* =============================================================================================== */
	/*                             LOOP TELEMETRY AND PERFORMANCE                                      */
	/* =============================================================================================== */

    // COMPUTED BY OPEN LOOP RETRIEVAL PROCESS
    double RMSmodes;
    double RMSmodesCumul;
    uint_fast64_t RMSmodesCumulcnt;

	// block statistics (instantaneous)
	double block_OLrms[100]; // open loop RMS
	double block_Crms[100]; // correction RMS
	double block_WFSrms[100]; // WFS residual RMS
	double block_limFrac[100]; // fraction of mode coefficients exceeding limit
	
	double ALL_OLrms; // open loop RMS
	double ALL_Crms; // correction RMS
	double ALL_WFSrms; // WFS residual RMS
	double ALL_limFrac; // fraction of mode coefficients exceeding limit
	
	// averaged
	uint_fast32_t AveStats_NBpt; // averaging interval
	double blockave_OLrms[100]; // open loop RMS
	double blockave_Crms[100]; // correction RMS
	double blockave_WFSrms[100]; // WFS residual RMS
	double blockave_limFrac[100]; // fraction of mode coefficients exceeding limit

	double ALLave_OLrms; // open loop RMS
	double ALLave_Crms; // correction RMS
	double ALLave_WFSrms; // WFS residual RMS
	double ALLave_limFrac; // fraction of mode coefficients exceeding limit





    // semaphores for communication with GPU computing threads
    //sem_t *semptr; // semaphore for this image

} AOLOOPCONTROL_CONF;






// data passed to each thread
typedef struct
{
    long nelem;
    float *arrayptr;
    float *result; // where to white status
} THDATA_IMTOTAL;





// image streams and semaphores












int_fast8_t init_AOloopControl();


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 1. INITIALIZATION                                                                               */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

static int_fast8_t AOloopControl_loadconfigure(long loop, int mode, int level);

// initialize memory - function called within C code only (no CLI call)
static int_fast8_t AOloopControl_InitializeMemory();



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 2. LOW LEVEL UTILITIES & TOOLS                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

/* =============================================================================================== */
/* 		2.1. LOAD DATA STREAMS                                                                     */
/* =============================================================================================== */

long AOloopControl_2Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize);

long AOloopControl_3Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize, long zsize);

/* =============================================================================================== */
/* 		2.2. DATA STREAMS PROCESSING                                                               */
/* =============================================================================================== */

int_fast8_t AOloopControl_AveStream(const char *IDname, double alpha, const char *IDname_out_ave, const char *IDname_out_AC, const char *IDname_out_RMS);

long AOloopControl_frameDelay(const char *IDin_name, const char *IDkern_name, const char *IDout_name, int insem);

/* =============================================================================================== */
/* 		2.3. MISC COMPUTATION ROUTINES                                                             */
/* =============================================================================================== */

// compute cross product between two 3D arrays
static long AOloopControl_CrossProduct(const char *ID1_name, const char *ID2_name, const char *IDout_name);

// compute sum of image pixels
static void *compute_function_imtotal( void *ptr );

static void *compute_function_dark_subtract( void *ptr );






/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 3. WFS INPUT                                                                                    */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_camimage_extract2D_sharedmem_loop(const char *in_name, const char *out_name, long size_x, long size_y, long xstart, long ystart);

int_fast8_t Read_cam_frame(long loop, int RM, int normalize, int PixelStreamMode, int InitSem);





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 4. ACQUIRING CALIBRATION                                                                        */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

long AOloopControl_Measure_WFSrespC(long loop, long delayfr, long delayRM1us, long NBave, long NBexcl, const char *IDpokeC_name, const char *IDoutC_name, int normalize, int AOinitMode, long NBcycle);

long AOloopControl_Measure_WFS_linResponse(long loop, float ampl, long delayfr, long delayRM1us, long NBave, long NBexcl, const char *IDpokeC_name, const char *IDrespC_name, const char *IDwfsref_name, int normalize, int AOinitMode, long NBcycle);

long AOloopControl_Measure_zonalRM(long loop, double ampl, long delayfr, long delayRM1us, long NBave, long NBexcl, const char *zrespm_name, const char *WFSref_name, const char *WFSmap_name, const char *DMmap_name, long mode, int normalize, int AOinitMode, long NBcycle);

int_fast8_t Measure_Resp_Matrix(long loop, long NbAve, float amp, long nbloop, long fDelay, long NBiter);

long AOloopControl_RespMatrix_Fast(const char *DMmodes_name, const char *dmRM_name, const char *imWFS_name, long semtrig, float HardwareLag, float loopfrequ, float ampl, const char *outname);



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 5. COMPUTING CALIBRATION                                                                        */
/*                                                                                                 */
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
/*                                                                                                 */
/* 6. REAL TIME COMPUTING ROUTINES                                                                 */
/*                                                                                                 */
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
/*                                                                                                 */
/* 7. PREDICTIVE CONTROL                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_mapPredictiveFilter(const char *IDmodecoeff_name, long modeout, double delayfr);

double AOloopControl_testPredictiveFilter(const char *IDtrace_name, long mode, double delayfr, long filtsize, const char *IDfilt_name, double SVDeps);

long AOloopControl_builPFloop_WatchInput(long loop, long PFblock);






/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 8. LOOP CONTROL INTERFACE                                                                       */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_setLoopNumber(long loop);

int_fast8_t AOloopControl_setparam(long loop, const char *key, double value);

/* =============================================================================================== */
/* 		8.1. MAIN CONTROL : LOOP ON/OFF START/STOP/STEP/RESET                                      */
/* =============================================================================================== */

int_fast8_t AOloopControl_loopon();

int_fast8_t AOloopControl_loopoff();

int_fast8_t AOloopControl_loopkill();

int_fast8_t AOloopControl_loopstep(long loop, long NBstep);

int_fast8_t AOloopControl_loopreset();

/* =============================================================================================== */
/* 		8.2. DATA LOGGING                                                                          */
/* =============================================================================================== */

int_fast8_t AOloopControl_logon();

int_fast8_t AOloopControl_logoff();

/* =============================================================================================== */
/* 		8.3. PRIMARY DM WRITE                                                                      */
/* =============================================================================================== */

int_fast8_t AOloopControl_DMprimaryWrite_on();

int_fast8_t AOloopControl_DMprimaryWrite_off();

/* =============================================================================================== */
/* 		8.4. INTEGRATOR AUTO TUNING                                                                */
/* =============================================================================================== */

int_fast8_t AOloopControl_AUTOTUNE_LIMITS_on();

int_fast8_t AOloopControl_AUTOTUNE_LIMITS_off();

int_fast8_t AOloopControl_set_AUTOTUNE_LIMITS_delta(float AUTOTUNE_LIMITS_delta);

int_fast8_t AOloopControl_set_AUTOTUNE_LIMITS_perc(float AUTOTUNE_LIMITS_perc);

int_fast8_t AOloopControl_AUTOTUNE_GAINS_on();

int_fast8_t AOloopControl_AUTOTUNE_GAINS_off();

/* =============================================================================================== */
/* 		8.5. PREDICTIVE FILTER ON/OFF                                                              */
/* =============================================================================================== */

int_fast8_t AOloopControl_ARPFon();

int_fast8_t AOloopControl_ARPFoff();

/* =============================================================================================== */
/* 		8.6. TIMING PARAMETERS                                                                     */
/* =============================================================================================== */

int_fast8_t AOloopControl_set_loopfrequ(float loopfrequ);

int_fast8_t AOloopControl_set_hardwlatency_frame(float hardwlatency_frame);

int_fast8_t AOloopControl_set_complatency_frame(float complatency_frame);

int_fast8_t AOloopControl_set_wfsmextrlatency_frame(float wfsmextrlatency_frame);

/* =============================================================================================== */
/* 		8.7. CONTROL LOOP PARAMETERS                                                               */
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
/*                                                                                                 */
/* 9. STATUS / TESTING / PERF MEASUREMENT                                                          */
/*                                                                                                 */
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
/*                                                                                                 */
/* 10. FOCAL PLANE SPECKLE MODULATION / CONTROL                                                    */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_OptimizePSF_LO(const char *psfstream_name, const char *IDmodes_name, const char *dmstream_name, long delayframe, long NBframes);

int_fast8_t AOloopControl_DMmodulateAB(const char *IDprobeA_name, const char *IDprobeB_name, const char *IDdmstream_name, const char *IDrespmat_name, const char *IDwfsrefstream_name, double delay, long NBprobes);



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 11. PROCESS LOG FILES                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

int_fast8_t AOloopControl_logprocess_modeval(const char *IDname);


















/* =============================================================================================== */
/*                                                                                                 */
/*                                              OBSOLETE ?                                         */
/*                                                                                                 */
/* =============================================================================================== */

// "old" blocks (somewhat obsolete)
int_fast8_t AOloopControl_setgainrange(long m0, long m1, float gainval);
int_fast8_t AOloopControl_setlimitrange(long m0, long m1, float limval);
int_fast8_t AOloopControl_setmultfrange(long m0, long m1, float multfval);
int_fast8_t AOloopControl_setgainblock(long mb, float gainval);
int_fast8_t AOloopControl_setlimitblock(long mb, float limitval);
int_fast8_t AOloopControl_setmultfblock(long mb, float multfval);

int_fast8_t AOloopControl_AutoTune();



/*
int AOloopControl_Measure_WFScam_PeriodicError(long loop, long NBframes, long NBpha, char *IDout_name); // OBSOLETE
int AOloopControl_Remove_WFScamPE(char *IDin_name, char *IDcorr_name, double pha); // OBSOLETE
*/


#endif
