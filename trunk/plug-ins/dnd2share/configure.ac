#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dnd2share],[0.0.1],[Yann Dulieu (nochka85@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_DND2SHARE=cd-dnd2share
AC_DEFINE_UNQUOTED(GETTEXT_DND2SHARE,"$GETTEXT_DND2SHARE", [Gettext package.])
AC_SUBST(GETTEXT_DND2SHARE)

AC_SUBST(VERSION_DND2SHARE, "$VERSION")
AC_SUBST(PACKAGE_DND2SHARE, "$PACKAGE")

pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/dnd2share]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/dnd2share.conf])
AC_OUTPUT