#ifndef _CLICORE_H
#define _CLICORE_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <fftw3.h>
#include <gsl/gsl_rng.h>	// for random numbers
#include <signal.h>


#define PI 3.14159265358979323846264338328

/// Size of array CLICOREVARRAY
#define SZ_CLICOREVARRAY 1000

/// important directories and info
extern pid_t CLIPID;
extern char DocDir[200];		// location of documentation
extern char SrcDir[200];		// location of source
extern char BuildFile[200];	// file name for source
extern char BuildDate[200];
extern char BuildTime[200];

extern int C_ERRNO;			// C errno (from errno.h)

/* #define DEBUG */
#define CFITSEXIT  printf("Program abnormally terminated, File \"%s\", line %d\n", __FILE__, __LINE__);exit(0)

#ifdef DEBUG
#define nmalloc(f,type,n) f = (type*) malloc(sizeof(type)*n);if(f==NULL){printf("ERROR: pointer \"" #f "\" allocation failed\n");exit(0);}else{printf("\nMALLOC: \""#f "\" allocated\n");}
#define nfree(f) free(f);printf("\nMALLOC: \""#f"\" freed\n");
#else
#define nmalloc(f,type,n) f = (type*) malloc(sizeof(type)*n);if(f==NULL){printf("ERROR: pointer \"" #f "\" allocation failed\n");exit(0);}
#define nfree(f) free(f);
#endif

#define TEST_ALLOC(f) if(f==NULL){printf("ERROR: pointer \"" #f "\" allocation failed\n");exit(0);}


#define SHAREDMEMDIR "/tmp"


#define SEMAPHORE_MAX       1000
#define SEMAPHORE_MAXVAL    10 // maximum value for a semaphore
#define NB_ARG_MAX                 20


//Need to install process with setuid.  Then, so you aren't running privileged all the time do this:
extern uid_t euid_real;
extern uid_t euid_called;
extern uid_t suid;









/*^-----------------------------------------------------------------------------
| commands available through the CLI
+-----------------------------------------------------------------------------*/



typedef struct {
    char key[100];        // command keyword
    char module[50];      // module name
    int (* fp) ();         // command function pointer
    char info   [1000];     // short description/help
    char syntax [1000];   // command syntax
    char example[1000];  // command example
    char Ccall[1000];
} CMD;

typedef struct {
    char name[50];   // module name
    char info[1000]; // short description
} MODULE;




/* ---------------------------------------------------------- */
/*                                                            */
/*                                                            */
/*       COMMAND LINE ARGs / TOKENS                           */
/*                                                            */
/*                                                            */
/* ---------------------------------------------------------- */


// The command line is parsed and

// cmdargtoken type
// 0 : unsolved
// 1 : floating point (double precision)
// 2 : long
// 3 : string
// 4 : existing image
// 5 : command
typedef struct
{
    int type;
    union
    {
        double numf;
        long numl;
        char string[200];
    } val;
} CMDARGTOKEN;



int CLI_checkarg(int argnum, int argtype);
int CLI_checkarg_noerrmsg(int argnum, int argtype);







typedef struct
{
    float re;
    float im;
} complex_float;

typedef struct
{
    double re;
    double im;
} complex_double;



#define CHAR 1
#define INT 2
#define FLOAT 3
#define DOUBLE 4
#define COMPLEX_FLOAT 5
#define COMPLEX_DOUBLE 6
#define USHORT 7
#define LONG 8

extern int TYPESIZE[9];


//typedef float PRECISION;
//typedef complex_float CPRECISION;
//#define arrayD array.F
//#define arrayCD array.CF
#define Dtype 3
#define CDtype 5



typedef struct
{
    char name[16];
    char type; // N: unused, L: long, D: double, S: 16-char string
    union {
        long numl;
        double numf;
        char valstr[16];
    } value;
    char comment[80];
} IMAGE_KEYWORD;



typedef struct
{
    char name[80];                // image name

    long naxis;                   // number of axis
    long size[3];                 // image size
    long nelement;              // number of elements in image
    int atype;                  // data type code

    double creation_time;       // creation time (since program start)
    double last_access;         // last time the image was accessed  (since program start)
    struct timespec wtime;

    int shared;                 // 1 if in shared memory

    int write;               // 1 if image is being written
    int status;              // 1 to log image (default); 0 : do not log: 2 : stop log (then goes back to 2)
    long cnt0;               // counter (incremented if image is updated)
    long cnt1;               // in 3D rolling buffer image, this is the last slice written

    long NBkw;                  // number of keywords

} IMAGE_METADATA;








typedef struct          /* structure used to store data arrays */
{
    int used;
    int shmfd; // if shared memory, file descriptor
    size_t memsize; // total size in memory if shared

    IMAGE_METADATA *md;

    union
    {
        char *C;
        int *I;
        long *L;
        float *F;
        double *D;
        complex_float *CF;
        complex_double *CD;
        unsigned short int *U;
    } array;                      // pointer to data array

    IMAGE_KEYWORD *kw;

    int sem; // number of semaphores in use     
    sem_t **semptr; // semaphore array

    sem_t *semlog; // semaphore for logging
    
    
    char name[80]; // local name (can be different from name in shared memory)
    //long cnt00; // custom counter

    //	int logstatus; // 0: do not log this image (pause), 1: log me (default), 2: log program should nicely exit

} IMAGE;





typedef struct
{
    int used;
    char name[80];
    int type; /** 0: double, 1: long, 2: string */
    union
    {
        double f;
        long l;
        char s[80];
    } value;
    char comment[200];
} VARIABLE;





// THIS IS WHERE EVERYTHING THAT NEEDS TO BE WIDELY ACCESSIBLE GETS STORED
typedef struct
{
    struct sigaction sigact; 
    // signals toggle flags
    int signal_USR1;
    int signal_USR2;
    int signal_TERM;
    int signal_INT;
    int signal_SEGV;
    int signal_ABRT;
    int signal_BUS;
    int signal_HUP;
    int signal_PIPE;
    
    int Debug;
    int quiet;
    int overwrite;		// automatically overwrite FITS files
    double INVRANDMAX;
    gsl_rng *rndgen;		// random number generator
    int precision;		// default precision: 0 for float, 1 for double

    // logging
    int CLIlogON;
    char CLIlogname[200];

    // Command Line Interface (CLI) INPUT
    int fifoON;
    char processname[100];
    char fifoname[100];
    long NBcmd;
    long NB_MAX_COMMAND;
    CMD *cmd;
    int parseerror; // 1 if error, 0 otherwise
    long cmdNBarg;  // number of arguments in last command line
    CMDARGTOKEN cmdargtoken[NB_ARG_MAX];
    long cmdindex; // when command is found in command line, holds index of command
    long calctmp_imindex; // used to create temporary images
    int CMDexecuted; // 0 if command has not been executed, 1 otherwise
    long NBmodule;
    long NB_MAX_MODULE;
    MODULE *module;


    // shared memory default
    int SHARED_DFT;

    // Number of keyword per iamge default
    int NBKEWORD_DFT;

    // images, variables
    long NB_MAX_IMAGE;
    IMAGE *image;

    long NB_MAX_VARIABLE;
    VARIABLE *variable;
    /*
      long NB_MAX_VARIABLELONG;
      VARIABLELONG *variablelong;

      long NB_MAX_VARIABLESTRING;
      VARIABLESTRING *variablestring;
      */

    float FLOATARRAY[1000];	// array to store temporary variables
    double DOUBLEARRAY[1000];    // for convenience
    char SAVEDIR[500];

    // status counter (used for profiling)
    int status0;
    int status1;
} DATA;






#define MAX_NB_FRAMES 500
#define MAX_NB_FRAMENAME_CHAR 500
#define MAX_NB_EXCLUSIONS 40


void sig_handler(int signo);




#endif
