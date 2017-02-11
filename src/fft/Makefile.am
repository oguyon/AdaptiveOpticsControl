lib_LTLIBRARIES = libfft.la
libfft_la_SOURCES = fft.c fft.h

AM_CPPFLAGS = -I@abs_top_srcdir@/src -DFFTCONFIGDIR=\"@abs_top_srcdir@/src/fft/config\" -fopenmp -lfftw3_omp
