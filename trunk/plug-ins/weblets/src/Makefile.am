 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-weblets.la


libcd_weblets_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-widget.c\
	applet-widget.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_weblets_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS) $(WEBKIT_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(webletsdatadir)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""weblets.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""weblets"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_WEBLETS)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_WEBLETS)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-DCD_APPLET_MULTI_INSTANCE=1
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall

libcd_weblets_la_LIBADD = \
	$(PACKAGE_LIBS) $(WEBKIT_LIBS) -lm


libcd_weblets_la_LDFLAGS =\
	-avoid-version -module
