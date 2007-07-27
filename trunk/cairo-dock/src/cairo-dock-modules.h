
#ifndef __CAIRO_DOCK_MODULES__
#define  __CAIRO_DOCK_MODULES__

#include <glib.h>

#include "cairo-dock-struct.h"


gchar *cairo_dock_extract_module_name_from_path (gchar *cSoFilePath);

CairoDockModule * cairo_dock_load_module (gchar *cSoFilePath, GHashTable *pModuleTable, GError **erreur);

void cairo_dock_preload_module_from_directory (gchar *cModuleDirPath, GHashTable *pModuleTable, GError **erreur);

void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, GHashTable *pModuleTable, GtkWidget *pWidget);



void cairo_dock_free_module (CairoDockModule *module);

Icon * cairo_dock_activate_module (CairoDockModule *module, cairo_t *pCairoContext, GError **erreur);

void cairo_dock_deactivate_module (CairoDockModule *module);

void cairo_dock_configure_module (CairoDockModule *module, GtkWidget *pWidget, GError **erreur);


Icon *cairo_dock_find_icon_from_module (CairoDockModule *module);


#endif
