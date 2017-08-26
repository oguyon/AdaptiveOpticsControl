/**
 * @file    AOloopControl.h
 * @brief   Function prototypes for Adaptive Optics Control loop engine I/O tools
 * 
 * AO engine uses stream data structure
 * 
 * @author  O. Guyon
 * @date    22 Aug 2017
 *
 * @bug No known bugs. 
 * 
 */

#ifndef _AOLOOPCONTROL_IOTOOLS_H
#define _AOLOOPCONTROL_IOTOOLS_H



/** @brief Initialize command line interface. */
int_fast8_t init_AOloopControl_IOtools();



/* =============================================================================================== */
/* =============================================================================================== */
/** @name AOloopControl - 2. LOW LEVEL UTILITIES & TOOLS    
 *  Useful tools */
/* =============================================================================================== */
/* =============================================================================================== */


/* =============================================================================================== */
/** @name AOloopControl - 2.1. LOW LEVEL UTILITIES & TOOLS - LOAD DATA STREAMS                     */
/* =============================================================================================== */

/** @brief Load 2D image in shared memory */
long AOloopControl_2Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize);

/** @brief Load 3D image in shared memory */
long AOloopControl_3Dloadcreate_shmim(const char *name, const char *fname, long xsize, long ysize, long zsize);


/* =============================================================================================== */
/** @name AOloopControl - 2.2. LOW LEVEL UTILITIES & TOOLS - DATA STREAMS PROCESSING               */
/* =============================================================================================== */

/** @brief Average data stream */
int_fast8_t AOloopControl_AveStream(const char *IDname, double alpha, const char *IDname_out_ave, const char *IDname_out_AC, const char *IDname_out_RMS);

/** @brief Induces temporal offset between input and output streams */
long AOloopControl_frameDelay(const char *IDin_name, const char *IDkern_name, const char *IDout_name, int insem);

/** @brief Re-arrange a 3D cube into an array of images into a single 2D frame */
long AOloopControl_stream3Dto2D(const char *in_name, const char *out_name, int NBcols, int insem);



#endif
