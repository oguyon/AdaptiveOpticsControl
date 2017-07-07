#define _GNU_SOURCE


#ifndef _CLICORE_H
#define _CLICORE_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <fftw3.h>
#include <gsl/gsl_rng.h>	// for random numbers
#include <signal.h>


#include "ImageStruct.h"


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


#define NB_ARG_MAX                 20


//Need to install process with setuid.  Then, so you aren't running privileged all the time do this:
extern uid_t euid_real;
extern uid_t euid_called;
extern uid_t suid;









/*^-----------------------------------------------------------------------------
| commands available through the CLI
+-----------------------------------------------------------------------------*/



typedef struct {
    char key[100];            // command keyword
    char module[50];          // module name
    int_fast8_t (* fp) ();    // command function pointer
    char info   [1000];       // short description/help
    char syntax [1000];       // command syntax
    char example[1000];       // command example
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






extern int TYPESIZE[9];




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
    uint_fast16_t NBcmd;
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

uint_fast16_t RegisterCLIcommand(char *CLIkey, char *CLImodule, int_fast8_t (*CLIfptr)(), char *CLIinfo, char *CLIsyntax, char *CLIexample, char *CLICcall);



#endif
