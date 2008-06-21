#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock], [1.6.0.2], [Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL

IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cairo-dock
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([math.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_C_CONST
# AC_TYPE_PID_T

# Checks for library functions.
# AC_CHECK_FUNCS([sscanf])

PKG_CHECK_MODULES(PACKAGE, "gtk+-2.0 gthread-2.0 cairo librsvg-2.0 glitz dbus-1 dbus-glib-1 libxml-2.0 xtst")


AC_ARG_ENABLE(glitz,
	[  --enable-glitz  :  Turn on hardware acceleration],
	[case "${enableval}" in
		yes) useglitz=true ;;
		no)  useglitz=false ;;
		*) echo "bad value ${enableval} for --enable-glitz" ;;
	esac],[useglitz=false])

if test "$useglitz" = "true" ; then
	PKG_CHECK_MODULES(GLITZ,glitz-glx, [glitz_detected=true], [ glitz_detected=false; AC_MSG_WARN(Could not find glitz-glx; Cairo-Dock won't be built with hardware acceleration.) ])
	AC_SUBST(GLITZ_CFLAGS)
	AC_SUBST(GLITZ_LIBS)
	
	if test ! "$glitz_detected" = "false" ; then
		AC_DEFINE(HAVE_GLITZ,"1",[Defined if we can use glitz.])
	fi
fi
AM_CONDITIONAL( [WITH_GLITZ], [test "$useglitz" = "true"] )


AC_CONFIG_FILES([Makefile
		cairo-dock.pc
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/cairo-dock.conf
		data/main-dock.conf
		data/default-theme/Makefile
		data/default-theme/launchers/Makefile
		data/gauges/Makefile
		data/gauges/radium/Makefile
		data/gauges/radium-fuel/Makefile
		data/gauges/battery/Makefile
		data/gauges/old-square/Makefile
		data/gauges/turbo-night/Makefile
		data/gauges/turbo-night-fuel/Makefile
		data/gauges/turbo-night-dual/Makefile
		data/gauges/rainbow/Makefile
		data/gauges/tomato/Makefile
		data/emblems/Makefile])
AC_OUTPUT
