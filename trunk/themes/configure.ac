#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock-themes], [1.5.4], [Fabrice Rey (fabounet@users.berlios.de)])
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
		_Azur_/Makefile
		_Azur_/launchers/Makefile
		_Ubuntu_/Makefile
		_Ubuntu_/launchers/Makefile
		_Verde_/Makefile
		_Verde_/launchers/Makefile
		_Djoole_/Makefile
		_Djoole_/launchers/Makefile
		_Cobalt_/Makefile
		_Cobalt_/launchers/Makefile
		_Glattering_/Makefile
		_Glattering_/launchers/Makefile
		_TapisVert_/Makefile
		_TapisVert_/launchers/Makefile
		_TapisVert_/plug-ins/Makefile
		_TapisVert_/plug-ins/rendering/Makefile
		_TapisVert_/plug-ins/logout/Makefile
		_TapisVert_/plug-ins/dustbin/Makefile
		_MacOSX_/Makefile
		_MacOSX_/launchers/Makefile
		_MacOSX_/plug-ins/Makefile
		_MacOSX_/plug-ins/clock/Makefile
		_MacOSX_/plug-ins/dustbin/Makefile
		_MacOSX_/plug-ins/rendering/Makefile
		_I_Cairo_/Makefile
		_I_Cairo_/launchers/Makefile
		_I_Cairo_/plug-ins/Makefile
		_I_Cairo_/plug-ins/clock/Makefile
		_I_Cairo_/plug-ins/dustbin/Makefile
		_I_Cairo_/plug-ins/logout/Makefile
		_I_Cairo_/plug-ins/rendering/Makefile
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
		])
#		_Neon2_/Makefile
#		_Neon2_/launchers/Makefile
#		_Neon2_/plug-ins/Makefile
#		_Neon2_/plug-ins/dustbin/Makefile
#		_Neon2_/plug-ins/logout/Makefile
AC_OUTPUT
