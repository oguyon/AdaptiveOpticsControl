#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>



#include <fitsio.h>  /* required by every program that uses CFITSIO  */

#include "CLIcore.h"

#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"

#include "info/info.h"
#include "image_gen/image_gen.h"

#include "ZernikePolyn/ZernikePolyn.h"

#define SWAP(x,y)  tmp=(x);x=(y);y=tmp;

#define PI 3.14159265358979323846264338328

extern DATA data;
ZERNIKE Zernike;



// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//

int_fast8_t mk_zer_cli()
{
  if(CLI_checkarg(1, 3)+CLI_checkarg(2, 2)+CLI_checkarg(3, 2)+CLI_checkarg(4, 1)==0)
    {
      mk_zer(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl, data.cmdargtoken[3].val.numl, data.cmdargtoken[4].val.numf);
      return 0;
    }
  else
    return 1;
}




int_fast8_t ZERNIKEPOLYN_rmPiston_cli()
{
  if(CLI_checkarg(1, 4)+CLI_checkarg(2, 4)==0)
    {
      ZERNIKEPOLYN_rmPiston(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string);
      return 0;
    }
  else
    return 1;
}






int_fast8_t init_ZernikePolyn()
{
 
  Zernike.init = 0;
  Zernike.ZERMAX = 5000;

  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "create and fit Zernike polynomials");
  data.NBmodule++;

  strcpy(data.cmd[data.NBcmd].key,"mkzer");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = mk_zer_cli;
  strcpy(data.cmd[data.NBcmd].info,"create Zernike polynomial");
  strcpy(data.cmd[data.NBcmd].syntax,"<output image> <size> <zern index> <rpix>");
  strcpy(data.cmd[data.NBcmd].example,"mkzer z43 512 43 100.0");
  strcpy(data.cmd[data.NBcmd].Ccall,"mk_zer(const char *ID_name, long SIZE, long zer_nb, float rpix)");
  data.NBcmd++;
 
 
  strcpy(data.cmd[data.NBcmd].key,"rmcpiston");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = ZERNIKEPOLYN_rmPiston_cli;
  strcpy(data.cmd[data.NBcmd].info,"remove piston term from WF cube");
  strcpy(data.cmd[data.NBcmd].syntax,"<WF cube> <aperture mask>");
  strcpy(data.cmd[data.NBcmd].example,"rmcpiston wfc mask");
  strcpy(data.cmd[data.NBcmd].Ccall,"long ZERNIKEPOLYN_rmPiston(const char *ID_name, const char *IDmask_name);");
  data.NBcmd++;
 
 // add atexit functions here

  return 0;

}






double fact(int n)
{
  int i;
  double value;
  
  value =1;
  for(i=1;i<n+1;i++)
    value = value*i;
  return(value);
}

int zernike_init()
{
  long j,n,m,s;
  long ii,jj;

  if(Zernike.init != 1)
    {  
      printf("ZERMAX= %ld\n",Zernike.ZERMAX);
      fflush(stdout);

      Zernike.Zer_n = (long*) malloc(Zernike.ZERMAX*sizeof(long));
      Zernike.Zer_m = (long*) malloc(Zernike.ZERMAX*sizeof(long));
      Zernike.R_array = (double*) malloc(Zernike.ZERMAX*Zernike.ZERMAX*sizeof(double));
      
      if ((Zernike.Zer_n==NULL)||(Zernike.Zer_m==NULL)||(Zernike.R_array==NULL))
	printf("error in memory allocation in zernike_init !!!\n");
      
      
      /* Zer_n and Zer_m are initialised to 0 */
      for (ii = 0; ii < Zernike.ZERMAX; ii++)
	{
	  Zernike.Zer_n[ii] = 0;
	  Zernike.Zer_m[ii] = 0;
	}
      
      /* Zer_n and Zer_m are computed */
      j = 0;
      n = 0;
      m = 0;
      
      while (j<Zernike.ZERMAX)
	{
	  Zernike.Zer_n[j] = n;
	  Zernike.Zer_m[j] = m;
	  j++;
	  m += 2;
	  if (m>n)
	    {
	      n++;
	      m = -n;
	    }
	}
      
      /* R_array is initialised */
      for (ii=0; ii<Zernike.ZERMAX; ii++)
	for (jj=0; jj<Zernike.ZERMAX; jj++)
	  Zernike.R_array[jj*Zernike.ZERMAX+ii] = 0;
      
      /* now the R_array is computed */
      for (j=1; j<Zernike.ZERMAX; j++)
	{
	  m = abs(Zernike.Zer_m[j]);
	  for (s=0; s<((int) (0.5*(Zernike.Zer_n[j]-m)+1));s++){
	    Zernike.R_array[j*Zernike.ZERMAX+Zernike.Zer_n[j]-2*s] = pow(-1,s)*fact(Zernike.Zer_n[j]-s)/fact(s)/fact((Zernike.Zer_n[j]+m)/2-s)/fact((Zernike.Zer_n[j]-m)/2-s);	
	  }
	}
      
      for (ii=0; ii<Zernike.ZERMAX; ii++)
	for (jj=0; jj<Zernike.ZERMAX; jj++)
	  Zernike.R_array[jj*Zernike.ZERMAX+ii] *= sqrt(Zernike.Zer_n[jj]+1);
      
      /* the zernikes index are computed */
      
      
      Zernike.init = 1;
    }

  return(0);
}

long Zernike_n(long i)
{
  return(Zernike.Zer_n[i]);
}

long Zernike_m(long i)
{
  return(Zernike.Zer_m[i]);
}


double Zernike_value(long j, double r, double PA)
{
  long i;
  double value = 0.0;
  double tmp,s2;
  long n,m;

  n = Zernike.Zer_n[j]+1;
  m = Zernike.Zer_m[j];
  s2 = sqrt(2.0);
 
  if (m==0)
    {
      for (i=0;i<n;i++)
	{
	  tmp = Zernike.R_array[j*Zernike.ZERMAX+i];
	  if (tmp!=0)
	    value += pow(r,i)*tmp;
	}
    }
  else
    {
      for (i=0;i<n;i++)
	{
	  tmp = Zernike.R_array[j*Zernike.ZERMAX+i];
	  if (tmp!=0)
	    {
	      if (m<0)
		value -= tmp*s2*pow(r,i)*sin(m*PA);
	      else
		value += tmp*s2*pow(r,i)*cos(m*PA);
	    }
	}
    }

  return(value);
}

long mk_zer(const char *ID_name, long SIZE, long zer_nb, float rpix)
{
    long ii, jj;
    double r,theta;
    long ID;
    long naxes[2];
    double coeff_norm;
    long n, m;
    double coeffextend1 = -1.0;
    double coeffextend2 = 0.3;
    double coeffextend3 = 4.0;
    double ss = 0.0;
    double xoffset = 0.0;
    double yoffset = 0.0;
    double x, y;

    ID = variable_ID("ZEXTENDc1");
    if(ID!=-1)
      {
	coeffextend1 = data.variable[ID].value.f;
	printf("ZEXTENDc1 = %f\n", coeffextend1);
      }

    ID = variable_ID("ZEXTENDc2");
    if(ID!=-1)
      {
	coeffextend2 = data.variable[ID].value.f;
	printf("ZEXTENDc2 = %f\n", coeffextend2);
      }


    ID = variable_ID("Zxoffset");
    if(ID!=-1)
      {
	xoffset = data.variable[ID].value.f;
	printf("Zxoffset = %f\n", xoffset);
      }
    ID = variable_ID("Zyoffset");
    if(ID!=-1)
      {
	yoffset = data.variable[ID].value.f;
	printf("Zyoffset = %f\n", yoffset);
      }



    naxes[0] = SIZE;
    naxes[1] = SIZE;

    if(Zernike.init==0)
      zernike_init();

    n = Zernike.Zer_n[zer_nb];
    m = Zernike.Zer_m[zer_nb];
    printf("Z = %ld    :  n = %ld, m = %ld\n",zer_nb,n,m);
    create_2Dimage_ID(ID_name, SIZE, SIZE);
    ID = image_ID(ID_name);

    /* let's compute the polar coordinates */
    ss = 0.0;
    for (ii=0;ii<SIZE;ii++)
      for (jj=0;jj<SIZE;jj++)
	{
	  x = 1.0*(ii-SIZE/2)-xoffset;
	  y = 1.0*(jj-SIZE/2)-yoffset;
	  
	  r = sqrt(x*x+y*y)/rpix;
	  theta = atan2(y,x);
	  if(r<1.0)	    
	    {
	      data.image[ID].array.F[jj*naxes[0]+ii] = Zernike_value(zer_nb,r,theta);
	      //printf("%f\n", Zernike_value(zer_nb,r,theta));
	      ss += data.image[ID].array.F[jj*naxes[0]+ii]*data.image[ID].array.F[jj*naxes[0]+ii];
	    }
	  else
	    if(coeffextend1>0)
	      {
		r = 1.0 + (r-1.0)/(1.0+coeffextend1*(r-1.0));
		data.image[ID].array.F[jj*naxes[0]+ii] = Zernike_value(zer_nb, 1.0, theta);
		data.image[ID].array.F[jj*naxes[0]+ii] *= exp(-pow((r-1.0)/(rpix*coeffextend2), coeffextend3));
		//	data.image[ID].array.F[jj*naxes[0]+ii] = r;
		//printf("%f %f\n", Zernike_value(zer_nb, 1.0, theta), exp(-pow((r-1.0)/(rpix*coeffextend2), coeffextend3)));
	      }
	}
  
    if (zer_nb>0)
      {
	make_disk("disk_tmp", SIZE, SIZE, SIZE/2, SIZE/2, rpix);
	coeff_norm = sqrt(ssquare("disk_tmp")/ss);
	//	printf("coeff = %f\n", coeff_norm);
	arith_image_cstmult_inplace(ID_name, coeff_norm);
	delete_image_ID("disk_tmp");
      }


    if(zer_nb==0)
      {
	for (ii=0;ii<SIZE;ii++)
	  for (jj=0;jj<SIZE;jj++)
	    {
	      r = sqrt((ii-SIZE/2)*(ii-SIZE/2)+(jj-SIZE/2)*(jj-SIZE/2))/rpix;
	      if(r>1.0)
		{
		  if(coeffextend1<0)
		    data.image[ID].array.F[jj*naxes[0]+ii] = 0.0;
		  else
		    data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
		}
	    }
      }
    

    return(ID);
}

// continue Zernike exp. beyond nominal radius, using the same polynomial expression
long mk_zer_unbounded(const char *ID_name, long SIZE, long zer_nb, float rpix)
{
    long ii, jj;
    double r,theta;
    long ID;
    long naxes[2];
    double coeff_norm;
    long n,m;

    naxes[0] = SIZE;
    naxes[1] = SIZE;

    if(Zernike.init==0)
      zernike_init();

    n = Zernike.Zer_n[zer_nb];
    m = Zernike.Zer_m[zer_nb];
    printf("Z = %ld    :  n = %ld, m = %ld\n",zer_nb,n,m);
    create_2Dimage_ID(ID_name,SIZE,SIZE);
    ID = image_ID(ID_name);

    /* let's compute the polar coordinates */
    for (ii=0;ii<SIZE;ii++)
      for (jj=0;jj<SIZE;jj++)
	{
	  r = sqrt((ii-SIZE/2)*(ii-SIZE/2)+(jj-SIZE/2)*(jj-SIZE/2))/rpix;
	  theta = atan2((jj-SIZE/2),(ii-SIZE/2));
	  //	  if(r<1.0)
	  data.image[ID].array.F[jj*naxes[0]+ii] = Zernike_value(zer_nb,r,theta);
	}
    
    if (zer_nb>0)
      {
	make_disk("disk_tmp",SIZE,SIZE,SIZE/2,SIZE/2,rpix);
	coeff_norm=sqrt(ssquare("disk_tmp")/ssquare(ID_name));
	arith_image_cstmult_inplace(ID_name,coeff_norm);
	delete_image_ID("disk_tmp");
      }

    if(zer_nb==0)
      {
	for (ii=0;ii<SIZE;ii++)
	  for (jj=0;jj<SIZE;jj++)
	    {
	      r = sqrt((ii-SIZE/2)*(ii-SIZE/2)+(jj-SIZE/2)*(jj-SIZE/2))/rpix;
	      //    if(r<1.0)
	      data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
	    }
      }
 
    return(ID);
}

// continue Zernike exp. beyond nominal radius, using the r=1 for r>1
long mk_zer_unbounded1(const char *ID_name, long SIZE, long zer_nb, float rpix)
{
    long ii, jj;
    double r,theta;
    long ID;
    long naxes[2];
    double coeff_norm;
    long n,m;

    naxes[0] = SIZE;
    naxes[1] = SIZE;

    if(Zernike.init==0)
      zernike_init();

    n = Zernike.Zer_n[zer_nb];
    m = Zernike.Zer_m[zer_nb];
    printf("Z = %ld    :  n = %ld, m = %ld\n",zer_nb,n,m);
    create_2Dimage_ID(ID_name,SIZE,SIZE);
    ID = image_ID(ID_name);

    /* let's compute the polar coordinates */
    for (ii=0;ii<SIZE;ii++)
      for (jj=0;jj<SIZE;jj++)
	{
	  r = sqrt((ii-SIZE/2)*(ii-SIZE/2)+(jj-SIZE/2)*(jj-SIZE/2))/rpix;
	  theta = atan2((jj-SIZE/2),(ii-SIZE/2));
	  if(r>1.0)
	    r = 1.0;
	  data.image[ID].array.F[jj*naxes[0]+ii] = Zernike_value(zer_nb,r,theta);
	}
    
    if (zer_nb>0)
      {
	make_disk("disk_tmp",SIZE,SIZE,SIZE/2,SIZE/2,rpix);
	coeff_norm=sqrt(ssquare("disk_tmp")/ssquare(ID_name));
	arith_image_cstmult_inplace(ID_name,coeff_norm);
	delete_image_ID("disk_tmp");
      }

    if(zer_nb==0)
      {
	for (ii=0;ii<SIZE;ii++)
	  for (jj=0;jj<SIZE;jj++)
	    {
	      data.image[ID].array.F[jj*naxes[0]+ii] = 1.0;
	    }
      }
 
    return(ID);
}

int mk_zer_series(const char *ID_name, long SIZE, long zer_nb, float rpix)
{
  long ii, jj;
  double *r;
  double *theta;
  long ID;
  long naxes[2];
  double tmp;
  char fname[200];
  long j;
  
  j=0;
  naxes[0] = SIZE;
  naxes[1] = SIZE;
  
  if(Zernike.init==0)
    zernike_init();
  
  create_2Dimage_ID("ztmp",SIZE,SIZE);
  ID = image_ID("ztmp");
  
  r = (double*) malloc(SIZE*SIZE*sizeof(double));
  theta = (double*) malloc(SIZE*SIZE*sizeof(double));
  
  if ((r==NULL)||(theta==NULL))
    printf("error in memory allocation !!!\n");
  
  /* let's compute the polar coordinates */
  for (ii=0;ii<SIZE;ii++)
    for (jj=0;jj<SIZE;jj++)
      {
	r[jj*naxes[0]+ii] = sqrt((0.5+ii-SIZE/2)*(0.5+ii-SIZE/2)+(0.5+jj-SIZE/2)*(0.5+jj-SIZE/2))/rpix;
	theta[jj*naxes[0]+ii] = atan2((jj-SIZE/2),(ii-SIZE/2));
      }
  
  /* let's make the Zernikes */
  for (ii=0;ii<SIZE;ii++)
    for (jj=0;jj<SIZE;jj++)
      {
	tmp = r[jj*naxes[0]+ii];
	if(tmp < 1.0)
	  data.image[ID].array.F[jj*SIZE+ii] = 1.0;
	else
	  data.image[ID].array.F[jj*SIZE+ii] = 0.0;
      }
  sprintf(fname,"%s%ld",ID_name,j);
  save_fl_fits("ztmp",fname);
  
  for (j=1; j<zer_nb; j++)
    {
      /*	printf("%ld/%ld\n",j,zer_nb);*/
      fflush(stdout);
      
      for (ii=0;ii<SIZE;ii++)
	for (jj=0;jj<SIZE;jj++)
	  {
	    tmp = r[jj*naxes[0]+ii];
	    if(tmp < 1.0)
	      data.image[ID].array.F[jj*SIZE+ii] = Zernike_value(j,tmp,theta[jj*naxes[0]+ii]);
	    else
	      data.image[ID].array.F[jj*SIZE+ii] = 0.0;
	  }
      
      sprintf(fname,"%s%04ld",ID_name,j);
      save_fl_fits("ztmp",fname);
    }
  
  delete_image_ID("ztmp");
  
  free(r);
  free(theta);
  
  return(0);
}


long mk_zer_seriescube(const char *ID_namec, long SIZE, long zer_nb, float rpix)
{
    long ii, jj;
    double *r;
    double *theta;
    long ID;
    long naxes[2];
    double tmp;
    char fname[200];
    long j;

    j=0;
    naxes[0] = SIZE;
    naxes[1] = SIZE;

    if(Zernike.init==0)
        zernike_init();

    ID = create_3Dimage_ID(ID_namec, SIZE, SIZE, zer_nb);
//    ID = image_ID("ztmp");

    r = (double*) malloc(SIZE*SIZE*sizeof(double));
    theta = (double*) malloc(SIZE*SIZE*sizeof(double));

    if ((r==NULL)||(theta==NULL))
        printf("error in memory allocation !!!\n");

    /* let's compute the polar coordinates */
    for (ii=0; ii<SIZE; ii++)
        for (jj=0; jj<SIZE; jj++)
        {
            r[jj*naxes[0]+ii] = sqrt((0.5+ii-SIZE/2)*(0.5+ii-SIZE/2)+(0.5+jj-SIZE/2)*(0.5+jj-SIZE/2))/rpix;
            theta[jj*naxes[0]+ii] = atan2((jj-SIZE/2),(ii-SIZE/2));
        }

    /* let's make the Zernikes */
    for (ii=0; ii<SIZE; ii++)
        for (jj=0; jj<SIZE; jj++)
        {
            tmp = r[jj*naxes[0]+ii];
            if(tmp < 1.0)
                data.image[ID].array.F[jj*SIZE+ii] = 1.0;
            else
                data.image[ID].array.F[jj*SIZE+ii] = 0.0;
        }
    for (j=1; j<zer_nb; j++)
    {
        /*	printf("%ld/%ld\n",j,zer_nb);*/
//        fflush(stdout);

        for (ii=0; ii<SIZE; ii++)
            for (jj=0; jj<SIZE; jj++)
            {
                tmp = r[jj*naxes[0]+ii];
                if(tmp < 1.0)
                    data.image[ID].array.F[j*SIZE*SIZE + jj*SIZE + ii] = Zernike_value(j,tmp,theta[jj*naxes[0]+ii]);
                else
                    data.image[ID].array.F[j*SIZE*SIZE + jj*SIZE + ii] = 0.0;
            }
	}

    free(r);
    free(theta);

    return(ID);
}




double get_zer(const char *ID_name, long zer_nb, double radius)
{
  double value;
  long SIZE;
  long ID;
  char fname[200];
  char fname1[200];

  ID=image_ID(ID_name);
  SIZE = data.image[ID].md[0].size[0];
  make_disk("disktmp",SIZE,SIZE,0.5*SIZE,0.5*SIZE,radius);
  
  sprintf(fname,"/RAID0/tmp/Zernike/Z_%ld",zer_nb);
  sprintf(fname1,"Z_%ld",zer_nb);

  if((ID=image_ID(fname1))==-1)
    {
      if(file_exists(fname)==1)
	load_fits(fname,fname1, 1);
      else
	mk_zer(fname1,SIZE,zer_nb,radius);
    }
  
  arith_image_mult(fname1,ID_name,"mult_tmp");
  value = arith_image_total("mult_tmp")/arith_image_total("disktmp");
  /* printf("value is %e\n",value);*/
  delete_image_ID("disktmp");
  /*  delete_image_ID("zernike_tmp");*/
  delete_image_ID("mult_tmp");

  return(value);
}

double get_zer_crop(const char *ID_name, long zer_nb, double radius, double radius1)
{
  double value;
  long SIZE;
  long ID;
  char fname[200];
  char fname1[200];

  ID=image_ID(ID_name);
  SIZE = data.image[ID].md[0].size[0];
  make_disk("disktmp",SIZE,SIZE,0.5*SIZE,0.5*SIZE,radius1);
  
  sprintf(fname,"/RAID0/tmp/Zernike/Z_%ld",zer_nb);
  sprintf(fname1,"Z_%ld",zer_nb);

  if((ID=image_ID(fname1))==-1)
    {
      if(file_exists(fname)==1)
	load_fits(fname,fname1, 1);
      else
	mk_zer(fname1,SIZE,zer_nb,radius);
    }
  
  arith_image_mult(fname1,ID_name,"mult_tmp");
  arith_image_mult("mult_tmp","disktmp","mult_tmp1");
  value = arith_image_total("mult_tmp1")/arith_image_total("disktmp");
  /* printf("value is %e\n",value);*/
  delete_image_ID("disktmp");
  /*  delete_image_ID("zernike_tmp");*/
  delete_image_ID("mult_tmp");
  delete_image_ID("mult_tmp1");

  return(value);
}



int get_zerns(const char *ID_name, long max_zer, double radius)
{
  long i;
  
  for(i=0;i<max_zer;i++)
    printf("%ld %e\n",i,get_zer(ID_name,i,radius));

  return(0);
}



int get_zern_array(const char *ID_name, long max_zer, double radius, double *array)
{
  long i;
  double tmp;

  for(i=0;i<max_zer;i++)
    {
      tmp = get_zer(ID_name,i,radius);
      /*     printf("%ld %e\n",i,tmp);*/
      array[i] = tmp;
    }

  return(0);
}



int remove_zerns(const char *ID_name, const char *ID_name_out, int max_zer, double radius)
{
  int i;
  double coeff;
  long ID;
  long SIZE;

  copy_image_ID(ID_name, ID_name_out, 0);
  ID = image_ID(ID_name);
  SIZE = data.image[ID].md[0].size[0];
  for(i=0;i<max_zer;i++)
    {
      mk_zer("zer_tmp",SIZE,i,radius);
      coeff = -1.0*get_zer(ID_name,i,radius);
      arith_image_cstmult_inplace("zer_tmp",coeff);
      arith_image_add(ID_name_out,"zer_tmp","tmp");
      delete_image_ID(ID_name_out);
      copy_image_ID("tmp", ID_name_out, 0);
      delete_image_ID("tmp");
      delete_image_ID("zer_tmp");
    }
  return(0);
}


long ZERNIKEPOLYN_rmPiston(const char *ID_name, const char *IDmask_name)
{
	long ID, IDmask;
	long xsize, ysize, zsize, xysize;
	long ii, kk;
	double tot1, tot2, ave;
	
	
	ID = image_ID(ID_name);
	xsize = data.image[ID].md[0].size[0];
	ysize = data.image[ID].md[0].size[1];
	zsize = data.image[ID].md[0].size[2];
	xysize = xsize*ysize;
	
	IDmask = image_ID(IDmask_name);

	for(kk=0;kk<zsize;kk++)
		{
			tot1 = 0.0;
			tot2 = 0.0;
			for(ii=0;ii<xysize;ii++)
				{
					tot1 += data.image[ID].array.F[kk*xysize+ii]*data.image[IDmask].array.F[ii];
					tot2 += data.image[IDmask].array.F[ii];
				}
			ave = tot1/tot2;
			for(ii=0;ii<xysize;ii++)
				{
					data.image[ID].array.F[kk*xysize+ii] -= ave;
				}
		}


	return(ID);
}
	


int remove_TTF(const char *ID_name, const char *ID_name_out, double radius)
{
  int i;
  double coeff;
  long ID;
  long SIZE;

  //  printf("-- %s  --- %s --\n",ID_name,ID_name_out);
  copy_image_ID(ID_name, ID_name_out, 0);
  ID = image_ID(ID_name);
  SIZE = data.image[ID].md[0].size[0];
  make_disk("disktmpttf",SIZE,SIZE,0.5*SIZE,0.5*SIZE,radius);
  //  list_image_ID();
  for(i=0;i<5;i++)
    {
      if((i==0)||(i==1)||(i==2)||(i==4))
	{
	  mk_zer("zer_tmp",SIZE,i,radius);
	  arith_image_mult("zer_tmp",ID_name,"mult_tmp");
	  coeff = arith_image_total("mult_tmp")/arith_image_total("disktmpttf");
	  delete_image_ID("mult_tmp");
	  coeff = -1.0*get_zer(ID_name,i,radius);
	  data.DOUBLEARRAY[i] = coeff;
	  mk_zer("zer_tmpu",SIZE,i,radius);
	  arith_image_cstmult_inplace("zer_tmpu",coeff);
	  //	  basic_add(ID_name_out,"zer_tmpu","tmp",0,0);
	  arith_image_add(ID_name_out,"zer_tmpu","tmp");
	  delete_image_ID(ID_name_out);
	  copy_image_ID("tmp", ID_name_out, 0);
	  delete_image_ID("tmp");
	  delete_image_ID("zer_tmp");
	  delete_image_ID("zer_tmpu");
	}
    }
  delete_image_ID("disktmpttf");
 
  return(0);
}



double fit_zer(const char *ID_name, long maxzer_nb, double radius, double *zvalue, double *residual)
{
  long SIZE;
  long ID,IDZ,IDdisk,ID0;
  char fname[200];
  char fname1[200];
  long i;
  long ii;
  double tmp;
  double disktot=0.0;
  long NBpass,pass;
  double value;
  double residualf=0.0;

  NBpass = 10;

  ID0 = image_ID(ID_name);
  copy_image_ID(ID_name, "resid", 0);  

  ID = image_ID("resid");
  SIZE = data.image[ID].md[0].size[0];
  IDdisk = make_disk("dtmp",SIZE,SIZE,0.5*SIZE,0.5*SIZE,0.999*radius);

  for(ii=0;ii<SIZE*SIZE;ii++)
    if(data.image[IDdisk].array.F[ii]>0.5)
      disktot += 1.0;

  for(i=0;i<maxzer_nb;i++)
    {
      residual[i] = 0.0;
      zvalue[i] = 0.0;
    }

  for(pass=0;pass<NBpass;pass++)
    {
      for(i=0;i<maxzer_nb;i++)
	{
	  sprintf(fname,"/RAID0/tmp/Zernike/Z_%ld",i);
	  sprintf(fname1,"Z_%ld",i);
	  
	  if((IDZ=image_ID(fname1))==-1)
	    {
	      if(file_exists(fname)==1)
		IDZ = load_fits(fname,fname1, 1);
	      else
		IDZ = mk_zer(fname1,SIZE,i,radius);
	    }
	  tmp = 0.0;
	  for(ii=0;ii<SIZE*SIZE;ii++)
	    if(data.image[IDdisk].array.F[ii]>0.5)
	      tmp += data.image[IDZ].array.F[ii]*data.image[ID].array.F[ii];
	  value = tmp/disktot;
	  
	  for(ii=0;ii<SIZE*SIZE;ii++)
	    if(data.image[IDdisk].array.F[ii]>0.5)
	      data.image[ID].array.F[ii] -= value*data.image[IDZ].array.F[ii];
	  zvalue[i] += value;	  
	  tmp = 0.0;
	  for(ii=0;ii<SIZE*SIZE;ii++)
	    if(data.image[IDdisk].array.F[ii]>0.5)
	      tmp += data.image[ID].array.F[ii]*data.image[ID].array.F[ii];

	  residualf = sqrt(tmp/disktot);
	}
    }

  residual[maxzer_nb-1] = residualf;
  for(i=maxzer_nb-1;i>0;i--)
    {
      residual[i-1] = sqrt(residual[i]*residual[i]+zvalue[i]*zvalue[i]);
    }

  for(ii=0;ii<SIZE*SIZE;ii++)
    {
      if(data.image[IDdisk].array.F[ii]<0.5)
	data.image[ID].array.F[ii] = 0.0;
    }

  delete_image_ID("dtmp");

  return(residualf);
}
