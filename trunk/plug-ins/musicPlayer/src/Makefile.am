 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-musicPlayer.la


libcd_musicPlayer_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-dbus.c\
	applet-dbus.h\
	applet-draw.c\
	applet-draw.h\
	applet-musicplayer.c\
	applet-musicplayer.h\
	applet-exaile.c\
	applet-exaile.h\
	applet-songbird.c\
	applet-songbird.h\
	applet-xmms.c\
	applet-xmms.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_musicPlayer_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(musicplayerdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""musicPlayer.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""musicPlayer"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_MUSICPLAYER)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_MUSICPLAYER)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_musicPlayer_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_musicPlayer_la_LDFLAGS =\
	-avoid-version