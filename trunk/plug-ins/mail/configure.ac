#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-mail],[0.0.1],[Christophe Chapuis (chris.chapuis@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cd-mail
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/mail]
AC_SUBST(pkgdatadir)


# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpopt':

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h math.h netdb.h netinet/in.h signal.h stddef.h string.h sys/select.h sys/socket.h sys/stat.h sys/types.h sys/wait.h time.h unistd.h)
AC_CHECK_FUNCS(mmap sigaction srandom)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([sscanf])

PKG_CHECK_MODULES(PACKAGE, cairo-dock gnutls)

# SSL is present ? let's say yes :)
AC_DEFINE_UNQUOTED([HAVE_SSL_SUPPORT], [1], [Define if SSL support is enabled])

# source will be posix compliant
AC_DEFINE_UNQUOTED([_POSIX_SOURCE], [1], [source will be posix compliant])

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/mail.conf])
AC_OUTPUT