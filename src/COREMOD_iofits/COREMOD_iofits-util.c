/*^****************************************************************************
* FILE: COREMOD_iofits-util.c : Module utility file
*
*
*     File naming convention: Modulename-util.c
*
*
* modules-config.h was generated at build time by module names 
* in modules-included.list
*
*******************************************************************************/
#include <sys/stat.h>
#include <time.h>
#include <Cfits.h>  // Generic  header
#include <COREMOD_iofits.h>     // Header for this module


/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
PF mod_COREMOD_iofits_readkey           ( struct lxrcmd *c ); 
PF mod_COREMOD_iofits_loadfits           ( struct lxrcmd *c ); 
PF mod_COREMOD_iofits_save_fl           ( struct lxrcmd *c ); 
PF mod_COREMOD_iofits_save_db           ( struct lxrcmd *c ); 
PF mod_COREMOD_iofits_save_sh            ( struct lxrcmd *c );
PF mod_COREMOD_iofits_saveall           ( struct lxrcmd *c ); 
PF mod_COREMOD_iofits_breakcube           ( struct lxrcmd *c ); 
PF mod_COREMOD_iofits_img2cube           ( struct lxrcmd *c ); 


/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_COREMOD_iofits_cmds[] = {

    {    "readkey",
         mod_COREMOD_iofits_readkey,
         "read value of a keyword",
         "read value of a keyword",
         "readkey im1.fits exptime",
         "%s %s %s",
         3,
         "read_keyword_alone",
    },
    {    "loadfits",
         mod_COREMOD_iofits_loadfits,
         "loads a fits file in memory",
         "str1 is the file name on disk and str2 is the file name in memory",
         "loadfits image.fits im1",
         "%s %s %s",
         3,
         "load_fits",
    },
    {    "save_fl",
         mod_COREMOD_iofits_save_fl,
         "save a fits file in float format",
         "save a fits file in float format",
         "save_fl im1 im1.fits",
         "%s %s %s",
         3,
         "save_fl_fits",
    },
    {    "save_db",
         mod_COREMOD_iofits_save_db,
         "save a fits file in double format",
         "save a fits file in double format",
         "save_fl im1 im1.fits",
         "%s %s %s",
         3,
         "save_db_fits",
    },
    {    "save_sh",
         mod_COREMOD_iofits_save_sh,
         "save a fits file in short int format",
         "save a fits file in short int format",
         "save_sh im1 im1.fits",
         "%s %s %s",
         3,
         "save_sh_fits",
    },
    {    "saveall",
         mod_COREMOD_iofits_saveall,
         "save all image files on disk",
         "save all image files on disk",
         "saveall",
         "%s", 
         1,
         "saveall_fl_fits",
    },
    {    "breakcube",
         mod_COREMOD_iofits_breakcube,
         "break a cube",
         "break a cube",
         "breakcube cube",
         "%s %s",
         2,
         "break_cube",
    },
    {    "img2cube",
         mod_COREMOD_iofits_img2cube,
         "convert series of images into a cube",
         "str1 is the image name prefix, l1 is the number of images, str2 is the output image name",
         "img2cube frame_ 100 cube",
         "%s %s %ld %s",
         4,
         "images_to_cube",
    },
};




/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       COREMOD_iofits.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_init ( struct module *m )
{
  int i;		    	    
  FILE *fp;
  struct stat finfo;
  char str0[200];
  char str1[200];
  

  // Set module name
  sprintf( m->name, "COREMOD_iofits");
  if( Debug > 1) fprintf(stdout, "[mod_%s_init]\n",m->name);
  
  // Set module info line
  sprintf(m->info,"FITS files I/O routines with FITSIO library");

  // Set number of commands for this module
  m->ncommands = sizeof(mod_COREMOD_iofits_cmds) / sizeof(struct lxrcmd) ;
  // Set command control block
  m->cmds = mod_COREMOD_iofits_cmds;

  // set module-control-block index in every command-control-block 
  for( i=0; i<m->ncommands; ++i ) {
    mod_COREMOD_iofits_cmds[i].cdata_01 = m->module_number; 
  }

  // Set module compile time ascii entry
  sprintf(str0, "unknown");
  sprintf(str1, "%s/%s.o", OBJDIR, m->name);
  if (!stat(str1, &finfo)) {
    sprintf(str0, "%s", asctime(localtime(&finfo.st_mtime)));}
  else { fprintf(stderr,"%c[%d;%dm WARNING: cannot find file %s [ %s  %s  %d ] %c[%d;m\n",(char) 27, 1, 31, str1, __FILE__, __func__, __LINE__, (char) 27, 0);}
  strncpy( m->buildtime, str0, strlen(str0)-1);
  m->buildtime[strlen(str0)] = '\0';  

  return(PASS);
}




/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_readkey : command function for user command "readkey"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_readkey ( struct lxrcmd *c ) 
{

    read_keyword_alone ( c->args[1].v.s, c->args[2].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_loadfits : command function for user command "loadfits"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_loadfits ( struct lxrcmd *c ) 
{
  char fname[200];
  long i,istart,iend;
  int loopOK = 1;

  if(strlen(c->args[2].v.s)==0)
    {
      istart = 0;
      iend = strlen(c->args[1].v.s);
      for(i=0;i<strlen(c->args[1].v.s);i++)
	{
	  if((c->args[1].v.s[i]=='.')||((i>0)&&(c->args[1].v.s[i]=='"')))
	    iend = i;
	  if(c->args[1].v.s[i]=='/')
	    istart = i+1;
	}
      if((istart==0)&&(c->args[1].v.s[i]=='"'))
	istart = 1;
      
      for(i=0;i<iend-istart;i++)
	fname[i] = c->args[1].v.s[i+istart];
      fname[i] = '\0';

      if(iend-istart<1)
	{
	  printERROR(__FILE__,__func__,__LINE__,"Cannot create image name from file name\n");
	  exit(0);
	}

      printf("Loading image %s as %s\n",c->args[1].v.s,fname);
      load_fits ( c->args[1].v.s, fname);   
    }
  else
    load_fits ( c->args[1].v.s, c->args[2].v.s);   
  
  
  c->results = NULL;   // NULL results block. Change as appropriate


  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_save_fl : command function for user command "save_fl"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_save_fl ( struct lxrcmd *c ) 
{
  char fname[200];
  
  if(strlen(c->args[2].v.s)==0)
    {
      sprintf(fname,"%s.fits",c->args[1].v.s);
      printf("Saving image as %s\n",fname);
      save_fl_fits ( c->args[1].v.s, fname);   
    }
  else
    save_fl_fits ( c->args[1].v.s, c->args[2].v.s);   


  c->results = NULL;   // NULL results block. Change as appropriate


  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_save_db : command function for user command "save_db"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_save_db ( struct lxrcmd *c ) 
{
  char fname[200];
  
  if(strlen(c->args[2].v.s)==0)
    {
      sprintf(fname,"%s.fits",c->args[1].v.s);
      printf("Saving image as %s\n",fname);
      save_db_fits ( c->args[1].v.s, fname);   
    }
  else
    save_db_fits ( c->args[1].v.s, c->args[2].v.s);   
  
  c->results = NULL;   // NULL results block. Change as appropriate

  return(PASS);
}


PF mod_COREMOD_iofits_save_sh ( struct lxrcmd *c ) 
{
  char fname[200];
  
  if(strlen(c->args[2].v.s)==0)
    {
      sprintf(fname,"%s.fits",c->args[1].v.s);
      printf("Saving image as %s\n",fname);
      save_sh_fits ( c->args[1].v.s, fname);   
    }
  else
    save_sh_fits ( c->args[1].v.s, c->args[2].v.s);   
  
  c->results = NULL;   // NULL results block. Change as appropriate

  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_saveall : command function for user command "saveall"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_saveall ( struct lxrcmd *c ) 
{

    saveall_fl_fits ( );   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_breakcube : command function for user command "breakcube"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_breakcube ( struct lxrcmd *c ) 
{

    break_cube ( c->args[1].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_iofits_img2cube : command function for user command "img2cube"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_iofits_img2cube ( struct lxrcmd *c ) 
{

    images_to_cube ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}
