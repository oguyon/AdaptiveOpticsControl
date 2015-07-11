/*^****************************************************************************
/ FILE: memory-util.h : Utility file for module: "memory"
/
/     This file must be named : Modulename-util.c
/
/ Properly named functions from this module are already declared. 
/ These are included in module-config.h included by module.h
/
/ See modules-config.h 
/
/ modules-config.h was generated at build time by module names 
/ in modules-included.list
/
******************************************************************************/

#include <Cfits.h>    // include the generic  module header
#include <memory.h>  // include the specific module header

//
// Forward references for the module glue functions, with naming convention: 
// modcmd_ModuleName_CommandName 
PF mod_memory_list_image_ID      ( struct lxrcmd *c ); 
PF mod_memory_list_variable_ID   ( struct lxrcmd *c ); 
PF mod_memory_create_2Dimage_ID ( struct lxrcmd *c );
PF mod_memory_delete_image_ID    ( struct lxrcmd *c );  
PF mod_memory_clearall           ( struct lxrcmd *c ); 

struct lxrcmd mod_memory_cmds[] = {

 {  "listim",                   //  comand key
    mod_memory_list_image_ID,   //  glue function
    "List images in memory",
    "listim", 
    "listim", 
    "%s",                    //  conversion codes 
    1,                       //  number of subroutine arguements + 1
 },

 {  "listvar",               
    mod_memory_list_variable_ID,  
    "List variables in memory",
    "lisvar", 
    "listvar", 
    "%s",                    
    1,                       
 },

 {  "creaim",                        
    mod_memory_create_2Dimage_ID,  
    "Create an image",
    "creaim image-name width height. Create a l1 x l2 image of name image-name.", 
    "creaim image-name Width X Height", 
    "%s %s %ld %ld",         
    4,                       

 },


 {  "delim", 
    mod_memory_delete_image_ID,
    "delim",
    "delim ImageName", 
    "delim ImageName",
    "%s %s",
    2  

 },

 {  "clearall", 
    mod_memory_clearall,
    "Clear all buffers.",
    "clearall", 
    "clearall",
    "%s",
    1  

 },



};

/*^-----------------------------------------------------------------------------
| PF 
| mod_memory_init    :  The initialization function for this module.
|                        name must be in the form : mod_ModuleName_init
|
|    struct module *m :  Pointer to this module's module-control-block.
|
|  
+-----------------------------------------------------------------------------*/
PF mod_memory_init ( struct module *m )
{


   if( Debug > 1) fprintf(stdout, "[mod_memory_init]\n");

    // Set number of commands for this module
    m->ncommands = sizeof(mod_memory_cmds) / sizeof(struct lxrcmd) ;

    // Set command control block
    m->cmds = mod_memory_cmds;


    strncpy( m->name, "memory", MD_MAX_MODULE_NAME_CHARS );

    return(PASS);
}



/*^-----------------------------------------------------------------------------
| PF 
| mod_memory_list_image_ID ( struct lxrcmd *c )  
| 
|
|    struct module *m :  Pointer to this module's module-control-block.
|
|
+-----------------------------------------------------------------------------*/
PF mod_memory_list_image_ID ( struct lxrcmd *c ) 
{


    list_image_ID();

    c->results = NULL;  // NULL results block. Change as appropriate


    return(PASS);
}



/*^-----------------------------------------------------------------------------
| 
| 
| 
|
|    struct module *m :  Pointer to this module's module-control-block.
|
|
+-----------------------------------------------------------------------------*/
PF mod_memory_list_variable_ID ( struct lxrcmd *c ) 
{

    list_variable_ID();



    c->results = NULL;  // NULL results block. Change as appropriate


    return(PASS);
}
/*^-----------------------------------------------------------------------------
| 
| 
| 
|
|    struct module *m :  Pointer to this module's module-control-block.
|
|
+-----------------------------------------------------------------------------*/
PF mod_memory_create_2Dimage_ID ( struct lxrcmd *c ) 
{


    create_2Dimage_ID ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld );

    c->results = NULL;   // NULL results block. Change as appropriate

    return(PASS);
}

/*^-----------------------------------------------------------------------------
| 
| 
| 
|
|    struct module *m :  Pointer to this module's module-control-block.
|
|
+-----------------------------------------------------------------------------*/
PF mod_memory_delete_image_ID  ( struct lxrcmd *c ) 
{

    delete_image_ID(c->args[1].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}

/*^-----------------------------------------------------------------------------
| 
| 
| 
|
|    struct module *m :  Pointer to this module's module-control-block.
|
|
+-----------------------------------------------------------------------------*/
PF mod_memory_clearall ( struct lxrcmd *c ) 
{

    clearall();


    c->results = NULL;  // NULL results block. Change as appropriate


    return(PASS);
}
