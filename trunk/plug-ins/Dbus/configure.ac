#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Dbus],[0.5.0],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_DBUS=cd-Dbus
AC_DEFINE_UNQUOTED(GETTEXT_DBUS,"$GETTEXT_DBUS", [Gettext package.])
AC_SUBST(GETTEXT_DBUS)

AC_SUBST(VERSION_DBUS, "$VERSION")
AC_SUBST(PACKAGE_DBUS, "$PACKAGE")

dbusdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Dbus]
AC_SUBST(dbusdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/Dbus.conf])
AC_OUTPUT
