#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-switcher],[1.1.3],[Cchumi (cchumi@live.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SWITCHER=cd-switcher
AC_DEFINE_UNQUOTED(GETTEXT_SWITCHER,"$GETTEXT_SWITCHER", [Gettext package.])
AC_SUBST(GETTEXT_SWITCHER)

AC_SUBST(VERSION_SWITCHER, "$VERSION")
AC_SUBST(PACKAGE_SWITCHER, "$PACKAGE")

switcherdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/switcher]
AC_SUBST(switcherdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/switcher.conf])
AC_OUTPUT
