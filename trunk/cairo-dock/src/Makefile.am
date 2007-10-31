
SUBDIRS = .

bin_PROGRAMS = \
	cairo-dock


if COMPIL_VERBOSE
include_verbose = -DCAIRO_DOCK_VERBOSE=TRUE
endif
if COMPIL_WITH_GLITZ
include_glitz = -DHAVE_GLITZ=TRUE
glitz_cflags = $(GLITZ_CFLAGS)
glitz_libs = $(GLITZ_LIBS)
endif

cairo_dock_CFLAGS = \
	$(PACKAGE_CFLAGS) \
	$(glitz_cflags) \
	$(include_verbose)\
	$(include_glitz)\
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
	$(glitz_libs) \
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
	cairo-dock-applications-manager.c \
	cairo-dock-applications-manager.h \
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
	cairo-dock-animations.h\
	cairo-dock-themes-manager.c\
	cairo-dock-themes-manager.h\
	cairo-dock-notifications.c\
	cairo-dock-notifications.h\
	cairo-dock-dialogs.c\
	cairo-dock-dialogs.h\
	cairo-dock-gui-factory.c\
	cairo-dock-gui-factory.h\
	cairo-dock-surface-factory.c\
	cairo-dock-surface-factory.h\
	cairo-dock-renderer-manager.c\
	cairo-dock-renderer-manager.h\
	cairo-dock-default-view.c\
	cairo-dock-default-view.h
	

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
	cairo-dock-applications-manager.h\
	cairo-dock-menu.h\
	cairo-dock-modules.h\
	cairo-dock-global-variables.h\
	cairo-dock-launcher-factory.h\
	cairo-dock-application-factory.h\
	cairo-dock-applet-factory.h\
	cairo-dock-separator-factory.h\
	cairo-dock-desktop-file-factory.h\
	cairo-dock-keyfile-manager.h\
	cairo-dock-dock-factory.h\
	cairo-dock-themes-manager.h\
	cairo-dock-animations.h\
	cairo-dock-notifications.h\
	cairo-dock-dialogs.h\
	cairo-dock-gui-factory.h\
	cairo-dock-surface-factory.h\
	cairo-dock-renderer-manager.h\
	cairo-dock-default-view.h
