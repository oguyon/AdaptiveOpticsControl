#ifndef _AOSYSTSIM_H
#define _AOSYSTSIM_H



#define DISPCOMB_FILENAME_CONF "/tmp/dmdispcombconf.conf.shm"
#define DMTURBCONF_FILENAME "/tmp/dmturb.conf.shm"

#define DM_NUMBER_CHAN 8

typedef struct
{
    int ON;

    long loopcnt;
    long updatecnt;

    int busy; // if set to 1, hold off and wait
    float MAXVOLT; // maximum voltage on DM

    int status;

    float dmdispgain[DM_NUMBER_CHAN];

    struct timespec tstart;
    struct timespec tend;

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




int init_AOsystSim();

int AOloopControl_DM_setsize(long size1d);
int AOloopControl_DM_setname(char *name);
int AOloopControl_DM_disp2V(long IDdisp, long IDvolt);
int AOloopControl_DM_createconf();
int AOloopControl_DM_loadconf();
int AOloopControl_DM_unloadconf();
int AOloopControl_DM_CombineChannels(int mode);
int AOloopControl_DM_chan_setgain(int ch, float gain);
int AOloopControl_DM_dmdispcombstatus();
int AOloopControl_DM_dmdispcomboff();
int AOloopControl_DM_dmtrigoff();

int AOloopControl_DMturb_createconf();
int AOloopControl_DMturb_loadconf();
int AOloopControl_DM_dmturboff();
int AOloopControl_DM_dmturb_wspeed(double wspeed);
int AOloopControl_DM_dmturb_ampl(double ampl);
int AOloopControl_DM_dmturb_LOcoeff(double LOcoeff);
int AOloopControl_DM_dmturb_tint(long tint);
int AOloopControl_DM_dmturb_printstatus();
int AOloopControl_DM_turb();


#endif

