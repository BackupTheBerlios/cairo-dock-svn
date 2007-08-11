
SUBDIRS = .

bin_PROGRAMS = \
	cairo-dock


if COMPIL_VERBOSE
include_verbose = -DCAIRO_DOCK_VERBOSE=TRUE
endif

cairo_dock_CFLAGS = \
	$(PACKAGE_CFLAGS) \
	$(include_verbose)\
	-DCAIRO_DOCK_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DCAIRO_DOCK_SHARE_THEMES_DIR=\""$(pkgdatadir)/themes"\"\
	-DCAIRO_DOCK_MODULES_DIR=\""$(pkgdatadir)/plug-in"\"\
	-DCAIRO_DOCK_CONF_FILE=\""cairo-dock.conf"\"\
	-DCAIRO_DOCK_VERSION=\""$(VERSION)"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall -Wno-unused-parameter

cairo_dock_LDFLAGS = \
	-Wl,--export-dynamic

cairo_dock_LDADD = \
	$(PACKAGE_LIBS)

cairo_dock_SOURCES = \
	cairo-dock-struct.h\
	cairo-dock-global-variables.h \
	cairo-dock-callbacks.c \
	cairo-dock-callbacks.h \
	cairo-dock.c \
	cairo-dock-draw.c \
	cairo-dock-draw.h \
	cairo-dock-config.c \
	cairo-dock-config.h \
	cairo-dock-load.c \
	cairo-dock-load.h \
	cairo-dock-icons.h \
	cairo-dock-icons.c \
	cairo-dock-applications.c \
	cairo-dock-applications.h \
	cairo-dock-menu.c \
	cairo-dock-menu.h \
	cairo-dock-modules.c \
	cairo-dock-modules.h\
	cairo-dock-launcher-factory.c\
	cairo-dock-launcher-factory.h\
	cairo-dock-application-factory.c\
	cairo-dock-application-factory.h\
	cairo-dock-applet-factory.c\
	cairo-dock-applet-factory.h\
	cairo-dock-separator-factory.c\
	cairo-dock-separator-factory.h\
	cairo-dock-desktop-file-factory.c\
	cairo-dock-desktop-file-factory.h\
	cairo-dock-keyfile-manager.c\
	cairo-dock-keyfile-manager.h\
	cairo-dock-dock-factory.c\
	cairo-dock-dock-factory.h\
	cairo-dock-animations.c\
	cairo-dock-animations.h
	

cairo_dockincludedir =\
	$(includedir)/cairo-dock

cairo_dockinclude_HEADERS =\
	cairo-dock.h

cairo_docksubincludedir =\
	$(includedir)/cairo-dock/cairo-dock

cairo_docksubinclude_HEADERS =\
	cairo-dock-struct.h\
	cairo-dock-callbacks.h\
	cairo-dock-draw.h\
	cairo-dock-config.h\
	cairo-dock-load.h\
	cairo-dock-icons.h\
	cairo-dock-applications.h\
	cairo-dock-menu.h\
	cairo-dock-modules.h\
	cairo-dock-global-variables.h\
	cairo-dock-launcher-factory.h\
	cairo-dock-application-factory.h\
	cairo-dock-applet-factory.h\
	cairo-dock-separator-factory.h\
	cairo-dock-desktop-file-factory.h\
	cairo-dock-keyfile-manager.h\
	cairo-dock-dock-factory.h
