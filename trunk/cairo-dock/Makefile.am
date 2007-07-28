#dnl Process this file with automake to produce Makefile.in

if COMPIL_WITH_GLITZ
HAVE_GLITZ = true
endif

if COMPIL_VERBOSE
CAIRO_DOCK_VERBOSE = true
endif


SUBDIRS = . src data


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cairo-dock.pc

scriptdir = ${bindir}
script_DATA = launch-cairo-dock-after-beryl.sh

