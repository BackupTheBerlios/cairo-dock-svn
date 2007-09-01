
#ifndef __CAIRO_DOCK_DESKTOP_FILE_FACTORY__
#define  __CAIRO_DOCK_DESKTOP_FILE_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


gchar *cairo_dock_add_desktop_file_from_path (gchar *cFilePath, gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur);


gchar *cairo_dock_generate_desktop_filename (gchar *cCairoDockDataDir);


void cairo_dock_update_lanucher_desktop_file (gchar *cDesktopFilePath, gchar *cLanguage);


#endif

