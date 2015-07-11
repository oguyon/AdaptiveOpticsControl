/*^****************************************************************************
* FILE: COREMOD_arith-util.c : Module utility file
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
#include <COREMOD_arith.h>     // Header for this module


/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
PF mod_COREMOD_arith_setpix           ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_setrow           ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_setcol           ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_zero           ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_extractim           ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_extractim3D         ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_transl           ( struct lxrcmd *c ); 
PF mod_COREMOD_arith_ea           ( struct lxrcmd *c ); 


/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_COREMOD_arith_cmds[] = {

    {    "setpix",
         mod_COREMOD_arith_setpix,
         "set pixel (l1xl2) to f1",
         "set pixel (l1xl2) to f1",
         "setpix im 0.0 256 256",
         "%s %s %f %ld %ld",
         5,
         "arith_set_pixel",
    },
    {    "setrow",
         mod_COREMOD_arith_setrow,
         "set pixel row (l1) to f1",
         "set pixel row (l1) to f1",
         "setrow im 0.0 256",
         "%s %s %f %ld",
         4,
         "arith_set_row",
    },
    {    "setcol",
         mod_COREMOD_arith_setcol,
         "set pixel col (l1) to f1",
         "set pixel col (l1) to f1",
         "setcol im 0.0 256",
         "%s %s %f %ld",
         4,
         "arith_set_col",
    },
    {    "zero",
         mod_COREMOD_arith_zero,
         "set all pixels to 0",
         "set all pixels to 0",
         "zero im",
         "%s %s",
         2,
         "arith_image_zero",
    },
    {    "extractim",
         mod_COREMOD_arith_extractim,
         "extract a subimage",
         "str1 is input, str2 is output, l1xl2 is size of subimage and l3xl4 is lower left corner of subimage",
         "extractim im out 512 512 256 256",
         "%s %s %s %ld %ld %ld %ld",
         7,
         "arith_image_extract2D",
    },
    {    "extractim3D",
         mod_COREMOD_arith_extractim3D,
         "extract a 3D subimage",
         "str1 is input, str2 is output, l1xl2xl3 is size of subimage and l4xl5xl6 is lower left corner of subimage",
         "extractim3D im out 512 512 10 256 256 0",
         "%s %s %s %ld %ld %ld %ld %ld %ld",
         9,
         "arith_image_extract3D",
    },
    {    "transl",
         mod_COREMOD_arith_transl,
         "translation",
         "str1 is input, str2 is output, translation is by f1 x f2 pixels",
         "transl in out 10.2 -4.2",
         "%s %s %s %f %f",
         5,
         "arith_image_translate",
    },
    {    "ea",
         mod_COREMOD_arith_ea,
         "execute an arithmetic command line",
         "execute an arithmetic command line",
         "ea a=1+2",
         "%s %s",
         2,
         "execute_arith",
    },
};




/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       COREMOD_arith.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_init ( struct module *m )					    
{										    
  int i;		    	    
  FILE *fp;  struct stat finfo;
  char str0[200];
  char str1[200];


   if( Debug > 1) fprintf(stdout, "[mod_COREMOD_arith_init]\n");

   // Set module name
   sprintf(m->name,"COREMOD_arith");
   // Set module info line
   sprintf(m->info,"Simple operations on images");

   // Set number of commands for this module					    
    m->ncommands = sizeof(mod_COREMOD_arith_cmds) / sizeof(struct lxrcmd) ;		        
    // Set command control block
    m->cmds = mod_COREMOD_arith_cmds;
		 								    

    // set module-control-block index in every command-control-block 
    for( i=0; i<m->ncommands; ++i ) {
        mod_COREMOD_arith_cmds[i].cdata_01 = m->module_number; 
    }

  // Set module compile time ascii entry
    sprintf(str0, "unknown");
    sprintf(str1, "%s/COREMOD_arith.o", OBJDIR);
    if (!stat(str1, &finfo)) {
      sprintf(str0, "%s", asctime(localtime(&finfo.st_mtime)));}
    else { printf("ERROR: cannot find file %s\n",str1);}
    strncpy( m->buildtime, str0, strlen(str0)-1);
    m->buildtime[strlen(str0)] = '\0';
    
    return(PASS);
}






/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_setpix : command function for user command "setpix"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_setpix ( struct lxrcmd *c ) 
{

    arith_set_pixel ( c->args[1].v.s, c->args[2].v.f, c->args[3].v.ld, c->args[4].v.ld);   

    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_setrow : command function for user command "setrow"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_setrow ( struct lxrcmd *c ) 
{

    arith_set_row ( c->args[1].v.s, c->args[2].v.f, c->args[3].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_setcol : command function for user command "setcol"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_setcol ( struct lxrcmd *c ) 
{

    arith_set_col ( c->args[1].v.s, c->args[2].v.f, c->args[3].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_zero : command function for user command "zero"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_zero ( struct lxrcmd *c ) 
{

    arith_image_zero ( c->args[1].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_extractim : command function for user command "extractim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_extractim ( struct lxrcmd *c ) 
{

    arith_image_extract2D ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.ld, c->args[4].v.ld, c->args[5].v.ld, c->args[6].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


PF mod_COREMOD_arith_extractim3D ( struct lxrcmd *c ) 
{

    arith_image_extract3D ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.ld, c->args[4].v.ld, c->args[5].v.ld, c->args[6].v.ld, c->args[7].v.ld, c->args[8].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_transl : command function for user command "transl"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_transl ( struct lxrcmd *c ) 
{

    arith_image_translate ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.f, c->args[4].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_arith_ea : command function for user command "ea"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_arith_ea ( struct lxrcmd *c ) 
{

    execute_arith ( c->args[1].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}
