 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-Xgamma.la


libcd_Xgamma_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-struct.h\
	applet-xgamma.c\
	applet-xgamma.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_Xgamma_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(XGAMMA_PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(xgammadatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""Xgamma.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""Xgamma"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_XGAMMA)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_XGAMMA)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_Xgamma_la_LIBADD = \
	$(PACKAGE_LIBS) -lm \
	$(XGAMMA_PACKAGE_LIBS)


libcd_Xgamma_la_LDFLAGS =\
	-avoid-version -module
