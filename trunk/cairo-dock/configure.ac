#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock], [1.4.4], [Fabrice Rey (fabounet_03@yahoo.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL


# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.


AM_CONDITIONAL(COMPIL_PLUG_IN, test x$enable_plug_in = xyes)
AM_CONDITIONAL(COMPIL_VERBOSE, test x$enable_verbose = xyes)
AM_CONDITIONAL(COMPIL_WITH_GLITZ, test x$enable_glitz = xyes)


PKG_CHECK_MODULES(PACKAGE, "gtk+-2.0 cairo librsvg-2.0 glitz")
PKG_CHECK_MODULES(GLITZ, "glitz-glx")

AC_CONFIG_FILES([Makefile
		cairo-dock.pc
		src/Makefile
		data/Makefile
		data/themes/Makefile
		data/themes/_default_/Makefile
		data/themes/_default_/launchers/Makefile
		data/themes/_Azur_/Makefile
		data/themes/_Azur_/launchers/Makefile
		data/themes/_Ubuntu_/Makefile
		data/themes/_Ubuntu_/launchers/Makefile
		data/themes/_Verde_/Makefile
		data/themes/_Verde_/launchers/Makefile
		data/themes/_Djoole_/Makefile
		data/themes/_Djoole_/launchers/Makefile
		data/themes/_Cobalt_/Makefile
		data/themes/_Cobalt_/launchers/Makefile
		data/themes/_Glattering_/Makefile
		data/themes/_Glattering_/launchers/Makefile
		])
AC_OUTPUT
