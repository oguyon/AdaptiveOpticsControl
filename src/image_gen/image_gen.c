#include <fitsio.h>  /* required by every program that uses CFITSIO  */
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_arith/COREMOD_arith.h"

#include "statistic/statistic.h"

#include "image_gen/image_gen.h"

#define OMP_NELEMENT_LIMIT 1000000

#define SWAP(x,y)  tmp=(x);x=(y);y=tmp;

#define PI 3.14159265358979323846264338328

extern DATA data;



// CLI functions
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//

int make_disk_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)==0)
    make_disk(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf,  data.cmdargtoken[6].val.numf);
  else
    return 1;
  return 0;
}


int make_subpixdisk_cli()
{
 
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)==0)
    make_subpixdisk(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf,  data.cmdargtoken[6].val.numf);
 else
    return 1;
  return 0;
}



int make_gauss_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)==0)
    make_gauss(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf);
  else
    return 1;
  return 0;
}

int make_slopexy_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)==0)
    make_slopexy(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf);
    else
    return 1;
  return 0;
}

int make_dist_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)==0)
    make_dist(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf);
  else
    return 1;

  return 0;
}


int make_hexsegpupil_cli()
{

  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,1)==0)
    {
      make_hexsegpupil(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf);
      return 0;
    }
  else
    return 1;
}

int make_rectangle_cli()
{
  
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)+CLI_checkarg(7,1)==0)
    {
      make_rectangle(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf, data.cmdargtoken[7].val.numf);
      return 0;
    }
  else
    return 1;  
}

int make_line_cli()
{
  
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)+CLI_checkarg(7,1)+CLI_checkarg(8,1)==0)
    {
       make_line(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf, data.cmdargtoken[7].val.numf, data.cmdargtoken[8].val.numf);
       return 0;
    }
  else
    return 1;
}

int make_2Dgridpix_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,1)+CLI_checkarg(7,1)==0)
    {
      make_2Dgridpix(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numf, data.cmdargtoken[7].val.numf);
      return 0;
    }
  else
    return 1;
}



int make_rnd_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,2)+CLI_checkarg(3,2)==0)
    {
		make_rnd(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, ""); 
      return 0;
    }
  else
    return 1;
}


//long make_rnd(char *ID_name, long l1, long l2, char *options)





int init_image_gen()
{
  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "creating images (shapes, useful functions and patterns)");
  data.NBmodule++;

  
  strcpy(data.cmd[data.NBcmd].key,"mkdisk");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_disk_cli;
  strcpy(data.cmd[data.NBcmd].info,"make disk image");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <yize> <xcenter> <ycenter> <radius>");
  strcpy(data.cmd[data.NBcmd].example,"mkdisk imdisk 512 512 256.0 256.0 100.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_disk(char *ID_name, long l1, long l2, double x_center, double y_center, double radius)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"mkspdisk");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_subpixdisk_cli;
  strcpy(data.cmd[data.NBcmd].info,"make disk image with sub-pixel interpolation");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <yize> <xcenter> <ycenter> <radius>");
  strcpy(data.cmd[data.NBcmd].example,"mkspdisk imdisk 512 512 256.0 256.0 100.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_subpixdisk(char *ID_name, long l1, long l2, double x_center, double y_center, double radius)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"mkgauss");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_gauss_cli;
  strcpy(data.cmd[data.NBcmd].info,"make gaussian spot image, A*exp(-r*r/a/a). FWHM : ln(2)=r*r/a/a : r*r = a*a*ln(2) : r = a*sqrt(ln(2))");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <yize> <a> <A>");
  strcpy(data.cmd[data.NBcmd].example,"mkgauss imdisk 512 512 12.0 1.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_gauss(char *ID_name, long l1, long l2, double a, double A)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"mkslopexy");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_slopexy_cli;
  strcpy(data.cmd[data.NBcmd].info,"make slope image");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <yize> <slopex> <slopey>");
  strcpy(data.cmd[data.NBcmd].example,"mkslope im 512 512 1.2 1.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_slopexy(char *ID_name, long l1,long l2, double sx, double sy)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"mkdist");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_dist_cli;
  strcpy(data.cmd[data.NBcmd].info,"make image, pixel value = distance from point in image");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <yize> <centerx> <centery>");
  strcpy(data.cmd[data.NBcmd].example,"mkdist im 512 512 256.0 256.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_dist(char *ID_name, long l1,long l2, double f1, double f2)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"mkhexsegpup");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_hexsegpupil_cli;
  strcpy(data.cmd[data.NBcmd].info,"make hexagonal segmented pupil");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <size> <radius> <gap> <step>");
  strcpy(data.cmd[data.NBcmd].example,"mkhexsegpup imhex 4096 200 2.0 46.3");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_hexsegpupil(char *IDname, long size, double radius, double gap, double step)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"mkrect");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_rectangle_cli;
  strcpy(data.cmd[data.NBcmd].info,"make rectangle");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <ysize> <xcenter> <ycenter> <radius1> <radius2>");
  strcpy(data.cmd[data.NBcmd].example,"mkrect 512 512 256.0 256.0 100.0 200.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_rectangle(char *ID_name, long l1, long l2, double x_center, double y_center, double radius1, double radius2)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"mkline");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_line_cli;
  strcpy(data.cmd[data.NBcmd].info,"make line");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <ysize> <x1> <y1> <x2> <y2> <thickness>");
  strcpy(data.cmd[data.NBcmd].example,"mkline 512 512 256.0 256.0 100.0 200.0 3.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_line(char *IDname, long l1, long l2, double x1, double y1, double x2, double y2, double t)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"mkgridpix");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_2Dgridpix_cli;
  strcpy(data.cmd[data.NBcmd].info,"make regular grid");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image name> <xsize> <ysize> <xpitch> <ypitch> <xoffset> <yoffset>");
  strcpy(data.cmd[data.NBcmd].example,"mkgridpix 512 512 10.0 10.0 4.5 2.8");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_2Dgridpix(char *IDname, long xsize, long ysize, double pitchx, double pitchy, double offsetx, double offsety)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"mkrndim");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = make_rnd_cli;
  strcpy(data.cmd[data.NBcmd].info,"make random image");
  strcpy(data.cmd[data.NBcmd].syntax,"<name> <xsize> <ysize>");
  strcpy(data.cmd[data.NBcmd].example,"mkrndim 512 512");
  strcpy(data.cmd[data.NBcmd].Ccall,"long make_rnd(char *ID_name, long l1, long l2, char *options)");
  data.NBcmd++;

//long make_rnd(char *ID_name, long l1, long l2, char *options)

 // add atexit functions here

  return 0;

}







long make_double_star(char *ID_name, long l1, long l2, double intensity_1, double intensity_2, double separation, double position_angle) /* creates a double star */
{
  long ID;
  long naxes[2];

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  data.image[ID].array.F[((int) (naxes[1]/2))*naxes[0]+((int) (naxes[0]/2))] = intensity_1;
  data.image[ID].array.F[((int) (naxes[1]/2+separation*cos(position_angle)))*naxes[0]+((int) (naxes[0]/2+separation*sin(position_angle)))] = intensity_2;
  
  return(ID);
}

long make_disk(char *ID_name, long l1, long l2, double x_center, double y_center, double radius) /* creates a disk */
{
  long ID;
  long ii,jj;
  long naxes[2];
  long x1,x2,y1,y2;
  long x1i,x2i,y1i,y2i;
  double r2;
  /*
    int i,j;
    double r;
    double tot;
    int subgrid=100;
    double x,y;
  */

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 

  x1 = (long) (x_center-radius-2);
  x2 = (long) (x_center+radius+2);
  y1 = (long) (y_center-radius-2);
  y2 = (long) (y_center+radius+2);
  x1i = (long) (x_center-0.707106781*radius+2);
  x2i = (long) (x_center+0.707106781*radius-2);
  y1i = (long) (y_center-0.707106781*radius+2);
  y2i = (long) (y_center+0.707106781*radius-2);

  if(x1<0)
    x1 = 0;
  if(x1>naxes[0])
    x1 = naxes[0];

  if(x2<0)
    x2 = 0;
  if(x2>naxes[0])
    x2 = naxes[0];
  
  if(y1<0)
    y1 = 0;
  if(y1>naxes[1])
    y1 = naxes[1];

  if(y2>naxes[1])
    y2 = naxes[1];
  
  if(x1i<0)
    x1i = 0;
  if(x1i>naxes[0])
    x1i = naxes[0];

  if(x2i<0)
    x2i = 0;
  if(x2i>naxes[0])
    x2i = naxes[0];

  if(y1i<0)
    y1i = 0;
  if(y1i>naxes[1])
    y1i = naxes[1];

  if(y2i<0)
    y2i = 0;
  if(y2i>naxes[1])
    y2i = naxes[1];
  
  r2 = radius*radius;

  for (ii = x1i; ii < x2i; ii++) 
    for (jj = y1i; jj < y2i; jj++)
      data.image[ID].array.F[jj*naxes[0]+ii] = 1;

  for (ii = x1; ii < x1i; ii++) 
    for (jj = y1; jj < y2; jj++)
      if (((ii-x_center)*(ii-x_center)+(jj-y_center)*(jj-y_center))<r2)
	  data.image[ID].array.F[jj*naxes[0]+ii] = 1;

  for (ii = x2i; ii < x2; ii++) 
    for (jj = y1; jj < y2; jj++)
      if (((ii-x_center)*(ii-x_center)+(jj-y_center)*(jj-y_center))<r2)
	  data.image[ID].array.F[jj*naxes[0]+ii] = 1;

  for (ii = x1i; ii < x2i; ii++) 
    for (jj = y1; jj < y1i; jj++)
      if (((ii-x_center)*(ii-x_center)+(jj-y_center)*(jj-y_center))<r2)
	  data.image[ID].array.F[jj*naxes[0]+ii] = 1;

  for (ii = x1i; ii < x2i; ii++) 
    for (jj = y2i; jj < y2; jj++)
      if (((ii-x_center)*(ii-x_center)+(jj-y_center)*(jj-y_center))<r2)
	  data.image[ID].array.F[jj*naxes[0]+ii] = 1;


  /*
  for (jj = x1; jj < x2; jj++) 
    for (ii = y1; ii < y2; ii++)
      { 
	if (((ii-x_center)*(ii-x_center)+(jj-y_center)*(jj-y_center))<r2)
	  data.image[ID].array.F[jj*naxes[0]+ii] = 1;
      }  
  */
  /*
    for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++) 
    {
    r = sqrt(((ii-x_center)*(ii-x_center)+(jj-y_center)*(jj-y_center)));
    
    if (r<radius)
    data.image[ID].array.F[jj*naxes[0]+ii] = 1.0; 
    else
    data.image[ID].array.F[jj*naxes[0]+ii] = 0.0; 
    
    if(((radius-r)*(radius-r))<1.5)
    {
    tot = 0;
    for (j = 0; j < subgrid; j++) 
    for (i = 0; i < subgrid; i++) 
    {
    x = 1.0*ii-0.5+0.5/subgrid+1.0*i/subgrid;
    y = 1.0*jj-0.5+0.5/subgrid+1.0*j/subgrid;
    r = sqrt((x-1.0*x_center)*(x-1.0*x_center)+(y-1.0*y_center)*(y-1.0*y_center));
    if (r < radius)
    tot = tot + 1.0;
    else
    tot = tot + 0.0;
    }
    tot = tot/subgrid/subgrid;
    data.image[ID].array.F[jj*naxes[0]+ii] = tot;
    }
    }
  */
  return(ID);
}

long make_subpixdisk(char *ID_name, long l1, long l2, double x_center, double y_center, double radius) // creates a disk
{
    long ID;
    long ii,jj;
    long naxes[2];
    int i,j;
    double r;
    double tot;
    int subgrid=55;
    double grid[55]; // same number of points as subgrid
    double x,y;
    long x1,x2,y1,y2;
    long x1i,x2i,y1i,y2i;
    double r2,r2ref;
    double xdiff,ydiff;
    double subgrid2;

    create_2Dimage_ID(ID_name,l1,l2);
    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    x1 = (long) (x_center-radius-2);
    x2 = (long) (x_center+radius+2);
    y1 = (long) (y_center-radius-2);
    y2 = (long) (y_center+radius+2);
    x1i = (long) (x_center-0.707106781*radius+2);
    x2i = (long) (x_center+0.707106781*radius-2);
    y1i = (long) (y_center-0.707106781*radius+2);
    y2i = (long) (y_center+0.707106781*radius-2);


    if(x1<0)
        x1 = 0;
    if(x1>naxes[0])
        x1 = naxes[0];
    if(x2<0)
        x2 = 0;
    if(x2>naxes[0])
        x2 = naxes[0];

    if(y1<0)
        y1 = 0;
    if(y1>naxes[1])
        y1 = naxes[1];
    if(y2<0)
        y2 = 0;
    if(y2>naxes[1])
        y2 = naxes[1];

    if(x1i<0)
        x1i = 0;
    if(x1i>naxes[0]-1)
        x1i = naxes[0]-1;
    if(x2i<0)
        x2i = 0;
    if(x2i>naxes[0]-1)
        x2i = naxes[0]-1;

    if(y1i<0)
        y1i = 0;
    if(y1i>naxes[1]-1)
        y1i = naxes[1]-1;
    if(y2i<0)
        y2i = 0;
    if(y2i>naxes[1]-1)
        y2i = naxes[1]-1;


    r2ref = radius*radius;
    subgrid2 = subgrid*subgrid;


    for (ii = x1i; ii < x2i; ii++)
        for (jj = y1i; jj < y2i; jj++)
            data.image[ID].array.F[jj*naxes[0]+ii] = 1;

    for (i = 0; i < subgrid; i++)
        grid[i] = (0.5-0.5/subgrid-1.0*i/subgrid);

    for (ii = x1; ii < x1i; ii++)
        for (jj = y1; jj < y2; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            r2 = xdiff*xdiff+ydiff*ydiff;
            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        r = x*x+y*y;
                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }


    for (ii = x2i; ii < x2; ii++)
        for (jj = y1; jj < y2; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            r2 = xdiff*xdiff+ydiff*ydiff;
            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        r = x*x+y*y;
                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }

    for (ii = x1i; ii < x2i; ii++)
        for (jj = y1; jj < y1i; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            r2 = xdiff*xdiff+ydiff*ydiff;
            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        r = x*x+y*y;
                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }


    for (ii = x1i; ii < x2i; ii++)
        for (jj = y2i; jj < y2; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            r2 = xdiff*xdiff+ydiff*ydiff;
            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        r = x*x+y*y;
                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }

    return(ID);
}


// creates a shape with contour described by sum of sine waves
//
// r = radius + SUM[ ra[i] * cos( ka[i]*PA/2.0/PI + pa[i]) ]

long make_subpixdisk_perturb(char *ID_name, long l1, long l2, double x_center, double y_center, double radius, long n, double *ra, double *ka, double *pa)
{
    long ID;
    long ii,jj;
    long naxes[2];
    int i,j;
    double r;
    double tot;
    int subgrid=55;
    double grid[55]; // same number of points as subgrid
    double x,y;
    long x1,x2,y1,y2;
    long x1i,x2i,y1i,y2i;
    double r2,r2ref;
    double xdiff,ydiff;
    double subgrid2;
    double PA;
    double v0;
    long k;

    double radius1, radius2;

    radius1 = radius;
    radius2 = radius;
    for(k=0; k<n; k++)
        radius1 += radius*fabs(ra[k]);
    for(k=0; k<n; k++)
        radius2 -= radius*fabs(ra[k]);
    if(radius2<0.0)
        radius2 = 0.0;

    create_2Dimage_ID(ID_name,l1,l2);
    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    x1 = (long) (x_center-radius1-2);
    x2 = (long) (x_center+radius1+2);
    y1 = (long) (y_center-radius1-2);
    y2 = (long) (y_center+radius1+2);
    x1i = (long) (x_center-0.707106781*radius2+2);
    x2i = (long) (x_center+0.707106781*radius2-2);
    y1i = (long) (y_center-0.707106781*radius2+2);
    y2i = (long) (y_center+0.707106781*radius2-2);


    if(x1<0)
        x1 = 0;
    if(x1>naxes[0])
        x1 = naxes[0];
    if(x2<0)
        x2 = 0;
    if(x2>naxes[0])
        x2 = naxes[0];

    if(y1<0)
        y1 = 0;
    if(y1>naxes[1])
        y1 = naxes[1];
    if(y2<0)
        y2 = 0;
    if(y2>naxes[1])
        y2 = naxes[1];

    if(x1i<0)
        x1i = 0;
    if(x1i>naxes[0]-1)
        x1i = naxes[0]-1;
    if(x2i<0)
        x2i = 0;
    if(x2i>naxes[0]-1)
        x2i = naxes[0]-1;

    if(y1i<0)
        y1i = 0;
    if(y1i>naxes[1]-1)
        y1i = naxes[1]-1;
    if(y2i<0)
        y2i = 0;
    if(y2i>naxes[1]-1)
        y2i = naxes[1]-1;


    r2ref = radius*radius;
    subgrid2 = subgrid*subgrid;


    for (ii = x1i; ii < x2i; ii++)
        for (jj = y1i; jj < y2i; jj++)
            data.image[ID].array.F[jj*naxes[0]+ii] = 1;

    for (i = 0; i < subgrid; i++)
        grid[i] = (0.5-0.5/subgrid-1.0*i/subgrid);

    for (ii = x1; ii < x1i; ii++)
        for (jj = y1; jj < y2; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            PA = atan2(ydiff, xdiff);
            r2 = xdiff*xdiff+ydiff*ydiff;

            v0 = radius;
            for(k=0; k<n; k++)
                v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
            r2ref = v0*v0;

            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        PA = atan2(y,x);
                        r = x*x+y*y;

                        v0 = radius;
                        for(k=0; k<n; k++)
                            v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
                        r2ref = v0*v0;

                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }


    for (ii = x2i; ii < x2; ii++)
        for (jj = y1; jj < y2; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            PA = atan2(ydiff, xdiff);
            r2 = xdiff*xdiff+ydiff*ydiff;

            v0 = radius;
            for(k=0; k<n; k++)
                v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
            r2ref = v0*v0;

            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        r = x*x+y*y;
                        PA = atan2(y,x);
                        v0 = radius;
                        for(k=0; k<n; k++)
                            v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
                        r2ref = v0*v0;

                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }

    for (ii = x1i; ii < x2i; ii++)
        for (jj = y1; jj < y1i; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            PA = atan2(ydiff,xdiff);
            r2 = xdiff*xdiff+ydiff*ydiff;
            v0 = radius;
            for(k=0; k<n; k++)
                v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
            r2ref = v0*v0;

            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        PA = atan2(y,x);
                        r = x*x+y*y;
                        v0 = radius;
                        for(k=0; k<n; k++)
                            v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
                        r2ref = v0*v0;
                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }


    for (ii = x1i; ii < x2i; ii++)
        for (jj = y2i; jj < y2; jj++)
        {
            xdiff = x_center-ii;
            ydiff = y_center-jj;
            PA = atan2(ydiff, xdiff);
            r2 = xdiff*xdiff+ydiff*ydiff;
            v0 = radius;
            for(k=0; k<n; k++)
                v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
            r2ref = v0*v0;

            if(r2<r2ref)
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            if(fabs(sqrt(r2)-sqrt(r2ref))<1.5)
            {
                tot = 0;
                for (j = 0; j < subgrid; j++)
                    for (i = 0; i < subgrid; i++)
                    {
                        x = xdiff+grid[i];
                        y = ydiff+grid[j];
                        PA = atan2(y,x);
                        r = x*x+y*y;
                        v0 = radius;
                        for(k=0; k<n; k++)
                            v0 += radius*ra[k]*cos(ka[k]*PA + pa[k]);
                        r2ref = v0*v0;
                        if (r < r2ref)
                            tot += 1.0;
                    }
                tot = tot/subgrid2;
                data.image[ID].array.F[jj*naxes[0]+ii] = tot;
            }
        }

    return(ID);
}


long make_square(char *ID_name, long l1, long l2, double x_center, double y_center, double radius) /* creates a square */
{
    long ID;
    long ii,jj;
    long naxes[2];

    create_2Dimage_ID(ID_name,l1,l2);
    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    for (jj = 0; jj < naxes[1]; jj++)
        for (ii = 0; ii < naxes[0]; ii++)
        {
            if ((((ii-x_center)*(ii-x_center))<(radius*radius))&&(((jj-y_center)*(jj-y_center))<(radius*radius)))
                data.image[ID].array.F[jj*naxes[0]+ii] = 1;
        }

    return(ID);
}

long make_rectangle(char *ID_name, long l1, long l2, double x_center, double y_center, double radius1, double radius2) /* creates a square */
{
    long ID;
    long ii,jj;
    long naxes[2];

    create_2Dimage_ID(ID_name,l1,l2);
    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    for (jj = 0; jj < naxes[1]; jj++)
        for (ii = 0; ii < naxes[0]; ii++)
        {
            if ((((ii-x_center)*(ii-x_center))<(radius1*radius1))&&(((jj-y_center)*(jj-y_center))<(radius2*radius2)))
                data.image[ID].array.F[jj*naxes[0]+ii] = 1;
        }

    return(ID);
}

// line of thickness t from (x1,y1) to (x2,y2)
long make_line(char *IDname, long l1, long l2, double x1, double y1, double x2, double y2, double t)
{
    long ID;
    long ii,jj;
    double x, y, xr, yr, r0;
    double r, PA0;
    long naxes[2];

    create_2Dimage_ID(IDname,l1,l2);
    ID = image_ID(IDname);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    r0 = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    PA0 = atan2((y2-y1),(x2-x1));
    for (jj = 0; jj < naxes[1]; jj++)
        for (ii = 0; ii < naxes[0]; ii++)
        {
            x = 1.0*ii;
            y = 1.0*jj;
            x -= x1;
            y -= y1;
            xr = x*cos(PA0)+y*sin(PA0);
            yr = -x*sin(PA0)+y*cos(PA0);
            r=sqrt(xr*xr+yr*yr);
            xr /= r0;
            yr /= r0;
            if((xr>0)&&(xr<1.0)&&(yr<0.5*t/r0)&&(yr>-0.5*t/r0))
                data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
            else
                data.image[ID].array.F[jj*naxes[0]+ii] = 0.0;
        }

    return(ID);
}

long make_hexagon(char *IDname, long l1, long l2, double x_center, double y_center, double radius)
{
    long ID;
    long ii,jj;
    long naxes[2];
    double x, y, r;
    double value;

    printf("Making hexagon at %f x %f\n",x_center,y_center);

    create_2Dimage_ID(IDname,l1,l2);
    ID = image_ID(IDname);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];

    for (jj = 0; jj < naxes[1]; jj++)
        for (ii = 0; ii < naxes[0]; ii++)
        {
            value = 1.0;
            x = 1.0*ii-x_center;
            y = 1.0*jj-y_center;

            r = y;
            if(fabs(r)>radius)
                value = 0.0;

            // vect: cos(pi/6), sin(pi/6)

            r = cos(PI/6.0)*x + sin(PI/6.0)*y;
            if(fabs(r)>radius)
                value = 0.0;
            r = cos(-PI/6.0)*x + sin(-PI/6.0)*y;
            if(fabs(r)>radius)
                value = 0.0;
            data.image[ID].array.F[jj*naxes[0]+ii] = value;
        }


    return(ID);
}




long make_hexsegpupil(char *IDname, long size, double radius, double gap, double step)
{
    long ID,ID1,IDp;
    long x1, y1;
    double x2,y2;
    long IDdisk;
    long ii;
    double tot = 0.0;
    long size2;

    int PISTONerr = 0;
    int errSEGindex = -1;
    double pampl;
    double piston;
    long SEGcnt = 0;

    int mkInfluenceFunctions = 1;
    long IDif;
    int seg;
    long kk, jj;
    float xc, yc, tc;

    size2 = size*size;


    ID = variable_ID("hexpupnoif");
    if(ID!=-1)
        mkInfluenceFunctions = 0;

    ID = variable_ID("HEXPISTONerr");
    if(ID!=-1)
    {
        PISTONerr = 1;
        pampl = data.variable[ID].value.f;
        printf("Piston error = %f\n",pampl);
    }
    else
        pampl = 0.0;


    ID = variable_ID("HEXPISTONindex");
    if(ID!=-1)
    {
        errSEGindex = (long) (data.variable[ID].value.f+0.01);
        printf("SEGMENT INDEX = %ld\n", (long) errSEGindex);
    }


    create_2Dimage_ID(IDname,size,size);
    ID = image_ID(IDname);
    if(PISTONerr == 1)
        IDp = create_2Dimage_ID("hexpupPha",size,size);

    IDdisk = make_disk("_TMPdisk",size,size,size/2,size/2,radius);
    for(ii=0; ii<size2; ii++)
        data.image[IDdisk].array.F[ii] = 1.0-data.image[IDdisk].array.F[ii];


    SEGcnt = 0;
    for(x1 = -(long) (2*size/step); x1 < (long) (2*size/step); x1++)
        for(y1 = -(long) (2*size/step); y1 < (long) (2*size/step); y1++)
        {
            x2 = step*x1*3;
            y2 = step*sqrt(3.0)*y1;

            if(sqrt(x2*x2+y2*y2)<radius)
            {
                if(errSEGindex==-1)
                {
                    piston = pampl*(1.0-2.0*ran1());
                }
                else
                {
                    if(errSEGindex==SEGcnt)
                    {
                        piston = pampl;
                    }
                    else
                        piston = 0.0;
                }
                printf("Hexagon %ld: ", SEGcnt);
                ID1 = make_hexagon("_TMPhex",size, size, 0.5*size+x2, 0.5*size+y2, (step-gap)*(sqrt(3.0)/2.0));
                tot = 0.0;
                for(ii=0; ii<size2; ii++)
                    tot += data.image[ID1].array.F[ii]*data.image[IDdisk].array.F[ii];
                if(tot<0.1)
                {
                    SEGcnt++;
                    if(PISTONerr==1)
                    {
                        for(ii=0; ii<size2; ii++)
                            data.image[ID].array.F[ii] += data.image[ID1].array.F[ii];
                    }
                    else
                    {
                        for(ii=0; ii<size2; ii++)
                            data.image[ID].array.F[ii] += 1.0*SEGcnt*data.image[ID1].array.F[ii];
                    }

                    if(PISTONerr == 1)
                    {
                        for(ii=0; ii<size2; ii++)
                            data.image[IDp].array.F[ii] += data.image[ID1].array.F[ii]*piston;
                    }
                }
                delete_image_ID("_TMPhex");
            }



            x2 += step*1.5;
            y2 += step*sqrt(3.0)/2.0;
            if(sqrt(x2*x2+y2*y2)<radius)
            {
                // piston = pampl*(1.0-2.0*ran1());
                if(errSEGindex==-1)
                {
                    piston = pampl*(1.0-2.0*ran1());
                }
                else
                {
                    if(errSEGindex==SEGcnt)
                    {
                        piston = pampl;
                    }
                    else
                        piston = 0.0;
                }
                printf("Hexagon %ld: ", SEGcnt);
                ID1 = make_hexagon("_TMPhex",size, size, 0.5*size+x2, 0.5*size+y2, (step-gap)*(sqrt(3.0)/2.0));
                tot = 0.0;
                for(ii=0; ii<size2; ii++)
                    tot += data.image[ID1].array.F[ii]*data.image[IDdisk].array.F[ii];
                if(tot<0.1)
                {
                    SEGcnt++;
                    if(PISTONerr==1)
                    {
                        for(ii=0; ii<size2; ii++)
                            data.image[ID].array.F[ii] += data.image[ID1].array.F[ii];
                    }
                    else
                        for(ii=0; ii<size2; ii++)
                            data.image[ID].array.F[ii] += 1.0*SEGcnt*data.image[ID1].array.F[ii];

                    if(PISTONerr == 1)
                    {
                        for(ii=0; ii<size2; ii++)
                            data.image[IDp].array.F[ii] += data.image[ID1].array.F[ii]*piston;
                    }
                }
                delete_image_ID("_TMPhex");
            }


        }
    delete_image_ID("_TMPdisk");

    printf("%ld segments\n",SEGcnt);


    if(mkInfluenceFunctions==1) // TT and focus for each segment
    {
        IDif = create_3Dimage_ID("hexpupif", size, size, 3*SEGcnt);
        for(seg=0; seg<SEGcnt; seg++)
        {
            // piston
            kk = 3*seg;
            xc = 0.0;
            yc = 0.0;
            tc = 0.0;
            for(ii=0; ii<size; ii++)
                for(jj=0; jj<size; jj++)
                {
                    if(fabs(data.image[ID].array.F[jj*size+ii]-(seg+1.0))<0.01)
                    {
                        data.image[IDif].array.F[kk*size2+jj*size+ii] = 1.0;
                        xc += 1.0*ii;
                        yc += 1.0*jj;
                        tc += 1.0;
                    }
                }
            xc /= tc;
            yc /= tc;

            // tip and tilt
            for(ii=0; ii<size; ii++)
                for(jj=0; jj<size; jj++)
                {
                    if(fabs(data.image[ID].array.F[jj*size+ii]-(seg+1.0))<0.01)
                    {

                        data.image[IDif].array.F[(kk+1)*size2+jj*size+ii] = 1.0*ii-xc;
                        data.image[IDif].array.F[(kk+2)*size2+jj*size+ii] = 1.0*jj-yc;
                    }
                }
        }
    }

    return(ID);
}





long make_jacquinot_pupil(char *ID_name, long l1, long l2, double x_center, double y_center, double width, double height)
{
  long ID;
  long ii,jj;
  long naxes[2];

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      { 
	if ((fabs(jj-y_center)/height)<exp(-((ii-x_center)*(ii-x_center)/width/width)))
	  data.image[ID].array.F[jj*naxes[0]+ii] = 1;
      } 

  return(ID);
}

long make_sectors(char *ID_name, long l1, long l2, double x_center, double y_center, double step, long NB_sectors)
{
  long ID;
  long ii,jj;
  long naxes[2];
  double theta;

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      { 
	theta = atan2((ii-x_center),(jj-y_center));
	if (theta<0.0)
	  theta += 2.0*PI;
	data.image[ID].array.F[jj*naxes[0]+ii] = step*((long) (theta/2.0/PI*NB_sectors));
      }

  return(ID);
}

long make_rnd(char *ID_name, long l1, long l2, char *options)
{
  long ID;
  long ii;
  long naxes[2];
  int distrib;
  long nelement;

  if( data.Debug ) fprintf(stdout, "[make_rnd]\n");

  distrib = 0; /* uniform */
  if (strstr(options,"gauss")!=NULL)
    {
      distrib = 1; /* gauss */
      printf("gaussian distribution\n");
    }
  
  if (strstr(options,"trgauss")!=NULL)
    {
      distrib = 2; /* truncated gauss */
      printf("truncated gaussian distribution\n");
   }

  if ( data.Debug > 1) fprintf(stdout,"Image size = %ld %ld\n",l1,l2);

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  nelement=naxes[0]*naxes[1];
  /*  srand(time(NULL));*/


  // openMP is slow when calling gsl random number generator : do not use openMP here
  if(distrib==0)
    {
      for (ii = 0; ii < nelement; ii++) 
	data.image[ID].array.F[ii] = (double) ran1();
    }
  if(distrib==1)
    {
      for (ii = 0; ii < nelement; ii++) 
      	data.image[ID].array.F[ii] = (double) gauss();
    }
  if(distrib==2)
    {
      for (ii = 0; ii < nelement; ii++) {
	//if( data.Debug > 2 ) printf("ii:%d\n", ii); 
	data.image[ID].array.F[ii] = (double) gauss_trc();
      }
    }

  /*  for (ii = 0; ii < nelement; ii++) 
    { 
      if (distrib == 0)
	data.image[ID].array.F[ii] = (double) ran1();
      
      if (distrib == 1)
	data.image[ID].array.F[ii] = (double) gauss();
      
      if (distrib == 2)
	data.image[ID].array.F[ii] = (double) gauss_trc();
    } 
  */
  return(ID);
}

/*
int make_rnd1(char *ID_name, long l1, long l2, char *options)
{
  int ID;
  long naxes[2];
  int distrib;
  long nelements;
  struct prng *g;
 
  distrib = 0;
  if (strstr(options,"-gauss")!=NULL)
    {
      distrib = 1;
    }
  
  if (strstr(options,"-trgauss")!=NULL)
    {
      distrib = 2;
      printf("truncated gaussian distribution\n");
   }

  g = prng_new("eicg(2147483647,111,1,0)");

   if (g == NULL) 
   {
      fprintf(stderr,"Initialisation of generator failed.\n");
      exit (-1);
   }

   printf("Short name: %s\n",prng_short_name(g));
                  
   printf("Expanded name: %s\n",prng_long_name(g));
                  

   create_2Dimage_ID(ID_name,l1,l2);
   ID = image_ID(ID_name);
   naxes[0] = data.image[ID].md[0].size[0];
   naxes[1] = data.image[ID].md[0].size[1]; 
   nelements=naxes[0]*naxes[1];
   
   prng_get_array(g,data.image[ID].array.F,nelements); 
   prng_reset(g);                
   prng_free(g);                 


   return(0);
}
*/
long make_gauss(char *ID_name, long l1, long l2, double a, double A)
{
  long ID;
  long ii,jj;
  long naxes[2];
  double distsq;

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      { 
	distsq = (ii-naxes[0]/2)*(ii-naxes[0]/2)+(jj-naxes[1]/2)*(jj-naxes[1]/2);
	data.image[ID].array.F[jj*naxes[0]+ii] = (double) A*exp(-distsq/a/a);
      } 
  /*  printf("FWHM = %f\n",2.0*a*sqrt(log(2)));*/
  return(ID);
}

long make_2axis_gauss(char *ID_name, long l1, long l2, double a, double A, double E, double PA)
{
  long ID;
  long ii,jj;
  long naxes[2];
  double distsq;
  double iin,jjn;

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      { 
	iin=1.0*(ii-naxes[0]/2)*cos(PA)+1.0*(jj-naxes[1]/2)*sin(PA);
	jjn=1.0*(jj-naxes[1]/2)*cos(PA)-1.0*(ii-naxes[0]/2)*sin(PA);
	distsq = iin*iin+(1.0/(1.0+E))*jjn*jjn;
	data.image[ID].array.F[jj*naxes[0]+ii] = (double) A*exp(-distsq/a/a);
      } 
  
  return(ID);
}

long make_cluster(char *ID_name, long l1, long l2, char *options)
{
  long ID;
  long ii,jj;
  long naxes[2];
  long nb_star = 3000;
  double cluster_size = 0.1; /* relative to the FOV */
  double concentration = 1.0;
  long i;
  double tmp,dist,angle;
  char input[50];
  int str_pos;
  int sim = 0;
  long lii,ljj,hii,hjj;

  if (strstr(options,"-nbstars ")!=NULL)
    {
      str_pos=strstr(options,"-nbstars ")-options;
      str_pos = str_pos + strlen("-nbstars ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      nb_star = atol(input);
      printf("number of stars is %ld\n",nb_star);
    }

  if (strstr(options,"-conc ")!=NULL)
    {
      str_pos=strstr(options,"-conc ")-options;
      str_pos = str_pos + strlen("-conc ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      concentration = atof(input);
      printf("concentration is %f\n",concentration);
    }

  if (strstr(options,"-size ")!=NULL)
    {
      str_pos=strstr(options,"-size ")-options;
      str_pos = str_pos + strlen("-size ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      cluster_size = atof(input);
      printf("cluster size is %f\n",cluster_size);
    }

    if(strstr(options,"-sim")!=NULL)
      {
	printf("all sources in the central half array \n");
	sim = 1;
      }

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  if (sim==0)
    {
      lii = 0;
      ljj = 0;
      hii = naxes[0];
      hjj = naxes[1];
    }
  else
    {
      lii = naxes[0]/4;
      ljj = naxes[1]/4;
      hii = 3*naxes[0]/4;
      hjj = 3*naxes[1]/4;
    }

 // srand(time(NULL));  
  i = 0;
  while(i<nb_star)
      { 
	dist = gauss();
	dist = sqrt(sqrt(dist*dist));
	dist = pow(dist,concentration);
	angle = 2*PI*ran1();
	ii = (long) (naxes[0]/2+(cluster_size*naxes[0]/2)*dist*cos(angle));
	jj = (long) (naxes[1]/2+(cluster_size*naxes[1]/2)*dist*sin(angle));
	
	if ((ii>lii)&&(jj>ljj)&&(ii<hii)&&(jj<hjj))
	  {
	    tmp = gauss();
	    data.image[ID].array.F[jj*naxes[0]+ii] += tmp*tmp;
	    i++;
	  }


      } 
  
  return(ID);
}

long make_galaxy(char *ID_name, long l1, long l2, double S_radius, double S_L0, double S_ell, double S_PA, double E_radius, double E_L0, double E_ell, double E_PA)
{
  long ID;
  long naxes[2];
  long ii,jj;
  double x,y,r;
  double aob,boa; /* a over b and b over a */
  double total=0;

  /* E = 1-b/a */
  
  ID = create_2Dimage_ID(ID_name,l1,l2);
  naxes[0] = l1;
  naxes[1] = l2;
  
  /* Spiral component */
  aob=1.0/(1.0-S_ell);
  boa=1.0-S_ell;

  for(ii=0;ii<naxes[0];ii++)
    for(jj=0;jj<naxes[1]/2+1;jj++)
      {
	x=cos(S_PA)*(ii-naxes[0]/2)+sin(S_PA)*(jj-naxes[1]/2);
	y=-sin(S_PA)*(ii-naxes[0]/2)+cos(S_PA)*(jj-naxes[1]/2);
	r=sqrt(aob*x*x+boa*y*y);
	data.image[ID].array.F[jj*naxes[0]+ii] = S_L0*exp(-r/S_radius);
      }

  /* Elliptical component */
  aob=1.0/(1.0-E_ell);
  boa=1.0-E_ell;

  for(ii=0;ii<naxes[0];ii++)
    for(jj=0;jj<naxes[1]/2+1;jj++)
      {
	x=cos(E_PA)*(ii-naxes[0]/2)+sin(E_PA)*(jj-naxes[1]/2);
	y=-sin(E_PA)*(ii-naxes[0]/2)+cos(E_PA)*(jj-naxes[1]/2);
	r=sqrt(aob*x*x+boa*y*y);
	data.image[ID].array.F[jj*naxes[0]+ii] += E_L0*pow(10.0,(-3.3307*(pow((r/E_radius),0.25)-1.0)));
      }
  
  /* filling other half */
  for(ii=1;ii<naxes[0];ii++)
    for(jj=1;jj<naxes[1]/2;jj++)
      {
	data.image[ID].array.F[(naxes[1]-jj)*naxes[0]+(naxes[0]-ii)] = data.image[ID].array.F[jj*naxes[0]+ii];
      }
  ii=0;
  for(jj=naxes[1]/2;jj<naxes[1];jj++)
    {
      aob=1.0/(1.0-S_ell);
      boa=1.0-S_ell;
      x=cos(S_PA)*(ii-naxes[0]/2)+sin(S_PA)*(jj-naxes[1]/2);
      y=-sin(S_PA)*(ii-naxes[0]/2)+cos(S_PA)*(jj-naxes[1]/2);
      r=sqrt(aob*x*x+boa*y*y);
      data.image[ID].array.F[jj*naxes[0]+ii] = S_L0*exp(-r/S_radius);
      aob=1.0/(1.0-E_ell);
      boa=1.0-E_ell;
      x=cos(E_PA)*(ii-naxes[0]/2)+sin(E_PA)*(jj-naxes[1]/2);
      y=-sin(E_PA)*(ii-naxes[0]/2)+cos(E_PA)*(jj-naxes[1]/2);
      r=sqrt(aob*x*x+boa*y*y);
      data.image[ID].array.F[jj*naxes[0]+ii] += E_L0*pow(10.0,(-3.3307*(pow((r/E_radius),0.25)-1.0)));
    }

  total = 2.0*PI*S_L0*S_radius*S_radius+23.02*E_L0*E_radius*E_radius;
  printf("total : %f (%f)\n",arith_image_total(ID_name),total);
  return(total);
}


long make_Egalaxy(char *ID_name, long l1, long l2, char *options)
{
  long ID;
  long ii,jj;
  long naxes[2];
  double galaxy_size = 0.1; /* relative to the FOV */
  double concentration = 1.0;
  long i;
  double PA=0;
  double E=0.3; /* position angle and ellipticity */
  double peak = 1; /* maximum value */
  char input[50];
  int str_pos;
  int sim = 0;
  long lii,ljj,hii,hjj;
  double x,y,xcenter,ycenter,distsq;

  if (strstr(options,"-conc ")!=NULL)
    {
      str_pos=strstr(options,"-conc ")-options;
      str_pos = str_pos + strlen("-conc ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      concentration = atof(input);
      printf("concentration is %f\n",concentration);
    }

  if (strstr(options,"-size ")!=NULL)
    {
      str_pos=strstr(options,"-size ")-options;
      str_pos = str_pos + strlen("-size ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      galaxy_size = atof(input);
      printf("size is %f\n",galaxy_size);
    }

  if (strstr(options,"-pa ")!=NULL)
    {
      str_pos=strstr(options,"-pa ")-options;
      str_pos = str_pos + strlen("-pa ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      PA = atof(input);
      printf("galaxy pa size is %f radians \n",PA);
    }

  if (strstr(options,"-e ")!=NULL)
    {
      str_pos=strstr(options,"-e ")-options;
      str_pos = str_pos + strlen("-e ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  input[i] = options[i+str_pos];
	  i++;
	}
      input[i] = '\0';
      E = atof(input);
      printf("galaxy elipticity is %f \n",E);
    }

    if(strstr(options,"-sim")!=NULL)
      {
	printf("all sources in the central half array \n");
	sim = 1;
      }

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  xcenter = naxes[0]/2;
  ycenter = naxes[1]/2;

  if (sim==0)
    {
      lii = 0;
      ljj = 0;
      hii = naxes[0];
      hjj = naxes[1];
    }
  else
    {
      lii = naxes[0]/4;
      ljj = naxes[1]/4;
      hii = 3*naxes[0]/4;
      hjj = 3*naxes[1]/4;
    }

   for (jj = ljj; jj < hjj; jj++) 
      for (ii = lii; ii < hii; ii++)
	{
	  x = cos(PA)*(ii-xcenter)+sin(PA)*(jj-ycenter);
	  y = -sin(PA)*(ii-xcenter)+cos(PA)*(jj-ycenter);
	  /* E = sqrt(a*a-b*b)/a */
	  /* a = 1 */
	  x = x;
	  y = y/sqrt(1-E*E);
	  distsq = (x*x+y*y)/(naxes[0]*naxes[0]+naxes[1]*naxes[1])/galaxy_size/galaxy_size;
	  data.image[ID].array.F[jj*naxes[0]+ii] = (double) peak*exp(-concentration*distsq);
	}
  
  return(ID);
}

// for sol system, index ~2.4 with local zodi
long gen_image_EZdisk(char *ID_name, long size, double InnerEdge, double Index, double Incl)
{
  long ID;
  long ii,jj;
  double x,y,r,r0;
  double value;

  ID = create_2Dimage_ID(ID_name,size,size);
  r0 = 6.0;
  for(ii=0;ii<size;ii++)
    for(jj=0;jj<size;jj++)
      {
	x = 1.0*(ii+0.5)-size/2;
	y = 1.0*(jj+0.5)-size/2;
	y /= cos(Incl);
	r = sqrt(x*x+y*y);
	if(r<InnerEdge)
	  value = 0.0;
	else
	  value = pow(r,-Index);
	value /= cos(Incl);

	value += pow(r0,-Index);
	data.image[ID].array.F[jj*size+ii] = value;
      }

  return(0);
}

long make_slopexy(char *ID_name, long l1,long l2, double sx, double sy)
{
  long ID;
  long ii,jj;
  long naxes[2];
  double coeff;

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  coeff = sx*(naxes[0]/2)+sy*(naxes[1]/2);

  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	data.image[ID].array.F[jj*naxes[0]+ii] = sx*ii+sy*jj-coeff;
      }
  
  return(ID);
}

long make_dist(char *ID_name, long l1,long l2, double f1, double f2)
{
  long ID;
  long ii,jj;
  long naxes[2];
  
  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	data.image[ID].array.F[jj*naxes[0]+ii] = sqrt((f1-ii)*(f1-ii)+(f2-jj)*(f2-jj));
      }

  return(ID);
}


long make_PosAngle(char *ID_name, long l1,long l2, double f1, double f2)
{
  long ID;
  long ii,jj;
  long naxes[2];
  double x,y;

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	x = 1.0*ii-f1;
	y = 1.0*jj-f2;
	data.image[ID].array.F[jj*naxes[0]+ii] = atan2(y,x);
      }

  return(ID);
}




long make_psf_from_profile(char *profile_name, char *ID_name, long l1, long l2)
{
  long ID;
  long naxes[2];
  FILE *fp;
  long nb_lines;
  char lstring[1000];
  char line[200];
  double *distarr;
  double *valarr;
  long ii,jj,i;
  double dist;
  float fl1, fl2;

  create_2Dimage_ID(ID_name,l1,l2);
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  
  /* compute number of lines */
  sprintf(lstring,"wc -l %s > tmpcnt.txt",profile_name);
  if(system(lstring)==-1)
    {
      printf("ERROR: system(\"%s\"), %s line %d\n",lstring,__FILE__,__LINE__);
      exit(0);
    }
  if ((fp=fopen("tmpcnt.txt","r"))==NULL)
    printf("error : can't open file \"tmpcnt.txt\"\n");
  if(fgets(line,200,fp)==NULL)
    {
      printf("ERROR: fgets, %s line %d\n",__FILE__,__LINE__);
      exit(0);
    }
  fclose(fp);
  printf("%s\n",line);
  fflush(stdout);
  sscanf(line,"%ld %s",&nb_lines,lstring);
  
  printf("%ld lines\n",nb_lines);

  distarr = (double*) malloc(sizeof(double)*nb_lines);
  valarr = (double*) malloc(sizeof(double)*nb_lines);

  if ((fp=fopen(profile_name,"r"))==NULL)
    printf("error : can't open file \"%s\"\n",profile_name);

  for(i=0;i<nb_lines;i++)
    {
      if(fgets(line,200,fp)==NULL)
	 {
	   printf("ERROR: fgets, %s line %d\n",__FILE__,__LINE__);
	   exit(0);
	 }
      sscanf(line,"%f %f",&fl1,&fl2);
      distarr[i] = fl1;
      valarr[i] = fl2;
    }
  fclose(fp);
  

  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	dist=sqrt((ii-naxes[0]/2)*(ii-naxes[0]/2)+(jj-naxes[1]/2)*(jj-naxes[1]/2));
	i=0;
	while((distarr[i]<dist)&&(i!=nb_lines-1))
	  {
	    i++;
	  }
	if(i!=0)
	  data.image[ID].array.F[jj*naxes[0]+ii] = valarr[i-1]+(valarr[i]-valarr[i-1])*(dist-distarr[i-1])/(distarr[i]-distarr[i-1]);
	else
	  data.image[ID].array.F[jj*naxes[0]+ii] = valarr[0];
      }

  free(distarr);
  free(valarr);

  return(ID);
}

long make_offsetHyperGaussian(long size, double a, double b, long n, char* IDname)
{
  long ii,jj;
  long ID;
  double x,y,dist;

  ID = create_2Dimage_ID(IDname,size,size);
  for(ii=0;ii<size;ii++)
    for(jj=0;jj<size;jj++)
      {
	x = 1.0*ii-size/2;
	y = 1.0*jj-size/2;
	dist = sqrt(x*x+y*y);
	if(dist<a)
	  data.image[ID].array.F[jj*size+ii] = 0.0;
	else
	  data.image[ID].array.F[jj*size+ii] = 1.0-exp(-pow((dist-a)/b,n));
      }

  return(ID);
}


long make_cosapoedgePupil(long size, double a, double b, char *IDname)
{  
  long ii,jj;
  long ID;
  double x,y,dist;

  ID = create_2Dimage_ID(IDname,size,size);
  for(ii=0;ii<size;ii++)
    for(jj=0;jj<size;jj++)
      {
	x = 1.0*ii-size/2;
	y = 1.0*jj-size/2;
	dist = sqrt(x*x+y*y);
	if(dist<a)
	  data.image[ID].array.F[jj*size+ii] = 1.0;
	else if(dist>b)
	  data.image[ID].array.F[jj*size+ii] = 0.0;
	else
	  data.image[ID].array.F[jj*size+ii] = 0.5*(cos(PI*(dist-a)/(b-a))+1.0);
      }
  return(0);
}


// make square grid of pixels
long make_2Dgridpix(char *IDname, long xsize, long ysize, double pitchx, double pitchy, double offsetx, double offsety)
{
  long ii,jj;
  long ID;
  double x,y;
  long i,j;
  double u,t;

  ID = create_2Dimage_ID(IDname,xsize,ysize);
  for(x=offsetx;x<xsize-1;x+=pitchx)
    for(y=offsety;y<ysize-1;y+=pitchy)
      {
	i = (long) x;
	j = (long) y;
	u = x-i;
	t = y-j;
	data.image[ID].array.F[j*xsize+i] = (1.0-u)*(1.0-t);
	data.image[ID].array.F[(j+1)*xsize+i] = (1.0-u)*t;
	data.image[ID].array.F[j*xsize+i+1] = u*(1.0-t);
	data.image[ID].array.F[(j+1)*xsize+i+1] = u*t;	
      }

  return(ID);
}

// make tile
long make_tile(char *IDin_name, long size, char *IDout_name)
{
  long sizex0,sizey0; // input
  long ii,jj,ii0,jj0;
  long IDin,IDout;

  IDout = create_2Dimage_ID(IDout_name,size,size);
  IDin = image_ID(IDin_name);
  sizex0 = data.image[IDin].md[0].size[0];
  sizey0 = data.image[IDin].md[0].size[1];

  for(ii=0;ii<size;ii++)
    for(jj=0;jj<size;jj++)
      {
	ii0 = ii%sizex0;
	jj0 = jj%sizey0;
	data.image[IDout].array.F[jj*size+ii] = data.image[IDin].array.F[jj0*sizex0+ii0];
      }

  return(IDout);
}
