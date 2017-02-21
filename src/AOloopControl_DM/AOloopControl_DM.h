#ifndef _AOSYSTSIM_H
#define _AOSYSTSIM_H



#define DISPCOMB_FILENAME_CONF "/tmp/dmdispcombconf.conf.shm"
#define DMTURBCONF_FILENAME "/tmp/dmturb.conf.shm"

#define DM_NUMBER_CHANMAX 20 // max number of channel per DM

typedef struct
{
    int ON;
    
    long xsize; // DM xsize
    long ysize; // DM ysize
    long xysize;
    long NBchannel;
    
    long loopcnt;
    long updatecnt;

    int busy; // if set to 1, hold off and wait
    int voltmode;
    int voltON; // 1 if applying voltage 
    float MAXVOLT; // maximum voltage on DM
	int AveMode;
    float DClevel;

    int status;

    float dmdispgain[DM_NUMBER_CHANMAX];

    struct timespec tstart;
    struct timespec tend;

    long dmdispID[DM_NUMBER_CHANMAX];
    long IDdisp;
 
    int dm2dm_mode; // 1 if output disp should be remapped to output DM disp
    // following only applies of dm2dm_mode = 1
    long xsizeout;
    long ysizeout;
    long ID_dm2dm_DMmodes;
    long ID_dm2dm_outdisp;
    
    int wfsrefmode; // 1 if wfsref offset should be computed
    long xsizewfsref;
    long ysizewfsref;
    long ID_wfsref_RespMat;
    long ID_wfsref_out;
 
    long IDvolt;
    char voltname[200];

    long moninterval; // [us]
} AOLOOPCONTROL_DM_DISPCOMB_CONF;




typedef struct
{
    int on;
    long cnt;

    double wspeed; // wind speed [m/s]
    double ampl; // [um RMS]
    double LOcoeff; // 0 for full correction of low orders, 1 for no correction

    long tint; // interval between consecutive DM updates [us]


    double simtime;

    struct timespec tstart;
    struct timespec tend;

} AOLOOPCONTROL_DMTURBCONF;




int init_AOloopControl_DM();



int init_AOsystSim();

int AOloopControl_DM_disp2V(long DMindex);

int AOloopControl_DM_createconf();

int AOloopControl_DM_loadconf();

int AOloopControl_DM_unloadconf();

int AOloopControl_DM_CombineChannels(long DMindex, long xsize, long ysize, int NBchannel, int AveMode, int dm2dm_mode, const char *dm2dm_DMmodes, const char *dm2dm_outdisp, int wfsrefmode, const char *wfsref_WFSRespMat, const char *wfsref_out, int voltmode, const char *IDvolt_name, float DClevel, float maxvolt);


int AOloopControl_DM_chan_setgain(long DMindex, int ch, float gain);
int AOloopControl_DM_setvoltON(long DMindex);
int AOloopControl_DM_setvoltOFF(long DMindex);
int AOloopControl_DM_setMAXVOLT(long DMindex, float maxvolt);
int AOloopControl_DM_setDClevel(long DMindex, float DClevel);

int AOloopControl_DM_dmdispcombstatus(long DMindex);

int AOloopControl_DM_dmdispcomboff(long DMindex);

int AOloopControl_DM_dmtrigoff(long DMindex);





int AOloopControl_DMturb_createconf();

int AOloopControl_DMturb_loadconf();

int AOloopControl_DM_dmturboff(long DMindex);

int AOloopControl_DM_dmturb_wspeed(long DMindex, double wspeed);

int AOloopControl_DM_dmturb_ampl(long DMindex, double ampl);

int AOloopControl_DM_dmturb_LOcoeff(long DMindex, double LOcoeff);

int AOloopControl_DM_dmturb_tint(long DMindex, long tint);

int AOloopControl_DM_dmturb_printstatus(long DMindex);

int AOloopControl_DM_dmturb(long DMindex, int mode, const char *IDout_name, long NBsamples);


#endif

