/*^****************************************************************************
* FILE: image_gen-util.c : Module utility file
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
#include <image_gen.h>     // Header for this module


/*
* Forward references for the module glue functions. 
* Naming convention: mod_ModuleName_CommandName 
*/
PF mod_image_gen_mkdoublestar           ( struct lxrcmd *c ); 
PF mod_image_gen_mkdisk           ( struct lxrcmd *c ); 
PF mod_den_image_mkspdisk           ( struct lxrcmd *c ); 
PF mod_image_gen_mksquare           ( struct lxrcmd *c ); 
PF mod_image_gen_mkrect           ( struct lxrcmd *c ); 
PF mod_image_gen_mkline           ( struct lxrcmd *c ); 
PF mod_image_gen_mkhex           ( struct lxrcmd *c ); 
PF mod_image_gen_mkhexsegpup ( struct lxrcmd *c );
PF mod_image_gen_mkjpup           ( struct lxrcmd *c ); 
PF mod_image_gen_mkgauss           ( struct lxrcmd *c ); 
PF mod_image_gen_mkegauss           ( struct lxrcmd *c ); 
PF mod_image_gen_mkrnd           ( struct lxrcmd *c ); 
PF mod_image_gen_mkgal           ( struct lxrcmd *c ); 
PF mod_image_gen_genimezdisk           ( struct lxrcmd *c ); 
PF mod_image_gen_mkslopeim           ( struct lxrcmd *c ); 
PF mod_image_gen_mkdist           ( struct lxrcmd *c ); 
PF mod_image_gen_mkposangle           ( struct lxrcmd *c ); 
PF mod_image_gen_mkpsffromprof           ( struct lxrcmd *c ); 
PF mod_image_gen_mkoffhypgauss           ( struct lxrcmd *c ); 
PF mod_image_gen_mkcosapopup             ( struct lxrcmd *c );
PF mod_image_gen_mk2dpixgrid           ( struct lxrcmd *c ); 
PF mod_image_gen_mktile           ( struct lxrcmd *c ); 


/*
* Command-control-blocks for this module
*/
struct lxrcmd mod_image_gen_cmds[] = {

    {    "mkdoublestar",
         mod_image_gen_mkdoublestar,
         "creates a double star",
         "str1 is the output image file (n1 x n2 pixels). f1 is intensity 1, f2 intensity 2, f3 separation (pixels), f4 position angle (rad)",
         "mkdoublestar im1 512 512 1.0 0.2 34.5 3.45",
         "%s %s %d %d %f %f %f %f",
         8,
         "make_double_star",
    },
    {    "mkdisk",
         mod_image_gen_mkdisk,
         "creates a disk",
         "str1 is the output image file (n1 x n2 pixels). center of the disk is (f1,f2) and radius is f3.",
         "mkdisk disk 512 512 256.0 256.0 50.34",
         "%s %s %d %d %f %f %f",
         7,
         "make_disk",
    },
    {    "mkspdisk",
         mod_den_image_mkspdisk,
         "creates a disk with subpixel precision",
         "str1 is the output image file (n1 x n2 pixels). center of the disk is (f1,f2) and radius is f3.",
         "mkspdisk disk 512 512 256.0 256.0 50.34",
         "%s %s %d %d %f %f %f",
         7,
         "make_subpixdisk",
    },
    {    "mksquare",
         mod_image_gen_mksquare,
         "creates a square",
         "str1 is the output image file (l1xl2 pixels). center of the square is (f1,f2) and radius is f3.",
         "mksquare sq 512 512 256 256 50",
         "%s %s %ld %ld %f %f %f",
         7,
         "make_square",
    },
    {    "mkrect",
         mod_image_gen_mkrect,
         "creates a rectangle",
         "str1 is the output image file (l1xl2 pixels). center of the rectangle is (f1,f2) and size is f3xf4.",
         "mkrect rect 512 512 256 256 50 60",
         "%s %s %ld %ld %f %f %f %f",
         8,
         "make_rectangle",
    },
    {    "mkline",
         mod_image_gen_mkline,
         "creates a line",
         "str1 is the output image file (l1xl2 pixels). first point is (f1,f2), second point is (f3,f4) and thickness is f5.",
         "mkline line 512 512 256 256 300 350 5.0",
         "%s %s %ld %ld %f %f %f %f %f",
         9,
         "make_line",
    },
    {    "mkhex",
         mod_image_gen_mkhex,
         "creates an hexagon",
         "str1 is the output image file (l1xl2 pixels). center of the hexagon is (f1,f2) and size is f3",
         "mkhex hexim 512 512 256 256 50.0",
         "%s %s %ld %ld %f %f %f",
         7,
         "make_hexagon",
    },
    {     "mkhexsegpup",
	  mod_image_gen_mkhexsegpup,
	  "creates a hexagonal segmented pupil",
	  "str1 is the output image file (size l1). f1 is pupil radius, f2 is gap, f3 is seg size",
	  "mkhexsegpup hexpup 512 200.0 2.0 45.0",
	  "%s %s %ld %f %f %f",
	  6,
	  "make_hexsegpupil",
    },
    {    "mkjpup",
         mod_image_gen_mkjpup,
         "creates a Jacquinot pupil",
         "str1 is the output image file (l1xl2 pixels). center is (f1,f2) and size is f3xf4.",
         "mkjpup jpup 512 512 256 256 50 60",
         "%s %s %ld %ld %f %f %f %f",
         8,
         "make_jacquinot_pupil",
    },
    {    "mkgauss",
         mod_image_gen_mkgauss,
         "creates a gaussian",
         "str1 is the output image file (n1 x n2 pixels). A*exp(-r*r/a/a). FWHM : ln(2)=r*r/a/a : r*r = a*a*ln(2) : r = a*sqrt(ln(2))",
         "mkgauss gaussim 512 512 10 1.0",
         "%s %s %d %d %f %f",
         6,
         "make_gauss",
    },
    {    "mkegauss",
         mod_image_gen_mkegauss,
         "creates a 2 axis gaussian",
         "str1 is the output image file (n1 x n2 pixels). A*exp(-r*r/a/a). FWHM along long axis: ln(2)=r*r/a/a : r*r = a*a*ln(2) : r = a*sqrt(ln(2)). f3 is the elongation (0=round) f4 is the PA.",
         "mkegauss gaussim 512 512 10 1.0 2.3 180",
         "%s %s %d %d %f %f %f %f",
         8,
         "make_2axis_gauss",
    },
    {    "mkrnd",
         mod_image_gen_mkrnd,
         "creates a noise image",
         "options are (-gauss), (-trgauss). default is uniform distribution.",
         "mkrnd im1 512 512 -gauss",
         "%s %s %d %d",
         4,
         "make_rnd",
    },
    {    "mkgal",
         mod_image_gen_mkgal,
         "create a galaxy",
         "str1 is output of size (l1xl2), f1 is spiral radius, f2 is spiral central luminosity, f3 is spiral elongation (a/b), f4 is spiral PA, f5 is elliptical radius, f6 is elliptical lum, f7 is ell elongation, f8 is ell PA.",
         "mkgal gal 512 512 100.0 1.0 0.0 0.0 100.0 0.0 0.0 0.0",
         "%s %s %ld %ld %f %f %f %f %f %f %f %f",
         12,
         "make_galaxy",
    },
    {    "genimezdisk",
         mod_image_gen_genimezdisk,
         "make image of EZ disk",
         "str1 is output image name, l1 is size, f1 is inner edge in pixel, f2 is index (2.4 for sol), f3 is inclination (rad) ",
         "genimezdisk ezdisk 512 2.0 2.4 1.0",
         "%s %s %ld %f %f %f",
         6,
         "gen_image_EZdisk",
    },
    {    "mkslopeim",
         mod_image_gen_mkslopeim,
         "create a slope image",
         "output is image str1, l1xl2 pixels, with slope f1 on x and f2 on y",
         "mkslopeim out 1024 1024 0.2 -0.3",
         "%s %s %ld %ld %f %f",
         6,
         "make_slopexy",
    },
    {    "mkdist",
         mod_image_gen_mkdist,
         "create an image in which pixels values are distances to point (f1 x f2)",
         "create an image in which pixels values are distances to point (f1 x f2)",
         "mkdist out 1024 1024 512 512",
         "%s %s %ld %ld %f %f",
         6,
         "make_dist",
    },
    {    "mkposangle",
         mod_image_gen_mkposangle,
         "create an image in which pixels values are position angle to point (f1 x f2)",
         "create an image in which pixels values are position angle to point (f1 x f2)",
         "mkposangle out 1024 1024 512 512",
         "%s %s %ld %ld %f %f",
         6,
         "make_PosAngle",
    },
    {    "mkpsffromprof",
         mod_image_gen_mkpsffromprof,
         "creates a PSF from a radial profile",
         "str1 is the radial profile, str2 the output image, l1xl2 the size of the image",
         "mkpsffromprof prof out 512 512",
         "%s %s %s %ld %ld",
         5,
         "make_psf_from_profile",
    },
    {    "mkoffhypgauss",
         mod_image_gen_mkoffhypgauss,
         "make 2D offset hypergaussian ",
         "l1 is image size, f1,f2 are parameters a and b, l2 is parameter n, str1 is output",
         "mkoffhypgauss 1024 100 100 6 out",
         "%s %ld %f %f %ld %s",
         6,
         "make_offsetHyperGaussian",
    },
    {    "mkcosapopup",
         mod_image_gen_mkcosapopup,
         "make 2D cosine apodized pupil",
         "l1 is image size, f1,f2 are parameters a and b, str1 is output",
         "mkcosapopup 1024 100 100 out",
         "%s %ld %f %f %s",
         5,
         "make_cosapoedgePupil",
    },    
   {    "mk2dpixgrid",
         mod_image_gen_mk2dpixgrid,
         "make 2D grid of pixels",
         "str1 is output image, l1 is size, f1 f2 are pitch in x and y, f3 f4 are offset in x and y",
         "mk2dpixgrid grid 1024 10.0 10.0 0.0 0.0",
         "%s %s %ld %f %f %f %f",
         7,
         "make_2Dgridpix",
    },
    {    "mktile",
         mod_image_gen_mktile,
         "repeat image in rectangular tiling",
         "str1 in input, l1 is output image size, str2 is output",
         "mktile im1 1024 im2",
         "%s %s %ld %s",
         4,
         "make_tile",
    },
};




/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_init        :  The initialization function for this module.
|                       Naming convention: mod_ModuleName_init
|                       This function is declared in the include file:
|                       image_gen.h  included via Cfits.h which was generated at build 
|                       time from the file: modules-included.list
|
|   struct module *m : This module's module-control-block
|
|
+-----------------------------------------------------------------------------*/
PF mod_image_gen_init ( struct module *m )					    
{										    
  int i;		    	    
  FILE *fp;  struct stat finfo;
  char str0[200];
  char str1[200];


   if( Debug > 1) fprintf(stdout, "[mod_image_gen_init]\n");			    
	
   // Set module name
   sprintf(m->name,"image_gen");
   // Set module info line
   sprintf(m->info,"generates images (disks, star clusters etc...)");
									    
   // Set number of commands for this module					    
   m->ncommands = sizeof(mod_image_gen_cmds) / sizeof(struct lxrcmd) ;		    
   // Set command control block
   m->cmds = mod_image_gen_cmds;
		 								    

    // set module-control-block index in every command-control-block 
    for( i=0; i<m->ncommands; ++i ) {
        mod_image_gen_cmds[i].cdata_01 = m->module_number; 
    }

// Set module compile time ascii entry
   sprintf(str0, "unknown");
   sprintf(str1, "%s/image_gen.o", OBJDIR);
if (!stat(str1, &finfo)) {
   sprintf(str0, "%s", asctime(localtime(&finfo.st_mtime)));}
else { printf("ERROR: cannot find file %s\n",str1);}
   strncpy( m->buildtime, str0, strlen(str0)-1);
   m->buildtime[strlen(str0)] = '\0';

    return(PASS);
}






/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkdoublestar : command function for user command "mkdoublestar"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkdoublestar ( struct lxrcmd *c ) 
{
  
  make_double_star ( c->args[1].v.s, c->args[2].v.d, c->args[3].v.d, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f, c->args[7].v.f);   

  c->results = NULL;   // NULL results block. Change as appropriate


  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkdisk : command function for user command "mkdisk"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkdisk ( struct lxrcmd *c ) 
{
  make_disk ( c->args[1].v.s, c->args[2].v.d, c->args[3].v.d, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f);   

  c->results = NULL;   // NULL results block. Change as appropriate
  
  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_den_image_mkspdisk : command function for user command "mkspdisk"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_den_image_mkspdisk ( struct lxrcmd *c ) 
{

    make_subpixdisk ( c->args[1].v.s, c->args[2].v.d, c->args[3].v.d, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mksquare : command function for user command "mksquare"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mksquare ( struct lxrcmd *c ) 
{

    make_square ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkrect : command function for user command "mkrect"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkrect ( struct lxrcmd *c ) 
{

    make_rectangle ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f, c->args[7].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}

PF mod_image_gen_mkline ( struct lxrcmd *c ) 
{
  make_line ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f, c->args[7].v.f, c->args[8].v.f);   
  
  c->results = NULL;   // NULL results block. Change as appropriate
  return(PASS);
}



PF mod_image_gen_mkhex ( struct lxrcmd *c ) 
{
    make_hexagon ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f);   
    c->results = NULL;   // NULL results block. Change as appropriate

    return(PASS);
}


PF mod_image_gen_mkhexsegpup ( struct lxrcmd *c ) 
{
  make_hexsegpupil( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.f, c->args[4].v.f, c->args[5].v.f);   
  c->results = NULL;   // NULL results block. Change as appropriate
  
  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkjpup : command function for user command "mkjpup"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkjpup ( struct lxrcmd *c ) 
{

    make_jacquinot_pupil ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f, c->args[7].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkgauss : command function for user command "mkgauss"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkgauss ( struct lxrcmd *c ) 
{

    make_gauss ( c->args[1].v.s, c->args[2].v.d, c->args[3].v.d, c->args[4].v.f, c->args[5].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkegauss : command function for user command "mkegauss"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkegauss ( struct lxrcmd *c ) 
{

    make_2axis_gauss ( c->args[1].v.s, c->args[2].v.d, c->args[3].v.d, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f, c->args[7].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkrnd : command function for user command "mkrnd"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkrnd ( struct lxrcmd *c ) 
{

    make_rnd ( c->args[1].v.s, c->args[2].v.d, c->args[3].v.d, c->options);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkgal : command function for user command "mkgal"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkgal ( struct lxrcmd *c ) 
{

    make_galaxy ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f, c->args[7].v.f, c->args[8].v.f, c->args[9].v.f, c->args[10].v.f, c->args[11].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_genimezdisk : command function for user command "genimezdisk"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_genimezdisk ( struct lxrcmd *c ) 
{

    gen_image_EZdisk ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.f, c->args[4].v.f, c->args[5].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkslopeim : command function for user command "mkslopeim"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkslopeim ( struct lxrcmd *c ) 
{

    make_slopexy ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkdist : command function for user command "mkdist"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkdist ( struct lxrcmd *c ) 
{

    make_dist ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


PF mod_image_gen_mkposangle ( struct lxrcmd *c ) 
{

  make_PosAngle ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.ld, c->args[4].v.f, c->args[5].v.f);   
  c->results = NULL;   // NULL results block. Change as appropriate

  return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkpsffromprof : command function for user command "mkpsffromprof"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkpsffromprof ( struct lxrcmd *c ) 
{

    make_psf_from_profile ( c->args[1].v.s, c->args[2].v.s, c->args[3].v.ld, c->args[4].v.ld);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkoffhypgauss : command function for user command "mkoffhypgauss"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkoffhypgauss ( struct lxrcmd *c ) 
{

    make_offsetHyperGaussian ( c->args[1].v.ld, c->args[2].v.f, c->args[3].v.f, c->args[4].v.ld, c->args[5].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}



/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mkcosapopup : command function for user command "mkcosapopup"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mkcosapopup ( struct lxrcmd *c ) 
{

    make_cosapoedgePupil ( c->args[1].v.ld, c->args[2].v.f, c->args[3].v.f, c->args[4].v.s );   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}

 

/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mk2dpixgrid : command function for user command "mk2dpixgrid"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mk2dpixgrid ( struct lxrcmd *c ) 
{

    make_2Dgridpix ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.f, c->args[4].v.f, c->args[5].v.f, c->args[6].v.f);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}


/*^-----------------------------------------------------------------------------
| PF
| mod_image_gen_mktile : command function for user command "mktile"
|   struct lxrcmd *c: Command-control-block calling this command function.
|
|
|
| Returns : PASS/FAIL
+-----------------------------------------------------------------------------*/
PF mod_image_gen_mktile ( struct lxrcmd *c ) 
{

    make_tile ( c->args[1].v.s, c->args[2].v.ld, c->args[3].v.s);   


    c->results = NULL;   // NULL results block. Change as appropriate


    return(PASS);
}
