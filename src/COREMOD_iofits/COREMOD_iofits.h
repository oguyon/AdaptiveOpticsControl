#ifndef _IOFITS_H
#define _IOFITS_H


int init_COREMOD_iofits();



int file_exists(const char *file_name);

int is_fits_file(const char *file_name);

int read_keyword(const char* file_name, const char* KEYWORD, char* content);

int read_keyword_alone(const char* file_name, const char* KEYWORD);

int data_type_code(int bitpix);

long load_fits(const char *file_name, const char *ID_name, int errcode); 

int save_db_fits(const char *ID_name, const char *file_name);

int save_fl_fits(const char *ID_name, const char *file_name);

int save_sh_fits(const char *ID_name, const char *file_name);

int save_fits(const char *ID_name, const char *file_name);
int save_fits_atomic(const char *ID_name, const char *file_name);

int saveall_fits(const char *savedirname);

int break_cube(const char *ID_name);

int images_to_cube(const char *img_name, long nbframes, const char *cube_name);

#endif
