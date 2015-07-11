#ifndef _IOFITS_H
#define _IOFITS_H

int file_exists(char *file_name);

int is_fits_file(char *file_name);

int read_keyword(char* file_name, char* KEYWORD, char* content);

int read_keyword_alone(char* file_name, char* KEYWORD);

int data_type_code(int bitpix);

long load_fits(char *file_name, char *ID_name, int errcode); 

int save_db_fits(char *ID_name, char *file_name);

int save_fl_fits(char *ID_name, char *file_name);

int save_sh_fits(char *ID_name, char *file_name);

int saveall_fl_fits();

int break_cube(char *ID_name);

int images_to_cube(char *img_name, long nbframes, char *cube_name);

#endif
