#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#ifdef __MACH__
#include <mach/mach_time.h>long AOloopControl_ComputeOpenLoopModes(long loop)
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
static int clock_gettime(int clk_id, struct mach_timespec *t){
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t time;
    time = mach_absolute_time();
    double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
    double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
    t->tv_sec = seconds;
    t->tv_nsec = nseconds;
    return 0;
}
#else
#include <time.h>
#endif


#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "info/info.h"


#define SBUFFERSIZE 1000

extern DATA data;


static char errormessage[SBUFFERSIZE];

static FILE *fpgnuplot;


// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
// 5: string

int_fast8_t COREMOD_TOOLS_mvProcCPUset_cli()
{
    if(CLI_checkarg(1,3)==0)
    {
      COREMOD_TOOLS_mvProcCPUset(data.cmdargtoken[1].val.string);
      return 0;
    }
  else
    return 1;
}


int_fast8_t write_flot_file_cli()
{
  if(CLI_checkarg(1,3)+CLI_checkarg(2,1)==0)
    {
      write_float_file(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf);
      return 0;
    }
  else
    return 1;
}


int_fast8_t COREMOD_TOOLS_imgdisplay3D_cli()
{
 if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
    {
	COREMOD_TOOLS_imgdisplay3D(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
      return 0;
    }
  else
    return 1;
}

int_fast8_t COREMOD_TOOLS_statusStat_cli()
{
    if(CLI_checkarg(1,4)+CLI_checkarg(2,2)==0)
    {
        COREMOD_TOOLS_statusStat(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numl);
        return 0;
    }
    else
        return 1;
}




int init_COREMOD_tools()
{
    strcpy(data.module[data.NBmodule].name, __FILE__);
    strcpy(data.module[data.NBmodule].info, "image information and statistics");
    data.NBmodule++;


    strcpy(data.cmd[data.NBcmd].key,"csetpmove");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_TOOLS_mvProcCPUset_cli;
    strcpy(data.cmd[data.NBcmd].info,"move current process to CPU set");
    strcpy(data.cmd[data.NBcmd].syntax,"<CPU set name>");
    strcpy(data.cmd[data.NBcmd].example,"csetpmove realtime");
    strcpy(data.cmd[data.NBcmd].Ccall,"int COREMOD_TOOLS_mvProcCPUset(const char *csetname)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"writef2file");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = write_flot_file_cli;
    strcpy(data.cmd[data.NBcmd].info,"write float to file");
    strcpy(data.cmd[data.NBcmd].syntax,"<filename> <float variable>");
    strcpy(data.cmd[data.NBcmd].example,"writef2file val.txt a");
    strcpy(data.cmd[data.NBcmd].Ccall,"int write_float_file(const char *fname, float value)");
    data.NBcmd++;

    strcpy(data.cmd[data.NBcmd].key,"dispim3d");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_TOOLS_imgdisplay3D_cli;
    strcpy(data.cmd[data.NBcmd].info,"display 2D image as 3D surface using gnuplot");
    strcpy(data.cmd[data.NBcmd].syntax,"<imname> <step>");
    strcpy(data.cmd[data.NBcmd].example,"dispim3d im1 5");
    strcpy(data.cmd[data.NBcmd].Ccall,"int COREMOD_TOOLS_imgdisplay3D(const char *IDname, long step)");
    data.NBcmd++;

	strcpy(data.cmd[data.NBcmd].key,"ctsmstats");
    strcpy(data.cmd[data.NBcmd].module,__FILE__);
    data.cmd[data.NBcmd].fp = COREMOD_TOOLS_statusStat_cli;
    strcpy(data.cmd[data.NBcmd].info,"monitors shared variable status");
    strcpy(data.cmd[data.NBcmd].syntax,"<imname> <NBstep>");
    strcpy(data.cmd[data.NBcmd].example,"ctsmstats imst 100000");
    strcpy(data.cmd[data.NBcmd].Ccall,"long COREMOD_TOOLS_statusStat(const char *IDstat_name, long indexmax)");
    data.NBcmd++;


    return 0;
}





int COREMOD_TOOLS_mvProcCPUset(const char *csetname)
{
    int pid;
    char command[200];
    int ret;
    
    pid = getpid();
    sprintf(command, "sudo -n cset proc -m -p %d -t %s\n", pid, csetname);
    ret = system(command);
      
    return(0);
}



int create_counter_file(const char *fname, long NBpts)
{
  long i;
  FILE *fp;
  
  if((fp=fopen(fname,"w"))==NULL)
    {
      sprintf(errormessage,"cannot create file \"%s\"",fname);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }

  for(i=0;i<NBpts;i++)
    fprintf(fp,"%ld %f\n",i,(double) (1.0*i/NBpts));
  
  fclose(fp);

  return(0);
}

int bubble_sort(double *array, long count)
{
  register long a,b;
  register double t;

  for(a=1; a<count; a++)
    for(b=count-1; b>=a; b--)
      if(array[b-1]>array[b])
	{
	  t = array[b-1];
	  array[b-1] = array[b];
	  array[b] = t;
	}
  return(0);
}

void qs_float(float *array, long left, long right)
{
  register long i,j;
  float x,y;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;
      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs_float(array,left,j);
  if(i<right) qs_float(array,i,right);
}

void qs_long(long *array, long left, long right)
{
  register long i,j;
  long x,y;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;
      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs_long(array,left,j);
  if(i<right) qs_long(array,i,right);
}

void qs_double(double *array, long left, long right)
{
  register long i,j;
  double x,y;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;
      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs_double(array,left,j);
  if(i<right) qs_double(array,i,right);
}

void qs_ushort(unsigned short *array, long left, long right)
{
  register long i,j;
  unsigned short x,y;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;
      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs_ushort(array,left,j);
  if(i<right) qs_ushort(array,i,right);
}



void qs3(double *array, double *array1, double *array2, long left, long right)
{
  register long i,j;
  double x,y;
  double y1,y2;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;

      y1 = array1[i];
      array1[i] = array1[j];
      array1[j] = y1;

      y2 = array2[i];
      array2[i] = array2[j];
      array2[j] = y2;

      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs3(array,array1,array2,left,j);
  if(i<right) qs3(array,array1,array2,i,right);
}



void qs3_float(float *array, float *array1, float *array2, long left, long right)
{
  register long i,j;
  float x,y;
  float y1,y2;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;

      y1 = array1[i];
      array1[i] = array1[j];
      array1[j] = y1;

      y2 = array2[i];
      array2[i] = array2[j];
      array2[j] = y2;

      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs3_float(array,array1,array2,left,j);
  if(i<right) qs3_float(array,array1,array2,i,right);
}

void qs3_double(double *array, double *array1, double *array2, long left, long right)
{
  register long i,j;
  double x,y;
  double y1,y2;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;

      y1 = array1[i];
      array1[i] = array1[j];
      array1[j] = y1;

      y2 = array2[i];
      array2[i] = array2[j];
      array2[j] = y2;

      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs3_double(array,array1,array2,left,j);
  if(i<right) qs3_double(array,array1,array2,i,right);
}


void qs2l(double *array, long *array1, long left, long right)
{
  register long i,j;
  double x,y;
  long l1;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;

      l1 = array1[i];
      array1[i] = array1[j];
      array1[j] = l1;

      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs2l(array,array1,left,j);
  if(i<right) qs2l(array,array1,i,right);
}

void qs2l_double(double *array, long *array1, long left, long right)
{
  register long i,j;
  double x,y;
  long l1;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;

      l1 = array1[i];
      array1[i] = array1[j];
      array1[j] = l1;

      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs2l_double(array,array1,left,j);
  if(i<right) qs2l_double(array,array1,i,right);
}

void qs3ll_double(double *array, long *array1, long *array2, long left, long right)
{
  register long i,j;
  double x,y;
  long l1,l2;
  
  i = left; j = right;
  x = array[(left+right)/2];
  
  do {
    while(array[i]<x && i<right) i++;
    while(x<array[j] && j>left) j--;
    
    if(i<=j) {
      y = array[i];
      array[i] = array[j];
      array[j] = y;

      l1 = array1[i];
      array1[i] = array1[j];
      array1[j] = l1;

      l2 = array2[i];
      array2[i] = array2[j];
      array2[j] = l2;

      i++; j--;
    }
  } while(i<=j);
  
  if(left<j) qs3ll_double(array,array1,array2,left,j);
  if(i<right) qs3ll_double(array,array1,array2,i,right);
}

void quick_sort_float(float *array, long count)
{
  qs_float(array,0,count-1);
}

void quick_sort_long(long *array, long count)
{
  qs_long(array,0,count-1);
}

void quick_sort_double(double *array, long count)
{
  qs_double(array,0,count-1);
}

void quick_sort_ushort(unsigned short *array, long count)
{
  qs_ushort(array,0,count-1);
}

void quick_sort3(double *array, double *array1, double *array2, long count)
{
  qs3(array,array1,array2,0,count-1);
}

void quick_sort3_float(float *array, float *array1, float *array2, long count)
{
  qs3_float(array,array1,array2,0,count-1);
}

void quick_sort3_double(double *array, double *array1, double *array2, long count)
{
  qs3_double(array,array1,array2,0,count-1);
}

void quick_sort2l(double *array, long *array1, long count)
{
  qs2l(array, array1, 0, count-1);
}

void quick_sort2l_double(double *array, long *array1, long count)
{
  qs2l_double(array,array1,0,count-1);
}

void quick_sort3ll_double(double *array, long *array1, long *array2, long count)
{
  qs3ll_double(array,array1,array2,0,count-1);
}

int lin_regress(double *a, double *b, double *Xi2, double *x, double *y, double *sig, int nb_points)
{
  double S,Sx,Sy,Sxx,Sxy,Syy;
  int i;
  double delta;

  S = 0;
  Sx = 0;
  Sy = 0;
  Sxx = 0;
  Syy = 0;
  Sxy = 0;
  for(i=0;i<nb_points;i++)
    {
      S += 1.0/sig[i]/sig[i];
      Sx += x[i]/sig[i]/sig[i];
      Sy += y[i]/sig[i]/sig[i];
      Sxx += x[i]*x[i]/sig[i]/sig[i];
      Syy += y[i]*y[i]/sig[i]/sig[i];
      Sxy += x[i]*y[i]/sig[i]/sig[i];
    }

  delta = S*Sxx-Sx*Sx;
  *a = (Sxx*Sy-Sx*Sxy)/delta;
  *b = (S*Sxy-Sx*Sy)/delta;
  *Xi2 = Syy-2*(*a)*Sy-2*(*a)*(*b)*Sx+(*a)*(*a)*S+2*(*a)*(*b)*Sx-(*b)*(*b)*Sxx;
  
  return(0);
}

int replace_char(char *content, char cin, char cout)
{
  long i;

  for(i=0;i<strlen(content);i++)
    if(content[i]==cin)
      content[i] = cout;

  return(0);
}


int read_config_parameter_exists(const char *config_file, const char *keyword)
{
  FILE *fp;
  char line[1000];
  char keyw[200];
  char cont[200];
  int read;

  read = 0;
  if((fp=fopen(config_file,"r"))==NULL)
    {
      sprintf(errormessage,"cannot open file \"%s\"",config_file);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }
  
  while((fgets(line,1000,fp)!=NULL)&&(read==0))
    {
      sscanf(line,"%s",keyw);
      if(strcmp(keyw,keyword)==0)
	read = 1;
    }
  if(read==0)
    {
      sprintf(errormessage,"parameter \"%s\" does not exist in file \"%s\"",keyword,config_file);
      printWARNING(__FILE__,__func__,__LINE__,errormessage);
    }

  fclose(fp);
    
  return(read);
}


int read_config_parameter(const char *config_file, const char *keyword, char *content)
{
  FILE *fp;
  char line[1000];
  char keyw[200];
  char cont[200];
  int read;

  read = 0;
  if((fp=fopen(config_file,"r"))==NULL)
    {
      sprintf(errormessage,"cannot open file \"%s\"",config_file);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }
  
  strcpy(content,"---");
  while(fgets(line,1000,fp)!=NULL)
    {
      sscanf(line,"%s %s",keyw,cont);
      if(strcmp(keyw,keyword)==0)
	{
	  strcpy(content,cont);
	  read = 1;
	}
      /*      printf("KEYWORD : \"%s\"   CONTENT : \"%s\"\n",keyw,cont);*/
    }
  if(read==0)
    {
      sprintf(errormessage,"parameter \"%s\" does not exist in file \"%s\"",keyword,config_file);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      sprintf(content,"-");
      //  exit(0);
    }

  fclose(fp);
  
  return(read);
}

float read_config_parameter_float(const char *config_file, const char *keyword)
{
  float value;
  char content[SBUFFERSIZE];
  
  read_config_parameter(config_file,keyword,content);
  //printf("content = \"%s\"\n",content);
  value = atof(content);
  //printf("Value = %g\n",value);

  return(value);
}

long read_config_parameter_long(const char *config_file, const char *keyword)
{
  long value;
  char content[SBUFFERSIZE];
  
  read_config_parameter(config_file,keyword,content);
  value = atol(content);
  
  return(value);
}

int read_config_parameter_int(const char *config_file, const char *keyword)
{
  int value;
  char content[SBUFFERSIZE];
  
  read_config_parameter(config_file,keyword,content);
  value = atoi(content);
  
  return(value);
}




long file_number_lines(const char *file_name)
{
  long cnt;
  int c;
  FILE *fp;

  if((fp=fopen(file_name,"r"))==NULL)
    {
      sprintf(errormessage,"cannot open file \"%s\"",file_name);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }
  
  cnt = 0;
  while((c=fgetc(fp))!=EOF)
    if(c=='\n')
      cnt++;
  fclose(fp);

  return(cnt);
}


FILE* open_file_w(const char *filename)
{
  FILE *fp;

  if((fp=fopen(filename,"w"))==NULL)
    {
      sprintf(errormessage,"cannot create file \"%s\"",filename);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }
  
  return(fp);
}


FILE* open_file_r(const char *filename)
{
  FILE *fp;

  if((fp=fopen(filename,"r"))==NULL)
    {
      sprintf(errormessage,"cannot read file \"%s\"",filename);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }
  
  return(fp);
}

int write_1D_array(double *array, long nbpoints, const char *filename)
{
  FILE *fp;
  long ii;

  fp = open_file_w(filename);
  for(ii=0;ii<nbpoints;ii++)
    fprintf(fp,"%ld\t%f\n",ii,array[ii]);
  fclose(fp);
  
  return(0);
}

int read_1D_array(double *array, long nbpoints, const char *filename)
{
  FILE *fp;
  long ii;
  long tmpl;
  
  fp = open_file_r(filename);
  for(ii=0;ii<nbpoints;ii++)
    {
      if(fscanf(fp,"%ld\t%lf\n",&tmpl,&array[ii])!=2)
	{
	  printERROR(__FILE__,__func__,__LINE__,"fscanf error");
	  exit(0);
	}
    }
  fclose(fp);
  
  return(0);
}



/* test point */ 
int tp(const char *word)
{
  printf("---- Test point %s ----\n",word);
  fflush(stdout);

  return(0);
}

int read_int_file(const char *fname)
{
  int value;
  FILE *fp;
  
  if((fp = fopen(fname,"r"))==NULL)
    {
      value = 0;
    }
  else
    {
      if(fscanf(fp,"%d",&value)!=1)
	{
	  printERROR(__FILE__,__func__,__LINE__,"fscanf error");
	  exit(0);
	}
      fclose(fp);
    }

  return(value);
}

int write_int_file(const char *fname, int value)
{
  FILE *fp;
  
  if((fp = fopen(fname,"w"))==NULL)
    {
      sprintf(errormessage,"cannot create file \"%s\"\n",fname);
      printERROR(__FILE__,__func__,__LINE__,errormessage);
      exit(0);
    }
  
  fprintf(fp,"%d\n",value);
  fclose(fp);

  return(value);
}

int write_float_file(const char *fname, float value)
{
  FILE *fp;
  int mode = 0; // default, create single file
  
  if(variable_ID("WRITE2FILE_APPEND")!=-1)
    mode = 1;

  if(mode == 0)
    {
      if((fp = fopen(fname,"w"))==NULL)
	{
	  sprintf(errormessage,"cannot create file \"%s\"\n",fname);
	  printERROR(__FILE__,__func__,__LINE__,errormessage);
	  exit(0);
	}
      fprintf(fp,"%g\n",value);
      fclose(fp);
    }
  
  if(mode == 1)
   {
      if((fp = fopen(fname,"a"))==NULL)
	{
	  sprintf(errormessage,"cannot create file \"%s\"\n",fname);
	  printERROR(__FILE__,__func__,__LINE__,errormessage);
	  exit(0);
	}
      fprintf(fp," %g",value);
      fclose(fp);
   }

  return(0);
}


// displays 2D image in 3D using gnuplot
//
int COREMOD_TOOLS_imgdisplay3D(const char *IDname, long step)
{
    char ID;
    long xsize, ysize;
    long ii, jj;
    char cmd[512];
    FILE *fp;

    ID = image_ID(IDname);
    xsize = data.image[ID].md[0].size[0];
    ysize = data.image[ID].md[0].size[1];

    snprintf(cmd, 512, "gnuplot");

    if ((fpgnuplot = popen(cmd,"w")) == NULL)
    {
        fprintf(stderr, "could not connect to gnuplot\n");
        return -1;
    }

    printf("image: %s [%ld x %ld], step = %ld\n", IDname, xsize, ysize, step);

    fprintf(fpgnuplot, "set pm3d\n");
    fprintf(fpgnuplot, "set hidden3d\n");
    fprintf(fpgnuplot, "set palette\n");
    //fprintf(gnuplot, "set xrange [0:%li]\n", image.md[0].size[0]);
    //fprintf(gnuplot, "set yrange [0:1e-5]\n");
    //fprintf(gnuplot, "set xlabel \"Mode #\"\n");
    //fprintf(gnuplot, "set ylabel \"Mode RMS\"\n");
    fflush(fpgnuplot);

    fp = fopen("pts.dat", "w");
    fprintf(fpgnuplot, "splot \"-\" w d notitle\n");
    for(ii=0; ii<xsize; ii+=step)
    {
        for(jj=0; jj<xsize; jj+=step)
        {
            fprintf(fpgnuplot, "%ld %ld %f\n", ii, jj, data.image[ID].array.F[jj*xsize+ii]);
            fprintf(fp, "%ld %ld %f\n", ii, jj, data.image[ID].array.F[jj*xsize+ii]);
        }
        fprintf(fpgnuplot, "\n");
        fprintf(fp, "\n");
    }
    fprintf(fpgnuplot, "e\n");
    fflush(fpgnuplot);
    fclose(fp);


    return(0);
}




//
// watch shared memory status image and perform timing statistics
//
long COREMOD_TOOLS_statusStat(const char *IDstat_name, long indexmax)
{
    long IDout;
    int RT_priority = 91; //any number from 0-99
    struct sched_param schedpar;
    float usec0 = 50.0;
    float usec1 = 150.0;
    long long k;
    long long NBkiter = 2000000000;
    long IDstat;

    unsigned short st;

    struct timespec t1;
    struct timespec t2;
    struct timespec tdiff;
    double tdiffv;
    double tdisplay = 1.0; // interval
    double tdiffv1 = 0.0;
	long *sizearray;

	long cnttot;
	
	

    IDstat = image_ID(IDstat_name);

		sizearray = (long*) malloc(sizeof(long)*2);
    sizearray[0] = indexmax;
    sizearray[1] = 1;
    IDout = create_image_ID("statout", 2, sizearray, LONG, 0, 0);
	free(sizearray);

    for(st=0; st<indexmax; st++)
        data.image[IDout].array.L[st] = 0;

    schedpar.sched_priority = RT_priority;
    #ifndef __MACH__
    sched_setscheduler(0, SCHED_FIFO, &schedpar);
	#endif


    printf("Measuring status distribution \n");
    fflush(stdout);

    clock_gettime(CLOCK_REALTIME, &t1);
    for(k=0; k<NBkiter; k++)
    {
        usleep((long) (usec0+usec1*(1.0*k/NBkiter)));
        st = data.image[IDstat].array.U[0];
        if(st<indexmax)
            data.image[IDout].array.L[st]++;


        clock_gettime(CLOCK_REALTIME, &t2);
        tdiff = info_time_diff(t1, t2);
        tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;

        if(tdiffv>tdiffv1)
        {
            tdiffv1 += tdisplay;
            printf("\n");
            printf("============== %10lld  %d  ==================\n", k , st);
            printf("\n");
			cnttot = 0;
			for(st=0; st<indexmax; st++)
				cnttot += data.image[IDout].array.L[st];
            
            for(st=0; st<indexmax; st++)
                printf("STATUS  %5d    %20ld   %6.3f  \n", st, data.image[IDout].array.L[st], 100.0*data.image[IDout].array.L[st]/cnttot);
        }
    }


    printf("\n");
    for(st=0; st<indexmax; st++)
        printf("STATUS  %5d    %10ld\n", st, data.image[IDout].array.L[st]);

    printf("\n");



    return(IDout);
}


