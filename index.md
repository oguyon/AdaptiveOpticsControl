
# Installation


## Downloading source code
You can clone this repository, or download the latest .tar.gz distribution.


## Libraries 

### Standard libraries

- readline, for reading the command line input
- flex, for parsing the command line input
- bison, to interpret the command line input
- fftw, for performing Fourier Transforms
- gsl, for math functions and tools

Install above libraries (centOS):

		sudo yum install readline-devel flex bison-devel fftw3-devel gsl-devel

Install above libraries (Ubuntu):

		sudo apt-get install libcfitsio3 libcfitsio3-dev libreadline6-dev libncurses5-dev libfftw3-dev libgsl0-dev flex bison


### FITSIO

For reading and writing FITS image files

- Visit https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html and download the file Unix .tar file cfitsio3410.tar.gz
- Extract it , README , install it 
There is the fitsio.h in it. Move it to usr :

		./configure --prefix=/usr
		make 
		sudo make install 

### GPU acceleration (optional, but highly recommended)

- install NVIDIA driver
- install CUDA
- install MAGMA



## Compilation from git clone (recommended for developers)

### Additional libraries

#### CentOS

Install Development tools, use the command bellow. This will search the yum repositories, and install the tools from the closest repo.

		sudo yum groupinstall "Development tools"

#### Ubuntu

		sudo apt-get install autoconf libtool git


### Compilation

The source code follows the standard GNU build process:

		autoreconf -vif
		./configure
		make
		make install


## Compilation from tarball (recommended for users)


Unpack

		gunzip <package>.tar.gz
		tar -xvf <package>.tar

The source code follows the standard GNU build process:

		./configure
		make
		sudo make install








# User's guide

[AOCCE user's guide]( src/AOloopControl/doc/AOloopControl.html )







# Programmer's guide

[Source code documentation]( html/index.html ) 

