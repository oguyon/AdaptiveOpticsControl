#include <fitsio.h> 
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _OPENMP
#include <omp.h>
#define OMP_NELEMENT_LIMIT 1000000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_tools/COREMOD_tools.h"

#include "COREMOD_arith/COREMOD_arith.h"

#define SBUFFERSIZE 1000

extern DATA data;

char errmsg[SBUFFERSIZE];






// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//


int arith_image_extract2D_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,2)+CLI_checkarg(4,2)+CLI_checkarg(5,2)+CLI_checkarg(6,2)==0)
    {
      arith_image_extract2D(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl, data.cmdargtoken[5].val.numl, data.cmdargtoken[6].val.numl);
      //      arith_image_extract2D(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart)
      return 0;
    }
  else
    return 1;

}

int arith_set_pixel_cli()
{

  if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,2)+CLI_checkarg(4,2)==0)
    {
      arith_set_pixel(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numl);
      return 0;
    }
  else
    return 1;
}

int arith_set_row_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,2)==0)
    {
      arith_set_row(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl);
      return 0;
    }
  else
    return 1;
}

int arith_set_col_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,2)==0)
    {
      arith_set_col(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numl);
      return 0;
    }
  else
    return 1;
}


int arith_image_zero_cli()
{
    if(CLI_checkarg(1,4)==0)
    {
      arith_image_zero(data.cmdargtoken[1].val.string);
      return 0;
    }
  else
    return 1;
}

//long arith_image_zero(char *ID_name)
//long arith_set_row(char *ID_name, double value, long y)

int arith_image_trunc_cli()
{

  if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,1)+CLI_checkarg(4,3)==0)
    {
      arith_image_trunc(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.string);
      return 0;
    }
  else
    return 1;
}






//long arith_image_merge3D(char *ID_name1, char *ID_name2, char *IDout_name);

int arith_image_merge3D_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,4)+CLI_checkarg(3,3)==0)
    {
      arith_image_merge3D(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}




int init_COREMOD_arith()
{
  
  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "image arithmetic operations");
  data.NBmodule++;


  strcpy(data.cmd[data.NBcmd].key,"extractim");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_image_extract2D_cli;
  strcpy(data.cmd[data.NBcmd].info,"crop 2D image");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <output image> <sizex> <sizey> <xstart> <ystart>");
  strcpy(data.cmd[data.NBcmd].example,"extractim im ime 256 256 100 100");
  strcpy(data.cmd[data.NBcmd].Ccall,"int arith_image_extract2D(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart)"); 
  data.NBcmd++;
  

  strcpy(data.cmd[data.NBcmd].key,"setpix");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_set_pixel_cli;
  strcpy(data.cmd[data.NBcmd].info,"set pixel value");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <value> <x> <y>");
  strcpy(data.cmd[data.NBcmd].example,"setpix im 1.24 100 100");
  strcpy(data.cmd[data.NBcmd].Ccall,"int arith_set_pixel(char *ID_name, double value, long x, long y)");
  data.NBcmd++;
  
  strcpy(data.cmd[data.NBcmd].key,"setrow");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_set_row_cli;
  strcpy(data.cmd[data.NBcmd].info,"set pixel row value");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <value> <row>");
  strcpy(data.cmd[data.NBcmd].example,"setrow im 1.24 100");
  strcpy(data.cmd[data.NBcmd].Ccall,"int arith_set_row(char *ID_name, double value, long y)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"setcol");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_set_col_cli;
  strcpy(data.cmd[data.NBcmd].info,"set pixel column value");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <value> <col>");
  strcpy(data.cmd[data.NBcmd].example,"setcol im 1.24 100");
  strcpy(data.cmd[data.NBcmd].Ccall,"int arith_set_col(char *ID_name, double value, long x)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"imzero");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_image_zero_cli;
  strcpy(data.cmd[data.NBcmd].info,"set pixels to zero");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image>");
  strcpy(data.cmd[data.NBcmd].example,"imzero im");
  strcpy(data.cmd[data.NBcmd].Ccall,"int arith_image_zero(char *ID_name)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"imtrunc");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_image_trunc_cli;
  strcpy(data.cmd[data.NBcmd].info,"trucate pixel values");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <min> <max> <output image>");
  strcpy(data.cmd[data.NBcmd].example,"imtrunc im 0.0 1.0 out");
  strcpy(data.cmd[data.NBcmd].Ccall,"arith_image_trunc(char *ID_name, double f1, double f2, char *ID_out)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"merge3d");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = arith_image_merge3D_cli;
  strcpy(data.cmd[data.NBcmd].info,"merge two 3D cubes into one");
  strcpy(data.cmd[data.NBcmd].syntax,"<input cube 1> <input cube 2> <output cube>");
  strcpy(data.cmd[data.NBcmd].example,"merge3d imc1 imc2 imcout");
  strcpy(data.cmd[data.NBcmd].Ccall,"long arith_image_merge3D(char *ID_name1, char *ID_name2, char *IDout_name)");
  data.NBcmd++;


 // add atexit functions here


}








long arith_set_pixel(char *ID_name, double value, long x, long y)
{
  long ID;
  long naxes[2];
  int atype;
  int n;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];
  
  data.image[ID].md[0].write = 1;
  if(atype == FLOAT)
    data.image[ID].array.F[y*naxes[0]+x] = value;
  else if(atype == DOUBLE)
    data.image[ID].array.D[y*naxes[0]+x] = value;
  else
    {
      n = snprintf(errmsg,SBUFFERSIZE,"Wrong image type(s)\n");
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      exit(0);
    }
  data.image[ID].md[0].write = 0;
  data.image[ID].md[0].cnt0++;
  
  return(ID);
}



long arith_set_row(char *ID_name, double value, long y)
{
  long ID;
  long naxes[2];
  long ii;
  int atype;
  int n;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxes[0]=data.image[ID].md[0].size[0];
  naxes[1]=data.image[ID].md[0].size[1];

  data.image[ID].md[0].write = 1;
  if(atype==FLOAT)
    {
      for(ii=0;ii<naxes[0];ii++)
	data.image[ID].array.F[y*naxes[0]+ii] = value;
    }
  else if(atype==DOUBLE)
    {
      for(ii=0;ii<naxes[0];ii++)
	data.image[ID].array.D[y*naxes[0]+ii] = value;
    }
  else
    {
      n = snprintf(errmsg,SBUFFERSIZE,"Wrong image type(s)\n");
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      exit(0);
    }

  data.image[ID].md[0].write = 0;
  data.image[ID].md[0].cnt0++;

  return(ID);
}

long arith_set_col(char *ID_name, double value, long x)
{
  long ID;
  long naxes[2];
  long y;
  int atype;
  int n;
  
  ID = image_ID(ID_name);
  naxes[0]=data.image[ID].md[0].size[0];
  naxes[1]=data.image[ID].md[0].size[1];
  atype = data.image[ID].md[0].atype;


  data.image[ID].md[0].write = 1;
  if(atype == FLOAT)
    {
      for(y=0;y<naxes[1];y++)
	data.image[ID].array.F[y*naxes[0]+x] = value;
    }
  else if(atype == DOUBLE)
    {
      for(y=0;y<naxes[1];y++)
	data.image[ID].array.D[y*naxes[0]+x] = value;
    }
  else
    {
      n = snprintf(errmsg,SBUFFERSIZE,"Wrong image type(s)\n");
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      exit(0);
    }

  data.image[ID].md[0].write = 0;
  data.image[ID].md[0].cnt0++;

  return(ID);
}

long arith_image_zero(char *ID_name)
{
  long ID;
  long nelem;
  int n;

  ID = image_ID(ID_name);
  nelem = data.image[ID].md[0].nelement;

  data.image[ID].md[0].write = 0;
  if(data.image[ID].md[0].atype == FLOAT)
    memset(data.image[ID].array.F,0,sizeof(float)*nelem);
  else if(data.image[ID].md[0].atype == DOUBLE)
    memset(data.image[ID].array.D,0,sizeof(double)*nelem);
  else if(data.image[ID].md[0].atype == CHAR)
    memset(data.image[ID].array.C,0,sizeof(char)*nelem);
  else if(data.image[ID].md[0].atype == INT)
    memset(data.image[ID].array.I,0,sizeof(int)*nelem);
  else if(data.image[ID].md[0].atype == COMPLEX_FLOAT)
    memset(data.image[ID].array.CF,0,sizeof(float)*2*nelem);
  else if(data.image[ID].md[0].atype == COMPLEX_DOUBLE)
    memset(data.image[ID].array.CD,0,sizeof(double)*2*nelem);
  else if(data.image[ID].md[0].atype == USHORT)
    memset(data.image[ID].array.U,0,sizeof(unsigned short)*nelem);
  else 
    {
      n = snprintf(errmsg,SBUFFERSIZE,"cannot detect image type for image %s",ID_name);
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      exit(0);
    }
  data.image[ID].md[0].write = 0;
  data.image[ID].md[0].cnt0++;

  return(ID);
}


int arith_image_crop(char *ID_name, char *ID_out, long *start, long *end, long cropdim)
{
  long naxis;
  long IDin,IDout;
  long i;
  long *naxes = NULL;
  long *naxesout = NULL;
  long ii,jj,kk;
  int atype;
  int n;

  long start_c[3];
  long end_c[3];

  for(i=0;i<3;i++)
    {
      start_c[i] = 0;
      end_c[i] = 0;
    }

  IDin = image_ID(ID_name);
  if(IDin==-1)
    {
      n = snprintf(errmsg,SBUFFERSIZE,"Missing input image = %s",ID_name);
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      list_image_ID();
      exit(0);
    }

  naxis = data.image[IDin].md[0].naxis;
  if(naxis < 1)
    {
      printERROR(__FILE__,__func__,__LINE__,"naxis < 1");
      exit(0);
    }
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       n = snprintf(errmsg,SBUFFERSIZE,"malloc() error, naxis = %ld",naxis);
       if(n >= SBUFFERSIZE) 
	 printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
       printERROR(__FILE__,__func__,__LINE__,errmsg);
       exit(0);
     }
 
  naxesout  = (long*) malloc(sizeof(long)*naxis);
  if(naxesout==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  atype = data.image[IDin].md[0].atype;
 
  naxes[0] = 0;
  naxesout[0] = 0;
  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[IDin].md[0].size[i];
      naxesout[i] = end[i]-start[i];
    }
  IDout = create_image_ID(ID_out, naxis, naxesout, atype, data.SHARED_DFT, data.NBKEWORD_DFT);

  start_c[0] = start[0];
  if(start_c[0]<0)
    start_c[0] = 0;
  end_c[0] = end[0];
  if(end_c[0]>naxes[0])
    end_c[0] = naxes[0];
  if(naxis>1)
    {
      start_c[1] = start[1];
      if(start_c[1]<0)
	start_c[1] = 0;
      end_c[1] = end[1];
      if(end_c[1]>naxes[1])
	end_c[1] = naxes[1];
    }
  if(naxis>2)
    {
      start_c[2] = start[2];
      if(start_c[2]<0)
	start_c[2] = 0;
      end_c[2] = end[2];
      if(end_c[2]>naxes[2])
	end_c[2] = naxes[2];
    }


  printf("CROP: \n");
  for(i=0;i<3;i++)
    {
      printf("axis %ld: %ld -> %ld\n",i,start_c[i],end_c[i]);
    }


  if(cropdim!=naxis)
    {
      printf("Error (arith_image_crop): cropdim [%ld] and naxis [%ld] are different\n",cropdim,naxis);
    }
  

  if(naxis==1)
    {
      if(atype == FLOAT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    data.image[IDout].array.F[ii-start[0]] = data.image[IDin].array.F[ii];	  
	}
      else if(atype == DOUBLE)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    data.image[IDout].array.D[ii-start[0]] = data.image[IDin].array.D[ii];
	}
      else if(atype == COMPLEX_FLOAT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    {
	       data.image[IDout].array.CF[ii-start[0]].re = data.image[IDin].array.CF[ii].re;
	       data.image[IDout].array.CF[ii-start[0]].im = data.image[IDin].array.CF[ii].im;
	    }
	}
      else if(atype == COMPLEX_DOUBLE)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    {
	       data.image[IDout].array.CD[ii-start[0]].re = data.image[IDin].array.CD[ii].re;
	       data.image[IDout].array.CD[ii-start[0]].im = data.image[IDin].array.CD[ii].im;
	    }
	}
      else if(atype == USHORT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    data.image[IDout].array.U[ii-start[0]] = data.image[IDin].array.U[ii];	  
	}     
      else
	{
	  n = snprintf(errmsg,SBUFFERSIZE,"invalid data type");
	  if(n >= SBUFFERSIZE) 
	    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	  printERROR(__FILE__,__func__,__LINE__,errmsg);
	  exit(0);
	}
    }
  if(naxis==2)
    {
      if(atype == FLOAT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      data.image[IDout].array.F[(jj-start[1])*naxesout[0]+(ii-start[0])] = data.image[IDin].array.F[jj*naxes[0]+ii];
	}
      else if(atype == DOUBLE)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      data.image[IDout].array.D[(jj-start[1])*naxesout[0]+(ii-start[0])] = data.image[IDin].array.D[jj*naxes[0]+ii];
	}
      else if(atype == COMPLEX_FLOAT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      {	      
		data.image[IDout].array.CF[(jj-start[1])*naxesout[0]+(ii-start[0])].re = data.image[IDin].array.CF[jj*naxes[0]+ii].re;
		data.image[IDout].array.CF[(jj-start[1])*naxesout[0]+(ii-start[0])].im = data.image[IDin].array.CF[jj*naxes[0]+ii].im;
	      }
	}
      else if(atype == COMPLEX_DOUBLE)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      {
		data.image[IDout].array.CD[(jj-start[1])*naxesout[0]+(ii-start[0])].re = data.image[IDin].array.CD[jj*naxes[0]+ii].re;
		data.image[IDout].array.CD[(jj-start[1])*naxesout[0]+(ii-start[0])].im = data.image[IDin].array.CD[jj*naxes[0]+ii].im;
	      }
	}
      else if(atype == USHORT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      data.image[IDout].array.U[(jj-start[1])*naxesout[0]+(ii-start[0])] = data.image[IDin].array.U[jj*naxes[0]+ii];
	}
      else
	{
	  printERROR(__FILE__,__func__,__LINE__,"invalid data type");
	  exit(0);
	}
    }
  if(naxis==3)
    {
      if(atype == FLOAT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      for(kk=start_c[2];kk<end_c[2];kk++)
		data.image[IDout].array.F[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])] = data.image[IDin].array.F[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii];
	}
      else if(atype == DOUBLE)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      for(kk=start_c[2];kk<end_c[2];kk++)
		data.image[IDout].array.D[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])] = data.image[IDin].array.D[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii];
	}
      else if(atype == COMPLEX_FLOAT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      for(kk=start_c[2];kk<end_c[2];kk++)
		{
		  data.image[IDout].array.CF[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])].re = data.image[IDin].array.CF[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii].re;		  
		  data.image[IDout].array.CF[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])].im = data.image[IDin].array.CF[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii].im;
		}
	}
      else if(atype == COMPLEX_DOUBLE)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      for(kk=start_c[2];kk<end_c[2];kk++)
		{
		  data.image[IDout].array.CD[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])].re = data.image[IDin].array.CD[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii].re;		  
		  data.image[IDout].array.CD[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])].im = data.image[IDin].array.CD[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii].im;
		}
	}
      else if(atype == USHORT)
	{
	  for(ii=start_c[0];ii<end_c[0];ii++)
	    for(jj=start_c[1];jj<end_c[1];jj++)
	      for(kk=start_c[2];kk<end_c[2];kk++)
		data.image[IDout].array.U[(kk-start[2])*naxesout[0]*naxesout[1]+(jj-start[1])*naxesout[0]+(ii-start[0])] = data.image[IDin].array.U[kk*naxes[0]*naxes[1]+jj*naxes[0]+ii];
	}
      else
	{
	  printERROR(__FILE__,__func__,__LINE__,"invalid data type");
	  exit(0);
	}
    }

  free(naxesout);
  free(naxes);
 
 return(0);
}


int arith_image_extract2D(char *in_name, char *out_name, long size_x, long size_y, long xstart, long ystart)
{
  long *start = NULL;
  long *end = NULL;
  
  start = (long*) malloc(sizeof(long)*2);
  if(start==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  end = (long*) malloc(sizeof(long)*2);
  if(end==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  start[0]=xstart;
  start[1]=ystart;
  end[0]=xstart+size_x;
  end[1]=ystart+size_y;
  arith_image_crop(in_name, out_name, start, end, 2);

  free(start);
  free(end);

  return(0);
}





int arith_image_extract3D(char *in_name, char *out_name, long size_x, long size_y, long size_z, long xstart, long ystart, long zstart)
{
  long *start = NULL;
  long *end = NULL;
  
  start = (long*) malloc(sizeof(long)*3);
  if(start==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       printf("params: %s %s %ld %ld %ld %ld %ld %ld \n",in_name, out_name, size_x, size_y, size_z, xstart, ystart, zstart);
       exit(0);
     }

  end = (long*) malloc(sizeof(long)*3);
  if(end==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       printf("params: %s %s %ld %ld %ld %ld %ld %ld \n",in_name, out_name, size_x, size_y, size_z, xstart, ystart, zstart);
       exit(0);
     }

  start[0]=xstart;
  start[1]=ystart;
  start[2]=zstart;
  end[0]=xstart+size_x;
  end[1]=ystart+size_y;
  end[2]=zstart+size_z;
  arith_image_crop(in_name,out_name,start,end,3);

  free(start);
  free(end);

  return(0);
}


// join two cubes
long arith_image_merge3D(char *ID_name1, char *ID_name2, char *IDout_name)
{
  long ID1, ID2, IDout;
  long xsize, ysize, zsize1, zsize2, zsizeout;
  long ii, jj, kk;
  void *mapv;

  ID1 = image_ID(ID_name1);
  ID2 = image_ID(ID_name2);
  
  xsize = data.image[ID1].md[0].size[0];
  ysize = data.image[ID1].md[0].size[1];

  if(data.image[ID1].md[0].naxis==2)
    zsize1 = 1;
  else
    zsize1 = data.image[ID1].md[0].size[2];

  if(data.image[ID2].md[0].naxis==2)
    zsize2 = 1;
  else
    zsize2 = data.image[ID2].md[0].size[2];



  if((xsize != data.image[ID2].md[0].size[0])||(ysize != data.image[ID2].md[0].size[1]))
    {
      printf("ERROR: input images must have same x y sizes\n");
      printf("%s :  %ld %ld\n", ID_name1, xsize, ysize);
      printf("%s :  %ld %ld\n", ID_name2, data.image[ID2].md[0].size[0], data.image[ID2].md[0].size[1]);
      exit(0);
    }

  IDout = create_3Dimage_ID(IDout_name, xsize, ysize, zsize1+zsize2);
  
  mapv = (void*) data.image[IDout].array.F;

  memcpy ( mapv, (void*) data.image[ID1].array.F, sizeof(float)*xsize*ysize*zsize1);
  
  mapv += sizeof(float)*xsize*ysize*zsize1;
  memcpy ( mapv, data.image[ID2].array.F, sizeof(float)*xsize*ysize*zsize2);

  return(IDout);
}





double arith_image_total(char *ID_name)
{
  long double value;
  long ID;
  long ii;
  long nelement;
  int atype;
  
  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;

  nelement = data.image[ID].md[0].nelement;
   
  value = 0.0;

  if(atype==CHAR)
    {
      for (ii = 0; ii < nelement; ii++)
	value += (long double) data.image[ID].array.C[ii];
    }
  else if(atype==INT)
    {
      for (ii = 0; ii < nelement; ii++)
	value += (long double) data.image[ID].array.I[ii];
    }
  else if(atype==FLOAT)
    {
      for (ii = 0; ii < nelement; ii++)
	value += (long double) data.image[ID].array.F[ii];
    }
  else if(atype==DOUBLE)
    {
      for (ii = 0; ii < nelement; ii++)
	value += (long double) data.image[ID].array.D[ii];
    }
  else if(atype==USHORT)
    {
      for (ii = 0; ii < nelement; ii++)
	value += (long double) data.image[ID].array.U[ii];
    }
  else
    {
      printERROR(__FILE__,__func__,__LINE__,"invalid data type");
      exit(0);
    }

  return((double) value);
}



double arith_image_mean(char *ID_name)
{
  double value;
  long ID;
  
  ID = image_ID(ID_name);
  
  value = (double) (arith_image_total(ID_name)/data.image[ID].md[0].nelement);
  
  return(value);
}

double arith_image_min(char *ID_name)
{
  double value,value1;
  long ID;
  long ii;
  long nelement;
  int atype;
  int OK=0;
  
  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;

  nelement = data.image[ID].md[0].nelement;
    
  value = (double) 0.0;
  if(atype==CHAR)
    {
      value = (double) data.image[ID].array.C[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.C[ii];
	  if(value1<value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==INT)
    {
      value = (double) data.image[ID].array.I[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.I[ii];
	  if(value1<value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==FLOAT)
    {
      value = (double) data.image[ID].array.F[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.F[ii];
	  if(value1<value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==DOUBLE)
    {
      value = (double) data.image[ID].array.D[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.D[ii];
	  if(value1<value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==USHORT)
    {
      value = (double) data.image[ID].array.U[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.U[ii];
	  if(value1<value)
	    value = value1;
	}
      OK=1;
    }
  if(OK==0)
    printf("Error : Invalid data format for arith_image_min\n");

  return(value);
}

double arith_image_max(char *ID_name)
{
  double value,value1;
  long ID;
  long ii;
  long nelement;
  int atype;
  int OK=0;
  
  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;

  nelement = data.image[ID].md[0].nelement;
    
  value = (double) 0.0;
  if(atype==CHAR)
    {
      value = (double) data.image[ID].array.C[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.C[ii];
	  if(value1>value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==INT)
    {
      value = (double) data.image[ID].array.I[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.I[ii];
	  if(value1>value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==FLOAT)
    {
      value = (double) data.image[ID].array.F[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.F[ii];
	  if(value1>value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==DOUBLE)
    {
      value = (double) data.image[ID].array.D[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.D[ii];
	  if(value1>value)
	    value = value1;
	}
      OK=1;
    }
  if(atype==USHORT)
    {
      value = (double) data.image[ID].array.U[0];
      for (ii = 0; ii < nelement; ii++)
	{
	  value1 = (double) data.image[ID].array.U[ii];
	  if(value1>value)
	    value = value1;
	}
      OK=1;
    }
  if(OK==0)
    printf("Error : Invalid data format for arith_image_max\n");

  return(value);
}




double arith_image_percentile(char *ID_name, double fraction)
{
  long ID;
  long ii;
  double value = 0;
  long *arrayL = NULL;
  float *arrayF = NULL;
  double *array_D = NULL;
  unsigned short *array_U = NULL;
  long nelement;
  int atype;
  int atypeOK = 1; 
  
  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;

  nelement = data.image[ID].md[0].nelement;
  

  switch (atype) {

  case CHAR :
    arrayL = (long*) malloc(sizeof(long)*nelement);
    if(arrayL==NULL)
      {
	printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
      }
    arrayL[0] = 0;
    for (ii = 0; ii < nelement; ii++) 
      arrayL[ii] = (long) data.image[ID].array.C[ii];
    quick_sort_long(arrayL, nelement);
    value = (double) arrayL[(long) (fraction*nelement)];
    free(arrayL);
    break;

  case INT :
    arrayL = (long*) malloc(sizeof(long)*nelement);
    if(arrayL==NULL)
      {
	printERROR(__FILE__,__func__,__LINE__,"malloc() error");
	exit(0);
      }
    arrayL[0] = 0;   
    for (ii = 0; ii < nelement; ii++) 
      arrayL[ii] = (long) data.image[ID].array.I[ii];
    quick_sort_long(arrayL, nelement);
    value = (double) arrayL[(long) (fraction*nelement)];
    free(arrayL);    
    break;

  case FLOAT :
    arrayF = (float*) malloc(sizeof(float)*nelement);
    if(arrayF==NULL)
      {
	printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
      }
    arrayF[0] = 0.0;
    for (ii = 0; ii < nelement; ii++) 
      arrayF[ii] = data.image[ID].array.F[ii];
    value = (double) arrayF[(long) (fraction*nelement)];
    free(arrayF);   
    break;

  case DOUBLE :
    array_D = (double*) malloc(sizeof(double)*nelement);
    if(array_D==NULL)
      {
	printERROR(__FILE__,__func__,__LINE__,"malloc() error");
	exit(0);
      }
    array_D[0] = 0.0;
    for (ii = 0; ii < nelement; ii++) 
      array_D[ii] = data.image[ID].array.D[ii];
    quick_sort_double(array_D, nelement);
    value = array_D[(long) (fraction*nelement)];
    free(array_D);    
    break;

  case USHORT :
    array_U = (unsigned short*) malloc(sizeof(unsigned short)*nelement);
    if(array_U==NULL)
      {
	printERROR(__FILE__,__func__,__LINE__,"malloc() error");
	exit(0);
      }
    array_U[0] = 0.0;
    for (ii = 0; ii < nelement; ii++) 
      array_U[ii] = data.image[ID].array.U[ii];
    quick_sort_ushort(array_U, nelement);
    value = array_U[(long) (fraction*nelement)];
    free(array_U);    
    break;

  default:
    printERROR(__FILE__,__func__,__LINE__,"Image type not supported");
    atypeOK = 0;
    break;
  }

  if(atypeOK == 0)
    exit(0);

  return(value);
}


double arith_image_median(char *ID_name)
{
  double value = 0.0;
  value = arith_image_percentile(ID_name, 0.5);
  return(value);
}



long arith_image_dx(char *ID_name, char *IDout_name)
{
  long ID, IDout;
  long *naxes = NULL;
  long naxis;
  long ii,jj;
  long nelement;
  int atype;
  long i;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxis = data.image[ID].md[0].naxis;
  if(naxis!=2)
    {
      printERROR(__FILE__,__func__,__LINE__,"Function only supports 2-D images\n");
      exit(0);
    }
  naxes = (long*) malloc(sizeof(long)*naxis);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];

  IDout = create_image_ID(IDout_name, naxis, naxes, atype, data.SHARED_DFT, data.NBKEWORD_DFT);
  for(jj=0;jj<naxes[1];jj++)
    {
      for(ii=1;ii<naxes[0]-1;ii++)
	data.image[IDout].array.F[jj*naxes[0]+ii] = (data.image[ID].array.F[jj*naxes[0]+ii+1]-data.image[ID].array.F[jj*naxes[0]+ii-1])/2.0;
      data.image[IDout].array.F[jj*naxes[0]] = data.image[ID].array.F[jj*naxes[0]+1]-data.image[ID].array.F[jj*naxes[0]];
      data.image[IDout].array.F[jj*naxes[0]+naxes[0]-1] = data.image[ID].array.F[jj*naxes[0]+naxes[0]-1]-data.image[ID].array.F[jj*naxes[0]+naxes[0]-2];
    }
  
  free(naxes);
  
  return(IDout);
}


long arith_image_dy(char *ID_name, char *IDout_name)
{
  long ID, IDout;
  long *naxes = NULL;
  long naxis;
  long ii,jj;
  long nelement;
  int atype;
  long i;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxis = data.image[ID].md[0].naxis;
  if(naxis!=2)
    {
      printERROR(__FILE__,__func__,__LINE__,"Function only supports 2-D images\n");
      exit(0);
    }
  naxes = (long*) malloc(sizeof(long)*naxis);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];

  IDout = create_image_ID(IDout_name, naxis, naxes, atype, data.SHARED_DFT, data.NBKEWORD_DFT);
  for(ii=0;ii<naxes[0];ii++)
    {
      for(jj=1;jj<naxes[1]-1;jj++)
	data.image[IDout].array.F[jj*naxes[0]+ii] = (data.image[ID].array.F[(jj+1)*naxes[0]+ii]-data.image[ID].array.F[(jj-1)*naxes[0]+ii])/2.0;

      data.image[IDout].array.F[ii] = data.image[ID].array.F[1*naxes[0]+ii]-data.image[ID].array.F[ii];

      data.image[IDout].array.F[(naxes[1]-1)*naxes[0]+ii] = data.image[ID].array.F[(naxes[1]-1)*naxes[0]+ii]-data.image[ID].array.F[(naxes[1]-2)*naxes[0]+ii];

    }
  
  free(naxes);
  
  return(IDout);
}






/* ------------------------------------------------------------------------- */
/* Functions for bison / flex                                                */
/* im : image 													
  d : double
 
  function_<inputformat>_<outputformat>__<math function input>_<math function output> 
  
  examples:
  function_imim__dd_d  : input is (image, image), applies double,double -> double function
  
  ------------------------------------------------------------------------- */


int arith_image_function_im_im__d_d(char *ID_name, char *ID_out, double (*pt2function)(double))
{
    long ID;
    long IDout;
    long *naxes = NULL;
    long naxis;
    long ii;
    long nelement;
    int atype, atypeout;
    long i;


    if(data.Debug>0)
    {
        printf("arith_image_function_d_d  %s %s\n", ID_name, ID_out);
        fflush(stdout);
    }

    ID = image_ID(ID_name);
    atype = data.image[ID].md[0].atype;
    naxis=data.image[ID].md[0].naxis;
    naxes = (long*) malloc(sizeof(long)*naxis);
    if(naxes==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"malloc() error");
        exit(0);
    }


    for(i=0; i<naxis; i++)
    {
        naxes[i] = data.image[ID].md[0].size[i];
    }


    atypeout = FLOAT;
    if(atype==DOUBLE)
        atypeout = DOUBLE;

    IDout = create_image_ID(ID_out, naxis, naxes, atypeout, data.SHARED_DFT, data.NBKEWORD_DFT);
    free(naxes);

    nelement = data.image[ID].md[0].nelement;


# ifdef _OPENMP
    #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
    {
# endif


        if(atype==CHAR)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.C[ii]));
        }
        if(atype==INT)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.I[ii]));
        }
        if(atype==FLOAT)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.F[ii]));
        }
        if(atype==DOUBLE)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.D[ii] = pt2function(data.image[ID].array.D[ii]);
        }
# ifdef _OPENMP
    }
# endif


    if(data.Debug>0)
    {
        printf("arith_image_function_d_d  DONE\n");
        fflush(stdout);
    }


    return(0);
}





int arith_image_function_imd_im__dd_d(char *ID_name, double v0, char *ID_out, double (*pt2function)(double, double))
{
    long ID;
    long IDout;
    long *naxes = NULL;
    long naxis;
    long ii;
    long nelement;
    int atype, atypeout;
    long i;



    ID = image_ID(ID_name);
    atype = data.image[ID].md[0].atype;
    naxis=data.image[ID].md[0].naxis;
    naxes = (long*) malloc(sizeof(long)*naxis);
    if(naxes==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"malloc() error");
        exit(0);
    }


    for(i=0; i<naxis; i++)
    {
        naxes[i] = data.image[ID].md[0].size[i];
    }


    atypeout = FLOAT;
    if(atype==DOUBLE)
        atypeout = DOUBLE;

    IDout = create_image_ID(ID_out, naxis, naxes, atypeout, data.SHARED_DFT, data.NBKEWORD_DFT);
    free(naxes);

    nelement = data.image[ID].md[0].nelement;


# ifdef _OPENMP
    #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
    {
# endif


        if(atype==CHAR)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.C[ii]), v0);
        }
        if(atype==INT)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.I[ii]), v0);
        }
        if(atype==FLOAT)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.F[ii]), v0);
        }
        if(atype==DOUBLE)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.D[ii] = pt2function(data.image[ID].array.D[ii], v0);
        }
# ifdef _OPENMP
    }
# endif


    if(data.Debug>0)
    {
        printf("arith_image_function_d_d  DONE\n");
        fflush(stdout);
    }


    return(0);
}




int arith_image_function_imdd_im__ddd_d(char *ID_name, double v0, double v1, char *ID_out, double (*pt2function)(double, double, double))
{
    long ID;
    long IDout;
    long *naxes = NULL;
    long naxis;
    long ii;
    long nelement;
    int atype, atypeout;
    long i;



    ID = image_ID(ID_name);
    atype = data.image[ID].md[0].atype;
    naxis=data.image[ID].md[0].naxis;
    naxes = (long*) malloc(sizeof(long)*naxis);
    if(naxes==NULL)
    {
        printERROR(__FILE__,__func__,__LINE__,"malloc() error");
        exit(0);
    }


    for(i=0; i<naxis; i++)
    {
        naxes[i] = data.image[ID].md[0].size[i];
    }


    atypeout = FLOAT;
    if(atype==DOUBLE)
        atypeout = DOUBLE;

    IDout = create_image_ID(ID_out, naxis, naxes, atypeout, data.SHARED_DFT, data.NBKEWORD_DFT);
    free(naxes);

    nelement = data.image[ID].md[0].nelement;


# ifdef _OPENMP
    #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
    {
# endif


        if(atype==CHAR)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.C[ii]), v0, v1);
        }
        if(atype==INT)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.I[ii]), v0, v1);
        }
        if(atype==FLOAT)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.F[ii] = (float) pt2function((double) (data.image[ID].array.F[ii]), v0, v1);
        }
        if(atype==DOUBLE)
        {
# ifdef _OPENMP
            #pragma omp for
# endif
            for (ii = 0; ii < nelement; ii++)
                data.image[IDout].array.D[ii] = pt2function(data.image[ID].array.D[ii], v0, v1);
        }
# ifdef _OPENMP
    }
# endif


    if(data.Debug>0)
    {
        printf("arith_image_function_d_d  DONE\n");
        fflush(stdout);
    }


    return(0);
}










/* ------------------------------------------------------------------------- */
/* image  -> image                                                           */
/* ------------------------------------------------------------------------- */




int arith_image_function_1_1_byID(long ID, long IDout, double (*pt2function)(double))
{
  long *naxes = NULL;
  long naxis;
  long ii;
  long nelement;
  int atype, atypeout;
  long i;

  //  printf("arith_image_function_1_1\n");

  atype = data.image[ID].md[0].atype;
  naxis=data.image[ID].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }


  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[ID].md[0].size[i];
    }
  

  atypeout = FLOAT;
  if(atype==DOUBLE)
    atypeout = DOUBLE;

  free(naxes);

  nelement = data.image[ID].md[0].nelement;
 
 
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT) 
  {
  # endif


  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]));
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]));
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
      	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]));
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]));
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}


int arith_image_function_1_1(char *ID_name, char *ID_out, double (*pt2function)(double))
{
  long ID;
  long IDout;
  long *naxes = NULL;
  long naxis;
  long ii;
  long nelement;
  int atype, atypeout;
  long i;

  //  printf("arith_image_function_1_1\n");

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxis=data.image[ID].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }


  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[ID].md[0].size[i];
    }
  

  atypeout = FLOAT;
  if(atype==DOUBLE)
    atypeout = DOUBLE;

  IDout = create_image_ID(ID_out, naxis, naxes, atypeout, data.SHARED_DFT, data.NBKEWORD_DFT);
  free(naxes);

  nelement = data.image[ID].md[0].nelement;
 
 
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT) 
  {
  # endif


  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]));
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]));
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
      	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]));
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]));
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}







// imagein -> imagein (in place)
int arith_image_function_1_1_inplace_byID(long ID, double (*pt2function)(double))
{
  long ii;
  long nelement;
  int atype, atypeout;

  // printf("arith_image_function_1_1_inplace\n");

  atype = data.image[ID].md[0].atype;

  atypeout = atype;
  if(atype==DOUBLE)
    atypeout = DOUBLE;

  nelement = data.image[ID].md[0].nelement;

  data.image[ID].md[0].write = 0;
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  #endif

  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]));
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]));
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]));
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]));
    }
  
  # ifdef _OPENMP
  }
  # endif

  data.image[ID].md[0].write = 0;
  data.image[ID].md[0].cnt0++;

  return(0);
}




// imagein -> imagein (in place)
int arith_image_function_1_1_inplace(char *ID_name, double (*pt2function)(double))
{
  long ID;
  long ii;
  long nelement;
  int atype, atypeout;

  // printf("arith_image_function_1_1_inplace\n");

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;

  atypeout = atype;
  if(atype==DOUBLE)
    atypeout = DOUBLE;

  nelement = data.image[ID].md[0].nelement;

  data.image[ID].md[0].write = 0;
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  #endif

  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]));
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]));
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]));
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]));
    }
  
  # ifdef _OPENMP
  }
  # endif

  data.image[ID].md[0].write = 0;
  data.image[ID].md[0].cnt0++;

  return(0);
}








double Pacos(double a) {return((double) acos(a));}
double Pasin(double a) {return((double) asin(a));}
double Patan(double a) {return((double) atan(a));}
double Pceil(double a) {return((double) ceil(a));}
double Pcos(double a) {return((double) cos(a));}
double Pcosh(double a) {return((double) cosh(a));}
double Pexp(double a) {return((double) exp(a));}
double Pfabs(double a) {return((double) fabs(a));}
double Pfloor(double a) {return((double) floor(a));}
double Pln(double a) {return((double) log(a));}
double Plog(double a) {return((double) log10(a));}
double Psqrt(double a) {return((double) sqrt(a));}
double Psin(double a) {return((double) sin(a));}
double Psinh(double a) {return((double) sinh(a));}
double Ptan(double a) {return((double) tan(a));}
double Ptanh(double a) {return((double) tanh(a));}

double Ppositive(double a) 
{
double value = 0.0;
 if(a>0.0)
   value = (double) 1.0;
return(value);
}


int arith_image_acos_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pacos); return(0);}
int arith_image_asin_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pasin); return(0);}
int arith_image_atan_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Patan); return(0);}
int arith_image_ceil_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pceil); return(0);}
int arith_image_cos_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pcos); return(0);}
int arith_image_cosh_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pcosh); return(0);}
int arith_image_exp_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pexp); return(0);}
int arith_image_fabs_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pfabs); return(0);}
int arith_image_floor_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pfloor); return(0);}
int arith_image_ln_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Pln); return(0);}
int arith_image_log_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Plog); return(0);}
int arith_image_sqrt_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Psqrt); return(0);}
int arith_image_sin_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Psin); return(0);}
int arith_image_sinh_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Psinh); return(0);}
int arith_image_tan_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Ptan); return(0);}
int arith_image_tanh_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Ptanh); return(0);}
int arith_image_positive_byID(long ID, long IDout){ arith_image_function_1_1_byID(ID,IDout,&Ppositive); return(0);}

int arith_image_acos(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pacos); return(0);}
int arith_image_asin(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pasin); return(0);}
int arith_image_atan(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Patan); return(0);}
int arith_image_ceil(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pceil); return(0);}
int arith_image_cos(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pcos); return(0);}
int arith_image_cosh(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pcosh); return(0);}
int arith_image_exp(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pexp); return(0);}
int arith_image_fabs(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pfabs); return(0);}
int arith_image_floor(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pfloor); return(0);}
int arith_image_ln(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Pln); return(0);}
int arith_image_log(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Plog); return(0);}
int arith_image_sqrt(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Psqrt); return(0);}
int arith_image_sin(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Psin); return(0);}
int arith_image_sinh(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Psinh); return(0);}
int arith_image_tan(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Ptan); return(0);}
int arith_image_tanh(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Ptanh); return(0);}
int arith_image_positive(char *ID_name, char *ID_out){ arith_image_function_1_1(ID_name,ID_out,&Ppositive); return(0);}








int arith_image_acos_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pacos); return(0);}
int arith_image_asin_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pasin); return(0);}
int arith_image_atan_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Patan); return(0);}
int arith_image_ceil_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pceil); return(0);}
int arith_image_cos_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pcos); return(0);}
int arith_image_cosh_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pcosh); return(0);}
int arith_image_exp_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pexp); return(0);}
int arith_image_fabs_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pfabs); return(0);}
int arith_image_floor_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pfloor); return(0);}
int arith_image_ln_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Pln); return(0);}
int arith_image_log_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Plog); return(0);}
int arith_image_sqrt_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Psqrt); return(0);}
int arith_image_sin_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Psin); return(0);}
int arith_image_sinh_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Psinh); return(0);}
int arith_image_tan_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Ptan); return(0);}
int arith_image_tanh_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Ptanh); return(0);}
int arith_image_positive_inplace_byID(long ID){ arith_image_function_1_1_inplace_byID(ID,&Ppositive); return(0);}

int arith_image_acos_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pacos); return(0);}
int arith_image_asin_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pasin); return(0);}
int arith_image_atan_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Patan); return(0);}
int arith_image_ceil_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pceil); return(0);}
int arith_image_cos_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pcos); return(0);}
int arith_image_cosh_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pcosh); return(0);}
int arith_image_exp_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pexp); return(0);}
int arith_image_fabs_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pfabs); return(0);}
int arith_image_floor_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pfloor); return(0);}
int arith_image_ln_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Pln); return(0);}
int arith_image_log_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Plog); return(0);}
int arith_image_sqrt_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Psqrt); return(0);}
int arith_image_sin_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Psin); return(0);}
int arith_image_sinh_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Psinh); return(0);}
int arith_image_tan_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Ptan); return(0);}
int arith_image_tanh_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Ptanh); return(0);}
int arith_image_positive_inplace(char *ID_name){ arith_image_function_1_1_inplace(ID_name,&Ppositive); return(0);}








/* ------------------------------------------------------------------------- */
/* image, image  -> image                                                    */
/* ------------------------------------------------------------------------- */


int arith_image_function_2_1(char *ID_name1, char *ID_name2, char *ID_out, double (*pt2function)(double, double))
{
  long ID1,ID2;
  long IDout;
  long ii;
  long *naxes = NULL;
  long nelement1,nelement2,nelement;
  long naxis;
  int atype1,atype2;
  long i;
  int n;
  char errmsg[200];

  ID1 = image_ID(ID_name1);
  ID2 = image_ID(ID_name2);

	printf("IMAGE IMAGE -> IMAGE\n");
	fflush(stdout);


  if(ID1==-1)
    {
      sprintf(errmsg, "Image %s does not exist: cannot proceed\n", ID_name1);
      printRED(errmsg);
      return 1;
    }    

  if(ID2==-1)
    {
      sprintf(errmsg, "Image %s does not exist: cannot proceed\n", ID_name2);
      printRED(errmsg);
      return 1;
    }    

  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;
  naxis=data.image[ID1].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  for(i=0;i<naxis;i++)
      {
		  naxes[i] = data.image[ID1].md[0].size[i];
		//printf("axis %ld:  %ld\n", i, naxes[i]);
//fflush(stdout);
	  }
  
  
  IDout = create_image_ID(ID_out, naxis, naxes, atype1, data.SHARED_DFT, data.NBKEWORD_DFT);
  free(naxes);
  nelement1 = data.image[ID1].md[0].nelement;
  nelement2 = data.image[ID2].md[0].nelement;
  
  nelement = nelement1;
  if(nelement1!=nelement2)
    {
      n = snprintf(errmsg,SBUFFERSIZE,"images %s and %s have different number of elements ( %ld %ld )\n",ID_name1,ID_name2,nelement1,nelement2);
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);

  //      fprintf(stderr,"ERROR [arith_image_function_2_1_inplace]: images %s and %s have different number of elements ( %ld %ld )\n",ID_name1,ID_name2,nelement1,nelement2);
      exit(0);
    }

	//list_image_ID();

  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {  
  # endif

  if((atype1==CHAR)&&(atype2==CHAR))
    {
  		//printf("CHAR CHAR\n");
		//fflush(stdout);
		//exit(0);
     # ifdef _OPENMP
      #pragma omp for
      # endif

      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID1].array.C[ii]),(double) (data.image[ID2].array.C[ii]));
    }
  if((atype1==INT)&&(atype2==INT))
    {
  		//printf("INT INT\n");
		//fflush(stdout);
		//exit(0);
     # ifdef _OPENMP
      #pragma omp for
      # endif
     for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID1].array.I[ii]),(double) (data.image[ID2].array.I[ii]));
    }
  if((atype1==FLOAT)&&(atype2==FLOAT))
    {
//		printf("FLOAT FLOAT\n");
	//	fflush(stdout);
		//exit(0);
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID1].array.F[ii]),(double) (data.image[ID2].array.F[ii]));
    }
  if((atype1==DOUBLE)&&(atype2==DOUBLE))
    {
// 		printf("DOUBLE DOUBLE\n");
	//	fflush(stdout);
		//exit(0);
     # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.D[ii] = (double) pt2function((double) (data.image[ID1].array.D[ii]),(double) (data.image[ID2].array.D[ii]));
    }

  # ifdef _OPENMP
  }
  # endif


  return(0);
}






int arith_image_function_2_1_inplace(char *ID_name1, char *ID_name2, double (*pt2function)(double,double))
{
  long ID1,ID2;
  long ii;
  long nelement1,nelement2,nelement;
  int atype1,atype2;
  int n;
  
  ID1 = image_ID(ID_name1);
  ID2 = image_ID(ID_name2);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;
  nelement1 = data.image[ID1].md[0].nelement;
  nelement2 = data.image[ID2].md[0].nelement;

  nelement = nelement1;
  if(nelement1!=nelement2)
    {
      n = snprintf(errmsg,SBUFFERSIZE,"images %s and %s have different number of elements\n",ID_name1,ID_name2);
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      exit(0);
    }
  
  data.image[ID1].md[0].write = 1;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {  
  # endif
  if((atype1==CHAR)&&(atype2==CHAR))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.F[ii] = pt2function((double) (data.image[ID1].array.C[ii]),(double) (data.image[ID2].array.C[ii]));
    }
  if((atype1==INT)&&(atype2==INT))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.F[ii] = pt2function((double) (data.image[ID1].array.I[ii]),(double) (data.image[ID2].array.I[ii]));
    }
  if((atype1==FLOAT)&&(atype2==FLOAT))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.F[ii] = pt2function((double) (data.image[ID1].array.F[ii]),(double) (data.image[ID2].array.F[ii]));
    }
  if((atype1==DOUBLE)&&(atype2==DOUBLE))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.D[ii] = pt2function((double) (data.image[ID1].array.D[ii]),(double) (data.image[ID2].array.D[ii]));
    }

  if((atype1==COMPLEX_FLOAT)&&(atype2==COMPLEX_FLOAT))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	{
	  data.image[ID1].array.CF[ii].re = pt2function((double) (data.image[ID1].array.CF[ii].re),(double) (data.image[ID2].array.CF[ii].re));
	  data.image[ID1].array.CF[ii].im = pt2function((double) (data.image[ID1].array.CF[ii].im),(double) (data.image[ID2].array.CF[ii].im));
	}
    }

  if((atype1==COMPLEX_DOUBLE)&&(atype2==COMPLEX_DOUBLE))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	{
	  data.image[ID1].array.CD[ii].re = pt2function((double) (data.image[ID1].array.CD[ii].re),(double) (data.image[ID2].array.CD[ii].re));
	  data.image[ID1].array.CD[ii].im = pt2function((double) (data.image[ID1].array.CD[ii].im),(double) (data.image[ID2].array.CD[ii].im));
	}
    }

  # ifdef _OPENMP
  }
  # endif

  data.image[ID1].md[0].write = 0;
  data.image[ID1].md[0].cnt0++;

  return(0);
}








int arith_image_function_2_1_inplace_byID(long ID1, long ID2, double (*pt2function)(double,double))
{
  long ii;
  long nelement1,nelement2,nelement;
  int atype1,atype2;
  int n;
  
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;
  nelement1 = data.image[ID1].md[0].nelement;
  nelement2 = data.image[ID2].md[0].nelement;

  nelement = nelement1;
  if(nelement1!=nelement2)
    {
      n = snprintf(errmsg,SBUFFERSIZE,"images ID %ld and %ld have different number of elements\n",ID1,ID2);
      if(n >= SBUFFERSIZE) 
	printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
      printERROR(__FILE__,__func__,__LINE__,errmsg);
      exit(0);
    }
  
  data.image[ID1].md[0].write = 1;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {  
  # endif
  if((atype1==CHAR)&&(atype2==CHAR))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.F[ii] = pt2function((double) (data.image[ID1].array.C[ii]),(double) (data.image[ID2].array.C[ii]));
    }
  if((atype1==INT)&&(atype2==INT))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.F[ii] = pt2function((double) (data.image[ID1].array.I[ii]),(double) (data.image[ID2].array.I[ii]));
    }
  if((atype1==FLOAT)&&(atype2==FLOAT))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.F[ii] = pt2function((double) (data.image[ID1].array.F[ii]),(double) (data.image[ID2].array.F[ii]));
    }
  if((atype1==DOUBLE)&&(atype2==DOUBLE))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID1].array.D[ii] = pt2function((double) (data.image[ID1].array.D[ii]),(double) (data.image[ID2].array.D[ii]));
    }

  if((atype1==COMPLEX_FLOAT)&&(atype2==COMPLEX_FLOAT))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	{
	  data.image[ID1].array.CF[ii].re = pt2function((double) (data.image[ID1].array.CF[ii].re),(double) (data.image[ID2].array.CF[ii].re));
	  data.image[ID1].array.CF[ii].im = pt2function((double) (data.image[ID1].array.CF[ii].im),(double) (data.image[ID2].array.CF[ii].im));
	}
    }

  if((atype1==COMPLEX_DOUBLE)&&(atype2==COMPLEX_DOUBLE))
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	{
	  data.image[ID1].array.CD[ii].re = pt2function((double) (data.image[ID1].array.CD[ii].re),(double) (data.image[ID2].array.CD[ii].re));
	  data.image[ID1].array.CD[ii].im = pt2function((double) (data.image[ID1].array.CD[ii].im),(double) (data.image[ID2].array.CD[ii].im));
	}
    }

  # ifdef _OPENMP
  }
  # endif

  data.image[ID1].md[0].write = 0;
  data.image[ID1].md[0].cnt0++;

  return(0);
}












double Pfmod(double a, double b) {return((double) fmod(a,b));}
double Ppow(double a, double b) {if(b>0){return((double) pow(a,b));}else{return((double) pow(a,-b));}}
double Padd(double a, double b) {return((double) a+b);}
double Psubm(double a, double b) {return((double) b-a);}
double Psub(double a, double b) {return((double) a-b);}
double Pmult(double a, double b) {return((double) a*b);}
double Pdiv(double a, double b) {return((double) a/b);}
double Pdiv1(double a, double b) {return((double) b/a);}
double Pminv(double a, double b) {if(a<b){return(a);}else{return(b);}}
double Pmaxv(double a, double b) {if(a>b){return(a);}else{return(b);}}
double Ptestlt(double a, double b) {if(a<b){return( (double) 1.0);}else{return((double) 0.0);}}
double Ptestmt(double a, double b) {if(a<b){return((double) 0.0);}else{return((double) 1.0);}}



int arith_image_fmod(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Pfmod); return(0);}
int arith_image_pow(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Ppow); return(0);}
int arith_image_add(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Padd); return(0);}
int arith_image_sub(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Psub); return(0);}
int arith_image_mult(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Pmult); return(0);}
int arith_image_div(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Pdiv); return(0);}
int arith_image_minv(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Pminv); return(0);}
int arith_image_maxv(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Pmaxv); return(0);}
int arith_image_testlt(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Ptestlt); return(0);}
int arith_image_testmt(char *ID1_name, char *ID2_name, char *ID_out){ arith_image_function_2_1(ID1_name,ID2_name,ID_out,&Ptestmt); return(0);}


int arith_image_fmod_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Pfmod); return(0);}
int arith_image_pow_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Ppow); return(0);}
int arith_image_add_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Padd); return(0);}
int arith_image_sub_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Psub); return(0);}
int arith_image_mult_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name, &Pmult); return(0);}
int arith_image_div_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Pdiv); return(0);}
int arith_image_minv_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Pminv); return(0);}
int arith_image_maxv_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Pmaxv); return(0);}
int arith_image_testlt_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Ptestlt); return(0);}
int arith_image_testmt_inplace(char *ID1_name, char *ID2_name){ arith_image_function_2_1_inplace(ID1_name,ID2_name,&Ptestmt); return(0);}


int arith_image_fmod_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Pfmod); return(0);}
int arith_image_pow_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Ppow); return(0);}
int arith_image_add_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Padd); return(0);}
int arith_image_sub_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Psub); return(0);}
int arith_image_mult_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2, &Pmult); return(0);}
int arith_image_div_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Pdiv); return(0);}
int arith_image_minv_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Pminv); return(0);}
int arith_image_maxv_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Pmaxv); return(0);}
int arith_image_testlt_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Ptestlt); return(0);}
int arith_image_testmt_inplace_byID(long ID1, long ID2){ arith_image_function_2_1_inplace_byID(ID1,ID2,&Ptestmt); return(0);}










/* ------------------------------------------------------------------------- */
/* complex image, complex image  -> complex image                            */
/* ------------------------------------------------------------------------- */
// complex float (CF), complex float (CF) -> complex float (CF)
int arith_image_function_CF_CF__CF(char *ID_name1, char *ID_name2, char *ID_out, complex_float (*pt2function)(complex_float, complex_float))
{
  long ID1,ID2;
  long IDout;
  long ii;
  long *naxes = NULL;
  long nelement;
  long naxis;
  int atype1,atype2;
  long i;

  ID1 = image_ID(ID_name1);
  ID2 = image_ID(ID_name2);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;
  naxis=data.image[ID1].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[ID1].md[0].size[i];
    }
  
  IDout=create_image_ID(ID_out, naxis, naxes, atype1, data.SHARED_DFT, data.NBKEWORD_DFT);
  free(naxes);
  nelement = data.image[ID1].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  #pragma omp for
  # endif
  for (ii = 0; ii < nelement; ii++)
    data.image[IDout].array.CF[ii] = pt2function(data.image[ID1].array.CF[ii], data.image[ID2].array.CF[ii]);
  # ifdef _OPENMP
  }
  # endif

  return(0);
}

// complex double (CD), complex double (CD) -> complex double (CD)
int arith_image_function_CD_CD__CD(char *ID_name1, char *ID_name2, char *ID_out, complex_double (*pt2function)(complex_double, complex_double))
{
  long ID1,ID2;
  long IDout;
  long ii;
  long *naxes = NULL;
  long nelement;
  long naxis;
  int atype1,atype2;
  long i;

  ID1 = image_ID(ID_name1);
  ID2 = image_ID(ID_name2);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;
  naxis=data.image[ID1].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[ID1].md[0].size[i];
    }
  
  IDout = create_image_ID(ID_out, naxis, naxes, atype1, data.SHARED_DFT, data.NBKEWORD_DFT);
  free(naxes);
  nelement = data.image[ID1].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  #pragma omp for
  # endif
  for (ii = 0; ii < nelement; ii++)
    data.image[IDout].array.CD[ii] = pt2function(data.image[ID1].array.CD[ii], data.image[ID2].array.CD[ii]);
  # ifdef _OPENMP
  }
  # endif

  return(0);
}




complex_double CPadd_CD_CD(complex_double a, complex_double b) {complex_double v; v.re=a.re+b.re; v.im=a.im+b.im; return(v);}

complex_double CPsub_CD_CD(complex_double a, complex_double b) {complex_double v; v.re=a.re-b.re; v.im=a.im-b.im; return(v);}

complex_double CPmult_CD_CD(complex_double a, complex_double b) {complex_double v; v.re=a.re*b.re-a.im*b.im; v.im=a.re*b.im+a.im*b.re; return(v);}

complex_double CPdiv_CD_CD(complex_double a, complex_double b) 
{
  complex_double v; 
  double amp,pha; 
  double are, aim, bre, bim;
  
  are = a.re;
  aim = a.im;
  bre = b.re;
  bim = b.im;

  amp = sqrt(are*are+aim*aim);
  amp /= sqrt(bre*bre+bim*bim); 
  pha = atan2(aim,are);
  pha -= atan2(bim,bre); 

  v.re = (double) (amp*cos(pha)); 
  v.im = (double) (amp*sin(pha)); 

  return(v);
}

complex_float CPadd_CF_CF(complex_float a, complex_float b) {complex_float v; v.re=a.re+b.re; v.im=a.im+b.im; return(v);}

complex_float CPsub_CF_CF(complex_float a, complex_float b) {complex_float v; v.re=a.re-b.re; v.im=a.im-b.im; return(v);}

complex_float CPmult_CF_CF(complex_float a, complex_float b) {complex_float v; v.re=a.re*b.re-a.im*b.im; v.im=a.re*b.im+a.im*b.re; return(v);}

complex_float CPdiv_CF_CF(complex_float a, complex_float b) 
{
  complex_float v; 
  float amp,pha; 
  float are, aim, bre, bim;
  
  are = a.re;
  aim = a.im;
  bre = b.re;
  bim = b.im;

  amp = sqrt(are*are+aim*aim);
  amp /= sqrt(bre*bre+bim*bim); 
  pha = atan2(aim,are);
  pha -= atan2(bim,bre); 

  v.re = (float) (amp*cos(pha)); 
  v.im = (float) (amp*sin(pha)); 

  return(v);
}

int arith_image_Cadd(char *ID1_name, char *ID2_name, char *ID_out){ 
  int atype1, atype2;
  long ID1, ID2;

  ID1 = image_ID(ID1_name);
  ID2 = image_ID(ID2_name);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;

  if((atype1 == COMPLEX_FLOAT)&&(atype2 ==  COMPLEX_FLOAT))
    {
      arith_image_function_CF_CF__CF(ID1_name, ID2_name, ID_out, &CPadd_CF_CF); 
      return 0;
    }

  if((atype1 == COMPLEX_DOUBLE)&&(atype2 ==  COMPLEX_DOUBLE))
    {
      arith_image_function_CD_CD__CD(ID1_name, ID2_name, ID_out, &CPadd_CD_CD); 
      return 0;
    }
  printERROR(__FILE__,__func__,__LINE__,"data types do not match");

  return 1;
}

int arith_image_Csub(char *ID1_name, char *ID2_name, char *ID_out){ 
  int atype1, atype2;
  long ID1, ID2;

  ID1 = image_ID(ID1_name);
  ID2 = image_ID(ID2_name);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;

  if((atype1 == COMPLEX_FLOAT)&&(atype2 ==  COMPLEX_FLOAT))
    {
      arith_image_function_CF_CF__CF(ID1_name, ID2_name, ID_out, &CPsub_CF_CF); 
      return 0;
    }

  if((atype1 == COMPLEX_DOUBLE)&&(atype2 ==  COMPLEX_DOUBLE))
    {
      arith_image_function_CD_CD__CD(ID1_name, ID2_name, ID_out, &CPsub_CD_CD); 
      return 0;
    }
  printERROR(__FILE__,__func__,__LINE__,"data types do not match");

  return 1;
}

int arith_image_Cmult(char *ID1_name, char *ID2_name, char *ID_out){ 
  int atype1, atype2;
  long ID1, ID2;

  ID1 = image_ID(ID1_name);
  ID2 = image_ID(ID2_name);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;

  if((atype1 == COMPLEX_FLOAT)&&(atype2 ==  COMPLEX_FLOAT))
    {
      arith_image_function_CF_CF__CF(ID1_name, ID2_name, ID_out, &CPmult_CF_CF); 
      return 0;
    }

  if((atype1 == COMPLEX_DOUBLE)&&(atype2 ==  COMPLEX_DOUBLE))
    {
      arith_image_function_CD_CD__CD(ID1_name, ID2_name, ID_out, &CPmult_CD_CD); 
      return 0;
    }
  printERROR(__FILE__,__func__,__LINE__,"data types do not match");

  return(0);
}

int arith_image_Cdiv(char *ID1_name, char *ID2_name, char *ID_out){ 
  int atype1, atype2;
  long ID1, ID2;

  ID1 = image_ID(ID1_name);
  ID2 = image_ID(ID2_name);
  atype1 = data.image[ID1].md[0].atype;
  atype2 = data.image[ID2].md[0].atype;

  if((atype1 == COMPLEX_FLOAT)&&(atype2 ==  COMPLEX_FLOAT))
    {
      arith_image_function_CF_CF__CF(ID1_name, ID2_name, ID_out, &CPdiv_CF_CF); 
      return 0;
    }

  if((atype1 == COMPLEX_DOUBLE)&&(atype2 ==  COMPLEX_DOUBLE))
    {
      arith_image_function_CD_CD__CD(ID1_name, ID2_name, ID_out, &CPdiv_CD_CD); 
      return 0;
    }
  printERROR(__FILE__,__func__,__LINE__,"data types do not match");

  return(0);
}










/* ------------------------------------------------------------------------- */
/* image, double  -> image                                                */
/* ------------------------------------------------------------------------- */


int arith_image_function_1f_1(char *ID_name, double f1, char *ID_out, double (*pt2function)(double,double))
{
  long ID;
  long IDout;
  long ii;
  long *naxes = NULL;
  long nelement;
  long naxis;
  int atype, atypeout;
  long i;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxis=data.image[ID].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[ID].md[0].size[i];
    }
  
  atypeout = FLOAT;
  if(atype == DOUBLE)
    atypeout = DOUBLE;

  IDout = create_image_ID(ID_out, naxis, naxes, atype, data.SHARED_DFT, data.NBKEWORD_DFT);

  free(naxes);
  nelement = data.image[ID].md[0].nelement;
  


  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {  
  # endif
  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]),f1);
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]),f1);
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]),f1);
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]),f1);
    }
  # ifdef _OPENMP
  }
  # endif



  return(0);
}




int arith_image_function_1f_1_inplace(char *ID_name, double f1, double (*pt2function)(double,double))
{
  long ID;
  long ii;
  long nelement;
  int atype;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  nelement = data.image[ID].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  # endif
  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]),f1);
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]),f1);
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]),f1);
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]),f1);
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}



// by ID ... slightly faster
int arith_image_function_1f_1_inplace_byID(long ID, double f1, double (*pt2function)(double,double))
{
  long ii;
  long nelement;
  int atype;

  atype = data.image[ID].md[0].atype;
  nelement = data.image[ID].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  # endif
  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]),f1);
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]),f1);
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]),f1);
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.D[ii] = (double) pt2function((double) (data.image[ID].array.D[ii]),f1);
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}









int arith_image_cstfmod(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Pfmod); return(0);}
int arith_image_cstadd(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Padd); return(0);}
int arith_image_cstsub(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Psub); return(0);}
int arith_image_cstsubm(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Psubm); return(0);}
int arith_image_cstmult(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Pmult); return(0);}
int arith_image_cstdiv(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Pdiv); return(0);}
int arith_image_cstdiv1(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Pdiv1); return(0);}
int arith_image_cstpow(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Ppow); return(0);}
int arith_image_cstmaxv(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Pmaxv); return(0);}
int arith_image_cstminv(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Pminv); return(0);}
int arith_image_csttestlt(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Ptestlt); return(0);}
int arith_image_csttestmt(char *ID_name, double f1, char *ID_out){ arith_image_function_1f_1(ID_name,f1,ID_out,&Ptestmt); return(0);}

int arith_image_cstfmod_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Pfmod); return(0);}
int arith_image_cstadd_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Padd); return(0);}
int arith_image_cstsub_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Psub); return(0);}
int arith_image_cstmult_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Pmult); return(0);}
int arith_image_cstdiv_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Pdiv); return(0);}
int arith_image_cstdiv1_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Pdiv1); return(0);}
int arith_image_cstpow_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Ppow); return(0);}
int arith_image_cstmaxv_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Pmaxv); return(0);}
int arith_image_cstminv_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Pminv); return(0);}
int arith_image_csttestlt_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Ptestlt); return(0);}
int arith_image_csttestmt_inplace(char *ID_name, double f1){ arith_image_function_1f_1_inplace(ID_name,f1,&Ptestmt); return(0);}

int arith_image_cstfmod_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Pfmod); return(0);}
int arith_image_cstadd_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Padd); return(0);}
int arith_image_cstsub_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Psub); return(0);}
int arith_image_cstmult_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Pmult); return(0);}
int arith_image_cstdiv_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Pdiv); return(0);}
int arith_image_cstdiv1_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Pdiv1); return(0);}
int arith_image_cstpow_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Ppow); return(0);}
int arith_image_cstmaxv_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Pmaxv); return(0);}
int arith_image_cstminv_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Pminv); return(0);}
int arith_image_csttestlt_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Ptestlt); return(0);}
int arith_image_csttestmt_inplace_byID(long ID, double f1){ arith_image_function_1f_1_inplace_byID(ID,f1,&Ptestmt); return(0);}





/* ------------------------------------------------------------------------- */
/* image, double, double -> image                                      */
/* ------------------------------------------------------------------------- */


int arith_image_function_1ff_1(char *ID_name, double f1, double f2, char *ID_out, double (*pt2function)(double,double,double))
{
  long ID;
  long IDout;
  long ii;
  long *naxes = NULL;
  long nelement;
  long naxis;
  int atype;
  long i;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  naxis=data.image[ID].md[0].naxis;
  naxes = (long*) malloc(sizeof(long)*naxis);
  if(naxes==NULL)
     {
       printERROR(__FILE__,__func__,__LINE__,"malloc() error");
       exit(0);
     }

  for(i=0;i<naxis;i++)
    {
      naxes[i] = data.image[ID].md[0].size[i];
    }
  
  IDout = create_image_ID(ID_out, naxis, naxes, atype, data.SHARED_DFT, data.NBKEWORD_DFT);
  free(naxes);
  nelement = data.image[ID].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  # endif
  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]),f1,f2);
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]),f1,f2);
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]),f1,f2);
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[IDout].array.F[ii] = pt2function((double) (data.image[ID].array.D[ii]),f1,f2);
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}

int arith_image_function_1ff_1_inplace(char *ID_name, double f1, double f2, double (*pt2function)(double,double,double))
{
  long ID;
  long ii;
  long nelement;
  int atype;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;
  nelement = data.image[ID].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  # endif
  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]),f1,f2);
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]),f1,f2);
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]),f1,f2);
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.D[ii]),f1,f2);
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}

int arith_image_function_1ff_1_inplace_byID(long ID, double f1, double f2, double (*pt2function)(double,double,double))
{
  long ii;
  long nelement;
  int atype;

  atype = data.image[ID].md[0].atype;
  nelement = data.image[ID].md[0].nelement;
  
  # ifdef _OPENMP
  #pragma omp parallel if (nelement>OMP_NELEMENT_LIMIT)
  {
  # endif
  if(atype==CHAR)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.C[ii]),f1,f2);
    }
  if(atype==INT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.I[ii]),f1,f2);
    }
  if(atype==FLOAT)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.F[ii]),f1,f2);
    }
  if(atype==DOUBLE)
    {
      # ifdef _OPENMP
      #pragma omp for
      # endif 
      for (ii = 0; ii < nelement; ii++)
	data.image[ID].array.F[ii] = pt2function((double) (data.image[ID].array.D[ii]),f1,f2);
    }
  # ifdef _OPENMP
  }
  # endif

  return(0);
}


double Ptrunc(double a, double b, double c) {double value; value=a; if(a<b){value=b;}; if(a>c){value=c;}; return(value);}
 
int arith_image_trunc(char *ID_name, double f1, double f2, char *ID_out){ arith_image_function_1ff_1(ID_name, f1, f2, ID_out, &Ptrunc); return(0);}

int arith_image_trunc_inplace(char *ID_name, double f1, double f2) { arith_image_function_1ff_1_inplace(ID_name,f1,f2,&Ptrunc); return(0);}
int arith_image_trunc_inplace_byID(long ID, double f1, double f2) { arith_image_function_1ff_1_inplace_byID(ID,f1,f2,&Ptrunc); return(0);}



int isoperand(char *word)
{
  int value = 0;
  
  if (strcmp(word,"+")==0)
    value = 1;
  if (strcmp(word,"-")==0)
    value = 1;
  if (strcmp(word,"/")==0)
    value = 1;
  if (strcmp(word,"*")==0)
    value = 1;
  if (strcmp(word,"^")==0)
    value = 1;

  return(value);
}

int isfunction(char *word)
{
  int value = 0;

  if (strcmp(word,"acos")==0)
    value = 1;
  if (strcmp(word,"asin")==0)
    value = 1;
  if (strcmp(word,"atan")==0)
    value = 1;
  if (strcmp(word,"ceil")==0)
    value = 1;
  if (strcmp(word,"cos")==0)
    value = 1;
  if (strcmp(word,"cosh")==0)
    value = 1;
  if (strcmp(word,"exp")==0)
    value = 1;
  if (strcmp(word,"fabs")==0)
    value = 1;
  if (strcmp(word,"floor")==0)
    value = 1;
  if (strcmp(word,"imedian")==0)
    value = 1;
  if (strcmp(word,"itot")==0)
    value = 1;
  if (strcmp(word,"imean")==0)
    value = 1;
  if (strcmp(word,"imin")==0)
    value = 1;
  if (strcmp(word,"imax")==0)
    value = 1;
  if (strcmp(word,"ln")==0)
    value = 1;
  if (strcmp(word,"log")==0)
    value = 1;
  if (strcmp(word,"sqrt")==0)
    value = 1;
  if (strcmp(word,"sin")==0)
    value = 1;
  if (strcmp(word,"sinh")==0)
    value = 1;
  if (strcmp(word,"tan")==0)
    value = 1;
  if (strcmp(word,"tanh")==0)
    value = 1;
  if (strcmp(word,"posi")==0)
    value = 1;
  if (strcmp(word,"imdx")==0)
    value = 1;
  if (strcmp(word,"imdy")==0)
    value = 1;


/*  if (!strcmp(word,"pow"))
    value = 1;
  if (!strcmp(word,"max"))
    value = 1;
  if (!strcmp(word,"min"))
    value = 1;
  if (!strcmp(word,"median"))
    value = 1;
*/
  return(value);
}

int isfunction_sev_var(char *word)
{
  int value = 0; /* number of input variables */

  if (strcmp(word,"fmod")==0)
    value = 2;
  if (strcmp(word,"trunc")==0)
    value = 3;
  if (strcmp(word,"perc")==0)
    value = 2;
  if (strcmp(word,"min")==0)
    value = 2;
  if (strcmp(word,"max")==0)
    value = 2;
  if (strcmp(word,"testlt")==0)
    value = 2;
  if (strcmp(word,"testmt")==0)
    value = 2;
 

  return(value);
}


int isanumber(char *word)
{
  int value = 1; // 1 if number, 0 otherwise
  char *endptr;
  double v1;

  v1 = strtod(word,&endptr);
  if( (long) (endptr-word) == (long) strlen(word))
    value = 1;
  else
    value = 0;

  return(value);
}


/*int isanumber(char *word)
{
  int value = 1;
  unsigned int i;
  int passed_point=0;

  for(i=0;i<strlen(word);i++)
    {
      if(!isdigit(word[i]))
	{
	  if(word[i]=='.')
	    {
	      if (passed_point==0)
		passed_point = 1;
	      else
		value = 0;
	    }
	  else
	    value = 0;
	}
    }

  return(value);
}
*/


long arith_make_slopexy(char *ID_name, long l1,long l2, double sx, double sy)
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





/*^-----------------------------------------------------------------------------
| int 
| execute_arith : 
|     char *cmd : 
|
|
|    0 : unknown
|    1 : non-existing variable or image
|    2 : existing variable
|    3 : number
|    4 : operand
|    5 : opening brace
|    6 : closing brace
|    7 : coma
|    8 : function
|    9 : equal sign
|    10 : existing image
|    11 : function of several variables/images, returning one variable/image
|
|
+-----------------------------------------------------------------------------*/
int execute_arith( char *cmd1 )
{
  char word[100][100];
  int i,w,l,j;
  int nbword;
  int word_type[100];
  int par_level[100];
  int parlevel;
  int intr_priority[100]; /* 0 (+,-)  1 (*,/)  2 (functions) */


  int found_word_type;
  int highest_parlevel;
  int highest_intr_priority;
  int highest_priority_index;
  int passedequ;
  int tmp_name_index;
  char name[SBUFFERSIZE];
  char name1[SBUFFERSIZE];
  double tmp_prec;
  int nb_tbp_word;
  int type=0;
  int nbvarinput;
  
  char cmd[SBUFFERSIZE];
  long cntP;
  int OKea = 1;
  int n;

  //  if( Debug > 0 )   fprintf(stdout, "[execute_arith]\n");
  //  if( Debug > 0 )   fprintf(stdout, "[execute_arith] str: [%s]\n", cmd1);

  for (i=0;i<100;i++)
    {
      word_type[i] = 0;
      par_level[i] = 0;
      intr_priority[i] = 0;
    }
  

  /* 
     Pre-process string: 
     - remove any spaces in cmd1
     - replace "=-" by "=0-" and "=+" by "="
     copy result into cmd */
  j = 0;
  
  for(i=0;i<(int) (strlen(cmd1));i++)
    {
      if((cmd1[i]=='=')&&(cmd1[i+1]=='-'))
	{
	  cmd[j] = '=';
	  j++;
	  cmd[j] = '0';
	  j++;
	}
      else if((cmd1[i]=='=')&&(cmd1[i+1]=='+'))
	{
	  cmd[j] = '=';
	  j++;
	  i++;
	}
      else if(cmd1[i]!=' ')
	{
	  cmd[j] = cmd1[i];
	  j++;
	}      
    }
  cmd[j] = '\0';
  //  if( Debug > 0 )   fprintf(stdout, "[execute_arith] preprocessed str %s -> %s\n", cmd1, cmd);



  /*
  * cmd is first broken into words. 
  * The spacing between words is operands (+,-,/,*), equal (=), 
  * space ,comma and braces 
  */
  w = 0;
  l = 0;
  for (i=0;i<(signed) strlen(cmd);i++)
    {
      switch (cmd[i]) {

      case '+': case '-':
	if( ((cmd[i-1]=='e')||(cmd[i-1]=='E')) && (isdigit(cmd[i-2])) && (isdigit(cmd[i+1])) )
	  {
	    // + or - is part of exponent
	    word[w][l] = cmd[i];
	    l++;
	  }
	else
	  {
	    if(l>0)
	      {
		word[w][l] = '\0';
		w++;
	      }
	    l = 0;
	    word[w][l] = cmd[i]; 
	    word[w][1] = '\0';
	    if (i<(signed) (strlen(cmd)-1))
	      w++;
	    l = 0;
	  }
	break;

      case '*': case '/': case '^': case '(': case ')': case '=': case ',':
	if(l>0)
	  {
	    word[w][l] = '\0';
	    w++;
	  }
	l = 0;
	word[w][l] = cmd[i]; 
	word[w][1] = '\0';
	if (i<(signed) (strlen(cmd)-1))
	  w++;
	l = 0;
	break;

      case ' ':
	word[w][l] = '\0';
	  w++;
	  l = 0;

	/*word[w][l] = '\0';
	  w++;
	  l = 0;*/
	break;

      default:
	word[w][l] = cmd[i];
	l++;
	break;
      }
    }


  if (l>0)
    word[w][l] = '\0';
  nbword = w+1;


  //  printf("number of words is %d\n",nbword);

  for (i=0;i<nbword;i++)
    {
      //      if( Debug > 0 )
      //	printf("TESTING WORD %d = %s\n",i,word[i]);
      word_type[i] = 0;
      found_word_type = 0;
      if((isanumber(word[i])==1)&&(found_word_type==0))
	{
	  word_type[i] = 3;
	  found_word_type = 1;
	}
      if((isfunction(word[i])==1)&&(found_word_type==0))
	{
	  word_type[i] = 8;
	  found_word_type = 1;
	}
      if((isfunction_sev_var(word[i])!=0)&&(found_word_type==0))
	{
	  word_type[i] = 11;
	  found_word_type = 1;
	}
      if((isoperand(word[i])==1)&&(found_word_type==0))
	{
	  word_type[i] = 4;
	  found_word_type = 1;
	}
      if((strcmp(word[i],"=")==0)&&(found_word_type==0))
	{
	  word_type[i] = 9;
	  found_word_type = 1;
	}
      if((strcmp(word[i],",")==0)&&(found_word_type==0))
	{
	  word_type[i] = 7;
	  found_word_type = 1;
	}
      if((i<nbword-1)&&(found_word_type==0))
	{
	  if((strcmp(word[i+1],"(")==0)&&(isfunction(word[i])==1))
	    {
	      word_type[i] = 8;
	      found_word_type = 1;
	    }	
	}
      if((strcmp(word[i],"(")==0)&&(found_word_type==0))
	{
	  word_type[i] = 5;
	  found_word_type = 1;
	}
      if((strcmp(word[i],")")==0)&&(found_word_type==0))
	{
	  word_type[i] = 6;
	  found_word_type = 1;
	}
      if((variable_ID(word[i])!=-1)&&(found_word_type==0))
	{
	  word_type[i] = 2;
	  found_word_type = 1;
	}
      if((image_ID(word[i])!=-1)&&(found_word_type==0))
	{
	  word_type[i] = 10;
	  found_word_type = 1;
	}
      if(found_word_type==0)
	word_type[i] = 1;
      //        if( Debug > 0 ) printf("word %d is  \"%s\" word typ is %d\n",i,word[i],word_type[i]);
    }
  








  /* checks for obvious errors */

  passedequ = 0;
  for (i=(nbword-1);i>-1;i--)
    {
     if(passedequ == 1)
	{
	  if(word_type[i]==9)
	    {
	      n = snprintf(errmsg,SBUFFERSIZE,"line has multiple \"=\"");
	      if(n >= SBUFFERSIZE) 
		printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");	      printWARNING(__FILE__,__func__,__LINE__,errmsg);
	      OKea = 0;
	    }
	  if(word_type[i]==4)
	    {
	      n = snprintf(errmsg,SBUFFERSIZE,"operand on left side of \"=\"");
	      if(n >= SBUFFERSIZE) 
		printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters"); 
	      printWARNING(__FILE__,__func__,__LINE__,errmsg);
	      OKea = 0;
	    }
	  if(word_type[i]==5)
	    {
	      n = snprintf(errmsg,SBUFFERSIZE,"\"(\" on left side of \"=\"");
	      if(n >= SBUFFERSIZE) 
		printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	      printWARNING(__FILE__,__func__,__LINE__,errmsg);
	      OKea = 0;
	    }
	  if(word_type[i]==6)
	    {
	      n = snprintf(errmsg,SBUFFERSIZE,"\")\" on left side of \"=\"");
	      if(n >= SBUFFERSIZE) 
		printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	      printWARNING(__FILE__,__func__,__LINE__,errmsg);
	      OKea = 0;
	    }
	}
      if(word_type[i]==9)
	passedequ = 1;
      if ((passedequ==0)&&(word_type[i]==1)) /* non-existing variable or image as input */
	{
	  n  = snprintf(errmsg,SBUFFERSIZE,"%s is a non-existing variable or image",word[i]);
	  if(n >= SBUFFERSIZE) 
	    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	  printWARNING(__FILE__,__func__,__LINE__,errmsg);
	  OKea = 0;
	}      
    }

  for (i=0;i<nbword-1;i++)
    {
      if((word_type[i]==4)&&(word_type[i+1]==4))
	{
	  printWARNING(__FILE__,__func__,__LINE__,"consecutive operands");
	  OKea = 0;
	}
      if((word_type[i+1]==5)&& (!((word_type[i]==5)||(word_type[i]==8)||(word_type[i]==11)||(word_type[i]==9)||(word_type[i]==4))))
	{
	  printWARNING(__FILE__,__func__,__LINE__,"\"(\" should be preceeded by \"=\", \"(\", operand or function");
	  OKea = 0;
	} 
    }

  cntP = 0;
  for (i=0;i<nbword;i++)
    {
      if (word_type[i]==5)
	cntP ++;
      if (word_type[i]==6)
	cntP --;
      if(cntP<0)
	{
	  printWARNING(__FILE__,__func__,__LINE__,"parentheses error");
	  OKea = 0;
	}  
    }
  if(cntP!=0)
    {
      printWARNING(__FILE__,__func__,__LINE__,"parentheses error");
      OKea = 0;
    }  


  







  if(OKea == 1)
    {
      /* numbers are saved into variables */
      tmp_name_index = 0;
      for (i=0;i<nbword;i++)
	{
	  if(word_type[i]==3)
	    {
	      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); if(n >= SBUFFERSIZE) printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	      if(n >= SBUFFERSIZE) 
		printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	      /*	  if(sizeof(double)==sizeof(float))
		else*/
	      create_variable_ID(name,1.0*strtod(word[i],NULL));
	      strcpy(word[i],name);
	      word_type[i] = 2;
	      tmp_name_index++;
	    }
	}
      
      /* computing the number of to-be-processed words */
      passedequ = 0;
      nb_tbp_word = 0;
      for (i=(nbword-1);i>-1;i--)
	{
	  if(word_type[i]==9)
	    passedequ = 1;
	  if(passedequ==0)
	    nb_tbp_word++;
	}
      
      /* main loop starts here */
      while(nb_tbp_word>1)
	{
	  /* non necessary braces are removed 
	   */
	  for (i=0;i<nbword-2;i++)
	    if ((word_type[i]==5)&&(word_type[i+2]==6))
	      {
		strcpy(word[i],word[i+1]);
		word_type[i] = word_type[i+1];
		for(j=i+1;j<nbword-2;j++)
		  {
		    strcpy(word[j],word[j+2]);
		    word_type[j] = word_type[j+2];
		  }
		nbword = nbword-2;
	      }
	  for (i=0;i<nbword-3;i++)
	    if ((word_type[i]==5)&&(word_type[i+3]==6)&&(strcmp(word[i+1],"-")==0))
	      {
		data.variable[variable_ID(word[i+2])].value.f = -data.variable[variable_ID(word[i+2])].value.f;
		strcpy(word[i],word[i+2]);
		word_type[i] = word_type[i+2];
		for(j=i+2;j<nbword-3;j++)
		  {
		    strcpy(word[j],word[j+3]);
		    word_type[j] = word_type[j+3];
		  }
		nbword = nbword-3;
	      }
	  
	  /* now the priorities are given */
	  
	  parlevel = 0;
	  for (i=0;i<nbword;i++)
	    {
	      if (word_type[i]==5)
		parlevel++;
	      if (word_type[i]==6)
		parlevel--;
	      if ((word_type[i]==4)||(word_type[i]==8)||(word_type[i]==11))
		{
		  par_level[i] = parlevel;
		  if (word_type[i]==8)
		    intr_priority[i] = 2;
		  if (word_type[i]==11)
		    intr_priority[i] = 2;
		  if (word_type[i]==4)
		    {
		      if ((strcmp(word[i],"+")==0)||(strcmp(word[i],"-")==0))
			intr_priority[i] = 0;
		      if ((strcmp(word[i],"*")==0)||(strcmp(word[i],"/")==0))
			intr_priority[i] = 1;
		    }
		}
	    }
	  
	  /* the highest priority operation is executed */
	  highest_parlevel = 0;
	  highest_intr_priority = -1;
	  highest_priority_index = -1;
	  
	  for (i=0;i<nbword;i++)
	    {
	      if ((word_type[i]==4)||(word_type[i]==8)||(word_type[i]==11))
		{
		  /*printf("operation \"%s\" (%d,%d)\n",word[i],par_level[i],intr_priority[i]);*/
		  if (par_level[i]>highest_parlevel)
		    {
		      highest_priority_index = i;
		      highest_parlevel = par_level[i];
		      highest_intr_priority = 0;
		    }
		  else
		    {
		      if ((par_level[i]==highest_parlevel)&&(intr_priority[i]>highest_intr_priority))
			{
			  highest_priority_index = i;
			  highest_intr_priority = intr_priority[i];
			}
		    }
		}
	    }
	  
	  /*      printf("executing operation  %s\n",word[highest_priority_index]);*/
	  
	  i = highest_priority_index;
	  
	  /*      printf("before : ");
		  for (j=0;j<nbword;j++)
		  {
		  if(j==i)
		  printf(">>");
		  if(variable_ID(word[j])!=-1)
		  printf(" %s(%f) ",word[j],data.variable[variable_ID(word[j])].value.f);
		  else
		  printf(" %s ",word[j]);	
		  }
		  printf("\n");
	  */
	  if (word_type[i]==4)
	    {
	      if(strcmp(word[i],"+")==0)
		{
		  if((word_type[i-1]==2)&&(word_type[i+1]==2))
		    {
		      tmp_prec = data.variable[variable_ID(word[i-1])].value.f+data.variable[variable_ID(word[i+1])].value.f;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index,(int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i-1]==2)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index,(int) getpid()); 
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstadd(word[i+1],(double) data.variable[variable_ID(word[i-1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index,(int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstadd(word[i-1],(double) data.variable[variable_ID(word[i+1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index,(int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_add(word[i-1],word[i+1],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      if(strcmp(word[i],"-")==0)
		{
		  if((word_type[i-1]==2)&&(word_type[i+1]==2))
		    {
		      tmp_prec = data.variable[variable_ID(word[i-1])].value.f-data.variable[variable_ID(word[i+1])].value.f;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i-1]==2)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      n = snprintf(name1,SBUFFERSIZE,"_tmp1%d_%d",tmp_name_index, (int) getpid()); if(n >= SBUFFERSIZE) printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstsub(word[i+1],(double) data.variable[variable_ID(word[i-1])].value.f,name1);
		      arith_image_cstmult(name1, (double) -1.0, name);
		      delete_image_ID(name1);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstsub(word[i-1],(double) data.variable[variable_ID(word[i+1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_sub(word[i-1],word[i+1],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      if(strcmp(word[i],"*")==0)
		{
		  if((word_type[i-1]==2)&&(word_type[i+1]==2))
		    {
		      tmp_prec = data.variable[variable_ID(word[i-1])].value.f*data.variable[variable_ID(word[i+1])].value.f;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i-1]==2)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstmult(word[i+1],(double) data.variable[variable_ID(word[i-1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstmult(word[i-1],(double) data.variable[variable_ID(word[i+1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_mult(word[i-1],word[i+1],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      if(strcmp(word[i],"/")==0)
		{
		  if((word_type[i-1]==2)&&(word_type[i+1]==2))
		    {
		      tmp_prec = data.variable[variable_ID(word[i-1])].value.f/data.variable[variable_ID(word[i+1])].value.f;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i-1]==2)&&(word_type[i+1]==10))
		    {
		      //    printf("CASE 1\n");
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstdiv1(word[i+1],(double) data.variable[variable_ID(word[i-1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstdiv(word[i-1],(double) data.variable[variable_ID(word[i+1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_div(word[i-1],word[i+1],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      if(strcmp(word[i],"^")==0)
		{
		  if((word_type[i-1]==2)&&(word_type[i+1]==2))
		    {
		      if(data.variable[variable_ID(word[i+1])].value.f<0)
			{
			  tmp_prec = pow(data.variable[variable_ID(word[i-1])].value.f,-data.variable[variable_ID(word[i+1])].value.f);
			  tmp_prec = 1.0/tmp_prec;
			}
		      else
			tmp_prec = pow(data.variable[variable_ID(word[i-1])].value.f,data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i-1]==2)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      n = snprintf(name1,SBUFFERSIZE,"_tmp1%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstadd(word[i+1],(double) data.variable[variable_ID(word[i-1])].value.f,name1);
		      arith_image_pow(name1,word[i+1],name);
		      delete_image_ID(name1);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstpow(word[i-1],(double) data.variable[variable_ID(word[i+1])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i-1]==10)&&(word_type[i+1]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_pow(word[i-1],word[i+1],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      strcpy(word[i-1],name);
	      word_type[i-1] = type;
	      for (j=i;j<nbword-2;j++)
		{
		  strcpy(word[j],word[j+2]);
		  word_type[j] = word_type[j+2];
		}
	      nbword = nbword - 2;
	    }
	  
	  if (word_type[i]==8)
	    {
	      if(strcmp(word[i],"acos")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = acos(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_acos(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"asin")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = asin(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_asin(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"atan")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = atan(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_atan(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"ceil")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = (double) ceil(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_ceil(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"cos")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = cos(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cos(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"cosh")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = cosh(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cosh(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"exp")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = exp(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_exp(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"fabs")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = fabs(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index,(int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_fabs(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}

	      if(strcmp(word[i],"floor")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = floor(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_floor(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"imedian")==0)
		{
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_prec = arith_image_median(word[i+1]);
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		}

	      if(strcmp(word[i],"itot")==0)
		{
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_prec = arith_image_total(word[i+1]);
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		}

	      if(strcmp(word[i],"imean")==0)
		{
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_prec = arith_image_mean(word[i+1]);
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		}
	      
	      if(strcmp(word[i],"imin")==0)
		{
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_prec = arith_image_min(word[i+1]);
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		}
	      
	      if(strcmp(word[i],"imax")==0)
		{
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_prec = arith_image_max(word[i+1]);
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		}
	      
	      if(strcmp(word[i],"ln")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = log(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_ln(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"log")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = log10(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_log(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"sqrt")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = sqrt(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_sqrt(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"sin")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = sin(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_sin(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	  
	      if(strcmp(word[i],"sinh")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = sinh(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_sinh(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"tan")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = tan(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_tan(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	  
	      if(strcmp(word[i],"tanh")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = tanh(data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_tanh(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}
	      
	      if(strcmp(word[i],"posi")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      tmp_prec = Ppositive( (double) data.variable[variable_ID(word[i+1])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE)
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type=2;
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_positive(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}

	      if(strcmp(word[i],"imdx")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      printERROR(__FILE__,__func__,__LINE__,"Function imdx only applicable on images");
		      exit(0);
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_dx(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}

	      if(strcmp(word[i],"imdy")==0)
		{
		  if(word_type[i+1]==2)
		    {
		      printERROR(__FILE__,__func__,__LINE__,"Function imdy only applicable on images");
		      exit(0);
		    }
		  if(word_type[i+1]==10)
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_dy(word[i+1],name);
		      tmp_name_index++;
		      type=10;
		    }
		}

	      
	      strcpy(word[i],name);
	      word_type[i] = type;
	      for (j=i+1;j<nbword-1;j++)
		{
		  strcpy(word[j],word[j+1]);
		  word_type[j] = word_type[j+1];		
		}
	      nbword = nbword - 1;
	    }	
	  
	  if (word_type[i]==11)
	    {
	      nbvarinput = isfunction_sev_var(word[i]);
	      
	      if(strcmp(word[i],"fmod")==0)
		{
		  if((word_type[i+2]==2)&&(word_type[i+4]==2))
		    {
		      tmp_prec = fmod(data.variable[variable_ID(word[i+2])].value.f,data.variable[variable_ID(word[i+4])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i+2]==2)&&(word_type[i+4]==10))
		    {
		      printf("function not available\n");		  
		    }
		  if((word_type[i+2]==10)&&(word_type[i+4]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstfmod(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i+2]==10)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid()); 
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_fmod(word[i+2],word[i+4],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      
	      if(strcmp(word[i],"min")==0)
		{
		  if((word_type[i+2]==2)&&(word_type[i+4]==2))
		    {
		      if(data.variable[variable_ID(word[i+2])].value.f<data.variable[variable_ID(word[i+4])].value.f)
			tmp_prec = data.variable[variable_ID(word[i+2])].value.f;
		      else
			tmp_prec = data.variable[variable_ID(word[i+4])].value.f;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  if((word_type[i+2]==2)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstminv(word[i+4], (double) data.variable[variable_ID(word[i+2])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i+2]==10)&&(word_type[i+4]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstminv(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  if((word_type[i+2]==10)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_minv(word[i+2],word[i+4],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      

	      if(strcmp(word[i],"max")==0)
		{
		  if((word_type[i+2]==2)&&(word_type[i+4]==2))
		    {
		      if(data.variable[variable_ID(word[i+2])].value.f>data.variable[variable_ID(word[i+4])].value.f)
			tmp_prec = data.variable[variable_ID(word[i+2])].value.f;
		      else
			tmp_prec = data.variable[variable_ID(word[i+4])].value.f;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  else if((word_type[i+2]==2)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstmaxv(word[i+4], (double) data.variable[variable_ID(word[i+2])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else if((word_type[i+2]==10)&&(word_type[i+4]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_cstmaxv(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else if((word_type[i+2]==10)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_maxv(word[i+2],word[i+4],name);
		      tmp_name_index++;
		      type = 10;
		    }
		}
	      
	      if(strcmp(word[i],"testlt")==0)
		{
		  if((word_type[i+2]==2)&&(word_type[i+4]==2))
		    {
		      if(data.variable[variable_ID(word[i+2])].value.f>data.variable[variable_ID(word[i+4])].value.f)
			tmp_prec = 0.0;
		      else
			tmp_prec = 1.0;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  else if((word_type[i+2]==2)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_csttestmt(word[i+4], (double) data.variable[variable_ID(word[i+2])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else if((word_type[i+2]==10)&&(word_type[i+4]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_csttestlt(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else if((word_type[i+2]==10)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_testlt(word[i+2],word[i+4],name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else		   
		    printERROR(__FILE__,__func__,__LINE__,"Wrong input to function testlt");
		}

	      if(strcmp(word[i],"testmt")==0)
		{
		  if((word_type[i+2]==2)&&(word_type[i+4]==2))
		    {
		      if(data.variable[variable_ID(word[i+2])].value.f>data.variable[variable_ID(word[i+4])].value.f)
			tmp_prec = 1.0;
		      else
			tmp_prec = 0.0;
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		  else if((word_type[i+2]==2)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_csttestlt(word[i+4], (double) data.variable[variable_ID(word[i+2])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else if((word_type[i+2]==10)&&(word_type[i+4]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_csttestmt(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f,name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else if((word_type[i+2]==10)&&(word_type[i+4]==10))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      arith_image_testmt(word[i+2],word[i+4],name);
		      tmp_name_index++;
		      type = 10;
		    }
		  else		   
		    printERROR(__FILE__,__func__,__LINE__,"Wrong input to function testlt");
		}

	        
	      if(strcmp(word[i],"perc")==0)
		{
		  //	      printf("%d %d\n",word_type[i+2],word_type[i+4]);
		  if((word_type[i+2]!=10)||(word_type[i+4]!=2))		    
		    printERROR(__FILE__,__func__,__LINE__,"Wrong input to function perc\n");
		  else
		    {
		      //		  printf("Running percentile args = %s %f\n",word[i+2],data.variable[variable_ID(word[i+4])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_prec = arith_image_percentile(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f);
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      create_variable_ID(name,tmp_prec);
		      tmp_name_index++;
		      type = 2;
		    }
		}
	      
	      if(strcmp(word[i],"trunc")==0)
		{
		  if((word_type[i+2]==10)&&(word_type[i+4]==2)&&(word_type[i+6]==2))
		    {
		      n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",tmp_name_index, (int) getpid());
		      if(n >= SBUFFERSIZE) 
			printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
		      tmp_name_index++;
		      arith_image_trunc(word[i+2], (double) data.variable[variable_ID(word[i+4])].value.f, (double) data.variable[variable_ID(word[i+6])].value.f,name);
		      type = 10;
		    }
		  else
		    {
		      printf("Syntax error with function trunc\n");
		    }
		}
	      
	      strcpy(word[i],name);
	      word_type[i] = type;
	      for (j=i+1;j<nbword-(nbvarinput*2+1);j++)
		{
		  strcpy(word[j],word[j+(nbvarinput*2+1)]);
		  word_type[j] = word_type[j+(nbvarinput*2+1)];		
		}
	      nbword = nbword - nbvarinput*2-1;
	    }
	  
	  /*      printf("after : ");
		  for (i=0;i<nbword;i++)
		  {
		  if(variable_ID(word[i])!=-1)
		  printf(" %s(%f) ",word[i],data.variable[variable_ID(word[i])].value.f);
		  else
		  printf(" %s ",word[i]);	
		  }
		  printf("\n");
	  */    
	  /* computing the number of to-be-processed words */
	  passedequ = 0;
	  nb_tbp_word = 0;
	  for (i=(nbword-1);i>-1;i--)
	    {
	      if(word_type[i]==9)
		passedequ = 1;
	      if(passedequ==0)
		nb_tbp_word++;
	    }
	  
	}
      
      if (nbword>2)
	{
	  if (word_type[1]==9)
	    {
	      if(variable_ID(word[0])!=-1)
		delete_variable_ID(word[0]);
	      if(image_ID(word[0])!=-1)
		delete_image_ID(word[0]);
	      
	      if(word_type[2]==2)
		{
		  create_variable_ID(word[0],data.variable[variable_ID(word[2])].value.f);
		  printf("%.20g\n",data.variable[variable_ID(word[2])].value.f);
		}
	      if(word_type[2]==10)
		{
		  chname_image_ID(word[2],word[0]);
		}
	    }
	}
      else
	printf("%.20g\n",data.variable[variable_ID(word[0])].value.f);
      
      for(i=0;i<tmp_name_index;i++)
	{
	  n = snprintf(name,SBUFFERSIZE,"_tmp%d_%d",i, (int) getpid());
	  if(n >= SBUFFERSIZE)
	    printERROR(__FILE__,__func__,__LINE__,"Attempted to write string buffer with too many characters");
	  if(variable_ID(name)!=-1)
	    delete_variable_ID(name);
	  if(image_ID(name)!=-1)
	    delete_image_ID(name);
	}	
    }

  return(0);
}
