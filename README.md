[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.815633.svg)](https://doi.org/10.5281/zenodo.815633)




# Adaptive Optics Control Computation Engine (AOCCE)




Adaptive Optics Wavefront control tools for high contrast imaging

Uses shared memory for fast low-latency communication to hardware

Includes a simulator of hardware (DM, Camera): the same code can be tested on actual hardware or in a simulation mode.

Supports multiple simultaneously running control loops and data logging.





## Downloading source code
You can clone this repository, or download the latest .tar.gz distribution.


## Libraries

The following libraries are used:

- readline, for reading the command line input
- flex, for parsing the command line input
- bison, to interpret the command line input
- fftw, for performing Fourier Transforms
- gsl, for math functions and tools

Install above libraries (centOS):
		sudo yum install readline-devel flex bison-devel fftw3-devel gsl-devel



- fitsio, for reading and writing FITS image files : 
	Visit https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html and download the file Unix .tar file cfitsio3410.tar.gz
	Extract it , README , install it 
	There is the fitsio.h in it. Move it to usr :
		./configure --prefix=/usr
		make 
		sudo make install 



## Compilation from git clone

Install Development tools, use the command bellow. This will search the yum repositories, and install the tools from the closest repo.
		sudo yum groupinstall "Development tools"
		
The source code follows the standard GNU build process:
		autoreconf -vif
		./configure
		make
		make install


## Compilation from tarball


Unpack
		gunzip <package>.tar.gz
		tar -xvf <package>.tar

The source code follows the standard GNU build process:

		./configure
		make
		sudo make install


## Documentation 
Please consult the [online documentation]( http://oguyon.github.io/AdaptiveOpticsControl/).



## Source Code Architecture 
Written in C.
The main is a command line interface (CLI). Source code is in CLIcore.c and CLIcore.h.
Key data structures (such as the image data structure) are declared in CLIcore.h.

## How to run the turbulence simulator
Copy scripts from ./src/AtmosphericTurbulence/scripts directory to working directory.
Edit and execute the main script "runturb"


## Credits
This software was developed with support from the National Science Foundation (award #1006063), NINS / NAOJ / Subaru Telescope, NINS / Astrobiology Centerand the Japanese Society for the Promotion of Science (JSPS).
