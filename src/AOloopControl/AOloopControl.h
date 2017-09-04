/**
 * @file    AOloopControl.h
 * @brief   Function prototypes for Adaptive Optics Control loop engine
 * 
 * AO engine uses stream data structure
 * 
 * @author  O. Guyon
 * @date    26 Aug 2017
 *
 * @bug No known bugs. 
 * 
 */

#ifndef _AOLOOPCONTROL_H
#define _AOLOOPCONTROL_H






#define maxNBMB 100			// maximum number of mode blocks
#define MAXNBMODES 10000	// maximum number of control modes
#define MAX_NUMBER_TIMER 100


// logging
static int loadcreateshm_log = 0; // 1 if results should be logged in ASCII file
static FILE *loadcreateshm_fplog;




/**
 * Main AOloopControl structure. 
 *
 * Holds key parameters for the AO engine as well as higher level variables describing the state of the control loop
 * 
 * @ingroup AOloopControl_AOLOOPCONTROL_CONF
 */
typedef struct
{	
    /* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: TIMING 
	 * LOOP Timing info
	 */


	float loopfrequ;                          /**< Loop frequency [Hz] */

	// Hardware latency = time from DM command issued to WFS response changed 
	float hardwlatency;                       /**< hardware latency between DM command and WFS response [sec] */ 
	float hardwlatency_frame;                 /**< hardware latency between DM command and WFS response [frame] */

	// Computation time for direct WFS->DM mode through single matrix multiplication
	float complatency;                        /**< Computation latency [sec] */
	float complatency_frame;                  /**< Computation latency (main loop) from WFS image reception to DM command output [frame] */

	// Computation time for full computation including open loop computation
	float wfsmextrlatency;                    /**< WFS mode extraction latency [sec] */ 
	float wfsmextrlatency_frame;              /**< WFS mode extraction latency [frame] */

    int_fast8_t status;                       /**< loop status for main loop */
    int_fast8_t statusM;                      /**< loop status for modal loop */
    int_fast8_t statusM1;                     /**< loop status for modal loop */
  
    int_fast8_t GPUstatus[50];                /**<  GPU status index */
    uint_fast16_t NBtimer;                    /**<  Number of active timers - 1 timer per status value */
    struct timespec timer[MAX_NUMBER_TIMER];  /**<  Timers */
   
    /* =============================================================================================== */



    /* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: SETUP & INITIALIZATION STATE 
	 * 
	 */

    int_fast8_t init;                         /**< Has the structure been initialized ? */
    uint_fast64_t cnt;                        /**<  */
    uint_fast64_t cntmax;                     /**<  */
    uint_fast64_t DMupdatecnt;                /**<  */
    int_fast8_t kill;                         /**<  set to 1 to kill computation loop */
    char name[80];

    int_fast8_t init_RM;                      /**< Response Matrix loaded */
    int_fast8_t init_CM;                      /**< Control Matrix loaded */
    int_fast8_t init_CMc;                     /**< Combined control matrix computed */
    int_fast8_t initmapping;
    char respMname[80];
    char contrMname[80];
  
    /* =============================================================================================== */



    /* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: WFS CAMERA
	 * 
	 */
	
    /** @brief WFS stream name
     * 
     * Usually set to aol_wfsim by function AOloopControl_loadconfigure
     */
    char WFSname[80];                        
    
    uint_fast32_t sizexWFS;                   /**< WFS image x size*/
    uint_fast32_t sizeyWFS;                   /**< WFS image y size */
    uint_fast32_t sizeWFS;                    /**< WFS total image (= x size * y size) */
    uint_fast32_t activeWFScnt;               /**< Number of active WFS pixels */
    uint_fast32_t sizeWFS_active[100];        /**< Only takes into account WFS pixels in use/active for each slice */
    uint_fast64_t WFScnt;                     /**< WFS stream counter 0 value at WFS image read */
    uint_fast64_t WFScntRM;                   /**< WFS stream counter 0 value at WFS image read (RM acqu mode) */

    int_fast8_t WFSnormalize;                 /**< 1 if each WFS frame should be normalized to 1 */
    float WFSnormfloor;                       /**< normalized by dividing by (total + AOconf[loop].WFSnormfloor)*AOconf[loop].WFSsize */
    float WFStotalflux;                       /**< Total WFS flux after dark subtraction */
    /* =============================================================================================== */



    /* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: DEFORMABLE MIRROR
	 * 
	 */
	 
    char dmCname[80];
    char dmdispname[80];
    char dmRMname[80];
    uint_fast8_t DMMODE;                      /**< 0: zonal DM, 1: modal DM */
    uint_fast32_t sizexDM;                    /**< DM x size*/
    uint_fast32_t sizeyDM;                    /**< DM y size*/
    uint_fast32_t sizeDM;                     /**< DM total image (= x size * y size) */
    uint_fast32_t activeDMcnt;                /**< number of active actuators */
    uint_fast32_t sizeDM_active;              /**< only takes into account DM actuators that are active/in use */
    
    /* =============================================================================================== */



	/* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: CONTROL MODES 
	 * 
	 */

    char DMmodesname[80];
     // BLOCKS OF MODES
    uint_fast16_t DMmodesNBblock;             /**< number of mode blocks (read from parameter) */
    uint_fast16_t NBmodes_block[100];         /**< number of modes within each block (computed from files by AOloopControl_loadconfigure) */
    uint_fast16_t modeBlockIndex[MAXNBMODES]; /**< block index to which each mode belongs (computed by AOloopControl_loadconfigure) */
    uint_fast16_t indexmaxMB[maxNBMB]; 

	uint_fast16_t NBDMmodes;

    int_fast8_t init_wfsref0;    // WFS reference image loaded

    float maxlimit; // maximum absolute value for mode values
    float mult; // multiplication coefficient to be applied at each loop iteration
 
	/* =============================================================================================== */




	/* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: LOOP CONTROL
	 * 
	 */
		
    int_fast8_t on;                           /**< goes to 1 when loop starts, put to 0 to turn loop off */
    float gain;                               /**< overall loop gain */
    uint_fast16_t framesAve;                  /**< number of WFS frames to average */
	int_fast8_t DMprimaryWriteON;             /**< primary DM write */
	int_fast8_t CMMODE;                       /**< Combined matrix. 0: matrix is WFS pixels -> modes, 1: matrix is WFS pixels -> DM actuators */
	int_fast8_t DMfilteredWriteON;            /**< Filtered write to DM */
 
	// MODAL AUTOTUNING 
	// limits
	int_fast8_t AUTOTUNE_LIMITS_ON;
	float AUTOTUNE_LIMITS_perc; // percentile limit for autotuning
	float AUTOTUNE_LIMITS_mcoeff; // multiplicative coeff 
	float AUTOTUNE_LIMITS_delta; // autotune loop increment 

	int_fast8_t AUTOTUNE_GAINS_ON;
	float AUTOTUNEGAINS_updateGainCoeff;      /**< Averaging coefficient (usually about 0.1) */
	float AUTOTUNEGAINS_evolTimescale;        /**< Evolution timescale, beyond which errors stop growing */
	long AUTOTUNEGAINS_NBsamples;            /**< Number of samples */
   
	/* =============================================================================================== */

 
 
	/* =============================================================================================== */
	/** @name AOLOOPCONTROL_CONF: PREDICTIVE CONTROL
	 * 
	 */
	///@{  	
    int_fast8_t ARPFon; // 1 if auto-regressive predictive filter is ON
	float ARPFgain; 
    ///@}
	/* =============================================================================================== */



	/* =============================================================================================== */
 	/** @name AOLOOPCONTROL_CONF: COMPUTATION MODES
	 * 
	 */

    int_fast8_t GPU0; // NB of GPU devices in set 0. 1+ if matrix multiplication done by GPU (set 0)
    int_fast8_t GPU1; // NB of GPU devices in set 1. 1+ if matrix multiplication done by GPU (set 1)
    int_fast8_t GPU2; // NB of GPU devices in set 2. 1+ if matrix multiplication done by GPU (set 2)
    int_fast8_t GPU3; // NB of GPU devices in set 3. 1+ if matrix multiplication done by GPU (set 3)    
    int_fast8_t GPU4; // NB of GPU devices in set 4. 1+ if matrix multiplication done by GPU (set 4)
    int_fast8_t GPU5; // NB of GPU devices in set 5. 1+ if matrix multiplication done by GPU (set 5)
    int_fast8_t GPU6; // NB of GPU devices in set 6. 1+ if matrix multiplication done by GPU (set 6)
    int_fast8_t GPU7; // NB of GPU devices in set 7. 1+ if matrix multiplication done by GPU (set 7)
            
    int_fast8_t GPUall; // 1 if scaling computations done by GPU
    int_fast8_t GPUusesem; // 1 if using semaphores to control GPU
    int_fast8_t AOLCOMPUTE_TOTAL_ASYNC; // 1 if performing image total in separate thread (runs faster, but image total dates from last frame)
  
	/* =============================================================================================== */
    

    


	/* =============================================================================================== */
 	/** @name AOLOOPCONTROL_CONF: BASIC LOOP TELEMETRY AND PERFORMANCE
	 * 
	 */

    // COMPUTED BY OPEN LOOP RETRIEVAL PROCESS
    double RMSmodes;
    double RMSmodesCumul;
    uint_fast64_t RMSmodesCumulcnt;

	// block statistics (instantaneous)
	double block_PFresrms[100]; // Prediction residual, meas RMS
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
	double blockave_PFresrms[100]; // open loop RMS
	double blockave_OLrms[100]; // open loop RMS
	double blockave_Crms[100]; // correction RMS
	double blockave_WFSrms[100]; // WFS residual RMS
	double blockave_limFrac[100]; // fraction of mode coefficients exceeding limit

	double ALLave_OLrms; // open loop RMS
	double ALLave_Crms; // correction RMS
	double ALLave_WFSrms; // WFS residual RMS
	double ALLave_limFrac; // fraction of mode coefficients exceeding limit

	/* =============================================================================================== */
    
   

    // semaphores for communication with GPU computing threads
    //sem_t *semptr; // semaphore for this image

} AOLOOPCONTROL_CONF;

























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
int_fast8_t AOloopControl_loadconfigure(long loop, int mode, int level);


/** @brief Initialize memory - function called within C code only (no CLI call) */
int_fast8_t AOloopControl_InitializeMemory();












/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 6. REAL TIME COMPUTING ROUTINES
 *  calls CPU and GPU processing */
/* =============================================================================================== */
/* =============================================================================================== */


int_fast8_t AOloopControl_WFSzpupdate_loop(const char *IDzpdm_name, const char *IDzrespM_name, const char *IDwfszp_name);


int_fast8_t AOloopControl_WFSzeropoint_sum_update_loop(long loopnb, const char *ID_WFSzp_name, int NBzp, const char *IDwfsref0_name, const char *IDwfsref_name);

/** @brief Main loop function */
int_fast8_t AOloopControl_run();


int_fast8_t ControlMatrixMultiply( float *cm_array, float *imarray, long m, long n, float *outvect);

/** @brief Sends modal commands to DM by matrix-vector multiplication */
int_fast8_t set_DM_modes(long loop);

int_fast8_t set_DM_modesRM(long loop);

/** @brief Main computation function, runs once per loop iteration */
int_fast8_t AOcompute(long loop, int normalize);

int_fast8_t AOloopControl_CompModes_loop(const char *ID_CM_name, const char *ID_WFSref_name, const char *ID_WFSim_name, const char *ID_WFSimtot_name, const char *ID_coeff_name);

int_fast8_t AOloopControl_GPUmodecoeffs2dm_filt_loop(const char *modecoeffs_name, const char *DMmodes_name, int semTrigg, const char *out_name, int GPUindex, long loop, int offloadMode);

long AOloopControl_sig2Modecoeff(const char *WFSim_name, const char *IDwfsref_name, const char *WFSmodes_name, const char *outname);

long AOloopControl_computeWFSresidualimage(long loop, char *IDalpha_name);

long AOloopControl_ComputeOpenLoopModes(long loop);

int_fast8_t AOloopControl_AutoTuneGains(long loop, const char *IDout_name, float GainCoeff, long NBsamples);

long AOloopControl_dm2dm_offload(const char *streamin, const char *streamout, float twait, float offcoeff, float multcoeff);






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
/** @name AOloopControl - 8.3. LOOP CONTROL INTERFACE - PRIMARY AND FILTERED DM WRITE              */
/* =============================================================================================== */

int_fast8_t AOloopControl_DMprimaryWrite_on();

int_fast8_t AOloopControl_DMprimaryWrite_off();

int_fast8_t AOloopControl_DMfilteredWrite_on();

int_fast8_t AOloopControl_DMfilteredWrite_off();


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
