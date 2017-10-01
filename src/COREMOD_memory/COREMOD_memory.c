/**
 * @file    COREMOD_memory.c
 * @brief   cfitsTK memory functions
 * 
 * Functions to handle images and streams
 *  
 * @author  O. Guyon
 * @date    9 Sept 2017
 *
 * 
 * @bug No known bugs.
 * 
 */



#define _GNU_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <signal.h> 
#include <ncurses.h>

#include <errno.h>

#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close






#ifdef __MACH__
#include <mach/mach_time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
static int clock_gettime(int clk_id, struct mach_timespec *t){
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

#include "ImageStruct.h"
#include "ImageStreamIO/ImageStreamIO.h"
#include "CLIcore.h"
#include "info/info.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"

 
# ifdef _OPENMP
# include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
# endif

#define STYPESIZE 10
#define SBUFFERSIZE 1000




#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)





static pthread_t *thrarray_semwait;
static long NB_thrarray_semwait;


// MEMORY MONITOR 
static FILE *listim_scr_fpo;
static FILE *listim_scr_fpi;
static SCREEN *listim_scr; // for memory monitoring
static int MEM_MONITOR = 0; // 1 if memory monitor is on
static int listim_scr_wrow;
static int listim_scr_wcol;


extern DATA data;


static char errmsg_memory[SBUFFERSIZE];



/** data logging of shared memory image stream
 *
 */

struct savethreadmsg {
    char iname[100];
    char fname[200];
    int partial; // 1 if partial cube
    long cubesize; // size of the cube
	
	char fnameascii[200];
	uint64_t *arraycnt0;
	uint64_t *arraycnt1;
	double *arraytime;
};

static long tret; // thread return value







// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 1. MANAGE MEMORY AND IDENTIFIERS                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */


int_fast8_t delete_image_ID_cli()
{
    long i = 1;
    printf("%ld : %d\n", i, data.cmdargtoken[i].type);
    while(data.cmdargtoken[i].type != 0)
    {
        if(data.cmdargtoken[i].type == 4)
            delete_image_ID(data.cmdargtoken[i].val.string);
        else
            printf("Image %s does not exist\n", data.cmdargtoken[i].val.string);
        i++;
    }

    return 0;
}





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 2. KEYWORDS                                                                                     */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */


int_fast8_t image_write_keyword_L_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,3)==0)
        image_write_keyword_L(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string);
    else
        return 1;
}




int_fast8_t image_list_keywords_cli()
{
    if(CLI_checkarg(1,4)==0)
        image_list_keywords(data.cmdargtoken[1].val.string);
    else
        return 1;
}






/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 3. READ SHARED MEM IMAGE AND SIZE                                                               */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



int_fast8_t read_sharedmem_image_size_cli()
{
    if(CLI_checkarg(1,5)+CLI_checkarg(2,3)==0)
        read_sharedmem_image_size(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
    else
        return 1;
}




int_fast8_t read_sharedmem_image_cli()
{
    if(CLI_checkarg(1,3)==0)
        read_sharedmem_image(data.cmdargtoken[1].val.string);
    else
        return 1;
}





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 4. CREATE IMAGE                                                                                 */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



int_fast8_t create_image_cli()
{
    uint32_t *imsize;
    long naxis = 0;
    long i;
    uint8_t atype;



    if(CLI_checkarg(1,3)+CLI_checkarg_noerrmsg(2,2)==0)
    {
        naxis = 0;
        imsize = (uint32_t*) malloc(sizeof(uint32_t)*5);
        i = 2;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        switch(data.precision) {
        case 0:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);
            break;
        case 1:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT);
            break;
        }
        free(imsize);
    }
    else if (CLI_checkarg(1,3)+CLI_checkarg(2,3)+CLI_checkarg(3,2)==0) // type option exists
    {
        atype = 0;

        if(strcmp(data.cmdargtoken[2].val.string, "c")==0)
        {
            printf("type = CHAR\n");
            atype = _DATATYPE_UINT8;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "i")==0)
        {
            printf("type = INT\n");
            atype = _DATATYPE_INT32;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "f")==0)
        {
            printf("type = FLOAT\n");
            atype = _DATATYPE_FLOAT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "d")==0)
        {
            printf("type = DOUBLE\n");
            atype = _DATATYPE_DOUBLE;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "cf")==0)
        {
            printf("type = COMPLEX_FLOAT\n");
            atype = _DATATYPE_COMPLEX_FLOAT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "cd")==0)
        {
            printf("type = COMPLEX_DOUBLE\n");
            atype = _DATATYPE_COMPLEX_DOUBLE;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "u")==0)
        {
            printf("type = USHORT\n");
            atype = _DATATYPE_UINT16;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "l")==0)
        {
            printf("type = LONG\n");
            atype = _DATATYPE_INT64;
        }

        if(atype == 0)
        {
            printf("Data type \"%s\" not recognized\n", data.cmdargtoken[2].val.string);
            printf("must be : \n");
            printf("  c : CHAR\n");
            printf("  i : INT32\n");
            printf("  f : FLOAT\n");
            printf("  d : DOUBLE\n");
            printf("  cf: COMPLEX FLOAT\n");
            printf("  cd: COMPLEX DOUBLE\n");
            printf("  u : USHORT16\n");
            printf("  l : LONG64\n");
            return 1;
        }
        naxis = 0;
        imsize = (uint32_t*) malloc(sizeof(uint32_t)*5);
        i = 3;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }

        create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, atype, data.SHARED_DFT, data.NBKEWORD_DFT);

        free(imsize);
    }
    else
        return 1;
}




int_fast8_t create_image_shared_cli() // default precision
{
    uint32_t *imsize;
    long naxis = 0;
    long i;


    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        naxis = 0;
        imsize = (uint32_t*) malloc(sizeof(uint32_t)*5);
        i = 2;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        switch(data.precision) {
        case 0:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, _DATATYPE_FLOAT, 1, data.NBKEWORD_DFT);
            break;
        case 1:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, _DATATYPE_DOUBLE, 1, data.NBKEWORD_DFT);
            break;
        }
        free(imsize);
        printf("Creating 10 semaphores\n");
        COREMOD_MEMORY_image_set_createsem(data.cmdargtoken[1].val.string, 10);
    }
    else
        return 1;
}




int_fast8_t create_ushort_image_shared_cli() // default precision
{
    uint32_t *imsize;
    long naxis = 0;
    long i;


    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        naxis = 0;
        imsize = (uint32_t*) malloc(sizeof(uint32_t)*5);
        i = 2;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, _DATATYPE_UINT16, 1, data.NBKEWORD_DFT);

        free(imsize);
    }
    else
        return 1;
}




int_fast8_t create_2Dimage_float()
{
  uint32_t *imsize;

  // CHECK ARGS
//  printf("CREATING IMAGE\n");
  imsize = (uint32_t*) malloc(sizeof(uint32_t)*2);

  imsize[0] = data.cmdargtoken[2].val.numl;
  imsize[1] = data.cmdargtoken[3].val.numl;

  create_image_ID(data.cmdargtoken[1].val.string, 2, imsize, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);

  free(imsize);

  return 0;
}



int_fast8_t create_3Dimage_float()
{
  uint32_t *imsize;

  // CHECK ARGS
//  printf("CREATING 3D IMAGE\n");
  imsize = (uint32_t *) malloc(sizeof(uint32_t)*3);

  imsize[0] = data.cmdargtoken[2].val.numl;
  imsize[1] = data.cmdargtoken[3].val.numl;
  imsize[2] = data.cmdargtoken[4].val.numl;

  create_image_ID(data.cmdargtoken[1].val.string, 3, imsize, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);

  free(imsize);

  return 0;
}












/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 5. CREATE VARIABLE                                                                              */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 6. COPY IMAGE                                                                                   */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



int_fast8_t copy_image_ID_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  
  copy_image_ID(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, 0);

  return 0;
}




int_fast8_t copy_image_ID_sharedmem_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  
  copy_image_ID(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, 1);

  return 0;
}


int_fast8_t chname_image_ID_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  
  chname_image_ID(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);

  return 0;
}



int_fast8_t COREMOD_MEMORY_cp2shm_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)==0)
    {
        COREMOD_MEMORY_cp2shm(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
        return 0;
    }
    else
        return 1;
}




/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 7. DISPLAY / LISTS                                                                              */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



int_fast8_t memory_monitor_cli()
{
  memory_monitor(data.cmdargtoken[1].val.string);
  return 0;
}


int_fast8_t list_variable_ID_file_cli()
{
 if(CLI_checkarg(1,3)==0)
    list_variable_ID_file(data.cmdargtoken[1].val.string);
  else
    return 1;
}



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 8. TYPE CONVERSIONS TO AND FROM COMPLEX                                                         */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



int_fast8_t mk_complex_from_reim_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  if(data.cmdargtoken[2].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[2].val.string);
      return -1;
    }

  mk_complex_from_reim(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, 0);

  return 0;
}


int_fast8_t mk_complex_from_amph_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  if(data.cmdargtoken[2].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[2].val.string);
      return -1;
    }

  mk_complex_from_amph(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, 0);

  return 0;
}


int_fast8_t mk_reim_from_complex_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }

  mk_reim_from_complex(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, 0);

  return 0;
}

int_fast8_t mk_amph_from_complex_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }

  mk_amph_from_complex(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, 0);

  return 0;
}




/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 9. VERIFY SIZE                                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 10. COORDINATE CHANGE                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 11. SET IMAGE FLAGS / COUNTERS                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */


int_fast8_t COREMOD_MEMORY_image_set_status_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_status(data.cmdargtoken[1].val.string, (int) data.cmdargtoken[2].val.numl);
    else
        return 1;
}


int_fast8_t COREMOD_MEMORY_image_set_cnt0_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_cnt0(data.cmdargtoken[1].val.string, (int) data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int_fast8_t COREMOD_MEMORY_image_set_cnt1_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_cnt1(data.cmdargtoken[1].val.string, (int) data.cmdargtoken[2].val.numl);
    else
        return 1;
}





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 12. MANAGE SEMAPHORES                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




int_fast8_t COREMOD_MEMORY_image_set_createsem_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_createsem(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int_fast8_t COREMOD_MEMORY_image_set_sempost_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_sempost(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int_fast8_t COREMOD_MEMORY_image_set_sempost_loop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,2)==0)
        COREMOD_MEMORY_image_set_sempost_loop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl);
    else
        return 1;
}



int_fast8_t COREMOD_MEMORY_image_set_semwait_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_semwait(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int_fast8_t COREMOD_MEMORY_image_set_semflush_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_semflush(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}




/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 13. SIMPLE OPERATIONS ON STREAMS                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */





int_fast8_t COREMOD_MEMORY_streamDiff_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,5)+CLI_checkarg(4,3)+CLI_checkarg(5,2)==0)
    {
        COREMOD_MEMORY_streamDiff(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.numl);
        return 0;
    }
    else
        return 1;
}


int_fast8_t COREMOD_MEMORY_stream_halfimDiff_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,2)==0)
    {
        COREMOD_MEMORY_stream_halfimDiff(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl);
        return 0;
    }
    else
        return 1;
}



// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string



int_fast8_t COREMOD_MEMORY_streamAve_cli()
{
	if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,5)==0)
    {
        COREMOD_MEMORY_streamAve(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string);
        return 0;
    }
    else
        return 1;
}




int_fast8_t COREMOD_MEMORY_image_streamupdateloop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,5)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,2)+CLI_checkarg(6,2)+CLI_checkarg(7,5)+CLI_checkarg(8,2)+CLI_checkarg(9,2)==0)
    {
        COREMOD_MEMORY_image_streamupdateloop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numl, data.cmdargtoken[7].val.string, data.cmdargtoken[8].val.numl, data.cmdargtoken[9].val.numl);
        return 0;
    }
    else
        return 1;
}



int_fast8_t COREMOD_MEMORY_image_streamupdateloop_semtrig_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,5)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,5)+CLI_checkarg(6,2)+CLI_checkarg(7,2)==0)
    {
        COREMOD_MEMORY_image_streamupdateloop_semtrig(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.string, data.cmdargtoken[6].val.numl, data.cmdargtoken[7].val.numl);
        return 0;
    }
    else
        return 1;
}



int_fast8_t COREMOD_MEMORY_streamDelay_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,5)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)
    {
        COREMOD_MEMORY_streamDelay(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
        return 0;
    }
    else
        return 1;
}






int_fast8_t COREMOD_MEMORY_SaveAll_snapshot_cli()
{
	 if(CLI_checkarg(1,5)==0)
    {
        COREMOD_MEMORY_SaveAll_snapshot(data.cmdargtoken[1].val.string);
        return 0;
    }
    else
        return 1;
}


int_fast8_t COREMOD_MEMORY_SaveAll_sequ_cli()
{
	 if(CLI_checkarg(1,5)+CLI_checkarg(2,4)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)
    {
        COREMOD_MEMORY_SaveAll_sequ(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
        return 0;
    }
    else
        return 1;
}



int_fast8_t COREMOD_MEMORY_image_NETWORKtransmit_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,2)==0)
    {
        COREMOD_MEMORY_image_NETWORKtransmit(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl);
        return 0;
    }
    else
        return 1;
}


int_fast8_t COREMOD_MEMORY_image_NETWORKreceive_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,2)+CLI_checkarg(3,2)==0)
    {
        COREMOD_MEMORY_image_NETWORKreceive(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl);
        return 0;
    }
    else
        return 1;
}



int_fast8_t COREMOD_MEMORY_PixMapDecode_U_cli()
{
     if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,3)+CLI_checkarg(5,4)+CLI_checkarg(6,3)+CLI_checkarg(7,3)==0)
    {
        COREMOD_MEMORY_PixMapDecode_U(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.string, data.cmdargtoken[6].val.string, data.cmdargtoken[7].val.string);
        return 0;
    }
    else
        return 1;
}






/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 14. DATA LOGGING                                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




int_fast8_t COREMOD_MEMORY_logshim_printstatus_cli()
{
    if(CLI_checkarg(1,3)==0)
    {
        COREMOD_MEMORY_logshim_printstatus(data.cmdargtoken[1].val.string);
        return 0;
    }
    else
        return 1;
}


int_fast8_t COREMOD_MEMORY_logshim_set_on_cli()
{
    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        printf("logshim_set_on ----------------------\n");
        COREMOD_MEMORY_logshim_set_on(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
        return 0;
    }
    else
        return 1;
}

int_fast8_t COREMOD_MEMORY_logshim_set_logexit_cli()
{
    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        COREMOD_MEMORY_logshim_set_logexit(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
        return 0;
    }
    else
        return 1;
}



int_fast8_t COREMOD_MEMORY_sharedMem_2Dim_log_cli()
{

    if(CLI_checkarg_noerrmsg(4,3)!=0)
		sprintf(data.cmdargtoken[4].val.string, "null");
		
    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,3)==0)
    {
        COREMOD_MEMORY_sharedMem_2Dim_log(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string);
        return 0;
    }
    else
        return 1;
}












int_fast8_t init_COREMOD_memory()
{
    strcpy(data.module[data.NBmodule].name,__FILE__);
    strcpy(data.module[data.NBmodule].info,"memory management for images and variables");
    data.NBmodule++;



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 1. MANAGE MEMORY AND IDENTIFIERS                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("mmon", __FILE__, memory_monitor_cli, "Monitor memory content", "terminal tty name", "mmon /dev/pts/4", "int memory_monitor(const char *ttyname)");
    
    RegisterCLIcommand("rm", __FILE__, delete_image_ID_cli, "remove image(s)", "list of images", "rm im1 im4", "int delete_image_ID(char* imname)");
     
    RegisterCLIcommand("rmall", __FILE__, clearall, "remove all images", "no argument", "rmall", "int clearall()");
    


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 2. KEYWORDS                                                                                     */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

   RegisterCLIcommand("imwritekwL", __FILE__, image_write_keyword_L_cli, "write long type keyword", "<imname> <kname> <value [long]> <comment>", "imwritekwL im1 kw2 34 my_keyword_comment", "long image_write_keyword_L(const char *IDname, const char *kname, long value, const char *comment)");
    
    RegisterCLIcommand("imlistkw", __FILE__, image_list_keywords_cli, "list image keywords", "<imname>", "imlistkw im1", "long image_list_keywords(const char *IDname)");
  

/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 3. READ SHARED MEM IMAGE AND SIZE                                                               */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("readshmimsize", __FILE__, read_sharedmem_image_size_cli, "read shared memory image size", "<name> <output file>", "readshmimsize im1 imsize.txt", "read_sharedmem_image_size(const char *name, const char *fname)");
    
    RegisterCLIcommand("readshmim", __FILE__, read_sharedmem_image_cli, "read shared memory image", "<name>", "readshmim im1", "read_sharedmem_image(const char *name)");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 4. CREATE IMAGE                                                                                 */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 5. CREATE VARIABLE                                                                              */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("creaim", __FILE__, create_image_cli, "create image, default precision", "<name> <xsize> <ysize> <opt: zsize>", "creaim imname 512 512", "long create_image_ID(const char *name, long naxis, uint32_t *size, uint8_t atype, 0, 10)");
   
    RegisterCLIcommand("creaimshm", __FILE__, create_image_shared_cli, "create image in shared mem, default precision", "<name> <xsize> <ysize> <opt: zsize>", "creaimshm imname 512 512", "long create_image_ID(const char *name, long naxis, uint32_t *size, uint8_t atype, 0, 10)");
    
    RegisterCLIcommand("creaushortimshm", __FILE__, create_ushort_image_shared_cli, "create unsigned short image in shared mem", "<name> <xsize> <ysize> <opt: zsize>", "creaushortimshm imname 512 512", "long create_image_ID(const char *name, long naxis, long *size, _DATATYPE_UINT16, 0, 10)");
    
    RegisterCLIcommand("crea3dim", __FILE__, create_3Dimage_float, "creates 3D image, single precision", "<name> <xsize> <ysize> <zsize>", "crea3dim imname 512 512 100", "long create_image_ID(const char *name, long naxis, long *size, _DATATYPE_FLOAT, 0, 10)");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 6. COPY IMAGE                                                                                   */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("cp", __FILE__, copy_image_ID_cli, "copy image", "source, dest", "cp im1 im4", "long copy_image_ID(const char *name, const char *newname, 0)");
    
    RegisterCLIcommand("cpsh", __FILE__, copy_image_ID_sharedmem_cli, "copy image - create in shared mem if does not exist", "source, dest", "cp im1 im4", "long copy_image_ID(const char *name, const char *newname, 1)");
    
    RegisterCLIcommand("mv", __FILE__, chname_image_ID_cli, "change image name", "source, dest", "mv im1 im4", "long chname_image_ID(const char *name, const char *newname)");
    
     RegisterCLIcommand("imcp2shm", __FILE__, COREMOD_MEMORY_cp2shm_cli, "copy image ot shared memory", "<image> <shared mem image>", "imcp2shm im1 ims1", "long COREMOD_MEMORY_cp2shm(const char *IDname, const char *IDshmname)");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 7. DISPLAY / LISTS                                                                              */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

	RegisterCLIcommand("listim",__FILE__, list_image_ID, "list images in memory", "no argument", "listim", "int_fast8_t list_image_ID()");

    RegisterCLIcommand("listvar", __FILE__, list_variable_ID, "list variables in memory", "no argument", "listvar", "int list_variable_ID()");
    
    RegisterCLIcommand("listvarf", __FILE__, list_variable_ID_file_cli, "list variables in memory, write to file", "<file name>", "listvarf var.txt", "int list_variable_ID_file()");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 8. TYPE CONVERSIONS TO AND FROM COMPLEX                                                         */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

   RegisterCLIcommand("ri2c", __FILE__, mk_complex_from_reim_cli, "real, imaginary -> complex", "real imaginary complex", "ri2c imr imi imc", "int mk_complex_from_reim(const char *re_name, const char *im_name, const char *out_name)");

    RegisterCLIcommand("ap2c", __FILE__, mk_complex_from_amph_cli, "ampl, pha -> complex", "ampl pha complex", "ap2c ima imp imc", "int mk_complex_from_amph(const char *re_name, const char *im_name, const char *out_name, int sharedmem)");
    
    RegisterCLIcommand("c2ri", __FILE__, mk_reim_from_complex_cli, "complex -> real, imaginary", "complex real imaginary", "c2ri imc imr imi", "int mk_reim_from_complex(const char *re_name, const char *im_name, const char *out_name)");
    
    RegisterCLIcommand("c2ap", __FILE__, mk_amph_from_complex_cli, "complex -> ampl, pha", "complex ampl pha", "c2ap imc ima imp", "int mk_amph_from_complex(const char *re_name, const char *im_name, const char *out_name, int sharedmem)");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 9. VERIFY SIZE                                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 10. COORDINATE CHANGE                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 11. SET IMAGE FLAGS / COUNTERS                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("imsetstatus", __FILE__, COREMOD_MEMORY_image_set_status_cli, "set image status variable", "<image> <value [long]>", "imsetstatus im1 2", "long COREMOD_MEMORY_image_set_status(const char *IDname, int status)");  

    RegisterCLIcommand("imsetcnt0", __FILE__, COREMOD_MEMORY_image_set_cnt0_cli, "set image cnt0 variable", "<image> <value [long]>", "imsetcnt0 im1 2", "long COREMOD_MEMORY_image_set_cnt0(const char *IDname, int status)");
    
    RegisterCLIcommand("imsetcnt1", __FILE__, COREMOD_MEMORY_image_set_cnt1_cli, "set image cnt1 variable", "<image> <value [long]>", "imsetcnt1 im1 2", "long COREMOD_MEMORY_image_set_cnt1(const char *IDname, int status)");
 


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 12. MANAGE SEMAPHORES                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("imsetcreatesem", __FILE__, COREMOD_MEMORY_image_set_createsem_cli, "create image semaphore", "<image> <NBsem>", "imsetcreatesem im1 5", "long COREMOD_MEMORY_image_set_createsem(const char *IDname, long NBsem)");    

    RegisterCLIcommand("imsetsempost", __FILE__, COREMOD_MEMORY_image_set_sempost_cli, "post image semaphore. If sem index = -1, post all semaphores", "<image> <sem index>", "imsetsempost im1 2", "long COREMOD_MEMORY_image_set_sempost(const char *IDname, long index)");  

    RegisterCLIcommand("imsetsempostl", __FILE__, COREMOD_MEMORY_image_set_sempost_loop_cli, "post image semaphore loop. If sem index = -1, post all semaphores", "<image> <sem index> <time interval [us]>", "imsetsempost im1 2", "long COREMOD_MEMORY_image_set_sempost(const char *IDname, long index, long dtus)");
    
    RegisterCLIcommand("imsetsemwait", __FILE__, COREMOD_MEMORY_image_set_semwait_cli, "wait image semaphore", "<image>", "imsetsemwait im1", "long COREMOD_MEMORY_image_set_semwait(const char *IDname)");   

    RegisterCLIcommand("imsetsemflush", __FILE__, COREMOD_MEMORY_image_set_semflush_cli, "flush image semaphore", "<image> <sem index>", "imsetsemflush im1 0", "long COREMOD_MEMORY_image_set_semflush(const char *IDname, long index)");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 13. SIMPLE OPERATIONS ON STREAMS                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */

    RegisterCLIcommand("creaimstream", __FILE__ , COREMOD_MEMORY_image_streamupdateloop_cli, "create 2D image stream from 3D cube", "<image3d in> <image2d out> <interval [us]> <NBcubes> <period> <offsetus> <sync stream name> <semtrig> <timing mode>", "creaimstream imcube imstream 1000 3 3 154 ircam1 3 0", "long COREMOD_MEMORY_image_streamupdateloop(const char *IDinname, const char *IDoutname, long usperiod, long NBcubes, long period, long offsetus, const char *IDsync_name, int semtrig, int timingmode)");
    
    RegisterCLIcommand("creaimstreamstrig", __FILE__, COREMOD_MEMORY_image_streamupdateloop_semtrig_cli, "create 2D image stream from 3D cube, use other stream to synchronize", "<image3d in> <image2d out> <period [int]> <delay [us]> <sync stream> <sync sem index> <timing mode>", "creaimstreamstrig imcube outstream 3 152 streamsync 3 0", "long COREMOD_MEMORY_image_streamupdateloop_semtrig(const char *IDinname, const char *IDoutname, long period, long offsetus, const char *IDsync_name, int semtrig, int timingmode)"); 

    RegisterCLIcommand("streamdelay", __FILE__, COREMOD_MEMORY_streamDelay_cli, "delay 2D image stream", "<image2d in> <image2d out> <delay [us]> <resolution [us]>", "streamdelay instream outstream 1000 10", "long COREMOD_MEMORY_streamDelay(const char *IDin_name, const char *IDout_name, long delayus, long dtus)");

    RegisterCLIcommand("imsaveallsnap", __FILE__, COREMOD_MEMORY_SaveAll_snapshot_cli, "save all images in directory", "<directory>", "imsaveallsnap dir1", "long COREMOD_MEMORY_SaveAll_snapshot(const char *dirname)");
    
    RegisterCLIcommand("imsaveallseq", __FILE__, COREMOD_MEMORY_SaveAll_sequ_cli, "save all images in directory - sequence", "<directory> <trigger image name> <trigger semaphore> <NB frames>", "imsaveallsequ dir1 im1 3 20", "long COREMOD_MEMORY_SaveAll_sequ(const char *dirname, const char *IDtrig_name, long semtrig, long NBframes)");
    
    RegisterCLIcommand("imnetwtransmit", __FILE__, COREMOD_MEMORY_image_NETWORKtransmit_cli, "transmit image over network", "<image> <IP addr> <port [long]> <sync mode [int]>", "imnetwtransmit im1 127.0.0.1 0 8888 0", "long COREMOD_MEMORY_image_NETWORKtransmit(const char *IDname, const char *IPaddr, int port, int mode)");
    
    RegisterCLIcommand("imnetwreceive", __FILE__, COREMOD_MEMORY_image_NETWORKreceive_cli, "receive image(s) over network. mode=1 uses counter instead of semaphore", "<port [long]> <mode [int]> <RT priority>", "imnetwreceive 8887 0 80", "long COREMOD_MEMORY_image_NETWORKreceive(int port, int mode, int RT_priority)");
    
    RegisterCLIcommand("impixdecodeU", __FILE__, COREMOD_MEMORY_PixMapDecode_U_cli, "decode image stream", "<in stream> <xsize [long]> <ysize [long]> <nbpix per slice [ASCII file]> <decode map> <out stream> <out image slice index [FITS]>", "impixdecodeU streamin 120 120 pixsclienb.txt decmap outim outsliceindex.fits", "COREMOD_MEMORY_PixMapDecode_U(const char *inputstream_name, uint32_t xsizeim, uint32_t ysizeim, const char* NBpix_fname, const char* IDmap_name, const char *IDout_name, const char *IDout_pixslice_fname)");
    
    RegisterCLIcommand("streamdiff", __FILE__, COREMOD_MEMORY_streamDiff_cli, "compute difference between two image streams", "<in stream 0> <in stream 1> <out stream> <optional mask> <sem trigger index>", "streamdiff stream0 stream1 null outstream 3", "long COREMOD_MEMORY_streamDiff(const char *IDstream0_name, const char *IDstream1_name, const char *IDstreamout_name, long semtrig)");
	
    RegisterCLIcommand("streamhalfdiff", __FILE__, COREMOD_MEMORY_stream_halfimDiff_cli, "compute difference between two halves of an image stream", "<in stream> <out stream> <sem trigger index>", "streamhalfdiff stream outstream 3", "long COREMOD_MEMORY_stream_halfimDiff(const char *IDstream_name, const char *IDstreamout_name, long semtrig)");

	RegisterCLIcommand("streamave", __FILE__, COREMOD_MEMORY_streamAve_cli, "averages stream", "<instream> <NBave> <mode, 1 for single local instance, 0 for loop> <outstream>", "streamave instream 100 0 outstream", "long COREMODE_MEMORY_streamAve(const char *IDstream_name, int NBave, int mode, const char *IDout_name)");


/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 14. DATA LOGGING                                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */


    RegisterCLIcommand("shmimstreamlog", __FILE__, COREMOD_MEMORY_sharedMem_2Dim_log_cli, "logs shared memory stream (run in current directory)", "<shm image> <cubesize [long]> <logdir>", "shmimstreamlog wfscamim 10000 /media/data", "long COREMOD_MEMORY_sharedMem_2Dim_log(const char *IDname, uint32_t zsize, const char *logdir, const char *IDlogdata_name)");
    
    RegisterCLIcommand("shmimslogstat", __FILE__, COREMOD_MEMORY_logshim_printstatus_cli, "print log shared memory stream status", "<shm image>", "shmimslogstat wfscamim", "int COREMOD_MEMORY_logshim_printstatus(const char *IDname)");
    
    RegisterCLIcommand("shmimslogonset", __FILE__, COREMOD_MEMORY_logshim_set_on_cli, "set on variable in log shared memory stream", "<shm image> <setv [long]>", "shmimslogonset imwfs 1", "int COREMOD_MEMORY_logshim_set_on(const char *IDname, int setv)");
    
    RegisterCLIcommand("shmimslogexitset", __FILE__, COREMOD_MEMORY_logshim_set_logexit_cli, "set exit variable in log shared memory stream", "<shm image> <setv [long]>", "shmimslogexitset imwfs 1", "int COREMOD_MEMORY_logshim_set_logexit(const char *IDname, int setv)");
   
    

	

    



    // add atexit functions here

    return 0;
}




















/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */ 
/* 1. MANAGE MEMORY AND IDENTIFIERS                                                                */
/*                                                                                                 */                                                                                        
/* =============================================================================================== */
/* =============================================================================================== */


int_fast8_t memory_monitor(const char *termttyname)
{
    if(data.Debug>0)
        printf("starting memory_monitor on \"%s\"\n", termttyname);

    MEM_MONITOR = 1;
    init_list_image_ID_ncurses(termttyname);
    list_image_ID_ncurses();
    atexit(close_list_image_ID_ncurses);

}




long compute_nb_image()
{
    long i;
    long total=0;

    for(i=0; i<data.NB_MAX_IMAGE; i++)
    {
        if(data.image[i].used==1)
            total += 1;
    }
    return(total);
}

long compute_nb_variable()
{
    long i;
    long total=0;

    for(i=0; i<data.NB_MAX_VARIABLE; i++)
    {
        if(data.variable[i].used==1)
            total += 1;
    }
    return(total);
}

long long compute_image_memory()
{
    long i;
    long long total=0;

    for(i=0; i<data.NB_MAX_IMAGE; i++)
    {
        if(data.image[i].used==1)
            total += data.image[i].md[0].nelement*TYPESIZE[data.image[i].md[0].atype];
    }
    return(total);
}

long compute_variable_memory()
{
    long i;
    long total=0;

    for(i=0; i<data.NB_MAX_VARIABLE; i++)
    {
        total += sizeof(VARIABLE);
        if(data.variable[i].used==1)
        {
            total += 0;
        }
    }
    return(total);
}




long image_ID(const char *name) /* ID number corresponding to a name */
{
    long i,ID;
    int found;
    long tmp = 0;
    struct timespec timenow;

    i = 0;
    found = 0;
    while(found == 0)
    {
        if(data.image[i].used == 1)
        {
            if((strncmp(name,data.image[i].name,strlen(name))==0)&&(data.image[i].name[strlen(name)]=='\0'))
            {
                found = 1;
                tmp = i;
                clock_gettime(CLOCK_REALTIME, &timenow);
                data.image[i].md[0].last_access = 1.0*timenow.tv_sec + 0.000000001*timenow.tv_nsec;
            }
        }
        i++;
        if(i == data.NB_MAX_IMAGE)
        {
            found = 1;
            tmp = -1;
        }
    }
    ID = tmp;

    return(tmp);
}


long image_ID_noaccessupdate(const char *name) /* ID number corresponding to a name */
{
    long i,ID;
    int found;
    long tmp = 0;

    i = 0;
    found = 0;
    while(found == 0)
    {
        if(data.image[i].used == 1)
        {
            if((strncmp(name,data.image[i].name,strlen(name))==0)&&(data.image[i].name[strlen(name)]=='\0'))
            {
                found = 1;
                tmp = i;
            }
        }
        i++;
        if(i == data.NB_MAX_IMAGE)
        {
            found = 1;
            tmp = -1;
        }
    }
    ID = tmp;

    return(tmp);
}


long variable_ID(const char *name) /* ID number corresponding to a name */
{
    long i,ID;
    int found;
    long tmp = -1;

    i = 0;
    found = 0;
    while(found == 0)
    {
        if(data.variable[i].used == 1)
        {
            if((strncmp(name,data.variable[i].name,strlen(name))==0)&&(data.variable[i].name[strlen(name)]=='\0'))
            {
                found = 1;
                tmp = i;
            }
        }
        i++;
        if(i == data.NB_MAX_VARIABLE)
        {
            found = 1;
            tmp = -1;
        }
    }
    ID = tmp;

    /*  if(tmp==-1) printf("error : no variable named \"%s\" in memory\n", name);*/
    return(tmp);
}



long next_avail_image_ID() /* next available ID number */
{
    long i;
    long ID = -1;

# ifdef _OPENMP
    #pragma omp critical
    {
#endif
        for (i=0; i<data.NB_MAX_IMAGE; i++)
        {
            if(data.image[i].used == 0)
            {
                ID = i;
                data.image[ID].used = 1;
                break;
            }
        }
# ifdef _OPENMP
    }
# endif

    if(ID==-1)
        {
			printf("ERROR: ran out of image IDs - cannot allocate new ID\n");
			printf("NB_MAX_IMAGE should be increased above current value (%ld)\n", data.NB_MAX_IMAGE);
			exit(0);
		}

    return(ID);
}



long next_avail_variable_ID() /* next available ID number */
{
    long i;
    long ID = -1;
    int found = 0;

    for (i=0; i<data.NB_MAX_VARIABLE; i++)
    {
        if((data.variable[i].used == 0)&&(found == 0))
        {
            ID = i;
            found = 1;
        }
    }
    if(ID==-1)
    {
        ID = data.NB_MAX_VARIABLE;
    }
    return(ID);
}




int_fast8_t delete_image_ID(const char* imname) /* deletes an ID */
{
    long ID;
    char command[200];
    int r;
    long s;
    char fname[200];

    ID = image_ID(imname);

    if (ID!=-1)
    {
        data.image[ID].used = 0;

        for(s=0; s<data.image[ID].md[0].sem; s++)
            sem_close(data.image[ID].semptr[s]);

        data.image[ID].md[0].sem = 0;
        free(data.image[ID].semptr);
        data.image[ID].semptr = NULL;

        if(data.image[ID].md[0].shared == 1)
        {

            if(data.image[ID].semlog!=NULL)
            {
                sem_close(data.image[ID].semlog);
                data.image[ID].semlog = NULL;
            }

            if (munmap(data.image[ID].md, data.image[ID].memsize) == -1) {
                printf("unmapping ID %ld : %p  %ld\n", ID, data.image[ID].md, data.image[ID].memsize);
                perror("Error un-mmapping the file");
            }
            close(data.image[ID].shmfd);
            data.image[ID].md = NULL;
            data.image[ID].kw = NULL;
            data.image[ID].shmfd = -1;
            data.image[ID].memsize = 0;


            sprintf(command, "rm /dev/shm/sem.%s_sem*", imname);
            r = system(command);


            sprintf(fname, "/dev/shm/sem.%s_semlog", imname);
            remove(fname);


            sprintf(command, "rm %s/%s.im.shm", SHAREDMEMDIR, imname);
            r = system(command);
        }
        else
        {
            if(data.image[ID].md[0].atype == _DATATYPE_UINT8)
            {
                if(data.image[ID].array.UI8 == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.UI8);
                data.image[ID].array.UI8 = NULL;
            }
            if(data.image[ID].md[0].atype == _DATATYPE_INT32)
            {
                if(data.image[ID].array.SI32 == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.SI32);
                data.image[ID].array.SI32 = NULL;
            }
            if(data.image[ID].md[0].atype == _DATATYPE_FLOAT)
            {
                if(data.image[ID].array.F == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.F);
                data.image[ID].array.F = NULL;
            }
            if(data.image[ID].md[0].atype == _DATATYPE_DOUBLE)
            {
                if(data.image[ID].array.D == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.D);
                data.image[ID].array.D = NULL;
            }
            if(data.image[ID].md[0].atype == _DATATYPE_COMPLEX_FLOAT)
            {
                if(data.image[ID].array.CF == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.CF);
                data.image[ID].array.CF = NULL;
            }
            if(data.image[ID].md[0].atype == _DATATYPE_COMPLEX_DOUBLE)
            {
                if(data.image[ID].array.CD == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.CD);
                data.image[ID].array.CD = NULL;
            }
			
            if(data.image[ID].md == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                exit(0);
            }
            free(data.image[ID].md);
            data.image[ID].md = NULL;


            if(data.image[ID].kw!=NULL)
            {
                free(data.image[ID].kw);
                data.image[ID].kw = NULL;
            }

        }
        //free(data.image[ID].logstatus);
        /*      free(data.image[ID].size);*/
        //      data.image[ID].md[0].last_access = 0;
    }
    else
        fprintf(stderr,"%c[%d;%dm WARNING: image %s does not exist [ %s  %s  %d ] %c[%d;m\n", (char) 27, 1, 31, imname, __FILE__, __func__, __LINE__, (char) 27, 0);


    if(MEM_MONITOR == 1)
        list_image_ID_ncurses();

    return(0);
}



// delete all images with a prefix
int_fast8_t delete_image_ID_prefix(const char *prefix)
{
    long i;

    for (i=0; i<data.NB_MAX_IMAGE; i++)
    {
        if(data.image[i].used==1)
            if((strncmp(prefix,data.image[i].name,strlen(prefix)))==0)
            {
                printf("deleting image %s\n",data.image[i].name);
                delete_image_ID(data.image[i].name);
            }
    }
    return(0);
}



int_fast8_t delete_variable_ID(const char* varname) /* deletes a variable ID */
{
    long ID;

    ID = variable_ID(varname);
    if (ID!=-1)
    {
        data.variable[ID].used = 0;
        /*      free(data.variable[ID].name);*/
    }
    else
        fprintf(stderr,"%c[%d;%dm WARNING: variable %s does not exist [ %s  %s  %d ] %c[%d;m\n", (char) 27, 1, 31, varname, __FILE__, __func__, __LINE__, (char) 27, 0);

    return(0);
}



int_fast8_t clearall()
{
    long ID;

    for(ID=0; ID<data.NB_MAX_IMAGE; ID++)
    {
        if(data.image[ID].used==1)
            delete_image_ID(data.image[ID].name);
    }
    for(ID=0; ID<data.NB_MAX_VARIABLE; ID++)
    {
        if(data.variable[ID].used==1)
            delete_variable_ID(data.variable[ID].name);
    }

    return(0);
}




void *save_fits_function( void *ptr )
{
    long ID;
    struct savethreadmsg *tmsg; // = malloc(sizeof(struct savethreadmsg));
    uint32_t*imsizearray;
    uint32_t xsize, ysize;
    uint8_t atype;
    long IDc;
    long framesize; // in bytes
    char *ptr0; // source
    char *ptr1; // destination
    long k;
    FILE *fp;


    imsizearray = (uint32_t*) malloc(sizeof(uint32_t)*3);

    tmsg = (struct savethreadmsg*) ptr;
    // printf("THREAD : SAVING  %s -> %s \n", tmsg->iname, tmsg->fname);
    //fflush(stdout);
    if(tmsg->partial==0) // full image
    {
        save_fits(tmsg->iname, tmsg->fname);
	}
    else
    {
        //      printf("Saving partial image (name = %s   zsize = %ld)\n", tmsg->iname, tmsg->cubesize);

        //	list_image_ID();

        ID = image_ID(tmsg->iname);
        atype = data.image[ID].md[0].atype;
        xsize = data.image[ID].md[0].size[0];
        ysize = data.image[ID].md[0].size[1];

        //printf("step00\n");
        //fflush(stdout);

        imsizearray[0] = xsize;
        imsizearray[1] = ysize;
        imsizearray[2] = tmsg->cubesize;



        IDc = create_image_ID("tmpsavecube", 3, imsizearray, atype, 0, 1);

        // list_image_ID();

        switch ( atype ) {
			
        case _DATATYPE_UINT8:
            framesize = SIZEOF_DATATYPE_UINT8*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.UI8;  // source
			ptr1 = (char*) data.image[IDc].array.UI8; // destination
            break;
        case _DATATYPE_INT8:
            framesize = SIZEOF_DATATYPE_INT8*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.SI8;  // source
			ptr1 = (char*) data.image[IDc].array.SI8; // destination
            break;       
       
         case _DATATYPE_UINT16:
            framesize = SIZEOF_DATATYPE_UINT16*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.UI16;  // source
			ptr1 = (char*) data.image[IDc].array.UI16; // destination
            break;
        case _DATATYPE_INT16:
            framesize = SIZEOF_DATATYPE_INT16*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.SI16;  // source
			ptr1 = (char*) data.image[IDc].array.SI16; // destination
            break;       
 
         case _DATATYPE_UINT32:
            framesize = SIZEOF_DATATYPE_UINT32*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.UI32;  // source
			ptr1 = (char*) data.image[IDc].array.UI32; // destination
            break;
        case _DATATYPE_INT32:
            framesize = SIZEOF_DATATYPE_INT32*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.SI32;  // source
			ptr1 = (char*) data.image[IDc].array.SI32; // destination
            break;       
 
        case _DATATYPE_UINT64:
            framesize = SIZEOF_DATATYPE_UINT64*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.UI64;  // source
			ptr1 = (char*) data.image[IDc].array.UI64; // destination
            break;
        case _DATATYPE_INT64:
            framesize = SIZEOF_DATATYPE_INT64*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.SI64;  // source
			ptr1 = (char*) data.image[IDc].array.SI64; // destination
            break;       
       
        case _DATATYPE_FLOAT:
            framesize = SIZEOF_DATATYPE_FLOAT*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.F;  // source
			ptr1 = (char*) data.image[IDc].array.F; // destination
            break;
        case _DATATYPE_DOUBLE:
            framesize = SIZEOF_DATATYPE_DOUBLE*xsize*ysize;
			ptr0 = (char*) data.image[ID].array.D;  // source
			ptr1 = (char*) data.image[IDc].array.D; // destination
            break;

        default:
            printf("ERROR: WRONG DATA TYPE\n");
			free(imsizearray);
            free(tmsg);
            exit(0);
            break;
        }


        memcpy((void *) ptr1, (void *) ptr0, framesize*tmsg->cubesize);
        save_fits("tmpsavecube", tmsg->fname);
        delete_image_ID("tmpsavecube");
    }
    
    
    if((fp=fopen(tmsg->fnameascii, "w"))==NULL)
    {
		printf("ERROR: cannot create file \"%s\"\n", tmsg->fnameascii);
		exit(0);
	}
	for(k=0;k<tmsg->cubesize;k++)
		{
			fprintf(fp, "%5ld   %8lu  %8lu   %15.9lf\n", k, tmsg->arraycnt0[k], tmsg->arraycnt1[k], tmsg->arraytime[k]);
		}
	fclose(fp);

    //    printf(" DONE\n");
    //fflush(stdout);

    ID = image_ID(tmsg->iname);
    tret = ID;
    free(imsizearray);
    pthread_exit(&tret);
    
  //  free(tmsg);
}










/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */ 
/* 2. KEYWORDS                                                                                     */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




long image_write_keyword_L(const char *IDname, const char *kname, long value, const char *comment)
{
    long ID;
    long kw, NBkw, kw0;

    ID = image_ID(IDname);
    NBkw = data.image[ID].md[0].NBkw;

    kw = 0;
    while((data.image[ID].kw[kw].type!='N')&&(kw<NBkw))
        kw++;
    kw0 = kw;

    if(kw0==NBkw)
    {
        printf("ERROR: no available keyword entry\n");
        exit(0);
    }
    else
    {
        strcpy(data.image[ID].kw[kw].name, kname);
        data.image[ID].kw[kw].type = 'L';
        data.image[ID].kw[kw].value.numl = value;
        strcpy(data.image[ID].kw[kw].comment, comment);
    }

    return(kw0);
}

long image_write_keyword_D(const char *IDname, const char *kname, double value, const char *comment)
{
    long ID;
    long kw, NBkw, kw0;

    ID = image_ID(IDname);
    NBkw = data.image[ID].md[0].NBkw;

    kw = 0;
    while((data.image[ID].kw[kw].type!='N')&&(kw<NBkw))
        kw++;
    kw0 = kw;

    if(kw0==NBkw)
    {
        printf("ERROR: no available keyword entry\n");
        exit(0);
    }
    else
    {
        strcpy(data.image[ID].kw[kw].name, kname);
        data.image[ID].kw[kw].type = 'D';
        data.image[ID].kw[kw].value.numf = value;
        strcpy(data.image[ID].kw[kw].comment, comment);
    }

    return(kw0);
}

long image_write_keyword_S(const char *IDname, const char *kname, const char *value, const char *comment)
{
    long ID;
    long kw, NBkw, kw0;

    ID = image_ID(IDname);
    NBkw = data.image[ID].md[0].NBkw;

    kw = 0;
    while((data.image[ID].kw[kw].type!='N')&&(kw<NBkw))
        kw++;
    kw0 = kw;

    if(kw0==NBkw)
    {
        printf("ERROR: no available keyword entry\n");
        exit(0);
    }
    else
    {
        strcpy(data.image[ID].kw[kw].name, kname);
        data.image[ID].kw[kw].type = 'D';
        strcpy(data.image[ID].kw[kw].value.valstr, value);
        strcpy(data.image[ID].kw[kw].comment, comment);
    }

    return(kw0);
}




long image_list_keywords(const char *IDname)
{
    long ID;
    long kw, NBkw;

    ID = image_ID(IDname);

    for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
    {
        if(data.image[ID].kw[kw].type=='L')
            printf("%18s  %20ld %s\n", data.image[ID].kw[kw].name, data.image[ID].kw[kw].value.numl, data.image[ID].kw[kw].comment);
        if(data.image[ID].kw[kw].type=='D')
            printf("%18s  %20lf %s\n", data.image[ID].kw[kw].name, data.image[ID].kw[kw].value.numf, data.image[ID].kw[kw].comment);
        if(data.image[ID].kw[kw].type=='S')
            printf("%18s  %20s %s\n", data.image[ID].kw[kw].name, data.image[ID].kw[kw].value.valstr, data.image[ID].kw[kw].comment);
        //	if(data.image[ID].kw[kw].type=='N')
        //	printf("-------------\n");
    }

    return(ID);
}


long image_read_keyword_D(const char *IDname, const char *kname, double *val)
{
    long ID;
    long kw, NBkw, kw0;

    ID = image_ID(IDname);
    kw0 = -1;
    for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
    {
        if((data.image[ID].kw[kw].type=='D')&&(strncmp(kname, data.image[ID].kw[kw].name ,strlen(kname))==0))
        {
            kw0 = kw;
            *val = data.image[ID].kw[kw].value.numf;
        }
    }

    return(kw0);
}


long image_read_keyword_L(const char *IDname, const char *kname, long *val)
{
    long ID;
    long kw, NBkw, kw0;

    ID = image_ID(IDname);
    kw0 = -1;
    for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
    {
        if((data.image[ID].kw[kw].type=='L')&&(strncmp(kname, data.image[ID].kw[kw].name ,strlen(kname))==0))
        {
            kw0 = kw;
            *val = data.image[ID].kw[kw].value.numl;
        }
    }

    return(kw0);
}





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */ 
/* 3. READ SHARED MEM IMAGE AND SIZE                                                               */
/*                                                                                                 */ 
/* =============================================================================================== */
/* =============================================================================================== */




/**
 *  ## Purpose
 * 
 *  Read shared memory image size
 * 
 * 
 * ## Arguments
 * 
 * @param[in]
 * name		char*
 * -		stream name
 * 
 * @param[in]
 * fname	char*
 * 			file name to write image name
 * 
 */
long read_sharedmem_image_size(const char *name, const char *fname)
{
    int SM_fd;
    struct stat file_stat;
    char SM_fname[200];
    IMAGE_METADATA *map;
    int naxis;
    int i;
    FILE *fp;

    long ID;


    if((ID=image_ID(name))==-1)
    {
		
		
        sprintf(SM_fname, "%s/%s.im.shm", SHAREDMEMDIR, name);

        SM_fd = open(SM_fname, O_RDWR);
        if(SM_fd==-1)
            printf("Cannot import file - continuing\n");
        else
        {
            fstat(SM_fd, &file_stat);
            //        printf("File %s size: %zd\n", SM_fname, file_stat.st_size);

            map = (IMAGE_METADATA*) mmap(0, sizeof(IMAGE_METADATA), PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
            if (map == MAP_FAILED) {
                close(SM_fd);
                perror("Error mmapping the file");
                exit(0);
            }

            fp = fopen(fname, "w");
            for(i=0; i<map[0].naxis; i++)
                fprintf(fp, "%ld ", (long) map[0].size[i]);
            fprintf(fp, "\n");
            fclose(fp);


            if (munmap(map, sizeof(IMAGE_METADATA)) == -1) {
                printf("unmapping %s\n", SM_fname);
                perror("Error un-mmapping the file");
            }
            close(SM_fd);
        }
    }
    else
    {
        fp = fopen(fname, "w");
        for(i=0; i<data.image[ID].md[0].naxis; i++)
            fprintf(fp, "%ld ", (long) data.image[ID].md[0].size[i]);
        fprintf(fp, "\n");
        fclose(fp);
    }

    return 0;
}













long read_sharedmem_image(const char *name)
{
	long ID = -1;
	IMAGE *image;
	
	ID = next_avail_image_ID();
	
	image = &data.image[ID];
	if(ImageStreamIO_read_sharedmem_image_toIMAGE(name, image)==-1)
		ID = -1;

    if(MEM_MONITOR == 1)
		list_image_ID_ncurses();
	
	return(ID);
}









/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 4. CREATE IMAGE                                                                                 */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */






/* creates an image ID */
/* all images should be created by this function */
long create_image_ID(const char *name, long naxis, uint32_t *size, uint8_t atype, int shared, int NBkw)
{
    long ID;
    long i,ii;
    time_t lt;
    long nelement;
    struct timespec timenow;
    char sname[200];

    size_t sharedsize = 0; // shared memory size in bytes
    int SM_fd; // shared memory file descriptor
    char SM_fname[200];
    int result;
    IMAGE_METADATA *map;
    char *mapv; // pointed cast in bytes

    int kw;
    char comment[80];
    char kname[16];



    ID = -1;
    if(image_ID(name) == -1)
    {
        ID = next_avail_image_ID();
        ImageStreamIO_createIm(&data.image[ID], name, naxis, size, atype, shared, NBkw);
    }
    else
    {
        // Cannot create image : name already in use
        ID = image_ID(name);

        if(data.image[ID].md[0].atype != atype)
        {
            fprintf(stderr,"%c[%d;%dm ERROR: [ %s %s %d ] %c[%d;m\n", (char) 27, 1, 31, __FILE__, __func__, __LINE__, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Pre-existing image \"%s\" has wrong type %c[%d;m\n", (char) 27, 1, 31,name, (char) 27, 0);
            exit(0);
        }
        if(data.image[ID].md[0].naxis != naxis)
        {
            fprintf(stderr,"%c[%d;%dm ERROR: [ %s %s %d ] %c[%d;m\n", (char) 27, 1, 31, __FILE__, __func__, __LINE__, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Pre-existing image \"%s\" has wrong naxis %c[%d;m\n", (char) 27, 1, 31,name, (char) 27, 0);
            exit(0);
        }

        for(i=0; i<naxis; i++)
            if(data.image[ID].md[0].size[i] != size[i])
            {
                fprintf(stderr,"%c[%d;%dm ERROR: [ %s %s %d ] %c[%d;m\n", (char) 27, 1, 31, __FILE__, __func__, __LINE__, (char) 27, 0);
                fprintf(stderr,"%c[%d;%dm Pre-existing image \"%s\" has wrong size %c[%d;m\n", (char) 27, 1, 31,name, (char) 27, 0);
                fprintf(stderr,"Axis %ld :  %ld  %ld\n", i, (long) data.image[ID].md[0].size[i], (long) size[i]);
                exit(0);
            }
    }

    if(MEM_MONITOR == 1)
        list_image_ID_ncurses();

    return(ID);
}








long create_1Dimage_ID(const char *ID_name, uint32_t xsize)
{
    long ID = -1;
    long naxis = 1;
    uint32_t naxes[1];

    naxes[0]=xsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}



long create_1DCimage_ID(const char *ID_name, uint32_t xsize)
{
    long ID = -1;
    long naxis=1;
    uint32_t naxes[1];

    naxes[0]=xsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_COMPLEX_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_COMPLEX_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

long create_2Dimage_ID(const char *ID_name, uint32_t xsize, uint32_t ysize)
{
    long ID = -1;
    long naxis=2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    // printf("Creating 2D image %s, %ld x %ld [%d %d]", ID_name, xsize, ysize, data.SHARED_DFT, data.NBKEWORD_DFT);
    // fflush(stdout);

    if(data.precision == 0)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    else if (data.precision == 1)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision
    else
    {
        printf("Default precision (%d) invalid value: assuming single precision\n", data.precision);
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    }

    //  printf("\n");
    // fflush(stdout);



    return(ID);
}

long create_2Dimage_ID_double(const char *ID_name, uint32_t xsize, uint32_t ysize)
{
    long ID = -1;
    long naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT);

    return(ID);
}


/* 2D complex image */
long create_2DCimage_ID(const char *ID_name, uint32_t xsize, uint32_t ysize)
{
    long ID = -1;
    long naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_COMPLEX_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_COMPLEX_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

/* 2D complex image */
long create_2DCimage_ID_double(const char *ID_name, uint32_t xsize, uint32_t ysize)
{
    long ID = -1;
    long naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_COMPLEX_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}



/* 3D image, single precision */
long create_3Dimage_ID_float(const char *ID_name, uint32_t xsize, uint32_t ysize, uint32_t zsize)
{
    long ID = -1;
    long naxis = 3;
    uint32_t naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    //  printf("CREATING 3D IMAGE: %s %ld %ld %ld\n", ID_name, xsize, ysize, zsize);
    //  fflush(stdout);

    ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision

    //  printf("IMAGE CREATED WITH ID = %ld\n",ID);
    //  fflush(stdout);

    return(ID);
}


/* 3D image, double precision */
long create_3Dimage_ID_double(const char *ID_name, uint32_t xsize, uint32_t ysize, uint32_t zsize)
{
    long ID;
    long naxis = 3;
    uint32_t naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}



/* 3D image, default precision */
long create_3Dimage_ID(const char *ID_name, uint32_t xsize, uint32_t ysize, uint32_t zsize)
{
    long ID = -1;
    long naxis = 3;
    uint32_t naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name,naxis,naxes, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

/* 3D complex image */
long create_3DCimage_ID(const char *ID_name, uint32_t xsize, uint32_t ysize, uint32_t zsize)
{
    long ID = -1;
    long naxis = 3;
    uint32_t naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_COMPLEX_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_COMPLEX_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

















/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */ 
/* 5. CREATE VARIABLE                                                                              */
/*                                                                                                 */                                                                                        
/* =============================================================================================== */
/* =============================================================================================== */





/* creates floating point variable */
long create_variable_ID(const char *name, double value)
{
    long ID;
    long i1,i2;

    ID = -1;
    i1 = image_ID(name);
    i2 = variable_ID(name);

    if(i1!=-1)
    {
        printf("ERROR: cannot create variable \"%s\": name already used as an image\n",name);
    }
    else
    {
        if(i2!=-1)
        {
            //	  printf("Warning : variable name \"%s\" is already in use\n",name);
            ID = i2;
        }
        else
            ID = next_avail_variable_ID();

        data.variable[ID].used = 1;
        data.variable[ID].type = 0; /** floating point double */
        strcpy(data.variable[ID].name,name);
        data.variable[ID].value.f = value;

    }

    return(ID);
}


/* creates long variable */
long create_variable_long_ID(const char *name, long value)
{
    long ID;
    long i1,i2;

    ID = -1;
    i1 = image_ID(name);
    i2 = variable_ID(name);

    if(i1!=-1)
    {
        printf("ERROR: cannot create variable \"%s\": name already used as an image\n",name);
    }
    else
    {
        if(i2!=-1)
        {
            //	  printf("Warning : variable name \"%s\" is already in use\n",name);
            ID = i2;
        }
        else
            ID = next_avail_variable_ID();

        data.variable[ID].used = 1;
        data.variable[ID].type = 1; /** long */
        strcpy(data.variable[ID].name,name);
        data.variable[ID].value.l = value;

    }

    return(ID);
}



/* creates long variable */
long create_variable_string_ID(const char *name, const char *value)
{
    long ID;
    long i1,i2;

    ID = -1;
    i1 = image_ID(name);
    i2 = variable_ID(name);

    if(i1!=-1)
    {
        printf("ERROR: cannot create variable \"%s\": name already used as an image\n",name);
    }
    else
    {
        if(i2!=-1)
        {
            //	  printf("Warning : variable name \"%s\" is already in use\n",name);
            ID = i2;
        }
        else
            ID = next_avail_variable_ID();

        data.variable[ID].used = 1;
        data.variable[ID].type = 2; /** string */
        strcpy(data.variable[ID].name,name);
        strcpy(data.variable[ID].value.s, value);
    }

    return(ID);
}

















/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */ 
/* 6. COPY IMAGE                                                                                   */
/*                                                                                                 */                                                                                        
/* =============================================================================================== */
/* =============================================================================================== */




long copy_image_ID(const char *name, const char *newname, int shared)
{
    long ID, IDout;
    long naxis;
    uint32_t *size = NULL;
    uint8_t atype;
    long nelement;
    long i;
    int newim = 0;
    long s;
    char errstr[200];
    int semval;


    ID = image_ID(name);
    if(ID==-1)
        {
            sprintf(errstr, "image \"%s\" does not exist", name);
            printERROR(__FILE__,__func__,__LINE__, errstr);
            exit(0);
        }
    naxis = data.image[ID].md[0].naxis;

    size = (uint32_t*) malloc(sizeof(uint32_t)*naxis);
    if(size==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"malloc error");
        exit(0);
    }

    for(i=0; i<naxis; i++)
        size[i] = data.image[ID].md[0].size[i];
    atype  = data.image[ID].md[0].atype;

    nelement = data.image[ID].md[0].nelement;

    IDout = image_ID(newname);

    if(IDout!=-1)
    {
        // verify newname has the right size and type
        if(data.image[ID].md[0].nelement != data.image[IDout].md[0].nelement)
        {
            fprintf(stderr,"ERROR [copy_image_ID]: images %s and %s do not have the same size -> deleting and re-creating image\n", name, newname);
            newim = 1;
        }
        if(data.image[ID].md[0].atype!=data.image[IDout].md[0].atype)
        {
            fprintf(stderr,"ERROR [copy_image_ID]: images %s and %s do not have the same type -> deleting and re-creating image\n", name, newname);
            newim = 1;
        }

        if(newim == 1)
        {
            delete_image_ID(newname);
            IDout = -1;
        }
    }




    if(IDout==-1)
    {
        create_image_ID(newname, naxis, size, atype, shared, data.NBKEWORD_DFT);
        IDout = image_ID(newname);
    }
    else
    {
        // verify newname has the right size and type
        if(data.image[ID].md[0].nelement != data.image[IDout].md[0].nelement)
        {
            fprintf(stderr,"ERROR [copy_image_ID]: images %s and %s do not have the same size\n",name,newname);
            exit(0);
        }
        if(data.image[ID].md[0].atype!=data.image[IDout].md[0].atype)
        {
            fprintf(stderr,"ERROR [copy_image_ID]: images %s and %s do not have the same type\n",name,newname);
            exit(0);
        }
    }
    data.image[IDout].md[0].write = 1;


    if(atype == _DATATYPE_UINT8)
        memcpy (data.image[IDout].array.UI8, data.image[ID].array.UI8, SIZEOF_DATATYPE_UINT8*nelement);

    if(atype == _DATATYPE_INT8)
        memcpy (data.image[IDout].array.SI8, data.image[ID].array.SI8, SIZEOF_DATATYPE_INT8*nelement);

    if(atype == _DATATYPE_UINT16)
        memcpy (data.image[IDout].array.UI16, data.image[ID].array.UI16, SIZEOF_DATATYPE_UINT16*nelement);

    if(atype == _DATATYPE_INT16)
        memcpy (data.image[IDout].array.SI16, data.image[ID].array.SI16, SIZEOF_DATATYPE_INT8*nelement);

    if(atype == _DATATYPE_UINT32)
        memcpy (data.image[IDout].array.UI32, data.image[ID].array.UI32, SIZEOF_DATATYPE_UINT32*nelement);

    if(atype == _DATATYPE_INT32)
        memcpy (data.image[IDout].array.SI32, data.image[ID].array.SI32, SIZEOF_DATATYPE_INT32*nelement);

    if(atype == _DATATYPE_UINT64)
        memcpy (data.image[IDout].array.UI64, data.image[ID].array.UI64, SIZEOF_DATATYPE_UINT64*nelement);

    if(atype == _DATATYPE_INT64)
        memcpy (data.image[IDout].array.SI64, data.image[ID].array.SI64, SIZEOF_DATATYPE_INT64*nelement);


    if(atype == _DATATYPE_FLOAT)
        memcpy (data.image[IDout].array.F, data.image[ID].array.F, SIZEOF_DATATYPE_FLOAT*nelement);

    if(atype == _DATATYPE_DOUBLE)
        memcpy (data.image[IDout].array.D, data.image[ID].array.D, SIZEOF_DATATYPE_DOUBLE*nelement);

    if(atype == _DATATYPE_COMPLEX_FLOAT)
        memcpy (data.image[IDout].array.CF, data.image[ID].array.CF, SIZEOF_DATATYPE_COMPLEX_FLOAT*nelement);

    if(atype == _DATATYPE_COMPLEX_DOUBLE)
        memcpy (data.image[IDout].array.CD, data.image[ID].array.CD, SIZEOF_DATATYPE_COMPLEX_DOUBLE*nelement);


    
    COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
    data.image[IDout].md[0].write = 0;
    data.image[IDout].md[0].cnt0++;

    free(size);


    return(IDout);
}




long chname_image_ID(const char *ID_name, const char *new_name)
{
    long ID;

    ID=-1;
    if((image_ID(new_name)==-1)&&(variable_ID(new_name)==-1))
    {
        ID = image_ID(ID_name);
        strcpy(data.image[ID].name, new_name);
        //      if ( Debug > 0 ) { printf("change image name %s -> %s\n",ID_name,new_name);}
    }
    else
        printf("Cannot change name %s -> %s : new name already in use\n", ID_name, new_name);

    if(MEM_MONITOR==1)
        list_image_ID_ncurses();

    return(ID);
}






/** copy an image to shared memory
 *
 *
 */
long COREMOD_MEMORY_cp2shm(const char *IDname, const char *IDshmname)
{
    long ID;
    long IDshm;
    uint8_t atype;
    long naxis;
    uint32_t *sizearray;
    char *ptr1;
    char *ptr2;
    long k;
	int axis;
	int shmOK;
	

    ID = image_ID(IDname);
    naxis = data.image[ID].md[0].naxis;


    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*naxis);
    atype = data.image[ID].md[0].atype;
    for(k=0; k<naxis; k++)
        sizearray[k] = data.image[ID].md[0].size[k];

	shmOK = 1;
	IDshm = read_sharedmem_image(IDshmname);
	if(IDshm!=-1)
	{
		// verify type and size
		if(data.image[ID].md[0].naxis!=data.image[IDshm].md[0].naxis)
			shmOK = 0;
		if(shmOK==1)
			{
				for(axis=0;axis<data.image[IDshm].md[0].naxis;axis++)
					if(data.image[ID].md[0].size[axis]!=data.image[IDshm].md[0].size[axis])
						shmOK = 0;
			}
		if(data.image[ID].md[0].atype!=data.image[IDshm].md[0].atype)
			shmOK = 0;
	
		if(shmOK==0)
			{
				delete_image_ID(IDshmname);
				IDshm = -1;
			}
	}
	
	if(IDshm==-1)
		IDshm = create_image_ID(IDshmname, naxis, sizearray, atype, 1, 0);
    free(sizearray);

    //data.image[IDshm].md[0].nelement = data.image[ID].md[0].nelement;
    //printf("======= %ld %ld ============\n", data.image[ID].md[0].nelement, data.image[IDshm].md[0].nelement);

	data.image[IDshm].md[0].write = 1;

    switch (atype) {
		
    case _DATATYPE_FLOAT :
        ptr1 = (char*) data.image[ID].array.F;
        ptr2 = (char*) data.image[IDshm].array.F;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_FLOAT*data.image[ID].md[0].nelement);
        break;
    
    case _DATATYPE_DOUBLE :
        ptr1 = (char*) data.image[ID].array.D;
        ptr2 = (char*) data.image[IDshm].array.D;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_DOUBLE*data.image[ID].md[0].nelement);
        break;


    case _DATATYPE_INT8 :
        ptr1 = (char*) data.image[ID].array.SI8;
        ptr2 = (char*) data.image[IDshm].array.SI8;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_INT8*data.image[ID].md[0].nelement);
        break;
    
    case _DATATYPE_UINT8 :
        ptr1 = (char*) data.image[ID].array.UI8;
        ptr2 = (char*) data.image[IDshm].array.UI8;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_UINT8*data.image[ID].md[0].nelement);
        break;

    case _DATATYPE_INT16 :
        ptr1 = (char*) data.image[ID].array.SI16;
        ptr2 = (char*) data.image[IDshm].array.SI16;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_INT16*data.image[ID].md[0].nelement);
        break;
    
    case _DATATYPE_UINT16 :
        ptr1 = (char*) data.image[ID].array.UI16;
        ptr2 = (char*) data.image[IDshm].array.UI16;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_UINT16*data.image[ID].md[0].nelement);
        break;
    
    case _DATATYPE_INT32 :
        ptr1 = (char*) data.image[ID].array.SI32;
        ptr2 = (char*) data.image[IDshm].array.SI32;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_INT32*data.image[ID].md[0].nelement);
        break;

    case _DATATYPE_UINT32 :
        ptr1 = (char*) data.image[ID].array.UI32;
        ptr2 = (char*) data.image[IDshm].array.UI32;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_UINT32*data.image[ID].md[0].nelement);
        break;

    case _DATATYPE_INT64 :
        ptr1 = (char*) data.image[ID].array.SI64;
        ptr2 = (char*) data.image[IDshm].array.SI64;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_INT64*data.image[ID].md[0].nelement);
        break;

    case _DATATYPE_UINT64 :
        ptr1 = (char*) data.image[ID].array.UI64;
        ptr2 = (char*) data.image[IDshm].array.UI64;
        memcpy((void *) ptr2, (void *) ptr1, SIZEOF_DATATYPE_UINT64*data.image[ID].md[0].nelement);
        break;


    default :
        printf("data type not supported\n");
        break;
    }
    COREMOD_MEMORY_image_set_sempost_byID(IDshm, -1);
	data.image[IDshm].md[0].cnt0++;
	data.image[IDshm].md[0].write = 0;

    return(0);
}










/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 7. DISPLAY / LISTS                                                                              */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */







int_fast8_t init_list_image_ID_ncurses(const char *termttyname)
{
    int wrow, wcol;

    listim_scr_fpi = fopen(termttyname, "r");
    listim_scr_fpo = fopen(termttyname, "w");
    listim_scr = newterm(NULL, listim_scr_fpo, listim_scr_fpi);

    getmaxyx(stdscr, listim_scr_wrow, listim_scr_wcol);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_RED);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_BLACK, COLOR_GREEN);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(9, COLOR_YELLOW, COLOR_BLACK);

    return 0;
}


int_fast8_t list_image_ID_ncurses()
{
    char str[500];
    char str1[500];
    char str2[500];
    long i, j;
    long long tmp_long;
    char type[STYPESIZE];
    uint8_t atype;
    int n;
    long long sizeb, sizeKb, sizeMb, sizeGb;

    struct timespec timenow;
    double timediff;

    clock_gettime(CLOCK_REALTIME, &timenow);

    set_term(listim_scr);

    clear();


    sizeb = compute_image_memory();


    printw("INDEX    NAME         SIZE                    TYPE        SIZE  [percent]    LAST ACCESS\n");
    printw("\n");

    for (i=0; i<data.NB_MAX_IMAGE; i++)
    {
        if(data.image[i].used==1)
        {
            atype = data.image[i].md[0].atype;
            tmp_long = ((long long) (data.image[i].md[0].nelement)) * TYPESIZE[atype];

            if(data.image[i].md[0].shared == 1)
                printw("%4ldS", i);
            else
                printw("%4ld ", i);

            if(data.image[i].md[0].shared == 1)
                attron(A_BOLD | COLOR_PAIR(9));
            else
                attron(A_BOLD | COLOR_PAIR(6));
            sprintf(str, "%10s ", data.image[i].name);
            printw(str);

            if(data.image[i].md[0].shared == 1)
                attroff(A_BOLD | COLOR_PAIR(9));
            else
                attroff(A_BOLD | COLOR_PAIR(6));

            sprintf(str, "[ %6ld", (long) data.image[i].md[0].size[0]);

            for(j=1; j<data.image[i].md[0].naxis; j++)
            {
                sprintf(str1, "%s x %6ld", str, (long) data.image[i].md[0].size[j]);
            }
            sprintf(str2, "%s]", str1);

            printw("%-28s", str2);

            attron(COLOR_PAIR(3));
            n = 0;
            
            if(atype == _DATATYPE_UINT8)
                n = snprintf(type,STYPESIZE,"UINT8  ");
            if(atype == _DATATYPE_INT8)
                n = snprintf(type,STYPESIZE,"INT8   ");
            if(atype == _DATATYPE_UINT16)
                n = snprintf(type,STYPESIZE,"UINT16 ");
            if(atype == _DATATYPE_INT16)
                n = snprintf(type,STYPESIZE,"INT16  ");
            if(atype == _DATATYPE_UINT32)
                n = snprintf(type,STYPESIZE,"UINT32 ");
            if(atype == _DATATYPE_INT32)
                n = snprintf(type,STYPESIZE,"INT32  ");
            if(atype == _DATATYPE_UINT64)
                n = snprintf(type,STYPESIZE,"UINT64 ");
            if(atype == _DATATYPE_INT64)
                n = snprintf(type,STYPESIZE,"INT64  ");
            if(atype == _DATATYPE_FLOAT)
                n = snprintf(type,STYPESIZE,"FLOAT  ");            
            if(atype == _DATATYPE_DOUBLE)
                n = snprintf(type,STYPESIZE,"DOUBLE ");
            if(atype == _DATATYPE_COMPLEX_FLOAT)
                n = snprintf(type,STYPESIZE,"CFLOAT ");            
            if(atype == _DATATYPE_COMPLEX_DOUBLE)
                n = snprintf(type,STYPESIZE,"CDOUBLE");
            
            printw("%7s ", type);

            attroff(COLOR_PAIR(3));

            if(n >= STYPESIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

            printw("%10ld Kb %6.2f   ", (long) (tmp_long/1024), (float) (100.0*tmp_long/sizeb));

            timediff = (1.0*timenow.tv_sec + 0.000000001*timenow.tv_nsec) - data.image[i].md[0].last_access;

            if(timediff<0.01)
            {
                attron(COLOR_PAIR(4));
                printw("%15.9f\n", timediff);
                attroff(COLOR_PAIR(4));
            }
            else
                printw("%15.9f\n", timediff);
        }
        else
        {
            printw("\n");
        }
    }

    sizeGb = 0;
    sizeMb = 0;
    sizeKb = 0;
    sizeb = compute_image_memory();

    if(sizeb>1024-1)
    {
        sizeKb = sizeb/1024;
        sizeb = sizeb-1024*sizeKb;
    }
    if(sizeKb>1024-1)
    {
        sizeMb = sizeKb/1024;
        sizeKb = sizeKb-1024*sizeMb;
    }
    if(sizeMb>1024-1)
    {
        sizeGb = sizeMb/1024;
        sizeMb = sizeMb-1024*sizeGb;
    }

    //attron(A_BOLD);

    sprintf(str, "%ld image(s)      ", compute_nb_image());
    if(sizeGb>0){
        sprintf(str1, "%s %ld GB", str, (long) (sizeGb));
		strcpy(str, str1);
	}
    
    if(sizeMb>0){
        sprintf(str1, "%s %ld MB", str, (long) (sizeMb));
		strcpy(str, str1);
	}

    if(sizeKb>0){
        sprintf(str1, "%s %ld KB", str, (long) (sizeKb));
		strcpy(str, str1);
	}

    if(sizeb>0){
        sprintf(str1, "%s %ld B", str, (long) (sizeb));
		strcpy(str, str1);
	}

    mvprintw(listim_scr_wrow-1, 0, "%s\n", str);
    //  attroff(A_BOLD);

    refresh();


    return 0;
}



void close_list_image_ID_ncurses( void )
{
    printf("Closing monitor cleanly\n");
    set_term(listim_scr);
    endwin();
    fclose(listim_scr_fpo);
    fclose(listim_scr_fpi);
    MEM_MONITOR = 0;
}







int_fast8_t list_image_ID_ofp(FILE *fo)
{
    long i,j;
    long long tmp_long;
    char type[STYPESIZE];
    uint8_t atype;
    int n;
    unsigned long long sizeb, sizeKb, sizeMb, sizeGb;
    char str[500];
    char str1[500];
    struct timespec timenow;
    double timediff;
	struct mallinfo minfo;

    sizeb = compute_image_memory();
	minfo = mallinfo();

    clock_gettime(CLOCK_REALTIME, &timenow);
	fprintf(fo, "time:  %ld.%09ld\n", timenow.tv_sec % 60, timenow.tv_nsec);
 
    

    fprintf(fo, "\n");
    fprintf(fo, "INDEX    NAME         SIZE                    TYPE        SIZE  [percent]    LAST ACCESS\n");
    fprintf(fo, "\n");

    for (i=0; i<data.NB_MAX_IMAGE; i++)
        if(data.image[i].used==1)
        {
            atype = data.image[i].md[0].atype;
            tmp_long = ((long long) (data.image[i].md[0].nelement)) * TYPESIZE[atype];

            if(data.image[i].md[0].shared==1)
                fprintf(fo, "%4ld %c[%d;%dm%14s%c[%d;m ",i, (char) 27, 1, 34, data.image[i].name, (char) 27, 0);
            else
                fprintf(fo, "%4ld %c[%d;%dm%14s%c[%d;m ",i, (char) 27, 1, 33, data.image[i].name, (char) 27, 0);
            //fprintf(fo, "%s", str);

            sprintf(str, "[ %6ld", (long) data.image[i].md[0].size[0]);

            for(j=1; j<data.image[i].md[0].naxis; j++)
            {
                sprintf(str1, "%s x %6ld", str, (long) data.image[i].md[0].size[j]);
				strcpy(str, str1);
            }
            sprintf(str1, "%s]", str);
            strcpy(str, str1);

            fprintf(fo, "%-32s", str);


            n = 0;
            if(atype == _DATATYPE_UINT8)
                n = snprintf(type, STYPESIZE, "UINT8  ");
            if(atype == _DATATYPE_INT8)
                n = snprintf(type, STYPESIZE, "INT8   ");
            if(atype == _DATATYPE_UINT16)
                n = snprintf(type, STYPESIZE, "UINT16 ");
            if(atype == _DATATYPE_INT16)
                n = snprintf(type, STYPESIZE, "INT16  ");
            if(atype == _DATATYPE_UINT32)
                n = snprintf(type, STYPESIZE, "UINT32 ");
            if(atype == _DATATYPE_INT32)
                n = snprintf(type, STYPESIZE, "INT32  ");
            if(atype == _DATATYPE_UINT64)
                n = snprintf(type, STYPESIZE, "UINT64 ");
            if(atype == _DATATYPE_INT64)
                n = snprintf(type, STYPESIZE, "INT64  ");
            if(atype == _DATATYPE_FLOAT)
                n = snprintf(type, STYPESIZE, "FLOAT  ");            
            if(atype == _DATATYPE_DOUBLE)
                n = snprintf(type, STYPESIZE, "DOUBLE ");
            if(atype == _DATATYPE_COMPLEX_FLOAT)
                n = snprintf(type, STYPESIZE, "CFLOAT ");            
            if(atype == _DATATYPE_COMPLEX_DOUBLE)
                n = snprintf(type, STYPESIZE, "CDOUBLE");

            fprintf(fo, "%7s ", type);


            if(n >= STYPESIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

            fprintf(fo, "%10ld Kb %6.2f   ", (long) (tmp_long/1024), (float) (100.0*tmp_long/sizeb));

            timediff = (1.0*timenow.tv_sec + 0.000000001*timenow.tv_nsec) - data.image[i].md[0].last_access;

            fprintf(fo, "%15.9f\n", timediff);
        }
    fprintf(fo, "\n");


    sizeGb = 0;
    sizeMb = 0;
    sizeKb = 0;
    sizeb = compute_image_memory();

    if(sizeb>1024-1)
    {
        sizeKb = sizeb/1024;
        sizeb = sizeb-1024*sizeKb;
    }
    if(sizeKb>1024-1)
    {
        sizeMb = sizeKb/1024;
        sizeKb = sizeKb-1024*sizeMb;
    }
    if(sizeMb>1024-1)
    {
        sizeGb = sizeMb/1024;
        sizeMb = sizeMb-1024*sizeGb;
    }

    fprintf(fo, "%ld image(s)   ",compute_nb_image());
    if(sizeGb>0)
        fprintf(fo, " %ld Gb",(long) (sizeGb));
    if(sizeMb>0)
        fprintf(fo, " %ld Mb",(long) (sizeMb));
    if(sizeKb>0)
        fprintf(fo, " %ld Kb",(long) (sizeKb));
    if(sizeb>0)
        fprintf(fo, " %ld",(long) (sizeb));
    fprintf(fo, "\n");

    fflush(fo);



    return(0);
}



int_fast8_t list_image_ID_ofp_simple(FILE *fo)
{
    long i,j;
    long long tmp_long;
    uint8_t atype;

    for (i=0; i<data.NB_MAX_IMAGE; i++)
        if(data.image[i].used==1)
        {
            atype = data.image[i].md[0].atype;
            tmp_long = ((long long) (data.image[i].md[0].nelement)) * TYPESIZE[atype];

            fprintf(fo, "%20s %d %ld %d %4ld", data.image[i].name, atype, (long) data.image[i].md[0].naxis, data.image[i].md[0].shared, (long) data.image[i].md[0].size[0]);

            for(j=1; j<data.image[i].md[0].naxis; j++)
                fprintf(fo, " %4ld", (long) data.image[i].md[0].size[j]);
            fprintf(fo, "\n");
        }
    fprintf(fo, "\n");

    return(0);
}



int_fast8_t list_image_ID()
{
    list_image_ID_ofp(stdout);
	malloc_stats();
    return 0;
}



/* list all images in memory
   output is written in ASCII file
   only basic info is listed
   image name
   number of axis
   size
   type
 */

int_fast8_t list_image_ID_file(const char *fname)
{
    FILE *fp;
    long i,j;
    uint8_t atype;
    char type[STYPESIZE];
    int n;

    fp = fopen(fname,"w");
    if(fp == NULL)
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Cannot create file %s",fname);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }

    for (i=0; i<data.NB_MAX_IMAGE; i++)
        if(data.image[i].used == 1)
        {
            atype = data.image[i].md[0].atype;
            fprintf(fp,"%ld %s", i, data.image[i].name);
            fprintf(fp," %ld", (long) data.image[i].md[0].naxis);
            for(j=0; j<data.image[i].md[0].naxis; j++)
                fprintf(fp," %ld", (long) data.image[i].md[0].size[j]);

            n = 0;

            if(atype == _DATATYPE_UINT8)
                n = snprintf(type, STYPESIZE, "UINT8  ");
            if(atype == _DATATYPE_INT8)
                n = snprintf(type, STYPESIZE, "INT8   ");
            if(atype == _DATATYPE_UINT16)
                n = snprintf(type, STYPESIZE, "UINT16 ");
            if(atype == _DATATYPE_INT16)
                n = snprintf(type, STYPESIZE, "INT16  ");
            if(atype == _DATATYPE_UINT32)
                n = snprintf(type, STYPESIZE, "UINT32 ");
            if(atype == _DATATYPE_INT32)
                n = snprintf(type, STYPESIZE, "INT32  ");
            if(atype == _DATATYPE_UINT64)
                n = snprintf(type, STYPESIZE, "UINT64 ");
            if(atype == _DATATYPE_INT64)
                n = snprintf(type, STYPESIZE, "INT64  ");
            if(atype == _DATATYPE_FLOAT)
                n = snprintf(type, STYPESIZE, "FLOAT  ");            
            if(atype == _DATATYPE_DOUBLE)
                n = snprintf(type, STYPESIZE, "DOUBLE ");
            if(atype == _DATATYPE_COMPLEX_FLOAT)
                n = snprintf(type, STYPESIZE, "CFLOAT ");            
            if(atype == _DATATYPE_COMPLEX_DOUBLE)
                n = snprintf(type, STYPESIZE, "CDOUBLE");


            if(n >= STYPESIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

            fprintf(fp," %s\n",type);
        }
    fclose(fp);

    return(0);
}


int_fast8_t list_variable_ID()
{
    long i;

    for (i=0; i<data.NB_MAX_VARIABLE; i++)
        if(data.variable[i].used == 1)
            printf("%4ld %16s %25.18g\n",i, data.variable[i].name,data.variable[i].value.f);

    return(0);
}


int_fast8_t list_variable_ID_file(const char *fname)
{
    long i;
    FILE *fp;

    fp = fopen(fname, "w");
    for (i=0; i<data.NB_MAX_VARIABLE; i++)
        if(data.variable[i].used == 1)
            fprintf(fp, "%s=%.18g\n",data.variable[i].name, data.variable[i].value.f);

    fclose(fp);

    return(0);
}








/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 8. TYPE CONVERSIONS TO AND FROM COMPLEX                                                         */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




int_fast8_t mk_complex_from_reim(const char *re_name, const char *im_name, const char *out_name, int sharedmem)
{
    long IDre,IDim,IDout;
    uint32_t *naxes = NULL;
    long naxis;
    long nelement;
    long ii;
    long i;
    int n;
    uint8_t atype_re, atype_im, atype_out;

    IDre = image_ID(re_name);
    IDim = image_ID(im_name);

    atype_re = data.image[IDre].md[0].atype;
    atype_im = data.image[IDim].md[0].atype;
    naxis = data.image[IDre].md[0].naxis;

    naxes = (uint32_t *) malloc(sizeof(uint32_t)*naxis);
    if(naxes==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"malloc error");
        exit(0);
    }

    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDre].md[0].size[i];
    nelement = data.image[IDre].md[0].nelement;


    if((atype_re == _DATATYPE_FLOAT)&&(atype_im == _DATATYPE_FLOAT))
    {
        atype_out = _DATATYPE_COMPLEX_FLOAT;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CF[ii].re = data.image[IDre].array.F[ii];
            data.image[IDout].array.CF[ii].im = data.image[IDim].array.F[ii];
        }
    }
    else if((atype_re == _DATATYPE_FLOAT)&&(atype_im == _DATATYPE_DOUBLE))
    {
        atype_out = _DATATYPE_COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CD[ii].re = data.image[IDre].array.F[ii];
            data.image[IDout].array.CD[ii].im = data.image[IDim].array.D[ii];
        }
    }
    else if((atype_re == _DATATYPE_DOUBLE)&&(atype_im == _DATATYPE_FLOAT))
    {
        atype_out = _DATATYPE_COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CD[ii].re = data.image[IDre].array.D[ii];
            data.image[IDout].array.CD[ii].im = data.image[IDim].array.F[ii];
        }
    }
    else if((atype_re == _DATATYPE_DOUBLE)&&(atype_im == _DATATYPE_DOUBLE))
    {
        atype_out = _DATATYPE_COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CD[ii].re = data.image[IDre].array.D[ii];
            data.image[IDout].array.CD[ii].im = data.image[IDim].array.D[ii];
        }
    }
    else
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong image type(s)\n");
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }
    // Note: openMP doesn't help here

    free(naxes);

    return(0);
}





int_fast8_t mk_complex_from_amph(const char *am_name, const char *ph_name, const char *out_name, int sharedmem)
{
    long IDam,IDph,IDout;
    uint32_t naxes[3];
    long naxis;
    long nelement;
    long ii;
    long i;
    uint8_t atype_am, atype_ph, atype_out;
    int n;

    IDam = image_ID(am_name);
    IDph = image_ID(ph_name);
    atype_am = data.image[IDam].md[0].atype;
    atype_ph = data.image[IDph].md[0].atype;

    naxis = data.image[IDam].md[0].naxis;
    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDam].md[0].size[i];
    nelement = data.image[IDam].md[0].nelement;

    if((atype_am == _DATATYPE_FLOAT)&&(atype_ph == _DATATYPE_FLOAT))
    {
        atype_out = _DATATYPE_COMPLEX_FLOAT;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        
        data.image[IDout].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                data.image[IDout].array.CF[ii].re = data.image[IDam].array.F[ii]*((float) cos(data.image[IDph].array.F[ii]));
                data.image[IDout].array.CF[ii].im = data.image[IDam].array.F[ii]*((float) sin(data.image[IDph].array.F[ii]));
            }
# ifdef _OPENMP
        }
# endif
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;

    }
    else if((atype_am == _DATATYPE_FLOAT)&&(atype_ph == _DATATYPE_DOUBLE))
    {
        atype_out = _DATATYPE_COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        data.image[IDout].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                data.image[IDout].array.CD[ii].re = data.image[IDam].array.F[ii]*cos(data.image[IDph].array.D[ii]);
                data.image[IDout].array.CD[ii].im = data.image[IDam].array.F[ii]*sin(data.image[IDph].array.D[ii]);
            }
# ifdef _OPENMP
        }
# endif
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
    }
    else if((atype_am == _DATATYPE_DOUBLE)&&(atype_ph == _DATATYPE_FLOAT))
    {
        atype_out = _DATATYPE_COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        data.image[IDout].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                data.image[IDout].array.CD[ii].re = data.image[IDam].array.D[ii]*cos(data.image[IDph].array.F[ii]);
                data.image[IDout].array.CD[ii].im = data.image[IDam].array.D[ii]*sin(data.image[IDph].array.F[ii]);
            }
# ifdef _OPENMP
        }
# endif
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;

    }
    else if((atype_am == _DATATYPE_DOUBLE)&&(atype_ph == _DATATYPE_DOUBLE))
    {
        atype_out = _DATATYPE_COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes, atype_out, sharedmem, data.NBKEWORD_DFT);
        data.image[IDout].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                data.image[IDout].array.CD[ii].re = data.image[IDam].array.D[ii]*cos(data.image[IDph].array.D[ii]);
                data.image[IDout].array.CD[ii].im = data.image[IDam].array.D[ii]*sin(data.image[IDph].array.D[ii]);
            }
# ifdef _OPENMP
        }
# endif
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
    }
    else
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong image type(s)\n");
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }

    return(0);
}



int_fast8_t mk_reim_from_complex(const char *in_name, const char *re_name, const char *im_name, int sharedmem)
{
    long IDre,IDim,IDin;
    uint32_t naxes[3];
    long naxis;
    long nelement;
    long ii;
    long i;
    uint8_t atype;
    int n;

    IDin = image_ID(in_name);
    atype = data.image[IDin].md[0].atype;
    naxis = data.image[IDin].md[0].naxis;
    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDin].md[0].size[i];
    nelement = data.image[IDin].md[0].nelement;

    if(atype == _DATATYPE_COMPLEX_FLOAT) // single precision
    {
        IDre = create_image_ID(re_name, naxis, naxes, _DATATYPE_FLOAT, sharedmem, data.NBKEWORD_DFT);
        IDim = create_image_ID(im_name, naxis, naxes, _DATATYPE_FLOAT, sharedmem, data.NBKEWORD_DFT);
        
        data.image[IDre].md[0].write = 1;
        data.image[IDim].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                data.image[IDre].array.F[ii] = data.image[IDin].array.CF[ii].re;
                data.image[IDim].array.F[ii] = data.image[IDin].array.CF[ii].im;
            }
# ifdef _OPENMP
        }
# endif
		if(sharedmem==1)
		{
			COREMOD_MEMORY_image_set_sempost_byID(IDre, -1);
			COREMOD_MEMORY_image_set_sempost_byID(IDim, -1);
        }
        data.image[IDre].md[0].cnt0++;
        data.image[IDim].md[0].cnt0++;
        data.image[IDre].md[0].write = 0;
        data.image[IDim].md[0].write = 0;
    }
    else if(atype == _DATATYPE_COMPLEX_DOUBLE) // double precision
    {
        IDre = create_image_ID(re_name, naxis, naxes, _DATATYPE_DOUBLE, sharedmem, data.NBKEWORD_DFT);
        IDim = create_image_ID(im_name, naxis, naxes, _DATATYPE_DOUBLE, sharedmem, data.NBKEWORD_DFT);
        data.image[IDre].md[0].write = 1;
        data.image[IDim].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                data.image[IDre].array.D[ii] = data.image[IDin].array.CD[ii].re;
                data.image[IDim].array.D[ii] = data.image[IDin].array.CD[ii].im;
            }
# ifdef _OPENMP
        }
# endif
		if(sharedmem==1)
		{
			COREMOD_MEMORY_image_set_sempost_byID(IDre, -1);
			COREMOD_MEMORY_image_set_sempost_byID(IDim, -1);
        }
        data.image[IDre].md[0].cnt0++;
        data.image[IDim].md[0].cnt0++;
        data.image[IDre].md[0].write = 0;
        data.image[IDim].md[0].write = 0;

    }
    else
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong image type(s)\n");
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }


    return(0);
}




int_fast8_t mk_amph_from_complex(const char *in_name, const char *am_name, const char *ph_name, int sharedmem)
{
    long IDam,IDph,IDin;
    uint32_t naxes[3];
    long naxis;
    long nelement;
    long ii;
    long i;
    float amp_f,pha_f;
    double amp_d,pha_d;
    uint8_t atype;
    int n;

    IDin = image_ID(in_name);
    atype = data.image[IDin].md[0].atype;
    naxis = data.image[IDin].md[0].naxis;

    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDin].md[0].size[i];
    nelement = data.image[IDin].md[0].nelement;

    if(atype == _DATATYPE_COMPLEX_FLOAT) // single precision
    {
        IDam = create_image_ID(am_name, naxis, naxes,  _DATATYPE_FLOAT, sharedmem, data.NBKEWORD_DFT);
        IDph = create_image_ID(ph_name, naxis, naxes,  _DATATYPE_FLOAT, sharedmem, data.NBKEWORD_DFT);
        data.image[IDam].md[0].write = 1;
        data.image[IDph].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT) private(ii,amp_f,pha_f)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                amp_f = (float) sqrt(data.image[IDin].array.CF[ii].re*data.image[IDin].array.CF[ii].re + data.image[IDin].array.CF[ii].im*data.image[IDin].array.CF[ii].im);
                pha_f = (float) atan2(data.image[IDin].array.CF[ii].im,data.image[IDin].array.CF[ii].re);
                data.image[IDam].array.F[ii] = amp_f;
                data.image[IDph].array.F[ii] = pha_f;
            }
# ifdef _OPENMP
        }
# endif
		if(sharedmem==1)
		{
			COREMOD_MEMORY_image_set_sempost_byID(IDam, -1);
			COREMOD_MEMORY_image_set_sempost_byID(IDph, -1);
        }
        data.image[IDam].md[0].cnt0++;
        data.image[IDph].md[0].cnt0++;
        data.image[IDam].md[0].write = 0;
        data.image[IDph].md[0].write = 0;
    }
    else if(atype == _DATATYPE_COMPLEX_DOUBLE) // double precision
    {
        IDam = create_image_ID(am_name, naxis, naxes, _DATATYPE_DOUBLE, sharedmem, data.NBKEWORD_DFT);
        IDph = create_image_ID(ph_name, naxis, naxes, _DATATYPE_DOUBLE, sharedmem, data.NBKEWORD_DFT);
        data.image[IDam].md[0].write = 1;
        data.image[IDph].md[0].write = 1;
# ifdef _OPENMP
        #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT) private(ii,amp_d,pha_d)
        {
            #pragma omp for
# endif
            for(ii=0; ii<nelement; ii++)
            {
                amp_d = sqrt(data.image[IDin].array.CD[ii].re*data.image[IDin].array.CD[ii].re + data.image[IDin].array.CD[ii].im*data.image[IDin].array.CD[ii].im);
                pha_d = atan2(data.image[IDin].array.CD[ii].im,data.image[IDin].array.CD[ii].re);
                data.image[IDam].array.D[ii] = amp_d;
                data.image[IDph].array.D[ii] = pha_d;
            }
# ifdef _OPENMP
        }
# endif
		if(sharedmem==1)
		{
			COREMOD_MEMORY_image_set_sempost_byID(IDam, -1);
			COREMOD_MEMORY_image_set_sempost_byID(IDph, -1);
        }
        data.image[IDam].md[0].cnt0++;
        data.image[IDph].md[0].cnt0++;
        data.image[IDam].md[0].write = 0;
        data.image[IDph].md[0].write = 0;
    }
    else
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong image type(s)\n");
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }

    return(0);
}




int_fast8_t mk_reim_from_amph(const char *am_name, const char *ph_name, const char *re_out_name, const char *im_out_name, int sharedmem)
{
    mk_complex_from_amph(am_name, ph_name, "Ctmp", 0);
    mk_reim_from_complex("Ctmp", re_out_name, im_out_name, sharedmem);
    delete_image_ID("Ctmp");

    return(0);
}

int_fast8_t mk_amph_from_reim(const char *re_name, const char *im_name, const char *am_out_name, const char *ph_out_name, int sharedmem)
{
    mk_complex_from_reim(re_name, im_name, "Ctmp", 0);
    mk_amph_from_complex("Ctmp",am_out_name, ph_out_name, sharedmem);
    delete_image_ID("Ctmp");

    return(0);
}
















/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 9. VERIFY SIZE                                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */






//  check only is size > 0
int_fast8_t check_2Dsize(const char *ID_name, uint32_t xsize, uint32_t ysize)
{
    int value;
    long ID;

    value = 1;
    ID=image_ID(ID_name);
    if(data.image[ID].md[0].naxis!=2)
        value=0;
    if(value==1)
    {
        if((xsize>0)&&(data.image[ID].md[0].size[0]!=xsize))
            value = 0;
        if((ysize>0)&&(data.image[ID].md[0].size[1]!=ysize))
            value = 0;
    }

    return(value);
}

int_fast8_t check_3Dsize(const char *ID_name, uint32_t xsize, uint32_t ysize, uint32_t zsize)
{
    int value;
    long ID;

    value = 1;
    ID=image_ID(ID_name);
    if(data.image[ID].md[0].naxis!=3)
    {
        /*      printf("Wrong naxis : %ld - should be 3\n",data.image[ID].md[0].naxis);*/
        value = 0;
    }
    if(value==1)
    {
        if((xsize>0)&&(data.image[ID].md[0].size[0]!=xsize))
        {
            /*	  printf("Wrong xsize : %ld - should be %ld\n",data.image[ID].md[0].size[0],xsize);*/
            value = 0;
        }
        if((ysize>0)&&(data.image[ID].md[0].size[1]!=ysize))
        {
            /*	  printf("Wrong ysize : %ld - should be %ld\n",data.image[ID].md[0].size[1],ysize);*/
            value = 0;
        }
        if((zsize>0)&&(data.image[ID].md[0].size[2]!=zsize))
        {
            /*	  printf("Wrong zsize : %ld - should be %ld\n",data.image[ID].md[0].size[2],zsize);*/
            value = 0;
        }
    }
    /*  printf("CHECK = %d\n",value);*/

    return(value);
}







long COREMOD_MEMORY_check_2Dsize(const char *IDname, uint32_t xsize, uint32_t ysize)
{
    int sizeOK = 1; // 1 if size matches
    long ID;


    ID = image_ID(IDname);
    if(data.image[ID].md[0].naxis != 2)
    {
        printf("WARNING : image %s naxis = %d does not match expected value 2\n", IDname, (int) data.image[ID].md[0].naxis);
        sizeOK = 0;
    }
    if((xsize>0)&&(data.image[ID].md[0].size[0] != xsize))
    {
        printf("WARNING : image %s xsize = %d does not match expected value %d\n", IDname, (int) data.image[ID].md[0].size[0], (int) xsize);
        sizeOK = 0;
    }
    if((ysize>0)&&(data.image[ID].md[0].size[1] != ysize))
    {
        printf("WARNING : image %s ysize = %d does not match expected value %d\n", IDname, (int) data.image[ID].md[0].size[1], (int) ysize);
        sizeOK = 0;
    }

    return sizeOK;
}



long COREMOD_MEMORY_check_3Dsize(const char *IDname, uint32_t xsize, uint32_t ysize, uint32_t zsize)
{
    int sizeOK = 1; // 1 if size matches
    long ID;

    ID = image_ID(IDname);
    if(data.image[ID].md[0].naxis != 3)
    {
        printf("WARNING : image %s naxis = %d does not match expected value 3\n", IDname, (int) data.image[ID].md[0].naxis);
        sizeOK = 0;
    }
    if((xsize>0)&&(data.image[ID].md[0].size[0] != xsize))
    {
        printf("WARNING : image %s xsize = %d does not match expected value %d\n", IDname, (int) data.image[ID].md[0].size[0], (int) xsize);
        sizeOK = 0;
    }
    if((ysize>0)&&(data.image[ID].md[0].size[1] != ysize))
    {
        printf("WARNING : image %s ysize = %d does not match expected value %d\n", IDname, (int) data.image[ID].md[0].size[1], (int) ysize);
        sizeOK = 0;
    }
    if((zsize>0)&&(data.image[ID].md[0].size[2] != zsize))
    {
        printf("WARNING : image %s zsize = %d does not match expected value %d\n", IDname, (int) data.image[ID].md[0].size[2], (int) zsize);
        sizeOK = 0;
    }

    return sizeOK;
}





/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 10. COORDINATE CHANGE                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




int_fast8_t rotate_cube(const char *ID_name, const char *ID_out_name, int orientation)
{
    /* 0 is from x axis */
    /* 1 is from y axis */
    long ID,IDout;
    uint32_t xsize, ysize, zsize;
    uint32_t xsize1, ysize1, zsize1;
    uint32_t ii, jj, kk;
    uint8_t atype;
    int n;

    ID = image_ID(ID_name);
    atype = data.image[ID].md[0].atype;

    if(data.image[ID].md[0].naxis!=3)
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong naxis : %d - should be 3\n", (int) data.image[ID].md[0].naxis);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }
    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];
    zsize = data.image[ID].md[0].size[2];

    if(atype == _DATATYPE_FLOAT) // single precision
    {
        if(orientation==0)
        {
            xsize1 = zsize;
            ysize1 = ysize;
            zsize1 = xsize;
            IDout = create_3Dimage_ID_float(ID_out_name,xsize1,ysize1,zsize1);
            for(ii=0; ii<xsize1; ii++)
                for(jj=0; jj<ysize1; jj++)
                    for(kk=0; kk<zsize1; kk++)
                        data.image[IDout].array.F[kk*ysize1*xsize1+jj*xsize1+ii] = data.image[ID].array.F[ii*xsize*ysize+jj*xsize+kk];
        }
        else
        {
            xsize1 = xsize;
            ysize1 = zsize;
            zsize1 = ysize;
            IDout = create_3Dimage_ID_float(ID_out_name,xsize1,ysize1,zsize1);
            for(ii=0; ii<xsize1; ii++)
                for(jj=0; jj<ysize1; jj++)
                    for(kk=0; kk<zsize1; kk++)
                        data.image[IDout].array.F[kk*ysize1*xsize1+jj*xsize1+ii] = data.image[ID].array.F[jj*xsize*ysize+kk*xsize+ii];
        }
    }
    else if(atype == _DATATYPE_DOUBLE)
    {
        if(orientation==0)
        {
            xsize1 = zsize;
            ysize1 = ysize;
            zsize1 = xsize;
            IDout = create_3Dimage_ID_double(ID_out_name,xsize1,ysize1,zsize1);
            for(ii=0; ii<xsize1; ii++)
                for(jj=0; jj<ysize1; jj++)
                    for(kk=0; kk<zsize1; kk++)
                        data.image[IDout].array.D[kk*ysize1*xsize1+jj*xsize1+ii] = data.image[ID].array.D[ii*xsize*ysize+jj*xsize+kk];
        }
        else
        {
            xsize1 = xsize;
            ysize1 = zsize;
            zsize1 = ysize;
            IDout = create_3Dimage_ID_double(ID_out_name,xsize1,ysize1,zsize1);
            for(ii=0; ii<xsize1; ii++)
                for(jj=0; jj<ysize1; jj++)
                    for(kk=0; kk<zsize1; kk++)
                        data.image[IDout].array.D[kk*ysize1*xsize1+jj*xsize1+ii] = data.image[ID].array.D[jj*xsize*ysize+kk*xsize+ii];
        }
    }
    else
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong image type(s)\n");
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }

    return(0);
}










/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 11. SET IMAGE FLAGS / COUNTERS                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */





long COREMOD_MEMORY_image_set_status(const char *IDname, int status)
{
    long ID;

    ID = image_ID(IDname);
    data.image[ID].md[0].status = status;

    return(0);
}

long COREMOD_MEMORY_image_set_cnt0(const char *IDname, int cnt0)
{
    long ID;

    ID = image_ID(IDname);
    data.image[ID].md[0].cnt0 = cnt0;

    return(0);
}

long COREMOD_MEMORY_image_set_cnt1(const char *IDname, int cnt1)
{
    long ID;

    ID = image_ID(IDname);
    data.image[ID].md[0].cnt1 = cnt1;

    return(0);
}







/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 12. MANAGE SEMAPHORES                                                                           */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */





long COREMOD_MEMORY_image_set_createsem(const char *IDname, long NBsem)
{
    long ID;
    char sname[200];
    long s, s1;
    int r;
    char command[200];
    char fname[200];
    int semfile[100];

    ID = image_ID(IDname);

	if(ID != -1)
		ImageStreamIO_createSem(&data.image[ID], NBsem);

    return(ID);
}




// if index < 0, post all semaphores
long COREMOD_MEMORY_image_set_sempost(const char *IDname, long index)
{
    long ID;
    long s;
    int semval;


    ID = image_ID(IDname);

    if(ID==-1)
        ID = read_sharedmem_image(IDname);

    if(index<0)
    {
        for(s=0; s<data.image[ID].md[0].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].md[0].sem-1)
            printf("ERROR: image %s semaphore # %ld does no exist\n", IDname, index);
        else
        {
            sem_getvalue(data.image[ID].semptr[index], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[index]);
        }
    }

    return(ID);
}

//
// if index = -1, post all semaphores
//
long COREMOD_MEMORY_image_set_sempost_byID(long ID, long index)
{
    long s;
    int semval;

    if(index<0)
    {
        for(s=0; s<data.image[ID].md[0].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].md[0].sem-1)
            printf("ERROR: image ID %ld semaphore # %ld does no exist\n", ID, index);
        else
        {
            sem_getvalue(data.image[ID].semptr[index], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[index]);
        }
    }
    
   if(data.image[ID].semlog!=NULL)
    {
        sem_getvalue(data.image[ID].semlog, &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(data.image[ID].semlog);
    }  

    return(ID);
}


//
// post all semaphores except one
//
long COREMOD_MEMORY_image_set_sempost_excl_byID(long ID, long index)
{
    long s;
    int semval;


    for(s=0; s<data.image[ID].md[0].sem; s++)
        {
			if(s!=index)
			{
				sem_getvalue(data.image[ID].semptr[s], &semval);
				if(semval<SEMAPHORE_MAXVAL)
					sem_post(data.image[ID].semptr[s]);
			}
        }
        
   if(data.image[ID].semlog!=NULL)
    {
        sem_getvalue(data.image[ID].semlog, &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(data.image[ID].semlog);
    }

    return(ID);
}





// if index < 0, post all semaphores

long COREMOD_MEMORY_image_set_sempost_loop(const char *IDname, long index, long dtus)
{
    long ID;
    long s;
    int semval;


    ID = image_ID(IDname);

    if(ID==-1)
        ID = read_sharedmem_image(IDname);


	while(1)
	{
    if(index<0)
    {
        for(s=0; s<data.image[ID].md[0].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].md[0].sem-1)
            printf("ERROR: image %s semaphore # %ld does no exist\n", IDname, index);
        else
        {
            sem_getvalue(data.image[ID].semptr[index], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[index]);
        }
    }
    sleep(dtus);
	}
    return(ID);
}



long COREMOD_MEMORY_image_set_semwait(const char *IDname, long index)
{
    long ID;
    int semval;

    ID = image_ID(IDname);

    if(ID==-1)
        ID = read_sharedmem_image(IDname);

    if(index>data.image[ID].md[0].sem-1)
            printf("ERROR: image %s semaphore # %ld does no exist\n", IDname, index);
    else
            sem_wait(data.image[ID].semptr[index]);
        
    return(ID);
}



// only works for sem0
void *waitforsemID(void *ID)
{
    pthread_t tid;
    int t;
    int s;
//    int semval;
    
    
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    tid = pthread_self();


//    sem_getvalue(data.image[(long) ID].semptr, &semval);
//    printf("tid %u waiting for sem ID %ld   sem = %d   (%s)\n", (unsigned int) tid, (long) ID, semval, data.image[(long) ID].name);
//    fflush(stdout);
    sem_wait(data.image[(long) ID].semptr[0]);
//    printf("tid %u sem ID %ld done\n", (unsigned int) tid, (long) ID);
//    fflush(stdout);

    for(t=0; t<NB_thrarray_semwait; t++)
    {
        if(tid!=thrarray_semwait[t])
        {
//            printf("tid %u cancel thread %d tid %u\n", (unsigned int) tid, t, (unsigned int) (thrarray_semwait[t]));
//           fflush(stdout);
            s = pthread_cancel(thrarray_semwait[t]);
        }
    }

    pthread_exit(NULL);
}




/// \brief Wait for multiple images semaphores [OR], only works for sem0 only
long COREMOD_MEMORY_image_set_semwait_OR_IDarray(long *IDarray, long NB_ID)
{
    int t;
    int semval;

 //   printf("======== ENTER COREMOD_MEMORY_image_set_semwait_OR_IDarray [%ld] =======\n", NB_ID);
 //   fflush(stdout);
    
    thrarray_semwait = (pthread_t*) malloc(sizeof(pthread_t)*NB_ID);
    NB_thrarray_semwait = NB_ID;

    for(t = 0; t < NB_ID; t++)
    {
  //      printf("thread %d create, ID = %ld\n", t, IDarray[t]);
  //      fflush(stdout);
        pthread_create(&thrarray_semwait[t], NULL, waitforsemID, (void *)IDarray[t]);
    }

    for(t = 0; t < NB_ID; t++)
    {
   //         printf("thread %d tid %u join waiting\n", t, (unsigned int) thrarray_semwait[t]);
        //fflush(stdout);
            pthread_join(thrarray_semwait[t], NULL);
    //    printf("thread %d tid %u joined\n", t, (unsigned int) thrarray_semwait[t]);
    }

    free(thrarray_semwait);
   // printf("======== EXIT COREMOD_MEMORY_image_set_semwait_OR_IDarray =======\n");
//fflush(stdout);
 
    return(0);
}



/// \brief flush multiple semaphores
long COREMOD_MEMORY_image_set_semflush_IDarray(long *IDarray, long NB_ID)
{
    long i, cnt;
    int semval;
    int s;

    list_image_ID();
    for(i=0; i<NB_ID; i++)
    {
        for(s=0;s<data.image[IDarray[i]].md[0].sem;s++)
        {
            sem_getvalue(data.image[IDarray[i]].semptr[s], &semval);
           printf("sem %d/%d of %s [%ld] = %d\n", s, data.image[IDarray[i]].md[0].sem, data.image[IDarray[i]].name, IDarray[i], semval);
            fflush(stdout); 
            for(cnt=0; cnt<semval; cnt++)
                sem_trywait(data.image[IDarray[i]].semptr[s]);
        }
    }

    return(0);
}



/// set semaphore value to 0
// if index <0, flush all image semaphores
long COREMOD_MEMORY_image_set_semflush(const char *IDname, long index)
{
    long ID;
    int semval;
    long i;
    long s;

    ID = image_ID(IDname);
    if(ID==-1)
        ID = read_sharedmem_image(IDname);

    if(index<0)
    {
        for(s=0; s<data.image[ID].md[0].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            for(i=0; i<semval; i++)
                sem_trywait(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].md[0].sem-1)
            printf("ERROR: image %s semaphore # %ld does not exist\n", IDname, index);
        else
        {
            s = index;
            sem_getvalue(data.image[ID].semptr[s], &semval);
            for(i=0; i<semval; i++)
                sem_trywait(data.image[ID].semptr[s]);

        }
    }

    return(ID);
}

















/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 13. SIMPLE OPERATIONS ON STREAMS                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */




//
// compute difference between two 2D streams
// triggers on stream0
//
long COREMOD_MEMORY_streamDiff(const char *IDstream0_name, const char *IDstream1_name, const char *IDstreammask_name, const char *IDstreamout_name, long semtrig)
{
	long ID0, ID1, IDout;
	uint32_t xsize, ysize, xysize;
	long ii;
	uint32_t *arraysize;
	long long cnt;
	long IDmask; // optional 
	
	ID0 = image_ID(IDstream0_name);
	ID1 = image_ID(IDstream1_name);
	IDmask = image_ID(IDstreammask_name);
	
	xsize = data.image[ID0].md[0].size[0];
	ysize = data.image[ID0].md[0].size[1];	
	xysize = xsize*ysize;
	
	arraysize = (uint32_t*) malloc(sizeof(uint32_t)*2);
	arraysize[0] = xsize;
	arraysize[1] = ysize;
	
	IDout = image_ID(IDstreamout_name);
	if(IDout == -1)
	{
		IDout = create_image_ID(IDstreamout_name, 2, arraysize, _DATATYPE_FLOAT, 1, 0);
		COREMOD_MEMORY_image_set_createsem(IDstreamout_name, 10);
	}

	free(arraysize);
	
	
	while(1)
	{
		// has new frame arrived ?
		if(data.image[ID0].md[0].sem==0)
        {
            while(cnt==data.image[ID0].md[0].cnt0) // test if new frame exists
                usleep(5);
            cnt = data.image[ID0].md[0].cnt0;
        }
        else
            sem_wait(data.image[ID0].semptr[semtrig]);




        data.image[IDout].md[0].write = 1;
		if(IDmask==-1)
		{
			for(ii=0;ii<xysize;ii++)
				data.image[IDout].array.F[ii] = data.image[ID0].array.F[ii] - data.image[ID1].array.F[ii];		        
		}
		else
		{
			for(ii=0;ii<xysize;ii++)
				data.image[IDout].array.F[ii] = (data.image[ID0].array.F[ii] - data.image[ID1].array.F[ii]) * data.image[IDmask].array.F[ii];		  
		}
		COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);;
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
	}
	
	
	return(IDout);
}





//
// compute difference between two halves of an image stream
// triggers on instream
//
long COREMOD_MEMORY_stream_halfimDiff(const char *IDstream_name, const char *IDstreamout_name, long semtrig)
{
	long ID0, IDout;
	uint32_t xsizein, ysizein, xysizein;
	uint32_t xsize, ysize, xysize;
	long ii;
	uint32_t *arraysize;
	long long cnt;
	uint8_t atype;
	
	
	ID0 = image_ID(IDstream_name);
	
	xsizein = data.image[ID0].md[0].size[0];
	ysizein = data.image[ID0].md[0].size[1];	
	xysizein = xsizein*ysizein;
	
	xsize = xsizein;
	ysize = ysizein/2;
	xysize = xsize*ysize;
	
	
	arraysize = (uint32_t*) malloc(sizeof(uint32_t)*2);
	arraysize[0] = xsize;
	arraysize[1] = ysize;
	
	IDout = image_ID(IDstreamout_name);
	if(IDout == -1)
	{
		IDout = create_image_ID(IDstreamout_name, 2, arraysize, _DATATYPE_FLOAT, 1, 0);
		COREMOD_MEMORY_image_set_createsem(IDstreamout_name, 10);
	}

	free(arraysize);
	
	atype = data.image[ID0].md[0].atype;
	
	
	
	while(1)
	{
		// has new frame arrived ?
		if(data.image[ID0].md[0].sem==0)
        {
            while(cnt==data.image[ID0].md[0].cnt0) // test if new frame exists
                usleep(5);
            cnt = data.image[ID0].md[0].cnt0;
        }
        else
            sem_wait(data.image[ID0].semptr[semtrig]);

        data.image[IDout].md[0].write = 1;
        switch ( atype ) {
		case _DATATYPE_UINT16:
			for(ii=0;ii<xysize;ii++)
				data.image[IDout].array.F[ii] = data.image[ID0].array.UI16[ii] - data.image[ID0].array.UI16[xysize+ii];		        
			break;
		case _DATATYPE_FLOAT:
			for(ii=0;ii<xysize;ii++)
				data.image[IDout].array.F[ii] = data.image[ID0].array.F[ii] - data.image[ID0].array.F[xysize+ii];		        
			break;
		}

		COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
	}
	
	
	return(IDout);
}





long COREMOD_MEMORY_streamAve(const char *IDstream_name, int NBave, int mode, const char *IDout_name)
{
	long IDout, IDout0;
	long IDin;
	uint8_t atype;
	uint32_t xsize, ysize, xysize;
	uint32_t *imsize;
	int_fast8_t OKloop;
	int cntin = 0;
	long dtus = 20;
	long ii;
	long cnt0, cnt0old;
	
	IDin = image_ID(IDstream_name);
	atype = data.image[IDin].md[0].atype;
	xsize = data.image[IDin].md[0].size[0];
	ysize = data.image[IDin].md[0].size[1];
	xysize = xsize*ysize;
	

	IDout0 = create_2Dimage_ID("_streamAve_tmp", xsize, ysize);

	if(mode==1) // local image
		IDout = create_2Dimage_ID(IDout_name, xsize, ysize);
	else // shared memory
	{
		IDout = image_ID(IDout_name);
		if(IDout==-1) // CREATE IT
		{
			imsize = (uint32_t*) malloc(sizeof(uint32_t)*2);
			imsize[0] = xsize;
			imsize[1] = ysize;
			IDout = create_image_ID(IDout_name, 2, imsize, _DATATYPE_FLOAT, 1, 0);
			COREMOD_MEMORY_image_set_createsem(IDout_name, 10);
			free(imsize);
		}
    }
    
        
	cntin = 0;
	cnt0old = data.image[IDin].md[0].cnt0;

	for(ii=0;ii<xysize;ii++)
		data.image[IDout].array.F[ii] = 0.0;

	OKloop = 1;
	while(OKloop == 1)
	{
		// has new frame arrived ?	
		cnt0 = data.image[IDin].md[0].cnt0;
		if(cnt0!=cnt0old)
		{
			switch ( atype )
			{
				case _DATATYPE_UINT8 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.UI8[ii];
				break;
				case _DATATYPE_INT8 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.SI8[ii];
				break;
				
				case _DATATYPE_UINT16 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.UI16[ii];
				break;
				case _DATATYPE_INT16 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.SI16[ii];
				break;

				case _DATATYPE_UINT32 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.UI32[ii];
				break;
				case _DATATYPE_INT32 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.SI32[ii];
				break;

				case _DATATYPE_UINT64 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.UI64[ii];
				break;
				case _DATATYPE_INT64 :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.SI64[ii];
				break;

				
				
				case _DATATYPE_FLOAT :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.F[ii];
				break;

				case _DATATYPE_DOUBLE :
				for(ii=0;ii<xysize;ii++)
					data.image[IDout0].array.F[ii] += data.image[IDin].array.D[ii];
				break;


			}

			cntin++;
			if(cntin==NBave)
			{
				cntin = 0;
				data.image[IDout].md[0].write = 1;
				for(ii=0;ii<xysize;ii++)
					data.image[IDout].array.F[ii] = data.image[IDout0].array.F[ii]/NBave;					
				data.image[IDout].md[0].cnt0++;
				data.image[IDout].md[0].write = 0;
				COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);

				if(mode != 1)
				{
					for(ii=0;ii<xysize;ii++)
						data.image[IDout].array.F[ii] = 0.0;
				}
				else
					OKloop = 0;
			}
			cnt0old = cnt0;		
		}
		usleep(dtus);		
	}
	
	delete_image_ID("_streamAve_tmp");
	

	
	
	return(IDout);
}




/** @brief takes a 3Dimage(s) (circular buffer(s)) and writes slices to a 2D image with time interval specified in us
 *
 * 
 * If NBcubes=1, then the circular buffer named IDinname is sent to IDoutname at a frequency of 1/usperiod MHz
 * If NBcubes>1, several circular buffers are used, named ("%S_%03ld", IDinname, cubeindex). Semaphore semtrig of image IDsync_name triggers switch between circular buffers, with a delay of offsetus. The number of consecutive sem posts required to advance to the next circular buffer is period
 * 
 * @param IDinname      Name of DM circular buffer (appended by _000, _001 etc... if NBcubes>1)
 * @param IDoutname     Output DM channel stream
 * @param usperiod      Interval between consecutive frames [us]
 * @param NBcubes       Number of input DM circular buffers
 * @param period        If NBcubes>1: number of input triggers required to advance to next input buffer
 * @param offsetus      If NBcubes>1: time offset [us] between input trigger and input buffer switch
 * @param IDsync_name   If NBcubes>1: Stream used for synchronization
 * @param semtrig       If NBcubes>1: semaphore used for synchronization
 * @param timingmode    Not used
 * 
 *
 */
long COREMOD_MEMORY_image_streamupdateloop(const char *IDinname, const char *IDoutname, long usperiod, long NBcubes, long period, long offsetus, const char *IDsync_name, int semtrig, int timingmode)
{
    long *IDin;
    long cubeindex;
    char imname[200];
    long IDsync;
    long long cntsync;
    long pcnt = 0;
    long offsetfr = 0;
    long offsetfrcnt = 0;
    int cntDelayMode = 0;

    long IDout;
    long kk;
    uint32_t *arraysize;
    long naxis;
    uint8_t atype;
    char *ptr0s; // source start 3D array ptr
    char *ptr0; // source
    char *ptr1; // dest
    long framesize;
    int semval;

    int RT_priority = 80; //any number from 0-99
    struct sched_param schedpar;

    long twait1;
    struct timespec t0;
    struct timespec t1;
    double tdiffv;
    struct timespec tdiff;

    int SyncSlice = 0;




    schedpar.sched_priority = RT_priority;
#ifndef __MACH__
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
#endif


    if(NBcubes<1)
    {
        printf("ERROR: invalid number of input cubes, needs to be >0");
        return(-1);
    }



    IDin = (long*) malloc(sizeof(long)*NBcubes);
    SyncSlice = 0;
    if(NBcubes==1)
    {
        IDin[0] = image_ID(IDinname);

        // in single cube mode, optional sync stream drives updates to next slice within cube
        IDsync = image_ID(IDsync_name);
        if(IDsync!=-1)
            SyncSlice = 1;
    }
    else
    {
        IDsync = image_ID(IDsync_name);

        for(cubeindex=0; cubeindex<NBcubes; cubeindex++)
        {
            sprintf(imname, "%s_%03ld", IDinname, cubeindex);
            IDin[cubeindex] = image_ID(imname);
        }
        offsetfr = (long) ( 0.5 + 1.0*offsetus/usperiod );

        printf("FRAMES OFFSET = %ld\n", offsetfr);
    }

    printf("SyncSlice = %d\n", SyncSlice);

    printf("Creating / connecting to image stream ...\n");
    fflush(stdout);


    naxis = data.image[IDin[0]].md[0].naxis;
    arraysize = (uint32_t*) malloc(sizeof(uint32_t)*3);
    if(naxis != 3)
    {
        printf("ERROR: input image %s should be 3D\n", IDinname);
        exit(0);
    }
    arraysize[0] = data.image[IDin[0]].md[0].size[0];
    arraysize[1] = data.image[IDin[0]].md[0].size[1];
    arraysize[2] = data.image[IDin[0]].md[0].size[2];



    atype = data.image[IDin[0]].md[0].atype;

    IDout = image_ID(IDoutname);
    if(IDout == -1)
    {
        IDout = create_image_ID(IDoutname, 2, arraysize, atype, 1, 0);
        COREMOD_MEMORY_image_set_createsem(IDoutname, 10);
    }

    cubeindex = 0;
    pcnt = 0;
    if(NBcubes>1)
        cntsync = data.image[IDsync].md[0].cnt0;

    twait1 = usperiod;
    kk = 0;
    cntDelayMode = 0;

    for(;;)
    {

        if(NBcubes>1)
        {
            if(cntsync != data.image[IDsync].md[0].cnt0)
            {
                pcnt++;
                cntsync = data.image[IDsync].md[0].cnt0;
            }
            if(pcnt==period)
            {
                pcnt = 0;
                offsetfrcnt = 0;
                cntDelayMode = 1;
            }

            if(cntDelayMode == 1)
            {
                if(offsetfrcnt < offsetfr)
                {
                    offsetfrcnt++;
                }
                else
                {
                    cntDelayMode = 0;
                    cubeindex++;
                    kk = 0;
                }
            }
            if(cubeindex==NBcubes)
                cubeindex = 0;
        }


        switch ( atype ) {

        case _DATATYPE_INT8:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.SI8;
            ptr1 = (char*) data.image[IDout].array.SI8;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_INT8;
            break;

        case _DATATYPE_UINT8:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.UI8;
            ptr1 = (char*) data.image[IDout].array.UI8;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_UINT8;
            break;

        case _DATATYPE_INT16:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.SI16;
            ptr1 = (char*) data.image[IDout].array.SI16;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_INT16;
            break;

        case _DATATYPE_UINT16:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.UI16;
            ptr1 = (char*) data.image[IDout].array.UI16;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_UINT16;
            break;

        case _DATATYPE_INT32:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.SI32;
            ptr1 = (char*) data.image[IDout].array.SI32;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_INT32;
            break;

        case _DATATYPE_UINT32:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.UI32;
            ptr1 = (char*) data.image[IDout].array.UI32;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_UINT32;
            break;

        case _DATATYPE_INT64:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.SI64;
            ptr1 = (char*) data.image[IDout].array.SI64;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_INT64;
            break;

        case _DATATYPE_UINT64:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.UI64;
            ptr1 = (char*) data.image[IDout].array.UI64;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*SIZEOF_DATATYPE_UINT64;
            break;


        case _DATATYPE_FLOAT:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.F;
            ptr1 = (char*) data.image[IDout].array.F;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*sizeof(float);
            break;

        case _DATATYPE_DOUBLE:
            ptr0s = (char*) data.image[IDin[cubeindex]].array.D;
            ptr1 = (char*) data.image[IDout].array.D;
            framesize = data.image[IDin[cubeindex]].md[0].size[0]*data.image[IDin[cubeindex]].md[0].size[1]*sizeof(double);
            break;

        }




        clock_gettime(CLOCK_REALTIME, &t0);

        ptr0 = ptr0s + kk*framesize;
        data.image[IDout].md[0].write = 1;
        memcpy((void *) ptr1, (void *) ptr0, framesize);
        data.image[IDout].md[0].cnt1 = kk;
        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;
        COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);

        kk++;
        if(kk==data.image[IDin[0]].md[0].size[2])
            kk = 0;



        if(SyncSlice==0)
        {
            usleep(twait1);

            clock_gettime(CLOCK_REALTIME, &t1);
            tdiff = info_time_diff(t0, t1);
            tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

            if(tdiffv<1.0e-6*usperiod)
                twait1 ++;
            else
                twait1 --;

            if(twait1<0)
                twait1 = 0;
            if(twait1>usperiod)
                twait1 = usperiod;
        }
        else
        {
            sem_wait(data.image[IDsync].semptr[semtrig]);
        }


    }

    free(IDin);

    return(IDout);
}







// takes a 3Dimage (circular buffer) and writes slices to a 2D image synchronized with an image semaphore
long COREMOD_MEMORY_image_streamupdateloop_semtrig(const char *IDinname, const char *IDoutname, long period, long offsetus, const char *IDsync_name, int semtrig, int timingmode)
{
	long IDin;
    long IDout;
    long IDsync;
    
    long kk, kk1; 
    
    uint32_t *arraysize;
    long naxis;
    uint8_t atype;
    char *ptr0s; // source start 3D array ptr
    char *ptr0; // source
    char *ptr1; // dest
    long framesize;
    int semval;      
    
    int RT_priority = 80; //any number from 0-99
    struct sched_param schedpar;

	
	
	
    schedpar.sched_priority = RT_priority;
    #ifndef __MACH__
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    #endif


    printf("Creating / connecting to image stream ...\n");
    fflush(stdout);

    IDin = image_ID(IDinname);
    naxis = data.image[IDin].md[0].naxis;
    arraysize = (uint32_t*) malloc(sizeof(uint32_t)*3);
    if(naxis != 3)
    {
        printf("ERROR: input image %s should be 3D\n", IDinname);
        exit(0);
    }
    arraysize[0] = data.image[IDin].md[0].size[0];
    arraysize[1] = data.image[IDin].md[0].size[1];
    arraysize[2] = data.image[IDin].md[0].size[2];





    atype = data.image[IDin].md[0].atype;

	IDout = image_ID(IDoutname);
	if(IDout == -1)
	{
		IDout = create_image_ID(IDoutname, 2, arraysize, atype, 1, 0);
		COREMOD_MEMORY_image_set_createsem(IDoutname, 10);
	}
	
    switch ( atype ) {

    case _DATATYPE_INT8:
        ptr0s = (char*) data.image[IDin].array.SI8;
        ptr1 = (char*) data.image[IDout].array.SI8;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_INT8;
        break;

    case _DATATYPE_UINT8:
        ptr0s = (char*) data.image[IDin].array.UI8;
        ptr1 = (char*) data.image[IDout].array.UI8;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_UINT8;
        break;

    case _DATATYPE_INT16:
        ptr0s = (char*) data.image[IDin].array.SI16;
        ptr1 = (char*) data.image[IDout].array.SI16;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_INT16;
        break;

    case _DATATYPE_UINT16:
        ptr0s = (char*) data.image[IDin].array.UI16;
        ptr1 = (char*) data.image[IDout].array.UI16;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_UINT16;
        break;

    case _DATATYPE_INT32:
        ptr0s = (char*) data.image[IDin].array.SI32;
        ptr1 = (char*) data.image[IDout].array.SI32;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_INT32;
        break;

    case _DATATYPE_UINT32:
        ptr0s = (char*) data.image[IDin].array.UI32;
        ptr1 = (char*) data.image[IDout].array.UI32;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_UINT32;
        break;

    case _DATATYPE_INT64:
        ptr0s = (char*) data.image[IDin].array.SI64;
        ptr1 = (char*) data.image[IDout].array.SI64;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_INT64;
        break;

    case _DATATYPE_UINT64:
        ptr0s = (char*) data.image[IDin].array.UI64;
        ptr1 = (char*) data.image[IDout].array.UI64;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*SIZEOF_DATATYPE_UINT64;
        break;


    case _DATATYPE_FLOAT:
        ptr0s = (char*) data.image[IDin].array.F;
        ptr1 = (char*) data.image[IDout].array.F;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(float);
        break;

    case _DATATYPE_DOUBLE:
        ptr0s = (char*) data.image[IDin].array.D;
        ptr1 = (char*) data.image[IDout].array.D;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(double);
        break;
    }




	IDsync = image_ID(IDsync_name);

    kk = 0;
    kk1 = 0;
    
    while(1)
    {				
        sem_wait(data.image[IDsync].semptr[semtrig]);
     
        kk++;
        if(kk==period) // UPDATE
			{
				kk = 0;
				kk1++;
				if(kk1==data.image[IDin].md[0].size[2])
					kk1 = 0;
				usleep(offsetus);
				ptr0 = ptr0s + kk1*framesize;
				data.image[IDout].md[0].write = 1;
				memcpy((void *) ptr1, (void *) ptr0, framesize);
				COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
				data.image[IDout].md[0].cnt0++;
				data.image[IDout].md[0].write = 0;
			}        		
    }

    return(IDout);
}




/** 
 * 
 * IDout_name is a time-delayed copy of IDin_name
 * 
 */ 

long COREMOD_MEMORY_streamDelay(const char *IDin_name, const char *IDout_name, long delayus, long dtus)
{
	long IDimc;
	long IDin, IDout;
	uint32_t xsize, ysize, xysize;
	long zsize;
	long kkin;
	long cnt0, cnt0old;
	long ii;
	struct timespec *t0array;
	struct timespec tnow;
	double tdiffv;
	struct timespec tdiff;
	uint32_t *arraytmp;
	long cntskip = 0;
	long kkout;
	long kk;
	  
	IDin = image_ID(IDin_name);
	xsize = data.image[IDin].md[0].size[0];
	ysize = data.image[IDin].md[0].size[1];
	zsize = (long) (delayus/dtus);
	if(zsize<1)
		zsize = 1;
	xysize = xsize*ysize;
	
	t0array = (struct timespec*) malloc(sizeof(struct timespec)*zsize);
	
	IDimc = create_3Dimage_ID("_tmpc", xsize, ysize, zsize);
	
	list_image_ID();
	
	IDout = image_ID(IDout_name);
    if(IDout==-1) // CREATE IT
    {
		arraytmp = (uint32_t*) malloc(sizeof(uint32_t)*2);
		arraytmp[0] = xsize;
		arraytmp[1] = ysize;
        IDout = create_image_ID(IDout_name, 2, arraytmp, _DATATYPE_FLOAT, 1, 0);
        COREMOD_MEMORY_image_set_createsem(IDout_name, 10);
		free(arraytmp);
    }
    
    
	kkin = 0;
	kkout = 0;
	cnt0old = data.image[IDin].md[0].cnt0;

	clock_gettime(CLOCK_REALTIME, &tnow);
	for(kk=0;kk<zsize;kk++)
		t0array[kk] = tnow;
		
	
	while(1)
	{
		// has new frame arrived ?
		cnt0 = data.image[IDin].md[0].cnt0;
		if(cnt0!=cnt0old)
		{
			clock_gettime(CLOCK_REALTIME, &t0array[kkin]);
			for(ii=0;ii<xysize;ii++)
				data.image[IDimc].array.F[kkin*xysize+ii] = data.image[IDin].array.F[ii];
			kkin++;
			if(kkin==zsize)
				kkin = 0;
			cnt0old = cnt0;		
			printf("New frame detected: %ld  ->  %ld\n", cnt0, kkin);
			fflush(stdout);
		}
		
		clock_gettime(CLOCK_REALTIME, &tnow);
		
		
		cntskip = 0;
		tdiff = info_time_diff(t0array[kkout], tnow);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
		
	//	printf("tdiff = %f us   ", tdiffv*1e6);
	//	fflush(stdout);
		while((tdiffv>1.0e-6*delayus)&&(cntskip<zsize))
			{
				cntskip++;				
				kkout++;
				if(kkout==zsize)
					kkout = 0;
				tdiff = info_time_diff(t0array[kkout], tnow);
				tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
			}
	//	printf("cntskip = %ld\n", cntskip);
	//	fflush(stdout);
		
		if(cntskip>0)
		{
		//	printf("Updating %s  %ld\n", IDout_name, IDout);
			
			data.image[IDout].md[0].write = 1;
			for(ii=0;ii<xysize;ii++)
				data.image[IDout].array.F[ii] = data.image[IDimc].array.F[kkout*xysize+ii];	
			
			COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);;
			data.image[IDout].md[0].cnt0++;
			data.image[IDout].md[0].write = 0;
		}
		
	
		usleep(dtus);
	}
	
	delete_image_ID("_tmpc");
	
	free(t0array);
	
	return(0);
}





//
// save all current images/stream onto file
//
long COREMOD_MEMORY_SaveAll_snapshot(const char *dirname)
{
	long *IDarray;
	long *IDarraycp;
	long i;
	long imcnt = 0;
	char imnamecp[200];
	char fnamecp[500];
	long ID;
	char command[500];
	int ret;
	
	
	for (i=0; i<data.NB_MAX_IMAGE; i++)
       if(data.image[i].used==1)
		imcnt++;
    
    IDarray = (long*) malloc(sizeof(long)*imcnt);
    IDarraycp = (long*) malloc(sizeof(long)*imcnt);
    
    imcnt = 0;
    for (i=0; i<data.NB_MAX_IMAGE; i++)
       if(data.image[i].used==1)
		{
			IDarray[imcnt] = i;
			imcnt++;
		}
		
	
	sprintf(command, "mkdir -p %s", dirname);
	ret = system(command);
	
	// create array for each image
	for(i=0;i<imcnt;i++)
		{
			ID = IDarray[i];
			sprintf(imnamecp, "%s_cp", data.image[ID].name); 
			//printf("image %s\n", data.image[ID].name);
			IDarraycp[i] = copy_image_ID(data.image[ID].name, imnamecp, 0);
		}
	
	list_image_ID();
	
	for(i=0;i<imcnt;i++)
		{
			ID = IDarray[i];
			sprintf(imnamecp, "%s_cp", data.image[ID].name);
			sprintf(fnamecp, "!./%s/%s.fits", dirname, data.image[ID].name);
			save_fits(imnamecp, fnamecp);
		}
		
    free(IDarray);
    free(IDarraycp);
   
    
	return(0);
}



//
// save all current images/stream onto file
// only saves 2D float streams into 3D cubes
//
long COREMOD_MEMORY_SaveAll_sequ(const char *dirname, const char *IDtrig_name, long semtrig, long NBframes)
{
	long *IDarray;
	long *IDarrayout;
	long i;
	long imcnt = 0;
	char imnameout[200];
	char fnameout[500];
	long ID;
	char command[500];
	int ret;
	long IDtrig;
	
	long frame = 0;
	char *ptr0;
	char *ptr1;
	uint32_t *imsizearray;
	
	
	
	
	for (i=0; i<data.NB_MAX_IMAGE; i++)
       if(data.image[i].used==1)
		imcnt++;
    
    IDarray = (long*) malloc(sizeof(long)*imcnt);
    IDarrayout = (long*) malloc(sizeof(long)*imcnt);
    
    imcnt = 0;
    for (i=0; i<data.NB_MAX_IMAGE; i++)
       if(data.image[i].used==1)
		{
			IDarray[imcnt] = i;
			imcnt++;
		}
	imsizearray = (uint32_t*) malloc(sizeof(uint32_t)*imcnt);	
	
	
	
	sprintf(command, "mkdir -p %s", dirname);
	ret = system(command);
	
	IDtrig = image_ID(IDtrig_name);


	printf("Creating arrays\n");
	fflush(stdout);
	
	// create 3D arrays	
	for(i=0;i<imcnt;i++)
	{	
		sprintf(imnameout, "%s_out", data.image[IDarray[i]].name); 
		imsizearray[i] = sizeof(float)*data.image[IDarray[i]].md[0].size[0]*data.image[IDarray[i]].md[0].size[1];
		printf("Creating image %s  size %d x %d x %ld\n", imnameout, data.image[IDarray[i]].md[0].size[0], data.image[IDarray[i]].md[0].size[1], NBframes);
		fflush(stdout);
		IDarrayout[i] = create_3Dimage_ID(imnameout, data.image[IDarray[i]].md[0].size[0], data.image[IDarray[i]].md[0].size[1], NBframes);
	}
	list_image_ID();
	
	printf("filling arrays\n");
	fflush(stdout);
	
	// drive semaphore to zero
	while(sem_trywait(data.image[IDtrig].semptr[semtrig])==0) {}
	
	frame = 0;
	while ( frame < NBframes )
	{
		sem_wait(data.image[IDtrig].semptr[semtrig]);
		for(i=0;i<imcnt;i++)
			{
				ID = IDarray[i];
				ptr0 = (char*) data.image[IDarrayout[i]].array.F;
				ptr1 = ptr0 + imsizearray[i]*frame;
				memcpy(ptr1, data.image[ID].array.F, imsizearray[i]);
			}
		frame++;
	}
	
	
	printf("Saving images\n");
	fflush(stdout);
	
	list_image_ID();
	

	for(i=0;i<imcnt;i++)
		{
			ID = IDarray[i];
			sprintf(imnameout, "%s_out", data.image[ID].name);
			sprintf(fnameout, "!./%s/%s_out.fits", dirname, data.image[ID].name);
			save_fits(imnameout, fnameout);
		}
		
    free(IDarray);
    free(IDarrayout);
	free(imsizearray);
    
	return(0);
}

















/** continuously transmits 2D image through TCP link
 * mode = 1, force counter to be used for synchronization, ignore semaphores if they exist
 */


long COREMOD_MEMORY_image_NETWORKtransmit(const char *IDname, const char *IPaddr, int port, int mode, int RT_priority)
{
    long ID;
    struct sockaddr_in sock_server;
    int fds_client;
    int flag = 1;
    int result;
    long long cnt = -1;
    long long iter = 0;
    long framesize; // pixel data only
    uint32_t xsize, ysize;
    char *ptr0; // source
    char *ptr1; // source - offset by slice
    int rs;
    int sockOK;
   
    struct sched_param schedpar;
    struct timespec ts;
    long scnt;
    int semval;
    int semr;
    int slice, oldslice;
    int NBslices;
    TCP_BUFFER_METADATA *frame_md;
    long framesize1; // pixel data + metadata
    char *buff; // transmit buffer






    schedpar.sched_priority = RT_priority;
    #ifndef __MACH__
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    #endif

    ID = image_ID(IDname);

    if ( (fds_client=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        printf("ERROR creating socket\n");
        exit(0);
    }

    result = setsockopt(fds_client,            /* socket affected */
                        IPPROTO_TCP,     /* set option at TCP level */
                        TCP_NODELAY,     /* name of option */
                        (char *) &flag,  /* the cast is historical cruft */
                        sizeof(int));    /* length of option value */
    if (result < 0)
    {
        printf("ERROR setsockopt\n");
        exit(0);
    }


    memset((char *) &sock_server, 0, sizeof(sock_server));
    sock_server.sin_family = AF_INET;
    sock_server.sin_port = htons(port);
    sock_server.sin_addr.s_addr = inet_addr(IPaddr);

    if (connect(fds_client, (struct sockaddr *) &sock_server, sizeof(sock_server)) < 0)
    {
        perror("Error  connect() failed ");
        printf("port = %d\n", port);
        exit(0);
    }

    if (send(fds_client, (void *) data.image[ID].md, sizeof(IMAGE_METADATA), 0) != sizeof(IMAGE_METADATA))
    {
        printf("send() sent a different number of bytes than expected %ld\n", sizeof(IMAGE_METADATA));
        fflush(stdout);
    }



    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];
    NBslices = 1;
    if(data.image[ID].md[0].naxis>2)
        if(data.image[ID].md[0].size[2]>1)
            NBslices = data.image[ID].md[0].size[2];
            

    switch ( data.image[ID].md[0].atype ) {


    case _DATATYPE_INT8:
        framesize = SIZEOF_DATATYPE_INT8*xsize*ysize;
        break;
    case _DATATYPE_UINT8:
        framesize = SIZEOF_DATATYPE_UINT8*xsize*ysize;
        break;
        
    case _DATATYPE_INT16:
        framesize = SIZEOF_DATATYPE_INT16*xsize*ysize;
        break;
    case _DATATYPE_UINT16:
        framesize = SIZEOF_DATATYPE_UINT16*xsize*ysize;
        break;

    case _DATATYPE_INT32:
        framesize = SIZEOF_DATATYPE_INT32*xsize*ysize;
        break;
    case _DATATYPE_UINT32:
        framesize = SIZEOF_DATATYPE_UINT32*xsize*ysize;
        break;

    case _DATATYPE_INT64:
        framesize = SIZEOF_DATATYPE_INT64*xsize*ysize;
        break;
    case _DATATYPE_UINT64:
        framesize = SIZEOF_DATATYPE_UINT64*xsize*ysize;
        break;

    case _DATATYPE_FLOAT:
        framesize = SIZEOF_DATATYPE_FLOAT*xsize*ysize;
        break;
    case _DATATYPE_DOUBLE:
        framesize = SIZEOF_DATATYPE_DOUBLE*xsize*ysize;
        break;
 

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }

    printf("IMAGE FRAME SIZE = %ld\n", framesize);

    switch ( data.image[ID].md[0].atype ) {

    case _DATATYPE_INT8:
        ptr0 = (char*) data.image[ID].array.SI8;
        break;
    case _DATATYPE_UINT8:
        ptr0 = (char*) data.image[ID].array.UI8;
        break;

    case _DATATYPE_INT16:
        ptr0 = (char*) data.image[ID].array.SI16;
        break;
    case _DATATYPE_UINT16:
        ptr0 = (char*) data.image[ID].array.UI16;
        break;

    case _DATATYPE_INT32:
        ptr0 = (char*) data.image[ID].array.SI32;
        break;
    case _DATATYPE_UINT32:
        ptr0 = (char*) data.image[ID].array.UI32;
        break;

    case _DATATYPE_INT64:
        ptr0 = (char*) data.image[ID].array.SI64;
        break;
    case _DATATYPE_UINT64:
        ptr0 = (char*) data.image[ID].array.UI64;
        break;

    case _DATATYPE_FLOAT:
        ptr0 = (char*) data.image[ID].array.F;
        break;
    case _DATATYPE_DOUBLE:
        ptr0 = (char*) data.image[ID].array.D;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }


    if (sigaction(SIGINT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGBUS, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPIPE, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }


    frame_md = (TCP_BUFFER_METADATA*) malloc(sizeof(TCP_BUFFER_METADATA));
    framesize1 = framesize + sizeof(TCP_BUFFER_METADATA);
    buff = (char*) malloc(sizeof(char)*framesize1);

    oldslice = 0;
    sockOK = 1;
    printf("sem = %d\n", data.image[ID].md[0].sem);
    fflush(stdout);
    
    while(sockOK==1)
    {
        if((data.image[ID].md[0].sem==0)||(mode==1))
        {
            while(data.image[ID].md[0].cnt0==cnt) // test if new frame exists
                usleep(5);
            cnt = data.image[ID].md[0].cnt0;
            semr = 0;
        }
        else
        {
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
            ts.tv_sec += 1;
            
            #ifndef __MACH__
            semr = sem_timedwait(data.image[ID].semptr[0], &ts);
			#else
			alarm(1);
			semr = sem_wait(data.image[ID].semptr[0]);
			#endif
			
            if(iter == 0)
            {
                printf("driving semaphore to zero ... ");
                fflush(stdout);
                sem_getvalue(data.image[ID].semptr[0], &semval);
                for(scnt=0; scnt<semval; scnt++)
                    sem_trywait(data.image[ID].semptr[0]);
                printf("done\n");
                fflush(stdout);
            }
        }
        
        if(semr==0)
        {
            frame_md[0].cnt0 = data.image[ID].md[0].cnt0;
            frame_md[0].cnt1 = data.image[ID].md[0].cnt1;
            /*printf("counters    %8ld  %8ld\n", frame_md[0].cnt0, frame_md[0].cnt1); //TEST
            fflush(stdout);
           */ 
            slice = data.image[ID].md[0].cnt1;
            if(slice>oldslice+1)
                slice = oldslice+1;
            if(NBslices>1)
                if(oldslice==NBslices-1)
                    slice = 0;;

       //     printf("[%ld -> %ld] ", oldslice, slice); // TEST
            frame_md[0].cnt1 = slice;
         /*   if(slice == 0)
            {
                printf("\n");
                fflush(stdout);
            }*/

            ptr1 = ptr0 + framesize*slice; //data.image[ID].md[0].cnt1; // frame that was just written
            memcpy(buff, ptr1, framesize);
            
            memcpy(buff+framesize, frame_md, sizeof(TCP_BUFFER_METADATA));

            rs = send(fds_client, buff, framesize1, 0);

            if ( rs != framesize1)
            {
				perror("socket send error ");
                printf("send() sent a different number of bytes (%d) than expected %ld  %ld  %ld\n", rs, (long) framesize, (long) framesize1, (long) sizeof(TCP_BUFFER_METADATA));
                fflush(stdout);
                
                sockOK = 0;
            }
            oldslice = slice;
        }
       /* else//TEST
            {
                printf("semr = %d\n", semr);
                fflush(stdout);
            }*/
        
        if((data.signal_INT == 1)||(data.signal_TERM == 1)||(data.signal_ABRT==1)||(data.signal_BUS==1)||(data.signal_SEGV==1)||(data.signal_HUP==1)||(data.signal_PIPE==1))
            sockOK = 0;


        iter++;
    }

    free(buff);
	
    close(fds_client);
    printf("port %d closed\n", port);
    fflush(stdout);
    
	free(frame_md);

    return(ID);
}









long COREMOD_MEMORY_image_NETWORKreceive(int port, int mode, int RT_priority)
{
    struct sockaddr_in sock_server, sock_client;
    int fds_server, fds_client;
    socklen_t slen_client;
    long cnt;
    int flag = 1;
    long recvsize;
    int result;
    long totsize = 0;
    int MAXPENDING = 5;
    IMAGE_METADATA *imgmd;
    long ID;
    long framesize;
    uint32_t xsize, ysize;
    char *ptr0; // source
    char fname[200];
    long NBslices;
    int socketOpen = 1; // 0 if socket is closed
    int semval;
    int semnb;
	int OKim;
	int axis;
	
	
    imgmd = (IMAGE_METADATA*) malloc(sizeof(IMAGE_METADATA));

    TCP_BUFFER_METADATA *frame_md;
    long framesize1; // pixel data + metadata
    char *buff; // buffer
   


    struct sched_param schedpar;
    

    schedpar.sched_priority = RT_priority;
    #ifndef __MACH__
    // r = seteuid(euid_called); //This goes up to maximum privileges
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    // r = seteuid(euid_real);//Go back to normal privileges
    #endif

    // create TCP socket
    if((fds_server=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
    {
        printf("ERROR creating socket\n");
        exit(0);
    }


    memset((char*) &sock_server, 0, sizeof(sock_server));

    result = setsockopt(fds_server,            /* socket affected */
                        IPPROTO_TCP,     /* set option at TCP level */
                        TCP_NODELAY,     /* name of option */
                        (char *) &flag,  /* the cast is historical cruft */
                        sizeof(int));    /* length of option value */
    if (result < 0)
    {
        printf("ERROR setsockopt\n");
        exit(0);
    }


    sock_server.sin_family = AF_INET;
    sock_server.sin_port = htons(port);
    sock_server.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(fds_server , (struct sockaddr*)&sock_server, sizeof(sock_server) ) == -1)
    {
        printf("ERROR binding socket, port %d\n", port);
        exit(0);
    }


    if (listen(fds_server, MAXPENDING) < 0)
    {
        printf("ERROR listen socket\n");
        exit(0);
    }

    cnt = 0;

    /* Set the size of the in-out parameter */
    slen_client = sizeof(sock_client);

    /* Wait for a client to connect */
    if ((fds_client = accept(fds_server, (struct sockaddr *) &sock_client, &slen_client)) == -1)
    {
        printf("ERROR accept socket\n");
        exit(0);
    }

    printf("Client connected\n");
    fflush(stdout);

    // listen for image metadata
    if((recvsize = recv(fds_client, imgmd, sizeof(IMAGE_METADATA), MSG_WAITALL)) < 0)
    {
        printf("ERROR receiving image metadata\n");
        exit(0);
    }


	// is image already in memory ?
	OKim = 0;
	
	ID = image_ID(imgmd[0].name);
	if(ID==-1)
	{
		// is it in shared memory ?
		ID = read_sharedmem_image(imgmd[0].name);
	}	
	
	list_image_ID();
	
	if(ID == -1)
		OKim = 0;
	else
	{
		OKim = 1;
		if(imgmd[0].naxis != data.image[ID].md[0].naxis)
			OKim = 0;
		if(OKim==1)
			{
				for(axis=0;axis<imgmd[0].naxis;axis++)
					if(imgmd[0].size[axis] != data.image[ID].md[0].size[axis])
						OKim = 0;
			}
		if(imgmd[0].atype != data.image[ID].md[0].atype)
			OKim = 0;
			
		if(OKim==0)
			{
				delete_image_ID(imgmd[0].name);
				ID = -1;
			}
	}
	


	if(OKim==0)
	{
		printf("IMAGE %s HAS TO BE CREATED\n", imgmd[0].name);
		ID = create_image_ID(imgmd[0].name, imgmd[0].naxis, imgmd[0].size, imgmd[0].atype, imgmd[0].shared, 0);
		printf("Created image stream %s - shared = %d\n", imgmd[0].name, imgmd[0].shared);
    }
    else
		printf("REUSING EXISTING IMAGE %s\n", imgmd[0].name);
    
    
	COREMOD_MEMORY_image_set_createsem(imgmd[0].name, 10);

    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];
    NBslices = 1;
    if(data.image[ID].md[0].naxis>2)
        if(data.image[ID].md[0].size[2]>1)
            NBslices = data.image[ID].md[0].size[2];

    switch ( data.image[ID].md[0].atype ) {

    case _DATATYPE_INT8:
        framesize = SIZEOF_DATATYPE_INT8*xsize*ysize;
        break;

    case _DATATYPE_UINT8:
        framesize = SIZEOF_DATATYPE_UINT8*xsize*ysize;
        break;

    case _DATATYPE_INT16:
        framesize = SIZEOF_DATATYPE_INT16*xsize*ysize;
        break;

    case _DATATYPE_UINT16:
        framesize = SIZEOF_DATATYPE_UINT16*xsize*ysize;
        break;

    case _DATATYPE_INT32:
        framesize = SIZEOF_DATATYPE_INT32*xsize*ysize;
        break;

    case _DATATYPE_UINT32:
        framesize = SIZEOF_DATATYPE_UINT32*xsize*ysize;
        break;

    case _DATATYPE_INT64:
        framesize = SIZEOF_DATATYPE_INT64*xsize*ysize;
        break;

    case _DATATYPE_UINT64:
        framesize = SIZEOF_DATATYPE_UINT64*xsize*ysize;
        break;

    case _DATATYPE_FLOAT:
        framesize = SIZEOF_DATATYPE_FLOAT*xsize*ysize;
        break;

    case _DATATYPE_DOUBLE:
        framesize = SIZEOF_DATATYPE_DOUBLE*xsize*ysize;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }

    printf("image frame size = %ld\n", framesize);

    switch ( data.image[ID].md[0].atype ) {

    case _DATATYPE_INT8:
        ptr0 = (char*) data.image[ID].array.SI8;
        break;
    case _DATATYPE_UINT8:
        ptr0 = (char*) data.image[ID].array.UI8;
        break;

    case _DATATYPE_INT16:
        ptr0 = (char*) data.image[ID].array.SI16;
        break;
    case _DATATYPE_UINT16:
        ptr0 = (char*) data.image[ID].array.UI16;
        break;

    case _DATATYPE_INT32:
        ptr0 = (char*) data.image[ID].array.SI32;
        break;
    case _DATATYPE_UINT32:
        ptr0 = (char*) data.image[ID].array.UI32;
        break;

    case _DATATYPE_INT64:
        ptr0 = (char*) data.image[ID].array.SI64;
        break;
    case _DATATYPE_UINT64:
        ptr0 = (char*) data.image[ID].array.UI64;
        break;

    case _DATATYPE_FLOAT:
        ptr0 = (char*) data.image[ID].array.F;
        break;
    case _DATATYPE_DOUBLE:
        ptr0 = (char*) data.image[ID].array.D;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }


    if (sigaction(SIGINT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGBUS, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
   if (sigaction(SIGABRT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
  if (sigaction(SIGHUP, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
  if (sigaction(SIGPIPE, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // this line is not needed, as frame_md is declared below
    // frame_md = (TCP_BUFFER_METADATA*) malloc(sizeof(TCP_BUFFER_METADATA));
    
    framesize1 = framesize + sizeof(TCP_BUFFER_METADATA);
    buff = (char*) malloc(sizeof(char)*framesize1);

    frame_md = (TCP_BUFFER_METADATA*) (buff + framesize);

    socketOpen = 1;
    while(socketOpen==1)
    {
        if ((recvsize = recv(fds_client, buff, framesize1, MSG_WAITALL)) < 0)
        {
            printf("ERROR recv()\n");
            socketOpen = 0;
        }

        if(recvsize!=0)
        {
            totsize += recvsize;
        }
        else
            socketOpen = 0;
        
        if(socketOpen==1)
            {
                frame_md = (TCP_BUFFER_METADATA*) (buff + framesize);

            
                data.image[ID].md[0].cnt1 = frame_md[0].cnt1;
                                    

           
                if(NBslices>1)
                    memcpy(ptr0+framesize*frame_md[0].cnt1, buff, framesize);
                else
                     memcpy(ptr0, buff, framesize);
                data.image[ID].md[0].cnt0++;
                for(semnb=0;semnb<data.image[ID].md[0].sem ; semnb++)
                {
                    sem_getvalue(data.image[ID].semptr[semnb], &semval);
                    if(semval<SEMAPHORE_MAXVAL)
                        sem_post(data.image[ID].semptr[semnb]);
                }
                
            }
        if((data.signal_INT == 1)||(data.signal_TERM == 1)||(data.signal_ABRT==1)||(data.signal_BUS==1)||(data.signal_SEGV==1)||(data.signal_HUP==1)||(data.signal_PIPE==1))
            socketOpen = 0;
    }
    
    
    free(buff);

    close(fds_client);

    printf("port %d closed\n", port);
    fflush(stdout);

    free(imgmd);


    return(ID);
}






//
// pixel decode for unsigned short
// sem0, cnt0 gets updated at each full frame
// sem1 gets updated for each slice
// cnt1 contains the slice index that was just written
//
long COREMOD_MEMORY_PixMapDecode_U(const char *inputstream_name, uint32_t xsizeim, uint32_t ysizeim, const char* NBpix_fname, const char* IDmap_name, const char *IDout_name, const char *IDout_pixslice_fname)
{
    long IDout = -1;
    long IDin;
    long IDmap;
    long slice, sliceii;
    long oldslice = 0;
    long NBslice;
    long *nbpixslice;
    uint32_t xsizein, ysizein;
    FILE *fp;
    uint32_t *sizearray;
    long IDout_pixslice;
    int loopOK;
    long ii;
    long cnt = 0;
//    int RT_priority = 80; //any number from 0-99

    struct sched_param schedpar;
    struct timespec ts;
    long scnt;
    int semval;
    long long iter;
    int r;
    long tmpl0, tmpl1;
    int semr;

    double *dtarray;
    struct timespec *tarray;
    long slice1;




    sizearray = (uint32_t*) malloc(sizeof(uint32_t)*3);

    IDin = image_ID(inputstream_name);
    IDmap = image_ID(IDmap_name);

    xsizein = data.image[IDin].md[0].size[0];
    ysizein = data.image[IDin].md[0].size[1];

    if(xsizein != data.image[IDmap].md[0].size[0])
    {
        printf("ERROR: xsize for %s (%d) does not match xsize for %s (%d)\n", inputstream_name, xsizein, IDmap_name, data.image[IDmap].md[0].size[0]);
        exit(0);
    }
    if(ysizein != data.image[IDmap].md[0].size[1])
    {
        printf("ERROR: xsize for %s (%d) does not match xsize for %s (%d)\n", inputstream_name, ysizein, IDmap_name, data.image[IDmap].md[0].size[1]);
        exit(0);
    }
    sizearray[0] = xsizeim;
    sizearray[1] = ysizeim;
    IDout = create_image_ID(IDout_name, 2, sizearray, data.image[IDin].md[0].atype, 1, 0);
    COREMOD_MEMORY_image_set_createsem(IDout_name, 10); // create 10 semaphores
    IDout_pixslice = create_image_ID("outpixsl", 2, sizearray, _DATATYPE_UINT16, 0, 0);

    NBslice = data.image[IDin].md[0].size[2];

    dtarray = (double*) malloc(sizeof(double)*NBslice);
    tarray = (struct timespec *) malloc(sizeof(struct timespec)*NBslice);


    nbpixslice = (long*) malloc(sizeof(long)*NBslice);
    if((fp=fopen(NBpix_fname,"r"))==NULL)
    {
        printf("ERROR : cannot open file \"%s\"\n", NBpix_fname);
        exit(0);
    }

    for(slice=0; slice<NBslice; slice++)
        r = fscanf(fp, "%ld %ld %ld\n", &tmpl0, &nbpixslice[slice], &tmpl1);
    fclose(fp);

    for(slice=0; slice<NBslice; slice++)
        printf("Slice %5ld   : %5ld pix\n", slice, nbpixslice[slice]);




    for(slice=0; slice<NBslice; slice++)
    {
        sliceii = slice*data.image[IDmap].md[0].size[0]*data.image[IDmap].md[0].size[1];
        for(ii=0; ii<nbpixslice[slice]; ii++)
            data.image[IDout_pixslice].array.UI16[ data.image[IDmap].array.UI16[sliceii + ii] ] = (unsigned short) slice;
    }

    save_fits("outpixsl", IDout_pixslice_fname);
    delete_image_ID("outpixsl");

    if (sigaction(SIGINT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGBUS, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPIPE, &data.sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }


    iter = 0;
    loopOK = 1;
    while(loopOK == 1)
    {
        if(data.image[IDin].md[0].sem==0)
        {
            while(data.image[IDin].md[0].cnt0==cnt) // test if new frame exists
                usleep(5);
            cnt = data.image[IDin].md[0].cnt0;
        }
        else
        {
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
            ts.tv_sec += 1;
            #ifndef __MACH__
            semr = sem_timedwait(data.image[IDin].semptr[0], &ts);
            #else
            alarm(1);
            semr = sem_wait(data.image[IDin].semptr[0]);
            #endif

            if(iter == 0)
            {
                sem_getvalue(data.image[IDin].semptr[0], &semval);
                for(scnt=0; scnt<semval; scnt++)
                    sem_trywait(data.image[IDin].semptr[0]);
            }
        }

        if(semr==0)
        {
            slice = data.image[IDin].md[0].cnt1;
            if(slice>oldslice+1)
                slice = oldslice+1;

            if(oldslice==NBslice-1)
                slice = 0;

            //   clock_gettime(CLOCK_REALTIME, &tarray[slice]);
            //  dtarray[slice] = 1.0*tarray[slice].tv_sec + 1.0e-9*tarray[slice].tv_nsec;
            data.image[IDout].md[0].write = 1;

            if(slice<NBslice)
            {
                sliceii = slice*data.image[IDmap].md[0].size[0]*data.image[IDmap].md[0].size[1];
                for(ii=0; ii<nbpixslice[slice]; ii++)
                    data.image[IDout].array.UI16[data.image[IDmap].array.UI16[sliceii + ii] ] = data.image[IDin].array.UI16[sliceii + ii];
            }
            //     printf("[%ld] ", slice); //TEST

            if(slice==NBslice-1)   //if(slice<oldslice)
            {
				COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
				
/*                sem_getvalue(data.image[IDout].semptr[0], &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDout].semptr[0]);

                sem_getvalue(data.image[IDout].semptr[1], &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDout].semptr[1]);

                sem_getvalue(data.image[IDout].semptr[4], &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDout].semptr[4]);

                sem_getvalue(data.image[IDout].semptr[5], &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDout].semptr[5]);

                sem_getvalue(data.image[IDout].semlog, &semval);
                if(semval<SEMAPHORE_MAXVAL)
                    sem_post(data.image[IDout].semlog);
  */           
                data.image[IDout].md[0].cnt0 ++;

                //     printf("[[ Timimg [us] :   ");
                //  for(slice1=1;slice1<NBslice;slice1++)
                //      {
                //              dtarray[slice1] -= dtarray[0];
                //           printf("%6ld ", (long) (1.0e6*dtarray[slice1]));
                //      }
                // printf("]]");
                //  printf("\n");//TEST
                // fflush(stdout);
            }

            data.image[IDout].md[0].cnt1 = slice;
            
            sem_getvalue(data.image[IDout].semptr[2], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[IDout].semptr[2]);
            
            sem_getvalue(data.image[IDout].semptr[3], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[IDout].semptr[3]);
            
            data.image[IDout].md[0].write = 0;

            oldslice = slice;
        }

        if((data.signal_INT == 1)||(data.signal_TERM == 1)||(data.signal_ABRT==1)||(data.signal_BUS==1)||(data.signal_SEGV==1)||(data.signal_HUP==1)||(data.signal_PIPE==1))
            loopOK = 0;

        iter++;
    }

    free(nbpixslice);
    free(sizearray);
    free(dtarray);
    free(tarray);
    
    return(IDout);
}














/* =============================================================================================== */
/* =============================================================================================== */
/*                                                                                                 */
/* 14. DATA LOGGING                                                                                */
/*                                                                                                 */
/* =============================================================================================== */
/* =============================================================================================== */



/// creates logshimconf shared memory and loads it
LOGSHIM_CONF* COREMOD_MEMORY_logshim_create_SHMconf(const char *logshimname)
{
    int SM_fd;
    size_t sharedsize = 0; // shared memory size in bytes
    char SM_fname[200];
    int result;
    LOGSHIM_CONF *map;

    sharedsize = sizeof(LOGSHIM_CONF);

    sprintf(SM_fname, "%s/%s.logshimconf.shm", SHAREDMEMDIR, logshimname);
    
    SM_fd = open(SM_fname, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (SM_fd == -1) {
		printf("File \"%s\"\n", SM_fname);
		fflush(stdout);
        perror("Error opening file for writing");
        exit(0);
    }

    result = lseek(SM_fd, sharedsize-1, SEEK_SET);
    if (result == -1) {
        close(SM_fd);
        printERROR(__FILE__,__func__,__LINE__,"Error calling lseek() to 'stretch' the file");
        exit(0);
    }

    result = write(SM_fd, "", 1);
    if (result != 1) {
        close(SM_fd);
        perror("Error writing last byte of the file");
        exit(0);
    }

    map = (LOGSHIM_CONF*) mmap(0, sharedsize, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
    if (map == MAP_FAILED) {
        close(SM_fd);
        perror("Error mmapping the file");
        exit(0);
    }

    map[0].on = 0;
    map[0].cnt = 0;
    map[0].filecnt = 0;
    map[0].interval = 1;
    map[0].logexit = 0;
    strcpy(map[0].fname, SM_fname);

    return(map);
}



// IDname is name of image logged
int_fast8_t COREMOD_MEMORY_logshim_printstatus(const char *IDname)
{
    LOGSHIM_CONF* map;
    char SM_fname[200];
    int SM_fd;
    struct stat file_stat;

    // read shared mem
    sprintf(SM_fname, "%s/%s.logshimconf.shm", SHAREDMEMDIR, IDname);
    printf("Importing mmap file \"%s\"\n",SM_fname);

    SM_fd = open(SM_fname, O_RDWR);
    if(SM_fd==-1)
    {
        printf("Cannot import file - continuing\n");
        exit(0);
    }
    else
    {
        fstat(SM_fd, &file_stat);
        printf("File %s size: %zd\n", SM_fname, file_stat.st_size);

        map = (LOGSHIM_CONF*) mmap(0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
        if (map == MAP_FAILED) {
            close(SM_fd);
            perror("Error mmapping the file");
            exit(0);
        }

        printf("LOG   on = %d\n", map[0].on);
        printf("    cnt  = %lld\n", map[0].cnt);
        printf(" filecnt = %lld\n", map[0].filecnt);
        printf("interval = %ld\n", map[0].interval);
        printf("logexit  = %d\n", map[0].logexit);

        if (munmap(map, sizeof(LOGSHIM_CONF)) == -1) {
            printf("unmapping %s\n", SM_fname);
            perror("Error un-mmapping the file");
        }
        close(SM_fd);
    }
    return(0);
}
















// set the on field in logshim
// IDname is name of image logged
int_fast8_t COREMOD_MEMORY_logshim_set_on(const char *IDname, int setv)
{
    LOGSHIM_CONF* map;
    char SM_fname[200];
    int SM_fd;
    struct stat file_stat;

    // read shared mem
    sprintf(SM_fname, "%s/%s.logshimconf.shm", SHAREDMEMDIR, IDname);
    printf("Importing mmap file \"%s\"\n",SM_fname);

    SM_fd = open(SM_fname, O_RDWR);
    if(SM_fd==-1)
    {
        printf("Cannot import file - continuing\n");
        exit(0);
    }
    else
    {
        fstat(SM_fd, &file_stat);
        printf("File %s size: %zd\n", SM_fname, file_stat.st_size);

        map = (LOGSHIM_CONF*) mmap(0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
        if (map == MAP_FAILED) {
            close(SM_fd);
            perror("Error mmapping the file");
            exit(0);
        }

        map[0].on = setv;

        if (munmap(map, sizeof(LOGSHIM_CONF)) == -1) {
            printf("unmapping %s\n", SM_fname);
            perror("Error un-mmapping the file");
        }
        close(SM_fd);
    }
    return(0);
}



// set the on field in logshim
// IDname is name of image logged
int_fast8_t COREMOD_MEMORY_logshim_set_logexit(const char *IDname, int setv)
{
    LOGSHIM_CONF* map;
    char SM_fname[200];
    int SM_fd;
    struct stat file_stat;

    // read shared mem
    sprintf(SM_fname, "%s/%s.logshimconf.shm", SHAREDMEMDIR, IDname);
    printf("Importing mmap file \"%s\"\n",SM_fname);

    SM_fd = open(SM_fname, O_RDWR);
    if(SM_fd==-1)
    {
        printf("Cannot import file - continuing\n");
        exit(0);
    }
    else
    {
        fstat(SM_fd, &file_stat);
        printf("File %s size: %zd\n", SM_fname, file_stat.st_size);

        map = (LOGSHIM_CONF*) mmap(0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
        if (map == MAP_FAILED) {
            close(SM_fd);
            perror("Error mmapping the file");
            exit(0);
        }

        map[0].logexit = setv;

        if (munmap(map, sizeof(LOGSHIM_CONF)) == -1) {
            printf("unmapping %s\n", SM_fname);
            perror("Error un-mmapping the file");
        }
        close(SM_fd);
    }
    return(0);
}







/** logs a shared memory stream onto disk
 * uses semlog semaphore
 *
 * uses data cube buffer to store frames
 * if an image name logdata exists (should ideally be in shared mem), then this will be included in the timing txt file
 */
long __attribute__((hot)) COREMOD_MEMORY_sharedMem_2Dim_log(const char *IDname, uint32_t zsize, const char *logdir, const char *IDlogdata_name)
{
	// WAIT time. If no new frame during this time, save existing cube
	int WaitSec = 5; 
	
	
    long ID;
    uint32_t xsize, ysize;
    long ii;
    long i;
    long IDb, IDb0, IDb1;
    long index = 0;
    long cnt = -1;
    int buffer;
    uint8_t atype;
    uint32_t *imsizearray;
    char fname[200];
    char iname[200];
    time_t t;
    struct tm *uttime;
    struct tm *uttimeStart;
    struct timespec ts;
    struct timespec timenow;
    struct timespec timenowStart;
    long kw;
	int ret;
    long IDlogdata;

    char *ptr0; // source image data
    char *ptr1; // destination image data
    long framesize; // in bytes

    char *arraytime_ptr;
    char *arraycnt0_ptr;
    char *arraycnt1_ptr;

    FILE *fp;
    char fnameascii[200];

    pthread_t thread_savefits;
    int tOK = 0;
    int iret_savefits;
    //	char tmessage[500];
    struct savethreadmsg *tmsg = malloc(sizeof(struct savethreadmsg));

    long fnb = 0;
    long NBfiles = -1; // run forever

    long long cntwait;
    long waitdelayus = 50;  // max speed = 20 kHz
    long long cntwaitlim = 10000; // 5 sec
    int wOK;
    int noframe;


    char logb0name[500];
    char logb1name[500];

    int is3Dcube = 0; // this is a rolling buffer
    int exitflag = 0; // toggles to 1 when loop must exit

    LOGSHIM_CONF* logshimconf;

    // recording time for each frame
    double *array_time;
    double *array_time_cp;

    // counters
    uint64_t *array_cnt0;
    uint64_t *array_cnt0_cp;
    uint64_t *array_cnt1;
    uint64_t *array_cnt1_cp;

    int RT_priority = 60; //any number from 0-99
    struct sched_param schedpar;

    int use_semlog;
    int semval;


    int VERBOSE = 0;

	// convert wait time into number of couunter steps (counter mode only)
	cntwaitlim = (long) (WaitSec*1000000/waitdelayus);
	


    schedpar.sched_priority = RT_priority;
#ifndef __MACH__
    // r = seteuid(euid_called); //This goes up to maximum privileges
    sched_setscheduler(0, SCHED_FIFO, &schedpar); //other option is SCHED_RR, might be faster
    // r = seteuid(euid_real);//Go back to normal privileges
#endif



    IDlogdata = image_ID(IDlogdata_name);
    if(IDlogdata!=-1)
    {
        if(data.image[IDlogdata].md[0].atype != _DATATYPE_FLOAT)
            IDlogdata = -1;
    }
    printf("log data name = %s\n", IDlogdata_name);


    logshimconf = COREMOD_MEMORY_logshim_create_SHMconf(IDname);


    logshimconf[0].on = 1;
    logshimconf[0].cnt = 0;
    logshimconf[0].filecnt = 0;
    logshimconf[0].logexit = 0;
    logshimconf[0].interval = 1;



    imsizearray = (uint32_t*) malloc(sizeof(uint32_t)*3);



    read_sharedmem_image(IDname);
    ID = image_ID(IDname);
    atype = data.image[ID].md[0].atype;
    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];

    if(data.image[ID].md[0].naxis==3)
        is3Dcube = 1;

    /** create the 2 buffers */

    imsizearray[0] = xsize;
    imsizearray[1] = ysize;
    imsizearray[2] = zsize;

    sprintf(logb0name, "%s_logbuff0", IDname);
    sprintf(logb1name, "%s_logbuff1", IDname);

    IDb0 = create_image_ID(logb0name, 3, imsizearray, atype, 1, 1);
    IDb1 = create_image_ID(logb1name, 3, imsizearray, atype, 1, 1);
    COREMOD_MEMORY_image_set_semflush(logb0name, -1);
    COREMOD_MEMORY_image_set_semflush(logb1name, -1);


    array_time = (double*) malloc(sizeof(double)*zsize);
    array_cnt0 = (uint64_t*) malloc(sizeof(uint64_t)*zsize);
    array_cnt1 = (uint64_t*) malloc(sizeof(uint64_t)*zsize);

    array_time_cp = (double*) malloc(sizeof(double)*zsize);
    array_cnt0_cp = (uint64_t*) malloc(sizeof(uint64_t)*zsize);
    array_cnt1_cp = (uint64_t*) malloc(sizeof(uint64_t)*zsize);


    IDb = IDb0;

    switch ( atype ) {

    case _DATATYPE_FLOAT:
        framesize = SIZEOF_DATATYPE_FLOAT*xsize*ysize;
        break;

    case _DATATYPE_INT8:
        framesize = SIZEOF_DATATYPE_INT8*xsize*ysize;
        break;

    case _DATATYPE_UINT8:
        framesize = SIZEOF_DATATYPE_UINT8*xsize*ysize;
        break;

    case _DATATYPE_INT16:
        framesize = SIZEOF_DATATYPE_INT16*xsize*ysize;
        break;

    case _DATATYPE_UINT16:
        framesize = SIZEOF_DATATYPE_UINT16*xsize*ysize;
        break;

    case _DATATYPE_INT32:
        framesize = SIZEOF_DATATYPE_INT32*xsize*ysize;
        break;

    case _DATATYPE_UINT32:
        framesize = SIZEOF_DATATYPE_UINT32*xsize*ysize;
        break;

    case _DATATYPE_INT64:
        framesize = SIZEOF_DATATYPE_INT64*xsize*ysize;
        break;

    case _DATATYPE_UINT64:
        framesize = SIZEOF_DATATYPE_UINT64*xsize*ysize;
        break;


    case _DATATYPE_DOUBLE:
        framesize = SIZEOF_DATATYPE_DOUBLE*xsize*ysize;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }

    cnt = data.image[ID].md[0].cnt0 - 1;

    buffer = 0;
    index = 0;

    printf("logdata ID = %ld\n", IDlogdata);
    list_image_ID();

    exitflag = 0;


    // using semlog ?
    use_semlog = 0;
    if(data.image[ID].semlog != NULL)
    {
        use_semlog = 1;
        sem_getvalue(data.image[ID].semlog, &semval);

		// bring semaphore value to 1 to only save 1 frame
       while(semval>1)
           {
			   sem_wait(data.image[ID].semlog);
				sem_getvalue(data.image[ID].semlog, &semval);
		   }
		if(semval==0)
			sem_post(data.image[ID].semlog);
    }



    while( (logshimconf[0].filecnt != NBfiles) && (logshimconf[0].logexit==0) )
    {
        cntwait = 0;
        noframe = 0;
        wOK = 1;

        if(VERBOSE==1)
            printf("%5d  Entering wait loop   index = %ld %d\n", __LINE__, index, noframe);

        if(likely(use_semlog==1))
        {
            if(VERBOSE==1)
                printf("%5d  Waiting for semaphore\n", __LINE__);

			if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
			ts.tv_sec += WaitSec;

            ret = sem_timedwait(data.image[ID].semlog, &ts);
			if (ret == -1) { 
				if (errno == ETIMEDOUT)
					printf("sem_timedwait() timed out -> save (%ld)\n", index);
				
				if(VERBOSE==1)
                    printf("%5d  sem time elapsed -> Save current cube (%ld)\n", __LINE__, index);

                strcpy(tmsg->iname, iname);
                strcpy(tmsg->fname, fname);
                tmsg->partial = 1; // partial cube
                tmsg->cubesize = index;
                
                memcpy(array_time_cp, array_time, sizeof(double)*index);
                memcpy(array_cnt0_cp, array_cnt0, sizeof(uint64_t)*index);
                memcpy(array_cnt1_cp, array_cnt1, sizeof(uint64_t)*index);

                tmsg->arraycnt0 = array_cnt0_cp;
                tmsg->arraycnt1 = array_cnt1_cp;
                tmsg->arraytime = array_time_cp;
                
				wOK=0;
                if(index==0)
                    noframe = 1;
                else
                    noframe = 0;
               
				}


//            if(VERBOSE==1)
 //               printf("%5d  Image arrived  cntwait = %lld\n", __LINE__, cntwait);

        /*    cntwait++;
            if(cntwait>cntwaitlim) // save current cube
            {
                if(VERBOSE==1)
                    printf("%5d  ime elapsed -> Save current cube\n", __LINE__);

                strcpy(tmsg->iname, iname);
                strcpy(tmsg->fname, fname);
                tmsg->partial = 1; // partial cube
                tmsg->cubesize = index;

                memcpy(array_time_cp, array_time, sizeof(double)*index);
                memcpy(array_cnt0_cp, array_cnt0, sizeof(uint64_t)*index);
                memcpy(array_cnt1_cp, array_cnt1, sizeof(uint64_t)*index);

                tmsg->arraycnt0 = array_cnt0_cp;
                tmsg->arraycnt1 = array_cnt1_cp;
                tmsg->arraytime = array_time_cp;

                wOK=0;
                if(index==0)
                    noframe = 1;
                else
                    noframe = 0;
            }*/
        }
        else
        {
            if(VERBOSE==1)
                printf("%5d  Not using semaphore, watching counter\n", __LINE__);

            while(((cnt==data.image[ID].md[0].cnt0)||(logshimconf[0].on == 0))&&(wOK==1))
            {
                if(VERBOSE==1)
                    printf("%5d  waiting time step\n", __LINE__);

                usleep(waitdelayus);
                cntwait++;

                if(VERBOSE==1) {
                    printf("%5d  cntwait = %lld\n", __LINE__, cntwait);
                    fflush(stdout);
                }

                if(cntwait>cntwaitlim) // save current cube
                {
                    if(VERBOSE==1)
                        printf("%5d  cnt time elapsed -> Save current cube\n", __LINE__);


                    strcpy(tmsg->iname, iname);
                    strcpy(tmsg->fname, fname);
                    tmsg->partial = 1; // partial cube
                    tmsg->cubesize = index;

                    memcpy(array_time_cp, array_time, sizeof(double)*index);
                    memcpy(array_cnt0_cp, array_cnt0, sizeof(uint64_t)*index);
                    memcpy(array_cnt1_cp, array_cnt1, sizeof(uint64_t)*index);

                    tmsg->arraycnt0 = array_cnt0_cp;
                    tmsg->arraycnt1 = array_cnt1_cp;
                    tmsg->arraytime = array_time_cp;

                    wOK=0;
                    if(index==0)
                        noframe = 1;
                    else
                        noframe = 0;
                }
            }
        }



        if(index==0)
        {
            if(VERBOSE==1)
                printf("%5d  Setting cube start time\n", __LINE__);

            /// measure time
            t = time(NULL);
            uttimeStart = gmtime(&t);
            clock_gettime(CLOCK_REALTIME, &timenowStart);

            //     sprintf(fname,"!%s/%s_%02d:%02d:%02ld.%09ld.fits", logdir, IDname, uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
            //            sprintf(fnameascii,"%s/%s_%02d:%02d:%02ld.%09ld.txt", logdir, IDname, uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
        }


		if(VERBOSE==1)
            printf("%5d  logshimconf[0].on = %d\n", __LINE__, logshimconf[0].on);


        if(likely(logshimconf[0].on == 1))
        {
            if(likely(wOK==1)) // normal step: a frame has arrived
            {
                if(VERBOSE==1)
                    printf("%5d  Frame has arrived index = %ld\n", __LINE__, index);

                /// measure time
                t = time(NULL);
                uttime = gmtime(&t);

                clock_gettime(CLOCK_REALTIME, &timenow);

                /*           if(index==0)
                               fp = fopen(fname_asciilog, "w");
                */

                switch ( atype ) {

                case _DATATYPE_FLOAT:
                    ptr0 = (char*) data.image[ID].array.F;
                    ptr1 = (char*) data.image[IDb].array.F;
                    break;

                case _DATATYPE_INT8:
                    ptr0 = (char*) data.image[ID].array.SI8;
                    ptr1 = (char*) data.image[IDb].array.SI8;
                    break;

                case _DATATYPE_UINT8:
                    ptr0 = (char*) data.image[ID].array.UI8;
                    ptr1 = (char*) data.image[IDb].array.UI8;
                    break;

                case _DATATYPE_INT16:
                    ptr0 = (char*) data.image[ID].array.SI16;
                    ptr1 = (char*) data.image[IDb].array.SI16;
                    break;

                case _DATATYPE_UINT16:
                    ptr0 = (char*) data.image[ID].array.UI16;
                    ptr1 = (char*) data.image[IDb].array.UI16;
                    break;

                case _DATATYPE_INT32:
                    ptr0 = (char*) data.image[ID].array.SI32;
                    ptr1 = (char*) data.image[IDb].array.SI32;
                    break;

                case _DATATYPE_UINT32:
                    ptr0 = (char*) data.image[ID].array.UI32;
                    ptr1 = (char*) data.image[IDb].array.UI32;
                    break;

                case _DATATYPE_INT64:
                    ptr0 = (char*) data.image[ID].array.SI64;
                    ptr1 = (char*) data.image[IDb].array.SI64;
                    break;

                case _DATATYPE_UINT64:
                    ptr0 = (char*) data.image[ID].array.UI64;
                    ptr1 = (char*) data.image[IDb].array.UI64;
                    break;

                case _DATATYPE_DOUBLE:
                    ptr0 = (char*) data.image[ID].array.D;
                    ptr1 = (char*) data.image[IDb].array.D;
                    break;

                }


                if(is3Dcube==1)
                    ptr0 += framesize*data.image[ID].md[0].cnt1;

                ptr1 += framesize*index;

                memcpy((void *) ptr1, (void *) ptr0, framesize);


                //                fprintf(fp, "%02d:%02d:%02ld.%09ld ", uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
                array_cnt0[index] = data.image[ID].md[0].cnt0;
                array_cnt1[index] = data.image[ID].md[0].cnt1;
                array_time[index] = uttime->tm_hour*3600.0 + uttime->tm_min*60.0 + timenow.tv_sec % 60 + 1.0e-9*timenow.tv_nsec;

                /*                if(unlikely(IDlogdata!=-1))
                                {

                                    fprintf(fp, "%8ld", data.image[IDlogdata].md[0].cnt0);
                                    for(i=0; i<data.image[IDlogdata].md[0].nelement; i++)
                                        fprintf(fp, "  %f", data.image[IDlogdata].array.F[i]);
                                }
                */
                /*
                                for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
                                {
                                    switch (data.image[ID].kw[kw].type) {
                                    case 'D' :
                                        fprintf(fp, " %f", data.image[ID].kw[kw].value.numf);
                                        break;
                                    case 'L' :
                                        fprintf(fp, " %ld", data.image[ID].kw[kw].value.numl);
                                        break;
                                    }
                                }
                                fprintf(fp, "\n");
                */
                index++;
            }
        }
        else
        {
			// save partial if possible
			//if(index>0)
			wOK = 0;
				
		}


		if(VERBOSE==1)
            printf("%5d  index = %ld  wOK = %d\n", __LINE__, index, wOK);

        /// cases:
        /// index>zsize-1  buffer full
        /// wOK==0 && index>0  : partial
        if(  (index>zsize-1)  ||  ((wOK==0)&&(index>0)) )
        {
            /// save image
            if(VERBOSE==1)
                printf("%5d  Save image   index = %ld  wOK = %d\n", __LINE__, index, wOK);
            
            sprintf(iname, "%s_logbuff%d", IDname, buffer);
            if(buffer==0)
                IDb = IDb0;
            else
                IDb = IDb1;


				if(VERBOSE==1)
				{
					printf("%5d  Building file name: ascii\n", __LINE__);
					fflush(stdout);
				}
				
                sprintf(fnameascii,"%s/%s_%02d:%02d:%02ld.%09ld.txt", logdir, IDname, uttimeStart->tm_hour, uttimeStart->tm_min, timenowStart.tv_sec % 60, timenowStart.tv_nsec);
                
                
				if(VERBOSE==1)
				{
					printf("%5d  Building file name: fits\n", __LINE__);
					fflush(stdout);
				}
                sprintf(fname,"!%s/%s_%02d:%02d:%02ld.%09ld.fits", logdir, IDname, uttimeStart->tm_hour, uttimeStart->tm_min, timenowStart.tv_sec % 60, timenowStart.tv_nsec);
				
				
				
                strcpy(tmsg->iname, iname);
                strcpy(tmsg->fname, fname);
                strcpy(tmsg->fnameascii, fnameascii);



            if(wOK==1) // full cube
            {
                tmsg->partial = 0; // full cube           
				if(VERBOSE==1)
				{
					printf("%5d  FULL CUBE\n", __LINE__);
					fflush(stdout);
				}
				
            }
            else // partial cube
            {
				tmsg->partial = 1; // partial cube           
				if(VERBOSE==1)
				{
					printf("%5d  PARTIAL CUBE\n", __LINE__);
					fflush(stdout);
				}
			}
            

            //  fclose(fp);

            if(tOK == 1)
                pthread_join(thread_savefits, NULL); //(void**)&thread_savefits);

            COREMOD_MEMORY_image_set_sempost_byID(IDb, -1);
            data.image[IDb].md[0].cnt0++;
            data.image[IDb].md[0].write = 0;

            tmsg->cubesize = index;
            strcpy(tmsg->iname, iname);
            memcpy(array_time_cp, array_time, sizeof(double)*index);
            memcpy(array_cnt0_cp, array_cnt0, sizeof(uint64_t)*index);
            memcpy(array_cnt1_cp, array_cnt1, sizeof(uint64_t)*index);

			if(VERBOSE==1)
				{
					printf("%5d  Starting thread\n", __LINE__);
					fflush(stdout);
				}			

            tmsg->arraycnt0 = array_cnt0_cp;
            tmsg->arraycnt1 = array_cnt1_cp;
            tmsg->arraytime = array_time_cp;
            iret_savefits = pthread_create( &thread_savefits, NULL, save_fits_function, tmsg);

            logshimconf[0].cnt ++;

            tOK = 1;
            if(iret_savefits)
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n", iret_savefits);
                exit(EXIT_FAILURE);
            }

            index = 0;
            buffer++;
            if(buffer==2)
                buffer = 0;
            //            printf("[%ld -> %d]", cnt, buffer);
            //           fflush(stdout);
            if(buffer==0)
                IDb = IDb0;
            else
                IDb = IDb1;
            data.image[IDb].md[0].write = 1;
            logshimconf[0].filecnt ++;
        }


        cnt = data.image[ID].md[0].cnt0;
    }

    free(imsizearray);
    free(tmsg);

    free(array_time);
    free(array_cnt0);
    free(array_cnt1);

    free(array_time_cp);
    free(array_cnt0_cp);
    free(array_cnt1_cp);

    return(0);
}













