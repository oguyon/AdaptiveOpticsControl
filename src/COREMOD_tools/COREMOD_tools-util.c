/*^****************************************************************************
* FILE: COREMOD_tools-util.c : Module utility file
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
#include <COREMOD_tools.h>     // Header for this module


/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
PF mod_COREMOD_tools_mkcntfile           ( struct lxrcmd *c ); 
PF mod_COREMOD_tools_writef2file         ( struct lxrcmd *c ); 



/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_COREMOD_tools_cmds[] = {

  {    "mkcntfile",
       mod_COREMOD_tools_mkcntfile,
       "create a counter file",
       "str1 is the file name and l1 is the number of entries",
       "mkcntfile cnt.txt 2048",
       "%s %s %ld",
         3,
       "create_counter_file",
  },
  {    "writef2file",
       mod_COREMOD_tools_writef2file,
       "write float to file",
       "str1 is the file name and f1 is the float",
       "writef2file result.log 3.42",
       "%s %s %f",
       3,
       "write_float_file",
  },
  
};




/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_tools_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       COREMOD_tools.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_tools_init ( struct module *m )					    
{										    
  int i;		    	    
  FILE *fp;  struct stat finfo;
  char str0[200];
  char str1[200];


  if( Debug > 1) fprintf(stdout, "[mod_COREMOD_tools_init]\n");			    
  
  // Set module name
  sprintf(m->name,"COREMOD_tools");
  // Set module info line
  sprintf(m->info,"low level tools for Cfits");

  // Set number of commands for this module					    
  m->ncommands = sizeof(mod_COREMOD_tools_cmds) / sizeof(struct lxrcmd) ;		    
  // Set command control block
  m->cmds = mod_COREMOD_tools_cmds;
  
  // Set module-control-block index in every command-control-block 
  for( i=0; i<m->ncommands; ++i ) {
    mod_COREMOD_tools_cmds[i].cdata_01 = m->module_number; 
  }
  
  // Set module compile ascii entry
  sprintf(str0, "unknown");
  sprintf(str1, "%s/COREMOD_tools.o", OBJDIR);
  if (!stat(str1, &finfo)) {
    sprintf(str0, "%s", asctime(localtime(&finfo.st_mtime)));}
  else { printf("ERROR: cannot find file %s\n",str1);}
  strncpy( m->buildtime, str0, strlen(str0)-1);
  m->buildtime[strlen(str0)] = '\0';
  
  return(PASS);
}






/*^-----------------------------------------------------------------------------
| PF
| mod_COREMOD_tools_mkcntfile : command function for user command "mkcntfile"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_COREMOD_tools_mkcntfile ( struct lxrcmd *c ) 
{
    create_counter_file ( c->args[1].v.s, c->args[2].v.ld);   
    c->results = NULL;   // NULL results block. Change as appropriate
    return(PASS);
}


PF mod_COREMOD_tools_writef2file         ( struct lxrcmd *c )
{
  write_float_file ( c->args[1].v.s, c->args[2].v.f);
  c->results = NULL;   // NULL results block. Change as appropriate
  return(PASS);
}
