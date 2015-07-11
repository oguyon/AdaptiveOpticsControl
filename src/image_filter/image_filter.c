#include <fitsio.h>  /* required by every program that uses CFITSIO  */
#include <string.h>
#include <math.h>

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "info/info.h"
#include "fft/fft.h"
#include "image_gen/image_gen.h"
#include "statistic/statistic.h"

#include "image_filter/image_filter.h"


extern DATA data;


// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//





int gauss_filter_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,1)+CLI_checkarg(4,2)==0)
    {
      gauss_filter(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numl);
      return 0;
    }
  else
    return 1;
}



int init_image_filter()
{
  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "image filtering");
  data.NBmodule++;
  

  strcpy(data.cmd[data.NBcmd].key,"gaussfilt");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = gauss_filter_cli;
  strcpy(data.cmd[data.NBcmd].info,"gaussian 2D filtering");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <output image> <sigma> <filter box size>");
  strcpy(data.cmd[data.NBcmd].example,"gaussfilt imin imout 2.3 5");
  strcpy(data.cmd[data.NBcmd].Ccall,"long gauss_filter(char *ID_name, char *out_name, float sigma, int filter_size)");
  data.NBcmd++;
  
   
  // add atexit functions here


  return 0;

}




int median_filter(char *ID_name, char *out_name, int filter_size)
{
    long ID,ID_out;
    float *array;
    long ii,jj;
    long naxes[2];
    int i,j;

    /*  printf("Median filter...");
        fflush(stdout);*/
    save_fl_fits(ID_name,"!mf_in.fits");

    array = (float*) malloc((2*filter_size+1)*(2*filter_size+1)*sizeof(float));
    ID = image_ID(ID_name);
    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];
    printf("name = %s, ID = %ld, Size = %ld %ld (%d)\n",ID_name,ID,naxes[0],naxes[1],filter_size);
    fflush(stdout);
    copy_image_ID(ID_name, out_name, 0);
    ID_out = image_ID(out_name);

    for (jj = filter_size; jj < naxes[1]-filter_size; jj++)
        for (ii = filter_size; ii < naxes[0]-filter_size; ii++)
        {
            for (i=0; i<(2*filter_size+1); i++)
                for (j=0; j<(2*filter_size+1); j++)
                {
                    array[i*(2*filter_size+1)+j] = data.image[ID].array.F[(jj-filter_size+j)*naxes[0]+(ii-filter_size+i)];
                }
            quick_sort_float(array,(2*filter_size+1)*(2*filter_size+1));
            data.image[ID_out].array.F[jj*naxes[0]+ii] = array[((2*filter_size+1)*(2*filter_size+1)-1)/2];
        }
    free(array);

    save_fl_fits(out_name,"!mf_out.fits");

    /*  printf("Done\n");
        fflush(stdout);*/

    return(0);
}



long FILTER_percentile_interpol_fast(char *ID_name, char *IDout_name, double perc, long boxrad)
{
  long ID, ID1, IDout;
  long IDpermask;
  long step;
  long ii, jj, ii1, jj1, ii2, jj2;
  long iis, iie, jjs, jje;
  long xsize, ysize, xsize1, ysize1;
  double *array;
  double v00, v01, v10, v11;
  double u, t, ii1f, jj1f, x, y;
  long cnt;
  long pixstep = 5;
  long IDpercmask; // optional mask file


  step = (long) (0.7*boxrad);
  if(step<1)
    step = 1;
  

  ID = image_ID(ID_name);
  xsize = data.image[ID].md[0].size[0];
  ysize = data.image[ID].md[0].size[1];

  xsize1 = (long) (xsize/step);
  ysize1 = (long) (ysize/step);

  ID1 = create_2Dimage_ID("_tmppercintf",xsize1,ysize1);
  
  // identify mask if it exists
  IDpercmask = image_ID("_percmask");

  array = (double*) malloc(sizeof(double)*boxrad*boxrad*4);

  for(ii1=0;ii1<xsize1;ii1++)
    for(jj1=0;jj1<ysize1;jj1++)
      {
	x = 1.0*(ii1+0.5)/xsize1*xsize;
	y = 1.0*(jj1+0.5)/ysize1*ysize;

	iis = (long) ( x - boxrad );
	if(iis<0)
	  iis = 0;
	
	iie = (long) ( x + boxrad );
	if(iie>xsize)
	  iie = xsize;

	jjs = (long) ( y - boxrad );
	if(jjs<0)
	  jjs = 0;
	  
	jje = (long) ( y + boxrad );	
	if(jje>ysize)
	  jje = ysize;
	  

	cnt = 0;
	if(IDpercmask==-1)
	  {
	    for(ii=iis;ii<iie;ii+=pixstep)
	      for(jj=jjs;jj<jje;jj+=pixstep)
		{	      
		  array[cnt] = data.image[ID].array.F[jj*xsize+ii];
		  cnt ++;
		}
	  }
	else
	  {
	    for(ii=iis;ii<iie;ii+=pixstep)
	      for(jj=jjs;jj<jje;jj+=pixstep)
		{
		  if(data.image[IDpercmask].array.F[jj*xsize+ii]>0.5)
		    {
		      array[cnt] = data.image[ID].array.F[jj*xsize+ii];
		      cnt ++;
		    }
		}
	  }
	quick_sort_double(array,cnt);

	data.image[ID1].array.F[jj1*xsize1+ii1] = array[(long) (perc*cnt)];
	//	data.image[IDx].array.F[jj1*xsize1+ii1] = 0.5*(iis+iie);
	//data.image[IDy].array.F[jj1*xsize1+ii1] = 0.5*(jjs+jje);
      }
  free(array);

  IDout = create_2Dimage_ID(IDout_name,xsize,ysize);
  
  for(ii=0;ii<xsize;ii++)
    for(jj=0;jj<ysize;jj++)
      {
	ii1f = 1.0*ii/xsize*xsize1;
	jj1f = 1.0*jj/ysize*ysize1;
	ii1 = (long) (ii1f);
	jj1 = (long) (jj1f);
	
	ii2 = ii1+1;
	jj2 = jj1+1;

	while(ii2>xsize1-1)
	  {
	    ii1--;
	    ii2--;
	  }

	while(jj2>ysize1-1)
	  {
	    jj1--;
	    jj2--;
	  }
	
	u = ii1f - ii1;
	t = jj1f - jj1;
	
	v00 = data.image[ID1].array.F[jj1*xsize1+ii1];
	v10 = data.image[ID1].array.F[jj1*xsize1+ii2];
	v01 = data.image[ID1].array.F[jj2*xsize1+ii1];
	v11 = data.image[ID1].array.F[jj2*xsize1+ii2];

	data.image[IDout].array.F[jj*xsize+ii] = (1.0-u)*(1.0-t)*v00 + (1.0-u)*t*v01 + u*(1.0-t)*v10 + u*t*v11;
      }

  delete_image_ID("_tmppercintf");
  
  return(IDout);
}


//
// improvement of the spatial median filter
// percentile can be selected different than 50% percentile (parameter perc)
// spatial smoothing parameter (sigma)
// 
// this algorithm tests values and build the final map from these tests
// works well for smooth images, with perc between 0.1 and 0.9
// 
long FILTER_percentile_interpol(char *ID_name, char *IDout_name, double perc, double sigma)
{
  long ID, IDout, IDtmp, ID2;
  long NBstep = 10;
  double Imin, Imax;
  long xsize, ysize;
  double *array;
  long IDc;
  long k;
  double *varray;
  long ii;
  double value;
  double range;
  long IDkern;
  long double tot;
  long k1, k2;
  double x, v1, v2;
  double pstart, pend;

  ID = image_ID(ID_name);
  
  xsize = data.image[ID].md[0].size[0];
  ysize = data.image[ID].md[0].size[1];
  array = (double*) malloc(sizeof(double)*xsize*ysize);
  varray = (double*) malloc(sizeof(double)*NBstep);


  for(ii=0;ii<xsize*ysize;ii++)
    array[ii] = data.image[ID].array.F[ii];
  quick_sort_double(array,xsize*ysize);

  pstart = 0.8*perc-0.05;
  pend = 1.2*perc+0.05;
  if(pstart<0.01)
    pstart = 0.01;
  if(pend>0.99)
    pend = 0.99;

  Imin = array[(long) (pstart*xsize*ysize)];
  Imax = array[(long) (pend*xsize*ysize)];

  range = Imax-Imin;
  Imin -= 0.1*range;
  Imax += 0.1*range;

  for(k=0;k<NBstep;k++)
    varray[k] = Imin + 1.0*k/(NBstep-1)*(Imax-Imin);
  
  free(array);

  printf("Testing %ld values in range %g -> %g\n", NBstep, Imin, Imax);
  fflush(stdout);

  IDc = create_3Dimage_ID("_testpercim",xsize,ysize,NBstep);

  IDkern = make_gauss("_kern",xsize,ysize,sigma,1.0);
  tot = 0.0;
  for(ii=0;ii<xsize*ysize;ii++)
    tot += data.image[IDkern].array.F[ii];
  for(ii=0;ii<xsize*ysize;ii++)
    data.image[IDkern].array.F[ii] /= tot;
 

  IDtmp = create_2Dimage_ID("_testpercim1",xsize,ysize);
  for(k=0;k<NBstep;k++)
    {
      printf("   %ld/%ld threshold = %f\n",k,NBstep,varray[k]);
      for(ii=0;ii<xsize*ysize;ii++)
	{
	  value = data.image[ID].array.F[ii];
	  if(value<varray[k])
	    data.image[IDtmp].array.F[ii] = 1.0;
	  else
	    data.image[IDtmp].array.F[ii] = 0.0;
	}

      fconvolve_padd("_testpercim1","_kern",(long) (3.0*sigma), "_testpercim2");

      ID2 = image_ID("_testpercim2");
      for(ii=0;ii<xsize*ysize;ii++)
	data.image[IDc].array.F[k*xsize*ysize+ii] = data.image[ID2].array.F[ii];
      delete_image_ID("_testpercim2");
    }


  IDout = create_2Dimage_ID(IDout_name,xsize,ysize);
  for(ii=0;ii<xsize*ysize;ii++)
    {
      k = 0;
      k1 = 0;
      k2 = 0;
      v1 = 0.0;
      v2 = 0.0;
      while((v2<perc)&&(k<NBstep-1))	
	{
	  k++;
	  v1 = v2;
	  k1 = k2;
	  v2 = data.image[IDc].array.F[k*xsize*ysize+ii];	 	  
	  k2 = k;
	}
      // ideally, v1<perc<v2
      if((v1<perc)&&(perc<v2))
	{
	  x = (perc-v1)/(v2-v1);
	  data.image[IDout].array.F[ii] = (1.0-x)*varray[k1] + x*varray[k2];
	}
      else
	{
	  if(v1>perc)
	    data.image[IDout].array.F[ii] = varray[0];
	  else 
	    data.image[IDout].array.F[ii] = varray[NBstep-1];
	}
      
    }

  //  save_fl_fits("_testpercim","!_testpercim.fits");
  delete_image_ID("_kern");
  delete_image_ID("_testpercim");
  delete_image_ID("_testpercim1");
  free(varray);

  return(IDout);
}


long gauss_filter(char *ID_name, char *out_name, float sigma, int filter_size)
{
    int ID,ID_out,ID_tmp;
    float *array;
    long ii,jj,kk;
    long naxes[3];
    long naxis;
    long i,j, k;
    float sum;
    double tot;
    long jmax;
    
    
    // printf("sigma = %f\n",sigma);
    // printf("filter size = %d\n",filter_size);

    printf("STEP 000\n"); 
    fflush(stdout);

    array = (float*) malloc((2*filter_size+1)*sizeof(float));
    ID = image_ID(ID_name);
    naxis = data.image[ID].md[0].naxis;
    for(kk=0; kk<naxis; kk++)
        naxes[kk] = data.image[ID].md[0].size[kk];


    printf("STEP 010\n"); 
    fflush(stdout);

    if(naxis==2)
        naxes[2] = 1;
    copy_image_ID(ID_name, out_name, 0);
    arith_image_zero(out_name);
    ID_tmp = create_2Dimage_ID("gtmp", naxes[0], naxes[1]);
    //  copy_image_ID(ID_name,"gtmp", 0);
    // arith_image_zero("gtmp");
    // save_fl_fits("gtmp","!gtmp0");
    // ID_tmp = image_ID("gtmp");
    ID_out = image_ID(out_name);
    list_image_ID();

    printf("STEP 020\n"); 
    fflush(stdout);

    sum=0.0;
    for (i=0; i<(2*filter_size+1); i++)
    {
        array[i] = exp(-((i-filter_size)*(i-filter_size))/sigma/sigma);
        sum += array[i];
    }



    for (i=0; i<(2*filter_size+1); i++)
    {
        array[i] /= sum;
        //    printf("%ld %f\n",i,array[i]);
    }

    printf("STEP 030\n"); 
    fflush(stdout);

    for(k=0; k<naxes[2]; k++)
    {
        for (ii = 0; ii < naxes[0]*naxes[1]; ii++)
            data.image[ID_tmp].array.F[ii] = 0.0;

        for (jj = 0; jj < naxes[1]; jj++)
        {
            for (ii = 0; ii < naxes[0]-(2*filter_size+1); ii++)
            {
                for (i=0; i<(2*filter_size+1); i++)
                    data.image[ID_tmp].array.F[jj*naxes[0]+(ii+filter_size)] += array[i]*data.image[ID].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+(ii+i)];
            }
            for (ii=0; ii<filter_size; ii++)
            {
                tot = 0.0;
                for(i=filter_size-ii; i<(2*filter_size+1); i++)
                {
                    data.image[ID_tmp].array.F[jj*naxes[0]+ii] += array[i]*data.image[ID].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+(ii-filter_size+i)];
                    tot += array[i];
                }
                data.image[ID_tmp].array.F[jj*naxes[0]+ii] /= tot;
            }
            for (ii=naxes[0]-filter_size-1; ii<naxes[0]; ii++)
            {
                tot = 0.0;
                for(i=0; i<(2*filter_size+1)-(ii-naxes[0]+filter_size+1); i++)
                {
                    data.image[ID_tmp].array.F[jj*naxes[0]+ii] += array[i]*data.image[ID].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+(ii-filter_size+i)];
                    tot += array[i];
                }
                data.image[ID_tmp].array.F[jj*naxes[0]+ii] /= tot;
            }

        }

    printf("STEP 040\n");
    fflush(stdout);


        for (ii = 0; ii < naxes[0]; ii++)
        {
         //   printf("A jj : 0 -> %ld/%ld\n", naxes[1]-(2*filter_size+1), naxes[1]);
         //   fflush(stdout);
            for (jj = 0; jj < naxes[1]-(2*filter_size+1); jj++)
            {
         //       printf("00: %ld/%ld\n", k*naxes[0]*naxes[1]+(jj+filter_size)*naxes[0]+ii, naxes[0]*naxes[1]*naxes[2]);
         //       printf("01: %ld/%ld\n", (jj+j)*naxes[0]+ii, naxes[0]*naxes[1]);
                fflush(stdout);
                for (j=0; j<(2*filter_size+1); j++)
                    data.image[ID_out].array.F[k*naxes[0]*naxes[1]+(jj+filter_size)*naxes[0]+ii] += array[j]*data.image[ID_tmp].array.F[(jj+j)*naxes[0]+ii];
            }

        //    printf("B jj : 0 -> %d/%ld\n", filter_size, naxes[1]);
        //    fflush(stdout);
            for (jj=0; jj<filter_size; jj++)
            {
                tot = 0.0;
                jmax = (2*filter_size+1);
                if(jj-filter_size+jmax > naxes[1])
                    jmax = naxes[1]-jj+filter_size;
                for(j=filter_size-jj; j<jmax; j++)
                {
         //           printf("02: %ld/%ld\n", k*naxes[0]*naxes[1]+jj*naxes[0]+ii, naxes[0]*naxes[1]*naxes[2]);
         //           printf("03: %ld/%ld\n", (jj-filter_size+j)*naxes[0]+ii, naxes[0]*naxes[1]);
                    fflush(stdout);                                              
                    data.image[ID_out].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+ii] += array[j]*data.image[ID_tmp].array.F[(jj-filter_size+j)*naxes[0]+ii];
                    tot += array[j];
                }
                data.image[ID_out].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+ii] /= tot;
            }

            for (jj=naxes[1]-filter_size-1; jj<naxes[1]; jj++)
            {
                tot = 0.0;
                for(j=0; j<(2*filter_size+1)-(jj-naxes[1]+filter_size+1); j++)
                {
                    data.image[ID_out].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+ii] += array[j]*data.image[ID_tmp].array.F[(jj-filter_size+j)*naxes[0]+ii];
                    tot += array[j];
                }
                data.image[ID_out].array.F[k*naxes[0]*naxes[1]+jj*naxes[0]+ii] /= tot;
            }
        }

    }
    
    printf("STEP 100\n"); 
    fflush(stdout);


    //  save_fl_fits("gtmp","!gtmp");
    delete_image_ID("gtmp");

    free(array);

    return(ID_out);
}


int gauss_3Dfilter(char *ID_name, char *out_name, float sigma, int filter_size)
{
  int ID,ID_out,ID_tmp,ID_tmp1;
  float *array;
  long ii,jj,kk;
  long naxes[3];
  int i,j,k;
  float sum;
  
  array = (float*) malloc((2*filter_size+1)*sizeof(float));
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1]; 
  naxes[2] = data.image[ID].md[0].size[2]; 

  copy_image_ID(ID_name, out_name, 0);
  arith_image_zero(out_name);
  copy_image_ID(ID_name, "gtmp", 0);
  arith_image_zero("gtmp");
  copy_image_ID("gtmp", "gtmp1", 0);
  ID_tmp = image_ID("gtmp");
  ID_tmp1 = image_ID("gtmp1");  
  ID_out = image_ID(out_name);  

  sum=0.0;
  for (i=0;i<(2*filter_size+1);i++)
    {
      array[i] = exp(-((i-filter_size)*(i-filter_size))/sigma/sigma);
      sum+=array[i];
    }

  for (i=0;i<(2*filter_size+1);i++)
    array[i] /= sum;
  
  for(kk = 0; kk < naxes[2]; kk++)
    for (jj = 0; jj < naxes[1]; jj++) 
      for (ii = 0; ii < naxes[0]-(2*filter_size+1); ii++)
	{
	  for (i=0;i<(2*filter_size+1);i++)
	    data.image[ID_tmp].array.F[kk*naxes[0]*naxes[1]+jj*naxes[0]+(ii+filter_size)] += array[i]*data.image[ID].array.F[kk*naxes[0]*naxes[1]+jj*naxes[0]+(ii+i)];
	}
  
  for(kk = 0; kk < naxes[2]; kk++)
    for (ii = 0; ii < naxes[0]; ii++)
      for (jj = 0; jj < naxes[1]-(2*filter_size+1); jj++) 
	{
	  for (j=0;j<(2*filter_size+1);j++)
	    data.image[ID_tmp1].array.F[kk*naxes[0]*naxes[1]+(jj+filter_size)*naxes[0]+ii] += array[j]*data.image[ID_tmp].array.F[kk*naxes[0]*naxes[1]+(jj+j)*naxes[0]+ii];
	}

  for(ii = 0; ii < naxes[0]; ii++)
    for (jj = 0; jj < naxes[1]; jj++) 
      for (kk = 0; kk < naxes[2]-(2*filter_size+1); kk++) 
	{
	  for (k=0;k<(2*filter_size+1);k++)
	    data.image[ID_out].array.F[(kk+filter_size)*naxes[0]*naxes[1]+jj*naxes[0]+ii] += array[k]*data.image[ID_tmp1].array.F[(kk+k)*naxes[0]*naxes[1]+jj*naxes[0]+ii];
	}
  

  delete_image_ID("gtmp");
  delete_image_ID("gtmp1");

  free(array);
  return(0);
}

int f_filter(char *ID_name, char *ID_out, float f1, float f2)
{
  printf("%s %s %f %f\n",ID_name,ID_out,f1,f2);
  /*  char lstring[1000];
  int ID;
  long naxes[2];
  float a=200.0;
  
  ID=image_ID(ID_name);
  naxes[0]=data.image[ID].size[0];
  naxes[1]=data.image[ID].size[1];

  sprintf(lstring,"zer_tmp=%s*0",ID_name);
  execute_arith(lstring);
  pupfft(ID_name,"zer_tmp","ffamp","ffpha","");
  delete_image_ID("zer_tmp");
  
  make_dist("ffdist",naxes[0],naxes[1],1.0*naxes[0]/2,1.0*naxes[1]/2);
  sprintf(lstring,"ffd=(ffdist-%f)*(%f-ffdist)",f1,f2);
  execute_arith(lstring);
  sprintf(lstring,"ffd=ffd/(ffd+%f)",a);
  execute_arith(lstring);
  execute_arith("ffd=ffd*ffd");

  make_disk("ffd1",naxes[0],naxes[1],naxes[0]/2,naxes[1]/2,f1);
  make_disk("ffd2",naxes[0],naxes[1],naxes[0]/2,naxes[1]/2,f2);
  execute_arith("ffd=ffd*(ffd2-ffd1)");
  delete_image_ID("ffd1");
  delete_image_ID("ffd2");

  execute_arith("ffamp=ffamp*ffd");
  delete_image_ID("ffd");
  pupfft("ffamp","ffpha","fftbea","fftbep","-inv");
  delete_image_ID("ffamp");
  delete_image_ID("ffpha");
  ampl_pha_2_re_im("fftbea","fftbep",ID_out,"fftbe");

  delete_image_ID("fftbe");
  delete_image_ID("fftbea");
  delete_image_ID("fftbep");
  */
  return(0);
}


long fconvolve(char *name_in, char *name_ke, char *name_out)
{
  long ID_in,ID_ke;
  long naxes[2];
  long IDout;

  ID_in = image_ID(name_in);
  naxes[0]=data.image[ID_in].md[0].size[0];
  naxes[1]=data.image[ID_in].md[0].size[1];
  ID_ke = image_ID(name_ke);
  if((naxes[0] != data.image[ID_ke].md[0].size[0])||(naxes[1] != data.image[ID_ke].md[0].size[1]))
    {
      fprintf(stderr,"ERROR in function fconvolve: image and kernel have different sizes\n");
      exit(0);
    }
  //  save_fl_fits(name_in,"!test1.fits");
  // save_fl_fits(name_ke,"!test2.fits");
  
  do2drfft(name_in,"infft");
  do2drfft(name_ke,"kefft");

  arith_image_Cmult("infft","kefft","outfft");
  delete_image_ID("infft");
  delete_image_ID("kefft");
  do2dffti("outfft","outfft1");
  delete_image_ID("outfft");
  mk_reim_from_complex("outfft1","tmpre","tmpim");
  
  //  save_fl_fits("tmpre","!tmpre.fits");
  // save_fl_fits("tmpim","!tmpim.fits");

  delete_image_ID("outfft1");
  delete_image_ID("tmpim");
  arith_image_cstmult("tmpre",1.0/naxes[0]/naxes[1],name_out);
  delete_image_ID("tmpre");
  permut(name_out);

  IDout = image_ID(name_out);

  return(IDout);
}

// to avoid edge effects
long fconvolve_padd(char *name_in, char *name_ke, long paddsize, char *name_out)
{
  long ID_in,ID_ke,ID1,ID2,ID3,IDout;
  long naxes[2];
  long naxespadd[2];
  long ii,jj;

  ID_in = image_ID(name_in);
  naxes[0] = data.image[ID_in].md[0].size[0];
  naxes[1] = data.image[ID_in].md[0].size[1];
  ID_ke = image_ID(name_ke);
  if((naxes[0] != data.image[ID_ke].md[0].size[0])||(naxes[1] != data.image[ID_ke].md[0].size[1]))
    {
      fprintf(stderr,"ERROR in function fconvolve: image and kernel have different sizes\n");
      exit(0);
    }

  naxespadd[0] = naxes[0]+2*paddsize;
  naxespadd[1] = naxes[1]+2*paddsize;

  // printf("new axes : %ld %ld\n",naxespadd[0],naxespadd[1]);

  ID1 = create_2Dimage_ID("tmpimpadd",naxespadd[0],naxespadd[1]);
  ID2 = create_2Dimage_ID("tmpkepadd",naxespadd[0],naxespadd[1]);
  ID3 = create_2Dimage_ID("tmpim1padd",naxespadd[0],naxespadd[1]);

  for(ii=0;ii<naxes[0];ii++)
    for(jj=0;jj<naxes[1];jj++)
      {
	data.image[ID1].array.F[(jj+paddsize)*naxespadd[0]+(ii+paddsize)] = data.image[ID_in].array.F[jj*naxes[0]+ii];
	data.image[ID2].array.F[(jj+paddsize)*naxespadd[0]+(ii+paddsize)] = data.image[ID_ke].array.F[jj*naxes[0]+ii];	
	data.image[ID3].array.F[(jj+paddsize)*naxespadd[0]+(ii+paddsize)] = 1.0;
      }
  
  //  list_image_ID();
  //  printf("Doing convolutions...");
  //  fflush(stdout);

  fconvolve("tmpimpadd","tmpkepadd","tmpconv1");
  fconvolve("tmpim1padd","tmpkepadd","tmpconv2");

  //  printf(" done\n");
  // fflush(stdout);

  delete_image_ID("tmpimpadd");
  delete_image_ID("tmpkepadd");
  delete_image_ID("tmpim1padd");

  ID1 = image_ID("tmpconv1");
  ID2 = image_ID("tmpconv2");
  IDout = create_2Dimage_ID(name_out,naxes[0],naxes[1]);
  
  for(ii=0;ii<naxes[0];ii++)
    for(jj=0;jj<naxes[1];jj++)
      {
	data.image[IDout].array.F[jj*naxes[0]+ii] = data.image[ID1].array.F[(jj+paddsize)*naxespadd[0]+(ii+paddsize)]/data.image[ID2].array.F[(jj+paddsize)*naxespadd[0]+(ii+paddsize)];
      }
  delete_image_ID("tmpconv1");
  delete_image_ID("tmpconv2");

  return(IDout);
}


int fconvolve_1(char *name_in, char *kefft, char *name_out)
{
  /* FFT of kernel has already been done */
  long ID_in;
  long naxes[2];

  ID_in=image_ID(name_in);
  naxes[0]=data.image[ID_in].md[0].size[0];
  naxes[1]=data.image[ID_in].md[0].size[1];

  do2drfft(name_in,"infft");

  arith_image_Cmult("infft",kefft,"outfft");
  delete_image_ID("infft");
  do2dffti("outfft","outfft1");
  delete_image_ID("outfft");
  mk_reim_from_complex("outfft1","tmpre","tmpim");
  delete_image_ID("outfft1");
  delete_image_ID("tmpim");
  arith_image_cstmult("tmpre",1.0/naxes[0]/naxes[1],name_out);
  delete_image_ID("tmpre");
  permut(name_out);

  return(0);
}

// if blocksize = 512, for images > 512x512, break image in 512x512 overlapping blocks
// kernel image must be blocksize
int fconvolveblock(char *name_in, char *name_ke, char *name_out, long blocksize)
{
  long IDin,IDout,IDtmp,IDtmpout,IDcnt;
  long xsize,ysize;
  long overlap;
  long ii,jj,ii0,jj0;
  float gain;
  float alpha = 4.0;

  overlap = (long) (blocksize/10);
  IDin = image_ID(name_in);
  xsize = data.image[IDin].md[0].size[0];
  ysize = data.image[IDin].md[0].size[1];
  IDout = create_2Dimage_ID(name_out,xsize,ysize);

  IDtmp = create_2Dimage_ID("tmpblock",blocksize,blocksize);

  IDcnt = create_2Dimage_ID("tmpcnt",xsize,ysize);
  for(ii=0;ii<xsize*ysize;ii++)
    data.image[IDcnt].array.F[ii] = 0.0;

  for(ii0=0;ii0<xsize-overlap;ii0+=blocksize-overlap)
    for(jj0=0;jj0<ysize-overlap;jj0+=blocksize-overlap)
      {
	for(ii=0;ii<blocksize;ii++)
	  for(jj=0;jj<blocksize;jj++)
	    {
	      if((ii0+ii<xsize)&&(jj0+jj<ysize))
		data.image[IDtmp].array.F[jj*blocksize+ii] = data.image[IDin].array.F[(jj0+jj)*xsize+(ii0+ii)];
	      else
		data.image[IDtmp].array.F[jj*blocksize+ii] = 0.0;
	    }
	fconvolve("tmpblock",name_ke,"tmpblockc");
	IDtmpout = image_ID("tmpblockc");
	for(ii=0;ii<blocksize;ii++)
	  for(jj=0;jj<blocksize;jj++)
	    {
	      if((ii0+ii<xsize)&&(jj0+jj<ysize))
		{
		  gain = 1.0;
		  if(ii<overlap)
		    gain *= pow(1.0*(1.0*ii/overlap),alpha);
		  if(jj<overlap)
		    gain *= pow(1.0*(1.0*jj/overlap),alpha);
		  if(ii>blocksize-overlap)
		    gain *= pow(1.0*(1.0*(blocksize-ii)/overlap),alpha);
		  if(jj>blocksize-overlap)
		    gain *= pow(1.0*(1.0*(blocksize-jj)/overlap),alpha);
		  
		  data.image[IDout].array.F[(jj0+jj)*xsize+(ii0+ii)] += gain*data.image[IDtmpout].array.F[jj*blocksize+ii];
		  data.image[IDcnt].array.F[(jj0+jj)*xsize+(ii0+ii)] += gain*1.0;
		}
	    }
      }
  //  save_fl_fits("tmpcnt","!tmpcnt.fits");
  // exit(0);
  for(ii=0;ii<xsize*ysize;ii++)
    data.image[IDout].array.F[ii] /= data.image[IDcnt].array.F[ii]+1.0e-8;

  delete_image_ID("tmpcnt");
  delete_image_ID("tmpblock");
  delete_image_ID("tmpblockc");

  return(0);
}


int film_scanner_vsripes_remove(char *IDname, char *IDout, long l1, long l2)
{
  long ID;
  long naxes[2];
  long ii,jj;
  float *smarray;
  float value;

  ID=image_ID(IDname);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];
  printf("%s\n",IDout);

  /* fill up linarray */
  smarray = (float*) malloc(sizeof(float)*(l2-l1));

  for(ii=0;ii<naxes[0];ii++)
    {
      for(jj=l1;jj<l2;jj++)
	smarray[jj-l1] = data.image[ID].array.F[jj*naxes[0]+ii];
      quick_sort_float(smarray,l2-l1);
      value = smarray[(long) (0.5*(l2-l1))];
      for(jj=0;jj<naxes[1];jj++)
	data.image[ID].array.F[jj*naxes[0]+ii] -= value;
    }

  free(smarray);

  return(0);
}

int filter_fit1D(char *fname, long NBpts)
{
  FILE *fp;
  float *xarray;
  float *yarray;
  long i;
  long iter;
  long NBiter = 10000000;
  float *CX;//,CX,CX2,CX3,CX4,CX5;
  float *CXb;//,CXb,CX2b,CX3b,CX4b,CX5b;
  long PolyOrder = 10;
  long k;
  float amp;
  float x,value,bvalue,tmp;
  float cnt,coeff;

  xarray = (float*) malloc(sizeof(float)*NBpts);
  yarray = (float*) malloc(sizeof(float)*NBpts);

  CX = (float*) malloc(sizeof(float)*PolyOrder);
  CXb = (float*) malloc(sizeof(float)*PolyOrder);

  fp = fopen(fname,"r");
  for(i=0;i<NBpts;i++)
    {
      if(fscanf(fp,"%f %f\n",&xarray[i],&yarray[i])!=2)
	{
	  printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
	  exit(0);
	}
      printf("%ld %f %.10f\n",i,xarray[i],yarray[i]);
    }
  fclose(fp);

  for(k=0;k<PolyOrder;k++)
    CX[k] = 0.0;
    
  if(1==0)
    {
      // + side
    }
  else
    {
      // - side  RMS = 68 nm
      CX[0] = -1.39745e-07;
      CX[1] = 3.21897e-05;
      CX[2] = -0.00011522;
      CX[3] = 0.000149478;
      CX[4] = -6.17691e-05;
      CX[5] = -2.94572e-06;
    }
  for(k=0;k<PolyOrder;k++)
    CXb[k] = CX[k];
    
	bvalue	= 1000000.0;
  for(iter=0;iter<NBiter;iter++)
    {
      amp = 1.0e-7;
      if(iter>0)
	for(k=0;k<PolyOrder;k++)
	  CX[k] = CXb[k] + amp*2.0*(ran1()-0.5);
	
      value = 0.0;
      cnt = 0.0;
      for(i=0;i<NBpts;i++)
	{
	  x = xarray[i];
	  tmp = 0.0;
	  for(k=0;k<PolyOrder;k++)
	    tmp += CX[k]*pow(x,k);
	  coeff = pow(1.0+5.0*exp(-10.0*x*x),2.0);
	  value += coeff*(tmp-yarray[i])*(tmp-yarray[i]);
	  cnt += coeff;
	}
      value = sqrt(value/cnt);
      //      printf("value = %g\n");
      if(iter==0)
	bvalue = value;
      else
	{
	  if(value<bvalue)
	    {
	      for(k=0;k<PolyOrder;k++)
		CXb[k] = CX[k];
	      bvalue = value;
	      printf("BEST VALUE = %g\n",value);
	      printf("f(r) = ");
	      printf(" %g",CX[0]);
	      for(k=1;k<PolyOrder;k++)
		printf(" + r**%ld*%g",k,CX[k]);
	      printf("\n");

	      for(k=0;k<PolyOrder;k++)
		printf("CX[%ld] = %g\n",k,CX[k]);
	    }
	}	  
    }

  free(xarray);
  free(yarray);
  free(CX);
  free(CXb);

  return(0);
}

// fits a 2D image as a sum of cosines and sines
int filter_fit2Dcossin(char *IDname, float radius)
{
  long ID,IDres,IDfit;
  long size;
  long NBfrequ1D = 25;
  long NBfrequ;
  float *coscoeff;
  float *sincoeff;
  float frequStep = 1.0;
  float x,y;
  long i,j,ii,jj,i1,j1;
  float tmp,tmpc,tmpc1,tmpc2,tmps,tmps1,tmps2;
  long iter;
  long NBiter = 5000;
  float error;
  long errorcnt;
  float gain = 0.0;
  float Gain = 0.2;
  float gtmpc,gtmps;
  float rlim = 0.98;
  FILE *fp;
  long tmpl;
  long step = 2;
  float coeffc,coeffs;
  long ii1;
  float *xarray;
  float *yarray;
  float *rarray;

  NBfrequ = NBfrequ1D*(2*NBfrequ1D-1);
  coscoeff = (float*) malloc(sizeof(float)*NBfrequ);
  sincoeff = (float*) malloc(sizeof(float)*NBfrequ);
  for(i=0;i<NBfrequ1D;i++)
    for(j=0;j<2*NBfrequ1D-1;j++)
      {
	coscoeff[j*NBfrequ1D+i] = 0.0;
	sincoeff[j*NBfrequ1D+i] = 0.0;
	//	printf("%ld %ld -> %g %g\n",i,(j-NBfrequ1D+1),coscoeff[j*NBfrequ1D+i],sincoeff[j*NBfrequ1D+i]);
      }

  if(1==0)
    {
      fp = fopen("fitcoeff.dat","r");
      for(i=0;i<NBfrequ1D;i++)
	for(j=0;j<2*NBfrequ1D-1;j++)
	  {
	    if(fscanf(fp,"%ld %ld %ld %g %g\n",&i,&j,&tmpl,&coscoeff[j*NBfrequ1D+i],&sincoeff[j*NBfrequ1D+i])!=5)
	      {
		printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
		exit(0);
	      }
	  }
      fclose(fp);    
      /*  
      fp = fopen("fitcoeff1.dat","w");
      for(i=0;i<NBfrequ1D;i++)
	for(j=0;j<2*NBfrequ1D-1;j++)
	  fprintf(fp,"%ld %ld %ld %.20g %.20g\n",i,j,j-NBfrequ1D+1,coscoeff[j*NBfrequ1D+i],sincoeff[j*NBfrequ1D+i]);
	  fclose(fp);*/
    }
  //  exit(0);

  ID = image_ID(IDname);
  size = data.image[ID].md[0].size[0];
  printf("SIZE = %ld\n",size);
  IDres = create_2Dimage_ID("residual",size,size);
  IDfit = create_2Dimage_ID("fitim",size,size);

  xarray = (float*) malloc(sizeof(float)*size*size);
  yarray = (float*) malloc(sizeof(float)*size*size);
  rarray = (float*) malloc(sizeof(float)*size*size);

  for(ii=0;ii<size;ii+=step)
    for(jj=0;jj<size;jj+=step)
      {
	ii1 = jj*size+ii;
	x = 1.0*(ii-size/2)/radius;
	y = 1.0*(jj-size/2)/radius;
	xarray[ii1] = x;
	yarray[ii1] = y;
	rarray[ii1] = sqrt(x*x+y*y);
      }

  for(ii=0;ii<size*size;ii++)
    {
      data.image[IDres].array.F[ii] = data.image[ID].array.F[ii];
      data.image[IDfit].array.F[ii] = 0.0;
    }
  for(iter=0;iter<NBiter;iter++)
    {
      if((iter==0)||(iter==NBiter-1))
	gain = 0.0;
      else
	gain = Gain;
      // initialize IDfit
      for(ii=0;ii<size;ii+=step)
	for(jj=0;jj<size;jj+=step)
	  data.image[IDfit].array.F[jj*size+ii] = 0.0;
      for(i1=0;i1<NBfrequ1D;i1++)
	for(j1=0;j1<2*NBfrequ1D-1;j1++)
	  {
	    coeffc = coscoeff[j1*NBfrequ1D+i1];
	    coeffs = sincoeff[j1*NBfrequ1D+i1];
	    for(ii=0;ii<size;ii+=step)
	      for(jj=0;jj<size;jj+=step)
		{
		  ii1 = jj*size+ii;
		  if(rarray[ii1]<rlim)
		    {
		      tmp = frequStep*(xarray[ii1]*i1+yarray[ii1]*(j1-NBfrequ1D+1));
		      tmpc = cos(tmp);
		      tmps = sin(tmp);
		      data.image[IDfit].array.F[ii1] += coeffc*tmpc;
		      data.image[IDfit].array.F[ii1] += coeffs*tmps;
		    }
		}
	  }
    

      for(i=0;i<NBfrequ1D;i++)
	for(j=0;j<2*NBfrequ1D-1;j++)
	  {
	    tmpc1 = 0.0;
	    tmpc2 = 0.0;
	    tmps1 = 0.0;
	    tmps2 = 0.0;
	    for(ii=0;ii<size;ii+=step)
	      for(jj=0;jj<size;jj+=step)
		{
		  ii1 = jj*size+ii;
		  if(rarray[ii1]<rlim)
		    {
		      tmp = frequStep*(xarray[ii1]*i+yarray[ii1]*(j-NBfrequ1D+1));
		      tmpc = cos(tmp);
		      tmps = sin(tmp);
		      
		      tmpc1 += tmpc*tmpc;
		      tmpc2 += data.image[IDres].array.F[ii1]*tmpc;
		      
		      tmps1 += tmps*tmps;
		      tmps2 += data.image[IDres].array.F[ii1]*tmps;
		    }
		}
	    if(tmpc1>1e-8)
	      tmpc = tmpc2/tmpc1;
	    else
	      tmpc = 0.0;

	    if(tmps1>1e-8)
	      tmps = tmps2/tmps1;
	    else
	      tmps = 0.0;

	    //  printf("%ld (%ld,%ld) : %g %g\n",iter,i,(j-NBfrequ1D+1),tmpc,tmps);
	    coscoeff[j*NBfrequ1D+i] += gain*tmpc;
	    sincoeff[j*NBfrequ1D+i] += gain*tmps;
	    gtmpc = gain*tmpc;
	    gtmps = gain*tmps;
	    
	    
	    for(ii=0;ii<size;ii+=step)
	      for(jj=0;jj<size;jj+=step)
		{
		  ii1 = jj*size+ii;
		  if(rarray[ii1]<1.0)
		    {
		      tmp = frequStep*(xarray[ii1]*i+yarray[ii1]*(j-NBfrequ1D+1));
		      tmpc = cos(tmp);
		      tmps = sin(tmp);
		      data.image[IDfit].array.F[ii1] += gtmpc*tmpc;
		      data.image[IDfit].array.F[ii1] += gtmps*tmps;
		    }
		}
	  
	    
	    error = 0.0;
	    errorcnt = 0;
	    for(ii=0;ii<size;ii+=step)
	      for(jj=0;jj<size;jj+=step)
		{
		  ii1 = jj*size+ii;
		  if(rarray[ii1]<1.0)
		    {
		      data.image[IDres].array.F[ii1] = data.image[ID].array.F[ii1]-data.image[IDfit].array.F[ii1];
		      if(rarray[ii1]<rlim)
			{
			  error += data.image[IDres].array.F[ii1]*data.image[IDres].array.F[ii1];
			  errorcnt ++;
			}
		    }
		}
	    
	  }
      printf("iter %ld / %ld   error = %g\n",iter,NBiter,sqrt(error/errorcnt));
      save_fl_fits("fitim","!fitim");
      save_fl_fits("residual","!residual");
      
      fp = fopen("fitcoeff.dat","w");
      for(i=0;i<NBfrequ1D;i++)
	for(j=0;j<2*NBfrequ1D-1;j++)
	  fprintf(fp,"%ld %ld %ld %.20g %.20g\n",i,j,j-NBfrequ1D+1,coscoeff[j*NBfrequ1D+i],sincoeff[j*NBfrequ1D+i]);
      fclose(fp);
    }
  free(coscoeff);
  free(sincoeff);
  free(rarray);

  return(0);
}

int filter_fit2DcosKernel(char *IDname, float radius)
{
  long ID,ID1,ID2,ID3;
  long size;
  long NBgridpts1D = 20;
  long NBgridpts;
  long ii,jj,i,j;
  float *x0array;
  float *y0array;
  float *Varraytmp;
  float *Varray;
  float *Varraycnt;
  float x,y,r,x1,y1;
  float xstep,ystep;
  float value;
  long cnt;
  float error;
  long NBiter = 10;
  long iter;
  float cosa,tmp,tmp1,tmp2;
  float Cs;

  float CX1_1 = -3.52106e-05;
  float  CX2_1 = 0.000104827;
  float  CX3_1 = -0.000156806;
  float  CX4_1 = 0.000106682;
  float  CX5_1 = -2.61437e-05;
  
  float  CX1_2 = 3.16388e-05;
  float  CX2_2 = -0.000125175;
  float  CX3_2 = 0.000203591;
  float  CX4_2 = -0.000146805;
  float  CX5_2 = 3.87538e-05;
  
  // filter_fit1D("prof1m.dat",199);
  // exit(0);

  filter_fit2Dcossin(IDname,radius);
  
  exit(0);

  NBgridpts = NBgridpts1D*NBgridpts1D;

  x0array = (float*) malloc(sizeof(float)*NBgridpts);
  y0array = (float*) malloc(sizeof(float)*NBgridpts);
  Varray = (float*) malloc(sizeof(float)*NBgridpts);
  Varraytmp = (float*) malloc(sizeof(float)*NBgridpts);
  Varraycnt = (float*) malloc(sizeof(float)*NBgridpts);


  for(i=0;i<NBgridpts1D;i++)
    for(j=0;j<NBgridpts1D;j++)
      {
	x0array[j*NBgridpts1D+i] = -1.0+2.0*i/(NBgridpts1D-1);
	y0array[j*NBgridpts1D+i] = -1.0+2.0*j/(NBgridpts1D-1);
	Varray[j*NBgridpts1D+i] = 0.0;
 	Varraycnt[j*NBgridpts1D+i] = 0.0;
	//	printf("%ld %ld  %f %f\n",i,j,x0array[j*NBgridpts1D+i],y0array[j*NBgridpts1D+i]);
      }
  xstep = x0array[0*NBgridpts1D+1]-x0array[0*NBgridpts1D+0];
  ystep = y0array[1*NBgridpts1D+0]-y0array[0*NBgridpts1D+0];

  ID = image_ID(IDname);
  size = data.image[ID].md[0].size[0];

  
  ID1 = create_2Dimage_ID("testim",size,size);
  ID2 = create_2Dimage_ID("fitim",size,size);
  ID3 = create_2Dimage_ID("residual",size,size);

  for(ii=0;ii<size;ii++)
    for(jj=0;jj<size;jj++)
      {
	x = (1.0*ii-size/2)/radius;
	y = (1.0*jj-size/2)/radius;
	r = sqrt(x*x+y*y);
	cosa = x/(r+0.000001);
	Cs = 3.34e-05;
	tmp1 = r*CX1_1 + r*r*CX2_1 + r*r*r*CX3_1 + r*r*r*r*CX4_1 + r*r*r*r*r*CX5_1;
	tmp2 = r*CX1_2 + r*r*CX2_2 + r*r*r*CX3_2 + r*r*r*r*CX4_2 + r*r*r*r*r*CX5_2;
	
	//	Cs = 3.3e-05;
	//	tmp1 = -r*Cs + r*r*0.000104827 - r*r*r*0.000156806 + r*r*r*r*0.000106682 - r*r*r*r*r*2.61437e-05;
	//	tmp2 = r*Cs - r*r*0.000125175 + r*r*r*0.000203591 - r*r*r*r*0.000146805 + r*r*r*r*r*3.87538e-05;

	//	tmp1 = -2.70e-5*exp(-7.0*pow((r-0.013),2.0))*r-1.05e-7;
	//	tmp2 = -2.70e-5*exp(-7.0*pow(((-r)-0.013),2.0))*(-r)-1.05e-7;

	if(r<1.0)
	  tmp = tmp1*(1.0+cosa)/2.0 + tmp2*(1.0-cosa)/2.0;
	else
	  tmp = 0.0;
	tmp = 0.0;
	// for 15
	//tmp += 4.8e-6*exp(-280.0*(x-0.007)*(x-0.007))*(x-0.007)*exp(-200.0*y*y);
	tmp += -1.2e-7*exp(-80.0*r*r)+1.4e-7*exp(-40.0*r*r);

	data.image[ID1].array.F[jj*size+ii] = tmp;
	data.image[ID2].array.F[jj*size+ii] = data.image[ID1].array.F[jj*size+ii];
	data.image[ID3].array.F[jj*size+ii] = data.image[ID].array.F[jj*size+ii]-data.image[ID2].array.F[jj*size+ii];
      }

  save_fl_fits("fitim","!fitim");
  save_fl_fits("residual","!residual0");
  //   exit(0);

  for(iter=0;iter<NBiter;iter++)
    {
      for(i=0;i<NBgridpts1D;i++)
	for(j=0;j<NBgridpts1D;j++)
	  {
	    Varraytmp[j*NBgridpts1D+i] = 0.0;
	    Varraycnt[j*NBgridpts1D+i] = 0.0;
	  }
      
      for(ii=0;ii<size;ii++)
	for(jj=0;jj<size;jj++)
	  {
	    x = (1.0*ii-size/2)/radius;
	    y = (1.0*jj-size/2)/radius;
	    r = sqrt(x*x+y*y);
	    if(r<1.0)
	      for(i=0;i<NBgridpts1D;i++)
		for(j=0;j<NBgridpts1D;j++)
		  {
		    x1 = (x-x0array[j*NBgridpts1D+i])/xstep;
		    y1 = (y-y0array[j*NBgridpts1D+i])/ystep;
		    if((fabs(x1)<1.0)&&(fabs(y1)<1.0))
		      {
			//value = (fabs(x1)-1.0)*(fabs(y1)-1.0); //0.25*(cos(x1*PI)+1.0)*(cos(y1*PI)+1.0);		  
			value = 0.25*(cos(x1*PI)+1.0)*(cos(y1*PI)+1.0);
			Varraytmp[j*NBgridpts1D+i] += value*data.image[ID3].array.F[jj*size+ii];
			Varraycnt[j*NBgridpts1D+i] += value;
		      }
		  }	    
	  }
      for(i=0;i<NBgridpts1D;i++)
	for(j=0;j<NBgridpts1D;j++)
	  {
	    if(Varraycnt[j*NBgridpts1D+i]>1.0)
	      Varraytmp[j*NBgridpts1D+i] /= Varraycnt[j*NBgridpts1D+i];
	    else
	      Varraytmp[j*NBgridpts1D+i] = 0.0;
	  }
      
      for(i=0;i<NBgridpts1D;i++)
	for(j=0;j<NBgridpts1D;j++)
	  Varray[j*NBgridpts1D+i] += Varraytmp[j*NBgridpts1D+i];
      
      

      for(ii=0;ii<size;ii++)
	for(jj=0;jj<size;jj++)
	  data.image[ID2].array.F[jj*size+ii] = data.image[ID1].array.F[jj*size+ii];

      for(ii=0;ii<size;ii++)
	for(jj=0;jj<size;jj++)
	  {
	    x = (1.0*ii-size/2)/radius;
	    y = (1.0*jj-size/2)/radius;
	    r = sqrt(x*x+y*y);
	    if(r<1.0)
	      for(i=0;i<NBgridpts1D;i++)
		for(j=0;j<NBgridpts1D;j++)
		  {
		    x1 = (x-x0array[j*NBgridpts1D+i])/xstep;
		    y1 = (y-y0array[j*NBgridpts1D+i])/ystep;
		    if((fabs(x1)<1.0)&&(fabs(y1)<1.0))
		      {
			//value = (fabs(x1)-1.0)*(fabs(y1)-1.0);
			value = 0.25*(cos(x1*PI)+1.0)*(cos(y1*PI)+1.0);
			data.image[ID2].array.F[jj*size+ii] += value*Varray[j*NBgridpts1D+i];
		      }
		  }	    
	  }
      cnt = 0;
      error = 0.0;
      for(ii=0;ii<size;ii++)
	for(jj=0;jj<size;jj++)
	  {
	    x = (1.0*ii-size/2)/radius;
	    y = (1.0*jj-size/2)/radius;
	    r = sqrt(x*x+y*y);
	    if(r<1.0)	
	      {
		data.image[ID3].array.F[jj*size+ii] = data.image[ID].array.F[jj*size+ii]-data.image[ID2].array.F[jj*size+ii];
		error += data.image[ID3].array.F[jj*size+ii]*data.image[ID3].array.F[jj*size+ii];
		cnt ++;
	      }
	  }
      printf("Iteration %ld / %ld    error = %g RMS\n",iter,NBiter,sqrt(error/cnt));
      
      save_fl_fits("residual","!residual");
      save_fl_fits("fitim","!fitim");
    }

  free(x0array);
  free(y0array);
  free(Varray);
  free(Varraycnt);

  return(0);
}

long filter_CubePercentile(char *IDcin_name, float perc, char *IDout_name)
{
  long IDcin;
  long IDout;
  long xsize, ysize, zsize;
  long ii,kk;
  float *array;
  
  IDcin = image_ID(IDcin_name);
  xsize = data.image[IDcin].md[0].size[0];
  ysize = data.image[IDcin].md[0].size[1];
  zsize = data.image[IDcin].md[0].size[2];
  array = (float*) malloc(sizeof(float)*xsize*ysize);

  IDout = create_2Dimage_ID(IDout_name, xsize, ysize);
  for(ii=0;ii<xsize*ysize;ii++)
    {
      for(kk=0;kk<zsize;kk++)
	array[kk] = data.image[IDcin].array.F[kk*xsize*ysize+ii];
       
      quick_sort_float(array,zsize);
      data.image[IDout].array.F[ii] = array[(long) (perc*zsize)];
    }  
  
  free(array);

  return(IDout);
}

long filter_CubePercentileLimit(char *IDcin_name, float perc, float limit, char *IDout_name)
{
  long IDcin;
  long IDout;
  long xsize, ysize, zsize;
  long ii,kk;
  float *array;
  long cnt;
  float v1;

  IDcin = image_ID(IDcin_name);
  xsize = data.image[IDcin].md[0].size[0];
  ysize = data.image[IDcin].md[0].size[1];
  zsize = data.image[IDcin].md[0].size[2];
  array = (float*) malloc(sizeof(float)*xsize*ysize);

  IDout = create_2Dimage_ID(IDout_name, xsize, ysize);
  for(ii=0;ii<xsize*ysize;ii++)
    {
      cnt = 0;
      for(kk=0;kk<zsize;kk++)
	{
	  v1 = data.image[IDcin].array.F[kk*xsize*ysize+ii];
	  if(v1<limit)
	    {
	      array[cnt] = v1;
	      cnt ++;
	    }
       
	  if(cnt>0)
	    {
	      quick_sort_float(array,zsize);
	      data.image[IDout].array.F[ii] = array[(long) (perc*cnt)];
	    }
	  else
	    {
	      data.image[IDout].array.F[ii] = limit;
	    }
	}
    }  
  
  free(array);

  return(IDout);
}
