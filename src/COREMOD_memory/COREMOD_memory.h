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




int init_COREMOD_memory();



int memory_monitor(char *termttyname);


long compute_nb_image();

long compute_nb_variable();

long long compute_image_memory();

long compute_variable_memory();

long image_ID(char *name);

long image_ID_noaccessupdate(char *name);

long variable_ID(char *name);

long next_avail_image_ID();

long next_avail_variable_ID();

int delete_image_ID(char* imname);

int delete_image_ID_prefix(char *prefix);

int delete_variable_ID(char* varname);

long create_variable_long_ID(char *name, long value);

long create_variable_string_ID(char *name, char *value);

long create_image_ID(char *name, long naxis, long *size, int atype, int shared, int nbkw);


 

long image_write_keyword_L(char *IDname, char *kname, long value, char *comment);
long image_write_keyword_D(char *IDname, char *kname, double value, char *comment);
long image_write_keyword_S(char *IDname, char *kname, char *value, char *comment);

long image_list_keywords(char *IDname);

long image_read_keyword_D(char *IDname, char *kname, double *val);
long image_read_keyword_L(char *IDname, char *kname, long *val);

long read_sharedmem_image_size(char *name, char *fname);
long read_sharedmem_image(char *name);

long create_1Dimage_ID(char *ID_name, long xsize);

long create_1DCimage_ID(char *ID_name, long xsize);

long create_2Dimage_ID(char *ID_name, long xsize, long ysize);

long create_2Dimage_ID_double(char *ID_name, long xsize, long ysize);

long create_2DCimage_ID(char *ID_name, long xsize, long ysize);
long create_2DCimage_ID_double(char *ID_name, long xsize, long ysize);

long create_3Dimage_ID(char *ID_name, long xsize, long ysize, long zsize);

long create_3Dimage_ID_double(char *ID_name, long xsize, long ysize, long zsize);

long create_3DCimage_ID(char *ID_name, long xsize, long ysize, long zsize);

long copy_image_ID(char *name, char *newname, int shared);

long create_variable_ID(char *name, double value);

int list_image_ID_ncurses();

int list_image_ID_ofp(FILE *fo);

int list_image_ID_ofp_simple(FILE *fo);

int list_image_ID();

int list_image_ID_file(char *fname);

int list_variable_ID();

int list_variable_ID_file(char *fname);

long chname_image_ID(char *ID_name, char *new_name);

int mk_complex_from_reim(char *re_name, char *im_name, char *out_name, int sharedmem);

int mk_complex_from_amph(char *am_name, char *ph_name, char *out_name, int sharedmem);

int mk_reim_from_complex(char *in_name, char *re_name, char *im_name, int sharedmem);

int mk_amph_from_complex(char *in_name, char *am_name, char *ph_name, int sharedmem);

int mk_reim_from_amph(char *am_name, char *ph_name, char *re_out_name, char *im_out_name, int sharedmem);

int mk_amph_from_reim(char *re_name, char *im_name, char *am_out_name, char *ph_out_name, int sharedmem);

int clearall();

int check_2Dsize(char *ID_name, long xsize, long ysize);

int check_3Dsize(char *ID_name, long xsize, long ysize, long zsize);

int rotate_cube(char *ID_name, char *ID_out_name, int orientation);

long COREMOD_MEMORY_cp2shm(char *IDname, char *IDshmname);

long COREMOD_MEMORY_check_2Dsize(char *IDname, long xsize, long ysize);
long COREMOD_MEMORY_check_3Dsize(char *IDname, long xsize, long ysize, long zsize);


long COREMOD_MEMORY_image_set_status(char *IDname, int status);
long COREMOD_MEMORY_image_set_cnt0(char *IDname, int cnt0);
long COREMOD_MEMORY_image_set_cnt1(char *IDname, int cnt1);

long COREMOD_MEMORY_image_set_createsem(char *IDname, long NBsem);
long COREMOD_MEMORY_image_set_sempost(char *IDname, long index);
long COREMOD_MEMORY_image_set_sempost_byID(long ID, long index);

long COREMOD_MEMORY_image_set_sempost_loop(char *IDname, long index, long dtus);
long COREMOD_MEMORY_image_set_semwait(char *IDname, long index);
void *waitforsemID(void *ID);
long COREMOD_MEMORY_image_set_semwait_OR_IDarray(long *IDarray, long NB_ID);
long COREMOD_MEMORY_image_set_semflush_IDarray(long *IDarray, long NB_ID);
long COREMOD_MEMORY_image_set_semflush(char *IDname, long index);

long COREMOD_MEMORY_image_streamupdateloop(char *IDinname, char *IDoutname, long usperiod);

long COREMOD_MEMORY_image_NETWORKtransmit(char *IDname, char *IPaddr, int port, int mode);
long COREMOD_MEMORY_image_NETWORKreceive(int port, int mode);

long COREMOD_MEMORY_PixMapDecode_U(char *inputstream_name, long xsizeim, long ysizeim, char* NBpix_fname, char* IDmap_name, char *IDout_name, char *IDout_pixslice_fname);

int COREMOD_MEMORY_logshim_printstatus(char *IDname);
int COREMOD_MEMORY_logshim_set_on(char *IDname, int setv);
int COREMOD_MEMORY_logshim_set_logexit(char *IDname, int setv);
long COREMOD_MEMORY_sharedMem_2Dim_log(char *IDname, long zsize, char *logdir, char *IDlogdata_name);

#endif
