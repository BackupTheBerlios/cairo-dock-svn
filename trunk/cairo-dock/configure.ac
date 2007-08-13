#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock], [1.0.5], [Fabrice Rey (fabounet_03@yahoo.fr)])
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


AC_CONFIG_FILES([Makefile
		cairo-dock.pc
		src/Makefile
		data/Makefile
		data/themes/Makefile
		data/themes/default/Makefile
		data/themes/Fabounet/Makefile
		data/themes/Ubuntu/Makefile
		data/themes/Chilperik/Makefile
		data/themes/Djoole/Makefile
		])
AC_OUTPUT
