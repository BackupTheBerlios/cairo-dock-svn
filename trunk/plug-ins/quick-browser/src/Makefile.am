 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-quick-browser.la


libcd_quick_browser_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-menu.c\
	applet-menu.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_quick_browser_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(quick_browserdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""quick-browser.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""quick-browser"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_QUICK_BROWSER)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-DCD_APPLET_MULTI_INSTANCE=\"1\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_quick_browser_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_quick_browser_la_LDFLAGS =\
	-avoid-version -module
