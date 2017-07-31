[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.815633.svg)](https://doi.org/10.5281/zenodo.815633)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/8091ec2f16214595a72dcd41a4e18942)](https://www.codacy.com/app/oguyon/AdaptiveOpticsControl?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=oguyon/AdaptiveOpticsControl&amp;utm_campaign=Badge_Grade)




# Adaptive Optics Computation Engine (AOCCE)


## Overview

Adaptive Optics Wavefront control tools for high contrast imaging

Uses shared memory for fast low-latency communication to hardware

Includes a simulator of hardware (DM, Camera): the same code can be tested on actual hardware or in a simulation mode.

Supports multiple simultaneously running control loops and data logging.


[online documentation]( http://oguyon.github.io/AdaptiveOpticsControl/) 




## Downloading and installing

You can clone the repository, or download the latest .tar.gz distribution.


The source code follows the standard GNU build process. On linux :

	autoreconf -i
	./configure
	make
	make install


See [Downloading and compiling instructions]( doc/DownloadCompile.md )



## Reporting bugs, issues

Report bugs and issues on [this page]( https://github.com/oguyon/AdaptiveOpticsControl/issues )




## Contributing to project


See [coding standards]( http://oguyon.github.io/AdaptiveOpticsControl/html/page_coding_standards.html ) 




## Documentation

[Full online documentation]( http://oguyon.github.io/AdaptiveOpticsControl/ ) 



## Libraries 

The following libraries are used:

- libtool
- automake
- readline, for reading the command line input
- ncurses-dev
- flex, for parsing the command line input
- bison, to interpret the command line input
- fftw, for performing Fourier Transforms
- gsl, for math functions and tools
- fitsio, for reading and writing FITS image files
- CUDA, CuBLAS, MAGMA for GPU acceleration (optional)

If you use NVIDIA GPUs, install cuda and magma libraries, and add "--enable-cuda and --enable-magma" options to the configure command.


## Getting Started

All functions are accessible from the command line interface (CLI). Enter the CLI and type "help" for instructions.

		./bin/AdaptiveOpticsControl




## LICENCE


GNU General Public License v3.0

[LICENCE.txt]( https://github.com/oguyon/Cfits/blob/master/LICENCE.txt )




## Credits

This software was developed with support from the National Science Foundation (award #1006063), NINS / NAOJ / Subaru Telescope, NINS / Astrobiology Centerand the Japanese Society for the Promotion of Science (JSPS).
