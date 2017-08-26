/**
 * @file    AOloopControl_IOtools.c
 * @brief   Adaptive Optics Control loop engine I/O tools
 * 
 * AO engine uses stream data structure
 *  
 * @author  O. Guyon
 * @date    22 Aug 2017
 *
 * 
 * @bug No known bugs.
 * 
 * 
 */



#define _GNU_SOURCE

// uncomment for test print statements to stdout
//#define _PRINT_TEST



/* =============================================================================================== */
/* =============================================================================================== */
/*                                        HEADER FILES                                             */
/* =============================================================================================== */
/* =============================================================================================== */

#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


#ifdef __MACH__
#include <mach/mach_time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
int clock_gettime(int clk_id, struct mach_timespec *t) {
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t time;
    time = mach_absolute_time();
    double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
    double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
    t->tv_sec = seconds;
    t->tv_nsec = nseconds;
    return 0;
}
#else
#include <time.h>
#endif



#include <fitsio.h>

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "COREMOD_arith/COREMOD_arith.h"

#include "statistic/statistic.h"

#include "AOloopControl/AOloopControl.h"
#include "AOloopControl_IOtools/AOloopControl_IOtools.h"



/* =============================================================================================== */
/* =============================================================================================== */
/*                                      DEFINES, MACROS                                            */
/* =============================================================================================== */
/* =============================================================================================== */






/* =============================================================================================== */
/* =============================================================================================== */
/*                                  GLOBAL DATA DECLARATION                                        */
/* =============================================================================================== */
/* =============================================================================================== */




/* =============================================================================================== */
/*                                     MAIN DATA STRUCTURES                                        */
/* =============================================================================================== */

extern DATA data;

extern long LOOPNUMBER; // current loop index

extern AOLOOPCONTROL_CONF *AOconf; // configuration - this can be an array















// CLI commands
//
// function CLI_checkarg used to check arguments
// CLI_checkarg ( CLI argument index , type code )
//
// type codes:
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string
//





/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 2. LOW LEVEL UTILITIES & TOOLS    
 *  Useful tools */
/* =============================================================================================== */
/* =============================================================================================== */

/* =============================================================================================== */
/** @name AOloopControl - 2.1. LOW LEVEL UTILITIES & TOOLS - LOAD DATA STREAMS */
/* =============================================================================================== */

/* =============================================================================================== */
/** @name AOloopControl - 2.2. LOW LEVEL UTILITIES & TOOLS - DATA STREAMS PROCESSING */
/* =============================================================================================== */

/** @brief CLI function for AOloopControl_AveStream */
int_fast8_t AOloopControl_AveStream_cli() {
    if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,3)+CLI_checkarg(4,3)+CLI_checkarg(5,3)==0) {
        AOloopControl_AveStream(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.string);
        return 0;
    }
    else return 1;
}


/** @brief CLI function for AOloopControl_frameDelay */
int_fast8_t AOloopControl_frameDelay_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,5)+CLI_checkarg(4,2)==0)    {
        AOloopControl_frameDelay(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.numl);
        return 0;
    }
    else        return 1;
}


/** @brief CLI function for AOloopControl_stream3Dto2D */
int_fast8_t AOloopControl_stream3Dto2D_cli() {
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0) {
        AOloopControl_stream3Dto2D(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
        return 0;
    }
    else return 1;
}















/* =============================================================================================== */
/* =============================================================================================== */
/*                                    FUNCTIONS SOURCE CODE                                        */
/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl functions */



int_fast8_t init_AOloopControl_IOtools()
{

    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "AO loop control IO tools");
    data.NBmodule++;


/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 2. LOW LEVEL UTILITIES & TOOLS                                           */
/* =============================================================================================== */
/* =============================================================================================== */


/* =============================================================================================== */
/** @name AOloopControl - 2.1. LOW LEVEL UTILITIES & TOOLS - LOAD DATA STREAMS                     */
/* =============================================================================================== */

/* =============================================================================================== */
/** @name AOloopControl - 2.2. LOW LEVEL UTILITIES & TOOLS - DATA STREAMS PROCESSING               */
/* =============================================================================================== */

    RegisterCLIcommand("aveACshmim", __FILE__, AOloopControl_AveStream_cli, "average and AC shared mem image", "<input image> <coeff> <output image ave> <output AC> <output RMS>" , "aveACshmim imin 0.01 outave outAC outRMS", "int AOloopControl_AveStream(char *IDname, double alpha, char *IDname_out_ave, char *IDname_out_AC, char *IDname_out_RMS)");

    RegisterCLIcommand("aolframedelay", __FILE__, AOloopControl_frameDelay_cli, "introduce temporal delay", "<in> <temporal kernel> <out> <sem index>","aolframedelay in kern out 0","long AOloopControl_frameDelay(const char *IDin_name, const char *IDkern_name, const char *IDout_name, int insem)");

    RegisterCLIcommand("aolstream3Dto2D", __FILE__, AOloopControl_stream3Dto2D_cli, "remaps 3D cube into 2D image", "<input 3D stream> <output 2D stream> <# cols> <sem trigger>" , "aolstream3Dto2D in3dim out2dim 4 1", "long AOloopControl_stream3Dto2D(const char *in_name, const char *out_name, int NBcols, int insem)");





    // add atexit functions here
    // atexit((void*) myfunc);

    return 0;
}






























/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 2. LOW LEVEL UTILITIES & TOOLS                                           */
/* =============================================================================================== */
/* =============================================================================================== */




/* =============================================================================================== */
/** @name AOloopControl - 2.1. LOW LEVEL UTILITIES & TOOLS - LOAD DATA STREAMS                     */
/* =============================================================================================== */



long AOloopControl_2Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize)
{
    long ID;
    int CreateSMim = 0;
    int sizeOK;
    uint32_t *sizearray;


    int loadcreatestatus = -1;
    // value of loadcreatestatus :
    // 0 : existing stream has wrong size -> recreating stream
    // 1 : new stream created and content loaded
    // 2 : existing stream updated
    // 3 : FITS image <fname> has wrong size -> do nothing
    // 4 : FITS image <fname> does not exist, stream <name> exists -> do nothing
    // 5 : FITS image <fname> does not exist, stream <name> does not exist -> create empty stream



#ifdef AOLOOPCONTROL_LOGFUNC
	AOLOOPCONTROL_logfunc_level = 2;
    CORE_logFunctionCall( AOLOOPCONTROL_logfunc_level, AOLOOPCONTROL_logfunc_level_max, 0, __FUNCTION__, __LINE__, "");
#endif


    ID = image_ID(name);
    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*2);

    if(ID==-1) // if <name> is not loaded in memory
    {
        CreateSMim = 0;
        ID = read_sharedmem_image(name);
        if(ID!=-1)  // ... and <name> does not exist as a memory stream
        {
            sizeOK = COREMOD_MEMORY_check_2Dsize(name, xsize, ysize);
            if(sizeOK==0)  // if size is different, delete stream -> create new one
            {
                char command[500];

                printf("\n========== EXISTING %s HAS WRONG SIZE -> CREATING BLANK %s ===========\n\n", name, name);
                delete_image_ID(name);

                if(sprintf(command, "rm /tmp/%s.im.shm", name) < 1)
                    printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");

                if(system(command)!=0)
                    printERROR(__FILE__,__func__,__LINE__, "system() error");

                CreateSMim = 1;
                loadcreatestatus = 0;
            }
        }
        else   //  ... and <name> does not exist as a stream -> create new stream
        {
            CreateSMim = 1;
            loadcreatestatus = 1;
        }

        if(CreateSMim == 1)
        {
            sizearray[0] =  xsize;
            sizearray[1] =  ysize;
            if(xsize*ysize>0)
                ID = create_image_ID(name, 2, sizearray, _DATATYPE_FLOAT, 1, 0);
        }
    }
    free(sizearray);


    if(ID==-1)
    {
        printf("ERROR: could not load/create %s\n", name);
        exit(0);
    }
    else
    {
        long ID1;

        ID1 = load_fits(fname, "tmp2Dim", 3);
        
        if(ID1!=-1)
        {
            sizeOK = COREMOD_MEMORY_check_2Dsize("tmp2Dim", xsize, ysize);
            if(sizeOK==1)
            {
                memcpy(data.image[ID].array.F, data.image[ID1].array.F, sizeof(float)*xsize*ysize);
                printf("loaded file \"%s\" to shared memory \"%s\"\n", fname, name);
                loadcreatestatus = 2;
            }
            else
            {
                printf("File \"%s\" has wrong size (should be 2-D %ld x %ld,  is %d-D %ld x %ld): ignoring\n", fname, xsize, ysize, (int) data.image[ID1].md[0].naxis, (long) data.image[ID1].md[0].size[0], (long) data.image[ID1].md[0].size[1]);
                loadcreatestatus = 3;
            }
            delete_image_ID("tmp2Dim");
        }
        else
        {
            if(CreateSMim==0)
                loadcreatestatus = 4;
            else
                loadcreatestatus = 5;
        }
    }

    // logging


    if(loadcreateshm_log == 1) // results should be logged in ASCII file
    {
        switch ( loadcreatestatus ) {
        case 0 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: existing stream has wrong size -> recreating stream\n", fname, name);
            break;
        case 1 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: new stream created and content loaded\n", fname, name);
            break;
        case 2 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: existing stream updated\n", fname, name);
            break;
        case 3 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: FITS image has wrong size -> do nothing\n", fname, name);
            break;
        case 4 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: FITS image does not exist, stream exists -> do nothing\n", fname, name);
            break;
        case 5 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: FITS image does not exist, stream does not exist -> create empty stream\n", fname, name);
            break;
        default:
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: UNKNOWN ERROR CODE\n", fname, name);
            break;
        }
    }


#ifdef AOLOOPCONTROL_LOGFUNC
	AOLOOPCONTROL_logfunc_level = 2;
    CORE_logFunctionCall( AOLOOPCONTROL_logfunc_level, AOLOOPCONTROL_logfunc_level_max, 1, __FUNCTION__, __LINE__, "");
#endif


    return ID;
}





long AOloopControl_3Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize, long zsize)
{
    long ID;
    int CreateSMim;
    int sizeOK;
    uint32_t *sizearray;
    long ID1;
    int creashmimfromFITS = 0;

    int loadcreatestatus = -1;
    // value of loadcreatestatus :
    // 0 : existing stream has wrong size -> recreating stream
    // 1 : new stream created and content loaded
    // 2 : existing stream updated
    // 3 : FITS image <fname> has wrong size -> do nothing
    // 4 : FITS image <fname> does not exist, stream <name> exists -> do nothing
    // 5 : FITS image <fname> does not exist, stream <name> does not exist -> create empty stream
    // 6 : stream exists, size is correct



#ifdef AOLOOPCONTROL_LOGFUNC
	AOLOOPCONTROL_logfunc_level = 2;
    CORE_logFunctionCall( AOLOOPCONTROL_logfunc_level, AOLOOPCONTROL_logfunc_level_max, 0, __FUNCTION__, __LINE__, "");
#endif


    printf("-------- ENTERING AOloopControl_3Dloadcreate_shmim ----------\n");
    fflush(stdout);


    ID = image_ID(name);
    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*3);

    printf("        ENTERING AOloopControl_3Dloadcreate_shmim: ============== %ld  %ld  %ld ===== %ld ======\n", xsize, ysize, zsize, ID);
    fflush(stdout);

    if(ID==-1)
    {
        CreateSMim = 0;
        ID = read_sharedmem_image(name);

        printf("        AOloopControl_3Dloadcreate_shmim: ============== %ld  ======\n", ID);
        fflush(stdout);


        if(ID!=-1) // stream exists
        {

            sizeOK = COREMOD_MEMORY_check_3Dsize(name, xsize, ysize, zsize);
            if(sizeOK==0)
            {
                char command[500];

                //               printf("\n========== EXISTING %s HAS WRONG SIZE -> CREATING BLANK %s ===========\n\n", name, name);
                printf("        AOloopControl_3Dloadcreate_shmim: ===== EXISTING %s HAS WRONG SIZE -> CREATING BLANK %s\n", name, name);
                fflush(stdout);

                delete_image_ID(name);

                if(sprintf(command, "rm /tmp/%s.im.shm", name) < 1)
                    printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");

                if(system(command) != 0)
                    printERROR(__FILE__, __func__, __LINE__, "system() returns non-zero value");

                CreateSMim = 1;
                loadcreatestatus = 0;
            }
            else // SIZE OK
            {
                printf("        AOloopControl_3Dloadcreate_shmim: ===== SIZE OK\n");
                fflush(stdout);
                CreateSMim = 0;
                loadcreatestatus = 2;
            }
        }
        else
        {
            CreateSMim = 1;
            loadcreatestatus = 1;
        }

        if(CreateSMim == 1)
        {
            sizearray[0] = xsize;
            sizearray[1] = ysize;
            sizearray[2] = zsize;
            if(xsize*ysize*zsize>0)
            {
                printf("        AOloopControl_3Dloadcreate_shmim: ===== create_image_ID\n");
                fflush(stdout);
                ID = create_image_ID(name, 3, sizearray, _DATATYPE_FLOAT, 1, 0);
                creashmimfromFITS = 0;
            }
            else
                creashmimfromFITS = 1;
        }
    }
    free(sizearray);


    printf("        AOloopControl_3Dloadcreate_shmim: ===== TEST pt\n");
    fflush(stdout);

    // here, ID is either loaded, or it should be created from FITS image
    if((ID==-1)&&(creashmimfromFITS==0))
    {
        printf("ERROR: could not load/create %s\n", name);
        exit(0);
    }

    ID1 = load_fits(fname, "tmp3Dim", 3);
    printf("        AOloopControl_3Dloadcreate_shmim: ===== ID1 = %ld\n", ID1);
    fflush(stdout);
    if(ID1!=-1)
    {
        if(creashmimfromFITS == 1) // create shared mem from FITS
        {
            sizeOK = COREMOD_MEMORY_check_3Dsize("tmp3Dim", xsize, ysize, zsize);
            printf("        AOloopControl_3Dloadcreate_shmim: ===== sizeOK = %d\n", (int) sizeOK);
            fflush(stdout);
            if(sizeOK==1)
            {
                long xsize1, ysize1, zsize1;

                xsize1 = data.image[ID1].md[0].size[0];
                ysize1 = data.image[ID1].md[0].size[1];
                zsize1 = data.image[ID1].md[0].size[2];
                sizearray[0] = xsize1;
                sizearray[1] = ysize1;
                sizearray[2] = zsize1;
                ID = create_image_ID(name, 3, sizearray, _DATATYPE_FLOAT, 1, 0);

                printf("        AOloopControl_3Dloadcreate_shmim: ===== [1] memcpy  %ld %ld %ld\n", xsize1, ysize1, zsize1);
                fflush(stdout);

                memcpy(data.image[ID].array.F, data.image[ID1].array.F, sizeof(float)*xsize1*ysize1*zsize1);

                printf("        AOloopControl_3Dloadcreate_shmim: ===== [1] memcpy  DONE\n");
                fflush(stdout);

                loadcreatestatus = 1;
            }
            else
            {
                printf("File \"%s\" has wrong size (should be 3-D %ld x %ld, x %ld  is %d-D %ld x %ld x %ld): ignoring\n", fname, xsize, ysize, zsize, (int) data.image[ID1].md[0].naxis, (long) data.image[ID1].md[0].size[0], (long) data.image[ID1].md[0].size[1], (long) data.image[ID1].md[0].size[2]);
                loadcreatestatus = 3;
            }
        }
        else
        {
            printf("        AOloopControl_3Dloadcreate_shmim: ===== [2] memcpy %ld <- %ld     %ld %ld %ld\n", ID, ID1, xsize, ysize, zsize);
            fflush(stdout);
            list_image_ID();

            memcpy(data.image[ID].array.F, data.image[ID1].array.F, sizeof(float)*xsize*ysize*zsize);

            printf("        AOloopControl_3Dloadcreate_shmim: ===== [2] memcpy  DONE\n");
            fflush(stdout);

            loadcreatestatus = 2;
        }
        delete_image_ID("tmp3Dim");
    }
    else
    {
        if(CreateSMim==0)
            loadcreatestatus = 4;
        else
            loadcreatestatus = 5;
    }


    if(loadcreateshm_log == 1) // results should be logged in ASCII file
    {
        switch ( loadcreatestatus ) {
        case 0 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: existing stream has wrong size -> recreating stream\n", fname, name);
            break;
        case 1 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: new stream created and content loaded\n", fname, name);
            break;
        case 2 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: existing stream updated\n", fname, name);
            break;
        case 3 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: FITS image has wrong size -> do nothing\n", fname, name);
            break;
        case 4 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: FITS image does not exist, stream exists -> do nothing\n", fname, name);
            break;
        case 5 :
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: FITS image does not exist, stream does not exist -> create empty stream\n", fname, name);
            break;
        default:
            fprintf(loadcreateshm_fplog, "LOADING FITS FILE %s TO STREAM %s: UNKNOWN ERROR CODE\n", fname, name);
            break;
        }
    }


    printf("-------- EXITING AOloopControl_3Dloadcreate_shmim ----------\n");
    fflush(stdout);


#ifdef AOLOOPCONTROL_LOGFUNC
	AOLOOPCONTROL_logfunc_level = 2;
    CORE_logFunctionCall( AOLOOPCONTROL_logfunc_level, AOLOOPCONTROL_logfunc_level_max, 1, __FUNCTION__, __LINE__, "");
#endif

    return ID;
}







/* =============================================================================================== */
/** @name AOloopControl - 2.2. LOW LEVEL UTILITIES & TOOLS - DATA STREAMS PROCESSING               */
/* =============================================================================================== */





/**
 * ## Purpose
 * 
 * Averages input image stream
 * 
 * ## Arguments
 * 
 * @param[in]
 * IDname	CHAR*
 * 			Input stream name
 * 
 * @param[in]
 * alpha	DOUBLE
 * 			Averaging coefficient
 * 			new average = old average * (1-alpha) + alpha * new image
 * 
 * @param[out]
 * fIDname_out_ave	CHAR*
 * 			Stream name for output average image
 * 
 * @param[in]
 * IDname_out_AC	CHAR*
 * 			Stream name for output AC component (average-subtracted)
 * 
 * @param[in]
 * IDname_out_RMS	CHAR*
 * 			Stream name for output RMS component
 * 
 * 
 * 
 */

int_fast8_t AOloopControl_AveStream(const char *IDname, double alpha, const char *IDname_out_ave, const char *IDname_out_AC, const char *IDname_out_RMS)
{
    long IDin;
    long IDout_ave;
    long IDout_AC, IDout_RMS;
    long xsize, ysize;
    uint32_t *sizearray;
    long cnt0old = 0;
    long delayus = 10;




    IDin = image_ID(IDname);
    xsize = data.image[IDin].md[0].size[0];
    ysize = data.image[IDin].md[0].size[1];

    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*2);
    sizearray[0] = xsize;
    sizearray[1] = ysize;

    IDout_ave = create_image_ID(IDname_out_ave, 2, sizearray, _DATATYPE_FLOAT, 1, 0);
    COREMOD_MEMORY_image_set_createsem(IDname_out_ave, 10);

    IDout_AC = create_image_ID(IDname_out_AC, 2, sizearray, _DATATYPE_FLOAT, 1, 0);
    COREMOD_MEMORY_image_set_createsem(IDname_out_ave, 10);

    IDout_RMS = create_image_ID(IDname_out_RMS, 2, sizearray, _DATATYPE_FLOAT, 1, 0);
    COREMOD_MEMORY_image_set_createsem(IDname_out_RMS, 10);


    free(sizearray);

    for(;;)
    {
        if(data.image[IDin].md[0].cnt0 != cnt0old)
        {
            data.image[IDout_ave].md[0].write = 1;
            data.image[IDout_AC].md[0].write = 1;
            data.image[IDout_RMS].md[0].write = 1;
            uint_fast64_t ii;
            for(ii=0; ii<xsize*ysize; ii++)
            {
                data.image[IDout_ave].array.F[ii] = (1.0-alpha)*data.image[IDout_ave].array.F[ii] + alpha * data.image[IDin].array.F[ii];
                data.image[IDout_RMS].array.F[ii] = (1.0-alpha)*data.image[IDout_RMS].array.F[ii] + alpha * (data.image[IDin].array.F[ii] - data.image[IDout_ave].array.F[ii]) * (data.image[IDin].array.F[ii] - data.image[IDout_ave].array.F[ii]);
                data.image[IDout_AC].array.F[ii] = data.image[IDin].array.F[ii] - data.image[IDout_ave].array.F[ii];
            }
            data.image[IDout_ave].md[0].cnt0++;
            data.image[IDout_AC].md[0].cnt0++;
            data.image[IDout_RMS].md[0].cnt0++;
            data.image[IDout_ave].md[0].write = 0;
            data.image[IDout_AC].md[0].write = 0;
            data.image[IDout_RMS].md[0].write = 0;
            cnt0old = data.image[IDin].md[0].cnt0;
        }
        usleep(delayus);
    }


    return(0);
}



long AOloopControl_frameDelay(const char *IDin_name, const char *IDkern_name, const char *IDout_name, int insem)
{
    long IDout;
    long IDin;
    long IDkern;
    long ksize;
    long IDbuff;
    long xsize, ysize;
    long kindex = 0;
    long cnt;
    long framesize;
    long IDtmp;
    long xysize;
    float eps=1.0e-8;
    long ii, jj, kk, k1;
    uint32_t *sizearray;



    IDin = image_ID(IDin_name);
    xsize = data.image[IDin].md[0].size[0];
    ysize = data.image[IDin].md[0].size[1];
    IDtmp = create_2Dimage_ID("_tmpfr", xsize, ysize);
    xysize = xsize*ysize;

    printf("xsize = %ld\n", xsize);
    printf("ysize = %ld\n", ysize);
    fflush(stdout);





    IDkern = image_ID(IDkern_name);
    ksize = data.image[IDkern].md[0].size[0];
    printf("ksize = %ld\n", ksize);
    fflush(stdout);


    IDbuff = create_3Dimage_ID("_tmpbuff", xsize, ysize, ksize);

    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*2);
    sizearray[0] = xsize;
    sizearray[1] = ysize;
    IDout = create_image_ID(IDout_name, 2, sizearray, _DATATYPE_FLOAT, 1, 0);
    COREMOD_MEMORY_image_set_createsem(IDout_name, 10);
    free(sizearray);


    framesize = sizeof(float)*xsize*ysize;


    kindex = 0;
    cnt = 0;

    for(;;)
    {
        if(data.image[IDin].md[0].sem==0)
        {
            while(cnt==data.image[IDin].md[0].cnt0) // test if new frame exists
                usleep(5);
            cnt = data.image[IDin].md[0].cnt0;
        }
        else
            sem_wait(data.image[IDin].semptr[insem]);

        char *ptr0;

        ptr0 = (char*) data.image[IDbuff].array.F;
        ptr0 += kindex*framesize;

        data.image[IDbuff].md[0].write = 1;
        memcpy((void*) ptr0, data.image[IDin].array.F, framesize);
        data.image[IDbuff].md[0].cnt0++;
        data.image[IDbuff].md[0].write = 0;


        data.image[IDout].md[0].write = 1;

        for(ii=0; ii<xysize; ii++)
            data.image[IDtmp].array.F[ii] = 0.0;
        for(kk=0; kk<ksize; kk++)
        {
            if(fabs(data.image[IDkern].array.F[kk])>eps)
            {
                k1 = kindex-kk;
                if(k1<0)
                    k1 += ksize;

                for(ii=0; ii<xysize; ii++)
                    data.image[IDtmp].array.F[ii] += data.image[IDkern].array.F[kk] * data.image[IDbuff].array.F[k1*xysize + ii];
            }
        }
        memcpy(data.image[IDout].array.F, data.image[IDtmp].array.F, framesize);
        COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;


        kindex++;
        if(kindex == ksize)
            kindex = 0;
    }


    return IDout;
}




long AOloopControl_stream3Dto2D(const char *in_name, const char *out_name, int NBcols, int insem)
{
    long IDin, IDout;
    uint_fast16_t xsize0, ysize0, zsize0;
    uint_fast32_t xysize0;
    uint_fast16_t xsize1, ysize1;
    uint_fast16_t ii0, jj0, kk0, ii1, jj1, kk;
    uint_fast16_t Xindex, Yindex;
    uint_fast16_t iioffset, jjoffset;
    long long cnt;
    uint32_t *sizearray;
    uint8_t atype;
    char out0name[200]; // noise-free image, in contrast
    long IDout0;

    float ContrastCoeff = 0.0379;
    float Flux = 5.22e8; // flux per spectral channel [NBph]
    // spectral channel 1% broad = 0.00638 um
    // mR = 5
    // 6m radius disk (12m diam)





    Flux *= 0.4; // efficiency
    Flux *= 3600.0; // second -> hr



    IDin = image_ID(in_name);
    xsize0 = data.image[IDin].md[0].size[0];
    ysize0 = data.image[IDin].md[0].size[1];
    zsize0 = data.image[IDin].md[0].size[2];
    xysize0 = xsize0*ysize0;

    xsize1 = xsize0*NBcols;
    ysize1 = ysize0*(1 + (long) (1.0*zsize0/NBcols-0.00001));

    if(sprintf(out0name, "%sc", out_name) < 1)
        printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");

    atype = _DATATYPE_FLOAT;
    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*2);
    sizearray[0] = xsize1;
    sizearray[1] = ysize1;
    IDout = create_image_ID(out_name, 2, sizearray, atype, 1, 0);
    IDout0 = create_image_ID(out0name, 2, sizearray, atype, 1, 0);
    free(sizearray);

    for(;;)
    {
        if(data.image[IDin].md[0].sem==0)
        {
            while(cnt==data.image[IDin].md[0].cnt0) // test if new frame exists
                usleep(5);
            cnt = data.image[IDin].md[0].cnt0;
        }
        else
            sem_wait(data.image[IDin].semptr[insem]);


        printf("Updating image %s ...", out_name);
        fflush(stdout);

        data.image[IDout].md[0].write = 1;

        for(kk0=0; kk0<zsize0; kk0++)
        {
            kk = 0;
            Xindex = 0;
            Yindex = 0;
            while(kk<kk0)
            {
                Xindex++;
                if(Xindex==NBcols)
                {
                    Xindex = 0;
                    Yindex++;
                }
                kk++;
            }
            iioffset = Xindex * xsize0;
            jjoffset = Yindex * ysize0;

            for(ii0=0; ii0<xsize0; ii0++)
                for(jj0=0; jj0<ysize0; jj0++)
                {
                    ii1 = ii0+iioffset;
                    jj1 = jj0+jjoffset;
                    //data.image[IDout].array.F[jj1*xsize1+ii1] = data.image[IDin].array.F[kk0*xysize0+jj0*xsize0+ii0]/ContrastCoeff;
                    data.image[IDout].array.F[jj1*xsize1+ii1] = poisson(data.image[IDin].array.F[kk0*xysize0+jj0*xsize0+ii0]*Flux)/Flux/ContrastCoeff;

                    data.image[IDout0].array.F[jj1*xsize1+ii1] = data.image[IDin].array.F[kk0*xysize0+jj0*xsize0+ii0]/ContrastCoeff;
                }
        }
        COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;

        printf("done\n");
        fflush(stdout);
    }

    return(IDout);
}


