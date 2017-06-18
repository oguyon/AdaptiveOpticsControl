# AO loop control {#AOloopControl}


This module allows control of adaptive optics (AO) control loops.

Source code in AOloopControl.c and AOloopControl.h

Main features:
- uses shared memory to communicate between processes and link to wavefront sensor camera/data and deformable mirrors
- multiple loops can run simultaneously, independantly or together
- single process can control multiple loops
- Can use either CPU(s) or GPU(s) for computations




# Scripts



## aolconf (top level script)

The top level script is aolconf, which is used to configure the loop and run it. It also includes a test mode with a simulated AO system.
aolconf is located in the AOloopControl/scripts directory, along with multiple scripts that it calls.










## aolrun

The following table lists all input / files required prior to running aolrun

Variable Name                       | Description                                      | Setting location
------------------------------------|--------------------------------------------------|-----------------------------------
LOOPNUMBER                          | loop number [integer]                            | local file LOOPNUMBER
AOconf[loop].DMname                 | shared memory DM image (for correction)          | image aol<LOOPNUMBER>_dmC 
AOconf[loop].DMnameRM               | shared memory DM image (for response matrix acq) | image aol<LOOPNUMBER>_dmRM 
AOconf[loop].WFSname                | shared memory WFS image                          | image aol<LOOPNUMBER>_wfs 
AOconf[loop].DMMODESname            | DM modes                                         | image aol<LOOPNUMBER>_DMmodes 
AOconf[loop].respMname              | response matrix                                  | image aol<LOOPNUMBER>_RespM 
AOconf[loop].contrMname             | control matrix                                   | image aol<LOOPNUMBER>_ContrM 
AOconf[loop].name                   | loop name                                        | local file ./conf/conf_LOOPNAME.txt
AOconf[loop].GPU                    | number of GPUs used                              | local file ./conf/conf_GPU.txt
AOconf[loop].GPUall                 | skip CPU image scaling and go straight to CPU ?  | local file ./conf/conf_GPUall.txt
AOconf[loop].AOLCOMPUTE_TOTAL_ASYNC | compute image total in separate thread ?         | local file ./conf/conf_COMPUTE_TOTAL_ASYNC.txt
MATRIX_COMPUTATION_MODE             | use single combined act-wfs matrix ?             | local file ./conf/conf_CMmode.txt







# Step-by-step instruction for typical operation

Control loops are numbered - this is how multiple loops can work together. <LOOPNB> can take any positive integer value (0, 1, 2, etc...)

Important notes: 
- <executable> is the name given to the executable program
- some commands below are stand-alone executable scripts, others are commands to be issued within the main execuatable, which are shown starting with "> ".
- stand alone scripts will need to be created if they do not exist: the scripts are provided in this page.

### Directories, configurations

System configurations are saved in independent directories named ./confxxx/. 

A configuration consists of:
- fmodes.fits   : Modes definition
- modesfreqcpa.fits : Modes spatial frequency (CPA)
- refwfs.fits   : WFS reference
- respm.fits    : Response matrix
- cmat.fits     : Control matrix
- additional control matrixes, named: cmat_<beta>_<nbMrm>.fits
	- <beta> is the explonent for low order modes enhancement in the SVD: for example 0.43
	- <nbMrm> is the number of modes removed (low eigenvalues)
- eigenmodesM_<beta>.fits : eigenmodes
- eigenmodesrespM_<beta>.fits : WFS response to eigenmodes
- TPmask.fits   : Telescope pupil mask (optional)
- TPind.fits    : modes to be removed from control (optional)


Scripts are used to load and save configurations as follows:

	./saveconf xxx
	./loadconf xxx

saveconf source:

\verbatim
mkdir -p conf$1
cp fmodes.fits ./conf$1/
cp refwfs.fits ./conf$1/
cp respm.fits ./conf$1/
cp cmat.fits ./conf$1/
cp cmat_*_*.fits ./conf$1/
cp AOloop*.conf ./conf$1/
\endverbatim

loadconf source:

\verbatim
cp ./conf$1/fmodes.fits .
cp ./conf$1/refwfs.fits .
cp ./conf$1/respm.fits .
cp ./conf$1/cmat.fits .
cp ./conf$1/AOloop*.conf .
\endverbatim



### OPTIONAL VARIABLES

The following variables are optional, and may be specified as command line variable:
- AOLCAMDARK: dark level for the camera images. By default, it is assumed that the camera images fed to the software are dark-subtracted. If they are not, it is possible to offset them by a dark value by entering:

	AOLCAMDARK=100.0


It is best to keep optional variable in the CLI startup script, which is automatically executed by the program upon startup. By default, the file name is 'CLIstartup.txt'. It can also be changed and loaded by using the '-s' option when launching the executable. Example startup script:

	aolnb 1
	AOLCAMDARK=100.0



---------

## CONFIGURATION




### (1) CREATE/EDIT AOloop<LOOPNB>.conf

The configuration for running the AO loop is in this file. Shared memory images names are specified, as well as loop configuration paramaters.

The function AOloopControl_makeTemplateAOloopconf(long loopnb) creates a template that can be edited:

	<executable>
	> aolmkconf <LOOPNB>


### (2) CREATE MODES

If possible, create the TPmask.fits file which contains the DM illumination. Illuminated actuators are set to 1, others to 0.

For purely Fourier Modes:

    ./mkModes <confindex> <CPAmax>

confindex: index for the configuration, for example, 002 will save modes in conf002 directory
CPAmax: maximum spatial frequency in the basis of modes, in unit of Cycle Per Aperture.

The mode file name is ./conf<confindex>/fmodes.fits. 


This script calls the function AOloopControl_mkModes(char *ID_name, long msize, float CPAmax, float deltaCPA, double xc, double yx, double r0, double r1) in AOloopControl.c.

Optional input:
- TPmask.fits : telescope pupil mask. If this file is present in the configuration directory, it will be used as a multiplicative mask (with convolotion to avoid sharp transitions) to the modes
- TPind.fits : modes to be excluded from the control

mkModes source:

\verbatim
# args: <confindex> <maxCPA>
EXPECTED_ARGS=2

if [ $# -ne $EXPECTED_ARGS ]
then
  echo
  echo "-------- create control modes ------------"
  echo "Usage:  $0 <configuration index> <maxCPA>"
  echo "   INPUT:  <configuration index>  : index, 001, 002, 003 etc.."
  echo "   INPUT:  <max CPA>              : maximum spatial frequ [cycles per aperture]"
  echo " EXAMPLE: $0 002 4.0"
  echo
  exit
fi





mkdir -p conf$1

echo "MODES_MAX_CPA      $2" > ./conf$1/fmodes.conf.txt


Cfits << EOF
loadfits "./conf$1/TPmask.fits" Mmask
loadfits "./conf$1/TPind.fits" emodes
aolmkmodes fmodes 50 $2 0.8 24.5 24.5 21.0 6.5
savefits fmodes "!./conf$1/fmodes.fits"
savefits modesfreqcpa "!./conf$1/modesfreqcpa.fits"
exit
EOF
\endverbatim



### (3) FINDING LOOP LATENCY (OPTIONAL)

    ./MeasureTD <confindex>

This script will create the "TimeDelayRM.txt", which gives signal strength as a function of frame delay. 

The first peak of this curve indicates the time delay and should be updated in the acquRespM script below.

MeasureTD source:
\verbatim
cp ./conf$1/fmodes.fits .
<executable> << EOF
aolnb <LOOPNB>
aolacqresp 20 0.05 10 -1 1
exit
EOF
\endverbatim

Once the delay is known, edit the acquRespM script (see below) accordingly.



### (4) ACQUIRE RESPONSE MATRIX

    ./acquRespM 000

Edit script to change modulation amplitude, number of images per DM state, etc...

This script calls function Measure_Resp_Matrix(long loop, long NbAve, float amp, long nbloop, long fDelay, long NBiter) in module AOloopControl.c.

The response matrix acquision will go for a large number of iterations, and data is progressively averaged to improve SNR. The last parameter of the acquRespM command in the script is the number of matrixes averaged. You can either wait for all iterations to complete or CTRL-C the process. If you CTRL-C the process, load the result in the configuration directory:

	cp respm.fits ./conf<xxx>/
	cp refwfs.fits ./conf<xxx>/

output: respm.fits, refwfs.fits

acquRespM source:
\verbatim
# Acquires Response Matrix
# Parameters for aolacqresp are:
# number of images per DM state
# modulation amplitude [um DM displacement]
# number of loop per matrix generation
# frame delay offset [number of frames]
# number of matrixes generated (will be continuously averaged)

# creates files "respm.fits" and "refwfs.fits"

./loadconf $1
<executable> -n aolacqresp<< EOF
aolnb 0
aolacqresp 10 0.02 2 2 100
exit
EOF
cp respm.fits ./conf$1/
cp refwfs.fits ./conf$1/
\endverbatim



### (5) COMPUTE CONTROL MATRIXES

    ./cmmake <confindex> <beta> <#modes removed>

output: cmat_<beta>_<#modes removed>.fits

Also writes eigenvalues in file "eigenv.dat".
Check that number of modes removed is consistent with eigenvalues.

<beta> is the enhancement factor for low order aberrations. For beta=0, there is no enhancement (straight SVD), while beta >0 forces the lowest modes to be low-order. Typical values from 0.50 to 4.00. 

cmmake source:
\verbatim
#
# Make control matrix
# arg 1: configuration directory
# arg 2: Beta coefficient (typically 0.50 to 4.00), enhances low order aberrations for SVD
# arg 3: max number of modes removed

cp ./conf$1/modesfreqcpa.fits .
cp ./conf$1/fmodes.fits .
cp ./conf$1/respm.fits .

<executable> << EOF
aolnb 0
loadfits respm.fits respm
loadfits fmodes.fits modesM
aolcmmake $3 respm cmat $2
savefits evecM "!evecM.fits"
exit
EOF
mv cmat_*_*.fits ./conf$1/
mv evecM.fits ./conf$1/
mv eigenv.dat ./conf$1/
mv eigenmodesM*.fits ./conf$1/
mv eigenmodesrespM*.fits ./conf$1/
\endverbatim

The script calls compute_ControlMatrix(long loop, long NB_MODE_REMOVED, char *ID_Rmatrix_name, char *ID_Cmatrix_name, char *ID_VTmatrix_name, double Beta) in module AOloopControl.c.



----------

## RUNNING THE LOOP


### (6) SET UP / LOAD CONFIGURATION

Steps above can be repeated to create several configurations. When adopting a configuration:
- Choose a control matrix
	cp conf000/cmat_1.00_003.fits conf000/cmat.fits
- load configuration
	./loadconf 000


### (7) RUN LOOP 


Start TWO instances of the program

In process #1, type :

    > aolnb <LOOPNB>
    > aolrun

In process #2, type :

    > aolnb <LOOPNB>
    > aolsetgain <loopgain>
    > aolon

Other useful commands :
- aolsetmaxlim <maximum_value_for_each_mode>
- aolsetmultfb <block_number> <multiplicative_factor>


### (8) LAUNCH MONITOR 

Start new process, type:

    > aolmon <frequ[Hz]> <number_of_columns>

For each mode, the following info is displayed:
\verbatim
[<gain_factor> <limit_factor> <multiplier_factor>]   <current_DM_value> <last_WFS_estimate> <average_val> <RMS> 
\endverbatim

