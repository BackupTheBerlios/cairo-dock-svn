
#ifndef __CAIRO_DOCK_DESKTOP_FILE_FACTORY__
#define  __CAIRO_DOCK_DESKTOP_FILE_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


gchar *cairo_dock_add_desktop_file_from_uri_full (gchar *cURI, const gchar *cDockName, double fOrder, CairoDock *pDock, gboolean bIsContainer, GError **erreur);
#define cairo_dock_add_desktop_file_from_uri(cURI, cDockName, fOrder, pDock, erreur) cairo_dock_add_desktop_file_from_uri_full (cURI, cDockName, fOrder, pDock, FALSE, erreur)
#define cairo_dock_add_desktop_file_for_container(cDockName, fOrder, pDock, erreur) cairo_dock_add_desktop_file_from_uri_full (NULL, cDockName, fOrder, pDock, TRUE, erreur)


gchar *cairo_dock_generate_desktop_filename (gchar *cBaseName, gchar *cCairoDockDataDir);


void cairo_dock_update_launcher_desktop_file (gchar *cDesktopFilePath, gboolean bIsContainer);


gchar *cairo_dock_get_launcher_template_conf_file (gboolean bIsContainer);


#endif

