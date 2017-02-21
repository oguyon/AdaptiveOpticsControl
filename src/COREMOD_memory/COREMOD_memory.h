#ifndef _COREMODMEMORY_H
#define _COREMODMEMORY_H

/* the number of images in the data structure is kept NB_IMAGES_BUFFER above the number of used images prior to the execution of any function. It means that no function should create more than 100 images. */
#define NB_IMAGES_BUFFER 500
/* when the number of free images in the data structure is below NB_IMAGES_BUFFER, it is increased by  NB_IMAGES_BUFFER */
#define NB_IMAGES_BUFFER_REALLOC 600

/* the number of variables in the data structure is kept NB_VARIABLES_BUFFER above the number of used variables prior to the execution of any function. It means that no function should create more than 100 variables. */
#define NB_VARIABLES_BUFFER 100
/* when the number of free variables in the data structure is below NB_VARIABLES_BUFFER, it is increased by  NB_VARIABLES_BUFFER */
#define NB_VARIABLES_BUFFER_REALLOC 150


/*void print_sys_mem_info();*/




typedef struct
{
    int on; /// 1 if logging, 0 otherwise
    long long cnt;
    long long filecnt;
    long interval; /// log every x frames (default = 1)
    int logexit; /// toggle to 1 when exiting
    char fname[200];
} LOGSHIM_CONF;



typedef struct
{
    long cnt0;
    long cnt1;
} TCP_BUFFER_METADATA;




int_fast8_t init_COREMOD_memory();



int_fast8_t memory_monitor(const char *termttyname);


long compute_nb_image();

long compute_nb_variable();

long long compute_image_memory();

long compute_variable_memory();

long image_ID(const char *name);

long image_ID_noaccessupdate(const char *name);

long variable_ID(const char *name);

long next_avail_image_ID();

long next_avail_variable_ID();

int_fast8_t delete_image_ID(const char* imname);

int_fast8_t delete_image_ID_prefix(const char *prefix);

int_fast8_t delete_variable_ID(const char* varname);

long create_variable_long_ID(const char *name, long value);

long create_variable_string_ID(const char *name, const char *value);

long create_image_ID(const char *name, long naxis, long *size, int atype, int shared, int nbkw);


 

long image_write_keyword_L(const char *IDname, const char *kname, long value, const char *comment);
long image_write_keyword_D(const char *IDname, const char *kname, double value, const char *comment);
long image_write_keyword_S(const char *IDname, const char *kname, const char *value, const char *comment);

long image_list_keywords(const char *IDname);

long image_read_keyword_D(const char *IDname, const char *kname, double *val);
long image_read_keyword_L(const char *IDname, const char *kname, long *val);

long read_sharedmem_image_size(const char *name, const char *fname);
long read_sharedmem_image(const char *name);

long create_1Dimage_ID(const char *ID_name, long xsize);

long create_1DCimage_ID(const char *ID_name, long xsize);

long create_2Dimage_ID(const char *ID_name, long xsize, long ysize);

long create_2Dimage_ID_double(const char *ID_name, long xsize, long ysize);

long create_2DCimage_ID(const char *ID_name, long xsize, long ysize);
long create_2DCimage_ID_double(const char *ID_name, long xsize, long ysize);

long create_3Dimage_ID(const char *ID_name, long xsize, long ysize, long zsize);

long create_3Dimage_ID_double(const char *ID_name, long xsize, long ysize, long zsize);

long create_3DCimage_ID(const char *ID_name, long xsize, long ysize, long zsize);

long copy_image_ID(const char *name, const char *newname, int shared);

long create_variable_ID(const char *name, double value);

int_fast8_t list_image_ID_ncurses();

int_fast8_t list_image_ID_ofp(FILE *fo);

int_fast8_t list_image_ID_ofp_simple(FILE *fo);

int_fast8_t list_image_ID();

int_fast8_t list_image_ID_file(const char *fname);

int_fast8_t list_variable_ID();

int_fast8_t list_variable_ID_file(const char *fname);

long chname_image_ID(const char *ID_name, const char *new_name);

int_fast8_t mk_complex_from_reim(const char *re_name, const char *im_name, const char *out_name, int sharedmem);

int_fast8_t mk_complex_from_amph(const char *am_name, const char *ph_name, const char *out_name, int sharedmem);

int_fast8_t mk_reim_from_complex(const char *in_name, const char *re_name, const char *im_name, int sharedmem);

int_fast8_t mk_amph_from_complex(const char *in_name, const char *am_name, const char *ph_name, int sharedmem);

int_fast8_t mk_reim_from_amph(const char *am_name, const char *ph_name, const char *re_out_name, const char *im_out_name, int sharedmem);

int_fast8_t mk_amph_from_reim(const char *re_name, const char *im_name, const char *am_out_name, const char *ph_out_name, int sharedmem);

int_fast8_t clearall();

int_fast8_t check_2Dsize(const char *ID_name, long xsize, long ysize);

int_fast8_t check_3Dsize(const char *ID_name, long xsize, long ysize, long zsize);

int_fast8_t rotate_cube(const char *ID_name, const char *ID_out_name, int orientation);

long COREMOD_MEMORY_cp2shm(const char *IDname, const char *IDshmname);

long COREMOD_MEMORY_check_2Dsize(const char *IDname, long xsize, long ysize);
long COREMOD_MEMORY_check_3Dsize(const char *IDname, long xsize, long ysize, long zsize);


long COREMOD_MEMORY_image_set_status(const char *IDname, int status);
long COREMOD_MEMORY_image_set_cnt0(const char *IDname, int cnt0);
long COREMOD_MEMORY_image_set_cnt1(const char *IDname, int cnt1);

long COREMOD_MEMORY_image_set_createsem(const char *IDname, long NBsem);
long COREMOD_MEMORY_image_set_sempost(const char *IDname, long index);
long COREMOD_MEMORY_image_set_sempost_byID(long ID, long index);

long COREMOD_MEMORY_image_set_sempost_loop(const char *IDname, long index, long dtus);
long COREMOD_MEMORY_image_set_semwait(const char *IDname, long index);
void *waitforsemID(void *ID);
long COREMOD_MEMORY_image_set_semwait_OR_IDarray(long *IDarray, long NB_ID);
long COREMOD_MEMORY_image_set_semflush_IDarray(long *IDarray, long NB_ID);
long COREMOD_MEMORY_image_set_semflush(const char *IDname, long index);






/* ========================================================================================================================================= */
/*                                                   SIMPLE OPERATIONS ON STREAMS                                                            */
/* ========================================================================================================================================= */

// difference between two streams
long COREMOD_MEMORY_streamDiff(const char *IDstream0_name, const char *IDstream1_name, const char *IDstreammask_name, const char *IDstreamout_name, long semtrig);

// difference between two halves of stream image
long COREMOD_MEMORY_stream_halfimDiff(const char *IDstream_name, const char *IDstreamout_name, long semtrig);

// takes a 3Dimage (circular buffer) and writes slices to a 2D image with time interval specified in us
long COREMOD_MEMORY_image_streamupdateloop(const char *IDinname, const char *IDoutname, long usperiod);

// takes a 3Dimage (circular buffer) and writes slices to a 2D image synchronized with an image semaphore
long COREMOD_MEMORY_image_streamupdateloop_semtrig(const char *IDinname, const char *IDoutname, long period, long offsetus, const char *IDsync_name, int semtrig, int timingmode);

long COREMOD_MEMORY_streamDelay(const char *IDin_name, const char *IDout_name, long delayus, long dtus);

long COREMOD_MEMORY_SaveAll_snapshot(const char *dirname);
long COREMOD_MEMORY_SaveAll_sequ(const char *dirname, const char *IDtrig_name, long semtrig, long NBframes);

long COREMOD_MEMORY_image_NETWORKtransmit(const char *IDname, const char *IPaddr, int port, int mode, int RT_priority);
long COREMOD_MEMORY_image_NETWORKreceive(int port, int mode, int RT_priority);

long COREMOD_MEMORY_PixMapDecode_U(const char *inputstream_name, long xsizeim, long ysizeim, const char* NBpix_fname, const char* IDmap_name, const char *IDout_name, const char *IDout_pixslice_fname);


/* ========================================================================================================================================= */
/*                                                                DATA LOGGING                                                               */
/* ========================================================================================================================================= */


int_fast8_t COREMOD_MEMORY_logshim_printstatus(const char *IDname);
int_fast8_t COREMOD_MEMORY_logshim_set_on(const char *IDname, int setv);
int_fast8_t COREMOD_MEMORY_logshim_set_logexit(const char *IDname, int setv);
long COREMOD_MEMORY_sharedMem_2Dim_log(const char *IDname, long zsize, const char *logdir, const char *IDlogdata_name);

#endif
