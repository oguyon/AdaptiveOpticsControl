[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)



# Adaptive Optics Control Computation Engine (AOCCE)




Adaptive Optics Wavefront control tools for high contrast imaging

Uses shared memory for fast low-latency communication to hardware

Includes a simulator of hardware (DM, Camera): the same code can be tested on actual hardware or in a simulation mode.

Supports multiple simultaneously running control loops and data logging.





## Downloading source code
Latest distribution is on [github](
https://github.com/oguyon/AdaptiveOpticsControl).
You can clone the repository, or download the latest .tar.gz distribution.


## Compilation
The source code follows the standard GNU build process:

./configure

make

make install


## Documentation 
Please consult the [online documentation]( http://oguyon.github.io/AdaptiveOpticsControl/).


## Libraries
The following libraries are used:
- readline, for reading the command line input
- flex, for parsing the command line input
- bison, to interpret the command line input
- fftw, for performing Fourier Transforms
- gsl, for math functions and tools
- fitsio, for reading and writing FITS image files

## Source Code Architecture 
Written in C.
The main is a command line interface (CLI). Source code is in CLIcore.c and CLIcore.h.
Key data structures (such as the image data structure) are declared in CLIcore.h.

## How to run the turbulence simulator
Copy scripts from ./src/AtmosphericTurbulence/scripts directory to working directory.
Edit and execute the main script "runturb"


## Credits
This software was developed with support from the National Science Foundation (award #1006063), NINS / NAOJ / Subaru Telescope, NINS / Astrobiology Centerand the Japanese Society for the Promotion of Science (JSPS).
