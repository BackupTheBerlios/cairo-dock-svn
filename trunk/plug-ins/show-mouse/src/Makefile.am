 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-show_mouse.la


libcd_show_mouse_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-struct.h\
	star-tex.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_show_mouse_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(show_mousedatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""show_mouse.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""show_mouse"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_SHOW_MOUSE)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_SHOW_MOUSE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
        -DGL_GLEXT_PROTOTYPES=\""1"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_show_mouse_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_show_mouse_la_LDFLAGS =\
	-avoid-version -module
