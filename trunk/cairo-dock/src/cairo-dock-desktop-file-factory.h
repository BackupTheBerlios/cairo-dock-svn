
#ifndef __CAIRO_DOCK_DESKTOP_FILE_FACTORY__
#define  __CAIRO_DOCK_DESKTOP_FILE_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


gchar *cairo_dock_add_desktop_file_from_uri (gchar *cURI, const gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur);


gchar *cairo_dock_generate_desktop_filename (gchar *cBaseName, gchar *cCairoDockDataDir);


void cairo_dock_update_launcher_desktop_file (gchar *cDesktopFilePath, gchar *cLanguage);


gchar *cairo_dock_get_launcher_template_conf_file (void);


#endif

