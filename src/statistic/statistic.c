#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gsl/gsl_randist.h>
#include "CLIcore.h"


#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "statistic/statistic.h"





extern DATA data;



//int put_poisson_noise(char *ID_in_name, char *ID_out_name);



// CLI commands

int statistic_putphnoise_cli()
{
 
  if(CLI_checkarg(1, 4)+CLI_checkarg(2, 3)==0)
    {
      put_poisson_noise( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string);
      return 0;
    }
  else
    return 1;
}


int statistic_putgaussnoise_cli()
{
 
  if(CLI_checkarg(1, 4)+CLI_checkarg(2, 3)+CLI_checkarg(3, 1)==0)
    {
      put_gauss_noise( data.cmdargtoken[1].val.string,  data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf);
      return 0;
    }
  else
    return 1;
}



int init_statistic()
{
  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "statistics functions and tools");
  data.NBmodule++;

  strcpy(data.cmd[data.NBcmd].key,"putphnoise");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = statistic_putphnoise_cli;
  strcpy(data.cmd[data.NBcmd].info,"add photon noise to image");
  strcpy(data.cmd[data.NBcmd].syntax,"input output");
  strcpy(data.cmd[data.NBcmd].example,"putphnoise im0 im1");
  strcpy(data.cmd[data.NBcmd].Ccall,"int put_poisson_noise(char *ID_in_name, char *ID_out_name)");
  data.NBcmd++;
 
  strcpy(data.cmd[data.NBcmd].key,"putgaussnoise");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = statistic_putgaussnoise_cli;
  strcpy(data.cmd[data.NBcmd].info,"add gaussian noise to image");
  strcpy(data.cmd[data.NBcmd].syntax,"input output amplitude");
  strcpy(data.cmd[data.NBcmd].example,"putgaussnoise im0 im1 0.2");
  strcpy(data.cmd[data.NBcmd].Ccall,"long put_gauss_noise(char *ID_in_name, char *ID_out_name, doule ampl)");
  data.NBcmd++; 
 
 // add atexit functions here

  return 0;

}







double ran1()
{
  double value;

  value = data.INVRANDMAX*rand();
 // gsl_rng_uniform (data.rndgen);// data.INVRANDMAX*rand();

  return(value);
}

double gauss()
{
  // use first option if using ranlxs generator 
  // return(gsl_ran_ugaussian (data.rndgen));
  
  // for speed (4.1x faster than default), but not that random (some fringes appear in image)
  // return(gsl_ran_gaussian_ziggurat (data.rndgen,1.0));

  // default
  return(gsl_ran_gaussian (data.rndgen,1.0));
}

double gauss_trc()
{
  double value;

  value = gauss();
  while(fabs(value)>1.0)
    {
      value = gauss();
    }
  return(value);
}

long poisson(double mu){

  return(gsl_ran_poisson (data.rndgen,(double) mu));
}

double cfits_gammaln(double xx){
  /* ln of the Gamma function */
  int j;
  double cof[6];
  double stp;
  double ser;
  double tmp,x,y;
  double result;

  cof[0] = 76.18009172947146;
  cof[1] = -86.50532032941677;
  cof[2] = 24.01409824083091;
  cof[3] = -1.231739572450155;
  cof[4] = 0.001208650973866179;
  cof[5] = 0.000005395239384953;
  stp = 2.5066282746310005;
  ser = 1.000000000190015;
  
  x = xx;
  y = x;
  tmp = x + 5.5;
  tmp = (x + 0.5)*log(tmp) - tmp;
  for(j=0;j<6;j++)
    {
      y = y+1;
      ser = ser+cof[j]/y;
    }
  result = tmp+log(stp*ser/x);
  return(result);
}

double fast_poisson(double mu){
  /* a fast, but approximate, poisson distribution generator */
  double em;

  em = 0;
  em = (double) ((long long) (mu+gauss()*sqrt(mu)));
  if(em<0.0)
    em=0.0;

  return(em);  
}

// better_poisson seems to give a very weird value every once in a while
// probability this happens is ~1e-8 to 1e-9
double better_poisson(double mu) {
    /* a better poisson distribution generator... see num. rec. section 7.3. */
    double logmu;
    double inv_randmax;
    double sq,em,g,y,t;

    inv_randmax = 1.0/RAND_MAX;

    em = 0;
    if(mu<100)
        em = (double) poisson(mu);
    else
    {
        sq = sqrt(2*mu);
        logmu = log(mu);
        g = mu*logmu-cfits_gammaln(mu+1);


        y = tan(PI*(inv_randmax*rand()));
        em = sq*y+mu;
        while(em<0)
        {
            y = tan(PI*(inv_randmax*rand()));
            em = sq*y+mu;
        }
        em = (int) em;
        t = 0.9*(1+y*y)*exp(em*logmu-cfits_gammaln(em+1)-g);

        while ( (inv_randmax*rand()) > t)
        {
            y = tan(PI*(inv_randmax*rand()));
            em = sq*y+mu;
            while(em<0)
            {
                y = tan(PI*(inv_randmax*rand()));
                em = sq*y+mu;
            }
            em = (long) em;
            t = 0.9*(1+y*y)*exp(em*logmu-cfits_gammaln(em+1)-g);
        }
    }

    return(1.0*em);
}


long put_poisson_noise(char *ID_in_name, char *ID_out_name)
{
  long ID_in;
  long ID_out;
  long ii;
  long nelements;
  long naxis;
  long i;

  ID_in = image_ID(ID_in_name);
  naxis = data.image[ID_in].md[0].naxis;
  nelements=1;
  for(i=0;i<naxis;i++)
    nelements*=data.image[ID_in].md[0].size[i];

  copy_image_ID(ID_in_name, ID_out_name, 0);

  ID_out = image_ID(ID_out_name);
  //  srand(time(NULL));
  
  for (ii=0; ii < nelements; ii++)
    data.image[ID_out].array.F[ii] = poisson(data.image[ID_in].array.F[ii]);

  return(ID_out);
}



long put_gauss_noise(char *ID_in_name, char *ID_out_name, double ampl)
{
  long ID_in;
  long ID_out;
  long ii;
  long nelements;
  long naxis;
  long i;

  ID_in = image_ID(ID_in_name);
  naxis = data.image[ID_in].md[0].naxis;
  nelements=1;
  for(i=0;i<naxis;i++)
    nelements*=data.image[ID_in].md[0].size[i];

  copy_image_ID(ID_in_name, ID_out_name, 0);

  ID_out = image_ID(ID_out_name);
  //  srand(time(NULL));
  
  for (ii=0; ii < nelements; ii++)
    data.image[ID_out].array.F[ii] = data.image[ID_in].array.F[ii] + ampl*gauss();

  return(ID_out);
}

