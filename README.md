[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.815633.svg)](https://doi.org/10.5281/zenodo.815633)




# Adaptive Optics Computation Engine (AOCCE)



Adaptive Optics Wavefront control tools for high contrast imaging

Uses shared memory for fast low-latency communication to hardware

Includes a simulator of hardware (DM, Camera): the same code can be tested on actual hardware or in a simulation mode.

Supports multiple simultaneously running control loops and data logging.





## Downloading source code
You can clone this repository, or download the latest .tar.gz distribution.


## Libraries 

Note: please consult [online documentation]( http://oguyon.github.io/AdaptiveOpticsControl/) for detailed instructions.

- readline, for reading the command line input
- flex, for parsing the command line input
- bison, to interpret the command line input
- fftw, for performing Fourier Transforms
- gsl, for math functions and tools
- fitsio
- CUDA/CuBLAS and MAGMA for GPU acceleration



## Detailed documentation 

Please consult the [online documentation]( http://oguyon.github.io/AdaptiveOpticsControl/).




## Getting Started

All functions are accessible from the command line interface (CLI). Enter the CLI and type "help" for instructions.

		./bin/AdaptiveOpticsControl



## Credits

This software was developed with support from the National Science Foundation (award #1006063), NINS / NAOJ / Subaru Telescope, NINS / Astrobiology Centerand the Japanese Society for the Promotion of Science (JSPS).
