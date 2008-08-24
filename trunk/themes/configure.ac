#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock-themes], [1.6.2-RC3], [Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL

pkgdatadir=[`pkg-config cairo-dock --variable=themesdir`]
AC_SUBST(pkgdatadir)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		_MacOSX_/Makefile
		_MacOSX_/launchers/Makefile
		_MacOSX_/plug-ins/Makefile
		_MacOSX_/plug-ins/clock/Makefile
		_MacOSX_/plug-ins/dustbin/Makefile
		_MacOSX_/plug-ins/rendering/Makefile
		_Wood_/Makefile
		_Wood_/launchers/Makefile
		_Wood_/plug-ins/Makefile
		_Wood_/plug-ins/clock/Makefile
		_Wood_/plug-ins/dustbin/Makefile
		_Wood_/plug-ins/logout/Makefile
		_Wood_/plug-ins/rendering/Makefile
		_Wood_/plug-ins/weather/Makefile
		_Clear_/Makefile
		_Clear_/launchers/Makefile
		_Clear_/plug-ins/Makefile
		_Clear_/plug-ins/logout/Makefile
		_Clear_/plug-ins/weather/Makefile
		_Clear_/plug-ins/clock/Makefile
		_Clear_/plug-ins/cpusage/Makefile
		_Clear_/plug-ins/rendering/Makefile
		_Clear_/plug-ins/compiz-icon/Makefile
		_Clear_/plug-ins/AlsaMixer/Makefile
		_Clear_/plug-ins/shortcuts/Makefile
		_Neon2_/Makefile
		_Neon2_/launchers/Makefile
		_Neon2_/plug-ins/Makefile
		_Neon2_/plug-ins/dustbin/Makefile
		_Neon2_/plug-ins/logout/Makefile
		_Brit_/Makefile
		_Brit_/launchers/Makefile
		_Brit_/plug-ins/Makefile
		_Brit_/plug-ins/AlsaMixer/Makefile
		_Brit_/plug-ins/Cairo-Penguin/Makefile
		_Brit_/plug-ins/clock/Makefile
		_Brit_/plug-ins/compiz-icon/Makefile
		_Brit_/plug-ins/cpusage/Makefile
		_Brit_/plug-ins/dustbin/Makefile
		_Brit_/plug-ins/logout/Makefile
		_Brit_/plug-ins/mail/Makefile
		_Brit_/plug-ins/rendering/Makefile
		_Brit_/plug-ins/shortcuts/Makefile
		_Brit_/plug-ins/showDesktop/Makefile
		_Brit_/plug-ins/stacks/Makefile
		_Brit_/plug-ins/switcher/Makefile
		_Brit_/plug-ins/terminal/Makefile
		_Brit_/plug-ins/Xgamma/Makefile
		])
AC_OUTPUT
