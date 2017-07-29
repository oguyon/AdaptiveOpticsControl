#include <stdint.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include <string.h>

#include "CLIcore.h"

#include "COREMOD_memory/COREMOD_memory.h"
#include "info/info.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "image_gen/image_gen.h"
#include "image_basic/image_basic.h"
#include "image_filter/image_filter.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "fft/fft.h"


#include "psf/psf.h"

extern DATA data;

double FWHM_MEASURED;



// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//





int_fast8_t PSF_sequence_measure_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,1)+CLI_checkarg(3,3)==0)
    {
      PSF_sequence_measure(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf, data.cmdargtoken[3].val.string);
      return 0;
    }
  else
    return 1;
}





int_fast8_t init_psf()
{
		
  strcpy(data.module[data.NBmodule].name,__FILE__);
  strcpy(data.module[data.NBmodule].info,"memory management for images and variables");
  data.NBmodule++;

	
  strcpy(data.cmd[data.NBcmd].key,"psfseqmeas");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = PSF_sequence_measure_cli;
  strcpy(data.cmd[data.NBcmd].info,"measure PSF sequence");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image cube> <estimated PSF size> <output file>");
  strcpy(data.cmd[data.NBcmd].example,"psfseqmeas imc 20.0 outimc.txt");
  strcpy(data.cmd[data.NBcmd].Ccall,"int PSF_sequence_measure(const char *IDin_name, float PSFsizeEst, const char *outfname)"); 
  data.NBcmd++;

}



// make a chromatic PSF, assuming an achromatic amplitude and OPD in the pupil
// the phase is secified for the wavelength lambda0
// lamda goes from lambda0*coeff1 to lambda0*coeff2
long PSF_makeChromatPSF(const char *amp_name, const char *pha_name, float coeff1, float coeff2, long NBstep, float ApoCoeff, const char *out_name)
{
    long ID_out;
    long xsize,ysize;
    long IDamp,IDpha;
    //  float lambdafact;
    long step;
    float x,y,u,t;
    long ID_in;
    long ii,jj,i,j;
    float coeff,mcoeff,tmp;
    float eps = 1.0e-5;


    IDamp = image_ID(amp_name);
    IDpha = image_ID(pha_name);

    xsize = data.image[IDamp].md[0].size[0];
    ysize = data.image[IDamp].md[0].size[1];

    if((data.image[IDpha].md[0].size[0]!=xsize)||(data.image[IDpha].md[0].size[0]!=xsize))
    {
        printf("ERROR in makeChromatPSF: images %s and %s have different sizes\n",amp_name,pha_name);
        exit(0);
    }

    ID_out = create_2Dimage_ID(out_name,xsize,ysize);
    list_image_ID();

    for(step = 0; step < NBstep; step ++)
    {
        fprintf(stdout,"\rMake chromatic PSF [%3ld]: %.2f %s completed",step,100.0*step/NBstep,"%");
        fflush(stdout);
        coeff = coeff1*pow(pow((coeff2/coeff1),1.0/(NBstep-1)),step);// + (coeff2-coeff1)*(1.0*step/(NBstep-1));
        x = (coeff - (coeff1+coeff2)/2.0)/((coeff2-coeff1)/2.0);
        // x goes from -1 to 1
        if(ApoCoeff > eps)
            mcoeff = pow((1.0-pow((fabs(x)-(1.0-ApoCoeff))/ApoCoeff,2.0)),4.0);
        else
            mcoeff = 1.0;

        if((1.0-x*x)<eps)
            mcoeff = 0.0;
        if(fabs(x)<ApoCoeff)
            mcoeff = 1.0;
        //      fprintf(stdout,"(%f %f %f %f %f)",coeff,coeff1,coeff2,x,mcoeff);

        arith_image_cstmult(pha_name,coeff,"phamult");
        mk_complex_from_amph(amp_name, "phamult", "tmpimc", 0);
        delete_image_ID("phamult");
        permut("tmpimc");
        do2dfft("tmpimc","tmpimc1");
        delete_image_ID("tmpimc");
        permut("tmpimc1");
        mk_amph_from_complex("tmpimc1", "tmpamp", "tmppha", 0);
        delete_image_ID("tmpimc1");
        delete_image_ID("tmppha");
        arith_image_cstpow("tmpamp",2.0,"tmpint");
        delete_image_ID("tmpamp");
        list_image_ID();
        ID_in = image_ID("tmpint");
        for(ii=0; ii<xsize; ii++)
            for(jj=0; jj<ysize; jj++)
            {
                x = (1.0*(ii-xsize/2)*coeff)+xsize/2;
                y = (1.0*(jj-ysize/2)*coeff)+ysize/2;
                i = (long) x;
                j = (long) y;
                u = x-i;
                t = y-j;
                if((i<xsize-1)&&(j<ysize-1)&&(i>-1)&&(j>-1))
                {
                    tmp = (1.0-u)*(1.0-t)*data.image[ID_in].array.F[j*xsize+i];
                    tmp += (1.0-u)*t*data.image[ID_in].array.F[(j+1)*xsize+i];
                    tmp += u*(1.0-t)*data.image[ID_in].array.F[j*xsize+i+1];
                    tmp += u*t*data.image[ID_in].array.F[(j+1)*xsize+i+1];
                    data.image[ID_out].array.F[jj*xsize+ii] += mcoeff*tmp/coeff/coeff;
                }
            }
        delete_image_ID("tmpint");
    }

    printf("\n");

    return(ID_out);
}





int PSF_finddiskcent(const char *ID_name, float rad, float *result)
{
  // minimizes flux outside disk
  long ii;
  float xc,yc,xcb,ycb;
  long ID,IDd;
  long size;
  float step;
  float totin,totout;
  float v,value,bvalue;
  float xcstart,xcend,ycstart,ycend;
  long iter;
  long NBiter = 20;

  ID = image_ID(ID_name);
  size = data.image[ID].md[0].size[0];
  step = 0.1*size;

  xcstart = 0.0;
  xcend = 1.0*size;
  ycstart = 0.0;
  ycend = 1.0*size;

  xcb = 0.0;
  ycb = 0.0;

  bvalue = arith_image_total(ID_name);
  for(iter=0;iter<NBiter;iter++)
    {
      fprintf(stderr,"iter %ld / %ld  (%f %f  %f %f   %f %f) %g\n",iter,NBiter,xcstart,xcend,ycstart,ycend,xcb,ycb,bvalue);
      for(xc=xcstart;xc<xcend;xc+=step)
	for(yc=ycstart;yc<ycend;yc+=step)
	  {
	    IDd = make_subpixdisk("tmpd1",size,size,xc,yc,rad);
	    
	    totin = 0.0;
	    totout = 0.0;
	    for(ii=0;ii<size*size;ii++)
	      {
		v = data.image[ID].array.F[ii];
		if(data.image[IDd].array.F[ii] > 0.5)
		  totin += v;
		else
		  totout += v;
	      }
	    value = totout;
	    if(value<bvalue)
	      {
		xcb = xc;
		ycb = yc;
		bvalue = value;
	      }
	    delete_image_ID("tmpd1");
	  }
      xcstart = 0.5*(xcstart+xcb);
      xcend = 0.5*(xcend+xcb);
      ycstart = 0.5*(ycstart+ycb);
      ycend = 0.5*(ycend+ycb);
      step *= 0.5;
    }

  printf("Disk center = %f x %f\n",xcb,ycb);
  result[0] = xcb;
  result[1] = ycb;

  return(0);
}




int PSF_finddiskcent_alone(const char *ID_name, float rad)
{
  float *result;

  result = (float*) malloc(sizeof(float)*2);
  PSF_finddiskcent(ID_name,rad,result);
  free(result);

  return(0);
}


int PSF_measurePhotocenter(const char *ID_name)
{
  int ID;
  long ii,jj;
  long naxes[2];
  float iitot,jjtot,tot;
  float v;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  
  iitot = 0.0;
  jjtot = 0.0;
  tot = 0.0;
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	v = data.image[ID].array.F[jj*naxes[1]+ii];
	tot += v;
	iitot += v*ii;
	jjtot += v*jj;
      }

  printf("photocenter = %.2f %.2f\n",iitot/tot,jjtot/tot);
  data.FLOATARRAY[0] = iitot/tot;
  data.FLOATARRAY[1] = jjtot/tot;

  return(0);
}




float measure_enc_NRJ(const char *ID_name, float xcenter, float ycenter, float fraction)
{
  int ID;
  long ii,jj;
  long naxes[2];
  long nelements;
  float *total;
  float distance;
  long index;
  float sum;
  float sum_all;
  long arraysize;
  float value;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0] * naxes[1]; 
 
  arraysize = (long) (sqrt(2)*naxes[0]);
  total = (float*) malloc(sizeof(float)*arraysize);
  for(ii=0;ii<arraysize;ii++)
    total[ii]=0.0;
      
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	distance = sqrt((1.0*ii-xcenter)*(1.0*ii-xcenter)+(1.0*jj-ycenter)*(1.0*jj-ycenter));
	index = (long) distance;
	if(index<arraysize)
	  total[index] += data.image[ID].array.F[jj*naxes[0]+ii];
      }
  
  sum_all = 0.0;
  for(ii=0;ii<arraysize;ii++)
    {
      sum_all += total[ii];
    }
  
  sum = 0.0;
  sum_all *= fraction;
  ii=0;
  while(sum<sum_all)
    {
      sum += total[ii];
      ii++;
    }

  /*  printf("%ld %f %f\n",ii,total[ii-1],sum_all);*/
  value = 1.0*(ii-2)+(sum_all-(sum-total[ii-1]))/(total[ii-1]);
  printf("Enc. NRJ = %f pix\n",value);
  free(total);

  return(value);
}




int measure_enc_NRJ1(const char *ID_name, float xcenter, float ycenter, const char *filename)
{
  int ID;
  long ii,jj;
  long naxes[2];
  long nelements;
  float *total;
  float distance;
  long index;
  float sum_all;
  long arraysize;
  FILE *fp;
  float *ENCNRJ;
  
  printf("Center is %f %f\n",xcenter,ycenter);
  
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0] * naxes[1]; 
 
  arraysize = (long) (sqrt(2)*naxes[0]);
  total = (float*) malloc(sizeof(float)*arraysize);
  ENCNRJ = (float*) malloc(sizeof(float)*arraysize);
  for(index=0;index<arraysize;index++)
    {
      ENCNRJ[index]=0.0;
      total[index]=0.0;
    }
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	distance = sqrt((1.0*ii-xcenter)*(1.0*ii-xcenter)+(1.0*jj-ycenter)*(1.0*jj-ycenter));
	index = (long) distance;
	if(index<arraysize)
	  total[index] += data.image[ID].array.F[jj*naxes[0]+ii];
      }
  
  if((fp=fopen(filename,"w"))==NULL)
    {
      printf("ERROR: cannot create file \"%s\"\n",filename);
      fflush(stdout);
      exit(0);
    }

  sum_all = 0.0;
  for(ii=0;ii<arraysize;ii++)
    {
      ENCNRJ[ii] = sum_all;
      sum_all += total[ii];
      fprintf(fp,"%ld %f\n",ii,ENCNRJ[ii]);
    }
  fclose(fp);

  free(total);
  free(ENCNRJ);

  return(0);
}




/* measures the FWHM of a "perfect" PSF */
float measure_FWHM(const char *ID_name, float xcenter, float ycenter, float step, long nb_step)
{
  int ID;
  long ii,jj;
  long naxes[2];
  long nelements;
  float distance;
  float *dist;
  float *mean;
  float *rms;
  long *counts;
  long i;
  float FWHM;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0] * naxes[1]; 
  dist = (float*) malloc(nb_step*sizeof(float));
  mean = (float*) malloc(nb_step*sizeof(float));
  rms = (float*) malloc(nb_step*sizeof(float));
  counts = (long*) malloc(nb_step*sizeof(long));

  for (i=0;i<nb_step;i++)
    {
      dist[i] = 0;
      mean[i] = 0;
      rms[i] = 0;
      counts[i] = 0;
    }

  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++){
      distance = sqrt((1.0*ii-xcenter)*(1.0*ii-xcenter)+(1.0*jj-ycenter)*(1.0*jj-ycenter));
      i = (long) distance/step;
      if(i<nb_step)
        {
          dist[i] += distance;
          mean[i] += data.image[ID].array.F[jj*naxes[0]+ii];
          rms[i] += data.image[ID].array.F[jj*naxes[0]+ii]*data.image[ID].array.F[jj*naxes[0]+ii];
          counts[i] += 1;
        }
    }

  for (i=0;i<nb_step;i++)
    {
      dist[i] /= counts[i];
      mean[i] /= counts[i];
      rms[i] = sqrt(rms[i]-1.0*counts[i]*mean[i]*mean[i])/sqrt(counts[i]);
   }

  FWHM = 0.0;
  for (i=0;i<nb_step;i++)
    {
      if((mean[i+1]<mean[0]/2)&&(mean[i]>mean[0]/2))
	{
	  FWHM = 2.0*dist[i]+(dist[i+1]-dist[i])*(mean[i]-mean[0]/2)/(mean[i]-mean[i+1]);
	}
    }

  free(counts);
  free(dist);
  free(mean);
  free(rms);

  return(FWHM);
}



/* finds a PSF center with no a priori position information */
int center_PSF(const char *ID_name, double *xcenter, double *ycenter, long box_size)
{
  long ID;
  long n3; /* effective box size. =box_size if the star is not at the edge of the image field */
  double back_cont;
  double centerx,centery;
  double ocenterx,ocentery;
  double total_fl;
  long ii,jj;
  long naxes[2];
  int k;
  int nbiter = 10;
  long iistart,iiend,jjstart,jjend;
  
  n3 = box_size;

  /* for better performance, the background continuum needs to be computed for each image */
  back_cont = arith_image_median(ID_name);
  /* first approximation given by barycenter after median of image */
  median_filter(ID_name,"PSFctmp",1);

  
  ID = image_ID("PSFctmp");
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  
  centerx = (double) naxes[0]/2;
  centery = (double) naxes[1]/2;
  ocenterx = centerx;
  ocentery = centery;


  for (k=0;k<nbiter;k++)
    {
      n3 = (long) (1.0*naxes[0]/2/(1.0+(0.1*naxes[0]/2*k/(4*nbiter))));
 
      iistart = (long) (0.5 + ocenterx - n3);
      if(iistart<0)
	iistart = 0;
      iiend = (long) (0.5 + ocenterx + n3);
      if(iiend > naxes[0]-1)
	iiend = naxes[0]-1;

      jjstart = (long) (0.5 + ocentery - n3);
      if(jjstart<0)
	jjstart = 0;
      jjend = (long) (0.5 + ocentery + n3);
      if(jjend > naxes[1]-1)
	jjend = naxes[1]-1;
      
      //      printf("effective box size is %ld - center is %f %f\n",n3,ocenterx,ocentery);
      // fflush(stdout);
      centerx = 0.0;
      centery = 0.0;
      total_fl = 0.0;
      for (jj = jjstart; jj < jjend; jj++) 
	for (ii = iistart; ii < iiend; ii++) 
	  {
	    if(data.image[ID].array.F[jj*naxes[0]+ii]>back_cont)
	      {
		centerx += 1.0*ii*(data.image[ID].array.F[jj*naxes[0]+ii]-1.0*back_cont);
		centery += 1.0*jj*(data.image[ID].array.F[jj*naxes[0]+ii]-1.0*back_cont);
		total_fl += data.image[ID].array.F[jj*naxes[0]+ii]-1.0*back_cont;
	      }
	  }
      centerx /= total_fl;
      centery /= total_fl;
      
      /*      printf("step %d\n",k);
	      printf("image %s: center is %f %f for %ld by %ld pixels. Total_fl is %f\n",data.image[ID].name,centerx,centery,naxes[0],naxes[1],total_fl);
      */
      ocenterx = centerx;
      ocentery = centery;
    }
  
  delete_image_ID("PSFctmp");


  xcenter[0] = centerx;
  ycenter[0] = centery;

  return(0);
}




/* finds a PSF center with no a priori position information */
int fast_center_PSF(const char *ID_name, double *xcenter, double *ycenter, long box_size)
{
  int ID;
  long n3; /* effective box size. =box_size if the star is not at the edge of the image field */
  double centerx,centery;
  double ocenterx,ocentery;
  double total_fl;
  long ii,jj;
  long naxes[2];
  int k;
  int nbiter = 6;

	long iimin, iimax, jjmin, jjmax;


  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  
  centerx = (double) naxes[0]/2;
  centery = (double) naxes[1]/2;
  ocenterx = centerx;
  ocentery = centery;

  for (k=0;k<nbiter;k++)
    {
      n3 = (long) (1.0*naxes[0]/2/(1.0+(0.1*naxes[0]/2*k/(4*nbiter))));
      if (((long) (0.5+ocenterx) - n3) < 0)
		n3 = (long) (0.5+ocenterx);
      if (((long) (0.5+ocenterx) + n3+1) > naxes[0])
		n3 = naxes[0]-((long) (0.5+ocenterx) +1);
      if (((long) (0.5+ocentery) - n3) < 0)
		n3 = (long) (0.5+ocentery);
      if (((long) (0.5+ocentery) + n3+1) > naxes[1])
		n3 = naxes[1]-((long) (0.5+ocentery) +1);
      n3 -= 1;
      
      if(n3<box_size)
		n3 = box_size;
      

      
      centerx = 0.0;
      centery = 0.0;
      total_fl = 0.0;
      
		iimin = ((long) (0.5+ocenterx) - n3);
		if(iimin < 0)
			iimin = 0.0;
		iimax = ((long) (0.5+ocenterx) + n3+1);
		if( iimax > naxes[0]-1 )
			iimax = naxes[0]-1;
	
		jjmin = ((long) (0.5+ocentery) - n3);
		if(jjmin < 0)
			jjmin = 0.0;
		jjmax = ((long) (0.5+ocentery) + n3+1);
		if(jjmax > naxes[1]-1 )
			jjmax = naxes[1]-1;
		
      for (jj = jjmin; jj < jjmax; jj++) 
		for (ii = iimin; ii < iimax; ii++) 
		{
			centerx += 1.0*ii*(data.image[ID].array.F[jj*naxes[0]+ii]);
			centery += 1.0*jj*(data.image[ID].array.F[jj*naxes[0]+ii]);
			total_fl += data.image[ID].array.F[jj*naxes[0]+ii];
		}

//        printf("effective box size is %ld (%ld) - center is %f %f   [ %3ld %3ld   %3ld %3ld]   ", n3, box_size, ocenterx, ocentery, iimin, iimax, jjmin, jjmax);
//       fflush(stdout);


  //    printf("total_fl is %f\n",total_fl);
      centerx /= total_fl;
      centery /= total_fl;
      
      //printf("step %d\n",k);
      //printf("image %s: center is %f %f for %ld by %ld pixels. Total_fl is %f\n",data.image[ID].name,centerx,centery,naxes[0],naxes[1],total_fl);
      
      ocenterx = centerx;
      ocentery = centery;
    }

  xcenter[0] = centerx;
  ycenter[0] = centery;

  return(0);
}





int center_PSF_alone(const char *ID_name)
{
  int ID;
  double *xcenter;
  double *ycenter;
  long box_size;
  long naxes[2];

  xcenter = (double*) malloc(sizeof(double));
  ycenter = (double*) malloc(sizeof(double));
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];

  xcenter[0] = naxes[0]/2;
  ycenter[0] = naxes[1]/2;
  box_size = naxes[0]/3 - 1;

  /*remove_cosmics(ID_name,"tmpcen");*/
  copy_image_ID(ID_name, "tmpcen", 0);

  arith_image_trunc("tmpcen", img_percentile("tmpcen",0.99), img_percentile("tmpcen",1.0), "tmpcen1");
  delete_image_ID("tmpcen");

  center_PSF("tmpcen1", xcenter, ycenter, box_size);  
  delete_image_ID("tmpcen1");

  printf("center : %f %f\n", xcenter[0], ycenter[0]);

  create_variable_ID("xc", xcenter[0]);
  create_variable_ID("yc", ycenter[0]);

  free(xcenter);
  free(ycenter);

  return(0);
}




/* this simple routine finds the center of a PSF by barycenter technique */
int center_star(const char *ID_in_name, double *x_star, double *y_star)
{
  int ID_in;
  long ii,jj;
  long naxes[2];
  long n1, n2, n3;
  /* n2,n3 are the pixel coordinate, n3 is the pixel radius of the sampling box used.*/
  double sum,coeff;
  double xsum, ysum;
  int max_nb_iter = 500;
  int i,found;
  double limit;

  limit = 1.0/10000000000.0;

  ID_in = image_ID(ID_in_name);
  naxes[0] = data.image[ID_in].md[0].size[0];
  naxes[1] = data.image[ID_in].md[0].size[1];
  n1 = (long) x_star[0];
  n2 = (long) y_star[0];
  n3 = 20;
  
  i = 0;
  found = 0;
  while ((i<max_nb_iter)&&(found==0))
    {
      xsum  = 0;
      ysum  = 0;
      sum = 0;
      for (jj = (n2-n3); jj < (n2+n3); jj++)
	for (ii = (n1-n3); ii < (n1+n3); ii++)
	  {
	    coeff = (ii-x_star[0])*(ii-x_star[0])+(jj-y_star[0])*(jj-y_star[0]);
	    coeff = coeff/n3/n3;
	    coeff = exp(-coeff*50*(1.0*i/max_nb_iter));
	    sum = sum + coeff*data.image[ID_in].array.F[jj*naxes[0]+ii];
	    xsum  = xsum + coeff*(data.image[ID_in].array.F[jj*naxes[0]+ii]) * ii;
	    ysum  = ysum + coeff*(data.image[ID_in].array.F[jj*naxes[0]+ii]) * jj;
	  } 
      xsum  = xsum / sum;
      ysum  = ysum / sum;
      if (((x_star[0]-xsum)*(x_star[0]-xsum)+(y_star[0]-ysum)*(y_star[0]-ysum))<(limit*limit*n3*n3))
	found = 1;
      x_star[0] = xsum;
      y_star[0] = ysum;
      n1 = (long) xsum;
      n2 = (long) ysum;
      /*    printf(" i=%d x=%e  y=%e  sum=%20.18e \n",i,xsum,ysum,sum);*/
      i++;
   }
  
  x_star[0] = xsum;
  y_star[0] = ysum;
  printf("%f %f\n", xsum, ysum);
 
  return(0);
}





float get_sigma(const char *ID_name, float x, float y, const char *options)
{
  int ID,i;
  long naxes[2],ii,jj;
  float a,C,distsq;
  int n3=30;
  /* , nb_iter=40; */
  /* float sum,count; */
  long n1,n2;
  int str_pos;
  char boxsize[50];
  float *x1;
  float *y1;
  float *sig;
  long nbpixel;
  long pixelnb;
  float SATURATION = 50000.0000;
  float best_A,best_err,best_sigma,err,A,sigma,sigmasq;
  float dist[100];
  float value[100];
  int count[100];
  float peak,FWHM_m;
  int getmfwhm;
  int backg;
  
  printf("get_sigma .... ");
  fflush(stdout);
  if (strstr(options,"-box ")!=NULL)
    {
      str_pos=strstr(options,"-box ")-options;
      str_pos = str_pos + strlen("-box ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  boxsize[i] = options[i+str_pos];
	  i++;
	}
      boxsize[i] = '\0';
      n3 = atoi(boxsize);
      printf("box radius is %d pixels \n",n3);
    }

  getmfwhm=0;
  if (strstr(options,"-mfwhm ")!=NULL)
    {
      getmfwhm=1;
    }

  backg=0;
  if (strstr(options,"-backg ")!=NULL)
    {
      backg=1;
    }

  if (strstr(options,"-sat ")!=NULL)
    {
      str_pos=strstr(options,"-sat ")-options;
      str_pos = str_pos + strlen("-sat ");
      i=0;
      while((options[i+str_pos]!=' ')&&(options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
	{
	  boxsize[i] = options[i+str_pos];
	  i++;
	}
      boxsize[i] = '\0';
      SATURATION = atof(boxsize);
      printf("saturation level is %f\n",SATURATION);
    }

  n1 = (long) x;
  n2 = (long) y;
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  a = 10.0;
  C = 0.0;
  A = data.image[ID].array.F[n2*naxes[0]+n1+1];
  printf("A initial is %f\n",A);
  /* f = Aexp(-a*a*x*x)+C */

  nbpixel = 0;
  for (jj = (n2-n3); jj < (n2+n3); jj++)
    for (ii = (n1-n3); ii < (n1+n3); ii++)
      if((ii>0)&&(ii<naxes[0])&&(jj>0)&&(jj<naxes[1]))
	nbpixel += 1;
  x1 = (float*) malloc(nbpixel*sizeof(float));
  y1 = (float*) malloc(nbpixel*sizeof(float));
  sig = (float*) malloc(nbpixel*sizeof(float));

  printf("background is ");
  fflush(stdout);
  if(backg==1)
    C=0.0;
  else
    C = img_percentile(ID_name,0.5);
  printf("%f\n",C);
  fflush(stdout);
  pixelnb = 0;

  for (jj = (n2-n3); jj < (n2+n3); jj++)
    for (ii = (n1-n3); ii < (n1+n3); ii++)
      if((ii>0)&&(ii<naxes[0])&&(jj>0)&&(jj<naxes[1]))
	{
	  distsq = (ii-x)*(ii-x)+(jj-y)*(jj-y);
	  if(data.image[ID].array.F[jj*naxes[0]+ii]<SATURATION)
	    {
	      x1[pixelnb] = distsq;
	      y1[pixelnb] = data.image[ID].array.F[jj*naxes[0]+ii];
	      sig[pixelnb] = 1.0;
	      pixelnb++;
	    }
	}

  /* do the radial average */
  for(i=0;i<100;i++)
    {
      count[i]=0.0;
      dist[i]=0.0;
      value[i]=0.0;
    }

  for(i=0;i<pixelnb;i++)
    {
      count[(int) (sqrt(x1[i]))]++;
      dist[(int) (sqrt(x1[i]))] += sqrt(x1[i]);
      value[(int) (sqrt(x1[i]))] += y1[i];
    }
  
  for(i=0;i<100;i++)
    {
      dist[i]/=count[i];
      value[i]/=count[i];
    }

  sigma=10.0;
  err=0.0;
  sigmasq=sigma*sigma;
  for(i=0;i<100;i++)
    if(count[i]>0)
      {
	err=err+pow((value[i]-C-A*exp(-dist[i]*dist[i]/sigmasq)),2)*count[i];
      }
  best_err=err;

  best_sigma = 10.0;
  best_A = 1000.0;
  for(sigma=2.0;sigma<50.0;sigma=sigma*1.01)
    for(A=best_A*0.1;A<best_A*10.0;A=A*1.01)
      {
	err=0.0;
	sigmasq=sigma*sigma;
	for(i=0;i<100;i++)
	  if(count[i]>0)
	    {
	      err=err+pow((value[i]-C-A*exp(-dist[i]*dist[i]/sigmasq)),2)*0.00001*count[i];
	    }
	if(err<best_err)
	  {
	    best_err=err;
	    best_A = A;
	    best_sigma = sigma;
	  }
      }
  
  peak = value[0]-C;
  i=0;
  while((value[i]-C)>(peak/2.0))
    i++;
  FWHM_m = 2.0*dist[i-1]+2.0*(dist[i]-dist[i-1])*(value[i-1]-C-peak/2.0)/(value[i-1]-value[i]);

  printf("PSF center is %f x %f\n",x,y);
  printf("A = %f\n",best_A);
  printf("sigma = %f\n",best_sigma);
  printf("gaussian FWHM = %f\n",2.0*best_sigma*sqrt(log(2)));
  printf("measured peak = %f (background subtracted)\n",peak);
  printf("measured FWHM = %f\n",FWHM_m);
  

  free(sig);
  free(x1);
  free(y1);

  if(getmfwhm==1)
    sigma=FWHM_m;
  else
    sigma=best_sigma;

  return(sigma);
}



float get_sigma_alone(const char *ID_name)
{
  double *xcenter;
  double *ycenter;
  double sigma=0.0;
  long ID;
  long naxes[2];
  long box_size;
  /*  char lstring[1000];*/
  int FAST = 0;
  int FWHM = 0;

  xcenter = (double*) malloc(sizeof(double));
  ycenter = (double*) malloc(sizeof(double));

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];
  
  xcenter[0] = naxes[0]/2;
  ycenter[0] = naxes[1]/2;
  box_size = xcenter[0]/2-1;

  /*  remove_cosmics(ID_name,"tmpcen");*/
  copy_image_ID(ID_name, "tmpcen", 0);

  if(FAST==0)
    {
      arith_image_trunc("tmpcen",img_percentile("tmpcen",0.9),img_percentile("tmpcen",1.0),"tmpcen1");
      delete_image_ID("tmpcen");
      center_PSF("tmpcen1",xcenter,ycenter,box_size);  
      delete_image_ID("tmpcen1");

      sigma = get_sigma(ID_name,xcenter[0],ycenter[0],"");
    }
  else
    {
      fast_center_PSF("tmpcen",xcenter,ycenter,box_size); 
      center_star("tmpcen", xcenter, ycenter);
      printf("peak = %f\n",data.image[ID].array.F[((long) ycenter[0])*naxes[0]+((long) xcenter[0])]);
      if(FWHM==1)
	{     
	  sigma = get_sigma(ID_name,xcenter[0],ycenter[0],"");
	}
    }

  free(xcenter);
  free(ycenter);

  return(sigma);
}




int extract_psf(const char *ID_name, const char *out_name, long size)
{
  long ID;
  double *xcenter;
  double *ycenter;
  long box_size;
  long naxes[2];

  xcenter = (double*) malloc(sizeof(float));
  ycenter = (double*) malloc(sizeof(float));
  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];

  xcenter[0] = naxes[0]/2;
  ycenter[0] = naxes[1]/2;
  box_size = naxes[0]/2 - 1;
  /*remove_cosmics(ID_name,"tmpcen");*/
  copy_image_ID(ID_name, "tmpcen", 0);
  arith_image_trunc("tmpcen",img_percentile("tmpcen",0.99),img_percentile("tmpcen",1.0),"tmpcen1");
  delete_image_ID("tmpcen");
  center_PSF("tmpcen1", xcenter, ycenter, box_size);  

  printf("PSF center = %f %f   extracting window size %ld\n",xcenter[0],ycenter[0],size);
  delete_image_ID("tmpcen1");
  /*  arith_image_extract2D(ID_name,out_name,size,size,((long) (xcenter[0]+0.5))-(size/2),((long) (ycenter[0]+0.5))-(size/2));*/
  
  
  arith_image_extract2D(ID_name,"tmpf",size,size,((long) (xcenter[0]+0.5))-(size/2),((long) (ycenter[0]+0.5))-(size/2));
  
  fft_image_translate("tmpf", out_name,xcenter[0]-((long) (xcenter[0]+0.5)), ycenter[0]-((long) (ycenter[0]+0.5)));
  //arith_image_translate("tmpf", out_name,xcenter[0]-((long) (xcenter[0]+0.5)), ycenter[0]-((long) (ycenter[0]+0.5)));
  
  delete_image_ID("tmpf");
  free(xcenter);
  free(ycenter);

  return(0);
}



long extract_psf_photcent(const char *ID_name, const char *out_name, long size)
{
  long IDin,IDout;
  double totx,toty,tot;
  long naxes[2];
  long ii,jj,ii0,jj0,ii1,jj1;


  IDin = image_ID(ID_name);
  naxes[0] = data.image[IDin].md[0].size[0];
  naxes[1] = data.image[IDin].md[0].size[1];
  

  totx = 0.0;
  toty = 0.0;
  tot = 0.0;
  for(ii=0;ii<naxes[0];ii++)
    for(jj=0;jj<naxes[1];jj++)
      {
	totx += data.image[IDin].array.F[jj*naxes[0]+ii]*ii;
	toty += data.image[IDin].array.F[jj*naxes[0]+ii]*jj;
	tot += data.image[IDin].array.F[jj*naxes[0]+ii];
      }
  totx /= tot;
  toty /= tot;
 
  printf("Photocenter = %lf %lf\n", totx, toty);

  IDout = create_2Dimage_ID(out_name,size,size);
  ii0 = (long) totx;
  jj0 = (long) toty;
  
  for(ii1=0;ii1<size;ii1++)
    for(jj1=0;jj1<size;jj1++)
      {
		ii = ii0-size/2 + ii1;
		jj = jj0-size/2 + jj1;
		if((ii>-1)&&(jj>-1)&&(ii<naxes[0])&&(jj<naxes[1]))
			data.image[IDout].array.F[jj1*size+ii1] = data.image[IDin].array.F[jj*naxes[0]+ii];
		else
			data.image[IDout].array.F[jj1*size+ii1] = 0.0;
      }

  return(IDout);
}



int psf_variance(const char *ID_out_m, const char *ID_out_v, const char *options)
{
  int Nb_files;
  int file_nb;
  long ii,jj,i,j;
  int str_pos;
  int *IDn;
  char file_name[50];
  long naxes[2];
  float mean,tmp,rms;
  long IDoutm,IDoutv;

  Nb_files = 1;
  i=0;
  str_pos=0;
  
  printf("option is :%s\n",options);
  while((options[i+str_pos]!='\n')&&(options[i+str_pos]!='\0'))
    {
      if(options[i+str_pos]==' ')
	Nb_files += 1;
      i++;
    }
  
  printf("%d files\n",Nb_files);
  IDn = (int*) malloc (Nb_files*sizeof(int));
  j = 0;
  i = 0;
  file_nb = 0;
  while(file_nb<Nb_files)
    {
      if((options[i+str_pos]==' ')||(options[i+str_pos]=='\0')||(options[i+str_pos]=='\n'))
	{
	  file_name[j] = '\0';
	  IDn[file_nb] = image_ID(file_name);
	  printf("%d %s \n",IDn[file_nb],file_name);
	  file_nb += 1;
	  j = 0;
	}
      else
	{
	  file_name[j] = options[i+str_pos];	  
	  j++;
	}
      i++;
    }
  naxes[0] = data.image[IDn[0]].md[0].size[0];
  naxes[1] = data.image[IDn[0]].md[0].size[1];

  create_2Dimage_ID(ID_out_m,naxes[0],naxes[1]);
  IDoutm = image_ID(ID_out_m);
  create_2Dimage_ID(ID_out_v,naxes[0],naxes[1]);
  IDoutv = image_ID(ID_out_v); 
  /*  printf("%d %d - starting computations\n",naxes[0],naxes[1]);*/
  fflush(stdout);
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	mean = 0.0;
	for(file_nb=0;file_nb<Nb_files;file_nb++)
	  {
	    mean += data.image[IDn[file_nb]].array.F[jj*naxes[0]+ii];
	  }
	mean /= Nb_files;
	data.image[IDoutm].array.F[jj*naxes[0]+ii] = mean;
	rms =0.0;
	for(file_nb=0;file_nb<Nb_files;file_nb++)
	  {
	    tmp = (mean-data.image[IDn[file_nb]].array.F[jj*naxes[0]+ii]);
	    rms += tmp*tmp;
	  }
	rms = sqrt(rms/Nb_files);
	data.image[IDoutv].array.F[jj*naxes[0]+ii] = rms;
      }

  free(IDn);
 
  return(0);
}




int combine_2psf(const char *ID_name, const char *ID_name1, const char *ID_name2, float radius, float index)
{
  long ID1,ID2,ID;
  long naxes[2];
  long ii,jj;
  float dist;

  ID1=image_ID(ID_name1);
  ID2=image_ID(ID_name2);
  naxes[0] = data.image[ID1].md[0].size[0];
  naxes[1] = data.image[ID1].md[0].size[1];
  create_2Dimage_ID(ID_name,naxes[0],naxes[1]);
  ID= image_ID(ID_name);
  
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	dist = sqrt((ii-naxes[0]/2)*(ii-naxes[0]/2)+(jj-naxes[1]/2)*(jj-naxes[1]/2));
	data.image[ID].array.F[jj*naxes[0]+ii] = exp(-pow(dist/radius,index))*data.image[ID1].array.F[jj*naxes[0]+ii] + (1.0-exp(-pow(dist/radius,index)))*data.image[ID2].array.F[jj*naxes[0]+ii];
      }
  
  return(0);
}




float psf_measure_SR(const char *ID_name, float factor, float r1, float r2)
{
  long ID;
  long Csize = 128;
  long Csize2;
  double *xcenter;
  double *ycenter;
  long box_size;
  long naxes[2];
  double tmp1;
  double SR;
  long ii,jj;
  double peak;
  int fzoomfactor = 2;
  double background;
  double max;

  double total,total1,total2;
  long cnt1,cnt2;
  long peakii,peakjj;
  double dist;

  peakii = 0;
  peakjj = 0;
  Csize2 = Csize*fzoomfactor;
  xcenter = (double*) malloc(sizeof(double));
  ycenter = (double*) malloc(sizeof(double));

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];

  xcenter[0] = naxes[0]/2;
  ycenter[0] = naxes[1]/2;
  box_size = naxes[0]/3 - 1;

  /*remove_cosmics(ID_name,"tmpcen");*/
  copy_image_ID(ID_name, "tmpcen", 0);
  background = img_percentile("tmpcen",0.5);
  
  arith_image_trunc("tmpcen",img_percentile("tmpcen",0.99),img_percentile("tmpcen",1.0),"tmpcen1");
  delete_image_ID("tmpcen");

  center_PSF("tmpcen1",xcenter,ycenter,box_size);  
  delete_image_ID("tmpcen1");

  printf("center : %f %f\n",xcenter[0],ycenter[0]);
  
  if((xcenter[0]<Csize/2+1)||(xcenter[0]>naxes[0]-Csize/2-1)||(ycenter[0]<Csize/2+1)||(ycenter[0]>naxes[1]-Csize/2-1))
    {
      printf("PSF too close to edge of image - cannot measure SR\n");
      SR = -1;
    }
  else
    {
      arith_image_extract2D(ID_name,"tmpsr",Csize,Csize,((long) xcenter[0])-Csize/2,((long) ycenter[0])-Csize/2);
      ID = image_ID("tmpsr");
      peak = 0.0;
      for(ii=Csize/2-5;ii<Csize/2+5;ii++)
	for(jj=Csize/2-5;jj<Csize/2+5;jj++)
	  {
	    tmp1 = data.image[ID].array.F[jj*Csize+ii];
	    if(tmp1>peak)
	      {
		peak = tmp1;
		peakii = ii;
		peakjj = jj;
	      }
	  }
      for(ii=0;ii<Csize;ii++)
	for(jj=0;jj<Csize;jj++)
	  if(data.image[ID].array.F[jj*Csize+ii] > peak*1.001)
	      data.image[ID].array.F[jj*Csize+ii] = 0.0;
      
      fftzoom("tmpsr","tmpsrz",fzoomfactor);
      ID = image_ID("tmpsrz");
      peakii *= fzoomfactor;
      peakjj *= fzoomfactor;
      total1 = 0.0;
      total2 = 0.0;
      cnt1 = 0;
      cnt2 = 0;
      for(ii=0;ii<Csize2;ii++)
	for(jj=0;jj<Csize2;jj++)
	  {
	    dist = sqrt((peakii-ii)*(peakii-ii)+(peakjj-jj)*(peakjj-jj));
	    if(dist<r2*fzoomfactor)
	      {
		if(dist<r1*fzoomfactor)
		  {
		    total1 += data.image[ID].array.F[jj*Csize2+ii];
		    cnt1++;
		  }
		else
		  {
		    total2 += data.image[ID].array.F[jj*Csize2+ii];
		    cnt2++;		    
		  }
	      }
	  }
      background = total2/cnt2;
      total = total1-background*cnt1;
      max  = arith_image_max("tmpsrz");
  
      printf("background = %f\n",background);
      printf("max   = %f  (%f)\n",max,max*fzoomfactor*fzoomfactor);
      printf("total = %f (%f[%ld] %f[%ld])\n",total,total1,cnt1,total2,cnt2);
      
      printf("ratio = %f  \n",max/total*fzoomfactor);
      SR = max/total*fzoomfactor/factor;
      save_fl_fits("tmpsr","!tmpsr");
      save_fl_fits("tmpsrz","!tmpsrz");
      delete_image_ID("tmpsr");
      delete_image_ID("tmpsrz");

      printf("SR = %f\n",SR);
    }
  
  free(xcenter);
  free(ycenter); 

  return(SR);
}



// simple lucky imaging
// input must be co-centered flux normalized cube 
// algorithm will rank frames according to the total flux inside a radius r_pix
long PSF_coaddbest(const char *IDcin_name, const char *IDout_name, float r_pix)
{
  long IDcin, IDout;
  long IDmask;
  long xsize, ysize, ksize;
  long ii,jj,kk,kk1;
  double *flux_array;
  long *imgindex;

  IDcin = image_ID(IDcin_name);
  xsize = data.image[IDcin].md[0].size[0];
  ysize = data.image[IDcin].md[0].size[1];
  ksize = data.image[IDcin].md[0].size[2];

  //  printf("\"%s\" %ld SIZE = %ld %ld\n",IDcin_name, IDcin, xsize,ysize);

  flux_array = (double*) malloc(sizeof(double)*ksize);
  imgindex = (long*) malloc(sizeof(long)*ksize);

  IDmask = make_subpixdisk("tmpMask",xsize,ysize,xsize/2,ysize/2,r_pix);
  
  for(kk=0;kk<ksize;kk++)
    {
      imgindex[kk] = kk;
      flux_array[kk] = 0.0;
      for(ii=0;ii<xsize*ysize;ii++)
	flux_array[kk] -= data.image[IDcin].array.F[kk*xsize*ysize+ii]*data.image[IDmask].array.F[ii];
    }
  
  delete_image_ID("tmpMask");

  quick_sort2l(flux_array, imgindex, ksize);
  
  IDout = create_3Dimage_ID(IDout_name,xsize,ysize,ksize);
  
  for(kk=0;kk<ksize;kk++)
    {
      kk1 = imgindex[kk];
      for(ii=0;ii<xsize*ysize;ii++)
	data.image[IDout].array.F[kk*xsize*ysize+ii] = data.image[IDcin].array.F[kk1*xsize*ysize+ii];      
      if(kk>0)
	for(ii=0;ii<xsize*ysize;ii++)
	  data.image[IDout].array.F[kk*xsize*ysize+ii] += data.image[IDout].array.F[(kk-1)*xsize*ysize+ii];
    }
  
  free(imgindex);
  free(flux_array);

  
  return(IDout);
}


//
// if timing file exists, use it for output
// PSFsizeEst: estimated size of PSF (sigma)
//
int PSF_sequence_measure(const char *IDin_name, float PSFsizeEst, const char *outfname)
{
	long IDin;
	long xsize, ysize, xysize, zsize;
	FILE *fpout;
	long IDtmp;
	double *xcenter;
	double *ycenter;
	long boxsize;
	const char *ptr;
	long kk;
	char fname[200];
	
	
	boxsize = (long) (2.0*PSFsizeEst);
	printf("box size : %f -> %ld\n", PSFsizeEst, boxsize);

	xcenter = (double*) malloc(sizeof(double));
	ycenter = (double*) malloc(sizeof(double));
	
	IDin = image_ID(IDin_name);
	xsize = data.image[IDin].md[0].size[0];
	ysize = data.image[IDin].md[0].size[1];
	xysize = xsize*ysize;
	if(data.image[IDin].md[0].naxis == 3)
		zsize = data.image[IDin].md[0].size[2];
	else
		zsize = 1;
	
	IDtmp = create_2Dimage_ID("_tmppsfim", xsize, ysize);
	
	fpout = fopen(outfname, "w");
	for(kk=0;kk<zsize;kk++)
	{
		ptr = (char*) data.image[IDin].array.F;
		ptr += sizeof(float)*xysize*kk;
		memcpy((void*) data.image[IDtmp].array.F, (void*) ptr, sizeof(float)*xysize);
		fast_center_PSF("_tmppsfim", xcenter, ycenter, boxsize);
		printf("%5ld   CENTER = %f %f\n", kk, xcenter[0], ycenter[0]);
		fprintf(fpout, "%ld %20f %20f\n", kk, xcenter[0], ycenter[0]);
	
	//	sprintf(fname, "!_tmppsfim_%04ld.fits", kk);
	//	save_fits("_tmppsfim", fname);
	}
	fclose(fpout);
	
	
	free(xcenter);
	free(ycenter);
	
	return(0);
}
