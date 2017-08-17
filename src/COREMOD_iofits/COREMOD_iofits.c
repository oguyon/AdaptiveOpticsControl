/**
 * @file    COREMOD_iofits.c
 * @brief   I/O for FITS files 
 * 
 * Uses CFITSIO library heavily
 * 
 * @author  O. Guyon
 * @date    7 Jul 2017
 *
 * @bug No known bugs. 
 * 
 */

#include <stdint.h>
#include <fitsio.h> /* required by every program that uses CFITSIO  */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_memory/COREMOD_memory.h"


#define SBUFFERSIZE 1000

extern DATA data;

static char errormessage_iofits[SBUFFERSIZE];




// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string, not existing image
// 4: existing image
// 5: string 
//

int_fast8_t load_fits_cli()
{
  load_fits( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string, 1);

  return 0;
}


int_fast8_t save_fl_fits_cli()
{
  char fname[200];
  
  switch(data.cmdNBarg){
  case 3:
    save_fl_fits( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string);
    break;
  case 2:
    sprintf(fname, "%s.fits", data.cmdargtoken[1].val.string);
    save_fl_fits( data.cmdargtoken[1].val.string, fname);
    break;
  }
  return 0;
}



int_fast8_t save_db_fits_cli()
{
  char fname[200];
  
  switch(data.cmdNBarg){
  case 3:
    save_db_fits( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string);
    break;
  case 2:
    sprintf(fname, "%s.fits", data.cmdargtoken[1].val.string);
    save_db_fits( data.cmdargtoken[1].val.string, fname);
    break;
  }
  
  return 0;
}

int_fast8_t save_sh_fits_cli()
{
  char fname[200];
  
  switch(data.cmdNBarg){
  case 3:
    save_sh_fits( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string);
    break;
  case 2:
    sprintf(fname, "%s.fits", data.cmdargtoken[1].val.string);
    save_sh_fits( data.cmdargtoken[1].val.string, fname);
    break;
  }
  
  return 0;
}


int_fast8_t save_fits_cli()
{
  char fname[200];
  
  switch(data.cmdNBarg){
  case 3:
    save_fits( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string);
    break;
  case 2:
    sprintf(fname, "%s.fits", data.cmdargtoken[1].val.string);
    save_fits( data.cmdargtoken[1].val.string, fname);
    break;
  }
  
  return 0;
}




int_fast8_t break_cube_cli()
{
  break_cube( data.cmdargtoken[1].val.string);

  return 0;
}


int_fast8_t images_to_cube_cli()
{
  /*  if(data.cmdargtoken[1].type != 4)
    {
      printf("Image %s does not exist\n", data.cmdargtoken[1].val.string);
      return -1;
      }*/

  if(data.cmdargtoken[2].type != 2)
    {
      printf("second argument has to be integer\n");
      return -1;
    }
  
  images_to_cube(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.string);

  return 0;
 }











int init_COREMOD_iofits()
{

  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "FITS format input/output");
  data.NBmodule++;
  
  
/* =============================================================================================== */
/*                                                                                                 */
/* 1. LOAD / SAVE                                                                                  */
/*                                                                                                 */
/* =============================================================================================== */
  
  RegisterCLIcommand("loadfits", __FILE__, load_fits_cli, "load FITS format file", "input output", "loadfits im.fits im", "long load_fits()");
 
  RegisterCLIcommand("saveflfits", __FILE__, save_fl_fits_cli, "save FITS format file, float", "input output", "saveflfits im im.fits", "int save_fl_fits(char *ID_name, char *file_name)");
 
  RegisterCLIcommand("savedbfits", __FILE__, save_db_fits_cli, "save FITS format file, double", "input output", "savedbfits im im.fits", "int save_db_fits(char *ID_name, char *file_name)");

  RegisterCLIcommand("saveshfits", __FILE__, save_sh_fits_cli, "save FITS format file, short", "input output", "saveshfits im im.fits", "int save_sh_fits(char *ID_name, char *file_name)");

  RegisterCLIcommand("savefits", __FILE__, save_fits_cli, "save FITS format file", "input output", "savefits im im.fits", "int save_fits(char *ID_name, char *file_name)");
 
/* =============================================================================================== */
/*                                                                                                 */
/* 2. CUBES                                                                                        */
/*                                                                                                 */
/* =============================================================================================== */
  
  RegisterCLIcommand("breakcube", __FILE__, break_cube_cli, "break cube into individual images (slices)", "<input image>", "breakcube imc", "int break_cube(char *ID_name)");
 
  RegisterCLIcommand("imgs2cube", __FILE__, images_to_cube_cli, "combine individual images into cube, image name is prefix followed by 5 digits", "<input image format> <max index> <output cube>", "imgs2cube im_ 100 imc", "int images_to_cube(char *img_name, long nbframes, char *cube_name)");


  // add atexit functions here

  return 0;
}





















static int FITSIO_status = 0;
 
// set print to 0 if error message should not be printed to stderr
// set print to 1 if error message should be printed to stderr
int check_FITSIO_status(const char *cfile, const char *cfunc, long cline, int print)
{
    int Ferr = 0;

    if(FITSIO_status!=0)
    {
        if(print==1)
        {
			char errstr[SBUFFERSIZE];
            fits_get_errstatus(FITSIO_status, errstr);
            fprintf(stderr,"%c[%d;%dmFITSIO error %d [%s, %s, %ld]: %s%c[%d;m\n\a",(char) 27, 1, 31, FITSIO_status, cfile, cfunc, cline, errstr, (char) 27, 0);
        }
        Ferr = FITSIO_status;
    }
    FITSIO_status = 0;

    return(Ferr);
}


int file_exists(const char *file_name)
{
    FILE *fp;
    int exists = 1;

    if((fp = fopen(file_name,"r"))==NULL)
    {
        exists = 0;
        /*      printf("file %s does not exist\n",file_name);*/
    }
    else
        fclose(fp);

    return(exists);
}





int is_fits_file(const char *file_name)
{
    int value=0;
    fitsfile *fptr;


    if (!fits_open_file(&fptr,file_name, READONLY, &FITSIO_status))
    {
        fits_close_file(fptr, &FITSIO_status);
        value = 1;
    }
    if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)==1)
    {
        int n = snprintf(errormessage_iofits,SBUFFERSIZE,"Error in function is_fits_file(%s)",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errormessage_iofits);
        //      fprintf(stderr,"%c[%d;%dm Error in function is_fits_file for file \"%s\" %c[%d;m\n", (char) 27, 1, 31, file_name, (char) 27, 0);
    }

    return(value);
}




int read_keyword(const char* file_name, const char* KEYWORD, char* content)
{
    fitsfile *fptr;         /* FITS file pointer, defined in fitsio.h */
    int exists = 0;
    int n;

    if (!fits_open_file(&fptr, file_name, READONLY, &FITSIO_status))
    {
		char comment[SBUFFERSIZE];
		char str1[SBUFFERSIZE];
		
        if (fits_read_keyword(fptr, KEYWORD, str1, comment, &FITSIO_status))
        {
            n = snprintf(errormessage_iofits, SBUFFERSIZE,"Keyword \"%s\" does not exist in file \"%s\"", KEYWORD,file_name);
            if(n >= SBUFFERSIZE)
                printERROR(__FILE__, __func__, __LINE__, "Attempted to write string buffer with too many characters");
            printERROR(__FILE__, __func__, __LINE__, errormessage_iofits);

            //	  printf("%c[%d;%dm Keyword \"%s\" does not exist in file \"%s\" %c[%d;m\n", (char) 27, 1, 31, KEYWORD,file_name, (char) 27, 0);
            exists = 1;
        }
        else
        {
            n = snprintf(content, SBUFFERSIZE, "%s\n", str1);
            if(n >= SBUFFERSIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        }
        fits_close_file(fptr, &FITSIO_status);
    }
    if(check_FITSIO_status(__FILE__, __func__, __LINE__, 0)==1)
    {
        n = snprintf(errormessage_iofits, SBUFFERSIZE,"Error reading keyword \"%s\" in file \"%s\"", KEYWORD,file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__, __func__, __LINE__, "Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errormessage_iofits);
        //      fprintf(stderr,"%c[%d;%dm Error reading keyword \"%s\" in file \"%s\" %c[%d;m\n", (char) 27, 1, 31, KEYWORD,file_name, (char) 27, 0);
    }

    return(exists);
}




int read_keyword_alone(const char* file_name, const char* KEYWORD)
{
	char *content = (char*) malloc(sizeof(char)*SBUFFERSIZE); 
 
	if(content==NULL)
    {
      printERROR(__FILE__,__func__,__LINE__,"malloc error");
      exit(0);
    }
  
  read_keyword(file_name,KEYWORD,content);
  printf("%s\n",content);

  free(content);
  content = NULL;

  return(0);
}

int data_type_code(int bitpix)
{
    int code;
    /*
      bitpix      Datatype             typecode    Mnemonic
      1           bit, X                   1        TBIT
      8           byte, B                 11        TBYTE
                  logical, L              14        TLOGICAL
                  ASCII character, A      16        TSTRING
      16          short integer, I        21        TSHORT
      32          integer, J              41        TLONG
     -32          real, E                 42        TFLOAT
     -64          double precision, D     82        TDOUBLE
                  complex, C              83        TCOMPLEX
                  double complex, M      163        TDBLCOMPLEX
                  */
    code = 0;
    if (bitpix==1) code=1;
    if (bitpix==8) code=11;
    if (bitpix==16) code=21;
    if (bitpix==32) code=41;
    if (bitpix==-32) code=42;
    if (bitpix==-64) code=82;
    return(code);
}












/* =============================================================================================== */
/*                                                                                                 */
/* 1. LOAD / SAVE                                                                                  */
/*                                                                                                 */
/* =============================================================================================== */




/// if errcode = 0, do not show error messages
/// errcode = 1: print error, continue
/// errcode = 2: exit program at error
/// errcode = 3: do not show error message, try = 1, no wait
long load_fits(const char *file_name, const char *ID_name, int errcode)
{
    fitsfile *fptr = NULL;       /* pointer to the FITS file; defined in fitsio.h */
    int nulval, anynul;
    long bitpixl = 0;
    char errstr[SBUFFERSIZE];
    long naxis = 0;
    uint32_t naxes[3];
    long ID = -1;
    double bscale;
    double bzero;
    unsigned char *barray = NULL;
    long *larray = NULL;
    unsigned short *sarray = NULL;
    long NDR=1; /* non-destructive reads */


    int fileOK;
    int try;
    int NBtry = 3;
    int PrintErrorMsg = 1;
    int ExitOnErr = 0;

    nulval = 0;
    anynul = 0;
    bscale = 1;
    bzero = 0;

    naxes[0] = 0;
    naxes[1] = 0;
    naxes[2] = 0;




    fileOK = 0;

    if(errcode==0) {
        PrintErrorMsg = 0;
        ExitOnErr = 0;
    }

    if(errcode==1) {
        PrintErrorMsg = 1;
        ExitOnErr = 0;
    }

    if(errcode==2) {
        PrintErrorMsg = 1;
        ExitOnErr = 1;
    }

    if(errcode==3)
    {
        NBtry = 1;
        PrintErrorMsg = 0;
        ExitOnErr = 0;
    }



    for(try=0; try<NBtry; try++)
        {
            if(fileOK==0)
                {
                    if (fits_open_file(&fptr, file_name, READONLY, &FITSIO_status))
                        {
                            if(check_FITSIO_status(__FILE__, __func__, __LINE__, PrintErrorMsg) != 0)
                            {
                                if(PrintErrorMsg==1)
                                {
                                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_open_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                                    list_image_ID();
                                }

                                if(ExitOnErr==1)
                                    exit(0);

                                if(try!=NBtry-1) // don't wait on last try
                                        usleep(10000);
                            }
                        
                      ID = -1;
                    }
                    else
                        fileOK = 1;
					}
        }



    if(fileOK==1)
    {
        char keyword[SBUFFERSIZE];
        long  fpixel = 1;
        long i;
        long ii;
        char comment[SBUFFERSIZE];
        long  nelements;

        fits_read_key(fptr, TLONG, "NAXIS", &naxis, comment, &FITSIO_status);
        if(errcode!=0)
        {
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_key\" NAXIS %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                list_image_ID();
                if(errcode>1)
                    exit(0);
            }
        }


        for(i=0; i<naxis; i++)
        {
            int n;

            n = snprintf(keyword,SBUFFERSIZE,"NAXIS%ld",i+1);
            if(n >= SBUFFERSIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
            fits_read_key(fptr, TLONG, keyword, &naxes[i], comment, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_key\" NAXIS%ld %c[%d;m\n", (char) 27, 1, 31, i, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }
        }

        fits_read_key(fptr, TLONG, "BITPIX", &bitpixl, comment, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
        {
            if(errcode!=0)
            {
                fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_key\" BITPIX %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                list_image_ID();
                if(errcode>1)
                    exit(0);
            }
        }



        int bitpix = (int) bitpixl;
        fits_read_key(fptr, TDOUBLE, "BSCALE", &bscale, comment, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,0)==1)
        {
            //fprintf(stderr,"Error reading keyword \"BSCALE\" in file \"%s\"\n",file_name);
            bscale = 1.0;
        }
        fits_read_key(fptr, TDOUBLE, "BZERO", &bzero, comment, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,0)==1)
        {
            //fprintf(stderr,"Error reading keyword \"BZERO\" in file \"%s\"\n",file_name);
            bzero = 0.0;
        }

        fits_set_bscale(fptr, bscale, bzero, &FITSIO_status);
        check_FITSIO_status(__FILE__,__func__,__LINE__,1);

        if(1)
        {
            printf("[%ld", (long) naxes[0]);
            for(i=1; i<naxis; i++)
                printf(",%ld", (long) naxes[i]);
            printf("] %d %f %f\n", bitpix, bscale, bzero);
            fflush(stdout);
        }

        nelements = 1;
        for(i=0; i<naxis; i++)
            nelements *= naxes[i];

        /* bitpix = -32  TFLOAT */
        if(bitpix == -32) {
            ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_FLOAT, data.SHARED_DFT, data.NBKEWORD_DFT);
            fits_read_img(fptr, data_type_code(bitpix), fpixel, nelements, &nulval, data.image[ID].array.F, &anynul, &FITSIO_status);

            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }



            fits_close_file (fptr, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            check_FITSIO_status(__FILE__, __func__, __LINE__, 1);
        }

        /* bitpix = -64  TDOUBLE */
        if(bitpix == -64) {
            ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_DOUBLE, data.SHARED_DFT, data.NBKEWORD_DFT);

            fits_read_img(fptr, data_type_code(bitpix), fpixel, nelements, &nulval, data.image[ID].array.D , &anynul, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            fits_close_file (fptr, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            check_FITSIO_status(__FILE__, __func__, __LINE__, 1);
        }

        /* bitpix = 16   TSHORT */
        if(bitpix == 16) {
            // ID = create_image_ID(ID_name, naxis, naxes, Dtype, data.SHARED_DFT, data.NBKEWORD_DFT);
            ID = create_image_ID(ID_name, naxis, naxes, _DATATYPE_UINT16, data.SHARED_DFT, data.NBKEWORD_DFT);

            /* sarray = (unsigned short*) malloc(sizeof(unsigned short)*nelements);
             if(sarray==NULL)
             {
                 printERROR(__FILE__, __func__, __LINE__, "malloc error");
                 exit(0);
             }*/

            //           fits_read_img(fptr, 20, fpixel, nelements, &nulval, sarray, &anynul, &FITSIO_status);
            fits_read_img(fptr, 20, fpixel, nelements, &nulval, data.image[ID].array.UI16, &anynul, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            fits_close_file (fptr, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            check_FITSIO_status(__FILE__, __func__, __LINE__, 1);
            /*        for (ii = 0; ii < nelements; ii++)
                        data.image[ID].array.F[ii] = 1.0*sarray[ii];
                    free(sarray);
                    sarray = NULL;*/
        }


        /* bitpix = 32   TLONG */
        if(bitpix == 32) {
            fits_read_key(fptr, TLONG, "NDR", &NDR, comment, &FITSIO_status);
            if(check_FITSIO_status(__FILE__, __func__, __LINE__, 0)==1)
                NDR = 1;
            ID = create_image_ID(ID_name, naxis, naxes, Dtype, data.SHARED_DFT, data.NBKEWORD_DFT);
            larray = (long*) malloc(sizeof(long)*nelements);
            if(larray==NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"malloc error");
                exit(0);
            }

            fits_read_img(fptr, data_type_code(bitpix), fpixel, nelements, &nulval, larray, &anynul, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            fits_close_file (fptr, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            bzero = 0.0;
            for (ii = 0; ii < nelements; ii++)
                data.image[ID].array.F[ii] = ((1.0*larray[ii]*bscale + bzero)/NDR);
            free(larray);
            larray = NULL;
        }

        /* bitpix = 8   TBYTE */
        if(bitpix == 8) {
            ID = create_image_ID(ID_name, naxis, naxes, Dtype, data.SHARED_DFT, data.NBKEWORD_DFT);
            barray = (unsigned char*) malloc(sizeof(unsigned char)*naxes[1]*naxes[0]);
            if(barray==NULL)
            {
                printERROR(__FILE__, __func__, __LINE__, "malloc error");
                exit(0);
            }

            fits_read_img(fptr, data_type_code(bitpix), fpixel, nelements, &nulval, barray, &anynul, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_read_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }

            fits_close_file (fptr, &FITSIO_status);
            if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
            {
                if(errcode!=0)
                {
                    fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm within load_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
                    fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                    list_image_ID();
                    if(errcode>1)
                        exit(0);
                }
            }


            for (ii = 0; ii < nelements; ii++)
                data.image[ID].array.F[ii] = (1.0*barray[ii]*bscale+bzero);
            free(barray);
            barray = NULL;
        }
    }

    return(ID);
}






/* saves an image in a double format */

int save_db_fits(const char *ID_name, const char *file_name)
{
    fitsfile *fptr;
    long nelements;
    long ID;
    char file_name1[SBUFFERSIZE];
    int n;

    if((data.overwrite == 1)&&(file_name[0]!='!')&&(file_exists(file_name)==1))
    {
        n = snprintf(errormessage_iofits,SBUFFERSIZE,"automatic overwrite on file \"%s\"\n",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printWARNING(__FILE__,__func__,__LINE__,errormessage_iofits);
        n = snprintf(file_name1,SBUFFERSIZE,"!%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }
    else
    {
        n = snprintf(file_name1, SBUFFERSIZE, "%s", file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__, __func__, __LINE__, "Attempted to write string buffer with too many characters");
    }

    ID = image_ID(ID_name);

    if(ID!=-1)
    {
		long  fpixel = 1;
		uint32_t naxes[3];
		long naxesl[3];
		double *array;
		uint8_t atype;
		long naxis;
		long i;
		
		 
        atype = data.image[ID].md[0].atype;
        naxis = data.image[ID].md[0].naxis;
        for(i=0; i<naxis; i++)
        {
            naxes[i] = data.image[ID].md[0].size[i];
			naxesl[i] = (long) naxes[i];
		}
  
     

        nelements = 1;
        for(i=0; i<naxis; i++)
            nelements *= naxes[i];


        if (atype != _DATATYPE_DOUBLE) // data conversion required
		{
			 long ii;
			
			array = (double*) malloc(SIZEOF_DATATYPE_DOUBLE*nelements);
			if(array==NULL)
            {
				printERROR(__FILE__,__func__,__LINE__,"malloc error");
                exit(0);
            }
            
            switch (atype)
            {
				case _DATATYPE_UINT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.UI8[ii];
				break;

				case _DATATYPE_INT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.SI8[ii];
				break;

				case _DATATYPE_UINT16 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.UI16[ii];
				break;
				
				case _DATATYPE_INT16 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.SI16[ii];
				break;
				
				case _DATATYPE_UINT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.UI32[ii];
				break;

				case _DATATYPE_INT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.SI32[ii];
				break;

				case _DATATYPE_UINT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.UI64[ii];
				break;

				case _DATATYPE_INT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.SI64[ii];
				break;
        
				case _DATATYPE_FLOAT :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (double) data.image[ID].array.F[ii];
				break;
        		        		        			
				default :
				list_image_ID();
				printERROR(__FILE__,__func__,__LINE__,"atype value not recognised");
				printf("ID %ld  atype = %d\n", ID, atype);
				free(array);
				exit(0);
				break;				
			}            
		}    

    

        fits_create_file(&fptr, file_name1, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_create_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_db_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }


        fits_create_img(fptr, DOUBLE_IMG, naxis, naxesl, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_create_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_db_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        if(atype == _DATATYPE_DOUBLE)
            fits_write_img(fptr, TDOUBLE, fpixel, nelements, data.image[ID].array.D, &FITSIO_status);
        else
        {    
			fits_write_img(fptr, TDOUBLE, fpixel, nelements, array, &FITSIO_status);
            free(array);
            array = NULL;
		}        
        
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_write_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_db_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        fits_close_file (fptr, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1) != 0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_db_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }
    }
    else
    {
        printf("image does not exist in memory\n");
    }

    return( 0 );
}





/* saves an image in a float format */

int save_fl_fits(const char *ID_name, const char *file_name)
{
    fitsfile *fptr;
    long  naxis, nelements;
    float *array = NULL;
    long ID;
    long ii;
    long i;
    char file_name1[SBUFFERSIZE];
    int n;

    if((data.overwrite == 1) && (file_name[0]!='!') && (file_exists(file_name)==1))
    {
        n = snprintf(errormessage_iofits,SBUFFERSIZE,"automatic overwrite on file \"%s\"\n",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printWARNING(__FILE__,__func__,__LINE__,errormessage_iofits);
        //	printf("WARNING: automatic overwrite on file \"%s\"\n",file_name);
        n = snprintf(file_name1,SBUFFERSIZE,"!%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }
    else
    {
        n = snprintf(file_name1,SBUFFERSIZE,"%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }

    ID = image_ID(ID_name);

    if (ID!=-1)
    {
	    uint8_t atype;
        long naxesl[3];
        uint32_t naxes[3];
        long  fpixel = 1;
		
        atype = data.image[ID].md[0].atype;
        naxis = data.image[ID].md[0].naxis;
        for(i=0; i<naxis; i++)
        {    
			naxes[i] = data.image[ID].md[0].size[i];
			naxesl[i] = (long) naxes[i];
		}
		
        nelements = 1;
        for(i=0; i<naxis; i++)
            nelements *= naxes[i];


        if (atype != _DATATYPE_FLOAT) // data conversion required
		{
			array = (float*) malloc(SIZEOF_DATATYPE_FLOAT*nelements);
			if(array==NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"malloc error");
                exit(0);
            }
            
            switch (atype)
            {
				case _DATATYPE_UINT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.UI8[ii];
				break;

				case _DATATYPE_INT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.SI8[ii];
				break;

				case _DATATYPE_UINT16 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.UI16[ii];
				break;
				
				case _DATATYPE_INT16 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.SI16[ii];
				break;
				
				case _DATATYPE_UINT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.UI32[ii];
				break;

				case _DATATYPE_INT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.SI32[ii];
				break;

				case _DATATYPE_UINT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.UI64[ii];
				break;

				case _DATATYPE_INT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.SI64[ii];
				break;
        
				case _DATATYPE_DOUBLE :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (float) data.image[ID].array.D[ii];
				break;
        			
				default :
				list_image_ID();
				printERROR(__FILE__,__func__,__LINE__,"atype value not recognised");
				printf("ID %ld  atype = %d\n", ID, atype);
				free(array);
				exit(0);
				break;				
			}            
		}    

 		
		FITSIO_status = 0;
        fits_create_file(&fptr, file_name1, &FITSIO_status);
        if(check_FITSIO_status(__FILE__, __func__, __LINE__, 1) != 0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_create_file\" with filename \"%s\" %c[%d;m\n", (char) 27, 1, 31, file_name1, (char) 27, 0);
            if(file_exists(file_name1)==1)
            {
                fprintf(stderr,"%c[%d;%dm File \"%s\" already exists. Make sure you remove this file before attempting to write file with identical name. %c[%d;m\n", (char) 27, 1, 31,file_name1, (char) 27, 0);
                exit(0);
            }
            else
            {
                fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                list_image_ID();
            }
        }

        fits_create_img(fptr, FLOAT_IMG, (int) naxis, naxesl, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_create_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_fl_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        if(atype==_DATATYPE_FLOAT)
            fits_write_img(fptr, TFLOAT, fpixel, nelements, data.image[ID].array.F, &FITSIO_status);
        else
        {    
			fits_write_img(fptr, TFLOAT, fpixel, nelements, array, &FITSIO_status);
			free(array);
            array = NULL;
		}

        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_write_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_fl_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        fits_close_file (fptr, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr, "%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm within save_fl_fits ( %s, %s ) %c[%d;m\n", (char) 27, 1, 31, ID_name, file_name, (char) 27, 0);
            fprintf(stderr, "%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }
    }
    else
        fprintf(stderr,"%c[%d;%dm image \"%s\" does not exist in memory %c[%d;m\n", (char) 27, 1, 31, ID_name, (char) 27, 0);

    return(0);
}





/* saves an image in a short int format */

int save_sh_fits(const char *ID_name, const char *file_name)
{
    fitsfile *fptr;
    long  fpixel = 1, naxis, nelements;
    uint32_t naxes[3];
    long naxesl[3];
    int16_t *array = NULL;
    long ID;
    long ii;
    long i;
    uint8_t atype;
    char file_name1[SBUFFERSIZE];
    int n;


    if((data.overwrite == 1)&&(file_name[0]!='!')&&(file_exists(file_name)==1))
    {
        n = snprintf(errormessage_iofits,SBUFFERSIZE,"automatic overwrite on file \"%s\"\n",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printWARNING(__FILE__,__func__,__LINE__,errormessage_iofits);
        //	printf("WARNING: automatic overwrite on file \"%s\"\n",file_name);
        n = snprintf(file_name1,SBUFFERSIZE,"!%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }
    else
    {
        n = snprintf(file_name1,SBUFFERSIZE,"%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }

    ID = image_ID(ID_name);

    if (ID!=-1)
    {
        atype = data.image[ID].md[0].atype;
        naxis=data.image[ID].md[0].naxis;
        for(i=0; i<naxis; i++)
        {
            naxes[i] = data.image[ID].md[0].size[i];
			naxesl[i] = (long) naxes[i];
		}

        nelements = 1;
        for(i=0; i<naxis; i++)
            nelements *= naxes[i];
            
        if (atype != _DATATYPE_INT16) // data conversion required
		{
			array = (int16_t*) malloc(SIZEOF_DATATYPE_INT16*nelements);
			if(array==NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"malloc error");
                exit(0);
            }
            
            switch (atype)
            {
				case _DATATYPE_UINT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.UI8[ii];
				break;

				case _DATATYPE_INT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.SI8[ii];
				break;

				case _DATATYPE_UINT16 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.UI16[ii];
				break;
				
				case _DATATYPE_UINT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.UI32[ii];
				break;

				case _DATATYPE_INT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.SI32[ii];
				break;

				case _DATATYPE_UINT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.UI64[ii];
				break;

				case _DATATYPE_INT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.SI64[ii];
				break;
				
				case _DATATYPE_FLOAT :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.F[ii];
				break;
        
				case _DATATYPE_DOUBLE :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (int16_t) data.image[ID].array.D[ii];
				break;
        			
				default :
				list_image_ID();
				printERROR(__FILE__,__func__,__LINE__,"atype value not recognised");
				free(array);
				printf("ID %ld  atype = %d\n", ID, atype);
				exit(0);
				break;				
			}            
		}    

        fits_create_file(&fptr, file_name1, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_create_file\" with filename \"%s\" %c[%d;m\n", (char) 27, 1, 31,file_name1, (char) 27, 0);
            if(file_exists(file_name1)==1)
            {
                fprintf(stderr,"%c[%d;%dm File \"%s\" already exists. Make sure you remove this file before attempting to write file with identical name. %c[%d;m\n", (char) 27, 1, 31,file_name1, (char) 27, 0);
                exit(0);
            }
            else
            {
                fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                list_image_ID();
            }
        }
        //    16          short integer, I        21        TSHORT
        fits_create_img(fptr, SHORT_IMG, naxis, naxesl, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_create_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        if(atype==_DATATYPE_UINT16)
            fits_write_img(fptr, TSHORT, fpixel, nelements, data.image[ID].array.UI16, &FITSIO_status);
        else
        {    
			fits_write_img(fptr, TSHORT, fpixel, nelements, array, &FITSIO_status);
			free(array);
			array = NULL;
		}


        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_write_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        fits_close_file (fptr, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }
    }
    else
        fprintf(stderr,"%c[%d;%dm image \"%s\" does not exist in memory %c[%d;m\n", (char) 27, 1, 31, ID_name, (char) 27, 0);

    return(0);
}






/* saves an image in a unsigned short int format */

int save_ush_fits(const char *ID_name, const char *file_name)
{
    fitsfile *fptr;
    long  fpixel = 1, naxis, nelements;
    uint32_t naxes[3];
    long naxesl[3];
    uint16_t *array = NULL;
    long ID;
    long ii;
    long i;
    uint8_t atype;
    char file_name1[SBUFFERSIZE];
    int n;


    if((data.overwrite == 1)&&(file_name[0]!='!')&&(file_exists(file_name)==1))
    {
        n = snprintf(errormessage_iofits,SBUFFERSIZE,"automatic overwrite on file \"%s\"\n",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printWARNING(__FILE__,__func__,__LINE__,errormessage_iofits);
        //	printf("WARNING: automatic overwrite on file \"%s\"\n",file_name);
        n = snprintf(file_name1,SBUFFERSIZE,"!%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }
    else
    {
        n = snprintf(file_name1,SBUFFERSIZE,"%s",file_name);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
    }

    ID = image_ID(ID_name);

    if (ID!=-1)
    {
        atype = data.image[ID].md[0].atype;
        naxis=data.image[ID].md[0].naxis;
        for(i=0; i<naxis; i++)
        {
		    naxes[i] = data.image[ID].md[0].size[i];
			naxesl[i] = (long) naxes[i]; // CFITSIO expects long int *
		}

        nelements = 1;
        for(i=0; i<naxis; i++)
            nelements *= naxes[i];
            
        if (atype != _DATATYPE_UINT16) // data conversion required
		{
			array = (uint16_t*) malloc(SIZEOF_DATATYPE_UINT16*nelements);
			if(array==NULL)
            {
                printERROR(__FILE__,__func__,__LINE__,"malloc error");
                exit(0);
            }
            
            switch (atype)
            {
				case _DATATYPE_UINT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.UI8[ii];
				break;

				case _DATATYPE_INT8 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.SI8[ii];
				break;

				case _DATATYPE_INT16 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.SI16[ii];
				break;
				
				case _DATATYPE_UINT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.UI32[ii];
				break;

				case _DATATYPE_INT32 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.SI32[ii];
				break;

				case _DATATYPE_UINT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.UI64[ii];
				break;

				case _DATATYPE_INT64 :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.SI64[ii];
				break;
				
				case _DATATYPE_FLOAT :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.F[ii];
				break;
        
				case _DATATYPE_DOUBLE :
				for (ii = 0; ii < nelements; ii++)
					array[ii] = (uint16_t) data.image[ID].array.D[ii];
				break;
        			
				default :
				list_image_ID();
				printERROR(__FILE__,__func__,__LINE__,"atype value not recognised");
				free(array);
				printf("ID %ld  atype = %d\n", ID, atype);
				exit(0);
				break;				
			}            
		}    

        fits_create_file(&fptr, file_name1, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_create_file\" with filename \"%s\" %c[%d;m\n", (char) 27, 1, 31,file_name1, (char) 27, 0);
            if(file_exists(file_name1)==1)
            {
                fprintf(stderr,"%c[%d;%dm File \"%s\" already exists. Make sure you remove this file before attempting to write file with identical name. %c[%d;m\n", (char) 27, 1, 31,file_name1, (char) 27, 0);
                exit(0);
            }
            else
            {
                fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
                list_image_ID();
            }
        }

        fits_create_img(fptr, USHORT_IMG, naxis, naxesl, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_create_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        if(atype == _DATATYPE_UINT16)
            fits_write_img(fptr, TUSHORT, fpixel, nelements, data.image[ID].array.UI16, &FITSIO_status);
        else
        {   
			fits_write_img(fptr, TUSHORT, fpixel, nelements, array, &FITSIO_status);
			 free(array);
			 array = NULL;
		}
			
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_write_img\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }

        fits_close_file (fptr, &FITSIO_status);
        if(check_FITSIO_status(__FILE__,__func__,__LINE__,1)!=0)
        {
            fprintf(stderr,"%c[%d;%dm Error while calling \"fits_close_file\" %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            fprintf(stderr,"%c[%d;%dm Printing Cfits image buffer content: %c[%d;m\n", (char) 27, 1, 31, (char) 27, 0);
            list_image_ID();
        }
    }
    else
        fprintf(stderr,"%c[%d;%dm image \"%s\" does not exist in memory %c[%d;m\n", (char) 27, 1, 31, ID_name, (char) 27, 0);

    return(0);
}











int save_fits(const char *ID_name, const char *file_name)
{
    char savename[1000];
    if (file_name[0] == '!')
        strcpy(savename, file_name+1); // avoid the leading '!'
    else
        strcpy(savename, file_name);
    
    return save_fits_atomic(ID_name, savename);
}




int save_fits_atomic(const char *ID_name, const char *file_name)
{
    long ID;
    uint8_t atype;
    char fnametmp[1000];
    char savename[1000];
	pthread_t self_id;
	
    ID = image_ID(ID_name);

	self_id=pthread_self();
	
	if(sprintf(fnametmp, "_savefits_atomic_%s_%d_%ld.tmp.fits", ID_name, (int) getpid(), (long) self_id) < 1)
		printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");
		
	if(sprintf(savename, "!%s", fnametmp) < 1)
		printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");
	
    if (ID!=-1)
    {
		char command[2000];
		
        atype = data.image[ID].md[0].atype;
        switch(atype) {
        case _DATATYPE_UINT8:
            save_ush_fits(ID_name, savename);
            break;
        case _DATATYPE_INT8:
            save_sh_fits(ID_name, savename);
            break;
        case _DATATYPE_UINT16:
            save_ush_fits(ID_name, savename);
            break;
        case _DATATYPE_INT16:
            save_sh_fits(ID_name, savename);
            break;            
        // TBD: i32 and i64     
        case _DATATYPE_FLOAT:
            save_fl_fits(ID_name, savename);
            break;
        case _DATATYPE_DOUBLE:
            save_db_fits(ID_name, savename);
            break;
        }
		
		if(sprintf(command, "mv %s %s", fnametmp, file_name) < 1)
			printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");

		if(system(command) != 0)
			printERROR(__FILE__, __func__, __LINE__, "system() returns non-zero value");
    }

    return 0;
}



int saveall_fits(const char *savedirname)
{
    long i;
    char fname[200];
    char command[200];
    
    if(sprintf(command, "mkdir -p %s", savedirname) < 1)
		printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");

    if(system(command) != 0)
		printERROR(__FILE__, __func__, __LINE__, "system() returns non-zero value");
    
    for (i=0; i<data.NB_MAX_IMAGE; i++)
        if(data.image[i].used==1)
            {
                if(sprintf(fname, "!./%s/%s.fits", savedirname, data.image[i].name) < 1)
					printERROR(__FILE__, __func__, __LINE__, "sprintf wrote <1 char");

                
                save_fits(data.image[i].name, fname);
            }
    return(0);
}












/* =============================================================================================== */
/*                                                                                                 */
/* 2. CUBES                                                                                        */
/*                                                                                                 */
/* =============================================================================================== */


int break_cube(const char *ID_name)
{
    long ID;
    uint32_t naxes[3];
    long ii,jj,kk;
    char framename[SBUFFERSIZE];
    long i;
    int n;

    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];
    naxes[2] = data.image[ID].md[0].size[2];

    for(kk=0; kk<naxes[2]; kk++)
    {
		long ID1;
		
        n = snprintf(framename,SBUFFERSIZE,"%s_%5ld",ID_name,kk);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__, __func__, __LINE__, "Attempted to write string buffer with too many characters");
        for(i=0; i<(long) strlen(framename); i++)
        {
            if(framename[i] == ' ')
                framename[i] = '0';
        }
        ID1 = create_2Dimage_ID(framename,naxes[0],naxes[1]);
        for(ii=0; ii<naxes[0]; ii++)
            for(jj=0; jj<naxes[1]; jj++)
            {
                data.image[ID1].array.F[jj*naxes[0]+ii] = data.image[ID].array.F[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii];
            }
    }

    return(0);
}




int images_to_cube(const char *img_name, long nbframes, const char *cube_name)
{
    long ID,ID1;
    long frame;
    char imname[SBUFFERSIZE];
    uint32_t naxes[2];
    long ii,jj;
    uint32_t xsize, ysize;
    int n;

    frame = 0;
    n = snprintf(imname,SBUFFERSIZE,"%s%05ld", img_name, frame);
    if(n >= SBUFFERSIZE)
        printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");

    ID1 = image_ID(imname);
    if(ID1==-1)
    {
        n = snprintf(errormessage_iofits,SBUFFERSIZE,"Image \"%s\" does not exist",imname);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        printERROR(__FILE__,__func__,__LINE__,errormessage_iofits);
        exit(0);
    }
    naxes[0] = data.image[ID1].md[0].size[0];
    naxes[1] = data.image[ID1].md[0].size[1];
    xsize = naxes[0];
    ysize = naxes[1];

    printf("SIZE = %ld %ld %ld\n", (long) naxes[0], (long) naxes[1], (long) nbframes);
    fflush(stdout);
    ID = create_3Dimage_ID(cube_name,naxes[0],naxes[1],nbframes);
    for (ii = 0; ii < naxes[0]; ii++)
        for (jj = 0; jj < naxes[1]; jj++)
            data.image[ID].array.F[frame*naxes[0]*naxes[1]+(jj*naxes[0]+ii)] = data.image[ID1].array.F[jj*naxes[0]+ii];

    for(frame=1; frame<nbframes; frame++)
    {
        n = snprintf(imname,SBUFFERSIZE,"%s%05ld",img_name,frame);
        printf("Adding image %s -> %ld/%ld ... ",img_name,frame,nbframes);
        fflush(stdout);
        if(n >= SBUFFERSIZE)
            printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
        ID1 = image_ID(imname);
        if(ID1==-1)
        {
            n = snprintf(errormessage_iofits,SBUFFERSIZE,"Image \"%s\" does not exist - skipping",imname);
            if(n >= SBUFFERSIZE)
                printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
            printERROR(__FILE__,__func__,__LINE__,errormessage_iofits);
        }
        else
        {
            naxes[0] = data.image[ID1].md[0].size[0];
            naxes[1] = data.image[ID1].md[0].size[1];
            if((xsize != naxes[0])||(ysize != naxes[1]))
            {
                printERROR(__FILE__,__func__,__LINE__,"Image has wrong size");
                exit(0);
            }
            for (ii = 0; ii < naxes[0]; ii++)
                for (jj = 0; jj < naxes[1]; jj++)
                    data.image[ID].array.F[frame*naxes[0]*naxes[1]+(jj*naxes[0]+ii)] = data.image[ID1].array.F[jj*naxes[0]+ii];
        }
        printf("Done\n");
        fflush(stdout);
    }

    return(0);
}

