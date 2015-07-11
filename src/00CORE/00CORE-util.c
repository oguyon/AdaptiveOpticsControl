/*^****************************************************************************
* FILE: 00CORE-util.c : Module utility file
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

#include <00CORE.h>     // Header for this module



/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
int cfits_cmd_system ( struct lxrcmd *c );
PF cfits_cmd_help ( struct lxrcmd *c );
PF cfits_cmd_helpreadline ( struct lxrcmd *c );
int cfits_set_default_precision ( struct lxrcmd *c );
PF cfits_cmd_Cfitsinfo ( struct lxrcmd *c );
PF cfits_cmd_exit    ( struct lxrcmd *cmd );
PF cfits_cmd_writepid ( struct lxrcmd *cmd );

/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_00CORE_cmds[] = {

  { "!",
    cfits_cmd_system,
    "Run unix system command",
    "! \"system command string\" ",
    "! ls : list contents of current directory ",
    "%s %s",
    2,
  },
  { "help",
    cfits_cmd_help,
    "Get help",
    "help [cmd|symbol|topic]",
    "help commands : Get list of all commands. ",
    "%s %s",
    2,
  },
    { "helpreadline",
      cfits_cmd_helpreadline,
      "readline quick help",
      "helpreadline",
      "readline quick help",
      "%s",
      1,
    },
    { "setdp",
      cfits_set_default_precision,
      "Set default precision",
      "setdp 1",
      "0: float, 1: double",
      "%s %d",
      2,
    },
    { "ci",
      cfits_cmd_Cfitsinfo,
      "Cfits info",
      "ci",
      "ci : show Cfits compilation time, memory usage ",
      "%s",
      1,
    },
  { "writepid",
    cfits_cmd_writepid,
    "Writes PID to file",
    "writepid",
    "writepid : show Cfits compilation time, memory usage ",
    "%s",
    1,
    },
      { "quit",
      cfits_cmd_exit,
      "Exit program.",
      "exit",
      "exit",
      "%s",
      1,
    },
    { "exit",
      cfits_cmd_exit,
      "Exit program.",
      "exit",
      "exit",
      "%s",
      1,
    },
};




/*^-----------------------------------------------------------------------------
| PF
| mod_00CORE_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       00CORE.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_00CORE_init ( struct module *m )
{
  int i;		    	    
  FILE *fp;
  struct stat finfo;
  char str0[200];
  char str1[200];
  

  // Set module name
  sprintf( m->name, "00CORE");
  if( Debug > 1) fprintf(stdout, "[mod_%s_init]\n",m->name);
  
  // Set module info line
  sprintf( m->info, "Core standard functions module");
  
  
  // Set number of commands for this module
  m->ncommands = sizeof(mod_00CORE_cmds) / sizeof(struct lxrcmd) ;
  // Set command control block
  m->cmds = mod_00CORE_cmds;
  
  // Set module-control-block index in every command-control-block 
  for( i=0; i<m->ncommands; ++i ) {
    mod_00CORE_cmds[i].cdata_01 = m->module_number; 
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




int cfits_cmd_system ( struct lxrcmd *c )
{
  int rc;

  if( Debug )   fprintf(stdout, "[cfits_cmd_system]\n");
  if( Verbose ) fprintf(stdout, "System command: [%s]", 
			c->args[1].v.s);  
  // make system call
  rc = system( c->args[1].v.s );
  
  if( Verbose ) 
    {
      fprintf(stdout, "Result:%d", rc );  
      fprintf(stdout, "\n");
    }
  
  return(rc);
}


PF cfits_cmd_help ( struct lxrcmd *c )
{
  int rc;
  char command[100];
  
  sprintf(command,"more %s/help.txt",CfitsDocDir);
  rc = system(command);

  return(rc);
}

PF cfits_cmd_helpreadline ( struct lxrcmd *c )
{
  int rc;
  char command[100];
  
  sprintf(command,"more %s/helpreadline.txt",CfitsDocDir);
  rc = system(command);

  return(rc);
}

PF cfits_set_default_precision ( struct lxrcmd *c )
{
  int vp;

  vp = c->args[1].v.d;

  setCfits_precision(vp);
  
  return(PASS);
}

PF cfits_cmd_Cfitsinfo ( struct lxrcmd *c )
{

  CfitsinfoPrint();

  return(PASS);
}

PF cfits_cmd_writepid ( struct lxrcmd *c )
{
  CfitsWritePid();
  return(PASS);
}


PF cfits_cmd_exit ( struct lxrcmd *cmd )
{
  if( Debug ) fprintf(stdout, "[cfits_cmd_exit]\n" ); 
  
  fprintf(stdout, "%c[%d;%dmGoodbye%c[%dm\n",0x1B, 1, 36, 0x1B, 0);
  
  exit(0);
  
  return(0);
}
