/*^****************************************************************************
* FILE: COREMOD_memory-util.c : Module utility file
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
#include <COREMOD_memory.h>     // Header for this module


/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
PF mod_COREMOD_memory_listim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_listim_file      ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_listvar           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_creaim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_creacim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_crea3dim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_delim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_delims           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_copyim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_mv           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_reim2c           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_amph2c           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_c2reim           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_c2amph           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_clearall           ( struct lxrcmd *c ); 
PF mod_COREMOD_memory_rotcube           ( struct lxrcmd *c ); 


/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_COREMOD_memory_cmds[] = {

    {    "listim",
         mod_COREMOD_memory_listim,
         "lists images in memory",
         "lists images in memory",
         "listim",
         "%s", 
         1,
         "list_image_ID",
    },
    {    "listimf",
         mod_COREMOD_memory_listim_file,
         "lists images in memory to file",
         "lists images in memory to file",
         "listimf",
         "%s %s", 
         2,
         "list_image_ID_file",
    },
    {    "listvar",
         mod_COREMOD_memory_listvar,
         "lists variables in memory",
         "lists variables in memory",
         "listvar",
         "%s", 
         1,
         "list_variable_ID",
    },
    {    "creaim",
         mod_COREMOD_memory_creaim,
         "creates an image",
         "creates a l1 x l2 image of name str1.",
         "creaim im1 512 512",
         "%s %s %ld %ld",
         4,
         "create_2Dimage_ID",
    },
    {    "creacim",
         mod_COREMOD_memory_creacim,
         "creates an complex image",
         "creates a l1 x l2 complex image of name str1.",
         "creaim im1 512 512",
         "%s %s %ld %ld",
         4,
         "create_2DCimage_ID",
    },
    {    "crea3dim",
         mod_COREMOD_memory_crea3dim,
         "creates an image",
         "creates a l1 x l2 x l3 image of name str1.",
         "creaim im1 512 512 512",
         "%s %s %ld %ld %ld",
         5,
         "create_3Dimage_ID",
    },
    {    "delim",
         mod_COREMOD_memory_delim,
         "deletes an image",
         "deletes an image",
         "delim im1",
         "%s %s",
         2,
         "delete_image_ID",
    },
    {    "delims",
         mod_COREMOD_memory_delims,
         "delete images matching a prefix",
         "str1 is the prefix",
         "delims im_",
         "%s %s",
         2,
         "delete_image_ID_prefix",
    },
    {    "copyim",
         mod_COREMOD_memory_copyim,
         "copy of an image file",
         "copy of an image file",
         "copyim im1 im1_copy",
         "%s %s %s",
         3,
         "copy_image_ID",
    },
    {    "mv",
         mod_COREMOD_memory_mv,
         "change image name",
         "change image name",
         "mv old new",
         "%s %s %s",
         3,
         "chname_image_ID",
    },
    {    "reim2c",
         mod_COREMOD_memory_reim2c,
         "(re,im) to complex",
         "str1 is re, str2 is im, str3 is the output",
         "reim2c re im cplx",
         "%s %s %s %s",
         4,
         "mk_complex_from_reim",
    },
    {    "amph2c",
         mod_COREMOD_memory_amph2c,
         "(am,ph) to complex",
         "str1 is am, str2 is ph, str3 is the output",
         "amph2c am ph cplx",
         "%s %s %s %s",
         4,
         "mk_complex_from_amph",
    },
    {    "c2reim",
         mod_COREMOD_memory_c2reim,
         "complex to (re,im)",
         "str1 is input, str2 is re, str3 is im",
         "c2reim cplx re im",
         "%s %s %s %s",
         4,
         "mk_reim_from_complex",
    },
    {    "c2amph",
         mod_COREMOD_memory_c2amph,
         "complex to (am,ph)",
         "str1 is input, str2 is am, str3 is ph",
         "c2amph cplx am ph",
         "%s %s %s %s",
         4,
         "mk_amph_from_complex",
    },
    {    "clearall",
         mod_COREMOD_memory_clearall,
         "clear all buffers",
         "clear all buffers",
         "clearall",
         "%s", 
         1,
         "clearall",
    },
    {    "rotcube",
         mod_COREMOD_memory_rotcube,
         "view data cube from other orientation",
         "str1 is input, str2 is output, l1 is orientation: 0 for view from x axis, 1 for view from y axis.",
         "rotcube im out 0",
         "%s %s %s %ld",
         4,
         "rotate_cube",
    },
};




/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       COREMOD_memory.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_init ( struct module *m )					    
{										    
  int i;		    	    
  FILE *fp;  struct stat finfo;
  char str0[200];
  char str1[200];


  if( Debug > 1) fprintf(stdout, "[mod_COREMOD_memory_init]\n");
  
  // Set module name
  sprintf(m->name,"COREMOD_memory");
  // Set module info line
  sprintf(m->info,"routines for memory management");

  // Set number of commands for this module					    
  m->ncommands = sizeof(mod_COREMOD_memory_cmds) / sizeof(struct lxrcmd) ;		    
  // Set command control block
  m->cmds = mod_COREMOD_memory_cmds;
		 								    

    // set module-control-block index in every command-control-block 
    for( i=0; i<m->ncommands; ++i ) {
        mod_COREMOD_memory_cmds[i].cdata_01 = m->module_number; 
    }

 // Set module compile time ascii entry
   sprintf(str0, "unknown");
   sprintf(str1, "%s/COREMOD_memory.o", OBJDIR);
if (!stat(str1, &finfo)) {
   sprintf(str0, "%s", asctime(localtime(&finfo.st_mtime)));}
else { printf("ERROR: cannot find file %s\n",str1);}
   strncpy( m->buildtime, str0, strlen(str0)-1);
   m->buildtime[strlen(str0)] = '\0';

    return(PASS);
}






/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_listim : command function for user command "listim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_listim ( struct lxrcmd *c ) 
{

    list_image_ID ( );   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_listim : command function for user command "listim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_listim_file ( struct lxrcmd *c ) 
{

    list_image_ID_file ( c->args[1].v.s );   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_listvar : command function for user command "listvar"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_listvar ( struct lxrcmd *c ) 
{

    list_variable_ID ( );   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_creaim : command function for user command "creaim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_creaim ( struct lxrcmd *c ) 
{

    create_2Dimage_ID ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_creacim : command function for user command "creacim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_creacim ( struct lxrcmd *c ) 
{

    create_2DCimage_ID ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_crea3dim : command function for user command "crea3dim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_crea3dim ( struct lxrcmd *c ) 
{

    create_3Dimage_ID ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_delim : command function for user command "delim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_delim ( struct lxrcmd *c ) 
{

    delete_image_ID ( c->args[1].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_delims : command function for user command "delims"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_delims ( struct lxrcmd *c ) 
{

    delete_image_ID_prefix ( c->args[1].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_copyim : command function for user command "copyim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_copyim ( struct lxrcmd *c ) 
{

    copy_image_ID ( c->args[1].v.s, c->args[2].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_mv : command function for user command "mv"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_mv ( struct lxrcmd *c ) 
{

    chname_image_ID ( c->args[1].v.s, c->args[2].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_reim2c : command function for user command "reim2c"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_reim2c ( struct lxrcmd *c ) 
{

    mk_complex_from_reim ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_amph2c : command function for user command "amph2c"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_amph2c ( struct lxrcmd *c ) 
{

    mk_complex_from_amph ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_c2reim : command function for user command "c2reim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_c2reim ( struct lxrcmd *c ) 
{

    mk_reim_from_complex ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_c2amph : command function for user command "c2amph"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_c2amph ( struct lxrcmd *c ) 
{

    mk_amph_from_complex ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_clearall : command function for user command "clearall"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_clearall ( struct lxrcmd *c ) 
{

    clearall ( );   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_memory_rotcube : command function for user command "rotcube"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_memory_rotcube ( struct lxrcmd *c ) 
{

    rotate_cube ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}
