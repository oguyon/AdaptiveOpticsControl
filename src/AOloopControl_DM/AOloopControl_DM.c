#include <fitsio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <sched.h>
#include <ncurses.h>
#include <semaphore.h>

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"

#include "AOloopControl_DM/AOloopControl_DM.h"
#include "AtmosphericTurbulence/AtmosphericTurbulence.h"




extern DATA data;

int wcol, wrow; // window size


struct timespec semwaitts;





#define DMSTROKE100 0.7 // um displacement for 100V

long DM_Xsize = 50;
long DM_Ysize = 50;
long NBact = 2500; // default
char DMname[200];

AOLOOPCONTROL_DM_DISPCOMB_CONF *dispcombconf; // configuration
int dmdispcomb_loaded = 0;
int SMfd;


AOLOOPCONTROL_DMTURBCONF *dmturbconf; // DM turbulence configuration
int dmturb_loaded = 0;
int SMturbfd;
long IDturb;




// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//


int AOloopControl_DM_setsize_cli()
{
    if(CLI_checkarg(1,2)==0)
        AOloopControl_DM_setsize(data.cmdargtoken[1].val.numl);
    else
        return 1;

    return 0;
}


int AOloopControl_DM_setname_cli()
{
    if(CLI_checkarg(1,3)==0)
        AOloopControl_DM_setname(data.cmdargtoken[1].val.string);
    else
        return 1;

    return 0;
}


int AOloopControl_DM_CombineChannels_cli()
{
    if(CLI_checkarg(1,2)==0)
        AOloopControl_DM_CombineChannels(data.cmdargtoken[1].val.numl);
    else
        AOloopControl_DM_CombineChannels(1);

    return 1;
}



int AOloopControl_DM_chan_setgain_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,1)==0)
        AOloopControl_DM_chan_setgain(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numf);
    else
        return 1;
}



int AOloopControl_DM_dmturb_wspeed_cli()
{
    if(CLI_checkarg(1,1)==0)
        AOloopControl_DM_dmturb_wspeed(data.cmdargtoken[1].val.numf);
    else
        return 1;
}

int AOloopControl_DM_dmturb_ampl_cli()
{
    if(CLI_checkarg(1,1)==0)
        AOloopControl_DM_dmturb_ampl(data.cmdargtoken[1].val.numf);
    else
        return 1;
}

int AOloopControl_DM_dmturb_LOcoeff_cli()
{
    if(CLI_checkarg(1,1)==0)
        AOloopControl_DM_dmturb_LOcoeff(data.cmdargtoken[1].val.numf);
    else
        return 1;
}

int AOloopControl_DM_dmturb_tint_cli()
{
    if(CLI_checkarg(1,2)==0)
        AOloopControl_DM_dmturb_tint(data.cmdargtoken[1].val.numl);
    else
        return 1;
}





int init_AOloopControl_DM()
{
    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "AO loop Control DM operation");
    sprintf(DMname, "dmdisp");
    data.NBmodule++;


    strcpy(data.cmd[data.NBcmd].key,"aolcontroldmsetsize");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_setsize_cli;
    strcpy(data.cmd[data.NBcmd].info,"set DM size");
    strcpy(data.cmd[data.NBcmd].syntax,"linear size (assumes square DM)");
    strcpy(data.cmd[data.NBcmd].example,"aolcontroldmsetsize 32");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_setsize(int size1d)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"aolcontroldmsetname");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_setname_cli;
    strcpy(data.cmd[data.NBcmd].info,"set DM name");
    strcpy(data.cmd[data.NBcmd].syntax,"replaces default dmdisp name");
    strcpy(data.cmd[data.NBcmd].example,"aolcontroldmsetname dmtestdisp_");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_setname(char *name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aolcontrolDMcomb");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_CombineChannels_cli;
    strcpy(data.cmd[data.NBcmd].info,"combine channels");
    strcpy(data.cmd[data.NBcmd].syntax,"no arg");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontrolDMcomb");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_CombineChannels(int mode)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aolcontroldmchgain");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_chan_setgain_cli;
    strcpy(data.cmd[data.NBcmd].info,"set gain for DM displacement channel");
    strcpy(data.cmd[data.NBcmd].syntax,"<chan#> <gain>");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmchgain 3 0.2");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_chan_setgain(int ch, float gain)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmcomboff");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp =  AOloopControl_DM_dmdispcomboff;
    strcpy(data.cmd[data.NBcmd].info,"turn off DM combine");
    strcpy(data.cmd[data.NBcmd].syntax,"no arg");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmcomboff");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmdispcomboff()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmcombmon");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp =  AOloopControl_DM_dmdispcombstatus;
    strcpy(data.cmd[data.NBcmd].info,"monitor DM comb program");
    strcpy(data.cmd[data.NBcmd].syntax,"no arg");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmcombmon");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmdispcombstatus()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmtrigoff");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp =  AOloopControl_DM_dmtrigoff;
    strcpy(data.cmd[data.NBcmd].info,"turn off DM trigger");
    strcpy(data.cmd[data.NBcmd].syntax,"no arg");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmtrigoff");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmtrigoff()");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"aoloopcontrolDMturb");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_turb;
    strcpy(data.cmd[data.NBcmd].info,"DM turbulence");
    strcpy(data.cmd[data.NBcmd].syntax,"no arg");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontrolDMturb");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_turb()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmturboff");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp =  AOloopControl_DM_dmturboff;
    strcpy(data.cmd[data.NBcmd].info,"turn off DM turbulence");
    strcpy(data.cmd[data.NBcmd].syntax,"no arg");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmturboff");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmturboff()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmturws");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_dmturb_wspeed_cli;
    strcpy(data.cmd[data.NBcmd].info,"set turbulence wind speed");
    strcpy(data.cmd[data.NBcmd].syntax,"<wind speed [m/s]>");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmturws 5.2");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmturb_wspeed(double wspeed);");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmturampl");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_dmturb_ampl_cli;
    strcpy(data.cmd[data.NBcmd].info,"set turbulence amplitude");
    strcpy(data.cmd[data.NBcmd].syntax,"<amplitude [um]>");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmturampl 0.1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmturb_ampl(double ampl);");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmturlo");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_dmturb_LOcoeff_cli;
    strcpy(data.cmd[data.NBcmd].info,"set turbulence low order coefficient");
    strcpy(data.cmd[data.NBcmd].syntax,"<coeff>");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmturlo 0.2");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_dmturb_LOcoeff(double LOcoeff);");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"aoloopcontroldmturtint");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = AOloopControl_DM_dmturb_tint_cli;
    strcpy(data.cmd[data.NBcmd].info,"set turbulence interval time");
    strcpy(data.cmd[data.NBcmd].syntax,"<interval time [us] long>");
    strcpy(data.cmd[data.NBcmd].example,"aoloopcontroldmturtint 200");
    strcpy(data.cmd[data.NBcmd].Ccall,"int AOloopControl_DM_turb_tint(long tint);");
    data.NBcmd++;





    // add atexit functions here
    atexit((void*) AOloopControl_DM_unloadconf);

    return 0;
}








struct timespec time_diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}





int AOloopControl_DM_setsize(long size1d)
{
    DM_Xsize = size1d;
    DM_Ysize = size1d;
    NBact = DM_Xsize*DM_Ysize; // default

    return 0;
}



int AOloopControl_DM_setname(char *name)
{
    sprintf(DMname, "%s", name);

    return 0;
}





int AOloopControl_DM_disp2V(long IDdisp, long IDvolt)
{
    long ii;
    float volt;



    data.image[IDvolt].md[0].write = 1;
    for(ii=0; ii<NBact; ii++)
    {
        volt = 100.0*sqrt(data.image[IDdisp].array.F[ii]/DMSTROKE100);
        if(volt>dispcombconf[0].MAXVOLT)
            volt = dispcombconf[0].MAXVOLT;
        data.image[IDvolt].array.U[ii] = (unsigned short int) (volt/300.0*65536.0);
    }

    data.image[IDvolt].md[0].write = 0;
    data.image[IDvolt].md[0].cnt0++;



    return 0;
}




int AOloopControl_DM_createconf()
{
    int result;
    int ch;

    if( dmdispcomb_loaded == 0 )
    {
        printf("Create/read configuration\n");

        SMfd = open(DISPCOMB_FILENAME_CONF, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
        if (SMfd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        result = lseek(SMfd, sizeof(AOLOOPCONTROL_DM_DISPCOMB_CONF)-1, SEEK_SET);
        if (result == -1) {
            close(SMfd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }

        result = write(SMfd, "", 1);
        if (result != 1) {
            close(SMfd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }

        dispcombconf = (AOLOOPCONTROL_DM_DISPCOMB_CONF*)mmap(0, sizeof(AOLOOPCONTROL_DM_DISPCOMB_CONF), PROT_READ | PROT_WRITE, MAP_SHARED, SMfd, 0);
        if (dispcombconf == MAP_FAILED) {
            close(SMfd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }


        dispcombconf[0].ON = 1;
        dispcombconf[0].busy = 0;
        dispcombconf[0].MAXVOLT = 150.0;
        dispcombconf[0].moninterval = 30000; // 33Hz
        dispcombconf[0].status = 0;

        for(ch=0; ch<DM_NUMBER_CHAN; ch++)
            dispcombconf[0].dmdispgain[ch] = 1.0;

        dmdispcomb_loaded = 1;

    }

    return 0;
}



int AOloopControl_DM_loadconf()
{
    int result;

    if( dmdispcomb_loaded == 0 )
    {
        printf("Create/read configuration\n");

        SMfd = open(DISPCOMB_FILENAME_CONF, O_RDWR, (mode_t)0600);
        if (SMfd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }
        dispcombconf = (AOLOOPCONTROL_DM_DISPCOMB_CONF*)mmap(0, sizeof(AOLOOPCONTROL_DM_DISPCOMB_CONF), PROT_READ | PROT_WRITE, MAP_SHARED, SMfd, 0);
        if (dispcombconf == MAP_FAILED) {
            close(SMfd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }

        dmdispcomb_loaded = 1;
    }

    return 0;
}



int AOloopControl_DM_unloadconf()
{
    if( dmdispcomb_loaded == 1 )
    {
        if (munmap(dispcombconf, sizeof(AOLOOPCONTROL_DM_DISPCOMB_CONF)) == -1)
            perror("Error un-mmapping the file");
        close(SMfd);
        dmdispcomb_loaded = 0;
    }
    return 0;
}


//
// mode = 1 if DM volt computed
//
// NOTE: responds immediately to sem1 in dmdisp
//
int AOloopControl_DM_CombineChannels(int mode)
{
    long naxis = 2;
    long xsize = DM_Xsize;
    long ysize = DM_Ysize;
    long *size;
    long ch;
    char name[200];
    long *IDch;
    long cnt = 0;
    long long cntsumold;
    long long cntsum;
    long ii;
    long IDdisp;
    long IDvolt;
    double ave;
    long ID1;
    int RT_priority = 95; //any number from 0-99
    struct sched_param schedpar;
    int r;
    long sizexy;
    float *dmdispptr;
    float *dmdispptr_array[20];
    long IDdispt;
    char sname[200];
    long nsecwait = 100000; // 100 us

    schedpar.sched_priority = RT_priority;
    r = seteuid(euid_called); //This goes up to maximum privileges
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    r = seteuid(euid_real);//Go back to normal privileges


    size = (long*) malloc(sizeof(long)*naxis);
    IDch = (long*) malloc(sizeof(long)*DM_NUMBER_CHAN);
    size[0] = xsize;
    size[1] = ysize;
    sizexy = xsize*ysize;


    AOloopControl_DM_createconf();
    dispcombconf[0].ON = 1;
    dispcombconf[0].status = 0;

    printf("Initialize channels\n");

    for(ch=0; ch<DM_NUMBER_CHAN; ch++)
    {
        sprintf(name, "%s%ld", DMname, ch);
        printf("Channel %ld \n", ch);
        IDch[ch] = create_image_ID(name, naxis, size, FLOAT, 1, 10);
        dmdispptr_array[ch] = data.image[IDch[ch]].array.F;
    }


    IDdisp = create_image_ID(DMname, naxis, size, FLOAT, 1, 10);

    IDdispt = create_image_ID("dmdispt", naxis, size, FLOAT, 0, 0);
    dmdispptr = data.image[IDdispt].array.F;

    if(mode==1)
        IDvolt = create_image_ID("dmvolt", naxis, size, USHORT, 1, 10);

    cntsumold = 0;

    dispcombconf[0].status = 1;

  COREMOD_MEMORY_image_set_createsem(DMname);

    if(data.image[IDdisp].sem1==0)
    {
        sprintf(sname, "%s_sem1", data.image[IDdisp].md[0].name);
        if ((data.image[IDdisp].semptr1 = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
            perror("semaphore 1 initilization error");
            exit(1);
        }
        else
            printf("semaphore 1 initialized for image %s \n", DMname);
        data.image[IDdisp].sem1 = 1;
    }
    else
        printf("image %s already has semaphore\n", DMname);


    while(dispcombconf[0].ON == 1)
    {
        dispcombconf[0].status = 2;

        if (clock_gettime(CLOCK_REALTIME, &semwaitts) == -1) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        semwaitts.tv_nsec += nsecwait;
        if(semwaitts.tv_nsec >= 1000000000)
            semwaitts.tv_sec = semwaitts.tv_sec + 1;

        sem_timedwait(data.image[IDdisp].semptr1, &semwaitts);

        cntsum = 0;



        for(ch=0; ch<DM_NUMBER_CHAN; ch++)
            cntsum += data.image[IDch[ch]].md[0].cnt0;


        
            
        if(cntsum != cntsumold)
        {
            dispcombconf[0].status = 3;
            cnt++;

            memcpy (data.image[IDdispt].array.F, dmdispptr_array[0], sizeof(float)*sizexy);
            for(ch=1; ch<DM_NUMBER_CHAN; ch++)
            {
                for(ii=0; ii<sizexy; ii++)
                    dmdispptr[ii] += dispcombconf[0].dmdispgain[ch]*dmdispptr_array[ch][ii];
            }

            dispcombconf[0].status = 4;

            // REMOVE DC LEVEL AND MOVE TO MEAN MOTION RANGE
            ave = 0.0;
            for(ii=0; ii<NBact; ii++)
                ave += data.image[IDdispt].array.F[ii];
            ave /= NBact;

            dispcombconf[0].status = 5;

            for(ii=0; ii<NBact; ii++)
            {
                data.image[IDdispt].array.F[ii] += 0.5*(DMSTROKE100*dispcombconf[0].MAXVOLT/100.0*dispcombconf[0].MAXVOLT/100.0)-ave;
                if(data.image[IDdispt].array.F[ii]<0.0)
                    data.image[IDdispt].array.F[ii] = 0.0;
            }

            dispcombconf[0].status = 6;

            data.image[IDdisp].md[0].write = 1;
            memcpy (data.image[IDdisp].array.F,data.image[IDdispt].array.F, sizeof(float)*data.image[IDdisp].md[0].nelement);
            data.image[IDdisp].md[0].cnt0++;
            data.image[IDdisp].md[0].write = 0;            
            sem_post(data.image[IDdisp].semptr);
 
 
            dispcombconf[0].status = 7;

            if(mode==1)
                AOloopControl_DM_disp2V(IDdisp, IDvolt);

            dispcombconf[0].status = 8;

            cntsumold = cntsum;
        }
    }

    if(mode==1)
        arith_image_zero("dmvolt");



    printf("LOOP STOPPED\n");
    fflush(stdout);

    free(size);
    free(IDch);

    return 0;
}






int AOloopControl_DM_chan_setgain(int ch, float gain)
{
    AOloopControl_DM_loadconf();
    if(ch<DM_NUMBER_CHAN)
        dispcombconf[0].dmdispgain[ch] = gain;

    return 0;
}





int AOloopControl_DM_dmdispcombstatus()
{
    long long mcnt = 0;
    int ch;

    AOloopControl_DM_loadconf();

    initscr();
    getmaxyx(stdscr, wrow, wcol);

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_RED);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    while( !kbdhit() )
    {
        usleep(dispcombconf[0].moninterval);
        clear();
        attron(A_BOLD);
        print_header(" PRESS ANY KEY TO STOP MONITOR ", '-');
        attroff(A_BOLD);
        printw("    %ld\n", mcnt);
        printw("ON         %d\n", dispcombconf[0].ON);
        printw("cnt       %ld\n", dispcombconf[0].loopcnt);
        printw("updatecnt %ld\n", dispcombconf[0].updatecnt);
        printw("busy      %d\n", dispcombconf[0].busy);
        printw("MAXVOLT   %f\n", dispcombconf[0].MAXVOLT);
        printw("status    %d\n",  dispcombconf[0].status);
        printw("moninterval %d\n", dispcombconf[0].moninterval);
        printw("\n");
        for(ch=0; ch<DM_NUMBER_CHAN; ch++)
        {
            printw("  %2d   %5.3f\n", ch, dispcombconf[0].dmdispgain[ch]);
        }

        mcnt++;
        refresh();
    }
    endwin();

    return 0;
}






int AOloopControl_DM_dmdispcomboff()
{
    AOloopControl_DM_loadconf();
    dispcombconf[0].ON = 0;

    return 0;
}

int AOloopControl_DM_dmtrigoff()
{
    long ID;

    ID=image_ID("dmvolt");

    if(ID!=-1)
        data.image[ID].md[0].status = 101;
    else
    {
        ID = read_sharedmem_image("dmvolt");
        data.image[ID].md[0].status = 101;
    }

    return 0;
}



















int AOloopControl_DMturb_createconf()
{
    int result;
    long IDc1;
    char name[200];
    
    if( dmturb_loaded == 0 )
    {
        printf("Create/read configuration\n");
        fflush(stdout);

        sprintf(name, "%s1", DMname);
        IDc1 = image_ID(name);
        if(IDc1 == -1)
            IDc1 = read_sharedmem_image(name);
        
        if(IDc1==-1)
        {
            IDc1 = create_2Dimage_ID("turbch", DM_Xsize, DM_Ysize);
        }
        else
        {
            DM_Xsize = data.image[IDc1].md[0].size[0];
            DM_Ysize = data.image[IDc1].md[0].size[1];
        }

        IDturb = create_2Dimage_ID("turbs", DM_Xsize, DM_Ysize);

        

        SMturbfd = open(DMTURBCONF_FILENAME, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
        if (SMturbfd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        result = lseek(SMturbfd, sizeof(AOLOOPCONTROL_DMTURBCONF)-1, SEEK_SET);
        if (result == -1) {
            close(SMturbfd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }

        result = write(SMturbfd, "", 1);
        if (result != 1) {
            close(SMturbfd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }

        dmturbconf = (AOLOOPCONTROL_DMTURBCONF*)mmap(0, sizeof(AOLOOPCONTROL_DMTURBCONF), PROT_READ | PROT_WRITE, MAP_SHARED, SMturbfd, 0);
        if (dmturbconf == MAP_FAILED) {
            close(SMturbfd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }

        dmturbconf[0].on = 1;
        dmturbconf[0].ampl = 0.01; // [um]
        dmturbconf[0].tint = 100; // [us]
        dmturbconf[0].simtime = 0.0; // sec
        dmturbconf[0].wspeed = 10.0; // [m/s]
        dmturbconf[0].LOcoeff = 0.2;

        dmturb_loaded = 1;

    }

    return 0;
}







int AOloopControl_DMturb_loadconf()
{
    int result;

    if( dmturb_loaded == 0 )
    {
        printf("Create/read configuration\n");

        SMturbfd = open(DMTURBCONF_FILENAME, O_RDWR, (mode_t)0600);
        if (SMturbfd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        dmturbconf = (AOLOOPCONTROL_DMTURBCONF*)mmap(0, sizeof(AOLOOPCONTROL_DMTURBCONF), PROT_READ | PROT_WRITE, MAP_SHARED, SMturbfd, 0);
        if (dmturbconf == MAP_FAILED) {
            close(SMturbfd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
        dmturb_loaded = 1;
    }

    return 0;
}





int AOloopControl_DM_dmturboff()
{
    AOloopControl_DMturb_loadconf();
    dmturbconf[0].on = 0;
    AOloopControl_DM_dmturb_printstatus();

    return 0;
}

int AOloopControl_DM_dmturb_wspeed(double wspeed)
{
    AOloopControl_DMturb_loadconf();
    dmturbconf[0].wspeed = wspeed;
    AOloopControl_DM_dmturb_printstatus();

    return 0;
}

int AOloopControl_DM_dmturb_ampl(double ampl)
{
    AOloopControl_DMturb_loadconf();
    dmturbconf[0].ampl = ampl;
    AOloopControl_DM_dmturb_printstatus();

    return 0;
}

int AOloopControl_DM_dmturb_LOcoeff(double LOcoeff)
{
    AOloopControl_DMturb_loadconf();
    dmturbconf[0].LOcoeff = LOcoeff;
    AOloopControl_DM_dmturb_printstatus();

    return 0;
}

int AOloopControl_DM_dmturb_tint(long tint)
{
    AOloopControl_DMturb_loadconf();
    dmturbconf[0].tint = tint;
    AOloopControl_DM_dmturb_printstatus();

    return 0;
}



int AOloopControl_DM_dmturb_printstatus()
{
    AOloopControl_DMturb_loadconf();

    printf("Run time = %.3f sec\n", dmturbconf[0].simtime);
    printf("\n");
    printf("cnt              : %ld   (ave frequ = %.2f kHz)\n", dmturbconf[0].cnt, 0.001*dmturbconf[0].cnt/dmturbconf[0].simtime);
    printf("\n");

    if(dmturbconf[0].on == 1)
        printf("LOOP IS ON\n");
    else
        printf("LOOP IS OFF\n");

    printf("ampl    =  %.2f um\n", dmturbconf[0].ampl);
    printf("wspeed  =  %.2f m/s\n", dmturbconf[0].wspeed);
    printf("tint    =  %ld us\n", dmturbconf[0].tint);
    printf("Requested uptdate frequ = %.2f kHz\n", 0.001/(1.0e-6*dmturbconf[0].tint));
    printf("\n");
    printf("\n");

    return(0);
}


int AOloopControl_DM_turb()
{
    long size_sx; // screen size
    long size_sy;
    long IDs1, IDs2;
    char name[200];

    struct timespec tlast;
    struct timespec tdiff;
    struct timespec tdiff1;
    double tdiff1v;

    float screen0_X;
    float screen0_Y;
    long ii, jj, ii1;
    float x, y;
    float xpix, ypix;
    float xpixf, ypixf;
    long xpix1, xpix2, ypix1, ypix2;
    float ave;

    double angle = 1.0;
    double coeff = 0.001;

    double RMSval;
    long RMSvalcnt;
    double r;

    float pixscale = 0.1; // [m/pix]
    // Subaru pupil ~ 80 pix diam
    // Single actuator ~7 pix


    long IDturbs1;




    AOloopControl_DMturb_createconf();

    IDs1 = load_fits("~/conf/turb/turbscreen0.fits", "screen1", 1);
    IDs2 = load_fits("~/conf/turb/turbscreen0g.fits", "screen2", 1);
   
    if(IDs1==-1)
    {
        make_master_turbulence_screen("screen1", "screen2", 2048, 200.0, 1.0);
        IDs1 = image_ID("screen1");
        IDs2 = image_ID("screen2");
        save_fits("screen1", "!screen1.fits");
        save_fits("screen2", "!screen2.fits");
    }
    

    printf("ARRAY SIZE = %ld %ld\n", data.image[IDs1].md[0].size[0], data.image[IDs1].md[0].size[1]);
    size_sx = data.image[IDs1].md[0].size[0];
    size_sy = data.image[IDs1].md[0].size[1];

    clock_gettime(CLOCK_REALTIME, &dmturbconf[0].tstart);
    dmturbconf[0].tend = dmturbconf[0].tstart;

    IDturbs1 = create_2Dimage_ID("turbs1", DM_Xsize, DM_Ysize);

    while(dmturbconf[0].on == 1) // computation loop
    {
        usleep(dmturbconf[0].tint);

        tlast = dmturbconf[0].tend;
        clock_gettime(CLOCK_REALTIME, &dmturbconf[0].tend);
        tdiff = time_diff(dmturbconf[0].tstart, dmturbconf[0].tend);
        tdiff1 =  time_diff(tlast, dmturbconf[0].tend);
        tdiff1v = 1.0*tdiff1.tv_sec + 1.0e-9*tdiff1.tv_nsec;

        screen0_X += dmturbconf[0].wspeed*tdiff1v*cos(angle); // [m]
        screen0_Y += dmturbconf[0].wspeed*tdiff1v*sin(angle); // [m]


        dmturbconf[0].simtime = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;


        for(ii=0; ii<DM_Xsize; ii++)
            for(jj=0; jj<DM_Ysize; jj++)
            {
                ii1 = jj*DM_Xsize+ii;

                x = 10.0*ii/DM_Xsize + screen0_X; // [m]
                y = 10.0*jj/DM_Ysize + screen0_Y; // [m]

                xpix = 0.5*size_sx + x/pixscale;
                ypix = 0.5*size_sy + y/pixscale;

                xpix1 = ((long) xpix)%size_sx;
                xpix2 = (xpix1+1)%size_sx;
                xpixf = xpix- (long) xpix;
                ypix1 = ((long) ypix)%size_sy;
                ypix2 = (ypix1+1)%size_sy;
                ypixf = ypix - (long) ypix;

                while(xpix1<0)
                    xpix1 = 0;
                while(xpix1>size_sx-1)
                    xpix1 = size_sx-1;

                if(ypix1<0)
                    ypix1 = 0;
                if(ypix1>size_sy-1)
                    ypix1 = size_sy-1;

                data.image[IDturbs1].array.F[ii1] = 1.0*xpix1;

                data.image[IDturb].array.F[ii1] = (1.0-xpixf)*(1.0-ypixf)*(data.image[IDs1].array.F[ypix1*size_sx+xpix1]-(1.0-dmturbconf[0].LOcoeff)*data.image[IDs2].array.F[ypix1*size_sx+xpix1]);

                data.image[IDturb].array.F[ii1]  +=  (xpixf)*(1.0-ypixf)*(data.image[IDs1].array.F[ypix1*size_sx+xpix2]-(1.0-dmturbconf[0].LOcoeff)*data.image[IDs2].array.F[ypix1*size_sx+xpix2]);

                data.image[IDturb].array.F[ii1]  += (1.0-xpixf)*(ypixf)*(data.image[IDs1].array.F[ypix2*size_sx+xpix1]-(1.0-dmturbconf[0].LOcoeff)*data.image[IDs2].array.F[ypix2*size_sx+xpix1]);

                data.image[IDturb].array.F[ii1]  += xpixf*ypixf*(data.image[IDs1].array.F[ypix2*size_sx+xpix2]-(1.0-dmturbconf[0].LOcoeff)*data.image[IDs2].array.F[ypix2*size_sx+xpix2]);
            }

        // proccess array
        ave = 0.0;
        for(ii1=0; ii1<DM_Xsize*DM_Ysize; ii1++)
            ave += data.image[IDturb].array.F[ii1];
        ave /= NBact;
        for(ii1=0; ii1<DM_Xsize*DM_Ysize; ii1++)
        {
            data.image[IDturb].array.F[ii1] -= ave;
            data.image[IDturb].array.F[ii1] *= coeff;
        }

        RMSval = 0.0;
        RMSvalcnt = 0;

        for(ii=0; ii<DM_Xsize; ii++)
            for(jj=0; jj<DM_Ysize; jj++)
            {
                ii1 = DM_Xsize*jj+ii;
                x = 0.5*DM_Xsize - 0.5 - ii;
                y = 0.5*DM_Ysize - 0.5 - jj;
                r = sqrt(x*x+y*y);
                if(r<DM_Xsize*0.5-1.0)
                {
                    RMSval += data.image[IDturb].array.F[ii1]*data.image[IDturb].array.F[ii1];
                    RMSvalcnt++;
                }
            }
        RMSval = sqrt(RMSval/RMSvalcnt);

        if(RMSval>dmturbconf[0].ampl)
            coeff *= 0.999;
        else
            coeff *= 1.001;
        
        sprintf(name, "%s1", DMname);
        copy_image_ID("turbs", name, 0);
        save_fits("turbs", "!turbs.fits");
        save_fits("turbs1", "!turbs1.fits");
    }


    return(0);
}



