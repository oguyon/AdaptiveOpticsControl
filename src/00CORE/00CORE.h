/**
 * @file    00CORE.h
 * @brief   Function prototypes for module 00CORE
 * 
 *  
 * @author  O. Guyon
 * @date    Aug 4 2017
 *
 * 
 * @bug No known bugs.
 * 
 */

#ifndef _00CORE_H
#define _00CORE_H



/** @name INITIALIZATION
 * Module initialization functions
 */
int init_00CORE();



/* =========================================================*/
/** @name 00CORE - 1. PRINT, ERROR REPORTING, EVENT LOGGING */
/* =========================================================*/

/** @brief Print string in red */
int printRED(char *string);

/** @brief Print warning and continue */
int printWARNING(const char *file, const char *func, int line, char *warnmessage);

/** @brief Print error (in red) and continue */
int printERROR(const char *file, const char *func, int line, char *errmessage);

/** @brief Log function call to file */
static void CORE_logFunctionCall(const int funclevel, const int loglevel, const int logfuncMODE, const char *FunctionName, const long line, char *comments);



/* =========================================================*/
/** @name 00CORE - 2. CONFIGURATION AND INFORMATION */
/* =========================================================*/

int set_precision(int vp);

int CLIWritePid();


/* =========================================================*/
/** @name 00CORE - 3. TIME UTILITIES */
/* =========================================================*/

struct timespec timespec_diff(struct timespec start, struct timespec end);

double timespec_diff_double(struct timespec start, struct timespec end);

#endif
