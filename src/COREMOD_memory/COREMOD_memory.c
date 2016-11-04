#include <fitsio.h>
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



#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"

 
# ifdef _OPENMP
# include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
# endif

#define STYPESIZE 10
#define SBUFFERSIZE 1000








pthread_t *thrarray_semwait;
long NB_thrarray_semwait;


// MEMORY MONITOR 
FILE *listim_scr_fpo;
FILE *listim_scr_fpi;
SCREEN *listim_scr; // for memory monitoring
int MEM_MONITOR = 0; // 1 if memory monitor is on
int listim_scr_wrow;
int listim_scr_wcol;


extern DATA data;


char errmsg_memory[SBUFFERSIZE];



/** data logging of shared memory image stream
 *
 */

struct savethreadmsg {
    char iname[100];
    char fname[200];
    int partial; // 1 if partial cube
    long cubesize; // if partial cube, this is the size of the cube
};
long tret; // thread return value







// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string






int create_image_cli()
{
    long *imsize;
    long naxis = 0;
    long i;
    int atype;



    if(CLI_checkarg(1,3)+CLI_checkarg_noerrmsg(2,2)==0)
    {
        naxis = 0;
        imsize = (long*) malloc(sizeof(long)*5);
        i = 2;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        switch(data.precision) {
        case 0:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);
            break;
        case 1:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT);
            break;
        }
        free(imsize);
    }
    else if (CLI_checkarg(1,3)+CLI_checkarg(2,3)+CLI_checkarg(3,2)==0) // type option exists
    {
        atype = -1;

        if(strcmp(data.cmdargtoken[2].val.string, "c")==0)
        {
            printf("type = CHAR\n");
            atype = CHAR;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "i")==0)
        {
            printf("type = INT\n");
            atype = INT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "f")==0)
        {
            printf("type = FLOAT\n");
            atype = FLOAT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "d")==0)
        {
            printf("type = DOUBLE\n");
            atype = DOUBLE;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "cf")==0)
        {
            printf("type = COMPLEX_FLOAT\n");
            atype = COMPLEX_FLOAT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "cd")==0)
        {
            printf("type = COMPLEX_DOUBLE\n");
            atype = COMPLEX_DOUBLE;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "u")==0)
        {
            printf("type = USHORT\n");
            atype = USHORT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "l")==0)
        {
            printf("type = LONG\n");
            atype = LONG;
        }

        if(atype==-1)
        {
            printf("Data type \"%s\" not recognized\n", data.cmdargtoken[2].val.string);
            printf("must be : \n");
            printf("  c : CHAR\n");
            printf("  i : INT\n");
            printf("  f : FLOAT\n");
            printf("  d : DOUBLE\n");
            printf("  cf: COMPLEX FLOAT\n");
            printf("  cd: COMPLEX DOUBLE\n");
            printf("  u : USHORT\n");
            printf("  l : LONG\n");
            return 1;
        }
        naxis = 0;
        imsize = (long*) malloc(sizeof(long)*5);
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




int image_write_keyword_L_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,3)==0)
    image_write_keyword_L(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string);
  else
    return 1;
}



int image_list_keywords_cli()
{
  if(CLI_checkarg(1,4)==0)
    image_list_keywords(data.cmdargtoken[1].val.string);
  else
    return 1;
}


//long read_sharedmem_image_size(char *name, char *fname)
int read_sharedmem_image_size_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,3)==0)
    read_sharedmem_image_size(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
  else
    return 1;
}


int read_sharedmem_image_cli()
{
  if(CLI_checkarg(1,3)==0)
    read_sharedmem_image(data.cmdargtoken[1].val.string);
  else
    return 1;
}


int create_image_shared_cli() // default precision
{
    long *imsize;
    long naxis = 0;
    long i;


    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        naxis = 0;
        imsize = (long*) malloc(sizeof(long)*5);
        i = 2;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        switch(data.precision) {
        case 0:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, FLOAT, 1, data.NBKEWORD_DFT);
            break;
        case 1:
            create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, DOUBLE, 1, data.NBKEWORD_DFT);
            break;
        }
        free(imsize);
        printf("Creating 10 semaphores\n");
        COREMOD_MEMORY_image_set_createsem(data.cmdargtoken[1].val.string, 10);
    }
    else
        return 1;
}


int create_ushort_image_shared_cli() // default precision
{
    long *imsize;
    long naxis = 0;
    long i;


    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        naxis = 0;
        imsize = (long*) malloc(sizeof(long)*5);
        i = 2;
        while(data.cmdargtoken[i].type==2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        create_image_ID(data.cmdargtoken[1].val.string, naxis, imsize, USHORT, 1, data.NBKEWORD_DFT);

        free(imsize);
    }
    else
        return 1;
}




int create_2Dimage_float()
{
  long *imsize;

  // CHECK ARGS
//  printf("CREATING IMAGE\n");
  imsize = (long*) malloc(sizeof(long)*2);

  imsize[0] = data.cmdargtoken[2].val.numl;
  imsize[1] = data.cmdargtoken[3].val.numl;

  create_image_ID(data.cmdargtoken[1].val.string, 2, imsize, FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);

  free(imsize);

  return 0;
}

int create_3Dimage_float()
{
  long *imsize;

  // CHECK ARGS
//  printf("CREATING 3D IMAGE\n");
  imsize = (long*) malloc(sizeof(long)*3);

  imsize[0] = data.cmdargtoken[2].val.numl;
  imsize[1] = data.cmdargtoken[3].val.numl;
  imsize[2] = data.cmdargtoken[4].val.numl;

  create_image_ID(data.cmdargtoken[1].val.string, 3, imsize, FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);

  free(imsize);

  return 0;
}


int memory_monitor_cli()
{
  memory_monitor(data.cmdargtoken[1].val.string);
  return 0;
}

int list_variable_ID_file_cli()
{
 if(CLI_checkarg(1,3)==0)
    list_variable_ID_file(data.cmdargtoken[1].val.string);
  else
    return 1;
}


int delete_image_ID_cli()
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


int copy_image_ID_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  
  copy_image_ID(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, 0);

  return 0;
}

int copy_image_ID_sharedmem_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  
  copy_image_ID(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, 1);

  return 0;
}


int chname_image_ID_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }
  
  chname_image_ID(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);

  return 0;
}


int mk_complex_from_reim_cli()
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


int mk_complex_from_amph_cli()
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


int mk_reim_from_complex_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }

  mk_reim_from_complex(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, 0);

  return 0;
}

int mk_amph_from_complex_cli()
{
  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
    }

  mk_amph_from_complex(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string, 0);

  return 0;
}





int COREMOD_MEMORY_image_set_status_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_status(data.cmdargtoken[1].val.string, (int) data.cmdargtoken[2].val.numl);
    else
        return 1;
}


int COREMOD_MEMORY_image_set_cnt0_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_cnt0(data.cmdargtoken[1].val.string, (int) data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int COREMOD_MEMORY_image_set_cnt1_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_cnt1(data.cmdargtoken[1].val.string, (int) data.cmdargtoken[2].val.numl);
    else
        return 1;
}


int COREMOD_MEMORY_image_set_createsem_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_createsem(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int COREMOD_MEMORY_image_set_sempost_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_sempost(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int COREMOD_MEMORY_image_set_sempost_loop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,2)==0)
        COREMOD_MEMORY_image_set_sempost_loop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl);
    else
        return 1;
}



int COREMOD_MEMORY_image_set_semwait_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_semwait(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}

int COREMOD_MEMORY_image_set_semflush_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
        COREMOD_MEMORY_image_set_semflush(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
    else
        return 1;
}




int COREMOD_MEMORY_cp2shm_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)==0)
    {
        COREMOD_MEMORY_cp2shm(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
        return 0;
    }
    else
        return 1;
}



int COREMOD_MEMORY_image_streamupdateloop_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)==0)
    {
        COREMOD_MEMORY_image_streamupdateloop(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl);
        return 0;
    }
    else
        return 1;
}





int COREMOD_MEMORY_image_NETWORKtransmit_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)
    {
        COREMOD_MEMORY_image_NETWORKtransmit(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
        return 0;
    }
    else
        return 1;
}


int COREMOD_MEMORY_image_NETWORKreceive_cli()
{
    if(CLI_checkarg(1,2)+CLI_checkarg(2,2)==0)
    {
        COREMOD_MEMORY_image_NETWORKreceive(data.cmdargtoken[1].val.numl, data.cmdargtoken[2].val.numl);
        return 0;
    }
    else
        return 1;
}



int COREMOD_MEMORY_PixMapDecode_U_cli()
{
     if(CLI_checkarg(1,4)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,3)+CLI_checkarg(5,4)+CLI_checkarg(6,3)+CLI_checkarg(7,3)==0)
    {
        COREMOD_MEMORY_PixMapDecode_U(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.string, data.cmdargtoken[5].val.string, data.cmdargtoken[6].val.string, data.cmdargtoken[7].val.string);
        return 0;
    }
    else
        return 1;
}






int COREMOD_MEMORY_logshim_printstatus_cli()
{
    if(CLI_checkarg(1,3)==0)
    {
        COREMOD_MEMORY_logshim_printstatus(data.cmdargtoken[1].val.string);
        return 0;
    }
    else
        return 1;
}


int COREMOD_MEMORY_logshim_set_on_cli()
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

int COREMOD_MEMORY_logshim_set_logexit_cli()
{
    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)==0)
    {
        COREMOD_MEMORY_logshim_set_logexit(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
        return 0;
    }
    else
        return 1;
}



int COREMOD_MEMORY_sharedMem_2Dim_log_cli()
{
    //sprintf(data.cmdargtoken[4].val.string, "null");
    CLI_checkarg(4,3);
    if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,3)==0)
    {
        COREMOD_MEMORY_sharedMem_2Dim_log(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.string, data.cmdargtoken[4].val.string);
        return 0;
    }
    else
        return 1;
}










int init_COREMOD_memory()
{
    strcpy(data.module[data.NBmodule].name,__FILE__);
    strcpy(data.module[data.NBmodule].info,"memory management for images and variables");
    data.NBmodule++;

    strcpy(data.cmd[data.NBcmd].key,"listim");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = list_image_ID;
    strcpy(data.cmd[data.NBcmd].info,"list images in memory");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"listim");
    strcpy(data.cmd[data.NBcmd].Ccall,"int list_image_ID()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"mmon");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = memory_monitor_cli;
    strcpy(data.cmd[data.NBcmd].info,"Monitor memory content");
    strcpy(data.cmd[data.NBcmd].syntax,"terminal tty name");
    strcpy(data.cmd[data.NBcmd].example,"mmon /dev/pts/4");
    strcpy(data.cmd[data.NBcmd].Ccall,"int memory_monitor(char *ttyname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"listvar");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = list_variable_ID;
    strcpy(data.cmd[data.NBcmd].info,"list variables in memory");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"listvar");
    strcpy(data.cmd[data.NBcmd].Ccall,"int list_variable_ID()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"listvarf");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = list_variable_ID_file_cli;
    strcpy(data.cmd[data.NBcmd].info,"list variables in memory, write to file");
    strcpy(data.cmd[data.NBcmd].syntax,"<file name>");
    strcpy(data.cmd[data.NBcmd].example,"listvarf var.txt");
    strcpy(data.cmd[data.NBcmd].Ccall,"int list_variable_ID_file()");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"creaim");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = create_image_cli;
    strcpy(data.cmd[data.NBcmd].info,"create image, default precision");
    strcpy(data.cmd[data.NBcmd].syntax,"<name> <xsize> <ysize> <opt: zsize>");
    strcpy(data.cmd[data.NBcmd].example,"creaim imname 512 512");
    strcpy(data.cmd[data.NBcmd].Ccall,"long create_image_ID(char *name, long naxis, long *size, int atype, 0, 10)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imwritekwL");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = image_write_keyword_L_cli;
    strcpy(data.cmd[data.NBcmd].info,"write long type keyword");
    strcpy(data.cmd[data.NBcmd].syntax,"<imname> <kname> <value [long]> <comment>");
    strcpy(data.cmd[data.NBcmd].example,"imwritekwL im1 kw2 34 my_keyword_comment");
    strcpy(data.cmd[data.NBcmd].Ccall,"long image_write_keyword_L(char *IDname, char *kname, long value, char *comment)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imlistkw");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = image_list_keywords_cli;
    strcpy(data.cmd[data.NBcmd].info,"list image keywords");
    strcpy(data.cmd[data.NBcmd].syntax,"<imname>");
    strcpy(data.cmd[data.NBcmd].example,"imlistkw im1");
    strcpy(data.cmd[data.NBcmd].Ccall,"long image_list_keywords(char *IDname)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"readshmimsize");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = read_sharedmem_image_size_cli;
    strcpy(data.cmd[data.NBcmd].info,"read shared memory image size");
    strcpy(data.cmd[data.NBcmd].syntax,"<name> <output file>");
    strcpy(data.cmd[data.NBcmd].example,"readshmim im1 imsize.txt");
    strcpy(data.cmd[data.NBcmd].Ccall,"read_sharedmem_image_size(char *name, char *fname)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"readshmim");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = read_sharedmem_image_cli;
    strcpy(data.cmd[data.NBcmd].info,"read shared memory image");
    strcpy(data.cmd[data.NBcmd].syntax,"<name>");
    strcpy(data.cmd[data.NBcmd].example,"readshmim im1");
    strcpy(data.cmd[data.NBcmd].Ccall,"read_sharedmem_image(char *name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"creaimshm");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = create_image_shared_cli;
    strcpy(data.cmd[data.NBcmd].info,"create image in shared mem, default precision");
    strcpy(data.cmd[data.NBcmd].syntax,"<name> <xsize> <ysize> <opt: zsize>");
    strcpy(data.cmd[data.NBcmd].example,"creaimshm imname 512 512");
    strcpy(data.cmd[data.NBcmd].Ccall,"long create_image_ID(char *name, long naxis, long *size, int atype, 0, 10)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"creaushortimshm");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = create_ushort_image_shared_cli;
    strcpy(data.cmd[data.NBcmd].info,"create unsigned short image in shared mem");
    strcpy(data.cmd[data.NBcmd].syntax,"<name> <xsize> <ysize> <opt: zsize>");
    strcpy(data.cmd[data.NBcmd].example,"creaushortimshm imname 512 512");
    strcpy(data.cmd[data.NBcmd].Ccall,"long create_image_ID(char *name, long naxis, long *size, USHORT, 0, 10)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"crea3dim");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = create_3Dimage_float;
    strcpy(data.cmd[data.NBcmd].info,"creates 3D image, single precision");
    strcpy(data.cmd[data.NBcmd].syntax,"<name> <xsize> <ysize> <zsize>");
    strcpy(data.cmd[data.NBcmd].example,"crea3dim imname 512 512 100");
    strcpy(data.cmd[data.NBcmd].Ccall,"long create_image_ID(char *name, long naxis, long *size, FLOAT, 0, 10)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"rm");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = delete_image_ID_cli;
    strcpy(data.cmd[data.NBcmd].info,"remove image(s)");
    strcpy(data.cmd[data.NBcmd].syntax,"list of images");
    strcpy(data.cmd[data.NBcmd].example,"rm im1 im4");
    strcpy(data.cmd[data.NBcmd].Ccall,"int delete_image_ID(char* imname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"cp");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = copy_image_ID_cli;
    strcpy(data.cmd[data.NBcmd].info,"copy image");
    strcpy(data.cmd[data.NBcmd].syntax,"source, dest");
    strcpy(data.cmd[data.NBcmd].example,"cp im1 im4");
    strcpy(data.cmd[data.NBcmd].Ccall,"long copy_image_ID(char *name, char *newname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"cpsh");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = copy_image_ID_sharedmem_cli;
    strcpy(data.cmd[data.NBcmd].info,"copy image - create in shared mem if does not exist");
    strcpy(data.cmd[data.NBcmd].syntax,"source, dest");
    strcpy(data.cmd[data.NBcmd].example,"cp im1 im4");
    strcpy(data.cmd[data.NBcmd].Ccall,"long copy_image_ID(char *name, char *newname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"mv");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = chname_image_ID_cli;
    strcpy(data.cmd[data.NBcmd].info,"change image name");
    strcpy(data.cmd[data.NBcmd].syntax,"source, dest");
    strcpy(data.cmd[data.NBcmd].example,"mv im1 im4");
    strcpy(data.cmd[data.NBcmd].Ccall,"long chname_image_ID(char *name, char *newname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"ri2c");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = mk_complex_from_reim_cli;
    strcpy(data.cmd[data.NBcmd].info,"real, imaginary -> complex");
    strcpy(data.cmd[data.NBcmd].syntax,"real imaginary complex");
    strcpy(data.cmd[data.NBcmd].example,"ri2c imr imi imc");
    strcpy(data.cmd[data.NBcmd].Ccall,"int mk_complex_from_reim(char *re_name, char *im_name, char *out_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"ap2c");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = mk_complex_from_amph_cli;
    strcpy(data.cmd[data.NBcmd].info,"ampl, pha -> complex");
    strcpy(data.cmd[data.NBcmd].syntax,"ampl pha complex");
    strcpy(data.cmd[data.NBcmd].example,"ap2c ima imp imc");
    strcpy(data.cmd[data.NBcmd].Ccall,"int mk_complex_from_amph(char *re_name, char *im_name, char *out_name, int sharedmem)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"c2ri");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = mk_reim_from_complex_cli;
    strcpy(data.cmd[data.NBcmd].info,"complex -> real, imaginary");
    strcpy(data.cmd[data.NBcmd].syntax,"complex real imaginary");
    strcpy(data.cmd[data.NBcmd].example,"c2ri imc imr imi");
    strcpy(data.cmd[data.NBcmd].Ccall,"int mk_reim_from_complex(char *re_name, char *im_name, char *out_name)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"c2ap");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = mk_amph_from_complex_cli;
    strcpy(data.cmd[data.NBcmd].info,"complex -> ampl, pha");
    strcpy(data.cmd[data.NBcmd].syntax,"complex ampl pha");
    strcpy(data.cmd[data.NBcmd].example,"c2ap imc ima imp");
    strcpy(data.cmd[data.NBcmd].Ccall,"int mk_amph_from_complex(char *re_name, char *im_name, char *out_name, int sharedmem)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"rmall");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = clearall;
    strcpy(data.cmd[data.NBcmd].info,"remove all images");
    strcpy(data.cmd[data.NBcmd].syntax,"no argument");
    strcpy(data.cmd[data.NBcmd].example,"rmall");
    strcpy(data.cmd[data.NBcmd].Ccall,"int clearall()");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"imsetstatus");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_status_cli;
    strcpy(data.cmd[data.NBcmd].info,"set image status variable");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <value [long]>");
    strcpy(data.cmd[data.NBcmd].example,"imsetstatus im1 2");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_status(char *IDname, int status)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imsetcnt0");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_cnt0_cli;
    strcpy(data.cmd[data.NBcmd].info,"set image cnt0 variable");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <value [long]>");
    strcpy(data.cmd[data.NBcmd].example,"imsetcnt0 im1 2");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_cnt0(char *IDname, int status)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imsetcnt1");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_cnt1_cli;
    strcpy(data.cmd[data.NBcmd].info,"set image cnt1 variable");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <value [long]>");
    strcpy(data.cmd[data.NBcmd].example,"imsetcnt1 im1 2");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_cnt1(char *IDname, int status)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"imsetcreatesem");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_createsem_cli;
    strcpy(data.cmd[data.NBcmd].info,"create image semaphore");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <NBsem>");
    strcpy(data.cmd[data.NBcmd].example,"imsetcreatesem im1 5");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_createsem(char *IDname, long NBsem)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imsetsempost");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_sempost_cli;
    strcpy(data.cmd[data.NBcmd].info,"post image semaphore. If sem index = -1, post all semaphores");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <sem index>");
    strcpy(data.cmd[data.NBcmd].example,"imsetsempost im1 2");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_sempost(char *IDname, long index)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imsetsempostl");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_sempost_loop_cli;
    strcpy(data.cmd[data.NBcmd].info,"post image semaphore loop. If sem index = -1, post all semaphores");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <sem index> <time interval [us]>");
    strcpy(data.cmd[data.NBcmd].example,"imsetsempost im1 2");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_sempost(char *IDname, long index, long dtus)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imsetsemwait");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_semwait_cli;
    strcpy(data.cmd[data.NBcmd].info,"wait image semaphore");
    strcpy(data.cmd[data.NBcmd].syntax,"<image>");
    strcpy(data.cmd[data.NBcmd].example,"imsetsemwait im1");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_semwait(char *IDname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"imsetsemflush");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_set_semflush_cli;
    strcpy(data.cmd[data.NBcmd].info,"flush image semaphore");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <sem index>");
    strcpy(data.cmd[data.NBcmd].example,"imsetsemflush im1 0");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_set_semflush(char *IDname, long index)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"imcp2shm");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_cp2shm_cli;
    strcpy(data.cmd[data.NBcmd].info,"copy image ot shared memory");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <shared mem image>");
    strcpy(data.cmd[data.NBcmd].example,"imcp2shm im1 ims1");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_cp2shm(char *IDname, char *IDshmname)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"creaimstream");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_streamupdateloop_cli;
    strcpy(data.cmd[data.NBcmd].info,"create 2D image stream from 3D cube");
    strcpy(data.cmd[data.NBcmd].syntax,"<image3d in> <image2d out> <interval [us]>");
    strcpy(data.cmd[data.NBcmd].example,"creaimstream imcube imstream 1000");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_streamupdateloop(char *IDinname, char *IDoutname, long usperiod)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"imnetwtransmit");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_NETWORKtransmit_cli;
    strcpy(data.cmd[data.NBcmd].info,"transmit image over network");
    strcpy(data.cmd[data.NBcmd].syntax,"<image> <IP addr> <port [long]> <mode [int]>");
    strcpy(data.cmd[data.NBcmd].example,"imnetwtransmit im1 127.0.0.1 0 8888");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_NETWORKtransmit(char *IDname, char *IPaddr, int port, int mode)");
    data.NBcmd++;



    strcpy(data.cmd[data.NBcmd].key,"imnetwreceive");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_image_NETWORKreceive_cli;
    strcpy(data.cmd[data.NBcmd].info,"receive image(s) over network");
    strcpy(data.cmd[data.NBcmd].syntax,"<port [long]> <mode [int]>");
    strcpy(data.cmd[data.NBcmd].example,"imnetwreceive 8887 0");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_image_NETWORKreceive(int port, int mode)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"impixdecodeU");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_PixMapDecode_U_cli;
    strcpy(data.cmd[data.NBcmd].info,"decode image stream");
    strcpy(data.cmd[data.NBcmd].syntax,"<in stream> <xsize [long]> <ysize [long]> <nbpix per slice [ASCII file]> <decode map> <out stream> <out image slice index [FITS]>");
    strcpy(data.cmd[data.NBcmd].example,"impixdecodeU streamin 120 120 pixsclienb.txt decmap outim outsliceindex.fits");
    strcpy(data.cmd[data.NBcmd].Ccall,"COREMOD_MEMORY_PixMapDecode_U(char *inputstream_name, long xsizeim, long ysizeim, char* NBpix_fname, char* IDmap_name, char *IDout_name, char *IDout_pixslice_fname)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"shmimstreamlog");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_sharedMem_2Dim_log_cli;
    strcpy(data.cmd[data.NBcmd].info,"logs shared memory stream (run in current directory)");
    strcpy(data.cmd[data.NBcmd].syntax,"<shm image> <cubesize [long]> <logdir>");
    strcpy(data.cmd[data.NBcmd].example,"shmimstreamlog wfscamim 10000 /media/data");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_MEMORY_sharedMem_2Dim_log(char *IDname, long zsize, char *logdir)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"shmimslogstat");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_logshim_printstatus_cli;
    strcpy(data.cmd[data.NBcmd].info,"print log shared memory stream status");
    strcpy(data.cmd[data.NBcmd].syntax,"<shm image>");
    strcpy(data.cmd[data.NBcmd].example,"shmimslogstat wfscamim");
    strcpy(data.cmd[data.NBcmd].Ccall,"int COREMOD_MEMORY_logshim_printstatus(char *IDname)");
    data.NBcmd++;


    strcpy(data.cmd[data.NBcmd].key,"shmimslogonset");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_logshim_set_on_cli;
    strcpy(data.cmd[data.NBcmd].info,"set on variable in log shared memory stream");
    strcpy(data.cmd[data.NBcmd].syntax,"<shm image> <setv [long]>");
    strcpy(data.cmd[data.NBcmd].example,"shmimslogonset imwfs 1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int COREMOD_MEMORY_logshim_set_on(char *IDname, int setv)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"shmimslogexitset");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_MEMORY_logshim_set_logexit_cli;
    strcpy(data.cmd[data.NBcmd].info,"set exit variable in log shared memory stream");
    strcpy(data.cmd[data.NBcmd].syntax,"<shm image> <setv [long]>");
    strcpy(data.cmd[data.NBcmd].example,"shmimslogexitset imwfs 1");
    strcpy(data.cmd[data.NBcmd].Ccall,"int COREMOD_MEMORY_logshim_set_logexit(char *IDname, int setv)");
    data.NBcmd++;






    // add atexit functions here

    return 0;
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




long image_ID(char *name) /* ID number corresponding to a name */
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


long image_ID_noaccessupdate(char *name) /* ID number corresponding to a name */
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


long variable_ID(char *name) /* ID number corresponding to a name */
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




int delete_image_ID(char* imname) /* deletes an ID */
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

        for(s=0; s<data.image[ID].sem; s++)
            sem_close(data.image[ID].semptr[s]);

        data.image[ID].sem = 0;

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
            if(data.image[ID].md[0].atype==CHAR)
            {
                if(data.image[ID].array.C == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.C);
                data.image[ID].array.C = NULL;
            }
            if(data.image[ID].md[0].atype==INT)
            {
                if(data.image[ID].array.I == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.I);
                data.image[ID].array.I = NULL;
            }
            if(data.image[ID].md[0].atype==FLOAT)
            {
                if(data.image[ID].array.F == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.F);
                data.image[ID].array.F = NULL;
            }
            if(data.image[ID].md[0].atype==DOUBLE)
            {
                if(data.image[ID].array.D == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.D);
                data.image[ID].array.D = NULL;
            }
            if(data.image[ID].md[0].atype==COMPLEX_FLOAT)
            {
                if(data.image[ID].array.CF == NULL)
                {
                    printERROR(__FILE__,__func__,__LINE__,"data array pointer is null\n");
                    exit(0);
                }
                free(data.image[ID].array.CF);
                data.image[ID].array.CF = NULL;
            }
            if(data.image[ID].md[0].atype==COMPLEX_DOUBLE)
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
int delete_image_ID_prefix(char *prefix)
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


int delete_variable_ID(char* varname) /* deletes a variable ID */
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






/* creates an image ID */
/* all images should be created by this function */
long create_image_ID(char *name, long naxis, long *size, int atype, int shared, int NBkw)
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

    //	printf("NBkw = %ld\n", NBkw);

    ID = -1;
    if(image_ID(name) == -1)
    {
        ID = next_avail_image_ID();

        nelement = 1;
        for(i=0; i<naxis; i++)
            nelement*=size[i];

        // compute total size to be allocated
        if(shared==1)
        {
            // create semlog

            sprintf(sname, "%s_semlog", name);
            remove(sname);
            data.image[ID].semlog = NULL;

            if ((data.image[ID].semlog = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED)
                perror("semaphore creation / initilization");
            else
                sem_init(data.image[ID].semlog, 1, 0);



            sharedsize = sizeof(IMAGE_METADATA);

            if(atype==CHAR)
                sharedsize += nelement*sizeof(char);
            if(atype==INT)
                sharedsize += nelement*sizeof(int);
            if(atype==FLOAT)
                sharedsize += nelement*sizeof(float);
            if(atype==DOUBLE)
                sharedsize += nelement*sizeof(double);
            if(atype==COMPLEX_FLOAT)
                sharedsize += nelement*2*sizeof(float);
            if(atype==COMPLEX_DOUBLE)
                sharedsize += nelement*2*sizeof(double);
            if(atype==USHORT)
                sharedsize += nelement*sizeof(unsigned short int);
            if(atype==LONG)
                sharedsize += nelement*sizeof(long);


            sharedsize += NBkw*sizeof(IMAGE_KEYWORD);


            sprintf(SM_fname, "%s/%s.im.shm", SHAREDMEMDIR, name);
            SM_fd = open(SM_fname, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

            if (SM_fd == -1) {
                perror("Error opening file for writing");
                exit(0);
            }

            data.image[ID].sem = 0;
            data.image[ID].shmfd = SM_fd;
            data.image[ID].memsize = sharedsize;

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

            map = (IMAGE_METADATA*) mmap(0, sharedsize, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
            if (map == MAP_FAILED) {
                close(SM_fd);
                perror("Error mmapping the file");
                exit(0);
            }

            data.image[ID].md = (IMAGE_METADATA*) map;
            data.image[ID].md[0].shared = 1;
        }
        else
        {
            data.image[ID].md = (IMAGE_METADATA*) malloc(sizeof(IMAGE_METADATA));
            data.image[ID].md[0].shared = 0;
            if(NBkw>0)
                data.image[ID].kw = (IMAGE_KEYWORD*) malloc(sizeof(IMAGE_KEYWORD)*NBkw);
            else
                data.image[ID].kw = NULL;
        }

        data.image[ID].md[0].atype = atype;
        data.image[ID].md[0].naxis = naxis;
        strcpy(data.image[ID].name, name); // local name
        strcpy(data.image[ID].md[0].name, name);
        for(i=0; i<naxis; i++)
            data.image[ID].md[0].size[i] = size[i];
        data.image[ID].md[0].NBkw = NBkw;

        //data.image[ID].logstatus = (int*) malloc(sizeof(int));
        //data.image[ID].logstatus[0] = 1; // default value

        if(atype==CHAR)
        {
            if(shared==1)
                data.image[ID].array.C = (char*) (map + sizeof(IMAGE));
            else
                data.image[ID].array.C = (char*) calloc ((size_t) nelement, sizeof(char));


            if(data.image[ID].array.C == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(char));
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==INT)
        {
            if(shared==1)
                data.image[ID].array.I = (int*) (map + sizeof(IMAGE));
            else
                data.image[ID].array.I = (int*) calloc ((size_t) nelement,sizeof(int));

            if(data.image[ID].array.I == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(int));
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==LONG)
        {
            if(shared==1)
                data.image[ID].array.L = (long*) (map + sizeof(IMAGE));
            else
                data.image[ID].array.L = (long*) calloc ((size_t) nelement,sizeof(long));

            if(data.image[ID].array.L == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(long));
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==FLOAT)	{
            if(shared==1)
            {
                mapv = (char*) map;
                mapv += sizeof(IMAGE_METADATA);
                data.image[ID].array.F = (float*) (mapv);
                memset(data.image[ID].array.F, '\0', nelement*sizeof(float));
                mapv += sizeof(float)*nelement;
                data.image[ID].kw = (IMAGE_KEYWORD*) (mapv);
            }
            else
                data.image[ID].array.F = (float*) calloc ((size_t) nelement, sizeof(float));

            if(data.image[ID].array.F == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(float));
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==DOUBLE)
        {
            if(shared==1)
                data.image[ID].array.D = (double*) (map + sizeof(IMAGE));
            else
                data.image[ID].array.D = (double*) calloc ((size_t) nelement,sizeof(double));
            if(data.image[ID].array.D == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(double));
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==COMPLEX_FLOAT)
        {
            if(shared==1)
                data.image[ID].array.CF = (complex_float*) (map + sizeof(IMAGE));
            else
                data.image[ID].array.CF = (complex_float*) calloc ((size_t) nelement,sizeof(complex_float));
            for(ii=0; ii<nelement; ii++)
            {
                data.image[ID].array.CF[ii].re = 0.0;
                data.image[ID].array.CF[ii].im = 0.0;
            }
            if(data.image[ID].array.CF == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(float)*2);
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==COMPLEX_DOUBLE)
        {
            if(shared==1)
                data.image[ID].array.CD = (complex_double*) (map + sizeof(IMAGE));
            else
                data.image[ID].array.CD = (complex_double*) calloc ((size_t) nelement,sizeof(complex_double));
            if(data.image[ID].array.CD == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(double)*2);
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }
        if(atype==USHORT)
        {
            if(shared==1)
            {
                mapv = (char*) map;
                mapv += sizeof(IMAGE_METADATA);
                data.image[ID].array.U = (unsigned short*) (mapv);
                memset(data.image[ID].array.U, '\0', nelement*sizeof(unsigned short));
                mapv += sizeof(unsigned short)*nelement;
                data.image[ID].kw = (IMAGE_KEYWORD*) (mapv);
            }
            else
                data.image[ID].array.U = (unsigned short*) calloc ((size_t) nelement, sizeof(unsigned short));

            if(data.image[ID].array.U == NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"memory allocation failed");
                fprintf(stderr,"%c[%d;%dm", (char) 27, 1, 31);
                fprintf(stderr,"Image name = %s\n",name);
                fprintf(stderr,"Image size = ");
                fprintf(stderr,"%ld",size[0]);
                for(i=1; i<naxis; i++)
                    fprintf(stderr,"x%ld",size[i]);
                fprintf(stderr,"\n");
                fprintf(stderr,"Requested memory size = %ld elements = %f Mb\n",nelement,1.0/1024/1024*nelement*sizeof(double)*2);
                fprintf(stderr," %c[%d;m",(char) 27, 0);
                list_image_ID();
                exit(0);
            }
        }

        clock_gettime(CLOCK_REALTIME, &timenow);
        data.image[ID].md[0].last_access = 1.0*timenow.tv_sec + 0.000000001*timenow.tv_nsec;
        data.image[ID].md[0].creation_time = data.image[ID].md[0].last_access;
        data.image[ID].md[0].write = 0;
        data.image[ID].md[0].cnt0 = 0;
        data.image[ID].md[0].cnt1 = 0;
        data.image[ID].md[0].nelement = nelement;
    
		if(shared==1)
			COREMOD_MEMORY_image_set_createsem(name, 10);
    }
    else
    {
        //      printf("Cannot create image : name \"%s\" already in use\n",name);
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
                fprintf(stderr,"Axis %ld :  %ld  %ld\n", i, data.image[ID].md[0].size[i], size[i]);
                exit(0);
            }
    }


    // initialize keywords (test)
    for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
        data.image[ID].kw[kw].type = 'N';
    /*      {
            sprintf(kname, "KEY%d", kw);
            strcpy(data.image[ID].kw[kw].name, kname);
            data.image[ID].kw[kw].type = 'D';
            data.image[ID].kw[kw].value.f.numf = 1.0*kw;
            sprintf(comment, "this is keyword %d", kw);
            strcpy(data.image[ID].kw[kw].comment, comment);
          }
        */

	


    if(MEM_MONITOR == 1)
        list_image_ID_ncurses();

    return(ID);
}





long image_write_keyword_L(char *IDname, char *kname, long value, char *comment)
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

long image_write_keyword_D(char *IDname, char *kname, double value, char *comment)
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

long image_write_keyword_S(char *IDname, char *kname, char *value, char *comment)
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




long image_list_keywords(char *IDname)
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


long image_read_keyword_D(char *IDname, char *kname, double *val)
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


long image_read_keyword_L(char *IDname, char *kname, long *val)
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







long read_sharedmem_image_size(char *name, char *fname)
{
    int SM_fd;
    struct stat file_stat;
    char SM_fname[200];
    IMAGE_METADATA *map;
    int naxis;
    int i;
    FILE *fp;


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
        for(i=0;i<map[0].naxis;i++)
            fprintf(fp, "%ld ", map[0].size[i]);
        fprintf(fp, "\n"); 
        fclose(fp);
 
 
        if (munmap(map, sizeof(IMAGE_METADATA)) == -1) {
            printf("unmapping %s\n", SM_fname);
            perror("Error un-mmapping the file");
        }
        close(SM_fd);
    }
    
    return 0;
}





long read_sharedmem_image(char *name)
{
    long ID = -1;
    int SM_fd;
    struct stat file_stat;
    char SM_fname[200];
    IMAGE_METADATA *map;
    char *mapv;
    int atype;
    int kw;
    char sname[200];
    sem_t *stest;
    int sOK;
    long snb;
    long s;
 
    //	int *vint;

    ID = next_avail_image_ID();


    sprintf(SM_fname, "%s/%s.im.shm", SHAREDMEMDIR, name);
   // printf("Importing mmap file \"%s\"\n",SM_fname);


    SM_fd = open(SM_fname, O_RDWR);
    if(SM_fd==-1)
    {
        data.image[ID].used = 0;
        ID = -1;
        printf("Cannot import shared memory file %s \n", name);
    }
    else
    {
        fstat(SM_fd, &file_stat);
        printf("File %s size: %zd\n", SM_fname, file_stat.st_size);


        map = (IMAGE_METADATA*) mmap(0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);
        if (map == MAP_FAILED) {
            close(SM_fd);
            perror("Error mmapping the file");
            exit(0);
        }

        data.image[ID].memsize = file_stat.st_size;
        data.image[ID].sem = 0;
        data.image[ID].shmfd = SM_fd;

        data.image[ID].md = map;
        atype = data.image[ID].md[0].atype;
        data.image[ID].md[0].shared = 1;

        printf("image size = %ld %ld\n", data.image[ID].md[0].size[0], data.image[ID].md[0].size[1]);
        fflush(stdout);
        // some verification
        if(data.image[ID].md[0].size[0]*data.image[ID].md[0].size[1]>10000000000)
        {
            printf("IMAGE \"%s\" SEEMS BIG... ABORTING\n", name);
            exit(0);
        }
        if(data.image[ID].md[0].size[0]<1)
        {
            printf("IMAGE \"%s\" AXIS SIZE < 1... ABORTING\n", name);
            exit(0);
        }
        if(data.image[ID].md[0].size[1]<1)
        {
            printf("IMAGE \"%s\" AXIS SIZE < 1... ABORTING\n", name);
            exit(0);
        }

        mapv = (char*) map;
        mapv += sizeof(IMAGE_METADATA);


        printf("atype = %d\n", atype);
        fflush(stdout);
        if(atype==CHAR)
        {
            printf("atype = CHAR\n");
            data.image[ID].array.C = (char*) mapv;
            mapv += sizeof(char)*data.image[ID].md[0].nelement;
        }
        if(atype==INT)
        {
            printf("atype = INT\n");
            data.image[ID].array.I = (int*) mapv;
            mapv += sizeof(int)*data.image[ID].md[0].nelement;
        }
        if(atype==FLOAT)
        {
            printf("atype = FLOAT\n");
            data.image[ID].array.F = (float*) mapv;
            mapv += sizeof(float)*data.image[ID].md[0].nelement;
        }
        if(atype==DOUBLE)
        {
            printf("atype = DOUBLE\n");
            data.image[ID].array.D = (double*) mapv;
            mapv += sizeof(double)*data.image[ID].md[0].nelement;
        }
        if(atype==COMPLEX_FLOAT)
        {
            printf("atype = COMPLEX_FLOAT\n");
            data.image[ID].array.CF = (complex_float*) mapv;
            mapv += sizeof(complex_float)*data.image[ID].md[0].nelement;
        }
        if(atype==COMPLEX_DOUBLE)
        {
            printf("atype = COMPLEX_DOUBLE\n");
            data.image[ID].array.CD = (complex_double*) mapv;
            mapv += sizeof(complex_double)*data.image[ID].md[0].nelement;
        }
        if(atype==USHORT)
        {
            printf("atype = USHORT\n");
            data.image[ID].array.U = (unsigned short*) mapv;
            mapv += sizeof(unsigned short)*data.image[ID].md[0].nelement;
        }


        printf("%ld keywords\n", data.image[ID].md[0].NBkw);
        fflush(stdout);

        data.image[ID].kw = (IMAGE_KEYWORD*) (mapv);

        for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
        {
            if(data.image[ID].kw[kw].type == 'L')
                printf("%d  %s %ld %s\n", kw, data.image[ID].kw[kw].name, data.image[ID].kw[kw].value.numl, data.image[ID].kw[kw].comment);
            if(data.image[ID].kw[kw].type == 'D')
                printf("%d  %s %lf %s\n", kw, data.image[ID].kw[kw].name, data.image[ID].kw[kw].value.numf, data.image[ID].kw[kw].comment);
            if(data.image[ID].kw[kw].type == 'S')
                printf("%d  %s %s %s\n", kw, data.image[ID].kw[kw].name, data.image[ID].kw[kw].value.valstr, data.image[ID].kw[kw].comment);
        }


        mapv += sizeof(IMAGE_KEYWORD)*data.image[ID].md[0].NBkw;

        strcpy(data.image[ID].name, name);

        if(MEM_MONITOR == 1)
            list_image_ID_ncurses();

        // looking for semaphores
        snb = 0;
        sOK = 1;
        while(sOK==1)
        {
            //printf("name = %s\n", data.image[ID].md[0].name);
            sprintf(sname, "%s_sem%02ld", data.image[ID].md[0].name, snb);
            if((stest = sem_open(sname, 0, 0644, 0))== SEM_FAILED)
                sOK = 0;
            else
                {
                    sem_close(stest);
                    snb++;
                }
        }
        printf("%ld semaphores detected\n", snb);

        data.image[ID].sem = snb;
        data.image[ID].semptr = (sem_t**) malloc(sizeof(sem_t*)*data.image[ID].sem);
        for(s=0; s<snb; s++)
        {
            sprintf(sname, "%s_sem%02ld", data.image[ID].md[0].name, s);
            if ((data.image[ID].semptr[s] = sem_open(sname, 0, 0644, 0))== SEM_FAILED) {
                printf("ERROR: could not open semaphore %s\n", sname);
            }
        }
        
        sprintf(sname, "%s_semlog", data.image[ID].md[0].name);
        if ((data.image[ID].semlog = sem_open(sname, 0, 0644, 0))== SEM_FAILED) {
                printf("ERROR: could not open semaphore %s\n", sname);
            }
        

    }

    return(ID);
}









long create_1Dimage_ID(char *ID_name, long xsize)
{
    long ID = -1;
    long naxis = 1;
    long naxes[1];

    naxes[0]=xsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name, naxis, naxes, 3, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name, naxis, naxes, 4, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

long create_1DCimage_ID(char *ID_name, long xsize)
{
    long ID = -1;
    long naxis=1;
    long naxes[1];

    naxes[0]=xsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name,naxis,naxes,5, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name,naxis,naxes,6, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

long create_2Dimage_ID(char *ID_name, long xsize, long ysize)
{
    long ID = -1;
    long naxis=2;
    long naxes[2];

    naxes[0]=xsize;
    naxes[1]=ysize;

    // printf("Creating 2D image %s, %ld x %ld [%d %d]", ID_name, xsize, ysize, data.SHARED_DFT, data.NBKEWORD_DFT);
    // fflush(stdout);

    if(data.precision == 0)
        ID = create_image_ID(ID_name, naxis, naxes, 3, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    else if (data.precision == 1)
        ID = create_image_ID(ID_name, naxis, naxes, 4, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision
    else
    {
        printf("Default precision (%d) invalid value: assuming single precision\n", data.precision);
        ID = create_image_ID(ID_name, naxis, naxes, 3, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    }

    //  printf("\n");
    // fflush(stdout);



    return(ID);
}

long create_2Dimage_ID_double(char *ID_name, long xsize, long ysize)
{
    long ID = -1;
    long naxis=2;
    long naxes[2];

    naxes[0]=xsize;
    naxes[1]=ysize;

    ID = create_image_ID(ID_name,naxis,naxes,4, data.SHARED_DFT, data.NBKEWORD_DFT);

    return(ID);
}


/* 2D complex image */
long create_2DCimage_ID(char *ID_name, long xsize, long ysize)
{
    long ID = -1;
    long naxis=2;
    long naxes[2];

    naxes[0]=xsize;
    naxes[1]=ysize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name,naxis,naxes, 5, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name,naxis,naxes, 6, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

/* 2D complex image */
long create_2DCimage_ID_double(char *ID_name, long xsize, long ysize)
{
    long ID = -1;
    long naxis=2;
    long naxes[2];

    naxes[0]=xsize;
    naxes[1]=ysize;

    ID = create_image_ID(ID_name,naxis,naxes, 6, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}



/* 3D image, single precision */
long create_3Dimage_ID_float(char *ID_name, long xsize, long ysize, long zsize)
{
    long ID = -1;
    long naxis=3;
    long naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    //  printf("CREATING 3D IMAGE: %s %ld %ld %ld\n", ID_name, xsize, ysize, zsize);
    //  fflush(stdout);

    ID = create_image_ID(ID_name,naxis,naxes,3, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision

    //  printf("IMAGE CREATED WITH ID = %ld\n",ID);
    //  fflush(stdout);

    return(ID);
}


/* 3D image, double precision */
long create_3Dimage_ID_double(char *ID_name, long xsize, long ysize, long zsize)
{
    long ID;
    long naxis=3;
    long naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    ID = create_image_ID(ID_name,naxis,naxes,4, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}



/* 3D image, default precision */
long create_3Dimage_ID(char *ID_name, long xsize, long ysize, long zsize)
{
    long ID = -1;
    long naxis=3;
    long naxes[3];

    naxes[0]=xsize;
    naxes[1]=ysize;
    naxes[2]=zsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name,naxis,naxes,3, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name,naxis,naxes,4, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}

/* 3D complex image */
long create_3DCimage_ID(char *ID_name, long xsize, long ysize, long zsize)
{
    long ID = -1;
    long naxis=3;
    long naxes[3];

    naxes[0]=xsize;
    naxes[1]=ysize;
    naxes[2]=zsize;

    if(data.precision == 0)
        ID = create_image_ID(ID_name,naxis,naxes,5, data.SHARED_DFT, data.NBKEWORD_DFT); // single precision
    if(data.precision == 1)
        ID = create_image_ID(ID_name,naxis,naxes,6, data.SHARED_DFT, data.NBKEWORD_DFT); // double precision

    return(ID);
}



long copy_image_ID(char *name, char *newname, int shared)
{
    long ID, IDout;
    long naxis;
    long *size = NULL;
    int atype;
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

    size = (long*) malloc(sizeof(long)*naxis);
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

    if(atype==CHAR)
        memcpy (data.image[IDout].array.C, data.image[ID].array.C, sizeof(char)*nelement);

    if(atype==INT)
        memcpy (data.image[IDout].array.I, data.image[ID].array.I, sizeof(int)*nelement);

    if(atype==FLOAT)
        memcpy (data.image[IDout].array.F, data.image[ID].array.F, sizeof(float)*nelement);

    if(atype==DOUBLE)
        memcpy (data.image[IDout].array.D, data.image[ID].array.D, sizeof(double)*nelement);

    if(atype==COMPLEX_FLOAT)
        memcpy (data.image[IDout].array.CF, data.image[ID].array.CF, sizeof(float)*2*nelement);

    if(atype==COMPLEX_DOUBLE)
        memcpy (data.image[IDout].array.CD, data.image[ID].array.CD, sizeof(double)*2*nelement);

    if(atype==USHORT)
        memcpy (data.image[IDout].array.U, data.image[ID].array.U, sizeof(double)*nelement);

    for(s=0;s<data.image[IDout].sem; s++)
    {
        sem_getvalue(data.image[IDout].semptr[s], &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(data.image[IDout].semptr[s]);
    }
    
    if(data.image[IDout].semlog!=NULL)
    {
        sem_getvalue(data.image[IDout].semlog, &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(data.image[IDout].semlog);
    }
    
    COREMOD_MEMORY_image_set_sempost_byID(IDout, -1);
    data.image[IDout].md[0].write = 0;
    data.image[IDout].md[0].cnt0++;

    free(size);

    return(IDout);
}




/* creates floating point variable */
long create_variable_ID(char *name, double value)
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
long create_variable_long_ID(char *name, long value)
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
long create_variable_string_ID(char *name, char *value)
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











int init_list_image_ID_ncurses(char *termttyname)
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

int list_image_ID_ncurses()
{
    char str[500];
    long i, j;
    long long tmp_long;
    char type[STYPESIZE];
    int atype;
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

            sprintf(str, "[ %6ld",data.image[i].md[0].size[0]);

            for(j=1; j<data.image[i].md[0].naxis; j++)
            {
                sprintf(str, "%s x %6ld", str, data.image[i].md[0].size[j]);
            }
            sprintf(str, "%s]", str);

            printw("%-28s", str);

            attron(COLOR_PAIR(3));
            n = 0;
            if(atype==CHAR)
                n = snprintf(type,STYPESIZE,"CHAR   ");
            if(atype==INT)
                n = snprintf(type,STYPESIZE,"INT    ");
            if(atype==LONG)
                n = snprintf(type,STYPESIZE,"LONG   ");
            if(atype==FLOAT)
                n = snprintf(type,STYPESIZE,"FLOAT  ");
            if(atype==DOUBLE)
                n = snprintf(type,STYPESIZE,"DOUBLE ");
            if(atype==COMPLEX_FLOAT)
                n = snprintf(type,STYPESIZE,"CFLOAT ");
            if(atype==COMPLEX_DOUBLE)
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
    if(sizeGb>0)
        sprintf(str, "%s %ld GB", str, (long) (sizeGb));

    if(sizeMb>0)
        sprintf(str, "%s %ld MB", str, (long) (sizeMb));

    if(sizeKb>0)
        sprintf(str, "%s %ld KB", str, (long) (sizeKb));

    if(sizeb>0)
        sprintf(str, "%s %ld B", str, (long) (sizeb));

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


int memory_monitor(char *termttyname)
{
    if(data.Debug>0)
        printf("starting memory_monitor on \"%s\"\n", termttyname);

    MEM_MONITOR = 1;
    init_list_image_ID_ncurses(termttyname);
    list_image_ID_ncurses();
    atexit(close_list_image_ID_ncurses);

}







int list_image_ID_ofp(FILE *fo)
{
    long i,j;
    long long tmp_long;
    char type[STYPESIZE];
    int atype;
    int n;
    long long sizeb, sizeKb, sizeMb, sizeGb;
    char str[500];
    struct timespec timenow;
    double timediff;

    sizeb = compute_image_memory();


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

            sprintf(str, "[ %6ld",data.image[i].md[0].size[0]);

            for(j=1; j<data.image[i].md[0].naxis; j++)
            {
                sprintf(str, "%s x %6ld", str, data.image[i].md[0].size[j]);
            }
            sprintf(str, "%s]", str);

            fprintf(fo, "%-32s", str);


            n = 0;
            if(atype==CHAR)
                n = snprintf(type,STYPESIZE,"CHAR");
            if(atype==INT)
                n = snprintf(type,STYPESIZE,"INT");
            if(atype==LONG)
                n = snprintf(type,STYPESIZE,"LONG");
            if(atype==FLOAT)
                n = snprintf(type,STYPESIZE,"FLOAT");
            if(atype==DOUBLE)
                n = snprintf(type,STYPESIZE,"DOUBLE");
            if(atype==COMPLEX_FLOAT)
                n = snprintf(type,STYPESIZE,"CFLOAT");
            if(atype==COMPLEX_DOUBLE)
                n = snprintf(type,STYPESIZE,"CDOUBLE");
            if(atype==USHORT)
                n = snprintf(type,STYPESIZE,"USHORT");
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
    fprintf(fo, "\n\n");

    fflush(fo);

    return(0);
}

int list_image_ID_ofp_simple(FILE *fo)
{
    long i,j;
    long long tmp_long;
    int atype;

    for (i=0; i<data.NB_MAX_IMAGE; i++)
        if(data.image[i].used==1)
        {
            atype = data.image[i].md[0].atype;
            tmp_long = ((long long) (data.image[i].md[0].nelement)) * TYPESIZE[atype];

            fprintf(fo, "%20s %d %ld %d %4ld", data.image[i].name, atype, data.image[i].md[0].naxis, data.image[i].md[0].shared, data.image[i].md[0].size[0]);

            for(j=1; j<data.image[i].md[0].naxis; j++)
                fprintf(fo, " %4ld", data.image[i].md[0].size[j]);
            fprintf(fo, "\n");
        }
    fprintf(fo, "\n");

    return(0);
}



int list_image_ID()
{
    list_image_ID_ofp(stdout);
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

int list_image_ID_file(char *fname)
{
    FILE *fp;
    long i,j;
    int atype;
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
            fprintf(fp,"%ld %s",i, data.image[i].name);
            fprintf(fp," %ld",data.image[i].md[0].naxis);
            for(j=0; j<data.image[i].md[0].naxis; j++)
                fprintf(fp," %ld",data.image[i].md[0].size[j]);

            n = 0;
            if(atype==CHAR)
                n = snprintf(type,STYPESIZE,"CHAR");
            if(atype==INT)
                n = snprintf(type,STYPESIZE,"INT");
            if(atype==LONG)
                n = snprintf(type,STYPESIZE,"LONG");
            if(atype==FLOAT)
                n = snprintf(type,STYPESIZE,"FLOAT");
            if(atype==DOUBLE)
                n = snprintf(type,STYPESIZE,"DOUBLE");
            if(atype==COMPLEX_FLOAT)
                n = snprintf(type,STYPESIZE,"CFLOAT");
            if(atype==COMPLEX_DOUBLE)
                n = snprintf(type,STYPESIZE,"CDOUBLE");

            if(n >= STYPESIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

            fprintf(fp," %s\n",type);
        }
    fclose(fp);

    return(0);
}


int list_variable_ID()
{
    long i;

    for (i=0; i<data.NB_MAX_VARIABLE; i++)
        if(data.variable[i].used == 1)
            printf("%4ld %16s %25.18g\n",i, data.variable[i].name,data.variable[i].value.f);

    return(0);
}


int list_variable_ID_file(char *fname)
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



long chname_image_ID(char *ID_name, char *new_name)
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



int mk_complex_from_reim(char *re_name, char *im_name, char *out_name, int sharedmem)
{
    long IDre,IDim,IDout;
    long *naxes = NULL;
    long naxis;
    long nelement;
    long ii;
    long i;
    int n;
    int atype_re, atype_im, atype_out;

    IDre = image_ID(re_name);
    IDim = image_ID(im_name);

    atype_re = data.image[IDre].md[0].atype;
    atype_im = data.image[IDim].md[0].atype;
    naxis = data.image[IDre].md[0].naxis;

    naxes = (long *) malloc(sizeof(long)*naxis);
    if(naxes==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"malloc error");
        exit(0);
    }

    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDre].md[0].size[i];
    nelement = data.image[IDre].md[0].nelement;


    if((atype_re==FLOAT)&&(atype_im==FLOAT))
    {
        atype_out = COMPLEX_FLOAT;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CF[ii].re = data.image[IDre].array.F[ii];
            data.image[IDout].array.CF[ii].im = data.image[IDim].array.F[ii];
        }
    }
    else if((atype_re==FLOAT)&&(atype_im==DOUBLE))
    {
        atype_out = COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CD[ii].re = data.image[IDre].array.F[ii];
            data.image[IDout].array.CD[ii].im = data.image[IDim].array.D[ii];
        }
    }
    else if((atype_re==DOUBLE)&&(atype_im==FLOAT))
    {
        atype_out = COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
        for(ii=0; ii<nelement; ii++)
        {
            data.image[IDout].array.CD[ii].re = data.image[IDre].array.D[ii];
            data.image[IDout].array.CD[ii].im = data.image[IDim].array.F[ii];
        }
    }
    else if((atype_re==DOUBLE)&&(atype_im==DOUBLE))
    {
        atype_out = COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
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





int mk_complex_from_amph(char *am_name, char *ph_name, char *out_name, int sharedmem)
{
    long IDam,IDph,IDout;
    long naxes[3];
    long naxis;
    long nelement;
    long ii;
    long i;
    int atype_am, atype_ph, atype_out;
    int n;

    IDam = image_ID(am_name);
    IDph = image_ID(ph_name);
    atype_am = data.image[IDam].md[0].atype;
    atype_ph = data.image[IDph].md[0].atype;

    naxis = data.image[IDam].md[0].naxis;
    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDam].md[0].size[i];
    nelement = data.image[IDam].md[0].nelement;

    if((atype_am==FLOAT)&&(atype_ph==FLOAT))
    {
        atype_out = COMPLEX_FLOAT;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
        
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
    else if((atype_am==FLOAT)&&(atype_ph==DOUBLE))
    {
        atype_out = COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
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
    else if((atype_am==DOUBLE)&&(atype_ph==FLOAT))
    {
        atype_out = COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name,naxis,naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
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
    else if((atype_am==DOUBLE)&&(atype_ph==DOUBLE))
    {
        atype_out = COMPLEX_DOUBLE;
        IDout = create_image_ID(out_name, naxis, naxes,atype_out, sharedmem, data.NBKEWORD_DFT);
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

int mk_reim_from_complex(char *in_name, char *re_name, char *im_name, int sharedmem)
{
    long IDre,IDim,IDin;
    long naxes[3];
    long naxis;
    long nelement;
    long ii;
    long i;
    long atype;
    int n;

    IDin = image_ID(in_name);
    atype = data.image[IDin].md[0].atype;
    naxis = data.image[IDin].md[0].naxis;
    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDin].md[0].size[i];
    nelement = data.image[IDin].md[0].nelement;

    if(atype == COMPLEX_FLOAT) // single precision
    {
        IDre = create_image_ID(re_name,naxis,naxes,FLOAT, sharedmem, data.NBKEWORD_DFT);
        IDim = create_image_ID(im_name,naxis,naxes,FLOAT, sharedmem, data.NBKEWORD_DFT);
        
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
    else if(atype==COMPLEX_DOUBLE) // double precision
    {
        IDre = create_image_ID(re_name,naxis,naxes,DOUBLE, sharedmem, data.NBKEWORD_DFT);
        IDim = create_image_ID(im_name,naxis,naxes,DOUBLE, sharedmem, data.NBKEWORD_DFT);
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




int mk_amph_from_complex(char *in_name, char *am_name, char *ph_name, int sharedmem)
{
    long IDam,IDph,IDin;
    long naxes[3];
    long naxis;
    long nelement;
    long ii;
    long i;
    float amp_f,pha_f;
    double amp_d,pha_d;
    int atype;
    int n;

    IDin = image_ID(in_name);
    atype = data.image[IDin].md[0].atype;
    naxis = data.image[IDin].md[0].naxis;

    for(i=0; i<naxis; i++)
        naxes[i] = data.image[IDin].md[0].size[i];
    nelement = data.image[IDin].md[0].nelement;

    if(atype==COMPLEX_FLOAT) // single precision
    {
        IDam = create_image_ID(am_name, naxis, naxes, FLOAT, sharedmem, data.NBKEWORD_DFT);
        IDph = create_image_ID(ph_name, naxis, naxes, FLOAT, sharedmem, data.NBKEWORD_DFT);
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
    else if(atype==COMPLEX_DOUBLE) // double precision
    {
        IDam = create_image_ID(am_name,naxis,naxes,DOUBLE, sharedmem, data.NBKEWORD_DFT);
        IDph = create_image_ID(ph_name,naxis,naxes,DOUBLE, sharedmem, data.NBKEWORD_DFT);
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




int mk_reim_from_amph(char *am_name, char *ph_name, char *re_out_name, char *im_out_name, int sharedmem)
{
    mk_complex_from_amph(am_name, ph_name, "Ctmp", 0);
    mk_reim_from_complex("Ctmp", re_out_name, im_out_name, sharedmem);
    delete_image_ID("Ctmp");

    return(0);
}

int mk_amph_from_reim(char *re_name, char *im_name, char *am_out_name, char *ph_out_name, int sharedmem)
{
    mk_complex_from_reim(re_name, im_name, "Ctmp", 0);
    mk_amph_from_complex("Ctmp",am_out_name, ph_out_name, sharedmem);
    delete_image_ID("Ctmp");

    return(0);
}

int clearall()
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


//  check only is size > 0
int check_2Dsize(char *ID_name, long xsize, long ysize)
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

int check_3Dsize(char *ID_name, long xsize, long ysize, long zsize)
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

int rotate_cube(char *ID_name, char *ID_out_name, int orientation)
{
    /* 0 is from x axis */
    /* 1 is from y axis */
    long ID,IDout;
    long xsize,ysize,zsize;
    long xsize1,ysize1,zsize1;
    long ii,jj,kk;
    int atype;
    int n;

    ID = image_ID(ID_name);
    atype = data.image[ID].md[0].atype;

    if(data.image[ID].md[0].naxis!=3)
    {
        n = snprintf(errmsg_memory,SBUFFERSIZE,"Wrong naxis : %ld - should be 3\n",data.image[ID].md[0].naxis);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

        printERROR(__FILE__,__func__,__LINE__,errmsg_memory);
        exit(0);
    }
    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];
    zsize = data.image[ID].md[0].size[2];

    if(atype==FLOAT) // single precision
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
    else if(atype==DOUBLE)
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








void *save_fits_function( void *ptr )
{
    long ID;
    struct savethreadmsg *tmsg = malloc(sizeof(struct savethreadmsg));
    long *imsizearray;
    long xsize, ysize;
    int atype;
    long IDc;
    long framesize; // in bytes
    char *ptr0; // source
    char *ptr1; // destination


    imsizearray = (long*) malloc(sizeof(long)*3);

    tmsg = (struct savethreadmsg*) ptr;
    // printf("THREAD : SAVING  %s -> %s \n", tmsg->iname, tmsg->fname);
    //fflush(stdout);
    if(tmsg->partial==0) // full image
        save_fits(tmsg->iname, tmsg->fname);
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
        case CHAR:
            framesize = sizeof(char)*xsize*ysize;
            break;
        case INT:
            framesize = sizeof(int)*xsize*ysize;
            break;
        case FLOAT:
            framesize = sizeof(float)*xsize*ysize;
            break;
        case DOUBLE:
            framesize = sizeof(double)*xsize*ysize;
            break;
        case USHORT:
            framesize = sizeof(unsigned short)*xsize*ysize;
            break;

        default:
            printf("ERROR: WRONG DATA TYPE\n");
            exit(0);
            break;
        }

        ptr0 = (char*) data.image[ID].array.F;  // source
        ptr1 = (char*) data.image[IDc].array.F; // destination


        memcpy((void *) ptr1, (void *) ptr0, framesize*tmsg->cubesize);
        save_fits("tmpsavecube", tmsg->fname);
        delete_image_ID("tmpsavecube");
    }


    //    printf(" DONE\n");
    //fflush(stdout);

    ID = image_ID(tmsg->iname);
    tret = ID;
    free(imsizearray);
    pthread_exit(&tret);
}


/** copy an image to shared memory
 *
 *
 */
long COREMOD_MEMORY_cp2shm(char *IDname, char *IDshmname)
{
    long ID;
    long IDshm;
    long atype;
    long naxis;
    long *sizearray;
    char *ptr1;
    char *ptr2;
    long k;
	int axis;
	int shmOK;
	

    ID = image_ID(IDname);
    naxis = data.image[ID].md[0].naxis;


    sizearray = (long*) malloc(sizeof(long)*naxis);
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
    case FLOAT :
        ptr1 = (char*) data.image[ID].array.F;
        ptr2 = (char*) data.image[IDshm].array.F;
        memcpy(ptr2, ptr1, sizeof(float)*data.image[ID].md[0].nelement);
        break;
    case DOUBLE :
        ptr1 = (char*) data.image[ID].array.D;
        ptr2 = (char*) data.image[IDshm].array.D;
        memcpy(ptr2, ptr1, sizeof(float)*data.image[ID].md[0].nelement);
        break;
    case USHORT :
        ptr1 = (char*) data.image[ID].array.U;
        ptr2 = (char*) data.image[IDshm].array.U;
        memcpy(ptr2, ptr1, sizeof(unsigned short)*data.image[ID].md[0].nelement);
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



long COREMOD_MEMORY_check_2Dsize(char *IDname, long xsize, long ysize)
{
    int sizeOK = 1; // 1 if size matches
    long ID;


    ID = image_ID(IDname);
    if(data.image[ID].md[0].naxis != 2)
    {
        printf("WARNING : image %s naxis = %ld does not match expected value 2\n", IDname, data.image[ID].md[0].naxis);
        sizeOK = 0;
    }
    if((xsize>0)&&(data.image[ID].md[0].size[0] != xsize))
    {
        printf("WARNING : image %s xsize = %ld does not match expected value %ld\n", IDname, data.image[ID].md[0].size[0], xsize);
        sizeOK = 0;
    }
    if((ysize>0)&&(data.image[ID].md[0].size[1] != ysize))
    {
        printf("WARNING : image %s ysize = %ld does not match expected value %ld\n", IDname, data.image[ID].md[0].size[1], ysize);
        sizeOK = 0;
    }

    return sizeOK;
}



long COREMOD_MEMORY_check_3Dsize(char *IDname, long xsize, long ysize, long zsize)
{
    int sizeOK = 1; // 1 if size matches
    long ID;

    ID = image_ID(IDname);
    if(data.image[ID].md[0].naxis != 3)
    {
        printf("WARNING : image %s naxis = %ld does not match expected value 3\n", IDname, data.image[ID].md[0].naxis);
        sizeOK = 0;
    }
    if((xsize>0)&&(data.image[ID].md[0].size[0] != xsize))
    {
        printf("WARNING : image %s xsize = %ld does not match expected value %ld\n", IDname, data.image[ID].md[0].size[0], xsize);
        sizeOK = 0;
    }
    if((ysize>0)&&(data.image[ID].md[0].size[1] != ysize))
    {
        printf("WARNING : image %s ysize = %ld does not match expected value %ld\n", IDname, data.image[ID].md[0].size[1], ysize);
        sizeOK = 0;
    }
    if((zsize>0)&&(data.image[ID].md[0].size[2] != zsize))
    {
        printf("WARNING : image %s zsize = %ld does not match expected value %ld\n", IDname, data.image[ID].md[0].size[2], zsize);
        sizeOK = 0;
    }

    return sizeOK;
}


long COREMOD_MEMORY_image_set_status(char *IDname, int status)
{
    long ID;

    ID = image_ID(IDname);
    data.image[ID].md[0].status = status;

    return(0);
}

long COREMOD_MEMORY_image_set_cnt0(char *IDname, int cnt0)
{
    long ID;

    ID = image_ID(IDname);
    data.image[ID].md[0].cnt0 = cnt0;

    return(0);
}

long COREMOD_MEMORY_image_set_cnt1(char *IDname, int cnt1)
{
    long ID;

    ID = image_ID(IDname);
    data.image[ID].md[0].cnt1 = cnt1;

    return(0);
}




long COREMOD_MEMORY_image_set_createsem(char *IDname, long NBsem)
{
    long ID;
    char sname[200];
    long s, s1;
    int r;
    char command[200];
    char fname[200];
    int semfile[100];

    ID = image_ID(IDname);

    if(data.image[ID].sem!=NBsem)
    {
        for(s=0; s<data.image[ID].sem; s++)
            sem_close(data.image[ID].semptr[s]);
        data.image[ID].sem = 0;

        for(s1=NBsem; s1<100; s1++)
        {
            sprintf(fname, "/dev/shm/sem.%s_sem%02ld", IDname, s1);
            remove(fname);
        }
        free(data.image[ID].semptr);
        data.image[ID].semptr = NULL;
    }

   
    if(data.image[ID].sem == 0)
    {
        if(data.image[ID].semptr!=NULL)
            free(data.image[ID].semptr);

        data.image[ID].sem = NBsem;
        printf("malloc semptr %d entries\n", data.image[ID].sem);
        data.image[ID].semptr = (sem_t**) malloc(sizeof(sem_t**)*data.image[ID].sem);


        for(s=0; s<NBsem; s++)
        {
            sprintf(sname, "%s_sem%02ld", IDname, s);
            if ((data.image[ID].semptr[s] = sem_open(sname, 0, 0644, 0))== SEM_FAILED) {
                if ((data.image[ID].semptr[s] = sem_open(sname, O_CREAT, 0644, 1)) == SEM_FAILED) {
                    perror("semaphore initilization");
                }
                else
                    sem_init(data.image[ID].semptr[s], 1, 0);
            }

        }
    }

    //printf("sem  = %d\n", data.image[ID].sem);

    return(ID);
}




// if index < 0, post all semaphores
long COREMOD_MEMORY_image_set_sempost(char *IDname, long index)
{
    long ID;
    long s;
    int semval;


    ID = image_ID(IDname);

    if(ID==-1)
        ID = read_sharedmem_image(IDname);

    if(index<0)
    {
        for(s=0; s<data.image[ID].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].sem-1)
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


// if index < 0, post all semaphores
long COREMOD_MEMORY_image_set_sempost_byID(long ID, long index)
{
    long s;
    int semval;

    if(index<0)
    {
        for(s=0; s<data.image[ID].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].sem-1)
            printf("ERROR: image ID %ld semaphore # %ld does no exist\n", ID, index);
        else
        {
            sem_getvalue(data.image[ID].semptr[index], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[index]);
        }
    }

    return(ID);
}




// if index < 0, post all semaphores

long COREMOD_MEMORY_image_set_sempost_loop(char *IDname, long index, long dtus)
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
        for(s=0; s<data.image[ID].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].sem-1)
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



long COREMOD_MEMORY_image_set_semwait(char *IDname, long index)
{
    long ID;
    int semval;

    ID = image_ID(IDname);

    if(ID==-1)
        ID = read_sharedmem_image(IDname);

    if(index>data.image[ID].sem-1)
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
        for(s=0;s<data.image[IDarray[i]].sem;s++)
        {
            sem_getvalue(data.image[IDarray[i]].semptr[s], &semval);
           printf("sem %d/%d of %s [%ld] = %d\n", s, data.image[IDarray[i]].sem, data.image[IDarray[i]].name, IDarray[i], semval);
            fflush(stdout); 
            for(cnt=0; cnt<semval; cnt++)
                sem_trywait(data.image[IDarray[i]].semptr[s]);
        }
    }

    return(0);
}



/// set semaphore value to 0
// if index <0, flush all image semaphores
long COREMOD_MEMORY_image_set_semflush(char *IDname, long index)
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
        for(s=0; s<data.image[ID].sem; s++)
        {
            sem_getvalue(data.image[ID].semptr[s], &semval);
            for(i=0; i<semval; i++)
                sem_trywait(data.image[ID].semptr[s]);
        }
    }
    else
    {
        if(index>data.image[ID].sem-1)
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




/// takes a 3Dimage (circular buffer) and writes slices to a 2D image with time interval specified in us
long COREMOD_MEMORY_image_streamupdateloop(char *IDinname, char *IDoutname, long usperiod)
{
    long IDin;
    long IDout;
    long kk;
    long *arraysize;
    long naxis;
    int atype;
    char *ptr0s; // source start 3D array ptr
    char *ptr0; // source
    char *ptr1; // dest
    long framesize;
    int semval;

    printf("Creating image stream ...\n");
    fflush(stdout);

    IDin = image_ID(IDinname);
    naxis = data.image[IDin].md[0].naxis;
    arraysize = (long*) malloc(sizeof(long)*3);
    if(naxis != 3)
    {
        printf("ERROR: input image %s should be 3D\n", IDinname);
        exit(0);
    }
    arraysize[0] = data.image[IDin].md[0].size[0];
    arraysize[1] = data.image[IDin].md[0].size[1];
    arraysize[2] = data.image[IDin].md[0].size[2];

    atype = data.image[IDin].md[0].atype;
    IDout = create_image_ID(IDoutname, 2, arraysize, atype, 1, 0);

    COREMOD_MEMORY_image_set_createsem(IDoutname, 4);

    switch ( atype ) {
    case CHAR:
        ptr0s = (char*) data.image[IDin].array.C;
        ptr1 = (char*) data.image[IDout].array.C;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(char);
        break;
    case INT:
        ptr0s = (char*) data.image[IDin].array.I;
        ptr1 = (char*) data.image[IDout].array.I;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(int);
        break;
    case FLOAT:
        ptr0s = (char*) data.image[IDin].array.F;
        ptr1 = (char*) data.image[IDout].array.F;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(float);
        break;
    case DOUBLE:
        ptr0s = (char*) data.image[IDin].array.D;
        ptr1 = (char*) data.image[IDout].array.D;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(double);
        break;
    case USHORT:
        ptr0s = (char*) data.image[IDin].array.U;
        ptr1 = (char*) data.image[IDout].array.U;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(unsigned short);
        break;
    case LONG:
        ptr0s = (char*) data.image[IDin].array.L;
        ptr1 = (char*) data.image[IDout].array.L;
        framesize = data.image[IDin].md[0].size[0]*data.image[IDin].md[0].size[1]*sizeof(long);
        break;
    }


    kk = 0;
    while(1)
    {
        ptr0 = ptr0s + kk*framesize;
        data.image[IDout].md[0].write = 1;
        memcpy((void *) ptr1, (void *) ptr0, framesize);
        
        sem_getvalue(data.image[IDout].semptr[0], &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(data.image[IDout].semptr[0]);

        data.image[IDout].md[0].cnt0++;
        data.image[IDout].md[0].write = 0;

        kk++;
        if(kk==data.image[IDin].md[0].size[2])
            kk = 0;

        usleep(usperiod);
    }

    return(IDout);
}



/** continuously transmits 2D image through TCP link
 * mode is not currently used
 */
 

 

long COREMOD_MEMORY_image_NETWORKtransmit(char *IDname, char *IPaddr, int port, int mode)
{
    long ID;
    struct sockaddr_in sock_server;
    int fds_client;
    int flag = 1;
    int result;
    long long cnt = -1;
    long long iter = 0;
    long framesize; // pixel data only
    long xsize, ysize;
    char *ptr0; // source
    char *ptr1; // source - offset by slice
    int rs;
    int sockOK;
    int RT_priority = 80; //any number from 0-99
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
    case CHAR:
        framesize = sizeof(char)*xsize*ysize;
        break;
    case INT:
        framesize = sizeof(int)*xsize*ysize;
        break;
    case FLOAT:
        framesize = sizeof(float)*xsize*ysize;
        break;
    case DOUBLE:
        framesize = sizeof(double)*xsize*ysize;
        break;
    case USHORT:
        framesize = sizeof(unsigned short)*xsize*ysize;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }

    printf("IMAGE FRAME SIZE = %ld\n", framesize);

    switch ( data.image[ID].md[0].atype ) {
    case CHAR:
        ptr0 = (char*) data.image[ID].array.C;
        break;
    case INT:
        ptr0 = (char*) data.image[ID].array.I;
        break;
    case FLOAT:
        ptr0 = (char*) data.image[ID].array.F;
        break;
    case DOUBLE:
        ptr0 = (char*) data.image[ID].array.D;
        break;
    case USHORT:
        ptr0 = (char*) data.image[ID].array.U;
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
    printf("sem = %d\n", data.image[ID].sem);
    fflush(stdout);
    
    while(sockOK==1)
    {
        if(data.image[ID].sem==0)
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
                printf("send() sent a different number of bytes (%d) than expected %ld\n", rs, framesize);
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

    return(ID);
}









long COREMOD_MEMORY_image_NETWORKreceive(int port, int mode)
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
    long xsize, ysize;
    char *ptr0; // source
    char fname[200];
    long NBslices;
    int socketOpen = 1; // 0 if socket is closed
    int semval;
    int semnb;

    imgmd = (IMAGE_METADATA*) malloc(sizeof(IMAGE_METADATA));

    TCP_BUFFER_METADATA *frame_md;
    long framesize1; // pixel data + metadata
    char *buff; // buffer
   

    int RT_priority = 80; //any number from 0-99
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


    ID = create_image_ID(imgmd[0].name, imgmd[0].naxis, imgmd[0].size, imgmd[0].atype, imgmd[0].shared, 0);
    COREMOD_MEMORY_image_set_createsem(imgmd[0].name, 4);
    printf("Created image stream %s - shared = %d\n", imgmd[0].name, imgmd[0].shared);
    list_image_ID();
    
    
    

/*        sprintf(fname, "sock%d_stream", port);
        ID = create_image_ID(fname, imgmd[0].naxis, imgmd[0].size, imgmd[0].atype, imgmd[0].shared, 0);
        COREMOD_MEMORY_image_set_createsem(fname, 4);
   */
    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];
    NBslices = 1;
    if(data.image[ID].md[0].naxis>2)
        if(data.image[ID].md[0].size[2]>1)
            NBslices = data.image[ID].md[0].size[2];

    switch ( data.image[ID].md[0].atype ) {
    case CHAR:
        framesize = sizeof(char)*xsize*ysize;
        break;
    case INT:
        framesize = sizeof(int)*xsize*ysize;
        break;
    case FLOAT:
        framesize = sizeof(float)*xsize*ysize;
        break;
    case DOUBLE:
        framesize = sizeof(double)*xsize*ysize;
        break;
    case USHORT:
        framesize = sizeof(unsigned short)*xsize*ysize;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }

    printf("image frame size = %ld\n", framesize);

    switch ( data.image[ID].md[0].atype ) {
    case CHAR:
        ptr0 = (char*) data.image[ID].array.C;
        break;
    case INT:
        ptr0 = (char*) data.image[ID].array.I;
        break;
    case FLOAT:
        ptr0 = (char*) data.image[ID].array.F;
        break;
    case DOUBLE:
        ptr0 = (char*) data.image[ID].array.D;
        break;
    case USHORT:
        ptr0 = (char*) data.image[ID].array.U;
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
            //   printf("Received %ld bytes (expected %ld)\n", recvsize, framesize);
        }
        else
            socketOpen = 0;
        
        if(socketOpen==1)
            {
                frame_md = (TCP_BUFFER_METADATA*) (buff + framesize);
            
                data.image[ID].md[0].cnt1 = frame_md[0].cnt1;
                
                    
               // printf("[%ld]", data.image[ID].md[0].cnt1); // TEST

/*                if(data.image[ID].md[0].cnt1==0)
                    {
                        printf("\n"); // TEST
                        fflush(stdout);
                    }*/
                if(NBslices>1)
                    memcpy(ptr0+framesize*frame_md[0].cnt1, buff, framesize);
                else
                     memcpy(ptr0, buff, framesize);
                data.image[ID].md[0].cnt0++;
                for(semnb=0;semnb<data.image[ID].sem ; semnb++)
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
long COREMOD_MEMORY_PixMapDecode_U(char *inputstream_name, long xsizeim, long ysizeim, char* NBpix_fname, char* IDmap_name, char *IDout_name, char *IDout_pixslice_fname)
{
    long IDout;
    long IDin;
    long IDmap;
    long slice, sliceii;
    long oldslice = 0;
    long NBslice;
    long *nbpixslice;
    long xsizein, ysizein;
    FILE *fp;
    long *sizearray;
    long IDout_pixslice;
    int loopOK;
    long ii;
    long cnt = 0;
    int RT_priority = 80; //any number from 0-99

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

    sizearray = (long*) malloc(sizeof(long)*3);

    IDin = image_ID(inputstream_name);
    IDmap = image_ID(IDmap_name);

    xsizein = data.image[IDin].md[0].size[0];
    ysizein = data.image[IDin].md[0].size[1];

    if(xsizein != data.image[IDmap].md[0].size[0])
    {
        printf("ERROR: xsize for %s (%ld) does not match xsize for %s (%ld)\n", inputstream_name, xsizein, IDmap_name, data.image[IDmap].md[0].size[0]);
        exit(0);
    }
    if(ysizein != data.image[IDmap].md[0].size[1])
    {
        printf("ERROR: xsize for %s (%ld) does not match xsize for %s (%ld)\n", inputstream_name, ysizein, IDmap_name, data.image[IDmap].md[0].size[1]);
        exit(0);
    }
    sizearray[0] = xsizeim;
    sizearray[1] = ysizeim;
    IDout = create_image_ID(IDout_name, 2, sizearray, data.image[IDin].md[0].atype, 1, 0);
    COREMOD_MEMORY_image_set_createsem(IDout_name, 7); // create 7 semaphores
    IDout_pixslice = create_image_ID("outpixsl", 2, sizearray, USHORT, 0, 0);

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
            data.image[IDout_pixslice].array.U[ data.image[IDmap].array.U[sliceii + ii] ] = (unsigned short) slice;
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
        if(data.image[IDin].sem==0)
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
                    data.image[IDout].array.U[data.image[IDmap].array.U[sliceii + ii] ] = data.image[IDin].array.U[sliceii + ii];
            }
            //     printf("[%ld] ", slice); //TEST

            if(slice==NBslice-1)   //if(slice<oldslice)
            {
                sem_getvalue(data.image[IDout].semptr[0], &semval);
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

    return(IDout);
}












/// ---------------------------------------- LOGGING FUNCTIONS --------------------------------



/// creates logshimconf shared memory and loads it
LOGSHIM_CONF* COREMOD_MEMORY_logshim_create_SHMconf(char *logshimname)
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
int COREMOD_MEMORY_logshim_printstatus(char *IDname)
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
int COREMOD_MEMORY_logshim_set_on(char *IDname, int setv)
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
int COREMOD_MEMORY_logshim_set_logexit(char *IDname, int setv)
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
long COREMOD_MEMORY_sharedMem_2Dim_log(char *IDname, long zsize, char *logdir, char *IDlogdata_name)
{
    long ID;
    long xsize, ysize;
    long ii;
    long i;
    long IDb, IDb0, IDb1;
    long index = 0;
    long cnt = -1;
    int buffer;
    int atype;
    long *imsizearray;
    char fname[200];
    char iname[200];
    time_t t;
    struct tm *uttime;
    //struct timespec *thetime = (struct timespec *)malloc(sizeof(struct timespec));
    struct timespec timenow;
    long kw;

    long IDlogdata;

    char *ptr0; // source
    char *ptr1; // destination
    long framesize; // in bytes

    FILE *fp;
    char fname_asciilog[200];

    pthread_t thread_savefits; 
    int tOK = 0;
    int iret_savefits;
    //	char tmessage[500];
    struct savethreadmsg *tmsg = malloc(sizeof(struct savethreadmsg));

    long fnb = 0;
    long NBfiles = -1; // run forever
    long long cntwait;
    long waitdelayus = 50;
    long long cntwaitlim = 20000; // 10 sec
    int wOK;
    int noframe;

    int is3Dcube = 0; // this is a rolling buffer
    int exitflag = 0; // toggles to 1 when loop must exit

    LOGSHIM_CONF* logshimconf;


    IDlogdata = image_ID(IDlogdata_name);
    if(IDlogdata!=-1)
    {
        if(data.image[IDlogdata].md[0].atype != FLOAT)
            IDlogdata = -1;
    }
    printf("log data name = %s\n", IDlogdata_name);


    logshimconf = COREMOD_MEMORY_logshim_create_SHMconf(IDname);


    logshimconf[0].on = 1;
    logshimconf[0].cnt = 0;
    logshimconf[0].filecnt = 0;
    logshimconf[0].logexit = 0;
    logshimconf[0].interval = 1;



    imsizearray = (long*) malloc(sizeof(long)*3);



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

    IDb0 = create_image_ID("logbuff0", 3, imsizearray, atype, 0, 1);
    IDb1 = create_image_ID("logbuff1", 3, imsizearray, atype, 0, 1);

    IDb = IDb0;

    switch ( atype ) {
    case CHAR:
        framesize = sizeof(char)*xsize*ysize;
        break;
    case INT:
        framesize = sizeof(int)*xsize*ysize;
        break;
    case FLOAT:
        framesize = sizeof(float)*xsize*ysize;
        break;
    case DOUBLE:
        framesize = sizeof(double)*xsize*ysize;
        break;
    case USHORT:
        framesize = sizeof(unsigned short)*xsize*ysize;
        break;
    case LONG:
        framesize = sizeof(long)*xsize*ysize;
        break;

    default:
        printf("ERROR: WRONG DATA TYPE\n");
        exit(0);
        break;
    }

    cnt = data.image[ID].md[0].cnt0;

    buffer = 0;
    index = 0;

    printf("logdata ID = %ld\n", IDlogdata);
    list_image_ID();

    exitflag = 0;

    while( (logshimconf[0].filecnt != NBfiles) && (logshimconf[0].logexit==0) )
    {
        cntwait = 0;
        noframe = 0;
        wOK = 1;
        // printf("Entering wait loop   index = %ld %d\n", index, noframe);

        while(((cnt==data.image[ID].md[0].cnt0)||(logshimconf[0].on == 0))&&(wOK==1))
        {
            usleep(waitdelayus);
            cntwait++;
            if(cntwait>cntwaitlim) // save current cube
            {
                strcpy(tmsg->iname, iname);
                strcpy(tmsg->fname, fname);
                tmsg->partial = 1; // partial cube
                tmsg->cubesize = index;
                wOK=0;
                if(index==0)
                    noframe = 1;
                else
                    noframe = 0;
            }
        }



        if(index==0)
        {
            /// measure time
            t = time(NULL);
            uttime = gmtime(&t);
            //clock_gettime(CLOCK_REALTIME, thetime);
            
			clock_gettime(CLOCK_REALTIME, &timenow);
/*
   struct timespec timenow;
   clock_gettime(CLOCK_REALTIME, &timenow);
	fprintf(fo, "time:  %ld.%09ld\n", timenow.tv_sec % 60, timenow.tv_nsec);
  */          
            sprintf(fname,"!%s/%s_%02d:%02d:%02ld.%09ld.fits", logdir, IDname, uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
   //        sprintf(fname, "!%s/%s_%02d:%02d:%02d.%09ld.fits", logdir, IDname, uttime->tm_hour, uttime->tm_min, (long)thetime->tv_sec, (long) (thetime->tv_nsec));

            sprintf(fname_asciilog,"%s/%s_%02d:%02d:%02ld.%09ld.txt", logdir, IDname, uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);
            
         //   printf("fname           = %s\n", fname);
          //  printf("fname_asciilog  = %s\n", fname_asciilog);
         //   fflush(stdout);
        }

        if(logshimconf[0].on == 1)
        {
            if(wOK==1) // normal step: a frame has arrived
            {
                /// measure time
                t = time(NULL);
                uttime = gmtime(&t);
                //clock_gettime(CLOCK_REALTIME, thetime);
				clock_gettime(CLOCK_REALTIME, &timenow);

                if(index==0)
                    fp = fopen(fname_asciilog, "w");


                switch ( atype ) {
                case CHAR:
                    ptr0 = (char*) data.image[ID].array.C;
                    ptr1 = (char*) data.image[IDb].array.C;
                    break;
                case INT:
                    ptr0 = (char*) data.image[ID].array.I;
                    ptr1 = (char*) data.image[IDb].array.I;
                    break;
                case FLOAT:
                    ptr0 = (char*) data.image[ID].array.F;
                    ptr1 = (char*) data.image[IDb].array.F;
                    break;
                case DOUBLE:
                    ptr0 = (char*) data.image[ID].array.D;
                    ptr1 = (char*) data.image[IDb].array.D;
                    break;
                case USHORT:
                    ptr0 = (char*) data.image[ID].array.U;
                    ptr1 = (char*) data.image[IDb].array.U;
                    break;
                case LONG:
                    ptr0 = (char*) data.image[ID].array.L;
                    ptr1 = (char*) data.image[IDb].array.L;
                    break;
                }

                //  ptr0 = (char*) data.image[ID].array.F;



                if(is3Dcube==1)
                    ptr0 += framesize*data.image[ID].md[0].cnt1;

                //    ptr1 = (char*) data.image[IDb].array.F;
                ptr1 += framesize*index;

                memcpy((void *) ptr1, (void *) ptr0, framesize);

                fprintf(fp, "%02d:%02d:%02ld.%09ld ", uttime->tm_hour, uttime->tm_min, timenow.tv_sec % 60, timenow.tv_nsec);

                if(IDlogdata!=-1)
                {

                    fprintf(fp, "%8ld", data.image[IDlogdata].md[0].cnt0);
                    for(i=0; i<data.image[IDlogdata].md[0].nelement; i++)
                        fprintf(fp, "  %f", data.image[IDlogdata].array.F[i]);
                }

                for(kw=0; kw<data.image[ID].md[0].NBkw; kw++)
                {
                    switch (data.image[ID].kw[kw].type) {
                    case 'L' :
                        fprintf(fp, " %ld", data.image[ID].kw[kw].value.numl);
                        break;
                    case 'D' :
                        fprintf(fp, " %f", data.image[ID].kw[kw].value.numf);
                        break;
                    }
                }
                fprintf(fp, "\n");

                index++;
            }
        }


        /// cases:
        /// index>zsize-1  buffer full
        /// wOK==0 && index>0
        if(  (index>zsize-1)  ||  ((wOK==0)&&(index>0)) )
        {
            /// save image
            sprintf(iname, "logbuff%d", buffer);
            //          printf("Saving %s -> %s\n", iname, fname);
            //         fflush(stdout);
            if(wOK==1) // image has arrived
            {
                strcpy(tmsg->iname, iname);
                strcpy(tmsg->fname, fname);
                tmsg->partial = 0; // full cube
            }

            fclose(fp);

            if(tOK == 1)
            {
                //           printf("WAITING FOR SAVE THREAD TO COMPLETE ...");
                //          fflush(stdout);
                pthread_join(thread_savefits, NULL); //(void**)&thread_savefits);
                //          printf("OK\n");
                //          fflush(stdout);
            }

            strcpy(tmsg->iname, iname);
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

            logshimconf[0].filecnt ++;
        }


        cnt = data.image[ID].md[0].cnt0;
    }

    free(imsizearray);

    return(0);
}












