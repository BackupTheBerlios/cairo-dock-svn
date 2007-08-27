
#ifndef __CAIRO_DOCK_APPLET_FACTORY__
#define  __CAIRO_DOCK_APPLET_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


cairo_surface_t *cairo_dock_create_applet_surface (cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);


Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, GtkWidget *pMenu);  // GList *pMenuEntryList


gboolean cairo_dock_read_header_applet_conf_file (GKeyFile *pKeyFile, int *iWidth, int *iHeight, gchar **cName);


GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur);


#endif

