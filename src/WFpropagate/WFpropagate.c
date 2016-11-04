#include <fitsio.h> 
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "CLIcore.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "COREMOD_iofits/COREMOD_iofits.h"

#include "fft/fft.h"
#include "image_gen/image_gen.h"

#include "WFpropagate/WFpropagate.h"


extern DATA data;

#define SBUFFERSIZE 2000




int Fresnel_propagate_wavefront_cli()
{

  if(CLI_checkarg(1, 4)+CLI_checkarg(2, 3)+CLI_checkarg(3, 1)+CLI_checkarg(4, 1)+CLI_checkarg(5, 1)==0)
    {
      Fresnel_propagate_wavefront(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf);
      return 0;
    }
  else
    return 1;
}



int init_WFpropagate()
{
  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "light propagation");
  data.NBmodule++;
  
  strcpy(data.cmd[data.NBcmd].key,"fresnelpw");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = Fresnel_propagate_wavefront_cli;
  strcpy(data.cmd[data.NBcmd].info,"Fresnel propagate wavefront");
  strcpy(data.cmd[data.NBcmd].syntax,"<input image> <output image> <pupil scale m/s> <prop dist> <lambda>");
  strcpy(data.cmd[data.NBcmd].example,"fresnelpw in out 0.01 1000 0.0000005");
  strcpy(data.cmd[data.NBcmd].Ccall,"int Fresnel_propagate_wavefront(char *in, char *out, double PUPIL_SCALE, double z, double lambda)");
  data.NBcmd++;
 
 // add atexit functions here

  return 0;

}







int Fresnel_propagate_wavefront(char *in, char *out, double PUPIL_SCALE, double z, double lambda)
{
    /* all units are in m */
    double coeff;
    long ii,jj,ii1,jj1;
    long naxes[2];
    long ID;
    double sqdist;
    double re, im;
    double angle;
    double co1;
    long ii2,jj2;
    long n0h,n1h;
    int atype;


    do2dfft(in, "tmp");
    permut("tmp");
    ID = image_ID("tmp");
    atype = data.image[ID].md[0].atype;





    naxes[0] = data.image[ID].md[0].size[0];
    naxes[1] = data.image[ID].md[0].size[1];
    coeff = PI*z*lambda/(PUPIL_SCALE*naxes[0])/(PUPIL_SCALE*naxes[0]);

    co1 = 1.0*naxes[0]*naxes[1];
    n0h = naxes[0]/2;
    n1h = naxes[1]/2;


//	printf("coeff = %g     co1 = %g\n", coeff, co1);

    if(atype == COMPLEX_FLOAT)
    {
        for(jj=0; jj<naxes[1]; jj++)
        {
            jj1 = naxes[0]*jj;
            jj2 = (jj-naxes[1]/2)*(jj-naxes[1]/2);
            for(ii=0; ii<naxes[0]; ii++)
            {
                ii1 = jj1+ii;
                ii2 = ii-n0h;
                sqdist = ii2*ii2+jj2;
                angle = -coeff*sqdist;
                re = data.image[ID].array.CF[ii1].re/co1;
                im = data.image[ID].array.CF[ii1].im/co1;
                data.image[ID].array.CF[ii1].re = re*cos(angle) - im*sin(angle);
                data.image[ID].array.CF[ii1].im = re*sin(angle) + im*cos(angle);
            }
        }
    }
    else
    {
        for(jj=0; jj<naxes[1]; jj++)
        {
            jj1 = naxes[0]*jj;
            jj2 = (jj-naxes[1]/2)*(jj-naxes[1]/2);
            for(ii=0; ii<naxes[0]; ii++)
            {
                ii1 = jj1+ii;
                ii2 = ii-n0h;
                sqdist = ii2*ii2+jj2;
                angle = -coeff*sqdist;
                re = data.image[ID].array.CD[ii1].re/co1;
                im = data.image[ID].array.CD[ii1].im/co1;
                data.image[ID].array.CD[ii1].re = re*cos(angle) - im*sin(angle);
                data.image[ID].array.CD[ii1].im = re*sin(angle) + im*cos(angle);
            }
        }
    }
    
    permut("tmp");
	
    
    do2dffti("tmp", out);
    
   
    delete_image_ID("tmp");

    return(0);
}












/* takes better care of aliasing problems */
int Init_Fresnel_propagate_wavefront(char *Cim, long size, double PUPIL_SCALE, double z, double lambda, double FPMASKRAD, int Precision)
{
    /* all units are in m */
    double coeff;
    long ii,jj;
    double sqdist;
    double angle;
    double co1;
    long ID;
    long BIN = 4;
    long i,j;
    double x,y;
    double Re, Im, Amp, Pha, Re1, Im1;

    if(Precision == 0 ) // single precision
        ID = create_2DCimage_ID(Cim, size, size);
    else
        ID = create_2DCimage_ID_double(Cim, size, size);

    coeff=PI*z*lambda/(PUPIL_SCALE*size)/(PUPIL_SCALE*size);
    co1=1.0*size*size;

    if(Precision == 0)
    {
        for(ii=0; ii<size; ii++)
            for(jj=0; jj<size; jj++)
            {
                Re = 0.0;
                Im = 0.0;
                for(i=0; i<BIN; i++)
                    for(j=0; j<BIN; j++)
                    {
                        x = 1.0*ii+1.0*(0.5+i-BIN/2)/BIN-size/2;
                        y = 1.0*jj+1.0*(0.5+j-BIN/2)/BIN-size/2;
                        sqdist = x*x+y*y;
                        angle = -coeff*sqdist;
                        if(sqrt(sqdist)<FPMASKRAD)
                        {
                            Re += cos(angle);
                            Im += sin(angle);
                        }
                    }
                Amp = sqrt(Re*Re+Im*Im)/BIN/BIN;
                Pha = atan2(Im,Re);
                if(Amp>0.3)
                    Amp = 1.0;
                else
                    Amp = 0.0;

                Re1 = Amp*cos(Pha)/co1;
                Im1 = Amp*sin(Pha)/co1;

                data.image[ID].array.CF[jj*size+ii].re = Re1;
                data.image[ID].array.CF[jj*size+ii].im = Im1;
            }
    }
    else
    {
        for(ii=0; ii<size; ii++)
            for(jj=0; jj<size; jj++)
            {
                Re = 0.0;
                Im = 0.0;
                for(i=0; i<BIN; i++)
                    for(j=0; j<BIN; j++)
                    {
                        x = 1.0*ii+1.0*(0.5+i-BIN/2)/BIN-size/2;
                        y = 1.0*jj+1.0*(0.5+j-BIN/2)/BIN-size/2;
                        sqdist = x*x+y*y;
                        angle = -coeff*sqdist;
                        if(sqrt(sqdist)<FPMASKRAD)
                        {
                            Re += cos(angle);
                            Im += sin(angle);
                        }
                    }
                Amp = sqrt(Re*Re+Im*Im)/BIN/BIN;
                Pha = atan2(Im,Re);
                if(Amp>0.3)
                    Amp = 1.0;
                else
                    Amp = 0.0;

                Re1 = Amp*cos(Pha)/co1;
                Im1 = Amp*sin(Pha)/co1;

                data.image[ID].array.CD[jj*size+ii].re = Re1;
                data.image[ID].array.CD[jj*size+ii].im = Im1;
            }
    }


    permut(Cim);

    return(0);
}









int Fresnel_propagate_wavefront1(char *in, char *out, char *Cin)
{
    /* all units are in m */
    long ii;
    long naxes[2];
    long ID;
    long IDref;
    double re,im,reref,imref;
    long nbelem;
    char fname[SBUFFERSIZE];
    long sizein;
    int atype;

    ID = image_ID(in);
    sizein = data.image[ID].md[0].size[0];
    sprintf(fname,"tmpfpw%ld", sizein);
    atype = data.image[ID].md[0].atype;

    do2dfft(in,fname);

    ID = image_ID(fname);
    naxes[0]=data.image[ID].md[0].size[0];
    naxes[1]=data.image[ID].md[0].size[1];
    nbelem = naxes[0]*naxes[1];

    IDref = image_ID(Cin);

    if(atype == COMPLEX_FLOAT)
    {
        for(ii=0; ii<nbelem; ii++)
        {
            re = data.image[ID].array.CF[ii].re;
            im = data.image[ID].array.CF[ii].im;

            reref = data.image[IDref].array.CF[ii].re;
            imref = data.image[IDref].array.CF[ii].im;

            data.image[ID].array.CF[ii].re = re*reref-im*imref;
            data.image[ID].array.CF[ii].im = re*imref+im*reref;
        }
    }
    else
    {
        for(ii=0; ii<nbelem; ii++)
        {
            re = data.image[ID].array.CD[ii].re;
            im = data.image[ID].array.CD[ii].im;

            reref = data.image[IDref].array.CD[ii].re;
            imref = data.image[IDref].array.CD[ii].im;

            data.image[ID].array.CD[ii].re = re*reref-im*imref;
            data.image[ID].array.CD[ii].im = re*imref+im*reref;
        }
    }
    do2dffti(fname,out);
    delete_image_ID(fname);

    return(0);
}






long Fresnel_propagate_cube(char *IDcin_name, char *IDout_name_amp, char *IDout_name_pha, double PUPIL_SCALE, double zstart, double zend, long NBzpts, double lambda)
{
    long IDouta, IDoutp;
    long IDcin;
    long xsize, ysize;
    long ii,jj,kk;
    double zprop;
    long IDtmp;
    double re,im,amp,pha;
    int atype;

    IDcin = image_ID(IDcin_name);
    xsize = data.image[IDcin].md[0].size[0];
    ysize = data.image[IDcin].md[0].size[1];
    atype = data.image[IDcin].md[0].atype;

    if(atype == COMPLEX_FLOAT)
    {
        IDouta = create_3Dimage_ID(IDout_name_amp,xsize,ysize,NBzpts);
        IDoutp = create_3Dimage_ID(IDout_name_pha,xsize,ysize,NBzpts);
    }
    else
    {
        IDouta = create_3Dimage_ID_double(IDout_name_amp,xsize,ysize,NBzpts);
        IDoutp = create_3Dimage_ID_double(IDout_name_pha,xsize,ysize,NBzpts);
    }

    for(kk=0; kk<NBzpts; kk++)
    {
        zprop = zstart + (zend-zstart)*kk/NBzpts;
        printf("[%ld] propagating by %f m\n",kk,zprop);
        Fresnel_propagate_wavefront(IDcin_name, "_propim", PUPIL_SCALE, zprop, lambda);
        IDtmp = image_ID("_propim");
        if(atype == COMPLEX_FLOAT)
        {
            for(ii=0; ii<xsize; ii++)
                for(jj=0; jj<ysize; jj++)
                {
                    re = data.image[IDtmp].array.CF[jj*xsize+ii].re;
                    im = data.image[IDtmp].array.CF[jj*xsize+ii].im;
                    amp = sqrt(re*re+im*im);
                    pha = atan2(im,re);
                    data.image[IDouta].array.F[kk*xsize*ysize+jj*xsize+ii] = amp;
                    data.image[IDoutp].array.F[kk*xsize*ysize+jj*xsize+ii] = pha;
                }
        }
        else
        {
            for(ii=0; ii<xsize; ii++)
                for(jj=0; jj<ysize; jj++)
                {
                    re = data.image[IDtmp].array.CD[jj*xsize+ii].re;
                    im = data.image[IDtmp].array.CD[jj*xsize+ii].im;
                    amp = sqrt(re*re+im*im);
                    pha = atan2(im,re);
                    data.image[IDouta].array.D[kk*xsize*ysize+jj*xsize+ii] = amp;
                    data.image[IDoutp].array.D[kk*xsize*ysize+jj*xsize+ii] = pha;
                }
        }

        delete_image_ID("_propim");
    }

    return(0);
}





double WFpropagate_TestLyot(long NBmask, double *maskpos)
{
    long k;
    double lambda = 0.55e-6;
    double pixscale = 5.0693766e-5;
    double z = 0;

    double rin = 3.0; // in l/D
    double rout = 10.0; // in l/D
    double value = 0.0;
    double valuecnt = 0.0;

    long ID, IDm, IDa, IDp;
    long size;
    long ii, jj;
    double x, y, r;
    char fname[200];

    printf("Testing Lyot Masks\n");
    // input image is imc (complex amplitude)
    // masks are mask0, mask1 etc...

    copy_image_ID("imc", "imc0", 0);
    for(k=0; k<NBmask; k++)
    {
        sprintf(fname, "mask%ld", k);
        IDm = image_ID(fname);
        Fresnel_propagate_wavefront("imc0", "imc1", pixscale, maskpos[k]-z, lambda);
        z = maskpos[k];
        delete_image_ID("imc0");
        mk_amph_from_complex("imc1", "ima1", "imp1", 0);
        delete_image_ID("imc1");
        ID = image_ID("ima1");
        size = data.image[ID].md[0].size[0];
        sprintf(fname, "!ima_%ld_0.fits", k);
        save_fl_fits("ima1",fname);
        sprintf(fname, "!imp_%ld_0.fits", k);
        save_fl_fits("imp1",fname);
        for(ii=0; ii<size*size; ii++)
            data.image[ID].array.F[ii] *= data.image[IDm].array.F[ii];
        sprintf(fname, "!ima_%ld_1.fits", k);
        save_fl_fits("ima1",fname);
        sprintf(fname, "!imp_%ld_1.fits", k);
        save_fl_fits("imp1",fname);
        mk_complex_from_amph("ima1", "imp1", "imc0", 0);
        delete_image_ID("ima1");
        delete_image_ID("imp1");
    }

    mk_amph_from_complex("imc0", "pup0a", "pup0p", 0);
    delete_image_ID("pup0p");
    save_fl_fits("pup0a", "!pup0a.fits");
    delete_image_ID("pup0a");

    permut("imc0");
    do2dfft("imc0","imc1");
    delete_image_ID("imc0");
    permut("imc1");
    mk_amph_from_complex("imc1", "foca", "focp", 0);
    delete_image_ID("imc1");
    execute_arith("foci=foca*foca/98130");

    IDa = image_ID("foca");
    IDp = image_ID("focp");
    size = data.image[IDa].md[0].size[0];
    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            x = (1.0*ii-0.5*size)/5.12;
            y = (1.0*jj-0.5*size)/5.12;
            r = sqrt(x*x+y*y);
            if((r>5*rout)||(r<rin))
                data.image[IDa].array.F[jj*size+ii] = 0.0;
        }
    mk_complex_from_amph("foca", "focp", "focc", 0);

    delete_image_ID("focp");
    delete_image_ID("foca");
    permut("focc");
    do2dfft("focc","pupc1");
    delete_image_ID("focc");
    permut("pupc1");
    mk_amph_from_complex("pupc1", "pupa1", "pupp1", 0);
    delete_image_ID("pupc1");
    save_fl_fits("pupa1", "!pupa_res.fits");
    delete_image_ID("pupa1");
    delete_image_ID("pupp1");


    ID = image_ID("foci");
    // scale : 5.12 pix = 1.0 l/D
    size = data.image[ID].md[0].size[0];
    for(ii=0; ii<size; ii++)
        for(jj=0; jj<size; jj++)
        {
            x = (1.0*ii-0.5*size)/5.12;
            y = (1.0*jj-0.5*size)/5.12;
            r = sqrt(x*x+y*y);
            if((r>rin)&&(r<rout))
            {
                value += data.image[ID].array.F[jj*size+ii];
                valuecnt += 1.0;
            }
        }

    return(value/valuecnt);
}




long WFpropagate_run() // custom function
{
  FILE *fp;
  long NBmask = 4;
  double *maskpos;
  double value;
  double tot0, tot1;
  double x;

  maskpos = (double*) malloc(sizeof(double)*NBmask);


  load_fits("pa1a_post2.fits", "pa1a", 1);
  execute_arith("refpup=pa1a*pa1a");
  tot0 = arith_image_total("refpup");

  make_disk("mask0", 2048, 2048, 1024, 1024, 190.0);
  save_fl_fits("mask0", "!mask0.fits");
  maskpos[0] = -1.35;

  load_fits("mask_i100_o5.fits", "mask1", 1);
  maskpos[1] = 0.0;

  load_fits("mask_i40_o5.fits", "mask2", 1);
  maskpos[2] = 0.08;

  load_fits("mask_o5_r48.fits", "mask3", 1);
  maskpos[3] = 0.14;

  execute_arith("refpup1=refpup*mask0*mask1*mask2*mask3");
  tot1 = arith_image_total("refpup1");

  fp = fopen("result.txt","w");
  fclose(fp);

	x = 0.0;
  // for(x = 0.08; x< 0.3; x+= 0.01)
  // {
  //maskpos[3] = x;

  value = WFpropagate_TestLyot(NBmask, maskpos);
  save_fl_fits("foci", "!foci.fits");
  delete_image_ID("foci");
  
  printf("AVERAGE CONTRAST = %g\n", value);
  printf("MASK THROUGHPUT = %g\n", tot1/tot0);
  fp = fopen("result.txt","a");
  fprintf(fp,"%f %g %g\n", x, value, tot1/tot0);
  fclose(fp);      
  
  // }


  free(maskpos);

  return(0);
}
