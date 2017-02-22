bin_PROGRAMS = AdaptiveOpticsControl
AdaptiveOpticsControl_SOURCES = CLIcore.c CLIcore.h initmodules.c initmodules.h calc_bison.y calc_flex.l calc.h
AdaptiveOpticsControl_CPPFLAGS = -I@abs_top_srcdir@/src -DCONFIGDIR=\"@abs_top_srcdir@/config\" -DSOURCEDIR=\"@abs_top_srcdir@/src\" -DABSSRCTOPDIR=\"@abs_top_srcdir@\" -fopenmp

ACLOCAL_AMFLAGS = -I m4

## For Bison and FLEX
AM_YFLAGS = -d
AM_LFLAGS = -i

noinst_HEADERS = calc.h
BUILT_SOURCES = calc_bison.h

SUBDIRS =
AdaptiveOpticsControl_LDADD =



SUBDIRS += cudacomp
AdaptiveOpticsControl_LDADD += cudacomp/libcudacomp.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/cudacomp

SUBDIRS += AOloopControl
AdaptiveOpticsControl_LDADD += AOloopControl/libaoloopcontrol.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/AOloopControl

SUBDIRS += linARfilterPred
AdaptiveOpticsControl_LDADD += linARfilterPred/liblinarfilterpred.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/linARfilterPred

SUBDIRS += AOloopControl_DM
AdaptiveOpticsControl_LDADD += AOloopControl_DM/libaoloopcontroldm.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/AOloopControl_DM

SUBDIRS += AtmosphericTurbulence
AdaptiveOpticsControl_LDADD += AtmosphericTurbulence/libatmosphericturbulence.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/AtmosphericTurbulence

SUBDIRS += AtmosphereModel
AdaptiveOpticsControl_LDADD += AtmosphereModel/libatmospheremodel.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/AtmosphereModel

SUBDIRS += OptSystProp
AdaptiveOpticsControl_LDADD += OptSystProp/liboptsystprop.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/OptSystProp

SUBDIRS += psf
AdaptiveOpticsControl_LDADD += psf/libpsf.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/psf

SUBDIRS += ZernikePolyn
AdaptiveOpticsControl_LDADD += ZernikePolyn/libzernikepolyn.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/ZernikePolyn

SUBDIRS += WFpropagate
AdaptiveOpticsControl_LDADD += WFpropagate/libwfpropagate.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/WFpropagate

SUBDIRS += image_basic
AdaptiveOpticsControl_LDADD += image_basic/libimagebasic.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/image_basic

SUBDIRS += image_filter
AdaptiveOpticsControl_LDADD += image_filter/libimagefilter.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/image_filter

SUBDIRS += kdtree
AdaptiveOpticsControl_LDADD += kdtree/libkdtree.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/kdtree

SUBDIRS += image_gen
AdaptiveOpticsControl_LDADD += image_gen/libimagegen.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/image_gen

SUBDIRS += linopt_imtools
AdaptiveOpticsControl_LDADD += linopt_imtools/liblinoptimtools.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/linopt_imtools

SUBDIRS += statistic
AdaptiveOpticsControl_LDADD += statistic/libstatistic.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/statistic

SUBDIRS += fft
AdaptiveOpticsControl_LDADD += fft/libfft.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/fft

SUBDIRS += info
AdaptiveOpticsControl_LDADD += info/libinfo.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/info

SUBDIRS += COREMOD_arith
AdaptiveOpticsControl_LDADD += COREMOD_arith/libcoremodarith.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/COREMOD_arith

SUBDIRS += COREMOD_iofits
AdaptiveOpticsControl_LDADD += COREMOD_iofits/libcoremodiofits.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/COREMOD_iofits

SUBDIRS += COREMOD_memory
AdaptiveOpticsControl_LDADD += COREMOD_memory/libcoremodmemory.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/COREMOD_memory

SUBDIRS += COREMOD_tools
AdaptiveOpticsControl_LDADD += COREMOD_tools/libcoremodtools.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/COREMOD_tools

SUBDIRS += 00CORE
AdaptiveOpticsControl_LDADD += 00CORE/lib00core.la
AdaptiveOpticsControl_CPPFLAGS += -I@abs_top_srcdir@/src/00CORE

