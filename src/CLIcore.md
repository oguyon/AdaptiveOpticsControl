
## Command Line Interface (CLI)   

Files CLIcore.c and CLIcore.h contain the source code for the command line interpreter (CLI)

### COMMAND LINE OPTIONS

\verbatim
  -h, --help 
	print this message and exit
  -i, --info
	Print version, settings, info and exit
  -j, --journal
	keeps journal of commands - TO BE IMPLEMENTED 
	Write all commands to file "cfits_cmdlog.txt" as they are entered
  --verbose
	be verbose
  -d, --debug=DEBUGLEVEL
	Set debug level at startup
  -o, --overwrite 
	Automatically overwrite files if necessary (USE WITH CAUTION - WILL OVERWRITE EXISTING FITS FILES)
  -l
	Keeps a list of images in file imlist.txt
  -m, --mmon=TTYDEVICE
	open memory monitor on tty device
	example:
	<executable> -m /dev/tty2
	<executable> --mmon=/dev/tty2
  -n, --pname=<myprocessname>
	rename process to <processname>
  -p, --priority=<PR>
	change process priority (0-99)
	higher number: higher priority
	example:
	<executable> -p 90
  -f, --fifo==FIFONAME
        specify fifo name
        example
        <executable> -f /tmp/fifo24
        <executable> --fifo=/tmp/fifo24
  -s, --startup=STARTUPFILE
        execute specified script on startup
        requires the -f option, as the script is loaded into fifo
\endverbatim

## SYNTAX RULES, PARSER

- Spaces are used to separate arguments. Number of spaces irrelevant.
- Comments are written after the special character #
- If a command is not found, the input string will be interpreted as an arithmetic operation (See ARITHMETIC OPERATIONS below)

<command> <arg1> <arg2>   # comment


## TAB COMPLETION

Tab completion is provided and behaves as follows:
- first argument:        try to match command, then image, then filename
- additional arguments:  try to match image, then filename

## INPUT

GNU readline used to read input. See GNU readline documentation on http://tiswww.case.edu/php/chet/readline/rltop.html. For a quick help on readline input, type:
\verbatim
> helprl 
\endverbatim

The command line interpreter (CLI) will take input from file cmdfile.txt if it exists. If file cmdfile.txt exists commands will be read one by one from top to bottom, and will be removed from the file as they are read, until the file is empty


## HELP COMMANDS

\verbatim
> ?
> help			     
	# print this help file
> helprl		     
	# print readline quick help
> lm?                        
	# list all modules loaded
> m? <module>               
	# list all commands for a module
> m?                    
	# perform m? on all modules loaded
> cmd? <command>              
	# command description for <command>
> cmd?                    
	# command description for all commands
> h? str                    
	# search for string <str> in all commands and their descriptions
\endverbatim


## IMPORTANT COMMANDS

\verbatim
> ci
	# compilation time and memory usage
> listim 
	# list all images in memory
> listimf <filename> 
	# list all images in memory and write output to file <filename>
> !<syscommand>             
	# execute system command
> showhist                  
	# prints history of all commands
> quit                      
	# exit Cfits (exit also works)

> setdp <val> 		     
	# set default precision to float (<val> = 0) or double (<val> = 1)
> creaim <im> <xs> <ys>     
	# creates a 2D image named <im>, size = <xs> x <ys> pixels
\endverbatim

## FITS FILES I/O (see also modules COREMOD_memory and COREMOD_iofits

FITSIO is used for FITS files I/O, see FITSIO documentation for more detailed instructions\n

### LOADING FILES

\verbatim
> loadfits <fname> <imname> 
	# load FITS file <fname> into image <imname>
> loadfits im1.fits imf1    
	# load file im1.fits in memory with name imf1
> loadfits im1.fits	     
	# load file im1.fits in memory with name im1 (default name is composed of all chars before first ".")
> loadfits im1.fits.gz im1  
	# load compressed file
\endverbatim

### SAVING FILES

\verbatim
> save_fl  <imname> <fname> 
	# save image <imname> into FITS file <fname> (float)
> save_fl im1 imf1.fits      
	# write image im1 to disk file imf1.fits
> save_fl im1
	# write image im1 to disk file im1.fits (default file name = image name + ".fits")
> save_fl im1 "!im1.fits"   
	# overwrite file im1.fits if it exists
> save_fl im1 "../dir2/im1.fits"
	# specify full path
> save_fl im1 im1.fits.gz   
	# save compressed image
\endverbatim


## INTEGRATION WITH STANDARD LINUX TOOLS AND COMMANDS


USING "cmdfile.txt" TO DRIVE CFITS FROM UNIX PROMPT:

Cfits can use standard linux tools and commands thanks to the cmdfile.txt file, which, if it exists, is executed as Cfits commands.
For example, to load all im*.fits files in memory, you can type within Cfits:

> !ls im*.fits | xargs -I {} echo loadfits {} > cmdfile.txt

You can also drive Cfits from the unix command line if you are not in Cfits, but Cfits is running in the same directory. For example, the following command will load all im*.fits in Cfits from the unix command line:

bash$ ls im*.fits | xargs -I {} echo loadfits {} > cmdfile.txt


USING "imlist.txt" AND "cmdfile.txt"

If you start Cfits with the "-l" option,  the file "imlist.txt" contains the list of images currently in memory in a ASCII table. You can use standard unix tools to process this list and issue commands. For example, if you want to save all images with x size > 200 onto disk as single precision FITS files :

> !awk '{if ($4>200) print $2}' imlist.txt| xargs -I {} echo save_fl {} {}_tmp.fits > cmdfile.txt


## ARITHMETIC OPERATIONS

\verbatim
> im1=sqrt(im+2.0)          
	# will perform an arithmetic operation on image im and store the result in image im1
\endverbatim









