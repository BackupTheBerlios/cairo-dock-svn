#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-musicPlayer],[1.0.0-alpha],[ChAnGFu (Robertson Rémy)(changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_MUSICPLAYER=cd-musicPlayer
AC_DEFINE_UNQUOTED(GETTEXT_MUSICPLAYER,"$GETTEXT_MUSICPLAYER", [Gettext package.])
AC_SUBST(GETTEXT_MUSICPLAYER)

AC_SUBST(VERSION_MUSICPLAYER, "$VERSION")
AC_SUBST(PACKAGE_MUSICPLAYER, "$PACKAGE")

musicplayerdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/musicPlayer]
AC_SUBST(musicplayerdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/musicPlayer.conf
		data/themes/Makefile
		data/themes/cd_box_simple/Makefile
		])
AC_OUTPUT
