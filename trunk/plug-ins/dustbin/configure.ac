#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dustbin],[2.2.4],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_DUSTBIN=cd-dustbin
AC_DEFINE_UNQUOTED(GETTEXT_DUSTBIN,"$GETTEXT_DUSTBIN", [Gettext package.])
AC_SUBST(GETTEXT_DUSTBIN)

AC_SUBST(VERSION_DUSTBIN, "$VERSION")
AC_SUBST(PACKAGE_DUSTBIN, "$PACKAGE")

dustbinuserdirname="dustbin"
AC_SUBST(dustbinuserdirname)
dustbindatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/dustbin]
AC_SUBST(dustbindatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock gthread-2.0)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/dustbin.conf
		data/themes/Makefile
		data/themes/default/Makefile
		])
AC_OUTPUT
