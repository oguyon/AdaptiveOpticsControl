/*^****************************************************************************
* FILE: kdtree-util.c : Module utility file
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
#include <kdtree.h>     // Header for this module




/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/


/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_kdtree_cmds[] = {
};




/*^-----------------------------------------------------------------------------
| PF
| mod_kdtree_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       ExampleModule.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_kdtree_init ( struct module *m )
{
  int i;		    	    
  FILE *fp;
  struct stat finfo;
  char str0[200];
  char str1[200];
  

  // Set module name
  sprintf( m->name, "kdtree");
  if( Debug > 1) fprintf(stdout, "[mod_%s_init]\n",m->name);
  
  // Set module info line
  sprintf( m->info, "kdtree module Copyright (C) 2007-2009 John Tsiombikas");
  
  
  // Set number of commands for this module
  m->ncommands = sizeof(mod_kdtree_cmds) / sizeof(struct lxrcmd) ;
  // Set command control block
  m->cmds = mod_kdtree_cmds;
  
  // Set module-control-block index in every command-control-block 
  for( i=0; i<m->ncommands; ++i ) {
    mod_kdtree_cmds[i].cdata_01 = m->module_number; 
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


