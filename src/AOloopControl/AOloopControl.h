#ifndef _AOLOOPCONTROL_H
#define _AOLOOPCONTROL_H





#define maxNBMB 100
#define MAX_NUMBER_TIMER 100

typedef struct
{
    struct timespec tnow;  // computed at time of sending DM commands
    double time_sec; // converted in second

    // SETUP
    int init; // has been initialized
    unsigned long long cnt;
    unsigned long long cntmax;
    unsigned long long DMupdatecnt;
    int kill; // set to 1 to kill computation loop

    char name[80];

    // Wavefront sensor camera
    char WFSname[80];
    float DarkLevel;
    long sizexWFS;
    long sizeyWFS;
    long sizeWFS;
    long activeWFScnt; // number of active WFS pixels
    long sizeWFS_active[100]; // only takes into account WFS pixels in use/active for each slice
    long long WFScnt;
    long long WFScntRM;
    int WFSnormalize; // 1 if each WFS frame should be normalized to 1
    float WFSnormfloor;
    float WFStotalflux; // after dark subtraction

    // DM
    char dmCname[80];
    char dmdispname[80];
    char dmRMname[80];
    long sizexDM;
    long sizeyDM;
    long sizeDM;
    long activeDMcnt; // number of active actuators
    long sizeDM_active; // only takes into account DM actuators that are active/in use

    // Modes
    char DMmodesname[80];
    long DMmodesNBblock; // number of mode blocks
    long NBmodes_block[100]; // number of modes within each block

    int init_wfsref0;    // WFS reference image loaded

    int init_RM;        // Response Matrix loaded
    int init_CM;        // Control Matrix loaded
    int init_CMc;       // combine control matrix computed
    int initmapping;
    char respMname[80];
    char contrMname[80];


    long NBDMmodes;
    float maxlimit; // maximum absolute value for mode values
    float mult; // multiplication coefficient to be applied at each loop iteration

	// Timing info
	float loopfrequ; // Hz
	float hardlatency_frame; // hardware latency between DM command and WFS response 
	float complatency_frame; // computation latency (main loop)
	float wfsmextrlatency_frame; // WFS mode extraction latency

    // LOOP CONTROL
    int on;  // goes to 1 when loop starts, put to 0 to turn loop off
    float gain; // overall loop gain
    long framesAve; // number of frames to average
 
	// PREDICTICE CONTROL
    int ARPFon; // 1 if auto-regressive predictive filter is ON
	float ARPFgain; 
 

    int status;
    int GPUstatus[50];
    unsigned int NBtimer; // number of active timers - 1 timer per status value
    struct timespec timer[MAX_NUMBER_TIMER];
    
    int RMstatus;
    // 2: wait for image

    // LOOP TUNING
    // BLOCKS OF MODES
    long NBMblocks; // number of mode blocks
    long indexmaxMB[maxNBMB];
    float gainMB[maxNBMB];
    float limitMB[maxNBMB];
    float multfMB[maxNBMB];


    // COMPUTATION
    int GPU; // 1 if matrix multiplication  done by GPU
    int GPUall; // 1 if scaling computations done by GPU
    int GPUusesem; // 1 if using semaphores to control GPU

    
    int AOLCOMPUTE_TOTAL_ASYNC; // 1 if performing image total in separate thread (runs faster, but image total dates from last frame)
    

    

    // LOOP TELEMETRY AND PERFORMANCE 
    // COMPUTED BY OPEN LOOP RETRIEVAL PROCESS
    
    double RMSmodes;
    double RMSmodesCumul;
    long long RMSmodesCumulcnt;

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
	long AveStats_NBpt; // averaging interval
	double blockave_OLrms[100]; // open loop RMS
	double blockave_Crms[100]; // correction RMS
	double blockave_WFSrms[100]; // WFS residual RMS
	double blockave_limFrac[100]; // fraction of mode coefficients exceeding limit

	double ALLave_OLrms; // open loop RMS
	double ALLave_Crms; // correction RMS
	double ALLave_WFSrms; // WFS residual RMS
	double ALLave_limFrac; // fraction of mode coefficients exceeding limit



    long logdataID; // image ID containing additional info that can be attached to a image stream log


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


int init_AOloopControl();


long AOloopControl_makeTemplateAOloopconf(long loopnb);
long AOloopControl_CrossProduct(char *ID1_name, char *ID2_name, char *IDout_name);
long AOloopControl_mkloDMmodes(char *ID_name, long msizex, long msizey, float CPAmax, float deltaCPA, double xc, double yc, double r0, double r1, int MaskMode);

long AOloopControl_mkCM(char *respm_name, char *cm_name, float SVDlim);
long AOloopControl_mkSlavedAct(char *IDmaskRM_name, float pixrad, char *IDout_name);
long AOloopControl_mkModes(char *ID_name, long msizex, long msizey, float CPAmax, float deltaCPA, double xc, double yx, double r0, double r1, int MaskMode, int BlockNB, float SVDlim);

int AOloopControl_camimage_extract2D_sharedmem_loop(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart);
int compute_ControlMatrix(long loop, long NB_MODE_REMOVED, char *ID_Rmatrix_name, char *ID_Cmatrix_name, char *ID_VTmatrix_name, double Beta, long NB_MODE_REMOVED_STEP, float eigenvlim);
int AOloopControl_InitializeMemory();
void *compute_function_imtotal( void *ptr );
void *compute_function_dark_subtract( void *ptr );
int Read_cam_frame(long loop, int RM, int normalize, int PixelStreamMode, int InitSem);
//long AOloopControl_MakeDMModes(long loop, long NBmodes, char *IDname);
int AOloopControl_AveStream(char *IDname, double alpha, char *IDname_out_ave, char *IDname_out_AC, char *IDname_out_RMS);
long AOloopControl_loadCM(long loop, char *CMfname);

long AOloopControl_2Dloadcreate_shmim(char *name, char *fname, long xsize, long ysize);
long AOloopControl_3Dloadcreate_shmim(char *name, char *fname, long xsize, long ysize, long zsize);
int AOloopControl_loadconfigure(long loop, int mode, int level);

int AOloopControl_set_modeblock_gain(long loop, long blocknb, float gain, int add);// modal blocks

int set_DM_modes(long loop);
int set_DM_modesRM(long loop);
//long AOloopControl_mkHadamardModes50(char *outname);
long AOloopControl_mkHadamardModes(char *DMmask_name, char *outname);
long AOloopControl_Hadamard_decodeRM(char *inname, char *Hmatname, char *indexname, char *outname);
long AOcontrolLoop_TestDMSpeed(char *dmname, long delayus, long NBpts, float ampl);

long AOcontrolLoop_TestSystemLatency(char *dmname, char *wfsname, long NBiter);

long AOloopControl_TestDMmodeResp(char *DMmodes_name, long index, float ampl, float fmin, float fmax, float fmultstep, float avetime, long dtus, char *DMmask_name, char *DMstream_in_name, char *DMstream_out_name, char *IDout_name);

long AOloopControl_TestDMmodes_Recovery(char *DMmodes_name, float ampl, char *DMmask_name, char *DMstream_in_name, char *DMstream_out_name, char *DMstream_meas_name, long tlagus, long NBave, char *IDout_name, char *IDoutrms_name, char *IDoutmeas_name, char *IDoutmeasrms_name);
long Measure_zonalRM(long loop, double ampl, long delayfr, long NBave, long NBexcl, char *zrespm_name, char *WFSref_name, char *WFSmap_name, char *DMmap_name, long mode, int normalize, int AOinitMode);
int AOloopControl_mkCalib_map_mask(long loop, char *zrespm_name, char *WFSmap_name, char *DMmap_name, float dmmask_perclow, float dmmask_coefflow, float dmmask_perchigh, float dmmask_coeffhigh);
int AOloopControl_ProcessZrespM(long loop, char *zrespm_name, char *WFSref0_name, char *WFSmap_name, char *DMmap_name, double rmampl, int normalize);
int AOloopControl_WFSzpupdate_loop(char *IDzpdm_name, char *IDzrespM_name, char *IDwfszp_name);
int AOloopControl_WFSzeropoint_sum_update_loop(long loopnb, char *ID_WFSzp_name, int NBzp, char *IDwfsref0_name, char *IDwfsref_name);

int Measure_Resp_Matrix(long loop, long NbAve, float amp, long nbloop, long fDelay, long NBiter);
int ControlMatrixMultiply( float *cm_array, float *imarray, long m, long n, float *outvect);
long compute_CombinedControlMatrix(char *IDcmat_name, char *IDmodes_name, char* IDwfsmask_name, char *IDdmmask_name, char *IDcmatc_name, char *IDcmatc_active_name);
int AOcompute(long loop, int normalize);
int AOloopControl_CompModes_loop(char *ID_CM_name, char *ID_WFSref_name, char *ID_WFSim_name, char *ID_WFSimtot_name, char *ID_coeff_name);
int AOloopControl_GPUmodecoeffs2dm_filt_loop(char *modecoeffs_name, char *DMmodes_name, int semTrigg, char *out_name, int GPUindex, long loop, int offloadMode);
 
long AOloopControl_mapPredictiveFilter(char *IDmodecoeff_name, long modeout, double delayfr);
double AOloopControl_testPredictiveFilter(char *IDtrace_name, long mode, double delayfr, long filtsize, char *IDfilt_name, double SVDeps);

int AOloopControl_run();

long AOloopControl_sig2Modecoeff(char *WFSim_name, char *IDwfsref_name, char *WFSmodes_name, char *outname);
int AOloopControl_printloopstatus(long loop, long nbcol, long IDmodeval_dm, long IDmodeval, long IDmodevalave, long IDmodevalrms, long ksize);
int AOloopControl_loopMonitor(long loop, double frequ, long nbcol);
int AOloopControl_statusStats();
int AOloopControl_showparams(long loop);
long AOloopControl_blockstats(long loop, char *IDout_name);
long AOloopControl_computeWFSresidualimage(long loop, float alpha);
long AOloopControl_ComputeOpenLoopModes(long loop);
long AOloopControl_dm2dm_offload(char *streamin, char *streamout, float twait, float offcoeff, float multcoeff);

int AOloopControl_setLoopNumber(long loop);
int AOloopControl_loopkill();
int AOloopControl_loopon();
int AOloopControl_loopoff();
int AOloopControl_logon();
int AOloopControl_loopstep(long loop, long NBstep);
int AOloopControl_logoff();
int AOloopControl_loopreset();
int AOloopControl_ARPFon();
int AOloopControl_ARPFoff();

int AOloopControl_set_loopfrequ(float loopfrequ);
int AOloopControl_set_hardlatency_frame(float hardlatency_frame);
int AOloopControl_set_complatency_frame(float complatency_frame);
int AOloopControl_set_wfsmextrlatency_frame(float wfsmextrlatency_frame);
int AOloopControl_setgain(float gain);
int AOloopControl_setARPFgain(float gain);
int AOloopControl_setWFSnormfloor(float WFSnormfloor);
int AOloopControl_setmaxlimit(float maxlimit);
int AOloopControl_setmult(float multcoeff);
int AOloopControl_setframesAve(long nbframes);




// "old" blocks (somewhat obsolete)
int AOloopControl_setgainrange(long m0, long m1, float gainval);
int AOloopControl_setlimitrange(long m0, long m1, float limval);
int AOloopControl_setmultfrange(long m0, long m1, float multfval);
int AOloopControl_setgainblock(long mb, float gainval);
int AOloopControl_setlimitblock(long mb, float limitval);
int AOloopControl_setmultfblock(long mb, float multfval);
int AOloopControl_resetRMSperf();
int AOloopControl_scanGainBlock(long NBblock, long NBstep, float gainStart, float gainEnd, long NBgain);

int AOloopControl_InjectMode( long index, float ampl );
int AOloopControl_AutoTune();


int AOloopControl_setparam(long loop, char *key, double value);



int AOloopControl_Measure_WFScam_PeriodicError(long loop, long NBframes, long NBpha, char *IDout_name); // OBSOLETE
int AOloopControl_Remove_WFScamPE(char *IDin_name, char *IDcorr_name, double pha); // OBSOLETE


int AOloopControl_DMmodulateAB(char *IDprobeA_name, char *IDprobeB_name, char *IDdmstream_name, char *IDrespmat_name, char *IDwfsrefstream_name, double delay, long NBprobes);

long AOloopControl_frameDelay(char *IDin_name, char *IDkern_name, char *IDout_name, int insem);

long AOloopControl_AnalyzeRM_sensitivity(char *IDdmmodes_name, char *IDdmmask_name, char *IDwfsref_name, char *IDwfsresp_name, char *IDwfsmask_name, float amplimitnm, float lambdanm, char *foutname);

#endif
