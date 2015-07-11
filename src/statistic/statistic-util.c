/*^****************************************************************************
* FILE: statistic-util.c : Module utility file
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
#include <statistic.h>     // Header for this module


/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
PF mod_statistic_putphnoise           ( struct lxrcmd *c ); 


/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_statistic_cmds[] = {

    {    "putphnoise",
         mod_statistic_putphnoise,
         "put photon noise",
         "str1 is input, str2 is output",
         "putphnoise in out",
         "%s %s %s",
         3,
         "put_poisson_noise",
    },
};




/*^-----------------------------------------------------------------------------
| PF
| mod_statistic_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       statistic.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_statistic_init ( struct module *m )					    
{										    
  int i;		    	    
  FILE *fp;  struct stat finfo;
  char str0[200];
  char str1[200];


  if( Debug > 1) fprintf(stdout, "[mod_statistic_init]\n");			    
  
  // Set module name
  sprintf(m->name,"statistic");
  // Set module info line
  sprintf(m->info,"statistical tools and functions");

  // Set number of commands for this module					    
  m->ncommands = sizeof(mod_statistic_cmds) / sizeof(struct lxrcmd) ;		    
  // Set command control block
  m->cmds = mod_statistic_cmds;
		 								    

  // set module-control-block index in every command-control-block 
  for( i=0; i<m->ncommands; ++i ) {
    mod_statistic_cmds[i].cdata_01 = m->module_number; 
  }

  // Set module compile time ascii entry
  sprintf(str0, "unknown");
  sprintf(str1, "%s/statistic.o", OBJDIR);
  if (!stat(str1, &finfo)) {
    sprintf(str0, "%s", asctime(localtime(&finfo.st_mtime)));}
  else { printf("ERROR: cannot find file %s\n",str1);}
  strncpy( m->buildtime, str0, strlen(str0)-1);
  m->buildtime[strlen(str0)] = '\0';
  
  return(PASS);
}






/*^-----------------------------------------------------------------------------
| PF
| mod_statistic_putphnoise : command function for user command "putphnoise"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_statistic_putphnoise ( struct lxrcmd *c ) 
{

    put_poisson_noise ( c->args[1].v.s, c->args[2].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}
