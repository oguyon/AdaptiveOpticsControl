#include <fitsio.h>  /* required by every program that uses CFITSIO  */
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h> //for the mkdir options
#include <sys/mman.h>

#include <fcntl.h> 
#include <termios.h>
#include <sys/types.h> //pid
#include <sys/wait.h> //pid

#include <ncurses.h>


#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "fft/fft.h"

#include "info/info.h"

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

extern DATA data;

int wcol, wrow; // window size

long long cntlast;
struct timespec tlast;


int info_image_monitor(char *ID_name, double frequ);
int info_image_stats(char *ID_name, char *options);



// CLI commands
//
// function CLI_checkarg used to check arguments
// 1: float
// 2: long
// 3: string
// 4: existing image
//

int info_profile_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,3)+CLI_checkarg(3,1)+CLI_checkarg(4,1)+CLI_checkarg(5,1)+CLI_checkarg(6,2)==0)
    {
      profile(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.string, data.cmdargtoken[3].val.numf, data.cmdargtoken[4].val.numf, data.cmdargtoken[5].val.numf, data.cmdargtoken[6].val.numl);
      return 0;
    }
  else
    return 1;
}


int info_image_monitor_cli()
{
  if(CLI_checkarg(1,4)+CLI_checkarg(2,1)==0)
    {
      info_image_monitor(data.cmdargtoken[1].val.string, data.cmdargtoken[2].val.numf);
      return 0;
    }
  else
    return 1;
}

int info_image_stats_cli()
{
  if(CLI_checkarg(1,4)==0)
    {
      info_image_stats(data.cmdargtoken[1].val.string, "");
      return 0;
    }
  else
    return 1;
}


int info_image_statsf_cli()
{
  if(CLI_checkarg(1,4)==0)
    {
      info_image_stats(data.cmdargtoken[1].val.string, "fileout");
      return 0;
    }
  else
    return 1;
}



int init_info()
{
  strcpy(data.module[data.NBmodule].name, __FILE__);
  strcpy(data.module[data.NBmodule].info, "image information and statistics");
  data.NBmodule++;

  
  strcpy(data.cmd[data.NBcmd].key,"imgmon");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = info_image_monitor_cli;
  strcpy(data.cmd[data.NBcmd].info,"image monitor");
  strcpy(data.cmd[data.NBcmd].syntax,"<image> <frequ>");
  strcpy(data.cmd[data.NBcmd].example,"imgmon im1 30");
  strcpy(data.cmd[data.NBcmd].Ccall,"int info_image_monitor(char *ID_name, double frequ)");
  data.NBcmd++;


  strcpy(data.cmd[data.NBcmd].key,"profile");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = info_profile_cli;
  strcpy(data.cmd[data.NBcmd].info,"radial profile");
  strcpy(data.cmd[data.NBcmd].syntax,"<image> <output file> <xcenter> <ycenter> <step> <Nbstep>");
  strcpy(data.cmd[data.NBcmd].example,"profile psf psf.prof 256 256 1.0 100");
  strcpy(data.cmd[data.NBcmd].Ccall,"int profile(char *ID_name, char *outfile, double xcenter, double ycenter, double step, long nb_step)");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"imstats");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = info_image_stats_cli;
  strcpy(data.cmd[data.NBcmd].info,"image stats");
  strcpy(data.cmd[data.NBcmd].syntax,"<image>");
  strcpy(data.cmd[data.NBcmd].example,"imgstats im1");
  strcpy(data.cmd[data.NBcmd].Ccall,"int info_image_stats(char *ID_name, \"\")");
  data.NBcmd++;

  strcpy(data.cmd[data.NBcmd].key,"imstatsf");
  strcpy(data.cmd[data.NBcmd].module,__FILE__);
  data.cmd[data.NBcmd].fp = info_image_statsf_cli;
  strcpy(data.cmd[data.NBcmd].info,"image stats with file output");
  strcpy(data.cmd[data.NBcmd].syntax,"<image>");
  strcpy(data.cmd[data.NBcmd].example,"imgstatsf im1");
  strcpy(data.cmd[data.NBcmd].Ccall,"int info_image_stats(char *ID_name, \"fileout\")");
  data.NBcmd++;


  return 0;
}



struct timespec info_time_diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}



int kbdhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
     
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();
 
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if(ch != EOF)
      {
	//     ungetc(ch, stdin);
        return 1;
      }
    
    return 0;
}


int print_header(char *str, char c)
{
    long n;
    long i;

    attron(A_BOLD);
    n = strlen(str);
    for(i=0; i<(wcol-n)/2; i++)
        printw("%c",c);
    printw("%s", str);
    for(i=0; i<(wcol-n)/2-1; i++)
        printw("%c",c);
    printw("\n");
    attroff(A_BOLD);

    return(0);
}


int printstatus(long ID)
{
    struct timespec tnow;
    struct timespec tdiff;
    double tdiffv;
    char str[500];

    long j;
    double frequ;
    long NBhistopt = 20;
    long *vcnt;
    long h;
    long cnt, i, ii;

    int customcolor;


    float minPV = 60000;
    float maxPV = 0;
    double average;
    double imtotal;

    int atype;
    char line1[200];

    double tmp;
    double RMS = 0.0;

    double RMS01 = 0.0;
    long vcntmax;
    int semval;

    printw("%s\n", data.image[ID].md[0].name);

    atype = data.image[ID].md[0].atype;
    if(atype==CHAR)
        printw("type:  CHAR               ");
    if(atype==INT)
        printw("type:  INT               ");
    if(atype==FLOAT)
        printw("type:  FLOAT              ");
    if(atype==DOUBLE)
        printw("type:  DOUBLE             ");
    if(atype==COMPLEX_FLOAT)
        printw("type:  COMPLEX_FLOAT      ");
    if(atype==COMPLEX_DOUBLE)
        printw("type:  COMPLEX_DOUBLE     ");
    if(atype==USHORT)
        printw("type:  USHORT             ");

    sprintf(str, "[ %6ld",data.image[ID].md[0].size[0]);

    for(j=1; j<data.image[ID].md[0].naxis; j++)
    {
        sprintf(str, "%s x %6ld", str, data.image[ID].md[0].size[j]);
    }
    sprintf(str, "%s]", str);

    printw("%-28s\n", str);


    /* printw("write  = %d\n", data.image[ID].md[0].write);
    printw("status = %d\n", data.image[ID].md[0].status);
    printw("cnt0   = %d\n", data.image[ID].md[0].cnt0);
    printw("cnt1   = %d\n", data.image[ID].md[0].cnt1);
    */



    clock_gettime(CLOCK_REALTIME, &tnow);
    tdiff = info_time_diff(tlast, tnow);
    clock_gettime(CLOCK_REALTIME, &tlast);

    tdiffv = 1.0*tdiff.tv_sec + 1.0e-9*tdiff.tv_nsec;
    frequ = (data.image[ID].md[0].cnt0-cntlast)/tdiffv;
    cntlast = data.image[ID].md[0].cnt0;



    printw("[write %d] ", data.image[ID].md[0].write);
    printw("[status %2d] ", data.image[ID].md[0].status);
    printw("[cnt0 %8d] [%6.2f Hz] ", data.image[ID].md[0].cnt0, frequ);
    printw("[cnt1 %8d] ", data.image[ID].md[0].cnt1);
   // printw("[logstatus %2d] ", data.image[ID].logstatus[0]);
 
    if(data.image[ID].sem==1)
    {
        sem_getvalue(data.image[ID].semptr, &semval);
        printw("[Semaphore %3d] ", semval);
    }
    if(data.image[ID].sem1==1)
    {
        sem_getvalue(data.image[ID].semptr1, &semval);
        printw("[Semaphore 1 %3d] ", semval);
    }
    else
    {
        printw("[sem1=0]");
    }
    if(data.image[ID].semlog==1)
    {
        sem_getvalue(data.image[ID].semptrlog, &semval);
        printw("[Semaphore log %3d] ", semval);
    }
    else
    {
        printw("[semlog=0]");
    }


    printw("\n");

    average = arith_image_mean(data.image[ID].md[0].name);
    imtotal = arith_image_total(data.image[ID].md[0].name);
    printw("median %12g   ", arith_image_median(data.image[ID].md[0].name));
    printw("average %12g    total = %12g\n", imtotal/data.image[ID].md[0].nelement, imtotal);

    // printw("  RMS var = %g\n", );


    /*
    tdifflast = time_diff(tlast, zylaconf[0].tend);
    tdiffvlast = 1.0*tdifflast.tv_sec + 1.0e-9*tdifflast.tv_nsec;

    tlast = zylaconf[0].tend;
    freq = (zyladata[0].cnt - cntlast)/tdiffvlast;
    cntlast = zyladata[0].cnt;
    */

    /*  printw("\n");
    printw("Run time = %9.3f s\n", tdiffv);
    printw("Monitor refresh every %6.2f ms  (%5.1f Hz)", 1000.0*tdiffvlast, 1.0/tdiffvlast);
    printw("\n");
    printw("Main program PID : %ld\n", (long) zylaconf[0].pid_main);
    printw("CLI PID          : %ld\n", (long) zylaconf[0].pid_prompt);
    printw("\n");

    */

    /*

    switch (zylaconf[0].status) {
    case 0:
      attron(A_BLINK | A_BOLD | COLOR_PAIR(4));
      print_header(" CAMERA IS OFF ", ' ');
      attroff(A_BLINK | A_BOLD | COLOR_PAIR(4));
      break;
    case 1:
      attron(A_BLINK | A_BOLD | COLOR_PAIR(4));
      print_header(" SETTING UP ", ' ');
      attroff(A_BLINK | A_BOLD | COLOR_PAIR(4));
      break;
    case 2:
      attron(A_BLINK | A_BOLD | COLOR_PAIR(3));
      print_header(" CAMERA IS ACQUIRING ", ' ');
      attroff(A_BLINK | A_BOLD | COLOR_PAIR(3));
      break;
    case 3:
      attron(A_BLINK | A_BOLD | COLOR_PAIR(3));
      print_header(" PAUSED ", ' ');
      attroff(A_BLINK | A_BOLD | COLOR_PAIR(3));
      break;
    }

    printw("Exposure time    : %.2f ms\n", 1000.0*zylaconf[0].etime);
    */
    // printw("Frame %10ld   ", zyladata[0].cnt);

    /*  attron(A_BOLD);
    printw("%6.2f Hz\n", freq);
    attroff(A_BOLD);
    */


    vcnt = (long*) malloc(sizeof(long)*NBhistopt);

    if(atype==FLOAT)
    {
        minPV = data.image[ID].array.F[0];
        maxPV = minPV;

        for(h=0; h<NBhistopt; h++)
            vcnt[h] = 0;
        for(ii=0; ii<data.image[ID].md[0].nelement; ii++)
        {
            if(data.image[ID].array.F[ii]<minPV)
                minPV = data.image[ID].array.F[ii];
            if(data.image[ID].array.F[ii]>maxPV)
                maxPV = data.image[ID].array.F[ii];
            tmp = (1.0*data.image[ID].array.F[ii]-average);
            RMS += tmp*tmp;
            h = (long) (1.0*NBhistopt*((float) (data.image[ID].array.F[ii]-minPV))/(maxPV-minPV));
            if((h>-1)&&(h<NBhistopt))
                vcnt[h]++;
        }
    }

    if(atype==USHORT)
    {
        minPV = data.image[ID].array.U[0];
        maxPV = minPV;

        for(h=0; h<NBhistopt; h++)
            vcnt[h] = 0;
        for(ii=0; ii<data.image[ID].md[0].nelement; ii++)
        {
            if(data.image[ID].array.U[ii]<minPV)
                minPV = data.image[ID].array.U[ii];
            if(data.image[ID].array.U[ii]>maxPV)
                maxPV = data.image[ID].array.U[ii];
            tmp = (1.0*data.image[ID].array.U[ii]-average);
            RMS += tmp*tmp;
            h = (long) (1.0*NBhistopt*((float) (data.image[ID].array.U[ii]-minPV))/(maxPV-minPV));
            if((h>-1)&&(h<NBhistopt))
                vcnt[h]++;
        }
    }



    RMS = sqrt(RMS/data.image[ID].md[0].nelement);
    RMS01 = 0.9*RMS01 + 0.1*RMS;

    printw("RMS = %12.6g     ->  %12.6g\n", RMS, RMS01);

    print_header(" PIXEL VALUES ", '-');
    printw("min - max   :   %12.6e - %12.6e\n", minPV, maxPV);

    if(data.image[ID].md[0].nelement>10)
    {
        vcntmax = 0;
        for(h=0; h<NBhistopt; h++)
            if(vcnt[h]>vcntmax)
                vcntmax = vcnt[h];


        for(h=0; h<NBhistopt; h++)
        {
            customcolor = 1;
            if(h==NBhistopt-1)
                customcolor = 2;
            sprintf(line1, "[%12.4e - %12.4e] %7ld", (minPV + 1.0*(maxPV-minPV)*h/NBhistopt), (minPV + 1.0*(maxPV-minPV)*(h+1)/NBhistopt), vcnt[h]);

            printw("%s", line1); //(minPV + 1.0*(maxPV-minPV)*h/NBhistopt), (minPV + 1.0*(maxPV-minPV)*(h+1)/NBhistopt), vcnt[h]);
            attron(COLOR_PAIR(customcolor));

            cnt=0;
            i = 0;
            while((cnt<wcol-strlen(line1)-1)&&(i<vcnt[h]))
            {
                printw(" ");
                i += (long) (vcntmax/(wcol-strlen(line1)))+1;
                cnt++;
            }
            attroff(COLOR_PAIR(customcolor));
            printw("\n");
        }
    }
    else
    {
        if(data.image[ID].md[0].atype == FLOAT)
        {
            for(ii=0; ii<data.image[ID].md[0].nelement; ii++)
            {
                printw("%3ld  %f\n", ii, data.image[ID].array.F[ii]);
            }
        }

        if(data.image[ID].md[0].atype == USHORT)
        {
            for(ii=0; ii<data.image[ID].md[0].nelement; ii++)
            {
                printw("%3ld  %5u\n", ii, data.image[ID].array.U[ii]);
            }
        }

    }

    free(vcnt);


    return(0);
}




int info_pixelstats_smallImage(long ID, long NBpix)
{
    long ii;

    if(data.image[ID].md[0].atype == FLOAT)
    {
        for(ii=0; ii<NBpix; ii++)
        {
            printw("%3ld  %f\n", ii, data.image[ID].array.F[ii]);
        }
    }

    if(data.image[ID].md[0].atype == USHORT)
    {
        for(ii=0; ii<NBpix; ii++)
        {
            printw("%3ld  %5u\n", ii, data.image[ID].array.U[ii]);
        }
    }



    return(0);
}




int info_image_monitor(char *ID_name, double frequ)
{
    long ID;
    long mode = 0; // 0 for large image, 1 for small image
    long NBpix;
    long npix;

    ID = image_ID(ID_name);
    npix = data.image[ID].md[0].nelement;


    initscr();
    getmaxyx(stdscr, wrow, wcol);

    if(npix<100)
        mode = 1;

    NBpix = npix;
    if(NBpix > wrow)
        NBpix = wrow-2;

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_RED);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    while( !kbdhit() )
    {
        usleep((long) (1000000.0/frequ));
        clear();
        attron(A_BOLD);
        print_header(" PRESS ANY KEY TO STOP MONITOR ", '-');
        attroff(A_BOLD);

        //if(mode==0)
        printstatus(ID);
        //else
        //  info_pixelstats_smallImage(ID, NBpix);

        refresh();
    }
    endwin();

    return 0;
}





long brighter(char *ID_name, double value) /* number of pixels brighter than value */
{
  int ID;
  long ii,jj;
  long naxes[2];
  long brighter, fainter;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
    
  brighter = 0;
  fainter = 0;
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++){
      if(data.image[ID].array.F[jj*naxes[0]+ii]>value)
	brighter++;
      else
	fainter++;
    }
  printf("brighter %ld   fainter %ld\n", brighter, fainter );

  return(brighter);
}

int img_nbpix_flux(char *ID_name)
{
  int ID;
  long ii,jj;
  long naxes[2];
  double value = 0;
  double *array;
  long nelements,i;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0]*naxes[1];
  
  array = (double*) malloc(naxes[1]*naxes[0]*sizeof(double));
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      array[jj*naxes[0]+ii] = data.image[ID].array.F[jj*naxes[0]+ii];

  quick_sort_double(array, nelements);
  
  for(i=0;i<nelements;i++)
    {
      value += array[i];
      printf("%ld  %20.18e\n", i, value);
    }

  free(array);
  return(0);
}

float img_percentile_float(char *ID_name, float p)
{
  int ID;
  long ii;
  long naxes[2];
  float value = 0;
  float *array;
  long nelements;
  long n;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0]*naxes[1];
  
  array = (float*) malloc(nelements*sizeof(float));
  for (ii = 0; ii < nelements; ii++) 
    array[ii] = data.image[ID].array.F[ii];

  quick_sort_float(array, nelements);
  
  n = (long) (p*naxes[1]*naxes[0]);
  if(n>(nelements-1))
    n = (nelements-1);
  if(n<0)
    n = 0;
  value = array[n];
  free(array);

	printf("percentile %f = %f (%ld)\n", p, value, n);

  return(value);
}

double img_percentile_double(char *ID_name, double p)
{
  int ID;
  long ii;
  long naxes[2];
  double value = 0;
  double *array;
  long nelements;
  long n;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0]*naxes[1];
  
  array = (double*) malloc(nelements*sizeof(double));
  for (ii = 0; ii < nelements; ii++) 
    array[ii] = data.image[ID].array.F[ii];

  quick_sort_double(array, nelements);
  
  n = (long) (p*naxes[1]*naxes[0]);
  if(n>(nelements-1))
    n = (nelements-1);
  if(n<0)
    n = 0;
  value = array[n];
  free(array);

  return(value);
}

double img_percentile(char *ID_name, double p)
{
  long ID;
  int atype;
  double value = 0.0;

  ID = image_ID(ID_name);
  atype = data.image[ID].md[0].atype;

  if(atype==FLOAT)
    value = (double) img_percentile_float(ID_name, (float) p);
  if(atype==DOUBLE)
    value = img_percentile_double(ID_name, p);

  return value;
}


int img_histoc_float(char *ID_name, char *fname)
{
  FILE *fp;
  int ID;
  long ii,jj;
  long naxes[2];
  float value = 0;
  float *array;
  long nelements;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0]*naxes[1];
  
  array = (float*) malloc(naxes[1]*naxes[0]*sizeof(float));
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      array[jj*naxes[0]+ii] = data.image[ID].array.F[jj*naxes[0]+ii];

  quick_sort_float(array, nelements);
  
  if((fp=fopen(fname,"w"))==NULL)
    {
      printf("ERROR: cannot open file \"%s\"\n",fname);
      exit(0);
    }
  value = 0.0;
  for(ii=0;ii<nelements;ii++)
    {
      value += array[ii];
      if(ii>0.99*nelements)
	fprintf(fp,"%ld %g %g\n", nelements-ii, value, array[ii]);
    }

  fclose(fp);
  free(array);

  return(0);
}

int img_histoc_double(char *ID_name, char *fname)
{
  FILE *fp;
  int ID;
  long ii,jj;
  long naxes[2];
  double value = 0;
  double *array;
  long nelements;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0]*naxes[1];
  
  array = (double*) malloc(naxes[1]*naxes[0]*sizeof(double));
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      array[jj*naxes[0]+ii] = data.image[ID].array.F[jj*naxes[0]+ii];

  quick_sort_double(array, nelements);
  
  if((fp=fopen(fname,"w"))==NULL)
    {
      printf("ERROR: cannot open file \"%s\"\n",fname);
      exit(0);
    }
  value = 0.0;
  for(ii=0;ii<nelements;ii++)
    {
      value += array[ii];
      if(ii>0.99*nelements)
	fprintf(fp,"%ld %g %g\n",nelements-ii,value,array[ii]);
    }

  fclose(fp);
  free(array);

  return(0);
}


int make_histogram(char *ID_name, char *ID_out_name, double min, double max, long nbsteps)
{
  int ID,ID_out;
  long ii,jj;
  long naxes[2];
  long n;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    

  create_2Dimage_ID(ID_out_name,nbsteps,1);
  ID_out = image_ID(ID_out_name);
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      {
	n = (long) ((data.image[ID].array.F[jj*naxes[0]+ii]-min)/(max-min)*nbsteps);
	if((n>0)&&(n<nbsteps))
	  data.image[ID_out].array.F[n] += 1;
      }
  return(0);
}

double ssquare(char *ID_name)
{
  int ID;
  long ii,jj;
  long naxes[2];
  double ssquare;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
    
  ssquare = 0;
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++){
      ssquare = ssquare + data.image[ID].array.F[jj*naxes[0]+ii]*data.image[ID].array.F[jj*naxes[0]+ii];
    }
  return(ssquare);
}

double rms_dev(char *ID_name)
{
  int ID;
  long ii,jj;
  long naxes[2];
  double ssquare,rms;
  double constant;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
    
  ssquare = 0;
  constant = arith_image_total(ID_name)/naxes[0]/naxes[1];
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++){
      ssquare = ssquare + (data.image[ID].array.F[jj*naxes[0]+ii]-constant)*(data.image[ID].array.F[jj*naxes[0]+ii]-constant);
    }
  rms = sqrt(ssquare/naxes[1]/naxes[0]);
  return(rms);
}




// option "fileout" : output to file imstat.info.txt
int info_image_stats(char *ID_name, char *options)
{
    int ID;
    long ii,jj,j;
    double min,max;
    double rms;
    long nelements;
    double tot;
    double *array;
    long iimin,iimax;
    int atype;
    long tmp_long;
    char type[20];
    char vname[200];
    double xtot,ytot;
    double vbx,vby;
    FILE *fp;
    int mode = 0;

    // printf("OPTIONS = %s\n",options);
    if (strstr(options,"fileout")!=NULL)
        mode = 1;

    if(mode == 1)
        fp = fopen("imstat.info.txt","w");


    ID = image_ID_noaccessupdate(ID_name);
    if(ID!=-1)
    {
        nelements =  data.image[ID].md[0].nelement;

        atype = data.image[ID].md[0].atype;
        tmp_long = data.image[ID].md[0].nelement*TYPESIZE[atype];
        printf("\n");
        printf("Image size (->imsize0...):     [");
        printf("% ld",data.image[ID].md[0].size[0]);
        j = 0;
        sprintf(vname,"imsize%ld",j);
                
        create_variable_ID(vname,1.0*data.image[ID].md[0].size[j]);
        for(j=1; j<data.image[ID].md[0].naxis; j++)
        {
            printf(" %ld",data.image[ID].md[0].size[j]);
            sprintf(vname,"imsize%ld",j);
            create_variable_ID(vname,1.0*data.image[ID].md[0].size[j]);
        }
        printf(" ]\n");

		printf("write = %d   cnt0 = %ld   cnt1 = %ld\n", data.image[ID].md[0].write, data.image[ID].md[0].cnt0, data.image[ID].md[0].cnt1);


        if(atype==CHAR)
            sprintf(type,"CHAR");
        if(atype==INT)
            sprintf(type,"INT");
        if(atype==FLOAT)
            sprintf(type,"FLOAT");
        if(atype==DOUBLE)
            sprintf(type,"DOUBLE");
        if(atype==COMPLEX_FLOAT)
            sprintf(type,"CFLOAT");
        if(atype==COMPLEX_DOUBLE)
            sprintf(type,"CDOUBLE");
        printf("type:            %s\n",type);
        printf("Memory size:     %ld Kb\n",(long) tmp_long/1024);
        //      printf("Created:         %f\n", data.image[ID].creation_time);
        //      printf("Last access:     %f\n", data.image[ID].last_access);



        min = data.image[ID].array.F[0];
        max = data.image[ID].array.F[0];

        iimin = 0;
        iimax = 0;
        for (ii = 0; ii < nelements; ii++)
        {
            if (min > data.image[ID].array.F[ii])
            {
                min = data.image[ID].array.F[ii];
                iimin = ii;
            }
            if (max < data.image[ID].array.F[ii])
            {
                max = data.image[ID].array.F[ii];
                iimax = ii;
            }
        }

        array = (double*) malloc(nelements*sizeof(double));
        tot = 0.0;

        rms = 0.0;
        for (ii = 0; ii < nelements; ii++)
        {
            if(isnan(data.image[ID].array.F[ii])!=0)
            {
                printf("element %ld is NAN -> replacing by 0\n", ii);
                data.image[ID].array.F[ii] = 0.0;
            }
            tot += data.image[ID].array.F[ii];
            rms += data.image[ID].array.F[ii]*data.image[ID].array.F[ii];
            array[ii] = data.image[ID].array.F[ii];
        }
        rms = sqrt(rms);

        printf("minimum         (->vmin)     %20.18e [ pix %ld ]\n", min, iimin);
        if(mode == 1)
            fprintf(fp,"minimum                  %20.18e [ pix %ld ]\n", min, iimin);
        create_variable_ID("vmin",min);
        printf("maximum         (->vmax)     %20.18e [ pix %ld ]\n", max, iimax);
        if(mode == 1)
            fprintf(fp,"maximum                  %20.18e [ pix %ld ]\n", max, iimax);
        create_variable_ID("vmax",max);
        printf("total           (->vtot)     %20.18e\n",tot);
        if(mode == 1)
            fprintf(fp,"total                    %20.18e\n",tot);
        create_variable_ID("vtot",tot);
        printf("rms             (->vrms)     %20.18e\n",rms);
        if(mode == 1)
            fprintf(fp,"rms                      %20.18e\n",rms);
        create_variable_ID("vrms",rms);
        printf("rms per pixel   (->vrmsp)    %20.18e\n",rms/sqrt(nelements));
        if(mode == 1)
            fprintf(fp,"rms per pixel            %20.18e\n",rms/sqrt(nelements));
        create_variable_ID("vrmsp",rms/sqrt(nelements));
        printf("rms dev per pix (->vrmsdp)   %20.18e\n",sqrt(rms*rms/nelements - tot*tot/nelements/nelements));
        create_variable_ID("vrmsdp",sqrt(rms*rms/nelements - tot*tot/nelements/nelements));
        printf("mean            (->vmean)    %20.18e\n",tot/nelements);
        if(mode == 1)
            fprintf(fp,"mean                     %20.18e\n",tot/nelements);
        create_variable_ID("vmean",tot/nelements);

        if(data.image[ID].md[0].naxis==2)
        {
            xtot = 0.0;
            ytot = 0.0;
            for(ii=0; ii<data.image[ID].md[0].size[0]; ii++)
                for(jj=0; jj<data.image[ID].md[0].size[1]; jj++)
                {
                    xtot += data.image[ID].array.F[jj*data.image[ID].md[0].size[0]+ii]*ii;
                    ytot += data.image[ID].array.F[jj*data.image[ID].md[0].size[0]+ii]*jj;
                }
            vbx = xtot/tot;
            vby = ytot/tot;
            printf("Barycenter x    (->vbx)      %20.18f\n",vbx);
            if(mode == 1)
                fprintf(fp,"photocenterX             %20.18e\n",vbx);
            create_variable_ID("vbx",vbx);
            printf("Barycenter y    (->vby)      %20.18f\n",vby);
            if(mode == 1)
                fprintf(fp,"photocenterY             %20.18e\n",vby);
            create_variable_ID("vby",vby);
        }

        quick_sort_double(array, nelements);
        printf("\n");
        printf("percentile values:\n");

        printf("1  percent      (->vp01)     %20.18e\n",array[(long) (0.01*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile01             %20.18e\n",array[(long) (0.01*nelements)]);
        create_variable_ID("vp01",array[(long) (0.01*nelements)]);

        printf("5  percent      (->vp05)     %20.18e\n",array[(long) (0.05*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile05             %20.18e\n",array[(long) (0.05*nelements)]);
        create_variable_ID("vp05",array[(long) (0.05*nelements)]);

        printf("10 percent      (->vp10)     %20.18e\n",array[(long) (0.1*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile10             %20.18e\n",array[(long) (0.10*nelements)]);
        create_variable_ID("vp10",array[(long) (0.1*nelements)]);

        printf("20 percent      (->vp20)     %20.18e\n",array[(long) (0.2*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile20             %20.18e\n",array[(long) (0.20*nelements)]);
        create_variable_ID("vp20",array[(long) (0.2*nelements)]);

        printf("50 percent      (->vp50)     %20.18e\n",array[(long) (0.5*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile50             %20.18e\n",array[(long) (0.50*nelements)]);
        create_variable_ID("vp50",array[(long) (0.5*nelements)]);

        printf("80 percent      (->vp80)     %20.18e\n",array[(long) (0.8*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile80             %20.18e\n",array[(long) (0.80*nelements)]);
        create_variable_ID("vp80",array[(long) (0.8*nelements)]);

        printf("90 percent      (->vp90)     %20.18e\n",array[(long) (0.9*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile90             %20.18e\n",array[(long) (0.90*nelements)]);
        create_variable_ID("vp90",array[(long) (0.9*nelements)]);

        printf("95 percent      (->vp95)     %20.18e\n",array[(long) (0.95*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile95             %20.18e\n",array[(long) (0.95*nelements)]);
        create_variable_ID("vp95",array[(long) (0.95*nelements)]);

        printf("99 percent      (->vp99)     %20.18e\n",array[(long) (0.99*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile99             %20.18e\n",array[(long) (0.99*nelements)]);
        create_variable_ID("vp99",array[(long) (0.99*nelements)]);

        printf("99.5 percent    (->vp995)    %20.18e\n",array[(long) (0.995*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile995            %20.18e\n",array[(long) (0.995*nelements)]);
        create_variable_ID("vp995",array[(long) (0.995*nelements)]);

        printf("99.8 percent    (->vp998)    %20.18e\n",array[(long) (0.998*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile998            %20.18e\n",array[(long) (0.998*nelements)]);
        create_variable_ID("vp998",array[(long) (0.998*nelements)]);

        printf("99.9 percent    (->vp999)    %20.18e\n",array[(long) (0.999*nelements)]);
        if(mode == 1)
            fprintf(fp,"percentile999            %20.18e\n",array[(long) (0.999*nelements)]);
        create_variable_ID("vp999",array[(long) (0.999*nelements)]);

        printf("\n");
        free(array);
    }

    if(mode == 1)
        fclose(fp);

    return(0);
}




double img_min(char *ID_name)
{
  int ID;
  long ii;
  double min;

  ID = image_ID(ID_name);

  min = data.image[ID].array.F[0];
  for (ii = 0; ii < data.image[ID].md[0].nelement; ii++) 
    if (min > data.image[ID].array.F[ii])
      min = data.image[ID].array.F[ii];

   return(min);
}

double img_max(char *ID_name)
{
  int ID;
  long ii;
  double max;

  ID = image_ID(ID_name);

  max = data.image[ID].array.F[0];
    for (ii = 0; ii < data.image[ID].md[0].nelement; ii++) 
      if (max < data.image[ID].array.F[ii])
	max = data.image[ID].array.F[ii];

   return(max);
}

int profile(char *ID_name, char *outfile, double xcenter, double ycenter, double step, long nb_step)
{
  int ID;
  long ii,jj;
  long naxes[2];
  long nelements;
  double distance;
  double *dist;
  double *mean;
  double *rms;
  long *counts;
  FILE *fp;
  long i;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0] * naxes[1]; 
  dist = (double*) malloc(nb_step*sizeof(double));
  mean = (double*) malloc(nb_step*sizeof(double));
  rms = (double*) malloc(nb_step*sizeof(double));
  counts = (long*) malloc(nb_step*sizeof(long));
  
  //  if( Debug )
  //printf("Function profile. center = %f %f, step = %f, NBstep = %ld\n",xcenter,ycenter,step,nb_step);
  
  for (i=0;i<nb_step;i++)
    {
      dist[i] = 0.0;
      mean[i] = 0.0;
      rms[i] = 0.0;
      counts[i] = 0;
    }

  if ((fp=fopen(outfile,"w"))==NULL)
    printf("error : can't open file %s\n",outfile);
 
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
      rms[i] = 0.0;
    }

  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++){
      distance = sqrt((1.0*ii-xcenter)*(1.0*ii-xcenter)+(1.0*jj-ycenter)*(1.0*jj-ycenter));
      i = (long) distance/step;
      if(i<nb_step)
	{
	  rms[i] += (data.image[ID].array.F[jj*naxes[0]+ii]-mean[i])*(data.image[ID].array.F[jj*naxes[0]+ii]-mean[i]);
	  //	  counts[i] += 1;
	}
    }

  printf("--------- TEST ----------\n");
  
  for (i=0;i<nb_step;i++)
    {
      //     dist[i] /= counts[i];
      // mean[i] /= counts[i];
      // rms[i] = sqrt(rms[i]-1.0*counts[i]*mean[i]*mean[i])/sqrt(counts[i]);
      rms[i] = sqrt(rms[i]/counts[i]);
      fprintf(fp,"%.18f %.18f %.18f %ld %ld\n", dist[i], mean[i], rms[i], counts[i], i);
    }
  

  fclose(fp);

  free(counts);
  free(dist);
  free(mean);
  free(rms);
  return(0);
}

int profile2im(char *profile_name, long nbpoints, long size, double xcenter, double ycenter, double radius, char *out)
{
  FILE *fp;
  long ID;
  double *profile_array;
  long i;
  long index;
  double tmp;
  long ii,jj;
  double r,x;

  ID = create_2Dimage_ID(out,size,size);
  profile_array = (double*) malloc(sizeof(double)*nbpoints);

  if((fp=fopen(profile_name,"r"))==NULL)
    {
      printf("ERROR: cannot open profile file \"%s\"\n",profile_name);
      exit(0);
    }
  for(i=0;i<nbpoints;i++)
    {
      if(fscanf(fp,"%ld %lf\n",&index,&tmp)!=2)
	{
	  printf("ERROR: fscanf, %s line %d\n",__FILE__,__LINE__);
	  exit(0);
	}
      profile_array[i] = tmp;
    }
  fclose(fp);

  for(ii=0;ii<size;ii++)
    for(jj=0;jj<size;jj++)
      {
	r = sqrt((1.0*ii-xcenter)*(1.0*ii-xcenter)+(1.0*jj-ycenter)*(1.0*jj-ycenter))/radius;
	i = (long) (r*nbpoints);
	x = r*nbpoints-i; // 0<x<1

	if(i+1<nbpoints)
	  {
	    data.image[ID].array.F[jj*size+ii] = (1.0-x)*profile_array[i]+x*profile_array[i+1];
	  }
	else
	  if(i<nbpoints)
	    data.image[ID].array.F[jj*size+ii] = profile_array[i];
      }

  free(profile_array);

  return(0);
}

int printpix(char *ID_name, char *filename)
{
  int ID;
  long ii,jj,kk;
  long nelements;
  long nbaxis;
  long naxes[3];
  FILE *fp;

  long iistep = 1;
  long jjstep = 1;

  ID = variable_ID("_iistep");
  if(ID!=-1)
      {
      iistep = (long) (0.1+data.variable[ID].value.f);
      printf("iistep = %ld\n", iistep);
    }
  ID = variable_ID("_jjstep");
  if(ID!=-1)
    {
      jjstep = (long) (0.1+data.variable[ID].value.f);
       printf("jjstep = %ld\n", jjstep);
    }

  if((fp=fopen(filename,"w"))==NULL)
    {
      printf("ERROR: cannot open file \"%s\"\n",filename);
      exit(0);
    }

  ID = image_ID(ID_name);
  nbaxis = data.image[ID].md[0].naxis;
  if(nbaxis==2)
    {
      naxes[0] = data.image[ID].md[0].size[0];
      naxes[1] = data.image[ID].md[0].size[1];    
      nelements = naxes[0] * naxes[1]; 
      for (ii = 0; ii < naxes[0]; ii+=iistep)
	{
	  for (jj = 0; jj < naxes[1]; jj+=jjstep)
	    {
	      //  fprintf(fp,"%f ",data.image[ID].array.F[jj*naxes[0]+ii]);
	      fprintf(fp,"%ld %ld %g\n",ii,jj,data.image[ID].array.F[jj*naxes[0]+ii]);
	    }
	  fprintf(fp,"\n");
	}      
    }
  if(nbaxis==3)
    {
      naxes[0] = data.image[ID].md[0].size[0];
      naxes[1] = data.image[ID].md[0].size[1];    
      naxes[2] = data.image[ID].md[0].size[2];    
      nelements = naxes[0] * naxes[1]; 
      for (ii = 0; ii < naxes[0]; ii+=iistep) 
	for (jj = 0; jj < naxes[1]; jj+=jjstep)
	  for (kk = 0; kk < naxes[2]; kk++)
	    {
	      fprintf(fp,"%ld %ld %ld %f\n",ii,jj,kk,data.image[ID].array.F[kk*naxes[1]*naxes[0]+jj*naxes[0]+ii]);
	    }
    
    }
  fclose(fp);

  return(0);
}

double background_photon_noise(char *ID_name)
{
  int ID;
  long ii,jj;
  long naxes[2];
  double value1, value2, value3, value;
  double *array;
  long nelements;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    
  nelements = naxes[0]*naxes[1];
  
  array = (double*) malloc(naxes[1]*naxes[0]*sizeof(double));
  for (jj = 0; jj < naxes[1]; jj++) 
    for (ii = 0; ii < naxes[0]; ii++)
      array[jj*naxes[0]+ii] = data.image[ID].array.F[jj*naxes[0]+ii];

  quick_sort_double(array,nelements);
  
  /* uses the repartition function F of the normal distribution law */
  /* F(0) = 0.5 */
  /* F(-0.1 * sig) = 0.460172162723 */
  /* F(-0.2 * sig) = 0.420740290562 */
  /* F(-0.3 * sig) = 0.382088577811 */
  /* F(-0.4 * sig) = 0.34457825839 */
  /* F(-0.5 * sig) = 0.308537538726 */
  /* F(-0.6 * sig) = 0.27425311775 */
  /* F(-0.7 * sig) = 0.241963652223 */
  /* F(-0.8 * sig) = 0.211855398584 */
  /* F(-0.9 * sig) = 0.184060125347 */
  /* F(-1.0 * sig) = 0.158655253931 */
  /* F(-1.1 * sig) = 0.135666060946 */
  /* F(-1.2 * sig) = 0.115069670222 */
  /* F(-1.3 * sig) = 0.0968004845855 */

  /* calculation using F(-0.9*sig) and F(-1.3*sig) */
  value1 = array[(long) (0.184060125347*naxes[1]*naxes[0])]-array[(long) (0.0968004845855*naxes[1]*naxes[0])];
  value1 /= (1.3-0.9);
  printf("(-1.3 -0.9) %f\n",value1);

  /* calculation using F(-0.6*sig) and F(-1.3*sig) */
  value2 = array[(long) (0.27425311775*naxes[1]*naxes[0])]-array[(long) (0.0968004845855*naxes[1]*naxes[0])];
  value2 /= (1.3-0.6);
  printf("(-1.3 -0.6) %f\n",value2);

  /* calculation using F(-0.3*sig) and F(-1.3*sig) */
  value3 = array[(long) (0.382088577811*naxes[1]*naxes[0])]-array[(long) (0.0968004845855*naxes[1]*naxes[0])];
  value3 /= (1.3-0.3);
  printf("(-1.3 -0.3) %f\n",value3);

  value = value3;
  
  free(array);
  return(value);
}

int test_structure_function(char *ID_name, long NBpoints, char *ID_out)
{
  int ID,ID1,ID2;
  long ii1,ii2,jj1,jj2,i,ii,jj;
  long naxes[2];
  long nelements;
  double v1,v2;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    

  nelements = naxes[0]*naxes[1];

  ID1=create_2Dimage_ID("tmp1",naxes[0],naxes[1]);
  ID2=create_2Dimage_ID("tmp2",naxes[0],naxes[1]);
  
  for(i=0;i<NBpoints;i++)
    {
      ii1=(long) (data.INVRANDMAX*rand()*naxes[0]);
      jj1=(long) (data.INVRANDMAX*rand()*naxes[1]);
      ii2=(long) (data.INVRANDMAX*rand()*naxes[0]);
      jj2=(long) (data.INVRANDMAX*rand()*naxes[1]);
      v1=data.image[ID].array.F[jj1*naxes[0]+ii1];
      v2=data.image[ID].array.F[jj2*naxes[0]+ii2];
      ii=(ii1-ii2);
      if(ii<0)
	ii=-ii;
      jj=(jj1-jj2);
      if(jj<0)
	jj=-jj;
      data.image[ID1].array.F[jj*naxes[0]+ii] += (v1-v2)*(v1-v2);
      data.image[ID2].array.F[jj*naxes[0]+ii] += 1.0;
    }
  arith_image_div("tmp1","tmp2",ID_out);


  return(0);
}

int full_structure_function(char *ID_name, long NBpoints, char *ID_out)
{
  int ID,ID1,ID2;
  long ii1,ii2,jj1,jj2;
  long naxes[2];
  double v1,v2;
  long i=0;
  long STEP1=2;
  long STEP2=3;

  ID = image_ID(ID_name);
  naxes[0] = data.image[ID].md[0].size[0];
  naxes[1] = data.image[ID].md[0].size[1];    

  ID1=create_2Dimage_ID("tmp1",naxes[0],naxes[1]);
  ID2=create_2Dimage_ID("tmp2",naxes[0],naxes[1]);
  

  for(ii1=0;ii1<naxes[0];ii1+=STEP1)
    {
      printf(".");
      for(jj1=0;jj1<naxes[1];jj1+=STEP1)
	{
	  if(i<NBpoints)
	    {
	      i++;
	      fflush(stdout);
	      for(ii2=0;ii2<naxes[0];ii2+=STEP2)
		for(jj2=0;jj2<naxes[1];jj2+=STEP2)
		  if((ii2>ii1)&&(jj2>jj1))
		    {
		      v1=data.image[ID].array.F[jj1*naxes[0]+ii1];
		      v2=data.image[ID].array.F[jj2*naxes[0]+ii2];
		      data.image[ID1].array.F[(jj2-jj1)*naxes[0]+ii2-ii1] += (v1-v2)*(v1-v2);
		      data.image[ID2].array.F[(jj2-jj1)*naxes[0]+ii2-ii1] += 1.0;
		    }
	    }
	}
    }
  printf("\n");

  arith_image_div("tmp1","tmp2",ID_out);

  return(0);
}

int fft_structure_function(char *ID_in, char *ID_out)
{
  long ID;
  double value;
  long nelement;

  autocorrelation(ID_in,"stftmp");
  ID=image_ID("stftmp");
  nelement = data.image[ID].md[0].nelement;
  value=-data.image[ID].array.F[0];
  
  arith_image_cstadd("stftmp",value,"stftmp1");
  delete_image_ID("stftmp");
  arith_image_cstmult("stftmp1",-2.0/sqrt(nelement),ID_out);
  delete_image_ID("stftmp1");

  return(0);
}
