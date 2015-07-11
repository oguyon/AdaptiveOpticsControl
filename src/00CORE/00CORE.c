#include <fitsio.h> 
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "CLIcore.h"
#include "COREMOD_memory/COREMOD_memory.h"

extern DATA data;


/* Color codes for printf
	30	Black
	31	Red
	32	Green
	33	Yellow
	34	Blue
	35	Magenta
	36	Cyan
	37	White
*/




int init_00CORE()
{
    return 0;
}



int printRED(char *string)
{
  printf("%c[%d;%dm %s %c[%d;m\n", (char) 27, 1, 31, string, (char) 27, 0);

  return(0);
}


int printWARNING(const char *file, const char *func, int line, char *warnmessage)
{
  char buff[256];

  fprintf(stderr,"%c[%d;%dm WARNING [ FILE: %s   FUNCTION: %s  LINE: %d ]  %c[%d;m\n", (char) 27, 1, 35, file, func, line, (char) 27, 0);
  if(C_ERRNO != 0)
    {
      if( strerror_r( errno, buff, 256 ) == 0 ) {
	fprintf(stderr,"C Error: %s\n", buff );
      }
      else
	fprintf(stderr,"Unknown C Error\n");
    }
  else
    fprintf(stderr,"No C error (errno = 0)\n");

  fprintf(stderr,"%c[%d;%dm %s  %c[%d;m\n", (char) 27, 1, 35, warnmessage, (char) 27, 0);
  C_ERRNO = 0;

  return(0);
}


int printERROR(const char *file, const char *func, int line, char *errmessage)
{
  char buff[256];

  fprintf(stderr,"%c[%d;%dm ERROR [ FILE: %s   FUNCTION: %s   LINE: %d ]  %c[%d;m\n", (char) 27, 1, 31, file, func, line, (char) 27, 0);
  if(C_ERRNO != 0)
    {
      if( strerror_r( errno, buff, 256 ) == 0 ) {
	fprintf(stderr,"C Error: %s\n", buff );
      }
      else
	fprintf(stderr,"Unknown C Error\n");
    }
  else
    fprintf(stderr,"No C error (errno = 0)\n");
   
  fprintf(stderr,"%c[%d;%dm %s  %c[%d;m\n", (char) 27, 1, 31, errmessage, (char) 27, 0);

  C_ERRNO = 0;
  
  return(0);
}

int set_precision(int vp)
{
  if(vp == 0)
    {
      data.precision = 0;
      printf("Default precision : single (float)\n");
    }
  else if(vp == 1)
    {
      data.precision = 1;
      printf("Default precision : double\n");
    }
  else
    {
      printf("Requested precision value (%d) not a valid choice\n",vp);
    }

  return(0);
}

int CLIinfoPrint()
{
  printf("Process ID   : %d\n",CLIPID);
  printf("%s BUILT     : %s %s\n", BuildFile, BuildDate, BuildTime);
  printf("Memory usage      : %ld Mb  (%ld images)\n",
                  ((long) (compute_image_memory(data)/1024/1024)),
                  compute_nb_image(data)  );
  printf("Default precision : %d (0: single, 1: double)\n",data.precision);

 return(0);
}

int CLIWritePid()
{
  int pid;
  char command[200];
  int n;
	int r;

  pid = getpid();
  n = snprintf(command,200,"touch CLI_%d",pid);
  if(n >= 200) 
    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");


  r = system(command);
  
  return(0);
}


struct timespec timespec_diff(struct timespec start, struct timespec end)
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

double timespec_diff_double(struct timespec start, struct timespec end)
{
  struct timespec temp;
  double val;

  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }

  val = temp.tv_sec;
  val += 0.000000001*temp.tv_nsec;

  return val;
}


int file_exist (char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}


